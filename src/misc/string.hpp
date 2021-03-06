//  Copyright 2016 Peter Georg
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef pMR_MISC_STRING_H
#define pMR_MISC_STRING_H

#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace pMR
{
    template<typename... Types>
    std::string toString(bool const arg, Types const &... args);

    template<typename T, typename... Types>
    std::string toString(T const &arg, Types const &... args);

    template<std::size_t N, typename... Types>
    std::string toString(char const (&arg)[N], Types const &... args);

    template<typename T, std::size_t N, typename... Types>
    std::string toString(std::array<T, N> const &arg, Types const &... args);

    template<typename T, typename A, typename... Types>
    std::string toString(std::vector<T, A> const &arg, Types const &... args);

    template<typename... Types>
    std::string toString(std::string const &arg, Types const &... args);

    std::string toString();
}

template<typename... Types>
std::string pMR::toString(bool const arg, Types const &... args)
{
    std::string str;
    std::ostringstream oss;
    oss << std::boolalpha;
    oss << arg;
    str += oss.str();
    str += " ";
    str += toString(args...);
    return str;
}

template<typename T, typename... Types>
std::string pMR::toString(T const &arg, Types const &... args)
{
    std::string str;
    std::ostringstream oss;
    oss << arg;
    str += oss.str();
    str += " ";
    str += toString(args...);
    return str;
}

template<std::size_t N, typename... Types>
std::string pMR::toString(char const (&arg)[N], Types const &... args)
{
    std::string str;
    str += arg;
    str += " ";
    str += toString(args...);
    return str;
}

template<typename T, std::size_t N, typename... Types>
std::string pMR::toString(std::array<T, N> const &arg, Types const &... args)
{
    std::string str;
    for(auto element : arg)
    {
        str += toString(element);
    }
    str += toString(args...);
    return str;
}

template<typename T, typename A, typename... Types>
std::string pMR::toString(std::vector<T, A> const &arg, Types const &... args)
{
    std::string str;
    for(auto element : arg)
    {
        str += toString(element);
    }
    str += toString(args...);
    return str;
}

template<typename... Types>
std::string pMR::toString(std::string const &arg, Types const &... args)
{
    std::string str;
    str += arg;
    str += " ";
    str += toString(args...);
    return str;
}
#endif // pMR_MISC_STRING_H
