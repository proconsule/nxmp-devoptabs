#include "cuebinfs.h"

void cuebinstat_entry(int _tracksize, struct stat *st);

CCUEBINFS::CCUEBINFS(std::string _url,std::string _name,std::string _mount_name){
	this->connect_url = _url;
	
	
	this->name       = _name;
    this->mount_name = _mount_name;
	
    this->devoptab = {
        .name         = CCUEBINFS::name.data(),

        .structSize   = sizeof(CCUEBINFSFile),
        .open_r       = CCUEBINFS::cuebinfs_open,
        .close_r      = CCUEBINFS::cuebinfs_close,
        .read_r       = CCUEBINFS::cuebinfs_read,
        .seek_r       = CCUEBINFS::cuebinfs_seek,
        .fstat_r      = CCUEBINFS::cuebinfs_fstat,

        .stat_r       = CCUEBINFS::cuebinfs_stat,
        .chdir_r      = CCUEBINFS::cuebinfs_chdir,

        .dirStateSize = sizeof(CCUBINFSDir),
        .diropen_r    = CCUEBINFS::cuebinfs_diropen,
        .dirreset_r   = CCUEBINFS::cuebinfs_dirreset,
        .dirnext_r    = CCUEBINFS::cuebinfs_dirnext,
        .dirclose_r   = CCUEBINFS::cuebinfs_dirclose,

        .statvfs_r    = CCUEBINFS::cuebinfs_statvfs,

        .deviceData   = this,

        .lstat_r      = CCUEBINFS::cuebinfs_stat,
    };
	
	if(connect() == 0){
		printf("CUEBIN Registering\n");
		register_fs();
	}
}

CCUEBINFS::~CCUEBINFS(){
	if(discimage!=nullptr)delete discimage;
	
	
}


int CCUEBINFS::connect(){
	
	discimage = new CCDDAISO(this->connect_url);
	if(discimage->valid)return 0;
	
	return -1;
}


int  CCUEBINFS::cuebinfs_open     (struct _reent *r, void *fileStruct, const char *path, int flags, int mode){
	auto *priv      = static_cast<CCUEBINFS     *>(r->deviceData);
    auto *priv_file = static_cast<CCUEBINFSFile *>(fileStruct);

	if(std::string(path).empty()){
		return -1;
	}
	
	
	
	int mytrackid = priv->discimage->findtrack(path);
	if(mytrackid == -1)return -1;
	int openbin = priv->discimage->openBinaryFile();
	if(openbin == -1)return -1;
	
	priv_file->trackid = mytrackid;
	priv_file->offset = 0;
	
	
    
    return 0;
	
}

int  CCUEBINFS::cuebinfs_close    (struct _reent *r, void *fd){
	auto *priv      = static_cast<CCUEBINFS     *>(r->deviceData);
    auto *priv_file = static_cast<CCUEBINFSFile *>(fd);

	priv->discimage->openBinaryFile();

    return 0;
}

ssize_t   CCUEBINFS::cuebinfs_read     (struct _reent *r, void *fd, char *ptr, size_t len){
	auto *priv      = static_cast<CCUEBINFS     *>(r->deviceData);
    auto *priv_file = static_cast<CCUEBINFSFile *>(fd);

    auto lk = std::scoped_lock(priv->session_mutex);
	
	ssize_t bytes_read = priv->discimage->track_data_read(priv_file->trackid,ptr,len,priv_file->offset);
	priv_file->offset=priv_file->offset+bytes_read;
	
	
	return bytes_read;

}

off_t     CCUEBINFS::cuebinfs_seek     (struct _reent *r, void *fd, off_t pos, int dir){
	auto *priv      = static_cast<CCUEBINFS     *>(r->deviceData);
    auto *priv_file = static_cast<CCUEBINFSFile *>(fd);

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
            offset = priv->discimage->getTrackSize(priv_file->trackid);
            break;
    }

    
	auto lk = std::scoped_lock(priv->session_mutex);
	
	priv_file->offset = offset + pos;
	
	
	
    return priv_file->offset;
}

int       CCUEBINFS::cuebinfs_fstat    (struct _reent *r, void *fd, struct stat *st){
	auto *priv = static_cast<CCUEBINFS *>(r->deviceData);
	auto *priv_file = static_cast<CCUEBINFSFile *>(fd);
    auto lk = std::scoped_lock(priv->session_mutex);
	cuebinstat_entry(priv->discimage->getTrackSize(priv_file->trackid), st);
	return 0;
}

int       CCUEBINFS::cuebinfs_stat     (struct _reent *r, const char *file, struct stat *st){
	auto *priv     = static_cast<CCUEBINFS    *>(r->deviceData);
	
	int mytrackid = priv->discimage->findtrack(file);
	
	if(mytrackid<0){
		
		return -1;
	}
	
	cuebinstat_entry(priv->discimage->getTrackSize(mytrackid), st);
	return 0;

}

int       CCUEBINFS::cuebinfs_chdir    (struct _reent *r, const char *name){
	auto *priv = static_cast<CCUEBINFS *>(r->deviceData);

    return 0;
}


DIR_ITER * CCUEBINFS::cuebinfs_diropen  (struct _reent *r, DIR_ITER *dirState, const char *path){
	auto *priv = static_cast<CCUEBINFS *>(r->deviceData);
	auto *priv_dir = static_cast<CCUBINFSDir *>(dirState->dirStruct);
	
	
	priv_dir->dirnext_idx = 0;
	
	
	
	return dirState;
}

int   CCUEBINFS::cuebinfs_dirreset (struct _reent *r, DIR_ITER *dirState){
	__errno_r(r) = ENOSYS;
    return -1;
}

int       CCUEBINFS::cuebinfs_dirnext  (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat){
	auto *priv     = static_cast<CCUEBINFS    *>(r->deviceData);
    auto *priv_dir = static_cast<CCUBINFSDir *>(dirState->dirStruct);

    auto lk = std::scoped_lock(priv->session_mutex);
	
	if(priv_dir->dirnext_idx >= priv->discimage->getAudioTracks().size()){
		return -1;
	}
	memset(filename, 0, NAME_MAX);
	memcpy(filename,priv->discimage->getTrackName(priv_dir->dirnext_idx).data(),priv->discimage->getTrackName(priv_dir->dirnext_idx).length());
	cuebinstat_entry(priv->discimage->getTrackSize(priv_dir->dirnext_idx), filestat);
	priv_dir->dirnext_idx +=1;
	
	return 0;
}

int       CCUEBINFS::cuebinfs_dirclose (struct _reent *r, DIR_ITER *dirState){
	auto *priv     = static_cast<CCUEBINFS    *>(r->deviceData);
    
	auto lk = std::scoped_lock(priv->session_mutex);
	
	
	
	return 0;
}

int       CCUEBINFS::cuebinfs_statvfs  (struct _reent *r, const char *path, struct statvfs *buf){
			
	
	return 0;
}

void cuebinstat_entry(int _tracksize, struct stat *st)
{
	*st = {};
	
	st->st_mode =   S_IFREG;
	st->st_nlink = 1;
	st->st_uid = 1;
	st->st_gid = 2;
	st->st_size = _tracksize;
	st->st_atime = 0;
	st->st_mtime = 0;
	st->st_ctime = 0;
	
}


