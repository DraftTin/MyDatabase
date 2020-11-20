//
// Created by Administrator on 2020/11/17.
//

#ifndef MYDATABASE_RM_BITMAP_H
#define MYDATABASE_RM_BITMAP_H
#include "../global.h"

class Bitmap {
public:
    // 计算bitmapz中第一个bit为0的slotNum并返回
    static int calcFirstZeroBit(char *bitmap, int bitmapSize);
    // 将bitNum位设置为1
    static RC setBit(char *bitmap, int bitNum);
    // 将bitNum位设置为0
    static RC unsetBit(char *bitmap, int bitNum);
    // 判断修改过后的bitmap, 页是否已经满了, numRecords表示bitmap能容纳的记录的数量(bitmapSize会有剩余)
    static int isBitmapFull(char *bitmap, int numRecords);
};

#endif //MYDATABASE_RM_BITMAP_H
