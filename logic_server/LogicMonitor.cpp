/*
 * LogicMonitor.cpp
 *
 * Created on: 2013-01-08 10:45
 *     Author: glendy
 */

#include <BackGameModifySys.h>
#include "LogicStruct.h"
#include "Transaction.h"
#include "CenterUnit.h"
#include "LogicUnit.h"
#include "BackUnit.h"
#include "LogicCommunicate.h"
#include "LogicPlayer.h"
#include "LogicTimerHandler.h"
#include "LogicMonitor.h"
#include "PoolMonitor.h"
#include "LogClientMonitor.h"
#include "MarketSystem.h"
#include "GameField.h"
#include "MongoConnector.h"
#include "TransactionMonitor.h"
#include "MMOFriendshipValue.h"
#include "TeamPlatform.h"
#include "LogicFriendSystem.h"
#include "LeagueSystem.h"
#include "ShopMonitor.h"
#include "RankSystem.h"
#include "ProtoDefine.h"
#include "FlowControl.h"
#include "GameNoticeSys.h"
#include "MMOLeague.h"
#include "FestActivitySys.h"

#include "DoubleEscort.h"
#include "QuintupleOnline.h"
#include "SendActReward.h"
#include "InvestRechargeSys.h"
#include "ArenaSys.h"
#include "ActivityTipsSystem.h"
#include "BackstageBrocastControl.h"
#include "RestrictionSystem.h"
#include "LogicGameSwitcherSys.h"
#include "BackActivityTick.h"
#include "MMOWorldBoss.h"
#include "MongoDataMap.h"
#include "MMOGlobal.h"
#include "WeddingMonitor.h"
#include "BackGameSwitcher.h"
#include "MMOSocialer.h"
#include "OpenActivitySys.h"
#include "LuckyWheelSys.h"
#include "LucktTableMonitor.h"
#include "DailyActSys.h"
#include "ActivityStruct.h"
#include "JYBackActivitySys.h"
#include "MayActivitySys.h"
#include "TrvlTeamSystem.h"

LogicMonitor::LogicMonitor(void) :
    global_key_map_(get_hash_table_size(LogicMonitor::MAP_OBJECT_BUCKET)),
    player_map_(get_hash_table_size(LogicMonitor::MAP_OBJECT_BUCKET)),
    name_player_map_(get_hash_table_size(LogicMonitor::MAP_OBJECT_BUCKET))
{
    this->client_packer_ = new LogicClientPacker();
    this->inner_packer_ = new LogicInnerPacker();
    this->connect_receiver_ = new LogicMonitor::ConnectReceiver();
    this->connect_packer_ = new LogicConnectPacker();
    this->php_receiver_ = new PhpReceiver();
    this->php_sender_ = new PhpSender();
    this->php_packer_ = new LogicPhpPacker();
    this->php_svc_monitor_ = new PhpServiceMonitor();
   
    this->logic_unit_ = new LogicUnit();
    this->center_unit_ = new CenterUnit();
    this->back_unit_ = new BackUnit();

    this->player_pool_ = new LogicPlayerPool();
    this->box_record_pool_ = new BoxRecordPool();
    this->rpm_recomand_info_pool_ = new RpmRecomandInfoPool();
    this->backstage_brocast_record_pool_ = new BackBrocastRecPool();
    this->recharge_rank_item_pool_ = new RechargeRankItemPool();
    this->wedding_detail_pool_ = new WeddingDetailPool();
    this->jyback_activity_item_pool_ = new JYBackActivityItemPool();
    this->boss_map_ = new BossMap();
    this->combine_first_ = 0;
}

LogicMonitor::~LogicMonitor(void)
{
    SAFE_DELETE(this->client_packer_);
    SAFE_DELETE(this->inner_packer_);
    SAFE_DELETE(this->connect_receiver_);
    SAFE_DELETE(this->connect_packer_);
    SAFE_DELETE(this->php_receiver_);
    SAFE_DELETE(this->php_sender_);
    SAFE_DELETE(this->php_packer_);
    SAFE_DELETE(this->php_svc_monitor_);
   
    SAFE_DELETE(this->logic_unit_);
    SAFE_DELETE(this->center_unit_);
    SAFE_DELETE(this->back_unit_);

    SAFE_DELETE(this->player_pool_);
    SAFE_DELETE(this->box_record_pool_);
    SAFE_DELETE(this->rpm_recomand_info_pool_);
    SAFE_DELETE(this->backstage_brocast_record_pool_);

    SAFE_DELETE(this->timer_handler_list_);
    SAFE_DELETE(this->midnight_timer_);
    SAFE_DELETE(this->one_second_timer_);
    SAFE_DELETE(this->ten_second_timer_);
    SAFE_DELETE(this->int_minute_timer_);
    SAFE_DELETE(this->one_minute_timer_);
    SAFE_DELETE(this->one_hour_timer_);
    SAFE_DELETE(this->recharge_rank_item_pool_);
    SAFE_DELETE(this->wedding_detail_pool_);
    SAFE_DELETE(this->jyback_activity_item_pool_);
    SAFE_DELETE(this->boss_map_);
}

