//
// Created by Administrator on 2020/11/9.
//

#include <cstring>
#include "rm.h"
#include "rm_internal.h"
#include "bufmgr.h"

using namespace std;

// 初始化RM_Manager实例
RM_Manager::RM_Manager(PF_Manager &_pfManager) : pfManager(&_pfManager) {

}

RM_Manager::~RM_Manager() {
    // Don't need to do anything
}

// createFile: 创建记录文件
// - 调用PF_Manager的创建文件的方法创建页文件
// - 在为文件申请一页新页File Hdr Page, 用于记录该表的(recordSize, nextFree, recordCount, firstFree)
RC RM_Manager::createFile(const char *fileName, int recordSize) {
    if(recordSize <= 0) {
        return RM_SMALL_RECORD;
    }
    if(recordSize > PF_PAGE_SIZE) {
        return RM_LARGE_RECORD;
    }
    int rc;
    char *attrFileName = new char[MAXNAME + 4];
    sprintf(attrFileName, "%s_attr", fileName);
    if((rc = pfManager->createFile(fileName)) ||
            (rc = pfManager->createFile(attrFileName))) {
        return rc;
    }
    PF_FileHandle recFH;
    PF_FileHandle attrFH;
    // 打开文件
    if((rc = pfManager->openFile(fileName, recFH)) ||
            (rc = pfManager->openFile(attrFileName, attrFH))) {
        return rc;
    }
    delete [] attrFileName;
    PageHandle recPH;
    PageHandle attrPH;
    // 申请一页用作RM_FileHeaderPage
    if((rc = recFH.allocatePage(recPH)) ||
            (rc = attrFH.allocatePage(attrPH))) {
        return rc;
    }
    char *pRecData;
    char *pAttrData;
    // 获取页内数据的首地址
    if((rc = recPH.getData(pRecData)) ||
            (rc = attrPH.getData(pAttrData))) {
        return rc;
    }

    PageNum recPageNum;
    PageNum attrPageNum;
    // 获取页号
    if((rc = recPH.getPageNum(recPageNum)) ||
            (rc = attrPH.getPageNum(attrPageNum))) {
        return rc;
    }
    // 向申请到的FileHeaderPage写入文件的相关信息，将该页mark dirty
    if((rc = recFH.markDirty(recPageNum)) ||
            (rc = attrFH.markDirty(attrPageNum))) {
        return rc;
    }
    // 实例化file header page的信息(recordSize=recordSize, 计算numberRecordOnPage, 没有空闲页firstFreePage， 当前申请的页数numberPages)
    RM_RecordFileHeaderPage *recFileHeaderPage = new RM_RecordFileHeaderPage;
    recFileHeaderPage->recordSize = recordSize;
    // 计算每页能容纳的记录数量
    recFileHeaderPage->numberRecords = calcNumberRecord(recordSize);
    // 初始化没有空闲页
    recFileHeaderPage->firstFreePage = RM_NO_FREE_PAGE;
    // 将file header page的信息写入申请的缓冲页中, 复制file header page的数据到缓冲区中
    // 修改bug, 去掉(char8)&fileHeaderPage中的&，因为fileHeaderPage声明的是指针类型
    memcpy(pRecData, (char*)recFileHeaderPage, sizeof(RM_RecordFileHeaderPage));
    delete recFileHeaderPage;
    RM_AttrFileHeaderPage *attrFileHeaderPage = new RM_AttrFileHeaderPage;
    for(int i = 0; i < NUMBER_ATTRBLOCK_SPECIES; ++i) {
        attrFileHeaderPage->firstFreeAttrPages[i] = RM_NO_FREE_PAGE;
    }
    memcpy(pAttrData, (char*)attrFileHeaderPage, sizeof(RM_AttrFileHeaderPage));
    delete attrFileHeaderPage;
    // 申请的块有pinCount = 1，所以退出的时候unpin page，关闭文件同时将数据写回
    // 修改部分，只需要closeFile即可，因为closeFile会调用pfFileHandle的flushPages, 去掉(rc = pfFileHandle.forcePages()) ||

    if((rc = recFH.unpinPage(recPageNum)) ||
            ((rc = attrFH.unpinPage(attrPageNum))) ||
            (rc = pfManager->closeFile(recFH)) ||
            (rc = pfManager->closeFile(attrFH))) {
        return rc;
    }
    return 0;
}

// 打开文件，并返回对应的RM_FileHandle
RC RM_Manager::openFile(const char *fileName, RM_FileHandle &fileHandle) {
    if(fileName == nullptr) {
        return RM_INVALID_FILENAME;
    }
    if(fileHandle.bFileOpen) {
        return RM_FILE_OPEN;
    }

    int rc;
    PF_FileHandle recFH;     // PF_FileHandle
    PF_FileHandle attrFH;
    char *attrFileName = new char[MAXNAME + 4];
    sprintf(attrFileName, "%s_attr", fileName);
    if((rc = pfManager->openFile(fileName, recFH)) ||
            (rc = pfManager->openFile(attrFileName, attrFH))) {
        return rc;
    }
    delete [] attrFileName;
    fileHandle.bFileOpen = TRUE;
    fileHandle.bHdrPageChanged = FALSE;
    fileHandle.fileHandle = recFH;     // 拷贝
    fileHandle.rmAttrFileHandle.bFileOpen = TRUE;
    fileHandle.rmAttrFileHandle.bHdrPageChanged = FALSE;
    fileHandle.rmAttrFileHandle.fileHandle = attrFH;
    PageHandle recPH;
    PageHandle attrPH;
    // RM组件创建的文件first page是file hdr page
    if((rc = recFH.getFirstPage(recPH)) ||
            (rc = attrFH.getFirstPage(attrPH))) {
        return rc;
    }
    char *pRecData;
    char *pAttrData;
    if((rc = recPH.getData(pRecData)) ||
            (rc = attrPH.getData(pAttrData))) {
        return rc;
    }
    // 将缓冲区对应file header page的数据读入
    memcpy((char*)&(fileHandle.hdrPage), pRecData, sizeof(RM_RecordFileHeaderPage));
    memcpy((char*)&(fileHandle.rmAttrFileHandle.hdrPage), pAttrData, sizeof(RM_AttrFileHeaderPage));
    int pageNum;
    // openFile时调用了pfFH的getFirstPage, 由于是打开文件，所以对应的first page一定不在缓冲池中，
    // 需要缓冲池管理器分配缓冲区，所以会有pinCount = 1, 所以此处需要unpinPage(pageNum)
    if((rc = recPH.getPageNum(pageNum)) ||
            (rc = recFH.unpinPage(pageNum)) ||
            (rc = attrPH.getPageNum(pageNum)) ||
            (rc = attrFH.unpinPage(pageNum))) {
        return rc;
    }
    return 0;   // ok
}

