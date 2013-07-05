/*
 * example0.cpp
 *
 *  Created on: 2012-11-18
 *  Author: lukasz.forynski@gmail.com
 *
 *  @brief This is the simplest example of using a program taking only parameters
 *         (i.e. no option specifier like -xx etc.).
 *
 *         Such a program has one, default option, for which parameters are also
 *         defined by defining the function/handler.
 */


#include <iostream>
#include <string>
#include <stdio.h>

#include "cmd_line_options.h"




#define PRINT_FCN_NAME 	printf("%s \n", __PRETTY_FUNCTION__)

/**
 * @brief prototype for a command-line option that takes an 'int' as a parameter..
 */
void handle_program_options(std::string what, int how_many_times)
{
    for (int i = 0; i < how_many_times; i++)
    {
        std::cout << what << " ";
    }
    std::cout << std::endl;
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

    // leave option name empty = and it will become a 'default' option
    parser.add_option(handle_program_options, "", "@brief Prints a string number of times. @param what string to print."
                                                  "                        @param num_times number of times it should be printed.");
    // Note, that when such an option is defined - no other option can be added / defined (add_option would throw option-error in such case)
    // Note2: You still can can also add handler for all other cmd-line params in such case(see example3.cpp).

    // this starts the parser..
    // All cmd-line parameters will be executed, and if options are recognised/params for them
    // properly parsed - appropriate functions will be called in the order they were found during
    // parsing. By default - if options is specified multiple times in cmd-line parameters - function handler
    // will execute multiple times, each time with parameters that are parsed from cmd-line. This can be
    // restricted - future releases and other examples will describe this.
    if(parser.run(argc, argv))
    {
        // rest of your program..
    }
    return 0;
}

/* Example output(s):

$ ./example0  ?

example0, version: 0.0.1

This is an example of how to use cmd_line_options library
Author: Lukasz Forynski (lukasz.forynski@gmail.com)

Use "?", "-h" or "--help" to print more information.

 Example (optional) description for option
     usage : example0 <string> <int>
 */

