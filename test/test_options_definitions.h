/*
 * test_options_definitions.h
 *
 *  Created on: 20 Nov 2012
 *      Author: lukasz.forynski
 */

#ifndef TEST_OPTIONS_DEFINITIONS_H_
#define TEST_OPTIONS_DEFINITIONS_H_

#include <string>
#include <vector>
#include <string.h>
#include <iostream>


// helper class to allow easy char* const argv[] creation/modifications in tests..
class my_argv
{
public:
    my_argv()
    {
        reset();
    }

    void reset()
    {
        params.clear();

    }

    int add_param(std::string param_str)
    {
        int curr_param_id = params.size();
        if(curr_param_id < MAX_NUM_OF_PARAMS)
        {
            params.push_back(param_str);
        }
        else
        {
            throw ("my_argv::add_param() failed: too many params already added..");
        }

        return curr_param_id;
    }

    void update_param(unsigned int param_no, const std::string& new_value)
    {
        if(param_no < params.size())
        {
            params[param_no] = new_value;
        }
        else
        {
            throw ("my_argv::update_param() failed: param id out of range");
        }
    }

    char** ptr()
    {
        for(size_t i = 0; i < params.size(); i++)
        {
            argv_ptr[i] =  const_cast<char*>(params[i].c_str());
        }
        return argv_ptr;
    }

    size_t size()
    {
        return params.size();
    }

    enum constants
    {
        MAX_NUM_OF_PARAMS = 50
    };

    friend std::ostream& operator<<(std::ostream &out,  my_argv& argv);

    char* argv_ptr[MAX_NUM_OF_PARAMS];
    std::vector <std::string> params;
};

inline std::ostream& operator<<(std::ostream &out,  my_argv& argv)
{
    for(size_t i = 0; i < argv.size(); i++)
    {
        char* s = argv.ptr()[i];
        out << "\"" << s << "\" ";
        //  out << "(@" << std::hex << (long)s << "): " << "\"" << s << "\" ";
    }
    return out;
}

void option0();
void option1_uchar(unsigned char param1);
void option1_char(char param1);
void option1_schar(signed char param1);
void option1_short(short param1);
void option1_ushort(unsigned short param1);
void option1_int(int param1);
void option1_uint(unsigned int param1);
void option1_long(long param1);
void option1_ulong(unsigned long integer);
void option1_float(float param1);
void option1_double(double integer);
void option1_ldouble(long double integer);
void option1_string(std::string param1);

class state
{
public:
    state() :
                    called_ok(false)
    {
    }

    virtual ~state()
    {
    }

    bool check_result_ok()
    {
        return called_ok;
    }

    void set_ok()
    {
        called_ok = true;
    }

    bool called_ok;
};

template<typename T>
class state_of_param: public state
{
public:
    void set_value(T value)
    {
        expected = value;
    }

    T get_stored_value()
    {
        return expected;
    }

    T expected;
};

template<>
class state_of_param<void> : public state
{
public:
    void set_value(bool value)
    {
        expected = value;
    }

    bool get_stored_value()
    {
        return expected;
    }

    bool expected;
};

// I would really try to do the same = as in code -> let the compiler work out
// the type from the function itself, but it would be inappropriate to have
// a test-code that is implemented in the same way (i.e. in case this 'way' was wrong)
// ANyway - will use below status class, containing expected results..
// and status-manager will be used to access to these to set/check if values were updated
// as a result of calls to functions etc.
class option_exec_status
{
public:
    state_of_param<void> s_param_void;
    state_of_param<unsigned char> s_param_uchar;
    state_of_param<char> s_param_char;
    state_of_param<signed char> s_param_schar;
    state_of_param<short> s_param_short;
    state_of_param<unsigned short> s_param_ushort;
    state_of_param<int> s_param_int;
    state_of_param<unsigned int> s_param_uint;
    state_of_param<long> s_param_long;
    state_of_param<unsigned long> s_param_ulong;
    state_of_param<float> s_param_float;
    state_of_param<double> s_param_double;
    state_of_param<long double> s_param_ldouble;
    state_of_param<std::string> s_param_string;
    state_of_param<std::vector<std::string> > s_param_vstring;
};

class status_manager
{
public:
    enum constants
    {
        MAX_FCN_PARAMS = 5
    };

    template<typename T>
    static void store_value(int num_of_param, T value)
    {
    }

