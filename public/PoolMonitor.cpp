/*
 * PoolMonitor.cpp
 *
 * Created on: 2013-01-07 11:46
 *     Author: glendy
 */

#include "MapStruct.h"
#include "LogicStruct.h"
#include "GamePackage.h"
#include "PoolMonitor.h"
#include "GameTimer.h"
#include "ObjectPoolEx.h"
#include "GameTimerHandler.h"
#include "Epoll_Watcher.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "Transaction.h"

PoolMonitor::TimerWatcher::TimerWatcher(void)
{
    this->watcher_ = new Epoll_Watcher();
    this->is_running_ = false;
}

PoolMonitor::TimerWatcher::~TimerWatcher(void)
{
    this->fini();
}

int PoolMonitor::TimerWatcher::start(void)
{
    if (this->is_running_ == true)
        return 0;

    this->is_running_ = true;
    this->thr_create();
    return 0;
}

void PoolMonitor::TimerWatcher::stop_wait(void)
{
	if (this->is_running_ == false)
		return;

	this->is_running_ = false;
	this->thr_cancel_join();
}

void PoolMonitor::TimerWatcher::fini(void)
{
	SAFE_DELETE(this->watcher_);
}

void PoolMonitor::TimerWatcher::run_handler(void)
{
	this->is_running_ = true;
    this->watcher_->loop();
}

int PoolMonitor::TimerWatcher::add(GameTimerHandler *handler, int op, Time_Value *interval)
{
    return this->watcher_->add(handler, op, interval);
}

int PoolMonitor::TimerWatcher::remove(GameTimerHandler *handler)
{
    return this->watcher_->remove(handler);
}

PoolMonitor::PoolMonitor(void) :
    timer_init_flag_(0)
{
    this->buf_pool_ = new BlockBufferPool();
    this->unit_msg_pool_ = new UnitMessagePool();
    this->transaction_pool_ = new TransactionPool();
    this->pack_item_pool_ = new PackageItemPool();
    this->game_pack_pool_ = new GamePackagePool();
    this->trade_mode_pool_ = new DBTradeModePool();
    this->shop_mode_pool_ = new DBShopModePool();
    this->mongo_data_pool_ = new MongoDataPool();
    this->mongo_data_map_pool_ = new MongoDataMapPool();
    this->mail_info_pool_ = new MailInfoPool();
    this->friend_detail_pool_ = new DBFriendDetailPool();
    this->achieve_detail_pool_ = new AchieveDetailPool();
    this->back_recharge_order_pool_ = new BackRechargeOrderPool();
    this->customer_service_record_pool_ = new CustomerServiceRecordPool();
}

PoolMonitor::~PoolMonitor(void)
{
    SAFE_DELETE(this->buf_pool_);
    SAFE_DELETE(this->unit_msg_pool_);
    SAFE_DELETE(this->transaction_pool_);
    SAFE_DELETE(this->pack_item_pool_);
    SAFE_DELETE(this->game_pack_pool_);
    SAFE_DELETE(this->trade_mode_pool_);
    SAFE_DELETE(this->shop_mode_pool_);
    SAFE_DELETE(this->mongo_data_pool_);
    SAFE_DELETE(this->mongo_data_map_pool_);
    SAFE_DELETE(this->mail_info_pool_);
    SAFE_DELETE(this->friend_detail_pool_);
    SAFE_DELETE(this->achieve_detail_pool_);
    SAFE_DELETE(this->back_recharge_order_pool_);
    SAFE_DELETE(this->customer_service_record_pool_);
}

void PoolMonitor::fina(void)
{
    for (TimerHeapList::iterator iter = this->reged_timer_set_list_.begin();
            iter != this->reged_timer_set_list_.end(); ++iter)
    {
        delete (*iter);
    }

    this->reged_timer_set_list_.clear();
    this->timer_flag_list_.clear();
    this->timer_watcher_.fini();

    this->buf_pool_->clear();
    this->unit_msg_pool_->clear();
    this->transaction_pool_->clear();
    this->pack_item_pool_->clear();
    this->game_pack_pool_->clear();
    this->trade_mode_pool_->clear();
    this->mongo_data_pool_->clear();
    this->mongo_data_map_pool_->clear();
    this->mail_info_pool_->clear();
    this->friend_detail_pool_->clear();
    this->achieve_detail_pool_->clear();
    this->back_recharge_order_pool_->clear();
    this->customer_service_record_pool_->clear();
}

PoolMonitor::TimerWatcher *PoolMonitor::global_timer_watcher(void)
{
    return &(this->timer_watcher_);
}

