
#include "sshfs.h"
#include "smb2fs.h"
#include "nfsfs.h"
#include "libarchivefs.h"
#include "m3u8fs.h"
#include "ftpfs.h"

CSSHFS * SSHFS = nullptr;
CSMB2FS * SMB2FS = nullptr;
CNFSFS * NFSFS = nullptr;
CARCHFS * ARCHFS = nullptr;

CFTPFS *FTPFS = nullptr;
CM3U8FS *M3U8FS = nullptr;

int main(int argc, const char* const* argv) {
	
	appletLockExit();
	romfsInit();
	
	SSHFS = new CSSHFS("127.0.01",22,"test","test","/home/test","ssh0","ssh0:");
	bool connected = SSHFS->RegisterFilesystem_v2();
	
	SMB2FS = new CSMB2FS("127.0.01","test","test","/home/test","smb0","smb0:");
	connected = SMB2FS->RegisterFilesystem_v2();
	
	NFSFS = new CNFSFS("127.0.01","/home/test","nfs0","nfs0:");
	connected = NFSFS->RegisterFilesystem_v2();
	
	ARCHFS = new CARCHFS("/switch/test.zip","arc0","arc0:");
	
	FTPFS = new CFTPFS("127.0.01",21,"test","test","/home/test","ftp0","ftp0:");
	
	M3U8FS = new CM3U8FS("/switch/test.zip","arc0","mfs0:");
	
	while(appletMainLoop()){
		
		
		
	}
	
	romfsExit();
	appletUnlockExit();
	
	
	return 0;
}