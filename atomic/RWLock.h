//
// Created by Administrator on 2020/11/27.
//

#ifndef MYDATABASE_RWLOCK_H
#define MYDATABASE_RWLOCK_H

#include <condition_variable>
#include <atomic>

// RWLock: 读写锁, 用于对操作进行并发控制
class RWLock {
public:
    RWLock();
    void readLock();
    void writeLock();
    void readUnlock();
    void writeUnlock();

private:
    std::atomic_int readCount;
    std::atomic_int writeCount;
    std::mutex rwMutex;             // 并发控制锁
    std::condition_variable wq;     // 控制写者
    std::condition_variable rq;     // 控制读者
};

// ReadGuard: 用于管理读锁
class ReadGuard {
public:
    ReadGuard(RWLock &rwLock);
    ReadGuard();
    ~ReadGuard();
    void unlock();
    void lock();
    ReadGuard& operator=(ReadGuard&& readGuard) noexcept;

public:
    ReadGuard& operator=(const ReadGuard&) = delete;
    ReadGuard(const ReadGuard&) = delete;

private:
    RWLock *rwLock;
    int myOwns;
};

// WriteGuard: 用于管理写锁
class WriteGuard {
public:
    explicit WriteGuard(RWLock& rwLock);
    WriteGuard();
    ~WriteGuard();
    void unlock();
    void lock();
    WriteGuard& operator=(WriteGuard&& writeGuard) noexcept;

public:
    WriteGuard& operator=(const WriteGuard&) = delete;
    WriteGuard(const WriteGuard&) = delete;

private:
    RWLock *rwLock;
    int myOwns;
};

#endif //MYDATABASE_RWLOCK_H