    template<typename T>
    static T get_stored_value(int num_of_param)
    {
    }

    static void store_value(int num_of_param) // special case for void..
    {
        status[num_of_param].s_param_void.set_value(true);
    }

    static option_exec_status status[MAX_FCN_PARAMS];
};

template<>
inline void status_manager::store_value<unsigned char>(int num_of_param, unsigned char value)
{
    status[num_of_param].s_param_uchar.set_value(value);
}

template<>
inline void status_manager::store_value<char>(int num_of_param, char value)
{
    status[num_of_param].s_param_char.set_value(value);
}

template<>
inline void status_manager::store_value<signed char>(int num_of_param, signed char value)
{
    status[num_of_param].s_param_schar.set_value(value);
}

template<>
inline void status_manager::store_value<short>(int num_of_param, short value)
{
    status[num_of_param].s_param_short.set_value(value);
}

template<>
inline void status_manager::store_value<unsigned short>(int num_of_param, unsigned short value)
{
    status[num_of_param].s_param_ushort.set_value(value);
}

template<>
inline void status_manager::store_value<int>(int num_of_param, int value)
{
    status[num_of_param].s_param_int.set_value(value);
}

template<>
inline void status_manager::store_value<unsigned int>(int num_of_param, unsigned int value)
{
    status[num_of_param].s_param_uint.set_value(value);
}

template<>
inline void status_manager::store_value<long>(int num_of_param, long value)
{
    status[num_of_param].s_param_long.set_value(value);
}

template<>
inline void status_manager::store_value<unsigned long>(int num_of_param, unsigned long value)
{
    status[num_of_param].s_param_ulong.set_value(value);
}

template<>
inline void status_manager::store_value<float>(int num_of_param, float value)
{
    status[num_of_param].s_param_float.set_value(value);
}

template<>
inline void status_manager::store_value<double>(int num_of_param, double value)
{
    status[num_of_param].s_param_double.set_value(value);
}

template<>
inline void status_manager::store_value<long double>(int num_of_param, long double value)
{
    status[num_of_param].s_param_ldouble.set_value(value);
}

template<>
inline void status_manager::store_value<std::string>(int num_of_param, std::string value)
{
    status[num_of_param].s_param_string.set_value(value);
}

/////--------
template<>
inline unsigned char status_manager::get_stored_value<unsigned char>(int num_of_param)
{
    return status[num_of_param].s_param_uchar.get_stored_value();
}

template<>
inline char status_manager::get_stored_value<char>(int num_of_param)
{
    return status[num_of_param].s_param_char.get_stored_value();
}

template<>
inline signed char status_manager::get_stored_value<signed char>(int num_of_param)
{
    return status[num_of_param].s_param_schar.get_stored_value();
}

template<>
inline short status_manager::get_stored_value<short>(int num_of_param)
{
    return status[num_of_param].s_param_short.get_stored_value();
}

template<>
inline unsigned short status_manager::get_stored_value<unsigned short>(int num_of_param)
{
    return status[num_of_param].s_param_ushort.get_stored_value();
}

template<>
inline int status_manager::get_stored_value<int>(int num_of_param)
{
    return status[num_of_param].s_param_int.get_stored_value();
}

template<>
inline unsigned int status_manager::get_stored_value<unsigned int>(int num_of_param)
{
    return status[num_of_param].s_param_uint.get_stored_value();
}

template<>
inline long status_manager::get_stored_value<long>(int num_of_param)
{
    return status[num_of_param].s_param_long.get_stored_value();
}

template<>
inline unsigned long status_manager::get_stored_value<unsigned long>(int num_of_param)
{
    return status[num_of_param].s_param_ulong.get_stored_value();
}

template<>
inline float status_manager::get_stored_value<float>(int num_of_param)
{
    return status[num_of_param].s_param_float.get_stored_value();
}

template<>
inline double status_manager::get_stored_value<double>(int num_of_param)
{
    return status[num_of_param].s_param_double.get_stored_value();
}

template<>
inline long double status_manager::get_stored_value<long double>(int num_of_param)
{
    return status[num_of_param].s_param_ldouble.get_stored_value();
}

template<>
inline std::string status_manager::get_stored_value<std::string>(int num_of_param)
{
    return status[num_of_param].s_param_string.get_stored_value();
}

/////--------
#endif /* TEST_OPTIONS_DEFINITIONS_H_ */
