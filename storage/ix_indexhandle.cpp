//
// Created by Administrator on 2020/12/5.
//
#include "ix.h"
#include <iostream>
#include <cstring>

using namespace std;

// Constructor: 初始化
IX_IndexHandle::IX_IndexHandle() : isOpen(FALSE), bHeaderModified(FALSE) {
    indexHeader.rootPage = IX_NO_PAGE;
}

IX_IndexHandle::~IX_IndexHandle() {
    // do nothing
}

// insertEntry: 插入一条索引项, key值为data, val值为rid
RC IX_IndexHandle::insertEntry(void *data, const RID &rid) {
    if(!isOpen) {
        return IX_FILE_CLOSED;
    }
    // 如果还未申请任何页, 则将根节点置为ROOT_LEAF, 并插入索引
    if(indexHeader.rootPage == IX_NO_PAGE) {
        return insertRootPage(data, rid);
    }
    // 根页存在则根据根页的类型插入索引项
    else {
        int rc;
        // 获取数据页
        PageHandle pageHandle;
        char *pData;
        PageNum pageMemento = indexHeader.rootPage;
        if((rc = pfFH.getThisPage(indexHeader.rootPage, pageHandle)) ||
           (rc = pageHandle.getData(pData))) {
            return rc;
        }
        // 提取索引页头
        IX_NodeHeader *header = (IX_NodeHeader*)pData;
        // 如果既是根也是叶结点, 则保存索引数据, 单独处理
        if(header->type == ROOT_LEAF) {
            // 先释放rootPage
            if((rc = pfFH.unpinPage(indexHeader.rootPage))) {
                return rc;
            }
            return insertRootLeaf(data, rid);
        }
        // 若根节点不是叶节点则按照算法对根节点进行递归插入
        else {
            // 先释放rootPage
            if((rc = pfFH.unpinPage(indexHeader.rootPage))) {
                return rc;
            }
            // 修改bug: pData参数改为data
            if((rc = insertEntryRecursively(data, rid, indexHeader.rootPage))) {
                return rc;
            }
        }
    }
    return 0;
}

RC IX_IndexHandle::insertRootPage(void *data, const RID &rid) {
    int rc;
    int degree = indexHeader.degree;
    int attrLength = indexHeader.attrLength;
    AttrType attrType = indexHeader.attrType;

    // 为根节点申请一页
    PageHandle pageHandle;
    if((rc = pfFH.allocatePage(pageHandle))) {
        return rc;
    }
    char *pageData;
    int pageNum;
    if((rc = pageHandle.getData(pageData)) ||
       (rc = pageHandle.getPageNum(pageNum))) {
        return rc;
    }
    if((rc = pfFH.markDirty(pageNum))) {
        return rc;
    }
    // 初始化结点头
    IX_NodeHeader *rootHeader = new IX_NodeHeader;
    rootHeader->numberKeys = 1;
    rootHeader->keyCapacity = degree;
    rootHeader->type = ROOT_LEAF;
    rootHeader->parent = IX_NO_PAGE;
    rootHeader->prePage = IX_NO_PAGE;

    // 写入节点头
    memcpy(pageData, (char*)rootHeader, sizeof(IX_NodeHeader));
    // 释放堆空间
    delete rootHeader;

    // 申请degree + 1 个空间, 方便插入
    IX_NodeValue *valueArray = new IX_NodeValue[degree + 1];
    valueArray[0].state = RID_FILLED;
    valueArray[0].rid = rid;
    valueArray[0].nextPage = IX_NO_PAGE;
    int valueOffset = sizeof(IX_NodeHeader) + degree * attrLength;
    memcpy(pageData + valueOffset, (char*)valueArray, sizeof(IX_NodeValue) * (degree + 1));
    delete [] valueArray;

    char *keyData = pageData + sizeof(IX_NodeHeader);
    if(attrType == INT) {
        int *keyArray = new int[degree];
        for(int i = 0; i < degree; ++i) {
            keyArray[i] = -1;
        }
        int givenKey = *static_cast<int*>(data);
        keyArray[0] = givenKey;
        memcpy(keyData, keyArray, attrLength * degree);
        delete [] keyArray;
    }
    else if(attrType == FLOAT) {
        float *keyArray = new float[degree];
        for(int i = 0; i < degree; ++i) {
            keyArray[i] = -1;
        }
        float givenKey = *static_cast<float*>(data);
        keyArray[0] = givenKey;
        memcpy(keyData, keyArray, attrLength * degree);
        delete [] keyArray;
    }
    else if(attrType == STRING) {
        char *keyArray = new char[attrLength * degree];
        for(int i = 0; i < attrLength * degree; ++i) {
            keyArray[i] = ' ';
        }
        char *givenKeyStr = static_cast<char*>(data);
        strcpy(keyArray, givenKeyStr);
        memcpy(keyData, keyArray, attrLength * degree);
        delete [] keyArray;
    }
    indexHeader.rootPage = pageNum;
    bHeaderModified = TRUE;

    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }

    return 0;   // ok
}

