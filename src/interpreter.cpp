#include "lang.hpp"
#include <algorithm>
#include <any>
#include <atomic>
#include <chrono>
#include <iostream>
#include <map>

std::vector<Token> reverse_list(std::vector<Token> list)
{
    std::vector<Token> res;

    for (int i = list.size() - 1; i >= 0; i--)
        res.push_back(list.at(i));
   
    return res;
}

void print_list(std::vector<Token> list)
{
    std::cout << "Count " << list.size() << std::endl;

    if (list.empty())
        return;

    int i = 0;

    for (Token t : list)
    {
        std::cout << i++ << " : " << get_token_string(t) << std::endl;
    }
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

Token get_tag(std::vector<Token> list, Token tag)
{
    if (is_tag(tag))
    {
        int pos = find_tag(list, tag);
        if (pos >= 0 && pos < list.size() - 1)
            return list.at(pos + 1);
    }

    return tag;
}

bool interpret(std::string program_path, Node* program, std::vector<Token> &backup_stack)
{
    auto start = std::chrono::high_resolution_clock::now();
    Node* current = program;
    bool exception = false;
    std::string exception_message = "";
    std::map<std::string, Node*> functions;
    std::map<std::string, Token> constants;

    current = program;

    std::vector<Token> stack;
    std::vector<std::string> includes;

    for (Token t : backup_stack)
        stack.push_back(t);

    bool in_list = false;
    Token temp;

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
            case TokenType::TAG_BLOCK:
            case TokenType::TAG_MEMBER:
            case TokenType::DATA_String:
            case TokenType::DATA_Char:
            case TokenType::DATA_Number:
            case TokenType::DATA_Bool:
            case TokenType::CONSTANT:
            {
                if (in_list == false)
                {
                    exception = true;
                    exception_message = "Stray value.\nValues must be pushed to the stack as part of a list.";
                    break;
                } else {
                    if (current->t.type == TokenType::CONSTANT)
                    {
                        if (constants.count(get_token_string(current->t)) == 0)
                        {
                            exception = true;
                            exception_message = "Undefined constant.\nConstants must be defined before being used.";
                            break;
                        }
                        stack.push_back(constants.at(get_token_string(current->t)));
                    } else {
                        stack.push_back(current->t);
                    }
                }
                break;
            }

            case TokenType::OPERATOR:
            {
                auto equal = [&exception, &exception_message](Token a, Token b)
                {
                    if (a.type != b.type)
                    {
                        exception = true;
                        exception_message = "Mismatched Types";
                        return false;
                    }
                    if (a.type == TokenType::DATA_Bool)
                        return std::any_cast<bool>(a.value) == std::any_cast<bool>(b.value);
                    if (a.type == TokenType::DATA_Char)
                        return std::any_cast<char>(a.value) == std::any_cast<char>(b.value);
                    if (a.type == TokenType::DATA_String)
                        return std::any_cast<std::string>(a.value) == std::any_cast<std::string>(b.value);
                    if (a.type == TokenType::DATA_Number)
                        return std::any_cast<float>(a.value) == std::any_cast<float>(b.value);

                    return false;
                };
                
                Token res;
                bool res_set = false;
                
                auto set_res = [&res, &res_set]()
                {
                    res.type = TokenType::DATA_Bool;
                    res.value = false;
                    res_set = true;
                };

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

                    if (pos >= stack.size())
                    {
                        exception = true;
                        exception_message = "Position not on stack";
                        break;
                    }

                    stack.at(pos) = values.front();
                } else if (command == "+" || command == "-" || command == "*" || command == "/")
                {
                    for (Token current_val : values)
                    {
                        Token val = current_val;
                        val = get_tag(stack, val);

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

                        if (res.type != val.type && res.type != TokenType::DATA_String)
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
                } else if (command == "==" || command == "!=" || command == ">" || command == ">=" || command == "<" || command == "<=")
                {
                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (is_tag(val))
                        {
                            exception = true;
                            exception_message = "Tag not found.";
                            break;
                        }

                        if (res.type == TokenType::NULL_TOKEN)
                        {
                            res = val;
                            continue;
                        }

                        if (res.type != val.type)
                        {
                            exception = true;
                            exception_message = "Can only compare values of matching types.";
                            break;
                        }

                        
                        auto greater = [](Token a, Token b)
                        {
                            if (a.type == TokenType::DATA_Bool)
                                return std::any_cast<bool>(a.value) > std::any_cast<bool>(b.value);
                            if (a.type == TokenType::DATA_Char)
                                return std::any_cast<char>(a.value) > std::any_cast<char>(b.value);
                            if (a.type == TokenType::DATA_String)
                                return std::any_cast<std::string>(a.value) > std::any_cast<std::string>(b.value);
                            if (a.type == TokenType::DATA_Number)
                                return std::any_cast<float>(a.value) > std::any_cast<float>(b.value);

                            return false;
                        };

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

                        if (res_set == true)
                            break;
                    }

                    if (exception == true)
                        break;

                    res.type = TokenType::DATA_Bool;
                    
                    if (!res_set)
                        res.value = true;
                    
                    push_list(stack, {res});
                } else if (command == "and" || command == "or")
                {
                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (is_tag(val))
                        {
                            exception = true;
                            exception_message = "Tag not found.";
                            break;
                        }

                        if (val.type != TokenType::DATA_Bool)
                        {
                            exception = true;
                            exception_message = "Can only compare booleans.";
                            break;
                        }


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

                        if (res_set == true)
                            break;
                    }

                    if (exception == true)
                        break;

                    res.type = TokenType::DATA_Bool;
                    if (res_set == false)
                        res.value = true;

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
                        Token val = get_tag(stack, t);
                        if (is_tag(val))
                        {
                            exception = true;
                            exception_message = "Tag not found.";
                            break;
                        }

                        std::cout << get_token_string(val);
                        if (command == "println")
                            std::cout << std::endl;
                    }
                    break;
                }

                if (command == "input")
                {
                    std::string val;
                    std::getline(std::cin, val);

                    Token t;
                    if (val.at(0) >= '0' && val.at(0) <= '9')
                    {
                        t.value = std::stof(val);
                        t.type = TokenType::DATA_Number;
                    } else if (val == "true")
                    {
                        t.value = true;
                        t.type = TokenType::DATA_Bool;
                    } else if (val == "false")
                    {
                        t.value = true;
                        t.type = TokenType::DATA_Bool;
                    } else {
                        t.value = val;
                        t.type = TokenType::DATA_String;
                    }

                    push_list(stack, {t});
                    break;
                }

                if (command == "print-stack")
                {
                    print_list(stack);
                    break;
                }

                if (command == "drop")
                {
                    stack.pop_back();
                    break;
                }

                if (command == "drop-list")
                {
                    pop_list(stack);
                    break;
                }

                if (command == "at")
                {
                    std::vector<Token> list = reverse_list(pop_list(stack));
                    // GLOBAL_TAG | LOCAL_TAG | MEMBER_TAG | DATA_String
                    // INT

                    if (list.size() == 0)
                    {
                        exception = true;
                        exception_message = "Expected a Tag as the first item.";
                        break;
                    }

                    int pos = find_tag(stack, list.at(0));

                    if (pos < 0 && list.at(0).type != TokenType::DATA_String)
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
                        else
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

                    Token res;

                    if (list.at(0).type != TokenType::DATA_String)
                    {
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

                        res.type = TokenType::DATA_Number;
                        res.value = float(pos + offset + 1);
                    } else {
                        if (offset > std::any_cast<std::string>(list.at(0).value).length())
                        {
                            exception = true;
                            exception_message = "Index not found in provided string.";
                            break;
                        }

                        res.type = TokenType::DATA_Char;
                        res.value = std::any_cast<std::string>(list.at(0).value).at(offset);
                    }

                    push_list(stack, {res});
                    break;
                }

                if (command == "get")
                {
                    std::vector<Token> tag = pop_list(stack);

                    if (tag.size() != 1 || tag.front().type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a single integer value.";
                        break;
        	        }

                    if (std::any_cast<float>(tag.front().value) >= stack.size() || stack.size() == 0)
                    {
                        exception = true;
                        exception_message = "Provided index does not exist on the stack.";
                        break;
                    }

                    if (!is_value(stack.at(std::any_cast<float>(tag.front().value))))
                    {
                        Token t_type;
                        t_type.type = TokenType::DATA_String;
                        t_type.value = TokenTypeString[stack.at(std::any_cast<float>(tag.front().value)).type];
                        push_list(stack, {t_type});
                    } else
                        push_list(stack, {stack.at(std::any_cast<float>(tag.front().value))});

                    break;
                }

                if (command == "get-list" || command == "get-list-values")
				{
                    std::vector<Token> tag = pop_list(stack);

                    if (tag.size() != 1 || tag.front().type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a single integer value.";
                        break;
        	        } int pos = 0;
                    
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
                        if (stack.at(i).type == TokenType::LIST_START)
                            break;

                        if (command == "get-list-values" && is_tag(stack.at(i)))
                            continue;
                        
                        append_list(stack, {stack.at(i)});
                    }
                    break;
				}

                if (command == "merge")
                {
				    append_list(stack, reverse_list(pop_list(stack)));
                    break;
                }

                if (command == "int")
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

                if (command == "if" || command == "?")
                {
                    if (command == "if")
                    {
                        std::vector<Token> condition = pop_list(stack);

                        if (condition.empty())
                        {
                            exception = true;
                            exception_message = "Expected a single Boolean value.";
                            break;
                        }

                        if (condition.size() > 1 || condition.front().type != TokenType::DATA_Bool)
                        {
                            exception = true;
                            exception_message = "Expected a single Boolean value.";
                            break;
                        }

                        if (std::any_cast<bool>(condition.front().value) == false)
                        {
                            current = current->alt_next;
                            skip_end = true;
                        }
                        
                        Token block;
                        block.type = TokenType::CONDITION_BLOCK;
                        block.value = TokenTypeString[TokenType::CONDITION_BLOCK];
                        push_list(stack, {block});
                    } else {
                        if (!exception)
                        {
                            current = current->alt_next;
                            skip_end = true;
                        } else {
                            exception = false;
                            Token e_msg;
                            e_msg.type = TokenType::DATA_String;
                            e_msg.value = exception_message;
                            
                            Token block;
                            block.type = TokenType::CONDITION_BLOCK;
                            block.value = TokenTypeString[TokenType::CONDITION_BLOCK];

                            push_list(stack, {block});
                            push_list(stack, {e_msg});
                        }
                    }
                    break;
                }

                if (command == "begin")
                {
                    Token block;
                    block.type = TokenType::BLOCK;
                    block.value = TokenTypeString[TokenType::BLOCK];
                    push_list(stack, {block});
                    break;
                }

                if (command == "loop")
                {
                    std::vector<Token> list = pop_list(stack);

                    if (list.empty())
                    {
                        exception = true;
                        exception_message = "Expected a single element list, containing an integer value.";
                        break;
                    }

                    list.front() = get_tag(stack, list.front());

                    if (list.front().type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a single element list, containing an integer value.";
                        break;
                    }
                    
                    if (int(std::any_cast<float>(list.front().value)) == 0)
                    {
                        current = current->alt_next->default_next;
                        skip_end = true;
                        break;
                    }

                    Token count;
                    count = list.front();
                    count.value = std::any_cast<float>(count.value) - 1;
                    push_list(stack, {count});

                    Token block;
                    block.type = TokenType::LOOP_BLOCK;
                    block.value = TokenTypeString[TokenType::LOOP_BLOCK];
                    push_list(stack, {block});
                    break;
                }

                if (command == "while")
                {
                    std::vector<Token> list = pop_list(stack);

                    if (list.empty() || list.front().type != TokenType::DATA_Bool)
                    {
                        exception = true;
                        exception_message = "Expected a single element list, containing a boolean value.";
                        exception_message += "\nA leading tag may be used as a way to access this value.";
                        break;
                    }

                    if (std::any_cast<bool>(list.front().value) == false)
                    {
                        current = current->alt_next->default_next;
                        skip_end = true;
                        break;
                    }

                    push_list(stack, reverse_list(list));

                    Token block;
                    block.type = TokenType::LOOP_BLOCK;
                    block.value = TokenTypeString[TokenType::LOOP_BLOCK];
                    push_list(stack, {block});
                    break;
                }

                if (command == "for")
                {
                    std::vector<Token> list = reverse_list(pop_list(stack));

                    if (list.empty() || list.size() < 3)
                    {
                        exception = true;
                        exception_message = "Expected a list of 3 numeric values.";
                        exception_message += "\nList may begin with a tag to access the counter.";
                        break;
                    }

                    Token step = list.back();
                    list.pop_back();
                    step = get_tag(stack, step);
                    if (step.type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a Number for step.";
                        break;
                    }

                    Token end_val = list.back();
                    list.pop_back();
                    end_val = get_tag(stack, end_val);
                    if (end_val.type != TokenType::DATA_Number)
                    {
                        exception = true;
                        exception_message = "Expected a Number for end value.";
                        break;
                    }

                    Token current_val = list.back();
                    list.pop_back();
                    current_val = get_tag(stack, current_val);
                    if (current_val.type != TokenType::DATA_Number)
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

                    if (std::any_cast<float>(step.value) < 0)
                    {
                        if (std::any_cast<float>(current_val.value) <= std::any_cast<float>(end_val.value))
                        {
                            current = current->alt_next->default_next;
                            skip_end = true;
                            break;
                        }
                    } else {
                        if (std::any_cast<float>(current_val.value) >= std::any_cast<float>(end_val.value))
                        {
                            current = current->alt_next->default_next;
                            skip_end = true;
                            break;
                        }
                    }

                    current_val.value = std::any_cast<float>(current_val.value) + std::any_cast<float>(step.value);

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

                if (command == "defunc")
                {
                    if (temp.type != TokenType::USER_FUNCTION)
                    {
                        exception = true;
                        exception_message = "User function identifier not provided";
                        break;
                    }

                    if (functions.count(std::any_cast<std::string>(temp.value)) > 0)
                    {
                        exception = true;
                        exception_message = "Duplicate definition: " + std::any_cast<std::string>(temp.value);
                        break;
                    }

                    functions.insert({std::any_cast<std::string>(temp.value), current});
                    temp.type = TokenType::NULL_TOKEN;
                    current = current->alt_next;
                    break;
                }

                if (command == "$")
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

                if (command == "return" || (command == "end" && std::any_cast<std::string>(current->alt_next->t.value) == "defunc"))
                {
                    std::vector<Token> ret_list = reverse_list(pop_list(stack));
                    std::vector<Token> list;
                    int stage = 0;

                    while (!stack.empty())
                    {
                        list = reverse_list(pop_list(stack));
                        
                        if (list.empty())
                            continue;

                        if (stage == 0 && list.back().type == TokenType::FUNCTION_CALL)
                            stage = 1;
                        else if (stage == 1 && list.back().type == TokenType::ADDRESS)
                            break;
                    }

                    if (list.back().type != TokenType::ADDRESS)
                    {
                        exception = true;
                        exception_message = "No return address";
                        break;
                    }

                    current = std::any_cast<Node*>(list.front().value);
                    skip_end = true;
                    
                    if (!ret_list.empty())
                        push_list(stack, ret_list);

                    break;
                }

                if (command == "end")
                {
                    std::string target = std::any_cast<std::string>(current->alt_next->t.value);
                    std::vector<Token> list = pop_list(stack);

                    if (target == "if" || target == "?")
                    {
                        while (!list.empty() && list.back().type != TokenType::CONDITION_BLOCK)
                            list = reverse_list(pop_list(stack));
                    } else if (target == "begin")
                    {
                        while (!list.empty() && list.back().type != TokenType::BLOCK)
                            list = reverse_list(pop_list(stack));
                    } else if (target == "loop" || target == "while" || target == "for")
                    {
                        while (!list.empty() && list.back().type != TokenType::LOOP_BLOCK)
                            list = reverse_list(pop_list(stack));

                        current = current->alt_next;
                        skip_end = true;
                        break;
                    }
                    break;
                }

                if (command == "break")
                {
                    std::string target = std::any_cast<std::string>(current->alt_next->t.value);
                    std::vector<Token> list = reverse_list(pop_list(stack));

                    while (!list.empty() && list.back().type != TokenType::LOOP_BLOCK)
                        list = reverse_list(pop_list(stack));

                    current = current->alt_next->alt_next;
                    break;
                }

                if (command == "swap")
                {
                    std::vector<Token> list = reverse_list(pop_list(stack));
                    Token a = list.back();
                    list.pop_back();
                    Token b = list.back();
                    list.pop_back();
                    list.push_back(a);
                    list.push_back(b);

                    push_list(stack, list);
                    break;
                }

                if (command == "swap-list")
                {
                    std::vector<Token> list_a = reverse_list(pop_list(stack));
                    std::vector<Token> list_b = reverse_list(pop_list(stack));
                    push_list(stack, list_a);
                    push_list(stack, list_b);
                    break;
                }

                if (command == "define")
                {
                    if (temp.type != TokenType::CONSTANT)
                    {
                        exception = true;
                        exception_message = "Constant identifier not provided";
                        break;
                    }

                    if (constants.count(std::any_cast<std::string>(temp.value)) > 0)
                    {
                        exception = true;
                        exception_message = "Constant already exists.";
                        break;
                    }

                    std::vector<Token> val = pop_list(stack);
                    constants.insert({std::any_cast<std::string>(temp.value), val.back()});
                    temp.type = TokenType::NULL_TOKEN;

                    break;
                }

                if (command == "include")
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

                        if (std::find(includes.begin(), includes.end(), get_token_string(f)) != includes.end())
                            continue;

                        includes.push_back(get_token_string(f));

                        std::string file_path = program_path + get_token_string(f);
                        std::string file_content = load_file(file_path.c_str());
                        
                        if (file_content.empty())
                        {
                            exception = true;
                            exception_message = "Could not load file: " + get_token_string(f);
                            break;
                        }

                        new_code += file_content + "\n";
                    }

                    // Break if at least one item is not a valid string.
                    if (exception)
                        break;

                    Node* new_nodes = tokenize(new_code.c_str());

                    if (!lex(new_nodes))
                    {
                        exception = true;
                        exception_message = "Lexical analysis failed.";
                        break;
                    }

                    if (!parse(new_nodes))
                    {
                        exception = true;
                        exception_message = "Parsing failed.";
                        break;
                    }

                    Node* next_half = current->default_next;
                    current->default_next = new_nodes;

                    Node* last_node = current;

                    while (last_node->default_next != nullptr)
                    {
                        last_node = last_node->default_next;
                    }

                    last_node->default_next = next_half;
                    break;
                }

                if (command == "exit")
                {
                    current = nullptr;
                    skip_end = true;
                    break;
                }
                break;
            }

            case TokenType::USER_FUNCTION:
            {
                std::vector<Token> args = reverse_list(pop_list(stack));

                if (functions.count(command) == 0)
                {
                    exception = true;
                    exception_message = "Undefined function: " + command;
                    break;
                }

                Token ret;
                ret.type = TokenType::ADDRESS;
                ret.value = current->default_next;
                push_list(stack, {ret});

                Token func;
                func.type = TokenType::FUNCTION_CALL;
                push_list(stack, {func});
                push_list(stack, args);

                current = functions.at(command);
                break;
            }
            default:
                break;
        }

        if (exception == true)
        {
            if (current->default_next == nullptr)
                break;

            if (current->default_next->t.type == TokenType::COMMAND)
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
