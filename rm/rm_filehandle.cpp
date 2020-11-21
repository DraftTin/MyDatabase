//
// Created by Administrator on 2020/11/10.
//
#include <cstring>
#include "rm.h"
#include "rm_internal.h"
#include "rm_bitmap.h"


// RM_FileHandle: 初始化RM_FileHandle实例
RM_FileHandle::RM_FileHandle() : bHdrPageChanged(FALSE), bFileOpen(FALSE) {
//    fileHdrPage.numberPages = 0;
    hdrPage.firstFreePage = RM_NO_FREE_PAGE;
}

RM_FileHandle::~RM_FileHandle() {
    // Don't need to do anything
}

// calcRecordOffset: 根据slotNum计算slotNum记录在页内的偏移
// - 计算bitmapSize的大小
// - 计算recorde offset的大小: sizeof(RM_PageHdr) + bitmapSize + (slotNum - 1) * recordSize
int RM_FileHandle::calcRecordOffset(int slotNum) const {
    int bitmapSize = hdrPage.numberRecords / 8;
    if(hdrPage.numberRecords % 8 != 0) {
        ++bitmapSize;
    }
    int offset =  sizeof(RM_PageHdr) + bitmapSize + (slotNum - 1) * hdrPage.recordSize;
    return offset;
}

// getRec: 获取rid对应的record
// - 读取rid的pageNum和slotNum
// - 从缓冲区获取该页
// - 计算slotNum对应的记录在该页内的偏移(calcRecordOffset)
// - 读取该记录到record并返回
RC RM_FileHandle::getRec(const RID &rid, RM_Record &record) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    // 防止内存泄漏，如果record.pData != nullptr，则释放内存
    if(record.pData != nullptr) {
        delete [] record.pData;
    }
    int rc;
    PageNum pageNum;
    SlotNum slotNum;
    if((rc = rid.getPageNum(pageNum)) ||
            (rc = rid.getSlotNum(slotNum))) {
        return rc;
    }
    //  根据rid的pageNum和slotNum获取对应的页指针
    PageHandle pfPH;
    char *pData;
    if((rc = fileHandle.getThisPage(pageNum, pfPH)) ||
            (rc = pfPH.getData(pData))) {
        return rc;
    }
    // 计算记录在页内的偏移
    int offset = calcRecordOffset(slotNum);
    // 修改bug: 此处pData需要申请fileHdrPage.recordSize大小的空间, 不然会出现空指针异常
    record.pData = new char[hdrPage.recordSize];
    memset(record.pData, 0, hdrPage.recordSize);
    // copy页内对应的记录
    memcpy(record.pData, pData + offset, hdrPage.recordSize);
    record.rid = rid;
    record.recordSize = hdrPage.recordSize;
    record.isValid = TRUE;
    // 每次getPage之后都会 ++pinCount, 所以getPage后记得unpin
    if((rc = fileHandle.unpinPage(pageNum))) {
        return rc;
    }
    return 0;
}

