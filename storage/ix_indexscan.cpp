//
// Created by Administrator on 2020/12/13.
//

#include "ix.h"

IX_IndexScan::IX_IndexScan() {

}

IX_IndexScan::~IX_IndexScan() {
    // do nothing
}

// openScan: 开启一个索引扫描, 通过(compOp, value)对索引文件进行扫描
// - 初始化保存信息
// - 获取第一个符合条件的索引项
RC IX_IndexScan::openScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value) {
    int rc;
    this->isOpen = TRUE;
    this->compOp = compOp;
    this->indexHandle = indexHandle;
    this->value = value;
    this->inOverflow = FALSE;
    this->currentPage = IX_NO_PAGE;
    this->overflowPos = -1;
    this->keyPos = -1;
    // 获取第一个符合条件的索引项
    int rootPage = indexHandle.indexHeader.rootPage;
    if(rootPage == IX_NO_PAGE) {
        currentPage = IX_NO_PAGE;
        keyPos = -1;
    }
    else if((rc = getFirstEntry(indexHandle.indexHeader.rootPage, currentPage, keyPos))) {
        return rc;
    }
    return 0;       // ok
}

RC IX_IndexScan::closeScan() {
    this->isOpen = FALSE;
    return 0;   // ok
}

// getFirstEntry: 获取nodePage的子节点中第一个符合条件的key位置
// - 获取nodePage页内数据
// - 判断是否是叶节点
// - 如果是叶节点, 循环寻找第一个符合条件的key, 返回
// - 如果不是叶节点, 循环找到第一个符合条件的区间, 获取该区间对应的页号, 递归
RC IX_IndexScan::getFirstEntry(PageNum nodePage, PageNum &rFirstEntryPage, int &rKeyPos) {
    int rc;

    PF_FileHandle pfFH = indexHandle.pfFH;
    PageHandle nodePageHandle;
    char *nodeData;
    // 获取该页
    if((rc = pfFH.getThisPage(nodePage, nodePageHandle)) ||
            (rc = nodePageHandle.getData(nodeData))) {
        return rc;
    }
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)nodeData;
    // 如果是叶节点, 循环找第一个符合条件的key返回位置
    if(nodeHeader->type == LEAF || nodeHeader->type == ROOT_LEAF) {
        // 循环判断值
        if(indexHandle.indexHeader.attrType == INT) {
            if((rc = getFirstIntFromLeaf(nodeHeader, nodePage, rFirstEntryPage, rKeyPos))) {
                return rc;
            }
        }
        else if(indexHandle.indexHeader.attrType == FLOAT) {

        }
        else if(indexHandle.indexHeader.attrType == STRING) {

        }
        if((rc = pfFH.unpinPage(nodePage))) {
            return rc;
        }
    }
    // 如果不是叶节点, 循环找到第一个符合条件的interval, 递归重新执行该方法
    else if(nodeHeader->type == ROOT || nodeHeader->type == NODE){
        if(indexHandle.indexHeader.attrType == INT) {
            PageNum nextPage;
            // 获取符合条件的下一个结点
            if((rc = getIntervalFromNode(nodeHeader, nextPage))) {
                return rc;
            }
            if((rc = pfFH.unpinPage(nodePage))) {
                return rc;
            }
            return getFirstEntry(nextPage, rFirstEntryPage, rKeyPos);
        }
        else if(indexHandle.indexHeader.attrType == FLOAT) {

        }
        else if(indexHandle.indexHeader.attrType == STRING) {

        }
    }
    return 0;   // ok
}

// getFirstIntFromLeaf: 从叶节点中获取第一个符合条件的int型索引项的位置
// - 循环找第一个符合条件的key返回位置
RC
IX_IndexScan::getFirstIntFromLeaf(IX_NodeHeader *nodeHeader, PageNum nodePage, PageNum &rFirstEntryPage, int &rKeyPos) {
    int rc;
    // 获取key数组和val数组
    char *keyData = (char*)nodeHeader + sizeof(IX_NodeHeader);
    int *keyArray = (int*)keyData;
    int val = *static_cast<int*>(value);
    // 循环查找符合条件的索引项
    int numberKeys = nodeHeader->numberKeys;
    int bFound = FALSE; // 标记是否找到
    for(int i = 0; i < numberKeys; ++i) {
        // 判断当前位置的索引key值是否符合条件
        // 找到符合条件的索引项, 返回
        if(matchIndex(keyArray[i], val)) {
            rFirstEntryPage = nodePage;
            rKeyPos = i;
            bFound = TRUE;
            break;
        }
    }
    // 如果在nodePage中没有找到符合条件的结点, 则将currentPage置为IX_NO_PAGE
    if(bFound == FALSE) {
        currentPage = IX_NO_PAGE;
        keyPos = -1;
    }
    return 0;
}

