#include <iostream>
#include "output.hpp"
#include <sstream>
#include <string>
#include <stdlib.h>

using std::string;
using std::stringstream;
using std::cout;
using std::endl;

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
    cout << "line " << lineno << ":" << " lexical error" << endl;
    exit(-1);
}

void output::error_syn(int lineno)
{
    cout << "line " << lineno << ":" << " syntax error" << endl;
    exit(-1);
}

void output::error_undef(int lineno, const string& id)
{
    cout << "line " << lineno << ":" << " variable " << id << " is not defined" << endl;
    exit(-1);
}

void output::error_def(int lineno, const string& id)
{
    cout << "line " << lineno << ":" << " identifier " << id << " is already defined" << endl;
    exit(-1);
}

void output::error_undef_func(int lineno, const string& id)
{
    cout << "line " << lineno << ":" << " function " << id << " is not defined" << endl;
    exit(-1);
}

void output::error_mismatch(int lineno)
{
    cout << "line " << lineno << ":" << " type mismatch" << endl;
    exit(-1);
}

void output::error_prototype_mismatch(int lineno, const string& id, std::vector<string>& arg_types)
{
    cout << "line " << lineno << ": prototype mismatch, function " << id << " expects arguments " << type_list_to_string(arg_types) << endl;
    exit(-1);
}

void output::error_unexpected_break(int lineno)
{
    cout << "line " << lineno << ":" << " unexpected break statement" << endl;
    exit(-1);
}

void output::error_unexpected_continue(int lineno)
{
    cout << "line " << lineno << ":" << " unexpected continue statement" << endl;
    exit(-1);
}

void output::error_main_missing()
{
    cout << "Program has no 'void main()' function" << endl;
    exit(-1);
}

void output::error_byte_too_large(int lineno, const string& value)
{
    cout << "line " << lineno << ": byte value " << value << " out of range" << endl;
    exit(-1);
}
