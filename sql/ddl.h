//
// Created by Administrator on 2020/11/12.
//

#ifndef MYDATABASE_DDL_H
#define MYDATABASE_DDL_H

#include <vector>
#include "../storage/rm.h"
#include "../storage/ix.h"
#include "../parser.h"

/* 数据字典中所有字符串的长度都为MAXNAME + 1 */

// RelcatRecord - 存储在relcat中的记录
// relName - 表名
// tupleLength - 元组的长度
// attrCount - 属性的数量
// indexCount - 索引属性的数量
struct RelcatRecord {
    char relName[MAXNAME + 1];
    int tupleLength;
    int attrCount;
    int indexCount;
    friend ostream &operator<<(std::ostream &s, const RelcatRecord &relcatRecord) {
        cout << relcatRecord.relName << " " << relcatRecord.tupleLength << " " << relcatRecord.attrCount << " " << relcatRecord.indexCount << "\n";
        return s;
    }
};

// AttrcatRecord - 存储在attrcat中的记录
// relName - 表名
// attrName - 属性名
// offset - 属性的偏移量
// attrType - 属性的类型
// attrLength - 属性的长度
// indexNo - 索引号 或 -1表示没有索引
struct AttrcatRecord {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
    friend ostream &operator<<(std::ostream &s, const AttrcatRecord &attrcatRecord) {
        cout << attrcatRecord.relName << " " << attrcatRecord.attrName << " " << attrcatRecord.offset << " ";
        switch (attrcatRecord.attrType) {
            case INT:
                cout << "int ";
                break;
            case FLOAT:
                cout << "float ";
                break;
            case STRING:
                cout << "string ";
                break;
            case VARCHAR:
                cout << "varchar ";
                break;
            default:
                break;
        }
        cout << attrcatRecord.attrLength << " " << attrcatRecord.indexNo << "\n";
        return s;
    }
};

#define RELCAT_ATTR_COUNT    4          // 表示表目录的记录的属性数量
#define ATTRCAT_ATTR_COUNT   6          // 表示属性目录的记录的属性数量

#define PRINT_ALL_DATA      -1          // 输出所有信息

class DDL_Manager {
public:
    DDL_Manager(RM_Manager &_rmManager, IX_Manager &_ixManager);
    ~DDL_Manager();
    RC createDb(const char *dbName);                // 创建数据库
    RC openDb(const char *dbName);                  // 打开一个数据库
    RC closeDb();                                   // 关闭数据库
    RC createTable(const char *relName,             // 表名
                    const int attrCount,            // 属性的数量
                   AttrInfo   *attributes);         // 属性的数据(属性名, 属性长度, 属性类型)
    RC dropTable(const char *relName);              // 删除一个表，如果属性列上存在索引则删除索引
    RC dropIndex(const char *relName,               // 删除表的一个索引
                 const char *attrName);
    RC createIndex(const char *relName,             // 创建表属性的索引
                    const char *attrName);
    RC getRelInfo(const char *relName, RelcatRecord &relinfo) const;    // 返回属性信息和表信息
    RC getAttrInfo(const char *relName, AttrcatRecord *attrinfo) const; // 返回表中所有属性的信息
    RC getAttrInfo(const char *relName, const char *attrName, AttrcatRecord &attrInfo); // 返回表中属性attrName属性的信息
    ////
    RC getAttributes(const string &relName, vector<string> &rAttributes);           // 获取表中所有的属性名
    RC getRelBlockNum(const string &relName, int &rNum);                            // 获取表申请的页数
    RC getRelTupleNum(const string &relName, int &rNum);                            // 获取表总共的元组数量
    RC getAttrDiffNum(const string &relName, const string &attrName, int &rNum);    // 获取表属性attrName的不同值的数目
    ////
    RC printAllData(char *relName, int lines = PRINT_ALL_DATA) const;
    RC printDataDic() const;

    RC indexExists(const char *relName, const char *attrName);
    RC relExists(const char *relationName);
private:
    RC getRelcatRec(const char *relName, RM_Record &relcatRecord);
    RC getAttrcatRec(const char *relName, const char *attrName, RM_Record &attrcatRecord);
private:
    int bDbOpen;                    // 标记数据库是否被打开
    RM_Manager *rmManager;          // 记录管理
    IX_Manager *ixManager;          // 索引管理
    RM_FileHandle relFileHandle;    // relcat文件管理
    RM_FileHandle attrFileHandle;   // attrcat文件管理
};


#define DDL_INDEX_EXISTS                        (START_DDL_WARN + 7)    // 索引已经存在
#define DDL_INDEX_NOT_EXISTS                    (START_DDL_WARN + 6)    // 索引不存在
#define DDL_NULL_ATTRIBUTE                      (START_DDL_WARN + 5)    // 传入attrName空指针
#define DDL_NULL_RELATION                       (START_DDL_WARN + 4)    // 传入relName空指针
#define DDL_DATABASE_NOT_OPEN                   (START_DDL_WARN + 3)    // 数据库没打开
#define DDL_DATABASE_OPEN                       (START_DDL_WARN + 2)    // 数据库已经打开了
#define DDL_REL_NOT_EXIST                       (START_DDL_WARN + 1)    // 进行操作的表不存在
#define DDL_LASTWARN                            DDL_REL_NOT_EXIST


#define DDL_INVALID_DBNAME                      (START_DDL_ERR - 0)
#define DDL_UNIX                                (START_DDL_ERR - 1)
#define DDL_LASTERROR                           DDL_UNIX

#endif //MYDATABASE_DDL_H
