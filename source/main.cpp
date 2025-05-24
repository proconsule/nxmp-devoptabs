
#include "sshfs.h"
#include "smb2fs.h"
#include "nfsfs.h"
#include "libarchivefs.h"
#include "m3u8fs.h"
#include "ftpfs.h"
#include "cuebinfs.h"

CSSHFS * SSHFS = nullptr;
CSMB2FS * SMB2FS = nullptr;
CNFSFS * NFSFS = nullptr;
CARCHFS * ARCHFS = nullptr;

CFTPFS *FTPFS = nullptr;
CM3U8FS *M3U8FS = nullptr;
CCUEBINFS *CUEBINFS = nullptr;


int main(int argc, const char* const* argv) {
	
	appletLockExit();
	romfsInit();
	
	consoleInit(NULL);

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
	
	CUEBINFS = new CCUEBINFS("/switch/NXMilk/audiotest.cue","cda0","cda0:");
	
	
	
	
	struct dirent *ent;
			DIR *dir;
			std::string path = "cda0:/";

			if (!path.empty()) {
				if ((dir = opendir(path.c_str())) != nullptr) {
				
					auto *reent    = __syscall_getreent();
					auto *devoptab = devoptab_list[dir->dirData->device];	
					
					
					
					while (/*(ent = readdir(dir)) != nullptr*/ true) {
							reent->deviceData = devoptab->deviceData;
							struct stat st{0};
							if (devoptab->dirnext_r(reent, dir->dirData, dir->fileData.d_name, &st))
							break;
							
							if ((/*path == "/" ||*/ strlen(dir->fileData.d_name) == 1) && dir->fileData.d_name[0] == '.') {
								continue;
							}
							if ((/*path == "/" ||*/ strlen(dir->fileData.d_name) == 2) && dir->fileData.d_name[0] == '.' && dir->fileData.d_name[1] == '.') {
								continue;
							}
							
							
							printf("%s %d\r\n",dir->fileData.d_name,st.st_size);
							
							
						}
						
						closedir(dir);
			
						
						
					}
				
			}
			
			
			
	
	
	while(appletMainLoop()){
		 padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu

        consoleUpdate(NULL);
		
		
	}
	
	
	delete CUEBINFS;
	
	//socketExit();
	consoleExit(NULL);
	romfsExit();
	appletUnlockExit();
	
	
	return 0;
}