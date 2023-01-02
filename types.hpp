#ifndef _TYPES_H_
#define _TYPES_H_

#include <string>

enum class type_kind { Invalid, Void, Int, Bool, Byte, String };

namespace types
{
    std::string to_string(type_kind type);

    type_kind parse(const std::string& str);

    bool is_implictly_convertible(type_kind from, type_kind to);

    bool is_numeric(type_kind type);

    bool is_special(type_kind type);

    type_kind cast_up(type_kind first, type_kind second);
}

#endif