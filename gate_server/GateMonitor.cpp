/*
 * GateMonitor.cpp
 *
 * Created on: 2013-04-12 17:32
 *     Author: lyz
 */

#include "PubStruct.h"
#include "GateMonitor.h"
#include "SessionManager.h"
#include "MongoConnector.h"
#include "GameField.h"
#include "BackField.h"
#include "Transaction.h"
#include "TransactionMonitor.h"
#include "ProtoDefine.h"
#include "MMORole.h"
#include "MongoDataMap.h"
#include "MongoData.h"
#include  <string.h>
#include <mongo/client/dbclient.h>

GateMonitor::GateOneSecTimer::GateOneSecTimer(void)
{
}

GateMonitor::GateOneSecTimer::~GateOneSecTimer(void)
{
}

int GateMonitor::GateOneSecTimer::type(void)
{
	return GTT_GATE_PLAYER;
}

int GateMonitor::GateOneSecTimer::handle_timeout(const Time_Value &nowtime)
{
    GATE_MONITOR->check_and_connect_svc();
    return 0;
}

GateMonitor::GateTenSecTimer::GateTenSecTimer(void)
{ /*NULL*/ }

GateMonitor::GateTenSecTimer::~GateTenSecTimer(void)
{ /*NULL*/ }

int GateMonitor::GateTenSecTimer::type(void)
{
	return GTT_GATE_TRANS;
}

int GateMonitor::GateTenSecTimer::handle_timeout(const Time_Value &nowtime)
{
	FLOW_INSTANCE->request_load_flow_detail();
    GATE_MONITOR->relogin_logic_for_all_player();
    GATE_MONITOR->check_and_fetch_travel_area();
    GATE_MONITOR->request_load_combine_server(nowtime.sec());

	return 0;
}

int GateMonitor::init_game_timer_handler(void)
{
    const double inter_tick_list[] = {
        GATE_PLAYER_INTERVAL,
        GATE_TRANSACTION_INTERVAL
    };
    const int timer_amount = sizeof(inter_tick_list) / sizeof(double);

    this->timer_handler_list_.resize(timer_amount);
    POOL_MONITOR->init_game_timer_list(timer_amount);

    double inter_sec = 0.0, inter_usec = 0.0;
    int index = 0;
    for (int i = GTT_GATE_TYPE_BEG + 1; i < GTT_GATE_TYPE_END; ++i)
    {
        index = i - GTT_GATE_TYPE_BEG - 1;
        inter_usec = modf(inter_tick_list[index], &inter_sec);
        Time_Value interval(inter_sec, inter_usec * 1000000);
        this->timer_handler_list_[index].set_type(i);
        this->timer_handler_list_[index].set_interval(interval);
    }
    return 0;
}

int GateMonitor::start_game_timer_handler(void)
{
	for (TimerHandlerList::iterator iter = this->timer_handler_list_.begin();
			iter != this->timer_handler_list_.end(); ++iter)
	{
		iter->schedule_timer(iter->interval());
	}

	//服务器启动时加载帮派领地(boss)场景
//	this->dispatch_to_logic(INNER_LOGIC_SYNC_LEAGUE_BOSS);

	return 0;
}

GateMonitor::GateMonitor(void) :
	broad_unit_(NULL),
	broad_unit_cnt_(0),
    global_key_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    player_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    player_sid_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    sid_player_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    sid_account_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    account_player_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET)),
    account_login_tick_map_(get_hash_table_size(GateMonitor::MAP_OBJECT_BUCKET))
{
	this->session_manager_ = 0;
	this->auth_sid_ = 0;
	this->connect_unit_ = new ConnectUnit();
}

int GateMonitor::init(const int config_index)
{
    Time_Value client_send_timeout(0, 1000);
    Time_Value inner_send_timeout(0, 100 * 1000);
    {
    	Time_Value recv_timeout(15000, 0);
#ifndef LOCAL_DEBUG
    	if (CONFIG_INSTANCE->tiny("heart").asInt() == 1)
        	recv_timeout.set(300, 0);
#endif
        this->client_receiver_.set(&recv_timeout, 1024);
    }
    {
        Time_Value send_timeout(0, 1000);
        this->connect_sender_.set(send_timeout);
    }
    this->client_monitor_.set_svc_max_recv_size(200);
    this->client_monitor_.set_svc_max_pack_size(60 * 1024);
    this->connect_monitor_.set_svc_max_list_size(20000);
    this->connect_monitor_.set_svc_max_pack_size(1024 * 1024 * 3);

    this->client_packer_.monitor(&(this->client_monitor_));
    this->client_monitor_.packer(&(this->client_packer_));

    this->inner_packer_.monitor(&(this->inner_monitor_));
    this->inner_monitor_.packer(&(this->inner_packer_));

    this->connect_receiver_.monitor(&(this->connect_monitor_));
    this->connect_monitor_.receiver(&(this->connect_receiver_));
    this->connect_packer_.monitor(&(this->connect_monitor_));
    this->connect_monitor_.packer(&(this->connect_packer_));

    this->auth_sid_ = 0;
    this->session_manager_ = SESSION_MANAGER;
    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][config_index];

    int sender_thr = 1;
    if (server_json.isMember("send_thr") == true)
    {
        sender_thr = server_json["send_thr"].asInt();
    }

    int broad_thr = 3;
    if (server_json.isMember("broad_thr") == true)
    {
    	broad_thr = server_json["broad_thr"].asInt();
    }

    //合服
    if (CONFIG_INSTANCE->do_combine_server() == true)
    {
    	sender_thr = std::max<int>(sender_thr / 2, 1);
    	broad_thr = std::max<int>(broad_thr / 2, 3);
    }

    this->broad_unit_cnt_ = broad_thr;
    this->broad_unit_ = new BroadUnit[broad_thr];

    return SUPPER::init(client_send_timeout, inner_send_timeout, sender_thr);
}

int GateMonitor::start(void)
{
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];

    int limit_accept = servers_json[this->server_config_index_]["limit_accept"].asInt();
    this->client_monitor_.acceptor()->set_limit_connect(limit_accept);

    this->relogin_all_tick_ = Time_Value::zero;
    this->reconnect_tick_ 	= Time_Value::zero;

    SUPPER::start();

    for (SceneSet::iterator iter = this->connect_scene_set_.begin();
            iter != this->connect_scene_set_.end(); ++iter)
    {
    	SceneToIndexMap::KeyValueMap *kv_map = 0;
    	JUDGE_CONTINUE(this->scene_to_index_map_.find_object_map(*iter, kv_map) == 0);

    	for (SceneToIndexMap::KeyValueMap::iterator kv_iter = kv_map->begin();
    			kv_iter != kv_map->end(); ++kv_iter)
    	{
    		this->reconnect_set_.insert(kv_iter->second);
    	}
    }

    this->start_reconnect_tick(5);
    this->init_travel_map_index();
//    this->broad_unit()->thr_create();
    this->connect_unit()->thr_create();

    for (int i = 0; i < this->broad_unit_cnt_; ++i)
    {
    	this->broad_unit_[i].thr_create();
    }

    MSG_USER("start gate server");
    return 0;
}

int GateMonitor::start_game(void)
{
    if (this->load_global_data() != 0)
    {
        MSG_USER("ERROR load global data");
        ::exit(-1);
    }

    FLOW_INSTANCE->load_flow_detail_when_init();

    this->set_combine_server_check_time(::time(NULL));
	this->tensec_timer_.schedule_timer(Time_Value(9, 900000));
	this->onesec_timer_.schedule_timer(1);

    return 0;
}

int GateMonitor::stop(void)
{
    SUPPER::stop();

    for (int i = 0; i < this->broad_unit_cnt_; ++i)
    {
    	this->broad_unit_[i].stop_wait();
    }
//    this->broad_unit()->stop_wait();
    this->connect_unit()->stop_wait();
    return 0;
}

