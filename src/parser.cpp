#include "lang.hpp"
#include <vector>
#include <iostream>

bool parse(Node* nodes)
{
    std::vector<Node*> node_stack;
    Node* current = nodes;

    bool in_list = false;

    while (current != nullptr)
    {
        std::string val = get_token_string(current->t);

        switch (current->t.type)
        {
            case TokenType::LIST_START:
            case TokenType::SUB_LIST_START:
            {
                if (!node_stack.empty() && (node_stack.back()->t.type == TokenType::LIST_START || node_stack.back()->t.type == TokenType::SUB_LIST_START))
                {
                    error_msg(val.c_str(), "Nested lists are not permitted.");
                    return false;
                }
                node_stack.push_back(current);
                in_list = true;
                break;
            }

            case TokenType::LIST_END:
            {
                if (node_stack.empty() || node_stack.back()->t.type != TokenType::LIST_START)
                {
                    error_msg(val.c_str(), "Unopened list.");
                    return false;
                }
                
                node_stack.pop_back();
                in_list = false;
                break;
            }

            case TokenType::SUB_LIST_END:
            {
                if (node_stack.empty() || node_stack.back()->t.type != TokenType::SUB_LIST_START)
                {
                    error_msg(val.c_str(), "Unopened sub list.");
                    return false;
                }
                
                node_stack.pop_back();
                in_list = false;
                break;
            }

            case TokenType::DATA_String:
            {
                std::string s;
                bool escape_char = false;

                for (char c : get_token_string(current->t))
                {
                    if (escape_char)
                    {
                        if (c == 'n')
                            s += '\n';
                        else if (c == '\\')
                            s += '\\';
                        else if (c == 't')
                            s += "    ";
                        else if (c == '\"')
                            s += '\"';
                        
                        escape_char = false;
                        continue;
                    }
                    if (c == '\\')
                    {
                        escape_char = true;
                        continue;
                    }

                    s += c;
                }

                current->t.value = s;
                break;
            }

            case TokenType::COMMAND:
            {
                if (in_list)
                {
                    error_msg(val.c_str(), "Commands cannot be placed in lists.\nClose the leading list before executing commands.");
                    return false;
                }
                if (val == "defunc" || val == "if" || val == "loop" || val == "for" || val == "while" || val == "?")
                    node_stack.push_back(current);
                else if (val == "end")
                {
                    if (node_stack.empty() || node_stack.back()->t.type != TokenType::COMMAND)
                    {
                        error_msg(val.c_str(), "Unopened code block.");
                        return false;
                    }
                    current->alt_next = node_stack.back();
                    current->alt_next->alt_next = current;
                    node_stack.pop_back();
                }
                break;
            }

            case TokenType::OPERATOR:
            {
                if (in_list)
                {
                    error_msg(val.c_str(), "Operators cannot be placed in lists.\nClose the leading list before using operators.");
                    return false;
                }
            }
        }

        current = current->default_next;
    }

    if (!node_stack.empty())
    {
        if (node_stack.back()->t.type == TokenType::COMMAND)
            error_msg(get_token_string(node_stack.back()->t).c_str(), "Block not closed with 'end' command.");
        else if (node_stack.back()->t.type == TokenType::LIST_START)
            error_msg(get_token_string(node_stack.back()->t).c_str(), "Unclosed list.");
        else if (node_stack.back()->t.type == TokenType::SUB_LIST_START)
            error_msg(get_token_string(node_stack.back()->t).c_str(), "Unclosed sub list.");
        return false;
    }
    return true;
}
