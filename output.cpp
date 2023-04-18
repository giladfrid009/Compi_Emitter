#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include "output.hpp"
#include "emit/ir_builder.hpp"

using std::string;
using std::stringstream;
using std::cout;

string type_list_to_string(const std::vector<string>& arg_types)
{
    stringstream res;
    res << "(";
    for (size_t i = 0; i < arg_types.size(); ++i)
    {
        res << arg_types[i];
        if (i + 1 < arg_types.size())
            res << ",";
    }
    res << ")";
    return res.str();
}

void output::error_lex(int lineno)
{
    cout << ir_builder::format_string("line %d: lexical error\n", lineno);
    exit(-1);
}

void output::error_syn(int lineno)
{
    cout << ir_builder::format_string("line %d: syntax error\n", lineno);
    exit(-1);
}

void output::error_undef(int lineno, const string& id)
{
    cout << ir_builder::format_string("line %d: variable %s is not defined\n", lineno, id);
    exit(-1);
}

void output::error_def(int lineno, const string& id)
{
    cout << ir_builder::format_string("line %d: identifier %s is already defined\n", lineno, id);
    exit(-1);
}

void output::error_undef_func(int lineno, const string& id)
{
    cout << ir_builder::format_string("line %d: function %s is not defined\n", lineno, id);
    exit(-1);
}

void output::error_mismatch(int lineno)
{
    cout << ir_builder::format_string("line %d: type mismatch\n", lineno);
    exit(-1);
}

void output::error_prototype_mismatch(int lineno, const string& id, std::vector<string>& arg_types)
{
    cout << ir_builder::format_string("line %d: prototype mismatch, function $s expects arguments %s\n", 
        lineno, id, type_list_to_string(arg_types));
    exit(-1);
}

void output::error_unexpected_break(int lineno)
{
    cout << ir_builder::format_string("line %d: unexpected break statement\n", lineno);
    exit(-1);
}

void output::error_unexpected_continue(int lineno)
{
    cout << ir_builder::format_string("line %d: unexpected continue statement\n", lineno);
    exit(-1);
}

void output::error_main_missing()
{
    cout << "Program has no 'void main()' function\n";
    exit(-1);
}

void output::error_byte_too_large(int lineno, const string& value)
{
    cout << ir_builder::format_string("line %d: byte value %s out of range\n", lineno, value);
    exit(-1);
}
