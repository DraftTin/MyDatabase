//
// Created by Administrator on 2020/12/11.
//

#include "io.h"
#include "../storage/pf.h"
#include "../storage/rm.h"
#include "../storage/ix.h"
#include "../sql/ddl.h"
#include "../sql/dml.h"
#include <iostream>

using namespace std;

const int nameSize = MAXNAME;
const int studentCount = 10000;     // 插入索引项的数量

struct Student {
    int id;             // INT
    char name[nameSize + 1]; // STRING
    float weight;       // FLOAT
    float height;       // FLOAT
};

Student students[studentCount];
RID rids[studentCount];

RC Test1();
RC Test2();
RC Test3();
RC Test4();
RC CreateTable(DDL_Manager &ddlManager, char *relName);
RC CreateDatabase();
RC InsertIndexEntry(IX_IndexHandle &indexHandle);
RC VertifyIndexEntry(IX_IndexHandle &indexHandle);
RC InsertTableItem(DML_Manager &dmlManager, IX_IndexHandle &indexHandle, char *relName);
RC VertifyItems(DDL_Manager &ddlManager, RM_FileHandle &rmFileHandle, IX_IndexHandle &indexHandle, char *relName);
RC DeleteIndexEntry(IX_IndexHandle &indexHandle);
void generateStr(char str[], int size);
bool compare(Student &student, int id);

int main() {
    int rc;
    if((rc = Test4())) {
        cout << "rc = " << rc << "\n";
    }
    return 0;
}

RC CreateDatabase() {
    char command[30] = ".\\dbcreate testdb";
    system(command);
    return 0;
}

RC DeleteDatabase() {
    chdir("..");
    char command[30] = "rd /s/q testdb";
    system(command);
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

// 创建relName表
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

// InsertIndexEntry: 插入索引
RC InsertIndexEntry(IX_IndexHandle &indexHandle) {
    int rc;
    // 插入若干条索引项
    for(int i = 0; i < studentCount; ++i) {
        RID rid(i, 2 * i);
        if((rc = indexHandle.insertEntry((void*)&i, rid))) {
            return rc;
        }
    }
    cout << "insert data(" << studentCount << ") successfully!\n";
    return 0;
}

// VertifyIndexEntry: 验证索引项
RC VertifyIndexEntry(IX_IndexHandle &indexHandle) {
    int rc;
    IX_IndexScan indexScan;
    if((rc = indexScan.openScan(indexHandle, NO_OP, (void*)&rc))) {
        return rc;
    }
    for(int i = 0; i < studentCount; ++i) {
        RID rid;
        if((rc = indexScan.getNextEntry(rid))) {
            return rc;
        }
        PageNum pageNum;
        SlotNum slotNum;
        if((rc = rid.getPageNum(pageNum)) ||
           (rc = rid.getSlotNum(slotNum))) {
            return rc;
        }
        cout << "i = " << i << " " << pageNum << " " << slotNum << "\n";
        if(pageNum != i || slotNum != 2 * i) {
            cout << "error: " << pageNum << "  " << slotNum << "  " << i << '\n';
            return 0;
        }
    }
    cout << "data vertified successfully!\n";
    return 0;
}

// InsertTableItem: 插入表项, 并将返回的rid作为索引val插入到索引中
RC InsertTableItem(DML_Manager &dmlManager, IX_IndexHandle &indexHandle, char *relName) {
    int rc;
    cout << "insert " << studentCount <<  " records..\n";
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
        RID rid;
        if((rc = dmlManager.insertForTest(relName, 4, values, rid))) {
            delete [] values;
            return rc;
        }
        // 将rid插入索引, 并将保存的索引记录用于删除
        if((rc = indexHandle.insertEntry((void*)&i, rid))) {
            return rc;
        }
        rids[i] = rid;
    }
    cout << "Insert Data and Index Successfully!\n";
    return 0;
}
// VertifyItems: 使用索引验证表项
RC VertifyItems(DDL_Manager &ddlManager, RM_FileHandle &rmFileHandle, IX_IndexHandle &indexHandle, char *relName) {
    int rc;
    cout << "vertify items....\n";
    RelcatRecord relInfo;
    if((rc = ddlManager.getRelInfo(relName, relInfo))) {
        return rc;
    }
    AttrcatRecord *attrInfo = new AttrcatRecord[relInfo.attrCount];
    if((rc = ddlManager.getAttrInfo(relName, attrInfo))) {
        return rc;
    }
    // 索引扫描法
    IX_IndexScan indexScan;
    if((rc = indexScan.openScan(indexHandle, NO_OP, (void*)&rc))) {
        return rc;
    }
    RID rid;
    RM_Record rec;
    int k = 0;
    // 遍历所有插入的索引, 根据返回的rid获取记录并验证记录
    while((rc = indexScan.getNextEntry(rid)) == 0) {
//        cout << "k = " << k << "\n";
        ++k;
        if((rc = rmFileHandle.getRec(rid, rec))) {
            return 0;
        }
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
        if(k % 50 == 0) {
            cout << "k: " << k << "\n"
                 << "id: " << student.id << "\n"
                 << "name: " << student.name << "\n"
                 << "height: " << student.height << "\n"
                 << "weight: " << student.weight << "\n";
        }
    }
    if(rc != IX_EOF) {
        return rc;
    }
    cout << "**********************************************************\n";
    cout << "Count of Vertified Data = " << k << "\n";
    if (k == studentCount) {
        cout << "Vertify Data Successfully!\n";
    }
    return 0;   // ok
}

