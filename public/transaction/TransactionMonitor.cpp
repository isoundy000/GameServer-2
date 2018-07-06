/*
 * TransactionMonitor.cpp
 *
 * Created on: 2013-01-26 17:41
 *     Author: glendy
 */

#include "TransactionMonitor.h"
#include "PubStruct.h"
#include "Transaction.h"
#include "PoolMonitor.h"
#include "MongoUnit.h"

TransactionMonitor::TransactionMonitor(void) :
    is_running_(false), server_type_(0), 
    transaction_map_(get_hash_table_size(5000)),
    mongo_unit_amount_(0), mongo_unit_list_(0)
{ /*NULL*/ }

int TransactionMonitor::init(const int server_type, const int mongo_unit)
{
    this->server_type_ = server_type;
    this->mongo_unit_amount_ = mongo_unit;
    if (this->mongo_unit_amount_ <= 0)
        this->mongo_unit_amount_ = 1;
    this->mongo_unit_list_ = new MongoUnit[this->mongo_unit_amount_];
    return 0;
}

int TransactionMonitor::start(void)
{
    for (int i = 0; i < this->mongo_unit_amount_; ++i)
    {
        this->mongo_unit_list_[i].thr_create();
    }

    this->is_running_ = true;
    return 0;
}

int TransactionMonitor::stop(void)
{
    this->is_running_ = false;
    for (int i = 0; i < this->mongo_unit_amount_; ++i)
        this->mongo_unit_list_[i].stop_wait();

    bool is_running = false;
    while (true)
    {
        is_running = false;
        for (int i = 0; i < this->mongo_unit_amount_; ++i)
        {
            if (this->mongo_unit_list_[i].is_running() == true)
            {
                ::sleep(1);
                is_running = true;
                break;
            }
        }
        if (is_running == false)
            break;
    }
    this->recycle();
    return 0;
}

void TransactionMonitor::fina(void)
{
    if (this->mongo_unit_list_ != 0)
        delete [] this->mongo_unit_list_;
    this->mongo_unit_list_ = 0;
}

int TransactionMonitor::start_timer(void)
{
    Time_Value interval(10, 0);
    return this->schedule_timer(interval);
}

Transaction *TransactionMonitor::pop_transaction(void)
{
    Transaction *trans = POOL_MONITOR->transaction_pool()->pop();
    if (trans == 0)
        return 0;

    trans->detail().__trans_id = this->generate_trans_id();
    trans->detail().__interval_tick = Time_Value(300);
    return trans;
}

Transaction *TransactionMonitor::pop_transaction(const int res_recogn, BaseUnit *res_unit, const int client_sid,
        const Time_Value interval, const int src_recogn, 
        const int64_t role_id, const int scene_id)
{
    Transaction *trans = this->pop_transaction();
    if (trans == 0)
        return 0;

    TransactionDetail &detail = trans->detail();
    detail.__scene_id = scene_id;
    detail.__role_id = role_id;
    detail.__interval_tick = interval;
    detail.__src_recogn = src_recogn;
    detail.__res_recogn = res_recogn;
    detail.__unit = res_unit;
    detail.__trans_status = Transaction::TRANS_NULL;
    detail.__sid = client_sid;

    return trans;
}

int TransactionMonitor::push_transaction(Transaction *trans)
{
    trans->recycle_self();
    return 0;
}

int TransactionMonitor::type(void)
{
    if (this->server_type_ == SERVER_LOGIC)
        return GTT_LOGIC_TRANS;
    else if (this->server_type_ == SERVER_CHAT)
        return GTT_CHAT_TRANS;
    else if (this->server_type_ == SERVER_MAP)
        return GTT_MAP_TRANS;
    else if (this->server_type_ == SERVER_GATE)
        return GTT_GATE_TRANS;
    else
        return GTT_LOGIC_TRANS;
}

MongoUnit *TransactionMonitor::mongo_unit(const int trans_id)
{
    if (this->mongo_unit_amount_ <= 0)
        return 0;
    size_t index = trans_id % this->mongo_unit_amount_;
    return (this->mongo_unit_list_ + index);
}

int TransactionMonitor::fetch_timeout_transaction(std::vector<Transaction *> &trans_list)
{
    GUARD_READ(RW_Mutex, mon, this->transaction_map_.mutex());

    Transaction *trans = 0;
    Time_Value nowtime = Time_Value::gettimeofday();
    for (TransactionMap::iterator iter = this->transaction_map_.begin();
            iter != this->transaction_map_.end(); ++iter)
    {
        trans = iter->second;
        if (trans->check_tick() < nowtime)
            trans_list.push_back(trans);
    }
    return 0;
}

int TransactionMonitor::recycle(void)
{
    Transaction *trans = 0;
    {
        GUARD_WRITE(RW_Mutex, mon, this->transaction_map_.mutex());
        for (TransactionMap::iterator iter = this->transaction_map_.begin();
                iter != this->transaction_map_.end(); ++iter)
        {
            trans = iter->second;
            this->push_transaction(trans);
        }
    }
    this->transaction_map_.unbind_all();

    return 0;
}

int TransactionMonitor::bind_transaction(Transaction *trans)
{
    return this->transaction_map_.bind(trans->trans_id(), trans);
}

int TransactionMonitor::unbind_transaction(Transaction *trans)
{
    return this->transaction_map_.unbind(trans->trans_id());
}

int TransactionMonitor::find_transaction(const int trans_id, Transaction *&trans)
{
    return this->transaction_map_.find(trans_id, trans);
}

int TransactionMonitor::handle_timeout(const Time_Value &tv)
{
    std::vector<Transaction *> trans_list;
    if (this->fetch_timeout_transaction(trans_list) != 0)
        return 0;

    Transaction *trans = 0;
    for (std::vector<Transaction *>::iterator iter = trans_list.begin();
            iter != trans_list.end(); ++iter)
    {
        trans = *iter;
        trans->rollback();
    }

    return 0;
}

int TransactionMonitor::global_trans_id_ = 0;
int TransactionMonitor::generate_trans_id(void)
{
    return (++(TransactionMonitor::global_trans_id_)) % 100000000 + 1;
}

int TransactionMonitor::request_mongo_transaction(const int64_t role_id, const int recogn,
            const int data_type, void *data, void *data_pool,
            BaseUnit *res_unit, const int client_sid, const int req_recogn)
{
    Transaction *transaction = this->pop_transaction(recogn, res_unit, client_sid);
    transaction->detail().__role_id = role_id;
    transaction->detail().__src_recogn = req_recogn;
    if (transaction->push_data(data_type, data, data_pool) != 0)
    {
        transaction->reset();
        this->push_transaction(transaction);
        return -1;
    }
    
    int id = int(role_id);
    if (id == 0)
    {
        id = transaction->trans_id();
    }

    MongoUnit *mongo_unit = this->mongo_unit(id);
    if (transaction->begin(mongo_unit) != 0)
    {
        transaction->reset();
        this->push_transaction(transaction);
        return -1;
    }
    return 0;
}

int TransactionMonitor::request_mongo_transaction(const int64_t role_id, const int recogn, MongoDataMap *data_map,
    		BaseUnit *res_unit, const int client_sid)
{
	return this->request_mongo_transaction(role_id, recogn,
			DB_MONGO_DATA_MAP, data_map, POOL_MONITOR->mongo_data_map_pool(),
			res_unit, client_sid);
}

