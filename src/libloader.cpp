#include "libloader.hpp"
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <map>
#include <functional>
#include <algorithm>

static std::map<std::string, std::function<void(void)>> cppfunctions;
static std::vector<void*> libs;

// Close library
void closelibs()
{
    for (; !libs.empty(); dlclose(libs.back()), libs.pop_back());
}

// Return library function pointer
bool bindlibfunc(const char* librarypath, const char* function)
{
    void* lib = dlopen(librarypath, RTLD_NOW);

    if (lib == NULL)
    {
        perror("dlopen");
        return false;
    }

    int (*func) (void);
    *(void**)(&func) = dlsym(lib, function);

    if (func == NULL)
    {
        dlclose(lib);
        return false;
    }

    if (std::find(libs.begin(), libs.end(), lib) == libs.end())
        libs.push_back(lib);

    if (cppfunctions.count(function) == 0)
        cppfunctions.insert({function, func});

    return true;
}

bool execute(const std::string& function)
{
    if (cppfunctions.count(function) > 0)
    {
        cppfunctions.at(function)();
        return true;
    }

    return false;
}

