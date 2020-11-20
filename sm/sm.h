//
// Created by Administrator on 2020/11/12.
//

#ifndef MYDATABASE_SM_H
#define MYDATABASE_SM_H

#include "../rm/rm.h"
#include "../parser.h"

// SM_RelcatRecord - 存储在relcat中的记录
// relName - 表名
// tupleLength - 元组的长度
// attrCount - 属性的数量
// indexCount - 索引属性的数量
struct SM_RelcatRecord {
    char relName[MAXNAME + 1];
    int tupleLength;
    int attrCount;
    int indexCount;
};

// SM_AttrcatRecord - 存储在attrcat中的记录
// relName - 表名
// attrName - 属性名
// offset - 偏移量
// attrType - 属性的类型
// attrLength - 属性的长度
// indexNo - indexNumber 或 -1表示没有索引
struct SM_AttrcatRecord {
    char relName[MAXNAME + 1];
    char attrName[MAXNAME + 1];
    int offset;
    AttrType attrType;
    int attrLength;
    int indexNo;
};

#define SM_RELCAT_ATTR_COUNT    4       // 表示表目录的记录的属性数量
#define SM_ATTRCAT_ATTR_COUNT   6       // 表示属性目录的记录的属性数量

class SM_Manager {
public:
    SM_Manager(RM_Manager &rmManager);
    ~SM_Manager();
    RC openDb(const char *dbName);                  // 打开一个数据库
    RC closeDb();                                   // 关闭数据库
    RC createTable(const char *relName,             // 表名
                   int        attrCount,            // 属性的数量
                   AttrInfo   *attributes);         // 属性的数据(属性名, 属性长度, 属性类型)
    RC dropTable(const char *relName);              // 删除一个表
private:
    int bDbOpen;
    RM_Manager *pRmManager;
    RM_FileHandle relFileHandle;
    RM_FileHandle attrFileHandle;
};


#define SM_DATABASE_OPEN                    (START_SM_WARN + 2)

#define SM_INVALID_DBNAME                   (START_SM_ERR - 0)

#endif //MYDATABASE_SM_H
