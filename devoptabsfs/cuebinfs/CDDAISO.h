#ifndef CDDAISO_H
#define CDDAISO_H

#include <string>
#include <vector>

#include "devoptabfs_defines.h"

#ifdef BUILD_CUEBIN

#include "CUEGlobals.h"




class CCDDAISO{
public:
    CCDDAISO(std::string _path);
	~CCDDAISO();
    bool valid = false;
    
    std::string getCueFile(){
        return cuefile;
    }
    std::string getBinFile(){
        return binfile;
    }
    
    long getBinFileSize(){
        return binfilesize;
    }
    
    std::vector<cdtrack_struct> getAudioTracks(){
        return cdaudio_tracks;
    }
	
	std::string getTrackName(int _track){
		return cdaudio_tracks[_track].trackname;
	}
	
	size_t getTrackSize(int _track);
	
	size_t track_data_read(int _track,char *_buf,size_t _size,size_t offset);
	
	int findtrack(std::string _name);
	
	int openBinaryFile();
    int closeBinaryFile();
	
	
    
    
private:
   
    std::string cuefile = "";
    std::string binfile = "";
    long binfilesize = 0;
    std::vector<cdtrack_struct> cdaudio_tracks;
	
	MEDIUM_TYPE mediumtype = MEDIUM_TYPE_UNKNOWN;
	
	FILE *binnaryfile;
    
};

#endif
#endif /* CDDAISO_H */