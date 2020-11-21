//
// Created by Administrator on 2020/11/17.
//

#include "rm.h"
#include "rm_internal.h"
#include "rm_bitmap.h"

RM_AttrFileHandle::RM_AttrFileHandle() {
    this->bFileOpen = FALSE;
    this->bHdrPageChanged = FALSE;
}

// TODO
//  insertVarValue: 将变长数据pVal写入attr文件中, 由RM_FileHandle触发
//  - 计算length对应的块的类型
//  - 寻找能够插入pVal的页, 若不存在, 则申请一页并写入页头和bitmap
//  - 将记录插入到对应页中
//  - unpin该页
RC RM_AttrFileHandle::insertVarValue(const char *pVal, int length, RM_VarLenAttr &varLenAttr) {
    int k = 0;
    while(k < NUMBER_ATTRBLOCK_SPECIES && ATTRBLOCK_SLOTSIZE[k] < length) {
        k++;
    }
    int rc;
    PageHandle pfPH;
    // 找到对应的属性块种类
    if(k < NUMBER_ATTRBLOCK_SPECIES) {
        int firstFreePage = hdrPage.firstFreeAttrPages[k];
        int bitmapSize = ATTRBLOCK_BIMAPSIZE[k];
        // 若对应的属性块没有空闲的了，则申请一块
        if(firstFreePage == RM_NO_FREE_PAGE) {
            char *pData;
            if((rc = fileHandle.allocatePage(pfPH)) ||
               (rc = pfPH.getData(pData))) {
                return rc;
            }
            char *bitmap = new char[bitmapSize];
            for(int i = 0; i < bitmapSize; ++i) {
                bitmap[i] = 0x00;
            }
            RM_PageHdr pageHdr;
            pageHdr.nextPage = RM_NO_FREE_PAGE;
            memcpy(pData, (char*)&pageHdr, sizeof(RM_PageHdr));
            memcpy(pData + sizeof(RM_PageHdr), bitmap, bitmapSize);
            bHdrPageChanged = TRUE;
            if((rc = pfPH.getPageNum(firstFreePage)) ||
               (rc = fileHandle.unpinPage(firstFreePage))) {
                return rc;
            }
            hdrPage.firstFreeAttrPages[k] = firstFreePage;
            delete [] bitmap;
        }

        char *pData;
        if((rc = fileHandle.getThisPage(firstFreePage, pfPH)) ||
           (rc = pfPH.getData(pData)) ||
           (rc = fileHandle.markDirty(firstFreePage))) {
            return rc;
        }
        char *bitmap = pData + sizeof(RM_PageHdr);
        int firstSlotNum = Bitmap::calcFirstZeroBit(bitmap, bitmapSize);
        int offset = sizeof(RM_PageHdr) + bitmapSize + (firstSlotNum - 1) * ATTRBLOCK_SLOTSIZE[k];
        memcpy(pData + offset, pVal, ATTRBLOCK_SLOTSIZE[k]);
        if((rc = Bitmap::setBit(bitmap, firstSlotNum))) {
            return rc;
        }
        if(Bitmap::isBitmapFull(bitmap, ATTRBLOCK_NUM_RECORDS[k])) {
            RM_PageHdr *pageHdr = (RM_PageHdr*)pData;
            hdrPage.firstFreeAttrPages[k] = pageHdr->nextPage;
            bHdrPageChanged = TRUE;
            pageHdr->nextPage = RM_NO_FREE_PAGE;
        }
        if((rc = fileHandle.unpinPage(firstFreePage))) {
            return rc;
        }
//        cout << "rmVar: " << firstFreePage << " " << firstSlotNum << endl;
        RM_VarLenAttr rmVarLenAttr(firstFreePage, firstSlotNum, k);
        varLenAttr = rmVarLenAttr;
    }
    return 0;   // ok
}

// TODO
//  updateVarValue: 更新一条变长属性, 输入删除属性的位置和所在的变长块类型的下标, 返回更新后属性的新位置和块类型下标
//  - 删除旧值
//  - 插入新值
//  注: 调用层需要更新对应的RM_VarLenAttr处的属性值
RC RM_AttrFileHandle::updateVarValue(char *pVal, int length, RM_VarLenAttr &varLenAttr) {
    int rc;
    if((rc = deleteVarValue(varLenAttr))) {
        return rc;
    }
    return insertVarValue(pVal, length, varLenAttr);
}

// TODO
//  deleteVarValue: 删除一条变长属性
//  - 获取页号和slotNum
//  - 获取该页内的数据, 标记为dirty
//  - 将slotNum对应的bit位置为0
//  - 如果删除之前该页是满的, 则删除之后放回对应位置的链表中
//  - unpin该页
RC RM_AttrFileHandle::deleteVarValue(const RM_VarLenAttr &varLenAttr) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;

    // 获取页号和slotNum
    PageNum pageNum;
    SlotNum slotNum;
    int k;
    if((rc = varLenAttr.getPageNum(pageNum)) ||
            (rc = varLenAttr.getSlotNum(slotNum)) ||
            (rc = varLenAttr.getBlockInfoIndex(k))) {
        return rc;
    }
    // 获取该页内的数据, 标记为dirty
    PageHandle pfPH;
    char *pData;
    if((rc = fileHandle.getThisPage(pageNum, pfPH)) ||
            (rc = pfPH.getData(pData)) ||
            (rc = fileHandle.markDirty(pageNum))) {
        return rc;
    }
    char *bitmap = pData + sizeof(RM_PageHdr);
    bool pageFull = Bitmap::isBitmapFull(bitmap, ATTRBLOCK_NUM_RECORDS[k]);
    if((rc = Bitmap::unsetBit(bitmap, slotNum))) {
        return rc;
    }
    // 如果删除之前该页是满的, 则删除之后放回对应位置的链表中
    if(pageFull) {
        PageNum firstFreePage = hdrPage.firstFreeAttrPages[k];
        RM_PageHdr *pH = (RM_PageHdr*)pData;
        pH->nextPage = firstFreePage;
        hdrPage.firstFreeAttrPages[k] = pageNum;
        bHdrPageChanged = TRUE;
    }
    if((rc = fileHandle.unpinPage(pageNum))) {
        return rc;
    }
    return 0;
}

// getVarValue: 获取varLenAttr对应的变长属性值
RC RM_AttrFileHandle::getVarValue(RM_VarLenAttr &varLenAttr, char *&pVal) {
    if(!bFileOpen) {
        return RM_CLOSEDFILE;
    }
    int rc;
    PageNum pageNum;
    SlotNum slotNum;
    int varBlockIndex;
    if((varLenAttr.getPageNum(pageNum)) ||
            (varLenAttr.getSlotNum(slotNum)) ||
            (varLenAttr.getBlockInfoIndex(varBlockIndex))) {
        return rc;
    }
    char *pData;
    PageHandle pfPH;
    if((rc = fileHandle.getThisPage(pageNum, pfPH)) ||
            (rc = pfPH.getData(pData))) {
        return rc;
    }
    int bitmapSize = ATTRBLOCK_BIMAPSIZE[varBlockIndex];
    int slotSize = ATTRBLOCK_SLOTSIZE[varBlockIndex];
    // 计算该属性在块内的偏移
    int off = bitmapSize + sizeof(RM_PageHdr) + (slotNum - 1) * slotSize;
    // 获取该属性值
    pVal = pData + off;
    if((rc = fileHandle.unpinPage(pageNum))) {
        return rc;
    }
    return 0;
}
