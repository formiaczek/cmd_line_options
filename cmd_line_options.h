/*
 * cmd_line_options.h
 *
 *  Created on: 2012-06-25
 *  Author: Lukasz Forynski
 *          (lukasz.forynski@gmail.com)
 *
 *   Command line options template library is meant to provide an easy way
 *   of adding command-line options to your program. Defining and adding these
 *   options is very generic and intuitive way and requires minimal programming effort.
 *   All that is needed is to express requirements as functions, and the framework
 *   will automatically match appropriate templates to implement all the logic.
 *
 *   In most cases programs, where command-line options are used, usually need to:
 *    - define these command-line options,
 *    - parse command line arguments to check if (and which) of these options was specified
 *    - attempt to extract parameters that a particular option requires (if any).
 *    - if extraction is successful - it usually results in further actions being taken
 *      by the program. These actions can, for example, define or alter the behaviour
 *      of a program, usually calling specified functions. This is often implemented as a
 *      'switch/case' statement in the main() function.
 *
 *   All of the above requires from the programmer to define all the details of specific 
 *   behaviour, and even with the use of existing command-line option parsing frameworks can 
 *   often still be a boring, complicated and error-prone task.
 *
 *   In order to try to simplify the above and make it more easy to use (trying to make it more
 *   fit for the purposes like above) this framework attempts to remove from the programmer
 *   the need of defining and implementing most of these details. It also simplifies implementation
 *   of the logic and required behaviour - simply allowing automatic creation of options using
 *   functions as prototypes.
 *
 *   Example:
 *   @code
 *    // prototype of a command-line option that takes an 'int' as a parameter..
 *    void hello_few_times(int number_of_times)
 *    {
 *        for (int i = 0; i < number_of_times; i++)
 *        {
 *            std::cout << "hello ";
 *        }
 *    }
 *
 *    // another prototype of function that takes more parameters:
 *    int do_something(char letter, double param1, unsigned long param2)
 *    {
 *      // ...
 *    }
 *
 *    // another prototype of function to which we'd like to pass an object (or some sort of address)
 *    struct MyObject
 *    {
 *        std::string buf;
 *    };
 *
 *    int do_something_else(MyObject* obj_ptr, std::string new_str)
 *    {
 *       // can now do something with it.., e.g.:
 *       obj_ptr->buf =  new_str;
 *    }

 *    // and now, in your main() - creating program options goes down to doing the following:
 *    int main(int argc, char **argv)
 *    {
 *        cmd_line_parser parser;
 *        parser.set_description("This tool is used to...(whatever)....");
 *        parser.set_version("1.0.33");
 *
 *        parser.add_option(hello_few_times, "hello_few_times", "prints \"hello\" few times");
 *        parser.add_option(do_something, "-d_sth", "does something (...)");
 *
 *        MyObject my_obj;
 *        parser.add_option(do_something_else, &my_obj,  "do_something_else", "does something else (...)");
 *
 *        parser.run(argc, argv); // this line will start parsing & execute appropriate function.
 *
 *        return 0;
 *    }
 *   @endcode
 *
 *   Integer values can be specified either as decimal (by default) or as hexadecimal (starting with 0x...)
 *   - appropriate parsers will automatically work out the format.
 *   If, at any stage of parsing, a parameter can not be extracted, the framework reports this as appropriate.
 *
 *   Program can also - display a list of all available commands, along with their usage and basic information,
 *   including brief description of the program (set using cmd_line_parser::set_description), its version
 *   (cmd_line_options::set_version()) and description of what parameters are required for specified options.
 *   Refer to examples for more details.
 *_____________________________
 *
 *  Help for examples like above looks as follows:
 *
 *  c:\>my_cmd_line_tool ?
 *
 *  my_cmd_line_tool, version: 1.0.33
 *
 *  This tool is used to...(whatever)....
 *
 *  Use "?" or "help" to get more information.
 *
 *  Available options:
 *
 *    -d_sth            : does something (...)
 *                usage : my_cmd_line_tool -d_sth <char> <double> <unsigned long>
 *
 *    do_something_else : does something else (...)
 *                usage : my_cmd_line_tool do_something_else <char> <unsigned short>
 *
 *    hello_few_times   : prints "hello" few times
 *                usage : my_cmd_line_tool hello_few_times <int>
 *
 * __________________________
 *
 * An attempt to run a command with wrong values for expected parameters can result
 * is messages like:
 *
 * ./my_cmd_line_tool hello_few_times 0xas
 *
 * Error when parsing parameters, expected: <int>, got "0xas".
 *
 * Usage:
 *   my_cmd_line_tool hello_few_times <int>
 * ___________________________
 *
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

#include <map>
#include <string>
#include <stdlib.h>
#include <exception>
#include <stdio.h>
#include <stdarg.h>
#include <memory>

#include <sstream>
#include <istream>
#include <algorithm>

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
    option_error(const char* format, ...)
    {
        va_list vArgs;
        va_start(vArgs, format);
        vsnprintf(msg, MAX_MSG_SIZE, format, vArgs);
        va_end(vArgs);
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
        MAX_MSG_SIZE = 512
    };
    char msg[MAX_MSG_SIZE];
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
 * @brief Helper function to extract the whole token from the stringstream containing list of cmd-param list.
 */
