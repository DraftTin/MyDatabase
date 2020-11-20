cmake_minimum_required(VERSION 3.17)
project(MyDatabase)

set(CMAKE_CXX_STANDARD 14)

add_executable(MyDatabase global.h pf/pf.h pf/pf_buffermgr.h pf/pf_internal.h pf/pf_hashtable.h main.cpp pf/pf_manager.cpp pf/pf_hashtable.cpp pf/pf_buffermgr.cpp pf/pf_filehandle.cpp pf/pf_pagehandle.cpp pf/pf_error.h rm/rm.h rm/rm_rid.h rm/rm_record.cpp rm/rm_rid.cpp rm/rm_internal.h rm/rm_manager.cpp rm/rm_filehandle.cpp rm/rm_error.h rm/rm_filescan.cpp sm/sm_manager.cpp parser.h rm/rm_varlenattr.cpp rm/rm_attrfilehandle.cpp rm/rm_bitmap.h rm/rm_bitmap.cpp)
add_executable(pf_test1 global.h pf/pf.h pf/pf_buffermgr.h pf/pf_internal.h pf/pf_hashtable.h pf/pf_test1.cpp pf/pf_manager.cpp pf/pf_hashtable.cpp pf/pf_buffermgr.cpp pf/pf_filehandle.cpp pf/pf_pagehandle.cpp pf/pf_error.h)
add_executable(pf_test2 global.h pf/pf.h pf/pf_buffermgr.h pf/pf_internal.h pf/pf_hashtable.h pf/pf_test2.cpp pf/pf_manager.cpp pf/pf_hashtable.cpp pf/pf_buffermgr.cpp pf/pf_filehandle.cpp pf/pf_pagehandle.cpp pf/pf_error.h)
add_executable(rm_test1 global.h pf/pf.h pf/pf_buffermgr.h pf/pf_internal.h pf/pf_hashtable.h rm/rm_test1.cpp pf/pf_manager.cpp pf/pf_hashtable.cpp pf/pf_buffermgr.cpp pf/pf_filehandle.cpp pf/pf_pagehandle.cpp pf/pf_error.h rm/rm.h rm/rm_rid.h rm/rm_record.cpp rm/rm_rid.cpp rm/rm_internal.h rm/rm_manager.cpp rm/rm_filehandle.cpp rm/rm_error.h rm/rm_filescan.cpp sm/sm_manager.cpp parser.h rm/rm_varlenattr.cpp rm/rm_attrfilehandle.cpp rm/rm_bitmap.h rm/rm_bitmap.cpp)
add_executable(dbcreate global.h pf/pf.h pf/pf_buffermgr.h pf/pf_internal.h pf/pf_hashtable.h dbcreate.cpp pf/pf_manager.cpp pf/pf_hashtable.cpp pf/pf_buffermgr.cpp pf/pf_filehandle.cpp pf/pf_pagehandle.cpp pf/pf_error.h rm/rm.h rm/rm_rid.h rm/rm_record.cpp rm/rm_rid.cpp rm/rm_internal.h rm/rm_manager.cpp rm/rm_filehandle.cpp rm/rm_error.h rm/rm_filescan.cpp sm/sm_manager.cpp parser.h)
