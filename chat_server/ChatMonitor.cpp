/*
 * ChatMonitor.cpp
 *
 * Created on: 2013-01-18 14:23
 *     Author: glendy
 */

#include "ChatMonitor.h"
#include "GameConfig.h"
#include "GameCommon.h"
#include "BackField.h"
#include "PoolMonitor.h"
#include "SessionManager.h"
#include "ProtoDefine.h"
#include "ChannelAgency.h"
#include "MongoConnector.h"
#include "PubStruct.h"
#include "Transaction.h"
#include <mongo/client/dbclient.h>

ChatMonitor::ChatTenSecTimer::ChatTenSecTimer()
	: FixedTimer(GTT_CHAT_TEN_SEC, TRANSACTION_INTERVAL)
{

}

int ChatMonitor::ChatTenSecTimer::handle_timeout(const Time_Value &tv)
{
	FLOW_INSTANCE->request_load_flow_detail();
	return 0;
}

ChatMonitor::ChatTimer::ChatTimer()
	: FixedTimer(GTT_CHAT_ONE_MINUTE, Time_Value::MINUTE)
{/* null */}

int ChatMonitor::ChatTimer::handle_timeout(const Time_Value &tv)
{
	CHAT_MONITOR->request_load_chat_limit();
	CHAT_MONITOR->request_load_word_check();
	CHAT_MONITOR->request_load_vip_limit();
	return 0;
}

int ChatMonitor::VipTimesTimer::type()
{
	return GTT_CHAT_ONE_SEC;
}

int ChatMonitor::VipTimesTimer::handle_timeout(const Time_Value &tv)
{
	return CHAT_MONITOR->reset_player_chat_times();
}

int ChatMonitor::init_game_timer_handler(void)
{
    const double inter_tick_list[] = {
        CHAT_PLAYER_INTERVAL,
        CHAT_TRANSACTION_INTERVAL,
        CHAT_CHANNEL_INTERVAL,
        1,
        1,
        1
    };
    const int timer_amount = sizeof(inter_tick_list) / sizeof(double);

    this->timer_handler_list_.resize(timer_amount);
    POOL_MONITOR->init_game_timer_list(timer_amount);

    double inter_sec = 0.0, inter_usec = 0.0;
    int index = 0;
    for (int i = GTT_CHAT_TYPE_BEG + 1; i < GTT_CHAT_TYPE_END; ++i)
    {
        index = i - GTT_CHAT_TYPE_BEG - 1;
        inter_usec = modf(inter_tick_list[index], &inter_sec);
        Time_Value interval(inter_sec, inter_usec * 1000000);
        this->timer_handler_list_[index].set_type(i);
        this->timer_handler_list_[index].set_interval(interval);
    }
    return 0;
}

int ChatMonitor::start_game_timer_handler(void)
{
	for (TimerHandlerList::iterator iter = this->timer_handler_list_.begin();
			iter != this->timer_handler_list_.end(); ++iter)
	{
		iter->schedule_timer(iter->interval());
	}
	return 0;
}

ChatMonitor::ChatMonitor(void) :
    player_map_(get_hash_table_size(ChatMonitor::MAP_OBJECT_BUCKET)),
    sid_player_map_(get_hash_table_size(ChatMonitor::MAP_OBJECT_BUCKET)),
    flaunt_map_(get_hash_table_size(ChatMonitor::MAP_OBJECT_BUCKET))
{
	this->session_manager_ = 0;
	this->channel_agency_ = 0;
	this->chat_times_map_.clear();
}

int ChatMonitor::init(void)
{
    Time_Value client_send_timeout(0, 100 * 1000);
    Time_Value inner_send_timeout(0, 100 * 1000);
    {
    	Time_Value recv_timeout(this->fetch_receive_timeout(), 0);
        this->client_receiver_.set(&recv_timeout);
    }
    {
        Time_Value send_timeout(0, 100 * 1000);
        this->connect_sender_.set(send_timeout);
    }
    this->client_monitor_.set_svc_max_recv_size(400);
    this->client_monitor_.set_svc_max_pack_size(1024 * 1024);
    this->inner_monitor_.set_svc_max_list_size(20000);
    this->inner_monitor_.set_svc_max_pack_size(1024 * 1024);
    this->connect_monitor_.set_svc_max_list_size(20000);

    this->client_packer_.monitor(&(this->client_monitor_));
    this->client_monitor_.packer(&(this->client_packer_));

    this->inner_packer_.monitor(&(this->inner_monitor_));
    this->inner_monitor_.packer(&(this->inner_packer_));

    this->session_manager_ = SESSION_MANAGER;
    this->channel_agency_ = new ChannelAgency(this);
    this->channel_agency_->init();

    return SUPPER::init(client_send_timeout, inner_send_timeout);
}

