
#include "sshfs.h"
#include "smb2fs.h"
#include "nfsfs.h"
#include "libarchivefs.h"
#include "m3u8fs.h"
#include "ftpfs.h"
#include "cuebinfs.h"
#include "iso9660fs.h"

#include <switch.h>


#ifdef BUILD_SSH
CSSHFS * SSHFS = nullptr;
#endif
#ifdef BUILD_SMB2
CSMB2FS * SMB2FS = nullptr;
#endif
#ifdef BUILD_NFS
CNFSFS * NFSFS = nullptr;
#endif
#ifdef BUILD_ARCHIVE
CARCHFS * ARCHFS = nullptr;
#endif
#ifdef BUILD_FTP
CFTPFS *FTPFS = nullptr;
#endif
#ifdef BUILD_M3U8
CM3U8FS *M3U8FS = nullptr;
#endif
#ifdef BUILD_CUEBIN
CCUEBINFS *CUEBINFS = nullptr;
#endif
#ifdef BUILD_ISO9660
CISO9660FS *ISO9660FS = nullptr;
#endif

int main(int argc, const char* const* argv) {
	
	appletLockExit();
	romfsInit();
	
	consoleInit(NULL);
	socketInitializeDefault();
	nxlinkStdio();
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	
	PadState pad;
    padInitializeDefault(&pad);
	
	
	
	
	
	
	
	/*
	SSHFS = new CSSHFS("127.0.01",22,"test","test","/home/test","ssh0","ssh0:");
	bool connected = SSHFS->RegisterFilesystem_v2();
	
	SMB2FS = new CSMB2FS("127.0.01","test","test","/home/test","smb0","smb0:");
	connected = SMB2FS->RegisterFilesystem_v2();
	
	NFSFS = new CNFSFS("127.0.01","/home/test","nfs0","nfs0:");
	connected = NFSFS->RegisterFilesystem_v2();
	
	ARCHFS = new CARCHFS("/switch/test.zip","arc0","arc0:");
	
	FTPFS = new CFTPFS("127.0.01",21,"test","test","/home/test","ftp0","ftp0:");
	
	M3U8FS = new CM3U8FS("/switch/test.zip","arc0","mfs0:");
	*/
	
	
	
	printf("STARTING\r\n");
	
	ISO9660FS = new CISO9660FS("/switch/NXMilk/TEST3.ISO","iso0","iso0:");
	
	
	//CUEBINFS = new CCUEBINFS("/switch/NXMilk/audiotest.cue","cda0","cda0:");
	
	
	
	
	struct dirent *ent;
			DIR *dir;
			std::string path = "iso0:/";

			if (!path.empty()) {
				if ((dir = opendir(path.c_str())) != nullptr) {
				
					auto *reent    = __syscall_getreent();
					auto *devoptab = devoptab_list[dir->dirData->device];	
					
					
					
					while (true) {
							reent->deviceData = devoptab->deviceData;
							struct stat st{0};
							if (devoptab->dirnext_r(reent, dir->dirData, dir->fileData.d_name, &st))
							break;
							
							if (( strlen(dir->fileData.d_name) == 1) && dir->fileData.d_name[0] == '.') {
								continue;
							}
							if (( strlen(dir->fileData.d_name) == 2) && dir->fileData.d_name[0] == '.' && dir->fileData.d_name[1] == '.') {
								continue;
							}
							
							
							printf("%s %d\r\n",dir->fileData.d_name,st.st_size);
							
							
						}
						
						closedir(dir);
			
						
						
					}
				
			}
			
	FILE*testfile;
	testfile = fopen("iso0:/06__hero.wav","rb");
	if(testfile != NULL){
		printf("FILE OPEN OK\r\n");
	}
	
	char mybuf[44];
	fread(mybuf, sizeof mybuf[0], 44, testfile);
	
	printf("%s\r\n",mybuf);
	
			
	
	
	while(appletMainLoop()){
		 padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu

        consoleUpdate(NULL);
		
		
	}
	
	
	//delete CUEBINFS;
	
	delete ISO9660FS;
	socketExit();
	consoleExit(NULL);
	romfsExit();
	appletUnlockExit();
	
	
	return 0;
}