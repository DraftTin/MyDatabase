//
// Created by Administrator on 2020/12/2.
//

#ifndef MYDATABASE_DML_H
#define MYDATABASE_DML_H

#include "../global.h"
#include "../storage/rm.h"
#include "ddl.h"

class DML_Manager {
public:
    DML_Manager(RM_Manager &_rmManager, DDL_Manager &_ddlManager);
    RC insert (const char  *relName,           // relation to insert into
               int         nValues,            // # values to insert
               const Value values[]);          // values to insert
    RC Select (int           nSelAttrs,        // # attrs in Select clause
               const RelAttr selAttrs[],       // attrs in Select clause
               int           nRelations,       // # relations in From clause
               const char * const relations[], // relations in From clause
               int           nConditions,      // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
    RC Delete (const char *relName,            // relation to delete from
               int        nConditions,         // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
    RC Update (const char *relName,            // relation to update
               const RelAttr &updAttr,         // attribute to update
               const int bIsValue,             // 0/1 if RHS of = is attribute/value
               const RelAttr &rhsRelAttr,      // attr on RHS of =
               const Value &rhsValue,          // value on RHS of =
               int   nConditions,              // # conditions in Where clause
               const Condition conditions[]);  // conditions in Where clause
    RC insertForTest(const char *relName, int nValues, const Value *values, RID &rRid);
private:
    RM_Manager *rmManager;                      // 用于控制记录的插入删除等
    DDL_Manager *ddlManager;                    // 用于获取表的信息和属性的信息
};

#define DML_ATTR_COUNT_INCORRECT     START_DML_WARN + 1;  // 插入的属性数量不一致
#define DML_ATTR_TYPE_INCORRECT      START_DML_WARN + 2;  // 插入的属性类型不一致
#define DML_DATABASE_CLOSED          START_DML_WARN + 3;  // 数据库关闭

#endif //MYDATABASE_DML_H
