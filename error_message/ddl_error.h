//
// Created by Administrator on 2021/1/13.
//

#ifndef DDL_ERROR_H
#define DDL_ERROR_H

#include <iostream>
#include "../sql/ddl.h"

using namespace std;

static char *DDL_WarnMsg[] = {
        (char*)"index exists",
        (char*)"index does not exist",
        (char*)"attribute pointer is nullptr",
        (char*)"relation pointer is nullptr",
        (char*)"database is not opened",
        (char*)"database is already opened",
        (char*)"relation doesn't exist",
};

static char *DDL_ErrorMsg[] = {
        (char*)"database does not exist",
        (char*)"unknown error",
};

// DDL_PrintError: 输出DDL组件的错误信息
void DDL_PrintError(RC rc) {
    // 输出warrnning信息
    if(rc >= START_DDL_WARN && rc <= DDL_LASTWARN) {
        cerr << "DDL WARNING: " << DDL_WarnMsg[rc - START_DDL_WARN] << "\n";
    }
        // 输出error信息
    else if(rc >= START_DDL_ERR && rc <= DDL_LASTERROR) {
        cerr << "DDL ERROR: " << DDL_ErrorMsg[-rc + START_DDL_ERR] << "\n";
    }
    else {
        cerr << "DDL ERROR: " << rc << " is out of bound\n";
    }
}

#endif