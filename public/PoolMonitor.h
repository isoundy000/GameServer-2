/*
 * PoolMonitor.h
 *
 * Created on: 2013-01-07 11:43
 *     Author: glendy
 */

#ifndef _POOLMONITOR_H_
#define _POOLMONITOR_H_

#include "boost/unordered_set.hpp"
#include "Block_Pool_Group.h"
#include "ObjectPoolEx.h"
#include "DynamicPool.h"
#include "Thread.h"
#include "GameTimer.h"

class UnitMessage;
class Transaction;
class GamePackage;
class MongoData;
class MongoDataMap;
class MailInformation;
class DBTradeMode;
class DBShopMode;
class DBFriendInfo;
class AchieveDetail;
class RechargeOrder;
class CustomerServiceRecord;
class GameTimerHandler;
class Epoll_Watcher;
class PackageItem;

class PoolMonitor
{
public:
    typedef Block_Buffer BlockBuffer;
    typedef DynamicPool<BlockBuffer> BlockBufferPool;
    typedef DynamicPool<UnitMessage> UnitMessagePool;
    typedef DynamicPool<Transaction> TransactionPool;
    typedef ObjectPoolEx<PackageItem> PackageItemPool;
    typedef ObjectPoolEx<GamePackage> GamePackagePool;
    typedef DynamicPool<MongoData> MongoDataPool;
    typedef DynamicPool<MongoDataMap> MongoDataMapPool;
    typedef ObjectPoolEx<MailInformation> MailInfoPool;
    typedef DynamicPool<DBTradeMode> DBTradeModePool;
    typedef ObjectPoolEx<DBShopMode> DBShopModePool;
    typedef DynamicPool<DBFriendInfo> DBFriendDetailPool;
    typedef DynamicPool<AchieveDetail> AchieveDetailPool;
    typedef ObjectPoolEx<RechargeOrder> BackRechargeOrderPool;
    typedef ObjectPoolEx<CustomerServiceRecord> CustomerServiceRecordPool;

    typedef boost::unordered_set<GameTimer *> TimerSet;
    typedef Heap<GameTimer, GameTimerCmp> TimerHeap;
    typedef std::vector<TimerHeap *> TimerHeapList;
    typedef std::vector<TimerSet *> TimerSetList;
    typedef std::vector<int> TimerFlagList;
    typedef std::vector<Time_Value> TimerAdjustList;

    class TimerWatcher : public Thread
    {
    public:
        TimerWatcher(void);
        virtual ~TimerWatcher(void);
        virtual int start(void);
        virtual void stop_wait(void);
        virtual void fini(void);
        virtual void run_handler(void);
        int add(GameTimerHandler *handler, int op, Time_Value *interval);
        int remove(GameTimerHandler *handler);

    private:
        Epoll_Watcher *watcher_;
        bool is_running_;
    };

public:
    PoolMonitor(void);
    ~PoolMonitor(void);

    void fina(void);

    BlockBuffer *pop_buf_block(int cid = 0);
    int push_buf_block(BlockBuffer *buff, int cid = 0);
    BlockBufferPool *block_buff_pool(void);

    UnitMessagePool *unit_msg_pool(void);
    TransactionPool *transaction_pool(void);
    PackageItemPool *pack_item_pool(void);
    GamePackagePool *game_pack_pool(void);
    DBTradeModePool *trade_mode_pool(void);
    MongoDataPool *mongo_data_pool(void);
    MongoDataMapPool *mongo_data_map_pool(void);
    MailInfoPool *mail_info_pool(void);
    DBShopModePool *shop_mode_pool(void);
    DBFriendDetailPool *friend_detail_pool(void);
    AchieveDetailPool *achieve_detail_pool(void);
    BackRechargeOrderPool *back_recharge_order_pool(void);
    CustomerServiceRecordPool *customer_service_record_pool(void);

    TimerWatcher *global_timer_watcher(void);
    void init_game_timer_list(const int size);
    int timer_type_to_index(const int type);
    int game_timer_timeout(const int type);
    int register_game_timer(GameTimer *timer);
    int unregister_game_timer(GameTimer *timer);
    int timer_flag(const int type);

    void report_pool_info(std::ostringstream &stream);

private:
    BlockBufferPool *buf_pool_;
    UnitMessagePool *unit_msg_pool_;
    TransactionPool *transaction_pool_;
    PackageItemPool *pack_item_pool_;
    GamePackagePool *game_pack_pool_;
    DBTradeModePool *trade_mode_pool_;
    DBShopModePool *shop_mode_pool_;
    MongoDataPool *mongo_data_pool_;
    MongoDataMapPool *mongo_data_map_pool_;
    MailInfoPool *mail_info_pool_;
    DBFriendDetailPool *friend_detail_pool_;
    AchieveDetailPool *achieve_detail_pool_;
    BackRechargeOrderPool *back_recharge_order_pool_;
    CustomerServiceRecordPool *customer_service_record_pool_;

    TimerWatcher timer_watcher_;
    TimerHeapList reged_timer_set_list_;
    TimerFlagList timer_flag_list_;

    int timer_init_flag_;
    TimerAdjustList timer_adjust_list_;
};

typedef Singleton<PoolMonitor> PoolMonitorSingle;
#define POOL_MONITOR    PoolMonitorSingle::instance()

#endif //_POOLMONITOR_H_
