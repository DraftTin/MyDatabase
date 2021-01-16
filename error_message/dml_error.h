//
// Created by Administrator on 2021/1/13.
//

#ifndef DML_ERROR_H
#define DML_ERROR_H

#include <iostream>
#include "../sql/dml.h"

using namespace std;

static char *DML_WarnMsg[] = {
        (char*)"attribute count is incorrect",
        (char*)"attribute type is incorrect",
        (char*)"database is already closed",
};

static char *DML_ErrorMsg[] = {
        (char*)"known error",
};

// DML_PrintError: 输出DML组件的错误信息
void DML_PrintError(RC rc) {
    // 输出warrnning信息
    if(rc >= START_DML_WARN && rc <= DML_LASTWARN) {
        cerr << "DML WARNING: " << DML_WarnMsg[rc - START_DML_WARN] << "\n";
    }
        // 输出error信息
    else if(rc >= START_DML_ERR && rc <= DML_LASTERROR) {
        cerr << "DML ERROR: " << DML_ErrorMsg[-rc + START_DML_ERR] << "\n";
    }
    else {
        cerr << "DML ERROR: " << rc << " is out of bound\n";
    }
}

#endif