#include "bp.hpp"
#include <list>
#include <string>
#include <iostream>
#include <stdexcept>

using std::list;
using std::string;

bool replace(string& str, const string& from, const string& to, const label_index index);

patch_record::patch_record(int line, label_index index): line(line), index(index)
{

}

code_buffer::code_buffer(): buffer(), global_defs()
{

}

code_buffer& code_buffer::instance()
{
    static code_buffer inst;
    return inst;
}

string code_buffer::emit_label()
{
    string label = label_name();

    emit(label + ":");

    return label;
}

int code_buffer::emit(const string& line)
{
    buffer.push_back(line);
    return buffer.size() - 1;
}

int code_buffer::emit_from_file(std::ifstream file)
{
    if (file.is_open() == false)
    {
        throw std::runtime_error("file not open.");
    }

    for (string line; std::getline(file, line); )
    {
        emit(line);
    }

    return buffer.size() - 1;
}

string code_buffer::register_name() const
{
    static unsigned long long count = 0;

    string reg = formatter::format("%%reg_%llu", count);

    count++;

    return reg;
}

string code_buffer::label_name() const
{
    static unsigned long long count = 0;

    string label = formatter::format("label_%llu", count);

    count++;

    return label;
}

void code_buffer::backpatch(const list<patch_record>& patch_list, const std::string& label)
{
    for (auto& entry : patch_list)
    {
        replace(buffer[entry.line], "@", "%" + label, entry.index);
    }
}

void code_buffer::print_code_buffer() const
{
    for (auto& line : buffer)
    {
        std::cout << line << std::endl;
    }
}

list<patch_record> code_buffer::make_list(patch_record item)
{
    return list<patch_record>({ item });
}

list<patch_record> code_buffer::merge(const list<patch_record>& first, const list<patch_record>& second)
{
    list<patch_record> new_list(first.begin(), first.end());
    new_list.insert(new_list.end(), second.begin(), second.end());
    return new_list;
}

void code_buffer::emit_global(const std::string& line)
{
    global_defs.push_back(line);
}

void code_buffer::print_global_buffer() const
{
    for (auto& line : global_defs)
    {
        std::cout << line << std::endl;
    }
}

bool replace(string& str, const string& from, const string& to, const label_index index)
{
    size_t pos = string::npos;

    if (index == label_index::first)
    {
        pos = str.find_first_of(from);
    }
    else if (index == label_index::second)
    {
        pos = str.find_last_of(from);
    }

    if (pos == string::npos)
    {
        return false;
    }

    str.replace(pos, from.length(), to);

    return true;
}