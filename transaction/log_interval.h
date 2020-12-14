//
// Created by Administrator on 2020/12/14.
//

#ifndef MYDATABASE_LOG_INTERVAL_H
#define MYDATABASE_LOG_INTERVAL_H
#include "../storage/pf.h"

enum LogType {
    UPDATE,
    COMMIT,
    ABORT,
    END
};

struct LogRecord {
    int prevLSN;
    int XID;
    LogType type;
    PageNum pageID;
    int length;
    int offset;
    char *before_image;
    char *after_image;
};

#endif //MYDATABASE_LOG_INTERVAL_H
