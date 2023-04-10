#include "code_buffer.hpp"
#include <list>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

using std::list;
using std::string;
using std::ifstream;
using std::stringstream;

bool replace(string& str, const string& from, const string& to);

code_buffer::code_buffer(): _indent(0), _buffer(), _global_defs()
{

}

code_buffer& code_buffer::instance()
{
    static code_buffer instance;
    return instance;
}

void code_buffer::increase_indent()
{
    _indent++;
}

void code_buffer::decrease_indent()
{
    if (_indent <= 0)
    {
        return;
    }

    _indent--;
}

string code_buffer::emit_label()
{
    string label = ir_builder::fresh_label();

    emit(label + ":");

    return label;
}

size_t code_buffer::emit(const string& line)
{
    stringstream instr;

    for (int i = 0; i < _indent; i++)
    {
        instr << "    ";
    }

    instr << line;

    _buffer.push_back(instr.str());

    return _buffer.size() - 1;
}

void code_buffer::new_line()
{
    emit("");
}

size_t code_buffer::emit_from_file(std::string file_path)
{
    ifstream file;

    file.open(file_path.c_str());

    if (file.is_open() == false)
    {
        throw std::runtime_error("couldn't open file.");
    }

    for (string line; std::getline(file, line); )
    {
        emit(line);
    }

    return _buffer.size() - 1;
}

void code_buffer::backpatch(const list<size_t>& patch_list, const std::string& label)
{
    string to = "%" + label;

    for (auto& line : patch_list)
    {
        bool res = replace(_buffer[line], "@", to);

        if (res == false)
        {
            std::runtime_error("nothing to backpatch in the patch record");
        }
    }
}

void code_buffer::print() const
{
    for (auto& line : _buffer)
    {
        std::cout << line << std::endl;
    }
}

void code_buffer::comment(std::string line)
{
    emit(";%s", line);
}

void code_buffer::emit_global(const std::string& line)
{
    _global_defs.push_back(line);
}

void code_buffer::print_globals() const
{
    for (auto& line : _global_defs)
    {
        std::cout << line << std::endl;
    }
}

bool replace(string& str, const string& from, const string& to)
{
    size_t pos = str.find_last_of(from);

    if (pos == string::npos)
    {
        return false;
    }

    str.replace(pos, from.length(), to);

    return true;
}