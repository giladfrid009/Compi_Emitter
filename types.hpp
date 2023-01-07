#ifndef _TYPES_H_
#define _TYPES_H_

#include <string>

enum class fundamental_type { Invalid, Void, Int, Bool, Byte, String };

namespace types
{
    std::string to_string(fundamental_type type);

    fundamental_type from_string(std::string str);

    bool is_convertible(fundamental_type source, fundamental_type destiation);

    bool is_numeric(fundamental_type type);

    bool is_special(fundamental_type type);

    fundamental_type cast_up(fundamental_type first, fundamental_type second);
}

#endif