int LogicMonitor::init_game_timer_handler(void)
{
    const double inter_tick_list[] = {
        LOGIC_PLAYER_INTERVAL,
        LOGIC_TRANSACTION_INTERVAL,
        LOGIC_MONITOR_INTERVAL,
        double(Time_Value::SECOND),
        double(Time_Value::MINUTE),
        double(Time_Value::HOUR)
    };
    const int timer_amount = sizeof(inter_tick_list) / sizeof(double);

    this->timer_handler_list_->resize(timer_amount);
    POOL_MONITOR->init_game_timer_list(timer_amount);

    double inter_sec = 0.0, inter_usec = 0.0;
    int index = 0;
    for (int i = GTT_LOGIC_TYPE_BEG + 1; i < GTT_LOGIC_TYPE_END; ++i)
    {
        index = i - GTT_LOGIC_TYPE_BEG - 1;
        inter_usec = modf(inter_tick_list[index], &inter_sec);
        Time_Value interval(inter_sec, inter_usec * 1000000);
        (*(this->timer_handler_list_))[index].set_type(i);
        (*(this->timer_handler_list_))[index].set_interval(interval);
    }
    return 0;
}

int LogicMonitor::start_game_timer_handler(void)
{
	for (TimerHandlerList::iterator iter = this->timer_handler_list_->begin();
			iter != this->timer_handler_list_->end(); ++iter)
	{
		iter->schedule_timer(iter->interval());
	}
	return 0;
}

int LogicMonitor::combine_first()
{
	return this->combine_first_;
}

int LogicMonitor::is_need_day_reset(Int64 last_tick)
{
	if (last_tick >= this->midnight_timer_->nextday_zero_)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int LogicMonitor::init(void)
{
	this->is_inited_ = true;
    this->timer_handler_list_ = new TimerHandlerList();
    this->midnight_timer_ = new LogicMidNightTimer();
    this->one_second_timer_ = new LogicOneSecTimer();
    this->ten_second_timer_ = new LogicTenSecTimer();
    this->int_minute_timer_ = new LogicIntMinTimer();
    this->one_minute_timer_ = new LogicOneMinTimer();
    this->one_hour_timer_ = new LogicOneHourTimer();

    Time_Value client_send_timeout(0, 100 * 1000);
    Time_Value inner_send_timeout(0, 50 * 1000);
    {
        Time_Value recv_timeout(300, 0);
        this->client_receiver_.set(&recv_timeout);
    }
    {
        Time_Value send_timeout(0, 100 * 1000);
        this->connect_sender_.set(send_timeout);
    }
    {
        Time_Value recv_timeout(30), send_timeout(0, 200 * 1000);
        this->php_receiver_->set(&recv_timeout);
        this->php_sender_->set(send_timeout);
    }
    this->client_monitor_.set_svc_max_recv_size(400);
    this->client_monitor_.set_svc_max_pack_size(10 * 1024);
    this->inner_monitor_.set_svc_max_list_size(20000);
    this->inner_monitor_.set_svc_max_pack_size(1024 * 1024);
    this->connect_monitor_.set_svc_max_list_size(20000);
    this->php_svc_monitor_->set_svc_max_recv_size(100);
    this->php_svc_monitor_->set_svc_max_pack_size(10 * 1024);

    this->client_packer_->monitor(&(this->client_monitor_));
    this->client_monitor_.packer(this->client_packer_);

    this->inner_packer_->monitor(&(this->inner_monitor_));
    this->inner_monitor_.packer(this->inner_packer_);

    this->connect_receiver_->monitor(&(this->connect_monitor_));
    this->connect_monitor_.receiver(this->connect_receiver_);
    this->connect_packer_->monitor(&(this->connect_monitor_));
    this->connect_monitor_.packer(this->connect_packer_);

    this->php_receiver_->monitor(this->php_svc_monitor_);
    this->php_sender_->monitor(this->php_svc_monitor_);
    this->php_packer_->monitor(this->php_svc_monitor_);
    this->php_svc_monitor_->receiver(this->php_receiver_);
    this->php_svc_monitor_->set_senders(this->php_sender_);
    this->php_svc_monitor_->packer(this->php_packer_);
    this->php_receiver()->init();
    this->php_sender()->init();
    this->php_acceptor()->init();

    return SUPPER::init(client_send_timeout, inner_send_timeout);
}

int LogicMonitor::start(void)
{
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];
    for (uint i = 0; i < servers_json.size(); ++i)
    {
        const Json::Value &server_json = servers_json[i];

        if (server_json["service"].asString() != SERVICE_NAME_MAP)
            continue;

        for (uint j = 0; j < server_json["scene"].size(); ++j)
        {
        	int scene_id = server_json["scene"][j].asInt();
            this->scene_to_chat_map_.rebind(scene_id, server_json["chat_scene"].asInt());
        }
    }

    if (this->load_global_data() != 0)
    {
        MSG_USER("ERROR load global data");
        ::exit(-1);
    }

    this->update_back_role_offline();

    const Json::Value &server_json = servers_json[this->server_config_index_];
    int limit_accept = server_json["limit_accept"].asInt();
    this->client_monitor_.acceptor()->set_limit_connect(limit_accept);

    int php_port = server_json["php_port"].asInt();
    if (php_port > 0)
    { 
        this->php_acceptor()->set(php_port);
        this->php_packer()->thr_create();
        this->php_sender()->thr_create();
        this->php_receiver()->thr_create();
        this->php_acceptor()->thr_create();
    }

    if (this->center_unit() != 0)
    {
        this->center_unit()->thr_create();
    }

    if (this->back_unit() != 0)
    {
        this->back_unit()->thr_create();
    }

    SUPPER::start();

    MSG_USER("start logic server...");
    return 0;
}

