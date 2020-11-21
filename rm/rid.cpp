//
// Created by Administrator on 2020/11/9.
//

#include "rm.h"

// 初始化pageNum和slotNum为INVALID
RID::RID() : pageNum(RM_INVALID_PAGE_NUMBER), slotNum(RM_INVALID_SLOT_NUMBER) {
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
    if(pageNum == RM_INVALID_PAGE_NUMBER) {
        return RID_NOT_VIABLE;
    }
    pageNum = this->pageNum;
    return 0;
}

RC RID::getSlotNum(SlotNum &slotNum) const {
    if(pageNum == RM_INVALID_PAGE_NUMBER) {
        return RID_NOT_VIABLE;
    }
    slotNum = this->slotNum;
    return 0;
}

