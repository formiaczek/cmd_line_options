/*
 * cmd_line_options.h
 *
 *  Created on: 2012-06-25
 *  Author: lukasz.forynski@gmail.com
 *
 *     Command line options template library is meant to provide an easy way for adding
 *   command-line options to your program. The goal is to be able to add these options
 *   in a generic and intuitive way that requires only minimal programming effort.
 *
 *    Programs, where command-line options are used, usually need to:
 *     - define command-line options,
 *     - parse command line arguments to check if (and which) of these options were specified,
 *     - attempt to extract parameters that a particular option requires (if any),
 *     - successful extraction usually results in further actions being taken by the program.
 *       These actions can, for example:
 *         - define or alter the behaviour of the program, e.g. by changing values of variable(s), or
 *         - result in calls to various functions to handle specified options.
 *       The later is often implemented as a 'switch/case' statement, usually defined in the "main()"
 *       function.
 *
 *   In order to try to simplify the above (trying to make it more fit for the purposes
 *   like above) this framework attempts to remove from the ( lazy ) programmer the need
 *   to define and implement most of these details. It also simplifies implementation of
 *   the program flow / logic that normally needs to be implemented manually.
 *
 *     All the programmer needs to do is to to express his requirements as functions.
 *   Command-line options created using these functions will automatically 'know' what
 *   parameter type(s) they expect and will implement all the details of extracting them
 *   and notifying if they weren't correct etc.
 *
 *     Unlike with other similar frameworks / libraries - there is no need to explicitly
 *   define parameter type(s) when creating the option. There is also no need to refer to
 *   their type again when looking at results of parsing.
 *
 *   The idea is simple, either:
 *     - we'll get a message what went wrong and why, or
 *     - our function(s) will be called and in parameters we'll get values that were passed
 *       directly.
 *
 *      Optionally dependencies between options can be defined and the framework will
 *   automatically implement all the logic needed to ensure that combination of specified
 *   command-line parameters is valid and allowed. This gives flexibility in defining options
 *   that require other additional options or do not allow use with other options - and
 *   once defined - it's all checked automatically: no need to implement it in your program,
 *   and if something isn't correct: framework will give the user a hint on what was wrong.
 *
 *   See the Wiki and examples for more details.
 *   Wiki: http://github.com/formiaczek/cmd_line_options/wiki
 *
 *  ________________________________________________________________
 *  Copyright (c) 2012 Lukasz Forynski <lukasz.forynski@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 *  software and associated documentation files (the "Software"), to deal in the Software
 *  without restriction, including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sub-license, and/or sell copies of the Software, and to permit persons
 *  to whom the Software is furnished to do so, subject to the following conditions:
 *
 *  - The above copyright notice and this permission notice shall be included in all copies
 *  or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 *  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 *  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 *  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

#ifndef CMD_LINE_OPTIONS_
#define CMD_LINE_OPTIONS_


#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <exception>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define MAX_LINE_SIZE 80

#define SPLIT_TO_NAME_AND_STR(identifier) identifier, #identifier

// in case there's no support for c++0x - below some
// defines to produce more descriptive
// compile-time assertions needed for our purposes.
#ifdef __cplusplus
#if __cplusplus <= 199711L  // if C++0x supported
template<bool> struct CTAssert;
template<> struct CTAssert<true>
{
};
#define PASTE0(x, y)  x ## y
#define PASTE(x, y) PASTE0(x, y)


#ifdef _MSC_VER
#define VSNPRINTF(dest, max_len, format, vargs) vsnprintf_s(dest, max_len, max_len, format, vargs)
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#define VSNPRINTF(dest, max_len, format, vargs) vsnprintf(dest, max_len, format, vargs)
#endif


/**
 * @brief Compile-time assert macro. Allows adding a message.
 * Note, - it was meant to be used within functions
 * Note, that 'what' message should be as variable name, so no spaces, special characters etc,
 * e.g. for assertion in line 15 defined like:
 *    ASSERT_COMPILE(expression, cant_compile_if_xxx);
 * compiler will generate error like:
 *    error: 'ct_assert_in_line_15__cant_compile_if_xxx' has incomplete type
 */
