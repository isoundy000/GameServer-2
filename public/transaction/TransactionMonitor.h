/*
 * TransactionMonitor.h
 *
 * Created on: 2013-01-26 14:52
 *     Author: glendy
 */

#ifndef _TRANSACTIONMONITOR_H_
#define _TRANSACTIONMONITOR_H_

#include "GameHeader.h"
#include "HashMap.h"
#include "GameTimer.h"

class TransactionMonitor : public GameTimer
{
public:
    typedef HashMap<int, Transaction*, RW_Mutex> TransactionMap;

public:
    TransactionMonitor(void);
    int init(const int server_type, const int mongo_unit = 1);
    int start(void);
    int stop(void);
    void fina(void);
    int start_timer(void);

    Transaction *pop_transaction(void);
    Transaction *pop_transaction(const int res_recogn, BaseUnit *res_unit = 0, const int client_sid = 0,
            const Time_Value interval = Time_Value(300), const int src_recogn = 0,
            const int64_t role_id = 0L, const int scene_id = 0);
    int push_transaction(Transaction *trans);
 
    virtual int type(void);

    MongoUnit *mongo_unit(const int trans_id);

    int bind_transaction(Transaction *trans);
    int unbind_transaction(Transaction *trans);
    int find_transaction(const int trans_id, Transaction *&trans);

    int request_mongo_transaction(const int64_t role_id, const int recogn,
            const int data_type, void *data, void *data_pool,
            BaseUnit *res_unit = 0, const int client_sid = 0, const int req_recogn = 0);
    int request_mongo_transaction(const int64_t role_id, const int recogn, MongoDataMap *data_map,
    		BaseUnit *res_unit = 0, const int client_sid = 0);

protected:
    int fetch_timeout_transaction(std::vector<Transaction *> &trans_list);
    int recycle(void);

    virtual int handle_timeout(const Time_Value &tv);
    int generate_trans_id(void);

private:
    static int global_trans_id_;

    bool is_running_;

    int server_type_;
    TransactionMap transaction_map_;

    int mongo_unit_amount_;
    MongoUnit *mongo_unit_list_;
};

typedef Singleton<TransactionMonitor> TransactionMonitorSingle;
#define TRANSACTION_MONITOR     TransactionMonitorSingle::instance()

#endif //_TRANSACTIONMONITOR_H_
