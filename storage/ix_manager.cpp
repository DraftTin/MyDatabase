//
// Created by Administrator on 2020/12/11.
//

#include "ix.h"
#include "ix_internal.h"
#include <cstring>
#include <sstream>

// Constructor
IX_Manager::IX_Manager(PF_Manager &_pfManager) : pfManager(&_pfManager) {
    // do nothing
}

IX_Manager::~IX_Manager() {
    // do nothing
}

// createIndex: 在fileName上创建索引, 索引号indexNo, 创建索引的属性的数据类型attrType, 属性长度attrLength
// - 索引文件名: {fileName}_{indexNo}
// - 创建索引文件
// - 打开文件
// - 申请文件的一个数据页, 写入索引头信息
// - 修改属性数据字典文件中对应的属性的索引号
// - 关闭文件
RC IX_Manager::createIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    int rc;
    string ss = generateIndexFileName(fileName, indexNo);
    const char *indexFileName = ss.c_str();
    // 创建索引文件
    if((rc = pfManager->createFile(indexFileName))) {
        return rc;
    }
    PF_FileHandle pfFH;
    if((rc = pfManager->openFile(indexFileName, pfFH))) {
        return rc;
    }
    PageHandle pageHandle;
    char *pData;
    PageNum pageNum;
    if((rc = pfFH.allocatePage(pageHandle)) ||
       (rc = pageHandle.getData(pData)) ||
       (rc = pageHandle.getPageNum(pageNum))) {
        return rc;
    }
    // 标记为脏页
    if((rc = pfFH.markDirty(pageNum))) {
        return rc;
    }
    // 写入数据
    IX_FileHeader fileHeader{};
    fileHeader.degree = calculateDegree(attrLength);
    fileHeader.attrLength = attrLength;
    fileHeader.attrType = attrType;
    fileHeader.rootPage = IX_NO_PAGE;
    memcpy(pData, (char*)&fileHeader, sizeof(IX_FileHeader));
    // unpin page
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    // 数据写回磁盘
    if((rc = pfFH.forcePages())) {
        return rc;
    }
    // 关闭文件
    if((rc = pfManager->closeFile(pfFH))) {
        return rc;
    }
    return 0;   // ok
}

// openIndex: 放回fileName, indexNo对应的索引文件
RC IX_Manager::openIndex(const char *fileName, int indexNo, IX_IndexHandle &indexHandle) {
    int rc;
    string ss = generateIndexFileName(fileName, indexNo);
    const char *indexFileName = ss.c_str();
    PF_FileHandle pfFH;
    if((rc = pfManager->openFile(indexFileName, pfFH))) {
        return rc;
    }
    PageHandle pageHandle;
    char *pData;
    PageNum pageNum;
    // 获取first page: header页
    if((rc = pfFH.getFirstPage(pageHandle)) ||
            (rc = pageHandle.getData(pData)) ||
            (rc = pageHandle.getPageNum(pageNum))) {
        return rc;
    }
    indexHandle.indexHeader = *(IX_FileHeader*)pData;
    indexHandle.pfFH = pfFH;
    indexHandle.isOpen = TRUE;
    indexHandle.bHeaderModified = FALSE;
    // unpin page
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    return 0;   // ok
}

// closeIndex: 关闭index
// - 检查indexHeader是否修改过
// - 若修改过则写回文件
// - 将该index的所有页强制写回磁盘
RC IX_Manager::closeIndex(IX_IndexHandle &indexHandle) {
    int rc;
    // 索引文件已经关闭
    if(!indexHandle.isOpen) {
        return IX_FILE_CLOSED;
    }
    // 检查indexHeader是否修改过
    // 被修改过, 写回文件
    if(indexHandle.bHeaderModified == TRUE) {
        PageHandle pageHandle;
        char *pData;
        PageNum pageNum;
        if((rc = indexHandle.pfFH.getFirstPage(pageHandle)) ||
                (rc = pageHandle.getData(pData)) ||
                (rc = pageHandle.getPageNum(pageNum))) {
            return rc;
        }
        // 写回数据
        memcpy(pData, (char*)&indexHandle.indexHeader, sizeof(IX_FileHeader));
        if((rc = indexHandle.pfFH.unpinPage(pageNum))) {
            return rc;
        }
    }
    indexHandle.isOpen = FALSE;
    indexHandle.bHeaderModified = FALSE;
    // 关闭索引文件
    if((rc = pfManager->closeFile(indexHandle.pfFH))) {
        return rc;
    }
    return 0;
}

string IX_Manager::generateIndexFileName(const char *fileName, int indexNo) {
    stringstream ss;
    ss << fileName << "_" << indexNo;
    return ss.str();
}

// calculateDegree: 计算页种能容纳的结点数量
int IX_Manager::calculateDegree(int attrLength) {
    // 初始化为 1
    int k = 1;
    int nodeHeaderSize = sizeof(IX_NodeHeader);
    int valueSize = sizeof(IX_NodeValue);
    while(nodeHeaderSize + k * attrLength + (k + 1) * valueSize <= PF_PAGE_SIZE) {
        ++k;
    }
    return k - 1;
}

RC IX_Manager::destroyIndex(char *fileName, int indexNo) {
    string ss = generateIndexFileName(fileName, indexNo);
    // 删除索引文件
    return pfManager->destroyFile(ss.c_str());
}

