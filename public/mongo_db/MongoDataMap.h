/*
 * MongoDataMap.h
 *
 * Created on: 2013-07-19 12:02
 *     Author: lyz
 */

#ifndef _MONGODATAMAP_H_
#define _MONGODATAMAP_H_

#include "HashMap.h"

class MongoData;

class MongoDataMap
{
public:
    typedef HashMap<std::string, MongoData *, NULL_MUTEX> DataMap;

public:
    void reset(void);

    MongoData *pop_data(void);
    int push_data(MongoData *data);

    void recycle(void);

    int bind_data(MongoData *data);
    int find_data(const std::string &table, MongoData *&data);
    int unbind_data(MongoData *data);

    void push_update(const std::string &table, BSONObj condition, BSONObj data, const bool is_insert = true);
    void push_find(const std::string &table, BSONObj condition);
    void push_query(const std::string &table, BSONObj condition);
    void push_remove(const std::string &table, BSONObj condition);
    void push_multithread_query(const std::string &table, BSONObj condition);
    void push_multithread_query(const std::string &table, BSONObj condition, BSONObj sort_condition);
    void push_find(const std::string &table);
    void push_query(const std::string &table);
    void push_multithread_query(const std::string &table);

    DataMap &data_map(void);

public:
    DataMap data_map_;
    int other_value_;
    int check_role_;
};

#endif //_MONGODATAMAP_H_