// 此处放需要启动前加载或初始化的代码接口
int LogicMonitor::start_game(void)
{
	MMOGlobal::load_combine_first_value(this->combine_first_);
	MSG_USER("LogicMonitor combine first %d", this->combine_first_);

	LOGIC_SWITCHER_SYS->start();

	RANK_SYS->start();
	MARKET_SYS->start();
	SHOP_MONITOR->start();

	QUINTUPLE_SYS->init_quintuple();
	DOUBLE_ESCORT->init_escort();
	SEND_ACTREWARD->init();
	BackIR_SYS->init();

	/*
	 * LOGIC_OPEN_ACT_SYS 需要在 LEAGUE_SYSTEM 前启动，
	 * 不然新的宗派排行数据会被数据库数据覆盖
	 * */
	LOGIC_OPEN_ACT_SYS->start();
	LUCKY_WHEEL_SYSTEM->start();
	DAILY_ACT_SYSTEM->start();

	LOGIC_FRIEND_SYSTEM->start();
	GAME_NOTICE_SYS->init();
	FEST_ACTIVITY_SYS->start();
	MAY_ACTIVITY_SYS->start();
	TRAVEL_TEAM_SYS->start();

	ACTIVITY_TIPS_SYSTEM->init();
	FLOW_INSTANCE->load_flow_detail_when_init();
	CACHED_INSTANCE->init_rpm_introduction_info();

	ARENA_SYS->init();
	BBC_INSTANCE->start();
    BACK_ACTIVITY_TICK->init();
    WEDDING_MONITOR->init();
    LEAGUE_SYSTEM->start();
    BACK_ACTIVITY_SYS->start();

    this->update_limit_team_scene_set();
	this->start_logic_monitor_timer();
	this->midnight_timer_->schedule_timer();
	this->one_second_timer_->schedule_timer();
    this->ten_second_timer_->schedule_timer();
    this->int_minute_timer_->schedule_timer();
	this->one_minute_timer_->schedule_timer();
	this->one_hour_timer_->schedule_timer();

	MMOGlobal::save_comnbine_first_value(0);
	return 0;
}

int LogicMonitor::stop()
{
	MARKET_SYS->stop();
    SHOP_MONITOR->stop();
	LEAGUE_SYSTEM->stop();
	LOGIC_FRIEND_SYSTEM->stop();

	RANK_SYS->stop();
	BackIR_SYS->fina();
	ARENA_SYS->fina();
	BBC_INSTANCE->stop();
	FEST_ACTIVITY_SYS->stop();
	MAY_ACTIVITY_SYS->stop();
	TRAVEL_TEAM_SYS->stop();
	
	LOGIC_OPEN_ACT_SYS->stop();
	LUCKY_WHEEL_SYSTEM->stop();
	DAILY_ACT_SYSTEM->stop();
	LOGIC_SWITCHER_SYS->stop();
	WEDDING_MONITOR->stop();

    this->midnight_timer_->cancel_timer();
	this->logic_monitor_timer_.cancel_timer();

	this->one_hour_timer_->cancel_timer();
    this->one_second_timer_->cancel_timer();
    this->one_minute_timer_->cancel_timer();
    this->ten_second_timer_->cancel_timer();
    this->int_minute_timer_->cancel_timer();

    this->php_acceptor()->thr_cancel_join();
    this->php_receiver_->thr_cancel_join();
    this->php_sender_->thr_cancel_join();
    this->php_packer_->thr_cancel_join();

    if (this->center_unit() != 0)
    {
        this->center_unit()->stop_wait();
    }

    if (this->back_unit() != 0)
    {
        this->back_unit()->stop_wait();
    }

    return SUPPER::stop();
}

void LogicMonitor::fina(void)
{
    this->connect_receiver_->fini();

    this->php_receiver_->fini();
    this->php_sender_->fini();
    this->php_svc_monitor_->fini();

    this->player_map_.unbind_all();
    this->name_player_map_.unbind_all();

    this->player_pool_->clear();
    this->box_record_pool_->clear();
    this->rpm_recomand_info_pool_->clear();
    this->recharge_rank_item_pool_->clear();
    this->wedding_detail_pool_->clear();
    this->jyback_activity_item_pool_->clear();

    this->timer_handler_list_->clear();
    this->global_key_map_.clear();

    MARKET_SYS->fina();
    BBC_INSTANCE->fina();

    ShopMonitorSingle::destroy();
    TeamPlatformSingle::destroy();
    MarketSystemSingle::destroy();

    SUPPER::fina();
}

int LogicMonitor::start_logic_monitor_timer(void)
{
    this->logic_monitor_timer_.monitor_ = LOGIC_MONITOR;
    this->logic_monitor_timer_.schedule_timer(Time_Value(1));

	MSG_USER("start_logic_monitor_timer");
	return 0;
}

BaseUnit *LogicMonitor::logic_unit(void)
{
    return this->logic_unit_;
}

CenterUnit *LogicMonitor::center_unit(void)
{
    return this->center_unit_;
}

BackUnit *LogicMonitor::back_unit(void)
{
    return this->back_unit_;
}

LogicMonitor::PhpServiceAcceptor *LogicMonitor::php_acceptor(void)
{
    return this->php_svc_monitor_->acceptor();
}

LogicMonitor::PhpReceiver *LogicMonitor::php_receiver(void)
{
    return this->php_receiver_;
}

LogicMonitor::PhpSender *LogicMonitor::php_sender(void)
{
    return this->php_sender_;
}

