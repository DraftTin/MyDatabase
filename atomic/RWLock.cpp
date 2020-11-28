//
// Created by Administrator on 2020/11/27.
//
#include "RWLock.h"
#include <shared_mutex>

using namespace std;

RWLock::RWLock() : writeCount(0), readCount(0) {
    // do nothing
}

// readLock: 读锁
void RWLock::readLock() {
    unique_lock<mutex> rwLck(rwMutex);
    rq.wait(rwLck, [this] {
        return writeCount == 0;
    });
    ++readCount;
}

void RWLock::readUnlock() {
    unique_lock<mutex> rwLck(rwMutex);
    if(--readCount == 0) {
        wq.notify_one();
    }
}

void RWLock::writeLock() {
    unique_lock<mutex> rwLck(rwMutex);
    // 修改bug, || -> &&
    ++writeCount;
    wq.wait(rwLck, [this] {
        return writeCount == 1 && readCount == 0;
    });
}

void RWLock::writeUnlock() {
    unique_lock<mutex> rwLck(rwMutex);
    if(--writeCount != 0) {
        wq.notify_one();
    }
    else {
        rq.notify_all();
    }
}

ReadGuard::ReadGuard(RWLock &_rwLock) : rwLock(&_rwLock) {
    lock();
    myOwns = true;
}

ReadGuard::~ReadGuard() {
    if(myOwns) {
        unlock();
    }
}

void ReadGuard::lock() {
    // if(!myOwns) throws
    rwLock->readLock();
    myOwns = true;
}

void ReadGuard::unlock() {
    // if(!myOwns) throws
    rwLock->readUnlock();
    myOwns = false;
}

ReadGuard::ReadGuard() : rwLock(nullptr), myOwns(false) {

}

// 将readGuard对锁的控制权转移到该锁上
ReadGuard &ReadGuard::operator=(ReadGuard &&readGuard)  noexcept {
    myOwns = readGuard.myOwns;
    readGuard.myOwns = false;
    rwLock = readGuard.rwLock;
    return *this;
}

WriteGuard::WriteGuard(RWLock& _rwLock) : rwLock(&_rwLock) {
    lock();
    myOwns = true;
}

WriteGuard::~WriteGuard() {
    if(myOwns) {
        unlock();
    }
}

void WriteGuard::lock() {
    rwLock->writeLock();
    myOwns = true;
}

void WriteGuard::unlock() {
    rwLock->writeUnlock();
    myOwns = false;
}

WriteGuard::WriteGuard() : rwLock(nullptr), myOwns(false) {

}

// 将writeGuard对锁的控制权转移到该锁上
WriteGuard& WriteGuard::operator=(WriteGuard&& writeGuard)  noexcept {
    myOwns = writeGuard.myOwns;
    writeGuard.myOwns = false;
    rwLock = writeGuard.rwLock;
    return *this;
}




