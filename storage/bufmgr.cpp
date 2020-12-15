//
// Created by Administrator on 2020/11/6.
//
#ifndef _UNISTD_H
#define _UNISTD_H
#include <io.h>
#include <process.h>
#endif
#include <iostream>
#include "bufmgr.h"

using namespace std;

// 暂存页的“文件描述符”
#define MEMORY_FD -1

// BufferMgr: 输入_numPages, 表示创建_numPages容量的缓冲池
// - 申请_numPages容量的缓冲池空间
// - 初始化缓冲页的size为pageSize = PF页头 + PF_PAGE_SIZE = 4096, 和文件页大小相同, 并将每个缓冲页的初始next和prev指针都设置成后一页和前一页
// - free空闲块号链的head, 初始化为缓冲区的第一页0
// - first被申请页链表的head(mru页), last被申请页链表的tail(lru页), 初始化为INVALID_SLOT表示不存在
BufferMgr::BufferMgr(int _numPages) : hashTable(PF_HASH_TBL_SIZE) {
    this->numPages = _numPages;
    // 一个页的大小是页内容 + 页头
    pageSize = PF_PAGE_SIZE + sizeof(PF_PageHdr);
    // 申请numPages页缓冲区
    bufTable = new BufPageDesc[numPages];

    for(int i = 0; i < numPages; ++i) {
        if((bufTable[i].pData = new char[pageSize]) == nullptr) {
            cerr << "Not enough memory for buffer\n";   // 输出错误信息
            exit(1);
        }

        memset((char *)bufTable[i].pData, 0, pageSize);

        // 设置缓冲区链表的相对位置，使用数组表示链表
        bufTable[i].prev = i - 1;
        bufTable[i].next = i + 1;
    }
    // 统一用INVALID_SLOT表示，方便操作
    bufTable[0].prev = bufTable[numPages - 1].next = INVALID_SLOT;
    freeHead = 0;
    usedHead = usedTail = INVALID_SLOT;    // 当前还未申请任何页，都设置成INVALID_SLOT
}

// 释放缓冲池空间
BufferMgr::~BufferMgr() {
    // 先把每个缓冲区页中保存的字符串内容删除
    for(int i = 0; i < this->numPages; ++i) {
        delete [] bufTable[i].pData;
    }
    delete [] bufTable;
}

// getPage: 获取文件fd的pageNum页并返回该页的数据部分, bMultiplePins表示此次申请是否想要固定该页
// - 查看该页是否在缓冲区内
// - 若不在缓冲区内, 则向缓冲池申请一页空闲缓冲页, 并将文件页读入该缓冲页, 向哈希表中插入映射关系, 初始化该缓冲页的fd, nextPage, pinCount和bDirty
// - 若在缓冲区内, 则直接返回该页, ++pinCount, 注: bMultiplePins表示一个标记, 如果bMultiplePins为FALSE而pinCount > 0则是一个错误(如disposePage时)
// - 返回文件页在缓冲区页的指针
RC BufferMgr::getPage(int fd, PageNum pageNum, char **ppBuffer, int bMultiplePins) {
    RC rc;      // return code
    int slot;   // 指定缓冲区中页的下标

    if((rc = hashTable.find(fd, pageNum, slot)) &&
            (rc != PF_HASHNOTFOUND)) {
        return rc;      // 返回0表示成功，如果不是PF_HASHNOTFOUND的话，则表示unexpected error
    }
    if (rc == PF_HASHNOTFOUND) {
        // 如果缓冲区中没找到该页则申请一个空页， 如果有空闲页直接分配，如果没有用替换算法淘汰一个页之后分配
        if((rc = bufferAlloc(slot))) {
            return rc;
        }

        // 如果缓冲区中一开始没有该页，申请到空页后，将该页的内容读入到申请到的缓冲页中并将该页插入到哈希表中
        if((rc = readPage(fd, pageNum, bufTable[slot].pData)) ||
                (rc = hashTable.insert(fd, pageNum, slot)) ||
                (rc = initPageDesc(fd, pageNum, slot))) {
            unlink(slot, true);
            insertFree(slot, true);
            return rc;
        }
    }
    else {
        // bMultiplePins控制读取页是否会pin该页，如果不想pin且该页的pinCount > 0则返回错误
        if(!bMultiplePins && bufTable[slot].pinCount > 0) {
            return PF_PAGEPINNED;
        }
        // 当前页在内存，增加pinCount
        ++bufTable[slot].pinCount;
        // 将该页放到used list的头部
        if((rc = unlink(slot, true)) ||
           (rc = linkHead(slot, true))) {
            return rc;
        }
    }
    *ppBuffer = bufTable[slot].pData;
    return 0;
}

