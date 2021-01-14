//
// Created by Administrator on 2020/12/5.
//

#ifndef MYDATABASE_IX_H
#define MYDATABASE_IX_H

#include "../global.h"
#include "pf.h"
#include "rm.h"
#include "ix_internal.h"

#define CREATE_OVERFLOW_PAGE 1;

// IX_FileHeader: 索引文件头结构
struct IX_FileHeader {
    AttrType attrType;
    int attrLength;
    PageNum rootPage;
    int degree;         // B+树的度, 结点内中最多有(degree - 1)个索引值, 每个结点最多有degree个子结点
};

// IX_Entry: 索引项的结构
struct IX_Entry {
    void *keyValue;
    RID rid;
};

// IX_IndexHandle
class IX_Manager;
class IX_IndexScan;
class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;
public:
    IX_IndexHandle();
    ~IX_IndexHandle();
    // 插入索引项
    RC insertEntry(void *pData, const RID &rid);
    RC deleteEntry(void *pData, const RID &rid);
private:
    RC insertRootPage(void *data, const RID &rid);                                  // 创建索引根项并且插入数据
    RC insertRootLeaf(void *data, const RID &rid);                                  // 向索引根页插入一条索引项
    RC insertEntryRecursively(void *data, const RID &rid, const PageNum pageNum);   // 递归插入数据, 由接口调用
    RC createOverflowPage(PageNum parentPage, PageNum &pageNum);                    // 创建溢出块返回创建的页号
    RC insertOverflowPage(PageNum pageNum, const RID &rid);                         // 向溢出块中插入rid记录
    // 块分裂成两块
    RC spliteTwoNodes(IX_NodeHeader *nodeHeader, PageNum leftNodePage, IX_NodeType targetType);
    bool sameRID(const RID &ridA, const RID &ridB) const;                           // 比较两个rid是否相同
    // 提供索引key, 寻找到删除索引所在区间的叶节点页号
    RC searchEntryFromNode(void *pData, PageNum nodePage, PageNum &rReafPage) const;
    // 从提供的叶节点号中寻找到pData索引key值的位置
    RC searchEntryFromLeaf(void *value, PageNum leafPage, int &keyPos) const;
    // 从叶节点的相应位置删除索引项
    RC deleteFromLeaf(PageNum pageNum, int keyPos, const RID &rid);
    // 寻找合适的区间
    RC getIntervalFromNode(void *value, IX_NodeHeader *nodeHeader, PageNum &nextPage) const;
    // 从父节点中移除一个子节点
    RC removeChildNode(PageNum parentNode, PageNum childNode);
private:
    PF_FileHandle pfFH;
    int isOpen;
    int bHeaderModified;
    IX_FileHeader indexHeader{};    // 索引文件的header页中保存的数据, 类似于rmFileHeader
private:
    template <typename T>
    bool matchInterval(T lVal, T rVal, T givenValue) const;
};

class IX_Manager {
public:
    IX_Manager(PF_Manager &_pfManager);
    ~IX_Manager();
    RC createIndex(const char *fileName,
                   int indexNo,
                   AttrType attrType,
                   int attrLength);
    RC openIndex(const char *fileName,
                 int indexNo,
                 IX_IndexHandle &indexHandle);
    RC closeIndex(IX_IndexHandle &indexHandle);
    RC destroyIndex(const char *fileName, int indexNo);
private:
    static string generateIndexFileName(const char *fileName, int indexNo);
    static int calculateDegree(int attrLength);
    PF_Manager *pfManager;
};

// 索引扫描器: 输入检索条件, 通过调用getNextScan获取所有符合条件的索引项
class IX_IndexScan {
public:
    IX_IndexScan();
    ~IX_IndexScan();
    RC openScan(const IX_IndexHandle &indexHandle,
                CompOp compOp,
                void *value);
    RC closeScan();
    RC getNextEntry(RID &rid);
private:
    // 获取从nodePage页开始第一个符合条件的索引项的位置
    RC getFirstEntry(PageNum nodePage, PageNum &rFirstEntryPage, int &rKeyPos);
    // 从叶节点中获取第一个满足条件的项(int)
    RC getFirstIntFromLeaf(PageNum nodePage, PageNum &rFirstEntryPage, int &rKeyPos);
    // 找到符合条件节点所在的区间
    RC getIntervalFromNode(IX_NodeHeader *nodeHeader, PageNum &nextPage);
    // 从当前位置开始(currentPage, keyPos), 查找到第一个符合条件的keyPos, 并返回rid
    RC getNextIntEntry(RID &rid);
    // 切换到下一个位置
    RC justifyNextPos();
private:
    int isOpen;                 // 记录是否被开启
    CompOp compOp;              // 比较的符号
    IX_IndexHandle indexHandle; // 用于处理索引项的handle
    PageNum currentPage;        // 记录当前的页号
    int keyPos;                 // 记录当前扫描到的索引key值在当前页的下标
    int overflowPos;            // 记录当前扫描的索引key在溢出块中的位置
    int inOverflow;             // 记录扫描的块是否是溢出块
    void *value;                // 比较的数值
private:
    template <typename T>
    bool matchIndex(T keyValue, T givenValue) const;

    template <typename T>
    bool matchInterval(T lVal, T rVal, T givenValue) const;
};


#define IX_FILE_CLOSED      (START_IX_WARN + 0)     // 文件已经关闭
#define IX_RID_EXISTED      (START_IX_WARN + 1)     // 已经存在的索引rid
#define IX_RID_FULL         (START_IX_WARN + 2)     // 溢出块插满
#define IX_SCAN_CLOSED      (START_IX_WARN + 3)     // indexScan关闭
#define IX_EOF              (START_IX_WARN + 4)     // 扫描到索引文件的end
#define IX_UNKONWN_ATTRTYPE (START_IX_WARN + 5)     // 未知的属性类型
#define IX_NOT_FOUND        (START_IX_WARN + 6)     // 索引项未找到
#define IX_LASTWARN         IX_NOT_FOUND

#define IX_UNIX             (START_IX_ERR - 0)      // 未知错误
#define IX_LASTERROR        IX_UNIX

#endif //MYDATABASE_IX_H
