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


DDL_Manager::DDL_Manager(RM_Manager &_rmManager, IX_Manager &_ixManager) {
    rmManager = &_rmManager;
    ixManager = &_ixManager;
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
RC DDL_Manager::createTable(const char *relName, const int attrCount, AttrInfo *attributes) {
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
//    PF_Manager pfManager;
//    RM_Manager rmManager(pfManager);
    int rc;
    // 创建表文件, 指定记录的长度
    if((rc = rmManager->createFile(relName, recordSize))) {
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
RC DDL_Manager::getAttrInfo(const char *relName, AttrcatRecord *attrinfo) const {
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
    if((rc = getAttrInfo(relName, attrInfos))) {
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
    // 增加判断rc != 0，否则不能正确关闭文件
    if(rc != RM_EOF && rc != 0) {
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

// getAttributes: 获取表的所有属性名
RC DDL_Manager::getAttributes(const string &relName, vector<string> &rAttributes) {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    int rc;
    RelcatRecord relcatRecord;
    if((rc = getRelInfo(relName.c_str(), relcatRecord))) {
        return rc;
    }
    int attrCount = relcatRecord.attrCount;
    AttrcatRecord *attrcatRecords = new AttrcatRecord[attrCount];
    if((rc = getAttrInfo(relName.c_str(), attrcatRecords))) {
        delete [] attrcatRecords;
        return rc;
    }
    rAttributes.clear();
    for(int i = 0; i < attrCount; ++i) {
        rAttributes.emplace_back(string(attrcatRecords[i].attrName));
    }
    delete [] attrcatRecords;
    return 0;   // ok
}

// dropTable: 删除一个表
// - 检查表是否存在
// - 如果存在, 则检查每个属性是否有索引, 如果有则删除索引文件
// - 删除该表在relcat文件和attrcat文件中对应的表项
// - 删除表文件
RC DDL_Manager::dropTable(const char *relName) {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    if(relName == nullptr) {
        return DDL_NULL_RELATION;
    }
    int rc;
    // 检查表是否存在
    if((rc = relExists(relName))) {
        return rc;
    }
    RM_Record rec;
    if((rc = getRelcatRec(relName, rec))) {
        return rc;
    }
    RID rid;
    if((rc = rec.getRid(rid))) {
        return rc;
    }
    // 删除relcat文件中对应的表项
    if((rc = relFileHandle.deleteRec(rid))) {
        return rc;
    }

    // 检查索引是否存在
    RM_FileScan attrcatFS;
    if((rc = attrcatFS.openScan(attrFileHandle, STRING, MAXNAME, 0, EQ_OP, (void*)relName))) {
        return rc;
    }
    while((rc = attrcatFS.getNextRec(rec)) == 0) {
        char *attrcatData;
        if((rc = rec.getData(attrcatData))) {
            return rc;
        }
        AttrcatRecord *attrcatRecord = (AttrcatRecord*)attrcatData;
        // 如果有索引, 则删除索引文件
        // 删除索引file
        if(attrcatRecord->indexNo != -1) {
            if((rc = ixManager->destroyIndex(relName, attrcatRecord->indexNo))) {
                return rc;
            }
        }
        if((rc = rec.getRid(rid))) {
            return rc;
        }
        // 删除该属性在attrcat中对应的表项
        if((rc = attrFileHandle.deleteRec(rid))) {
            return rc;
        }
    }
    // relcat和attrcat文件的对应的表项删除完成, 将信息写回磁盘
    if((rc = relFileHandle.forcePages()) ||
            (rc = attrFileHandle.forcePages())) {
        return rc;
    }
    // 删除表文件
    if((rc = rmManager->destroyFile(relName))) {
        return rc;
    }
    return 0; // ok
}

// dropIndex: 删除relName.attrName上的索引
// - 检查数据库是否打开
// - 检查是输入字符串是否为空
// - 检查索引是否存在
// - 如果存在则更新系统表并删除索引文件
RC DDL_Manager::dropIndex(const char *relName, const char *attrName) {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    if(relName == nullptr) {
        return DDL_NULL_RELATION;
    }
    if(attrName == nullptr) {
        return DDL_NULL_ATTRIBUTE;
    }
    int rc;
    // 检查索引是否存在
    if((rc = indexExists(relName, attrName))) {
        return rc;
    }
    // 更新relcat文件和attrcat文件
    RM_Record rec;
    char relationName[MAXNAME + 1];
    strcpy(relationName, relName);
    // 修改relcat文件元信息
    if((rc = getRelcatRec(relName, rec))) {
        return rc;
    }
    char *relcatData;
    if((rc = rec.getData(relcatData))) {
        return rc;
    }
    // 将表元信息的indexCount--
    RelcatRecord *relcatRecord = (RelcatRecord*)relcatData;
    relcatRecord->indexCount--;
    // 更新relcat
    if((rc = relFileHandle.updateRec(rec))) {
        return rc;
    }
    // 更新attrcat文件元信息
    if((rc = getAttrcatRec(relName, attrName, rec))) {
        return rc;
    }
    char *attrcatData;
    if((rc = rec.getData(attrcatData))) {
        return rc;
    }
    // 修改indexNo
    AttrcatRecord *attrcatRecord = (AttrcatRecord*)attrcatData;
    int indexNo = attrcatRecord->indexNo;   // 记录删除索引的属性在attrcat中记录的索引号
    attrcatRecord->indexNo = -1;
    if((rc = attrFileHandle.updateRec(rec))) {
        return rc;
    }

    // 强制写回
    if((rc = relFileHandle.forcePages()) ||
            (rc = attrFileHandle.forcePages())) {
        return rc;
    }
    // 删除索引文件
    if((rc = ixManager->destroyIndex(relName, indexNo))) {
        return rc;
    }
    return 0;   // ok
}

// createIndex: 调用ixManager提供的创建索引的接口, 创建索引
//
RC DDL_Manager::createIndex(const char *relName, const char *attrName) {
    if(!bDbOpen) {
        return DDL_DATABASE_NOT_OPEN;
    }
    if(relName == nullptr) {
        return DDL_NULL_RELATION;
    }
    if(attrName == nullptr) {
        return DDL_NULL_ATTRIBUTE;
    }
    int rc;
    if((rc = indexExists(relName, attrName)) == 0) {
        return DDL_INDEX_EXISTS;
    }
    else if(rc != DDL_INDEX_NOT_EXISTS) {
        return rc;
    }
    // 更新relcat
    RM_FileScan relcatFS;
    RM_Record rec;
    if((rc = getRelcatRec(relName, rec))) {
        return rc;
    }
    char *relcatData;
    if((rc = rec.getData(relcatData))) {
        return rc;
    }
    RelcatRecord *relcatRecord = (RelcatRecord*)relcatData;
    relcatRecord->indexCount++;
    if((rc = relFileHandle.updateRec(rec))) {
        return rc;
    }
    // 更新attrcat
    RM_FileScan attrcatFS;
    if ((rc = attrcatFS.openScan(attrFileHandle, STRING, MAXNAME, 0, EQ_OP, (void*)relName))) {
        return rc;
    }
    int indexNo = 0;
    // 索引属性的类型, 长度, 在记录中的偏移
    AttrType attrType;
    int attrLength;
    int attrOffset;
    while (rc != RM_EOF) {
        rc = attrcatFS.getNextRec(rec);
        if (rc != 0 && rc != RM_EOF) {
            return rc;
        }
        char *attrcatData;
        if (rc != RM_EOF) {
            if ((rc = rec.getData(attrcatData))) {
                return rc;
            }
            AttrcatRecord* attrcatRecord = (AttrcatRecord*)attrcatData;
            // update
            if (strcmp(attrcatRecord->attrName, attrName) == 0) {
                attrcatRecord->indexNo = indexNo;
                attrType = attrcatRecord->attrType;
                attrLength = attrcatRecord->attrLength;
                attrOffset = attrcatRecord->offset;
                if ((rc = attrFileHandle.updateRec(rec))) {
                    return rc;
                }
                break;
            }
        }
        ++indexNo;
    }
    if((rc = attrcatFS.closeScan())) {
        return rc;
    }
    // 强制写回
    if((rc = relFileHandle.forcePages()) ||
            (rc = attrFileHandle.forcePages())) {
        return rc;
    }
    // 创建索引
    if((rc = ixManager->createIndex(relName, indexNo, attrType, attrLength))) {
        return rc;
    }
    // 将该属性对应的项全部插入索引
    IX_IndexHandle ixIH;
    if((rc = ixManager->openIndex(relName, indexNo, ixIH))) {
        return rc;
    }
    // 扫描表中的所有项
    RM_FileHandle rmFileHandle;
    RM_FileScan rmFileScan;
    if((rc = rmManager->openFile(relName, rmFileHandle)) ||
            (rc = rmFileScan.openScan(rmFileHandle, attrType, attrLength, attrOffset, NO_OP, nullptr))) {
        return rc;
    }
    while((rc = rmFileScan.getNextRec(rec)) == 0) {
        char *recordData;
        RID rid;
        if((rc = rec.getData(recordData)) ||
                (rc = rec.getRid(rid))) {
            return rc;
        }
        if(attrType == INT) {
            int val;
            memcpy(&val, recordData + attrOffset, sizeof(val));
            if((rc = ixIH.insertEntry(&val, rid))) {
                return rc;
            }
        }
        else if(attrType == FLOAT) {
            float val;
            memcpy(&val, recordData + attrOffset, sizeof(val));
            if((rc = ixIH.insertEntry(&val, rid))) {
                return rc;
            }
        }
        else if(attrType == STRING) {
            char* val = new char[attrLength];
            strcpy(val, recordData + attrOffset);
            if((rc = ixIH.insertEntry(&val, rid))) {
                return rc;
            }
            delete [] val;
        }
    }
    if((rc = rmFileScan.closeScan())) {
        return rc;
    }
    // 关闭表文件, 关闭索引文件
    if((rc = rmManager->closeFile(rmFileHandle)) ||
            (rc = ixManager->closeIndex(ixIH))) {
        return rc;
    }
    return 0; // ok
}

// getAttrInfo: 获取relName.attrName的信息
RC DDL_Manager::getAttrInfo(const char *relName, const char *attrName, AttrcatRecord &attrInfo) {
    int rc;
    RM_Record rec;
    if((rc = getAttrcatRec(relName, attrName, rec))) {
        return rc;
    }
    char *attrcatData;
    if((rc = rec.getData(attrcatData))) {
        return rc;
    }
    AttrcatRecord *attrcatRecord = (AttrcatRecord*)attrcatData;
    attrInfo = *attrcatRecord;
    return 0;   // ok
}

RC DDL_Manager::indexExists(const char *relName, const char *attrName) {
    // 检查是否存在索引
    int rc;
    AttrcatRecord attrcatRecord;
    if((rc = getAttrInfo(relName, attrName, attrcatRecord))) {
        return rc;
    }
    if(attrcatRecord.indexNo == -1) {
        return DDL_INDEX_NOT_EXISTS;
    }
    return 0;   // ok
}

RC DDL_Manager::relExists(const char *relationName) {
    int rc;
    RM_Record rec;
    RM_FileScan relcatFileScan;
    if((rc = relcatFileScan.openScan(relFileHandle, STRING, MAXNAME, offsetof(RelcatRecord, relName), EQ_OP, (void*)relationName))) {
        return rc;
    }
    if((rc = relcatFileScan.getNextRec(rec))) {
        if(rc == RM_EOF) {
            return DDL_REL_NOT_EXISTS;
        }
        return rc;
    }
    return 0;   // ok
}

// getRelcatRec: 获取relcat表中relName表对应的记录
RC DDL_Manager::getRelcatRec(const char *relName, RM_Record &relcatRecord) {
    int rc;
    RM_FileScan relcatFS;
    if((rc = relcatFS.openScan(relFileHandle, STRING, MAXNAME, 0, EQ_OP, (void*)relName))) {
        return rc;
    }
    if((rc = relcatFS.getNextRec(relcatRecord))) {
        if(rc == RM_EOF) {
            return DDL_REL_NOT_EXISTS;
        }
        return rc;
    }
    return 0;   // ok
}

// getAttrcatRec: 获取attrcat表中relName.attrName对应的记录
RC DDL_Manager::getAttrcatRec(const char *relName, const char *attrName, RM_Record &attrcatRecord) {
    int rc;
    RM_FileScan attrcatFS;
    if((rc = attrcatFS.openScan(attrFileHandle, STRING, MAXNAME, 0, EQ_OP, (void *) relName))) {
        return rc;
    }
    // 查找该属性的表项
    while((rc = attrcatFS.getNextRec(attrcatRecord)) == 0) {
        char *pData;
        if((rc = attrcatRecord.getData(pData))) {
            return rc;
        }
        AttrcatRecord *tmpRec = (AttrcatRecord*)pData;
        // 找到attrcat表对应的表项
        if(strcmp(tmpRec->attrName, attrName) == 0) {
            break;
        }
    }
    if((rc = attrcatFS.closeScan())) {
        return rc;
    }
    return 0;   // ok
}


