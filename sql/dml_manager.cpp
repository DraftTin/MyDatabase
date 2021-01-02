//
// Created by Administrator on 2020/12/2.
//

#include <cstring>
#include "dml.h"

DML_Manager::DML_Manager(RM_Manager &_rmManager, DDL_Manager &_ddlManager) : rmManager(&_rmManager), ddlManager(&_ddlManager) {
    // Don't need to do anything
}

// insert: 向{relName}表中插入一条数据, nValues和数据字典中该表的属性数量对应, values插入的每个属性的值
// - 获取表信息和表属性信息, 验证
// - 将value数组按照顺序拼接为记录的字符串(如果有变长类型需要转换)
// - 将拼接好的字符串插入到{relName}对应的表文件中
// - 若有变长属性, 则将变长属性插入到表中
RC DML_Manager::insert(const char *relName, int nValues, const Value *values) {
    int rc;
    RelcatRecord relcatRecord;
    if((rc = ddlManager->getRelInfo(relName, relcatRecord))) {
        return rc;
    }
    int attrCount = relcatRecord.attrCount;
    AttrcatRecord *attrcatRecords = new AttrcatRecord[attrCount];
    if((rc = ddlManager->getAttrInfo(relName, attrcatRecords))) {
        delete [] attrcatRecords;
        return rc;
    }
    // 检查是否一致
    if(nValues != attrCount) {
        delete [] attrcatRecords;
        return DML_ATTR_COUNT_INCORRECT;
    }
    // 修改bug: if(values[i].type != attrcatRecords->attrType)修改为if(values[i].type != attrcatRecords[i]->attrType)
    for(int i = 0; i < nValues; ++i) {
        if(values[i].type != attrcatRecords[i].attrType) {
            delete [] attrcatRecords;
            return DML_ATTR_TYPE_INCORRECT;
        }
    }
    char *recordData = new char[relcatRecord.tupleLength];
    memset(recordData, 0, relcatRecord.tupleLength);
    // 构造记录字符串
    int begin = 0;
    for(int i = 0; i < nValues; ++i) {
        Value tmpValue = values[i];
        if(attrcatRecords[i].attrType == INT || attrcatRecords[i].attrType == FLOAT) {
            memcpy(recordData + begin, (char*)(tmpValue.value), 4);
            begin += 4;
        }
        else if(attrcatRecords[i].attrType == STRING) {
            memcpy(recordData + begin, (char*)(tmpValue.value), attrcatRecords[i].attrLength);
            begin += attrcatRecords[i].attrLength;
        }
        else if(attrcatRecords[i].attrType == VARCHAR) {
            RM_VarLenAttr varLenAttr;
            memcpy(recordData + begin, (char*)&varLenAttr, sizeof(RM_VarLenAttr));
            begin += sizeof(RM_VarLenAttr);
        }
    }

    RM_FileHandle rmFileHandle;
    if((rc = rmManager->openFile(relName, rmFileHandle))) {
        delete [] attrcatRecords;
        delete [] recordData;
        return rc;
    }
    RID rid;
    // 插入记录
    if((rc = rmFileHandle.insertRec(recordData, rid))) {
        delete [] attrcatRecords;
        delete [] recordData;
        return rc;
    }
    // 若有变长属性, 插入变长属性
    for(int i = 0; i < nValues; ++i) {
        if(values[i].type == VARCHAR) {
            RM_VarLenAttr rmVarLenAttr;
            if((rc = rmFileHandle.insertVarValue(rid, attrcatRecords[i].offset, (char*)(values[i].value),
                                                 attrcatRecords[i].attrLength, rmVarLenAttr))) {
                delete [] attrcatRecords;
                delete [] recordData;
                return rc;
            }
        }
    }
    delete [] attrcatRecords;
    delete [] recordData;
    // 关闭文件
    if((rc = rmManager->closeFile(rmFileHandle))) {
        return rc;
    }
    return 0;   // ok
}

