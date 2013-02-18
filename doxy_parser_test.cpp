/**
 * doxy_parser_test.cpp
 *
 *  Created on: 15 Jan 2013
 *      Author: forma
 */


#include "cmd_line_options.h"

#include <string>

//
//void test_simple()
//{
//    std::string temp("@brief: some brief. @param device_id: ID of the device."
//                         "@param num_times: number of times it should be"
//                         "       repeated.");
//    doxy_dictionary d(temp);
//
//    std::cout << "found : " << d.get_occurences("param").size() << "params";
//
//    d.dump();
//    doxy_dictionary e("@brief something else.");
//
//    e.dump();
//    doxy_dictionary f("@brief : somafewag else");
//
//    f.dump();
//    doxy_dictionary g("@author: lukasz.forynski@gmail.com. @param : awefg else");
//
//    g.dump();
//    doxy_dictionary h("Normal description.Something something, abc\n else");
//
//    h.dump();
//}


void print_something(std::string text_to_print)
{
    std::cout << text_to_print << "\n";
}


void print_hello_few_times(int number_of_times)
{
    for(int i = 0; i < number_of_times; i++)
        {
        std::cout << "hello ";
        }
}

int test1(int argc, char **argv)
{

    cmd_line_parser p;
    p.add_option(print_something, "sth", "@brief prints a text. @param text_to_print text to be printed");
    p.add_option(print_hello_few_times, "hello", "@brief prints hello number of times. @param number_of_times - specifies how many times print it.");

    p.run(argc, argv);


    return 0;
}




#define PRINT_FCN_NAME  printf("%s \n", __PRETTY_FUNCTION__)

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
    parser.set_description("@brief: This is an example of how to use cmd_line_options library\n"
                           "@author: Lukasz Forynski (lukasz.forynski@gmail.com)");



    std::string abc("brief: This is an    example          of how to use cmd_line_options library. One Line ends here\n"
                           "@author:     Lukasz Forynski (lukasz.forynski@gmail.com). There's more in this \n line ..abc");

    std::cout << "before : " << abc << "\n--\n";
    format_to_max_line_length(abc, 40);
    std::cout << "after  : " << abc << "\n--\n";


    append_to_lines(abc, "<start>", "<end>" );
    std::cout << "after2  : " << abc << "\n--\n";



    parser.add_group("Printing options");
    parser.add_option(hello_few_times, "-x", "prints \"hello\" a specified number of times");

    MyObject my_obj;
    parser.add_option(update_my_object, &my_obj, "-u", "Updates something (..)");


    parser.add_option(say, "-say", "@brief: will just print what you typed."
                                   "@param what1:  first thing to say"
                                   "@param what2 second thing to say");


    parser.add_group("Other options");
//    parser.add_option(do_something, "-dverylongoption", "does something (...)");

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





