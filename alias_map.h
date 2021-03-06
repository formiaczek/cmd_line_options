/**
 * @file   alias_map.h
 * @date   26 Aug 2013
 * @author lukasz.forynski@gmail.com
 * @brief  Map aggregate allowing to add multiple keys (aliases) for map items.
 *
 * ___________________________
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 Lukasz Forynski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION
 */

#ifndef ALIAS_MAP_H_
#define ALIAS_MAP_H_


#include <map>
#include <list>
#include <sstream>
#include <stdexcept>

/**
 * @brief map allowing to create aliases (multiple keys of the same type) that could be
 *        used to access items this map holds.
 */
template<typename KeyType, typename ObjType>
class alias_map
{
    typedef std::list<KeyType> aliases_container;
    typedef std::pair<ObjType, aliases_container> obj_wrapper;
    typedef std::list<obj_wrapper> obj_container;
    typedef std::map<KeyType, obj_wrapper*> obj_mapping;


public:
    class iterator_wrapper: public obj_container::iterator
    {
    public:
        typedef typename obj_container::iterator parent_type;

        /**
         * @brief Default constructor.
         */
        iterator_wrapper()
        {
        }

        /**
         * @brief "Conversion" constructor..
         */
        iterator_wrapper(const parent_type& m) :
             parent_type(m)
        {
        }

        ObjType& operator*() const
        {
            return parent_type::operator*().first;
        }

        ObjType* operator->() const
        {
            return &parent_type::iterator::operator->().first;
        }

        /**
         * @brief Returns reference to the key of the element pointed by this iterator
         */
        const aliases_container& aliases() const
        {
            return parent_type::operator*().second;
        }
    };

    typedef iterator_wrapper iterator;
    typedef iterator_wrapper const_iterator;
    typedef typename aliases_container::const_iterator aliases_iterator;

    /**
     * @brief Adds/creates a new element into the map.
     */
    void add_object(const KeyType& key, const ObjType& obj)
    {
        throw_if_found(key, __FUNCTION__);
        obj_wrapper w = std::make_pair(obj, aliases_container());
        w.second.push_back(key);
        typename obj_container::iterator o;
        o = objects.insert(objects.begin(), w); // copy object
        mapping.insert(std::make_pair(key, &(*o)));
    }

    /**
     * @brief Inserts a new element into the map.
     */
    void insert(const std::pair<KeyType, ObjType>& item)
    {
        add_object(item.first, item.second);
    }

    /**
     * @brief Removes element from the map.
     */
    void remove_object(const KeyType& key)
    {
        throw_if_not_found(key, __FUNCTION__);

        obj_wrapper* w = mapping[key];
        aliases_container& aliases = w->second;

        // remove all aliases from mapping
        typename aliases_container::iterator i;
        for(i = aliases.begin(); i != aliases.end(); i++)
        {
            mapping.erase(*i);
        }

        // remove the wrapper object itself
        typename obj_container::iterator o;
        for(o = objects.begin(); o != objects.end(); i++)
        {
            if(&(*o) == w)
            {
                objects.erase(o);
                break;
            }
        }
    }

    /**
     * @brief Removes element from the map.
     */
    void erase(const KeyType& item)
    {
        remove_object(item);
    }

    /**
     * @brief Removes element from the map.
     */
    void erase(const_iterator item_iterator)
    {
        if(item_iterator != end())
        {
            remove_object(*(item_iterator.aliases()).begin());
        }
    }

    /**
     * @brief Removes all elements from the map.
     */
    void clear()
    {
        if(size())
        {
            mapping.clear();
            objects.clear();
        }
    }

    ObjType& operator[](const KeyType& key)
    {
        throw_if_not_found(key, __FUNCTION__);
        return mapping[key]->first;
    }

    /**
     * @brief Creates and adds a new alias for an existing element held by the map.
     */
    void add_alias(const KeyType& existing_key, const KeyType& new_alias)
    {
        throw_if_not_found(existing_key, __FUNCTION__);
        throw_if_found(new_alias, __FUNCTION__);

        obj_wrapper* w = mapping[existing_key];
        w->second.push_back(new_alias);
        mapping.insert(std::make_pair(new_alias, w));
    }

    /**
     * @brief Removes alias to an existing element held by the map.
     *        If this element had only this one alias - it will be removed (erased).
     */
    void remove_alias(const KeyType& alias_or_key)
    {
        throw_if_not_found(alias_or_key, __FUNCTION__);

        aliases_container& a = mapping[alias_or_key]->second;
        if(a.size() == 1)
        {
            remove_object(alias_or_key); // if it's the only alias, remove the whole object
        }
        else
        {
            a.remove(alias_or_key);
            mapping.erase(alias_or_key);
        }
    }

    /**
     * @brief Returns number of elements this map holds.
     */
    size_t size()
    {
        return objects.size();
    }

    /**
     * @brief Begin iterator for the container
     */
    iterator begin()
    {
        return objects.begin();
    }

    /**
     * @brief Begin const iterator for the container
     */
    const_iterator begin() const
    {
        return objects.begin(); // list_iterator_wrapper conversion operator will allow this..
    }

    /**
     * @brief End iterator for the container
     */
    iterator end()
    {
        return objects.end();
    }

    /**
     * @brief End const iterator for the container
     */
    const_iterator end() const
    {
        return objects.end();
    }

    iterator find(const KeyType& alias_or_key)
    {
        typename obj_mapping::iterator m = mapping.find(alias_or_key);
        if(m != mapping.end())
        {
            typename obj_container::iterator o;
            for (o = objects.begin(); o != objects.end(); o++)
            {
                obj_wrapper* obj_ptr = &(*o);
                obj_wrapper* mapping_ptr = m->second;

                if(obj_ptr == mapping_ptr)
                {
                    return iterator(o); // make conversion here..
                }
            }
        }
        return end();
    }

    const_iterator find(const KeyType& alias_or_key) const
    {
        typename obj_mapping::iterator m = mapping.find(alias_or_key);
        if(m != mapping.end())
        {
            typename obj_container::iterator o;
            for (o = objects.begin(); o != objects.end(); o++)
            {
                obj_wrapper* obj_ptr = &(*o);
                obj_wrapper* mapping_ptr = m->second;

                if(obj_ptr == mapping_ptr)
                {
                    return iterator(o); // make conversion here..
                }
            }
        }
        return end();
    }

private:

    void throw_if_found(const KeyType& key, const char* fcn_name)
    {
        throw_if(key, fcn_name, true);
    }

    void throw_if_not_found(const KeyType& key, const char* fcn_name)
    {
        throw_if(key, fcn_name, false);
    }

    void throw_if(const KeyType& key, const char* fcn_name, bool found = false)
    {
        if((mapping.find(key) == mapping.end()) ^ found)
        {
            std::stringstream err;
            err << fcn_name << "(): key: \"" << key << "\" ";
            err << (found ? "already" : "does not");
            err << " exists!";
            throw std::runtime_error(err.str());
        }
    }

    obj_mapping mapping;
    obj_container objects;
};

#endif /* ALIAS_MAP_H_ */
