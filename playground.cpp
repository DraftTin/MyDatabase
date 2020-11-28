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
}

void Write() {
    WriteGuard writeGuard(rwLock);
    printf("writing\n");
}

int main() {
    ReadGuard readGuard(rwLock);
    readGuard.unlock();
//    thread thr1{Read};
//    thread thr2{Write};
//    thr1.join();
//    thr2.join();
}