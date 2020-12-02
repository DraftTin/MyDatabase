//
// Created by Administrator on 2020/11/17.
//

#include "rm.h"

RM_VarLenAttr::RM_VarLenAttr() : pageNum(-1), slotNum(-1) {
    //
}

RM_VarLenAttr::RM_VarLenAttr(PageNum _pageNum, SlotNum _slotNum, int _index) : pageNum(_pageNum), slotNum(_slotNum), blockInfoIndex(_index){
    // 初始化
}

RM_VarLenAttr::~RM_VarLenAttr() {
    // don't need to do anything
}

RC RM_VarLenAttr::getPageNum(PageNum &pageNum) const {
    if(pageNum == -1) {
        return RM_RID_NOT_VIABLE;
    }
    pageNum = this->pageNum;
    return 0;
}

RC RM_VarLenAttr::getSlotNum(SlotNum &slotNum) const {
    if(pageNum == -1) {
        return RM_RID_NOT_VIABLE;
    }
    slotNum = this->slotNum;
    return 0;
}

RC RM_VarLenAttr::getBlockInfoIndex(int &index) const {
    if(pageNum == -1) {
        return RM_RID_NOT_VIABLE;
    }
    index = this->blockInfoIndex;
    return 0;
}