LogicPhpPacker *LogicMonitor::php_packer(void)
{
    return this->php_packer_;
}

Block_Buffer *LogicMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int LogicMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

LogicMonitor::UnitMessagePool *LogicMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

void LogicMonitor::report_pool_info(const bool report)
{
    {
        std::ostringstream msg_stream;
        POOL_MONITOR->report_pool_info(msg_stream);
        MSG_USER("%s", msg_stream.str().c_str());
    }

    std::ostringstream msg_stream;
    msg_stream << "Logic Pool Info:" << std::endl;

    msg_stream << "ClientServicePool:" << std::endl;
    this->client_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Inner Service Pool:" << std::endl;
    this->inner_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Connect Service Pool:" << std::endl;
    this->connect_monitor_.service_pool()->dump_info_to_stream(msg_stream);

    msg_stream << "LogicPlayer Pool" << std::endl;
    this->player_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "BoxRecord Pool" << std::endl;
    this->box_record_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "RPMRecomand Pool" << std::endl;
    this->rpm_recomand_info_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "BackBrocastRec Pool" << std::endl;
    this->backstage_brocast_record_pool()->report_pool_info(msg_stream);
    msg_stream << "RechargeRankItem Pool" << std::endl;
    this->recharge_rank_item_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "WeddingDetail Pool" << std::endl;
    this->wedding_detail_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Player Offline Data Pool" << std::endl;
    RANK_SYS->offline_data_map().report_pool_info(msg_stream);

    if (report == false)
        return;
    MSG_USER("%s", msg_stream.str().c_str());
}

LogicMonitor::PlayerMap &LogicMonitor::player_map(void)
{
	return this->player_map_;
}

int LogicMonitor::gate_sid_list(std::vector<int> &gate_sid_vc)
{
    return this->inner_sid_list(gate_sid_vc);
}

int LogicMonitor::first_gate_sid(void)
{
    return this->first_inner_sid();
}

int LogicMonitor::dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, const ProtoHead *proto_head, const Message *msg_proto)
{
	JUDGE_RETURN(gate_sid >= 0, -1);

    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead),
             len = sizeof(ProtoHead), byte_size = 0;
    if (msg_proto != 0)
    {
        byte_size = msg_proto->ByteSize();
    }

    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)route_head, sizeof(InnerRouteHead));
    pbuff->write_uint32(len);
    pbuff->copy((char *)proto_head, sizeof(ProtoHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
    }

    int ret = this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int LogicMonitor::dispatch_to_scene(const int gate_sid, const InnerRouteHead &route_head, const ProtoHead &proto_head, Block_Buffer *msg_buff)
{
	JUDGE_RETURN(gate_sid >= 0, -1);

    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead),
             len = sizeof(ProtoHead), byte_size = 0;
    if (msg_buff != 0)
    {
        byte_size = msg_buff->readable_bytes();
    }

    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)(&route_head), sizeof(InnerRouteHead));
    pbuff->write_uint32(len);
    pbuff->copy((char *)(&proto_head), sizeof(ProtoHead));
    if (msg_buff != 0)
    {
        pbuff->copy(msg_buff);
    }

    int ret = this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int LogicMonitor::dispatch_to_scene(const int gate_sid, InnerRouteHead *route_head, ProtoHead *proto_head, const Message *msg_proto)
{
    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead),
             len = sizeof(ProtoHead), byte_size = 0;

    if (msg_proto != 0)
        byte_size = msg_proto->ByteSize();
    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)route_head, sizeof(InnerRouteHead));
    pbuff->write_uint32(len);
    pbuff->copy((char *)proto_head, sizeof(ProtoHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
    }
    return this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
}

int LogicMonitor::dispatch_to_chat(BaseLogicPlayer *player, const int recogn)
{
    int scene_id = this->chat_scene(player->scene_id());
    int sid = this->fetch_sid_of_scene(scene_id);
    if (sid < 0)
        return -1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = player->role_id();
    head.__scene_id = scene_id;

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(sizeof(ProtoHead) + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(sizeof(ProtoHead));
    pbuff->copy((char *)&head, sizeof(ProtoHead));
    return this->connect_sender()->push_pool_block_with_len(pbuff);
}

int LogicMonitor::dispatch_to_chat(BaseLogicPlayer *player, const Message *msg_proto)
{
    int scene_id = this->chat_scene(player->scene_id());
    int sid = this->fetch_sid_of_scene(scene_id);
    if (sid < 0)
        return -1;

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = player->role_id();
    head.__scene_id = scene_id;
    if (head.__recogn <= 0)
        return -1;

    uint32_t len = sizeof(ProtoHead), byte_size = msg_proto->ByteSize();
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoHead));
    msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
    pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);

    return this->connect_sender()->push_pool_block_with_len(pbuff);
}

int LogicMonitor::dispatch_to_chat(const int chat_scene, const Message *msg_proto)
{
    int sid = this->fetch_sid_of_scene(chat_scene);
    JUDGE_RETURN(sid >= 0, -1);

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    JUDGE_RETURN(recogn > 0, -1);

    ProtoHead head;
    head.__recogn = recogn;
    head.__scene_id = chat_scene;

    uint32_t len = sizeof(ProtoHead), byte_size = msg_proto->ByteSize();
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoHead));
    msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
    pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);

    return this->connect_sender()->push_pool_block_with_len(pbuff);
}