// insertEntryRecursively: 递归向pageNum叶对应的结点插入索引项
// - 遍历结点的keyList找到索引值插入的位置
// - 如果是叶结点则插入索引值
// - 如果不是叶节点则递归插入到下一层 easy
RC IX_IndexHandle::insertEntryRecursively(void *data, const RID &rid, const PageNum pageNum) {
    int rc;
    // 获取pageNum对应的页数据
    PageHandle pageHandle;
    char *pData;
    if((rc = pfFH.getThisPage(pageNum, pageHandle)) ||
                (rc = pageHandle.getData(pData))) {
        return rc;
    }
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)(pData);
    int offset = sizeof(IX_NodeHeader) + indexHeader.attrLength * indexHeader.degree;
    IX_NodeValue *valueArray = (IX_NodeValue*)(pData + offset);
    if(indexHeader.attrType == INT) {
        int val = *static_cast<int*>(data);
        int *keyArray = (int*)(pData + sizeof(IX_NodeHeader));
        int pos = 0;
        // 查找索引key值val插入的位置
        while(pos < nodeHeader->numberKeys && keyArray[pos] < val) {
            ++pos;
        }
        // 判断当前结点是否是叶节点
        // 如果当前结点是叶节点则直接插入索引值, 并对下溢出进行判断(修改结点值, 进行脏页标记)
        if(nodeHeader->type == LEAF) {
            if((rc = pfFH.markDirty(pageNum))) {
                return rc;
            }
            // 叶结点层, 索引key和索引val一一对应
            for(int i = nodeHeader->numberKeys; i > pos; --i) {
                keyArray[i] = keyArray[i - 1];
                valueArray[i] = valueArray[i - 1];
            }
            keyArray[pos] = *static_cast<int*>(data);
            valueArray[pos].rid = rid;
            valueArray[pos].state = RID_FILLED;
            valueArray[pos].nextPage = IX_NO_PAGE;
            nodeHeader->numberKeys += 1;

            // 判断是否发生下溢出
            // 发生下溢出, 进行分裂
            if(nodeHeader->numberKeys >= indexHeader.degree) {
                // 结点分裂后仍为LEAF结点
                if((rc = spliteTwoNodes(nodeHeader, pageNum, LEAF))) {
                    return rc;
                }
            }
        }
        // 如果当前遍历的结点类型不是叶结点则继续递归插入
        else {
            // pos位置正好对应递归插入的子节点
            PageNum childPage = valueArray[pos].nextPage;
            // 进行递归插入
            if((rc = insertEntryRecursively(data, rid, childPage))) {
                return rc;
            }
        }
    }
    // 插入成功, 释放pageNum页
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    return 0;   // ok
}