void GateMonitor::fina(void)
{
    this->connect_receiver_.fini();

    this->global_key_map_.unbind_all();
    this->session_manager_->clear();
//    SAFE_DELETE(this->session_manager_);

    this->player_map_.unbind_all();
    this->sid_player_map_.unbind_all();
    this->player_sid_map_.unbind_all();
    this->sid_account_map_.unbind_all();
    this->timer_handler_list_.clear();
    this->player_pool_.clear();

    SUPPER::fina();
}

int GateMonitor::logout_all_player(void)
{
	MSG_USER("Stop Server...");

    std::vector<GatePlayer *> player_list;
    for (PlayerMap::iterator iter = this->player_map_.begin();
            iter != this->player_map_.end(); ++iter)
    {
        player_list.push_back(iter->second);
    }

    for (std::vector<GatePlayer *>::iterator iter = player_list.begin();
            iter != player_list.end(); ++iter)
    {
    	GatePlayer* player = *iter;
        player->stop_game(GateMonitor::STOP_SERVER);
    }

    return 0;
}

BaseUnit *GateMonitor::logic_unit(void)
{
    return &(this->logic_unit_);
}

BaseUnit *GateMonitor::broad_unit(const int index)
{
	int idx = index % this->broad_unit_cnt_;
    return this->broad_unit_ + idx;
}

BaseUnit *GateMonitor::connect_unit(void)
{
    return this->connect_unit_;
}

SessionManager *GateMonitor::session_manager(void)
{
    return this->session_manager_;
}

Block_Buffer *GateMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int GateMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

GateMonitor::UnitMessagePool *GateMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

GateMonitor::GatePlayerPool *GateMonitor::player_pool(void)
{
    return &(this->player_pool_);
}

int GateMonitor::process_init_scene(const int scene_id, const int config_index, const int space_id)
{
    return 0;
}

int GateMonitor::load_global_data(void)
{
    if (CACHED_INSTANCE->load_global_key(&(this->global_key_map_)) != 0)
        return -1;
    return 0;
}

// 启动时跨服不连接，防止网络不稳定导致区服的网关卡住
int GateMonitor::check_all_connect_set(void)
{
	for (SceneSet::iterator iter = this->connect_scene_set_.begin();
			iter != this->connect_scene_set_.end(); ++iter)
	{
		SceneToIndexMap::KeyValueMap *kv_map = 0;
		JUDGE_CONTINUE(this->scene_to_index_map_.find_object_map(*iter, kv_map) == 0);

		int total_size = std::max<int>(1, kv_map->size());
		for (int i = 0; i < total_size; ++i)
		{
			JUDGE_CONTINUE(this->connect_scene_to_sid(*iter, i) < 0);

			if (GameCommon::is_travel_scene(*iter) == false)
			{
				this->connect_server(*iter, i);
			}
			else
			{
				int cfg_index = 0;
				JUDGE_CONTINUE(this->scene_to_index_map_.find(*iter, i, cfg_index) == 0);
				this->add_reconnect_index(cfg_index);
			}
		}
	}
	return 0;
}

int GateMonitor::fetch_sid_of_scene(const int scene_id, const bool for_log, const int64_t src_line_id)
{
	SceneToIndexMap::KeyValueMap *kv_map = 0;
	JUDGE_RETURN(this->scene_to_index_map_.find_object_map(scene_id, kv_map) == 0, -1);

	int line_id = 0;
	if (kv_map->size() > 0)
	{
		line_id = src_line_id % kv_map->size();
	}

	int sid = this->connect_scene_to_sid(scene_id, line_id);
	JUDGE_RETURN(sid < 0, sid);

	int cfg_index = 0;
	JUDGE_RETURN(this->scene_to_index_map_.find(scene_id, line_id, cfg_index) == 0, -1);

	this->add_reconnect_index(cfg_index);
	return -1;

}

int GateMonitor::make_up_client_block(Block_Buffer *buff, const int recogn, const int error, Block_Buffer *body)
{
    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = error;

    uint32_t len = sizeof(ProtoClientHead);
    if (body != 0)
        len += body->readable_bytes();

    buff->ensure_writable_bytes(len + sizeof(int) * 4);
    buff->write_uint32(len);
    buff->copy((char *)&head, sizeof(ProtoClientHead));
    if (body != 0)
        buff->copy(body);
    return 0;
}

int GateMonitor::dispatch_to_scene(ProtoHead *head, Block_Buffer *body)
{
    int sid = this->fetch_sid_of_scene(head->__scene_id, false, head->__src_line_id);
    return this->dispatch_to_scene_by_sid(sid, head, body);
}

int GateMonitor::dispatch_to_scene(ProtoHead *head, const Message *msg_proto)
{
	int sid = this->fetch_sid_of_scene(head->__scene_id, false, head->__src_line_id);
	return this->dispatch_to_scene_by_sid(sid, head, msg_proto);
}

int GateMonitor::dispatch_to_scene_by_sid(int sid, ProtoHead *head, Block_Buffer *body)
{
    if (sid < 0)
    {
    	MSG_USER("1 ERROR server/special**.json field[scene] no scene_id %d, %d",
    			head->__scene_id, head->__recogn);
        return -1;
    }

    uint32_t len = sizeof(ProtoHead);
    if (body != 0)
    {
        len += body->readable_bytes();
    }

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(int) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)head, sizeof(ProtoHead));
    if (body != 0)
    {
        pbuff->copy(body);
    }

    int ret = this->connect_sender()->push_pool_block_with_len(pbuff);
    if (ret != 0)
    {
        this->push_block(pbuff);
    }
    return ret;
}

int GateMonitor::dispatch_to_scene_by_sid(int sid, ProtoHead *head, const Message *msg_proto)
{
    if (sid < 0)
    {
    	MSG_USER("2 ERROR server/special**.json field[scene] no scene_id %d, %d",
    			head->__scene_id, head->__recogn);
        return -1;
    }

    uint32_t len = sizeof(ProtoHead), byte_size = 0;
    if (msg_proto != 0)
        byte_size = msg_proto->ByteSize();

    len += byte_size;
    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)head, sizeof(ProtoHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), byte_size);
        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
    }

    int ret = this->connect_sender()->push_pool_block_with_len(pbuff);
    if (ret != 0)
        this->push_block(pbuff);
    return ret;
}

int GateMonitor::dispatch_to_server(UnitMessage *unit_msg)
{
    ProtoHead &proto_head = unit_msg->__msg_head;

    switch(unit_msg->__route_head.__broad_type)
    {
    case BT_DIRECT_TARGET_SCENE:
    case BT_DIRECT_TARGET_SCENE_BACK:
    {
    	GatePlayer* player = NULL;
    	JUDGE_RETURN(this->find_player(proto_head.__role_id, player) == 0, -1);

    	if (unit_msg->need_adjust_player_scene() == true)
    	{
			proto_head.__recogn = unit_msg->__route_head.__recogn;
			proto_head.__scene_id = player->detail().__scene_id;
    	}

        proto_head.__src_line_id = this->fetch_line_id_by_scene(proto_head.__role_id, proto_head.__scene_id);
        if (proto_head.__scene_id > 0 && proto_head.__scene_id != SCENE_GATE)
        {
            return this->dispatch_to_scene(&proto_head, unit_msg->data_buff());
        }

    	break;
    }
    case BT_DIRECT_TARGET_LINE_SCENE:
    {
        if (proto_head.__scene_id > 0 && proto_head.__scene_id != SCENE_GATE)
        {
            return this->dispatch_to_scene(&proto_head, unit_msg->data_buff());
        }
        break;
    }
    case BT_DIRECT_CLIENT:
    {
        int64_t role_id = unit_msg->__route_head.__role_id;
        GatePlayer *player = 0;
        if (this->find_player(role_id, player) != 0)
        {
            MSG_USER("ERROR no role %ld", role_id);
            return -1;
        }

        int sid = player->client_sid();

        ProtoClientHead client_head;
        client_head.__recogn = unit_msg->__msg_head.__recogn;
        client_head.__error = unit_msg->__msg_head.__error;

        Block_Buffer *buff = unit_msg->data_buff();
        int data_len = buff->readable_bytes(), org_write_idx = buff->get_write_idx(), total_len = 0;
        total_len = data_len + sizeof(ProtoClientHead);
        buff->set_read_idx(buff->get_read_idx() - sizeof(ProtoClientHead) - sizeof(int32_t) * 2);
        buff->set_write_idx(buff->get_read_idx());
        buff->write_int32(sid);
        buff->write_int32(total_len);
        buff->copy(&client_head, sizeof(ProtoClientHead));
        buff->set_write_idx(org_write_idx);
        int ret = this->client_sender(sid)->push_pool_block_with_len(buff);
        if (ret == 0)
            unit_msg->reset();

        return ret;
    }
    }
    return -1;
}

