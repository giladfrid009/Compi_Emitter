#include "bp.hpp"
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

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

string code_buffer::generate_label()
{
    static unsigned long long count = 0;

    std::stringstream label;

    label << "label_" << count;

    count++;

    string ret = label.str();

    label << ":";

    emit(label.str());

    return ret;
}

int code_buffer::emit(const string& line)
{
    buffer.push_back(line);
    return buffer.size() - 1;
}

void code_buffer::backpatch(const vector<patch_record>& patch_list, const std::string& label)
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
        cout << line << endl;
    }
}

vector<patch_record> code_buffer::make_list(patch_record item)
{
    return vector<patch_record>({ item });
}

vector<patch_record> code_buffer::merge(const vector<patch_record>& first, const vector<patch_record>& second)
{
    vector<patch_record> new_list(first.begin(), first.end());
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
        cout << line << endl;
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