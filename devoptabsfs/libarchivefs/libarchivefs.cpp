/* based on https://gist.github.com/averne/527f3a739d19c8d573b2d6995a33edea */


#include "libarchivefs.h"
#ifdef BUILD_ARCHIVE
#include <filesystem>


CARCHFS::CARCHFS(std::string _url,std::string _name,std::string _mount_name){
	this->connect_url = _url;
	
	
	this->name       = _name;
    this->mount_name = _mount_name;
	
    this->devoptab = {
        .name         = CARCHFS::name.data(),

        .structSize   = sizeof(CARCHFSFile),
        .open_r       = CARCHFS::archfs_open,
        .close_r      = CARCHFS::archfs_close,
        .read_r       = CARCHFS::archfs_read,
        .seek_r       = CARCHFS::archfs_seek,
        .fstat_r      = CARCHFS::archfs_fstat,

        .stat_r       = CARCHFS::archfs_stat,
        .chdir_r      = CARCHFS::archfs_chdir,

        .dirStateSize = sizeof(CARCHFSDir),
        .diropen_r    = CARCHFS::archfs_diropen,
        .dirreset_r   = CARCHFS::archfs_dirreset,
        .dirnext_r    = CARCHFS::archfs_dirnext,
        .dirclose_r   = CARCHFS::archfs_dirclose,

        .statvfs_r    = CARCHFS::archfs_statvfs,

        .deviceData   = this,

        .lstat_r      = CARCHFS::archfs_stat,
    };
	
	if(connect() == 0){
		printf("ARCHFS REGISTRING\n");
		register_fs();
	}
}

CARCHFS::~CARCHFS(){
	//auto lk = std::scoped_lock(this->session_mutex);
	if (this->is_connected)
        this->disconnect();
	
	unregister_fs();
	
}

void CARCHFS::disconnect(){
	
}


int CARCHFS::connect(){
	
	
	FILE *arch_file = fopen(connect_url.c_str(), "rb");
	struct archive *arch_ctx = archive_read_new();
	archive_read_support_filter_all(arch_ctx);
	archive_read_support_format_all(arch_ctx);
	int ret = archive_read_open_FILE(arch_ctx, arch_file);
	
	
	
	if(ret!= ARCHIVE_OK){
		return -1;
	}
	while (true) {
		struct archive_entry *entry = archive_entry_new();
		
		archfileentry_struct archentry;
		if(archive_read_next_header2(arch_ctx, entry) != ARCHIVE_OK)break;
		
		std::string fullpathname = archive_entry_pathname(entry);
		//size_t pos = fullpathname.find_last_of("/");
		std::string myfilename = fullpathname.substr(fullpathname.find_last_of("/")+1);
		std::string parentname =  "/" + fullpathname.substr(0,fullpathname.find_last_of("/"));
		
		if(myfilename == ""){
			myfilename = parentname.substr(parentname.find_last_of("/")+1);
			parentname = parentname.substr(0,parentname.find_last_of("/"));
		}
		
		
		if("/" + myfilename == parentname)parentname = "";
		
		parentname = parentname +"/";
		
		
		archentry.filename = myfilename;
		archentry.parent = parentname;
		printf("FILE: %s -> %s\n",archentry.filename.c_str(),archentry.parent.c_str());
		memcpy(&archentry.st,archive_entry_stat(entry),sizeof(struct stat));
		totalfilelist.push_back(archentry);
		archive_read_data_skip(arch_ctx);
		archive_entry_free(entry);
	}
	archive_read_close(arch_ctx);
	archive_read_free(arch_ctx);
	fclose(arch_file);
	is_connected = true;
	
	return 0;
}


