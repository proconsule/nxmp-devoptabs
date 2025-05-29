#include "iso9660fs.h"
#include <regex>
#include <filesystem>

void isostat_translate_entry(iso9660_stat_t  *entry, struct stat *st);


CISO9660FS::CISO9660FS(std::string _url,std::string _name,std::string _mount_name){
	this->connect_url = _url;
	
	
	this->name       = _name;
    this->mount_name = _mount_name;

    this->devoptab = {
        .name         = CISO9660FS::name.data(),

        .structSize   = sizeof(CISO9660File),
        .open_r       = CISO9660FS::iso9660fs_open,
        .close_r      = CISO9660FS::iso9660fs_close,
        .read_r       = CISO9660FS::iso9660fs_read,
        .seek_r       = CISO9660FS::iso9660fs_seek,
        .fstat_r      = CISO9660FS::iso9660fs_fstat,

        .stat_r       = CISO9660FS::iso9660fs_stat,
        .chdir_r      = CISO9660FS::iso9660fs_chdir,

        .dirStateSize = sizeof(CISO9660Dir),
        .diropen_r    = CISO9660FS::iso9660fs_diropen,
        .dirreset_r   = CISO9660FS::iso9660fs_dirreset,
        .dirnext_r    = CISO9660FS::iso9660fs_dirnext,
        .dirclose_r   = CISO9660FS::iso9660fs_dirclose,

        .statvfs_r    = CISO9660FS::iso9660fs_statvfs,

        .deviceData   = this,

        .lstat_r      = CISO9660FS::iso9660fs_stat,
    };
	
	connect();
	
}

CISO9660FS::~CISO9660FS(){
	if(ISOReader!=nullptr)delete ISOReader;
	if(fs_regisered){
		unregister_fs();
	}
}

bool CISO9660FS::connect(){
	ISOReader = new CISOReader(connect_url);
	if(!ISOReader->valid)return false;
	register_fs();
	this->fs_regisered = true;
	
	return true;
}


int CISO9660FS::iso9660fs_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode) {
    auto *priv      = static_cast<CISO9660FS     *>(r->deviceData);
    auto *priv_file = static_cast<CISO9660File *>(fileStruct);

	auto lk = std::scoped_lock(priv->session_mutex);
	
	std::filesystem::path const p( std::regex_replace(std::string(path), std::regex("^"+ priv->name), ""));
	std::string mypath = p.filename().c_str();
	
	memset(priv_file->filename,0,255*sizeof(char));
	memcpy(priv_file->filename,mypath.c_str(),mypath.length());
	iso9660_stat_t * p_statbuf =  iso9660_ifs_stat_translate (priv->ISOReader->getHandle(), priv_file->filename);
    isostat_translate_entry(p_statbuf,&priv_file->filest);
	priv_file->offset = 0;
	iso9660_stat_free(p_statbuf);

	return 0;
}

ssize_t CISO9660FS::iso9660fs_read(struct _reent *r, void *fd, char *ptr, size_t len) {
    auto *priv      = static_cast<CISO9660FS     *>(r->deviceData);
    auto *priv_file = static_cast<CISO9660File *>(fd);

    auto lk = std::scoped_lock(priv->session_mutex);

	
	int ret = priv->ISOReader->ISO_ReadFileData(priv_file->filename,priv_file->offset,len,ptr);
	if(ret == -1)return -1;
	
	priv_file->offset = priv_file->offset+len;
	
	return len;
}

off_t CISO9660FS::iso9660fs_seek(struct _reent *r, void *fd, off_t pos, int dir) {
    auto *priv      = static_cast<CISO9660FS     *>(r->deviceData);
    auto *priv_file = static_cast<CISO9660File *>(fd);

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
    priv_file->offset = offset + pos;

	return priv_file->offset;
}

int CISO9660FS::iso9660fs_fstat(struct _reent *r, void *fd, struct stat *st) {
    auto *priv_file = static_cast<CISO9660File *>(fd);
	memcpy(st,&priv_file->filest,sizeof(struct stat));
	return 0;
}

