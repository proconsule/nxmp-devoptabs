#ifndef CUE_GLOBLAS_H
#define CUE_GLOBLAS_H

#include <string>
#include <fcntl.h>

typedef struct{
    std::string trackname;
    bool audiotrack = false;
    unsigned int startoffset = 0;
    unsigned int endoffset = 0;
    double playtime = 0;
    
}cdtrack_struct;

enum MEDIUM_TYPE{
    MEDIUM_TYPE_CDAUDIO,
    MEDIUM_TYPE_MIXED,
    MEDIUM_TYPE_DATA,
    MEDIUM_TYPE_UNKNOWN = -1
    
};


#endif /* CUE_GLOBLAS_H */