// insertRootLeaf: 根类型为ROOT_LEAF, 向根结点插入一条索引项
// - 找到插入索引值的位置, 对索引项进行插入
// - 查看是否发生下溢出
// - 发生下溢出, 分裂结点
RC IX_IndexHandle::insertRootLeaf(void *data, const RID &rid) {
    int rc;
    // 读取根页的数据
    PageHandle pageHandle;
    PageNum pageMemento = indexHeader.rootPage;
    char *pData;
    if((rc = pfFH.getThisPage(indexHeader.rootPage, pageHandle)) ||
            (rc = pageHandle.getData(pData))) {
        return rc;
    }
    // 插入索引项
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)pData;
    int offset = sizeof(IX_NodeHeader) + indexHeader.attrLength * indexHeader.degree;
    // 读取索引val
    IX_NodeValue *valueArray = (IX_NodeValue*)(pData + offset);
    AttrType attrType = indexHeader.attrType;
    // 处理三种类型属性做索引key的情况
    if(attrType == INT) {
        int val = *static_cast<int*>(data);
        int *keyArray = (int*)(pData + sizeof(IX_NodeHeader));
        // 查找索引项插入的位置
        int pos = 0;
        while(pos < nodeHeader->numberKeys && keyArray[pos] < val) {
            ++pos;
        }
        // 插入根叶两种情况: 索引key值已经存在; 索引key值不存在
        // 如果key值已经存在, 向该key值的溢出块中插入该rid
        if(pos < nodeHeader->numberKeys && keyArray[pos] == val) {
            PageNum overflowPage = valueArray[pos].nextPage;
            // 如果还没有溢出块则先创建后插入
            if(overflowPage == IX_NO_PAGE) {
                // 创建溢出块
                if((rc = createOverflowPage(indexHeader.rootPage, overflowPage))) {
                    return rc;
                }
            }
            // 插入rid
            if((rc = insertOverflowPage(overflowPage, rid))) {
                return rc;
            }
        }
        // 索引key值不存在, 对node进行赋值然后判断是否发生下溢出
        else {
            // 将根页标记为脏页
            if((rc = pfFH.markDirty(indexHeader.rootPage))) {
                return rc;
            }
            // 叶结点层, 索引key和索引val一一对应
            for(int i = nodeHeader->numberKeys; i > pos; --i) {
                keyArray[i] = keyArray[i - 1];
                valueArray[i] = valueArray[i - 1];
            }
            keyArray[pos] = *static_cast<int*>(data);
            valueArray[pos].rid = rid;
            valueArray[pos].state = RID_FILLED;
            valueArray[pos].nextPage = IX_NO_PAGE;
            // 判断是否发生下溢出
            nodeHeader->numberKeys += 1;
            // 发生下溢出, 发生分裂
            if(nodeHeader->numberKeys >= indexHeader.degree) {
                // 根节点分裂后成为叶节点
                if((rc = spliteTwoNodes(nodeHeader, indexHeader.rootPage, LEAF))) {
                    return rc;
                }
            }
            // 插入结束
        }
    }
    else if(attrType == FLOAT) {
        float val = *static_cast<float*>(data);
        float *keyArray = (float*)(pData + sizeof(IX_NodeHeader));
        // 查找索引项插入的位置
        int pos = 0;
        while(pos < nodeHeader->numberKeys && keyArray[pos] < val) {
            ++pos;
        }
        for(int i = nodeHeader->numberKeys; i > pos ; --i) {
            keyArray[i] = keyArray[i - 1];
        }
        keyArray[pos] = val;
    }
    else if(attrType == STRING) {
        char *val = static_cast<char*>(data);
        char *strArray = new char[indexHeader.attrLength * indexHeader.degree];
        int pos = 0;
        char *tmpStr = strArray;
        while(pos < nodeHeader->numberKeys && strcmp(tmpStr, val) < 0) {
            ++pos;
            tmpStr += indexHeader.attrLength;
        }
        strcpy(tmpStr, val);
    }
    // 释放原结点
    if((rc = pfFH.unpinPage(pageMemento))) {
        return rc;
    }
    return 0;   // ok
}

// inserOverflowPage: 向溢出块插入一条rid记录
RC IX_IndexHandle::insertOverflowPage(PageNum pageNum, const RID &rid) {
    int rc;
    PageHandle pageHandle;
    char *pData;
    if((rc = pfFH.getThisPage(pageNum, pageHandle)) ||
            (rc = pageHandle.getData(pData))) {
        return rc;
    }
    // 将pageNum页标记为脏页
    if((rc = pfFH.markDirty(pageNum))) {
        return rc;
    }
    auto overflowPageHeader = (IX_OverflowPageHeader*)pData;
    // 查看记录是否已经被插满
    if(overflowPageHeader->numberRecords == overflowPageHeader->recordCapacity) {
        return IX_RID_FULL;
    }
    RID *rids = (RID*)(pData + sizeof(IX_OverflowPageHeader));
    // 检查rid是否已经存在
    for(int i = 0; i < overflowPageHeader->numberRecords; ++i) {
        if(sameRID(rid, rids[i])) {
            return IX_RID_EXISTED;
        }
    }
    // 如果不存在, 则插入
    rids[overflowPageHeader->numberRecords] = rid;
    overflowPageHeader->numberRecords += 1;
    // unpin page
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    return 0;   // ok
}

// createOverflowPage: 创建溢出块, 并返回pageNum
RC IX_IndexHandle::createOverflowPage(PageNum parentPage, PageNum &pageNum) {
    int rc;
    PageHandle pageHandle;
    char *pData;
    if((rc = pfFH.allocatePage(pageHandle)) ||
            (rc = pageHandle.getData(pData)) ||
            (rc = pageHandle.getPageNum(pageNum))) {
        return rc;
    }
    // 将页标记为脏页, 用于写回数据
    if((rc = pfFH.markDirty(pageNum))) {
        return rc;
    }
    // 写入头部信息
    IX_OverflowPageHeader overflowPageHeader{};
    overflowPageHeader.parentPage = parentPage;
    overflowPageHeader.recordCapacity = RID_COUNT_OF_OVERFLOW_PAGE;
    overflowPageHeader.numberRecords = 0;
    memcpy(pData, (char*)&overflowPageHeader, sizeof(IX_OverflowPageHeader));
    // 写入rid记录信息
    RID *rids = new RID[RID_COUNT_OF_OVERFLOW_PAGE];
    memcpy(pData + sizeof(IX_OverflowPageHeader), (char*)rids, sizeof(RID) * RID_COUNT_OF_OVERFLOW_PAGE);
    // unpin page
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    return 0;   // ok
}