int ChatMonitor::start(void)
{
    SUPPER::start();

    MSG_USER("start chat server");
    return 0;
}

int ChatMonitor::start_game(void)
{
	FLOW_INSTANCE->load_flow_detail_when_init();

	this->channel_agency_->start();
	this->ten_sec_timer_.schedule_timer();
	this->chat_timer_.schedule_timer();
	this->request_load_chat_limit();
	this->request_load_word_check();
	this->request_load_vip_limit();

	return 0;
}

void ChatMonitor::fina(void)
{
	this->channel_agency_->stop();

    this->timer_handler_list_.clear();
//    SAFE_DELETE(this->session_manager_);
    SAFE_DELETE(this->channel_agency_);

    this->player_map_.unbind_all();
    this->sid_player_map_.unbind_all();
    this->flaunt_map_.unbind_all();
    this->chat_times_map_.clear();

    this->player_pool_.clear();
    this->flaunt_record_pool_.clear();
    this->chat_times_pool_.clear();

    SUPPER::fina();
}

int ChatMonitor::logout_all_player(void)
{
    std::vector<ChatPlayer *> player_list;
    for (PlayerMap::iterator iter = this->player_map_.begin();
            iter != this->player_map_.end(); ++iter)
    {
        player_list.push_back(iter->second);
    }

    for (std::vector<ChatPlayer *>::iterator iter = player_list.begin();
            iter != player_list.end(); ++iter)
    {
    	ChatPlayer* player = *iter;
        player->sign_out();
    }

    return 0;
}

BaseUnit *ChatMonitor::logic_unit(void)
{
    return &(this->logic_unit_);
}

Block_Buffer *ChatMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int ChatMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

ChatMonitor::UnitMessagePool *ChatMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

int ChatMonitor::fetch_receive_timeout()
{
#ifndef LOCAL_DEBUG
	if (CONFIG_INSTANCE->cur_server().isMember("chat_heart") == true)
	{
		return Time_Value::MINUTE * 45;
	}
	if (CONFIG_INSTANCE->tiny("heart").asInt() == 1)
	{
		return Time_Value::MINUTE * 5;
	}
#endif
	return 1500000;
}

int ChatMonitor::find_client_service(const int sid, ChatClientService *&svc)
{
    Svc *tmp_svc = 0;
    if (this->client_monitor_.find_service(sid, tmp_svc) != 0)
        return -1;

    svc = dynamic_cast<ChatClientService *>(tmp_svc);
    if (svc == 0)
        return -1;
    return 0;
}

int ChatMonitor::find_inner_service(const int sid, ChatInnerService *&svc)
{
    Svc *tmp_svc = 0;
    if (this->inner_monitor_.find_service(sid, tmp_svc) != 0)
        return -1;

    svc = dynamic_cast<ChatInnerService *>(tmp_svc);
    if (svc == 0)
        return -1;
    return 0;
}

int ChatMonitor::find_connect_service(const int sid, ChatConnectService *&svc)
{
    Svc *tmp_svc = 0;
    if (this->connect_monitor_.find_service(sid, tmp_svc) != 0)
        return -1;

    svc = dynamic_cast<ChatConnectService *>(tmp_svc);
    if (svc == 0)
        return -1;
    return 0;
}

int ChatMonitor::dispatch_to_client(const int sid, Block_Buffer *buff)
{
    if (sid <= 0)
    {
        MSG_USER("ERROR chat player no coonect ");
        return -1;
    }
    return this->client_sender(sid)->push_data_block_with_len(sid, *buff);
}