#define STATIC_ASSERT(x, what) \
        CTAssert< (x) >  PASTE(PASTE(ct_assert_in_line_, __LINE__), __##what); \
        (void)PASTE(PASTE(ct_assert_in_line_, __LINE__), __##what);

#else /* if C++0x supported - use static_assert()*/
#define STATIC_ASSERT(x, what) static_assert(x, #what)
#endif /*C++0x supported*/
#endif /*__cplusplus*/

/**
 @brief  option_error type to allow to reporting errors with some useful info.
 */
class option_error: public std::exception
{
public:
    /**
     @brief  Overloaded constructor. Use it as printf() for exceptions..
     */
    option_error(std::string format, ...)
    {
        if(format.length() > 0)
        {
            va_list vArgs;
            va_start(vArgs, format);
            VSNPRINTF(msg, MAX_MSG_LEN, format.c_str(), vArgs);
            va_end(vArgs);
        }
    }

    /**
     @brief  Overridden method to print exception info..
     */
    virtual const char* what() const throw ()
    {
        return msg;
    }
protected:
    enum constants
    {
        MAX_MSG_LEN = 512
    };
    char msg[MAX_MSG_LEN];
};

/**
 * @brief A helper class needed to safely allocate and use (using smar_tptr)
 *        An array of char - that is used below.
 */
class char_array
{
public:
    char_array(int size)
    {
        ptr = new char[size];
    }

    ~char_array()
    {
        delete[] ptr;
    }
    char* ptr;
};

/**
 * @brief Helper function to extract the whole token from the stringstream,
 * based on a specified delimiter list.
 */
inline std::string get_next_token(std::stringstream& from, std::string delimiter_list = "\"")
{
    std::string next_token;
    long where = static_cast<long>(from.tellg());
    if (where >= 0)
    {
        long max_token_size = from.str().size() - where;
        std::string temp_token = from.str().substr(where, max_token_size);
        int end = temp_token.find_first_of(delimiter_list);

        // skip all delimiters before next token
        while (end == 0 && max_token_size > 0)
        {
            from.get();
            where = static_cast<long>(from.tellg());
            max_token_size--;
            if (max_token_size > 0)
            {
                temp_token = from.str().substr(where, max_token_size);
                end = temp_token.find_first_of(delimiter_list);
                if (end < 0)
                {
                    end = max_token_size;
                    break;
                }
            }
        }
        if (max_token_size > 0)
        {
            next_token = from.str().substr(where, end);
            std::auto_ptr<char_array> buff(new char_array(next_token.size() + 1));
            from.get(buff->ptr, next_token.size() + 1);
        }
    }
    return next_token;
}

/**
 * @brief Helper function template that returns intersection of two vectors.
 * @param v1 first vector. Precondition: has to be sorted.
 * @param v2 second vector.
 * @return intersection of v1 and v2 (by value).
 */
template <class T>
inline std::vector<T> get_vect_intersection(std::vector<T>& v1, std::vector<T>& v2)
{
    std::vector<T> tmp(
                    std::max<size_t>(v1.size(), v2.size()));

    std::vector<std::string> isect = std::vector<T>(
                    tmp.begin(),
                    std::set_intersection(v1.begin(), v1.end(),
                                          v2.begin(), v2.end(),
                                          tmp.begin()));
    return isect;
}


/**
 * @brief Helper function template that returns difference between two vectors.
 * @param v1 first vector. Precondition: has to be sorted.
 * @param v2 second vector.
 * @return difference between v1 and v2 (by value).
 */
template <class T>
inline std::vector<T> get_vect_difference(std::vector<T>& v1, std::vector<T>& v2)
{
    std::vector<T> tmp(
                    std::max<size_t>(v1.size(), v2.size()));

    std::vector<std::string> isect = std::vector<T>(
                    tmp.begin(),
                    std::set_difference(v1.begin(), v1.end(),
                                          v2.begin(), v2.end(),
                                          tmp.begin()));
    return isect;
}


/**
 * @brief helper template function that merges values of a vector into a string.
 * @param vect reference to a source vector
 * @param parenthesis (optional) - default parenthesis in which each of the values will be inserted
 * @param separator (optional) - separator that will be placed between values
 * @return resulting string.
 */
template <class T>
inline std::string merge_from_vector(std::vector<T>& vect,
                                     char parenthesis = '\"',
                                     char separator=',')
{
    std::stringstream result;
    typename std::vector<T>::iterator i;
    for (i = vect.begin(); i != vect.end();i++)
    {
        if (i != vect.begin())
        {
            result << separator << " ";
        }
        result << parenthesis << *i << parenthesis;
    }
    return result.str();
}

/**
 * @brief This is a default template for a helper class used to extract parameters.
 *        It must not be used directly (in fact it's purpose is to report compile-time
 *        errors if cmd_line_parser::add_option() is called with a function for which any
 *        of the parameters can not be extracted (see below and examples for more detail.
 */
template<class ParamType, class Other = void>
class param_extractor
{
public:
    /**
     * @brief Default constructor. The onliy reason for it in generic template implementation
     *        is to create run-time error.
     */
    param_extractor()
    {
        // default template should fail at compile time: this will allow for
        // compile-time reporting if parameters of specified functions are supported.
        STATIC_ASSERT(false, EXTRACTING_PARAMETERS_OF_THIS_TYPE_IS_NOT_SUPPORTED);
    }

    /**
     * @brief Extracts a parameter and returns a value according to the type.
     *        In specialisation-classes this method should either attempt to extract
     *        the parameter from next token of the 'from' and return it by value, or it
     *        should throw an exception on failure.
     * @param from - stream pointing at the next token where this parameter is expected to be found.
     */
    static ParamType extract(std::stringstream& from)
    {
        return ParamType();
    }

    /**
     * @brief This method should return a 'usage' for the parameter. It will be called by the
     *        framework to prepare usage information for params that use it.
     */
    static std::string usage()
    {
        return std::string();
    }
};

/**
 * @brief A helper-macro that is using above template to define a temporary param_extractor.
 *        If - no specialisation was implemented for a ParamType, the default (above) template
 *        will report a compile time error.
 */
#define STATIC_ASSERT_IF_CAN_BE_EXTRACTED(param) \
		{ param_extractor<param> a; (void)a;}

/**
 * @brief Specialisation of param_extractor for "int" type.
 */
template<>
class param_extractor<int>
{
public:
    /**
     * @brief See generic template for description
     */
    static int extract(std::stringstream& from)
    {
        int param;
        int sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }
        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param * sign;
    }

    /**
     * @brief see generic template for description
     */
    static std::string usage()
    {
        return std::string("<int>");
    }
};

/**
 * @brief Specialisation of param_extractor for "unsigned int" type.
 */
template<>
class param_extractor<unsigned int>
{
public:
    /**
     * @brief See generic template for description.
     */
    static unsigned int extract(std::stringstream& from)
    {
        unsigned int param;
        int sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    static std::string usage()
    {
        return std::string("<unsigned int>");
    }
};

/**
 * @brief Specialisation of param_extractor for "long" type.
 */
template<>
class param_extractor<long>
{
public:
    /**
     * @brief See generic template for description.
     */
    static long extract(std::stringstream& from)
    {
        long param;
        long sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param * sign;
    }

    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<long>");
    }
};

/**
 * @brief Specialisation of param_extractor for "unsigned long" type.
 */
template<>
class param_extractor<unsigned long>
{
public:
    /**
     * @brief See generic template for description.
     */
    static unsigned long extract(std::stringstream& from)
    {
        unsigned long param;
        int sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<unsigned long>");
    }
};

/**
 * @brief Specialisation of param_extractor for "short" type.
 */
template<>
class param_extractor<short>
{
public:
    /**
     * @brief See generic template for description.
     */
    static short extract(std::stringstream& from)
    {
        short param;
        short sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param * sign;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<short>");
    }
};

/**
 * @brief Specialisation of param_extractor for "unsigned short" type.
 */
template<>
class param_extractor<unsigned short>
{
public:
    /**
     * @brief See generic template for description.
     */
    static unsigned short extract(std::stringstream& from)
    {
        unsigned short param;
        int sign = 1;
        std::stringstream token(get_next_token(from));
        token.unsetf(std::ios_base::skipws);

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        // try as if it was decimal first..
        token >> param;

        // but in case there was something left.. try it as hex
        if (!token.eof())
        {
            token.unget();
            token >> std::hex >> param;
        }

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<unsigned short>");
    }
};

/**
 * @brief Specialisation of param_extractor for "char" type.
 */
template<>
class param_extractor<char>
{
public:
    /**
     * @brief See generic template for description.
     */
    static char extract(std::stringstream& from)
    {
        char param;
        std::stringstream token(get_next_token(from));
        param = token.get();
        if (token.fail() || token.get() != std::char_traits<char>::eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<char>");
    }
};

/**
 * @brief Specialisation of param_extractor for "signed char" type.
 */
template<>
class param_extractor<signed char>
{
public:
    /**
     * @brief See generic template for description.
     */
    static signed char extract(std::stringstream& from)
    {
        signed char param;
        std::stringstream token(get_next_token(from));
        param = token.get();
        if (token.fail() || token.get() != std::char_traits<char>::eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<signed char>");
    }
};

/**
 * @brief Specialisation of param_extractor for "unsigned char" type.
 */
template<>
class param_extractor<unsigned char>
{
public:
    /**
     * @brief See generic template for description.
     */
    static unsigned char extract(std::stringstream& from)
    {
        unsigned char param;
        std::stringstream token(get_next_token(from));
        token.clear();
        param = token.get();
        if (token.fail() || token.get() != std::char_traits<char>::eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<unsigned char>");
    }
};

/**
 * @brief Specialisation of param_extractor for "std::string" type.
 */
template<>
class param_extractor<std::string>
{
public:
    /**
     * @brief See generic template for description
     */
    static std::string extract(std::stringstream& from)
    {
        std::string s = get_next_token(from);
        if (s.size() == 0)
        {
            throw option_error("%s, got \"\".", usage().c_str());
        }
        return s;
    }

    /**
     * @brief see generic template for description
     */
    static std::string usage()
    {
        return std::string("<string>");
    }
};

/**
 * @brief Specialisation of param_extractor for "float" type.
 */
template<>
class param_extractor<float>
{
public:
    /**
     * @brief See generic template for description.
     */
    static float extract(std::stringstream& from)
    {
        float param;
        std::stringstream token(get_next_token(from));
        token >> param;
        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<float>");
    }
};

/**
 * @brief Specialisation of param_extractor for "double" type.
 */
template<>
class param_extractor<double>
{
public:
    /**
     * @brief See generic template for description.
     */
    static double extract(std::stringstream& from)
    {
        double param;
        std::stringstream token(get_next_token(from));
        token >> param;

        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<double>");
    }
};

/**
 * @brief Specialisation of param_extractor for "long double" type.
 */
template<>
class param_extractor<long double>
{
public:
    /**
     * @brief See generic template for description.
     */
    static long double extract(std::stringstream& from)
    {
        long double param;
        std::stringstream token(get_next_token(from));

        token >> param;

        if (token.fail() || !token.eof())
        {
            throw option_error("%s, got \"%s\".", usage().c_str(), token.str().c_str());
        }
        return param;
    }
    /**
     * @brief See generic template for description.
     */
    static std::string usage()
    {
        return std::string("<long double>");
    }
};

/**
 * @brief Helper class to allow specifying default values for parameters
 *        And also allowing the framework to try to extract them accordingly.
 *        Since there is no syntactic way? to tell in C/C++ if the function
 *        takes default parameters, this function could define these parameters
 *        with the macro OPTIONAL_VALUE defined below.
 */
template<class T, T defalt_value>
class optional_value
{
public:
    /**
     * @brief Default constructor - it will assign a constant to the real value.
     */
    optional_value() :
                    value(defalt_value)
    {
    }

    /**
     * @brief Overloaded constructor - will copy parameter to value.
     *       It provides implicit conversion to allow to call function with type
     *       specified with default parameter (using OPTIONAL_VALUE) with the value
     *       (see description of OPTIONAL_VALUE).
     */
    optional_value(T val) :
                    value(val)
    {
    }

    /**
     * @brief Getter..
     */
    void set_value(T val)
    {
        value = val;
    }

    /**
     * @brief Setter..
     */
    T get_value()
    {
        return value;
    }

    T value;
};

/**
 * @brief Specialisation of param_extractor for "optional_value" type.
 */
template<class ParamType, ParamType default_val>
class param_extractor<optional_value<ParamType, default_val> >
{
public:

    /**
     * @brief Extracts a parameter and returns a value according to the ParamType.
     *        If extraction was not successful, from will be re-winded and
     *        default_value will be used for option that requires this type.
     */
    static optional_value<ParamType, default_val> extract(std::stringstream& from)
    {
        optional_value<ParamType, default_val> param;
        std::streamoff tellg = from.tellg();
        try
        {
            param.value = param_extractor<ParamType>::extract(from);
        } catch (option_error& /*e*/)
        {
            // in this case -this could have been some other (next) option
            // so unget (rewind) it allowing next parsers to continue from last place
            from.seekg(tellg);
        }
        return param;
    }

    /**
     * @brief This method should return a 'usage' for the parameter. It will be called by the
     *        framework to prepare usage information for params that use it.
     */
    static std::string usage()
    {
        std::stringstream usg;
        usg << param_extractor<ParamType>::usage() << "(optional=" << default_val <<")";
        return usg.str();
    }
};

/**
 * @brief Macro to be used to define default parameters in a function.
 *       TODO: due to C++ restrictions that only integer types can be
 *        used as template constant, only int  long (and also char) types
 *        can be used this way.. Well - perhaps could do it differently,
 *        but this was a neat and quick extension for at least these types:)
 *
 * Example:
 * @code
 * void fuc(int required_param1, OPTIONAL_VALUE(int, param2, 128)
 * {
 *    param2.get_val()  => will have a value that was specified, otherwise (if no proper integer specified) it will be 128.
 * }
 *  // note also, that this function can be used in rest of the program as standard overloaded function, e.g.:
 *  // e.g.: without specifying this param, e.g.:
 *  func(12); // 128 will be returned by get_val()
 *
 *  // or providing it:
 *  func(12, 324); // 324 will now be returned by get_val()
 * @endcode
 */
#define OPTIONAL_VALUE(type, name, value) optional_value<type, value> name = optional_value<type, value>()

/**
 * @brief Base class for options. It is mainly to provide a common interface
 *        To allow all options (sort of 'commands' to be called using a common interface).
 */
class option
{
public:
    /**
     * @brief Constructor.
     * @param  name - name of the option. It will be used also as a key that the option
     *         is selected from the command line.
     */
    option(std::string& name) :
                    standalone(false),
                    name(name)
    {
    }
    /**
     * @brief Destructor. Required because of virtuals, but there's nothing really to clean-up
     * at the moment.
     */
    virtual ~option()
    {
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options) = 0;

    /**
     * @brief adds another option that this option requires
     */
    void add_required_option(std::string option_name)
    {
        required_options.push_back(option_name);
        std::sort(required_options.begin(), required_options.end());
    }

    /**
     * @brief adds another option that this option must not be specified with.
     */
    void add_not_wanted_option(std::string option_name)
    {
        not_wanted_options.push_back(option_name);
        std::sort(not_wanted_options.begin(), not_wanted_options.end());
    }

    /**
     * @brief sets this option as a standalone (i.e. it must not be specified with any other option).
     */
    void set_as_standalone()
    {
        standalone = true;
    }

    /**
     * @brief Checks if specified options are valid with this option.
     * @param all_specified_options - vector of all specified options.
     * @throws option_error if specified options do not match requirements of this option.
     */
    void check_if_valid_with_these_options(std::vector<std::string> all_specified_options)
    {
        std::stringstream result;
        if (all_specified_options.size())
        {
            std::sort(all_specified_options.begin(), all_specified_options.end());

            if (required_options.size() && all_specified_options.size())
            {
                Container diff = get_vect_difference(required_options,
                                                     all_specified_options);
                if (diff.size())
                {
                    result << "option \"" << name << "\" requires also: ";
                    result << merge_from_vector(diff);
                }
            }

            if (not_wanted_options.size())
            {
                Container isect = get_vect_intersection(not_wanted_options,
                                                        all_specified_options);
                if (isect.size())
                {
                    if (result.str().size() == 0)
                    {
                        result << "option \"" << name << "\"";
                    }
                    else
                    {
                        result << ", and";
                    }
                    result << " can't be used with: ";
                    result << merge_from_vector(isect);
                }
            }

            if (standalone)
            {
                if (all_specified_options.size() > 1)
                {
                    result.str().clear();
                    result << "option \"" << name << "\"";
                    result << " can't be used with other options, but specified with: ";
                    all_specified_options.erase(std::remove(all_specified_options.begin(),
                                                            all_specified_options.end(),
                                                            name), all_specified_options.end());
                    result << merge_from_vector(all_specified_options);
                }
            }
        }

        if (result.str().length() > 0)
        {
            throw option_error("error: %s", result.str().c_str());
        }
    }

    /**
     * @brief Set the description of the program.
     * @param description - a sort of brief that would usually say what your tool is meant for etc.
     */
    void set_description(std::string& description)
    {
        descr = description;
    }

    /**
     * @brief Main interface that will be called by the cmd_line_parser if it will match a method being called.
     *        For each of the possible options - it will usually implement calls to type-dependent param_extractor
     *        methods to extract parameters and, on success (i.e. once all parameters are extracted)- it will call
     *        the function with these values.
     */
    virtual void execute() = 0;

    bool standalone;
    std::string name;
    std::string usage;
    std::string descr;

    typedef std::vector<std::string> Container;

    Container required_options;
    Container not_wanted_options;
};

/**
 * @brief Template for options taking no parameters.
 *        This template will be created / used to create a command-line options
 *        using existing functions that take no parameters.
 */
template<typename Fcn>
class option_no_params: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     */
    option_no_params(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
    }

    /**
     * @brief  Does nothing for this option type.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
    }

    /**
     * @brief Execute method. Calls to the function.
     */
    virtual void execute()
    {
        f();
    }
protected:
    /**
     * @brief Hide default constructor to allow creating only named-options.
     */
    option_no_params()
    {
    }
    Fcn f;
};

/**
 * @brief Template for options taking one parameter.
 *        This template will be created / used to create a command-line options
 *        using existing functions that take one parameter of type P1.
 */
template<typename Fcn, typename P1>
class option_1_param: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter type.
     */
    option_1_param(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
        usage = param_extractor<P1>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract the parameter and, on success - it calls the requested function.
     * @param input stream from which next token points to the parameter that needs to be extracted.
     */
    virtual void execute()
    {
        f(p1);
    }
protected:
    option_1_param()
    {
    }
    Fcn f;
    P1 p1;
};

template<typename Fcn, typename P1, typename P2>
class option_2_params: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_2_params(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(p1, p2);
    }
protected:
    option_2_params()
    {
    }
    Fcn f;
    P1 p1;
    P2 p2;
};

template<typename Fcn, typename P1, typename P2, typename P3>
class option_3_params: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_3_params(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
        p3 = param_extractor<P3>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(p1, p2, p3);
    }
protected:
    option_3_params()
    {
    }
    Fcn f;
    P1 p1;
    P2 p2;
    P3 p3;
};

template<typename Fcn, typename P1, typename P2, typename P3, typename P4>
class option_4_params: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_4_params(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage() + " ";
        usage += param_extractor<P4>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
        p3 = param_extractor<P3>::extract(cmd_line_options);
        p4 = param_extractor<P4>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(p1, p2, p3, p4);
    }
protected:
    option_4_params()
    {
    }
    Fcn f;
    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
};