// insertRec: 向表中插入一条记录pData, 并将插入的rid返回
// - 从file hdr page中获取有空闲slot能够插入记录的页, 若没有空闲页, 则向pfFH申请一页空闲页
// - 读入该页的rm page hdr和bitmap
// - 获取bitmap中第一位bit为0的位置
// - 计算该位置对应在页内的偏移
// - 在偏移处插入该记录, 将bitmap该位置的bit置为1表示被占用
// - 如果置为1后该bitmap满了, 则将file hdr page中的firstFreePage置为该page hdr中的nextPage表示从空闲链表中移除
RC RM_FileHandle::insertRec(const char *pData, RID &rid) {
    // Check if the file is open
    if (!bFileOpen) {
        return RM_CLOSEDFILE;
    }

    if (pData == nullptr) {
        return RM_NULL_RECORD;
    }

    int rc;

    int numberRecords = hdrPage.numberRecords;
    int bitmapSize = numberRecords / 8;
    if (numberRecords % 8 != 0) bitmapSize++;

    PageNum freePageNumber = hdrPage.firstFreePage;
    PageHandle pfPH;

    // 如果文件中没有空闲页, 则向缓冲区申请一页空闲页, 并进行初始化(写page hdr, 写bitmap)
    if (freePageNumber == RM_NO_FREE_PAGE) {
        if ((rc = fileHandle.allocatePage(pfPH))) {
            return rc;
        }

        char* pHData;
        if ((rc = pfPH.getData(pHData))) {
            return rc;
        }

        RM_PageHdr* pageHeader = new RM_PageHdr;
        pageHeader->nextPage = RM_NO_FREE_PAGE;
        // 初始化都为0
        char* bitmap = new char[bitmapSize];
        for (int i = 0; i < bitmapSize; i++) {
            bitmap[i] = 0x00;
        }
        memcpy(pHData, pageHeader, sizeof(RM_PageHdr));
        char* offset = pHData + sizeof(RM_PageHdr);
        memcpy(offset, bitmap, bitmapSize);

        // 获取申请到的
        if ((rc = pfPH.getPageNum(freePageNumber))) {
            return rc;
        }

//        fileHdrPage.numberPages++;
        bHdrPageChanged = TRUE;

        hdrPage.firstFreePage = freePageNumber;

        delete pageHeader;
        delete[] bitmap;

        // Unpin the allocated page
        if ((rc = fileHandle.unpinPage(freePageNumber))) {
            return rc;
        }
    }

    if ((rc = fileHandle.getThisPage(freePageNumber, pfPH))) {
        return rc;
    }
    char* freePageData;
    if ((rc = pfPH.getData(freePageData))) {
        return rc;
    }
    char* bitmap = freePageData + sizeof(RM_PageHdr);
    int freeSlotNumber = Bitmap::calcFirstZeroBit(bitmap, bitmapSize);
    if (freeSlotNumber == RM_INCONSISTENT_BITMAP) {
        return RM_INCONSISTENT_BITMAP;
    }

    // 将该页标记为dirty
    if ((rc = fileHandle.markDirty(freePageNumber))) {
        // Return the error from the PF FileHandle
        return rc;
    }

    // 计算record的offset
    int recordOffset = calcRecordOffset(freeSlotNumber);

    // 将记录插入到缓冲块中
    memcpy(freePageData + recordOffset, pData, hdrPage.recordSize);

    // 将对应的slotNumber置为1
    if ((rc = Bitmap::setBit(bitmap, freeSlotNumber))) {
        // Return the error from the set bit method
        return rc;
    }

    // 如果插入之后导致该页满了, 则file hdr page的firstFreePage赋值为当前页的nextPage
    if (Bitmap::isBitmapFull(bitmap, numberRecords)) {
        // Get the next freeHead page number
        RM_PageHdr* pH = (RM_PageHdr*) freePageData;
        int nextFreePageNumber = pH->nextPage;

        // 更新first freeHead page为该页的下一个空闲页(可能为空)
        hdrPage.firstFreePage = nextFreePageNumber;
        bHdrPageChanged = TRUE;

        // 设置装满记录的页为单独的页, 不在链表中存在
        pH->nextPage = RM_NO_FREE_PAGE;
    }

    // Unpin the page
    if ((rc = fileHandle.unpinPage(freePageNumber))) {
        return rc;
    }
    // Set the RID
    RID newRid(freePageNumber, freeSlotNumber);
    rid = newRid;
//    cout << "rid: " << freePageNumber << "  " << freeSlotNumber << endl;
    return OK_RC;   // ok
}