// bufferAlloc: 向缓冲池申请一页空闲的缓冲页, 用于放入文件页, 并返回申请到的缓冲页的下标
// - 如果存在空闲区, 直接分配(freeHead !- INVALID_SLOT)
// - 如果不存在空闲区, 则使用LRU算法替换last页
    // - 判断last页是否是脏页, 如果是, 则写回文件
    // - 从哈希表中移除对应的映射关系
// - 将first的值置为该缓冲页(置为mru)
RC BufferMgr::bufferAlloc(int &slot) {
    RC rc;          // return code

    // 加读锁
    ReadGuard readGuard(listLock);
    // 如果存在空闲区，直接分配
    if(freeHead != INVALID_SLOT) {
        readGuard.unlock();
        WriteGuard writeGuard(listLock);
        // 判断是否还存在空闲页
        if(freeHead == INVALID_SLOT) {
            return bufferAlloc(slot);
        }
        slot = freeHead;
        freeHead = bufTable[freeHead].next;
    }
    // 不存在空闲区
    // 已经拥有读锁
    else {
        // 用LRU算法替换未被固定的页面
        for(slot = usedTail; slot != INVALID_SLOT; slot = bufTable[slot].prev) {
            if(bufTable[slot].pinCount == 0) break;
        }
        // 读取链表后释放锁
        readGuard.unlock();

        if(slot == INVALID_SLOT) {
            return PF_NOBUF;
        }
        // 脏页则写回
        if(bufTable[slot].bDirty) {
            // 将缓冲区的内容写回到文件
            if((rc = writePage(bufTable[slot].fd, bufTable[slot].pageNum,
                               bufTable[slot].pData))) {
                return rc;
            }
            bufTable[slot].bDirty = FALSE;  // 写回后，将页的置为非脏页
        }
        // 从hashTable和bufferTable的used队列中移除
        if((rc = hashTable.remove(bufTable[slot].fd, bufTable[slot].pageNum)) ||
                (rc = unlink(slot, true))) {
            return rc;
        }
    }
    // 申请之后，将申请的slot放到栈顶(LRU算法)
    if((rc = linkHead(slot, true))) return rc;
    return 0;
}

// writePage: 将source的内容写入到文件中
// - 计算该页在文件中的偏移offset
// - 移动文件句柄的指针到文件内偏移处
// - 写入数据
RC BufferMgr::writePage(int fd, PageNum pageNum, char *source) {
    long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
    // 将fd定位到offset位置
    if(lseek(fd, offset, L_SET) < 0) {
        return PF_UNIX;
    }
    // 读取数据
    int numBytes = write(fd, source, pageSize);
    if(numBytes < 0) return PF_UNIX;
    else if(numBytes != pageSize) return PF_INCOMPLETEWRITE;
    else return 0;  // ok
}

// 从used list中移除bufTable[slot]
// unlink: 从used list中移除bufTable[slot]
// 输入: slot - 缓冲页的下标    blck - 是否需要单独对链表加锁
// 输出: RC码
// - 如果需要, 对链表加写锁
// - 判断该页是否是first或last, 若是, 则first和last指向对应的下一个位置和前一个位置
// - 修改链表对应的prev和next
RC BufferMgr::unlink(int slot, bool blck) {
    WriteGuard writeGuard;
    // 如果需要, 对链表加锁
    if(blck) {
        writeGuard = WriteGuard(listLock);
    }
    // 是MRU页
    if(usedHead == slot) {
        usedHead = bufTable[slot].next;
    }
    // 是LRU页
    if(usedTail == slot) {
        usedTail = bufTable[slot].prev;
    }
    // 如果不是最后一页
    if(bufTable[slot].next != INVALID_SLOT) {
        bufTable[bufTable[slot].next].prev = bufTable[slot].prev;
    }
    // 如果不是第一页
    if(bufTable[slot].prev != INVALID_SLOT) {
        bufTable[bufTable[slot].prev].next = bufTable[slot].next;
    }

    bufTable[slot].prev = bufTable[slot].next = INVALID_SLOT;
    return 0;   // ok
}

// linkHead: 将slot页置为first(mru页), 即放到栈顶
// 输入: slot - 缓冲页的下标    blck - 是否需要单独对链表加锁
// 输出: RC码
// - 如果需要, 对链表加写锁
// - 修改链表指针
RC BufferMgr::linkHead(int slot, bool blck) {
    WriteGuard writeGuard;
    // 如果需要, 对链表加锁
    if(blck) {
        writeGuard = WriteGuard(listLock);
    }
    bufTable[slot].next = usedHead;
    bufTable[slot].prev = INVALID_SLOT;

    if(usedHead != INVALID_SLOT) {
        bufTable[usedHead].prev = slot;
    }
    usedHead = slot;
    // 若list为空, 初始化last
    if(usedTail == INVALID_SLOT) {
        usedTail = slot;
    }
    return 0;
}

