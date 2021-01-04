//
// Created by Administrator on 2020/12/3.
//

#include <iostream>
#include <ctime>
#include "../storage/rm.h"
#include "../storage/pf.h"
#include "../sql/dml.h"
#include "../sql/ddl.h"
#include "../storage/rm_error.h"

using namespace std;

const int studentCount = 10000;
const int nameSize = MAXNAME;

struct Student {
    int id;             // INT
    char name[nameSize + 1]; // STRING
    float weight;       // FLOAT
    float height;       // FLOAT
};



Student students[studentCount];

RC Test1();
RC Test2();
RC InsertData(DML_Manager &dmlManager, char *relName);
RC VertifyData(DDL_Manager &ddlManager, RM_FileHandle &rmFileHandle, char *relName);
RC CreateTable(DDL_Manager &ddlManager, char *relName);
RC CreateDatabase();


int main() {
    srand(time(nullptr));
    int rc;
    if((rc = Test2())) {
        RM_PrintError(rc);
    }
    return 0;
}

bool compare(Student &student, int id) {
    return student.height != students[id].height ||
            student.weight != students[id].weight || strcmp(student.name, students[id].name);
}

// 随机生成size大小的字符串
void generateStr(char str[], int size) {
    for(int i = 0; i < size - 1; ++i) {
        str[i] = rand() % 95 + 32;
    }
    str[size - 1] = '\0';
}

RC CreateDatabase() {
    char command[30] = ".\\dbcreate testdb";
    system(command);
    return 0;
}

RC InsertData(DML_Manager &dmlManager, char *relName) {
    int rc;
    cout << "insert 10000 records..\n";
    for(int i = 0; i < studentCount; ++i) {
        students[i].id = i;
        generateStr(students[i].name, nameSize);
        students[i].weight = rand() % 249 + 1;
        students[i].height = rand() % 249 + 1;
        Value *values = new Value[4];
        values[0].value = (void*)&(students[i].id);
        values[0].type = INT;
        values[1].value = (void*)(students[i].name);
        values[1].type = STRING;
        values[2].value = (void*)&(students[i].weight);
        values[2].type = FLOAT;
        values[3].value = (void*)&(students[i].height);
        values[3].type = FLOAT;
        if(dmlManager.insert(relName, 4, values)) {
            delete [] values;
            return rc;
        }
    }
    cout << "Insert Data Successfully!\n";
    return 0;
}

// 验证使用DML插入的数据
RC VertifyData(DDL_Manager &ddlManager, RM_FileHandle &rmFileHandle, char *relName) {
    int rc;
    cout << "vertify data....\n";
    RM_FileScan rmScan;
    int num = 0;
    vector<string> attributeNames;
    if((rc = ddlManager.getAttributes(relName, attributeNames))) {
        return rc;
    }
    for(string &tmp : attributeNames) {
        cout << ":: " << tmp << "\n";
    }
    // 设置NO_OP
    if((rc = rmScan.openScan(rmFileHandle, INT, 4, 0, NO_OP, (void*)&num))) {
        return rc;
    }
    RelcatRecord relInfo;
    if((rc = ddlManager.getRelInfo(relName, relInfo))) {
        return rc;
    }
    AttrcatRecord *attrInfo = new AttrcatRecord[relInfo.attrCount];
    if((rc = ddlManager.getAttrInfo(relName, attrInfo))) {
        return rc;
    }
    RM_Record rec;
    // 扫描表中的数据，并和保存到数组中的数据做比较验证
    while((rc = rmScan.getNextRec(rec)) == 0) {
        char *pData;
        if((rc = rec.getData(pData))) {
            return rc;
        }
        Student student;
        student.id = *(int*)(pData + attrInfo[0].offset);
        int id = student.id;
        strcpy(student.name, pData + attrInfo[1].offset);
        student.weight = *(float*)(pData + attrInfo[2].offset);
        student.height = *(float*)(pData + attrInfo[3].offset);
        if(compare(student, id)) {
            cout << "inconsistent error\n";
            return 0;
        }
    }
    cout << "vertify data successfully!\n";
    return 0;   // ok
}

RC CreateTable(DDL_Manager &ddlManager, char *relName) {
    int rc;
    AttrInfo *attrs = new AttrInfo[4];
    attrs[0].attrLength = 4;
    attrs[0].attrType = INT;
    attrs[0].attrName = new char[MAXNAME + 1];
    sprintf(attrs[0].attrName, "%s", "id");
    attrs[1].attrLength = MAXNAME + 1;
    attrs[1].attrType = STRING;
    attrs[1].attrName = new char[MAXNAME + 1];
    sprintf(attrs[1].attrName, "%s", "name");
    attrs[2].attrLength = 4;
    attrs[2].attrType = FLOAT;
    attrs[2].attrName = new char[MAXNAME + 1];
    sprintf(attrs[2].attrName, "%s", "weight");
    attrs[3].attrLength = 4;
    attrs[3].attrType = FLOAT;
    attrs[3].attrName = new char[MAXNAME + 1];
    sprintf(attrs[3].attrName, "%s", "height");
    // 创建表
    return ddlManager.createTable(relName, 4, attrs);
}

// 测试创建表操作, 查看数据字典是否正确存储
RC Test1() {
    cout << "Test1 start....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    DDL_Manager ddlManager(rmManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";        // 创建数据库的名称
    char relName[MAXNAME + 1] = "student";      // 创建表的名称
    // 创建数据库
    if((rc = CreateDatabase())) {
        return rc;
    }
    // 打开数据库
    if((rc = ddlManager.openDb(dbName))) {
        return rc;
    }
    // 创建数据库表
    if((rc = CreateTable(ddlManager, relName))) {
        return rc;
    }
    // 输出数据字典信息
    if((rc = ddlManager.printDataDic())) {
        return rc;
    }
    // 关闭数据库
    if((rc = ddlManager.closeDb())) {
        return rc;
    }
    cout << "Test1 done!\n";
    return 0;   // ok
}

// Test2: 测试insert语句，并验证插入的正确性
RC Test2() {
    cout << "Test2 start....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    DDL_Manager ddlManager(rmManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";
    char relName[MAXNAME + 1] = "student";
    // 创建数据库
    if((rc = CreateDatabase())) {
        return rc;
    }
    // 打开数据库
    if((rc = ddlManager.openDb(dbName))) {
        return rc;
    }
    // 创建表
    if((rc = CreateTable(ddlManager, relName))) {
        return rc;
    }
    // 向表中插入数据
    if((rc = InsertData(dmlManager, relName))) {
        return rc;
    }
    // 输出数据字典
    if((rc = ddlManager.printDataDic())) {
        return rc;
    }
    cout << "****************************************\n";
    // 输出表中所有信息
    if((rc = ddlManager.printAllData(relName, 3))) {
        return rc;
    }
    // 验证表中的信息
    RM_FileHandle rmFileHandle;
    if((rc = rmManager.openFile(relName, rmFileHandle))) {
        return rc;
    }
    if((rc = VertifyData(ddlManager, rmFileHandle, relName))) {
        return rc;
    }
    if((rc = rmManager.closeFile(rmFileHandle))) {
        return rc;
    }
    // 需要手动删除生成的库文件
    cout << "Test2 done!\n";
    return 0;   // ok
}