int ChatMonitor::dispatch_to_client(const int sid, const int recogn, const int error, const Message *msg_proto)
{
    if (sid <= 0)
    {
        MSG_USER("ERROR chat player no coonect ");
        return -1;
    }

    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = error;

    uint32_t len = sizeof(ProtoClientHead), body_size = 0;
    if (msg_proto != 0)
    {
    	body_size = msg_proto->ByteSize();
    	len += body_size;
    }

    Block_Buffer *pbuff = this->pop_block(sid);
    pbuff->ensure_writable_bytes(len + sizeof(int32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + body_size);
    }

    return this->client_sender(sid)->push_pool_block_with_len(pbuff);
}

int ChatMonitor::dispatch_to_client(ChatPlayer *player, Block_Buffer *buff)
{
    if (player->client_sid() <= 0)
    {
        MSG_USER("ERROR chat player no coonect %ld", player->role_id());
        return -1;
    }

    return this->client_sender(player->client_sid())->push_data_block_with_len(player->client_sid(), *buff);
}

int ChatMonitor::dispatch_to_client(ChatPlayer *player, const int recogn, const int error)
{
    if (player->client_sid() <= 0)
    {
        MSG_USER("ERROR chat player no coonect %ld", player->role_id());
        return -1;
    }
    return this->dispatch_to_client(player->client_sid(), recogn, error);
}

int ChatMonitor::dispatch_to_client(ChatPlayer *player, const Message *msg_proto, const int error)
{
    if (player->client_sid() <= 0)
    {
        MSG_USER("ERROR chat player no coonect %ld", player->role_id());
        return -1;
    }

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    uint32_t len = sizeof(ProtoClientHead), byte_size = msg_proto->ByteSize();
    len += byte_size;

    ProtoClientHead head;
    head.reset();
    head.__error = error;
    head.__recogn = recogn;
    
    Block_Buffer *pbuff = this->pop_block(player->client_sid());
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(player->client_sid());
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));
    msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
    pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);

    return this->client_sender(player->client_sid())->push_pool_block_with_len(pbuff);
}

// 此接口没效，聊天只能发消息到日志进程，不能发消息到其他进程
//int ChatMonitor::dispatch_to_scene(ProtoHead *head, const Message *msg_proto)
//{
//    int sid = this->fetch_sid_of_scene(head->__scene_id);
//    if (sid < 0)
//        return -1;
//
//    uint32_t len = sizeof(ProtoHead), byte_size = 0;
//    if (msg_proto != 0)
//        byte_size = msg_proto->ByteSize();
//
//    len += byte_size;
//    Block_Buffer *pbuff = this->pop_block(sid);
//    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
//    pbuff->write_int32(sid);
//    pbuff->write_uint32(len);
//    pbuff->copy((char *)head, sizeof(ProtoHead));
//    if (msg_proto != 0)
//    {
//        msg_proto->SerializeToArray(pbuff->get_write_ptr(), byte_size);
//        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
//    }
//
//    int ret = this->connect_sender()->push_pool_block_with_len(pbuff);
//    if (ret != 0)
//        this->push_block(pbuff);
//    return ret;
//}

Int64 ChatMonitor::generate_flaunt_record(Proto30200117* proto)
{
	FlauntRecord* record = this->flaunt_pool()->pop();
	JUDGE_RETURN(record != NULL, -1);

	record->__flaunt_id   = this->flaunt_map_.size() + 1;
	record->__flaunt_type = proto->flaunt_type();
	record->__len = proto->msg().length();
	record->__buffer->copy(proto->msg().c_str(), proto->msg().length());

//	MSG_DEBUG(store proto30200117: %s, proto->Utf8DebugString().c_str());
//	MSG_DEBUG(len: %d, record->__len);

	this->flaunt_map_.bind(record->__flaunt_id, record);
	return record->__flaunt_id;
}

ChatMonitor::FlauntRecordPool *ChatMonitor::flaunt_pool(void)
{
	return &(this->flaunt_record_pool_);
}

int ChatMonitor::bind_flaunt_record(const int64_t flaunt_id, FlauntRecord *flaunt)
{
	return this->flaunt_map_.bind(flaunt_id, flaunt);
}

int ChatMonitor::unbind_flaunt_record(const int64_t flaunt_id)
{
	return this->flaunt_map_.unbind(flaunt_id);
}

