//
// Created by Administrator on 2020/12/5.
//

#ifndef MYDATABASE_IX_INTERNAL_H
#define MYDATABASE_IX_INTERNAL_H

#include <string>
#include "ix.h"

#define IX_NULL_POINTER -1
#define IX_NO_PAGE -1


enum IX_NodeType {
    ROOT,
    NODE,
    LEAF,
    ROOT_LEAF
};

enum IX_ValueType {
    EMPTY,
    PAGE_ONLY,
    RID_FILLED
};

struct IX_NodeValue {
    IX_ValueType state;
    RID rid;
    PageNum nextPage;       // 溢出页的页号, 链表结构

    IX_NodeValue() : state(EMPTY), rid(-1, -1), nextPage(IX_NO_PAGE) {}
};

struct IX_NodeHeader {
    int numberKeys;
    int keyCapacity;
    IX_NodeType type;
    PageNum parent;
};

struct IX_OverflowPageHeader {
    int numberRecords;  // 记录桶中记录的数量
    int recordCapacity; // 记录最多能容纳的记录数量
    PageNum parentPage; // 记录父节点的页号
};

// 溢出块中能容纳的rid记录数量
const int RID_COUNT_OF_OVERFLOW_PAGE = (PF_PAGE_SIZE - sizeof(IX_OverflowPageHeader)) / sizeof(RID);


#endif //MYDATABASE_IX_INTERNAL_H
