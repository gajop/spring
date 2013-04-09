#ifndef _lrumap_ 
#define _lrumap_

#include <boost/bimap.hpp> 
#include <boost/bimap/list_of.hpp> 
#include <boost/bimap/unordered_set_of.hpp> 
#include <boost/bimap/set_of.hpp> 
#include <boost/function.hpp> 
#include <boost/functional/hash.hpp>
#include <cassert> 

#define SET boost::bimaps::unordered_set_of

// Class providing fixed-size (by number of records) 
// LRU-replacement cache of a function with signature 
// Value f(Key ). 
// SET is expected to be one of boost::bimaps::set_of 
// or boost::bimaps::unordered_set_of 
template <typename Key, typename Value> 
class lrumap { 

public: 	
    typedef boost::bimaps::bimap<
        SET<Key >, boost::bimaps::list_of<Value> > container_type; 

	typedef Value value_type;
    // Constuctor specifies the cached function and 
    // the maximum number of records to be stored. 
    lrumap(const boost::function<Value(const Key &)>& f, size_t c);

    // Obtain value of the cached function for k 
    Value& operator[](const Key& key);
private: 

    //typename lrumap<Key, Value>::container_type::left_iterator& insert(const Key & key, const Value& value);
	void insert(const Key & key, const Value& value);

    const boost::function<Value(const Key &)> _fn; 
    const size_t _capacity; 
    container_type _container; 
}; 

#include "lrumapImpl.h"

#endif
