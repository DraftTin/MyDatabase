//
// Created by Administrator on 2020/11/7.
//
#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include "pf_internal.h"
#include "pf_buffermgr.h"
#include "pf.h"
#include <iostream>

using namespace std;


PF_FileHandle::PF_FileHandle() : bFileOpen(FALSE), pBufferMgr(nullptr) {
    // 将文件开闭flag初始化为FALSE, 防止PF_Manager调用closeFile出错
}

PF_FileHandle::~PF_FileHandle() {
    // do nothing
}

// isValidPageNum: 判断pageNum是否是合法的页号
// - 判断是否大于等于零 且 小于当前已经申请的页数
int PF_FileHandle::isValidPageNum(PageNum pageNum) const {
    return ((pageNum >= 0) &&
            (pageNum < hdr.numPages));
}

// flushPages: 刷新文件中的所有页面
// - 如果文件头hdr被修改, 则将hdr写回文件 write(fd, (char*)&hdr, sizeof(PF_FileHdr)
// - 将文件bHdrChanged修改为FALSE
// - 调用bufferMgr.flushPages(unixfd), 表示刷新unixfd文件的所有页面
RC PF_FileHandle::flushPages() const {
    // 文件必须被打开
    if(!bFileOpen) return PF_CLOSEDFILE;
    // 文件头信息写回
    if(bHdrChanged) {
        // 将指针定位到文件起始位置
        if(lseek(unixfd, 0, L_SET) < 0) {
            return PF_UNIX;
        }
        // 将缓冲的header信息（空闲页号，页数）重新写回文件中
        // 修改bug: 将PF_PageHdr修改成PF_FileHdr
        int numBytes = write(unixfd, (char *)&hdr, sizeof(PF_FileHdr));
        if(numBytes < 0) {
            return PF_UNIX;
        }
        if(numBytes != sizeof(PF_FileHdr)) {
            return PF_HDRWRITE;
        }
        const_cast<PF_FileHandle*>(this)->bHdrChanged = FALSE;  // 将对应的标记置为FALSE
    }
    // 调用缓冲区管理器的flushPages
    return (pBufferMgr->flushPages(unixfd));
}

// getFirstPage: 将文件第一页读入缓冲池
RC PF_FileHandle::getFirstPage(PF_PageHandle &pageHandle) const {
    // -1转换为PageNum
    return getNextPage((PageNum)-1, pageHandle);
}

