//
// Created by Administrator on 2020/11/6.
//

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <iostream>
#include <fcntl.h>
#include "storage/bufmgr.h"
#include "storage/rm.h"
#include "sql/ddl.h"

using namespace std;


// 客户端创建fileHandle和pageHandle，向缓冲区申请页空间
void test2() {
    PF_Manager *pfManager = new PF_Manager();
    PF_FileHandle pfFileHandle;
    char *filename = new char[10];
    cout << "enter the filename: ";
    cin >> filename;
    cout << "filename: " << filename << endl;
    cout << "create file: " << pfManager->createFile(filename) << endl;
    cout << "openfile: " << pfManager->openFile(filename, pfFileHandle) << endl;
    PageHandle pageHandle;
    cout << "allocate page from buffer pool: " << pfFileHandle.allocatePage(pageHandle) << endl;
    char *pData = nullptr;
    int pageNum = -999;
    pageHandle.getData(pData);
    pageHandle.getPageNum(pageNum);
    cout << "nextPage: " << pageNum << endl;
    if(pData != nullptr)
        cout << "pData: " << ((PF_PageHdr*)pData)->nextFree << endl;
}

int calcNumberRecord(int recordSize) {
    int n = 1;
    int headSize = 4;  // 页头的size
    int bitmapSize;
    while(true) {
        // 位示图1byte能够表示8条记录
        bitmapSize = n / 8;
        if(n % 8 != 0) {
            ++bitmapSize;
        }
        int sumSize = headSize + bitmapSize + n * recordSize;
        // 循环直到：页头Size + 位示图Size + 记录Size > 页Size，之后返回(n - 1)
        if(sumSize > PF_PAGE_SIZE) {
            break;
        }
        ++n;
    }
    return (n - 1);
}

void test1() {
        PF_Manager pfManager;
        RM_Manager rmManager(pfManager);
        DDL_Manager smManager(rmManager);
        char dbName[15] = "db3";
        smManager.openDb(dbName);
        while(true) {
            char tableName[MAXNAME] = "student";
            cin >> tableName;
            if(tableName[0] == '#') break;
            int attrCount = 2;
            AttrInfo *attrInfos = new AttrInfo[attrCount];
            attrInfos[0].attrType = STRING;
            attrInfos[0].attrLength = MAXNAME;
            attrInfos[0].attrName = new char[MAXNAME];
            sprintf(attrInfos[0].attrName, "%s", "name");
            attrInfos[1].attrType = FLOAT;
            attrInfos[1].attrLength = 4;
            attrInfos[1].attrName = new char[MAXNAME];
            sprintf(attrInfos[1].attrName, "%s", "height");

            smManager.createTable(tableName, attrCount, attrInfos);
        }
};

int main() {
    cout << calcNumberRecord(4087) << endl;
//    int ATTRBLOCK_SLOTSIZE[6] = {70, 127, 255, 510, 1021, 2041};
//    for(int i = 0; i < 6; ++i) {
//        cout << calcNumberRecord(ATTRBLOCK_SLOTSIZE[i]) << endl;
//    }
//    RM_VarLenAttr varLenAttr(1, 1, 1);
//    char *cc = new char[sizeof(RM_VarLenAttr)];
//    memset(cc, 0, sizeof(RM_VarLenAttr));
//    memcpy(cc, (char*)&varLenAttr, sizeof(RM_VarLenAttr));
//    int p;
//    int s;
//    int i;
//    ((RM_VarLenAttr*)cc)->getPageNum(p);
//    ((RM_VarLenAttr*)cc)->getSlotNum(s);
//    ((RM_VarLenAttr*)cc)->getBlockInfoIndex(i);
//    cout << p << s << i << endl;
//    uint32 a = ((uint32) 1 << 24);
//    uint32 b = a & 1;
//    cout << b << "\n";
    return 0;
}
// attrcat dbinfo relcat
