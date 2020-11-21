//
// Created by Administrator on 2020/11/6.
//

#ifndef PF_INTERNAL_H
#define PF_INTERNAL_H

#include <cstring>
#include "pf.h"

const int PF_BUFFER_SIZE = 40;          // 缓冲区的页数
const int PF_HASH_TBL_SIZE = 20;        // 哈希表的size

#define CREATION_MASK      0600         // 只有拥有者具有r/w权限
#define PF_PAGE_USED      -2            // 该页正在被使用

#ifndef L_SET
#define L_SET 0                         // 表示lseek从offset处开始读取
#endif

struct PF_PageHdr {
    // 有三种类型的值：下一页空闲页或表示该页非空闲页（PF_LIST_END），或表示该页正在被使用（文件中的页）（PF_PAGE_USED）
    int nextFree;
};



#define PF_PAGE_LIST_END -1

const int PF_FILE_HDR_SIZE = PF_PAGE_SIZE + sizeof(PF_PageHdr);

#endif
