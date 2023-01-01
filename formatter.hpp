#ifndef _FORMATTER_HPP_
#define _FORMATTER_HPP_

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

class formatter
{
    public:

    template<typename ... Args>
    static std::string format(const std::string& str, Args ... args)
    {
        int size_s = std::snprintf(nullptr, 0, str.c_str(), strignify(args) ...) + 1;
        
        if (size_s <= 0) 
        { 
            throw std::runtime_error("Error during formatting."); 
        }
        
        auto size = static_cast<size_t>(size_s);
        
        std::unique_ptr<char[]> buf(new char[size]);
        
        std::snprintf(buf.get(), size, str.c_str(), strignify(args) ...);
        
        return std::string(buf.get(), buf.get() + size - 1);
    }

    private:

    template<class T>
    static T const& strignify(T const& arg)
    {
        return arg;
    }

    static const char* strignify(std::string const& arg)
    {
        return arg.c_str();
    }
};

#endif