inline std::string get_next_token(std::stringstream& from)
{
    std::string next_token;
    int start = from.tellg();
    if (start >= 0)
    {
        while (from.peek() == '\"')
        {
            from.get();
        }
        start = from.tellg();
        int max_len = from.str().length() - start;
        int token_len = from.str().substr(start, max_len).find('\"');
        if (token_len > 0)
        {
            std::auto_ptr<char_array> buff(new char_array(token_len + 1));
            from.getline(buff->ptr, token_len + 1, '\"');
            next_token.insert(0, buff->ptr, token_len);
        }
    }
    return next_token;
}

/**
 * @brief This is a default template for a helper class used to extract parameters.
 *        It must not be used directly (in fact it's purpose is to report compile-time
 *        errors if cmd_line_parser::add_option() is called with a function for which any
 *        of the parameters can not be extracted (see below and examples for more detail.
 */
template<class ParamType>
class param_extractor
{
public:
    /**
     * @brief Default constructor. The only reason for it in generic template implementation
     *        is to create run-time error.
     */
    param_extractor()
    {
        // default template should fail at compile time: this will allow for
        // compile-time reporting if parameters of specified functions are supported.
        STATIC_ASSERT(false,
                      EXTRACTING_PARAMETERS_OF_THIS_TYPE_IS_NOT_SUPPORTED);
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
		{ param_extractor<typeof(param)> a; (void)a;}

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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
        int sign = 1;
        std::stringstream token(get_next_token(from));

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        token >> param;

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
        int sign = 1;
        std::stringstream token(get_next_token(from));

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        token >> param;

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
        int sign = 1;
        std::stringstream token(get_next_token(from));

        if (token.peek() == '-')
        {
            sign = -1;
            token.get();
        }

        token >> param;

        if (token.fail() || !token.eof() || sign == -1)
        {
            throw option_error("%s, got \"%s\".", usage().c_str(),
                               token.str().c_str());
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
            id(-1),
            fcn_name(name)
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
    virtual void execute(std::stringstream& cmd_line_options) = 0;

    int id;
    std::string fcn_name;
    std::string usage;
    std::string descr;
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
     * @brief Execute method. Calls to the function.
     */
    virtual void execute(std::stringstream& cmd_line_options)
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
     * @brief Attempts to extract the parameter and, on success - it calls the requested function.
     * @param input stream from which next token points to the parameter that needs to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        f(p1);
    }
protected:
    option_1_param()
    {
    }
    Fcn f;
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
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        f(p1, p2);
    }
protected:
    option_2_params()
    {
    }
    Fcn f;
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
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        P3 p3 = param_extractor<P2>::extract(cmd_line_options);
        f(p1, p2, p3);
    }
protected:
    option_3_params()
    {
    }
    Fcn f;
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
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        P3 p3 = param_extractor<P3>::extract(cmd_line_options);
        P4 p4 = param_extractor<P4>::extract(cmd_line_options);

        f(p1, p2, p3, p4);
    }
protected:
    option_4_params()
    {
    }
    Fcn f;
};

template<typename Fcn, typename P1, typename P2, typename P3, typename P4,
        typename P5>
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
     * @brief Attempts to extract parameters and, on success - it calls the requested function.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        P3 p3 = param_extractor<P3>::extract(cmd_line_options);
        P4 p4 = param_extractor<P4>::extract(cmd_line_options);
        P4 p5 = param_extractor<P4>::extract(cmd_line_options);
        f(p1, p2, p3, p4, p5);
    }
