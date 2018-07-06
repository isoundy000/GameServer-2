/*
 * ChannelAgency.cpp
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#include "ChannelAgency.h"
#include "ChatMonitor.h"
#include "ProtoDefine.h"
#include "WorldChannel.h"
#include "NearbyChannel.h"
#include "LeagueChannel.h"
#include "TeamChannel.h"
#include "LoudSpeaker.h"
#include "PrivateChat.h"
#include "TransactionMonitor.h"
#include "MongoConnector.h"
#include "SerialRecord.h"
#include "GameCommon.h"

#define CHANNEL_TIME_INTERVAL 0.1*1000000

ChannelAgency::ChannelAgency(ChatMonitor *monitor):
	league_channel_map_(get_hash_table_size(2000)),
	nearby_channel_map_(get_hash_table_size(300)),
	team_channel_map_(get_hash_table_size(3000)),
	private_channel_map_(get_hash_table_size(6000))
{
    this->monitor_ = monitor;
    this->world_channel_ = 0;
    this->trvl_channel_ = 0;
    this->system_channel_ = 0;
    this->loud_speaker_ = 0;
    this->global_voice_id_ = 0;
    this->latest_save_time_ = 0;
}

ChannelAgency::~ChannelAgency()
{
	SAFE_DELETE(this->world_channel_);
	SAFE_DELETE(this->system_channel_);
	SAFE_DELETE(this->loud_speaker_);

    this->league_channel_map_.clear();
    this->nearby_channel_map_.clear();
    this->team_channel_map_.clear();
    this->private_channel_map_.clear();

    this->league_channel_pool_.clear();
    this->nearby_channel_pool_.clear();
    this->team_channel_pool_.clear();
    this->private_channel_pool_.clear();
    this->chat_record_pool_.clear();
}

int64_t ChannelAgency::generate_voice_id(void)
{
	GUARD(Thread_Mutex,lock,this->mutex_);
	++this->global_voice_id_;
	if (this->global_voice_id_ > 10000000)
		this->global_voice_id_ = 1;
	return this->global_voice_id_;
}
int64_t ChannelAgency::max_voice_id(void)
{
	GUARD(Thread_Mutex,lock,this->mutex_);
	return this->global_voice_id_;
}

BaseChannel* ChannelAgency::fetch_channel_by_type(ChatPlayer* player, int type)
{
	BaseChannel* chan = NULL;
	switch (type)
	{
	case CHANNEL_WORLD:
	{
		chan = this->world_channel_;
		break;
	}
	case CHANNEL_SYSTEM:
	{
		chan = this->system_channel_;
		break;
	}
	case CHANNEL_SCENE:
	{
		JUDGE_RETURN(player != NULL, NULL);
		chan = this->find_nearby_channel(player->scene_id());
		break;
	}
	case CHANNEL_LEAGUE:
	{
		JUDGE_RETURN(player != NULL, NULL);
		chan = this->find_league_channel(player->league_id());
		break;
	}
	case CHANNEL_TEAM:
	{
		JUDGE_RETURN(player != NULL, NULL);
		chan = this->find_team_channel(player->team_id());
		break;
	}
	case CHANNEL_TRAVEL:
	{
		JUDGE_RETURN(player != NULL, NULL);
		chan = this->trvl_channel_;
		break;
	}
	}

	return chan;
}

BaseChannel* ChannelAgency::fetch_channel_by_groud_id(Int64 group_id, int type)
{
	BaseChannel* chan = NULL;
	switch (type)
	{
	case CHANNEL_WORLD:
	{
		chan = this->world_channel_;
		break;
	}
	case CHANNEL_SYSTEM:
	{
		chan = this->system_channel_;
		break;
	}
	case CHANNEL_SCENE:
	{
		chan = this->find_nearby_channel(group_id);
		break;
	}
	case CHANNEL_LEAGUE:
	{
		chan = this->find_league_channel(group_id);
		break;
	}
	case CHANNEL_TEAM:
	{
		chan = this->find_team_channel(group_id);
		break;
	}
	case CHANNEL_TRAVEL:
	{
		chan = this->trvl_channel_;
	}
	}

	return chan;
}

int ChannelAgency::notify_league_offline_message(ChatPlayer *player)
{
	LeagueChannel* chan = this->find_league_channel(player->league_id());
	JUDGE_RETURN(chan != NULL, -1);
	chan->notify_offline(player);
	return 0;
}

int ChannelAgency::init(void)
{
    MongoConnector connector;
    int64_t voice_id = connector.load_global_voice_id();
	this->global_voice_id_ = voice_id;

	this->world_channel_ = new WorldChannel(CHANNEL_WORLD);
	this->world_channel_->init(this);

	this->trvl_channel_ = new WorldChannel(CHANNEL_TRAVEL);
	this->trvl_channel_->init(this);

	this->system_channel_ = new SystemChannel();
	this->system_channel_->init(this);

	this->loud_speaker_ = new LoudSpeaker();
	this->loud_speaker_->init(this);
	this->latest_save_time_ = Time_Value::gettimeofday().sec();
	return 0;
}

int ChannelAgency::start(void)
{
	this->world_channel_->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	this->trvl_channel_->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	this->system_channel_->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	this->loud_speaker_->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	return 0;

//	//请求数据库线程加载数据
//	return TRANSACTION_MONITOR->request_mongo_transaction(1,TRANS_CHAT_AGENCY_INIT,
//				DB_CHAT_AGENCY,this,NULL);
}

int ChannelAgency::stop(void)
{
    MongoConnector connector;
    connector.save_global_voice_id(this->generate_voice_id());

	this->world_channel_->stop();
	this->system_channel_->stop();
	this->loud_speaker_->stop();

	LeagueChannelMap::iterator liter = this->league_channel_map_.begin();
	for(;liter!=this->league_channel_map_.end();liter++)
	{
		liter->second->stop();
	}

	PrivateChatMap::iterator piter = this->private_channel_map_.begin();
	for (; piter != this->private_channel_map_.end(); piter++)
	{
		piter->second->stop();
	}

	return 0;
}

int ChannelAgency::sign_in(ChatPlayer *player)
{
	this->register_to_all_channel(player);
	this->notify_league_offline_message(player);
	this->create_private_channel(player->role_id());

	PrivateChat *pChat = this->find_private_channel(player->role_id());
	if (NULL!=pChat && pChat->is_suspend())
	{
		pChat->notify_offline();
		pChat->restart();
	}

	return 0;
}

int ChannelAgency::sign_out(ChatPlayer *player)
{
	this->unregister_from_all_channel(player);
	this->del_private_channel(player->role_id());
	return 0;
}

BaseUnit *ChannelAgency::logic_unit(void)
{
	return this->monitor_->logic_unit();
}

Block_Buffer *ChannelAgency::pop_block(void)
{
	return this->monitor_->pop_block();
}

int ChannelAgency::push_block(Block_Buffer *buff, int cid)
{
	return this->monitor_->push_block(buff,cid);
}

ChatRecord *ChannelAgency::pop_record(void)
{
	return this->chat_record_pool_.pop();
}

int ChannelAgency::push_record(ChatRecord *record)
{
	return this->chat_record_pool_.push(record);
}

ChannelAgency::LeagueChannelPool &ChannelAgency::league_channel_pool(void)
{
	return this->league_channel_pool_;
}

ChannelAgency::ChatRecordPool &ChannelAgency::chat_record_pool(void)
{
	return this->chat_record_pool_;
}

int ChannelAgency::register_to_all_channel(ChatPlayer *player)
{
	this->register_to_world_channel(player);
	this->register_to_system_channel(player);
	this->register_to_nearby_channel(player);
	this->register_to_league_channel(player);
	this->register_to_team_channel(player);
	this->register_to_loudspeaker(player);
    this->register_to_travel_channel(player);
	return 0;
}

int ChannelAgency::register_to_world_channel(ChatPlayer *player)
{
	return this->world_channel_->bind_sid(player->client_sid());
}

int ChannelAgency::register_to_system_channel(ChatPlayer *player)
{
	return this->system_channel_->bind_sid(player->client_sid());
}

int ChannelAgency::register_to_nearby_channel(ChatPlayer *player)
{
	NearbyChannel *near = this->find_nearby_channel(player->scene_id());
	if (NULL == near)
	{
		this->create_nearby_channel(player->scene_id());
		near = this->find_nearby_channel(player->scene_id());
	}
	return near->bind_sid(player->client_sid());
}

int ChannelAgency::register_to_league_channel(ChatPlayer *player)
{
	LeagueChannel *league = this->find_league_channel(player->league_id());
	if (NULL != league && league->channel_id() == player->league_id())
	{
		return league->bind_sid(player->client_sid());
	}
	return -1;
}

int ChannelAgency::register_to_team_channel(ChatPlayer *player)
{
	TeamChannel *team = this->find_team_channel(player->team_id());
	if(NULL!=team && team->channel_id() == player->team_id())
	{
		return team->bind_sid(player->client_sid());
	}
	return -1;
}

int ChannelAgency::register_to_loudspeaker(ChatPlayer *player)
{
	return this->loud_speaker_->bind_sid(player->client_sid());
}

int ChannelAgency::register_to_travel_channel(ChatPlayer *player)
{
    JUDGE_RETURN(GameCommon::is_travel_scene(player->scene_id()) == true, -1);

    WorldChannel *chan = this->find_travel_channel(player->travel_area_id());
    if (NULL != chan)
    {
        return chan->bind_sid(player->client_sid());
    }

    return -1;
}

int ChannelAgency::unregister_from_all_channel(ChatPlayer *player)
{
	this->unregister_from_world_channel(player);
	this->unregister_from_system_channel(player);
	this->unregister_from_nearby_channel(player);
	this->unregister_from_league_channel(player);
	this->unregister_from_team_channel(player);
	this->unregister_from_loudspeaker(player);
    this->unregister_from_travel_channel(player);
	return 0;
}

int ChannelAgency::unregister_from_world_channel(ChatPlayer *player)
{
	return this->world_channel_->unbind_sid(player->client_sid());
}

int ChannelAgency::unregister_from_system_channel(ChatPlayer *player)
{
	return this->system_channel_->unbind_sid(player->client_sid());
}

int ChannelAgency::unregister_from_nearby_channel(ChatPlayer *player)
{
	NearbyChannel *near = this->find_nearby_channel(player->scene_id());
	JUDGE_RETURN(near != NULL, -1);
	return near->unbind_sid(player->client_sid());
}

int ChannelAgency::unregister_from_league_channel(ChatPlayer *player)
{
	LeagueChannel *league = this->find_league_channel(player->league_id());
	if (NULL != league)
	{
//		MSG_DEBUG(unregister_sid : %d, player->client_sid());
		return league->unbind_sid(player->client_sid());
	}
	return -1;
}

int ChannelAgency::unregister_from_team_channel(ChatPlayer *player)
{
	TeamChannel *team = this->find_team_channel(player->team_id());
	if(NULL!=team)
	{
		return team->unbind_sid(player->client_sid());
	}
	return -1;
}

int ChannelAgency::unregister_from_loudspeaker(ChatPlayer *player)
{
	return this->loud_speaker_->unbind_sid(player->client_sid());
}

int ChannelAgency::unregister_from_travel_channel(ChatPlayer *player)
{
	WorldChannel *chan = this->find_travel_channel(player->travel_area_id());
    if (NULL != chan)
    {
        chan->unbind_sid(player->client_sid());
        return 0;
    }

    return -1;
}

int ChannelAgency::chat_private(ChatPlayer *player,Proto10200003 *proto, bool trick/*=false*/)
{
	if(trick == true)
	{
		return this->trick_player_chat_in_private_channel(player, proto);
	}

	PrivateChat *pChat=this->find_private_channel(player->role_id());
	JUDGE_RETURN(pChat != NULL, -1);

	if(pChat->is_suspend())
	{
		pChat->restart();
	}

	ChatRecord *record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	record->__src_role_id = player->role_id();
	record->__dst_role_id = proto->dst_role_id();
	record->__type = proto->type();
	record->__voice_len = proto->voice_len();

	PrivateChat *dst_chat=this->find_private_channel(proto->dst_role_id());
	if(NULL==dst_chat)
	{
		this->create_private_channel(proto->dst_role_id(),true);
		dst_chat = this->find_private_channel(proto->dst_role_id());
	}
	else if (dst_chat->is_in_black_list(player->role_id()))
	{
		return 0;
	}

	if(record->__type==R_VOICE)//语音内容另外保存
	{
		record->__voice_id = this->generate_voice_id();
		ChatRecord voice;// = this->pop_record();
		voice.copy(record);
		voice.__type = R_VOICE_SAVE;
		voice.__buffer->copy(proto->vcontent());
		pChat->push_voice(&voice);//保存到自己
		dst_chat->push_voice(&voice);//保存到对方

		MSG_USER("private voice %ld -> %ld id[%ld] %d %d", player->role_id(), proto->dst_role_id(),
				record->__voice_id, proto->voice_len(), voice.__buffer->readable_bytes());
	}

	SERIAL_RECORD->record_chat(player->role_id(), CHANNEL_PRIVATE, ::time(NULL),
			proto->wcontent(), player->role_detail().__agent_code,
			player->role_detail().__server_flag, player->role_detail().__market_code);

	//增加频道聊天次数
	CHAT_MONITOR->add_player_chat_times(player->role_id(), CHANNEL_PRIVATE);

	this->serial_record(CHANNEL_PRIVATE, player, record, proto->wcontent());
	return pChat->send_record(record);
}

