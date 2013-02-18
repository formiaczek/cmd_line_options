/*
 * example3.cpp
 *
 *  Created on: 2012-11-17
 *  Author: lukasz.forynski@gmail.com
 *  @brief This example shows how to use other cmd-line arguments
 *         (i.e. arguments that were neither options, nor params they require).
 *         It also shows howo to use optional / default values for option parameters.
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <typeinfo>
#include "cmd_line_options.h"

#define PRINT_FCN_NAME  printf("%s \n", __PRETTY_FUNCTION__)

/**
 * @brief prototype for a command-line option that takes an 'int' as a parameter..
 */
void hello_few_times(int number_of_times, int not_used)
{
    for (int i = 0; i < number_of_times; i++)
    {
        std::cout << "hello ";
    }
    std::cout << std::endl;
}

/**
 * @brief another prototype for option with optional value (works exactly as implicit/default
 *        function argument). Behaviour is as expected: if the param is given in cmd line: value will
 *        be extracted. Otherwise  = a default value is used. See OPTIONAL_VALUE description/documentation
 *        for more details
 */
void default_param(OPTIONAL_VALUE(int, param2, 14))
{
    PRINT_FCN_NAME;
    std::cout << param2.get_value() << std::endl;

    std::string s;
}

void print_hello_world()
{
    std::cout << "hello world!\n";
}

/**
 * @brief if this handler is added with cmd_line_parser::add_handler_for_other_arguments
 *        all 'unrecognised' (i.e. not related to any registered options) arguments will be passed
 *        to this handler. It will be executed after all option-handlers have executed.
 */
void other_cmd_line_arguments(std::vector<std::string>& other_args)
{
    std::cout << "other arguments were:\n";

    for(std::vector<std::string>::iterator arg = other_args.begin(); arg != other_args.end();)
    {
        std::cout << "\"" << *arg << "\"";
        arg++;
        if (arg != other_args.end())
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;
}


/**
 * @brief main
 */
int main(int argc, char **argv)
{
    std::stringstream desc;

    cmd_line_parser parser;
    parser.set_version("0.0.1");
    desc << "This is an example of how to use cmd_line_options library\n";
    desc << "with the mix of defined-options and other arguments. All arguments\n";
    desc << "that are not recognised as option / params for defined options\n";
    desc << "will be passed to a registered handler after all option-handlers have\n";
    desc << "executed\n";
    desc << "Author: Lukasz Forynski (lukasz.forynski@gmail.com)";
    parser.set_description(desc.str());

    parser.add_option(hello_few_times,
                      "hello_few_times",
                      " prints \"hello\" a specified number of times. Nothing particular,"
                      "but it is only tho show how to use this framework."
                      "@param num_times number of times \"hello\" should be printed"
                      "to the std output. Note, that if you specify a big number"
                      "it might take up all of your screen. @param other_something this param is not used");

    parser.add_option(print_hello_world, "hello", "prints \"hello world\"");
    parser.add_option(default_param, "--dp", "optional parameter..");

    parser.add_handler_for_other_arguments(other_cmd_line_arguments);


    parser.setup_options_require_any_of("hello_few_times, hello");


    parser.run(argc, argv); // this starts the parser..

    // rest of your program..

    return 0;
}

/* Example output:
$ ./example3 ?

example3, version: 0.0.1

This is an example of how to use cmd_line_options library
with the mix of defined-options and other arguments. All arguments
that are not recognised as option / params for defined options
will be passed to a registered handler after all option-handlers have
executed
Author: Lukasz Forynski (lukasz.forynski@gmail.com)

Use "?", "-h" or "--help" to print more information.

Options:
  hello_few_times : prints "hello" a specified number of times
            usage : hello_few_times <int>

  hello           : prints "hello world"
            usage : hello

  --dp            : optional parameter..
            usage : --dp <int>(optional=14)

 ____________________________________

 */