int GateMonitor::dispatch_to_client(const int sid, const Message *msg_proto)
{ 
    uint32_t len = sizeof(sizeof(ProtoClientHead)), body_len = msg_proto->ByteSize();
    len += body_len;

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    
    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = 0;

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));

    msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
    pbuff->set_write_idx(pbuff->get_write_idx() + body_len);

    int ret = this->client_sender(sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
        this->push_block(pbuff);
    return ret;
}

int GateMonitor::dispatch_to_client(const int sid, const int org_recogn, const int error, const Message *msg_proto)
{
    uint32_t len = sizeof(sizeof(ProtoClientHead)), body_len = 0;
    int recogn = org_recogn;

    if (msg_proto != 0)
    {
        recogn = type_name_to_recogn(msg_proto->GetTypeName());
        body_len = msg_proto->ByteSize();
        len  += body_len;
    }

    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = error;

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(int32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));

    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + body_len);
    }

    int ret = this->client_sender(sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
        this->push_block(pbuff);
    return ret;
}

int GateMonitor::dispatch_to_auth(const int auth_sid, const int recogn, Block_Buffer *body)
{
    Block_Buffer *pbuff = this->pop_block(auth_sid);
    pbuff->write_int32(auth_sid);
    int org_read_idx = pbuff->get_read_idx();
    pbuff->set_read_idx(pbuff->get_write_idx());

    this->make_up_client_head(pbuff, recogn);
    if (body != 0)
        pbuff->copy(body);
    this->update_block_length(pbuff);
    pbuff->set_read_idx(org_read_idx);

    int ret = this->inner_sender(auth_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
        this->push_block(pbuff, auth_sid);
    return ret;
}

int GateMonitor::dispatch_to_logic(const int recogn)
{
	ProtoHead head;
	head.__recogn = recogn;
	head.__scene_id = SCENE_LOGIC;
	return this->dispatch_to_scene(&head, (Message*)NULL);
}

int GateMonitor::set_role_scene_info(const int client_sid, int64_t &role_id, int &scene_id)
{
    GatePlayer *player = 0;
    if (this->find_player_by_sid(client_sid, player) != 0)
        return -1;

    role_id = player->role_id();
    int64_t split_id = player->detail().__space_id;
    scene_id = player->detail().__scene_id;
    if (split_id > 0)
        scene_id = this->scene_convert_to(player->detail().__scene_id, split_id);
    return 0;
}

int GateMonitor::fetch_line_id_by_scene(const Int64 role_id, const int scene_id)
{
	int line_id = 0;
	if (GameCommon::is_travel_scene(scene_id) == false)
	{
		SceneToIndexMap::KeyValueMap *kv_map = 0;
		if (this->scene_to_index_map_.find_object_map(scene_id, kv_map) != 0)
		{
			return line_id;
		}

		if (kv_map->size() > 0)
		{
			line_id = role_id % kv_map->size();
		}
	}
	else
	{
        this->travel_scene_line_map_.find(scene_id, line_id);
	}
	return line_id;
}

int GateMonitor::bind_player_by_role_id(const Int64 role_id, GatePlayer *player)
{
    return this->player_map_.bind(role_id, player);
}

int GateMonitor::unbind_player_by_role_id(const Int64 role_id)
{
    return this->player_map_.unbind(role_id);
}

int GateMonitor::find_player(const int64_t role_id, GatePlayer *&player)
{
    return this->player_map_.find(role_id, player);
}

int GateMonitor::bind_sid_by_role_id(const Int64 role_id, const int sid)
{
    return this->player_sid_map_.rebind(role_id, sid);
}

int GateMonitor::unbind_sid_by_role_id(const Int64 role_id)
{
    return this->player_sid_map_.unbind(role_id);
}

int GateMonitor::find_sid_by_role_id(const int64_t role_id)
{
    int sid = 0;
    if (this->player_sid_map_.find(role_id, sid) == 0)
        return sid;
    return -1;
}

int GateMonitor::bind_sid_player(const int sid, GatePlayer *player)
{
	return this->sid_player_map_.rebind(sid, player);
}

int GateMonitor::unbind_sid_player(const int sid)
{
	return this->sid_player_map_.unbind(sid);
}

int GateMonitor::find_player_by_sid(const int sid, GatePlayer *&player)
{
    return this->sid_player_map_.find(sid, player);
}

int GateMonitor::bind_sid_account(const int sid, std::string &account)
{
    return this->sid_account_map_.rebind(sid, account);
}

int GateMonitor::unbind_sid_account(const int sid)
{
    return this->sid_account_map_.unbind(sid);
}

int GateMonitor::find_sid_account(const int sid, std::string &account)
{
    return this->sid_account_map_.find(sid, account);
}

int GateMonitor::bind_account_sid(std::string &account, const int sid)
{
	this->account_sid_map_[account] = sid;
	return 0;
}

int GateMonitor::unbind_account_sid(std::string &account)
{
	this->account_sid_map_.erase(account);
	return 0;
}

int GateMonitor::find_account_sid(std::string &account)
{
	AccountSidMap::iterator iter = this->account_sid_map_.find(account);
	if (iter != this->account_sid_map_.end())
		return iter->second;
	return -1;
}

int GateMonitor::bind_account_player(const std::string &account, GatePlayer *player)
{
    return this->account_player_map_.rebind(account, player);
}

int GateMonitor::unbind_account_player(const std::string &account)
{
    return this->account_player_map_.unbind(account);
}

GatePlayer *GateMonitor::find_account_player(const std::string &account)
{
    GatePlayer *player = NULL;
    this->account_player_map_.find(account, player);
    return player;
}

int GateMonitor::sync_session(const int auth_sid, Block_Buffer *msg)
{
    std::string account, session, tick;
    int client_sid = 0, conf_index = 0;
    int org_read_idx = msg->get_read_idx();
    (*msg) >> account >> session >> tick >> client_sid >> conf_index;
    msg->set_read_idx(org_read_idx);

    SessionDetail *p_session = 0;
    SessionManager *manager = this->session_manager_;
    if (manager->find_account_session(account, p_session) != 0)
    {
        p_session = manager->session_pool()->pop();
        p_session->__account = account;
        if (manager->bind_account_session(account, p_session) != 0)
        {
            manager->session_pool()->push(p_session);
            return -1;
        }
    }

    p_session->__session = session;
    p_session->__timeout_tick = Time_Value::gettimeofday() + Time_Value(SESSION_TIMEOUT_TICK);
    p_session->__manager = this->session_manager();
    p_session->schedule_timer(SESSION_TIMEOUT_TICK);

    MSG_USER("gate sync session(%d) %s %s %s %d %d", auth_sid, account.c_str(),
    		session.c_str(), tick.c_str(), client_sid, conf_index);
    return this->dispatch_to_auth(auth_sid, INNER_SYNC_SEESION, msg);
}

int GateMonitor::fetch_role_amount(const int sid, Block_Buffer *msg)
{
    int gate_scene = this->gate_scene();
    int amount = this->player_map_.size();

    Block_Buffer body;
    body << gate_scene << amount;
    return this->dispatch_to_auth(sid, INNER_INIT_ROLEAMOUNT, &body);
}

int GateMonitor::force_player_logout(Block_Buffer *msg)
{
    string account;
    (*msg) >> account;

    IntStrPair pair;
    (*msg) >> pair.first >> pair.second;

    AccountSidMap::iterator iter = this->account_sid_map_.find(account);
    JUDGE_RETURN(iter != this->account_sid_map_.end(), 0);

    GatePlayer *player = 0;
    JUDGE_RETURN(this->find_player_by_sid(iter->second, player) == 0, 0);
    JUDGE_RETURN(account == player->account(), 0);

	return player->stop_game(pair.first, pair.second);
}

#ifndef NO_USE_PROTO
int GateMonitor::validate_gate_session(const int sid, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto10600101 *, request, -1);

    string account = request->account(),
        session = request->session(), 
        tick = request->tick(),
        agent = request->agent(),
        platform = request->platform(),
        net_type = request->net_type(),
        sys_version = request->sys_version(),
        sys_model = request->sys_model(),
        mac = request->mac(),
        c_sid = request->c_sid(),
        server_flag = request->server_flag();
    int market_code = request->market();

#else

int GateMonitor::validate_gate_session(const int sid, Block_Buffer *msg)
{
    std::string account, session, tick, agent, platform, net_type, sys_version, sys_model, mac, c_sid, server_flag;
    int market_code = 0;
    (*msg) >> account >> session >> tick >> agent >> platform >> net_type >> sys_version >> sys_model >> mac >> c_sid >> server_flag >> market_code;

#endif
    if (account.empty() == true || session.empty() == true || tick.empty() == true)
    {
    	MSG_USER("ERROR gate arg %d %s %s %s", sid, account.c_str(), session.c_str(), tick.c_str());
        return this->dispatch_to_client(sid, RETURN_GATE_SESSION, ERROR_INVALID_PARAM);
    }

    MSG_USER("before correct gate session %d %s %s %s agent(%s) platform(%s) uc_sid(%s) %s %s %s %s flag(%s) %d",
    		sid, account.c_str(), session.c_str(), tick.c_str(), agent.c_str(), platform.c_str(),
    		c_sid.c_str(), net_type.c_str(), sys_version.c_str(), sys_model.c_str(), mac.c_str(),
    		server_flag.c_str(), market_code);

    std::string account_agent = this->get_agent_from_account(account);
    this->correct_agent_market(account_agent, agent, market_code);
    if (agent.empty())
    {
    	agent = account_agent;
    	int cfg_agent_code = CONFIG_INSTANCE->agent_code(agent);
		if (market_code <= 0)
			market_code = cfg_agent_code * 10000 + 1;
    }

    MSG_USER("after correct gate session %d %s %s %s agent(%s) platform(%s) uc_sid(%s) %s %s %s %s flag(%s) %d",
    		sid, account.c_str(), session.c_str(), tick.c_str(),
    		agent.c_str(), platform.c_str(), c_sid.c_str(), net_type.c_str(), sys_version.c_str(),
    		sys_model.c_str(), mac.c_str(), server_flag.c_str(), market_code);

    std::string client_address;
    int client_port = 0;
    Svc *svc = 0;
    if (this->client_monitor_.find_service(sid, svc) == 0)
    {
        svc->get_peer_addr(client_address, client_port);
    }

    SessionDetail *p_session = 0;
    if (this->session_manager_->find_account_session(account, p_session) != 0)
    {
        MSG_USER("ERROR no account session %s %s %s %s:%d",
                account.c_str(), session.c_str(), tick.c_str(), client_address.c_str(), client_port);
        return this->dispatch_to_client(sid, RETURN_GATE_SESSION, ERROR_SESSION_ILLEGAL);
    }

    Time_Value nowtime = Time_Value::gettimeofday();
    if (/*p_session->__timeout_tick < nowtime || */p_session->__session != session)
    {
        MSG_USER("ERROR session %s %s %s:%d timeout: %d.%06d nowtime %d.%06d",
                account.c_str(), tick.c_str(), client_address.c_str(), client_port,
                p_session->__timeout_tick.sec(), p_session->__timeout_tick.usec(),
                nowtime.sec(), nowtime.usec());
        MSG_USER("auth_flag: %s", p_session->__session.c_str());
        MSG_USER("gate_flag: %s", session.c_str());
        return this->dispatch_to_client(sid, RETURN_GATE_SESSION, ERROR_SESSION_ILLEGAL);
    }

    // 检查登录频率
    int64_t op_tick = 0;
    if (this->account_login_tick_map_.find(account, op_tick) == 0 && op_tick > nowtime.sec())
    {
        MSG_USER("ERROR login too frequently %s %ld %ld", account.c_str(), op_tick, nowtime.sec());
        return 0;
    }

    this->account_login_tick_map_[account] = nowtime.sec() + 5;

    GatePlayer *tmp_player = 0;
    if (this->find_player_by_sid(sid, tmp_player) == 0)
    {
    	// 对于重复发网关验证请求的错误
    	if (account == tmp_player->account())
    	{
    		MSG_USER("ERROR gate validate session repeated %d %s", sid, account.c_str());
    		tmp_player->stop_game(GateMonitor::STOP_SAME_SID);
    	}
    	else
    	{
    		MSG_USER("ERROR gate svc has registerd for %s [%d %s]", tmp_player->account(), sid, account.c_str());
    		return -1;
    	}
    }

    tmp_player = this->find_account_player(account);
    if (tmp_player != NULL)
    {
    	MSG_USER("ERROR gate same account login %d %s", sid, account.c_str());
        tmp_player->stop_game(GateMonitor::SAME_ACCCCOUNT);
    }

    GatePlayer *player = this->player_pool()->pop();
    GateRoleDetail& detail = player->detail();
    detail.__server_flag = server_flag;
    detail.__net_type = net_type;
    detail.__sys_version = sys_version;
    detail.__sys_model = sys_model;
    detail.__mac = mac;
    detail.__uc_sid = c_sid;

    player->set_account(account.c_str());
    player->set_client_sid(sid);
    player->set_agent(agent.c_str());
    player->set_client_ip(client_address);
    player->set_client_port(client_port);
    player->set_market_code(market_code);

//    p_session->__net_type = net_type;
//    p_session->__sys_version = sys_version;
//    p_session->__sys_model = sys_model;
//    p_session->__mac = mac;
//    p_session->__uc_sid = c_sid;
//    p_session->__market_code = market_code;

    this->bind_sid_player(sid, player);
    this->bind_sid_account(sid, account);
    this->bind_account_sid(account, sid);
    this->bind_account_player(account, player);
    player->set_is_loading_mongo(true);

    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_LOAD_GATE_PLAYER,
                DB_GATE_PLAYER, player, 0, GATE_MONITOR->logic_unit(), sid) != 0)
    {
        return this->dispatch_to_client(sid, RETURN_GATE_SESSION, ERROR_SERVER_INNER);
    }
    return 0;
}

