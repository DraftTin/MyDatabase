#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>

#include "../global.h"
#include "pf.h"
#include "rm.h"
#include "pf_error.h"
#include "rm_error.h"

using namespace std;

//
// Defines
//
#define FILENAME   "testrel"         // test file name
#define STRLEN      29               // length of string in testrec
#define PROG_UNIT   50               // how frequently to give progress
                                     // reports when adding lots of recs
#define FEW_RECS   10000             // number of records added in

//
// Computes the offset of a field in a record (should be in <stddef.h>)
//
#ifndef offsetof
#       define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

//
// 将TestRec结构体表示为一条数据记录
//
struct TestRec {
    char  str[STRLEN];
    int   num;
    float r;
};

struct TestRec2 {
    char str[sizeof(RM_VarLenAttr)];
    int num;
    float r;
};

//
// Global PF_Manager and RM_Manager variables
//
PF_Manager pfm;
RM_Manager rmm(pfm);

//
// Function declarations
//
RC Test1(void);
RC Test2(void);
RC Test3(void);
RC Test4(void);
RC Test5(void);
RC Test6(void);
RC Test7(void);

void PrintError(RC rc);
void LsFile(char *fileName);
void PrintRecord(TestRec &recBuf);
RC AddRecs(RM_FileHandle &fh, int numRecs);
RC VerifyFile(RM_FileHandle &fh, int numRecs, CompOp op);
RC PrintFile(RM_FileHandle &fh);

RC CreateFile(char *fileName, int recordSize);
RC DestroyFile(char *fileName);
RC OpenFile(char *fileName, RM_FileHandle &fh);
RC CloseFile(char *fileName, RM_FileHandle &fh);
RC GetRec(RM_FileHandle &fh, RID &rid, RM_Record &rec);
RC InsertRec(RM_FileHandle &fh, char *record, RID &rid);
RC UpdateRec(RM_FileHandle &fh, RM_Record &rec);
RC DeleteRec(RM_FileHandle &fh, RID &rid);
RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec);

//
// Array of pointers to the test functions
//
#define NUM_TESTS       7               // number of tests
int (*tests[])() =                      // RC doesn't work on some compilers
{
    Test1,
    Test2,
    Test3,
    Test4,
    Test5,
    Test6,
    Test7,
};

//
// main
//
int main(int argc, char *argv[])
{
    RC   rc;
    char *progName = argv[0];   // since we will be changing argv
    int  testNum;

    // Write out initial starting message
    cerr.flush();
    cout.flush();
    cout << "Starting RM component test.\n";
    cout.flush();

    // Delete files from usedTail time
    unlink(FILENAME);

    // If no argument given, do all tests
    if (argc == 1) {
        for (testNum = 0; testNum < NUM_TESTS; testNum++) {
            if ((rc = (tests[testNum])())) {

                // Print the error and exit
                PrintError(rc);
                return (1);
            }
        }
    }
    else {
        // Otherwise, perform specific tests
        while (*++argv != NULL) {

            // Make sure it's a number
            if (sscanf(*argv, "%d", &testNum) != 1) {
                cerr << progName << ": " << *argv << " is not a number\n";
                continue;
            }

            // Make sure it's in range
            if (testNum < 1 || testNum > NUM_TESTS) {
                cerr << "Valid test numbers are between 1 and " << NUM_TESTS << "\n";
                continue;
            }

            // Perform the test
            if ((rc = (tests[testNum - 1])())) {

                // Print the error and exit
                PrintError(rc);
                return (1);
            }
        }
    }

    // Write ending message and exit
    cout << "Ending RM component test.\n\n";

    return (0);
}

//
// PrintError
//
// Desc: Print an error message by calling the proper component-specific
//       print-error function
//
void PrintError(RC rc)
{
    if (abs(rc) <= END_PF_WARN)
        PF_PrintError(rc);
    else if (abs(rc) <= END_RM_WARN)
        RM_PrintError(rc);
    else
        cerr << "Error code out of range: " << rc << "\n";
}

////////////////////////////////////////////////////////////////////
// The following functions may be useful in tests that you devise //
////////////////////////////////////////////////////////////////////

//
// LsFile
//
// Desc: list the filename's directory entry
//
void LsFile(char *fileName)
{
    char command[80];

    sprintf(command, "dir %s", fileName);
    printf("doing \"%s\"\n", command);
    system(command);
}