int LogicMonitor::dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const int recogn)
{
    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, 0);
}

int LogicMonitor::dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_TARGET_SCENE;

    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

// line_id >= 0, 从0开始, 配置则从1开始
int LogicMonitor::dispatch_to_special_line_scene(const int gate_sid, const int64_t role_id, const int line_id, const int scene_id, const int recogn)
{
    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_TARGET_LINE_SCENE;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;
    route_head.__line_id = line_id;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;
    head.__src_line_id = line_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, 0);
}

// line_id >= 0, 从0开始, 配置则从1开始
int LogicMonitor::dispatch_to_special_line_scene(const int gate_sid, const int64_t role_id, const int line_id, const int scene_id, const Message *msg_proto)
{
	int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_TARGET_LINE_SCENE;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;
    route_head.__line_id = line_id;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;
    head.__src_line_id = line_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_scene(BaseLogicPlayer *player, const Message *msg_proto)
{
    return this->dispatch_to_scene(player->gate_sid(), player->role_id(), player->scene_id(), msg_proto);
}

int LogicMonitor::dispatch_to_scene_with_back(BaseLogicPlayer *player, const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    int gate_sid = player->gate_sid();
    int scene_id = player->scene_id();

    Int64 role_id = player->role_id();

    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_TARGET_SCENE_BACK;

    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_scene(BaseLogicPlayer *player, const int scene_id, const Message *msg_proto)
{
	return this->dispatch_to_scene(player->gate_sid(), player->role_id(), scene_id, msg_proto);
}

int LogicMonitor::dispatch_to_client(const int gate_sid, const int64_t role_id, const int recogn, const int error)
{
    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__error = error;
    head.__scene_id = 1;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, 0);
}

int LogicMonitor::dispatch_to_client(const int gate_sid, const int64_t role_id, const int error, const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__error = error;
    head.__scene_id = 1;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_client(BaseLogicPlayer *player, const int recogn, const std::string& msg_body)
{
	int gate_sid = player->gate_sid();
	Int64 role_id = player->role_id();

    InnerRouteHead route_head;
    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;

    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(ProtoHead) + sizeof(uint32_t),
             len = sizeof(ProtoHead), byte_size = msg_body.size();

    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(player->gate_sid());
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);

    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)&route_head, sizeof(InnerRouteHead));

    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoHead));

    pbuff->copy(msg_body.c_str(), byte_size);
    return this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
}

