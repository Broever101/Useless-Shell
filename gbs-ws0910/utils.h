#pragma once
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <errno.h>
#include <sys/wait.h>
#include <filesystem>
#include <fcntl.h>
#include <unordered_map>
#include <iterator>
#include <sstream>


typedef std::deque<std::string> strVec;
typedef std::deque<char **> progVec;
typedef std::unordered_map<size_t, std::pair<std::string, std::string>> fileMap;
namespace fs = std::filesystem;

static const int READ_END = 0;
static const int WRITE_END = 1;
void exec_with_args(strVec &);

extern std::string getPrompt();
extern std::string getpwd();
extern void ls(strVec &);
extern void cd(std::string &, strVec &);
extern void clear();
extern strVec envr();
extern void set_env(strVec &);
extern void unset_env(strVec &);
extern strVec tokenize(std::string);
extern char **getNextProgram(strVec &);
extern std::string getFile(strVec &);
extern void initPipes(int[][2], size_t);
extern void closePipesTill(int[][2], size_t);
extern int openFile(fileMap &, const int, const int);
extern void execute(progVec &, fileMap &);
extern void exec_with_args(strVec &);
extern void run(strVec &, std::string&);