protected:
    option_5_params()
    {
    }
    Fcn f;
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
    option_no_params_pass_obj(Fcn f_ptr, ObjType* object_address,
                              std::string& name) :
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
    option_1_param_pass_obj(Fcn f_ptr, ObjType* object_address,
                            std::string& name) :
            option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage();
    }

    /**
     * @brief Attempts to extract parameter and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameter.
     * @param input stream from which next tokens points to the parameter that needs to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        f(obj_addr, p1);
    }
protected:
    option_1_param_pass_obj() :
            obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
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
    option_2_params_pass_obj(Fcn f_ptr, ObjType* object_address,
                             std::string& name) :
            option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage();
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        f(obj_addr, p1, p2);
    }
protected:
    option_2_params_pass_obj() :
            obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
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
    option_3_params_pass_obj(Fcn f_ptr, ObjType* object_address,
                             std::string& name) :
            option(name), f(f_ptr), obj_addr(object_address)
    {
        usage = param_extractor<P1>::usage() + " ";
        usage += param_extractor<P2>::usage() + " ";
        usage += param_extractor<P3>::usage();
    }

    /**
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        P3 p3 = param_extractor<P3>::extract(cmd_line_options);
        f(obj_addr, p1, p2, p3);
    }
protected:
    option_3_params_pass_obj() :
            obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
};

template<typename Fcn, typename ObjType, typename P1, typename P2, typename P3,
        typename P4>
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
     * @brief Attempts to extract parameters and, on success - it calls the requested function,
     *        passing both: the address of object and extracted parameters.
     * @param input stream from which next and following tokens point to next parameters that need to be extracted.
     */
    virtual void execute(std::stringstream& cmd_line_options)
    {
        P1 p1 = param_extractor<P1>::extract(cmd_line_options);
        P2 p2 = param_extractor<P2>::extract(cmd_line_options);
        P3 p3 = param_extractor<P3>::extract(cmd_line_options);
        P4 p4 = param_extractor<P4>::extract(cmd_line_options);
        f(obj_addr, p1, p2, p3, p4);
    }