// 唯一不同就是返回插入记录的rid
RC DML_Manager::insertForTest(const char *relName, int nValues, const Value *values, RID &rRid) {
    int rc;
    RelcatRecord relcatRecord;
    if((rc = ddlManager->getRelInfo(relName, relcatRecord))) {
        return rc;
    }
    int attrCount = relcatRecord.attrCount;
    AttrcatRecord *attrcatRecords = new AttrcatRecord[attrCount];
    if((rc = ddlManager->getAttrInfo(relName, attrcatRecords))) {
        delete [] attrcatRecords;
        return rc;
    }
    // 检查是否一致
    if(nValues != attrCount) {
        delete [] attrcatRecords;
        return DML_ATTR_COUNT_INCORRECT;
    }
    // 修改bug: if(values[i].type != attrcatRecords->attrType)修改为if(values[i].type != attrcatRecords[i]->attrType)
    for(int i = 0; i < nValues; ++i) {
        if(values[i].type != attrcatRecords[i].attrType) {
            delete [] attrcatRecords;
            return DML_ATTR_TYPE_INCORRECT;
        }
    }
    char *recordData = new char[relcatRecord.tupleLength];
    memset(recordData, 0, relcatRecord.tupleLength);
    // 构造记录字符串
    int begin = 0;
    for(int i = 0; i < nValues; ++i) {
        Value tmpValue = values[i];
        if(attrcatRecords[i].attrType == INT || attrcatRecords[i].attrType == FLOAT) {
            memcpy(recordData + begin, (char*)(tmpValue.value), 4);
            begin += 4;
        }
        else if(attrcatRecords[i].attrType == STRING) {
            memcpy(recordData + begin, (char*)(tmpValue.value), attrcatRecords[i].attrLength);
            begin += attrcatRecords[i].attrLength;
        }
        else if(attrcatRecords[i].attrType == VARCHAR) {
            RM_VarLenAttr varLenAttr;
            memcpy(recordData + begin, (char*)&varLenAttr, sizeof(RM_VarLenAttr));
            begin += sizeof(RM_VarLenAttr);
        }
    }

    RM_FileHandle rmFileHandle;
    if((rc = rmManager->openFile(relName, rmFileHandle))) {
        delete [] attrcatRecords;
        delete [] recordData;
        return rc;
    }
    RID rid;
    // 插入记录
    if((rc = rmFileHandle.insertRec(recordData, rid))) {
        delete [] attrcatRecords;
        delete [] recordData;
        return rc;
    }
    /////////////
    rRid = rid;
    /////////////
    // 若有变长属性, 插入变长属性
    for(int i = 0; i < nValues; ++i) {
        if(values[i].type == VARCHAR) {
            RM_VarLenAttr rmVarLenAttr;
            if((rc = rmFileHandle.insertVarValue(rid, attrcatRecords[i].offset, (char*)(values[i].value),
                                                 attrcatRecords[i].attrLength, rmVarLenAttr))) {
                delete [] attrcatRecords;
                delete [] recordData;
                return rc;
            }
        }
    }
    delete [] attrcatRecords;
    delete [] recordData;
    // 关闭文件
    if((rc = rmManager->closeFile(rmFileHandle))) {
        return rc;
    }
    return 0;   // ok
}

// Select: 執行Select執行物理計劃
//
RC DML_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations,
                       int nConditions, const Condition *conditions) {
    if(!ddlManager->isOpen()) {
        return DML_DATABASE_CLOSED;
    }
    int rc;
    // 記錄各個表的信息
    RelcatRecord relInfoList[nRelations];
    // 記錄每個表對應的屬性的信息
    AttrcatRecord *attributesList[nRelations];
    // 循环獲取各個表的信息，和各個表中屬性的信息
    for(int i = 0; i < nRelations; ++i) {
        if((rc = ddlManager->getRelInfo(relations[i], relInfoList[i]))) {
            for(int j = 0; j < i; ++j) {
                delete [] attributesList[j];
            }
            return rc;
        }
        attributesList[i] = new AttrcatRecord[relInfoList[i].attrCount];
        if((rc = ddlManager->getAttrInfo(relations[i], attributesList[i]))) {
            return rc;
        }
    }
    RelAttr *changedSelectList = nullptr;
    // 如果是 select * , 则将各个表的所有属性重新添加到changedSelectList中
    if (nSelAttrs == 1 && strcmp(selAttrs[0].attrName, "*") == 0) {
        nSelAttrs = 0;
        for (int i = 0; i < nRelations; ++i) {
            nSelAttrs += relInfoList[i].attrCount;
        }
        changedSelectList = new RelAttr[nSelAttrs];
        int index = 0;
        for (int i = 0; i < nRelations; ++i) {
            for (int j = 0; j < relInfoList[i].attrCount; ++j) {
                changedSelectList[index].relName = relInfoList[i].relName;
                changedSelectList[index].attrName = attributesList[i][j].attrName;
                ++index;
            }
        }
    }
    // 直接添加
    else {
        int index = 0;
        changedSelectList = const_cast<RelAttr*>(selAttrs);
    }
    Condition changedConditionList[nConditions];


}

RC DML_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
    Select();
    return 0;
}

RC DML_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr,
                       const Value &rhsValue, int nConditions, const Condition *conditions) {
    Update();
    return 0;
}