#define CHECK_GATE_TRANSACTION_ERROR(SID, P_TRANS, RECOGN) \
{ \
    if (P_TRANS == 0) return -1; \
    if (P_TRANS->detail().__error != 0) { \
        this->dispatch_to_client(SID, RECOGN, P_TRANS->detail().__error); \
        P_TRANS->rollback(); \
        return -1; \
    } \
}

int GateMonitor::after_load_player(Transaction *transaction)
{
    JUDGE_RETURN(transaction != NULL, -1);

    int client_sid = transaction->sid();
    Proto50600101 respond;

    const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
    for (GameConfig::ServerList::const_iterator iter = server_list.begin();
            iter != server_list.end(); ++iter)
    {
        const ServerDetail &detail = *iter;
        JUDGE_CONTINUE(detail.__server_type == SERVER_CHAT);
        JUDGE_CONTINUE(detail.__outer_port > 0);

        for (BIntSet::const_iterator vc_iter = detail.__scene_list.begin();
                vc_iter != detail.__scene_list.end(); ++vc_iter)
        {
            ProtoSceneAddress *scene_address = respond.add_scene_addrs();
            scene_address->set_scene_id(*vc_iter);
            scene_address->set_domain(detail.__domain);
            scene_address->set_address(detail.__address);
            scene_address->set_port(detail.__outer_port);
        }
    }

    GatePlayer *player = 0;
    {
    	TransactionData *trans_data = transaction->fetch_data(DB_GATE_PLAYER);
    	if (trans_data != 0)
    		player = trans_data->__data.__gate_player;
        // 加载mongo完成
        if (player != 0)
            player->set_is_loading_mongo(false);
    }

    if (player != 0)
    {
#ifndef TEST_COMMAND
    	if (player->detail().__permission != GameEnum::PERT_GM)
    	{
			if (FLOW_INSTANCE->check_flow_detail_type(GameEnum::IS_FORBIT_LOGIN) != 0 &&
					FLOW_INSTANCE->is_forbit_channel(player->agent()) == true)
			{
				MSG_USER("ERROR backstage forbit login %s %s", player->account(), player->agent());
				return this->dispatch_to_client(client_sid, RETURN_GATE_SESSION, ERROR_NO_OPEN_LOGIN);
			}
    	}
#endif

    	if (player->is_ban_login())
    	{
    		respond.set_ban_type(player->ban_type());
    		respond.set_expired_time(player->ban_expried_time());
    		respond.set_left_ban_sec(player->ban_expried_time() - ::time(0));
    		MSG_USER("player[%ld] is ban login ban[%d]", player->role_id(), player->ban_type());
    		return this->dispatch_to_client(client_sid, RETURN_GATE_SESSION, ERROR_NOT_ALLOW_LOGIN, &respond);
    	}

    	MSG_USER("is_ban_login:%d,this->ban_type_:%d,expried_time:%ld by_xzm",
    			player->is_ban_login(),player->ban_type(),player->ban_expried_time());
    }

    if (transaction->detail().__error != 0)
    {
        if (transaction->detail().__error != ERROR_ROLE_NOT_EXISTS)
        {
            this->dispatch_to_client(transaction->sid(), RETURN_GATE_SESSION, transaction->detail().__error, &respond);
            transaction->rollback();
            return -1;
        }

        player = 0;
        transaction->rollback();
        respond.set_role_id(0);
        MSG_USER("mongo no player");

        //开服天数限制
        const Json::Value& conf = CONFIG_INSTANCE->cur_servers_list();
        string agent_name = conf["agent_name"].asString();
        int agent_code = CONFIG_INSTANCE->agent_code(agent_name);

        const Json::Value& limit_ids = CONFIG_INSTANCE->const_set_conf("limit_ids")["arr"];
        if (limit_ids.empty() == false)
        {
        	for (uint i = 0; i < limit_ids.size(); ++i)
        	{
        		int limit_code = limit_ids[i][0u].asInt();
        		JUDGE_CONTINUE(agent_code == limit_code);

        		int create_limit_day = limit_ids[i][1u].asInt();
        		int open_server_day = CONFIG_INSTANCE->open_server_days();
        		JUDGE_CONTINUE(open_server_day > create_limit_day);

        		MSG_USER("ERROR pass open server day %d %d", create_limit_day, open_server_day);
        		this->dispatch_to_client(client_sid, RETURN_GATE_SESSION, ERROR_PASS_OPEN_SERVER_DAY);
        		return -1;
        	}
        }
    }
    else
    {
        TransactionData *trans_data = transaction->fetch_data(DB_GATE_PLAYER);
        if (trans_data == 0)
        {
            this->dispatch_to_client(transaction->sid(), RETURN_GATE_SESSION, ERROR_SERVER_INNER, &respond);
            transaction->rollback();
            return -1;
        }

        player = trans_data->__data.__gate_player;
        player->set_is_loading_mongo(false);
        {
            GatePlayer *account_player = NULL, *sid_player = NULL;

            // 检查原连接是否还存在
            if (this->account_player_map_.find(player->account(), account_player) == 0)
            {
                if (account_player != player)   // 检查GatePlayer原连接被另一条连接冲掉
                {
                    MSG_USER("account rebinded [%s %s] sid [%d %d]", 
                            player->account(), account_player->account(),
                            player->client_sid(), account_player->client_sid());

                    // 当前的连接还没被回收再利用
                    if (this->sid_player_map_.find(player->client_sid(), sid_player) == 0 &&
                            sid_player == player)
                    {
                        this->sid_player_map_.unbind(player->client_sid());
                        this->sid_account_map_.unbind(player->client_sid());
                    }

                    this->player_pool()->push(player);
                    return -1;
                }
            }
            else
            {
                MSG_USER("account disconnect %s %d", player->account(), player->client_sid());

                // 原连接不存在，删除对应的绑定
                // 当前的连接还没被回收再利用
                if (this->sid_player_map_.find(player->client_sid(), sid_player) == 0 &&
                        sid_player == player)
                {
                    this->sid_player_map_.unbind(player->client_sid());
                    this->sid_account_map_.unbind(player->client_sid());
                }
                std::string account = player->account();
                this->account_sid_map_.erase(account);
                
                this->player_pool()->push(player);
                return -1;
            }
        }

        if (player->role_id() > 0)
        {
            respond.set_role_id(player->role_id());
            ProtoGateRole *proto_gate_role = respond.add_role_list();
            proto_gate_role->set_role_id(player->role_id());
            proto_gate_role->set_name(player->detail().name());
            proto_gate_role->set_sex(player->detail().__sex);
            proto_gate_role->set_career(player->detail().__career);
            proto_gate_role->set_level(player->detail().__level);
        }
        else
        {
            player = 0;
            transaction->rollback();
            respond.set_role_id(0);
            MSG_USER("mongo no player");
        }
    }

    this->dispatch_to_client(client_sid, &respond);

    if (player != 0)
    {
        std::string ip;
        int port = 0;
        Svc *client_svc = 0;
        if (this->client_monitor_.find_service(client_sid, client_svc) == 0)
        {
            client_svc->get_peer_addr(ip, port); 
        }
        player->set_client_ip(ip);
        player->set_client_port(port);

        if (player->detail().__server_flag.empty())
        {
            player->detail().__server_flag = CONFIG_INSTANCE->server_flag();
        }

        this->account_login_tick_map_[player->account()] = Time_Value::gettimeofday().sec() + 3;
        player->start_game();
    }
    return 0;
}

