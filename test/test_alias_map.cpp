/*
 * test_alias_map.cpp
 *
 *  Created on: 32 Oct 2013
 *      Author: lukasz.forynski
 */

#include "test_generic.h"

#include <alias_map.h>
#include <iostream>
#include <sstream>


void test_alias_map()
{
    try
    {
    alias_map<std::string, std::string> m;

    REQUIRE(m.size() == 0);

    m.add_object("first", "the first!");
    REQUIRE(m.size() == 1);

    m.add_alias("first", "1");
    REQUIRE(m.size() == 1); // adding alias doesn't change size..

    REQUIRE(m["first"] == "the first!");
    REQUIRE(m["1"] == "the first!");

    REQUIRE_THROWS(m.add_alias("first", "1")); // try adding alias again

    alias_map<std::string, std::string>::iterator i;

    i = m.find("1");
    REQUIRE(i != m.end());
    REQUIRE(*i == "the first!");

    alias_map<std::string, std::string>::iterator j;
    j = m.find("first");
    REQUIRE(j != m.end());
    REQUIRE(j == j);
    REQUIRE(*j == "the first!");

    m.remove_alias("1");
    i = m.find("1");
    REQUIRE(i == m.end());

    REQUIRE_THROWS( m["1"] += "abc" );
    REQUIRE_THROWS( m.remove_alias("1") ); // can't remove alias: it doesn't exist

    m["first"] = "something else"; // another alias still works.
    m.remove_alias("first");
    REQUIRE_THROWS(m["first"] = "doesnt exist!"); // but now it doesn't
    REQUIRE(m.size() == 0);

    m.add_object("second", "222");
    m.add_alias("second", "2");
    REQUIRE(m.size() == 1);

    REQUIRE(m["second"] == "222");
    REQUIRE(m["2"] == "222");

    REQUIRE_THROWS(m.remove_object("23"));

    m.remove_object("2");
    REQUIRE_THROWS( m["second"] += "abc" );
    REQUIRE(m.size() == 0);


    m.add_object("first", "the first!");
    m.add_alias("first", "one");
    m.add_alias("first", "1");

    m.add_object("second", "the Second!");
    m.add_alias("second", "2");
    m.add_alias("2", "two");

    m.add_object("third", "the third!");
    m.add_alias("third", "3");
    m.add_alias("3", "the3");

    alias_map<std::string, std::string>::aliases_iterator a;
    for (i = m.begin(); i != m.end(); i++)
    {
        for (a = i.aliases().begin(); a != i.aliases().end(); a++)
        {
            std::cout << *a << " ";
        }

        std::string& k = *i;
        std::cout <<" => " << k << "\n";
    }

    size_t size = m.size();
    i = m.find("3");
    m.erase(i);
    REQUIRE_THROWS( m["3"] += "abc" );
    REQUIRE(size > m.size());

    m.clear();
    REQUIRE(m.size() == 0);

    std::cout << "all tests passed OK!";
    }
    catch(std::runtime_error& e)
    {
        std::stringstream err;
        err << "testing failed: " << e.what();
        throw std::runtime_error(err.str());
    }
}

TEST_CASE("version and description", "should not throw")
{
    REQUIRE_NOTHROW( test_alias_map() );
}