// TODO
//  deleteRec: 删除rid对应的记录
//  - 获取rid的pageNum和slotNum
//  - 获取该页的指针, 计算slotNum对应在页内的偏移量
//  - 获取bitmap并将对应的bit位置为0, 将该页标记为dirty
//  - 如果bitmap因此从满到非满, 则将该页链到file hdr page的free list的头部
RC RM_FileHandle::deleteRec(const RID &rid) {
    if (!bFileOpen) {
        return RM_CLOSEDFILE;
    }

    int rc;

    // 获取rid对应的pageNumber和slotNumber
    int pageNumber, slotNumber;
    if ((rc = rid.getPageNum(pageNumber)) ||
            (rc = rid.getSlotNum(slotNumber))) {
        return rc;
    }

    // 检查pageNumber是否合法
    if (pageNumber <= 0) {
        return RM_INVALID_PAGE_NUMBER;
    }
    // 检查slotNumber是否合法
    int numberRecords = hdrPage.numberRecords;
    if (slotNumber < 1 || slotNumber > numberRecords) {
        return RM_INVALID_SLOT_NUMBER;
    }

    // 获取该记录对应的页内的数据, 并将pageNum标记为脏页
    PageHandle pfPH;
    char* pageData;
    if ((rc = fileHandle.getThisPage(pageNumber, pfPH))) {
        return rc;
    }
    if ((rc = pfPH.getData(pageData)) ||
            (rc = fileHandle.markDirty(pageNumber))) {
        return rc;
    }

    // 检查在删除该记录前该页是否是full的, 如果是, 则将该页插入到fileHdrPage的free list中, 将bitmap对应的slotNumber位置0
    char* bitmap = pageData + sizeof(RM_PageHdr);
    bool pageFull = Bitmap::isBitmapFull(bitmap, numberRecords);

    // 将slotNumber位设置为0, 表示该位置不存在记录
    if ((rc = Bitmap::unsetBit(bitmap, slotNumber))) {
        return rc;
    }

    // 如果该页在删除记录前是满的, 则将该页插入到free链表中: pH->nextFreePage = firstFreePage, firstFreePage = pageNumber
    if (pageFull) {
        PageNum firstFreePage = hdrPage.firstFreePage;
        // 将该页插入到链表头部
        RM_PageHdr *pH = (RM_PageHdr*) pageData;
        pH->nextPage = firstFreePage;

        hdrPage.firstFreePage = pageNumber;
        bHdrPageChanged = TRUE;
    }

    // getThisPage之后unpin
    if ((rc = fileHandle.unpinPage(pageNumber))) {
        return rc;
    }
    return 0;   // ok
}



// TODO
//  updateRec: 更新记录rec
//  - 获取rec的pageNum和slotNum
//  - 获取pageNum页的指针
//  - 计算slotNum偏移, 并将rec写入到该偏移处
RC RM_FileHandle::updateRec(const RM_Record &rec) {
    if (!bFileOpen) {
        return RM_CLOSEDFILE;
    }

    int rc;

    RID rid;
    if ((rc = rec.getRid(rid))) {
        return rc;
    }

    PageNum pageNumber;
    SlotNum slotNumber;
    if ((rc = rid.getPageNum(pageNumber)) ||
            (rc = rid.getSlotNum(slotNumber))) {
        return rc;
    }
    // 检查pageNumber是否合法
    if (pageNumber <= 0) {
        return RM_INVALID_PAGE_NUMBER;
    }
    // 检查slotNumber是否合法
    int numberRecords = hdrPage.numberRecords;
    if (slotNumber < 1 || slotNumber > numberRecords) {
        return RM_INVALID_SLOT_NUMBER;
    }

    // 获取该记录所在文件页的句柄, 获取页数据
    PageHandle pfPH;
    char* pData;
    if ((rc = fileHandle.getThisPage(pageNumber, pfPH)) ||
            (rc = pfPH.getData(pData))) {
        return rc;
    }

    // 将该页标记为脏页
    if ((rc = fileHandle.markDirty(pageNumber))) {
        return rc;
    }

    // 计算出该记录在该页的偏移
    int recordOffset = calcRecordOffset(slotNumber);
    char* recordData = pData + recordOffset;

    char* recData;
    if ((rc = rec.getData(recData))) {
        return rc;
    }
    // 将更新的记录写到缓冲页中
    memcpy(recordData, recData, hdrPage.recordSize);
    // unpin该页
    if ((rc = fileHandle.unpinPage(pageNumber))) {
        return rc;
    }

    return OK_RC;       // ok
}

