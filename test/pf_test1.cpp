#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "../storage/pf.h"
#include "../storage/pf_internal.h"
#include "../storage/bufhashtable.h"
#include "../storage/pf_error.h"

using namespace std;

// The PF_STATS indicates that we will be tracking statistics for the PF
// Layer.  The Manager is defined within pf_buffermgr.cc.  Here we must
// place the initializer and then the final call to printout the statistics
// once main has finished
#ifdef PF_STATS
#include "statistics.h"

// This is defined within pf_buffermgr.cc
extern StatisticsMgr *pStatisticsMgr;

// This method is defined within pf_statistics.cc.  It is called at the end
// to display the final statistics, or by the debugger to monitor progress.
extern void PF_Statistics();

//
// PF_ConfirmStatistics
//
// This function will be run at the end of the program after all the tests
// to confirm that the buffer manager operated correctly.
//
// These numbers have been confirmed.  Note that if you change any of the
// tests, you will also need to change these numbers as well.
//
void PF_ConfirmStatistics()
{
   // Must remember to delete the memory returned from StatisticsMgr::Get
   cout << "Verifying the statistics for buffer manager: ";
   int *piGP = pStatisticsMgr->Get("GetPage");
   int *piPF = pStatisticsMgr->Get("PageFound");
   int *piPNF = pStatisticsMgr->Get("PageNotFound");
   int *piWP = pStatisticsMgr->Get("WritePage");
   int *piRP = pStatisticsMgr->Get("ReadPage");
   int *piFP = pStatisticsMgr->Get("FlushPage");

   if (piGP && (*piGP != 702)) {
      cout << "Number of GetPages is incorrect! (" << *piGP << ")\n";
      // No built in error code for this
      exit(1);
   }
   if (piPF && (*piPF != 23)) {
      cout << "Number of pages found in the buffer is incorrect! (" <<
        *piPF << ")\n";
      // No built in error code for this
      exit(1);
   }
   if (piPNF && (*piPNF != 679)) {
      cout << "Number of pages not found in the buffer is incorrect! (" <<
        *piPNF << ")\n";
      // No built in error code for this
      exit(1);
   }
   if (piRP && (*piRP != 679)) {
      cout << "Number of read requests to the Unix file system is " <<
         "incorrect! (" << *piPNF << ")\n";
      // No built in error code for this
      exit(1);
   }
   if (piWP && (*piWP != 339)) {
      cout << "Number of write requests to the Unix file system is "<<
         "incorrect! (" << *piPNF << ")\n";
      // No built in error code for this
      exit(1);
   }
   if (piFP && (*piFP != 16)) {
      cout << "Number of requests to flush the buffer is "<<
         "incorrect! (" << *piPNF << ")\n";
      // No built in error code for this
      exit(1);
   }
   cout << " Correct!\n";

   // Delete the memory returned from StatisticsMgr::Get
   delete piGP;
   delete piPF;
   delete piPNF;
   delete piWP;
   delete piRP;
   delete piFP;
}
#endif    // PF_STATS



//
// Defines
//
#define FILE1	"testfile1"
#define FILE2	"testfile2"

//
// Function declarations
//
RC WriteFile(PF_Manager &pfm, char *fname);
RC PrintFile(PF_FileHandle &fh);
RC ReadFile(PF_Manager &pfm, char* fname);
RC TestPF();
RC TestHash();