//
// PrintRecord
//
// Desc: Print the TestRec record components
//
void PrintRecord(TestRec &recBuf)
{
    printf("[%s, %d, %f]\n", recBuf.str, recBuf.num, recBuf.r);
}

//
// AddRecs
//
// Desc: Add a number of records to the file
//
RC AddRecs(RM_FileHandle &fh, int numRecs)
{
    RC      rc;
    int     i;
    TestRec recBuf;
    RID     rid;
    PageNum pageNum;
    SlotNum slotNum;

    // We set all of the TestRec to be 0 initially.  This heads off
    // warnings that Purify will give regarding UMR since sizeof(TestRec)
    // is 40, whereas actual size is 37.
    memset((void *)&recBuf, 0, sizeof(recBuf));

    printf("\nadding %d records\n", numRecs);
    for (i = 0; i < numRecs; i++) {
        memset(recBuf.str, ' ', STRLEN);
        sprintf(recBuf.str, "a%d", i);
        recBuf.num = i;
        recBuf.r = (float)i;
        if ((rc = InsertRec(fh, (char *)&recBuf, rid)) ||
            (rc = rid.getPageNum(pageNum)) ||
            (rc = rid.getSlotNum(slotNum))) {
            return (rc);
        }
        if ((i + 1) % PROG_UNIT == 0){
            printf("%d  ", i + 1);
            fflush(stdout);
        }
    }
    if (i % PROG_UNIT != 0)
        printf("%d\n", i);
    else
        putchar('\n');

    // Return ok
    return (0);
}

RC AddRecs_2(RM_FileHandle &fh, int numRecs)
{
    cout << "addRec_2\n";
    RC      rc;
    TestRec2 recBuf;
    RID     rid;
    PageNum pageNum;
    SlotNum slotNum;
    char realStr[60] = "1123456789789654321321";

    memset((void *)&recBuf, 0, sizeof(recBuf));
    int count = 0;
    for (int i = 0; i < numRecs; ++i) {
        memset(recBuf.str, 0, sizeof(RM_VarLenAttr));
        RM_VarLenAttr rmVarLenAttr;
        recBuf.num = i;
        recBuf.r = (float)i;
        if ((rc = InsertRec(fh, (char *)&recBuf, rid))) {
            return (rc);
        }
        if((rc = fh.insertVarValue(rid, offsetof(TestRec2, str), realStr, 60, rmVarLenAttr))) {
            return rc;
        }
        if(++count % 50 == 0) {
            int p, s;
            rid.getPageNum(p); rid.getSlotNum(s);
            cout << "add: " << p << "  " << s << endl;
        }
    }
    return (0);
}

RC VerifyFile_2(RM_FileHandle &fh, int numRecs, CompOp op)
{
    RC        rc;
    int       n;
    RID       rid;
    char      *found;
    RM_Record rec;

    printf("\nverifying file contents\n");

    RM_FileScan fs;
    TestRec2* tempRecord = new TestRec2();
    tempRecord->num = 100;
    // 扫描文件中的记录
    if ((rc=fs.openScan(fh,VARCHAR, 60, offsetof(TestRec2, str),
                        NO_OP, (char*) tempRecord + offsetof(TestRec2, str))))
        return (rc);

    found = new char[numRecs];
    memset(found, 0, numRecs);
    // 转换成int*再取值
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {
    }
    cout << "n = " << n << "\n";
    cout << '\n';

    if (rc != RM_EOF)
        goto err;

    delete tempRecord;
    delete[] found;

    if ((rc=fs.closeScan()))
        return (rc);

    if (n != numRecs) {
        printf("%d records in file (supposed to be %d)\n",
               n, numRecs);
        // exit(1);
    }
    else {
        printf("Success!\n");
    }

    // Return ok
    rc = 0;

    return rc;

    err:
    fs.closeScan();
    delete[] found;
    return (rc);
}

