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

    friend std::ostream &operator<<(std::ostream &s, const Value &value) {
        switch (value.type) {
            case INT: 
                s << "int: " << *static_cast<int*>(value.value) << '\n';
                break;
            case FLOAT:
                s << "float: " << *static_cast<float *>(value.value) << '\n';
                break;
            case STRING:
                s << "string: " << *static_cast<char*>(value.value) << '\n';
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


#endif //MYDATABASE_PARSER_H
