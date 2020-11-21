//
// Created by Administrator on 2020/11/21.
//

#ifndef MYDATABASE_PROCLIST_TYPES_H
#define MYDATABASE_PROCLIST_TYPES_H

// 进程双向链表
struct proclist_node {
    int next;
    int prev;
};

struct proclist_head {
    int head;
    int tail;
};

#endif //MYDATABASE_PROCLIST_TYPES_H
