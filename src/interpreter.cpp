#include "lang.hpp"
#include <algorithm>
#include <any>

#ifdef DEBUG
#include <chrono>
#endif

#include <iostream>
#include <string>
#include <map>

std::vector<Token> reverse_list(const std::vector<Token> &list)
{
    std::vector<Token> res;

    for (int i = list.size() - 1; i >= 0; i--)
        res.push_back(list[i]);

    return res;
}

void print_list(const std::vector<Token> &list)
{
    std::cout << "Count " << list.size() << std::endl;

    if (list.empty())
        return;

    std::string last_item = "";
    bool match = false;
    std::string item;
    int i = 0;

    for (; i < list.size(); i++)
    {
        item = get_token_string(list[i]);
        match = (item == last_item);

        if (!match)
            std::cout << (match ? "...\n" : "") << i << " : " << item << std::endl;

        last_item = item;
    }

    if (match)
        std::cout << "\n...\n" << std::to_string(i) << " : " << last_item;
    
    std::cout << std::endl;
}

inline bool is_stack_break(const Token &tok)
{
    return tok.type == TokenType::FUNCTION_CALL || tok.type == TokenType::CONDITION_BLOCK || tok.type == TokenType::BLOCK || tok.type == TokenType::LOOP_BLOCK;
}

std::vector<Token> pop_list(std::vector<Token> &stack)
{
    std::vector<Token> list;

    while (!stack.empty() && stack.back().type != TokenType::LIST_START && !is_stack_break(stack.back()))
    {
        list.push_back(stack.back());
        stack.pop_back();
    }

    if (!stack.empty())
    {
        if (is_stack_break(stack.back()))
            list.push_back(stack.back());

        stack.pop_back();
    }

    return list;
}

inline void append_list(std::vector<Token> &base, const std::vector<Token> &list)
{
    for (Token t : list)
        base.push_back(t);
}

void push_list(std::vector<Token> &stack, const std::vector<Token> &list)
{
    if (!list.empty() && !is_stack_break(list.back()))
        stack.push_back({.type = TokenType::LIST_START});

    append_list(stack, list);
}

Token get_tag(const std::vector<Token> &list, const Token &tag)
{
    if (!is_tag(tag))
        return tag;

    int pos = find_tag(list, tag);
    return (pos >= 0 && pos < list.size() - 1) ? list.at(pos + 1) : tag;
}

inline bool check_exception(std::string &exception_message, const bool &condition, const std::string &message)
{
    exception_message = condition ? message : "";
    return condition;
}

