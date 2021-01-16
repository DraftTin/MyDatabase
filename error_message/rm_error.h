//
// Created by Administrator on 2021/1/13.
//

#ifndef RM_ERROR_H
#define RM_ERROR_H

#include <iostream>
#include "../storage/rm.h"

using namespace std;

static char *RM_WarnMsg[] = {
        (char*)"record size too large",
        (char*)"record size too small",
        (char*)"file is already opened",
        (char*)"file is already closed",
        (char*)"invalid record",
        (char*)"invalid slot number",
        (char*)"invalid page number",
        (char*)"attribute is not consistent",
        (char*)"scan is not open",
        (char*)"invalid file name",
        (char*)"invalid attribute type",
        (char*)"invalid attribute offset",
        (char*)"invalid operator type",
        (char*)"record pointer is nullptr",
        (char*)"end of file",
        (char*)"rid is not viable",
};

static char *RM_ErrorMsg[] = {
        (char*)"bimap inconsistent in page file",
        (char*)"unkown error",
};

// RM_PrintError: 输出RM组件的错误信息
void RM_PrintError(RC rc) {
    // 输出warrnning信息
    if(rc >= START_RM_WARN && rc <= RM_LASTWARN) {
        cerr << "RM WARNING: " << RM_WarnMsg[rc - START_RM_WARN] << "\n";
    }
        // 输出error信息
    else if(rc >= START_RM_ERR && rc <= RM_LASTERROR) {
        cerr << "RM ERROR: " << RM_ErrorMsg[-rc + START_RM_ERR] << "\n";
    }
    else {
        cerr << "RM ERROR: " << rc << " is out of bound\n";
    }
}

#endif