int ChannelAgency::chat_loudspeaker(ChatPlayer *player, const string& content, bool trick/*=false*/)
{
	ChatRecord *record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	record->__type = R_WORD;
	record->__src_role_id = player->role_id();

	this->serial_record(CHANNEL_LOUDSPEAKER, player,record,content);

	if(trick == true)
		return this->display_trick_chat_message_to_sender(ACTIVE_CHAT_NORMAL, record);

	{
		ChatRecord *world_rec = this->pop_record();
		JUDGE_RETURN(world_rec != NULL, -1);

		world_rec->__type = R_WORD;
		world_rec->__src_role_id = player->role_id();

		this->serial_record(CHANNEL_LOUDSPEAKER, player, world_rec, content);

		BaseChannel *world_channel = this->fetch_channel_by_type(player, CHANNEL_WORLD);
		if (world_channel != NULL)
			world_channel->send_record(world_rec);
	}

	SERIAL_RECORD->record_chat(player->role_id(), CHANNEL_LOUDSPEAKER, ::time(NULL), content,
			player->role_detail().__agent_code, player->role_detail().__server_flag, player->role_detail().__market_code);

	return this->loud_speaker_->send_record(record);
}

int ChannelAgency::trick_player_chat_in_normal_channel(int type, ChatPlayer *player, Proto10200002 *proto)
{
	JUDGE_RETURN((player != 0 && proto != 0), -1);
	BaseChannel* chan = this->fetch_channel_by_type(player, type);
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord record;
	record.__type = proto->type();
	record.__voice_len = proto->voice_len();

	int client_channel = this->fetch_client_channel(type);

	this->serial_record(client_channel, player, &record, proto->wcontent());
	return this->display_trick_chat_message_to_sender(ACTIVE_CHAT_NORMAL, &record);
}

