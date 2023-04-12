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

code_buffer::code_buffer(): _indent(0), _buffer(), _global_buffer()
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

size_t code_buffer::emit_from(std::istream& stream)
{
    if (stream.fail())
    {
        throw std::runtime_error("bad input steam.");
    }

    for (string line; std::getline(stream, line); )
    {
        emit(line);
    }

    return _buffer.size() - 1;
}

size_t code_buffer::emit_from_file(std::string path)
{
    ifstream file;

    file.open(path.c_str());

    return emit_from(file);
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
    for (auto& line : _global_buffer)
    {
        std::cout << line << std::endl;
    }

    for (auto& line : _buffer)
    {
        std::cout << line << std::endl;
    }
}

size_t code_buffer::emit_global(const std::string& line)
{
    _global_buffer.push_back(line);

    return _global_buffer.size() - 1;
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