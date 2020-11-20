//
// Created by Administrator on 2020/11/11.
//

#include <iostream>
#include "rm.h"
#include "rm_internal.h"

using namespace std;

RM_FileScan::RM_FileScan() {
    bScanOpen = FALSE;
}

RM_FileScan::~RM_FileScan() {
    // don't need to do anything
}

// TODO
//  openScan: 初始化一个file scan实例
//  - 输入对应的fileHandle(表文件), attrType(扫描比较的属性的类型), attrLength(扫描比较的属性长度bytes数), (属性在一条记录中的offset), CompOp(比较符), value(比较的值)
//  - 获取文件的第一页存在记录的页(file hdr page的nextPage), 将pageNum设置为file hdr page的nextPage(pageNum表示当前扫描到的记录页号)
//  - 将类中的fileHandle和attrType...初始化, 将当前扫描的slotNum置为1
RC RM_FileScan::openScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset,
                         CompOp compOp, void *value) {
    // 检查是否有错误的输入
    if(attrType != INT && attrType != FLOAT && attrType != STRING && attrType != VARCHAR) {
        return RM_INVALID_ATTRIBUTE;
    }
    // 修改bug, fileHandle.bFileOpen -> !fileHandle.bFileOpen
    if(!fileHandle.bFileOpen) {
        return RM_CLOSEDFILE;
    }

    int recordSize = (fileHandle.hdrPage).recordSize;

    if(attrOffset > recordSize || attrOffset < 0) {
        return RM_INVALID_OFFSET;
    }
    // 检查查询符号是否存在
    if (compOp != NO_OP && compOp != EQ_OP && compOp != NE_OP && compOp != LT_OP &&
        compOp != GT_OP && compOp != LE_OP && compOp != GE_OP) {
        return RM_INVALID_OPERATOR;
    }
    // 检查类型的长度是否正确
    if((attrType == INT || attrType == FLOAT) && attrLength != 4) {
        return RM_ATTRIBUTE_NOT_CONSISTENT;
    }

    if(compOp != NO_OP && value == nullptr) {
        compOp = NO_OP;
    }
    // 将类的信息存储
    this->rmFileHandle = fileHandle;
    this->attrType = attrType;
    this->attrLength = attrLength;
    this->attrOffset = attrOffset;
    this->compOp = compOp;
    this->value = value;
    // 设置scanOpen TRUE
    bScanOpen = TRUE;
    int rc;

    PF_FileHandle pfFH = fileHandle.fileHandle;
    PF_PageHandle pfPH;

    if((rc = pfFH.getFirstPage(pfPH))) {
        return rc;
    }
    PageNum headerPageNumber;
    PageNum pageNumber;
    int pageFound = TRUE;
    // 获取第一个数据页
    if((rc = pfPH.getPageNum(headerPageNumber)) ||
            (rc = pfFH.getNextPage(headerPageNumber, pfPH))) {
        // 如果没有数据页
        if(rc == PF_EOF) {
            pageNumber = RM_NO_FREE_PAGE;
            pageFound = FALSE;
        }
        else {
            return rc;
        }
    }
    if(pageFound) {
        if((rc = pfPH.getPageNum(pageNumber))) {
            return rc;
        }
    }
    this->pageNum = pageNumber;
    this->slotNum = 1;
    // 将file hdr page取消固定
    if((rc = pfFH.unpinPage(headerPageNumber))) {
        return rc;
    }
    if(pageFound) {
        if((rc = pfFH.unpinPage(pageNumber))) {
            return rc;
        }
    }
    return 0;   // ok
}

// isBitFilled: 判断bitmap的bitNumber位是否被占用
bool RM_FileScan::isBitFilled(char *bitmap, int bitNumber) {
    bitNumber--;
    int byteNumber = bitNumber / 8;
    char currentByte = bitmap[byteNumber];

    int bitOffset = bitNumber - (byteNumber * 8);

    return ((currentByte | (0x80 >> bitOffset)) == currentByte);
}

// TODO
//  closeScan: 关闭file scan
//  - 将bScanOpen置为FALSE
RC RM_FileScan::closeScan() {
    if (!bScanOpen) {
        return RM_SCAN_CLOSED;
    }
    bScanOpen = FALSE;
    return OK_RC;   // ok
}

// getIntegerValue: 获取recordData对应的整型值
int RM_FileScan::getIntegerValue(char *recordData) const {
    int recordValue;
    char *attrPointer = recordData + attrOffset;
    memcpy(&recordValue, attrPointer, sizeof(recordValue));
    return recordValue;
}

// getFloatValue: 获取recordData对应的float类型值
float RM_FileScan::getFloatValue(char *recordData) const {
    float recordValue;
    char* attrPointer = recordData + attrOffset;
    memcpy(&recordValue, attrPointer, sizeof(recordValue));
    return recordValue;
}

// getStringValue: 获取recordData对应的的string值
std::string RM_FileScan::getStringValue(char *recordData) const {
    string recordValue = "";
    char* attrPointer = recordData + attrOffset;
    for (int i=0; i<attrLength; i++) {
        recordValue += attrPointer[i];
    }
    return recordValue;
}

// matchRecord: 判断给定记录的属性值是否满足比较的条件
template<typename T>
bool RM_FileScan::matchRecord(T recordValue, T givenValue) {
    bool recordMatch = false;
    switch(compOp) {
        case EQ_OP:
            if (recordValue == givenValue) recordMatch = true;
            break;
        case LT_OP:
            if (recordValue < givenValue) recordMatch = true;
            break;
        case GT_OP:
            if (recordValue > givenValue) recordMatch = true;
            break;
        case LE_OP:
            if (recordValue <= givenValue) recordMatch = true;
            break;
        case GE_OP:
            if (recordValue >= givenValue) recordMatch = true;
            break;
        case NE_OP:
            if (recordValue != givenValue) recordMatch = true;
            break;
        default:
            recordMatch = true;
            break;
    }
    return recordMatch;
}