int ChannelAgency::trick_player_chat_in_private_channel(ChatPlayer *player, Proto10200003 *proto)
{
	JUDGE_RETURN((player != 0 && proto != 0), -1);

	PrivateChat *pChat=this->find_private_channel(player->role_id());
	JUDGE_RETURN(pChat != NULL, -1);

	ChatRecord record;
	record.__type = proto->type();
	record.__voice_len = proto->voice_len();
	record.__dst_role_id = proto->dst_role_id();

	PrivateChat *dst_chat=this->find_private_channel(proto->dst_role_id());
	if(NULL==dst_chat)
	{
		this->create_private_channel(proto->dst_role_id(),true);
		dst_chat = this->find_private_channel(proto->dst_role_id());
	}
	else if (dst_chat->is_in_black_list(player->role_id()))
	{
		return 0;
	}

	if(record.__type==R_VOICE)//语音内容另外保存
	{
		record.__voice_id = this->generate_voice_id();
		ChatRecord voice;// = this->pop_record();
		voice.copy(&record);
		voice.__type = R_VOICE_SAVE;
		voice.__buffer->copy(proto->vcontent());
		pChat->push_voice(&voice);//保存到自己
	}

	this->serial_record(CHANNEL_PRIVATE, player, &record, proto->wcontent());
	return this->display_trick_chat_message_to_sender(ACTIVE_CHAT_PRIVATE, &record);
}