template<typename Fcn, typename P1, typename P2, typename P3, typename P4, typename P5>
class option_5_params: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_5_params(Fcn f_ptr, std::string& name) :
                    option(name), f(f_ptr)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage() + " ";
        usage += param_extractor<P4>::usage() + " ";
        usage += param_extractor<P5>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
        p3 = param_extractor<P3>::extract(cmd_line_options);
        p4 = param_extractor<P4>::extract(cmd_line_options);
        p5 = param_extractor<P5>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(p1, p2, p3, p4, p5);
    }
protected:
    option_5_params()
    {
    }
    Fcn f;
    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
    P5 p5;
};

template<typename Fcn, typename ObjType>
class option_no_params_pass_obj: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     */
    option_no_params_pass_obj(Fcn f_ptr, ObjType* object_address, std::string& name) :
                    option(name), f(f_ptr), obj_addr(object_address)
    {
    }

    /**
     * @brief Calls the requested function passing it an address of the object specified when this option was created.
     * @param (not used)
     */
    virtual void execute(std::stringstream& /*cmd_line_options*/)
    {
        f(obj_addr);
    }
protected:
    option_no_params_pass_obj() :
                    obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
};

template<typename Fcn, typename ObjType, typename P1>
class option_1_param_pass_obj: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter type.
     */
    option_1_param_pass_obj(Fcn f_ptr, ObjType* object_address, std::string& name) :
                    option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameter and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameter.
     * @param input stream from which next tokens points to the parameter that needs to be extracted.
     */
    virtual void execute()
    {
        f(obj_addr, p1);
    }
