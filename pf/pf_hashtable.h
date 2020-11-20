//
// Created by Administrator on 2020/11/6.
//

#ifndef PF_HASHTABLE_H
#define PF_HASHTABLE_H

#include "pf_internal.h"

struct PF_HashEntry {
    PF_HashEntry* next;
    PF_HashEntry* prev;
    int fd;
    PageNum pageNum;
    int slot;               // 标记该页在缓冲区的位置
};

// 页的哈希表，表项对应的是: (fd, pageNum) -> slot 的映射，其中slot表示的是该页在缓冲区的位置
class PF_HashTable {
public:
    explicit PF_HashTable(int numBuckets);
    ~PF_HashTable();
    RC find(int fd, PageNum pageNum, int& slot);
    RC insert(int fd, PageNum pageNum, int slot);
    RC remove(int fd, PageNum pageNum);
private:
    int Hash(int fd, PageNum pageNum) const {
        return (fd + pageNum) % numBuckets;
    }
    int numBuckets;             // 记录桶的数量
    PF_HashEntry** hashTable;   // 哈希表
};

#endif

