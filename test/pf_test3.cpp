#include <cstdio>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "../storage/pf.h"
#include "../storage/pf_internal.h"
#include "../storage/bufhashtable.h"
#include "../storage/pf_error.h"

using namespace std;

//
// Defines
//
#define FILE1	"testfile1"
#define FILE2	"testfile2"

// Allocate a group of main memory pages from the buffer
RC AllocateChunk(PF_Manager &pfm, int iBlocks, char *ptr[]);

// Verify chunks that were allocated in the buffer
RC VerifyChunks(int iBlocks, char *ptr[]);

// Dispose of the chunks from the buffer pool
RC DisposeChunk(PF_Manager &pfm, int iBlocks, char *ptr[]);

RC TestChunk();

//
// AllocateChunk
//
RC AllocateChunk(PF_Manager &pfm, int iBlocks, char *ptr[])
{
   RC rc = OK_RC;
   int i;

   cout << "Asking for " << iBlocks << " chunks from the buffer manager: ";

   // for (i = 0; i < PF_BUFFER_SIZE; i++) {
   for (i = 0; i < iBlocks; i++) {
      if ((rc = pfm.allocateBlock(ptr[i])))
         break;
      // Put some simple data inside
      memcpy (ptr[i], (void *) &i, sizeof(int));
   }

   return rc;
}

//
// VerifyChunks
//
RC VerifyChunks(int iBlocks, char *ptr[])
{
   int i;

   cout << "Verifying the contents of " << iBlocks << " chunks from the buffer manager: ";

   for (i = 0; i < iBlocks; i++) {
      int k;
      memcpy ((void *)&k, ptr[i], sizeof(int));

      if (k!=i)
         return 1;
   }
   return 0;
}

//
// DisposeChunk
//
RC DisposeChunk(PF_Manager &pfm, int iBlocks, char *ptr[])
{
   RC rc = OK_RC;
   int i;

   cout << "Disposing of " << iBlocks << " chunks from the buffer manager: ";

   // for (i = 0; i < PF_BUFFER_SIZE; i++) {
   for (i = 0; i < iBlocks; i++) {
      if ((rc = pfm.disposeBlock(ptr[i])))
         break;
   }
   return rc;
}


RC TestChunk()
{
   PF_Manager pfm;
   RC rc;
   char *ptr[10];

   if ((rc = AllocateChunk(pfm, 10, ptr))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   if ((rc = VerifyChunks(10, ptr))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   if ((rc = DisposeChunk(pfm, 5, ptr))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // Verify the first 5 chunks, notice that this passes okay!  This is
   // a bit odd since we have Disposed of the first 5 chunks.
   // However, things have not changed since then and the memory is
   // sitting there for awhile before someone else comes along and asks
   // to change it.
   if ((rc = VerifyChunks(10, ptr))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // Now five chunks are left in the buffer
   // Ask for 35 chunks
   char *ptr2[35];
   if ((rc = AllocateChunk(pfm, 35, ptr2))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   char *ptr3[1];
   ptr3[0] = NULL;
   // Now ask for one more chunk -- shouldn't be able to do it with
   // buffer pool size of 40.
   if ((rc = AllocateChunk(pfm, 1, ptr3))==0) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // Now ask to remove a chunk which doesn't exist
   if ((rc = DisposeChunk(pfm, 1, ptr3))==0) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // Now remove the block of 35 chunks from before
   if ((rc = DisposeChunk(pfm, 35, ptr2))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // And ask for 25 more chunks to ensure that I haven't pinned in some
   // way the previous ones.
   if ((rc = AllocateChunk(pfm, 25, (ptr2 + 10)))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";


   // And make sure that all is well...
   if ((rc = VerifyChunks(25, (ptr2+10)))) {
      cout << "FAILED!\a\a\n";
      return rc;
   }
   cout << "Pass\n";

   // Finally, leave the chunks that are there lying around.  They will
   // be cleaned up by the PF Manager instance and no purify warnings
   // should result.
   return 0;
}

int main()
{
   RC rc;

   // Write out initial starting message
   cerr.flush();
   cout.flush();
   cout << "Starting PF Chunk layer test.\n";
   cout.flush();

   // If we are tracking the PF Layer statistics
#ifdef PF_STATS
   cout << "Note: Statistics are turned on.\n";
#endif

   // Do tests
   if ((rc = TestChunk())) {
      PF_PrintError(rc);
      return (1);
   }

   // Write ending message and exit
   cout << "Ending PF Chunk layer test.\n\n";

   return (0);
}