int LogicMonitor::dispatch_to_scene(int scene_id, const Message* msg_proto)
{
    int gate_sid = this->first_gate_sid();
    JUDGE_RETURN(gate_sid >= 0, -1);

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;
    route_head.__inner_req = 1;
    route_head.__scene_id = scene_id;
    route_head.__recogn = recogn;

    ProtoHead head;
    head.__recogn = recogn;
    head.__scene_id = scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_aim_sid(const ProtoHead& src_head, const Message* msg_proto)
{
    int gate_sid = this->first_gate_sid();
    JUDGE_RETURN(gate_sid >= 0, -1);

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;
    route_head.__inner_req = 1;
    route_head.__scene_id = src_head.__src_scene_id;
    route_head.__line_id = src_head.__src_line_id;

    ProtoHead head;
    head.__recogn = recogn;
    head.__scene_id = src_head.__src_scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_scene_by_noplayer(BaseLogicPlayer *player, const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    InnerRouteHead route_head;
    route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;

    route_head.__recogn = recogn;
    route_head.__role_id = player->role_id();
    route_head.__scene_id = player->scene_id();
    route_head.__inner_req = 1;

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = player->role_id();
    head.__scene_id = player->scene_id();

    return this->dispatch_to_scene(player->gate_sid(), &route_head, &head, msg_proto);
}

int LogicMonitor::dispatch_to_all_map(const Message* msg_proto)
{
	const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
	for (GameConfig::ServerList::const_iterator iter = server_list.begin();
			iter != server_list.end(); iter++)
	{
		JUDGE_CONTINUE(iter->__server_type == SERVER_MAP);
#ifndef LOCAL_DEBUG
		JUDGE_CONTINUE(iter->__is_travel == false);
#endif
		JUDGE_CONTINUE(iter->__scene_list.empty() == false);

		int first_scene = *(iter->__scene_list.begin());
		this->dispatch_to_scene(first_scene, msg_proto);
	}

	return 0;
}

int LogicMonitor::dispatch_to_php(const int sid, const Message *proto_msg)
{
	uint32_t len = sizeof(sizeof(ProtoClientHead)), body_len = proto_msg->ByteSize();
	len += body_len;

	int recogn = type_name_to_recogn(proto_msg->GetTypeName());

	ProtoClientHead head;
	head.__recogn = recogn;
	head.__error = 0;

	Block_Buffer *pbuff = this->pop_block(sid);
	pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
	pbuff->write_int32(sid);
	pbuff->write_uint32(len);
	pbuff->copy((char *)&head, sizeof(ProtoClientHead));

	proto_msg->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
	pbuff->set_write_idx(pbuff->get_write_idx() + body_len);

	int ret = this->php_sender()->push_pool_block_with_len(pbuff);
	if (ret != 0)
		this->push_block(pbuff);
	return ret;
}

int LogicMonitor::process_inner_center_thread(Message &request, int64_t role_id)
{
    int recogn = type_name_to_recogn(request.GetTypeName());
    Message *msg = create_message(request.GetTypeName());
    JUDGE_RETURN(msg != NULL, -1);

    msg->CopyFrom(request);
    UnitMessage unit_msg;
    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;
    unit_msg.set_msg_proto(msg);
    if (this->center_unit()->push_request(unit_msg) != 0)
    {
        SAFE_DELETE(msg);
        return -1;
    }
    return 0;
}

int LogicMonitor::process_inner_back_thread(const int recogn, int64_t role_id)
{
    UnitMessage unit_msg;
    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;
    unit_msg.set_msg_proto(NULL);
    if (this->back_unit()->push_request(unit_msg) != 0)
    {
        return -1;
    }
    return 0;
}

int LogicMonitor::process_inner_back_thread(Message &request, int64_t role_id)
{
    int recogn = type_name_to_recogn(request.GetTypeName());
    Message *msg = create_message(request.GetTypeName());
    if (msg == 0)
        return -1;

    msg->CopyFrom(request);
    UnitMessage unit_msg;
    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;
    unit_msg.set_msg_proto(msg);
    if (this->back_unit()->push_request(unit_msg) != 0)
    {
        SAFE_DELETE(msg);
        return -1;
    }
    return 0;

}

int LogicMonitor::process_inner_logic_request(const int64_t role_id, Message &msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto.GetTypeName());
    Message *request = create_message(msg_proto.GetTypeName());

    JUDGE_RETURN(request != NULL, -1);
    request->CopyFrom(msg_proto);

    UnitMessage unit_msg;
    unit_msg.set_msg_proto(request);

    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;

    if (this->logic_unit()->push_request(unit_msg) != 0)
    {
    	if (request != 0)
    		delete request;
    }
    return 0;
}

int LogicMonitor::load_global_data(void)
{
    if (CACHED_INSTANCE->load_global_key(&(this->global_key_map_)) != 0)
        return -1;

    return 0;
}

const ServerItemSet& LogicMonitor::get_server_record()
{
	return server_record_set_;
}

int LogicMonitor::update_back_role_offline(void)
{
	CACHED_INSTANCE->update_back_role_offline();
	return 0;
}

LogicMonitor::LogicPlayerPool *LogicMonitor::player_pool(void)
{
    return this->player_pool_;
}

LogicMonitor::BoxRecordPool *LogicMonitor::box_record_pool(void)
{
    return this->box_record_pool_;
}

LogicMonitor::RpmRecomandInfoPool *LogicMonitor::rpm_recomand_info_pool()
{
	return this->rpm_recomand_info_pool_;
}

LogicMonitor::BackBrocastRecPool *LogicMonitor::backstage_brocast_record_pool(void)
{
	return this->backstage_brocast_record_pool_;
}

LogicMonitor::RechargeRankItemPool *LogicMonitor::recharge_rank_item_pool(void)
{
	return this->recharge_rank_item_pool_;
}

LogicMonitor::WeddingDetailPool *LogicMonitor::wedding_detail_pool(void)
{
	return this->wedding_detail_pool_;
}

LogicMonitor::JYBackActivityItemPool *LogicMonitor::jyback_activity_item_pool(void)
{
    return this->jyback_activity_item_pool_;
}

int LogicMonitor::request_login_player(const int gate_sid, const int64_t role_id, Message *msg)
{
    Proto30100101 *request = dynamic_cast<Proto30100101 *>(msg);
    JUDGE_RETURN(request != NULL, -1);

    LogicPlayer *player = this->player_pool()->pop();
    player->set_role_id_for_load(role_id);
    player->set_session(request->session_info().session());
    player->set_uc_sid(request->uc_sid());
    player->role_detail().__scene_id = request->scene_id();
    player->set_client_ip(request->client_ip());
    player->set_client_mac(request->client_mac());

    if (TRANSACTION_MONITOR->request_mongo_transaction(role_id, TRANS_LOAD_LOGIC_PLAYER, 
                DB_LOGIC_PLAYER, player, LOGIC_MONITOR->player_pool(),
                LOGIC_MONITOR->logic_unit(), gate_sid, INNER_LOGIC_LOGIN) != 0)
    {
        this->player_pool()->push(player);
        //this->dispatch_to_client(gate_sid, role_id, RETURN_START_GAME, ERROR_SERVER_INNER);
        return -1;
    }
    return 0;
}

int LogicMonitor::after_load_player(Transaction *transaction)
{
    JUDGE_RETURN(transaction != NULL, -1);

    if (transaction->detail().__error != 0)
    {
        //this->dispatch_to_client(transaction->sid(), transaction->role_id(),
        //        RETURN_START_GAME, transaction->detail().__error);
        transaction->rollback();
        return -1;
    }

    int gate_sid = transaction->sid();
    TransactionData *trans_data = transaction->fetch_data(DB_LOGIC_PLAYER);
    JUDGE_RETURN(trans_data != NULL, -1);

    LogicPlayer *player = trans_data->__data.__logic_player;

    trans_data->reset();
    transaction->summit();

    if (player->sign_in(gate_sid) != 0)
    {
        this->player_pool()->push(player);
        return -1;
    }

//    LogicRoleDetail &detail = player->role_detail();
//    if (!(is_same_day(Time_Value(detail.__login_tick), Time_Value(::time(NULL)))))
//    {
//    	detail.__login_day ++;
//    	detail.__login_tick = ::time(NULL);
//    }

    MSG_USER("logic player login %ld %s %d", player->role_id(),
    		player->name(), gate_sid);

    return 0;
}

int LogicMonitor::bind_player(const int64_t role_id, LogicPlayer *player)
{
    return this->player_map_.bind(role_id, player);
}

int LogicMonitor::unbind_player(const int64_t role_id)
{
    return this->player_map_.unbind(role_id);
}

int LogicMonitor::find_player(const int64_t role_id, LogicPlayer *&player)
{
    return this->player_map_.find(role_id, player);
}

int LogicMonitor::bind_player(const std::string &name, LogicPlayer *player)
{
    return this->name_player_map_.bind(name, player);
}

int LogicMonitor::unbind_player(const std::string &name)
{
    return this->name_player_map_.unbind(name);
}

int LogicMonitor::find_player(const std::string &name, LogicPlayer *&player)
{
    return this->name_player_map_.find(name, player);
}

int LogicMonitor::find_player_name(const std::string &name, StringVec &player_set)
{
	for (NamePlayerMap::iterator iter = this->name_player_map_.begin();
			iter != this->name_player_map_.end(); ++iter)
	{
		std::string player_name = iter->first;
		if (player_name.find(name) != std::string::npos)
		{
			player_set.push_back(player_name);
		}
	}
	return 0;
}

int LogicMonitor::find_global_value(const std::string &key, int64_t &value)
{
    return this->global_key_map_.find(key, value);
}

int LogicMonitor::bind_global_value(const std::string &key, int64_t &value)
{
    return this->global_key_map_.rebind(key, value);
}

int LogicMonitor::fetch_global_value(const std::string &key, int64_t &value)
{
    if (this->global_key_map_.find(key, value) != 0)
        return -1;

    this->global_key_map_.rebind(key, ++value);
    MMOGlobal::save_global_key_to_mongo_unit(key, (int)value);
    return 0;
}

int LogicMonitor::logout_all_player(void)
{
    MSG_USER("logout all player");

    std::vector<LogicPlayer *> player_list;

    LogicPlayer *player = 0;
    for (PlayerMap::iterator iter = this->player_map_.begin();
            iter != this->player_map_.end(); ++iter)
    {
        player_list.push_back(iter->second);
    }

    for (std::vector<LogicPlayer *>::iterator iter = player_list.begin();
            iter != player_list.end(); ++iter)
    {
        player = *iter;
        MSG_USER("stop logic player logout %s %ld", player->name(), player->role_id());

        player->sign_out();
    }
    return 0;
}

int LogicMonitor::notify_all_player(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100104*, request, -1);

	for (PlayerMap::const_iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		this->dispatch_to_client(player, request->recogn(), request->msg());
	}

	return 0;
}