#ifndef NO_USE_PROTO
int GateMonitor::create_new_role(const int sid, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto10600102*, request, -1);

    std::string role_name = request->name(), 
        account = request->account(), 
        session = request->session(),
        agent = request->agent();

    int sex = request->sex();
    int market = request->market();

#else
int GateMonitor::create_new_role(const int sid, Block_Buffer *msg)
{
    std::string role_name, account, session, agent, platform;
    int sex = 0, market = 0, career = 0;

    (*msg) >> role_name >> sex >> account >> session >> agent >> platform >> market >> career;
#endif

#ifndef LOCAL_DEBUG
    if (CONFIG_INSTANCE->agent_code(agent) <= 0)
    {
        MSG_USER("ERROR gate create role[no agent code] %s %s %s %s", role_name.c_str(), account.c_str(), session.c_str(), agent.c_str());
    }
#endif
    MSG_USER("before correct request create new role %d %s %s %s %s %d %d", sid,
    		role_name.c_str(), account.c_str(), session.c_str(), agent.c_str(), sex, market);

    std::string account_agent = this->get_agent_from_account(account);
    this->correct_agent_market(account_agent, agent, market);
    if (agent.empty())
    {
    	agent = account_agent;
    	int cfg_agent_code = CONFIG_INSTANCE->agent_code(agent);
		if (market <= 0)
			market = cfg_agent_code * 10000 + 1;
    }

    MSG_USER("after correct request create new role %d %s %s %s %s %d", sid,
    		role_name.c_str(), account.c_str(), session.c_str(), agent.c_str(), sex);
#ifdef TASK_RENAME
    if (account.empty() == true || session.empty() == true)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_CLIENT_OPERATE);
    }
#else
    if (role_name.empty() == true || account.empty() == true || session.empty() == true)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_CLIENT_OPERATE);
    }
