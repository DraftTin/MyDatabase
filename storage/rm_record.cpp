//
// Created by Administrator on 2020/11/9.
//

#include "rm.h"

// RM_Record: 初始化RM_Record实例
RM_Record::RM_Record() : isValid(FALSE), pData(nullptr) {
    // do nothing
}

RM_Record::~RM_Record() {
    if(isValid) {
        isValid = FALSE;
        delete [] pData;
    }
}

// getData: 获取页内的数据
RC RM_Record::getData(char *&pData) const {
    if(!isValid) {
        return RM_INVALIDRECORD;
    }
    pData = this->pData;
    return 0;   // ok
}

// getRid: 获取rid
RC RM_Record::getRid(RID &rid) const {
    // 保证该记录可用
    if(!isValid) {
        return RM_INVALIDRECORD;
    }
    rid = this->rid;
    return 0;       // ok
}
