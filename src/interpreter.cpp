#include "lang.hpp"
#include <algorithm>
#include <any>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <map>

std::vector<Token> reverse_list(std::vector<Token> list)
{
    std::vector<Token> res;

    while (!list.empty())
    {
        res.push_back(list.back());
        list.pop_back();
    }

    return res;
}

void print_list(std::vector<Token> list)
{
    std::cout << "Count " << list.size() << std::endl;

    if (list.empty())
        return;

    int i = 0;

    std::string last_item = "";
    bool match = false;

    for (Token t : list)
    {
        std::string item = get_token_string(t);

        if (item != last_item)
        {
            if (match)
                std::cout << "\n...";

            std::cout << "\n" << i << " : " << item << " ";
            match = false;
        }
        else
            match = true;

        i++;

        last_item = item;
    }

    if (match)
        std::cout << "\n...\n" << i << " : " << last_item << " ";

    std::cout << std::endl;
}

std::vector<Token> pop_list(std::vector<Token> &stack)
{
    std::vector<Token> list;
    bool end = false;

    while (!stack.empty() && stack.back().type != TokenType::LIST_START && stack.back().type != TokenType::FUNCTION_CALL)
    {
        list.push_back(stack.back());
        stack.pop_back();
    }

    if (!stack.empty() && stack.back().type == TokenType::FUNCTION_CALL)
        list.push_back(stack.back());

    stack.pop_back();

    return list;
}

void append_list(std::vector<Token> &base, std::vector<Token> list)
{
    for (Token t : list)
        base.push_back(t);
}

void push_list(std::vector<Token> &stack, std::vector<Token> list)
{
    if (list.empty())
        return;

    if (list.front().type != TokenType::FUNCTION_CALL)
    {
        Token start;
        start.type = TokenType::LIST_START;
        start.value = "[";
        stack.push_back(start);
    }

    append_list(stack, list);
}

Token get_tag(std::vector<Token> list, Token tag)
{
    if (!is_tag(tag))
        return tag;

    int pos = find_tag(list, tag);
    if (pos >= 0 && pos < list.size() - 1)
        return list.at(pos + 1);

    return tag;
}

