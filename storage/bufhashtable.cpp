//
// Created by Administrator on 2020/11/6.
//

#include "pf_internal.h"
#include "bufhashtable.h"
#include <iostream>

using namespace std;

// 哈希表构造函数，创建numBuckets个桶
BufHashTable::BufHashTable(int _numBuckets) {
    this->numBuckets = _numBuckets;
    // 初始化哈希表
    hashTable = new BufHashEntry* [numBuckets];

    for(int i = 0; i < numBuckets; ++i) {
        hashTable[i] = nullptr;
    }
}

// 哈希表析构函数, 逐项删除哈希表中的项
BufHashTable::~BufHashTable() {
    for(int i = 0; i < numBuckets; ++i) {
        // 删除所有项
        BufHashEntry* entry = hashTable[i];
        while(entry != nullptr) {
            BufHashEntry* next = entry->next;
            delete entry;
            entry = next;
        }
    }
    delete[] hashTable;
}
 // find: 哈希表find函数
 // 输入: (nextPage, slot)唯一标识一条记录
 // 输出: slot - 缓冲区页下标
 RC BufHashTable::find(int fd, PageNum pageNum, int& slot) {
    // 加读锁
    ReadGuard readGuard(rwLock);
    int bucket = Hash(fd, pageNum);
    if(bucket < 0) {
        return PF_HASHNOTFOUND;
    }

    for(BufHashEntry* entry = hashTable[bucket]; entry != nullptr;
            entry = entry->next) {
        if(entry->fd == fd && entry->pageNum == pageNum) {
             slot = entry->slot;
             return 0;
        }
    }
    return PF_HASHNOTFOUND;
 }

 // insert: 哈希表insert函数
 // 输入: key: (fd, nextPage), value: slot
 // 输出: 无
 RC BufHashTable::insert(int fd, PageNum pageNum, int slot) {
     // 加写锁
    WriteGuard writeGuard(rwLock);
    int bucket = Hash(fd, pageNum);

    BufHashEntry* entry;
    for(entry = hashTable[bucket]; entry != nullptr; entry = entry->next) {
        if(entry->fd == fd && entry->pageNum == pageNum) {
            return PF_HASHPAGEEXIST;
        }
    }

    if((entry = new BufHashEntry) == nullptr) {
        return PF_NOMEM;
    }
    entry->fd = fd;
    entry->pageNum = pageNum;
    entry->slot = slot;
    entry->next = hashTable[bucket];
    entry->prev = nullptr;
    if(hashTable[bucket] != nullptr) {
        hashTable[bucket]->prev = entry;
    }
    hashTable[bucket] = entry;

//    printf("insert: (%d, %d, %d) \n", fd, nextPage, slot);
    // 并发产生不一致
    if(fd != hashTable[bucket]->fd || pageNum != hashTable[bucket]->pageNum) {
        return HASH_INCONSISTENT;
    }
    return 0;   // 插入成功
}

// remove: 哈希表remove函数
// 输入: key: (fd, nextPage)
// 输出: 无
RC BufHashTable::remove(int fd, PageNum pageNum) {
    // 加写锁
//    WriteGuard writeGuard(rwLock);
    int bucket = Hash(fd, pageNum);
    BufHashEntry* entry;
    for(entry = hashTable[bucket]; entry != nullptr; entry = entry->next) {
        if(entry->fd == fd && entry->pageNum == pageNum) {
            break;
        }
    }
    if(entry == nullptr) return PF_HASHNOTFOUND;

    if(entry == hashTable[bucket]) {
        hashTable[bucket] = entry->next;
    }
    if(entry->prev != nullptr){
        entry->prev->next = entry->next;
    }
    if(entry->next != nullptr) {
        entry->next->prev = entry->prev;
    }
    delete entry;
//    printf("remove: (%d, %d, %d)\n", fd, nextPage);
    return 0;
}

// show: 输出hashcode处的key, val链表
// 输入: hashcode
// 输出: 无
void BufHashTable::show(int hashcode) {
    if(hashTable[hashcode] == nullptr) {
        printf("null\n");
        return ;
    }
    BufHashEntry* entry;
    for(entry = hashTable[hashcode]; entry->next != nullptr; entry = entry->next) {
        printf("(%d, %d, %d) -> ", entry->fd, entry->pageNum, entry->slot);
    }
    printf("(%d, %d, %d)\n", entry->fd, entry->pageNum, entry->slot);
}