protected:
    option_1_param_pass_obj() :
                    obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
    P1 p1;
};

template<typename Fcn, typename ObjType, typename P1, typename P2>
class option_2_params_pass_obj: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_2_params_pass_obj(Fcn f_ptr, ObjType* object_address, std::string& name) :
                    option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage();
    }

    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(obj_addr, p1, p2);
    }
protected:
    option_2_params_pass_obj() :
                    obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
    P1 p1;
    P2 p2;
};

template<typename Fcn, typename ObjType, typename P1, typename P2, typename P3>
class option_3_params_pass_obj: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_3_params_pass_obj(Fcn f_ptr, ObjType* object_address, std::string& name) :
                    option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage();
    }
    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
        p3 = param_extractor<P3>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(obj_addr, p1, p2, p3);
    }
protected:
    option_3_params_pass_obj() :
                    obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
    P1 p1;
    P2 p2;
    P3 p3;
};

template<typename Fcn, typename ObjType, typename P1, typename P2, typename P3, typename P4>
class option_4_params_pass_obj: public option
{
public:
    /**
     * @brief Constructor.
     * @param name - name of the option.
     *        This name is used as a keyword to select this option on the command line.
     *        It also prepares a 'usage' string based on parameter types.
     */
    option_4_params_pass_obj(Fcn f_ptr, ObjType* obj_address, std::string& name) :
                    option(name), f(f_ptr), obj_addr(obj_address)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage() + " ";
        usage += param_extractor<P4>::usage();
    }
    /**
     * @brief  Attempts to extract the parameter.
     * @param  input stream from which next token points to the parameter that needs to be extracted.
     * @throws option_error if param can't be extracted.
     */
    virtual void extract_params(std::stringstream& cmd_line_options)
    {
        p1 = param_extractor<P1>::extract(cmd_line_options);
        p2 = param_extractor<P2>::extract(cmd_line_options);
        p3 = param_extractor<P3>::extract(cmd_line_options);
        p4 = param_extractor<P4>::extract(cmd_line_options);
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute()
    {
        f(obj_addr, p1, p2, p3, p4);
    }
protected:
    option_4_params_pass_obj() :
                    obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
    P1 p1;
    P2 p2;
    P3 p3;
    P4 p4;
};


/**
 * @brief Wrapper class used to keep options and information about their groups etc.
 */
class grouped_options
{
public:
    /**
     * @brief TYpe for container used to keep options.
     */
    typedef std::map<std::string, option*> OptionContainer;

    /**
     * @brief Destructor. Cleans up allocated options.
     */
    ~grouped_options()
    {
        OptionContainer::iterator i;
        for (i = options.begin(); i != options.end(); i++)
        {
            delete i->second;
        }
    }

    /**
     * @brief Adds new group for options. Once this method is called, all options that
     *        Are added following this call will be added to this group (and listed under
     *        this group in help message).
     */
    void add_new_group(std::string name, std::string description = "")
    {
        groups.push_back(group(name, description));
    }

    /**
     * @brief Adds new option.
     */
    void add_new_option(option* new_option)
    {
        if(new_option)
        {
            if(!groups.size())
            {
                add_new_group("Options");
            }

            std::string name = new_option->name;
            if(options.find(name) == options.end())
            {
                options.insert(std::make_pair(name, new_option));
                std::vector<group>::iterator g = groups.end() - 1;
                g->add_option(name);
            }
        }
    }