int ChannelAgency::trick_flaunt(int channel, Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto)
{
	JUDGE_RETURN((player != 0 && proto != 0), -1);

	BaseChannel* chan = NULL;
	int recogn = ACTIVE_CHAT_NORMAL;
	ChatRecord record;

	switch(channel)
	{
	case CHANNEL_WORLD:
	case CHANNEL_LEAGUE:
	case CHANNEL_TEAM:
	case CHANNEL_SCENE:
	{
		recogn = ACTIVE_CHAT_NORMAL;
		chan = this->fetch_channel_by_type(player, channel);
		JUDGE_RETURN(chan != NULL, -1);
		break;
	}

	case CHANNEL_PRIVATE:
	{
		recogn = ACTIVE_CHAT_PRIVATE;
		PrivateChat *dst_chat=this->find_private_channel(proto->role_id());
		if(0 == dst_chat)
		{
			this->create_private_channel(proto->role_id(),true);
			dst_chat = this->find_private_channel(proto->role_id());
		}

		if( dst_chat && dst_chat->is_in_black_list(player->role_id()))
		{
			return -1;
		}

		record.__dst_role_id = proto->role_id();
		break;
	}

	default:
		recogn = ACTIVE_CHAT_NORMAL;
		break;
	}

	this->serial_flaunt(flaunt_id, player, &record, proto);
	return this->display_trick_chat_message_to_sender(recogn, &record);
}

