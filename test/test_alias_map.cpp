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

TEST_CASE("version and description", "should not throw")
{
    REQUIRE_NOTHROW( test_alias_map() );
}

