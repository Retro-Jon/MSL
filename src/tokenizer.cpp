#include "lang.hpp"
#include <algorithm>

std::vector<std::string> included_files;

Node* tokenize(const std::string& executable_path, const std::string& program_path, const std::string& code, const std::string& file_name)
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

    std::string code_string(code);
    code_string += "\n";
    int line_count = 1;

    if (std::count(included_files.begin(), included_files.end(), file_name.c_str()) == 0)
        included_files.push_back(file_name.c_str());

    int file = std::find(included_files.begin(), included_files.end(), file_name) - included_files.begin();

    {
        pointer->default_next = new Node();
        pointer = pointer->default_next;
        pointer->line = 1;
        pointer->t.type = TokenType::ROOT;
        pointer->t.value = TokenTypeString[TokenType::ROOT];
        pointer->file_source = file;
    }

    for (char c : code_string)
    {
        line_count += (c == '\n');

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
            if (directive == "include" && std::find(included_files.begin(), included_files.end(), directive_arg) == included_files.end())
            {
                std::string file_path;

                if (directive_arg.substr(0, 3) == "std")
                    file_path = stdlibpath + directive_arg;
                else
                    file_path = program_path + directive_arg;
                
                std::string file_content = load_file(file_path);
                pointer->default_next = tokenize(executable_path, program_path, file_content, file_path);

                while (pointer->default_next != nullptr)
                    pointer = pointer->default_next;
            }

            directive.clear();
            directive_arg.clear();
            continue;
        }

        if (c == '\"' && !in_char)
            in_string = !in_string;

        if (c == '\'' && !in_string)
            in_char = !in_char;

        if ((c != ' ' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\n') || (in_string || in_char))
            current += c;
        else
        {
            if (c == '[' || c == ']' || c == '{' || c == '}')
            {
                if (current != "")
                {
                    pointer->default_next = new Node();
                    pointer = pointer->default_next;
                    pointer->line = (c == '\n') ? line_count - 1 : line_count;
                    pointer->t.value = current;
                    pointer->file_source = file;
                }
                current = c;
            }

            if (current == "")
                continue;

            pointer->default_next = new Node();
            pointer = pointer->default_next;
            pointer->line = line_count - (c == '\n');
            pointer->t.value = current;
            pointer->file_source = file;
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
        pointer->line = line_count;
        pointer->file_source = file;
    }

    return list;
}