int       CARCHFS::archfs_open     (struct _reent *r, void *fileStruct, const char *path, int flags, int mode){
	auto *priv      = static_cast<CARCHFS     *>(r->deviceData);
    auto *priv_file = static_cast<CARCHFSFile *>(fileStruct);

	if(std::string(path).empty()){
		return -1;
	}

	std::filesystem::path const p(path);
	
    auto lk = std::scoped_lock(priv->session_mutex);
	priv_file->arch_ctx = archive_read_new();
	archive_read_support_filter_all(priv_file->arch_ctx);
	archive_read_support_format_all(priv_file->arch_ctx);
	priv_file->arch_file = fopen(priv->connect_url.c_str(), "rb");
	//priv_file->int_arch_file = open(priv->connect_url.c_str(),O_RDONLY );
	//if(priv_file->int_arch_file == -1){
	//	printf("Error Opening FD for %s\r\n",priv->connect_url.c_str());
	//}
	//int ret = archive_read_open_filename(priv_file->arch_ctx,priv->connect_url.c_str(),4096);
	int ret = archive_read_open_FILE(priv_file->arch_ctx, priv_file->arch_file);
	if(ret == ARCHIVE_FATAL){
		printf("UNABLE TO OPEN ARCHIVE\r\n");
	}
	
	priv_file->entry = archive_entry_new2( priv_file->arch_ctx );
	priv_file->offset = 0;
	while (true) {
		
		archfileentry_struct archentry;
		if(archive_read_next_header2(priv_file->arch_ctx, priv_file->entry) != ARCHIVE_OK){
			archive_read_free(priv_file->arch_ctx);
			return -1;
		}
		if(p.filename() == std::string(archive_entry_pathname(priv_file->entry))){
			break;
		}
	
	}
	
	if(archive_entry_size(priv_file->entry) < 100000000){
		printf("MEMFILE\r\n");
		priv_file->memfile = true;
		memcpy(&priv_file->filest,archive_entry_stat(priv_file->entry),sizeof(struct stat));
		//priv_file->filest = archive_entry_stat(priv_file->entry);
		priv_file->filebuffer = (uint8_t *)malloc(priv_file->filest.st_size);
		archive_read_data(priv_file->arch_ctx, (uint8_t *)priv_file->filebuffer, priv_file->filest.st_size);
	}
	
	
	return 0;
	
}


int       CARCHFS::archfs_close    (struct _reent *r, void *fd){
	auto *priv      = static_cast<CARCHFS     *>(r->deviceData);
    auto *priv_file = static_cast<CARCHFSFile *>(fd);

    auto lk = std::scoped_lock(priv->session_mutex);

	if(priv_file->memfile){
		free(priv_file->filebuffer);
		
	}
	
	
	archive_entry_free(priv_file->entry);
	archive_read_close(priv_file->arch_ctx);
	archive_read_free(priv_file->arch_ctx);
	
    fclose(priv_file->arch_file);
	
    return 0;
}

ssize_t   CARCHFS::archfs_read     (struct _reent *r, void *fd, char *ptr, size_t len){
	auto *priv      = static_cast<CARCHFS     *>(r->deviceData);
    auto *priv_file = static_cast<CARCHFSFile *>(fd);

	
    auto lk = std::scoped_lock(priv->session_mutex);
	//archive_seek_data(priv_file->arch_ctx, priv_file->offset,SEEK_END);
	
	if(priv_file->memfile){
		if(len+priv_file->offset>priv_file->filest.st_size)len = priv_file->filest.st_size-priv_file->offset;
		memcpy(ptr,priv_file->filebuffer+priv_file->offset,len);
		priv_file->offset=priv_file->offset+len;
		return (ssize_t)len;
	}else{
		
		ssize_t bytes = archive_read_data(priv_file->arch_ctx, ptr, len);
		priv_file->offset=priv_file->offset+bytes;
		return bytes;

	}
	
	return -1;
}


off_t     CARCHFS::archfs_seek     (struct _reent *r, void *fd, off_t pos, int dir){
	auto *priv      = static_cast<CARCHFS     *>(r->deviceData);
    auto *priv_file = static_cast<CARCHFSFile *>(fd);


	
    off_t offset;
    switch (dir) {
        default:
        case SEEK_SET:
            offset = 0;
            break;
        case SEEK_CUR:
            offset = priv_file->offset;
            break;
        case SEEK_END:
            offset = priv_file->filest.st_size;
            break;
    }

	auto lk = std::scoped_lock(priv->session_mutex);
	if(priv_file->memfile){
		priv_file->offset = offset + pos;
		printf("SEEK : %d %d %d\r\n",priv_file->offset,pos,dir);		
	}else{
		
		priv_file->offset = offset + pos;
		int retseek = archive_seek_data(priv_file->arch_ctx,priv_file->offset,dir);
	
	}
	
	
	
    return priv_file->offset;
}

