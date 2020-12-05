//
// Created by Administrator on 2020/11/27.
//

#include <process.h>
#include <io.h>
#include <iostream>
#include <fcntl.h>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "atomic/RWLock.h"

using namespace std;

RWLock rwLock;

void Read() {
    ReadGuard readGuard(rwLock);
    printf("reading\n");
    while(1) {
        cout << 44 << endl;
    }
}

void Write() {
    WriteGuard writeGuard(rwLock);
    printf("writing\n");
    while(1) {

    }
}

void test(WriteGuard &writeGuard) {

}


void test2(thread::id id) {

}

struct AA {
    int a;
    friend std::ostream &operator<<(std::ostream &s, const AA &aa) {
        return s;
    }
};

int main() {
    cout << sizeof(AA);
//    mutex a;
//    unique_lock<mutex> aa;
//    aa = unique_lock<mutex>(a);
//    aa.lock();
/////
//    WriteGuard writeGuard(rwLock);
//
//    thread thr2(test, ref(writeGuard));
//    thread thr1(test, ref(writeGuard));
//    cout << thr2.get_id() << endl;
//    cout << thr1.get_id() << endl;

//    thr1.join();
//    thr2.join();
}