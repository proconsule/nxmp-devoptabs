#ifndef CUEBINFS_DEVOPTAB_H
#define CUEBINFS_DEVOPTAB_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/dirent.h>
#include <sys/iosupport.h>
#include <sys/param.h>
#include <unistd.h>

#include <mutex>
#include <vector>

#include "CDDAISO.h"

#include <switch.h>



class CCUEBINFS{
public:
	CCUEBINFS(std::string _url,std::string _name,std::string _mount_name);
	~CCUEBINFS();
	std::string name, mount_name;
	
	
	int unregister_fs() const {
		return RemoveDevice(this->mount_name.data());
	}
	
	int register_fs() const {
		
		//auto id = FindDevice(this->mount_name.data());
		
		//if (id < 0){
		auto id = AddDevice(&this->devoptab);
		//	printf("AddDevice\n");
		//}
		if (id < 0)
			return id;

		return 0;
	}
	
	static int       cuebinfs_open     (struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
	static int       cuebinfs_close    (struct _reent *r, void *fd);
	static ssize_t   cuebinfs_read     (struct _reent *r, void *fd, char *ptr, size_t len);
	static off_t     cuebinfs_seek     (struct _reent *r, void *fd, off_t pos, int dir);
	static int       cuebinfs_fstat    (struct _reent *r, void *fd, struct stat *st);
	static int       cuebinfs_stat     (struct _reent *r, const char *file, struct stat *st);
	static int       cuebinfs_chdir    (struct _reent *r, const char *name);
	static DIR_ITER *cuebinfs_diropen  (struct _reent *r, DIR_ITER *dirState, const char *path);
	static int       cuebinfs_dirreset (struct _reent *r, DIR_ITER *dirState);
	static int       cuebinfs_dirnext  (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
	static int       cuebinfs_dirclose (struct _reent *r, DIR_ITER *dirState);
	static int       cuebinfs_statvfs  (struct _reent *r, const char *path, struct statvfs *buf);
	static int       cuebinfs_lstat    (struct _reent *r, const char *file, struct stat *st);
	
	bool is_connected = false;
	
private:
	std::string connect_url;
	
	int connect();
	void disconnect();
	
	int find_fileentry(std::string filepath);

	
	struct CCUEBINFSFile {
			
			int trackid = 0;
			off_t offset;
        };

        struct CCUBINFSDir {
			char dirpath[256] = "";
			int dirnext_idx = 0;
        };
	
	
	
	//struct archive *arch_ctx;
	std::string cwd = "";
	std::mutex session_mutex;
	CCDDAISO *discimage = nullptr;
	
	
	protected:
		devoptab_t devoptab = {};
	
};

#endif