int ChatMonitor::find_flaunt_record(const int64_t flaunt_id, FlauntRecord *&flaunt)
{
	return this->flaunt_map_.find(flaunt_id, flaunt);
}

ChatMonitor::ChatTimesPool *ChatMonitor::chat_times_pool(void)
{
	return &(this->chat_times_pool_);
}

int ChatMonitor::add_player_chat_times(Int64 role_id, int channel_id)
{
	if (this->chat_times_map_.count(role_id) <= 0)
	{
		ChatTimes *chat_times = this->chat_times_pool()->pop();
		JUDGE_RETURN(chat_times != NULL, -1);

		chat_times->role_id_ = role_id;
		this->chat_times_map_.insert(ChatTimesMap::value_type(role_id, chat_times));
	}

	ChatTimes *chat_times = this->chat_times_map_[role_id];
	JUDGE_RETURN(chat_times != NULL, -1);

	++(chat_times->channel_times_[channel_id]);
	return 0;
}

ChatTimes *ChatMonitor::fetch_player_chat_times(Int64 role_id)
{
	JUDGE_RETURN(this->chat_times_map_.count(role_id) > 0, NULL);
	return this->chat_times_map_[role_id];
}

int ChatMonitor::reset_player_chat_times()
{
	for (ChatTimesMap::iterator iter = this->chat_times_map_.begin();
			iter != this->chat_times_map_.end(); ++iter)
	{
		ChatTimes *chat_times = iter->second;
		JUDGE_CONTINUE(chat_times != NULL);

		chat_times->channel_times_.clear();
	}
	this->vip_times_timer_.cancel_timer();
	JUDGE_RETURN(this->vip_limit_.time_ > 0, 0);

	this->vip_times_timer_.schedule_timer(this->vip_limit_.time_);
	return 0;
}

SessionManager *ChatMonitor::session_manager(void)
{
    return this->session_manager_;
}

ChatMonitor::ChatPlayerPool *ChatMonitor::player_pool(void)
{
    return &(this->player_pool_);
}

int ChatMonitor::bind_player(const int64_t role_id, ChatPlayer *player)
{
    return this->player_map_.rebind(role_id, player);
}

int ChatMonitor::unbind_player(const int64_t role_id)
{
    return this->player_map_.unbind(role_id);
}

int ChatMonitor::find_player(const int64_t role_id, ChatPlayer *&player)
{
    return this->player_map_.find(role_id, player);
}

int ChatMonitor::client_login_chat(const int client_sid, Message *msg_proto)
{
    ChatClientService *svc = 0;
    if (this->find_client_service(client_sid, svc) != 0)
    {
    	MSG_USER("no find sid %d", client_sid);
        return -1;
    }

    Proto10200001 *request = dynamic_cast<Proto10200001 *>(msg_proto);
    if (request == 0)
    {
        svc->handle_close();
        return -1;
    }

    int64_t role_id = request->role_id();
    MSG_USER("client login chat %d, %ld, %s", client_sid, role_id, request->account().c_str());

    ChatPlayer *player = 0;
    if (this->find_player(role_id, player) != 0)
    {
    	MSG_USER("no find player %ld", role_id);
        svc->handle_close();
        return -1;
    }

    if (request->account() != player->account())
    {
        svc->handle_close();
        return -1;
    }

    SessionDetail* session = 0;
    if (SESSION_MANAGER->find_account_session(request->account(), session) != 0)
    {
        MSG_USER("ERROR chat check session null %ld %s", role_id, request->account().c_str());
#ifndef LOCAL_DEBUG
        svc->handle_close();
        return -1;
#endif
    }

    if (session->__session != request->session())
    {
        MSG_USER("ERROR chat check session %ld %s %s %s", role_id, request->account().c_str(),
        		session->__session.c_str(), request->session().c_str());
#ifndef LOCAL_DEBUG
        svc->handle_close();
        return -1;
#endif
    }

    ChatPlayer* error_player = NULL;
    if (this->find_sid_player(client_sid, error_player) == 0)
	{
		MSG_USER("error chat client sid %d, %ld, %s %d same: %d", client_sid,
				error_player->role_id(), error_player->name(),
				error_player->client_sid(), error_player == player);

	    if (error_player == player)
	    {
	    	return -1;
	    }

	    error_player->sign_out();
    }

    player->set_client_sid(client_sid);
    this->bind_sid_player(client_sid, player);
    this->channel_agency()->sign_in(player);
    return 0;
}