// getNextPage: 获取current页号的下一个used page
// - 从current开始, 顺序查找文件中的页, 查看该页是否是used page
// - 如果是used page, 则表示找到了下一页, 否则继续循环
RC PF_FileHandle::getNextPage(PageNum current, PF_PageHandle &pageHandle) const {
    // 必须指向一个open的文件
    if(!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    // 可以计算current = -1的情况，相当于first page
    if(current != -1 && !isValidPageNum((current))) {
        return PF_INVALIDPAGE;
    }
    int rc;
    // 扫描文件直到找到合法的used page
    for(++current; current < hdr.numPages; ++current) {
        if(!(rc = getThisPage(current, pageHandle))) {
            return 0;
        }
        // 未知的错误
        if(rc != PF_INVALIDPAGE) {
            return rc;
        }
    }
    return PF_EOF;
}

// getThisPage: 将pageNum页读入缓冲池并将对应的PF_PageHandle返回
// - 从缓冲区获取该页
// - 判断该页是否被使用(used page)
// - 若不是used page, 则表示该页还未被allocate, 不能使用
RC PF_FileHandle::getThisPage(int pageNum, PF_PageHandle &pageHandle) const {
    int rc;
    char *pPageBuf;
    if(!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    if(!isValidPageNum(pageNum)) {
        return PF_INVALIDPAGE;
    }
    // 从缓冲区管理区中获取该页
    if((rc = pBufferMgr->getPage(unixfd, pageNum, &pPageBuf))) {
        return rc;
    }
    // 设置pageHandle, 将缓冲区内容强制转换为PF_PageHdr*类型, 相当于取出缓冲区页的前面的PF_PageHdr的内容
    // 判断该页是否被使用
    if(((PF_PageHdr*)pPageBuf)->nextFree == PF_PAGE_USED) {
        pageHandle.pageNum = pageNum;
        pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);
        return 0;
    }
    if((rc = unpinPage(pageNum))) {
        return rc;
    }
    return PF_INVALIDPAGE;
}

// getLastPage: 获取最后一个used page
RC PF_FileHandle::getLastPage(PF_PageHandle &pageHandle) const {
    // 获取最后一页的前一页
    return getPrevPage((PageNum)hdr.numPages, pageHandle);
}

// getPrevPage: 获取current页的前一个used page
RC PF_FileHandle::getPrevPage(PageNum current, PF_PageHandle &pageHandle) const {
    // 如果文件没打开则返回error
    if(!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    // 非合法页返回error, 可以计算numPages的上一页，表示最后一页
    if(current != hdr.numPages && !isValidPageNum(current)) {
        return PF_INVALIDPAGE;
    }
    int rc;
    char *pData = nullptr;
    // 从current - 1页开始查找合法的页
    for(--current; current >= 0; --current) {
        if(!(rc = getThisPage(current, pageHandle))) {
            return 0;
        }
        // 返回未知错误
        if(rc != PF_INVALIDPAGE) {
            return rc;
        }
    }
    return PF_EOF;
}

// allocatePage: 申请一页空页并返回给pageHandle
// - 从hdr的firstFree中判断是否有现成的空页
// - 若有, 则直接返回该页
// - 若没有, 则表示fileHdr中的numPages页都已经被使用, 需要申请新的页, 页号为当前numPages + 1, 申请后++numPages
// - 对于申请到的空页, 将其pageHdr的nextFree置为PF_PAGE_USED表示该页被使用, 并清空该页的数据区(因为可能有数据页被dispose然后有残留数据)
// - 标记该页为dirty
// - 对pageHandle的pData和pageNum进行赋值, 将其返回
RC PF_FileHandle::allocatePage(PF_PageHandle &pageHandle) {
    if (!bFileOpen) {
        return (PF_CLOSEDFILE);
    }

    int     rc;
    int     pageNum;
    char    *pPageBuf;          // 申请的缓冲区页的指针

    // 文件头记录文件是否有未使用的页，如果有，则直接将该页放入缓冲池中
    if (hdr.firstFree != PF_PAGE_LIST_END) {
        pageNum = hdr.firstFree;
        if ((rc = pBufferMgr->getPage(unixfd,
                                      pageNum,
                                      &pPageBuf))) {
            return (rc);
        }
        hdr.firstFree = ((PF_PageHdr*)pPageBuf)->nextFree;
    }
    else {
        // 如果没有空闲页，则从numPages开始申请，numPages = 0, 1, 2, ... !
        pageNum = hdr.numPages;

        // 在缓冲区中申请一页，执行后pPageBuf指针指向申请到的页
        if ((rc = pBufferMgr->allocatePage(unixfd, pageNum, &pPageBuf))) {
            return (rc);
        }
        // 增加文件申请到的页数的标记
        hdr.numPages++;
    }

    // 修改了头部的页
    bHdrChanged = TRUE;

    // 直接在修改申请到的页的内容! 修改的页头信息在flush或force后会写回文件中页的头部
    ((PF_PageHdr *)pPageBuf)->nextFree = PF_PAGE_USED;

    // 将申请到的页除了页头的部分都memset
    memset(pPageBuf + sizeof(PF_PageHdr), 0, PF_PAGE_SIZE);

    // 由于修改了该页的内容，所以标记为dirty
    if ((rc = markDirty(pageNum)))
        return (rc);

    // Set the pageHandle local variables
    pageHandle.pageNum = pageNum;
    // pageHandle中页的内容(pPageData)不包括pf页头(PF_PageHdr)
    pageHandle.pPageData = pPageBuf + sizeof(PF_PageHdr);

    // Return ok
    return (0);
}

// disposePage: 将pageNum页置为空闲页
// - 该页对应的缓冲区页数据, 判断该页是否已经是空闲页
// - 如果是空闲页, 则不需要释放, 返回warrning
// - 如果不是空闲页, 则释放该页, 将该页插入到空闲页链表中
// - 将该页置为dirty
RC PF_FileHandle::disposePage(PageNum pageNum) {
    if (!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    if(!isValidPageNum(pageNum)) {
        return PF_INVALIDPAGE;
    }
    int rc;
    char *pPageBuf;     // 页在缓冲池中的地址

    // 获取该页, bMultiplePins为FALSE表示获取该页用于dispose
    if((rc = pBufferMgr->getPage(unixfd, pageNum, &pPageBuf, FALSE))) {
        return rc;
    }
    // 如果该页已经是空闲状态, 则不需要dispose
    if (((PF_PageHdr *)pPageBuf)->nextFree != PF_PAGE_USED) {
        if ((rc = unpinPage(pageNum)))
            return (rc);
        // 该页已经是空闲状态
        return (PF_PAGEFREE);
    }
    // 将该页插入空闲页链表中
    ((PF_PageHdr *)pPageBuf)->nextFree = hdr.firstFree;
    hdr.firstFree = pageNum;
    bHdrChanged = TRUE;

    // PF_PageHdr改变了, 将该页标记为dirty
    if ((rc = markDirty(pageNum)))
        return (rc);

    // unpin page
    if ((rc = unpinPage(pageNum)))
        return (rc);
    return 0;       // ok
}

// unpinPage: 将pageNum页减少一个固定标记, 表示对该页的某项操作结束
// - 调用bufferMgr的unpinPage函数
RC PF_FileHandle::unpinPage(PageNum pageNum) const {
    if(!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    // 非合法页返回error
    if(!isValidPageNum(pageNum)) {
        return PF_INVALIDPAGE;
    }
    // 缓冲区管理器unpin该页
    return pBufferMgr->unpinPage(unixfd, pageNum);
}

// markDirty: 将pageNum页标记为dirty, 表示该页被修改过, 在flushPage或forcePage时需要写回磁盘
// - 调用bufferMgr的markDirty
RC PF_FileHandle::markDirty(PageNum pageNum) {
    if (!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    if(!isValidPageNum(pageNum)) {
        return PF_INVALIDPAGE;
    }
    return pBufferMgr->markDirty(unixfd, pageNum);
}

// forcePages: 将unixfd的pageNum页内容写回磁盘, 如果pageNum=ALL_PAGES, 则表示需要写回unixfd的所有页
// - 和flushPages一样, 如果文件头Hdr修改过, 则将文件头的信息写回磁盘中, 将bHdrChanged置为FALSE
// - 调用bufferMgr的forcePages方法将unixfd的pageNum页写回磁盘
RC PF_FileHandle::forcePages(PageNum pageNum) const {
    if(!bFileOpen) {
        return PF_CLOSEDFILE;
    }
    // forcePages的时候都检查hdr是否被修改，如果被修改，将hdr的信息写回文件中
    if(bHdrChanged) {
        // 将fd定位到文件开头
        if(lseek(unixfd, 0, L_SET) < 0) {
            return PF_UNIX;
        }
        int writeCount = write(unixfd, (char*)&hdr, sizeof(PF_FileHdr));
        // 老两样
        if(writeCount < 0) {
            return PF_UNIX;
        }
        else if(writeCount != sizeof(PF_FileHdr)) {
            return PF_HDRWRITE;
        }
        // 修改文件头的change flag
        const_cast<PF_FileHandle*>(this)->bHdrChanged = FALSE;
    }
    // 判断pageNum == ALL_PAGES就写回所有pinCount==0的脏页，否则只写回pageNum
    if(pageNum == ALL_PAGES) {
        return pBufferMgr->forcePages(unixfd);
    }
    return pBufferMgr->forceSinglePage(unixfd, pageNum);
}