int       CARCHFS::archfs_fstat    (struct _reent *r, void *fd, struct stat *st){
	auto *ctx = static_cast<CARCHFS *>(r->deviceData);
	auto *priv_file = static_cast<CARCHFSFile *>(fd);
    auto lk = std::scoped_lock(ctx->session_mutex);
	memcpy(st,&priv_file->filest,sizeof(struct stat));
	//st = (struct stat *)archive_entry_stat(priv_file->entry);
	//archive_entry_copy_stat(priv_file->entry,st);
	printf("FSTAT: %d\r\n",st->st_size);
	return 0;
}

int       CARCHFS::archfs_stat     (struct _reent *r, const char *file, struct stat *st){
	auto *priv     = static_cast<CARCHFS    *>(r->deviceData);
	char* colonPos = strchr(file, ':');
    if (colonPos) file = colonPos+1;
	auto lk = std::scoped_lock(priv->session_mutex);
	int rc = priv->find_fileentry(file);
	if(rc<0){
		
		return -1;
	}
	
	memcpy(st,&priv->totalfilelist[rc].st,sizeof(struct stat));
	printf("STAT: %d\r\n",st->st_size);
	
	return 0;
}
int       CARCHFS::archfs_chdir    (struct _reent *r, const char *name){
	auto *priv = static_cast<CARCHFS *>(r->deviceData);

    if (!name) {
        __errno_r(r) = EINVAL;
        return -1;
    }

    priv->cwd = name;

    return 0;
}

DIR_ITER * CARCHFS::archfs_diropen  (struct _reent *r, DIR_ITER *dirState, const char *path){
	auto *priv = static_cast<CARCHFS *>(r->deviceData);
	auto *priv_dir = static_cast<CARCHFSDir *>(dirState->dirStruct);
	
	char* colonPos = strchr(path, ':');
    if (colonPos) path = colonPos+1;
	
	std::string mypath = path;
	int pos = mypath.find_last_of("\\/");
	if(pos != mypath.length()-1)mypath = mypath+"/";
	
	auto lk = std::scoped_lock(priv->session_mutex);
	
	memset(priv_dir->dirpath,0,256);
	strncpy(priv_dir->dirpath,mypath.c_str(),mypath.length());
	priv_dir->dirnext_idx = 0;
	
	
	
	return dirState;
}

int       CARCHFS::archfs_dirreset (struct _reent *r, DIR_ITER *dirState){
	__errno_r(r) = ENOSYS;
    return -1;
}

int       CARCHFS::archfs_dirnext  (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat){
	auto *priv     = static_cast<CARCHFS    *>(r->deviceData);
    auto *priv_dir = static_cast<CARCHFSDir *>(dirState->dirStruct);

    auto lk = std::scoped_lock(priv->session_mutex);
	
	while(true){
	
		if(priv_dir->dirnext_idx>=priv->totalfilelist.size()){
			return -1;
		}
		
		if(priv->totalfilelist[priv_dir->dirnext_idx].parent != std::string(priv_dir->dirpath)){
			priv_dir->dirnext_idx +=1;
			continue;
		}
		
		memset(filename, 0, NAME_MAX);
		memcpy(filename,priv->totalfilelist[priv_dir->dirnext_idx].filename.data(),priv->totalfilelist[priv_dir->dirnext_idx].filename.length());
		
		memcpy(filestat,&priv->totalfilelist[priv_dir->dirnext_idx].st,sizeof(struct stat));

		
		priv_dir->dirnext_idx +=1;
		break;
	}
		
	return 0;
}

int       CARCHFS::archfs_dirclose (struct _reent *r, DIR_ITER *dirState){
	auto *priv     = static_cast<CARCHFS    *>(r->deviceData);
    
	auto lk = std::scoped_lock(priv->session_mutex);
	
	
	
	return 0;
}

int       CARCHFS::archfs_statvfs  (struct _reent *r, const char *path, struct statvfs *buf){
			
	
	return 0;
}


int CARCHFS::find_fileentry(std::string filepath){
	for(int i=0;i<totalfilelist.size();i++){
		if(filepath == totalfilelist[i].parent+totalfilelist[i].filename || filepath == totalfilelist[i].parent+totalfilelist[i].filename + "/"){
			return i;
		}
	}
	return -1;
}

#endif