int ChatMonitor::process_init_scene(const int scene_id, const int config_index, const int space_id)
{
    return 0;
}

int ChatMonitor::bind_sid_player(const int client_sid, ChatPlayer *player)
{
    return this->sid_player_map_.rebind(client_sid, player);
}

int ChatMonitor::unbind_sid_player(const int client_sid)
{
    return this->sid_player_map_.unbind(client_sid);
}

int ChatMonitor::find_sid_player(const int client_sid, ChatPlayer *&player)
{
    return this->sid_player_map_.find(client_sid, player);
}

int ChatMonitor::announce_world(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200123*, request, -1);

	ChannelAgency* agency = this->channel_agency();
	JUDGE_RETURN(agency != NULL, -1);

	const ProtoBrocastNewInfo& brocast_info = request->brocast_info();
	BaseChannel* chan = agency->fetch_channel_by_groud_id(brocast_info.group_id(),
			brocast_info.channel_type());
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord* record = agency->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	agency->brocast_serial_record(record, &brocast_info);
	return chan->send_record(record);
}

int ChatMonitor::back_stage_push_system_announce(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200125*, request, -1);

	ChannelAgency* agency = this->channel_agency();
	JUDGE_RETURN(agency != NULL, -1);

	int type = request->type();
	BaseChannel* chan = agency->fetch_channel_by_type(NULL, CHANNEL_WORLD);
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord* record = agency->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	const string& content = request->content();
	agency->backstage_brocast_serial_record(record, content, type);
	return chan->send_record(record);
}

void ChatMonitor::report_pool_info(void)
{
    std::ostringstream msg_stream;
    msg_stream << "Pool Info:" << std::endl;

	POOL_MONITOR->report_pool_info(msg_stream);

    msg_stream << "ClientServicePool:" << std::endl;
    this->client_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Inner Service Pool:" << std::endl;
    this->inner_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Connect Service Pool:" << std::endl;
    this->connect_monitor_.service_pool()->dump_info_to_stream(msg_stream);

    msg_stream << "SessionDetailPool:" << std::endl;
    this->session_manager()->session_pool()->dump_info_to_stream(msg_stream);

    msg_stream << "ChatPlayerPool:" << std::endl;
    this->player_pool()->dump_info_to_stream(msg_stream);

    MSG_USER("%s", msg_stream.str().c_str());
}

ChannelAgency* ChatMonitor::channel_agency(void)
{
	return this->channel_agency_;
}

int ChatMonitor::after_db_opera_reset_base_channel(int channel_type, Int64 channel_id)
{
	BaseChannel* base = this->channel_agency()->fetch_channel_by_type(NULL, channel_type);
	JUDGE_RETURN(base != NULL, -1);

	base->reset();
	return 0;
}

int ChatMonitor::request_load_chat_limit()
{
	return this->db_load_mode_begin(TRANS_LOAD_CHAT_LIMIT);
}

int ChatMonitor::after_load_chat_limit(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL, -1);

	for (BSONVec::iterator iter = shop_mode->output_argv_.bson_vec_->begin();
			iter != shop_mode->output_argv_.bson_vec_->end(); ++iter)
	{
		BSONObj res = *iter;
		JUDGE_CONTINUE(res.isEmpty() == false);

		int channel_type = res[DBChatLimit::ID].numberInt();
		Int64 update_tick = res[DBChatLimit::UPDATE_TICK].numberLong();

		ChatLimit &chat_Limit = this->limit_map_[channel_type];
		JUDGE_CONTINUE(update_tick > chat_Limit.update_tick_);

		chat_Limit.channel_type_  = channel_type;
		chat_Limit.update_tick_   = update_tick;
		chat_Limit.limit_level_   = res[DBChatLimit::LIMIT_LEVEL].numberInt();
		chat_Limit.chat_interval_ = res[DBChatLimit::CHAT_INTREVAL].numberInt();
	}

	return 0;
}