int LogicMonitor::process_init_scene(const int scene_id, const int config_index, const int space_id)
{
    return 0;
}

int LogicMonitor::db_load_mode_begin(int trans_recogn, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(trans_recogn, this->logic_unit(), role_id);
}

int LogicMonitor::db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(shop_mode, this->logic_unit(), role_id);
}

int LogicMonitor::db_load_mode_done(Transaction* trans)
{
	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
	JUDGE_RETURN(trans_data != NULL, -1);

	DBShopMode* shop_mode = trans_data->__data.__shop_mode;
	switch(shop_mode->recogn_)
	{
	case TRANS_LOAD_SHOP_INFO:
	{
		SHOP_MONITOR->after_load_game_shop(shop_mode);
		break;
	}
	case TRANS_LOAD_GAME_NOTICE:
	{
		GAME_NOTICE_SYS->after_load_notice(shop_mode);
		break;
	}
	case TRANS_LOAD_LUCKY_WHEEL:
	{
		if (shop_mode->sub_value_ == 1)
			LUCKY_WHEEL_SYSTEM->after_load_activity_done(shop_mode);
		else
			DAILY_ACT_SYSTEM->after_load_activity_done(shop_mode);
		break;
	}
	case TRANS_LOAD_FESTIVAL_TIME:
	{
		FEST_ACTIVITY_SYS->request_festival_time_done(shop_mode);
		break;
	}
	case TRANS_LOAD_PLAYER_FOR_MALL_DONATION:
	{
		LogicPlayer* player = NULL;
		JUDGE_RETURN(this->find_player(trans->role_id(), player) == 0, -1);
		player->after_load_for_mall_donate(shop_mode);
		break;
	}
	case TRANS_LOAD_MAY_ACTIVITY:
	{
		MAY_ACTIVITY_SYS->after_load_activity_done(shop_mode);
		break;
	}
	default:
	{
		break;
	}
	}

    return trans->summit();
}

int LogicMonitor::back_stage_push_system_announce(const std::string &content,
		int type /*=BANNER_BROCAST_JUST_TOP*/)
{
	Proto30200125 brocast_info;
	brocast_info.set_content(content);
	brocast_info.set_type(type);
	return this->dispatch_to_chat(SCENE_CHAT, &brocast_info);
}

int LogicMonitor::notify_all_player(const LongMap& player_set, int recogn, Message* msg)
{
	for (LongMap::const_iterator iter = player_set.begin();
			iter != player_set.end(); ++iter)
	{
		LogicPlayer* player = NULL;
		JUDGE_CONTINUE(this->find_player(iter->first, player) == 0);

		player->respond_to_client(recogn, msg);
	}

	return 0;
}

int LogicMonitor::notify_all_player(int recogn, Message* msg)
{
	for (PlayerMap::const_iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(recogn, msg);
	}

	return 0;
}

