#ifndef PF_ERROR_H
#define PF_ERROR_H

#include <cerrno>
#include <cstdio>
#include <iostream>
#include "pf_internal.h"

using namespace std;

//
// Error table
//
static char *PF_WarnMsg[] = {
  (char*)"page pinned in buffer",
  (char*)"page is not in the buffer",
  (char*)"invalid page number",
  (char*)"file open",
  (char*)"invalid file descriptor (file closed)",
  (char*)"page already freeHead",
  (char*)"page already unpinned",
  (char*)"end of file",
  (char*)"attempting to resize the buffer too small",
  (char*)"invalid filename"
};

static char *PF_ErrorMsg[] = {
  (char*)"no memory",
  (char*)"no buffer space",
  (char*)"incomplete read of page from file",
  (char*)"incomplete write of page to file",
  (char*)"incomplete read of header from file",
  (char*)"incomplete write of header from file",
  (char*)"new page to be allocated already in buffer",
  (char*)"hash table entry not found",
  (char*)"page already in hash table",
  (char*)"invalid file name"
};

// 根据rc输出错误信息
void PF_PrintError(RC rc)
{
  // Check the return code is within proper limits
  if (rc >= START_PF_WARN && rc <= PF_LASTWARN)
    // Print warning
    cerr << "PF warning: " << PF_WarnMsg[rc - START_PF_WARN] << "\n";
  // Error codes are negative, so invert everything
  else if (-rc >= -START_PF_ERR && -rc < -PF_LASTERROR)
    // Print error
    cerr << "PF error: " << PF_ErrorMsg[-rc + START_PF_ERR] << "\n";
  else if (rc == PF_UNIX)
#ifdef PC
      cerr << "OS error\n";
#else
      cerr << strerror(errno) << "\n";
#endif
  else if (rc == 0)
    cerr << "PF_PrintError called with return code of 0\n";
  else
    cerr << "PF error: " << rc << " is out of bounds\n";
}

#endif