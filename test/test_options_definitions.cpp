/*
 * test_options_definitions.cpp
 *
 *  Created on: 20 Nov 2012
 *      Author: lukasz.forynski
 */

#include "test_options_definitions.h"

//state_option0
//state_option1_uchar
//state_option1_char
//state_option1_schar
//state_option1_short
//state_option1_ushort
//state_option1_int
//state_option1_uint
//state_option1_long
//state_option1_ulong
//state_option1_float
//state_option1_double
//state_option1_ldouble


option_exec_status status_manager::status[status_manager::MAX_FCN_PARAMS];


void option0()
{
    status_manager::store_value(0);
}

void option1_uchar(unsigned char param1)
{
    status_manager::store_value(1, param1);
}

void option1_char(char param1)
{
    status_manager::store_value(1, param1);
}

void option1_schar(signed char param1)
{
    status_manager::store_value(1, param1);
}

void option1_short(short param1)
{
    status_manager::store_value(1, param1);
}

void option1_ushort(unsigned short param1)
{
    status_manager::store_value(1, param1);
}

void option1_int(int param1)
{
    status_manager::store_value(1, param1);
}

void option1_uint(unsigned int param1)
{
    status_manager::store_value(1, param1);
}

void option1_long(long param1)
{
    status_manager::store_value(1, param1);
}

void option1_ulong(unsigned long param1)
{
    status_manager::store_value(1, param1);
}

void option1_float(float param1)
{
    status_manager::store_value(1, param1);
}

void option1_double(double param1)
{
    status_manager::store_value(1, param1);
}

void option1_ldouble(long double param1)
{
    status_manager::store_value(1, param1);
}

void option1_string(std::string param1)
{
    status_manager::store_value(1, param1);
}


