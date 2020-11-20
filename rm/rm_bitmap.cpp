//
// Created by Administrator on 2020/11/17.
//
#include "rm_bitmap.h"
#include "rm.h"

// calcFirstZeroBit: 计算该bitmap的第一个值为0的bit位, 插入记录的时候会用到
// - 由于char类型的长度为1byte即8bits, 对于char*的j位byte, currentByte | (0x80 >> j) != currentByte来判断第j位bit是否为0
int Bitmap::calcFirstZeroBit(char *bitmap, int bitmapSize) {
    for (int i = 0; i < bitmapSize; i++) {
        char currentByte = bitmap[i];
        for (int j = 0; j < 8; j++) {
            // 检查对应位是否为0, 若为0则返回对应的位
            if ((currentByte | (0x80 >> j)) != currentByte) {
                return i * 8 + j + 1;
            }
        }
    }
    // 返回error
    return RM_INCONSISTENT_BITMAP;
}

// 将第bitNumbit位设置为1, 表示被使用
RC Bitmap::setBit(char *bitmap, int bitNum) {
    bitNum--;
    int byteNum = bitNum / 8;
    char currentByte = bitmap[byteNum];
    int bitOffset = bitNum - (byteNum * 8);

    // 对应的bit已经为1了
    if((currentByte | (0x80 >> bitOffset)) == currentByte) {
        return RM_INCONSISTENT_BITMAP;
    }
    currentByte |= (0x80 >> bitOffset);
    bitmap[byteNum] = currentByte;
    return 0;       // ok
}

// 判断bitmap是否满了, numberRecords为bitmap实际使用的bit数
int Bitmap::isBitmapFull(char *bitmap, int numberRecords) {
    int count = 0;
    int bitNumber = 0;
    int byteNumber = 0;
    char currentByte = bitmap[byteNumber];
    // 按位检查
    while(count < numberRecords) {
        if((currentByte | (0x80 >> bitNumber)) != currentByte) {
            return FALSE;
        }
        ++count;
        ++bitNumber;
        if(bitNumber == 8) {
            // 按字节检查
            byteNumber++;
            bitNumber = 0;
            currentByte = bitmap[byteNumber];
        }
    }
    return TRUE;
}

//  unsetBit: 将bitmap的bitNum位由1设置为0
RC Bitmap::unsetBit(char *bitmap, int bitNum) {
    // Change bit number to start from 0
    bitNum--;

    // Calculate the byte number (start from 0)
    int byteNumber = bitNum/8;
    char currentByte = bitmap[byteNumber];

    // Calculate the bit offset in the current byte
    int bitOffset = bitNum - (byteNumber*8);

    // Return error if the bit is not 1
    if ((currentByte | (0x80 >> bitOffset)) != currentByte) {
        return RM_INCONSISTENT_BITMAP;
    }

    // Set the bit to 0
    currentByte &= ~(0x80 >> bitOffset);
    bitmap[byteNumber] = currentByte;

    // Return OK
    return OK_RC;
}