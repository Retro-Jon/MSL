#include "lang.hpp"

Node* tokenize(const char* code)
{
    Node* list = new Node();
    Node* pointer = list;

    std::string current;
    bool in_char = false;
    bool in_string = false;
    bool in_comment = false;

    std::string code_string(code);
    code_string += "\n";
    int line_count = 1;

    for (char c : code_string)
    {
        line_count += (c == '\n') ? 1 : 0;

        in_comment = (c == '#' && !in_string) ? true : (c == '\n' && in_comment) ? false : in_comment;
        
        if (in_comment)
            continue;
        
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
