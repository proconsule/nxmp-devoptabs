#ifndef ISOREADER_H
#define ISOREADER_H


#include "devoptabfs_defines.h"

#ifdef BUILD_ISO9660

#include <string>
#include <vector>
#include <cstring>

#include <sys/iosupport.h>
#include <cdio/iso9660.h>


typedef struct{
    int32_t lsn;
    bool isdir;
    uint32_t size;
	struct stat filest;
    std::string filename;
    
}dirlist_struct;


class CISOReader{
public:
    CISOReader(std::string _path);
    ~CISOReader();
    
    int GetPathDir(std::string _path);

    int ISO_ReadFileData(std::string _path,size_t _offset,size_t size,char * ptr);
    
    std::vector<dirlist_struct> getCurrentDirlist(){
        return currdirlist;
    }
	
	bool valid = false;
	iso9660_t * getHandle(){
		return p_iso;
	}

protected:
	iso9660_t *p_iso;
    
private:
    
    std::vector<dirlist_struct> currdirlist;
    
};


#endif
#endif