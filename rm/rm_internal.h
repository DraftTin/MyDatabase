//
// Created by Administrator on 2020/11/9.
//

#ifndef MYDATABASE_RM_INTERNAL_H
#define MYDATABASE_RM_INTERNAL_H

#include <string>
#include "rm.h"

#define RM_NO_FREE_PAGE     -1      // null pointer for the freeHead list

// 每页相当于list上的一个节点，头部记录链表的next信息，RM_NO_FREE_PAGE表示null pointer
struct RM_PageHdr {
    PageNum nextPage;
};

#endif //MYDATABASE_RM_INTERNAL_H