bool IX_IndexHandle::sameRID(const RID &ridA, const RID &ridB) const {
    int rc;
    PageNum pageNumA, pageNumB;
    SlotNum slotNumA, slotNumB;
    if((rc = ridA.getPageNum(pageNumA)) ||
            (rc = ridA.getSlotNum(slotNumA)) ||
            (rc = ridB.getPageNum(pageNumB)) ||
            (rc = ridB.getSlotNum(slotNumB))) {
        return rc;
    }
    return pageNumA == pageNumB && slotNumA == slotNumB;
}

// spliteTwoNodes: leftNodePage分裂成两个结点, nodeHeader为该页的IX_NodeHeader
// - nodeHeader: 分裂的原结点, targetType: 结点分类后成为的类型, leftNodePage: 原结点的页号
// - 如果是叶节点, 则分裂后修改结点的parentPage, 及后继节点的指针
RC IX_IndexHandle::spliteTwoNodes(IX_NodeHeader *nodeHeader, PageNum leftNodePage, IX_NodeType targetType) {
    int rc;
    IX_NodeHeader newNodeHeader{};
    if(indexHeader.attrType == INT) {
        char *keyData = (char*)nodeHeader + sizeof(IX_NodeHeader);
        char *valueData = keyData + indexHeader.attrLength * indexHeader.degree;
        int *keyArray = (int*)keyData;
        IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
        int *newKeyArray = new int[indexHeader.degree];
        IX_NodeValue *newValueArray = new IX_NodeValue[indexHeader.degree + 1];
        for(int i = indexHeader.degree / 2; i < nodeHeader->numberKeys; ++i) {
            newKeyArray[i - indexHeader.degree / 2] = keyArray[i];
            newValueArray[i - indexHeader.degree / 2] = valueArray[i];
        }
        // 为新结点申请页
        PageHandle pageHandle;
        char *pData;
        PageNum rightNodePage;
        // 获取申请页的地址
        if((rc = pfFH.allocatePage(pageHandle)) ||
                (rc = pageHandle.getData(pData)) ||
                (rc = pageHandle.getPageNum(rightNodePage))) {
            delete [] newKeyArray;
            delete [] newValueArray;
            return rc;
        }
        // 将申请页标记脏页
        if((rc = pfFH.markDirty(rightNodePage))) {
            delete [] newKeyArray;
            delete [] newValueArray;
            return rc;
        }
        // 写入数据
        char *pKey = pData + sizeof(IX_NodeHeader);
        char *pVal = pKey + indexHeader.attrLength * indexHeader.degree;
        newNodeHeader.numberKeys = nodeHeader->numberKeys - nodeHeader->numberKeys / 2;
        newNodeHeader.keyCapacity = indexHeader.degree;

        // 分裂后的类型为targetType
        newNodeHeader.type = targetType;
        newNodeHeader.parent = nodeHeader->parent;

        // 新生成的子节点的前继节点是左边的结点
        newNodeHeader.prePage = leftNodePage;

        // 如果分裂后是叶节点则需要修改结点的前继后继指针
        if(targetType == LEAF) {
            // rightNode的后继节点是leftNode的后继节点, leftNode的前继结点是leftNode, leftNode的后继节点修改为rightNode
            newValueArray[indexHeader.degree].nextPage = valueArray[indexHeader.degree].nextPage;
            valueArray[indexHeader.degree].nextPage = rightNodePage;
            newNodeHeader.prePage = leftNodePage;
        }
        memcpy(pData, (char*)&newNodeHeader, sizeof(IX_NodeHeader));
        memcpy(pKey, (char*)newKeyArray, indexHeader.attrLength * indexHeader.degree);
        memcpy(pVal, (char*)newValueArray, sizeof(IX_NodeValue) * (indexHeader.degree + 1));
        // 修改分裂节点保存的值
        nodeHeader->numberKeys = nodeHeader->numberKeys / 2;
        nodeHeader->type = targetType;
        // 如果是叶节点分裂, 则修改后继节点指针
        if(targetType == LEAF || targetType == ROOT_LEAF) {
            PageNum tmp = valueArray[indexHeader.degree].nextPage;
            valueArray[indexHeader.degree].nextPage = rightNodePage;
            newValueArray[indexHeader.degree].nextPage = tmp;

        }

        // 判断parent是否为IX_NO_PAGE
        // 如果是则先生成新的根结点之后插入结点值, 否则直接插入, 并将rootPage赋值为新生成的根节点
        if(nodeHeader->parent == IX_NO_PAGE) {
            // 新生成结点的类型为根节点
            IX_NodeHeader newRootHeader{};
            newRootHeader.numberKeys = 1;
            newRootHeader.type = ROOT;
            newRootHeader.parent = IX_NO_PAGE;
            newRootHeader.keyCapacity = indexHeader.degree;
            newRootHeader.prePage = IX_NO_PAGE;
            int *newRootKeyArray = new int[indexHeader.degree];
            IX_NodeValue *newRootValueArray = new IX_NodeValue[indexHeader.degree + 1];
            newRootKeyArray[0] = newKeyArray[0];
            // 生成指向两个叶节点的指针
            newRootValueArray[0].state = PAGE_ONLY;
            newRootValueArray[0].nextPage = leftNodePage;
            newRootValueArray[1].state = PAGE_ONLY;
            newRootValueArray[1].nextPage = rightNodePage;
            // 为新结点申请新页
            PageHandle newRootPageHandle;
            char *newRootData;
            PageNum newRootPage;
            if((rc = pfFH.allocatePage(newRootPageHandle)) ||
                    (rc = newRootPageHandle.getData(newRootData)) ||
                    (rc = newRootPageHandle.getPageNum(newRootPage))) {
                return rc;
            }
            // 将根节点赋值为新生成的根节点
            this->indexHeader.rootPage = newRootPage;
            this->bHeaderModified = TRUE;

            // 数据页标记为脏
            if((rc = pfFH.markDirty(newRootPage))) {
                return rc;
            }
            char *newRootKey = (char*)(newRootData + sizeof(IX_NodeHeader));
            char *newRootValue = (char*)(newRootKey + indexHeader.attrLength * indexHeader.degree);
            // 将数据写入新页
            memcpy(newRootData, (char*)&newRootHeader, sizeof(IX_NodeHeader));
            memcpy(newRootKey, (char*)newRootKeyArray, indexHeader.attrLength * indexHeader.degree);
            memcpy(newRootValue, (char*)newRootValueArray, sizeof(IX_NodeValue) * (indexHeader.degree + 1));
            delete [] newRootKeyArray;
            delete [] newRootValueArray;
            // 修改两个子节点的父指针, 插入完成
            nodeHeader->parent = newRootPage;
            newNodeHeader.parent = newRootPage;
            memcpy(pData, (char*)&newNodeHeader, sizeof(IX_NodeHeader));
            if((rc = pfFH.unpinPage(rightNodePage)) ||
                    (rc = pfFH.unpinPage(newRootPage))) {
                delete [] newKeyArray;
                delete [] newValueArray;
                return rc;
            }
        }
        // 如果有父节点则直接插入结点值
        else {
            PageNum parentPage = nodeHeader->parent;
            int newKey = newKeyArray[0];
            // 获取父节点数据页
            PageHandle parentPageHandle;
            char *parentData;
            if((rc = pfFH.getThisPage(parentPage, parentPageHandle)) ||
               (rc = parentPageHandle.getData(parentData))) {
                return rc;
            }
            // 将该页标记为脏页
            if((rc = pfFH.markDirty(parentPage))) {
                return rc;
            }
            IX_NodeHeader *parentHeader = (IX_NodeHeader*)parentData;
            // 插入结点, 查找插入的位置, 进行插入
            char *keyData = parentData + sizeof(IX_NodeHeader);
            char *valueData = keyData + indexHeader.attrLength * indexHeader.degree;
            int *parentKeyArray = (int*)keyData;
            IX_NodeValue *parentValueArray = (IX_NodeValue*)valueData;
            int pos = 0;
            // 查找newKey在父节点中插入的位置
            while(pos < parentHeader->numberKeys && parentKeyArray[pos] < newKey) {
                ++pos;
            }
            for(int i = parentHeader->numberKeys; i > pos; --i) {
                parentKeyArray[i] = parentKeyArray[i - 1];
                parentValueArray[i + 1] = parentValueArray[i];
            }
            parentKeyArray[pos] = newKey;
            parentValueArray[pos + 1].nextPage = rightNodePage;
            parentValueArray[pos + 1].state = PAGE_ONLY;
            parentHeader->numberKeys += 1;
            // 查看是否发生下溢出, 若发生下溢出递归向上分裂
            if(parentHeader->numberKeys >= indexHeader.degree) {
                // 如果有子节点, 则分裂后成为NODE类型, 若没有子节点则分裂后成为LEAF类型
                if((rc = spliteTwoNodes(parentHeader, parentPage, NODE))) {
                    return rc;
                }
            }
            // 分裂成功
            if((rc = pfFH.unpinPage(parentPage)) ||
                    (rc = pfFH.unpinPage(rightNodePage))) {
                return rc;
            }
        }
    }
    else if(indexHeader.attrType == FLOAT) {

    }
    else if(indexHeader.attrType == STRING) {

    }
    return 0;   // ok
}

