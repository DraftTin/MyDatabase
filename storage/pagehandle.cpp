//
// Created by Administrator on 2020/11/8.
//
#include "pf.h"
#include "../global.h"

#define  INVALID_PAGE -1

PageHandle::PageHandle() : pageNum(INVALID_PAGE), pPageData(nullptr) {
    // do nothing
}

PageHandle::~PageHandle() {
    // do nothing
}

// getData: 返回页的指针
RC PageHandle::getData(char *&pData) const {
    if(pPageData == nullptr) {
        return PF_PAGEUNPINNED;
    }
    pData = pPageData;
    return 0;
}

// 获取该页的页号
RC PageHandle::getPageNum(PageNum &_pageNum) const {
    if(pPageData == nullptr) {
        return PF_PAGEUNPINNED;
    }
    _pageNum = this->pageNum;
    return 0;
}


