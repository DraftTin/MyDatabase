//
// Created by Administrator on 2020/11/12.
//

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <iostream>
#include "global.h"
#include "storage/pf.h"
#include "storage/rm.h"
#include "sql/ddl.h"
#include "error_message/all_error.h"

using namespace std;

// 描述: 命令行创建数据库
// - 创建数据库目录
// - 创建relcat和attrcat的RM文件
int main(int argc, char *argv[]) {
    char *dbname;
    char command[255] = "mkdir ";

    if(argc != 2) {
        cerr << "to use: .\\dbcreate <dbname>\n";
        exit(1);
    }
    // argv[0]是执行文件名
    // 创建数据库目录
    dbname = argv[1];
    if((system(strcat(command, dbname)) != 0)) {
        cerr << "create " << dbname << " failed\n";
    }
    if(chdir(dbname) < 0) {
        cerr << "chdir error: " << dbname << '\n';
    }
    // 创建relcat和attrcat文件
    RC rc;
    char relCatFileName[10] = "relcat";
    char attrCatFileName[10] = "attrcat";
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    if((rc = rmManager.createFile(relCatFileName, sizeof(RelcatRecord))) ||
       (rc = rmManager.createFile(attrCatFileName, sizeof(AttrcatRecord)))) {
        PrintError(rc);
        return rc;
    }

    return 0;
}
