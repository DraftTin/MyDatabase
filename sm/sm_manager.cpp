//
// Created by Administrator on 2020/11/12.
//

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <cstring>
#include "sm.h"

SM_Manager::SM_Manager(RM_Manager &rmManager) {
    pRmManager = &rmManager;
    bDbOpen = FALSE;
}

SM_Manager::~SM_Manager() {

}

// openDb: 打开一个数据库
// - 进入dbname工作目录
// - 打开relcat和attrcat文件
// - 将数据库的状态置为打开
RC SM_Manager::openDb(const char *dbName) {
    // 数据库已经打开
    if(bDbOpen) {
        return SM_DATABASE_OPEN;
    }
    if(chdir(dbName) < 0) {
        return SM_INVALID_DBNAME;
    }
    int rc;
    char relFileName[10] = "relcat";
    char attrFileName[10] = "attrcat";
    // 打开relcat和attrcat文件
    if((rc = pRmManager->openFile(relFileName, relFileHandle)) ||
            (rc = pRmManager->openFile(attrFileName, attrFileHandle))) {
        return rc;
    }
    bDbOpen = TRUE;
    return 0;   // ok
}

// createTable: 创建relName表名的文件, attrCount属性数量, attributes属性具体信息(属性名称, 属性长度, 属性类型)
// - 计算recordSize
// - 创建relName文件(relName, recordSize)
// - 将表信息和属性信息写入relcat和attrcat
// - 将relcat和attrcat缓冲区的内容写回到文件中
RC SM_Manager::createTable(const char *relName, int attrCount, AttrInfo *attributes) {
    // 计算recordSize
    int recordSize = 0;
    int offsets[attrCount]; // 记录每个属性在记录中的偏移
    for(int i = 0; i < attrCount; ++i) {
        offsets[i] = recordSize;
        recordSize += attributes[i].attrLength;
    }
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    int rc;
    // 创建表文件
    if((rc = rmManager.createFile(relName, recordSize))) {
        return rc;
    }
    // 将表信息和属性信息写入relcat和attrcat
    // 写入relcat表中
    SM_RelcatRecord relcatRecord{};
    strcpy(relcatRecord.relName, relName);
    relcatRecord.tupleLength = recordSize;
    relcatRecord.attrCount = attrCount;
    relcatRecord.indexCount = 0;
    RID rid;
    relFileHandle.insertRec((char*)&relcatRecord, rid);
    // 写入attrcat表中
    SM_AttrcatRecord attrcatRecord{};
    strcpy(attrcatRecord.relName, relName);
    for(int i = 0; i < attrCount; ++i) {
        strcpy(attrcatRecord.attrName, attributes[i].attrName);
        attrcatRecord.attrType = attributes[i].attrType;
        attrcatRecord.attrLength = attributes[i].attrLength;
        attrcatRecord.offset = offsets[i];
        attrcatRecord.indexNo = -1; // -1表示没有索引
        if((rc = attrFileHandle.insertRec((char*)&attrcatRecord, rid))) {
            return rc;
        }
    }
    // 创建的表信息直接写回文件, 防止找不到数据字典
    if((rc = relFileHandle.forcePages()) ||
            (rc = attrFileHandle.forcePages())) {
        return rc;
    }
    return 0;
}


RC SM_Manager::closeDb() {
    return 0;
}

RC SM_Manager::dropTable(const char *relName) {
    return 0;
}