template<typename T>
bool IX_IndexScan::matchIndex(T keyValue, T givenValue) const {
    bool recordMatch = false;
    switch(compOp) {
        case EQ_OP:
            if (keyValue == givenValue) recordMatch = true;
            break;
        case LT_OP:
            if (keyValue < givenValue) recordMatch = true;
            break;
        case GT_OP:
            if (keyValue > givenValue) recordMatch = true;
            break;
        case LE_OP:
            if (keyValue <= givenValue) recordMatch = true;
            break;
        case GE_OP:
            if (keyValue >= givenValue) recordMatch = true;
            break;
        case NE_OP:
            if (keyValue != givenValue) recordMatch = true;
            break;
        default:
            recordMatch = true;
            break;
    }
    return recordMatch;
}

// getIntervalFromNode: 获取该node的符合条件的下一个结点(叶节点或非叶节点)并返回
// - 获取key数组和val数组
// - 循环找到符合条件的下一个结点
RC IX_IndexScan::getIntervalFromNode(IX_NodeHeader *nodeHeader, PageNum &nextPage) {
    int rc;

    // 获取key数组和val数组
    char *keyData = (char*)nodeHeader + sizeof(IX_NodeHeader);
    char *valueData = keyData + indexHandle.indexHeader.attrLength * indexHandle.indexHeader.degree;
    int *keyArray = (int*)keyData;
    IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
    int val = *static_cast<int*>(value);
    int bFound = FALSE;
    // 寻找val值所在的区间
    int numberKeys = nodeHeader->numberKeys;
    // 判断第一个区间
    if(val < keyArray[0] || compOp == NO_OP) {
        nextPage = valueArray[0].nextPage;
        bFound = TRUE;
    }
    // 判断最后一个区间
    else if(val >= keyArray[numberKeys] || compOp == NO_OP) {
        nextPage = valueArray[numberKeys].nextPage;
        bFound = TRUE;
    }
    // 判断中间(numberKeys - 1)个区间, 返回nextPage
    else {
        for(int i = 1; i < numberKeys; ++i) {
            if(matchInterval(keyArray[i - 1], keyArray[i], val)) {
                nextPage = valueArray[i].nextPage;
                bFound = TRUE;
                break;
            }
        }
    }

    if(bFound == FALSE) {
        currentPage = IX_NO_PAGE;
        keyPos = -1;
    }
    return 0;
}

// getNextEntry: 获取下一个符合条件的索引项
// - 查看是否还有页能够遍历
// - 如果没有, 则直接返回
// - 如果有, 则继续遍历
// - 查看当前是否处于溢出块
// - 如果处于溢出块, 则直接获取rid返回, 并将overflowPos++, 判断是否超界, 调整overflowPos和keyPos
// - 如果不是溢出块, 则将当前位置的索引值和value做比较
// - 如果符合条件则直接返回该位置
// - 如果不符合条件, 则按照寻找下一个结点, 判断是否超界, 调整keyPos
RC IX_IndexScan::getNextEntry(RID &rid) {
    if(!isOpen) {
        return IX_SCAN_CLOSED;
    }
    // 查看是否还有页能够遍历
    if(currentPage == IX_NO_PAGE) {
        return IX_EOF;
    }
    int rc;

    // 查看当前是否处于溢出块
    // 处于溢出块, 获取rid返回
    if(inOverflow) {
        // 获取当前页的数据
        PageHandle pageHandle;
        char *pData;
        int pageMemento = currentPage;
        if((rc = indexHandle.pfFH.getThisPage(currentPage, pageHandle)) ||
           (rc = pageHandle.getData(pData))) {
            return rc;
        }
        // 获取块内数据
        auto overflowPageHeader = (IX_OverflowPageHeader*)pData;
        RID *rids = (RID*)(overflowPageHeader + sizeof(IX_OverflowPageHeader));
        // 获取rid
        rid = rids[overflowPos];
        if((rc = justifyNextPos())) {
            return rc;
        }
        if((rc = indexHandle.pfFH.unpinPage(pageMemento))) {
            return rc;
        }
    }
    // 不是溢出块, 返回下一个符合条件的索引位置
    else {
        AttrType attrType = indexHandle.indexHeader.attrType;
        if(attrType == INT) {
            return getNextIntEntry(rid);
        }
        else if(attrType == FLOAT) {

        }
        else if(attrType == STRING) {

        }
    }
    return 0;   // ok
}