int ChannelAgency::fetch_client_channel(const int channel_type)
{
	return channel_type;
}

int ChannelAgency::chat_in_normal_channel(int type, ChatPlayer *player, Proto10200002 *proto, bool trick/*=false*/)
{
	int client_channel = this->fetch_client_channel(type);
	if (trick == true)
	{
		return this->trick_player_chat_in_normal_channel(type, player, proto);
	}

	BaseChannel* chan = this->fetch_channel_by_type(player, type);
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord* record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	record->__type = proto->type();
	record->__voice_len = proto->voice_len();

	if (record->__type == R_VOICE)
	{
		record->__voice_id = this->generate_voice_id();
		ChatRecord voice ;//= this->pop_record();
		voice.copy(record);
		voice.__type = R_VOICE_SAVE;
		voice.__buffer->copy(proto->vcontent());
		chan->push_voice(&voice);
	}

	// 暂时只记录文字消息
	if (type != CHANNEL_TRAVEL)
	{
		SERIAL_RECORD->record_chat(player->role_id(), type, ::time(NULL), proto->wcontent(),
				player->role_detail().__agent_code, player->role_detail().__server_flag,
				player->role_detail().__market_code);
	}

	//增加频道聊天次数
	CHAT_MONITOR->add_player_chat_times(player->role_id(), type);

	this->serial_record(client_channel,player,record, proto->wcontent());
	return chan->send_record(record);
}

void ChannelAgency::serial_flaunt(Int64 flaunt_id, ChatPlayer *player,
		ChatRecord *record, Proto30200117* proto)
{
	if (player != NULL)
	{
		record->__src_role_id = player->role_id();
	}
	else
	{
		record->__src_role_id = 0;
	}
	record->__time = Time_Value::gettimeofday().sec();

	Proto80200003 out;//Proto80200003 和 Proto80200001消息体是一样的
	ProtoChatInfo* info = out.add_content();
	info->set_channel(proto->channel_type());
	if (proto->flaunt_type() == GameEnum::FLAUNT_POSITION)
		info->set_type(R_POSITION);
	else
		info->set_type(R_FLAUNT);
	info->set_voice_id(record->__voice_id);
	info->set_voice_len(record->__voice_len);
	if (player != NULL)
	{
		info->set_role_id(player->role_id());
		info->set_name(player->name());
		info->set_sex(player->role_detail().__sex);
		info->set_level(player->role_detail().__level);
		info->set_vip(player->role_detail().__vip_type);
		info->set_label(player->label_id());
		info->set_team(player->team_id());
		info->set_career(player->role_detail().__career);
		info->set_permission(player->role_detail().__permission);
		info->set_league_name(player->role_detail().__league_name);
	}
	info->set_time(record->__time);
	info->set_target_id(record->__dst_role_id);

	ChatPlayer *chat_player = NULL;
	if (CHAT_MONITOR->find_player(record->__dst_role_id, chat_player) == 0)
	{
		info->set_target_name(chat_player->name());
		info->set_target_sex(chat_player->role_detail().__sex);
		info->set_target_level(chat_player->role_detail().__level);
		info->set_target_vip(chat_player->role_detail().__vip_type);
		info->set_target_label(chat_player->label_id());
		info->set_target_team(chat_player->team_id());
		info->set_target_league_name(chat_player->role_detail().__league_name);
	}

	ProtoFlaunt* proto_flaunt = info->mutable_flaunt_detail();
	proto_flaunt->set_flaunt_id(flaunt_id);
	proto_flaunt->set_color(proto->color());
	proto_flaunt->set_content(proto->show_content());

	uint32_t  byte_size = out.ByteSize();

	Block_Buffer* buffer = record->__buffer;
	buffer->ensure_writable_bytes(byte_size + sizeof(uint32_t) * 4);
	out.SerializeToArray(buffer->get_write_ptr(), buffer->writable_bytes());
	buffer->set_write_idx(buffer->get_write_idx() + byte_size);
}

