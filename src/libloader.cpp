#include "libloader.hpp"
#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <map>
#include <functional>

static std::map<std::string, std::function<void(void)>> cppfunctions;
static std::map<std::string, void*> libs;

// Close library
void closelibs()
{
    for (std::pair<std::string, void*> kv : libs)
        dlclose(libs.at(kv.first));

    libs.clear();
}

// Return library function pointer
bool bindlibfunc(const char* librarypath, const char* function)
{
    void* lib;

    if (libs.count(std::string(librarypath)) == 0)
    {
        lib = dlopen(librarypath, RTLD_NOW);

        if (lib == NULL)
        {
            perror("dlopen");
            return false;
        }

        libs.insert(std::pair<std::string, void*>(std::string(librarypath), lib));
    } else {
        lib = libs.at(std::string(librarypath));
    }

    int (*func) (void);
    *(void**)(&func) = dlsym(lib, function);

    if (func == NULL)
    {
        dlclose(lib);
        return false;
    }

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