/*!
 *
 * @param pData : 索引key
 * @param rid : 索引val, 由于能够存储重复的索引key, 所以删除的时候要将索引val也提供出来
 * @return : 无
 */
RC IX_IndexHandle::deleteEntry(void *pData, const RID &rid) {
    int rc;
    // 检查索引是否打开
    if(!isOpen) {
        return IX_FILE_CLOSED;
    }
    // 如果根节点为空, 则返回未找到
    if(indexHeader.rootPage == IX_NO_PAGE) {
        return IX_NOT_FOUND;
    }
    // 获取根叶
    PageHandle rootPageHandle;
    char *rootData;
    if((rc = pfFH.getThisPage(indexHeader.rootPage, rootPageHandle)) ||
            (rc = rootPageHandle.getData(rootData))) {
        return rc;
    }
    PageNum pageMemento = indexHeader.rootPage;
    IX_NodeHeader *rootNodeHeader = (IX_NodeHeader*)rootData;
    // 根节点为叶节点不需要递归向下查找
    if(rootNodeHeader->type == ROOT_LEAF) {
        int keyPos;
        // 从叶节点中查找该key值的位置
        if((rc = searchEntryFromLeaf(pData, indexHeader.rootPage, keyPos))) {
            return rc;
        }
        if((rc = deleteFromLeaf(indexHeader.rootPage, keyPos, rid))) {
            return rc;
        }
    }
    // 根节点为非页结点需要递归查找pData所在的区间
    else {
        int leafPage;
        int keyPos;
        // 查找叶节点所在的页
        if((rc = searchEntryFromNode(pData, indexHeader.rootPage, leafPage))) {
            return rc;
        }
        if((rc = searchEntryFromLeaf(pData, leafPage, keyPos))) {
            return rc;
        }
        if((rc = deleteFromLeaf(leafPage, keyPos, rid))) {
            return rc;
        }
    }
    if((rc = pfFH.unpinPage(pageMemento))) {
        return rc;
    }
    return 0;   // ok
}