RC DeleteRec_2(RM_FileHandle &fh, int numRecs) {
    RC rc;
    RM_FileScan fileScan;
    TestRec2* tempRecord = new TestRec2();
    tempRecord->num = 100;
    if ((rc=fileScan.openScan(fh,VARCHAR, 60, offsetof(TestRec2, str),
                        NO_OP, (char*) tempRecord + offsetof(TestRec2, str))))
        return (rc);
    RM_Record rec;
    int n;
    for (rc = GetNextRecScan(fileScan, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fileScan, rec), n++) {
        char *pData;
        rec.getData(pData);
        RID ird;
        rec.getRid(ird);
        int p, s, b;
        RM_VarLenAttr *pVar = (RM_VarLenAttr*)(pData + offsetof(TestRec2, str));
        pVar->getPageNum(p);
        pVar->getSlotNum(s);
        pVar->getBlockInfoIndex(b);
        DeleteRec(fh, ird);
        cout << "delete: " << p << "  " << s << endl;
        fh.deleteVarValue(ird, offsetof(TestRec2, str), *pVar);
    }
    fileScan.closeScan();
    cout << "n = " << n << endl;
    return 0;
}

//
// VerifyFile
//
// Desc: verify that a file has records as added by AddRecs
//
RC VerifyFile(RM_FileHandle &fh, int numRecs, CompOp op)
{
    RC        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    char      stringBuf[STRLEN];
    char      *found;
    RM_Record rec;

    printf("\nverifying file contents\n");

    RM_FileScan fs;
    TestRec* tempRecord = new TestRec();
    tempRecord->num = 100;
    // 扫描文件中的记录
    if ((rc=fs.openScan(fh,INT, sizeof(int), offsetof(TestRec, num),
                        NO_OP, (char*) tempRecord + offsetof(TestRec, num))))
        return (rc);


    found = new char[numRecs];
    memset(found, 0, numRecs);
    // 转换成int*再取值
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {
        // Make sure the record is correct
        if ((rc = rec.getData((char *&)pRecBuf)) ||
            (rc = rec.getRid(rid))) {
            goto err;
        }
        int s;
        int p;
        rid.getSlotNum(s);
        rid.getPageNum(p);
        memset(stringBuf,' ', STRLEN);
        sprintf(stringBuf, "a%d", pRecBuf->num);
        if(pRecBuf->num == 2000) continue;
        if (pRecBuf->num < 0 || pRecBuf->num >= numRecs ||
            strcmp(pRecBuf->str, stringBuf) ||
            pRecBuf->r != (float)pRecBuf->num) {
            printf("VerifyFile: invalid record = [%s, %d, %f]\n",
                pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        if (found[pRecBuf->num]) {
            printf("VerifyFile: duplicate record = [%s, %d, %f]\n",
                   pRecBuf->str, pRecBuf->num, pRecBuf->r);
            exit(1);
        }

        found[pRecBuf->num] = 1;
    }
    cout << '\n';

    if (rc != RM_EOF)
        goto err;

    delete tempRecord;
    delete[] found;

    if ((rc=fs.closeScan()))
        return (rc);

    // make sure we had the right number of records in the file
    if (n != numRecs) {
        printf("%d records in file (supposed to be %d)\n",
               n, numRecs);
        // exit(1);
    }
    else {
        printf("Success!\n");
    }

    // Return ok
    rc = 0;

    return rc;

err:
    fs.closeScan();
    delete[] found;
    return (rc);
}

//
// PrintFile
//
// Desc: Print the contents of the file
//
RC PrintFile(RM_FileScan &fs)
{
    RC        rc;
    int       n;
    TestRec   *pRecBuf;
    RID       rid;
    RM_Record rec;

    printf("\nprinting file contents\n");

    // for each record in the file
    for (rc = GetNextRecScan(fs, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fs, rec), n++) {

        // Get the record data and record id
        if ((rc = rec.getData((char *&)pRecBuf)) ||
            (rc = rec.getRid(rid)))
            return (rc);

        // Print the record contents
        PrintRecord(*pRecBuf);
    }

    if (rc != RM_EOF)
        return (rc);

    printf("%d records found\n", n);

    // Return ok
    return (0);
}


//
// UpdateRecords
//
// Desc: Update the records in a file
//
RC UpdateRecords(RM_FileHandle &fh, int numRecs)
{
    RC        rc;
    int       n;
    RM_Record rec;

    printf("\nUpdating records in the file\n");

    for (int i=1; i<=10; i++) {
        RID rid(1, i);
        if ((rc = GetRec(fh, rid, rec))) {
            return rc;
        }
        char* pData;
        if ((rc = rec.getData(pData))) {
            return rc;
        }
        TestRec* tempRecord = (TestRec*) pData;
        tempRecord->num = 2000;
        // Update record
        if ((rc = UpdateRec(fh, rec))) {
            return rc;
        }
    }

    // Return ok
    rc = 0;

    return rc;
}

// 测试变长属性的更新问题
RC UpdateRecords_2(RM_FileHandle &fh, int numRecs) {
    RC rc;
    RM_FileScan fileScan;
    TestRec2* tempRecord = new TestRec2();
    tempRecord->num = 100;
    char str[100];
    for(int i = 0; i < 99; ++i) {
        str[i] = 's';
    }
    str[99] = '\0';
    if ((rc=fileScan.openScan(fh,VARCHAR, 60, offsetof(TestRec2, str),
                              NO_OP, (char*) tempRecord + offsetof(TestRec2, str))))
        return (rc);
    RM_Record rec;
    int n;
    for (rc = GetNextRecScan(fileScan, rec), n = 0;
         rc == 0;
         rc = GetNextRecScan(fileScan, rec), n++) {
        char *pData;
        rec.getData(pData);
        RID ird;
        rec.getRid(ird);
        RM_VarLenAttr *pVar = (RM_VarLenAttr*)(pData + offsetof(TestRec2, str));
        fh.updateVarValue(ird, offsetof(TestRec2, str), str, 100, *pVar);
    }
    fileScan.closeScan();
    return 0;

}

//
// DeleteRecords
//
// Desc: Delete the records in a file
//
RC DeleteRecords(RM_FileHandle &fh, int numRecs)
{
    RC        rc;
    int       n;
    RM_Record rec;

    cout << "numRecs: " << numRecs << endl;
    printf("\nDeleting records in the file\n");

    for (int i=1; i <= 10; i++) {
        RID rid(1, i);

        // Delete record
        if ((rc = DeleteRec(fh, rid))) {
            return rc;
        }
    }

    // Return ok
    rc = 0;

    return rc;
}



////////////////////////////////////////////////////////////////////////
// The following functions are wrappers for some of the RM component  //
// methods.  They give you an opportunity to add debugging statements //
// and/or set breakpoints when testing these methods.                 //
////////////////////////////////////////////////////////////////////////

//
// CreateFile
//
// Desc: call RM_Manager::CreateFile
//
RC CreateFile(char *fileName, int recordSize)
{
    printf("\ncreating %s\n", fileName);
    return (rmm.createFile(fileName, recordSize));
}

//
// DestroyFile
//
// Desc: call RM_Manager::DestroyFile
//
RC DestroyFile(char *fileName)
{
    printf("\ndestroying %s\n", fileName);
    return (rmm.destroyFile(fileName));
}

//
//
//
// Desc: call RM_Manager::OpenFile
//
RC OpenFile(char *fileName, RM_FileHandle &fh)
{
    printf("\nopening %s\n", fileName);
    return (rmm.openFile(fileName, fh));
}

//
// CloseFile
//
// Desc: call RM_Manager::CloseFile
//
RC CloseFile(char *fileName, RM_FileHandle &fh)
{
    if (fileName != NULL)
        printf("\nClosing %s\n", fileName);
    return (rmm.closeFile(fh));
}

//
// GetRec
//
// Desc: call RM_FileHandle::GetRec
//
RC GetRec(RM_FileHandle &fh, RID &rid, RM_Record &rec)
{
    return (fh.getRec(rid, rec));
}

//
// InsertRec
//
// Desc: call RM_FileHandle::InsertRec
//
RC InsertRec(RM_FileHandle &fh, char *record, RID &rid)
{
    return (fh.insertRec(record, rid));
}

//
// DeleteRec
//
// Desc: call RM_FileHandle::DeleteRec
//
RC DeleteRec(RM_FileHandle &fh, RID &rid)
{
    return (fh.deleteRec(rid));
}

//
// UpdateRec
//
// Desc: call RM_FileHandle::UpdateRec
//
RC UpdateRec(RM_FileHandle &fh, RM_Record &rec) {
    return (fh.updateRec(rec));
}

//
// GetNextRecScan
//
// Desc: call RM_FileScan::GetNextRec
//
RC GetNextRecScan(RM_FileScan &fs, RM_Record &rec)
{
    return (fs.getNextRec(rec));
}

/////////////////////////////////////////////////////////////////////
// Sample test functions follow.                                   //
/////////////////////////////////////////////////////////////////////

//
// Test1 tests simple creation, opening, closing, and deletion of files
//
RC Test1(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest1 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char*)FILENAME, fh)) ||
        (rc = CloseFile((char*)FILENAME, fh)))
        return (rc);

    LsFile((char*)FILENAME);

    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);

    printf("\ntest1 done\n*****************************\n");
    return (0);
}

