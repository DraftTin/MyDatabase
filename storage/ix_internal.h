//
// Created by Administrator on 2020/12/5.
//

#ifndef MYDATABASE_IX_INTERNAL_H
#define MYDATABASE_IX_INTERNAL_H

#include <string>
#include "ix.h"

#define IX_NULL_POINTER -1
#define IX_NO_PAGE -1

// Bp树的结点的类型：根节点，非叶结点，叶结点，同时为根节点和叶结点
enum IX_NodeType {
    ROOT,
    NODE,
    LEAF,
    ROOT_LEAF
};

// 结点中保存的若干索引项值的类型，如果是叶结点对应的类型就是EMPTY或RID_FILLED表示索引val里面存放着rid的值，并且nextPage指针指向溢出块的页号
// 如果非叶结点则为EMPTY或PAGE_ONLY表示索引项val不存放rid，而且nextPage保存子节点的页号
enum IX_ValueType {
    EMPTY,
    PAGE_ONLY,
    RID_FILLED
};

// 索引val: 能够表示子节点的页号，或索引到rid的值并存储溢出块的位置
struct IX_NodeValue {
    IX_ValueType state;
    RID rid;
    PageNum nextPage;       // 如果是叶节点，则表示溢出页的页号, 链表结构，如果是非叶结点则是子节点的页号

    IX_NodeValue() : state(EMPTY), rid(-1, -1), nextPage(IX_NO_PAGE) {}
};

// 索引文件中索引节点的页头结构
struct IX_NodeHeader {
    int numberKeys;
    int keyCapacity;
    PageNum prePage;    // 前一个结点也就是左边的结点
    IX_NodeType type;
    PageNum parent;
};

// 索引文件中溢出块的页头结构
struct IX_OverflowPageHeader {
    int numberRecords;  // 记录桶中记录的数量
    int recordCapacity; // 记录最多能容纳的记录数量
    PageNum parentPage; // 记录父节点的页号
};

// 溢出块中能容纳的rid记录数量
const int RID_COUNT_OF_OVERFLOW_PAGE = (PF_PAGE_SIZE - sizeof(IX_OverflowPageHeader)) / sizeof(RID);


#endif //MYDATABASE_IX_INTERNAL_H
