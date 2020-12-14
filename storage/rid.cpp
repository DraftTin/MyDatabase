//
// Created by Administrator on 2020/11/9.
//

#include "rm.h"

// 初始化pageNum和slotNum为INVALID
RID::RID() : pageNum(-1), slotNum(-1) {
    // do nothing
}

RID::RID(PageNum _pageNum, SlotNum _slotNum) {
    this->pageNum = _pageNum;
    this->slotNum = _slotNum;
}

RID::~RID() {
    // do nothing
}

RC RID::getPageNum(PageNum &pageNum) const {
    // 修改bug: 将RM_INVALID_PAGE_NUMBER修改成-1
    if(pageNum == -1) {
        return RID_NOT_VIABLE;
    }
    pageNum = this->pageNum;
    return 0;
}

RC RID::getSlotNum(SlotNum &slotNum) const {
    // 修改bug: 将RM_INVALID_PAGE_NUMBER修改成-1
    if(pageNum == -1) {
        return RID_NOT_VIABLE;
    }
    slotNum = this->slotNum;
    return 0;
}

