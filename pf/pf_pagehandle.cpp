//
// Created by Administrator on 2020/11/8.
//
#include "pf.h"
#include "../global.h"

#define  INVALID_PAGE -1

PF_PageHandle::PF_PageHandle() : pageNum(INVALID_PAGE), pPageData(nullptr) {
    // do nothing
}

PF_PageHandle::~PF_PageHandle() {
    // do nothing
}

// getData: 返回页的指针
RC PF_PageHandle::getData(char *&pData) const {
    if(pPageData == nullptr) {
        return PF_PAGEUNPINNED;
    }
    pData = pPageData;
    return 0;
}

// 获取该页的页号
RC PF_PageHandle::getPageNum(PageNum &_pageNum) const {
    if(pPageData == nullptr) {
        return PF_PAGEUNPINNED;
    }
    _pageNum = this->pageNum;
    return 0;
}


