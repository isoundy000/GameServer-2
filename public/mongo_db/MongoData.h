/*
 * MongoData.h
 *
 * Created on: 2013-07-18 20:10
 *     Author: lyz
 */

#ifndef _MONGODATA_H_
#define _MONGODATA_H_

#include <memory>
#include "GameHeader.h"
using std::auto_ptr;
class TQueryCursor;
class ProtoMongoData;

class MongoData
{
public:
    enum {
        MONGO_OP_UPDATE     = 1,
        MONGO_OP_FIND       = 2,
        MONGO_OP_QUERY      = 3,
        MONGO_OP_REMOVE     = 4,
        MONGO_OP_THREAD_QUERY = 5,  // query和more在不同线程
        MONGO_OP_THREAD_QUERY_WITH_SORT = 6,  // query和more在不同线程,并且需要排序
        MONGO_DATA_BSON     = 101,
        MONGO_DATA_CURSOR   = 102,
        MONGO_DATA_TCURSOR  = 103,  // 可多线程处理的CURSOR
//        MONGO_DATA_TCURSOR_WITH_SORT = 104,  // 可多线程处理的CURSOR,排序
    };
public:
    MongoData(void);
    ~MongoData(void);
    MongoData &operator = (MongoData &data);
    void reset(void);

    void set_update(const std::string &table, BSONObj condition, const bool is_insert = true);
    void set_update(const std::string &table, BSONObj condition, BSONObj data, const bool is_insert = true);
    void set_find(const std::string &table, BSONObj condition);
    void set_query(const std::string &table, BSONObj condition);
    void set_remove(const std::string &table, BSONObj condition);
    void set_multithread_query(const std::string &table, BSONObj condition);
    void set_multithread_query(const std::string &table, BSONObj condition, BSONObj sort_condition);

    void set_data_bson(BSONObj bson);
    void set_data_cursor(auto_ptr<DBClientCursor> cursor);
    void set_multithread_cursor(auto_ptr<DBClientCursor> cursor);

    int op_type(void);
    bool is_update(void);
    bool is_auto_insert(void);
    bool is_find(void);
    bool is_query(void);
    bool is_multithread_query(void);
    bool is_multithread_query_with_sort(void);
    bool is_remove(void);
    std::string &table(void);
    BSONObj &condition(void);
    BSONObj &sort_condition(void);

    BSONObj &data_bson(void);
    auto_ptr<DBClientCursor> data_cursor(void);
    auto_ptr<TQueryCursor> multithread_cursor(void);

    void serialize(ProtoMongoData *proto_mongo_data);
    void unserialize(const ProtoMongoData &proto_mongo_data);
    
private:
    int op_type_;
    std::string table_name_;
    BSONObj *cond_bson_;
    BSONObj *sort_cond_bson_;

    bool is_insert_;
    int data_type_;
    BSONObj *data_bson_;
    auto_ptr<DBClientCursor> *data_cursor_;
    auto_ptr<TQueryCursor> *t_data_cursor_;
};

#endif //_MONGODATA_H_
