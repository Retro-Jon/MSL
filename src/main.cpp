#include "lang.hpp"
#include <iostream>
#include <chrono>

int main(int argc, char** argv)
{
    std::string executable_path = argv[0];

    while (executable_path.at(executable_path.size() - 1) != '/' && executable_path.at(executable_path.size() - 1) != '\\')
        executable_path.erase(executable_path.size() - 1, 1);

    if (argc == 2)
    {
        std::string program_path = argv[1];

        while (program_path.at(program_path.size() - 1) != '/' && program_path.at(program_path.size() - 1) != '\\')
            program_path.erase(program_path.size() - 1, 1);

        std::string code = load_file(argv[1]);
        std::vector<Token> stack;
        auto start = std::chrono::high_resolution_clock::now();
    
        Node* program = tokenize(code.c_str());
        if (lex(program))
            if (parse(program))
                interpret(executable_path, program_path, program, stack);
        
        delete_nodes(program);

        auto stop = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

//        std::cout << "\nExecution time: " << duration.count() << " microseconds" << std::endl;
    }
    else if (argc == 1)
    {
        std::string program_path = argv[0];

        while (program_path.at(program_path.size() - 1) != '/' && program_path.at(program_path.size() - 1) != '\\')
            program_path.erase(program_path.size() - 1, 1);

        std::vector<Token> stack;
        std::cout << "REPL Mode" << std::endl;
        std::string input;

        do
        {
            std::cout << "\nEnter commands:\n\n> ";
            std::getline(std::cin, input);

            Node* program = tokenize(input.c_str());
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