// readPage: 将文件fd的pageNum页内容读取到dest中并返回, 用于将页的内容读入缓冲区的时候
// - 计算pageNum页在文件中的偏移
// - 将该页的内容读取到dest中并返回
RC BufferMgr::readPage(int fd, PageNum pageNum, char *dest) {
    // readPage跳过文件头 PF_FILE_HDR_SIZE, 如果pageNum为0则读取的是file usedHead page
    long offset = pageNum * (long)pageSize + PF_FILE_HDR_SIZE;
    if(lseek(fd, offset, L_SET) < 0) {
        return PF_UNIX;
    }
    int numBytes = read(fd, dest, pageSize);
    if(numBytes < 0) {
        return PF_UNIX;
    }
    if(numBytes != pageSize) {
        return PF_INCOMPLETEREAD;
    }
    return 0;   // ok
}

// initPageDesc: 初始化bufTable[slot]缓冲页, 设置fd, nextPage, bDirty, pinCount
RC BufferMgr::initPageDesc(int fd, PageNum pageNum, int slot) {
    bufTable[slot].fd = fd;
    bufTable[slot].pageNum = pageNum;
    bufTable[slot].bDirty = FALSE;
    bufTable[slot].pinCount = 1;
    return 0;
}

// insertFree: 将slot缓冲页插入到空闲区链表首部
// 输入: slot - 缓冲页的下标    blck - 是否需要单独对链表加锁
// 输出: RC码
// - 如果需要, 加写锁
// - 修改链表指针
RC BufferMgr::insertFree(int slot, bool blck) {
    WriteGuard writeGuard;
    // 如果需要, 对链表加锁
    if(blck) {
        writeGuard = WriteGuard(listLock);
    }
    bufTable[slot].next = freeHead;
    bufTable[slot].prev = INVALID_SLOT;
    freeHead = slot;
    return 0;   // ok
}

// allocatePage: 向缓冲区申请一个空闲缓冲页, 用于文件申请新页时(新页一定不在缓冲区, 所以直接internalAlloc)
// - 向缓冲区申请一页空闲缓冲页
// - 向哈希表中插入(fd, nextPage) -> slotNum 的映射, 初始化该缓冲页
// - 将ppBuffer指向该缓冲页并返回
RC BufferMgr::allocatePage(int fd, PageNum pageNum, char **ppBuffer) {
    RC rc;
    int slot;
    // 如果已经在缓冲区了，则返回error
    if(!(rc = hashTable.find(fd, pageNum, slot))) {
        return PF_PAGEINBUF;
    }
    else if(rc != PF_HASHNOTFOUND) {
        return rc;      // 未知的错误
    }

    // 申请一个空闲页，如果申请失败则返回error
    if((rc = bufferAlloc(slot))) {
        return rc;
    }

    // 将申请的页插入到hashTable中
    // 初始化申请的页, 将pinCount置为1，表示正在使用
    if((rc = hashTable.insert(fd, pageNum, slot)) ||
            (rc = initPageDesc(fd, pageNum, slot))) {
        unlink(slot, true);
        insertFree(slot, true);
        return rc;
    }

    // 刚开始申请到的页没有任何内容，用指针指向该区域
    *ppBuffer = bufTable[slot].pData;
    return 0;
}


RC BufferMgr::getBlockSize(int &length) const {
    length = pageSize;
    return 0;   // ok
}

// printBuffer: 输出缓冲池的信息
RC BufferMgr::printBuffer() const {
    cout << "Buffer contains " << numPages << " pages of size "
         << pageSize <<".\n";
    cout << "Contents in order from most recently used to "
         << "least recently used.\n";

    int slot, next;
    slot = usedHead;
    while (slot != INVALID_SLOT) {
        next = bufTable[slot].next;
        cout << slot << " :: \n";
        cout << "  fd = " << bufTable[slot].fd << "\n";
        cout << "  pageNum = " << bufTable[slot].pageNum << "\n";
        cout << "  bDirty = " << bufTable[slot].bDirty << "\n";
        cout << "  pinCount = " << bufTable[slot].pinCount << "\n";
        slot = next;
    }

    if (usedHead == INVALID_SLOT)
        cout << "Buffer is empty!\n";
    else
        cout << "All remaining slots are freeHead.\n";

    return 0;
}

// TODO
//  unpinPage: 将fd文件的pageNum页的pinCount减一
//  - 判断能够unpin(是否存在于缓冲区, 是否已经是unpinned状态)
//  - 如果取消固定后没有进程使用该页，则将该页放到队尾, 防止pinCount为0, 之后在申请缓冲页的时候直接被换出缓冲区, 导致页的换入换出频繁
RC BufferMgr::unpinPage(int fd, PageNum pageNum) {
    RC rc;
    int slot;

    // 如果哈希表中没有则返回警告
    if((rc = hashTable.find(fd, pageNum, slot))) {
        if(rc == PF_HASHNOTFOUND) {
            return PF_PAGENOTINBUF;
        }
        else {
            return fd;
        }
    }
    // 该页已经unpinned
    if(bufTable[slot].pinCount == 0) {
        return PF_PAGEUNPINNED;
    }
    // 如果取消固定后没有进程使用该页，则将该页放到队尾
    if(--(bufTable[slot].pinCount) == 0) {
        if((rc = unlink(slot, true)) ||
           (rc = linkHead(slot, true))) {
            return rc;
        }
    }
    return 0;       // ok
}

