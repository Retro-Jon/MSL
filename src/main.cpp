#include "lang.hpp"
#include <iostream>
#include <chrono>
#include <unistd.h>

#define LINUX

#ifdef LINUX
#include <linux/limits.h>

std::string getexepath()
{
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) -1);
    std::string result = std::string(buffer, len);
    while (result.back() != '/' && result.back() != '\\')
        result.erase(result.length() - 1, 1);

    return result;
}
#endif

#ifdef WINDOWS
#include <windows.h>

std::string getexepath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string result = std::string(buffer, sizeof buffer);
    while (result.back() != '/' && result.back() != '\\')
        result.erase(result.length() - 1, 1);

    return result;
}
#endif

int main(int argc, char** argv)
{
    std::string executable_path = getexepath();

    if (argc == 2)
    {
        std::string program_path = argv[1];

        std::string code = load_file(argv[1]);
        std::vector<Token> stack;
        auto start = std::chrono::high_resolution_clock::now();
    
        Node* program = tokenize(executable_path, program_path, code.c_str());
        if (lex(program))
            if (parse(program))
                interpret(executable_path, program_path, program, stack);
        
        delete_nodes(program);

        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

//        std::cout << "\nExecution time: " << duration.count() << " microseconds" << std::endl;
    }
    else if (argc == 1)
    {
        std::string program_path = argv[0];

        std::vector<Token> stack;
        std::cout << "REPL Mode" << std::endl;
        std::string input;

        do
        {
            std::cout << "\nEnter commands:\n\n> ";
            std::getline(std::cin, input);

            Node* program = tokenize(executable_path, program_path, input.c_str());
            if (lex(program))
                if (parse(program))
                    interpret(executable_path, program_path, program, stack);

            delete_nodes(program);
        }
        while (input != "exit");
    } else
        std::cout << "Invalid parameters." << std::endl;

    return 0;
}
