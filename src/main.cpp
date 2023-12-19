#include "lang.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        std::string code = load_file(argv[1]);
        std::vector<Token> stack;
    
        Node* program = tokenize(code.c_str());
        if (lex(program))
            if (parse(program))
                interpret(program, stack);
        
        delete_nodes(program);
    }
    else if (argc == 1)
    {
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
                    interpret(program, stack);

            delete_nodes(program);
        }
        while (input != "exit");
    } else
        std::cout << "Invalid parameters." << std::endl;

    return 0;
}
