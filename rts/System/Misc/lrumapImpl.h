template <typename Key, typename Value>
lrumap<Key, Value>::lrumap(const boost::function<lrumap<Key, Value>::value_type(const Key&)>& f, 
        size_t c) :_fn(f), _capacity(c) { 
    assert(_capacity!=0); 
} 

template <typename Key, typename Value>
Value& lrumap<Key, Value>::operator[](const Key& key) { 

    // Attempt to find existing record 
    const typename container_type::left_iterator it =_container.left.find(key);

    if (it == _container.left.end()) {
        // We don't have it: 

        // Evaluate function and create new record 
        //return insert(key, _fn(key))->second;
		insert(key, _fn(key));
		return _container.left.find(key)->second;
    } else { 
        // We do have it: 

        // Update the access record view. 
        _container.right.relocate(_container.right.end(),
                _container.project_right(it)); 

        // Return the retrieved value 
        return it->second; 
    } 
} 
/*
template<typename Key, typename Value>
template <typename IT> void lrumap::get_keys(IT dst) const { 
    typename container_type::right_const_reverse_iterator src 
        =_container.right.rbegin(); 
    while (src!=_container.right.rend()) { 
        *dst++=(*src++).second; 
    } 
} */

template <typename Key, typename Value>
//typename lrumap<Key, Value>::container_type::left_iterator& lrumap<Key, Value>::insert(const Key& key, const Value& value) { 
void lrumap<Key, Value>::insert(const Key& key, const Value& value) { 

    assert(_container.size() <= _capacity); 

    // If necessary, make space 
    if (_container.size() == _capacity) { 
        // by purging the least-recently-used element 
        _container.right.erase(_container.right.begin()); 
    } 

    // Create a new record from the key and the value 
    // bimap's list_view defaults to inserting this at 
    // the list tail (considered most-recently-used). 
    _container.insert(typename container_type::value_type(key, value));
} 
