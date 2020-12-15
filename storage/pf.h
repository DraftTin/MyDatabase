//
// Created by Administrator on 2020/11/6.
//

#ifndef PF_H
#define PF_H

#include "../global.h"

typedef int PageNum;

const int PF_PAGE_SIZE = 4092;                      // 4096 - sizeof(PF_PageHdr)
const int ALL_PAGES = -1;                           // 用于forcePages时判断是否要将文件fd的所有脏页都写回


// PF_FileHdr: 文件头
// 描述: 文件的创建会写入到文件头的部分，占用一页空间
struct PF_FileHdr {
    int firstFree;  // 文件中第一个空闲页
    int numPages;   // 文件申请的页数, 初始为0, 创建文件的时候会向文件中写入PF_FileHdr, 占用一页的空间, 不计入numPages内
};

// PageHandle
// 描述: 页内数据的Handle
class PageHandle {
    friend class PF_FileHandle;
public:
    PageHandle();
    ~PageHandle();
    RC getData(char *&pData) const;             // 不包括PF_PageHdr
    RC getPageNum(PageNum &_pageNum) const;     // 获取该页的页号
private:
    int pageNum;                // 该页的页号
    char *pPageData;            //指向页数据的指针
};


//
// PF_FileHandle: PF File的接口，对该文件做的所有操作都必须在bFileOpen为TURE的情况下进行
//                  该类的方法会调用pBufferMgr中对应的方法
class BufferMgr;
class PF_FileHandle {
    friend class PF_Manager;
public:
    PF_FileHandle();
    ~PF_FileHandle();
public:
    // 获取文件中第一个页面并且返回该页的pageHandle
    RC getFirstPage(PageHandle &pageHandle) const;
    // 获取current页号的下一页，因为页号未必连续
    RC getNextPage(PageNum current, PageHandle &pageHandle) const;
    // 获取当前页
    RC getThisPage(int pageNum, PageHandle &pageHandle) const;
    // 获取最后一页
    RC getLastPage(PageHandle &pageHandle) const;
    // 获取current页号的前一页
    RC getPrevPage(PageNum current, PageHandle &pageHandle) const;

    RC allocatePage(PageHandle &pageHandle);                    // 申请新页
    RC disposePage(PageNum pageNum);                            // free一页
    RC markDirty(PageNum pageNum);                              // 将pageNum标记为脏
    RC unpinPage(PageNum pageNum) const;                        // unpin the page
    RC flushPages() const;                                      // 将缓冲区的脏页写回文件中
    RC forcePages(PageNum pageNum = ALL_PAGES) const;           // 仅将缓冲区内容写回而不从缓冲区移除, 使用pageNum标记是force特定页还是所有页

    int isValidPageNum(PageNum pageNum) const;                  // 检测访问的页号是否合法

private:
    BufferMgr *pBufferMgr;                      // 缓冲区的指针
    PF_FileHdr hdr;                             // file header, 保存文件中第一个空闲页的页号和文件内的总页数
    int bFileOpen;                              // 查看文件是否打开
    int bHdrChanged;                            // 文件的dirty标志
    int unixfd;                                 // 文件描述符
};

// PF_FileHandle依赖于该类，调用该类的openFile方法将文件打开
// 页管理，所有对文件的创建，删除，打开，关闭最后都会由该类的一个实例执行
class PF_Manager {
public:
    PF_Manager();
    ~PF_Manager();
    // 创建文件, 并写入文件头信息
    RC createFile(const char *filename);
    RC destroyFile(const char *fileName);

    RC openFile(const char *fileName, PF_FileHandle &fileHandle);
    // 关闭和fileHandle相关联的文件
    RC closeFile(PF_FileHandle &fileHandle);
    // 由pBufferMgr执行
    RC printBuffer() const;
private:
    BufferMgr* pBufferMgr;
};


#define PF_PAGEPINNED       (START_PF_WARN + 0)     // 页是pinned的状态
#define PF_PAGENOTINBUF     (START_PF_WARN + 1)     // 该页不在缓冲区中
#define PF_INVALIDPAGE      (START_PF_WARN + 2)     // 不合法的页号
#define PF_FILEOPEN         (START_PF_WARN + 3)     // 文件已经打开
#define PF_CLOSEDFILE       (START_PF_WARN + 4)     // 文件已经关闭
#define PF_PAGEFREE         (START_PF_WARN + 5)     // 该页已经是空闲状态
#define PF_PAGEUNPINNED     (START_PF_WARN + 6)     // 该页已经unpinned, 不能再unpin了
#define PF_EOF              (START_PF_WARN + 7)     // 没有找到合法的页面
#define PF_TOOSMALL         (START_PF_WARN + 8)     // Resize buffer too small
#define PF_LASTWARN         PF_TOOSMALL

#define PF_NOMEM            (START_PF_ERR - 0)      // 没申请到内存空间
#define PF_NOBUF            (START_PF_ERR - 1)      // 缓冲区的页都被占用，不能申请
#define PF_INCOMPLETEREAD   (START_PF_ERR - 2)      // 对文件的不完全读
#define PF_INCOMPLETEWRITE  (START_PF_ERR - 3)      // 对文件的不完全写
#define PF_HDRREAD          (START_PF_ERR - 4)      // 对文件header的不完全读
#define PF_HDRWRITE         (START_PF_ERR - 5)      // 对文件header的不完全写
#define PF_PAGEINBUF        (START_PF_ERR - 6)      // 申请的页已经在缓冲区中了
#define PF_HASHNOTFOUND     (START_PF_ERR - 7)      // 没有找到对应的项
#define PF_HASHPAGEEXIST    (START_PF_ERR - 8)      // 已经在哈希表中存在
#define PF_UNIX             (START_PF_ERR - 10)     // 未知的错误
#define PF_LASTERROR        PF_UNIX

#endif //MYSQL_PF_H