#endif

    MSG_USER("request create new role %s %s 1", role_name.c_str(), account.c_str());

    GatePlayer *player = 0;
    if (this->find_player_by_sid(sid, player) != 0)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_CLIENT_OPERATE);
    }

    MSG_USER("request create new role %s %s 2", role_name.c_str(), account.c_str());

    if (account != player->account() || player->is_active() == true)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_CLIENT_OPERATE);
    }

    MSG_USER("request create new role %s %s 3", role_name.c_str(), account.c_str());

    SessionDetail *p_session = 0;
    if (this->session_manager_->find_account_session(account, p_session) != 0
    		|| p_session->__session != session)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_SESSION_ILLEGAL);
    }

    if (!(0 < role_name.length() && role_name.length() <= MAX_NAME_LENGTH))
    {
    	return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_NAME_TOO_LONG);
    }

	char buffer[MAX_NAME_LENGTH + 1] = {0,};
	string_remove_black_char(buffer, MAX_NAME_LENGTH, role_name.c_str(), role_name.length());

   	int utf8_length = string_utf8_len(buffer);
   	if (!(0 < utf8_length && utf8_length <= GameEnum::MAX_PLAYER_NAME_LEN))
   	{
   		return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_NAME_TOO_LONG);
   	}

    MSG_USER("request create new role %s %s 4", role_name.c_str(), account.c_str());

    GateRoleDetail &detail = player->detail();
	detail.set_cur_server_flag();
    detail.set_name(buffer);

    //职业和性别一样
    detail.__sex = (sex != 2 ? 1 : 2);
    detail.__career = detail.__sex;
    detail.__level = 1;

    const Json::Value &role_json = CONFIG_INSTANCE->role(detail.__sex);
    detail.__scene_id = role_json["scene"].asInt();

    MoverCoord create_coord;
    create_coord.set_pixel(role_json["coord"][0u].asInt(), role_json["coord"][1u].asInt());
    detail.__pos_x = create_coord.pos_x();
    detail.__pos_y = create_coord.pos_y();
    detail.__pixel_x = create_coord.pixel_x();
    detail.__pixel_y = create_coord.pixel_y();

	std::string ip;
	int port = 0;
	Svc *client_svc = 0;
	if (this->client_monitor_.find_service(sid, client_svc) == 0)
	{
		client_svc->get_peer_addr(ip, port);
	}
	player->set_client_ip(ip);
	player->set_client_port(port);

    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_SAVE_NEW_ROLE,
                DB_GATE_PLAYER, player, 0,
                GATE_MONITOR->logic_unit(), sid) != 0)
    {
        return this->dispatch_to_client(sid, RETURN_CREATE_ROLE, ERROR_SERVER_INNER);
    }
    return 0;
}

int GateMonitor::after_save_new_role(Transaction *transaction)
{
    CHECK_GATE_TRANSACTION_ERROR(transaction->sid(), transaction, RETURN_CREATE_ROLE);

    TransactionData *trans_data = transaction->fetch_data(DB_GATE_PLAYER);
    if (trans_data == 0)
        return -1;

    GatePlayer *player = trans_data->__data.__gate_player;
    trans_data->reset();
    transaction->summit();

    MSG_USER("gate newed role %ld %s %d", player->role_id(), player->name(), player->client_sid());

    Proto50600102 respond;
    respond.set_test(1);
    this->dispatch_to_client(player->client_sid(), &respond);

#ifndef LOCAL_DEBUG
	if (player->detail().__permission != GameEnum::PERT_GM)
	{
		if (FLOW_INSTANCE->check_flow_detail_type(GameEnum::IS_FORBIT_LOGIN) != 0 &&
				FLOW_INSTANCE->is_forbit_channel(player->agent()) == true)
		{
			MSG_USER("ERROR backstage forbit login %s %s",
					player->account(), player->agent());
			return this->dispatch_to_client(transaction->sid(), RETURN_CREATE_ROLE, ERROR_NO_OPEN_LOGIN);
		}
	}
#endif

    // 更新登录频率控制，防止刚登录需要切换场景时玩家突然发一个登录请求
    this->account_login_tick_map_[player->account()] = Time_Value::gettimeofday().sec() + 3;

    return player->start_game();
}

void GateMonitor::report_pool_info(const bool report)
{
    {
        std::ostringstream msg_stream;
        POOL_MONITOR->report_pool_info(msg_stream);
        MSG_USER("%s", msg_stream.str().c_str());
    }

    {
		std::ostringstream msg_stream;
		msg_stream << "Gate Pool Info:" << std::endl;
		msg_stream << "ClientServicePool:" << std::endl;
		this->client_monitor_.service_pool()->dump_info_to_stream(msg_stream);
		msg_stream << "Inner Service Pool:" << std::endl;
		this->inner_monitor_.service_pool()->dump_info_to_stream(msg_stream);
		msg_stream << "Connect Service Pool:" << std::endl;
		this->connect_monitor_.service_pool()->dump_info_to_stream(msg_stream);

		msg_stream << "SessionDetailPool:" << std::endl;
		this->session_manager()->session_pool()->dump_info_to_stream(msg_stream);

		msg_stream << "GatePlayerPool:" << std::endl;
		this->player_pool()->dump_info_to_stream(msg_stream);

		MSG_USER("%s", msg_stream.str().c_str());
    }
}

int GateMonitor::update_relogin_tick(void)
{
    this->relogin_all_tick_ = Time_Value::gettimeofday() + Time_Value(15);
    return 0;
}

int GateMonitor::relogin_logic_for_all_player(void)
{
    JUDGE_RETURN(this->relogin_all_tick_ != Time_Value::zero, 0);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->relogin_all_tick_ <= nowtime, 0);

    this->relogin_all_tick_ = Time_Value::zero;
    for (GateMonitor::PlayerMap::iterator iter = this->player_map_.begin();
            iter != this->player_map_.end(); ++iter)
    {
        JUDGE_CONTINUE(iter->second->is_active() == true);
        iter->second->reqeuest_login_logic();
    }

    return 0;
}

int GateMonitor::gate_scene(void)
{
    int gate_scene = CONFIG_INSTANCE->global()["server_list"][this->server_config_index_]["scene"][0u].asInt();
    return gate_scene;
}

void GateMonitor::set_auth_sid(const int sid)
{
    this->auth_sid_ = sid;
}

int GateMonitor::auth_sid(void)
{
    return this->auth_sid_;
}

int GateMonitor::get_client_ip_port(int sid, std::string &ip, int &port)
{
	Svc *svc = NULL;
	JUDGE_RETURN(this->client_monitor_.find_service(sid, svc) == 0, -1);
	return svc->get_peer_addr(ip, port);
}

int GateMonitor::add_reconnect_index(int index)
{
	this->reconnect_set_.insert(index);
	this->start_reconnect_tick();
	return 0;
}

int GateMonitor::start_reconnect_tick(int tick)
{
	JUDGE_RETURN(this->reconnect_tick_ == Time_Value::zero, -1);
	this->reconnect_tick_ = Time_Value::gettimeofday() + Time_Value(tick);
	return 0;
}

int GateMonitor::reset_reconnect_tick()
{
    if (this->reconnect_set_.size() > 0)
    {
        this->reconnect_tick_ = Time_Value::gettimeofday() + Time_Value(5);
    }
    else
    {
        this->reconnect_tick_ = Time_Value::zero;
    }

    return 0;
}

int GateMonitor::check_client_accelerate(const int sid)
{
	int heart_no_return = CONFIG_INSTANCE->tiny("heart_no_return").asInt();
	JUDGE_RETURN(heart_no_return == 0, 0);

    GatePlayer* player = 0;
    JUDGE_RETURN(this->find_player_by_sid(sid, player) == 0, 0);

    int check_flag = CONFIG_INSTANCE->const_set("accelerate_validate");
    SESSION_MANAGER->update_session_time(player->account());

	if (check_flag == 0)
	{
		player->set_alive_amount(0);
	}
	else
	{
		player->check_accelerate(Time_Value::gettimeofday());
		player->set_alive_amount(player->alive_amount() + 1);
	}

    Proto50999999 req;
    return this->dispatch_to_client(sid, RETURN_SERVER_KEEP_ALIVE, 1, &req);
}

