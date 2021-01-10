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
#include <sstream>
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
    char dbname[20] = "cmake-build-debug";
    chdir(dbname);
    char commandA[20] = ".\\dbcreate testdb";
    system(commandA);
    char commandB[40] = "cmake-build-debug\\dbcreate testdb";
    chdir("..");
    system(commandB);
    int b = 2;
    int c = 3;
}