#include "CDDAISO.h"

#ifdef BUILD_CUEBIN

#include "CUEParser.h"
#include <filesystem>
#include "string.h"

// wav header taken from https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450

typedef struct WAV_HEADER {
  /* RIFF Chunk Descriptor */
  uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
  uint32_t ChunkSize;                     // RIFF Chunk Size
  uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
  /* "fmt" sub-chunk */
  uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
  uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
  uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
                            // Mu-Law, 258=IBM A-Law, 259=ADPCM
  uint16_t NumOfChan = 2;   // Number of channels 1=Mono 2=Sterio
  uint32_t SamplesPerSec = 44100;   // Sampling Frequency in Hz
  uint32_t bytesPerSec = 44100 * 4; // bytes per second
  uint16_t blockAlign = 4;          // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = 16;      // Number of bits per sample
  /* "data" sub-chunk */
  uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
  uint32_t Subchunk2Size;                        // Sampled data length
} wav_hdr;



long filesize(std::string _path){
    FILE *file;
    long file_size;
    
    file = fopen(_path.c_str(), "rb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return -1; // Esce con un codice di errore
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    return file_size;
}

int readfile(std::string _path,char **buffer){
    
    
    
    FILE *file;
    long file_size;
    
    file = fopen(_path.c_str(), "rb");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return 1; // Esce con un codice di errore
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *buffer = (char *)malloc((file_size + 1)*sizeof(char));
    if (*buffer == NULL) {
        perror("Errore nell'allocazione della memoria");
        fclose(file); 
        return 1;
    }

    size_t bytes_read = fread(*buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "Errore nella lettura del file o lettura incompleta.\n");
        free(*buffer);   
        fclose(file);   
        return 1;
    }
    
    fclose(file);
    
    return 0;
}

int CCDDAISO::findtrack(std::string _name){
	
	std::filesystem::path const p(_name);
	for(int i=0;i<cdaudio_tracks.size();i++){
		if(cdaudio_tracks[i].trackname == p.filename()){
			
			return i;
		}
	}
	return -1;
	
}

size_t CCDDAISO::getTrackSize(int _track){
	
	if(_track >= 0 && _track <cdaudio_tracks.size()){
		return cdaudio_tracks[_track].endoffset-cdaudio_tracks[_track].startoffset+44;
	}
	
	return -1;
}

int CCDDAISO::openBinaryFile(){
	if(binfile=="")return -1;
	
	binnaryfile = fopen(binfile.c_str(),"rb");
	if(binnaryfile == NULL)return -1;
	
	
	
	return 0;
	
	
}

int CCDDAISO::closeBinaryFile(){
	
	if(binnaryfile != NULL)fclose(binnaryfile);
	
	
	return 0;
	
	
}


size_t CCDDAISO::track_data_read(int _track,char *_buf,size_t _size,size_t offset){
	
	wav_hdr wav;
	wav.ChunkSize = cdaudio_tracks[_track].endoffset-cdaudio_tracks[_track].startoffset + sizeof(wav_hdr) - 8;
	wav.Subchunk2Size = cdaudio_tracks[_track].endoffset-cdaudio_tracks[_track].startoffset + sizeof(wav_hdr) - 44;
	
	if(offset >=44){
	
		if(_track >= 0 && _track < cdaudio_tracks.size()){
		
			if(binnaryfile != NULL){
				if(offset>cdaudio_tracks[_track].endoffset+44)return -1;
				fseek(binnaryfile,cdaudio_tracks[_track].startoffset+offset-44,SEEK_SET);
				size_t bytes_read = fread(_buf,sizeof( char ),_size,binnaryfile);
				return bytes_read;
			}
		}else{
			
		}
	
	}else{
		
		if(_size >= 44-offset){
			char * headerchar = reinterpret_cast<const char *>(&wav);
		    memcpy(_buf,&headerchar[offset],44-offset);
			fseek(binnaryfile,cdaudio_tracks[_track].startoffset+offset-44,SEEK_SET);
			size_t bytes_read = fread(&_buf[44-offset],sizeof( char ),_size-44+offset,binnaryfile);
			return bytes_read+44-offset;
		}else{
			char * headerchar = reinterpret_cast<const char *>(&wav);
		    memcpy(_buf,&headerchar[offset],_size);
			return _size;
		
		}
		
		
	}
	return -1;
}

CCDDAISO::CCDDAISO(std::string _path){
    
	
	cuefile = _path;
    char *buffer;
    readfile(_path.c_str(),&buffer);
    std::string _isodirectory = std::filesystem::path(cuefile);
    CUEParser *test = new CUEParser(buffer);
    
    const CUETrackInfo *tracktest = test->next_track();
    
    if(tracktest!=nullptr){
        std::filesystem::path const p(_path);
        binfile = p.parent_path().string()+"/"+tracktest->filename;
        binfilesize = filesize(binfile);
        if(binfilesize == -1)return;
        while(tracktest != nullptr){
            cdtrack_struct tmp;
            
			if(tracktest->track_mode == 0){
				if(mediumtype == MEDIUM_TYPE_UNKNOWN){
					mediumtype = MEDIUM_TYPE_CDAUDIO;
				}else if(mediumtype == MEDIUM_TYPE_DATA){
					mediumtype = MEDIUM_TYPE_MIXED;
				}
                tmp.audiotrack = true;
            }
			if(tracktest->track_mode != 0){
				if(mediumtype == MEDIUM_TYPE_UNKNOWN){
					mediumtype = MEDIUM_TYPE_DATA;
				}else if(mediumtype == MEDIUM_TYPE_CDAUDIO){
					mediumtype = MEDIUM_TYPE_MIXED;
				}
			}
            tmp.startoffset = tracktest->track_start*tracktest->sector_length;
           
            tracktest = test->next_track();
            if(tracktest != nullptr){
                tmp.endoffset = tracktest->file_offset;
            }else{
                tmp.endoffset = binfilesize;
            }
			
			std::stringstream ss;
			ss << std::setw(2) << std::setfill('0') << cdaudio_tracks.size()+1;
			std::string s_trackid = ss.str();
			
			if(tmp.audiotrack){
                tmp.trackname = std::string("Track_" + s_trackid  + std::string(".wav"));
            }else{
                tmp.trackname = std::string("Track_" + s_trackid  + std::string(".data"));
            }
            tmp.playtime = ((tmp.endoffset-tmp.startoffset)/2352.0)/75.0;
            cdaudio_tracks.push_back(tmp);

        }
        
    }else{
		delete test;
		free(buffer);
        return;
		
    }
    valid = true;
	free(buffer);
	delete test;
    return;
    
    
}


CCDDAISO::~CCDDAISO(){
	if(binnaryfile != NULL)fclose(binnaryfile);
	
}
#endif