void ChannelAgency::serial_record(int channel, ChatPlayer *player, ChatRecord *record, const string& content)
{
	if (player != NULL)
	{
		record->__src_role_id = player->role_id();
	}
	else
	{
		record->__src_role_id = 0;
	}
	record->__time = Time_Value::gettimeofday().sec();

	Proto80200003 out;//Proto80200003 和 Proto80200001消息体是一样的
	ProtoChatInfo* info = out.add_content();
	info->set_channel(channel);
	info->set_type(record->__type);
	info->set_voice_id(record->__voice_id);
	info->set_voice_len(record->__voice_len);
	if (player != NULL)
	{
		info->set_role_id(player->role_id());
		info->set_name(player->role_detail().name());
		info->set_sex(player->role_detail().__sex);
		info->set_vip(player->role_detail().__vip_type);
		info->set_level(player->role_detail().__level);
		info->set_label(player->label_id());
		info->set_team(player->team_id());
		info->set_career(player->role_detail().__career);
		info->set_server_prev(player->role_detail().__server_prev);
		info->set_server_id(player->role_detail().__server_id);
		info->set_permission(player->role_detail().__permission);
		info->set_league_name(player->role_detail().__league_name);
	}

	info->set_time(record->__time);
	info->set_target_id(record->__dst_role_id);

	ChatPlayer *chat_player = NULL;
	if (CHAT_MONITOR->find_player(record->__dst_role_id, chat_player) == 0)
	{
		info->set_target_name(chat_player->name());
		info->set_target_sex(chat_player->role_detail().__sex);
		info->set_target_level(chat_player->role_detail().__level);
		info->set_target_vip(chat_player->role_detail().__vip_type);
		info->set_target_label(chat_player->label_id());
		info->set_target_team(chat_player->team_id());
		info->set_target_league_name(chat_player->role_detail().__league_name);
	}

//	if(chinese_utf8_len(content.c_str()) >= MAX_CHAT_CONTENT_LENGTH)
//	{
//		int cut_string_len = string_utf8_len_to_raw_size(content.c_str(), MAX_CHAT_CONTENT_LENGTH);
//		if(cut_string_len > content.length())
//			cut_string_len = content.length();
//		info->set_content(content.c_str(), cut_string_len);
//	}
//
//	else
		info->set_content(content);

	uint32_t  byte_size = out.ByteSize();

	Block_Buffer* buffer = record->__buffer;
	buffer->ensure_writable_bytes(byte_size + sizeof(uint32_t) * 4);
	out.SerializeToArray(buffer->get_write_ptr(), buffer->writable_bytes());
	buffer->set_write_idx(buffer->get_write_idx() + byte_size);
}

void ChannelAgency::brocast_serial_record(ChatRecord* record, const ProtoBrocastNewInfo* brocast_info)
{
	JUDGE_RETURN(record != NULL && brocast_info != NULL, ;);

	record->__type = R_BROCAST_INFO;
	record->__time = Time_Value::gettimeofday().sec();

	Proto80200006 chat_info;
	GameCommon::make_up_chat_broadcast_info(&chat_info, brocast_info);

	Block_Buffer* buffer = record->__buffer;
	uint32_t  byte_size = chat_info.ByteSize();

	buffer->ensure_writable_bytes(byte_size);
	chat_info.SerializeToArray(buffer->get_write_ptr(), buffer->writable_bytes());

	buffer->set_write_idx(buffer->get_write_idx() + byte_size);
}

void ChannelAgency::backstage_brocast_serial_record(ChatRecord *record,
		const std::string& content, int type)
{
	JUDGE_RETURN(record != NULL && content.length() > 0, ;);

	record->__type = R_BACK_STAGE_ANNOUNCE;
	record->__time = Time_Value::gettimeofday().sec();

	Proto80200007 chat_info;
	chat_info.set_content(content);
	chat_info.set_type(type);

	Block_Buffer* buffer = record->__buffer;
	uint32_t  byte_size = chat_info.ByteSize();

	buffer->ensure_writable_bytes(byte_size);
	chat_info.SerializeToArray(buffer->get_write_ptr(), buffer->writable_bytes());

	buffer->set_write_idx(buffer->get_write_idx() + byte_size);
}

int ChannelAgency::create_nearby_channel(int scene_id)
{
	JUDGE_RETURN(this->find_nearby_channel(scene_id) == NULL, -1);

	NearbyChannel *chan = this->nearby_channel_pool_.pop();
	chan->init(this, scene_id);
	chan->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	return this->nearby_channel_map_.rebind(scene_id, chan);
}

LeagueChannel* ChannelAgency::create_league_channel(Int64 league_id)
{
	if (this->find_league_channel(league_id)==NULL)
	{
		LeagueChannel *chan = this->league_channel_pool_.pop();
		chan->init(this, league_id);
		chan->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
		this->league_channel_map_.rebind(league_id, chan);
		return chan;
	}
	return NULL;
}

int ChannelAgency::create_team_channel(int team_id)
{
	if(this->find_team_channel(team_id)==NULL)
	{
		TeamChannel *chan = this->team_channel_pool_.pop();
		chan->init(this, team_id);
		chan->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
		this->team_channel_map_.rebind(team_id, chan);
		return 0;
	}
	return -1;
}