// 将文件的所有脏页写回磁盘, 不从缓冲区移除, 调用pfPH的forcePages
RC RM_FileHandle::forcePages(PageNum pageNum) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    return fileHandle.forcePages(pageNum);
}

// TODO
//  insertVarValue: 插入变长值
//  输入: 插入变长属性值所属的记录id, 该记录中该变长属性指针的偏移量, 修改的新值pVal, 新值的长度length
//  返回: varLenAttr属性值插入的pageNum, slotNum, 插入的blockindex
//  - 插入变长记录
//  - 写入记录的变长属性指针
RC RM_FileHandle::insertVarValue(const RID &rid, int offset, const char *pVal, int length, RM_VarLenAttr &varLenAttr) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;
    if((rc = rmAttrFileHandle.insertVarValue(pVal, length, varLenAttr))) {
        return rc;
    }
    PageHandle pfPH;
    PageNum pageNum;
    SlotNum slotNum;
    char *pageData;
    // 获取rid对应的页
    if((rc = rid.getPageNum(pageNum)) ||
            (rc = rid.getSlotNum(slotNum)) ||
            (rc = fileHandle.getThisPage(pageNum, pfPH)) ||
            (rc = pfPH.getData(pageData))) {
        return rc;
    }
    // 计算该记录对应在页中的偏移
    int recOffset = calcRecordOffset(slotNum);
    char *recData = pageData + recOffset;
    if((rc = fileHandle.markDirty(pageNum))) {
        return rc;
    }
    // 更新记录文件中该记录对应的变长属性指针位置
    memcpy(recData + offset, (char*)&varLenAttr, sizeof(RM_VarLenAttr));
    if((rc = fileHandle.unpinPage(pageNum))) {
        return rc;
    }
    return 0;
}

RC RM_FileHandle::deleteVarValue(const RID &rid, int offset, const RM_VarLenAttr &varLenAttr) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;
    if((rc = rmAttrFileHandle.deleteVarValue(varLenAttr))) {
        return rc;
    }
    return 0;
}

// TODO
//  updateVarValue: 更新一个变长属性值
//  输入: 插入变长属性值所属的记录id, 该记录中该变长属性指针的偏移量, 修改的新值pVal, 新值的长度length
//  输出: varLenAttr属性值插入的pageNum, slotNum, 插入的blockindex
//  - 更新变长属性值
//  - 写入缓冲区页的该变长属性的指针值
RC RM_FileHandle::updateVarValue(const RID &rid, int offset, char *pVal, int length, RM_VarLenAttr &varLenAttr) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;
    if((rc = rmAttrFileHandle.updateVarValue(pVal, length, varLenAttr))) {
        return rc;
    }
    PageHandle pfPH;
    PageNum pageNum;
    SlotNum slotNum;
    char *pageData;
    // 获取rid对应的页
    if((rc = rid.getPageNum(pageNum)) ||
       (rc = rid.getSlotNum(slotNum)) ||
       (rc = fileHandle.getThisPage(pageNum, pfPH)) ||
       (rc = pfPH.getData(pageData))) {
        return rc;
    }
    // 计算该记录对应在页中的偏移
    int recOffset = calcRecordOffset(slotNum);
    char *recData = pageData + recOffset;
    if((rc = fileHandle.markDirty(pageNum))) {
        return rc;
    }
    // 更新记录文件中该记录对应的变长属性指针位置
    memcpy(recData + offset, (char*)&varLenAttr, sizeof(RM_VarLenAttr));
    if((rc = fileHandle.unpinPage(pageNum))) {
        return rc;
    }
    return 0;
}

RC RM_FileHandle::getVarValue(RM_VarLenAttr &varLenAttr, char *&pVal) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    return rmAttrFileHandle.getVarValue(varLenAttr, pVal);
}