// 删除indexHandle上一定数量的索引项
RC DeleteIndexEntry(IX_IndexHandle &indexHandle) {
    int rc;
    for(int i = 0; i < studentCount; ++i) {
        if((rc = indexHandle.deleteEntry((void*)&i, rids[i]))) {
            return rc;
        }
    }
    return 0;
}

// Test1: 简单的测试索引的创建, open, close
RC Test1() {
    cout << "Test1 starts....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    IX_Manager ixManager(pfManager);
    DDL_Manager ddlManager(rmManager, ixManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";
    char relName[MAXNAME + 1] = "student";
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
    // 输出表的字典信息
    if((rc = ddlManager.printDataDic())) {
        return rc;
    }
    // 在id上创建索引
    if((rc = ixManager.createIndex(relName, 0, INT, 4))) {
        return rc;
    }
    // 打开索引文件
    IX_IndexHandle indexHandle;
    if((rc = ixManager.openIndex(relName, 0, indexHandle))) {
        return rc;
    }
    // 关闭索引文件
    if((rc = ixManager.closeIndex(indexHandle))) {
        return rc;
    }

    // 检查创建索引文件是否正常进行
    cout << "Test1 done!\n";
    return 0;   // ok
}

// Test2: 测试索引项的插入
RC Test2() {
    cout << "Test2 starts....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    IX_Manager ixManager(pfManager);
    DDL_Manager ddlManager(rmManager, ixManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";
    char relName[MAXNAME + 1] = "student";
    int indexNo = 0;
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
    // 创建索引文件
    if((rc = ixManager.createIndex(relName, indexNo, INT, 4))) {
        return rc;
    }
    IX_IndexHandle indexHandle;
    if((rc = ixManager.openIndex(relName, indexNo, indexHandle))) {
        return rc;
    }
    // 插入索引项
    if((rc = InsertIndexEntry(indexHandle))) {
        return rc;
    }
    // 验证索引项
    if((rc = VertifyIndexEntry(indexHandle))) {
        return rc;
    }
    // 关闭索引
    if((rc = ixManager.closeIndex(indexHandle))) {
        return rc;
    }
    cout << "Test2 done!\n";
    return 0; // ok
}

// Test3: 测试在数据表上创建索引并进行插入
RC Test3() {
    cout << "Test3 starts....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    IX_Manager ixManager(pfManager);
    DDL_Manager ddlManager(rmManager, ixManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";
    char relName[MAXNAME + 1] = "student";
    int indexNo = 0;
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
    // 创建索引文件
    if((rc = ixManager.createIndex(relName, indexNo, INT, 4))) {
        return rc;
    }
    IX_IndexHandle indexHandle;
    if((rc = ixManager.openIndex(relName, indexNo, indexHandle))) {
        return rc;
    }
    // 插入表项并插入对应的索引
    if((rc = InsertTableItem(dmlManager, indexHandle, relName))) {
        return rc;
    }
    // 使用索引查找表项并进行验证
    RM_FileHandle rmFileHandle;
    if((rc = rmManager.openFile(relName, rmFileHandle))) {
        return rc;
    }
    if((rc = VertifyItems(ddlManager, rmFileHandle, indexHandle, relName))) {
        return rc;
    }
    // 关闭索引
    if((rc = ixManager.closeIndex(indexHandle))) {
        return rc;
    }
    cout << "Test3 done!\n";
    return 0; // ok
}

// 测试索引的删除
RC Test4() {
    cout << "Test4 starts....\n";
    int rc;
    PF_Manager pfManager;
    RM_Manager rmManager(pfManager);
    IX_Manager ixManager(pfManager);
    DDL_Manager ddlManager(rmManager, ixManager);
    DML_Manager dmlManager(rmManager, ddlManager);
    char dbName[MAXNAME + 1] = "testdb";
    char relName[MAXNAME + 1] = "student";
    int indexNo = 0;
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
    // 创建索引文件
    if((rc = ixManager.createIndex(relName, indexNo, INT, 4))) {
        return rc;
    }
    IX_IndexHandle indexHandle;
    if((rc = ixManager.openIndex(relName, indexNo, indexHandle))) {
        return rc;
    }
    // 插入表项并插入对应的索引
    if((rc = InsertTableItem(dmlManager, indexHandle, relName))) {
        return rc;
    }
    if((rc = DeleteIndexEntry(indexHandle))) {
        return rc;
    }
    // 使用索引查找表项并进行验证
    RM_FileHandle rmFileHandle;
    if((rc = rmManager.openFile(relName, rmFileHandle))) {
        return rc;
    }
    if((rc = VertifyItems(ddlManager, rmFileHandle, indexHandle, relName))) {
        return rc;
    }
    // 关闭索引
    if((rc = ixManager.closeIndex(indexHandle))) {
        return rc;
    }
    cout << "Test4 done!\n";
    return 0; // ok
}