int ChatMonitor::after_load_word_check(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL,-1);

	for (BSONVec::iterator iter = shop_mode->output_argv_.bson_vec_->begin();
			iter != shop_mode->output_argv_.bson_vec_->end();++iter)
	{
		BSONObj res = *iter;
		JUDGE_CONTINUE(res.isEmpty() == false);

		Int64 tick = res[DBWordCheck::TIME].numberLong();
		JUDGE_CONTINUE(this->workcheck_.update_tick_ != tick);

		this->workcheck_.words_.clear();
		this->workcheck_.words_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
		BSONObjIterator iter = res.getObjectField(DBWordCheck::LIST.c_str());

		while (iter.more())
		{
			this->workcheck_.words_.push_back(iter.next().str());
		}
		this->workcheck_.update_tick_= res[DBWordCheck::TIME].numberLong();
	}

	return 0;
}

int ChatMonitor::request_load_word_check()
{
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, 0);

	shop_mode->recogn_ = TRANS_LOAD_WORDS_CHECK;
	shop_mode->input_argv_.type_int64_ = this->workcheck_.update_tick_;
	return this->db_load_mode_begin(shop_mode);
}

int ChatMonitor::request_load_vip_limit()
{
	DBShopMode* shop_mode = GameCommon::pop_shop_mode();
	JUDGE_RETURN(shop_mode != NULL, 0);

	shop_mode->recogn_ = TRANS_LOAD_VIP_CHAT_LIMIT;
	shop_mode->input_argv_.type_int64_ = this->vip_limit_.update_tick_;
	return this->db_load_mode_begin(shop_mode);
}

int ChatMonitor::after_load_vip_check(DBShopMode* shop_mode)
{
	JUDGE_RETURN(shop_mode != NULL,-1);

	BSONObj& res = *shop_mode->output_argv_.bson_obj_;
	JUDGE_RETURN(res.isEmpty() == false, -1);

	this->vip_limit_.update_tick_ = res[DBVipChat::UPDATE_TICK].numberLong();
	this->vip_limit_.time_ = res[DBVipChat::TIME].numberInt();

	this->vip_limit_.vip_map_.clear();
	BSONObjIterator iter = res.getObjectField(DBVipChat::DETAIL.c_str());
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();
		int vip_lv = obj[DBVipChat::VipLimit::VIP_LV].numberInt();
		VipLimit::VipTimes &vip_times = this->vip_limit_.vip_map_[vip_lv];
		vip_times.vip_lv_ = vip_lv;

		GameCommon::bson_to_map(vip_times.channel_times_map_,
			 obj.getObjectField(DBVipChat::VipLimit::INFO.c_str()));
	}
	JUDGE_RETURN(this->vip_limit_.time_ > 0, 0);

	if (this->vip_times_timer_.is_registered() == false)
	{
		this->vip_times_timer_.schedule_timer(this->vip_limit_.time_);
	}

	return 0;
}

ChatLimit* ChatMonitor::fetch_chat_limit(int channel_type)
{
	JUDGE_RETURN(this->limit_map_.count(channel_type) > 0, NULL);
	return &(this->limit_map_[channel_type]);
}

VipLimit& ChatMonitor::fetch_vip_limit()
{
	return this->vip_limit_;
}

WordCheck &ChatMonitor::word_check()
{
	return this->workcheck_;
}

int ChatMonitor::db_load_mode_begin(int trans_recogn, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(trans_recogn, this->logic_unit(), role_id);
}

int ChatMonitor::db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(shop_mode, this->logic_unit(), role_id);
}

int ChatMonitor::db_load_mode_done(Transaction* trans)
{
	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
	JUDGE_RETURN(trans_data != NULL, -1);

	DBShopMode* shop_mode = trans_data->__data.__shop_mode;
	switch(shop_mode->recogn_)
	{
	case TRANS_LOAD_CHAT_LIMIT:
	{
		this->after_load_chat_limit(shop_mode);
		break;
	}
	case TRANS_LOAD_WORDS_CHECK:
	{
		this->after_load_word_check(shop_mode);
		break;
	}
	case TRANS_LOAD_VIP_CHAT_LIMIT:
	{
		this->after_load_vip_check(shop_mode);
		break;
	}
	default:
	{
		break;
	}
	}

	return trans->summit();
}