//
// Test2 tests adding a few records to a file.
//
RC Test2(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest2 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec))) ||
        (rc = OpenFile((char*)FILENAME, fh)) ||
        (rc = AddRecs(fh, FEW_RECS)) ||
        (rc = CloseFile((char*)FILENAME, fh)))
        return (rc);

    LsFile((char*)FILENAME);

    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);

    printf("\ntest2 done\n*****************************\n");
    return (0);
}


// Test3: 验证数据插入的正确性
RC Test3(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest3 starting\n*****************************\n");

    // 创建表文件
    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec)))) {
        return rc;
    }
    // 打开表文件
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    // 向表中添加若干条记录
    if ((rc = AddRecs(fh, FEW_RECS))) {
        return rc;
    }
    // 关闭 + 写回数据
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }
    // 重新打开文件
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    // 验证数据是否正确
    if ((rc = VerifyFile(fh, FEW_RECS, NO_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);

    printf("\ntest3 done\n*****************************\n");
    return (0);
}


//
// Test4 tests updating of records in a file
//
RC Test4(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest4 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec)))) {
        return rc;
    }
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    if ((rc = AddRecs(fh, FEW_RECS))) {
        return rc;
    }
    if ((rc = VerifyFile(fh, FEW_RECS, LT_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

    printf("\n---------------------------------\n");
    // Update records in the file
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    if ((rc = UpdateRecords(fh, FEW_RECS))) {
        return rc;
    }
    if ((rc = VerifyFile(fh, FEW_RECS, LT_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

//    LsFile((char*)FILENAME);

    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);

    printf("\ntest4 done\n*****************************\n");
    return (0);
}

//
// Test5 tests deleting of records in a file
//
RC Test5(void)
{
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest5 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec)))) {
        return rc;
    }
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    if ((rc = AddRecs(fh, FEW_RECS))) {
        return rc;
    }
    if ((rc = VerifyFile(fh, FEW_RECS, NO_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

    printf("\n---------------------------------\n");

    // Delete records in the file
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }
    if ((rc = DeleteRecords(fh, FEW_RECS))) {
        return rc;
    }
    if ((rc = VerifyFile(fh, FEW_RECS, NO_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

    LsFile((char*)FILENAME);

    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);

    printf("\ntest5 done\n*****************************\n");
    return (0);
}

// Test6: 测试变长属性扫描和删除的问题
RC Test6(void) {
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest6 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec)))) {
        return rc;
    }
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }

    if ((rc = AddRecs_2(fh, 200))) {
        return rc;
    }
    if((rc = DeleteRec_2(fh, 10))) {
        return rc;
    }
    if ((rc = VerifyFile_2(fh, 200, LT_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }

    cout << "#######: do again ##############\n";

    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }

    if ((rc = AddRecs_2(fh, 200))) {
        return rc;
    }
    if ((rc = VerifyFile_2(fh, 200, LT_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }
    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);
    printf("\ntest6 done\n*****************************\n");
    return 0;
}

// Test7: 测试变长属性的update问题
RC Test7(void) {
    RC            rc;
    RM_FileHandle fh;

    printf("\ntest7 starting\n*****************************\n");

    if ((rc = CreateFile((char*)FILENAME, sizeof(TestRec)))) {
        return rc;
    }
    if ((rc = OpenFile((char*)FILENAME, fh))) {
        return rc;
    }

    if ((rc = AddRecs_2(fh, 200))) {
        return rc;
    }
    if ((rc = VerifyFile_2(fh, 200, LT_OP))) {
        return rc;
    }
    if((rc = UpdateRecords_2(fh, 200))) {
        return rc;
    }
    if ((rc = VerifyFile_2(fh, 200, LT_OP))) {
        return rc;
    }
    if ((rc = CloseFile((char*)FILENAME, fh))) {
        return (rc);
    }
    if ((rc = DestroyFile((char*)FILENAME)))
        return (rc);
    printf("\ntest7 done\n*****************************\n");
    return 0;
}