// searchEntryFromNode: 从非叶结点nodePage上查找到pData所在的区间并返回最后的叶节点
// - 获取nodePage页, 判断是否是叶结点
// - 如果是, 直接返回
// - 如果不是, 循环查找区间, 递归
RC IX_IndexHandle::searchEntryFromNode(void *pData, PageNum nodePage, PageNum &rReafPage) const {
    int rc;
    PageHandle nodePageHandle;
    char *nodeData;
    if((rc = pfFH.getThisPage(nodePage, nodePageHandle)) ||
            (rc = nodePageHandle.getData(nodeData))) {
        return rc;
    }

    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)nodeData;
    // 如果是叶节点则直接返回
    if(nodeHeader->type == LEAF) {
        if((rc = pfFH.unpinPage(nodePage))) {
            return rc;
        }
        rReafPage = nodePage;
    }
    // 如果是非叶结点则循环查找区间, 递归执行
    else {
        PageNum nextPage;
        if((rc = getIntervalFromNode(pData, nodeHeader, nextPage))) {
            return rc;
        }
        if((rc = pfFH.unpinPage(nodePage))) {
            return rc;
        }
        return searchEntryFromNode(pData, nextPage, rReafPage);
    }
    return 0;
}

RC IX_IndexHandle::searchEntryFromLeaf(void *value, PageNum leafPage, int &keyPos) const {
    int rc;
    PageHandle pageHandle;
    char *pData;
    if((rc = pfFH.getThisPage(leafPage, pageHandle)) ||
            (pageHandle.getData(pData))) {
        return rc;
    }
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)pData;
    int bFound = FALSE;
    if(indexHeader.attrType == INT) {
        int intVal = *static_cast<int*>(value);
        int *keyArray = (int*)(pData + sizeof(IX_NodeHeader));
        for(int i = 0; i < nodeHeader->numberKeys; ++i) {
            if(keyArray[i] == intVal) {
                keyPos = i;
                bFound = TRUE;
                break;
            }
        }
    }
    else if(indexHeader.attrType == FLOAT) {

    }
    else if(indexHeader.attrType == STRING) {

    }
    if((rc = pfFH.unpinPage(leafPage))) {
        return rc;
    }
    if(bFound == FALSE) {
        return IX_NOT_FOUND;
    }
    return 0;
}

