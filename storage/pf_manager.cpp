//
// Created by Administrator on 2020/11/6.
//

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <fcntl.h>
#include "pf_internal.h"
#include "bufmgr.h"

#include <iostream>

using namespace std;

// PF_Manager: 初始化PF_Manager实例
// - 初始化内存缓冲区
 PF_Manager::PF_Manager() {
     // 申请内存中的一个缓冲区
     pBufferMgr = new BufferMgr(PF_BUFFER_SIZE);
 }

 PF_Manager::~PF_Manager() {
     // 移除缓冲区
     delete pBufferMgr;
 }

 // createFile: 创建filename的文件, 并写入PF_FileHdr信息
 // - 创建打开filename文件, 权限为只写
 // - 在文件头部写入PF_FileHdr信息(nextFree, numPages)
 // - 关闭文件
RC PF_Manager::createFile(const char* filename) {
    int fd;
    int numBytes;       // return code from write syscall
    // 打开文件，只写，设置文件的读写权限: -rw-------
    if((fd = open(filename, O_BINARY | O_EXCL | O_CREAT | O_WRONLY, CREATION_MASK)) < 0) {
        return PF_UNIX;
    }
    char hdrBuf[PF_FILE_HDR_SIZE];

    memset(hdrBuf, 0, PF_FILE_HDR_SIZE);

    PF_FileHdr* hdr = (PF_FileHdr*)hdrBuf;
    hdr->firstFree = PF_PAGE_LIST_END;
    hdr->numPages = 0;
    // 创建文件直接向文件头中写入信息(第一个空闲页，文件中申请的页数)
    if((numBytes = write(fd, hdrBuf, PF_FILE_HDR_SIZE))
       != PF_FILE_HDR_SIZE) {
        close(fd);
        unlink(filename);

        if(numBytes < 0)
            return PF_UNIX;
        else
            return PF_HDRWRITE;
    }
    if(close(fd) < 0)
        return PF_UNIX;

    return 0;
}

// destroyFile: 删除文件
RC PF_Manager::destroyFile(const char* filename) {
    if(unlink(filename) < 0) {
        return PF_UNIX;
    }
    return 0;
}

// openFile: 打开filename对应的文件并返回该文件的PF_FileHandle
// - 打开文件, 可读可写
// - 将PF_FileHdr信息读入PF_FileHandle的hdr中
// - 返回fileHandle
RC PF_Manager::openFile(const char *fileName, PF_FileHandle &fileHandle) {
    int rc;
    // 如果文件已经打开，则返回warrning
    if(fileHandle.bFileOpen) {
        return PF_FILEOPEN;
    }
    // 打开文件可读可写
    if((fileHandle.unixfd = open(fileName, O_BINARY | O_RDWR)) < 0) {
        return PF_UNIX;
    }
    {
        // 将文件头的信息读入到fileHandle中的hdr中（直接以char *）的方式从文件读取
        int numBytes = read(fileHandle.unixfd, (char *)&fileHandle.hdr,
                            sizeof(PF_FileHdr));
        if(numBytes != sizeof(PF_FileHdr)) {
            rc = (numBytes < 0) ? PF_UNIX : PF_HDRREAD;
            close(fileHandle.unixfd);
            fileHandle.bFileOpen = FALSE;
            return rc;
        }
    }
    fileHandle.bFileOpen = TRUE;
    fileHandle.bHdrChanged = FALSE;
    fileHandle.pBufferMgr = pBufferMgr;
    return 0;
}

// closeFile: 关闭和fileHandle相关联的文件，刷新文件中的pages
// - 刷新该文件对应的所有页(写回所有页, 移除缓冲区)
// - 关闭该文件
RC PF_Manager::closeFile(PF_FileHandle &fileHandle) {
    RC rc;
    // 文件如果未打开，则返回warning
    if(!fileHandle.bFileOpen) {
        return PF_CLOSEDFILE;
    }
    // 关闭文件调用对应fileHandle的flushPages
    if((rc = fileHandle.flushPages())) {
        return rc;
    }
    if(close(fileHandle.unixfd) < 0) {
        return PF_UNIX;
    }
    // 修改bug: 关闭文件同时将fileHandle中的bFileOpen置为FALSE
    fileHandle.bFileOpen = FALSE;
    // 关闭文件，清空buffer manager的指针
    fileHandle.pBufferMgr = nullptr;

    return 0;   // ok
}

// printBuffer: 输出缓冲区
RC PF_Manager::printBuffer() const {
    return pBufferMgr->printBuffer();
}


