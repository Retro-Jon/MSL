#include "lang.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "MSL\nversion " << VERSION << std::endl;
    std::string executable_path = getexepath();

    if (argc == 2)
    {
        std::string program_path = argv[1];
        if (!is_valid_extension(program_path, EXTENSION))
        {
            std::cout << "Invalid file extension: " << program_path << "\nExpected .msol extension." << std::endl;
            return 0;
        }

        std::string code = load_file(argv[1]);
        std::vector<Token> stack;

        Node* program = tokenize(executable_path, program_path, code, program_path);

        if (lex(program))
            if (parse(program))
                interpret(executable_path, get_base_path(program_path), program, stack);
        
        delete_nodes(program);
    }
    else if (argc == 1)
    {
        std::string program_path = "user session";

        std::vector<Token> stack;
        std::cout << "REPL Mode" << std::endl;
        std::string input;

        do
        {
            std::cout << "\nEnter commands:\n\n> ";
            std::getline(std::cin, input);

            Node* program = tokenize(executable_path, program_path, input, program_path);

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
