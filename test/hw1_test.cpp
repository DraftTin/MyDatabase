//
// Created by Administrator on 2020/12/1.
//

#include <iostream>
#include <cmath>
#include <ctime>
#include "../storage/pf.h"
#include "../storage/rm.h"
#include "../storage/rm_error.h"
#include "../storage/pf_error.h"
#include "../error_message/all_error.h"

using namespace std;

const int nameSize = 40;
const int studentNumberSize = 40;
const int studentCount = 10000;

struct Student {
    int id;
    char name[nameSize];
    char studentNumber[studentNumberSize];
};

struct Student2 {
    int id;
    char name[nameSize];
    RM_VarLenAttr varLenAttr;
};

PF_Manager pfManager;
RM_Manager rmManager(pfManager);

Student students[studentCount];
Student2 students2[studentCount];
char varStudentNums[studentCount][4000];


RC Test1();
RC Test2();
RC InsertData(RM_FileHandle& rmFileHandle);
RC InsertData2(RM_FileHandle& rmFileHandle);
RC VertifyData(RM_FileHandle& rmFileHandle);
void generateStr(char str[], int size);
int compare(Student& student, int id);

int main() {
    int rc;
    srand(time(nullptr));
    if((rc = Test2()) != 0) {
        PrintError(rc);
    }
}

// 比较student和students[id]是否相同, 用于验证
int compare(Student& student, int id) {
    if(student.id != students[id].id || strcmp(student.name, students[id].name) || strcmp(student.studentNumber, students[id].studentNumber)) {
        return 1;
    }
    return 0;
}

int compare(Student2& student2, int id) {
    if(student2.id != students2[id].id || strcmp(student2.name, students2[id].name)) {
        return 1;
    }
    return 0;
}

// 随机生成size大小的字符串
void generateStr(char str[], int size) {
    for(int i = 0; i < size - 1; ++i) {
        str[i] = rand() % 95 + 32;
    }
    str[size - 1] = '\0';
}

// 向表中插入10000条数据
RC InsertData(RM_FileHandle& rmFileHandle) {
    int rc;
    cout << "insert 10000 records..\n";
    for(int i = 0; i < studentCount; ++i) {
        students[i].id = i;
        generateStr(students[i].name, nameSize);
        generateStr(students[i].studentNumber, studentNumberSize);
        RID rid;
        if((rc = rmFileHandle.insertRec((char*)&students[i], rid))) {
            return rc;
        }
    }
    cout << "Insert Data Successfully!\n";
    return 0;
}

// 验证插入的数据
RC VertifyData(RM_FileHandle& rmFileHandle) {
    int rc;
    RM_FileScan rmFileScan;
    int num = 0;
    if((rc = rmFileScan.openScan(rmFileHandle, INT, 4, offsetof(Student, id), GE_OP, &num))) {
        return rc;
    }
    RM_Record rmRecord;
    int i = 0;
    while((rc = rmFileScan.getNextRec(rmRecord)) == 0) {
        char* pData;
        if((rc = rmRecord.getData(pData))) {
            return rc;
        }
        Student* tmp = (Student*)pData;
        if(compare(*tmp, tmp->id)) {
            cout << "inconsisitent error" << endl;
            return 0;
        }
        cout << i++ << ": " << tmp->id << " " << tmp->name << " " << tmp->studentNumber << endl;
    }
    if(rc != RM_EOF) {
        return rc;
    }
    cout << "Vertify Successfully!\n";
    return 0;
}

// 向表中插入10000条变长数据
RC InsertData2(RM_FileHandle& rmFileHandle) {
    int rc;
    cout << "insert 10000 records..\n";
    for(int i = 0; i < studentCount; ++i) {
        students2[i].id = i;
        generateStr(students2[i].name, nameSize);
        RID rid;
        // 生成随机1 ~ 4000长度的字符串(计'\0')
        const int length = rand() % 3999 + 1;
        generateStr(varStudentNums[i], length);
        if((rc = rmFileHandle.insertRec((char*)&students2[i], rid))) {
            return rc;
        }
        if((rc = rmFileHandle.insertVarValue(rid, offsetof(Student2, varLenAttr), varStudentNums[i],
                                             length, students2[i].varLenAttr))) {
            return rc;
        }
    }
    cout << "Insert Data Successfully!\n";
    return 0;
}

RC VertifyData2(RM_FileHandle& rmFileHandle) {
    int rc;
    RM_FileScan rmFileScan;
    int num = 0;
    if((rc = rmFileScan.openScan(rmFileHandle, INT, 4, offsetof(Student2, id), GE_OP, &num))) {
        return rc;
    }
    RM_Record rmRecord;
    int i = 0;
    while((rc = rmFileScan.getNextRec(rmRecord)) != RM_EOF) {
        char* pData;
        if((rc = rmRecord.getData(pData))) {
            return rc;
        }
        Student2* tmp = (Student2*)pData;
        char* pVarVal;
        if((rc = rmFileHandle.getVarValue(tmp->varLenAttr, pVarVal))) {
            return rc;
        }
        if(compare(*tmp, tmp->id) || strcmp(pVarVal, varStudentNums[tmp->id])) {
            cout << "inconsisitent error" << endl;
            return 0;
        }
        cout << i++ << ": " << pVarVal << "\n";
    }
    cout << "Vertify Successfully!\n";
    return 0;
}

// Test: 测试初始化表空间, 缓冲区空间, 并插入数据
RC Test1() {
    cout << "Test1 Start\n";
    int rc;
    char fileName[20] = "testTable";
    RM_FileHandle rmFileHandle;
    // 测试创建表文件
    if((rc = rmManager.createFile(fileName, sizeof(Student)))) {
        return rc;
    }
    // 打开文件
    if((rc = rmManager.openFile(fileName, rmFileHandle))) {
        return rc;
    }
    // 插入数据
    if((rc = InsertData(rmFileHandle))) {
        return rc;
    }
    // 验证数据
    if((rc = VertifyData(rmFileHandle))) {
        return rc;
    }
    if((rc = rmManager.closeFile(rmFileHandle))) {
        return rc;
    }
    // 再验证一次数据
    if((rc = rmManager.openFile(fileName, rmFileHandle))) {
        return rc;
    }
    if((rc = VertifyData(rmFileHandle))) {
        return rc;
    }
    if((rc = rmManager.closeFile(rmFileHandle))) {
        return rc;
    }
    // 删除表文件
    if((rc = rmManager.destroyFile(fileName))) {
        return rc;
    }
    cout << "Test1 done!\n";
    return 0;
}

RC Test2() {
    cout << "Test2 Start\n";
    int rc;
    char fileName[20] = "testTable";
    RM_FileHandle rmFileHandle;
    // 测试创建表文件
    if((rc = rmManager.createFile(fileName, sizeof(Student2)))) {
        return rc;
    }
    // 打开文件
    if((rc = rmManager.openFile(fileName, rmFileHandle))) {
        return rc;
    }
    // 插入数据
    if((rc = InsertData2(rmFileHandle))) {
        return rc;
    }
    // 验证数据
    if((rc = VertifyData2(rmFileHandle))) {
        return rc;
    }
    if((rc = rmManager.closeFile(rmFileHandle))) {
        return rc;
    }
    // 删除表文件
    if((rc = rmManager.destroyFile(fileName))) {
        return rc;
    }
    cout << "Test2 done!\n";
    return 0;
}