int ChannelAgency::create_private_channel(int64_t role_id,bool is_offline)
{
	PrivateChat *pChat = this->find_private_channel(role_id);
	if (pChat==NULL)
	{
		PrivateChat *chan = this->private_channel_pool_.pop();
		chan->init(this, role_id,is_offline);
		this->private_channel_map_.rebind(role_id, chan);
		return 0;
	}
	return -1;
}

WorldChannel* ChannelAgency::world_channel(void)
{
	return this->world_channel_;
}

SystemChannel* ChannelAgency::system_channel(void)
{
	return this->system_channel_;
}

LoudSpeaker* ChannelAgency::loud_speaker(void)
{
	return this->loud_speaker_;
}

NearbyChannel* ChannelAgency::find_nearby_channel(int scene_id)
{
	NearbyChannel* chan = NULL;
	this->nearby_channel_map_.find(scene_id,chan);
	return chan;
}
LeagueChannel* ChannelAgency::find_league_channel(Int64 league_id)
{
	LeagueChannel* chan=NULL;
	this->league_channel_map_.find(league_id,chan);
	return chan;
}
TeamChannel* ChannelAgency::find_team_channel(int team_id)
{
	TeamChannel* chan =NULL;
	this->team_channel_map_.find(team_id,chan);
	return chan;
}

PrivateChat* ChannelAgency::find_private_channel(int64_t role_id)
{
	PrivateChat* chan = NULL;
	this->private_channel_map_.find(role_id,chan);
	return chan;
}

WorldChannel* ChannelAgency::find_travel_channel(int area_id)
{
	return this->trvl_channel_;
}

int ChannelAgency::del_league_channel(ChatPlayer *player)
{
	LeagueChannel* chan = NULL;
	this->league_channel_map_.find(player->league_id(),chan);
	if(chan!=NULL)
	{
		chan->stop();
		this->league_channel_pool_.push(chan);
		this->league_channel_map_.unbind(player->league_id());
		return 0;
	}
	return -1;
}
int ChannelAgency::del_team_channel(ChatPlayer *player)
{
	TeamChannel* chan = NULL;
	this->team_channel_map_.find(player->team_id(),chan);
	if(chan!=NULL)
	{
		chan->stop();
		this->team_channel_pool_.push(chan);
		this->team_channel_map_.unbind(player->team_id());
		return 0;
	}
	return -1;
}

int ChannelAgency::del_private_channel(int64_t role_id)
{
	PrivateChat *pChat = this->find_private_channel(role_id);
	if(NULL!=pChat)
	{
		if(!pChat->is_suspend())
		{
			pChat->suspend();
		}
		return 0;
	}
	return -1;
}

int ChannelAgency::load_league_history(Int64 league_id)
{
	LeagueChannel *league = this->find_league_channel(league_id);
	if (league == NULL)
	{
		return -1;
	}
	return league->start();
}

int ChannelAgency::load_private_history(int64_t roleid)
{
	PrivateChat *pChat = this->find_private_channel(roleid);
	if (NULL == pChat)
	{
		return -1;
	}

	if(!pChat->is_suspend())
	{
		pChat->notify_offline();
	}
	else
	{
		pChat->import_offline_record();
	}
	return pChat->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
}

int ChannelAgency::load_loudspeaker(void)
{
	this->loud_speaker_->schedule_timer(Time_Value(0, CHANNEL_TIME_INTERVAL));
	return 0;
}
int ChannelAgency::save_loudspeaker(void)
{
	this->loud_speaker_->cancel_timer();
	return 0;
}

int ChannelAgency::channel_notify(int type, ChatRecord *record, bool offline/*=false*/)
{
	int sid = record->__sid;
	ProtoClientHead head;

	switch(type)
	{
	case CHANNEL_PRIVATE:
	{
		ChatPlayer *player = NULL;
		if (this->monitor_->find_player(record->__dst_role_id,player) != 0)
		{
			PrivateChat* dst = this->find_private_channel(record->__dst_role_id);
			JUDGE_RETURN(NULL != dst, 0);

			dst->recv_record(record);
			this->display_private_chat_message_to_sender(record);
			return 0;
		}

		sid = player->client_sid();
		head.__recogn = ACTIVE_CHAT_PRIVATE;

		if(offline == false)
		{
			this->display_private_chat_message_to_sender(record);
		}
		break;
	}
	case CHANNEL_LOUDSPEAKER:
	{
		head.__recogn = ACTIVE_CHAT_LOUDSPEAKER;
		break;
	}
	case CHANNEL_LEAGUE:
	default:
	{
		if (record->__type == R_BROCAST_INFO)
		{
			head.__recogn = ACTIVE_CHAT_NEW_BROAD;
		}
		else if (record->__type == R_BACK_STAGE_ANNOUNCE)
		{
			head.__recogn = ACTIVE_CHAT_BACKSTAGE_BROAD;
		}
		else
		{
			head.__recogn = ACTIVE_CHAT_NORMAL;
		}
		break;
	}
	}

	uint32_t len = sizeof(ProtoClientHead);
	uint32_t byte_size = record->__buffer->readable_bytes();
	len += byte_size;

	Block_Buffer sbuf;
	sbuf.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
	sbuf.write_uint32(len);
	sbuf.copy(&head,sizeof(ProtoClientHead));
	sbuf.copy(record->__buffer);

	return this->monitor_->dispatch_to_client(sid, &sbuf);
}

