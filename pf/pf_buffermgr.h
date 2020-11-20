//
// Created by Administrator on 2020/11/6.
//

#ifndef PF_BUFFERMGR_H
#define PF_BUFFERMGR_H

#include "pf_internal.h"
#include "pf_hashtable.h"

#define INVALID_SLOT -1

// 缓存页
struct PF_BufPageDesc {
    char* pData;                // 缓冲区页的数据指针, 指针位置: pBuf + sizeof(PF_PageHdr), 即指针会指向RM_PageHdr的开头
    int next;                   // 缓冲区页链表中的next
    int prev;                   // 缓冲区页链表中的prev
    int bDirty;                 // 标记脏页，1/0
    short int pinCount;         // 当前使用该页的数量
    PageNum pageNum;            // 对应在fd文件中的页号，定位：pageNum * (long)pageSize + PF_FILE_HDR_SIZE
    int fd;                     // 该页对应的文件
};

// 用于缓冲区的管理
class PF_BufferMgr {
public:
    explicit PF_BufferMgr(int numPages);                         // 申请numPages个页的缓冲区
    ~PF_BufferMgr();
public:

    // 将fd和pageNum读取到缓冲池中并获取缓冲池的指针
    RC getPage(int fd, PageNum pageNum, char** ppBuffer,
               int bMultiplePins = TRUE);
    // 申请缓冲池中的一个新页，并用ppBuffer指向它
    RC allocatePage(int fd, PageNum pageNum, char** ppBuffer);
    // 移除缓冲区中所有的entries
    RC clearBuffer();
    // 输出缓冲区的所有页
    RC printBuffer() const;

    // 获取能够被申请的块的大小
    RC getBlockSize(int &length) const;

    // 将以也标记为dirty
    RC markDirty(int fd, PageNum pageNum);
    // unpin page from the buffer
    RC unpinPage(int fd, PageNum pageNum);
    // 释放fd文件在缓冲区中的所有未被使用的页
    RC flushPages(int fd);

    // 将文件中所有的脏页页写回文件，并且不从缓冲区移除这些页
    RC forcePages(int fd);
    // 将一页写回文件，并且不从缓冲区中移除
    RC forceSinglePage(int fd, int pageNum);
private:
    // 申请缓冲区的一个空闲页，如果没有则替换一页
    RC internalAlloc(int& slot);
    // 将页写回
    RC writePage(int fd, PageNum pageNum, char* source);
    // 将slot从bufTable中的used列表中移除
    RC unlink(int slot);
    // 将被申请的页放到栈顶/队列头（LRU算法）
    RC linkHead(int slot);
    // 读取页
    RC readPage(int fd, PageNum pageNum, char* dest);
    // 初始化缓冲区页的entry
    RC initPageDesc(int fd, PageNum pageNum, int slot);
    // 将slot插入到free list的头部
    RC insertFree(int slot);

private:
    PF_BufPageDesc* bufTable;   // 缓冲页s, 初始化的时候申请固定数量的缓冲页
    PF_HashTable hashTable;     // 缓冲区哈希表, (fd, pageNum) -> slotNum, slotNum表示bufTable的下标
    int numPages;               // 缓冲区的块数
    int pageSize;               // 缓冲区的块size
    int first;                  // MRU page slot，也是used list的第一个页
    int last;                   // LRU page slot，也是used list的最后一页
    int free;                   // 空闲链表的头
};

#endif