bool interpret(const std::string &executable_path, const std::string &program_path, Node* program, std::vector<Token> &backup_stack)
{
    Node* current = program;
    std::string exception_message = "";
    std::map<std::string, Function> functions;
    std::map<std::string, Token> constants;

    current = program;

    std::vector<Token> stack;
    std::vector<std::string> includes;

    const std::string stdlibpath = executable_path + "libs/std/";

    append_list(stack, backup_stack);

    bool in_list = false;
    Token temp;

#ifdef DEBUG
    auto start = std::chrono::high_resolution_clock::now();
#endif

    while (current != nullptr)
    {
        if (current == nullptr)
            return false;

        bool skip_end = false;
        std::string command = get_token_string(current->t);

        switch (current->t.type)
        {
            case TokenType::LIST_START:
                append_list(stack, {current->t});
            case TokenType::SUB_LIST_START:
                in_list = !check_exception(exception_message, stack.empty(), "No lists are present on the stack.");
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
            {
                append_list(stack, {current->t});
                break;
            }

            case TokenType::CONSTANT:
            {
                if (check_exception(exception_message, !in_list, "Stray value.\nValues must be pushed to the stack as part of a list."))
                    break;

                if (current->t.type == TokenType::CONSTANT)
                {
                    if (check_exception(exception_message, constants.count(get_token_string(current->t)) == 0, "Undefined constant.\nConstants must be defined before being used."))
                        break;
                    
                    append_list(stack, {constants.at(get_token_string(current->t))});
                }

                break;
            }

            case TokenType::OPERATOR:
            {
                int pos = -1;

                auto equal = [&exception_message](const Token &a, const Token &b)
                {
                    if (check_exception(exception_message, a.type != b.type, "Mismatched Types"))
                        return false;

                    switch (a.type)
                    {
                        case TokenType::DATA_Bool:
                            return std::any_cast<bool>(a.value) == std::any_cast<bool>(b.value);
                        case TokenType::DATA_Char:
                            return std::any_cast<char>(a.value) == std::any_cast<char>(b.value);
                        case TokenType::DATA_String:
                            return std::any_cast<std::string>(a.value) == std::any_cast<std::string>(b.value);
                        case TokenType::DATA_Number:
                            return std::any_cast<float>(a.value) == std::any_cast<float>(b.value);
                        default:
                            return false;
                    }
                };
                
                Token res;
                bool res_set = false;
                
                auto set_res = [&res, &res_set](const bool &value)
                {
                    res.type = TokenType::DATA_Bool;
                    res.value = value;
                    res_set = true;
                };

                std::vector<Token> values = reverse_list(pop_list(stack));

                if (command == "=")
                {
                    std::vector<Token> destination = pop_list(stack);

                    if (check_exception(exception_message, destination.size() != 1, "Expected a single integer value or tag as the destination."))
                        break;

                    if (is_tag(destination.back()))
                        pos = find_tag(stack, destination.back()) + 1;
                    else if (destination.back().type == TokenType::DATA_Number)
                        pos = int(std::any_cast<float>(destination.back().value));

                    if (check_exception(exception_message, pos < 0 || pos >= stack.size(), "Position not on stack"))
                        break;
                    
                    stack.at(pos) = values.front();
                    break;
                } else if (command == "+" || command == "-" || command == "*" || command == "/")
                {
                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (check_exception(exception_message, (!is_value(val) && res.type == TokenType::DATA_Number), get_token_string(val) + " : Expected a numeric value."))
                            break;

                        if (is_value(val))
                        {
                            if (res.type == TokenType::NULL_TOKEN)
                            {
                                res = val;
                                continue;
                            }
                            if (check_exception(exception_message, (res.type == TokenType::DATA_Number && val.type != TokenType::DATA_Number), "Value types do not agree."))
                                break;

                            if (res.type != TokenType::DATA_String)
                            {
                                if (command == "+")
                                    res.value = std::any_cast<float>(res.value) + std::any_cast<float>(val.value);
                                else if (command == "-")
                                    res.value = std::any_cast<float>(res.value) - std::any_cast<float>(val.value);
                                else if (command == "*")
                                    res.value = std::any_cast<float>(res.value) * std::any_cast<float>(val.value);
                                else if (command == "/")
                                {
                                    if (check_exception(exception_message, std::any_cast<float>(res.value) == 0, "Divide by Zero."))
                                        break;

                                    res.value = std::any_cast<float>(res.value) / std::any_cast<float>(val.value);
                                }
                                continue;
                            }

                            if (check_exception(exception_message, res.type == DATA_String && command != "+", "Invalid string operation."))
                                break;

                            res.value = std::any_cast<std::string>(res.value) + get_token_string(val);
                        } else {
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
                    
                    push_list(stack, {res});
                } else if (command == "==" || command == "!=" || command == ">" || command == ">=" || command == "<" || command == "<=")
                {
                    auto greater = [&exception_message](const Token &a, const Token &b)
                    {
                        if (check_exception(exception_message, a.type != b.type, "Mismatched Types"))
                            return false;

                        switch (a.type)
                        {
                            case TokenType::DATA_Bool:
                                return std::any_cast<bool>(a.value) > std::any_cast<bool>(b.value);
                            case TokenType::DATA_Char:
                                return std::any_cast<char>(a.value) > std::any_cast<char>(b.value);
                            case TokenType::DATA_String:
                                return std::any_cast<std::string>(a.value) > std::any_cast<std::string>(b.value);
                            case TokenType::DATA_Number:
                                return std::any_cast<float>(a.value) > std::any_cast<float>(b.value);
                            default:
                                return false;
                        }
                    };

                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);

                        if (check_exception(exception_message, is_tag(val), get_token_string(val) + ": Tag not found."))
                            break;

                        switch (res.type)
                        {
                            case TokenType::NULL_TOKEN:
                                res = val;
                                break;

                            default:
                            {
                                if (check_exception(exception_message, (res.type != val.type && res.type != TokenType::DATA_String), (TokenTypeString[res.type] + " " + TokenTypeString[val.type] + " : Can only compare values of matching types.")))
                                    break;

                                bool sr = (command == "==" && !equal(res, val));
                                sr = (sr || (command == "!=" && equal(res, val)));
                                sr = (sr || (command == ">" && !greater(res, val)));
                                sr = (sr || (command == ">=" && !greater(res, val) && !equal(res, val)));
                                sr = (sr || (command == "<" && greater(res, val)));
                                sr = (sr || (command == "<=" && greater(res, val) && !equal(res, val)));

                                if (sr)
                                    set_res(false);

                                break;
                            }
                        }
                    }

                    if (!exception_message.empty())
                        break;

                    push_list(stack, {{.type = TokenType::DATA_Bool, .value = !res_set}});
                } else if (command == "and" || command == "or")
                {
                    res.type = TokenType::DATA_Bool;

                    for (Token current_val : values)
                    {
                        Token val = get_tag(stack, current_val);
                        if (check_exception(exception_message, is_tag(val), get_token_string(val) + ": Tag not found.") || check_exception(exception_message, val.type != TokenType::DATA_Bool, "Can only compare booleans."))
                            break;

                        if (res.type == TokenType::NULL_TOKEN)
                        {
                            res = val;
                            continue;
                        }

                        if ((command == "and" && !equal(res, val)) || (command == "or" && (std::any_cast<bool>(res.value) == false && std::any_cast<bool>(val.value) == false)))
                            set_res(false);
                        
                        if (res_set)
                            break;
                    }

                    if (!exception_message.empty())
                        break;

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
                        std::vector<Token> list = reverse_list(pop_list(stack));
                        
                        for (Token t : list)
                        {
                            Token val = get_tag(stack, t);
                            if (check_exception(exception_message, !is_value(val), get_token_string(val) + ": Tag not found."))
                                break;

                            std::cout << get_token_string(val) << (command_enum == PRINTLN ? "\n" : "");
                            std::cout.flush();
                        }
                        break;
                    }

                    case CommandEnum::INPUT:
                    {
                        std::string val;
                        std::getline(std::cin, val);
    
                        Token t = {.type = TokenType::DATA_String, .value = val};
    
                        if (!val.empty())
                        {
                            t.type = (val.front() >= '0' && val.front() <= '9') ? TokenType::DATA_Number : TokenType::NULL_TOKEN;
                            t.type = (t.type == TokenType::NULL_TOKEN && (val == "true" || val == "false")) ? TokenType::DATA_Bool : t.type;
                            t.type = (t.type == TokenType::NULL_TOKEN) ? TokenType::DATA_String : t.type;

                            t.value = t.type == TokenType::DATA_Number ? stof(val) : t.value;
                            t.value = t.type == TokenType::DATA_Bool ? (val == "true") : t.value;
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

                        int pos = !list.empty() ? find_tag(stack, list.front()) : -1;

                        if (check_exception(exception_message, list.empty(), "Expected a Tag as the first item.") || (check_exception(exception_message, (pos < 0 && list.front().type != TokenType::DATA_String), get_token_string(list.front()) + ": Tag not found.")))
                            break;

                        int offset = 0;

                        if (list.size() == 2)
                        {
                            offset = list.back().type == TokenType::DATA_Number ? int(std::any_cast<float>(list.back())) : 0;

                            if (list.back().type != TokenType::DATA_Number)
                            {
                                int o_pos = find_tag(stack, list.back());

                                if (check_exception(exception_message, (o_pos < 0), "Offset tag not found."))
                                    break;
                                
                                if (list.back().type == TokenType::TAG_MEMBER)
                                    offset = o_pos;
                                else if (is_tag(list.back()))
                                {
                                    if (check_exception(exception_message, (stack.at(o_pos + 1).type != TokenType::DATA_Number), "Offset tag does not mark a Number."))
                                        break;
                                
                                    offset = int(std::any_cast<float>(stack.at(o_pos + 1).value));
                                }
                            }
                        }

                        if (check_exception(exception_message, offset < 0, "Offset cannot be less than 0"))
                            break;

                        Token res;

                        if (list.front().type != TokenType::DATA_String)
                        {
                            if (check_exception(exception_message, (pos + offset + 1 > stack.size() - 1 || pos + offset + 1 < 0), std::to_string(pos + offset + 1) + "Index not found in stack."))
                                break;
                                
                            int end = pos + offset + 1;

                            for (int i = 0; i < ((offset + 1) / 2) + ((offset + 1) % 2); i++)
                            {
                                if (check_exception(exception_message, (stack.at(pos + i).type >= TokenType::LIST_START || stack.at(end - i).type >= TokenType::LIST_START), std::to_string(i) + ": Index crosses into another list."))
                                    break;
                            }

                            res = {.type = TokenType::DATA_Number, .value = float(pos + offset + 1)};
                        } else {
                            if (check_exception(exception_message, offset >= std::any_cast<std::string>(list.front().value).length(), "Index not found in provided string."))
                                break;

                            res = {.type = TokenType::DATA_Char, .value = std::any_cast<std::string>(list.front().value).at(offset)};
                        }

                        push_list(stack, {res});
                        break;
                    }

                    case CommandEnum::GET:
                    {
                        std::vector<Token> list = pop_list(stack);

                        if (check_exception(exception_message, list.size() != 1, "Expected a tag or a single integer value."))
                            break;

                        Token tok = list.back();

                        if (is_tag(tok))
                        {
                            Token val = get_tag(stack, tok);
                            if (check_exception(exception_message, !is_value(val), "Provided index does not exist on the stack."))
                                break;
                                
                            push_list(stack, {val});
                        } else if (tok.type == TokenType::DATA_Number)
                        {
                            int pos = int(std::any_cast<float>(tok.value));
                            if (check_exception(exception_message, pos < 0 || pos >= stack.size(), "Provided index does not exist on the stack."))
                                break;

                            if (is_value(stack.at(pos)))
                                push_list(stack, {stack.at(pos)});
                            else
                                push_list(stack, {{.type = TokenType::DATA_String, .value = TokenTypeString[stack.at(pos).type]}});
                        }
                        break;
                    }

                    case CommandEnum::GET_LIST:
                    case CommandEnum::GET_LIST_VALUES:
				    {
                        std::vector<Token> tag = pop_list(stack);
                        if (check_exception(exception_message, (tag.size() != 1 || tag.back().type != TokenType::DATA_Number), "Expected a single integer value."))
                            break;
    
                        std::vector<Token> list;

                        for (int i = int(std::any_cast<float>(tag.front().value)); i < stack.size(); i++)
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

                    case CommandEnum::MERGE:
                    {
				        append_list(stack, reverse_list(pop_list(stack)));
                        break;
                    }

                    case CommandEnum::MERGE_X:
                    {
                        std::vector<Token> args = pop_list(stack);

                        if (check_exception(exception_message, args.size() != 1, "Expected a single element list, containing a numeric value."))
                            break;

                        Token count = get_tag(stack, args.back());

                        if (check_exception(exception_message, count.type != TokenType::DATA_Number, exception_message = "Expected a single element list, containing a numeric value."))
                            break;
    
                        std::vector<Token> result;
                        
                        for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                        {
                            std::vector<Token> b = reverse_list(pop_list(stack));
                            append_list(b, result);
                            result = b;
                        }
    
                        push_list(stack, result);
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

                            if (check_exception(exception_message, condition.size() != 1 || condition.front().type != TokenType::DATA_Bool, "Expected a single Boolean value."))
                                break;

                            if (std::any_cast<bool>(condition.front().value) == false)
                            {
                                current = current->alt_next;
                                skip_end = true;
                            }
                            
                            push_list(stack, {{.type = TokenType::CONDITION_BLOCK}});
                        } else {
                            push_list(stack, {{.type = TokenType::CONDITION_BLOCK}});
                            
                            if (!exception_message.empty())
                            {
                                push_list(stack, {{.type = TokenType::DATA_String, .value = exception_message}});
                                exception_message = "";
                            } else {
                                current = current->alt_next;
                                skip_end = true;
                            }
                        }
                        break;
                    }

                    case CommandEnum::BEGIN:
                    {
                        push_list(stack, {{.type = TokenType::BLOCK}});
                        break;
                    }

                    case CommandEnum::LOOP:
                    {
                        std::vector<Token> list = pop_list(stack);

                        if (check_exception(exception_message, list.empty(), "Expected a single element list, containing an integer value."))
                            break;

                        list.front() = get_tag(stack, list.front());
                        
                        if (check_exception(exception_message, list.front().type != TokenType::DATA_Number, "Expected a single element list, containing an integer value."))
                            break;
    
                        if (int(std::any_cast<float>(list.front().value)) == 0)
                        {
                            current = current->alt_next->default_next;
                            skip_end = true;
                            break;
                        }

                        Token count = list.front();
                        count.value = std::any_cast<float>(count.value) - 1;
    
                        push_list(stack, {count, {.type = TokenType::LOOP_BLOCK, .value = TokenTypeString[TokenType::LOOP_BLOCK]}});
                        break;
                    }

                    case CommandEnum::WHILE:
                    {
                        std::vector<Token> list = reverse_list(pop_list(stack));
                        if (check_exception(exception_message, ((list.size() != 1 && list.size() != 2) || list.back().type != TokenType::DATA_Bool), "Expected a single element list, containing a boolean value.\nA leading tag may be used as a way to access this value."))
                            break;

                        if (std::any_cast<bool>(list.back().value) == false)
                        {
                            current = current->alt_next->default_next;
                            skip_end = true;
                            break;
                        }

                        push_list(stack, list);
    
                        push_list(stack, {{.type = TokenType::LOOP_BLOCK, .value = TokenTypeString[TokenType::LOOP_BLOCK]}});
                        break;
                    }                   

                    case CommandEnum::FOR:
                    {
                        std::vector<Token> list = reverse_list(pop_list(stack));

                        if (check_exception(exception_message, list.size() < 3 || list.size() > 4, "Expected a list of 3 numeric values.\nList may begin with a tag to access the counter."))
                            break;
    
                        Token step = list.back();
                        list.pop_back();
                        step = get_tag(stack, step);
                        float step_f;

                        if (check_exception(exception_message, step.type != TokenType::DATA_Number, "Expected a Number for step."))
                            break;

                        step_f = std::any_cast<float>(step.value);

                        Token end_val = list.back();
                        list.pop_back();
                        end_val = get_tag(stack, end_val);
                        float end_f;

                        if (check_exception(exception_message, end_val.type != TokenType::DATA_Number, "Expected a Number for end value."))
                            break;

                        end_f = std::any_cast<float>(end_val.value);

                        Token current_val = list.back();
                        list.pop_back();
                        current_val = get_tag(stack, current_val);
                        float current_f;

                        if (check_exception(exception_message, current_val.type != TokenType::DATA_Number, "Expected a Number for start value."))
                            break;

                        current_f = std::any_cast<float>(current_val.value);

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

                        Token tag;
    
                        if (!list.empty())
                        {
                            tag = list.back();
                            list.pop_back();
                        }

                        push_list(stack, (tag.type != TokenType::NULL_TOKEN) ? std::vector<Token>{tag} : std::vector<Token>{});
                        append_list(stack, {current_val, end_val, step});

                        push_list(stack, {{.type = TokenType::LOOP_BLOCK, .value = TokenTypeString[TokenType::LOOP_BLOCK]}});
                        break;
                    }

                    case CommandEnum::DEFUNC:
                    {
                        if (temp.type == TokenType::USER_FUNCTION)
                        {
                            if (check_exception(exception_message, functions.count(std::any_cast<std::string>(temp.value)) != 0, "Duplicate definition: " + std::any_cast<std::string>(temp.value)))
                                break;

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
                            exception_message = "User function identifier not provided";
                            break;
                        }
                    }

                    case CommandEnum::CACHE:
                    {
                        current = current->default_next;
                        if (check_exception(exception_message, current == nullptr, "No value found."))
                            break;
    
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

                        append_list(stack, {a, b});
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
                        if (check_exception(exception_message, stack.empty(), "Empty stack."))
                            break;

                        append_list(stack, {stack.back()});
                        break;
                    }

                    case CommandEnum::DUP_X:
                    {
                        std::vector<Token> list = pop_list(stack);
                        Token count = list.size() == 1 ? list.back() : (Token){.type = TokenType::NULL_TOKEN};

                        if (check_exception(exception_message, count.type != TokenType::DATA_Number, get_token_string(count) + ": Expected one integer value."))
                            break;

                        for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                            append_list(stack, {stack.back()});
                        break;
                    }

                    case CommandEnum::DEFINE:
                    {

                        if (check_exception(exception_message, temp.type != TokenType::CONSTANT, "Constant identifier not provided"))
                            break;

                        if (check_exception(exception_message, constants.count(get_token_string(temp)) != 0, "Constant already exists."))
                            break;

                        constants.insert({std::any_cast<std::string>(temp.value), pop_list(stack).back()});
                        temp.type = TokenType::NULL_TOKEN;
                        break;
                    }

                    case CommandEnum::INCLUDE:
                    {
                        std::vector<Token> file_list = pop_list(stack);
    
                        if (check_exception(exception_message, file_list.empty(), "No file path provided."))
                            break;
    
                        std::string new_code = "";
    
                        // Load files
                        for (Token f : file_list)
                        {
                            if (check_exception(exception_message, f.type != TokenType::DATA_String, "Expected a string."))
                                break;

                            if (check_exception(exception_message, is_valid_extension(get_token_string(f), EXTENSION), "Invalid file extension: " + get_token_string(f) + "\nExpected " + EXTENSION + " extension."))
                                break;
    
                            std::string file_name = get_token_string(f);
    
                            if (std::find(includes.begin(), includes.end(), file_name) != includes.end())
                                continue;
    
                            includes.push_back(file_name);
    
                            std::string file_path;
                            
                            if (file_name.substr(0, 3) == "std")
                                file_path = stdlibpath + file_name;
                            else
                                file_path = program_path + file_name;
                        
                            std::string file_content = load_file(file_path);
                            
                            if (check_exception(exception_message, file_content.empty(), "Could not load file: " + file_path))
                                break;
    
                            new_code += file_content + "\n";
                        }
    
                        // Break if at least one item is not a valid string.
                        if (!exception_message.empty())
                            break;
    
                        Node* new_nodes = tokenize(executable_path, program_path, new_code);

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
                                exception_message = "Parsing failed.";
                                break;
                            }
                        } else {
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
                        exception_message = command + " : Unknown command.";
                        break;
                    }
                }
                break;
            }

            case TokenType::USER_FUNCTION:
            {
                if (check_exception(exception_message, functions.count(command) == 0, "Undefined function: " + command))
                    break;

                std::vector<Token> function_args;
                function_args.push_back({.type = TokenType::FUNCTION_CALL, .value = current->default_next});

                for (int i = 0; i < functions.at(command).arg_count; i++)
                {
                    push_list(function_args, {functions.at(command).argument_tags[i]});
                    append_list(function_args, reverse_list(pop_list(stack)));
                }

                append_list(stack, function_args);

                current = functions.at(command).location;
                break;
            }
            default:
                break;
        }

        if (!exception_message.empty())
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

        current = !skip_end ? current->default_next : current;
    }
    
#ifdef DEBUG
    auto stop = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "\nExecution time: " << duration.count() << " milliseconds" << std::endl;
#endif

    if (!exception_message.empty())
    {
        error_msg(current, exception_message);
        print_list(stack);
        std::cout << "\nStack reverted to backup upon error." << std::endl;
        return false;
    }

    backup_stack = stack;

    return true;
}