// 删除文件
RC RM_Manager::destroyFile(const char *fileName) {
    if(fileName == nullptr) {
        return RM_INVALID_FILENAME;
    }
    int rc;
    char *attrFileName = new char[MAXNAME + 5];
    sprintf(attrFileName, "%s_attr", fileName);
    if((rc = pfManager->destroyFile(fileName)) ||
            (rc = pfManager->destroyFile(attrFileName))) {
        delete [] attrFileName;
        return rc;
    }
    delete [] attrFileName;
    return 0;
}

// 关闭文件
RC RM_Manager::closeFile(RM_FileHandle &fileHandle) {
    if(!fileHandle.bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;

    PF_FileHandle recFH = fileHandle.fileHandle;
    PF_FileHandle attrFH = fileHandle.rmAttrFileHandle.fileHandle;
    // 如果对应的记录文件头被修改, 则写回
    if(fileHandle.bHdrPageChanged) {
        PageHandle pfPH;
        PageNum pageNum;
        char *pData;
        // 获取file hdr page, 对应的pageNum, 和页内数据
        // 获取pageNum目的是unpin该pageNum
        if((rc = recFH.getFirstPage(pfPH)) ||
                (rc = pfPH.getPageNum(pageNum)) ||
                (rc = pfPH.getData(pData))) {
            return rc;
        }
        // 将头部信息写回该文件 fileHandle.fileHdrPage
        memcpy(pData, (char*)&fileHandle.hdrPage, sizeof(RM_RecordFileHeaderPage));
        // 修改缓冲区数据，将该pageNum标记为dirty
        if((rc = recFH.markDirty(pageNum))) {
            return rc;
        }
        // 关闭文件的同时刷新文件
        // 修改的地方，unpinPage和closeFile之间不需要加forcePages，只需要提前将pageNum标记为dirty
        if((rc = recFH.unpinPage(pageNum))) {
            return rc;
        }
    }
    // 如果对应的属性文件头被修改, 则写回
    if(fileHandle.rmAttrFileHandle.bHdrPageChanged) {
        cout << "rmAttrFileHandle write " << endl;
        PageHandle pfPH;
        PageNum pageNum;
        char *pData;
        // 获取file hdr page, 对应的pageNum, 和页内数据
        // 获取pageNum目的是unpin该pageNum
        if((rc = attrFH.getFirstPage(pfPH)) ||
           (rc = pfPH.getPageNum(pageNum)) ||
           (rc = pfPH.getData(pData))) {
            return rc;
        }
        // 修改bug: 该处写回的应该是fileHandle.rmAttrFileHandle.hdrPage
        // 将头部信息写回该文件 fileHandle.fileHdrPage
        memcpy(pData, (char*)&fileHandle.rmAttrFileHandle.hdrPage, sizeof(RM_RecordFileHeaderPage));
        // 修改缓冲区数据，将该pageNum标记为dirty
        if((rc = attrFH.markDirty(pageNum))) {
            return rc;
        }
        // 关闭文件的同时刷新文件
        // 修改的地方，unpinPage和closeFile之间不需要加forcePages，只需要提前将pageNum标记为dirty
        if((rc = attrFH.unpinPage(pageNum))) {
            return rc;
        }
    }
    if((rc = pfManager->closeFile(recFH)) ||
            (rc = pfManager->closeFile(attrFH))) {
        return rc;
    }
    fileHandle.bFileOpen = FALSE;
    fileHandle.bHdrPageChanged = FALSE;
    fileHandle.rmAttrFileHandle.bFileOpen = FALSE;
    fileHandle.rmAttrFileHandle.bHdrPageChanged = FALSE;

    return 0;   // ok
}

// 计算页内能容纳的记录数量，每页需要保存的信息：PageHdr的信息, 位示图bitmap, 页内数据
int RM_Manager::calcNumberRecord(int recordSize) {
    int n = 1;
    int headSize = sizeof(RM_PageHdr);  // 页头的size
    int bitmapSize;
    while(true) {
        // 位示图1byte能够表示8条记录
        bitmapSize = n / 8;
        if(n % 8 != 0) {
            ++bitmapSize;
        }
        int sumSize = headSize + bitmapSize + n * recordSize;
        // 循环直到：页头Size + 位示图Size + 记录Size > 页Size，之后返回(n - 1)
        if(sumSize > PF_PAGE_SIZE) {
            break;
        }
        ++n;
    }
    return (n - 1);
}