// flushPages: 刷新页面，将缓冲区中fd对应的所有页的脏页都写回的文件中，并且从缓冲区中移除
// - 加写锁
// - 从first开始刷新, 将脏页写回文件, 如果此时fd文件在缓冲区仍有固定的页面, 则在方法后会返回警告
RC BufferMgr::flushPages(int fd) {
    RC rc, rcWarn = 0;

    // 处理方法，从MRU页开始处理
    // 加写锁
    WriteGuard writeGuard(listLock);
    int slot = usedHead;

    while(slot != INVALID_SLOT) {
        // 保存该结点的下一个结点的位置
        int next = bufTable[slot].next;
        // 找到缓冲区中对应fd的脏页
        if(bufTable[slot].fd == fd) {
            // 确保该页unpinned
            if(bufTable[slot].pinCount) {
                rcWarn = PF_PAGEPINNED;
            }
            else {
                // 找到了缓冲区中对应的脏页则写回到文件中
                if(bufTable[slot].bDirty) {
                    // 写回到文件
                    if((rc = writePage(fd, bufTable[slot].pageNum, bufTable[slot].pData))) {
                        return rc;
                    }
                    bufTable[slot].bDirty = FALSE;
                }
                if((rc = hashTable.remove(fd, bufTable[slot].pageNum)) ||
                   (rc = unlink(slot, false)) ||
                   (rc = insertFree(slot, false))) {
                    return rc;
                }
            }
        }
        slot = next;
    }
    return rcWarn;
}

// markDirty: 将缓冲池中的(fd, nextPage)页标记为脏，不需要移除缓冲区, 并将该页放到used list的首部
RC BufferMgr::markDirty(int fd, PageNum pageNum) {
    int rc;
    int slot;
    if((rc = hashTable.find(fd, pageNum, slot))) {
        if(rc == PF_HASHNOTFOUND) {
            return PF_PAGENOTINBUF;
        }
        else {
            // 返回未定义的数
            return rc;
        }
    }
    // 如果未被pinned则不能操作该页
    if (bufTable[slot].pinCount == 0) {
        return PF_PAGEUNPINNED;
    }
    bufTable[slot].bDirty = TRUE;
    // 修改bug: 移除 (rc = hashTable.remove),
    // unlink: 从used list中移出，linkHead: 放入used list的first中，使其成为mru页
    if((rc = unlink(slot, true)) ||
       (rc = linkHead(slot, true))){
        return rc;
    }
    return 0;
}

// forcePages: 将fd文件的所有缓冲区的脏页写回磁盘, 不移除缓冲区
// - 对链表加读锁
// - 遍历used list链表, 将fd文件的所有脏页写回
RC BufferMgr::forcePages(int fd) {
    RC rc;
    ReadGuard readGuard(listLock);
    int slot = usedHead;
    while(slot != INVALID_SLOT) {
        int next = bufTable[slot].next;
        // 对该文件中的所有脏页进行写回
        if(fd == bufTable[slot].fd) {
            // 如果是脏页则写回磁盘
            if(bufTable[slot].bDirty) {
                if((rc = writePage(fd, bufTable[slot].pageNum, bufTable[slot].pData))) {
                    return rc;
                }
                bufTable[slot].bDirty = FALSE;
            }
        }
        slot = next;
    }
    return 0;
}

// forceSinglePage: 将fd文件的pageNum页内容写回磁盘, 不移除缓冲区
// - 对链表加读锁
// - 将fd文件的pageNum页写回磁盘
RC BufferMgr::forceSinglePage(int fd, int pageNum) {
    RC rc;
    ReadGuard readGuard(listLock);
    int slot = usedHead;
    while(slot != INVALID_SLOT) {
        int next = bufTable[slot].next;
        // 仅写回一页
        if(bufTable[slot].pageNum == pageNum) {
            if(fd == bufTable[slot].fd) {
                // 如果是脏页则写回磁盘
                if(bufTable[slot].bDirty) {
                    if((rc = writePage(fd, bufTable[slot].pageNum, bufTable[slot].pData))) {
                        return rc;
                    }
                    bufTable[slot].bDirty = FALSE;
                }
                break;
            }
            // 若该页不是脏页, 则直接退出
            else {
                break;
            }
        }
        slot = next;
    }
    return 0;
}