int ChannelAgency::display_trick_chat_message_to_sender(int recogn, ChatRecord *record)
{
	JUDGE_RETURN(record != NULL, -1);
	ChatPlayer* player = NULL;

	if(this->monitor_->find_player(record->__src_role_id, player) == 0)
	{
		ProtoClientHead head;
		head.__recogn = recogn;

		int sid = player->client_sid();
		uint32_t len = sizeof(ProtoClientHead);
		uint32_t byte_size = record->__buffer->readable_bytes();
		len += byte_size;

		Block_Buffer sbuf;
		sbuf.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
		sbuf.write_uint32(len);
		sbuf.copy(&head,sizeof(ProtoClientHead));
		sbuf.copy(record->__buffer);
		this->monitor_->dispatch_to_client(sid,&sbuf);

		MSG_USER("src_id:%ld - name:%s - sid:%d -record_sid:%d-target_id:%ld-",
				player->role_id(), player->name(), sid, record->__sid, record->__dst_role_id);
	}

	return 0;
}

int ChannelAgency::display_private_chat_message_to_sender(ChatRecord *record)
{
	JUDGE_RETURN(record != NULL, -1);
	ChatPlayer* player = NULL;
	if(this->monitor_->find_player(record->__src_role_id, player) == 0)
	{
		ProtoClientHead head;
		head.__recogn = ACTIVE_CHAT_PRIVATE;

		int sid = player->client_sid();
		uint32_t len = sizeof(ProtoClientHead), byte_size =
				record->__buffer->readable_bytes();
		len += byte_size;

		Block_Buffer sbuf;
		sbuf.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
		sbuf.write_uint32(len);
		sbuf.copy(&head,sizeof(ProtoClientHead));
		sbuf.copy(record->__buffer);
		this->monitor_->dispatch_to_client(sid,&sbuf);

		MSG_USER("src_id:%ld - name:%s - sid:%d -record_sid:%d-target_id:%ld-",
				player->role_id(), player->name(), sid,
				record->__sid, record->__dst_role_id);
	}
	return 0;
}

int ChannelAgency::flaunt_in_normal_chanel(Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto)
{
	BaseChannel* chan = this->fetch_channel_by_type(player, proto->channel_type());
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord* record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	this->serial_flaunt(flaunt_id, player,record, proto);
	return chan->send_record(record);
}

int ChannelAgency::flaunt_private(Int64 flaunt_id, ChatPlayer *player,Proto30200117 *proto)
{
	PrivateChat *pChat=this->find_private_channel(player->role_id());
	JUDGE_RETURN(pChat != NULL, -1);

	if(pChat->is_suspend())
	{
		pChat->restart();
	}

	ChatRecord *record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	record->__dst_role_id = proto->role_id();

	PrivateChat *dst_chat=this->find_private_channel(proto->role_id());
	if(NULL==dst_chat)
	{
		this->create_private_channel(proto->role_id(),true);
		dst_chat = this->find_private_channel(proto->role_id());
	}
	else if (dst_chat->is_in_black_list(player->role_id()))
	{
		return 0;
	}

	this->serial_flaunt(flaunt_id,player,record, proto);
	return pChat->send_record(record);
}

int ChannelAgency::flaunt_loudspeaker(Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto)
{
	ChatRecord *record = this->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	this->serial_flaunt(flaunt_id,player,record, proto);
	return this->loud_speaker_->send_record(record);
}

int ChannelAgency::routine_timeout(void)
{
	long int now = Time_Value::gettimeofday().sec();
	if(now-this->latest_save_time_>=30)
	{
		this->latest_save_time_=now;
		//请求数据库线程
		TRANSACTION_MONITOR->request_mongo_transaction(0,TRANS_CHAT_SAVE_VOICE_ID,
						DB_CHAT_AGENCY,this,NULL);
	}
	return 0;
}
