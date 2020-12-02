//
// Created by Administrator on 2020/11/12.
//

#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <cstring>
#include "ddl.h"

DDL_Manager::DDL_Manager(RM_Manager &_rmManager) {
    rmManager = &_rmManager;
    bDbOpen = FALSE;
}

DDL_Manager::~DDL_Manager() {

}

// openDb: 打开一个数据库
// - 进入dbname工作目录
// - 打开relcat和attrcat文件
// - 将数据库的状态置为打开
RC DDL_Manager::openDb(const char *dbName) {
    // 数据库已经打开
    if(bDbOpen) {
        return DDL_DATABASE_OPEN;
    }
    if(chdir(dbName) < 0) {
        return DDL_INVALID_DBNAME;
    }
    int rc;
    char relFileName[10] = "relcat";
    char attrFileName[10] = "attrcat";
    // 打开relcat和attrcat文件
    if((rc = rmManager->openFile(relFileName, relFileHandle)) ||
       (rc = rmManager->openFile(attrFileName, attrFileHandle))) {
        return rc;
    }
    bDbOpen = TRUE;
    return 0;   // ok
}

// createTable: 创建relName表名的文件, attrCount属性数量, attributes属性具体信息(属性名称, 属性长度, 属性类型)
// - 计算recordSize
// - 创建relName文件(relName, recordSize)
// - 将表信息和属性信息写入relcat和attrcat
// - 将relcat和attrcat缓冲区的内容写回到文件中(运行过程中需要读取)
RC DDL_Manager::createTable(const char *relName, int attrCount, AttrInfo *attributes) {
    // 计算recordSize
    int recordSize = 0;
    int offsets[attrCount];
    // 计算记录的长度, 记录每个属性的偏移
    for(int i = 0; i < attrCount; ++i) {
        offsets[i] = recordSize;
        recordSize += attributes[i].attrLength;
    }
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    int rc;
    // 创建表文件, 指定记录的长度
    if((rc = rmManager.createFile(relName, recordSize))) {
        return rc;
    }
    // 将表信息和属性信息写入relcat和attrcat
    // 写入relcat表中
    RelcatRecord relcatRecord{};
    strcpy(relcatRecord.relName, relName);
    relcatRecord.tupleLength = recordSize;
    relcatRecord.attrCount = attrCount;
    relcatRecord.indexCount = 0;
    RID rid;
    relFileHandle.insertRec((char*)&relcatRecord, rid);
    // 写入attrcat表中
    AttrcatRecord attrcatRecord{};
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

// closeDb: 关闭数据库
// - 关闭数据字典文件
// - 将bDbOpen置false
RC DDL_Manager::closeDb() {
    int rc;
    // 关闭数据字典文件
    if((rc = rmManager->closeFile(relFileHandle)) ||
            (rc = rmManager->closeFile(attrFileHandle))) {
        return rc;
    }
    // 将bDbOpen置false
    bDbOpen = false;
    return 0;
}

// getRelInfo: 获取表的数据字典信息
RC DDL_Manager::getRelInfo(const char *relName, RelcatRecord &relinfo) {
    int rc;
    RM_FileScan relScan;
    // 开启对表信息文件的扫描
    if((rc = relScan.openScan(relFileHandle, STRING, MAXNAME + 1, offsetof(RelcatRecord, relName), EQ_OP, relName))) {
        return rc;
    }
    RM_Record relRecord;
    if((rc = relScan.getNextRec(relRecord)) != 0 && rc != RM_EOF) {
        return rc;
    }
    if(rc == RM_EOF) {
        return DDL_REL_NOT_EXISTS;
    }

    char *relData;
    if((rc = relRecord.getData(relData))) {
        return rc;
    }
    // 拷贝获取的表信息
    RelcatRecord *smRelcatRecord = (RelcatRecord*)relData;
    strcpy(relinfo.relName, smRelcatRecord->relName);
    relinfo.attrCount = smRelcatRecord->attrCount;
    relinfo.indexCount = smRelcatRecord->indexCount;
    relinfo.tupleLength = smRelcatRecord->tupleLength;
    return 0;   // ok
}

// getAttrInfo: 获取{relName}表的所有属性信息
// - 扫描属性文件
// - 将每个属于该表的属性录入到attrinfo中
RC DDL_Manager::getAttrInfo(const char *relName, int attrCount, AttrcatRecord *attrinfo) {
    int rc;
    RM_FileScan attrScan;
    if((rc = attrScan.openScan(attrFileHandle, STRING, MAXNAME + 1, offsetof(AttrcatRecord, relName), EQ_OP, relName))) {
        return rc;
    }
    int i = 0;
    RM_Record attrRec;
    while((rc = attrScan.getNextRec(attrRec))) {
        char *pData;
        if((rc = attrRec.getData(pData))) {
            return rc;
        }
        AttrcatRecord *attrcatRecord = (AttrcatRecord*)pData;
        strcpy(attrinfo[i].relName, attrcatRecord->relName);
        strcpy(attrinfo[i].attrName, attrcatRecord->attrName);
        attrinfo[i].attrType = attrcatRecord->attrType;
        attrinfo[i].attrLength = attrcatRecord->attrLength;
        attrinfo[i].offset = attrcatRecord->offset;
        attrinfo[i].indexNo = attrcatRecord->indexNo;
    }
    if((rc = attrScan.closeScan())) {
        return rc;
    }
    return 0;
}


