/*
 * example.cpp
 *
 *  Created on: 2012-10-09
 *  Author: lukasz.forynski@gmail.com
 *
 */

#include <iostream>

#include <stdio.h>
#include <typeinfo>
#include "cmd_line_options.h"

#define PRINT_FCN_NAME 	printf("%s \n", __PRETTY_FUNCTION__)


// prototype of a command-line options that takes an 'int' as a parameter..
void hello_few_times(int number_of_times)
{
    for (int i = 0; i < number_of_times; i++)
    {
        std::cout << "hello ";
    }
    std::cout << std::endl;
}

// another prototype of functions that takes more parameters:
int do_something(char letter, double param1, unsigned long param2)
{
    PRINT_FCN_NAME;
    // ...
    return 0;
}

// another prototype of functions to which we'd like to pass an object (or some sort of address) to use
struct MyObject
{
    int val;
};

int update_my_object(MyObject* obj_ptr, int new_value)
{
    printf("old: obj_ptr->val: %d\n", obj_ptr->val);
    printf("updating it to %d\n", new_value);
    obj_ptr->val = new_value;
    return 1; // note, that fuctions can various return type. At the moment it is not used by the framework.
}

void print_hello_world()
{
    std::cout << "hello world!\n";
}


int main(int argc, char **argv)
{
    cmd_line_parser parser;
    parser.set_version("0.0.1");

    parser.set_description("This is an example of how to use cmd_line_options library\n"
            "Author: Lukasz Forynski (lukasz.forynski@gmail.com)");

    parser.add_option(hello_few_times, "hello_few_times", "prints \"hello\" a specified number of times");

    parser.add_option(do_something, "-d_sth", "does something (...)");

    MyObject my_obj;
    parser.add_option(update_my_object, &my_obj,  "update_my_object", "Updates something (..)");

    // If someone is really lazy - he can also add an option using SPLIT_TO_NAME_AND_STR
    // macro. This will cause the function name to be used as a name for the option, e.g.:
    parser.add_option(SPLIT_TO_NAME_AND_STR(print_hello_world), "prints \"hello world\"");

    parser.run(argc, argv); // this starts the parser..

    printf("my_obj before exit: my_obj.val: %d\n", my_obj.val);

    return 0;
}

/* Example output:
____________________________________

~$ ./example  ?

example, version: 0.0.1

This is an example of how to use cmd_line_options library
Author: Lukasz Forynski (lukasz.forynski@gmail.com)

Use "?" or "help" to print more information.

Available options:

  -d_sth            : does something (...)
              usage : example -d_sth <char> <double> <unsigned long>

  hello_few_times   : prints "hello" a specified number of times
              usage : example hello_few_times <int>

  print_hello_world : prints "hello world"
              usage : example print_hello_world

  update_my_object  : Updates something (..)
              usage : example update_my_object <int>
____________________________________

$ ./example  -d_sth wfa

Error when parsing parameters, expected: <char>, got "wfa".

 Usage:
    example -d_sth <char> <double> <unsigned long>

____________________________________

$ ./example  update_my_object 43
old: obj_ptr->val: 0
updating it to 43
my_obj before exit: my_obj.val: 43

 */
