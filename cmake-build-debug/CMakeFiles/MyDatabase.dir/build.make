# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "D:\clion\CLion 2020.2.4\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "D:\clion\CLion 2020.2.4\bin\cmake\win\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Administrator\CLionProjects\MyDatabase

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/MyDatabase.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MyDatabase.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MyDatabase.dir/flags.make

CMakeFiles/MyDatabase.dir/main.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MyDatabase.dir/main.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\main.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\main.cpp

CMakeFiles/MyDatabase.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/main.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\main.cpp > CMakeFiles\MyDatabase.dir\main.cpp.i

CMakeFiles/MyDatabase.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/main.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\main.cpp -o CMakeFiles\MyDatabase.dir\main.cpp.s

CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.obj: ../storage/pf_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\pf_manager.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_manager.cpp

CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_manager.cpp > CMakeFiles\MyDatabase.dir\storage\pf_manager.cpp.i

CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_manager.cpp -o CMakeFiles\MyDatabase.dir\storage\pf_manager.cpp.s

CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.obj: ../storage/bufhashtable.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\bufhashtable.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufhashtable.cpp

CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufhashtable.cpp > CMakeFiles\MyDatabase.dir\storage\bufhashtable.cpp.i

CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufhashtable.cpp -o CMakeFiles\MyDatabase.dir\storage\bufhashtable.cpp.s

CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.obj: ../storage/bufmgr.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\bufmgr.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufmgr.cpp

CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufmgr.cpp > CMakeFiles\MyDatabase.dir\storage\bufmgr.cpp.i

CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\bufmgr.cpp -o CMakeFiles\MyDatabase.dir\storage\bufmgr.cpp.s

CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.obj: ../storage/pf_filehandle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\pf_filehandle.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_filehandle.cpp

CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_filehandle.cpp > CMakeFiles\MyDatabase.dir\storage\pf_filehandle.cpp.i

CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\pf_filehandle.cpp -o CMakeFiles\MyDatabase.dir\storage\pf_filehandle.cpp.s

CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.obj: ../storage/pagehandle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\pagehandle.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\pagehandle.cpp

CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\pagehandle.cpp > CMakeFiles\MyDatabase.dir\storage\pagehandle.cpp.i

CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\pagehandle.cpp -o CMakeFiles\MyDatabase.dir\storage\pagehandle.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.obj: ../storage/rm_record.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_record.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_record.cpp

CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_record.cpp > CMakeFiles\MyDatabase.dir\storage\rm_record.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_record.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_record.cpp.s

CMakeFiles/MyDatabase.dir/storage/rid.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rid.cpp.obj: ../storage/rid.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rid.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rid.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rid.cpp

CMakeFiles/MyDatabase.dir/storage/rid.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rid.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rid.cpp > CMakeFiles\MyDatabase.dir\storage\rid.cpp.i

CMakeFiles/MyDatabase.dir/storage/rid.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rid.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rid.cpp -o CMakeFiles\MyDatabase.dir\storage\rid.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.obj: ../storage/rm_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_manager.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_manager.cpp

CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_manager.cpp > CMakeFiles\MyDatabase.dir\storage\rm_manager.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_manager.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_manager.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.obj: ../storage/rm_filehandle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_filehandle.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filehandle.cpp

CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filehandle.cpp > CMakeFiles\MyDatabase.dir\storage\rm_filehandle.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filehandle.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_filehandle.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.obj: ../storage/rm_filescan.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_filescan.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filescan.cpp

CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filescan.cpp > CMakeFiles\MyDatabase.dir\storage\rm_filescan.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_filescan.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_filescan.cpp.s

CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.obj: ../sql/ddl_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\sql\ddl_manager.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\sql\ddl_manager.cpp

CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\sql\ddl_manager.cpp > CMakeFiles\MyDatabase.dir\sql\ddl_manager.cpp.i

CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\sql\ddl_manager.cpp -o CMakeFiles\MyDatabase.dir\sql\ddl_manager.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.obj: ../storage/rm_varlenattr.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_varlenattr.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_varlenattr.cpp

CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_varlenattr.cpp > CMakeFiles\MyDatabase.dir\storage\rm_varlenattr.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_varlenattr.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_varlenattr.cpp.s

CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.obj: ../storage/attrfilehandle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\attrfilehandle.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\attrfilehandle.cpp

CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\attrfilehandle.cpp > CMakeFiles\MyDatabase.dir\storage\attrfilehandle.cpp.i

CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\attrfilehandle.cpp -o CMakeFiles\MyDatabase.dir\storage\attrfilehandle.cpp.s

CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.obj: ../storage/rm_bitmap.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Building CXX object CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\rm_bitmap.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_bitmap.cpp

CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_bitmap.cpp > CMakeFiles\MyDatabase.dir\storage\rm_bitmap.cpp.i

CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\rm_bitmap.cpp -o CMakeFiles\MyDatabase.dir\storage\rm_bitmap.cpp.s

CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.obj: ../atomic/RWLock.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_16) "Building CXX object CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\atomic\RWLock.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\atomic\RWLock.cpp

CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\atomic\RWLock.cpp > CMakeFiles\MyDatabase.dir\atomic\RWLock.cpp.i

CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\atomic\RWLock.cpp -o CMakeFiles\MyDatabase.dir\atomic\RWLock.cpp.s

CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.obj: ../sql/dml_manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_17) "Building CXX object CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\sql\dml_manager.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\sql\dml_manager.cpp

CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\sql\dml_manager.cpp > CMakeFiles\MyDatabase.dir\sql\dml_manager.cpp.i

CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\sql\dml_manager.cpp -o CMakeFiles\MyDatabase.dir\sql\dml_manager.cpp.s

CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.obj: ../storage/ix_indexhandle.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_18) "Building CXX object CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\ix_indexhandle.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexhandle.cpp

CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexhandle.cpp > CMakeFiles\MyDatabase.dir\storage\ix_indexhandle.cpp.i

CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexhandle.cpp -o CMakeFiles\MyDatabase.dir\storage\ix_indexhandle.cpp.s

CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.obj: CMakeFiles/MyDatabase.dir/flags.make
CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.obj: ../storage/ix_indexscan.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_19) "Building CXX object CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.obj"
	C:\MinGW64\mingw64\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\MyDatabase.dir\storage\ix_indexscan.cpp.obj -c C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexscan.cpp

CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.i"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexscan.cpp > CMakeFiles\MyDatabase.dir\storage\ix_indexscan.cpp.i

CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.s"
	C:\MinGW64\mingw64\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\Administrator\CLionProjects\MyDatabase\storage\ix_indexscan.cpp -o CMakeFiles\MyDatabase.dir\storage\ix_indexscan.cpp.s

# Object files for target MyDatabase
MyDatabase_OBJECTS = \
"CMakeFiles/MyDatabase.dir/main.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rid.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.obj" \
"CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.obj" \
"CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.obj" \
"CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.obj" \
"CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.obj"

# External object files for target MyDatabase
MyDatabase_EXTERNAL_OBJECTS =

MyDatabase.exe: CMakeFiles/MyDatabase.dir/main.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/pf_manager.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/bufhashtable.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/bufmgr.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/pf_filehandle.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/pagehandle.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_record.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rid.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_manager.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_filehandle.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_filescan.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/sql/ddl_manager.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_varlenattr.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/attrfilehandle.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/rm_bitmap.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/atomic/RWLock.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/sql/dml_manager.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/ix_indexhandle.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/storage/ix_indexscan.cpp.obj
MyDatabase.exe: CMakeFiles/MyDatabase.dir/build.make
MyDatabase.exe: CMakeFiles/MyDatabase.dir/linklibs.rsp
MyDatabase.exe: CMakeFiles/MyDatabase.dir/objects1.rsp
MyDatabase.exe: CMakeFiles/MyDatabase.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_20) "Linking CXX executable MyDatabase.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\MyDatabase.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MyDatabase.dir/build: MyDatabase.exe

.PHONY : CMakeFiles/MyDatabase.dir/build

CMakeFiles/MyDatabase.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\MyDatabase.dir\cmake_clean.cmake
.PHONY : CMakeFiles/MyDatabase.dir/clean

CMakeFiles/MyDatabase.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\Administrator\CLionProjects\MyDatabase C:\Users\Administrator\CLionProjects\MyDatabase C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug C:\Users\Administrator\CLionProjects\MyDatabase\cmake-build-debug\CMakeFiles\MyDatabase.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MyDatabase.dir/depend

