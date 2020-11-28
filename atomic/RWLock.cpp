//
// Created by Administrator on 2020/11/27.
//
#include "RWLock.h"

using namespace std;

RWLock::RWLock() : writeCount(0), readCount(0) {
    // do nothing
}

// readLock: 读锁
void RWLock::readLock() {
    unique_lock<mutex> rqLck(readMutex);
    rq.wait(rqLck, [this] {return writeCount == 0;});
    unique_lock<mutex> readCountLck(readCountMutex);
    ++readCount;
}

void RWLock::readUnlock() {
    unique_lock<mutex> readCountLck(readCountMutex);
    if(--readCount == 0) {
        wq.notify_one();
    }
}

void RWLock::writeLock() {
    unique_lock<mutex> wqLck(writeMutex);
    wq.wait(wqLck, [this] {
        unique_lock<mutex> writeCountLck(writeCountMutex);
        ++writeCount;
        return writeCount == 1 || readCount == 0;
    });
}

void RWLock::writeUnlock() {
    unique_lock<mutex> writeCountLck(writeCountMutex);
    if(--writeCount != 0) {
        wq.notify_one();
    }
    else {
        rq.notify_all();
    }
}

ReadGuard::ReadGuard(RWLock &_rwLock) : rwLock(_rwLock) {
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
    rwLock.readLock();
    myOwns = true;
}

void ReadGuard::unlock() {
    // if(!myOwns) throws
    rwLock.readUnlock();
    myOwns = false;
}

WriteGuard::WriteGuard(RWLock &_rwLock) : rwLock(_rwLock) {
    lock();
    myOwns = true;
}

WriteGuard::~WriteGuard() {
    if(myOwns) {
        unlock();
    }
}

void WriteGuard::lock() {
    rwLock.writeLock();
    myOwns = true;
}

void WriteGuard::unlock() {
    rwLock.writeUnlock();
    myOwns = false;
}



