//
// Created by Administrator on 2020/11/6.
//

#ifndef GLOBAL_H
#define GLOBAL_H

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

#define MAXNAME         24          // 表示一个表或一个属性名的最大长度

#define MAXSTRINGLEN    255         // 字符串类型属性的最大长度值


typedef int RC;

#define OK_RC  0

// 属性类型
enum AttrType {
    INT,
    FLOAT,
    STRING,
    VARCHAR
};

// 比较符, 表示无比较, 大于, 小于等
enum CompOp {
    NO_OP,                                      // no comparison
    EQ_OP, NE_OP, LT_OP, GT_OP, LE_OP, GE_OP    // binary atomic operators
};

//
enum ClientHint {
    NO_HINT                                     // default value
};


#define START_PF_ERR    -1
#define START_RM_ERR    -101
#define END_RM_ERR      (-200)
#define START_DDL_ERR    (-301)
#define END_DDL_ERR      (-400)
#define START_DML_ERR    (-401)
#define END_DML_ERR      (-500)

#define START_PF_WARN   1
#define END_PF_WARN     100
#define START_RM_WARN   101
#define END_RM_WARN     200
#define START_DDL_WARN   301
#define END_DDL_WARN     400
#define START_DML_WARN   401
#define END_DML_WARN     500

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#endif