RC WriteFile(PF_Manager &pfm, char *fname)
{
    PF_FileHandle fh;
    PageHandle ph;
    RC            rc;
    char          *pData;
    PageNum       pageNum;
    int           i;

    cout << "Opening file: " << fname << "\n";

    if ((rc = pfm.openFile(fname, fh))) {
        return (rc);
    }

    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        if ((rc = fh.allocatePage(ph)) ||
            (rc = ph.getData(pData)) ||
            (rc = ph.getPageNum(pageNum)))
            return(rc);

        if (i != pageNum) {
            cout << "Page number incorrect: " << (int)pageNum << " " << i << "\n";
            exit(1);
        }

        memcpy(pData, (char *)&pageNum, sizeof(PageNum));
        //  memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &nextPage, sizeof(PageNum));

        cout << "Page allocated: " << (int)pageNum << "\n";
    }

    // Test pinning too many pages
    if ((rc = fh.allocatePage(ph)) != PF_NOBUF) {
        cout << "Pin too many pages should fail: ";
        return(rc);
    }

    cout << "Unpinning pages and closing the file\n";

    for (i = 0; i < PF_BUFFER_SIZE; i++) {
        if ((rc = fh.unpinPage(i))) {
            return (rc);
        }
    }

    if ((rc = pfm.closeFile(fh))) {
        return (rc);
    }
    // Return ok
    return (0);
}

RC PrintFile(PF_FileHandle &fh)
{
   PageHandle ph;
   RC            rc;
   char          *pData;
   PageNum       pageNum, temp;

   cout << "Reading file\n";

   if ((rc = fh.getFirstPage(ph)))
      return(rc);

   do {
      if ((rc = ph.getData(pData)) ||
            (rc = ph.getPageNum(pageNum)))
         return(rc);

      memcpy((char *)&temp, pData, sizeof(PageNum));
      cout << "Got page: " << (int)pageNum << " " << (int)temp << "\n";

      //    if (memcmp(pData + PF_PAGE_SIZE - sizeof(PageNum),
      //	       pData, sizeof(PageNum))) {
      //      memcpy(&temp, pData + PF_PAGE_SIZE - sizeof(PageNum), sizeof(PageNum));
      //      cout << "ERROR!" << (int)temp << "\n";
      //      return (-1);
      //    }
      if ((rc = fh.unpinPage(pageNum)))
         return(rc);
   } while (!(rc = fh.getNextPage(pageNum, ph)));

   if (rc != PF_EOF)
      return(rc);

   cout << "EOF reached\n";

   // Return ok
   return (0);
}

RC ReadFile(PF_Manager &pfm, char* fname)
{
   PF_FileHandle fh;
   RC            rc;

   cout << "Opening: " << fname << "\n";

   if ((rc = pfm.openFile(fname, fh)) ||
         (rc = PrintFile(fh)) ||
         (rc = pfm.closeFile(fh)))
      return (rc);
   else
      return (0);
}

