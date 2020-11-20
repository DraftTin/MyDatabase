//
// Created by Administrator on 2020/11/6.
//

#include "pf_internal.h"
#include "pf_hashtable.h"
#include <iostream>

using namespace std;

// 哈希表构造函数，创建numBuckets个桶
PF_HashTable::PF_HashTable(int _numBuckets) {
    this->numBuckets = _numBuckets;
    // 初始化哈希表
    hashTable = new PF_HashEntry* [numBuckets];

    for(int i = 0; i < numBuckets; ++i) {
        hashTable[i] = nullptr;
    }
}

// 哈希表析构函数, 逐项删除哈希表中的项
PF_HashTable::~PF_HashTable() {
    for(int i = 0; i < numBuckets; ++i) {
        // 删除所有项
        PF_HashEntry* entry = hashTable[i];
        while(entry != nullptr) {
            PF_HashEntry* next = entry->next;
            delete entry;
            entry = next;
        }
    }
    delete[] hashTable;
}

 // (pageNum, slot)唯一标识一条记录
 RC PF_HashTable::find(int fd, PageNum pageNum, int& slot) {
     int bucket = Hash(fd, pageNum);
     if(bucket < 0) {
         return PF_HASHNOTFOUND;
     }

     for(PF_HashEntry* entry = hashTable[bucket]; entry != nullptr;
             entry = entry->next) {
         if(entry->fd == fd && entry->pageNum == pageNum) {
             // 找到
             slot = entry->slot;
             return 0;
         }
     }
     return PF_HASHNOTFOUND;
 }
int flag = 0;
 RC PF_HashTable::insert(int fd, PageNum pageNum, int slot) {
    int bucket = Hash(fd, pageNum);

    PF_HashEntry* entry;
    for(entry = hashTable[bucket]; entry != nullptr; entry = entry->next) {
        if(entry->fd == fd && entry->pageNum == pageNum) {
            return PF_HASHPAGEEXIST;
        }
    }

    if((entry = new PF_HashEntry) == nullptr) {
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

    return 0;   // 插入成功
}

RC PF_HashTable::remove(int fd, PageNum pageNum) {
    int bucket = Hash(fd, pageNum);
    PF_HashEntry* entry;
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

    return 0;
}



