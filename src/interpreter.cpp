#include "lang.hpp"
#include <iostream>

std::vector<Token> reverse_list(std::vector<Token> list)
{
    std::vector<Token> res;

    for (int i = list.size() - 1; i >= 0; i--)
        res.push_back(list.at(i));
    
    return res;
}

void print_list(std::vector<Token> list)
{
    int i = 0;

    for (Token t : list)
        std::cout << i++ << " : " << get_token_string(t) << std::endl;
}

std::vector<Token> pop_list(std::vector<Token> &stack)
{
    std::vector<Token> list;

    for (int i = stack.size() - 1; i >= 0; i--)
    {
        if (stack.at(i).type == TokenType::LIST_START)
        {
            stack.pop_back();
            break;
        }
        list.push_back(stack.back());
        stack.pop_back();
    }

    return list;
}

void append_list(std::vector<Token> &base, std::vector<Token> list)
{
    for (Token t : list)
        base.push_back(t);
}

void push_list(std::vector<Token> &stack, std::vector<Token> list)
{
    Token start;
    start.type = TokenType::LIST_START;
    start.value = "[";
    stack.push_back(start);

    append_list(stack, list);
}

bool interpret(Node* program)
{
    Node* current = program;
    std::vector<Token> stack;
    bool in_list = false;
    bool exception = false;
    std::string exception_message = "";

    while (current != nullptr)
    {
        if (current == nullptr)
            return false;

        // std::cout << get_token_string(current->t) << " : " << TokenTypeString[current->t.type] << std::endl;
        bool skip_end = false;
        std::string command = get_token_string(current->t);

        switch (current->t.type)
        {
            case TokenType::LIST_START:
                stack.push_back(current->t);
            case TokenType::SUB_LIST_START:
                if (stack.empty() || stack.front().type != TokenType::LIST_START)
                {
                    exception = true;
                    exception_message = "No lists are present on the stack.";
                    break;
                }
                in_list = true;
                break;
            
            case TokenType::LIST_END:
            case TokenType::SUB_LIST_END:
                in_list = false;
                break;
            
            case TokenType::TAG_GLOBAL:
            case TokenType::TAG_LOCAL:
            case TokenType::TAG_MEMBER:
            case TokenType::DATA_String:
            case TokenType::DATA_Char:
            case TokenType::DATA_Number:
            case TokenType::DATA_Bool:
            {
                if (in_list == false)
                {
                    exception = true;
                    exception_message = "Stray value.\nValues must be pushed to the stack as part of a list.";
                    break;
                } else
                    stack.push_back(current->t);
                
                break;
            }

            case TokenType::OPERATOR:
            {
                std::vector<Token> values = reverse_list(pop_list(stack));

                if (command == "=")
                {
                    std::vector<Token> destination = pop_list(stack);
                    
                    if (destination.size() != 1 || (destination.front().type != TokenType::DATA_Number && !is_tag(destination.front())))
                    {
                        exception = true;
                        exception_message = "Expected a single integer value or tag as the destination.";
                        break;
        	        }

                    int pos = 0;

                    if (is_tag(destination.front()))
                        pos += find_tag(stack, destination.front()) + 1;
                    else
                    {
                        pos += std::any_cast<float>(destination.front().value);
                    }

                    stack.at(pos) = values.front();
                } else if (command == "+" || command == "-" || command == "*" || command == "/")
                {
                    Token res;

                    for (Token current_val : values)
                    {
                        Token val = current_val;
                        if (is_tag(val))
                            val = stack.at(find_tag(stack, val) + 1);
                        
                        if (!is_value(val))
                        {
                            exception = true;
                            exception_message = "Expected a numeric value.";
                            break;
                        }
                        
                        if (res.type == TokenType::NULL_TOKEN)
                        {
                            res = val;
                            continue;
                        }

                        try
                        {
                            std::any_cast<float>(val.value);
                        } catch (std::exception& e)
                        {
                            exception = true;
                            exception_message = "Value types do not agree.";
                            break;
                        }

                        if (res.type == TokenType::DATA_Number)
                        {
                            if (command == "+")
                                res.value = std::any_cast<float>(res.value) + std::any_cast<float>(val.value);
                            else if (command == "-")
                                res.value = std::any_cast<float>(res.value) - std::any_cast<float>(val.value);
                            else if (command == "*")
                                res.value = std::any_cast<float>(res.value) * std::any_cast<float>(val.value);
                            else if (command == "/")
                            {
                                if (std::any_cast<float>(res.value) == 0)
                                {
                                    exception = true;
                                    exception_message = "Divide by Zero.";
                                    break;
                                }
                                res.value = std::any_cast<float>(res.value) / std::any_cast<float>(val.value);
                            }
                        } else if (res.type == TokenType::DATA_String)
                        {
                            if (command != "+")
                            {
                                exception = true;
                                exception_message = "Invalid string operation.";
                                break;
                            }

                            res.value = std::any_cast<std::string>(res.value) + get_token_string(val);
                        }
                    }
                    if (!exception)
                        push_list(stack, {res});
                }
                break;
            }

            case TokenType::COMMAND:
            {
                if (command == "print" || command == "println")
                {
                    std::vector<Token> list = pop_list(stack);
                    
                    for (Token t : reverse_list(list))
                    {
                        Token val = t;
                        if (val.type == TokenType::TAG_GLOBAL || val.type == TokenType::TAG_LOCAL || val.type == TokenType::TAG_MEMBER)
                        {
                            int pos = find_tag(stack, val);
                            if (pos < 0)
                            {
                                exception = true;
                                exception_message = "Tag not found.";
                                break;
                            }

                            val = stack.at(pos + 1);
                        }
                        std::cout << get_token_string(val);
                        if (command == "println")
                            std::cout << std::endl;
                    }
                } else if (command == "print-stack")
                {
                    print_list(stack);
                } else if (command == "drop")
                {
                    stack.pop_back();
                } else if (command == "drop-list")
                {
                    pop_list(stack);
                } else if (command == "at")
                {
                    std::vector<Token> list = reverse_list(pop_list(stack));
                    // GLOBAL_TAG | LOCAL_TAG | MEMBER_TAG
                    // INT

                    if (list.size() == 0)
                    {
                        exception = true;
                        exception_message = "Expected a Tag as the first item.";
                        break;
                    }

                    int pos = find_tag(stack, list.at(0));

                    if (pos < 0)
                    {
                        exception = true;
                        exception_message = "Tag not found.";
                        break;
                    }

                    int offset = 0;

                    if (list.size() == 2)
                    {
                        if (list.at(1).type == TokenType::DATA_Number)
                            offset = std::any_cast<float>(list.at(1).value);
                        else if (is_tag(list.at(1)))
                        {
                            int o_pos = find_tag(stack, list.at(1));
                            if (o_pos < 0)
                            {
                                exception = true;
                                exception_message = "Offset tag not found.";
                                break;
                            }

                            if (list.at(1).type == TokenType::TAG_MEMBER)
                                offset = o_pos;
                            else if (is_tag(list.at(1)))
                            {
                                if (stack.at(o_pos + 1).type != TokenType::DATA_Number)
                                {
                                    exception = true;
                                    exception_message = "Offset tag does not mark a Number.";
                                    break;
                                }
                                Token val;
                                val = stack.at(o_pos + 1);
                                offset = std::any_cast<float>(val.value);
                            }
                        }
                    }

                    if (offset < 0)
                    {
                        exception = true;
                        exception_message = "Offset cannot be less than 0";
                        break;
                    }

                    if (pos + offset + 1 > stack.size() - 1 || pos + offset + 1 < 0)
                    {
                        exception = true;
                        exception_message = "Index not found in stack.";
                        break;
                    }

                    for (int i = pos + offset + 1; i >= pos; i--)
                    {
                        if (stack.at(i).type >= TokenType::LIST_START)
                        {
                            exception = true;
                            exception_message = "Index crosses into another list.";
                            break;
                        }
                    }

                    Token res;
                    res.type = TokenType::DATA_Number;
                    res.value = float(pos + offset + 1);

                    push_list(stack, {res});
                } else if (command == "get")
                {
                    std::vector<Token> tag = pop_list(stack);

                    if (tag.size() != 1 || tag.front().type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a single integer value.";
                        break;
        	        }

                    push_list(stack, {stack.at(std::any_cast<float>(tag.front().value))});
                } else if (command == "get-list")
				{
                    std::vector<Token> tag = pop_list(stack);

                    if (tag.size() != 1 || tag.front().type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a single integer value.";
                        break;
        	        }

                    int pos = 0;
                    
                    try
                    {
                        pos = std::any_cast<float>(tag.front().value);
                    }
                    catch (std::exception& e)
                    {
                        pos = std::any_cast<int>(tag.front().value);
                    }

                    push_list(stack, {});

                    for (int i = pos; i < stack.size() - 1; i++)
                    {
                        if (!is_value(stack.at(i)))
                            break;

                        append_list(stack, {stack.at(i)});
                    }
				} else if (command == "merge")
                {
				    append_list(stack, reverse_list(pop_list(stack)));
                } else if (command == "int")
                {
                    std::vector<Token> values = pop_list(stack);

                    for (int i = 0; i < values.size() - 1; i++)
                    {
                        if (values.at(i).type == TokenType::DATA_Number)
                            values.at(i).value = float(int(std::any_cast<float>(values.at(i).value)));
                    }

                    push_list(stack, reverse_list(values));
                } else if (command == "if" || command == "?")
                {
                    if (command == "if")
                    {
                        std::vector<Token> condition = pop_list(stack);

                        if (condition.front().type != TokenType::DATA_Bool || condition.size() > 1)
                        {
                            exception = true;
                            exception_message = "Expected a singal Boolean value.";
                            break;
                        }

                        if (std::any_cast<bool>(condition.front().value) == false)
                        {
                            current = current->alt_next;
                            skip_end = true;
                        }
                    } else {
                        if (!exception)
                        {
                            current = current->alt_next;
                            skip_end = true;
                        }

                        exception = false;
                    }

                    Token block;
                    block.type = TokenType::CONDITION_BLOCK;
                    block.value = TokenTypeString[TokenType::CONDITION_BLOCK];
                    push_list(stack, {block});
                } else if (command == "end")
                {
                    if (std::any_cast<std::string>(current->alt_next->t.value) == "if" || std::any_cast<std::string>(current->alt_next->t.value) == "?")
                    {
                        std::vector<Token> list = reverse_list(pop_list(stack));

                        while (!list.empty() && list.back().type != TokenType::CONDITION_BLOCK)
                            list = reverse_list(pop_list(stack));
                    }
                }
                break;
            }
        }

        if (exception == true)
        {
            if (current->default_next->t.type == TokenType::COMMAND)
            {
                if (std::any_cast<std::string>(current->default_next->t.value) == "?")
                {
                    Token e_msg;
                    e_msg.type = TokenType::DATA_String;
                    e_msg.value = exception_message;
                }
            } else
                break;
        }

        if (!skip_end)
            current = current->default_next;
    }

    if (exception == true)
    {
        error_msg(get_token_string(current->t).c_str(), exception_message.c_str());
        print_list(stack);
        return false;
    }

    return true;
}