void PoolMonitor::init_game_timer_list(const int size)
{
#ifdef LOCAL_DEBUG
    if (this->timer_init_flag_ != 0)
        return;
    this->timer_init_flag_ = 1;
    int total_size = (GTT_LOGIC_TYPE_END - GTT_LOGIC_TYPE_BEG - 1) +
        (GTT_CHAT_TYPE_END - GTT_CHAT_TYPE_BEG - 1) +
        (GTT_MAP_TYPE_END - GTT_MAP_TYPE_BEG - 1) +
        (GTT_GATE_TYPE_END - GTT_GATE_TYPE_BEG - 1);
#else
    int total_size = size;
#endif
    this->timer_flag_list_.resize(total_size);
    this->reged_timer_set_list_.resize(total_size);
    this->timer_adjust_list_.resize(total_size);
    for (int i = 0; i < total_size; ++i)
    {
    	this->reged_timer_set_list_[i] = new TimerHeap();
    	this->timer_flag_list_[i] = 0;
    }
}

void PoolMonitor::report_pool_info(std::ostringstream &msg_stream)
{
    msg_stream << "Block Buffer Pool:" << std::endl;
    this->block_buff_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Unit Message Pool:" << std::endl;
    this->unit_msg_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Transaction Pool:" << std::endl;
    this->transaction_pool_->dump_info_to_stream(msg_stream);
    msg_stream << "PackageItemPool" << std::endl;
    this->pack_item_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "GamePackagePool" << std::endl;
    this->game_pack_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "MongoData Pool" << std::endl;
    this->mongo_data_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "MongoDataMap Pool" << std::endl;
    this->mongo_data_map_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "MailInfoPool" << std::endl;
    this->mail_info_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "TradeMode Pool:" << std::endl;
    this->trade_mode_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "ShopeMode Pool:" << std::endl;
    this->shop_mode_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "FriendDetail Pool" << std::endl;
    this->friend_detail_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "AchieveDetail Pool" << std::endl;
    this->achieve_detail_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "BackRechargeOrder Pool" << std::endl;
    this->back_recharge_order_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "CustomerServiceRecord Pool" << std::endl;
    this->customer_service_record_pool()->dump_info_to_stream(msg_stream);
}

PoolMonitor::BlockBuffer *PoolMonitor::pop_buf_block(int cid)
{
    PoolMonitor::BlockBuffer *pbuff = this->buf_pool_->pop();
    pbuff->reset();
    return pbuff;
}

int PoolMonitor::push_buf_block(BlockBuffer *buff, int cid)
{
    return this->buf_pool_->push(buff);
}

PoolMonitor::BlockBufferPool *PoolMonitor::block_buff_pool(void)
{
    return this->buf_pool_;
}

PoolMonitor::UnitMessagePool *PoolMonitor::unit_msg_pool(void)
{
    return this->unit_msg_pool_;
}

PoolMonitor::TransactionPool *PoolMonitor::transaction_pool(void)
{
    return this->transaction_pool_;
}

PoolMonitor::PackageItemPool *PoolMonitor::pack_item_pool(void)
{
    return this->pack_item_pool_;
}

PoolMonitor::GamePackagePool *PoolMonitor::game_pack_pool(void)
{
    return this->game_pack_pool_;
}

PoolMonitor::DBTradeModePool *PoolMonitor::trade_mode_pool(void)
{
	return this->trade_mode_pool_;
}

PoolMonitor::MongoDataPool *PoolMonitor::mongo_data_pool(void)
{
    return this->mongo_data_pool_;
}

PoolMonitor::MongoDataMapPool *PoolMonitor::mongo_data_map_pool(void)
{
    return this->mongo_data_map_pool_;
}

PoolMonitor::MailInfoPool *PoolMonitor::mail_info_pool(void)
{
	return this->mail_info_pool_;
}

PoolMonitor::DBShopModePool* PoolMonitor::shop_mode_pool(void)
{
	return this->shop_mode_pool_;
}

PoolMonitor::DBFriendDetailPool* PoolMonitor::friend_detail_pool(void)
{
	return this->friend_detail_pool_;
}

PoolMonitor::AchieveDetailPool* PoolMonitor::achieve_detail_pool(void)
{
	return this->achieve_detail_pool_;
}

PoolMonitor::BackRechargeOrderPool* PoolMonitor::back_recharge_order_pool(void)
{
	return this->back_recharge_order_pool_;
}

PoolMonitor::CustomerServiceRecordPool* PoolMonitor::customer_service_record_pool(void)
{
	return this->customer_service_record_pool_;
}

