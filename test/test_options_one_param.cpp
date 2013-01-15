/*
 * test_default_option.cpp
 *
 *  Created on: 19 Nov 2012
 *      Author: lukasz.forynski
 */

#include "test_generic.h"

#include <string.h>
#include <cmd_line_options.h>
#include <sstream>
#include <iostream>

#include "test_options_definitions.h"

static const char* program_name = "some/path/program/name";

TEST_CASE("test argv stuff", "should work as expected..")
{
    std::cout << "test argv stuff.\n";

    my_argv argv;
    argv.add_param(program_name);
    argv.add_param("int");

    int value_id = argv.add_param("13");
    std::cout << "1: cmdline: " << argv << std::endl;

    argv.update_param(value_id, "-215");
    std::cout << "2: cmdline: " << argv << std::endl;
}



TEST_CASE("version and description", "should not throw")
{
    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.set_description("description_abc") );
    REQUIRE_NOTHROW( parser.set_version("1.2.43") );
}

TEST_CASE("adding options stuff..", "should not throw")
{
    std::cout << "adding options stuff...\n";
    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option0, "no_params", "option that takes no params") );
    REQUIRE_THROWS( parser.add_option(option0, "no_params", "option that takes no params") ); // already exists..

    // can't add default option if other options exist already..
    REQUIRE_THROWS( parser.add_option(option0, "", "option that takes no params") );

    // similarly for adding in reversed order..
    cmd_line_parser parser2;
    REQUIRE_NOTHROW( parser2.add_option(option0, "", "option that takes no params") );
    REQUIRE_THROWS( parser2.add_option(option0, "no_params", "option that takes no params") );
}

TEST_CASE("test option 0 params..", "should not throw")
{
    std::cout << "test option 0 params..\n";

    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option0, "opt_no_params", "option that takes no params") );

    my_argv argv;
    argv.add_param(program_name);
    argv.add_param("opt_no_params");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE ( parser.run(argv.size(), argv.ptr()) );
    std::cout << "cmdline(after run): " << argv << std::endl;

    argv.update_param(1, "wefw"); // whatever else..
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE( parser.run(argv.size(), argv.ptr()) );
}

TEST_CASE("test option 1 param: int..", "..")
{
    std::cout << "test option 1 param: int..\n";

    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option1_int, "int", "that takes int") );

    my_argv argv;
    argv.add_param(program_name);
    argv.add_param("int");
    int value_id = argv.add_param("13");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<int>(1) == 13);

    argv.update_param(value_id, "-215");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<int>(1) == -215);

    argv.update_param(value_id, "0x1aB4");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<int>(1) == 0x1aB4);

    argv.update_param(value_id, "3afD");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<int>(1) == 0x3afD);

    // some invalid values..
    argv.update_param(value_id, "0xfat");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "abfawef");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));
}


TEST_CASE("test option 1 param: char..", "..")
{
    std::cout << "test option 1 param: char..\n";
    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option1_char, "char", "that takes a character") );

    my_argv argv;
    argv.add_param(program_name);
    argv.add_param("char");
    int value_id = argv.add_param("x");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<char>(1) == 'x');

    argv.update_param(value_id, "A");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<char>(1) == 'A');

    argv.update_param(value_id, "5");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<char>(1) == '5');

    // char param means only single character, so more than one is wrong..
    argv.update_param(value_id, "ab");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "ab da");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));
}


TEST_CASE("test option 1 param: double..", "..")
{
    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option1_double, "double", "that takes double") );

    my_argv argv;
    argv.add_param(program_name);
    argv.add_param("double");
    int value_id = argv.add_param("2.32");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<double>(1) == 2.32);

    argv.update_param(value_id, "11221");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<double>(1) == 11221);

    argv.update_param(value_id, "-3.1415");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<double>(1) == -3.1415);

    argv.update_param(value_id, "-133323");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE (parser.run(argv.size(), argv.ptr()));
    REQUIRE (status_manager::get_stored_value<double>(1) == -133323);

    // bad values..
    argv.update_param(value_id, "a13d");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "-13.abd");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "a13w");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "--1.2");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "1.2.3");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "1.2.3.4");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));

    argv.update_param(value_id, "-1.2.3.4");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE (parser.run(argv.size(), argv.ptr()));
}
