//
// Created by Administrator on 2020/11/27.
//

#ifndef MYDATABASE_RWLOCK_H
#define MYDATABASE_RWLOCK_H

#include <condition_variable>

class RWLock {
public:
    RWLock();
    void readLock();
    void writeLock();
    void readUnlock();
    void writeUnlock();

private:
    int readCount;
    int writeCount;
    std::mutex readCountMutex;          // readCount的锁
    std::mutex writeCountMutex;         // writeCount的锁
    std::mutex readMutex;
    std::mutex writeMutex;
    std::condition_variable wq;     // 控制写者
    std::condition_variable rq;     // 控制读者
};

class ReadGuard {
public:
    ReadGuard(RWLock &rwLock);
    ~ReadGuard();
    void unlock();
    void lock();

public:
    ReadGuard& operator=(const ReadGuard&) = delete;
    ReadGuard(const ReadGuard&) = delete;

private:
    RWLock &rwLock;
    int myOwns;
};

class WriteGuard {
public:
    WriteGuard(RWLock &rwLock);
    ~WriteGuard();
    void unlock();
    void lock();

public:
    WriteGuard& operator=(const WriteGuard&) = delete;
    WriteGuard(const WriteGuard&) = delete;

private:
    RWLock &rwLock;
    int myOwns;
};

#endif //MYDATABASE_RWLOCK_H
