#ifndef ISO9660FS_DEVOPTAB_H
#define ISO9660FS_DEVOPTAB_H

#include "devoptabfs_defines.h"

#ifdef BUILD_ISO9660


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

#include "ISOReader.h"

#include <switch.h>


class CISO9660FS{
public:
	CISO9660FS(std::string _url,std::string _name,std::string _mount_name);
	//CSSHFS(std::string _url,std::string _name,std::string _mount_name);
	//CSSHFS(std::string _server,int _port,std::string _username,std::string _password,std::string _path,std::string _name,std::string _mount_name);
	//CSSHFS(std::string _server,int _port,std::string _username,std::string _pubkeypath,std::string _privkeypath,std::string _passphrase,std::string _path,std::string _name,std::string _mount_name);
	
	~CISO9660FS();
	std::string name, mount_name;
	
	
	
	int unregister_fs() const {
		return RemoveDevice(this->mount_name.data());
	}
	
	int register_fs() const {
		
		auto id = FindDevice(this->mount_name.data());
		
		if (id < 0){
			id = AddDevice(&this->devoptab);
		}
		if (id < 0)
			return id;

		return 0;
	}
	
	bool fs_regisered = false;
	
	static int       iso9660fs_open     (struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
	static int       iso9660fs_close    (struct _reent *r, void *fd);
	static ssize_t   iso9660fs_read     (struct _reent *r, void *fd, char *ptr, size_t len);
	static off_t     iso9660fs_seek     (struct _reent *r, void *fd, off_t pos, int dir);
	static int       iso9660fs_fstat    (struct _reent *r, void *fd, struct stat *st);
	static int       iso9660fs_stat     (struct _reent *r, const char *file, struct stat *st);
	static int       iso9660fs_chdir    (struct _reent *r, const char *name);
	static DIR_ITER *iso9660fs_diropen  (struct _reent *r, DIR_ITER *dirState, const char *path);
	static int       iso9660fs_dirreset (struct _reent *r, DIR_ITER *dirState);
	static int       iso9660fs_dirnext  (struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
	static int       iso9660fs_dirclose (struct _reent *r, DIR_ITER *dirState);
	static int       iso9660fs_statvfs  (struct _reent *r, const char *path, struct statvfs *buf);
	//static int       iso9660fs_lstat    (struct _reent *r, const char *file, struct stat *st);
	
	bool is_connected = false;
	
	bool connect();
	
private:
	std::string connect_url;
	
	int connect(std::string host, std::uint16_t port,
        std::string username, std::string password);
	
	
	void disconnect();
	
	
	
	struct CISO9660File {
		char filename[255];
		size_t size;
		off_t offset;
    };

        struct CISO9660Dir {
			int diridx = 0;
		};
		
	std::mutex session_mutex;
	
protected:
	devoptab_t devoptab = {};
	CISOReader *ISOReader = nullptr;
		

	
};

#endif

#endif