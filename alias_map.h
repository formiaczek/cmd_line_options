/**
 * alias_map.h
 *
 *  Created on: 26 Aug 2013
 *      Author: forma
 */

#ifndef ALIAS_MAP_H_
#define ALIAS_MAP_H_


#include <map>
#include <list>
#include <sstream>
#include <stdexcept>

template<typename KeyType, typename ObjType>
class alias_map
{

    typedef std::list<KeyType> aliases_container;
    typedef std::pair<ObjType, aliases_container> obj_wrapper;
    typedef std::list<obj_wrapper> obj_container;
    typedef std::map<KeyType, obj_wrapper*> obj_mapping;

public:

    void add_object(const KeyType& key, const ObjType& obj)
    {
        throw_if_found(key, __FUNCTION__);
        obj_wrapper w = std::make_pair(obj, aliases_container());
        w.second.push_back(key);
        typename obj_container::iterator o;
        o = objects.insert(objects.begin(), w); // copy object
        mapping.insert(std::make_pair(key, &(*o)));
    }

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

    ObjType& operator[](const KeyType& key)
    {
        throw_if_not_found(key, __FUNCTION__);
        return mapping[key]->first;
    }

    void add_alias(const KeyType& existing_key, const KeyType& new_alias)
    {
        throw_if_not_found(existing_key, __FUNCTION__);
        throw_if_found(new_alias, __FUNCTION__);

        obj_wrapper* w = mapping[existing_key];
        w->second.push_back(new_alias);
        mapping.insert(std::make_pair(new_alias, w));
    }

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

    size_t size()
    {
        return objects.size();
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


#define TEST_COND_(_cond_) ({ if(!(_cond_)) { \
                               std::stringstream s; s << "test error: assertion at line: " << __LINE__ << "\n"; \
                               throw std::runtime_error(s.str()); } })

#define TEST_THROWS_(_st_) ({ bool failed = true; try { (_st_); \
                            } catch(...) {failed = false;} \
                            if(failed) { std::stringstream s; \
                            s << "test error: statement at line: " << __LINE__ << " should throw an exception!\n"; \
                            throw std::runtime_error(s.str()); }})


inline void test_alias_map()
{
    try
    {
    alias_map<std::string, std::string> m;

    TEST_COND_(m.size() == 0);

    m.add_object("first", "the first!");
    TEST_COND_(m.size() == 1);

    m.add_alias("first", "1");
    TEST_COND_(m.size() == 1); // adding alias doesn't change size..

    TEST_COND_(m["first"] == "the first!");
    TEST_COND_(m["1"] == "the first!");

    TEST_THROWS_(m.add_alias("first", "1")); // try adding alias again

    m.remove_alias("1");
    TEST_THROWS_( m["1"] += "abc" );
    TEST_THROWS_( m.remove_alias("1") ); // can't remove alias: it doesn't exist

    m["first"] = "something else"; // another alias still works.
    m.remove_alias("first");
    TEST_THROWS_(m["first"] = "doesnt exist!"); // but now it doesn't
    TEST_COND_(m.size() == 0);

    m.add_object("second", "222");
    m.add_alias("second", "2");
    TEST_COND_(m.size() == 1);

    TEST_COND_(m["second"] == "222");
    TEST_COND_(m["2"] == "222");

    TEST_THROWS_(m.remove_object("23"));

    m.remove_object("2");
    TEST_THROWS_( m["second"] += "abc" );
    TEST_COND_(m.size() == 0);

    std::cout << "all tests passed OK!";
    }
    catch(std::runtime_error& e)
    {
        std::stringstream err;
        err << "testing failed: " << e.what();
        throw std::runtime_error(err.str());
    }
}

#endif /* ALIAS_MAP_H_ */