int GateMonitor::fetch_config_index_by_sid(const int sid)
{
	int scene_id = 0, line_id = 0;
	{
		GUARD_READ(RW_MUTEX, mon, this->connect_scene_to_sid_map_.mutex());
		for (SceneToSidMap::iterator scene_iter = this->connect_scene_to_sid_map_.begin();
				scene_iter != this->connect_scene_to_sid_map_.end(); ++scene_iter)
		{
			SceneToSidMap::KeyValueMap *kv_map = scene_iter->second;
			{
				GUARD_READ(RW_MUTEX, mon2, kv_map->mutex());
				for (SceneToSidMap::KeyValueMap::iterator kv_iter = kv_map->begin();
						kv_iter != kv_map->end(); ++kv_iter)
				{
					if (kv_iter->second == sid)
					{
						scene_id = scene_iter->first;
						line_id = kv_iter->first;
						break;
					}
				}
			}
			if (scene_id > 0)
				break;
		}
	}

	int config_index = -1;
    if (this->scene_to_index_map_.find(scene_id, line_id, config_index) != 0)
        return -1;

    return config_index;
}

int GateMonitor::process_connect_svc_close(Block_Buffer *buff)
{
	int cfg_index = -1;

	buff->read_int32(cfg_index);
	JUDGE_RETURN(cfg_index >= 0, -1);

    return this->add_reconnect_index(cfg_index);
}

int GateMonitor::process_connected_svc(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30600103 *, request, 0);

    int cfg_index = request->cfg_index();
    this->reconnect_set_.erase(cfg_index);
    this->reset_reconnect_tick();

    IntMap scene_map;
    const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(request->cfg_index());
    for (BIntSet::const_iterator iter = server_detail.__scene_list.begin();
    		iter != server_detail.__scene_list.end(); ++iter)
    {
    	int scene_id = *iter;
    	JUDGE_CONTINUE(scene_id > 10000);
    	scene_map[scene_id] = true;
    }

    int total = 0;
    for (PlayerMap::iterator iter = this->player_map_.begin();
    		iter != this->player_map_.end(); ++iter)
    {
    	GatePlayer* player = iter->second;
    	JUDGE_CONTINUE(player != NULL);
    	JUDGE_CONTINUE(scene_map.count(player->detail().__scene_id) > 0);

    	++total;
    	player->update_map_sid();
    }

    MSG_USER("[3]reconnect %d %d %d %d %d", request->scene_id(), request->line_id(),
    		cfg_index, total, request->connect_sid());
    return 0;
}

int GateMonitor::check_and_connect_svc(void)
{
	JUDGE_RETURN(this->reconnect_tick_ > Time_Value::zero, 0);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->reconnect_tick_ <= nowtime, 0);

    MSG_USER("check reconnect, %d", this->reconnect_set_.size());

    IntVec remove_index;
    for (IntSet::iterator iter = this->reconnect_set_.begin();
            iter != this->reconnect_set_.end(); ++iter)
    {
        int config_index = *iter;
        const Json::Value &server_json = CONFIG_INSTANCE->server(config_index);

        int scene_id = server_json["scene"][0u].asInt();
        int line_id = std::max<int>(server_json["line"].asInt(), 1) - 1;

        int sid = this->connect_scene_to_sid(scene_id, line_id);
        MSG_USER("[1]reconnect %d %d %d %d", scene_id, line_id, config_index, sid);

        if (sid < 0)
        {
            // 发到专负责连接的线程处理连接
            const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(config_index);
            JUDGE_CONTINUE(server_detail.need_connect() == true);

            Proto30600103 *inner_req = new Proto30600103;
            inner_req->set_scene_id(scene_id);
            inner_req->set_line_id(line_id);
            inner_req->set_cfg_index(config_index);
            inner_req->set_address(server_detail.__address);
            inner_req->set_port(server_detail.__inner_port);

            UnitMessage unit_msg;
            unit_msg.__msg_head.__recogn = INNER_CONNECT_SERVER;
            unit_msg.set_msg_proto(inner_req);
            this->connect_unit()->push_request(unit_msg);

            MSG_USER("[2]reconnect %d %d %d %s", scene_id, line_id, config_index,
            		server_detail.__address.c_str());
        }
        else
        {
        	remove_index.push_back(config_index);
        }
    }

    for (IntVec::iterator iter = remove_index.begin(); iter != remove_index.end(); ++iter)
    {
        this->reconnect_set_.erase(*iter);
    }

    return this->reset_reconnect_tick();
}

int GateMonitor::select_random_name(const int sid, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto10600105 *, request, -1);
    JUDGE_RETURN(0 < request->sex() && request->sex() <= 2, -1);

    GatePlayer *player = NULL;
    JUDGE_RETURN(this->find_player_by_sid(sid, player) == 0, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    int64_t rename_tick = 0;
    if (this->account_rename_tick_map_.find(player->detail().__account, rename_tick) == 0 && rename_tick >= nowtime.sec())
    	return -1;

    rename_tick = nowtime.sec() + 2L;
    this->account_rename_tick_map_.rebind(player->detail().__account, rename_tick);

    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    MMORole::update_sex_condition(data_map, request->sex(), this->sid_account_map_[sid]);

    if (TRANSACTION_MONITOR->request_mongo_transaction(0, TRANS_FETCH_RAND_NAME,
    		data_map, this->logic_unit(), sid) != 0)
    {
        POOL_MONITOR->mongo_data_map_pool()->push(data_map);
        return -1;
    }

    return 0;
}

int GateMonitor::after_select_random_name(Transaction *transaction)
{
    if (transaction->is_failure())
    {
        this->dispatch_to_client(transaction->sid(), RETURN_RANDOM_NAME, transaction->detail().__error);
        transaction->rollback();
        return 0;
    } 
 
    MongoDataMap *data_map = transaction->fetch_mongo_data_map(); 
    MongoData *mongo_data = 0; 
    if  (data_map->find_data(Role::COLLECTION, mongo_data) != 0) 
    { 
        this->dispatch_to_client(transaction->sid(), RETURN_RANDOM_NAME, ERROR_SERVER_INNER); 
        transaction->rollback(); 
        return -1; 
    }

    std::string name = MMORole::fetch_role_name(mongo_data->data_bson());
    Proto50600105 respond;
    respond.set_name(name);
    return this->dispatch_to_client(transaction->sid(), &respond);
}

int GateMonitor::process_travel_mongo_save(int sid, Message* msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400054*, request, -1);

    //超时
    if ((request->tick() > 0) && (request->tick() + 45 < ::time(NULL)))
    {
    	MSG_USER("Trvl Data Timeout %ld %ld", request->role_id(), request->tick());
    	return -1;
    }

    MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    for (int i = 0; i < request->mongo_data_size(); ++i)
    {
        const ProtoMongoData &proto_data = request->mongo_data(i);
        MongoData *mongo_data = data_map->pop_data();
        mongo_data->unserialize(proto_data);

        if (data_map->bind_data(mongo_data) != 0)
        {
            data_map->push_data(mongo_data);
        }
    }

    data_map->check_role_ = true; //check role id

    int mongo_recogn = request->mongo_recogn();
    if (TRANSACTION_MONITOR->request_mongo_transaction(request->role_id(), mongo_recogn, data_map) != 0)
	{
		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
		return -1;
	}

    return 0;
}

int GateMonitor::process_travel_serial_save(int sid, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30600104*, request, -1);
	return Log::instance()->logging_mysql(request->name(), request->content());
}

