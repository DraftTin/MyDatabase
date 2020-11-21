//
// Created by Administrator on 2020/11/9.
//

#ifndef MYDATABASE_RID_H
#define MYDATABASE_RID_H

#include "../global.h"

typedef int SlotNum;
typedef int PageNum;

class RID {
public:
    RID();
    RID(PageNum _pageNum, SlotNum _slotNum);
    ~RID();
    RC getPageNum(PageNum &pageNum) const;
    RC getSlotNum(SlotNum &slotNum) const;

private:
    PageNum pageNum;
    SlotNum slotNum;
};

#define RID_NOT_VIABLE      (START_RM_WARN + 0)     // 记录不可用

#endif //MYDATABASE_RID_H
