#include "ISOReader.h"


#ifdef BUILD_ISO9660

#define CEILING(x, y) ((x+(y-1))/y)

CISOReader::CISOReader(std::string _path){
    
    
   
    p_iso = iso9660_open (_path.c_str());
    
    if (NULL == p_iso) {
        fprintf(stderr, "Sorry, couldn't open %s as an ISO-9660 image\n",
	    _path.c_str());
        return;
    }
	
	valid = true;
    
}

CISOReader::~CISOReader(){
    
    if(p_iso!=NULL){
        iso9660_close(p_iso);
    }
    
}

int CISOReader::GetPathDir(std::string _path){
    
    CdioISO9660FileList_t *p_entlist;
    CdioListNode_t *p_entnode;
    currdirlist.clear();
    if (NULL == p_iso) {
        return -1;
    }
    
    p_entlist = iso9660_ifs_readdir (p_iso, _path.c_str());
    
    if (p_entlist) {
      _CDIO_LIST_FOREACH (p_entnode, p_entlist)
        {
          char filename[4096];
          iso9660_stat_t *p_statbuf =
            (iso9660_stat_t *) _cdio_list_node_data (p_entnode);
         
          iso9660_name_translate(p_statbuf->filename, filename);
          
          dirlist_struct tmp{};
          tmp.isdir = p_statbuf->type == 2 ? true : false;
          tmp.filename = filename;
          tmp.lsn = p_statbuf->lsn;
          tmp.size = p_statbuf->size;
          currdirlist.push_back(tmp);
		  /*
          printf ("%s [LSN %6d] %8u %s %s\n",
                  2 == p_statbuf->type ? "d" : "-",
                  p_statbuf->lsn, p_statbuf->size, _path.c_str(), filename);
          
          */
          
          
        }
        iso9660_filelist_free(p_entlist);
    }else{
        return -1;
    }
    
    return 0;
}



int CISOReader::ISO_ReadFileData(std::string _path,size_t _offset,size_t _size,char * ptr){
    
    if (NULL == p_iso) {
		printf("NULL HANDLE\r\n");
        return -1;
    }
    
    iso9660_stat_t *p_statbuf =  iso9660_ifs_stat_translate (p_iso, _path.c_str());
    unsigned int i_blocks = CEILING(p_statbuf->size, ISO_BLOCKSIZE);
    unsigned int readblocks = CEILING(_size, ISO_BLOCKSIZE)+1;
    unsigned int startblock = CEILING(_offset, ISO_BLOCKSIZE)-1;
    if(_offset == 0 )startblock = 0;
    char buf[ISO_BLOCKSIZE*readblocks];
    
    const lsn_t lsn = p_statbuf->lsn + startblock;
    
	int readret = iso9660_iso_seek_read (p_iso, buf, lsn, readblocks);
	
    //if ( ISO_BLOCKSIZE != readret) {
    //    printf("BLOCK SIZE ERROR %d\r\n",readret);
	//	return -1;
    //}
    
    const unsigned int startbyte = _offset%2048;
    memcpy(ptr,&buf[startbyte],_size);
    
    return 0;
}

#endif