int GateMonitor::process_connect_server(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30600103 *, request, 0);

    int scene_id = request->scene_id();
    int line_id = request->line_id();

    int connect_sid = this->connect_scene_to_sid(scene_id, line_id);
    JUDGE_RETURN(connect_sid <= 0, 0);

    std::string address = request->address();
    connect_sid = this->connect_connector()->connect(address.c_str(), request->port());
    JUDGE_RETURN(connect_sid >= 0, 0);

    this->sid_line_map_.rebind(connect_sid, line_id);
    request->set_connect_sid(connect_sid);

    const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(request->cfg_index());
    for (BIntSet::const_iterator iter = server_detail.__scene_list.begin();
    		iter != server_detail.__scene_list.end(); ++iter)
    {
        int other_scene_id = *iter;
        this->connect_scene_to_sid_map_.rebind(other_scene_id, line_id, connect_sid);
    }

    return this->connect_unit_->dispatch_to_gate_unit(*request);
}

int GateMonitor::db_load_mode_done(Transaction* trans)
{
	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
	JUDGE_RETURN(trans_data != NULL, -1);

	DBShopMode* shop_mode = trans_data->__data.__shop_mode;
	switch(shop_mode->recogn_)
	{
	case TRANS_LOAD_COMBINE_SERVER:
	{
		this->after_load_combine_server(shop_mode);
		break;
	}
	default:
	{
		break;
	}
	}

    return trans->summit();
}

int GateMonitor::set_combine_server_check_time(Int64 now_tick)
{
#ifdef TEST_COMMAND
	this->combine_server_.check_tick_ = now_tick + Time_Value::MINUTE;
#else
	this->combine_server_.check_tick_ = now_tick + Time_Value::HOUR;
#endif
	return 0;
}

int GateMonitor::request_load_combine_server(Int64 now_tick)
{
	JUDGE_RETURN(this->combine_server_.check_tick_ < now_tick, -1);
	this->set_combine_server_check_time(now_tick);

	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, -1);

	shop_mode->recogn_ = TRANS_LOAD_COMBINE_SERVER;
	shop_mode->input_argv_.type_int64_ = this->combine_server_.update_tick_;

	return GameCommon::db_load_mode_begin(shop_mode, this->logic_unit());
}

int GateMonitor::after_load_combine_server(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode->sub_value_ == 1, -1);

	MSG_USER("combine server");
	return 0;
}

int GateMonitor::force_reconnect_trvl(ServerDetail& server_detail)
{
	if (CONFIG_INSTANCE->check_update_sub_value(
			server_detail.__is_travel) == true)
	{
		return true;
	}

	return false;
}

int GateMonitor::check_need_reconnect_trvl(ServerDetail& server_detail)
{
	JUDGE_RETURN(server_detail.__is_travel >= 1, false);
	JUDGE_RETURN(this->force_reconnect_trvl(server_detail) == false, true);

	switch(server_detail.__is_travel)
	{
	case 1:
	{
		return server_detail.need_reconnect_travel();
	}
	case 2:
	{
		return server_detail.need_reconnect_channel();
	}
	}

	return false;
}

int GateMonitor::init_travel_map_index()
{
	JUDGE_RETURN(this->travel_map_index_set_.empty() == true, -1);

	const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
	for (GameConfig::ServerList::const_iterator iter = server_list.begin();
			iter != server_list.end(); ++iter)
	{
		const ServerDetail &server_detail = *iter;
		JUDGE_CONTINUE(server_detail.__is_travel > 0);
		this->travel_map_index_set_.insert(server_detail.__index);
	}

	return 0;
}

int GateMonitor::check_update_trvl_url()
{
	GameConfig::ServerList& server_list = CONFIG_INSTANCE->server_list();
	for (GameConfig::ServerList::iterator iter = server_list.begin();
			iter != server_list.end(); ++iter)
	{
		ServerDetail& server_detail = *iter;
		JUDGE_CONTINUE(this->check_need_reconnect_trvl(server_detail) == true);

		//配置有变化，重新连接
		Svc *svc = NULL;
		const Json::Value &server_json = CONFIG_INSTANCE->server(server_detail.__index);

		int scene_id = server_json["scene"][0u].asInt();
        int line_id = std::max<int>(server_json["line"].asInt(), 1) - 1;

        int sid = this->fetch_sid(scene_id, line_id);
        MSG_USER("%d, %d", scene_id, sid);

		if (sid >= 0 && this->connect_monitor_.find_service(sid, svc) == 0)
		{
			svc->handle_close();
		}
		else
		{
			this->add_reconnect_index(server_detail.__index);
		}
	}

	return 0;
}

int GateMonitor::check_and_fetch_travel_area(void)
{
    for (IntSet::iterator iter = this->travel_map_index_set_.begin();
            iter != this->travel_map_index_set_.end(); ++iter)
    {
        int idx = *iter;
        const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(idx);
        this->fetch_travel_area(idx, server_detail);
    }

    return 0;
}

int GateMonitor::fetch_travel_area(int index, const ServerDetail &server_detail)
{
    Proto30402018 inner_req;
    inner_req.set_server_flag(CONFIG_INSTANCE->server_flag());

    int scene_id = 0;
    for (BIntSet::iterator iter = server_detail.__scene_list.begin();
            iter != server_detail.__scene_list.end(); ++iter)
    {
#ifdef LOCAL_DEBUG
    	JUDGE_CONTINUE(GameCommon::is_travel_scene(*iter) == true);
#endif
        scene_id = *iter;
    	inner_req.add_scene_list(*iter);
    }

    ProtoHead head;
    head.__recogn = INNER_FETCH_TRAVEL_AREA;
    head.__scene_id = scene_id;
    head.__src_scene_id = scene_id;
    head.__src_line_id = index;
    return this->dispatch_to_scene(&head, &inner_req);
}

int GateMonitor::process_set_travel_area(int sid, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30402018 *, request, -1);

    for (int i = 0; i < request->scene_list_size(); ++i)
    {
        int scene_id = request->scene_list(i), line_id = 0;
        if (i < request->line_list_size())
            line_id = request->line_list(i);        // 网关内存的line_id + 1才是跨服传过来的值
        JUDGE_CONTINUE(line_id > 0);

        int tmp_line_id = 0;
        if (this->travel_scene_line_map_.find(scene_id, tmp_line_id) != 0 ||
        		tmp_line_id != (line_id - 1))
        {
        	MSG_USER("travel scene line %d %d", scene_id, line_id - 1);
        }

        this->travel_scene_line_map_.rebind(scene_id, line_id - 1);
    }

    return 0;
}

int GateMonitor::process_travel_map_keep_alive(int sid, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400027 *, request, -1);

	ProtoHead head;
	head.__recogn = INNER_MAP_TRVL_KEEP_ALIVE;
	head.__scene_id = request->scene();
	head.__src_line_id = request->index();
	return this->dispatch_to_scene_by_sid(sid, &head, (Block_Buffer*)NULL);
}

std::string GateMonitor::get_agent_from_account(const string &account)
{
	int i = 0;
	char chr_agent[64 + 1];
	for (i = 0; i < int(account.length()) && i < 64; ++i)
	{
		if (account[i] == '_')
			break;
		chr_agent[i] = account[i];
	}
	chr_agent[i] = '\0';
	return std::string(chr_agent);
}

void GateMonitor::correct_agent_market(const std::string &account_agent, std::string &agent, int &market_code)
{
	const Json::Value &correct_agent_json = CONFIG_INSTANCE->correct_agent(account_agent);
	JUDGE_RETURN(correct_agent_json != Json::Value::null, ;);

	for (uint i = 0; i < correct_agent_json.size(); ++i)
	{
		const Json::Value &condition_json = correct_agent_json[i]["condition"];
		if (condition_json.isMember("market"))
		{
			JUDGE_CONTINUE(condition_json["market"].asInt() == market_code);
		}
		if (condition_json.isMember("agent"))
		{
			JUDGE_CONTINUE(condition_json["agent"].asString() == agent);
		}

		const Json::Value &correct_json = correct_agent_json[i]["correct"];
		if (correct_json.isMember("agent"))
			agent = correct_json["agent"].asString();
		if (correct_json.isMember("market"))
			market_code = correct_json["market"].asInt();
		break;
	}
}
