#ifndef CDDAISO_H
#define CDDAISO_H

#include <string>
#include <vector>


typedef struct{
    std::string trackname;
    unsigned int startoffset = 0;
    unsigned int endoffset = 0;
    double playtime = 0;
    
}cdaudio_struct;


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
    
    std::vector<cdaudio_struct> getAudioTracks(){
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
    std::vector<cdaudio_struct> cdaudio_tracks;
	
	FILE *binnaryfile;
    
};

#endif /* CDDAISO_H */