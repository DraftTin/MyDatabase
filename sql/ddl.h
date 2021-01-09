//
// Created by Administrator on 2020/11/12.
//

#ifndef MYDATABASE_DDL_H
#define MYDATABASE_DDL_H

#include <vector>
#include "../storage/rm.h"
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
    DDL_Manager(RM_Manager &_rmManager);
    ~DDL_Manager();
    RC openDb(const char *dbName);                  // 打开一个数据库
    RC closeDb();                                   // 关闭数据库
    RC createTable(const char *relName,             // 表名
                    const int attrCount,            // 属性的数量
                   AttrInfo   *attributes);         // 属性的数据(属性名, 属性长度, 属性类型)
    RC getRelInfo(const char *relName, RelcatRecord& relinfo) const;   // 返回属性信息和表信息
    RC getAttrInfo(const char *relName, AttrcatRecord *attrinfo) const;
    ////
    RC getAttributes(const string &relName, vector<string> &rAttributes);   // 获取表中所有的属性名
    ////
    RC printAllData(char *relName, int lines = PRINT_ALL_DATA) const;
    RC printDataDic() const;
private:
    int bDbOpen;                    // 标记数据库是否被打开
    RM_Manager *rmManager;          // 记录文件管理
    RM_FileHandle relFileHandle;
    RM_FileHandle attrFileHandle;
};


#define DDL_DATABASE_NOT_OPEN                   (START_DDL_WARN + 3)    // 数据库没打开
#define DDL_DATABASE_OPEN                       (START_DDL_WARN + 2)    // 数据库已经打开了
#define DDL_REL_NOT_EXISTS                      (START_DDL_WARN + 1)    // 进行操作的表不存在


#define DDL_INVALID_DBNAME                      (START_DDL_ERR - 0)

#endif //MYDATABASE_DDL_H
