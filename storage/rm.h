//
// Created by Administrator on 2020/11/9.
//

#ifndef MYDATABASE_RM_H
#define MYDATABASE_RM_H

#include <string>
#include "../global.h"
#include "rid.h"
#include "pf.h"
#include "../parser.h"

typedef int PageNum;

const int NUMBER_ATTRBLOCK_SPECIES = 7;
const int ATTRBLOCK_BIMAPSIZE[NUMBER_ATTRBLOCK_SPECIES] = {8, 4, 2, 1, 1, 1, 1};
const int ATTRBLOCK_SLOTSIZE[NUMBER_ATTRBLOCK_SPECIES] = {70, 127, 255, 510, 1021, 2041, 4087};
const int ATTRBLOCK_NUM_RECORDS[NUMBER_ATTRBLOCK_SPECIES] = {58, 32, 16, 8, 4, 2, 1};

// RM_RecordFileHeaderPage
struct RM_RecordFileHeaderPage {
    int recordSize;                 // 记录大小
    int numberRecords;              // 一页中记录的数量
    PageNum firstFreePage;          // 文件中第一个空闲页(有剩余空间的页)
};

// RM_AttrFileHeaderPage
struct RM_AttrFileHeaderPage {
    PageNum firstFreeAttrPages[NUMBER_ATTRBLOCK_SPECIES];
};

// 用于在记录中代替变长字段的属性, 其指向表中另一块的一个slotNum
class RM_VarLenAttr {
public:
    RM_VarLenAttr();
    RM_VarLenAttr(PageNum _pageNum, SlotNum _slotNum, int _index);
    ~RM_VarLenAttr();
    RC getPageNum(PageNum &pageNum) const;
    RC getSlotNum(SlotNum &slotNum) const;
    RC getBlockInfoIndex(int &index) const;
private:
    int pageNum;        // 记录页号
    int slotNum;        // 记录slotNum
    int blockInfoIndex; // 记录该块信息对应在VARATTRBLOCK_BIMAPSIZE和VARATTRBLOCK_SLOTSIZE中的下标
};

// RM_Record: RM Record接口
class RM_Record {
    friend class RM_FileHandle;
    friend class RM_FileScan;
public:
    RM_Record();
    ~RM_Record();
    // 获取记录的数据
    RC getData(char *&pData) const;
    // 获取记录的rid
    RC getRid(RID &rid) const;
public:
    char *pData;    // 记录的数据
    RID rid;        // 记录的rid(nextPage, slotID)
    int isValid;    // flag
    int recordSize; // 记录的size
};


// 属性文件的句柄
class RM_AttrFileHandle {
    friend class RM_Manager;
    friend class RM_FileScan;
    friend class RM_FileHandle;
public:
    RM_AttrFileHandle();
    RC insertVarValue(const char *pVal, int length, RM_VarLenAttr &varLenAttr);
    RC updateVarValue(char *pVal, int length, RM_VarLenAttr &varLenAttr);
    RC deleteVarValue(const RM_VarLenAttr &varLenAttr);
    RC getVarValue(RM_VarLenAttr &varLenAttr, char *&pVal);
private:
    int bFileOpen;
    int bHdrPageChanged;
    PF_FileHandle fileHandle;
    RM_AttrFileHeaderPage hdrPage;
};

// RM文件处理类
class RM_FileHandle {
    friend class RM_Manager;
    friend class RM_FileScan;
public:
    RM_FileHandle();
    ~RM_FileHandle();
public:
    // 获取rid的记录并将获取的记录返回给record
    RC getRec(const RID &rid, RM_Record &record);
    // 插入pData并将插入的记录返回给rid
    RC insertRec(const char *pData, RID &rid);
    // 更新记录rec记录
    RC updateRec(const RM_Record &rec);
    // 删除rid记录
    RC deleteRec(const RID &rid);

    // offset标识该变长属性值在其所在记录中的偏移量
    // rid为操作的变长属性所属的元组id
    // 插入变长属性值pVal, 并将插入的属性位置和对应的block类型下标返回, varLenAttr为返回的该属性的在文件中的位置
    RC insertVarValue(const RID &rid, int offset, const char *pVal, int length, RM_VarLenAttr &varLenAttr);
    // 删除变长属性值pVal, varLenAttr标识该属性在文件中的位置
    RC deleteVarValue(const RID &rid, int offset, const RM_VarLenAttr &varLenAttr);
    // 更新变长属性值pVal, varLenAttr标识该属性在文件中的位置
    RC updateVarValue(const RID &rid, int offset, char *pVal, int length, RM_VarLenAttr &varLenAttr);
    // 获取变长属性值, 返回pVal, varLenAttr标识该属性在文件中的位置
    RC getVarValue(RM_VarLenAttr &varLenAttr, char *&pVal);
    // 写回文件
    RC forcePages(PageNum pageNum = ALL_PAGES);
private:
    // 计算slotNum记录在页内的偏移
    int calcRecordOffset(int slotNum) const;
    // 虽然有pfFileHandle，但是由于bFileOpen是其私有变量所以在rmFileHandle中也要加一个bFileOpen以便判断文件是否打开
    int bFileOpen;                                  // 所有操作必须要在文件打开的情况下进行
    int bHdrPageChanged;                            // 用于判断fileHeaderPage是否修改过，如果修改过在关闭文件的时候应该将内容写回file header page中
    PF_FileHandle fileHandle;                       // 记录文件句柄
    RM_RecordFileHeaderPage hdrPage;                // 记录文件头信息
    RM_AttrFileHandle rmAttrFileHandle;             // 属性文件处理
};