    /**
     * @brief Fins option of a given name.
     * @param name - name of the option to find.
     * @return  - pointer to option if found, NULL otherwise.
     */
    option* find_option(std::string name)
    {
        option* result = NULL;
        OptionContainer::iterator i = options.find(name);
        if(name.length() && i != options.end())
        {
            result = i->second;
        }
        return result;
    }

    /**
     * @brief Returns size of options.
     */
    size_t size()
    {
        return options.size();
    }

    /**
     * @brief Creates help using all information about options and their groups.
     * @param help_content - stream into which help message is inserted.
     */
    void create_help(std::stringstream& help_content)
    {
        size_t max_cmd_len = 0;
        OptionContainer::iterator i;
        for (i = options.begin(); i != options.end(); i++)
        {
            const std::string& s = i->first;
            max_cmd_len = std::max<size_t>(max_cmd_len, s.length());
        }
        max_cmd_len += 1;



        std::vector<group>::iterator g;
        for(g = groups.begin(); g != groups.end(); g++)
        {
            help_content << "\n" << g->name();
            if(g->description().length())
                {
                help_content << "(" << g->description() << ")";
                }
            help_content << ":\n";

            group::options_iterator oi;

            for (oi = g->options_begin(); oi != g->options_end(); oi++)
            {
                const std::string& s = *oi;
                option* o = find_option(s);

                int indent_size = max_cmd_len - s.length();

                help_content << "  " << s ; // option name
                help_content << std::string(indent_size, ' ') << ": ";

                std::stringstream d(o->descr);
                std::string next_desc_part;
                size_t curr_desc_len = 0;
                do
                {
                    d >> next_desc_part;
                    curr_desc_len += next_desc_part.length();

                    if (curr_desc_len >= MAX_LINE_SIZE - max_cmd_len)
                    {
                        help_content << '\n' << std::string(max_cmd_len + 4, ' ');
                        curr_desc_len = next_desc_part.length();
                    }
                    help_content << next_desc_part << " ";

                } while (d.good());

                help_content << "\n";

                indent_size = max_cmd_len > 4 ? max_cmd_len - 4 : max_cmd_len;
                help_content << std::string(indent_size, ' ');
                help_content  << "usage : " << s << " " << o->usage;
                help_content << "\n\n";
            }
        }
    }


protected:

    /**
     * @brief Helper class to allow associating options with groups.
     */
    class group
    {
    public:
        /**
         * @brief Constructor.
         * @param name - name of the group.
         * @param (optional) description of the option group
         */
        explicit group(std::string name, std::string description = "") :
            group_name(name), group_description(description)
        {
        }

        /**
         * @brief Returns name of the group.
         */
        std::string name()
        {
            return group_name;
        }

        /**
         * @brief Returns description of the group.
         */
        std::string description()
        {
            return group_description;
        }

        /**
         * @brief Adds option to the group.
         */
        void add_option(std::string name)
        {
            option_names.push_back(name);
        }

        /**
         * @brief Iterator for options.
         */
        typedef std::vector<std::string>::iterator options_iterator;

        /**
         * @brief Helper for iterators  to allow iterating through options within this group.
         */
        options_iterator options_begin()
        {
            return option_names.begin();
        }

        /**
         * @brief Helper for iterators  to allow iterating through options within this group.
         */
        options_iterator options_end()
        {
            return option_names.end();
        }

    private:
        std::string group_name;
        std::string group_description;
        std::vector<std::string> option_names;
    };

    std::map<std::string, option*> options;
    std::vector<group> groups;
};


/**
 * @brief This is the main class of this library.
 */
class cmd_line_parser
{
public:
    /**
     * @brief Options should be unique and referred by the key, so they are kept
     *        by the parser using a map.
     */
    typedef grouped_options OptionContainer;

    /**
     * @brief Default constructor.
     */
    cmd_line_parser() :
                    version("(not set)"), default_option(NULL), other_args_handler(NULL)
    {
    }


    /**
     * @brief Method to set the description of the program.
     * @param desc - brief description of what the program does.
     */
    void set_description(std::string desc)
    {
        description = desc;
    }

    /**
     * @brief Method to set the version of the program.
     * @param new_version - new version (as string) to be used / presented by the program.
     */
    void set_version(std::string new_version)
    {
        version = new_version;
    }

    /**
     * @brief adds new group of options. All options that are added following this call
     *        will be associated with this group. If this method is not called before
     *        first option is added- a default group called "Options" is created
     *        (although it is still valid to add new option groups in such case).
     */
    void add_group(std::string group_name, std::string description="")
    {
        options.add_new_group(group_name, description);
    }

    /**
     * @brief Method to display help. This involves generating and printing to stdout:
     *        - name of the executable (from argv[0])
     *        - version
     *        - description
     *        - list of options and usage information
     */
    void display_help()
    {
        std::stringstream help;

        help << "\n" << program_name;
        help << ", version: " << version << "\n\n";
        help << description << std::endl;

        if (default_option != NULL)
        {
            help << "\n " << default_option->descr;
            help << "\n\n     " << "Usage : " << program_name;
            help << " " << default_option->usage;
            help << "\n\n";
        }
        else
        {
           options.create_help(help);
        }

        help << "\n Use \"?\", \"-h\" or \"--help\" to print this help message.\n";
        std::cout << help.str();
    }


    void setup_required_options(std::string list_of_required_options)
    {
        // now extract options from the list, check and add them to current one
        std::stringstream s(list_of_required_options);
        std::string next_option_name;

        next_option_name = get_next_token(s, " ,;\"\t\n\r");
        while (next_option_name.length() > 0)
        {
            option* other_option = options.find_option(next_option_name);
            if (other_option != NULL)
            {
                required_options.push_back(next_option_name);
                std::sort(required_options.begin(), required_options.end());
            }
            else
            {
                throw option_error(
                                "error: setting option \"%s\" as required failed: option not valid",
                                next_option_name.c_str());
            }
            next_option_name = get_next_token(s, " ,;\"\t\n\r");
        }
    }

    /**
     * @brief Use this method to specify dependent options that also need to be present
     *        whenever option_name is specified.
     * @param option_name option name, for which dependent options are being specified
     * @param list_of_dependent_options string containing list of options (comma/semicolon/space separated)
     * @throws option_error if any of specified options is not valid (i.e. has not been previously added)
     */
    void setup_dependent_required(std::string option_name, std::string list_of_dependent_options)
    {
        try
        {
            try_to_add_dependent_options(option_name, list_of_dependent_options,
                                         &option::add_required_option);
        } catch (option_error& err)
        {
            std::cout << err.what() << std::endl;
            throw; // re-throw. This should indicate to the user that setup is wrong..
        }
    }

    /**
     * @brief Use this method to specify dependent options that must not be present
     *        whenever option_name is specified.
     * @param option_name option name, for which dependent options are being specified
     //     * @param list_of_not_wanted_options string containing list of options (comma/semicolon/space separated)
     * @throws option_error if any of specified options is not valid (i.e. has not been previously added)
     */
    void setup_dependent_not_wanted(std::string option_name, std::string list_of_not_wanted_options)
    {
        try
        {
            try_to_add_dependent_options(option_name, list_of_not_wanted_options,
                                         &option::add_not_wanted_option);
        } catch (option_error& err)
        {
            std::cout << err.what() << std::endl;
            throw; // re-throw. This should indicate to the user that setup is wrong..
        }
    }

