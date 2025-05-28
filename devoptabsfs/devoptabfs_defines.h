#ifndef DEVOPTABFS_DEFINES_H
#define DEVOPTABFS_DEFINES_H


#define BUILD_SSH
#define BUILD_NFS
#define BUILD_SMB2
#define BUILD_M3U8
#define BUILD_FTP
#define BUILD_ARCHIVE
#define BUILD_CUEBIN
#define BUILD_ISO9660


#ifdef EXCLUDE_BUILD_SSH
	#undef BUILD_SSH
#endif
#ifdef EXCLUDE_BUILD_SMB2
	#undef BUILD_SMB2
#endif
#ifdef EXCLUDE_BUILD_NFS
	#undef BUILD_NFS
#endif
#ifdef EXCLUDE_BUILD_M3U8
	#undef BUILD_M3U8
#endif
#ifdef EXCLUDE_BUILD_FTP
	#undef BUILD_FTP
#endif
#ifdef EXCLUDE_BUILD_ARCHIVE
	#undef BUILD_ARCHIVE
#endif
#ifdef EXCLUDE_BUILD_CUEBIN
	#undef BUILD_CUEBIN
#endif
#ifdef EXCLUDE_BUILD_ISO9660
	#undef BUILD_ISO9660
#endif




#endif