// RM_FileScan
// 每个FileScan对应着一个查询条件的SELECT语句(也可能没有条件), 通过openScan(fileHandle, attrType
class RM_FileScan {
public:
    RM_FileScan();
    ~RM_FileScan();

    RC openScan(const RM_FileHandle &fileHandle,
                AttrType attrType,
                int attrLength,
                int attrOffset,
                CompOp compOp,
                void *value
                );                      // 初始化file scan
    RC getNextRec(RM_Record &rec);      // 获取下一个匹配的记录
    RC closeScan();                     // close the scan
private:
    PageNum pageNum;                    // 当前的pageNum
    SlotNum slotNum;                    // 当前的slotNum
    RM_FileHandle rmFileHandle;         // 文件句柄的实例
    AttrType attrType;                  // 需要比较的属性的类型
    int attrOffset;                     // 需要比较的属性在记录中的偏移
    int attrLength;                     // 需要比较的属性的长度
    CompOp compOp;                      // 比较符
    void *value;                        // 比较属性的值
    int bScanOpen;                      // 表示该scan是否能够进行扫描
private:
    // 根据attrOffset找到recordData中对应的Integer类型的值, 并返回
    int getIntegerValue(char *recordData) const;
    // 同Integer
    float getFloatValue(char *recordData) const;
    // 同string
    std::string getStringValue(char *recordData) const;
    // 判断bitNumber位是否被使用
    bool isBitFilled(char *bitmap, int bitNumber);
    // 根据记录的值和void *value的值和compOp比较二者来判断是否是满足条件的记录
    template<typename T>
    bool matchRecord(T recordValue, T givenValue);
};


// 记录管理器: 负责文件的创建, 文件的open(返回RM_FileHandle), 文件的close(返回RM_FileHandle)
class RM_Manager {
public:
    // 使用PF_Manager初始化RM_Manager实例
    explicit RM_Manager(PF_Manager &pfManager);
    ~RM_Manager();
    // 根据fileName和recordSize来创建文件，需要调用PF_Manager的createFile, 会声明一个RM_FileHdrPage并将其写到创建的文件的单独的一个页中
    RC createFile(const char *filename, int recordSize);
    // 删除一个文件，调用PF_Manager接口
    RC destroyFile(const char *filename);
    // 打开一个文件，调用PF_Manager接口
    RC openFile(const char *fileName, RM_FileHandle &fileHandle);
    // 关闭一个文件，调用PF_Manager的接口
    RC closeFile  (RM_FileHandle &fileHandle);

private:
    // 输入recSize, 返回每页最多能够容纳的记录数，也就是bitmap的位数
    int calcNumberRecord(int recordSize);
private:
    PF_Manager *pfManager;      // PF_Manager的指针
};


// warnings
#define RM_LARGE_RECORD             (START_RM_WARN + 0)     // size过大的记录
#define RM_SMALL_RECORD             (START_RM_WARN + 1)     // size过小的记录
#define RM_FILE_OPEN                (START_RM_WARN + 2)     // 文件已经打开
#define RM_CLOSEDFILE               (START_RM_WARN + 3)     // 文件已经关闭
#define RM_INVALIDRECORD            (START_RM_WARN + 4)     // 不合法的记录
#define RM_INVALID_SLOT_NUMBER      (START_RM_WARN + 5)     // 不合法的slot号
#define RM_INVALID_PAGE_NUMBER      (START_RM_WARN + 6)     // 不合法的页号
#define RM_ATTRIBUTE_NOT_CONSISTENT (START_RM_WARN + 7)     // 属性不一致
#define RM_SCAN_CLOSED              (START_RM_WARN + 8)     // Scan is not open
#define RM_INVALID_FILENAME         (START_RM_WARN + 9)     // 不合法的页号
#define RM_INVALID_ATTRIBUTE        (START_RM_WARN + 10)    // 不合法的属性
#define RM_INVALID_OFFSET           (START_RM_WARN + 11)    // Invalid offset
#define RM_INVALID_OPERATOR         (START_RM_WARN + 12)    // Invalid operator
#define RM_NULL_RECORD              (START_RM_WARN + 13)    // 空的record
#define RM_EOF                      (START_RM_WARN + 14)    // End of file
#define RM_RID_NOT_VIABLE           (START_RM_WARN + 15)    // rid未初始化
#define RM_LASTWARN                 RM_RID_NOT_VIABLE

// errors
#define RM_INCONSISTENT_BITMAP  (START_RM_ERR - 0)  // bitmap未知错误
#define RM_UNIX                 (START_RM_ERR - 1)  // Unix error
#define RM_LASTERROR            RM_UNIX


#endif //MYDATABASE_RM_H
