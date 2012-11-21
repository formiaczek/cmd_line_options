/**
 * type_to_int.h
 *
 *  Created on: 21 Nov 2012
 *      Author: forma
 */

#ifndef TYPE_TO_INT_H_
#define TYPE_TO_INT_H_


template<class T>
class Type2Int
{
    enum v
    {
        value = -1
    };
};

// egh.. ugly, but there's no nice way of doing it..

template<>
class Type2Int<unsigned char>
{
    enum v
    {
        value = 0
    };
};

class Type2Int<char>
{
    enum v
    {
        value = Type2Int<unsigned char>::value + 1
    };
};

class Type2Int<signed char>
{
    enum v
    {
        value = Type2Int<char>::value + 1
    };
};

class Type2Int<short>
{
    enum v
    {
        value = Type2Int<signed char>::value + 1
    };
};

class Type2Int<unsigned short>
{
    enum v
    {
        value = Type2Int<short>::value + 1
    };
};

class Type2Int<int>
{
    enum v
    {
        value = Type2Int<unsigned short>::value + 1
    };
};

class Type2Int<unsigned int>
{
    enum v
    {
        value = Type2Int<int>::value + 1
    };
};

class Type2Int<long>
{
    enum v
    {
        value = Type2Int<unsigned int>::value + 1
    };
};

class Type2Int<unsigned long>
{
    enum v
    {
        value = Type2Int<long>::value + 1
    };
};

class Type2Int<float>
{
    enum v
    {
        value = Type2Int<unsigned long>::value + 1
    };
};

class Type2Int<double>
{
    enum v
    {
        value = Type2Int<float>::value + 1
    };
};

class Type2Int<long double>
{
    enum v
    {
        value = Type2Int<double>::value + 1
    };
};

class Type2Int<std::string>
{
    enum v
    {
        value = Type2Int<unsigned double>::value + 1
    };
};


#endif /* TYPE_TO_INT_H_ */