RC IX_IndexHandle::deleteFromLeaf(PageNum pageNum, int keyPos, const RID &rid) {
    int rc;
    PageHandle pageHandle;
    char *pData;
    int bFound = FALSE;
    int disposed = FALSE;
    if((rc = pfFH.getThisPage(pageNum, pageHandle)) ||
            (rc = pageHandle.getData(pData))) {
        return rc;
    }
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)pData;
    if(indexHeader.attrType == INT) {
        char *valueData = pData + sizeof(IX_NodeHeader) + indexHeader.attrLength * indexHeader.degree;
        int *keyArray = (int*)(pData + sizeof(IX_NodeHeader));
        IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
        // 找到删除项
        if((sameRID(rid, valueArray[keyPos].rid))) {
            bFound = TRUE;
            // 将该页标记为脏页
            if((rc = pfFH.markDirty(pageNum))) {
                return rc;
            }
            // 判断是否有溢出块
            // 如果有溢出块, 则将溢出块中的rid拿出一个放到该索引项中
            if(valueArray[keyPos].nextPage != IX_NO_PAGE) {
                int overflowPage = valueArray[keyPos].nextPage;
                PageHandle overflowPageHandle;
                char *overflowPageData;
                if((rc = pfFH.getThisPage(overflowPage, overflowPageHandle)) ||
                        (rc = overflowPageHandle.getData(overflowPageData))) {
                    return rc;
                }
                // 将溢出块标记为脏页
                if((rc = pfFH.markDirty(overflowPage))) {
                    return rc;
                }
                IX_OverflowPageHeader *overflowPageHeader = (IX_OverflowPageHeader*)overflowPageData;
                RID *rids = (RID*)(overflowPageData + sizeof(IX_OverflowPageHeader));
                valueArray[keyPos].rid = rids[overflowPageHeader->numberRecords - 1];
                overflowPageHeader->numberRecords -= 1;
                if(overflowPageHeader->numberRecords == 0) {
                    valueArray[keyPos].nextPage = IX_NO_PAGE;
                    if((rc = pfFH.unpinPage(overflowPage))) {
                        return rc;
                    }
                    if((rc = pfFH.disposePage(overflowPage))) {
                        return rc;
                    }
                }
                else {
                    if((rc = pfFH.unpinPage(overflowPage))) {
                        return rc;
                    }
                }
            }
            // 如果没有, 则删除该索引项
            else {
                for(int i = keyPos; i < nodeHeader->numberKeys - 1; ++i) {
                    keyArray[i] = keyArray[i + 1];
                    valueArray[i] = valueArray[i + 1];
                }
                nodeHeader->numberKeys -= 1;
                // 判断是否索引项的数量减为0
                if(nodeHeader->numberKeys == 0) {
                    // 需要释放
                    disposed = TRUE;
                    // 修改指针值
                    PageNum prePage = nodeHeader->prePage;  // 前继结点
                    PageNum nextPage = valueArray[indexHeader.degree].nextPage; // 后继结点
                    // 修改前结点的后继指针
                    if(prePage != IX_NO_PAGE) {
                        PageHandle prePageHandle;
                        char *prePageData;
                        if((rc = pfFH.getThisPage(prePage, prePageHandle)) ||
                                (rc = prePageHandle.getData(prePageData))) {
                            return rc;
                        }
                        // 标记为脏页
                        if((rc = pfFH.markDirty(prePage))) {
                            return rc;
                        }
                        char *prePageValueData = prePageData + sizeof(IX_NodeHeader) + indexHeader.attrLength * indexHeader.degree;
                        IX_NodeValue *prePageValueArray = (IX_NodeValue*)prePageValueData;;
                        prePageValueArray[indexHeader.degree].nextPage = nextPage;
                        if((rc = pfFH.unpinPage(prePage))) {
                            return rc;
                        }
                    }
                    // 修改后结点的前继指针
                    if(nextPage != IX_NO_PAGE) {
                        PageHandle nextPageHandle;
                        char *nextPageData;
                        if((rc = pfFH.getThisPage(nextPage, nextPageHandle)) ||
                                (nextPageHandle.getData(nextPageData))) {
                            return rc;
                        }
                        // 标记为脏页
                        if((rc = pfFH.markDirty(nextPage))) {
                            return rc;
                        }
                        IX_NodeHeader *nextNodeHeader = (IX_NodeHeader*)nextPageData;
                        nextNodeHeader->prePage = prePage;
                        if((rc = pfFH.unpinPage(nextPage))) {
                            return rc;
                        }
                    }
                    PageNum parentNode = nodeHeader->parent;
                    if((rc = removeChildNode(parentNode, pageNum))) {
                        return rc;
                    }
                }
            }
        }
        // 未找到
        else {
            // 从溢出块开始找
            int overflowPage = valueArray[keyPos].nextPage;
            // 如果有溢出块
            if(overflowPage != IX_NO_PAGE) {
                PageHandle overflowPageHandle;
                char *overflowPageData;
                if((rc = pfFH.getThisPage(overflowPage, overflowPageHandle)) ||
                   (rc = overflowPageHandle.getData(overflowPageData))) {
                    return rc;
                }
                IX_OverflowPageHeader *overflowPageHeader = (IX_OverflowPageHeader*)overflowPageData;
                RID *rids = (RID*)(overflowPageData + sizeof(IX_OverflowPageHeader));
                for(int i = 0; i < overflowPageHeader->numberRecords; ++i) {
                    // 如果找到则删除
                    if(sameRID(rid, rids[i])) {
                        if((rc = pfFH.markDirty(overflowPage))) {
                            return rc;
                        }
                        bFound = TRUE;
                        for(int j = i; j < overflowPageHeader->numberRecords - 1; ++j) {
                            rids[j] = rids[j + 1];
                        }
                        overflowPageHeader->numberRecords -= 1;
                        break;
                    }
                }
                // 如果溢出块没有rid, 则删除溢出块
                if(overflowPageHeader->numberRecords == 0) {
                    valueArray[keyPos].nextPage = IX_NO_PAGE;
                    if((rc = pfFH.unpinPage(overflowPage))) {
                        return rc;
                    }
                    if((rc = pfFH.disposePage(overflowPage))) {
                        return rc;
                    }
                }
                else {
                    if((rc = pfFH.unpinPage(overflowPage))) {
                        return rc;
                    }
                }
            }
        }
    }
    else if(indexHeader.attrType == FLOAT) {

    }
    else if(indexHeader.attrType == STRING) {

    }
    if((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }
    if(disposed) {
        if((rc = pfFH.disposePage(pageNum))) {
            return rc;
        }
    }
    if(bFound == FALSE) {
        return IX_NOT_FOUND;
    }
    return 0;
}

