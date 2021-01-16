//
// Created by Administrator on 2021/1/16.
//

#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]) {
    char *dbName;
    char command[255] = "rd /s /q ";
    if(argc != 2) {
        cerr << "to use: .\\dbdestroy <dbname>\n";
        exit(1);
    }
    dbName = argv[1];
    if(system(strcat(command, dbName)) != 0) {
        cerr << "destroy " << dbName << " failed\n";
        exit(1);
    }
    return 0;
}
