## nxmp-devoptabs

this repo contains the devoptabs classes used in 
- NXMP https://github.com/proconsule/nxmp 
- NXMilk https://github.com/proconsule/NXMilk


Supported protocols/files

- SSH (SFTP) using libssh2 https://libssh2.org/
- NFS using libnfs https://github.com/sahlberg/libnfs
- SMB using libsmb2 https://github.com/sahlberg/libsmb2
- FTP using an internal ugly-ftplib based on https://github.com/codebrainz/ftplib/blob/master/ftplib.c
- RAR,ZIP,TAR.GZ (and many others) with libarchive https://www.libarchive.org/
- CUE/BIN Audio CD files using an internal reader and use of https://github.com/rabbitholecomputing/CUEParser for cue parsing
- ISO9660 ISO image using libcdio (libiso9660) https://github.com/libcdio/libcdio
- M3U8 Parser using an custom made parser (WIP)

The goal is to support as many as possible (can  be individualy disabled with define flags)


The Makefile here is just as a ref the nro is "useless" if main.cpp is not edited for doing something


