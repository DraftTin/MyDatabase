//
// Created by Administrator on 2020/11/6.
//

#ifndef PF_HASHTABLE_H
#define PF_HASHTABLE_H

#include "pf_internal.h"
#include "../atomic/RWLock.h"

#define HASH_INCONSISTENT -301;

struct BufHashEntry {
    BufHashEntry* next;
    BufHashEntry* prev;
    int fd;
    PageNum pageNum;
    int slot;               // 标记该页在缓冲区的位置
};

// 缓冲区页的哈希表，表项对应的是: (fd, pageNum) -> slot 的映射，其中slot表示的是该页在缓冲区的位置
class BufHashTable {
public:
    explicit BufHashTable(int numBuckets);
    ~BufHashTable();
    RC find(int fd, PageNum pageNum, int& slot);
    RC insert(int fd, PageNum pageNum, int slot);
    RC remove(int fd, PageNum pageNum);
    void show(int hashcode);
private:
    int Hash(int fd, PageNum pageNum) const {
        return (fd + pageNum) % numBuckets;
    }
    int numBuckets;             // 记录桶的数量
    BufHashEntry** hashTable;   // 哈希表
    RWLock rwLock;              // 控制哈希表访问的读写锁
};

#endif

