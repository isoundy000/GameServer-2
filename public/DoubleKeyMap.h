/*
 * DoubleKeyMap.h
 *
 * Created on: 2013-02-28 16:25
 *     Author: glendy
 */

#ifndef _DOUBLEKEYMAP_H_
#define _DOUBLEKEYMAP_H_

#include "HashMap.h"

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
class DoubleKeyMap
{
public:
    typedef HashMap<KEY_2, VALUE, MAP_MUTEX> KeyValueMap;
    typedef HashMap<KEY_1, KeyValueMap *, MAP_MUTEX> DKeyValueMap;

    typedef typename HashMap<KEY_1, KeyValueMap *, MAP_MUTEX>::iterator iterator;

public:
    DoubleKeyMap(void);
    ~DoubleKeyMap(void);
    
    int find_object_map(const KEY_1 &key, KeyValueMap *&k_map);
    int find(const KEY_1 &key_1, const KEY_2 &key_2, VALUE &value);
    int bind(const KEY_1 &key_1, const KEY_2 &key_2, const VALUE &value);
    int rebind(const KEY_1 &key_1, const KEY_2 &key_2, const VALUE &value);
    int unbind(const KEY_1 &key_1, const KEY_2 &key_2);
    int unbind(const KEY_1 &key_1, const KEY_2 &key_2, VALUE &value);

    int unbind_all(void);

    iterator begin(void)
    {
    	return this->double_map_.begin();
    }

    iterator end(void)
    {
    	return this->double_map_.end();
    }

    MAP_MUTEX &mutex(void);

private:
    DKeyValueMap double_map_;
};

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::DoubleKeyMap(void)
{ /*NULL*/ }

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::~DoubleKeyMap(void)
{
    this->unbind_all();
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::find_object_map(const KEY_1 &key, KeyValueMap *&k_map)
{
    return this->double_map_.find(key, k_map);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::find(const KEY_1 &key_1, const KEY_2 &key_2, VALUE &value)
{
	KeyValueMap *k_map = 0;
	if (this->find_object_map(key_1, k_map) != 0)
		return -1;
    return k_map->find(key_2, value);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::bind(const KEY_1 &key_1, const KEY_2 &key_2, const VALUE &value)
{
    KeyValueMap *k_map = 0;
    if (this->double_map_.find(key_1, k_map) == 0)
        return k_map->bind(key_2, value);
    
    k_map = new KeyValueMap();
    if (this->double_map_.bind(key_1, k_map) != 0)
    {
        delete k_map;
        k_map = 0;
        this->double_map_.find(key_1, k_map);
    }
    return k_map->bind(key_2, value);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::rebind(const KEY_1 &key_1, const KEY_2 &key_2, const VALUE &value)
{
    KeyValueMap *k_map = 0;
    if (this->double_map_.find(key_1, k_map) == 0)
        return k_map->rebind(key_2, value);
    
    k_map = new KeyValueMap();
    if (this->double_map_.bind(key_1, k_map) != 0)
    {
        delete k_map;
        k_map = 0;
        this->double_map_.find(key_1, k_map);
    }
    return k_map->rebind(key_2, value);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::unbind(const KEY_1 &key_1, const KEY_2 &key_2)
{
    KeyValueMap *k_map = 0;
    if (this->find_object_map(key_1, k_map) != 0)
        return -1;

    return k_map->unbind(key_2);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::unbind(const KEY_1 &key_1, const KEY_2 &key_2, VALUE &value)
{
    KeyValueMap *k_map = 0;
    if (this->find_object_map(key_1, k_map) != 0)
        return -1;

    return k_map->unbind(key_2, value);
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
int DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::unbind_all(void)
{
    {
        GUARD_READ(MAP_MUTEX, mon, this->mutex());

        for (typename DKeyValueMap::iterator iter = this->double_map_.begin();
                iter != this->double_map_.end(); ++iter)
        {
            iter->second->unbind_all();
            delete (iter->second);
        }
    }
    this->double_map_.unbind_all();
    return 0;
}

template<class KEY_1, class KEY_2, class VALUE, class MAP_MUTEX>
MAP_MUTEX &DoubleKeyMap<KEY_1, KEY_2, VALUE, MAP_MUTEX>::mutex(void)
{
    return this->double_map_.mutex();
}

#endif //_DOUBLEKEYMAP_H_
