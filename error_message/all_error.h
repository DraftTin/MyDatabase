//
// Created by Administrator on 2021/1/14.
//

#ifndef MYDATABASE_ALL_ERROR_H
#define MYDATABASE_ALL_ERROR_H

#include <iostream>
#include "ddl_error.h"
#include "dml_error.h"
#include "ix_error.h"
#include "pf_error.h"
#include "rm_error.h"

using namespace std;

// 根据错误码输出错误信息
void PrintError(RC rc) {
    if(abs(rc) < END_PF_ERR) {
        PF_PrintError(rc);
    }
    else if(abs(rc) < END_RM_WARN) {
        RM_PrintError(rc);
    }
    else if(abs(rc) < END_IX_WARN) {
        IX_PrintError(rc);
    }
    else if(abs(rc) < END_DDL_WARN) {
        DDL_PrintError(rc);
    }
    else if(abs(rc) < END_DML_WARN) {
        DML_PrintError(rc);
    }
    else {
        cerr << "rc number is out of bound\n";
    }
}

#endif //MYDATABASE_ALL_ERROR_H