bool interpret(std::string executable_path, std::string program_path, Node* program, std::vector<Token> &backup_stack)
{
    Node* current = program;
    bool exception = false;
    std::string exception_message = "";
    std::map<std::string, Function> functions;
    std::map<std::string, Token> constants;

    current = program;

    std::vector<Token> stack;
    std::vector<std::string> includes;

    std::string stdlibpath = executable_path + "libs/std/";

    for (Token t : backup_stack)
        stack.push_back(t);

    bool in_list = false;
    Token temp;

    auto start = std::chrono::high_resolution_clock::now();
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
                if (stack.empty() || (stack.front().type != TokenType::LIST_START && stack.front().type != TokenType::FUNCTION_CALL))
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
            case TokenType::TAG_BLOCK:
            case TokenType::TAG_MEMBER:
            case TokenType::DATA_String:
            case TokenType::DATA_Char:
            case TokenType::DATA_Number:
            case TokenType::DATA_Bool:
            case TokenType::CONSTANT:
            {
                
                if (in_list)
                {
                    if (current->t.type == TokenType::CONSTANT)
                    {
                        if (constants.count(get_token_string(current->t)) == 1)
                        {
                            stack.push_back(constants.at(get_token_string(current->t)));
                            break;
                        }
                        exception = true;
                        exception_message = "Undefined constant.\nConstants must be defined before being used.";
                        break;
                    }

                    stack.push_back(current->t);
                    break;
                } else {
                    exception = true;
                    exception_message = "Stray value.\nValues must be pushed to the stack as part of a list.";
                    break;
                }
            }

            case TokenType::OPERATOR:
            {
                auto equal = [&exception, &exception_message](Token a, Token b)
                {
                    if (a.type == b.type)
                    {
                        if (a.type == TokenType::DATA_Bool)
                            return std::any_cast<bool>(a.value) == std::any_cast<bool>(b.value);
                        if (a.type == TokenType::DATA_Char)
                            return std::any_cast<char>(a.value) == std::any_cast<char>(b.value);
                        if (a.type == TokenType::DATA_String)
                            return std::any_cast<std::string>(a.value) == std::any_cast<std::string>(b.value);
                        if (a.type == TokenType::DATA_Number)
                            return std::any_cast<float>(a.value) == std::any_cast<float>(b.value);
                    }

                    exception = true;
                    exception_message = "Mismatched Types";
                    return false;
                };
                
                Token res;
                bool res_set = false;
                
                auto set_res = [&res, &res_set](bool value = false)
                {
                    res.type = TokenType::DATA_Bool;
                    res.value = value;
                    res_set = true;
                };

                std::vector<Token> values = reverse_list(pop_list(stack));

                if (command == "=")
                {
                    std::vector<Token> destination = pop_list(stack);

                    if (destination.size() == 1 && (destination.back().type == TokenType::DATA_Number || is_tag(destination.back())))
                    {
                        Token tok = destination.back();
                        int pos = 0;

                        pos = is_tag(tok) ? find_tag(stack, tok) + 1 : std::any_cast<float>(tok.value);

                        if (pos < stack.size())
                        {
                            stack.at(pos) = values.front();
                            break;
                        }

                        exception = true;
                        exception_message = "Position not on stack";
                        break;
                    } else {
                        exception = true;
                        exception_message = "Expected a single integer value or tag as the destination.";
                        break;
        	        }
                } else if (command == "+" || command == "-" || command == "*" || command == "/")
                {
                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (is_value(val))
                        {
                            if (res.type == TokenType::NULL_TOKEN)
                            {
                                res = val;
                                continue;
                            }

                            if (res.type == TokenType::DATA_Number && val.type == TokenType::DATA_Number)
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
                                if (command == "+")
                                    res.value = std::any_cast<std::string>(res.value) + get_token_string(val);
                                else {
                                    exception = true;
                                    exception_message = "Invalid string operation.";
                                    break;
                                }
                            } else {
                                exception = true;
                                exception_message = "Value types do not agree.";
                                break;
                            }
                        } else {
                            exception = true;
                            
                            switch (res.type)
                            {
                                case TokenType::DATA_Number:
                                    exception_message = get_token_string(val) + " : Expected a numeric value.";
                                    break;

                                case TokenType::DATA_String:
                                    exception_message = get_token_string(val) + " : Expected a numeric or string value.";
                                    break;

                                default:
                                    break;
                            }

                            break;
                        }
                    }
                    if (!exception)
                        push_list(stack, {res});
                } else if (command == "==" || command == "!=" || command == ">" || command == ">=" || command == "<" || command == "<=")
                {
                    auto greater = [](Token a, Token b)
                    {
                        if (a.type == b.type)
                        {
                            if (a.type == TokenType::DATA_Bool)
                                return std::any_cast<bool>(a.value) > std::any_cast<bool>(b.value);
                            if (a.type == TokenType::DATA_Char)
                                return std::any_cast<char>(a.value) > std::any_cast<char>(b.value);
                            if (a.type == TokenType::DATA_String)
                                return std::any_cast<std::string>(a.value) > std::any_cast<std::string>(b.value);
                            if (a.type == TokenType::DATA_Number)
                                return std::any_cast<float>(a.value) > std::any_cast<float>(b.value);
                        }
                        return false;
                    };

                    Token val;
                    for (Token current_val : values)
                    {
                        val = get_tag(stack, current_val);

                        if (!is_tag(val))
                        {
                            switch (res.type)
                            {
                                case TokenType::NULL_TOKEN:
                                {
                                    res = val;
                                    break;
                                }

                                default:
                                {
                                    if (res.type == val.type || res.type == TokenType::DATA_String)
                                    {
                                        if (command == "==" && !equal(res, val))
                                            set_res();
                                        else if (command == "!=" && equal(res, val))
                                            set_res();
                                        else if (command == ">" && !greater(res, val))
                                            set_res();
                                        else if (command == ">=" && !greater(res, val) && !equal(res, val))
                                            set_res();
                                        else if (command == "<" && greater(res, val))
                                            set_res();
                                        else if (command == "<=" && greater(res, val) && !equal(res, val))
                                            set_res();
                                    } else {
                                        exception = true;
                                        exception_message = TokenTypeString[res.type] + " " + TokenTypeString[val.type] + " : Can only compare values of matching types.";
                                    }
                                    break;
                                }
                            }
                        } else {
                            exception = true;
                            exception_message = get_token_string(val) + ": Tag not found.";
                            break;
                        }

                        if (exception)
                            break;
                    }

                    if (exception == true)
                        break;

                    res.type = TokenType::DATA_Bool;
                    
                    if (!res_set)
                        res.value = true;
                    else
                        res.value = false;
                    
                    push_list(stack, {res});
                } else if (command == "and" || command == "or")
                {
                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (is_tag(val))
                        {
                            exception = true;
                            exception_message = get_token_string(val) + ": Tag not found.";
                            break;
                        }

                        if (val.type == TokenType::DATA_Bool)
                        {
                            if (res.type == TokenType::NULL_TOKEN)
                            {
                                res = val;
                                continue;
                            }

                            if (command == "and" && !equal(res, val))
                                set_res();
                            else if (command == "or" && (std::any_cast<bool>(res.value) == false && std::any_cast<bool>(val.value) == false))
                                set_res();
                        
                            res.type = TokenType::DATA_Bool;

                            if (res_set)
                                break;
                        } else {
                            exception = true;
                            exception_message = "Can only compare booleans.";
                            break;
                        }
                    }

                    if (exception == true)
                        break;

                    res.type = TokenType::DATA_Bool;
                    res.value = !res_set;

                    push_list(stack, {res});
                }
                break;
            }

            case TokenType::COMMAND:
            {
                CommandEnum command_enum = get_command_enum(command);

                switch (command_enum)
                {
                    case CommandEnum::PRINTLN:
                    case CommandEnum::PRINT:
                    {
                        std::vector<Token> list = pop_list(stack);
                    
                        for (Token t : reverse_list(list))
                        {
                            Token val = get_tag(stack, t);

                            if (is_value(val))
                            {
                                std::cout << get_token_string(val) << (command_enum == PRINTLN ? "\n" : "");
                                std::cout.flush();
                            } else {
                                exception = true;
                                exception_message = get_token_string(val) + ": Tag not found.";
                                break;
                            }
                        }
                        break;
                    }

                    case CommandEnum::INPUT:
                    {
                        std::string val;
                        std::getline(std::cin, val);
    
                        Token t;
                        t.value = val;
                        t.type = TokenType::DATA_String;
    
                        if (!val.empty())
                        {
                            if (val.at(0) >= '0' && val.at(0) <= '9')
                            {
                                t.value = std::stof(val);
                                t.type = TokenType::DATA_Number;
                            } else if (val == "true" || val == "false")
                            {
                                t.value = (val == "true");
                                t.type = TokenType::DATA_Bool;
                            }
                        }

                        push_list(stack, {t});
                        break;
                    }

                    case CommandEnum::PRINT_STACK:
                    {
                        print_list(stack);
                        break;
                    }

                    case CommandEnum::DROP:
                    {
                        stack.pop_back();
                        break;
                    }

                    case CommandEnum::DROP_LIST:
                    {
                        pop_list(stack);
                        break;
                    }

                    case CommandEnum::AT:
                    {
                        std::vector<Token> list = reverse_list(pop_list(stack));
                        // GLOBAL_TAG | LOCAL_TAG | MEMBER_TAG | DATA_String
                        // INT
    
                        if (!list.empty())
                        {
                            int pos = find_tag(stack, list.at(0));
    
                            if (pos < 0 && list.front().type != TokenType::DATA_String)
                            {
                                exception = true;
                                exception_message = get_token_string(list.at(0)) + ": Tag not found.";
                                break;
                            }
    
                            int offset = 0;
    
                            if (list.size() == 2)
                            {
                                offset = list.at(1).type == TokenType::DATA_Number ? int(std::any_cast<float>(list.at(1).value)) : 0;

                                if (list.at(1).type != TokenType::DATA_Number)
                                {
                                    int o_pos = find_tag(stack, list.at(1));

                                    if (o_pos >= 0)
                                    {
                                        if (list.at(1).type == TokenType::TAG_MEMBER)
                                            offset = o_pos;
                                        else if (is_tag(list.at(1)))
                                        {
                                            if (stack.at(o_pos + 1).type == TokenType::DATA_Number)
                                                offset = int(std::any_cast<float>(stack.at(o_pos + 1).value));
                                            else
                                            {
                                                exception = true;
                                                exception_message = "Offset tag does not mark a Number.";
                                                break;
                                            }
                                        }
                                    } else {
                                        exception = true;
                                        exception_message = "Offset tag not found.";
                                        break;
                                    }
                                }
                            }
    
                            if (offset >= 0)
                            {
                                Token res;
    
                                if (list.front().type != TokenType::DATA_String)
                                {
                                    if (pos + offset + 1 > stack.size() - 1 || pos + offset + 1 < 0)
                                    {
                                        exception = true;
                                        exception_message = std::to_string(pos + offset + 1) + ": Index not found in stack.";
                                        break;
                                    }

                                    int distance = std::abs((offset + 1) - pos);
                                    int end = pos + offset + 1;
    
                                    for (int i = 0; i < ((offset + 1) / 2) + ((offset + 1) % 2); i++)
                                    {
                                        if (stack.at(pos + i).type >= TokenType::LIST_START || stack.at(end - i).type >= TokenType::LIST_START)
                                        {
                                            exception = true;
                                            exception_message = std::to_string(i) + ": Index crosses into another list.";
                                            break;
                                        }
                                    }
    
                                    res.type = TokenType::DATA_Number;
                                    res.value = float(pos + offset + 1);
                                } else {
                                    if (offset < std::any_cast<std::string>(list.front().value).length())
                                    {
                                        res.type = TokenType::DATA_Char;
                                        res.value = std::any_cast<std::string>(list.front().value).at(offset);
                                    } else {
                                        exception = true;
                                        exception_message = "Index not found in provided string.";
                                        break;
                                    }
                                }
    
                                push_list(stack, {res});
                            } else {
                                exception = true;
                                exception_message = "Offset cannot be less than 0";
                                break;
                            }
                        } else {
                            exception = true;
                            exception_message = "Expected a Tag as the first item.";
                            break;
                        }
                        break;
                    }

                    case CommandEnum::GET:
                    {
                        std::vector<Token> list = pop_list(stack);
    
                        if (list.size() == 1)
                        {
                            Token tok = list.back();

                            if (is_tag(tok))
                            {
                                if (is_value(get_tag(stack, tok)))
                                    push_list(stack, {get_tag(stack, tok)});
                                else
                                {
                                    exception = true;
                                    exception_message = "Provided index does not exist on the stack.";
                                    break;
                                }
                            } else if (tok.type == TokenType::DATA_Number)
                            {
                                int pos = int(std::any_cast<float>(tok.value));

                                if (pos >= 0 && pos < stack.size())
                                {
                                    if (is_value(stack.at(pos)))
                                        push_list(stack, {stack.at(pos)});
                                    else
                                    {
                                        Token t_type;
                                        t_type.type = TokenType::DATA_String;
                                        t_type.value = TokenTypeString[stack.at(pos).type];
                                        push_list(stack, {t_type});
                                    }
                                } else {
                                    exception = true;
                                    exception_message = "Provided index does not exist on the stack.";
                                    break;
                                }
                            }
                        } else {
                            exception = true;
                            exception_message = "Expected a tag or a single integer value.";
                            break;
            	        }
    
                        break;
                    }

                    case CommandEnum::GET_LIST:
                    case CommandEnum::GET_LIST_VALUES:
				    {
                        std::vector<Token> tag = pop_list(stack);
    
                        if (tag.size() == 1 && tag.back().type == TokenType::DATA_Number)
                        {
                            int pos = int(std::any_cast<float>(tag.front().value));

                            std::vector<Token> list;

                            for (int i = pos; i < stack.size(); i++)
                            {
                                Token tok = stack.at(i);
                                
                                if (command_enum != CommandEnum::GET_LIST_VALUES && !is_tag(tok))
                                {
                                    if (tok.type != TokenType::LIST_START && tok.type != TokenType::FUNCTION_CALL)
                                        list.push_back(tok);
                                    else
                                        break;
                                }
                            }

                            push_list(stack, list);
                            break;
                        }

                        exception = true;
                        exception_message = "Expected a single integer value.";
                        break;
				    }

                    case CommandEnum::MERGE:
                    {
				        append_list(stack, reverse_list(pop_list(stack)));
                        break;
                    }

                    case CommandEnum::MERGE_X:
                    {
                        std::vector<Token> args = pop_list(stack);
                        Token count;

                        if (args.size() == 1)
                            count = get_tag(stack, args.back());
                        else
                        {
                            exception = true;
                            exception_message = "Expected a single element list, containing a numeric value.";
                            break;
                        }
    
                        if (count.type == TokenType::DATA_Number)
                        {
                            std::vector<Token> result;
                        
                            for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                            {
                                std::vector<Token> b = reverse_list(pop_list(stack));
                                append_list(b, result);
                                result = b;
                            }
    
                            push_list(stack, result);
                        } else {
                            exception = true;
                            exception_message = "Expected a single element list, containing a numeric value.";
                            break;
                        }
                        break;
                    }

                    case CommandEnum::INT:
                    {
                        std::vector<Token> values = pop_list(stack);
    
                        for (int i = 0; i < values.size() - 1; i++)
                        {
                            if (values.at(i).type == TokenType::DATA_Number)
                                values.at(i).value = float(int(std::any_cast<float>(values.at(i).value)));
                        }
    
                        push_list(stack, reverse_list(values));
                        break;
                    }

                    case CommandEnum::IF:
                    case CommandEnum::ERROR_HANDLER:
                    {
                        if (command == "if")
                        {
                            std::vector<Token> condition = pop_list(stack);
    
                            if (condition.size() == 1 && condition.front().type == TokenType::DATA_Bool)
                            {
                                if (std::any_cast<bool>(condition.front().value) == false)
                                {
                                    current = current->alt_next;
                                    skip_end = true;
                                }
                            } else {
                                exception = true;
                                exception_message = "Expected a single Boolean value.";
                                break;
                            }
                            
                            Token block;
                            block.type = TokenType::CONDITION_BLOCK;
                            push_list(stack, {block});
                        } else {
                            Token block;
                            block.type = TokenType::CONDITION_BLOCK;
                            push_list(stack, {block});
                            
                            if (exception)
                            {
                                exception = false;
                                Token e_msg;
                                e_msg.type = TokenType::DATA_String;
                                e_msg.value = exception_message;
                                
                                push_list(stack, {e_msg});
                            } else {
                                current = current->alt_next;
                                skip_end = true;
                            }
                        }
                        break;
                    }

                    case CommandEnum::BEGIN:
                    {
                        Token block;
                        block.type = TokenType::BLOCK;
                        push_list(stack, {block});
                        break;
                    }

                    case CommandEnum::LOOP:
                    {
                        std::vector<Token> list = pop_list(stack);
    
                        if (!list.empty())
                        {
                            list.front() = get_tag(stack, list.front());
                        } else {
                            exception = true;
                            exception_message = "Expected a single element list, containing an integer value.";
                            break;
                        }
    
                        if (list.front().type == TokenType::DATA_Number)
                        {
                            if (int(std::any_cast<float>(list.front().value)) == 0)
                            {
                                current = current->alt_next->default_next;
                                skip_end = true;
                                break;
                            }
                        } else {
                            exception = true;
                            exception_message = "Expected a single element list, containing an integer value.";
                            break;
                        }

                        Token count;
                        count = list.front();
                        count.value = std::any_cast<float>(count.value) - 1;
                        push_list(stack, {count});
    
                        Token block;
                        block.type = TokenType::LOOP_BLOCK;
                        push_list(stack, {block});
                        break;
                    }

                    case CommandEnum::WHILE:
                    {
                        std::vector<Token> list = pop_list(stack);

                        if (list.size() == 1 && list.back().type == TokenType::DATA_Bool)
                        {
                            if (std::any_cast<bool>(list.back().value) == false)
                            {
                                current = current->alt_next->default_next;
                                skip_end = true;
                                break;
                            }
                        } else {
                            exception = true;
                            exception_message = "Expected a single element list, containing a boolean value.";
                            exception_message += "\nA leading tag may be used as a way to access this value.";
                            break;
                        }

                        push_list(stack, reverse_list(list));
    
                        Token block;
                        block.type = TokenType::LOOP_BLOCK;
                        push_list(stack, {block});
                        break;
                    }                   

                    case CommandEnum::FOR:
                    {
                        std::vector<Token> list = reverse_list(pop_list(stack));
    
                        if (list.size() >= 3 && list.size() <= 4)
                        {
                            Token step = list.back();
                            list.pop_back();
                            step = get_tag(stack, step);
                            float step_f;

                            Token end_val = list.back();
                            list.pop_back();
                            end_val = get_tag(stack, end_val);
                            float end_f;

                            Token current_val = list.back();
                            list.pop_back();
                            current_val = get_tag(stack, current_val);
                            float current_f;

                            if (step.type == TokenType::DATA_Number)
                                step_f = std::any_cast<float>(step.value);
                            else
                            {
                                exception = true;
                                exception_message = "Expected a Number for step.";
                                break;
                            }

                            if (end_val.type == TokenType::DATA_Number)
                                end_f = std::any_cast<float>(end_val.value);
                            else
                            {
                                exception = true;
                                exception_message = "Expected a Number for end value.";
                                break;
                            }
    
                            if (current_val.type == TokenType::DATA_Number)
                                current_f = std::any_cast<float>(current_val.value);
                            else
                            {
                                exception = true;
                                exception_message = "Expected a Number for start value.";
                                break;
                            }

                            Token tag;
                            tag.type = TokenType::NULL_TOKEN;
    
                            if (!list.empty())
                            {
                                tag = list.back();
                                list.pop_back();
                            }
    
                            if (step_f < 0)
                            {
                                if (current_f <= end_f)
                                {
                                    current = current->alt_next->default_next;
                                    skip_end = true;
                                    break;
                                }
                            } else if (current_f >= end_f)
                            {
                                current = current->alt_next->default_next;
                                skip_end = true;
                                break;
                            }
    
                            current_val.value = current_f + step_f;
    
                            Token block;
                            block.type = TokenType::LOOP_BLOCK;
                            block.value = TokenTypeString[TokenType::LOOP_BLOCK];
                        
                            if (tag.type != TokenType::NULL_TOKEN)
                                push_list(stack, {tag, current_val, end_val, step});
                            else
                                push_list(stack, {current_val, end_val, step});
    
                            push_list(stack, {block});
                            break;
                        }

                        exception = true;
                        exception_message = "Expected a list of 3 numeric values.";
                        exception_message += "\nList may begin with a tag to access the counter.";
                        break;
                    }

                    case CommandEnum::DEFUNC:
                    {
                        if (temp.type == TokenType::USER_FUNCTION)
                        {
                            if (functions.count(std::any_cast<std::string>(temp.value)) == 0)
                            {
                                std::vector<Token> function_args = pop_list(stack);
                                Function new_func;
                                new_func.location = current;
                                new_func.argument_tags = new Token[function_args.size()];
                                new_func.arg_count = function_args.size();
    
                                for (int i = 0; i < new_func.arg_count; i++)
                                    new_func.argument_tags[i] = function_args[i];
    
                                functions.insert({std::any_cast<std::string>(temp.value), new_func});
                                temp.type = TokenType::NULL_TOKEN;
                                current = current->alt_next;
                                break;
                            } else {
                                exception = true;
                                exception_message = "Duplicate definition: " + std::any_cast<std::string>(temp.value);
                                break;
                            }
                        } else {
                            exception = true;
                            exception_message = "User function identifier not provided";
                            break;
                        }
                    }

                    case CommandEnum::CHACHE:
                    {
                        current = current->default_next;
    
                        if (current == nullptr)
                        {
                            exception = true;
                            exception_message = "No value found.";
                            break;
                        }
    
                        temp.value = current->t.value;
                        temp.type = current->t.type;
                        break;
                    }

                    case CommandEnum::END:
                    case CommandEnum::RETURN:
                    {
                        if (command == "return" || (command == "end" && std::any_cast<std::string>(current->alt_next->t.value) == "defunc"))
                        {
                            std::vector<Token> ret_list;

                            if (command == "return")
                                ret_list = reverse_list(pop_list(stack));

                            std::vector<Token> list;

                            while (!stack.empty())
                            {
                                list = reverse_list(pop_list(stack));

                                if (list.empty())
                                    continue;

                                if (list.back().type == TokenType::FUNCTION_CALL)
                                    break;
                            }

                            current = std::any_cast<Node*>(list.back().value);
                            skip_end = true;
                    
                            if (!ret_list.empty())
                                push_list(stack, ret_list);

                            break;
                        } else if (command == "end")
                        {
                            std::string target = std::any_cast<std::string>(current->alt_next->t.value);
                            std::vector<Token> list = pop_list(stack);
        
                            switch (get_command_enum(target))
                            {
                                case CommandEnum::IF:
                                case CommandEnum::ERROR_HANDLER:
                                    while (!list.empty() && list.back().type != TokenType::CONDITION_BLOCK)
                                        list = reverse_list(pop_list(stack));
                                    break;
                                case CommandEnum::BEGIN:
                                    while (!list.empty() && list.back().type != TokenType::BLOCK)
                                        list = reverse_list(pop_list(stack));
                                    break;
                                case CommandEnum::LOOP:
                                case CommandEnum::WHILE:
                                case CommandEnum::FOR:
                                    while (!list.empty() && list.back().type != TokenType::LOOP_BLOCK)
                                        list = reverse_list(pop_list(stack));

                                    current = current->alt_next;
                                    skip_end = true;
                                    break;

                                default:
                                    break;
                            }
                        }
                        break;
                    }

                    case CommandEnum::BREAK:
                    {
                        std::string target = std::any_cast<std::string>(current->alt_next->t.value);
                        std::vector<Token> list = reverse_list(pop_list(stack));
    
                        while (!list.empty() && list.back().type != TokenType::LOOP_BLOCK)
                            list = reverse_list(pop_list(stack));
    
                        current = current->alt_next->alt_next;
                        break;
                    }   

                    case CommandEnum::CONTINUE:
                    {
                        std::string target = std::any_cast<std::string>(current->alt_next->t.value);
                        std::vector<Token> list = reverse_list(pop_list(stack));
    
                        while (!list.empty() && list.back().type != TokenType::LOOP_BLOCK)
                            list = reverse_list(pop_list(stack));
    
                        current = current->alt_next;
                        skip_end = true;
                        break;
                    }

                    case CommandEnum::SWAP:
                    {
                        if (stack.size() < 2)
                            break;

                        Token a = stack.back();
                        stack.pop_back();

                        Token b = stack.back();
                        stack.pop_back();

                        stack.push_back(a);
                        stack.push_back(b);
                        break;
                    }

                    case CommandEnum::SWAP_LIST:
                    {
                        std::vector<Token> list_a = reverse_list(pop_list(stack));
                        std::vector<Token> list_b = reverse_list(pop_list(stack));
                        push_list(stack, list_a);
                        push_list(stack, list_b);
                        break;
                    }

                    case CommandEnum::DUP:
                    {
                        if (!stack.empty())
                        {
                            Token tok = stack.back();
                            stack.push_back(tok);
                        } else {
                            exception = true;
                            exception_message = "Empty stack.";
                        }

                        break;
                    }

                    case CommandEnum::DUP_X:
                    {
                        std::vector<Token> list = pop_list(stack);
                        Token count;

                        if (list.size() == 1)
                            count = list.back();

                        if (count.type == TokenType::DATA_Number)
                        {
                            for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                                stack.push_back(stack.back());
                        } else {
                            exception = true;
                            exception_message = get_token_string(count) + ": Expected one integer value.";
                            break;
                        }
                        break;
                    }

                    case CommandEnum::DEFINE:
                    {
                        Token constant;

                        if (temp.type == TokenType::CONSTANT)
                            constant = temp;
                        else {
                            exception = true;
                            exception_message = "Constant identifier not provided";
                            break;
                        }

                        if (constants.count(std::any_cast<std::string>(constant.value)) == 0)
                        {
                            std::vector<Token> val = pop_list(stack);
                            constants.insert({std::any_cast<std::string>(constant.value), val.back()});
                            temp.type = TokenType::NULL_TOKEN;
                        } else {
                            exception = true;
                            exception_message = "Constant already exists.";
                            break;
                        }
                        break;
                    }

                    case CommandEnum::INCLUDE:
                    {
                        std::vector<Token> file_list = reverse_list(pop_list(stack));
    
                        if (file_list.empty())
                        {
                            exception = true;
                            exception_message = "No file path provided.";
                            break;
                        }
    
                        std::string new_code = "";
    
                        // Load files
                        for (Token f : file_list)
                        {
                            if (f.type != TokenType::DATA_String)
                            {
                                exception = true;
                                exception_message = "Expected a string.";
                                break;
                            }
    
                            std::string file_name = get_token_string(f);
    
                            if (std::find(includes.begin(), includes.end(), file_name) != includes.end())
                                continue;
    
                            includes.push_back(file_name);
    
                            std::string file_path;
                            
                            if (file_name.substr(0, 3) == "std")
                                file_path = stdlibpath + file_name;
                            else
                                file_path = program_path  + file_name;
                        
                            std::string file_content = load_file(file_path.c_str());
                            
                            if (file_content.empty())
                            {
                                exception = true;
                                exception_message = "Could not load file: " + file_path;
                                break;
                            }
    
                            new_code += file_content + "\n";
                        }
    
                        // Break if at least one item is not a valid string.
                        if (exception)
                            break;
    
                        Node* new_nodes = tokenize(new_code.c_str());

                        if (lex(new_nodes))
                        {
                            if (parse(new_nodes))
                            {
                                Node* next_half = current->default_next;
                                current->default_next = new_nodes;
    
                                Node* last_node = current;
    
                                while (last_node->default_next != nullptr)
                                    last_node = last_node->default_next;
    
                                last_node->default_next = next_half;
                            } else {
                                exception = true;
                                exception_message = "Parsing failed.";
                                break;
                            }
                        } else {
                            exception = true;
                            exception_message = "Lexical analysis failed.";
                            break;
                        }

                        break;
                    }

                    case CommandEnum::EXIT:
                    {
                        current = nullptr;
                        skip_end = true;
                        break;
                    }

                    default:
                    {
                        exception = true;
                        exception_message = command + " : Unknown command.";
                        break;
                    }
                }
                break;
            }

            case TokenType::USER_FUNCTION:
            {
                if (functions.count(command) == 0)
                {
                    exception = true;
                    exception_message = "Undefined function: " + command;
                    break;
                }

                std::vector<Token> function_args;

                for (int i = 0; i < functions.at(command).arg_count; i++)
                {
                    push_list(function_args, {functions.at(command).argument_tags[i]});
                    append_list(function_args, reverse_list(pop_list(stack)));
                }

                Token func;
                func.type = TokenType::FUNCTION_CALL;
                func.value = current->default_next;
                push_list(stack, {func});
                append_list(stack, function_args);

                current = functions.at(command).location;
                break;
            }
            default:
                break;
        }

        if (exception == true)
        {
            if (current->default_next != nullptr && current->default_next->t.type == TokenType::COMMAND)
            {
                if (std::any_cast<std::string>(current->default_next->t.value) == "?")
                {
                    current = current->default_next;
                    continue;
                }
            }

            break;
        }

        if (!skip_end)
            current = current->default_next;
    }
    
    auto stop = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\nExecution time: " << duration.count() / 1000.0f << " milliseconds" << std::endl;

    if (exception == true)
    {
        error_msg(current, exception_message.c_str());
        print_list(stack);
        std::cout << "\nStack reverted to backup upon error." << std::endl;
        return false;
    }

    backup_stack = stack;

    return true;
}
