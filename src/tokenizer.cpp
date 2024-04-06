#include "lang.hpp"
#include <vector>
#include <algorithm>

Node* tokenize(const std::string &executable_path, const std::string &program_path, const std::string &code)
{
    Node* list = new Node();
    Node* pointer = list;

    std::string current;
    bool in_char = false;
    bool in_string = false;
    bool in_comment = false;
    bool in_directive = false;
    bool in_directive_arg = false;
    std::string directive = "";
    std::string directive_arg = "";

    std::string stdlibpath = executable_path + "libs/std/";
    std::vector<std::string> includes;

    std::string code_string(code);
    code_string += "\n";
    int line_count = 1;

    for (char c : code_string)
    {
        line_count += (c == '\n') ? 1 : 0;

        in_comment = (c == '#' && !in_string) ? true : (c == '\n' && in_comment) ? false : in_comment;
        
        if (in_comment)
            continue;

        if (c == '@' && !in_string)
        {
            in_directive = true;
            continue;
        }

        if (in_directive)
        {
            if (c != ' ')
                directive += c;
            else
            {
                in_directive = false;
                in_directive_arg = true;
            }
            continue;
        } else if (in_directive_arg)
        {
            if (c != '\n')
            {
                directive_arg += c;
                continue;
            }
            
            in_directive_arg = false;
        }

        if (!directive.empty())
        {
            if (directive == "include" && std::find(includes.begin(), includes.end(), directive_arg) == includes.end())
            {
                includes.push_back(directive_arg);
    
                std::string file_path;

                if (directive_arg.substr(0, 3) == "std")
                    file_path = stdlibpath + directive_arg;
                else
                    file_path = program_path + directive_arg;
                
                std::string file_content = load_file(file_path);
                Node* new_nodes = tokenize(executable_path, program_path, file_content);
                pointer->default_next = new_nodes;

                while (pointer->default_next != nullptr)
                    pointer = pointer->default_next;
            }

            directive.clear();
            directive_arg.clear();
            continue;
        }
        
        in_string = (c == '\"' && !in_char) ? !in_string : in_string;
        in_char = (c == '\'' && !in_string) ? !in_char : in_char;
        
        if ((c != ' ' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\n') || (in_string || in_char))
            current += c;
        else
        {
            if (c == '[' || c == ']' || c == '{' || c == '}')
            {
                if (current != "")
                {
                    Node* n_node = new Node();
                    n_node->line = c == '\n' ? line_count - 1 : line_count;
                    n_node->t.value = current;
                    pointer->default_next = n_node;
                    pointer = pointer->default_next;
                }
                current = c;
            }

            if (current == "")
                continue;

            Node* n_node = new Node();
            n_node->line = c == '\n' ? line_count - 1 : line_count;
            n_node->t.value = current;
            pointer->default_next = n_node;
            pointer = pointer->default_next;
            current.clear();
        }
    }

    pointer = list->default_next;
    delete list;
    list = pointer;

    if (!current.empty())
    {
        Node* n_node = new Node();
        pointer = n_node;
        pointer->t.value = current;
    }

    return list;
}
