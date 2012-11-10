cmd_line_options
================

   https://github.com/formiaczek/cmd_line_options/wiki/command-line-options

   Command line options template library is meant to provide an easy way
   of adding command-line options to your program. Defining and adding these
   options is very generic and intuitive way and requires minimal programming effort.
   All that is required is to express requirements as functions, and the framework
   will automatically match appropriate templates to implement all the logic.

   In most cases programs, where command-line options are used, usually need to:
    - define these command-line options,
    - parse command line arguments to check if (and which) of these options was specified
    - attempt to extract parameters that a particular option requires (if any).
    - if extraction is successful - it usually results in further actions being taken
       by the program. These actions can, for example, define or alter the behaviour
       of a program, usually calling specified functions. This is often implemented as a
      'switch/case' statement in the main() function.

   All of the above often requires the programmer to define all details of specific behaviour
   and even with the use of existing command-line option parsing frameworks it might often
   still be boring, complicated and error-prone task.
 
   In order to try to simplify the above and make it more easy to use (trying to make it more
   fit for the purposes like above) this framework attempts to remove from the programmer
   the burden of defining and implementing all these details. It also simplifies implementation
   of logic and required behaviour by simply allowing automatic creation of options using functions
   as prototypes.


Example:
{{{

    // prototype of a command-line option that takes an 'int' as a parameter
    void hello_few_times(int number_of_times)
    {
        for (int i = 0; i < number_of_times; i++)
        {
            std::cout << "hello ";
        }
    }

    // another prototype of function that takes more parameters:
    int do_something(char letter, double param1, unsigned long param2)
    {
      // ...
    }

    // another prototype of function to which we'd like to pass an object (or some sort of address)
    struct MyObject
    {
        char* buf;
    };

    int do_something_else(MyObject* obj_ptr, char letter, unsigned short param1)
    {
       obj_ptr->buf[0] (...) // do something with it..
       // ...
    }

    // and now, in your main() - creating program options goes down to doing the following:
    int main(int argc, char **argv)
    {
        cmd_line_parser parser;
        parser.set_description("This tool is used to...(whatever)....");
        parser.set_version("1.0.33");

        parser.add_option(hello_few_times, "hello_few_times", "prints \"hello\" few times");
        parser.add_option(do_something, "-d_sth", "does something (...)");

        MyObject my_obj;
        parser.add_option(do_something_else, &my_obj,  "do_something_else", "does something else (...)");

        parser.run(argc, argv); // this line will start parsing & execute appropriate function.

        return 0;
    }
     
}}}
   integer values can be specified either as decimal (by default) or as hexadecimal (starting with 0x...)
   - appropriate parsers will automatically work out the format.
   If, at any stage of parsing, a parameter can not be extracted, the framework reports this as appropriate.

   Program can also - display a list of all available commands, along with their usage and basic information, including brief description of the program (set using cmd_line_parser::set_description), its version (cmd_line_options::set_version()) and description of what parameters are required for specified options. Help for examples like above looks as follows:

{{{

c:\>my_cmd_line_tool ?

 my_cmd_line_tool, version: 1.0.33

 This tool is used to...(whatever)....

 Use "?" or "help" to get more information.

 Available options:

    -d_sth            : does something (...)
                usage : my_cmd_line_tool -d_sth <char> <double> <unsigned long>

    do_something_else : does something else (...)
                usage : my_cmd_line_tool do_something_else <char> <unsigned short>

    hello_few_times   : prints "hello" few times
                usage : my_cmd_line_tool hello_few_times <int>
}}}

An attempt to run a command with wrong values for expected parameters can result is messages like:
{{{
./my_cmd_line_tool hello_few_times 0xas

  Error when parsing parameters, expected: <int>, got "0xas".

  Usage:
     my_cmd_line_tool hello_few_times <int>
}}}

Also note, that if the function is not suitable to be used as a command-line option (i.e. contains parameters that can't be parsed/extracted from the command-line, e.g. pointers etc) - attempt to use such a function will result in a compile time error like below. Note also, that const char* and std::string are going to be added sometime soon)
{{{
// if one of the functions from above example is modified as follows:
29: int do_something(char letter, double* param1, unsigned long param2)
   {
   }


56: int main(int argc, char **argv)
    {
       (...)
66:    parser.add_option(do_something, "-d_sth", "does something (...)");
}}}
The example will fail to compile with the error message containing the following:

{{{
g++ example.cpp 
In file included from example.cpp:13:
cmd_line_options.h: In constructor ‘param_extractor<ParamType>::param_extractor() [with ParamType = double*]’:
cmd_line_options.h:1658:   instantiated from ‘void cmd_line_parser::add_option(RetType (*)(P1, P2, P3), std::string, std::string) [with RetType = int, P1 = char, P2 = double*, P3 = long unsigned int]’
example.cpp:66:   instantiated from here
cmd_line_options.h:302: error: ‘ct_assert_in_line_302__extracting_parameters_of_this_type_is_not_supported’ has incomplete type
}}}

Which should be just as enough to figure out what went wrong :) 
(i.e. that extracting parameters for double* is not supported, and this came from calling add_option in example.cpp, line 66.



