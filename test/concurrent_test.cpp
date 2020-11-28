//
// Created by Administrator on 2020/11/28.
//
#include <iostream>
#include <thread>
#include "../storage/bufhashtable.h"

using namespace std;

void test1();
void test2();

void (*test[])() = {
        test1,
        test2
};

void Insert1(BufHashTable& hashTable) {
    int i = 0, j = 1000;
    for(; i < 1000; ++i, --j) {
        int slot = i + j;
        printf("insert1:\n");
        hashTable.insert(i, j, slot);
    }
}

void Insert2(BufHashTable& hashTable) {
    int i = 1000, j = 0;
    for(; i < 2000; ++i, --j) {
        int slot = i + j;
        printf("insert2:\n");
        hashTable.insert(i, j, slot);
    }
}


int main(int argc, char *argv[]) {
    test2();
}

// 测试基本的插入删除
void test1() {
    cout.flush();
    cout.flush();
    cout << "Starting Concurrent test 1\n";
    cout.flush();
    BufHashTable hashTable(40);
    for(int i = 0, j = 10; i < 10; ++i, --j) {
            int slot = i + j;
            hashTable.insert(i, j, slot);
    }
    for(int i = 0, j = 10; i < 10; ++i, --j) {
            int slot;
            hashTable.find(i, j, slot);
            if(slot != i + j) {
                cout << "insert error\n";
            }
    }
    for(int i = 0, j = 10; i < 10; ++i, --j) {
            hashTable.remove(i, j);
    }
    for(int i = 0, j = 10; i < 10; ++i, --j) {
            int slot = i + j;
            int rc = hashTable.find(i, j, slot);
            if(rc != PF_HASHNOTFOUND) {
                cout << "remove error\n";
            }
    }
}

// 测试并发插入删除
void test2() {
    BufHashTable hashTable(40);
    thread thr1(Insert1, ref(hashTable));
    thread thr2(Insert2, ref(hashTable));
    if(thr1.joinable()) {
        thr1.join();
    }
    if(thr2.joinable()) {
        thr2.join();
    }
}