// 测试PF组件: 创建文件，在文件中申请页，删除文件等
RC TestPF()
{
   PF_Manager    pfm;
   PF_FileHandle fh1, fh2;
   PageHandle ph;
   RC            rc;
   char          *pData;
   PageNum       pageNum, temp;
   int           i;

int len;
//pfm.getBlockSize(len);
//printf("get bock size returned %d\n",len);
   cout << "Creating and opening two files\n";

   if ((rc = pfm.createFile(FILE1)) ||
         (rc = pfm.createFile(FILE2)) ||
         (rc = WriteFile(pfm, (char*)FILE1)) ||
         (rc = ReadFile(pfm, (char*)FILE1)) ||
         (rc = WriteFile(pfm, (char*)FILE2)) ||
         (rc = ReadFile(pfm, (char*)FILE2)) ||
         (rc = pfm.openFile(FILE1, fh1)) ||
         (rc = pfm.openFile(FILE2, fh2))) {
       return (rc);
   }

   cout << "Disposing of alternate pages\n";

   for (i = 0; i < PF_BUFFER_SIZE; i++) {
      if (i & 1) {
         if ((rc = fh1.disposePage(i)))
            return(rc);
      }
      else
         if ((rc = fh2.disposePage(i)))
            return(rc);
   }

   cout << "Closing and destroying both files\n";

   if ((rc = fh1.flushPages()) ||
         (rc = fh2.flushPages()) ||
         (rc = pfm.closeFile(fh1)) ||
         (rc = pfm.closeFile(fh2)) ||
         (rc = ReadFile(pfm, (char*)FILE1)) ||
         (rc = ReadFile(pfm, (char*)FILE2)) ||
         (rc = pfm.destroyFile(FILE1)) ||
         (rc = pfm.destroyFile(FILE2)))
      return(rc);

   cout << "Creating and opening files again\n";

   if ((rc = pfm.createFile(FILE1)) ||
         (rc = pfm.createFile(FILE2)) ||
         (rc = WriteFile(pfm, (char*)FILE1)) ||
         (rc = WriteFile(pfm, (char*)FILE2)) ||
         (rc = pfm.openFile(FILE1, fh1)) ||
         (rc = pfm.openFile(FILE2, fh2))) {
       return (rc);
   }

   cout << "Allocating additional pages in both files\n";

   for (i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
      if ((rc = fh2.allocatePage(ph)) ||
            (rc = ph.getData(pData)) ||
            (rc = ph.getPageNum(pageNum)))
         return(rc);

      if (i != pageNum) {
         cout << "Page number is incorrect:" << (int)pageNum << " " << i << "\n";
         exit(1);
      }

      memcpy(pData, (char*)&pageNum, sizeof(PageNum));
      //  memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &nextPage, sizeof(PageNum));

      if ((rc = fh2.markDirty(pageNum)) ||
            (rc = fh2.unpinPage(pageNum)))
         return(rc);

      if ((rc = fh1.allocatePage(ph)) ||
            (rc = ph.getData(pData)) ||
            (rc = ph.getPageNum(pageNum)))
         return(rc);

      if (i != pageNum) {
         cout << "Page number is incorrect:" << (int)pageNum << " " << i << "\n";
         exit(1);
      }

      memcpy(pData, (char*)&pageNum, sizeof(PageNum));
 // memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &nextPage, sizeof(PageNum));

      if ((rc = fh1.markDirty(pageNum)) ||
            (rc = fh1.unpinPage(pageNum)))
         return(rc);
   }

   cout << "Disposing of alternate additional pages\n";

   for (i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
      if (i & 1) {
         if ((rc = fh1.disposePage(i)))
            return(rc);
      }
      else
         if ((rc = fh2.disposePage(i)))
            return(rc);
   }

   cout << "Getting file 2 remaining additional pages\n";

   for (i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
      if (i & 1) {
         if ((rc = fh2.getThisPage(i, ph)) ||
               (rc = ph.getData(pData)) ||
               (rc = ph.getPageNum(pageNum)))
            return(rc);

         memcpy((char *)&temp, pData, sizeof(PageNum));

         cout << "Page: " << (int)pageNum << " " << (int)temp << "\n";

         if ((rc = fh2.unpinPage(i)))
            return(rc);
      }
   }

   cout << "Getting file 1 remaining additional pages\n";

   for (i = PF_BUFFER_SIZE; i < PF_BUFFER_SIZE * 2; i++) {
      if (!(i & 1)) {
         if ((rc = fh1.getThisPage(i, ph)) ||
               (rc = ph.getData(pData)) ||
               (rc = ph.getPageNum(pageNum)))
            return(rc);

         memcpy((char *)&temp, pData, sizeof(PageNum));

         cout << "Page: " << (int)pageNum << " " << (int)temp << "\n";

         if ((rc = fh1.unpinPage(i)))
            return(rc);
      }
   }

   cout << "Printing file 2, then file 1\n";

   if ((rc = PrintFile(fh2)) ||
         (rc = PrintFile(fh1)))
      return(rc);

   cout << "Putting stuff into the holes of file 1\n";

   for (i = 0; i < PF_BUFFER_SIZE / 2; i++) {
      if ((rc = fh1.allocatePage(ph)) ||
            (rc = ph.getData(pData)) ||
            (rc = ph.getPageNum(pageNum)))
         return(rc);

      memcpy(pData, (char *)&pageNum, sizeof(PageNum));
 // memcpy(pData + PF_PAGE_SIZE - sizeof(PageNum), &nextPage, sizeof(PageNum));

      if ((rc = fh1.markDirty(pageNum)) ||
            (rc = fh1.unpinPage(pageNum)))
         return(rc);
   }

   cout << "Print file 1 and then close both files\n";

   if ((rc = PrintFile(fh1)) ||
         (rc = pfm.closeFile(fh1)) ||
         (rc = pfm.closeFile(fh2)))
      return(rc);

   cout << "Reopen file 1 and test some error conditions\n";

   if ((rc = pfm.openFile(FILE1, fh1)))
      return(rc);

   //  if ((rc = pfm.DestroyFile(FILE1)) != PF_FILEOPEN) {
   //    cout << "Destroy file while open should fail: ";
   //    return(rc);
   //  }

   if ((rc = fh1.disposePage(100)) != PF_INVALIDPAGE) {
      cout << "Dispose invalid page should fail: ";
      return(rc);
   }

   // Get page 1

   if ((rc = fh1.getThisPage(1, ph)))
      return(rc);

   if ((rc = fh1.disposePage(1)) != PF_PAGEPINNED) {
      cout << "Dispose pinned page should fail: ";
      return(rc);
   }

   if ((rc = ph.getData(pData)) ||
         (rc = ph.getPageNum(pageNum)))
      return(rc);

   memcpy((char *)&temp, pData, sizeof(PageNum));

   if (temp != 1 || pageNum != 1) {
      cout << "Asked for page 1, got: " << (int)pageNum << " " <<
         (int)temp << "\n";
      exit(1);
   }

   if ((rc = fh1.unpinPage(pageNum)))
      return(rc);

   if ((rc = fh1.unpinPage(pageNum)) != PF_PAGEUNPINNED) {
      cout << "Unpin unpinned page should fail: ";
      return(rc);
   }

   cout << "Opening file 1 twice, printing out both copies\n";

   if ((rc = pfm.openFile(FILE1, fh2)))
      return(rc);

   if ((rc = PrintFile(fh1)) ||
         (rc = PrintFile(fh2)))
      return(rc);

   cout << "Closing and destroying both files\n";

   if ((rc = pfm.closeFile(fh1)) ||
         (rc = pfm.closeFile(fh2)) ||
         (rc = pfm.destroyFile(FILE1)) ||
         (rc = pfm.destroyFile(FILE2)))
      return(rc);

   // If we are dealing with statistics then we should output the final
   // numbers
#ifdef PF_STATS
   PF_Statistics();
   PF_ConfirmStatistics();
#endif

   // Return ok
   return (0);
}