    /**
     * @brief Use this method to setup an option to be the only one, that can be specified.
     *        i.e. no other option must be used with it at the same time.
     * @param option_name option name, for which dependent options are being specified
     * @throws option_error if option is not valid (i.e. has not been previously added)
     */
    void setup_option_as_standalone(std::string option_name)
    {
        option* o = options.find_option(option_name);
        if (o == NULL)
        {
            throw option_error(
                            "error: adding dependencies for option \"%s\" failed, option is not valid",
                            option_name.c_str());
        }
        else
        {
            o->set_as_standalone();
        }
    }

    /**
     * @brief Typedef for handler to be used with add_handler_for_other_options.
     */
    typedef void (*other_arguments_handler)(std::vector<std::string>& other_arguments);

    /**
     * @brief Adds handler for other (unrecognised) arguments.
     *        If this method is used and proper handler is added: all command-line arguments
     *        not recognised as options or option-arguments will be passed to this handler
     *        at the end of execution (i.e. once all option-handlers have executed).
     */
    void add_handler_for_other_arguments(other_arguments_handler handler)
    {
        other_args_handler = handler;
    }

    /**
     * @brief When done creating / adding options, run this method giving proper argc/argv values
     *        To parse command-line options. All command-line arguments will be parsed.
     *        If all is successful - appropriate handlers (functions used to create options) will
     *        be executed in the order they were found in cmd-line arguments. By default - options
     *        can be specified multiple times, and if all parameters are correct, their functions
     *        will be called multiple times with parsed values. Once all recognised option-handlers
     *        (functions) have executed - a handler for other arguments is executed.
     * @param argc: number of elements in argv
     * @param argv: command-line parameters.
     * @return true - if parsing was successful, false otherwise or if help was requested.
     * @throws option_error if argc/argv are not valid.
     */
    bool run(int argc, char *const argv[])
    {
        bool result = false;
        std::stringstream cmd_line(convert_cmd_line_to_string(argc, argv));

        if (default_option != NULL)
        {
            if(!handle_default_option(cmd_line))
                {
                // will return false if it's help or error extracting
                // params. No point to contiune any further for default option
                // (otherwise - if returns true: following loop would extract
                // other (non-option) params from cmd_line etc.
                return false;
                }
        }

        bool found = false;
        do
        {
            try
            {
                found = could_find_next_option(cmd_line);
            } catch (option_error& err)
            {
                std::cout << err.what();
                return false;
            }
        } while (found);


        result = check_options_and_execute();

        // regardless of result from options - execute other_args_handler
        // and update result if successful
        if (other_args_handler != NULL && other_args.size() > 0)
        {
            other_args_handler(other_args);
            result = true;
        }
        return result;
    }

    /**
     * @brief Checks if option was specified.
     * @param option_name name of option to check.
     * @return true if option was specified, false otherwise.
     */
    bool check_if_option_specified(std::string option_name)
    {
        std::set<std::string> el(execute_list.begin(), execute_list.end()); // lazy 'two-liner' way..
        return (el.find(option_name) != el.end());
    }

    template<class RetType>
    void add_option(RetType function_ptr(), std::string name, std::string description);

    template<class RetType, typename P1>
    void add_option(RetType function_ptr(P1), std::string name, std::string description);

    template<class RetType, typename P1, typename P2>
    void add_option(RetType function_ptr(P1, P2), std::string name, std::string description);

    template<class RetType, typename P1, typename P2, typename P3>
    void add_option(RetType function_ptr(P1, P2, P3), std::string name, std::string description);

    template<class RetType, typename P1, typename P2, typename P3, typename P4>
    void add_option(RetType function_ptr(P1, P2, P3, P4), std::string name,
                    std::string description);

    template<class RetType, typename P1, typename P2, typename P3, typename P4, typename P5>
    void add_option(RetType function_ptr(P1, P2, P3, P4, P5), std::string name,
                    std::string description);

// adding options for functions taking pointer (to object) as a first parameter
    template<class RetType, typename ObjType>
    void add_option(RetType function_ptr(ObjType*), ObjType* obj_address, std::string name,
                    std::string description);

