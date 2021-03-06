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


TEST_CASE("test option 2 params", "should pass")
{
    std::cout << "test option 2 params..\n";

    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option2<int, int>, "a,optiona", "option a") );
    REQUIRE_NOTHROW( parser.add_option(option2<long, double>, "d,-d", "option d") );
    REQUIRE_NOTHROW( parser.add_option(option2<float, std::string>, "e", "option e") );
    REQUIRE_NOTHROW( parser.add_option(option2<short, unsigned char>, "f,-f", "option f") );

    REQUIRE_THROWS( parser.add_option(option2<short, unsigned char>, "-f,duplicatedf", "-f is duplicated(alias exist already)") );
    REQUIRE_THROWS( parser.add_option(option2<short, unsigned char>, "d,duplicatedd", "d is duplicated(alias exist already)") );

    REQUIRE_NOTHROW( parser.add_option(option2<char, int>,
                                       "b",
                                       "@brief option b that takes 2 arguments."
                                       "@param letter some letter."
                                       "@param num a number..") );

    REQUIRE_THROWS( parser.add_option(option2<char, int>,
                                       "c",
                                       "@brief option c that takes 2 arguments."
                                       "@param letter some letter.")); // not all params have description!

    REQUIRE_NOTHROW( parser.add_option(option2<char, int>,
                                       "c", "@brief option b that takes 2 arguments.")); // but OK if no param is described.


    REQUIRE_NOTHROW( parser.add_option(option0, "g", "option g that takes no params") ); // mixing them should work too..
}

TEST_CASE("test setup require all", "should pass")
{
    std::cout << "test setup of require all..\n";

    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option0, "a", "option a that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "b", "option b that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "c", "option c that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "d", "option d that takes no params") );
    REQUIRE_THROWS( parser.add_option(option0, "a", "option a again - should throw..") );

    REQUIRE_NOTHROW( parser.setup_options_require_all("a,b,c"));
    REQUIRE_THROWS( parser.setup_options_require_all("f"));

    my_argv argv;
    argv.add_param(program_name);
    int param_id = argv.add_param("c");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE ( parser.run(argv.size(), argv.ptr()) );

    argv.update_param(param_id, "a"); // whatever else..
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE( parser.run(argv.size(), argv.ptr()) );

    argv.add_param("b");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE( parser.run(argv.size(), argv.ptr()) );

    argv.add_param("c");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE( parser.run(argv.size(), argv.ptr()) );
}


TEST_CASE("test setup require any of", "should pass")
{
    std::cout << "test setup of require all..\n";

    cmd_line_parser parser;
    REQUIRE_NOTHROW( parser.add_option(option0, "a", "option a that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "b", "option b that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "c", "option c that takes no params") );
    REQUIRE_NOTHROW( parser.add_option(option0, "d", "option d that takes no params") );
    REQUIRE_THROWS( parser.add_option(option0, "a", "option a again - should throw..") );

    REQUIRE_NOTHROW( parser.setup_options_require_any_of("a,b,c"));
    REQUIRE_THROWS( parser.setup_options_require_any_of("f"));

    my_argv argv;
    argv.add_param(program_name);
    int param_id = argv.add_param("d");

    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE_FALSE ( parser.run(argv.size(), argv.ptr()) );

    argv.update_param(param_id, "a"); // whatever else..
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE( parser.run(argv.size(), argv.ptr()) );

    argv.add_param("b");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE( parser.run(argv.size(), argv.ptr()) );

    argv.add_param("c");
    std::cout << "cmdline: " << argv << std::endl;
    REQUIRE( parser.run(argv.size(), argv.ptr()) );
}
