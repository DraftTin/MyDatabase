//
// Created by Administrator on 2020/11/12.
//
// 描述: parser从用户处获取命令，对其进行解析，然后调用SM或QL组件方法来执行该命令
#ifndef MYDATABASE_PARSER_H
#define MYDATABASE_PARSER_H

#include <iostream>

using namespace std;

// 属性信息
struct AttrInfo {
    char *attrName;     // 属性名
    AttrType attrType;  // 属性类型
    int attrLength;     // 属性的长度
};

// 属性值
struct Value {
    void *value;
    AttrType type;

    // 输出自身信息
    friend std::ostream &operator<<(std::ostream &s, const Value &value) {
        // 修改bug: 强转后输出字符串不需要再取地址
        switch (value.type) {
            case INT:
                s << "int: " << *static_cast<int*>(value.value) << '\n';
                break;
            case FLOAT:
                s << "float: " << *static_cast<float *>(value.value) << '\n';
                break;
            case STRING:
                s << "string: " << static_cast<char*>(value.value) << '\n';
                break;
            case VARCHAR:
                s << "varchar: " << (char*)value.value << '\n';
                break;
            default:
                s << "unknown type" << '\n';
        }
        return s;
    }
};

struct RelAttr {
    char *relName;     // relation name (may be NULL)
    char *attrName;    // attribute name
    friend ostream &operator<<(ostream &s, const RelAttr &ra) {
        return s << "relname: " << ra.relName << "\n"
                 << "attrname: "<< ra.attrName << "\n";
    }
};

struct Condition {
    RelAttr lhsAttr;      // left-hand side
    CompOp  op;           // comparison operator
    int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
                          //   and not a value
    RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
    Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
    friend ostream &operator<<(ostream &s, const Condition &c) {
        if(c.bRhsIsAttr) {
            return s << c.lhsAttr << "\n"
                     << c.op << "\n"
                     << c.rhsAttr << "\n";
        }
        else {
            return s << c.lhsAttr << "\n"
                     << c.op << "\n"
                     << c.rhsValue << "\n";
        }
    }
};

#endif //MYDATABASE_PARSER_H