// getNextIntEntry
RC IX_IndexScan::getNextIntEntry(RID &rid) {
    int rc;
    // 获取当前页的数据
    PageHandle pageHandle;
    char *pData;
    // 用于记录currentPage
    int pageMemento = currentPage;
    if((rc = indexHandle.pfFH.getThisPage(currentPage, pageHandle)) ||
       (rc = pageHandle.getData(pData))) {
        return rc;
    }
    char *keyData = pData + sizeof(IX_NodeHeader);
    int *keyArray = (int*)keyData;
    int intVal = *static_cast<int*>(value);
    // 如果当前位置的值符合条件, 则直接返回rid, 并换到下一个位置
    if(matchIndex(keyArray[keyPos], intVal)) {
        char *valueData = keyData + indexHandle.indexHeader.attrLength * indexHandle.indexHeader.degree;
        IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
        rid = valueArray[keyPos].rid;
        // 释放原pageMemento页
        if((rc = indexHandle.pfFH.unpinPage(pageMemento))) {
            return rc;
        }
        // 切换到下一个位置
        if((rc = justifyNextPos())) {
            return rc;
        }
    }
    // 如果不符合条件, 由于索引key是有序的, 所以不需要继续查找了, 直接返回
    else {
        // 先释放页
        if((rc = indexHandle.pfFH.unpinPage(pageMemento))) {
            return rc;
        }
        currentPage = IX_NO_PAGE;
        return IX_EOF;
    }
    return 0;
}

// justifyNextPos: 调整当前扫描的位置到下一个索引项
RC IX_IndexScan::justifyNextPos() {
    if(!isOpen) {
        return IX_SCAN_CLOSED;
    }
    int rc;
    PageHandle pageHandle;
    char *pData;
    if((rc = indexHandle.pfFH.getThisPage(currentPage, pageHandle)) ||
       (rc = pageHandle.getData(pData))) {
        return rc;
    }
    // 用于记录currentPage的值, 防止currentPage被修改导致无法unpin
    PageNum pageMemento = currentPage;
    // 判断是否处于溢出块
    // 如果处于溢出块, 则将overflowPos++, 并判断是否越界
    if(inOverflow) {
        // ++overflowPos 并检查是否超界
        ++overflowPos;
        auto overflowPageHeader = (IX_OverflowPageHeader*)pData;
        // 如果超界, 则调整keyPos的位置到下一个结点的0或当前结点的(keyPos + 1)
        if(overflowPos >= overflowPageHeader->numberRecords) {
            // 获取原页
            int parentPage = overflowPageHeader->parentPage;
            PageHandle parentPageHandle;
            char *parentPageData;
            if((rc = indexHandle.pfFH.getThisPage(parentPage, parentPageHandle)) ||
               (rc = parentPageHandle.getData(parentPageData))) {
                return rc;
            }
            // 获取原页的页内数据
            IX_NodeHeader *nodeHeader = (IX_NodeHeader*)parentPageData;
            // 调整keyPos, 并判断调整后的keyPos是否越界
            ++keyPos;
            inOverflow = FALSE;
            overflowPos = 0;
            currentPage = parentPage;
            // 如果发生越界, 则调整到下一个节点的位置
            if(keyPos == nodeHeader->numberKeys) {
                // 获取valueArray
                int offset = sizeof(IX_NodeHeader) + indexHandle.indexHeader.attrLength * indexHandle.indexHeader.degree;
                auto valueArray = (IX_NodeValue*)(parentPageData + offset);
                // 调整
                currentPage = valueArray[indexHandle.indexHeader.degree].nextPage;
                keyPos = 0;
            }
            // 调整完毕, unpin page
            if((rc = indexHandle.pfFH.unpinPage(parentPage))) {
                return rc;
            }
        }
    }
    // 如果没有处于溢出块, 则调整keyPos++, 并判断是否越界
    else {
        IX_NodeHeader *nodeHeader = (IX_NodeHeader*)pData;
        ++keyPos;
        if(keyPos >= nodeHeader->numberKeys) {
            int offset = sizeof(IX_NodeHeader) + indexHandle.indexHeader.attrLength * indexHandle.indexHeader.degree;
            IX_NodeValue *valueArray = (IX_NodeValue*)(pData + offset);
            // 调整
            currentPage = valueArray[indexHandle.indexHeader.degree].nextPage;
            keyPos = 0;
        }
    }
    if((rc = indexHandle.pfFH.unpinPage(pageMemento))) {
        return rc;
    }
    return 0;   // ok
}

// matchInterval: 查看givenValue是否处于lVal和rVal之间
// 条件为 前开后闭
template<typename T>
bool IX_IndexScan::matchInterval(T lVal, T rVal, T givenValue) const {
    // 如果条件是无条件则直接返回true
    if(compOp == NO_OP) return true;
    return givenValue >= lVal && givenValue < rVal;
}

