#include "lang.hpp"
#include <any>
#include <iostream>
#include <ostream>
#include <string>
#include <map>
#include <algorithm>

void print_list(const std::vector<Token>& list)
{
    std::cout << "Count " << list.size() << std::endl;

    std::string last_item = "";
    std::string last_type = "";
    bool last_match = false;
    bool match = false;
    std::string item;
    std::string type;
    int i = 0;

    std::string output = "";

    for (; i < list.size(); i++)
    {
        item = get_token_string(list[i]);
        type = TokenTypeString[list[i].type];
        match = (item == last_item);

        if (!last_match)
            output += match ? "...\n" : std::to_string(i) + " : " + item + " as " + type + "\n";

        last_item = item;
        last_type = type;
        last_match = match;
    }

    output += (match ? ("\n...\n" + std::to_string(i) + " : " + last_item + " as " + last_type) : "");
    std::cout << output;
    std::flush(std::cout);
}

std::vector<Token> pop_list(std::vector<Token>& stack)
{
    auto it = std::find_if(stack.rbegin(), stack.rend(), is_stack_break);
    int pos = it != stack.rend() ? (it - stack.rbegin()) : 0;

    std::vector<Token> list;
    list.reserve(stack.size() - (stack.size() - pos));

    for (int i = 0; i < pos && !is_stack_break(stack.back()); i++)
    {
        list.push_back(stack.back());
        stack.pop_back();
    }

    if (!stack.empty() && stack.back().type == TokenType::LIST_START)
        stack.pop_back();

    if (!stack.empty() && list.empty() && is_stack_break(stack.back()))
    {
        list.push_back(stack.back());
        stack.pop_back();
    }

    return list;
}

std::vector<Token> top_list(const std::vector<Token>& stack)
{
    int i = stack.size() - 1;

    for (; i >= 0; i--)
    {
        Token t = stack.at(i);

        if (is_stack_break(t))
            break;
    }

    i += stack.at(i).type == TokenType::LIST_START;

    std::vector<Token> res;
    res.reserve(stack.size() - (stack.size() - i));
    res.insert(res.end(), stack.begin() + i, stack.end());

    return res;
}

__attribute__((always_inline))
inline void push_list(std::vector<Token>& stack, const std::vector<Token>& list)
{
    if (list.empty() || !is_stack_break(list.front()))
        stack.push_back({.type = TokenType::LIST_START});

    stack.insert(stack.end(), list.begin(), list.end());
}

__attribute__((always_inline))
inline void push_list_empty(std::vector<Token>& stack, const std::vector<Token>& list)
{
    return list.empty() ? void() : push_list(stack, list);
}

__attribute__((always_inline))
inline Token get_tag(const std::vector<Token>& list, const Token& tag)
{
    const int pos = find_tag(list, tag);
    return is_tag(tag) && pos >= 0 ? list.at(pos + 1) : tag;
}

__attribute__((always_inline))
inline void check_exception(const bool& condition, const std::string& message)
{
    if (condition)
        throw InterpreterException(message);
}

