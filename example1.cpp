/*
 * example1.cpp
 *
 *  Created on: 2012-10-09
 *  Author: lukasz.forynski@gmail.com
 *
 *  @brief This is basic example on how to use this library to create multiple options.
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <typeinfo>
#include "cmd_line_options.h"

#define PRINT_FCN_NAME 	printf("%s \n", __PRETTY_FUNCTION__)

/**
 * @brief prototype for a command-line option that takes an 'int' as a parameter..
 */
void hello_few_times(int number_of_times)
{
    for (int i = 0; i < number_of_times; i++)
    {
        std::cout << "hello ";
    }
    std::cout << std::endl;
}

/**
 * @brief another prototype for option that requires more parameters
 */
void do_something(char letter, double param1, unsigned long param)
{
    PRINT_FCN_NAME;
}

struct MyObject
{
    std::string str;
};

/**
 * @brief another prototype for option in which we'd like to alter the state of an object
 * (or other memory location)
 */
int update_my_object(MyObject* obj_ptr, std::string new_str)
{
    obj_ptr->str = new_str;
    return 1; // note, that functions can various return type. At the moment it is not used by the framework.
}

/**
 * @brief prototype for a simple option with no parameters..
 */
void print_hello_world()
{
    std::cout << "hello world!\n";
}

/**
 * @brief prototype of a command-line options that takes two strings..
 */
void say(std::string what, std::string what2)
{
    std::cout << "\nfirst  :(" << what << ")\n";
    std::cout << "second :(" << what2 << ")\n";
}

/**
 * @brief main...
 */
int main(int argc, char **argv)
{
    cmd_line_parser parser;
    parser.set_version("0.0.1");
    parser.set_description("This is an example of how to use cmd_line_options library\n"
                           "Author: Lukasz Forynski (lukasz.forynski@gmail.com)");


    parser.add_group("Printing options");
    parser.add_option(hello_few_times, "-x,hello_few_times", "prints \"hello\" a specified number of times");

    MyObject my_obj;
    parser.add_option(update_my_object, &my_obj, "update_my_object", "Updates something (..)");


    // If someone is really lazy - he can also add an option using SPLIT_TO_NAME_AND_STR
    // macro. This will cause the function name to be used as a name for the option, e.g.:
    parser.add_option(SPLIT_TO_NAME_AND_STR(print_hello_world), "prints \"hello world\"");

    parser.add_option(say, "say", "will just print what you typed");

    parser.add_group("Other options");
    parser.add_option(do_something, "-d,d_sth", "does something (...)");

    parser.setup_options_require_all("-d, -x");

    // this starts the parser..
    // All cmd-line parameters will be executed, and if options are recognised/params for them
    // properly parsed - appropriate functions will be called in the order they were found during
    // parsing. By default - if options is specified multiple times in cmd-line parameters - function handler
    // will execute multiple times, each time with parameters that are parsed from cmd-line. This can be
    // restricted - future releases and other examples will describe this.
    if(parser.run(argc, argv))
    {
        // rest of your program..
        if (parser.check_if_option_specified("update_my_object"))
        {
            std::cout << " my updated object (my_obj.str): \'" << my_obj.str << "\'\n";
        }
    }
    return 0;
}

/* Example output(s):
 ____________________________________

 ~$ example1 ?

example1, version: 0.0.1

 This is an example of how to use cmd_line_options library
 Author: Lukasz Forynski (lukasz.forynski@gmail.com)


Printing options:

 -x,hello_few_times: prints "hello" a specified number of times
              usage: -x <int>


   update_my_object: Updates something (..)
              usage: update_my_object <string>


  print_hello_world: prints "hello world"
              usage: print_hello_world


                say: will just print what you typed
              usage: say <string> <string>


Other options:

           -d,d_sth: does something (...)
              usage: -d <char> <double> <unsigned long>
 ____________________________________

 $ ./example  d_sth wfa

 Error when parsing parameters, expected: <char>, got "wfa".

 Usage:
 example -d_sth <char> <double> <unsigned long>

 ____________________________________

 $ ./example  update_my_object new_name
 my updated object (my_obj.str): 'new_name'
 */

