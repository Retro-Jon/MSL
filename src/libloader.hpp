#pragma once
#include <string>

void closelibs();
bool bindlibfunc(const char* librarypath, const char* function);
bool execute(const std::string& function);