int PoolMonitor::timer_type_to_index(const int type)
{
    int type_beg = (type / 100 * 100);
#ifdef LOCAL_DEBUG
    int index = type - type_beg - 1;
    int logic_size = GTT_LOGIC_TYPE_END - GTT_LOGIC_TYPE_BEG - 1,
        chat_size = GTT_CHAT_TYPE_END - GTT_CHAT_TYPE_BEG - 1,
        map_size = GTT_MAP_TYPE_END - GTT_MAP_TYPE_BEG - 1,
//        gate_size = GTT_GATE_TYPE_END - GTT_GATE_TYPE_BEG - 1,
        type_index = type / 100;
    if (type_index > 1)
    {
        if (type_index == 2)
            index += logic_size;
        else if (type_index == 4)
            index += logic_size + chat_size;
        else if (type_index == 5)
            index += logic_size + chat_size + map_size;
    }
#else
    int index = type - type_beg - 1;
#endif
    return index;
}

int PoolMonitor::game_timer_timeout(const int type)
{
    int index = this->timer_type_to_index(type);
    if (index < 0 || index >= int(this->reged_timer_set_list_.size()))
        return -1;

    TimerHeap *timer_heap = this->reged_timer_set_list_[index];
    Time_Value nowtime = Time_Value::gettimeofday();
    if (type == GTT_MAP_PLAYER && this->timer_adjust_list_[index] > nowtime)
    	return 0;

    GameTimer *timer = 0;
    int size = timer_heap->size(), i = 0;

    std::vector<GameTimer *> pop_timer_list;
    while (i < size && (timer = timer_heap->top()) != 0)
    {
        ++i;
        if (timer->check_tick() > nowtime)
            break;
        timer_heap->pop();

        if (timer->is_registered())
            timer->timeout(nowtime);

        pop_timer_list.push_back(timer);
    }

    for (std::vector<GameTimer *>::iterator iter = pop_timer_list.begin(); iter != pop_timer_list.end(); ++iter)
    {
    	timer = (*iter);
		if (timer->is_registered())
		{
			if (timer_heap->is_in_heap(timer) == false)
				timer_heap->push(timer);
		}
		else
		{
			if (timer_heap->is_in_heap(timer) == true)
				timer_heap->remove(timer);
		}
    }

    if (timer_heap->size() <= 0)
        this->timer_flag_list_[index] = 0;
    else
        this->timer_flag_list_[index] = 1;

    if (type == GTT_MAP_PLAYER)
    {
    	size_t player_size = timer_heap->size();
    	Time_Value &tick = this->timer_adjust_list_[index];
    	tick = nowtime;
    	tick.usec(tick.usec() / 100000 * 100000);
    	if (player_size <= 600)
    		tick += Time_Value(0, 50000);
    	else if (player_size <= 1000)
    		tick += Time_Value(0, 150000);
    	else if (player_size <= 1500)
    		tick += Time_Value(0, 250000);
    	else if (player_size <= 2000)
    		tick += Time_Value(0, 350000);
    	else if (player_size <= 2500)
    		tick += Time_Value(0, 450000);
    	else if (player_size <= 3000)
    		tick += Time_Value(0, 650000);
    }
    return 0;
}

int PoolMonitor::register_game_timer(GameTimer *timer)
{
    int index = this->timer_type_to_index(timer->type());
    if (index < 0 || index >= int(this->reged_timer_set_list_.size()))
        return -1;

    TimerHeap *timer_heap = this->reged_timer_set_list_[index];
    if (timer_heap->is_in_heap(timer) == false)
    {
        Time_Value tick = Time_Value::gettimeofday() + timer->interval_tick();
        timer->set_check_tick(tick);
        timer_heap->push(timer);
    }
    if (timer_heap->size() > 0)
    { 
        this->timer_flag_list_[index] = 1;
    }
    return 0;
}

int PoolMonitor::unregister_game_timer(GameTimer *timer)
{
    int index = this->timer_type_to_index(timer->type());
    if (index < 0 || index >= int(this->reged_timer_set_list_.size()))
        return -1;
    
    TimerHeap *timer_heap = this->reged_timer_set_list_[index];
    if (timer_heap->is_in_heap(timer) == true)
    {
        timer_heap->remove(timer);
    }

    if (timer_heap->size() <= 0)
    {
        this->timer_flag_list_[index] = 0;
    }
    return 0;
}

int PoolMonitor::timer_flag(const int type)
{
    int index = this->timer_type_to_index(type);
    if (index < 0 || index >= int(this->reged_timer_set_list_.size()))
        return 0;

    return this->timer_flag_list_[index];
}

