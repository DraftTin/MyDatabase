//
// Created by Administrator on 2021/1/13.
//

#ifndef IX_ERROR_H
#define IX_ERROR_H

#include <iostream>
#include "../storage/ix.h"

using namespace std;

static char *IX_WarnMsg[] = {
        (char*)"file is already closed",
        (char*)"rid exists",
        (char*)"overflow block is full",
        (char*)"scan is already closed",
        (char*)"unknown attribute type",
        (char*)"entry not found"
};

static char *IX_ErrorMsg[] = {
        (char*)"unknown error",
};

// IX_PrintError: 输出IX组件的错误信息
void IX_PrintError(RC rc) {
    // 输出warrnning信息
    if(rc >= START_IX_WARN && rc <= IX_LASTWARN) {
        cerr << "IX WARNING: " << IX_WarnMsg[rc - START_IX_WARN] << "\n";
    }
        // 输出error信息
    else if(rc >= START_IX_ERR && rc <= IX_LASTERROR) {
        cerr << "IX ERROR: " << IX_ErrorMsg[-rc + START_IX_ERR] << "\n";
    }
    else {
        cerr << "IX ERROR: " << rc << " is out of bound\n";
    }
}

#endif