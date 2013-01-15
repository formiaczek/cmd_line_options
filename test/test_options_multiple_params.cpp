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
