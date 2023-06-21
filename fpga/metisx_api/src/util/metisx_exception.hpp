#pragma once
#include <string>
#if (_SIM_)
    #include <iostream>
    #include <assert.h>
#endif

namespace metisx
{

namespace api
{
namespace util
{
class MetisException : public std::exception
{
        std::string message;

    public:
        MetisException(std::string _m)
            : message(_m)
        {
#if (_SIM_)
            std::cout << _m << std::endl;
            assert(0);
#endif
        }

        virtual const char* what() const throw()
        {

            return message.c_str();
        }
};
} // namespace util
} // namespace api

} // namespace metisx