RC TestHash()
{
    cout << "test hash\n";
   BufHashTable ht(PF_HASH_TBL_SIZE);
   RC           rc;
   int          i, s;
   PageNum      p;

   cout << "Testing hash table.  Inserting entries\n";

   for (i = 1; i < 11; i++)
      for (p = 1; p < 11; p++)
         if ((rc = ht.insert(i, p, i + p)))
            return(rc);

   cout << "Searching for entries\n";

   for (i = 1; i < 11; i++)
      for (p = 1; p < 11; p++)
         if ((rc = ht.find(i, p, s)))
            return(rc);

   cout << "Deleting entries in reverse order\n";

   for (p = 10; p > 0; p--)
      for (i = 10; i > 0; i--)
         if ((rc = ht.remove(i,p)))
            return(rc);

   cout << "Ensuring all entries were deleted\n";

   for (i = 1; i < 11; i++)
      for (p = 1; p < 11; p++)
         if ((rc = ht.find(i, p, s)) != PF_HASHNOTFOUND) {
            cout << "Find deleted hash entry should fail: ";
            return(rc);
         }

   // Return ok
   return (0);
}

int main()
{
   RC rc;

   // Write out initial starting message
   cerr.flush();
   cout.flush();
   cout << "Starting PF layer test.\n";
   cout.flush();

   // If we are tracking the PF Layer statistics
#ifdef PF_STATS
   cout << "Note: Statistics are turned on.\n";
#endif

   // Delete files from usedTail time
   unlink(FILE1);
   unlink(FILE2);

   // Do tests
   if ((rc = TestPF()) ||
         (rc = TestHash())) {
        PF_PrintError(rc);
        return (1);
   }

   // Write ending message and exit
   cout << "Ending PF layer test.\n\n";

   return (0);
}


