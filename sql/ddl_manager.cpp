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
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
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
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
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
RC DDL_Manager::getRelInfo(const char *relName, RelcatRecord &relinfo) const {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    int rc;
    RM_FileScan relScan;
    // 开启对表信息文件的扫描
    if((rc = relScan.openScan(relFileHandle, STRING, MAXNAME + 1, offsetof(RelcatRecord, relName), EQ_OP, (void*)relName))) {
        return rc;
    }
    RM_Record relRecord;
    if((rc = relScan.getNextRec(relRecord))) {
        return rc;
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
RC DDL_Manager::getAttrInfo(const char *relName, int attrCount, AttrcatRecord *attrinfo) const {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    int rc;
    RM_FileScan attrScan;
    if((rc = attrScan.openScan(attrFileHandle, STRING, MAXNAME + 1, offsetof(AttrcatRecord, relName), EQ_OP, (void*)relName))) {
        return rc;
    }
    int i = 0;
    RM_Record attrRec;
    // 修改bug: while循环 == 0
    while((rc = attrScan.getNextRec(attrRec)) == 0) {
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
        // 修改bug: i在循环中递增
        ++i;
    }
    if(rc != RM_EOF) {
        return rc;
    }
    if((rc = attrScan.closeScan())) {
        return rc;
    }
    return 0;
}

// printAllData: 输出relName表的所有内容
// - 打开数据字典查看表的属性信息
// - 开启扫描
// - 获取记录, 根据属性信息输出数据, 循环
// - 关闭表
RC DDL_Manager::printAllData(char *relName, int lines) const {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    int rc;
    // 获取表信息
    RelcatRecord relcatRecord;
    if((rc = getRelInfo(relName, relcatRecord))) {
        return rc;
    }
    // 获取属性信息
    AttrcatRecord *attrInfos = new AttrcatRecord[relcatRecord.attrCount];
    if((rc = getAttrInfo(relName, relcatRecord.attrCount, attrInfos))) {
        delete [] attrInfos;
        return rc;
    }
    // 开启扫描
    RM_FileHandle rmFileHandle;
    RM_FileScan rmScan;
    if((rc = rmManager->openFile(relName, rmFileHandle))) {
        delete [] attrInfos;
        return rc;
    }
    int tmp;
    if((rc = rmScan.openScan(rmFileHandle, attrInfos[0].attrType, attrInfos[0].attrLength, attrInfos[0].offset, NO_OP, (void*)&tmp))) {
        delete [] attrInfos;
        return rc;
    }
    // 输出表的属性信息
    cout << "Table Data Dictionary: ";
    for(int i = 0; i < relcatRecord.attrCount; ++i) {
        cout << attrInfos[i].attrName << ", ";
    }
    cout << "\n";
    // 循环
    int k = 0;
    RM_Record rmRecord;
    while((rc = rmScan.getNextRec(rmRecord)) == 0 && (lines == PRINT_ALL_DATA || k < lines)) {
        // 按照属性信息输出属性值
        char *pData;
        if((rmRecord.getData(pData))) {
            delete [] attrInfos;
            return rc;
        }
        cout << k++ << ": \n";
        for(int i = 0; i < relcatRecord.attrCount; ++i) {
            Value value;
            value.value = pData + attrInfos[i].offset;
            value.type = attrInfos[i].attrType;
            cout << value;
        }
    }
    delete [] attrInfos;
    if(rc != RM_EOF) {
        return rc;
    }
    // 关闭表
    if((rc = rmManager->closeFile(rmFileHandle))) {
        return rc;
    }
    return 0;
}

// printDataDic: 输出数据字典
// - 读取数据字典文件数据数据, 按照格式输出, 循环
// - 关闭数据字典文件
RC DDL_Manager::printDataDic() const {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    int rc;
    int tmp;
    RM_FileScan rmFileScan;
    if((rc = rmFileScan.openScan(relFileHandle, INT, 4, 0, NO_OP, (void*)&tmp))) {
        return rc;
    }
    RM_Record rec;
    cout << "Relation Table Data: \n";
    while((rc = rmFileScan.getNextRec(rec)) == 0) {
        char *pData;
        if((rc = rec.getData(pData))) {
            return rc;
        }
        RelcatRecord relcatRecord;
        memcpy((char*)&relcatRecord, pData, sizeof(RelcatRecord));
        cout << relcatRecord;
    }
    if(rc != RM_EOF) {
        return rc;
    }
    if((rc = rmFileScan.openScan(attrFileHandle, INT, 4, 0, NO_OP, (void*)&tmp))) {
        return rc;
    }
    cout << "Attribution Table Data: \n";
    while((rc = rmFileScan.getNextRec(rec)) == 0) {
        char *pData;
        if((rc = rec.getData(pData))) {
            return rc;
        }
        AttrcatRecord attrcatRecord;
        memcpy((char*)&attrcatRecord, pData, sizeof(AttrcatRecord));
        cout << attrcatRecord;
    }
    return 0;   // ok
}