int CISO9660FS::iso9660fs_stat(struct _reent *r, const char *file, struct stat *st) {
    auto *priv = static_cast<CISO9660FS *>(r->deviceData);
	
    auto lk = std::scoped_lock(priv->session_mutex);

	
	std::string filename = std::regex_replace(std::string(file), std::regex("^"+ priv->name), "");
	iso9660_stat_t * p_statbuf =  iso9660_ifs_stat_translate (priv->ISOReader->getHandle(), filename.c_str());
    isostat_translate_entry(p_statbuf,st);
	iso9660_stat_free(p_statbuf);
	
    return 0;
}

int CISO9660FS::iso9660fs_chdir(struct _reent *r, const char *name) {
    auto *priv = static_cast<CISO9660FS *>(r->deviceData);

    if (!name) {
        __errno_r(r) = EINVAL;
        return -1;
    }

    

    return 0;
}

int CISO9660FS::iso9660fs_close(struct _reent *r, void *fd) {
    auto *priv      = static_cast<CISO9660FS     *>(r->deviceData);
    auto *priv_file = static_cast<CISO9660File *>(fd);

    auto lk = std::scoped_lock(priv->session_mutex);

    return 0;
}


DIR_ITER *CISO9660FS::iso9660fs_diropen(struct _reent *r, DIR_ITER *dirState, const char *path) {
    auto *priv     = static_cast<CISO9660FS    *>(r->deviceData);
    auto *priv_dir = static_cast<CISO9660Dir *>(dirState->dirStruct);

	std::string pathname = std::regex_replace(std::string(path), std::regex("^"+ priv->mount_name), "");
	
	
    auto lk = std::scoped_lock(priv->session_mutex);
	
	priv->ISOReader->GetPathDir(pathname);
	
	priv_dir->diridx = 0;
	
    return dirState;
}

int CISO9660FS::iso9660fs_dirclose(struct _reent *r, DIR_ITER *dirState) {
    auto *priv     = static_cast<CISO9660FS    *>(r->deviceData);
    auto *priv_dir = static_cast<CISO9660Dir *>(dirState->dirStruct);

    auto lk = std::scoped_lock(priv->session_mutex);
	
	return 0;
	
}

int CISO9660FS::iso9660fs_dirnext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat) {
    auto *priv     = static_cast<CISO9660FS    *>(r->deviceData);
    auto *priv_dir = static_cast<CISO9660Dir *>(dirState->dirStruct);

    auto lk = std::scoped_lock(priv->session_mutex);
	if(priv_dir->diridx>=priv->ISOReader->getCurrentDirlist().size()){
		return -1;
	}
	
	memset(filename, 0, NAME_MAX);
	memcpy(filename,priv->ISOReader->getCurrentDirlist()[priv_dir->diridx].filename.data(),priv->ISOReader->getCurrentDirlist()[priv_dir->diridx].filename.length());
    memcpy(filestat,&priv->ISOReader->getCurrentDirlist()[priv_dir->diridx].filest,sizeof(struct stat));
	/*
	*filestat = {
        .st_mode     = priv->ISOReader->getCurrentDirlist()[priv_dir->diridx].isdir ? S_IFDIR:S_IFREG,
        .st_uid      =  1,
        .st_gid      =  1,
        .st_size     =  priv->ISOReader->getCurrentDirlist()[priv_dir->diridx].size,
        .st_atim     = {
            .tv_sec  = 0,
            .tv_nsec = 0,
        },
        .st_mtim = {
            .tv_sec  = 0,
            .tv_nsec = 0,
        },
        .st_ctim = {
            .tv_sec  = 0,
            .tv_nsec = 0,
        },
    };
	*/
	
	priv_dir->diridx = priv_dir->diridx+1;
		
    return 0;
}

int CISO9660FS::iso9660fs_dirreset(struct _reent *r, DIR_ITER *dirState) {
    __errno_r(r) = ENOSYS;
    return -1;
}

int CISO9660FS::iso9660fs_statvfs(struct _reent *r, const char *path, struct statvfs *buf) {
    auto *priv = static_cast<CISO9660FS *>(r->deviceData);

    

    return 0;
}



void isostat_translate_entry(iso9660_stat_t  *entry, struct stat *st)
{
	*st = {};
	
	st->st_mode =  entry->type == 2 ? S_IFDIR : S_IFREG;
	st->st_nlink = 1;
	st->st_uid = 1;
	st->st_gid = 2;
	st->st_size = entry->size;
	st->st_atime = 0;
	st->st_mtime =0;
	st->st_ctime = 0;
	
}