LogicMonitor::LoigcMonitorTimer::LoigcMonitorTimer(void) : monitor_(0)
{ /*NULL*/ }

LogicMonitor::LoigcMonitorTimer::~LoigcMonitorTimer(void)
{ /*NULL*/ }

int LogicMonitor::LoigcMonitorTimer::type(void)
{
    return GTT_LOGIC_MONITOR;
}

int LogicMonitor::LoigcMonitorTimer::handle_timeout(const Time_Value &nowtime)
{
    return 0;
}

int LogicMonitor::inner_notify_player_assist_event(Int64 role_id, int event_id, int event_value)
{
	/* 系统提示 */
	LogicPlayer *player = NULL;
	JUDGE_RETURN(LOGIC_MONITOR->find_player(role_id, player) == 0, -1);

	Proto31401710 inner_event;
	inner_event.set_event_id(event_id);
	inner_event.set_event_value(event_value);
	return LOGIC_MONITOR->dispatch_to_scene(player, &inner_event);
}

int LogicMonitor::parse_msg_to_all_player(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30101301*, request, msg, -1);

	std::string proto_name = "Proto" + GameCommon::int2str(request->proto_id());
	Message *respond = create_message(proto_name);
	int ret = 0;
	if (respond == NULL || !respond->ParseFromString(request->proto_bytes()))
	{
		ret = -1;
		goto end;
	}

	for (PlayerMap::iterator iter = player_map_.begin();
			iter != player_map_.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		player->respond_to_client(request->proto_id(), respond);
	}

end:
	SAFE_DELETE(respond);
	return ret;
}

int LogicMonitor::parse_msg_to_one_player(Message *msg)
{
	DYNAMIC_CAST_RETURN(Proto30101302*, request, msg, -1);

//	MSG_DEBUG("role %ld, proto_id: %d", request->role_id(), request->proto_id());
	int ret = 0;
	LogicPlayer *player = NULL;

	this->find_player(request->role_id(), player);
	if(!player)
	{
		MSG_USER("error!!! no logic player %ld", request->role_id());
		return -1;
	}

	if(request->has_proto_bytes())
	{
		std::string proto_name = "Proto" + GameCommon::int2str(request->proto_id());
		Message *respond = create_message(proto_name);
		if (respond && respond->ParseFromString(request->proto_bytes()))
		{
			ret += player->respond_to_client(request->proto_id(), respond);
		}
		SAFE_DELETE(respond);
	}

	if(request->has_error_code())
	{
		ret += player->respond_to_client_error(request->proto_id(), request->error_code());
	}

	return ret;
}

void LogicMonitor::reset_all_player_everyday(int test)
{
	PlayerMap &player_map = this->player_map();
	for(PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		LogicPlayer* player = iter->second;
		JUDGE_CONTINUE(player != NULL);

		player->reset_every_day(test);
		player->reset_every_week();
		player->notify_server_activity_info();
	}

}

void LogicMonitor::update_boss_lastest_attacker_name(const Int64 role_id, const std::string &name)
{
	for (BossMap::iterator boss_iter = this->boss_map_->begin(); boss_iter != this->boss_map_->end(); ++boss_iter)
	{
		BossDetail &detail = boss_iter->second;
		if (detail.__last_award_role_id == role_id)
			detail.__last_award_role_name = name;
	}
}

int LogicMonitor::notify_player_from_map(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100106*, request, -1);

	LogicPlayer* player = LogicPlayer::find_player(request->role_id());
	JUDGE_RETURN(player != NULL, -1);

	int recogn = request->recogn();
	return player->respond_to_client(recogn);
}

int LogicMonitor::send_tmarena_rank_reward(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30100232*, request, -1);

	int rank = request->rank();
	int mail_id = CONFIG_INSTANCE->const_set("tmarena_mail");

	GameConfig::ConfigMap& rank_map = CONFIG_INSTANCE->tmarena_rank_map();
	for (GameConfig::ConfigMap::iterator iter = rank_map.begin();
			iter != rank_map.end(); ++iter)
	{
		const Json::Value& conf = *(iter->second);
		JUDGE_CONTINUE(rank >= conf["rang"][0u].asInt() && rank <= conf["rang"][1u].asInt());

		MailInformation* mail_info = GameCommon::create_sys_mail(mail_id);
		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), rank);

		mail_info->add_goods(conf["reward"].asInt());
		GameCommon::request_save_mail_content(request->role(), mail_info);
	}

	return 0;
}

int LogicMonitor::limit_team_scene(int scene_id)
{
	return this->limit_team_scene_set_.count(scene_id > 0);
}

int LogicMonitor::update_limit_team_scene_set()
{
	return 0;
}

LongMap& LogicMonitor::fetch_qq_49()
{
	return this->qq_49_;
}

int LogicMonitor::save_server_record(SpecialBoxItem& slot_info, ServerItemRecord &item_info)
{
	if (slot_info.server_record_ == true)
	{
		server_record_set_.push_back(item_info);
		// 全服记录排序
		std::sort(server_record_set_.begin(), server_record_set_.end(), GameCommon::comp_by_time_desc);
	}

	if(slot_info.is_shout_ == true)
	{
		//todo
	}
		//全服播报
//		if (slot_info.is_shout_ == true)
//			this->activity_brocast(act_detail->shout_id_, item_info);

//		if(act_detail.treasure_shout_id_ > 0)
//			GameCommon::announce(act_detail->treasure_shout_id_);
	return 0;
}
