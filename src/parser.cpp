#include "lang.hpp"
#include <vector>

bool parse(Node* nodes)
{
    std::vector<Node*> node_stack;
    Node* current = nodes;
    Token previous;

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
                        error_msg(current, "Nested lists are not permitted.");
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
                        error_msg(current, "Unopened list.");
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
                        error_msg(current, "Unopened sub list.");
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
                }
            case TokenType::DATA_Bool:
            case TokenType::DATA_Char:
            case TokenType::DATA_Number:
            case TokenType::CONSTANT:
                {
                    if (!in_list && !(previous.type == TokenType::COMMAND && get_command_enum(get_token_string(previous)) == CommandEnum::CACHE))
                    {
                        error_msg(current, "Stray value outside of list.");
                        return false;
                    }
                    break;
                }

            case TokenType::COMMAND:
                {
                    if (in_list)
                    {
                        error_msg(current, "Commands cannot be placed in lists.\nClose the leading list before executing commands.");
                        return false;
                    }
                    if (val == "defunc" || val == "if" || val == "loop" || val == "for" || val == "while" || val == "begin" || val == "?")
                        node_stack.push_back(current);
                    else if (val == "end")
                    {
                        if (node_stack.empty() || node_stack.back()->t.type != TokenType::COMMAND)
                        {
                            error_msg(current, "Unopened code block.");
                            return false;
                        }
                        current->alt_next = node_stack.back();
                        current->alt_next->alt_next = current;
                        node_stack.pop_back();
                    }
                    else if (val == "break" || val == "continue")
                    {
                        for (int i = node_stack.size() - 1; i >= 0; i--)
                        {
                            if (node_stack[i]->t.type != TokenType::COMMAND)
                                continue;

                            std::string tok = get_token_string(node_stack[i]->t);
                            if (tok == "loop" || tok == "for" || tok == "while")
                            {
                                current->alt_next = node_stack[i];
                                break;
                            }
                        }

                        if (current->alt_next == nullptr)
                        {
                            error_msg(current, "Break can only be used inside a loop.");
                            return false;
                        }
                    }
                    break;
                }

            case TokenType::OPERATOR:
                {
                    if (in_list)
                    {
                        error_msg(current, "Operators cannot be placed in lists.\nClose the leading list before using operators.");
                        return false;
                    }
                }
            default:
                break;
        }

        previous = current->t;
        current = current->default_next;
    }

    if (!node_stack.empty())
    {
        if (node_stack.back()->t.type == TokenType::COMMAND)
            error_msg(node_stack.back(), "Block not closed with 'end' command.");
        else if (node_stack.back()->t.type == TokenType::LIST_START)
            error_msg(node_stack.back(), "Unclosed list.");
        else if (node_stack.back()->t.type == TokenType::SUB_LIST_START)
            error_msg(node_stack.back(), "Unclosed sub list.");
        return false;
    }
    return true;
}
