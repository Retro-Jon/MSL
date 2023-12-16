#include "lang.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "No program provided." << std::endl;
        return 0;
    }
    
    std::string code = load_file(argv[1]);

    Node* program = tokenize(code.c_str());
    if (lex(program))
        if (parse(program))
            interpret(program);
    
    delete_nodes(program);
    return 0;
}