RC IX_IndexHandle::getIntervalFromNode(void *value, IX_NodeHeader *nodeHeader, PageNum &nextPage) const {
    int rc;

    // 获取key数组和val数组
    char *keyData = (char*)nodeHeader + sizeof(IX_NodeHeader);
    char *valueData = keyData + indexHeader.attrLength * indexHeader.degree;
    int *keyArray = (int*)keyData;
    IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
    int val = *static_cast<int*>(value);
    int bFound = FALSE;
    // 寻找val值所在的区间
    int numberKeys = nodeHeader->numberKeys;
    // 判断第一个区间
    if(val < keyArray[0]) {
        nextPage = valueArray[0].nextPage;
        bFound = TRUE;
    }
    // 判断最后一个区间
    else if(val >= keyArray[numberKeys - 1]) {
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

    return 0;
}

template<typename T>
bool IX_IndexHandle::matchInterval(T lVal, T rVal, T givenValue) const {
    return givenValue >= lVal && givenValue < rVal;
}

/*!
 *
 * @param parentNode : 从parentNode页中移除子节点childNode
 * @param childNode : 子节点号
 * @return : 无
 */
RC IX_IndexHandle::removeChildNode(PageNum parentNode, PageNum childNode) {
    int rc;
    PageHandle parentPageHandle;
    char *parentData;
    if((rc = pfFH.getThisPage(parentNode, parentPageHandle)) ||
            (rc = parentPageHandle.getData(parentData))) {
        return rc;
    }
    // 标记为脏页
    if((rc = pfFH.markDirty(parentNode))) {
        return rc;
    }
    IX_NodeHeader *nodeHeader = (IX_NodeHeader*)parentData;
    char *keyData = parentData + sizeof(IX_NodeHeader);
    char *valueData = keyData + indexHeader.attrLength * indexHeader.degree;
    if((indexHeader.attrType == INT)) {
        int *keyArray = (int*)keyData;
        IX_NodeValue *valueArray = (IX_NodeValue*)valueData;
        // 遍历找到子节点的位置, 由于是遍历valueArray, 子节点的数量为numberKeys + 1
        int childPos;
        for(int i = 0; i < nodeHeader->numberKeys + 1; ++i) {
            // 找到子节点的位置
            if(valueArray[i].nextPage == childNode) {
                childPos = i;
                break;
            }
        }
        // 如果是第一个子节点, 则先移动边界处的一个子节点
        if(childPos == 0) {
            valueArray[0] = valueArray[1];
            childPos = 1;
        }
        for(int i = childPos; i < nodeHeader->numberKeys; ++i) {
            valueArray[i] = valueArray[i + 1];
            keyArray[i - 1] = keyArray[i];
        }
        nodeHeader->numberKeys -= 1;
        int numberKeys = nodeHeader->numberKeys;
        int pparentNode = nodeHeader->parent;
        // 释放
        if((rc = pfFH.unpinPage(parentNode))) {
            return rc;
        }
        // 如果也成为了空结点, 则递归向上移除子节点
        // 修改bug: 因为numberKeys的值是(节点数 - 1), 所以使用(numberKeys + 1）进行比较
        if(numberKeys + 1 == 0) {
            // 如果pparentNode == IX_NO_PAGE则表示根节点也为空
            if(pparentNode == IX_NO_PAGE) {
                indexHeader.rootPage = IX_NO_PAGE;
                bHeaderModified = TRUE;
            }
            else {
                if((rc = removeChildNode(pparentNode, parentNode))) {
                    return rc;
                }
            }
        }
    }
    else if(indexHeader.attrType == FLOAT) {

    }
    else if(indexHeader.attrType == STRING) {

    }
    return 0;
}