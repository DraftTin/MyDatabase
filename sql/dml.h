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
    RC insertForTest(const char *relName, int nValues, const Value *values, RID &rRid);
private:
    RM_Manager *rmManager;                      // 用于控制记录的插入删除等
    DDL_Manager *ddlManager;                      // 用于获取表的信息和属性的信息
};

#define DML_ATTR_COUNT_INCORRECT     START_DML_WARN + 1;  // 插入的属性数量不一致
#define DML_ATTR_TYPE_INCORRECT      START_DML_WARN + 2;  // 插入的属性类型不一致

#endif //MYDATABASE_DML_H
