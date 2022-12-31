#include "bp.hpp"
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

bool replace(string& str, const string& from, const string& to, const branch_label_index index);

code_buffer::code_buffer(): buffer(), global_defs() { }

code_buffer& code_buffer::instance()
{
    static code_buffer inst;
    return inst;
}

string code_buffer::generate_label()
{
    std::stringstream label;
    label << "label_";
    label << buffer.size();
    std::string ret(label.str());
    label << ":";
    emit(label.str());
    return ret;
}

int code_buffer::emit(const string& s)
{
    buffer.push_back(s);
    return buffer.size() - 1;
}

void code_buffer::backpatch(const vector<pair<int, branch_label_index>>& address_list, const std::string& label)
{
    for (vector<pair<int, branch_label_index>>::const_iterator i = address_list.begin(); i != address_list.end(); i++)
    {
        int address = (*i).first;
        branch_label_index label_index = (*i).second;
        replace(buffer[address], "@", "%" + label, label_index);
    }
}

void code_buffer::print_code_buffer() const
{
    for (std::vector<string>::const_iterator it = buffer.begin(); it != buffer.end(); ++it)
    {
        cout << *it << endl;
    }
}

vector<pair<int, branch_label_index>> code_buffer::make_list(pair<int, branch_label_index> item)
{
    vector<pair<int, branch_label_index>> new_list;
    new_list.push_back(item);
    return new_list;
}

vector<pair<int, branch_label_index>> code_buffer::merge(const vector<pair<int, branch_label_index>>& l1, const vector<pair<int, branch_label_index>>& l2)
{
    vector<pair<int, branch_label_index>> new_list(l1.begin(), l1.end());
    new_list.insert(new_list.end(), l2.begin(), l2.end());
    return new_list;
}

void code_buffer::emit_global(const std::string& data_line)
{
    global_defs.push_back(data_line);
}

void code_buffer::print_global_buffer() const
{
    for (vector<string>::const_iterator it = global_defs.begin(); it != global_defs.end(); ++it)
    {
        cout << *it << endl;
    }
}

bool replace(string& str, const string& from, const string& to, const branch_label_index index)
{
    size_t pos;
    if (index == SECOND)
    {
        pos = str.find_last_of(from);
    }
    else
    { //FIRST
        pos = str.find_first_of(from);
    }
    if (pos == string::npos)
        return false;
    str.replace(pos, from.length(), to);
    return true;
}