// TODO
//  getNextRec: 获取当前扫描的记录的下一条匹配的记录
//  - (pageNum, slotNum)为当前扫描的记录, 获取pageNum页的指针
//  - 从slotNum开始循环扫描, 若该bitmap对应slotNum的bit为1, 则进行比较, 若符合条件, 则设置返回的record
//  - 如果slotNum达到了页能够容纳的记录的数量, 则获取下一页并将slotNum置为1
//  - 直到找到符合条件的记录或到达文件尾没找到记录
RC RM_FileScan::getNextRec(RM_Record &rec) {
    if (!bScanOpen) {
        return RM_SCAN_CLOSED;
    }

    if (rec.isValid) {
        rec.isValid = FALSE;
        delete[] rec.pData;
    }

    int rc;

    PF_FileHandle pfFH = rmFileHandle.fileHandle;
    PF_PageHandle pfPH;
    char* pageData;
    char* bitmap;

    // 如果扫描页是链表的尾部, 返回
    if (pageNum == RM_NO_FREE_PAGE) {
        return RM_EOF;
    }

    if ((rc = pfFH.getThisPage(pageNum, pfPH))) {
        return rc;
    }
    if ((rc = pfPH.getData(pageData))) {
        return rc;
    }
    // 定位bitmap
    bitmap = pageData + sizeof(RM_PageHdr);

    bool recordMatch = false;

    while(!recordMatch) {
        // 检查slotNum号记录是否存在, 如果类型为varchar类型, 则需要进一步取值
        if (isBitFilled(bitmap, slotNum)) {
            int recordOffset = rmFileHandle.calcRecordOffset(slotNum);
            char* recordData = pageData + recordOffset;

            if (attrType == INT) {
                int recordValue = getIntegerValue(recordData);
                int givenValue = *static_cast<int*>(value);
                recordMatch = matchRecord(recordValue, givenValue);
            }
                // 如果属性是float
            else if (attrType == FLOAT) {
                float recordValue = getFloatValue(recordData);
                float givenValue = *static_cast<float*>(value);
                recordMatch = matchRecord(recordValue, givenValue);
            }
                // 如果属性是string
            else if (attrType == STRING) {
                string recordValue(recordData + attrOffset);
                char* givenValueChar = static_cast<char*>(value);
                string givenValue(givenValueChar);
                recordMatch = matchRecord(recordValue, givenValue);
            }
                // 如果属性是varchar类型
                // - 获取对应varchar属性值的pageNum和slotNum和varchar字段的长度
                // - 根据slotNum计算出varchar的偏移
                // - 根据varchar size从偏移处获取varchar属性值
            else if(attrType == VARCHAR) {
                RM_VarLenAttr *varLenAttr = (RM_VarLenAttr*)(recordData + attrOffset);
                int attrPageNum;
                int attrSlotNum;
                int blockInfoIndex;
                if((rc = varLenAttr->getPageNum(attrPageNum)) ||
                        (rc = varLenAttr->getSlotNum(attrSlotNum)) ||
                        (rc = varLenAttr->getBlockInfoIndex(blockInfoIndex))) {
                    return rc;
                }
                char *pVal;
                if((rc = rmFileHandle.getVarValue(*varLenAttr, pVal))) {
                    return rc;
                }
                // 获取该属性值
                string attrValue(pVal);
                cout << "attrValue: " << attrValue << endl;
                char *givenValueChar = static_cast<char*>(value);
                string givenValue(givenValueChar);
                recordMatch = matchRecord(attrValue, givenValue);
                // 修改bug: 去掉此处的unpinPage
//                if((rc = rmFileHandle.rmAttrFileHandle.fileHandle.unpinPage(attrPageNum))) {
//                    return rc;
//                }
            }

            if (recordMatch) {
                // 设置返回的record
                rec.isValid = TRUE;

                int recordSize = (rmFileHandle.hdrPage).recordSize;
                char* newPData = new char[recordSize];
                memcpy(newPData, recordData, recordSize);
                rec.pData = newPData;

                RID newRid(pageNum, slotNum);
                rec.rid = newRid;
                rec.recordSize = recordSize;
            }
        }
        // 如果到达了该页最后的记录, 则转到nextPage, 否则slotNum += 1
        if (slotNum == (rmFileHandle.hdrPage).numberRecords) {
            // 该页扫描完毕, 取消固定
            if ((rc = pfFH.unpinPage(pageNum))) {
                return rc;
            }

            // 获取nextPage
            rc = pfFH.getNextPage(pageNum, pfPH);
            if (rc == PF_EOF) {
                pageNum = RM_NO_FREE_PAGE;
                // 如果没有下一个数据页并且当前记录不匹配, 则返回RM_EOF
                if (recordMatch) return OK_RC;
                else return RM_EOF;
            }
            else if (rc) {
                return rc;
            }
            // 将pageNum设置为获取的页
            if ((rc = pfPH.getPageNum(pageNum))) {
                return rc;
            }

            // slotNum设置为1
            slotNum = 1;

            // 重新设置bitmap
            if ((rc = pfPH.getData(pageData))) {
                return rc;
            }
            bitmap = pageData + sizeof(RM_PageHdr);
        }
        else {
            slotNum++;
        }
    }

    if ((rc = pfFH.unpinPage(pageNum))) {
        return rc;
    }

    return OK_RC;   // ok
}