bool interpret(const std::string& executable_path, const std::string& program_path, Node* program, std::vector<Token>& backup_stack)
{
    #ifdef DEBUG
        const auto start = std::chrono::high_resolution_clock::now();
    #endif

    Node* current = program;
    std::string exception_message = "";
    std::map<std::string, Function> functions;
    std::map<std::string, Token> constants;

    std::vector<Token> stack;

    const std::string stdlibpath = executable_path + "libs/std/";

    stack.insert(stack.end(), backup_stack.begin(), backup_stack.end());
    Token temp;

    bool skip_end = false;
    for (current = program; current != nullptr; current = (!skip_end) ? current->default_next : current, skip_end = false)
    {
        try {
            switch (current->t.type)
            {
                case TokenType::SUB_LIST_START:
                    check_exception(stack.empty(), "No lists are present on the stack.");
                    break;
            
                case TokenType::LIST_START:
                case TokenType::TAG_GLOBAL:
                case TokenType::TAG_LOCAL:
                case TokenType::TAG_BLOCK:
                case TokenType::TAG_MEMBER:
                case TokenType::DATA_String:
                case TokenType::DATA_Char:
                case TokenType::DATA_Number:
                case TokenType::DATA_Bool:
                {
                    stack.push_back(current->t);
                    break;
                }

                case TokenType::CONSTANT:
                {
                    check_exception(constants.count(get_token_string(current->t)) == 0, "Undefined constant.\nConstants must be defined before being used.");
                    
                    stack.push_back(constants.at(get_token_string(current->t)));
                    break;
                }

                case TokenType::OPERATOR:
                {
                    int pos = -1;

                    auto equal = [](const Token &a, const Token &b)
                    {
                        check_exception(a.type != b.type, "Mismatched Types");

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

                    std::vector<Token> values = pop_list(stack);
                    if (values.empty())
                        break;

                    std::reverse(values.begin(), values.end());
                    OperatorEnum op = std::any_cast<OperatorEnum>(current->t.value);
                    
                    auto greater = [](const Token &a, const Token &b)
                    {
                        check_exception(a.type != b.type, "Mismatched Types");

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

                    switch (op)
                    {
                        case OperatorEnum::ASSIGNMENT:
                        {
                            std::vector<Token> destination = pop_list(stack);

                            check_exception(destination.size() != 1, "Expected a single integer value or tag as the destination.");
                            pos = (destination.back().type == TokenType::DATA_Number) ? int(std::any_cast<float>(destination.back().value)) : find_tag(stack, destination.back()) + 1;

                            check_exception(pos < 0 || pos >= stack.size(), "Position not on stack");

                            Token value = get_tag(stack, values.front());

                            check_exception(is_tag(value), get_token_string(value) + " Tag not found.");
                    
                            stack.at(pos) = value;
                            break;
                        }

                        case OperatorEnum::ADDITION:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val = get_tag(stack, values[v]);

                                check_exception(val.type != DATA_Number, get_token_string(val) + " : Expected a numeric value.");
                                res.value = std::any_cast<float>(res.value) + std::any_cast<float>(val.value);
                            }
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::SUBTRACTION:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val = get_tag(stack, values[v]);

                                check_exception(val.type != DATA_Number, get_token_string(val) + " : Expected a numeric value.");
                                res.value = std::any_cast<float>(res.value) - std::any_cast<float>(val.value);
                            }
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::MULTIPLICATION:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val = get_tag(stack, values[v]);

                                check_exception(val.type != DATA_Number, get_token_string(val) + " : Expected a numeric value.");
                                res.value = std::any_cast<float>(res.value) * std::any_cast<float>(val.value);
                            }
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::DIVISION:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val = get_tag(stack, values[v]);

                                check_exception(val.type != DATA_Number, get_token_string(val) + " : Expected a numeric value.");
                                res.value = std::any_cast<float>(res.value) / std::any_cast<float>(val.value);
                            }
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::INCREMENT:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            res.value = std::any_cast<float>(res.value) + 1;
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::DECREMENT:
                        {
                            res = get_tag(stack, values.front());
                            check_exception((!is_value(res)), get_token_string(res) + " : Expected a numeric value.");

                            res.value = std::any_cast<float>(res.value) - 1;
                            push_list(stack, {res});
                            break;
                        }

                        case OperatorEnum::EQUAL:
                        case OperatorEnum::NOT_EQUAL:
                        case OperatorEnum::GREATER_THAN:
                        case OperatorEnum::GREATER_THAN_EQUAL:
                        case OperatorEnum::LESS_THAN:
                        case OperatorEnum::LESS_THAN_EQUAL:
                        {
                            Token val_a = get_tag(stack, values.front());

                            check_exception(is_tag(res), get_token_string(res) + ": Tag not found.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val_b = get_tag(stack, values[v]);

                                check_exception(is_tag(val_b), get_token_string(val_b) + ": Tag not found.");
                                check_exception((val_a.type != val_b.type && val_a.type != TokenType::DATA_String), (std::string(get_token_string(val_a)) + " (" + TokenTypeString[val_a.type] + ") " + get_token_string(val_b) + " (" + TokenTypeString[val_b.type] + ") : Can only compare values of matching types."));

                                switch (op)
                                {
                                    case OperatorEnum::EQUAL:
                                        if (!equal(val_a, val_b))
                                            set_res(false);
                                        break;

                                    case OperatorEnum::NOT_EQUAL:
                                        if (equal(val_a, val_b))
                                            set_res(false);
                                        break;

                                    case OperatorEnum::GREATER_THAN:
                                        if (greater(val_a, val_b))
                                            set_res(false);
                                        break;
                                    
                                    case OperatorEnum::GREATER_THAN_EQUAL:
                                        if (!greater(val_a, val_b) && !equal(val_a, val_b))
                                            set_res(false);
                                        break;

                                    case OperatorEnum::LESS_THAN:
                                        if (greater(val_a, val_b))
                                            set_res(false);
                                        break;

                                    case OperatorEnum::LESS_THAN_EQUAL:
                                        if (greater(val_a, val_b) && !equal(val_a, val_b))
                                            set_res(false);
                                        break;

                                    default:
                                        set_res(false);
                                        break;
                                }

                                val_a = val_b;
                            }

                            push_list(stack, {{.type = TokenType::DATA_Bool, .value = !res_set}});
                            break;
                        }

                        case OperatorEnum::AND:
                        case OperatorEnum::OR:
                        {
                            res = get_tag(stack, values[0]);
                            check_exception(is_tag(res), get_token_string(res) + ": Tag not found.");
                            check_exception(res.type != TokenType::DATA_Bool, "Can only compare booleans.");

                            for (int v = 1; v < values.size(); v++)
                            {
                                Token val = get_tag(stack, values[v]);
                                check_exception(is_tag(val), get_token_string(val) + ": Tag not found.");
                                check_exception(val.type != TokenType::DATA_Bool, "Can only compare booleans.");

                                if ((op == OperatorEnum::AND && !equal(res, val)) || (op == OperatorEnum::OR && (std::any_cast<bool>(res.value) == false && std::any_cast<bool>(val.value) == false)))
                                    set_res(false);
                        
                                if (res_set)
                                    break;
                            }

                            res.value = !res_set;

                            push_list(stack, {res});
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }

                case TokenType::COMMAND:
                {
                    CommandEnum command_enum = std::any_cast<CommandEnum>(current->t.value);

                    switch (command_enum)
                    {
                        case CommandEnum::PRINTLN:
                        {
                            std::vector<Token> list = pop_list(stack);
                            std::string output;

                            for (int i = list.size() - 1; i >= 0; i--)
                            {
                                Token val = get_tag(stack, list.at(i));

                                check_exception(!is_value(val), get_token_string(val) + ": Tag not found.");
                                output += get_token_string(val) + "\n";
                            }

                            std::cout << output;
                            std::flush(std::cout);
                            break;
                        }

                        case CommandEnum::PRINT:
                        {
                            std::vector<Token> list = pop_list(stack);
                            std::string output;

                            for (int i = list.size() - 1; i >= 0; i--)
                            {
                                Token val = get_tag(stack, list.at(i));

                                check_exception(!is_value(val), get_token_string(val) + ": Tag not found.");
                                output += get_token_string(val);
                            }

                            std::cout << output;
                            std::flush(std::cout);

                            break;

                        }

                        case CommandEnum::INPUT:
                        {
                            std::string val;
                            std::getline(std::cin, val);
    
                            Token t = {.type = TokenType::DATA_String, .value = val};
    
                            if (!val.empty())
                            {
                                t.type = is_num(val) ? TokenType::DATA_Number : TokenType::NULL_TOKEN;
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
                            std::vector<Token> list = pop_list(stack);
                            std::reverse(list.begin(), list.end());
                            // GLOBAL_TAG | LOCAL_TAG | MEMBER_TAG | DATA_String
                            // INT

                            int pos = !list.empty() ? find_tag(stack, list.front()) : -1;

                            check_exception(list.empty(), "Expected a Tag as the first item.");
                            check_exception((pos < 0 && list.front().type != TokenType::DATA_String), get_token_string(list.front()) + ": Tag not found.");

                            int offset = 0;

                            if (list.size() == 2)
                            {
                                offset = list.back().type == TokenType::DATA_Number ? int(std::any_cast<float>(list.back().value)) : 0;

                                if (list.back().type != TokenType::DATA_Number)
                                {
                                    int o_pos = find_tag(stack, list.back());

                                    check_exception((o_pos < 0), "Offset tag not found.");
                                
                                    offset = (list.back().type == TokenType::TAG_MEMBER) ? o_pos : -1;
                                
                                    if (is_tag(list.back()))
                                    {
                                        check_exception((stack.at(o_pos + 1).type != TokenType::DATA_Number), "Offset tag does not mark a Number.");
                                
                                        offset = int(std::any_cast<float>(stack.at(o_pos + 1).value));
                                    }
                                }
                            }

                            check_exception(offset < 0, "Offset cannot be less than 0");

                            Token res;

                            if (list.front().type != TokenType::DATA_String)
                            {
                                int end = pos + offset + 1;
                                check_exception((end > stack.size() - 1 || end < 0), std::to_string(pos + offset + 1) + "Index not found in stack.");

                                for (int i = pos; i < end; i++)
                                    check_exception(stack.at(i).type >= TokenType::LIST_START, std::to_string(i) + ": Index crosses into another list.");

                                res = {.type = TokenType::DATA_Number, .value = float(end)};
                                push_list(stack, {res});
                                break;
                            }
                            
                            check_exception(offset < 0 || offset >= get_token_string(list.front()).length(), std::to_string(offset) + ": Index not found in provided string.");
                            res = {.type = TokenType::DATA_Char, .value = std::any_cast<std::string>(list.front().value).at(offset)};
                            push_list(stack, {res});
                            break;
                        }

                        case CommandEnum::GET:
                        {
                            std::vector<Token> list = pop_list(stack);

                            check_exception(list.size() != 1, "Expected a tag or a single integer value.");

                            Token tok = list.back();

                            if (is_tag(tok))
                            {
                                Token val = get_tag(stack, tok);
                                check_exception(!is_value(val), get_token_string(val) + " Provided tag does not exist on the stack.");
                                
                                push_list(stack, {val});
                                break;
                            }

                            if (tok.type == TokenType::DATA_Number)
                            {
                                int pos = int(std::any_cast<float>(tok.value));
                                check_exception(pos < 0 || pos >= stack.size(), "Provided index does not exist on the stack.");

                                if (is_value(stack.at(pos)))
                                    push_list(stack, {stack.at(pos)});
                                else
                                    push_list(stack, {{.type = TokenType::DATA_String, .value = TokenTypeString[stack.at(pos).type]}});
                            }
                            break;
                        }

                        case CommandEnum::GET_LIST:
                        {
                            std::vector<Token> pos = pop_list(stack);
                            check_exception((pos.size() != 1 || pos.back().type != TokenType::DATA_Number), "Expected a single integer value.");
                            push_list(stack, {});
                            
                            for (int i = int(std::any_cast<float>(pos.front().value)); i < stack.size(); i++)
                            {
                                Token tok = stack.at(i);

                                if (tok.type == TokenType::LIST_START || is_stack_break(tok))
                                    break;

                                stack.push_back(tok);
                            }
                            break;
                        }

                        case CommandEnum::MERGE:
                        {
                            std::vector<Token> list = pop_list(stack);
                            std::reverse(list.begin(), list.end());
                            stack.insert(stack.end(), list.begin(), list.end());
                            break;
                        }

                        case CommandEnum::MERGE_X:
                        {
                            std::vector<Token> args = pop_list(stack);

                            check_exception(args.size() != 1, "Expected a single element list, containing a numeric value.");

                            Token count = get_tag(stack, args.back());

                            check_exception(count.type != TokenType::DATA_Number, "Expected a single element list, containing a numeric value.");
    
                            std::vector<Token> result;
                        
                            for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                            {
                                std::vector<Token> b = pop_list(stack);
                                std::reverse(b.begin(), b.end());
                                b.insert(b.end(), result.begin(), result.end());
                                result = b;
                            }
    
                            push_list(stack, result);
                            break;
                        }

                        case CommandEnum::INT:
                        {
                            std::vector<Token> values = pop_list(stack);
    
                            for (int i = 0; i < values.size(); i++)
                            {
                                if (values.at(i).type == TokenType::DATA_Number)
                                    values.at(i).value = float(int(std::any_cast<float>(values.at(i).value)));
                            }

                            std::reverse(values.begin(), values.end());
                            push_list(stack, values);
                            break;
                        }

                        case CommandEnum::IF:
                        {
                            std::vector<Token> condition = pop_list(stack);

                            check_exception(condition.size() != 1 || condition.front().type != TokenType::DATA_Bool, "Expected a single Boolean value.");

                            if (std::any_cast<bool>(condition.front().value) == false)
                            {
                                current = current->alt_next;
                                skip_end = true;
                            }
                            
                            push_list(stack, {{.type = TokenType::CONDITION_BLOCK}});
                            break;
                        }

                        case CommandEnum::ERROR_HANDLER:
                        {
                            push_list(stack, {{.type = TokenType::CONDITION_BLOCK}});
                            
                            if (!exception_message.empty())
                            {
                                push_list(stack, {{.type = TokenType::DATA_String, .value = exception_message}});
                                exception_message = "";
                                break;
                            }
                            
                            current = current->alt_next;
                            skip_end = true;
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

                            check_exception(list.empty(), "Expected a single element list, containing an integer value.");

                            list.front() = get_tag(stack, list.front());
                        
                            check_exception(list.front().type != TokenType::DATA_Number, "Expected a single element list, containing an integer value.");
    
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
                            std::vector<Token> list = pop_list(stack);

                            check_exception(((list.size() != 1 && list.size() != 2) || list.front().type != TokenType::DATA_Bool), "Expected a single element list, containing a boolean value.\nA leading tag may be used as a way to access this value.");

                            if (std::any_cast<bool>(list.front().value) == false)
                            {
                                current = current->alt_next->default_next;
                                skip_end = true;
                                break;
                            }

                            std::reverse(list.begin(), list.end());
                            push_list(stack, list);
                            push_list(stack, {{.type = TokenType::LOOP_BLOCK, .value = TokenTypeString[TokenType::LOOP_BLOCK]}});
                            break;
                        }                   

                        case CommandEnum::FOR:
                        {
                            std::vector<Token> list = pop_list(stack);
                            std::reverse(list.begin(), list.end());

                            check_exception(list.size() < 3 || list.size() > 4, "Expected a list of 3 numeric values.\nThe list may begin with a tag to access the counter.");
    
                            Token step = list.back();
                            list.pop_back();
                            step = get_tag(stack, step);
                            float step_f;

                            check_exception(step.type != TokenType::DATA_Number, get_token_string(step) + ": Expected a Number for step.");

                            step_f = std::any_cast<float>(step.value);

                            Token end_val = list.back();
                            list.pop_back();
                            end_val = get_tag(stack, end_val);
                            float end_f;

                            check_exception(end_val.type != TokenType::DATA_Number, get_token_string(end_val) + ": Expected a Number for end value.");

                            end_f = std::any_cast<float>(end_val.value);

                            Token current_val = list.back();
                            list.pop_back();
                            current_val = get_tag(stack, current_val);
                            float current_f;

                            check_exception(current_val.type != TokenType::DATA_Number, get_token_string(current_val) + ": Expected a Number for start value.");

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

                            if (!list.empty())
                            {
                                Token tag = list.back();
                                list.pop_back();
                                push_list(stack, (tag.type != TokenType::NULL_TOKEN) ? std::vector<Token>{tag} : std::vector<Token>{});
                            }

                            std::vector<Token> stack_items = {current_val, end_val, step, {.type = TokenType::LOOP_BLOCK, .value = TokenTypeString[TokenType::LOOP_BLOCK]}};

                            stack.insert(stack.end(), stack_items.begin(), stack_items.end());
                            break;
                        }

                        case CommandEnum::DEFUNC:
                        {
                            check_exception(temp.type != TokenType::USER_FUNCTION, "User function identifier not provided");
                            check_exception(functions.count(std::any_cast<std::string>(temp.value)) != 0, "Duplicate definition: " + std::any_cast<std::string>(temp.value));

                            std::vector<Token> function_args = pop_list(stack);
                            Function new_func = {.location = current};
                            new_func.argument_tags = new Token[function_args.size()];
                            new_func.arg_count = function_args.size();
    
                            for (int i = 0; i < new_func.arg_count; i++)
                                new_func.argument_tags[i] = function_args[i];

                            functions.insert({std::any_cast<std::string>(temp.value), new_func});
                            temp.type = TokenType::NULL_TOKEN;
                            current = current->alt_next;
                            break;
                        }

                        case CommandEnum::CACHE:
                        {
                            current = current->default_next;
                            check_exception(current == nullptr, "No value found.");
    
                            temp.value = current->t.value;
                            temp.type = current->t.type;
                            break;
                        }

                        case CommandEnum::RETURN:
                        {
                            std::vector<Token> ret_list = pop_list(stack);

                            std::vector<Token> list;

                            while (!stack.empty())
                            {
                                list = pop_list(stack);

                                if (!list.empty() && list.front().type == TokenType::FUNCTION_CALL)
                                    break;
                            }

                            current = std::any_cast<Node*>(list.back().value);
                            skip_end = true;
                    
                            std::reverse(ret_list.begin(), ret_list.end());
                            push_list_empty(stack, ret_list);
                            break;
                        }
                        
                        case CommandEnum::END:
                        {
                            std::vector<Token> list = pop_list(stack);

                            switch (std::any_cast<CommandEnum>(current->alt_next->t.value))
                            {
                                case CommandEnum::IF:
                                case CommandEnum::ERROR_HANDLER:
                                {
                                    while (!list.empty() && list.front().type != TokenType::CONDITION_BLOCK)
                                        list = pop_list(stack);

                                    break;
                                }
                                case CommandEnum::BEGIN:
                                {
                                    while (!list.empty() && list.front().type != TokenType::BLOCK)
                                        list = pop_list(stack);

                                    break;
                                }
                                case CommandEnum::LOOP:
                                case CommandEnum::WHILE:
                                case CommandEnum::FOR:
                                {
                                    while (!list.empty() && list.front().type != TokenType::LOOP_BLOCK)
                                        list = pop_list(stack);

                                    current = current->alt_next;
                                    skip_end = true;
                                    break;
                                }
                                case CommandEnum::DEFUNC:
                                {
                                    print_list(stack);
                                    std::vector<Token> list = pop_list(stack);

                                    while (!stack.empty() && list.empty())
                                    {
                                        if ((!list.empty() && list.back().type == TokenType::FUNCTION_CALL) || stack.empty())
                                            break;

                                        list = pop_list(stack);
                                    }

                                    print_list(list);

                                    current = std::any_cast<Node*>(list.back().value);
                                    skip_end = true;
                                    break;
                                }

                                default:
                                    break;
                            }

                            break;
                        }

                        case CommandEnum::BREAK:
                        {
                            CommandEnum target = std::any_cast<CommandEnum>(current->alt_next->t.value);
                            std::vector<Token> list = pop_list(stack);
                            std::reverse(list.begin(), list.end());
    
                            while (!list.empty() && list.front().type != TokenType::LOOP_BLOCK)
                                list = pop_list(stack);
    
                            current = current->alt_next->alt_next;
                            break;
                        }   

                        case CommandEnum::CONTINUE:
                        {
                            std::vector<Token> list = pop_list(stack);
                            std::reverse(list.begin(), list.end());
    
                            while (!list.empty() && list.front().type != TokenType::LOOP_BLOCK)
                                list = pop_list(stack);
    
                            current = current->alt_next;
                            skip_end = true;
                            break;
                        }

                        case CommandEnum::SWAP:
                        {
                            // Don't swap when < 2 elements on stack
                            if (stack.size() < 2)
                                break;

                            Token temp = stack.back();
                            stack.back() = stack.at(stack.size() - 2);
                            stack.at(stack.size() - 2) = temp;
                            break;
                        }

                        case CommandEnum::SWAP_LIST:
                        {
                            std::vector<Token> list_a = pop_list(stack);
                            std::vector<Token> list_b = pop_list(stack);
                            std::reverse(list_a.begin(), list_a.end());
                            std::reverse(list_b.begin(), list_b.end());
                            push_list_empty(stack, list_a);
                            push_list_empty(stack, list_b);
                            break;
                        }

                        case CommandEnum::DUP:
                        {
                            check_exception(stack.empty(), "Empty stack.");
                            stack.push_back(stack.back());
                            break;
                        }

                        case CommandEnum::DUP_X:
                        {
                            std::vector<Token> list = pop_list(stack);
                            check_exception(list.size() != 1, "Expected a single integer value");
                            Token count = list.back();

                            check_exception(count.type != TokenType::DATA_Number, get_token_string(count) + ": Expected one integer value.");

                            for (int i = 0; i < int(std::any_cast<float>(count.value)); i++)
                                stack.push_back(stack.back());
                            break;
                        }

                        case CommandEnum::DUP_LIST:
                        {
                            check_exception(stack.empty(), "Empty stack.");
                            std::vector<Token> list = top_list(stack);
                            // std::reverse(list.begin(), list.end());
                            push_list(stack, list);
                            break;
                        }

                        case CommandEnum::DEFINE:
                        {
                            check_exception(temp.type != TokenType::CONSTANT, "Constant identifier not provided");
                            check_exception(constants.count(get_token_string(temp)) != 0, "Constant already exists.");

                            constants.insert({std::any_cast<std::string>(temp.value), pop_list(stack).back()});
                            temp.type = TokenType::NULL_TOKEN;
                            break;
                        }

                        case CommandEnum::INCLUDE:
                        {
                            std::vector<Token> file_list = pop_list(stack);
    
                            check_exception(file_list.size() != 1, "Expected one file name.");
    
                            Token f = get_tag(stack, file_list.back());

                            check_exception(f.type != TokenType::DATA_String, "Expected a string.");
                            check_exception(!is_valid_extension(get_token_string(f), EXTENSION), "Invalid file extension: " + get_token_string(f) + "\nExpected " + EXTENSION + " extension.");
    
                            std::string file_name = get_token_string(f);
                            std::string file_path = (file_name.substr(0, 3) == "std" ? stdlibpath : program_path) + file_name;
                            std::string file_content = load_file(file_path);
                            file_content += "\n";

                            Node* new_nodes = tokenize(executable_path, program_path, file_content, file_name);

                            check_exception(!lex(new_nodes), "Lexical analysis failed.");
                            check_exception(!parse(new_nodes, stack), "Parsing failed.");

                            Node* next_half = current->default_next;
                            current->default_next = new_nodes;
    
                            Node* last_node = current;
    
                            while (last_node->default_next != nullptr)
                                last_node = last_node->default_next;
    
                            last_node->default_next = next_half;
                            break;
                        }

                        case CommandEnum::STRLEN:
                        {
                            std::vector<Token> list = pop_list(stack);
                            check_exception(!is_value(list.back()) || list.size() != 1, "Expected a list with 1 value.");
                            Token size = {TokenType::DATA_Number, float(get_token_string(list.back()).length())};
                            push_list(stack, {size});
                            break;
                        }

                        case CommandEnum::LEN:
                        {
                            Token size = {TokenType::DATA_Number, float(top_list(stack).size())};
                            push_list(stack, {size});
                            break;
                        }

                        case CommandEnum::CAT:
                        {
                            std::vector<Token> list = pop_list(stack);
                            Token res = {TokenType::DATA_String, std::string("")};

                            while (!list.empty())
                            {
                                Token t = get_tag(stack, list.back());
                                check_exception(is_tag(t), get_token_string(t) + ": The provided tag cannot be found.");
                                res.value = get_token_string(res) + get_token_string(t);
                                list.pop_back();
                            }

                            push_list(stack, {res});
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
                            throw InterpreterException(std::string(get_command_string(command_enum)) + " : Unknown command.");
                        }
                    }
                    break;
                }

                case TokenType::USER_FUNCTION:
                {
                    std::string func = get_token_string(current->t);
                    check_exception(functions.count(func) == 0, "Undefined function: " + func);

                    std::vector<Token> function_args;
                    function_args.push_back({.type = TokenType::FUNCTION_CALL, .value = current->default_next});

                    for (int i = 0; i < functions.at(func).arg_count; i++)
                    {
                        push_list(function_args, {functions.at(func).argument_tags[i]});
                        std::vector<Token> values = pop_list(stack);
                        std::reverse(values.begin(), values.end());
                        function_args.insert(function_args.end(), values.begin(), values.end());
                    }

                    stack.insert(stack.end(), function_args.begin(), function_args.end());

                    current = functions.at(func).location;
                    break;
                }
                default:
                    break;
            }

        }
        
        catch (InterpreterException &e)
        {
            exception_message = e.what();

            if (current->default_next != nullptr && current->default_next->t.type == TokenType::COMMAND && get_command_enum(get_token_string(current->default_next->t).c_str()) == CommandEnum::ERROR_HANDLER)
            {
                current = current->default_next;
                continue;
            }
            
            print_list(stack);
            std::cout << "\nStack reverted to backup upon error." << std::endl;
            error_msg(current, e.what());
            break;
        }
    }

    for (std::pair<std::string, Function> f : functions)
    {
        delete[] f.second.argument_tags;
    }

    if (exception_message.empty())
        backup_stack = stack;

    #ifdef DEBUG
        const auto stop = std::chrono::high_resolution_clock::now();
    
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n[TIME] Execution: " << (float(duration.count()) / 1000) << " milliseconds" << std::endl;
    #endif

    std::cout << std::endl;

    return true;
}
