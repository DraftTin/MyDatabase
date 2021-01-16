//
// Created by Administrator on 2021/1/13.
//
#ifndef PF_ERROR_H
#define PF_ERROR_H

#include <iostream>
#include "../storage/pf.h"

using namespace std;

static char *PF_WarnMsg[] = {
        (char*)"page is pinned in buffer",
        (char*)"page is not in the buffer pool",
        (char*)"invalid page number",
        (char*)"file is already opened",
        (char*)"file is already closed",
        (char*)"file is already free",
        (char*)"page is already unpinned",
        (char*)"end of file",
};

static char *PF_ErrorMsg[] = {
        (char*)"no memory can assign",
        (char*)"buffer pool is full",
        (char*)"incomplete read of page from file",
        (char*)"incomplete write of page to file",
        (char*)"incomplete read of header from file",
        (char*)"incomplete write of header to file",
        (char*)"new page to be allocated is already in buffer",
        (char*)"hash entry not found",
        (char*)"page is already in hash table",
        (char*)"unknown error",
};

// PF_PrintError: 根据rc码输出PF模块的错误信息
void PF_PrintError(RC rc) {
    // 输出warrnning信息
    if(rc >= START_PF_WARN && rc <= PF_LASTWARN) {
        cerr << "PF WARNING: " << PF_WarnMsg[rc - START_PF_WARN] << "\n";
    }
    // 输出error信息
    else if(rc >= START_PF_ERR && rc <= PF_LASTERROR) {
        cerr << "PF ERROR: " << PF_ErrorMsg[-rc + START_PF_ERR] << "\n";
    }
    else {
        cerr << "PF ERROR: " << rc << " is out of bound\n";
    }
}

#endif