    template<class RetType, typename ObjType, typename P1>
    void add_option(RetType function_ptr(ObjType*, P1), ObjType* obj_address, std::string name,
                    std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2>
    void add_option(RetType function_ptr(ObjType*, P1, P2), ObjType* obj_address, std::string name,
                    std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2, typename P3>
    void add_option(RetType function_ptr(ObjType*, P1, P2, P3), ObjType* obj_address,
                    std::string name, std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2, typename P3, typename P4>
    void add_option(RetType function_ptr(ObjType*, P1, P2, P3, P4), ObjType* obj_address,
                    std::string name, std::string description);
protected:
    /**
     * @brief Internal method to add a raw-option.
     *        It adds an option or a default option (if name of option is zero-length),
     *        performing various checks if it is valid do to so.
     */
    void add_option(option* a, std::string description)
    {
        std::stringstream err;
        if (a != NULL)
        {
            if (a->name.length() != 0)
            {
                if (default_option == NULL)
                {
                    a->set_description(description);
                    if (options.find_option(a->name) != NULL) // TODO: perhaps should move it to add_new_option() ?
                    {
                        err << __FUNCTION__ << "(): option \"" << a->name << "\" already exists";
                    }
                    else
                    {
                        options.add_new_option(a);
                    }
                }
                else
                {
                    err << __FUNCTION__ << "(): trying to add \"" << a->name;
                    err << "\" option, but default option was set";
                }
            }
            else
            {
                if (default_option == NULL)
                {
                    if(options.size() > 0)
                    {
                        err << __FUNCTION__ << "(): Trying to add default option when other options exist";
                    }
                    else
                    {
                        default_option = a;
                        default_option->set_description(description);
                    }
                }
                else
                {
                    err << __FUNCTION__;
                    err << "(): Trying to add another default option";
                }
            }
        }
        else
        {
            err << __FUNCTION__ << "(): option can't be NULL";
        }

        if (err.str().length() > 0)
        {
            throw option_error("%s", err.str().c_str());
        }
    }

    /**
     * @brief Internal method to extract program name and the rest of arguments
     *        from argc/argv
     * @returns string containing parameters, delimited by space.
     * @throws option_error if argc / argv are not valid.
     */
    std::string convert_cmd_line_to_string(int argc, char* const argv[])
    {
        if (argv == NULL || argc < 1)
        {
            throw option_error("%s(): argc/argv are not valid", __FUNCTION__);
        }

        std::string cmd_line;
        program_name = argv[0];
        int path_end = program_name.rfind('\\');
        if (path_end < 0)
        {
            path_end = program_name.rfind('/');
            if (path_end > 0)
            {
                program_name.erase(0, path_end + 1);
            }
        }

        if (argc > 1)
        {
            int cnt = 1;
            while (cnt < argc)
            {
                // surround them with ""
                cmd_line += '\"';
                cmd_line += argv[cnt++];
                cmd_line += "\"";
            }
            // strip it at the end (removing also space added above)
            cmd_line.erase(cmd_line.find_last_not_of(" \t\n\r") + 1);
        }
        return cmd_line;
    }

    bool is_it_help(std::stringstream& from)
    {
        std::string option;
        std::streamoff pos = from.tellg();
        option = get_next_token(from);
        bool is_help = false;

        std::string h(option);
        h.erase(0, h.find_first_not_of("-"));
        std::transform(h.begin(), h.end(), h.begin(), ::tolower);
        if (h == "?" || h == "h" || h == "help")
        {
            is_help = true;
        }
        else
        {
            from.seekg(pos);
        }
        return is_help;
    }

    void try_to_extract_params(option* opt, std::stringstream& from)
    {
        if(opt != NULL)
        {
            try
            {
                opt->extract_params(from);

            } catch (option_error& e)
            {
                std::stringstream s;
                int indent_size = 0;
                if(opt->name.length() > 0)
                {
                    s << "\nOption: \"" << opt->name << "\": ";
                    indent_size = opt->name.size();
                }
                else
                {
                    s << "\n " << program_name << ": ";
                    indent_size = program_name.size();
                }
                s << "error while parsing parameters,\n";
                s << std::string(indent_size + 3, ' ') << "expected: " << e.what();
                s << "\n\n" << opt->descr << "\n";
                s << "\n Usage: \n    " << program_name << " ";
                if(opt->name.length()>0)
                    {
                    s << opt->name << " ";
                    }
                s << opt->usage;
                throw option_error("%s\n", s.str().c_str());
            }
        }
        else
        {
            throw option_error("error using %s(): option can't be NULL", __FUNCTION__);
        }
    }

    /**
     * @brief Internal method to check if there are more options in the stream
     *        IF there are any, true will be returned and option list will be stored
     *        in the execute_list. Additionally - if other_args_handler is not NULL
     *        any unrecognised options will be added to other_args.
     * @returns true if new option has been found, false - otherwise.
     * @throws option_error if option is not valid or parameters for the option
     *         that has been found are not correct.
     */
    bool could_find_next_option(std::stringstream& from)
    {
        std::string option_name;
        bool found = false;

        if (is_it_help(from))
        {
            display_help();
            execute_list.clear();
        }
        else
        {
            option_name = get_next_token(from);

            if (option_name.length() != 0)
            {
                option* o = options.find_option(option_name);
                if(o != NULL)
                {
                    try_to_extract_params(o, from);
                    execute_list.push_back(option_name); // TODO: if options can be specified more than once - we should really make copies of option* objects here..
                    found = true;
                }
                else
                {
                    if (other_args_handler == NULL/* && default_option == NULL*/)
                    {
                        throw option_error(
                                        "\"%s\": no such option, try \"?\" or \"help\" to see usage.\n",
                                        option_name.c_str());
                    }
                    else
                    {
                        other_args.push_back(option_name);
                        found = true;
                    }
                }
            }
        }
        return found;
    }

    /**
     * @brief Internal typedef for option member pointer (will be used either for
     *        option::add_required_option or option::add_not_wanted_option
     */
    typedef void (option::*operation_type)(std::string);

    /**
     * @brief Internal method to add selected list of option to either required or not wanted list.
     * @throws option_error any of specified options is not valid.
     */
    void try_to_add_dependent_options(std::string& to_option, std::string& list_of_options,
                    operation_type add_dependent_option)
    {
        option* curr_option = options.find_option(to_option);
        if (curr_option == NULL)
        {
            throw option_error(
                            "error: adding dependencies for option \"%s\" failed, option is not valid",
                            to_option.c_str());
        }


        // now extract options from the list, check and add them to current one
        std::stringstream s(list_of_options);
        std::string next_option_name;

        next_option_name = get_next_token(s, " ,;\"\t\n\r");
        while (next_option_name.length() > 0)
        {
            option* other_option = options.find_option(next_option_name);
            if (other_option != NULL)
            {
                (curr_option->*add_dependent_option)(next_option_name);
            }
            else
            {
                throw option_error(
                                "error: adding dependencies for option \"%s\" failed: option \"%s\" is not valid",
                                to_option.c_str(), next_option_name.c_str());
            }
            next_option_name = get_next_token(s, " ,;\"\t\n\r");
        }
    }

    bool handle_default_option(std::stringstream& cmd_line)
    {
        bool result = false;
        if (is_it_help(cmd_line))
        {
            display_help();
        }
        else
        {
            try
            {
                try_to_extract_params(default_option, cmd_line);
                result = true;
            } catch (option_error& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
        return result;
    }


    bool check_options_and_execute()
    {
        bool result = false;
        if (default_option != NULL)
        {
            default_option->execute();
            result = true;
        }
        else
        {
            if (required_options.size())
            {
                std::stringstream result;
                if (required_options.size())
                {
                    std::vector<std::string> isect = get_vect_intersection(required_options, execute_list);
                    if (isect.size() != required_options.size())
                    {
                        result << "required following option(s): \n ";
                        result << merge_from_vector(required_options) << "\n\n";

                        if(execute_list.size())
                        {
                            result << "but specified only:\n ";
                            result << merge_from_vector(execute_list);
                        }
                        else
                        {
                            result << "but nothing was specified";
                        }
                        std::cout << "error: " << result.str() << "\n\n";
                        return false;
                    }
                }
            }

            std::vector<std::string>::iterator i;
            for (i = execute_list.begin(); i != execute_list.end(); i++)
            {
                try
                {
                    option* option_to_execute = options.find_option(*i);
                    if(option_to_execute != NULL) // TODO: assert? it's not possible that this is NULL, unless a bug is introduced during development etc.
                        {
                        option_to_execute->check_if_valid_with_these_options(execute_list);
                        }
                } catch (option_error& e)
                {
                    std::cout << e.what() << std::endl;

                    // should skip any execution if options were not right.
                    execute_list.clear();
                    break;
                }
            }

            if (execute_list.size())
            {
                for (i = execute_list.begin(); i != execute_list.end(); i++)
                {
                    option* option_to_execute = options.find_option(*i);

                    if(option_to_execute) // TODO: similarly here..
                        {
                        option_to_execute->execute();
                        }
                }
                result = true;
            }
        }
        return result;
    }


    OptionContainer options;
    std::string description;
    std::string program_name;
    std::string version;
    option* default_option;
    other_arguments_handler other_args_handler;
    std::vector<std::string> other_args;
    std::vector<std::string> execute_list;
    std::vector<std::string> required_options;
};

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func() as a prototype for the option.
 * @param function_ptr: a pointer to the function defined as above.
 * @param name: name of the option. This name is a key, that the command-line option is selected with.
 *        It should be unique, and if an option with a given name exists - option_error will be thrown.
 * @param description: a sort of brief explanation what the option is meant for.
 * @return id for_this option, or -1 if it couldn't be added (e.g. option with the same name exists).
 *         This id can be used against cmd_line_parser::run() return to see if this option has executed.
 */
template<class RetType>
inline void cmd_line_parser::add_option(RetType function_ptr(), std::string name,
                std::string description)
{
    add_option(new option_no_params<RetType (*)()>(function_ptr, name), description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1) as a prototype for the option.
 * @param function_ptr: a pointer to the function defined as above.
 * @param name: name of the option. This name is a key, that the command-line option is selected with.
 *        It should be unique, and if an option with a given name exists - option_error will be thrown.
 * @param description: a sort of brief explanation what the option is meant for.
 * @return id for_this option, or -1 if it couldn't be added (e.g. option with the same name exists).
 *         This id can be used against cmd_line_parser::run() return to see if this option has executed.
 * @note that this template uses STATIC_ASSERT macros to check if each of the parameters could be extracted.
 *       This is to ensure at compile time - that the function pointed by function_ptr is suitable.
 *       if you get this assertion it means that one or more of the parameter types for this function is
 *       not supported - and such a function can't be used as 'command-line option' prototype.
 */
template<class RetType, typename P1>
inline void cmd_line_parser::add_option(RetType function_ptr(P1), std::string name,
                std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    add_option(new option_1_param<RetType (*)(P1), P1>(function_ptr, name), description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2) as a prototype for the option.
 * @param function_ptr: a pointer to the function defined as above.
 * @param name: name of the option. This name is a key, that the command-line option is selected with.
 *        It should be unique, and if an option with a given name exists - option_error will be thrown.
 * @param description: a sort of brief explanation what the option is meant for.
 * @return id for_this option, or -1 if it couldn't be added (e.g. option with the same name exists).
 *         This id can be used against cmd_line_parser::run() return to see if this option has executed.
 * @note that this template uses STATIC_ASSERT macros to check if each of the parameters could be extracted.
 *       This is to ensure at compile time - that the function pointed by function_ptr is suitable.
 *       if you get this assertion it means that one or more of the parameter types for this function is
 *       not supported - and such a function can't be used as 'command-line option' prototype.
 */
template<class RetType, typename P1, typename P2>
inline void cmd_line_parser::add_option(RetType function_ptr(P1, P2), std::string name,
                std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    add_option(new option_2_params<RetType (*)(P1, P2), P1, P2>(function_ptr, name), description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3) as a prototype for the option.
 * Description is similar to other similar add_option method templates.
 */
template<class RetType, typename P1, typename P2, typename P3>
inline void cmd_line_parser::add_option(RetType function_ptr(P1, P2, P3), std::string name,
                std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    add_option(new option_3_params<RetType (*)(P1, P2, P3), P1, P2, P3>(function_ptr, name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3, P4) as a prototype for the option.
 * Description is similar to other similar add_option method templates.
 */
template<class RetType, typename P1, typename P2, typename P3, typename P4>
inline void cmd_line_parser::add_option(RetType function_ptr(P1, P2, P3, P4), std::string name,
                std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    add_option(new option_4_params<RetType (*)(P1, P2, P3, P4), P1, P2, P3, P4>(function_ptr, name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3, P4, P5) as a prototype for the option.
 *  Note: Defining templates for options (functions) taking more than 5 parameters would be a little bit crazy!!
 *        In fact, 5 is crazy already, but feel free to copy-paste-extend more templates if you really need more :P
 * Description is similar to other similar add_option method templates.
 */
template<class RetType, typename P1, typename P2, typename P3, typename P4, typename P5>
inline void cmd_line_parser::add_option(RetType function_ptr(P1, P2, P3, P4, P5), std::string name,
                std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P5);
    add_option(new option_5_params<RetType (*)(P1, P2, P3, P4, P5), P1, P2, P3, P4, P5>(function_ptr, name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*) as a prototype for the option.
 * @param function_ptr: a pointer to the function defined as above.
 * @param obj_address: address of the object of ObjType. This address will be passed
 *        to the function, when the option is selected / executed.
 * @param name: name of the option. This name is a key, that the command-line option is selected with.
 *        It should be unique, and if an option with a given name exists - option_error will be thrown.
 * @param description: a sort of brief explanation what the option is meant for.
 * @return id for_this option, or -1 if it couldn't be added (e.g. option with the same name exists).
 *         This id can be used against cmd_line_parser::run() return to see if this option has executed.
 */
template<class RetType, typename ObjType>
inline void cmd_line_parser::add_option(RetType function_ptr(ObjType*), ObjType* obj_address,
                std::string name, std::string description)
{
    add_option(new option_no_params_pass_obj<RetType (*)(ObjType*), ObjType>(function_ptr,
                                                                            obj_address, name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1) as a prototype for the option.
 * @param function_ptr: a pointer to the function defined as above.
 * @param obj_address: address of the object of ObjType. This address will be passed
 *        to the function, when the option is selected / executed.
 * @param name: name of the option. This name is a key, that the command-line option is selected with.
 *        It should be unique, and if an option with a given name exists - option_error will be thrown.
 * @param description: a sort of brief explanation what the option is meant for.
 * @return id for_this option, or -1 if it couldn't be added (e.g. option with the same name exists).
 *         This id can be used against cmd_line_parser::run() return to see if this option has executed.
 * @note that this template uses STATIC_ASSERT macros to check if each of the parameters could be extracted.
 *       This is to ensure at compile time - that the function pointed by function_ptr is suitable.
 *       if you get this assertion it means that one or more of the parameter types for this function is
 *       not supported - and such a function can't be used as 'command-line option' prototype.
 */
template<class RetType, typename ObjType, typename P1>
inline void cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1), ObjType* obj_address,
                std::string name, std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    add_option(new option_1_param_pass_obj<RetType (*)(ObjType*, P1), ObjType, P1>(function_ptr,
                                                                              obj_address, name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2) as a prototype for the option.
 * Description is similar to other add_option method templates that take ObjType parameter.
 */
template<class RetType, typename ObjType, typename P1, typename P2>
inline void cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1, P2),
                ObjType* obj_address, std::string name, std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    add_option(new option_2_params_pass_obj<RetType (*)(ObjType*, P1, P2), ObjType, P1, P2>(function_ptr,
                                                                                   obj_address,
                                                                                   name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2, P3) as a prototype for the option.
 * Description is similar to other add_option method templates that take ObjType parameter.
 */
template<class RetType, typename ObjType, typename P1, typename P2, typename P3>
inline void cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1, P2, P3),
                ObjType* obj_address, std::string name, std::string description)
{
//
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    add_option(new option_3_params_pass_obj<RetType (*)(ObjType*, P1, P2, P3), ObjType, P1, P2, P3>(function_ptr,
                                                                                       obj_address,
                                                                                       name),
               description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2, P3, P4) as a prototype for the option.
 *  Note: Defining templates for options (functions) taking more than 5 parameters would be a little bit crazy!!
 *        In fact, 5 is crazy already, but feel free to copy-paste-extend more templates if you really need more :P
 * Description is similar to other add_option method templates that take ObjType parameter.
 */
template<class RetType, typename ObjType, typename P1, typename P2, typename P3, typename P4>
inline void cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1, P2, P3, P4),
                ObjType* obj_address, std::string name, std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    add_option(new option_4_params_pass_obj<RetType (*)(ObjType*, P1, P2, P3, P4), ObjType, P1, P2, P3, P4>(
                    function_ptr, obj_address, name),
                    description);
}

#endif /* CMD_LINE_OPTIONS_ */
