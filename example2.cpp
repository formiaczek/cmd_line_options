/*
 * example2.cpp
 *
 *  Created on: 2012-11-15
 *  Author: lukasz.forynski@gmail.com
 *
 *  @brief This is an example of using command-line options that have dependencies.
 */


#include <iostream>
#include <sstream>

#include "cmd_line_options.h"

#define PRINT_FCN_NAME 	std::cout << __PRETTY_FUNCTION__ << "\n"

void aa()
{
    PRINT_FCN_NAME;
}

void bb()
{
    PRINT_FCN_NAME;
}

void a_b()
{
    PRINT_FCN_NAME;
}

void a_only()
{
    PRINT_FCN_NAME;
}

void b_only()
{
    PRINT_FCN_NAME;
}

void standalone()
{
    PRINT_FCN_NAME;
}

int main(int argc, char **argv)
{

    cmd_line_parser parser;
    parser.set_version("0.0.1");

    std::stringstream desc;
    desc << "This is an example of how to use cmd_line_options library\n";
    desc << "to specify options with dependencies. The framework will provide all\n";
    desc << "the logic required to check if a combination of selected options is valid.\n";
    desc << "Author: Lukasz Forynski (lukasz.forynski@gmail.com)";
    parser.set_description(desc.str());

    // can specify multiple aliases for option: separate them with either of: ",/| ".
    parser.add_option(aa, "-a,option_a", "simple option - no specific requirements");

    parser.add_option(bb, "bb", "another option - no specific requirements");
    parser.add_option(a_b, "a_b", "option that requires specifying another two..");
    parser.add_option(a_only, "a_only", "only use with a specific sub-set of options.");
    parser.add_option(b_only, "b_only", "also with only specific sub-set of options.");
    parser.add_option(standalone, "standalone", "If specified-it should be the only option.");

    // now setup dependencies..
    // setup required options - in form of a comma/colon/semicolon or space separated list.
    // All options used - should be valid (i.e. should have been added already)
    parser.setup_option_add_required("a_b", "-a, bb");
    parser.setup_option_add_not_wanted("a_only", "bb, a_b, standalone");
    parser.setup_option_add_not_wanted("b_only", "option_a, a_b, standalone");

    // this adds option as a standalone (i.e. it must not be specified with any other option)
    parser.setup_option_as_standalone("standalone");

    parser.run(argc, argv); // now start parsing..

    return 0;
}

/* Example outputs:
 ____________________________________

 ~$ ./example2 ?

example2, version: 0.0.1

 This is an example of how to use cmd_line_options library
 to specify options with dependencies. The framework will provide all
 the logic required to check if a combination of selected options is valid.
 Author: Lukasz Forynski (lukasz.forynski@gmail.com)


Options:

 -a,option_a: simple option - no specific requirements
       usage: -a


          bb: another option - no specific requirements
       usage: bb


         a_b: option that requires specifying another two..
       usage: a_b


      a_only: only use with a specific sub-set of options.
       usage: a_only


      b_only: also with only specific sub-set of options.
       usage: b_only


  standalone: If specified-it should be the only option.
       usage: standalone

 ____________________________________

 ~$ ./example2 a_b
 error: option "a_b" requires also: "aa", "bb"
 ____________________________________

 $ ./example2 a_b bb aa
 void a_b()
 void bb()
 void aa()
_____________________

 $ ./example2 b_only aa
 error: option "b_only" can't be used with: "aa"
 ____________________________________

 $ ./example2 b_only aa a_b
 error: option "b_only" can't be used with: "a_b", "aa"
 ____________________________________

 $ ./example2 b_only bb
 void b_only()
 void bb()
 ____________________________________

 $ ./example2 standalone aa a_b bb
 error: option "standalone" can't be used with other options, but specified with: "a_b", "aa", "bb"
*/