protected:
    option_4_params_pass_obj() :
            obj_addr(NULL)
    {
    }
    Fcn f;
    ObjType* obj_addr;
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
    typedef std::map<std::string, option*> Container;

    /**
     * @brief Default constructor.
     */
    cmd_line_parser() :
            version("(not set)"),
            option_cnt(0)
    {
    }

    /**
     * @brief Destructor. Cleans-up, dealocating all the options (as they were created on the heap).
     */
    ~cmd_line_parser()
    {
        Container::iterator i;
        for (i = options.begin(); i != options.end(); i++)
        {
            delete i->second;
        }
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
     * @brief Method to display help. This involves generating and printing to stdout:
     *        - name of the executable (from argv[0])
     *        - version
     *        - description
     *        - list of parameters along with the usage
     */
    void display_help()
    {
        Container::iterator i;
        std::stringstream res;

        res << "\n" << program_name;
        res << ", version: " << version << "\n\n";
        res << description << std::endl;
        res << "\nUse \"?\" or \"help\" to print more information.\n";
        res << "\nAvailable options:\n\n";

        size_t max_cmd_len = 0;
        for (i = options.begin(); i != options.end(); i++)
        {
            const std::string& s = i->first;
            max_cmd_len = std::max(max_cmd_len, s.length());
        }

        max_cmd_len += 1;

        for (i = options.begin(); i != options.end(); i++)
        {
            const std::string& s = i->first;
            option*& option = i->second;
            std::string indent;
            indent.resize(max_cmd_len - s.length(), ' ');
            res << "  " << s << indent << ": ";
            res << option->descr << "\n";
            indent.resize(max_cmd_len - 4, ' ');
            res << indent << "usage : " << program_name;
            res << " " << s << " " << option->usage << "\n\n";
        }
        std::cout << res.str();
    }

    /**
     * @brief When done creating / adding options, run this method giving proper argc/argv values
     *        To parse command-line options.
     * @param argc: number of elements in argv
     * @param argv: command-line parameters.
     * @return option_id of option that executed, or -1 if parsing was not successful and none
     *         (of the required options) executed.
     * @throws option_error if argc/argv are not valid.
     */
    int run(int argc, char **argv)
    {
        int option_id = -1;
        std::stringstream cmd_line(convert_cmd_line_to_string(argc, argv));

        // now extract the first token: it should specify our option
        std::string option = get_next_token(cmd_line);
        if (option == "?" || option == "help")
        {
            display_help();
        }
        else
        {
            try
            {
                Container::iterator i = options.find(option);
                if (option.length() && i != options.end())
                {
                    try
                    {
                        i->second->execute(cmd_line);
                        option_id = i->second->id;
                    }
                    catch (option_error& e)
                    {
                        std::stringstream s;
                        s << "\nError when parsing parameters, expected: "
                                << e.what();
                        s << "\n\n Usage: \n    " << program_name << " "
                                << option;
                        s << " " << i->second->usage;
                        throw option_error("%s\n", s.str().c_str());
                    }
                }
                else
                {
                    if (option.length() == 0)
                    {
                        throw option_error(
                                "Options required but none was specified. try \"?\" or \"help\" to see usage.\n");
                    }
                    else
                    {
                        throw option_error(
                                "\"%s\": no such option, try \"?\" or \"help\" to see usage.\n",
                                option.c_str());
                    }
                }
            } catch (option_error& err)
            {
                std::cout << err.what();
            }
        }
        return option_id;
    }

    template<class RetType>
    int add_option(RetType function_ptr(), std::string name,
                    std::string description);

    template<class RetType, typename P1>
    int add_option(RetType function_ptr(P1), std::string name,
                    std::string description);

    template<class RetType, typename P1, typename P2>
    int add_option(RetType function_ptr(P1, P2), std::string name,
                    std::string description);

    template<class RetType, typename P1, typename P2, typename P3>
    int add_option(RetType function_ptr(P1, P2, P3), std::string name,
                    std::string description);

    template<class RetType, typename P1, typename P2, typename P3, typename P4>
    int add_option(RetType function_ptr(P1, P2, P3, P4), std::string name,
                    std::string description);

    template<class RetType, typename P1, typename P2, typename P3, typename P4,
            typename P5>
    int add_option(RetType function_ptr(P1, P2, P3, P4, P5), std::string name,
                    std::string description);

    // adding options for functions taking pointer (to object) as a first parameter
    template<class RetType, typename ObjType>
    int add_option(RetType function_ptr(ObjType*), ObjType* obj_address,
                    std::string name, std::string description);

    template<class RetType, typename ObjType, typename P1>
    int add_option(RetType function_ptr(ObjType*, P1), ObjType* obj_address,
                    std::string name, std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2>
    int add_option(RetType function_ptr(ObjType*, P1, P2),
                    ObjType* obj_address, std::string name,
                    std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2,
            typename P3>
    int add_option(RetType function_ptr(ObjType*, P1, P2, P3),
                    ObjType* obj_address, std::string name,
                    std::string description);

    template<class RetType, typename ObjType, typename P1, typename P2,
            typename P3, typename P4>
    int add_option(RetType function_ptr(ObjType*, P1, P2, P3, P4),
                    ObjType* obj_address, std::string name,
                    std::string description);
protected:
    /**
     * @brief Internal method to add a raw-option.
     */
    int add_option(option* a, std::string description)
    {
        int option_id = -1;
        if (a != NULL)
        {
            a->set_description(description);

            if (options.find(a->fcn_name) != options.end())
            {
                throw option_error("%s(): option \"%s\" already exists",
                                   __FUNCTION__, a->fcn_name.c_str());
            }
            options.insert(std::make_pair(a->fcn_name, a));
            option_id = option_cnt++;
            options[a->fcn_name]->id = option_id;
        }
        else
        {
            throw option_error("%s(): option can't be NULL", __FUNCTION__);
        }
        return option_id;
    }

    /**
     * @brief Internal method to extract program name and the rest of arguments
     *        from argc/argv
     * @returns string containing parameters, delimited by space.
     * @throws option_error if argc / argv are not valid.
     */
    std::string convert_cmd_line_to_string(int argc, char **argv)
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

    Container options;
    std::string description;
    std::string program_name;
    std::string version;
    int option_cnt;
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
inline int cmd_line_parser::add_option(RetType function_ptr(),
                                        std::string name,
                                        std::string description)
{
    return add_option(new option_no_params<typeof(function_ptr)>(function_ptr, name),
               description);
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
inline int cmd_line_parser::add_option(RetType function_ptr(P1),
                                        std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    return add_option(new option_1_param<typeof(function_ptr), P1>(function_ptr, name),
               description);
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
inline int cmd_line_parser::add_option(RetType function_ptr(P1, P2),
                                        std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    return add_option(
            new option_2_params<typeof(function_ptr), P1, P2>(function_ptr,
                                                              name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3) as a prototype for the option.
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
template<class RetType, typename P1, typename P2, typename P3>
inline int cmd_line_parser::add_option(RetType function_ptr(P1, P2, P3),
                                        std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    return add_option(
            new option_3_params<typeof(function_ptr), P1, P2, P3>(function_ptr,
                                                                  name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3, P4) as a prototype for the option.
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
template<class RetType, typename P1, typename P2, typename P3, typename P4>
inline int cmd_line_parser::add_option(RetType function_ptr(P1, P2, P3, P4),
                                        std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    return add_option(
            new option_4_params<typeof(function_ptr), P1, P2, P3, P4>(
                    function_ptr, name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(P1, P2, P3, P4, P5) as a prototype for the option.
 *  Note: Defining templates for options (functions) taking more than 5 parameters would be a little bit crazy!!
 *        In fact, 5 is crazy already, but feel free to copy-paste-extend more templates if you really need more :P
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
template<class RetType, typename P1, typename P2, typename P3, typename P4,
        typename P5>
inline int cmd_line_parser::add_option(
        RetType function_ptr(P1, P2, P3, P4, P5), std::string name,
        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P5);
    return add_option(
            new option_5_params<typeof(function_ptr), P1, P2, P3, P4, P5>(
                    function_ptr, name),
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
inline int cmd_line_parser::add_option(RetType function_ptr(ObjType*),
                                        ObjType* obj_address, std::string name,
                                        std::string description)
{
    return add_option(
            new option_no_params_pass_obj<typeof(function_ptr), ObjType>(
                    function_ptr, obj_address, name),
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
inline int cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1),
                                        ObjType* obj_address, std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    return add_option(
            new option_1_param_pass_obj<typeof(function_ptr), ObjType, P1>(
                    function_ptr, obj_address, name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2) as a prototype for the option.
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
template<class RetType, typename ObjType, typename P1, typename P2>
inline int cmd_line_parser::add_option(RetType function_ptr(ObjType*, P1, P2),
                                        ObjType* obj_address, std::string name,
                                        std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    return add_option(
            new option_2_params_pass_obj<typeof(function_ptr), ObjType, P1, P2>(
                    function_ptr, obj_address, name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2, P3) as a prototype for the option.
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
template<class RetType, typename ObjType, typename P1, typename P2, typename P3>
inline int cmd_line_parser::add_option(
        RetType function_ptr(ObjType*, P1, P2, P3), ObjType* obj_address,
        std::string name, std::string description)
{
    //
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    return add_option(
            new option_3_params_pass_obj<typeof(function_ptr), ObjType, P1, P2,
                    P3>(function_ptr, obj_address, name),
            description);
}

/**
 * @brief Adds another option to your command-line parser.
 *        This template takes a function of type:
 *        func(ObjType*, P1, P2, P3, P4) as a prototype for the option.
 *  Note: Defining templates for options (functions) taking more than 5 parameters would be a little bit crazy!!
 *        In fact, 5 is crazy already, but feel free to copy-paste-extend more templates if you really need more :P
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
template<class RetType, typename ObjType, typename P1, typename P2, typename P3,
        typename P4>
inline int cmd_line_parser::add_option(
        RetType function_ptr(ObjType*, P1, P2, P3, P4), ObjType* obj_address,
        std::string name, std::string description)
{
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P1);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P2);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P3);
    STATIC_ASSERT_IF_CAN_BE_EXTRACTED(P4);
    return add_option(
            new option_4_params_pass_obj<typeof(function_ptr), ObjType, P1, P2,
                    P3, P4>(function_ptr, obj_address, name),
            description);
}

#endif /* CMD_LINE_OPTIONS_ */
