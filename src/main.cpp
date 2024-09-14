#include "lang.hpp"
#include <cctype>
#include <iostream>
#include <string>
#include <algorithm>

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
            if (parse(program, stack))
                interpret(executable_path, get_base_path(program_path), program, stack);
        
        delete_nodes(program);
    }
    else if (argc == 1)
    {
        std::string program_path = "user session";

        std::vector<Token> stack;
        std::cout << "REPL Mode" << std::endl;
        std::string input;
        std::map<int, std::string> code;
        bool was_insert = false;

        do
        {
            if (!was_insert)
                std::cout << "\nEnter commands:\n\n> ";
            else
                std::cout << "> ";

            std::getline(std::cin, input);

            was_insert = false;

            if (input[0] == ':')
            {
                std::string command = input;
                std::string arg = "";
                command.erase(0, 1);

                int index = 0;
                char c = command[index];

                while (c != ' ' && index < command.length())
                {
                    index++;
                    c = command[index];
                }

                // A random comment

                int tmp_index = index;

                if (command[index] == ' ')
                    command.erase(index, 1);

                while (index < command.length())
                {
                    arg += command[index];
                    index++;
                }

                command.erase(tmp_index, command.length());

                if (!command.empty())
                {
                    if (std::all_of(command.begin(), command.end(), ::isdigit))
                    {
                        int line = std::stoi(command);
                        
                        if (code.find(line) != code.end())
                        {
                            code.at(line) = arg;
                            was_insert = true;
                        } else if (line > 0)
                        {
                            code.insert(std::pair<int, std::string>(line, arg));
                            was_insert = true;
                        } else {
                            std::cout << "Invalid line number." << std::endl;
                            was_insert = false;
                        }

                        continue;
                    } else if (command == "list" || command == "l")
                    {
                        for (std::pair<int, std::string> l : code)
                        {
                            std::cout << l.first << " " << l.second << std::endl;
                        }
                        continue;
                    } else if (command == "run" || command == "r")
                    {
                        input = "";

                        for (int i = 1; i <= code.rbegin()->first; i++)
                        {
                            if (code.count(i) == 0)
                                input += "\n";
                            else
                                input += code.at(i) + "\n";
                        }
                    } else {
                        continue;
                    }
                }
            }

            Node* program = tokenize(executable_path, program_path, input, program_path);

            if (lex(program))
                if (parse(program, stack))
                    interpret(executable_path, program_path, program, stack);

            delete_nodes(program);
        }
        while (input != ":exit");
    } else
        std::cout << "Invalid parameters." << std::endl;

    return 0;
}
