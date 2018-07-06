/*
 * ChatPlayer.cpp
 *
 * Created on: 2013-03-05 10:31
 *     Author: glendy
 */

#include "ChatPlayer.h"
#include "ChatMonitor.h"
#include "SessionManager.h"
#include "ProtoDefine.h"
#include "ChannelAgency.h"
#include "SerialRecord.h"
#include "GameCommon.h"

ChatPlayer::ChatPlayer(void) : 
    is_active_(false), role_id_(0), client_sid_(0), monitor_(0)
{
    this->monitor_ = CHAT_MONITOR;
}

ChatPlayer::~ChatPlayer(void)
{ /*NULL*/ }

ChatMonitor *ChatPlayer::monitor(void)
{
	return this->monitor_;
}

int ChatPlayer::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
    if (msg_proto == 0)
        return this->monitor()->dispatch_to_client(this, recogn, error);
    return this->monitor()->dispatch_to_client(this, msg_proto, error);
}

int ChatPlayer::respond_to_client(Block_Buffer *buff)
{
    return this->monitor()->dispatch_to_client(this, buff);
}

int64_t ChatPlayer::entity_id(void)
{
    return this->role_id_;
}

int64_t ChatPlayer::role_id(void)
{
    return this->role_id_;
}

int ChatPlayer::role_id_low(void)
{
    return (int32_t)this->role_id_;
}

int ChatPlayer::role_id_high(void)
{
    return (int32_t)(this->role_id_ >> 32);
}

void ChatPlayer::set_client_sid(const int client_sid)
{
    this->client_sid_ = client_sid;
}

int ChatPlayer::client_sid(void)
{
    return this->client_sid_;
}

const char *ChatPlayer::account(void)
{
    return this->role_detail_.__account.c_str();
}

const char* ChatPlayer::name()
{
	return this->role_detail_.name();
}

bool ChatPlayer::is_active(void)
{
    return this->is_active_;
}

void ChatPlayer::reset(void)
{
    this->is_active_ = false;
    this->role_id_ = 0;
    this->client_sid_ = 0;
    this->role_detail_.reset();

    this->last_world_chat_ = Time_Value::zero;
    this->last_nearby_chat_ = Time_Value::zero;
    this->last_league_chat_ = Time_Value::zero;
    this->last_team_chat_ = Time_Value::zero;
    this->last_private_chat_ = Time_Value::zero;

    this->last_flaunt_tick_ = Time_Value::zero;
    this->last_flaunt_position_tick_ = Time_Value::zero;
}

int ChatPlayer::resign(const int64_t role_id, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200101*, request, -1);
    this->role_id_ = role_id;

    string account = request->session_info().account();
    SESSION_MANAGER->update_session(this->client_sid(), account,
    		request->session_info().session(), role_id);
    this->init_role_detail(request);

    MSG_USER("chat resign in %ld %s last_sign_out:%d, %d, sub:%d, sid:%d", this->role_id(),
    		this->name(), this->role_detail_.__last_sign_out, this->scene_id(),
    		Time_Value::gettimeofday().sec() - this->role_detail_.__last_sign_out, this->client_sid());
    return 0;
}

int ChatPlayer::sign_in(const int64_t role_id, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200101*, request, -1);

    this->role_id_ = role_id;
    string account = request->session_info().account();
    SESSION_MANAGER->update_session(this->client_sid(), account,
    		request->session_info().session(), role_id);

    this->init_role_detail(request);
    this->monitor()->bind_player(this->role_id(), this);

    this->is_active_ = true;
    this->role_detail_.__sign_in =  Time_Value::gettimeofday().sec();

    MSG_USER("chat sign in %ld %s last_sign_out:%d, %d, sub:%d, sid:%d %s", this->role_id(),
    		this->name(), this->role_detail_.__last_sign_out, this->scene_id(),
    		Time_Value::gettimeofday().sec() - this->role_detail_.__last_sign_out,
    		this->client_sid(), request->session_info().session().c_str());
    return 0;
}

int ChatPlayer::sign_out(void)
{
    this->is_active_ = false;
    MSG_USER("role close connect %ld %s %s %d", this->role_id(),
    		this->name(), this->account(), this->client_sid());

//    ChatClientService *svc = 0;
//    if (this->monitor()->find_client_service(this->client_sid(), svc) == 0)
//    {
//        svc->handle_close();
//    }

    this->monitor()->unbind_sid_player(this->client_sid());
    this->monitor()->channel_agency()->sign_out(this);

#ifndef LOCAL_DEBUG
    SESSION_MANAGER->unbind_and_push(this->account());
#endif

    this->monitor()->unbind_player(this->role_id());
    this->monitor()->player_pool()->push(this);
    return 0;
}

int ChatPlayer::client_close(void)
{
//	this->monitor()->unbind_sid_player(this->client_sid_);
//	this->monitor()->channel_agency()->sign_out(this);
//	this->client_sid_ = 0;
	return this->sign_out();
}

int ChatPlayer::init_role_detail(Proto30200101 *request)
{
    ChatRoleDetail &detail = this->role_detail();

    detail.set_server_flag(request->server_flag());
    detail.__name = request->name();
    detail.__account = request->session_info().account();
    detail.__level = request->level();
    detail.__sex = request->sex();
    detail.__career = request->career();
    detail.__permission = request->permission();
    detail.__scene_id = request->scene_id();

    detail.__vip_type= request->vip();
    detail.__label_id = request->label_id();
    detail.__league_id = request->league_id();
    detail.__team_id = request->team_id();
    detail.__last_sign_out = request->last_sign_out();
    detail.__agent_code = request->agent_code();
    detail.__market_code = request->market_code();
    detail.__travel_area_id = request->area_id();
    detail.__league_name = request->league_name();

    this->check_league_channel();
    this->check_travel_channel();

    return 0;
}

ChatRoleDetail &ChatPlayer::role_detail(void)
{
    return this->role_detail_;
}

const int ChatPlayer::team_id(void)
{
	return this->role_detail_.__team_id;
}

const Int64 ChatPlayer::league_id(void)
{
	return this->role_detail_.__league_id;
}

const int ChatPlayer::scene_id(void)
{
	return this->role_detail_.__scene_id;
}
const int ChatPlayer::label_id(void)
{
	return this->role_detail_.__label_id;
}
const int ChatPlayer::travel_area_id(void)
{
	return this->role_detail_.__travel_area_id;
}
bool ChatPlayer::is_vip(void)
{
	return this->role_detail_.__vip_type;
}

int ChatPlayer::chat_normal(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10200002*,proto,-1);

	int ret = this->check_chat_channel_level(proto->channel());
	if (ret != 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_NORMAL, ret);
	}

	ret = this->check_chat_vip_times(proto->channel());
	if (ret != 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_NORMAL, ret);
	}

	// 禁言的处理
	int speak_state = this->speak_state();
	if(GameEnum::OPER_BAN_SPEAK == speak_state)
	{
		return this->respond_to_client_error(RETURN_CHAT_NORMAL, ERROR_FORBID_SPEAK);
	}

	bool trick_chat = (GameEnum::OPER_BAN_SPEAK_TRICK == speak_state);
	switch(proto->channel())
	{
	case CHANNEL_WORLD:
	{
		return this->chat_world(proto, trick_chat);
	}
	case CHANNEL_SCENE:
	{
		return this->chat_nearby(proto, trick_chat);
	}
	case CHANNEL_LEAGUE:
	{
		return this->chat_league(proto, trick_chat);
	}
	case CHANNEL_TEAM:
	{
		return this->chat_team(proto, trick_chat);
	}
	case CHANNEL_SYSTEM:
	{
		return this->chat_system(proto, trick_chat);
	}
	case CHANNEL_TRAVEL:
	{
		return this->chat_travel(proto, trick_chat);
	}
	default:
	{
		MSG_USER("ERROR CHANNEL %d", proto->channel());
		break;
	}
	}
	return 0;
}

int ChatPlayer::chat_private(Message *msg_proto)
{
	int ret = this->check_chat_channel_level(CHANNEL_PRIVATE);
	if (ret != 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ret);
	}

	ret = this->check_chat_vip_times(CHANNEL_PRIVATE);
	if (ret != 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ret);
	}

	// 禁言的处理
	int speak_state = this->speak_state();
	if(GameEnum::OPER_BAN_SPEAK == speak_state)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ERROR_FORBID_SPEAK);
	}

	bool trick_chat = (GameEnum::OPER_BAN_SPEAK_TRICK == speak_state);
//	if (Time_Value::gettimeofday().sec() - this->last_private_chat_.sec() <= 1)
//	{
//		return -1;
//	}

	this->last_private_chat_ = Time_Value::gettimeofday();
	DYNAMIC_CAST_RETURN(Proto10200003*,proto,msg_proto,-1);

	//聊天屏蔽
	if (proto->type() == R_WORD)
	{
		string new_word = this->check_workcheck_to_client(proto->wcontent());
		proto->set_wcontent(new_word);
	}

	if (this->check_black_list(proto->dst_role_id()) < 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ERROR_IN_BACK_LIST);
	}

	ChatPlayer *player = NULL;
	if (CHAT_MONITOR->find_player(proto->dst_role_id(), player) != 0)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ERROR_PLAYER_OFFLINE);
	}

	PrivateChat *dst_chat = CHANNEL_AGENCY->find_private_channel(proto->dst_role_id());
	if (dst_chat != NULL && dst_chat->is_in_black_list(this->role_id()) == true)
	{
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ERROR_HAVE_BANNED);
	}

	return CHANNEL_AGENCY->chat_private(this, proto, trick_chat);
}

string ChatPlayer::check_workcheck_to_client(const string& word)
{
	WordCheck& word_check = CHAT_MONITOR->word_check();

	string new_word = word;
	for(StringVec::iterator iter = word_check.words_.begin();
			iter != word_check.words_.end(); ++iter)
	{
		GameCommon::string_hidden_replace(new_word, *iter);
	}

	return new_word;
}


int ChatPlayer::check_chat_channel_level(int channel_type)
{
	int limit_level = 0;
	int chat_interval = 0;
	Int64 last_interval = 0;
	int error_code = 0;
	ChatLimit* chat_limit = CHAT_MONITOR->fetch_chat_limit(channel_type);
	if (chat_limit != NULL)
	{
		limit_level = chat_limit->limit_level_;
		chat_interval = chat_limit->chat_interval_;
	}

	switch(channel_type)
	{
	case CHANNEL_WORLD:
	case CHANNEL_SYSTEM:
	{
		limit_level   = limit_level > 0 ? limit_level : CONFIG_INSTANCE->const_set("chat_world_level");
		chat_interval = chat_interval > 0 ? chat_interval : CONFIG_INSTANCE->const_set("chat_world_interval");
		last_interval = this->last_world_chat_.sec();
		error_code = ERROR_CHAT_NORMAL_LEVEL;
		break;
	}
	case CHANNEL_LEAGUE:
	{
		limit_level = limit_level > 0 ? limit_level : CONFIG_INSTANCE->const_set("chat_league_level");
		chat_interval = chat_interval > 0 ? chat_interval : CONFIG_INSTANCE->const_set("chat_league_interval");
		last_interval = this->last_league_chat_.sec();
		error_code = ERROR_CHAT_LEAGUE_LEVEL;
		break;
	}
	case CHANNEL_TEAM:
	{
		limit_level = limit_level > 0 ? limit_level : CONFIG_INSTANCE->const_set("chat_team_level");
		chat_interval = chat_interval > 0 ? chat_interval : CONFIG_INSTANCE->const_set("chat_team_interval");
		last_interval = this->last_team_chat_.sec();
		error_code = ERROR_CHAT_TEAM_LEVEL;
		break;
	}
	case CHANNEL_PRIVATE:
	{
		limit_level = limit_level > 0 ? limit_level : CONFIG_INSTANCE->const_set("chat_private_level");
		chat_interval = chat_interval > 0 ? chat_interval : CONFIG_INSTANCE->const_set("chat_private_interval");
		last_interval = this->last_private_chat_.sec();
		error_code = ERROR_CHAT_PRIVATE_LEVEL;
		break;
	}
	case CHANNEL_SCENE:
	default:
	{
		break;
	}
	}

	JUDGE_RETURN(this->role_detail_.__level >= limit_level, error_code);
	JUDGE_RETURN((Time_Value::gettimeofday().sec()-last_interval) >= chat_interval,
			ERROR_CHAT_LIMIT_INTERVAL);

	return 0;
}

int ChatPlayer::check_chat_vip_times(int channel_type)
{
	VipLimit &vip_limit = CHAT_MONITOR->fetch_vip_limit();
	JUDGE_RETURN(vip_limit.vip_map_.size() > 0, 0);

	int vip_lv = this->role_detail().__vip_type;
	VipLimit::VipTimes *vip_times = vip_limit.fetch_vip_times(vip_lv);
	JUDGE_RETURN(vip_times != NULL, 0);
	JUDGE_RETURN(vip_times->channel_times_map_.count(channel_type) > 0, 0);

	int limit_times = vip_times->channel_times_map_[channel_type];
	if (limit_times == -1)
	{
		return 0;
	}
	else if (limit_times == 0)
	{
		return ERROR_CHAT_TIMES_LIMIT;
	}

	ChatTimes *chat_times = CHAT_MONITOR->fetch_player_chat_times(this->role_id());
	JUDGE_RETURN(chat_times != NULL, 0);
	JUDGE_RETURN(chat_times->channel_times_[channel_type] < limit_times, ERROR_CHAT_TIMES_LIMIT);

	return 0;
}

int ChatPlayer::check_chat_conditon(void)
{
	if (CONFIG_INSTANCE->arrive_fun_open_level("fun_chat",
			this->role_detail_.__level) == false)
	{
		return ERROR_WORLD_CHAT_LEVEL_LIMIT;
	}

	return 0;
}

int ChatPlayer::check_black_list(Int64 tar_player_id)
{
	PrivateChat* pChat = CHANNEL_AGENCY->find_private_channel(this->role_id());
	JUDGE_RETURN(pChat != NULL, 0);
	return pChat->is_in_black_list(tar_player_id) == true ? -1 : 0;
}

int ChatPlayer::check_travel_channel()
{
	return 0;
}

int ChatPlayer::check_league_channel()
{
	JUDGE_RETURN(this->league_id() > 0, -1);

	LeagueChannel* channel = CHANNEL_AGENCY->find_league_channel(this->league_id());
	JUDGE_RETURN(channel == NULL, -1);

	CHANNEL_AGENCY->create_league_channel(this->league_id());
	return 0;
}

int ChatPlayer::check_nearby_channel()
{
	return 0;
}

int ChatPlayer::chat_world(Proto10200002 *proto, bool trick/*=false*/)
{
	//聊天屏蔽
	if (proto->type() == R_WORD)
	{
		string new_word = this->check_workcheck_to_client(proto->wcontent());
		proto->set_wcontent(new_word);
	}

	this->last_world_chat_ = Time_Value::gettimeofday();
	return CHANNEL_AGENCY->chat_in_normal_channel(CHANNEL_WORLD, this, proto, trick);
}

int ChatPlayer::chat_nearby(Proto10200002 *proto, bool trick/*=false*/)
{
	this->last_nearby_chat_ = Time_Value::gettimeofday();
	return CHANNEL_AGENCY->chat_in_normal_channel(CHANNEL_SCENE, this, proto, trick);
}

int ChatPlayer::chat_league(Proto10200002 *proto, bool trick/*=false*/)
{
	JUDGE_RETURN(this->role_detail_.__league_id > 0, 0);

	this->last_league_chat_ = Time_Value::gettimeofday();
	return CHANNEL_AGENCY->chat_in_normal_channel(CHANNEL_LEAGUE, this, proto, trick);
}

int ChatPlayer::chat_team(Proto10200002 *proto, bool trick/*=false*/)
{
	this->last_team_chat_ = Time_Value::gettimeofday();
	return CHANNEL_AGENCY->chat_in_normal_channel(CHANNEL_TEAM, this, proto, trick);
}

int ChatPlayer::chat_travel(Proto10200002 *proto, bool trick)
{
	this->last_world_chat_ = Time_Value::gettimeofday();
	return CHANNEL_AGENCY->chat_in_normal_channel(CHANNEL_TRAVEL, this, proto, trick);
}

int ChatPlayer::chat_system(Proto10200002 *proto, bool trick)
{
	this->last_world_chat_ = Time_Value::gettimeofday();

	Proto30200123 brocast_info;
	ProtoBrocastNewInfo* proto_info = brocast_info.mutable_brocast_info();
	proto_info->set_channel_type(CHANNEL_SYSTEM);

	ProtoShoutDetail* proto_shout  = proto_info->add_shout_detail_list();
	proto_shout->set_parse_type(GameEnum::PARSE_TYPE_STRING);
	proto_shout->set_single_content(proto->wcontent());

	return CHAT_MONITOR->announce_world(&brocast_info);
}

int ChatPlayer::chat_loudspeaker(Message *msg_proto)
{
	// 禁言的处理
	int speak_state = this->speak_state();
	if(GameEnum::OPER_BAN_SPEAK == speak_state)
	{
		return this->respond_to_client_error(RETURN_CHAT_NORMAL, ERROR_FORBID_SPEAK);
	}

	bool trick_chat = (GameEnum::OPER_BAN_SPEAK_TRICK == speak_state);

	DYNAMIC_CAST_RETURN(Proto30200103*,proto,msg_proto,-1);
	CHANNEL_AGENCY->chat_loudspeaker(this, proto->content(), trick_chat);

	return 0;
}

int ChatPlayer::announce_with_player(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200124*, request, -1);

	const ProtoBrocastNewInfo& brocast_info = request->brocast_info();
	BaseChannel* chan = CHANNEL_AGENCY->fetch_channel_by_type(this,
			brocast_info.channel_type());
	JUDGE_RETURN(chan != NULL, -1);

	ChatRecord* record = CHANNEL_AGENCY->pop_record();
	JUDGE_RETURN(record != NULL, -1);

	CHANNEL_AGENCY->brocast_serial_record(record, &brocast_info);
	return chan->send_record(record);
}

int ChatPlayer::flaunt_dispatch(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200117*, request, -1);

	// 禁言的处理
	int speak_state = this->speak_state();
	if(GameEnum::OPER_BAN_SPEAK == speak_state)
		return this->respond_to_client_error(RETURN_CHAT_PRIVATE, ERROR_FORBID_SPEAK);

    if (this->role_detail().__level < CONFIG_INSTANCE->tiny("level_limit")["flaunt"].asInt())
        return this->respond_to_client_error(RETURN_CHAT_FLAUNT, ERROR_PLAYER_LEVEL_LIMIT);

    Time_Value nowtime = Time_Value::gettimeofday();
    if (request->flaunt_type() == GameEnum::FLAUNT_POSITION)
    {
    	if (this->last_flaunt_position_tick_ > nowtime)
    		return this->respond_to_client_error(RETURN_CHAT_FLAUNT, ERROR_ARENA_COOL_TIME);
    	this->last_flaunt_position_tick_ = nowtime + Time_Value(CONFIG_INSTANCE->tiny("flaunt_position_interval").asInt());
    }
    else
    {
		if (this->last_flaunt_tick_ > nowtime)  // TODO 临时用竞技场的错误号提示，不用更新客户端
			return this->respond_to_client_error(RETURN_CHAT_FLAUNT, ERROR_ARENA_COOL_TIME);
		this->last_flaunt_tick_ = nowtime + Time_Value(CONFIG_INSTANCE->tiny("flaunt_interval").asInt());
    }

	bool trick_chat = (GameEnum::OPER_BAN_SPEAK_TRICK == speak_state);


	Int64 flaunt_id = CHAT_MONITOR->generate_flaunt_record(request);
	JUDGE_RETURN(flaunt_id != -1, -1);

	int channel_type = request->channel_type();

    SERIAL_RECORD->record_chat(this->role_id(), channel_type, ::time(NULL), 
            CONFIG_INSTANCE->tiny("flaunt_word").asString(),
            this->role_detail().__agent_code, 
            this->role_detail().__server_flag,
            this->role_detail().__market_code);

	if(trick_chat == true)
		return CHANNEL_AGENCY->trick_flaunt(channel_type, flaunt_id, this, request);

	switch(channel_type)
	{
	{
		case CHANNEL_WORLD:
		case CHANNEL_LEAGUE:
		case CHANNEL_TEAM:
		case CHANNEL_SCENE:
			return CHANNEL_AGENCY->flaunt_in_normal_chanel(flaunt_id, this, request);
	}
	{
		case CHANNEL_PRIVATE:

			return CHANNEL_AGENCY->flaunt_private(flaunt_id, this, request);
	}
	{
		case CHANNEL_LOUDSPEAKER:

			return CHANNEL_AGENCY->flaunt_loudspeaker(flaunt_id, this, request);
	}
	default:
		break;
	}
	return 0;
}

int ChatPlayer::chat_get_league_history(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto10200004*,proto,msg_proto,-1);
	if(this->role_detail().__league_id<=0)
	{
		return 0;
	}
	Int64 league_id = this->role_detail().__league_id;
	LeagueChannel* chan= CHANNEL_AGENCY->find_league_channel(league_id);
	if(NULL==chan)
	{
		return 0;
	}

	std::list<ChatRecord*> list;
	chan->get_history(list,proto->time_offset());

	Block_Buffer buffer;
	if(list.empty())
	{
		return this->respond_to_client_error(RETURN_CHAT_GET_LEAGUE_HISTORY,0);
	}
	else
	{
		ProtoClientHead head;
		head.__error = 0;
		head.__recogn = RETURN_CHAT_GET_LEAGUE_HISTORY;

		std::list<ChatRecord*>::iterator iter=list.begin();
		for(;iter!=list.end();iter++)
		{
			uint32_t len = sizeof(ProtoClientHead), byte_size =
					(*iter)->__buffer->readable_bytes();
			len += byte_size;

			buffer.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
			buffer.write_uint32(len);
			buffer.copy(&head,sizeof(ProtoClientHead));
			buffer.copy((*iter)->__buffer);
			this->respond_to_client(&buffer);
			buffer.reset();
		}
	}
	return 0;
}

int ChatPlayer::chat_get_private_history(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto10200005*,proto,msg_proto,-1);
	if(this->role_detail().__league_id<=0)
	{
		return 0;
	}
	PrivateChat* chan= CHANNEL_AGENCY->find_private_channel(this->role_id());
	if(NULL==chan)
	{
		return 0;
	}

	std::list<ChatRecord*> list;
	chan->get_history(list,proto->peer_role_id(),proto->time_offset());

	Block_Buffer buffer;
	if(list.empty())
	{
		return this->respond_to_client_error(RETURN_CHAT_GET_PRIVATE_HISTORY,0);
	}
	else
	{
		ProtoClientHead head;
		head.__error = 0;
		head.__recogn = RETURN_CHAT_GET_PRIVATE_HISTORY;

		std::list<ChatRecord*>::iterator iter=list.begin();
		for(;iter!=list.end();iter++)
		{
			uint32_t len = sizeof(ProtoClientHead), byte_size =
					(*iter)->__buffer->readable_bytes();
			len += byte_size;

			buffer.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
			buffer.write_uint32(len);
			buffer.copy(&head,sizeof(ProtoClientHead));
			buffer.copy((*iter)->__buffer);
			this->respond_to_client(&buffer);
			buffer.reset();
		}
	}
	return 0;
}

int ChatPlayer::chat_get_voice(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto10200006*, proto, msg_proto, -1);

	MSG_USER("chat get voice %ld %d %ld", this->role_id(), proto->channel(), proto->voice_id());

	BaseChannel* chan = NULL;
	if(proto->channel()==CHANNEL_PRIVATE)
	{
		chan = CHANNEL_AGENCY->find_private_channel(this->role_id());
	}
	else if(proto->channel()==CHANNEL_LEAGUE)
	{
		chan = CHANNEL_AGENCY->find_league_channel(this->league_id());
	}
	else if(proto->channel()==CHANNEL_TEAM)
	{
		chan = CHANNEL_AGENCY->find_team_channel(this->team_id());
	}
	else if(proto->channel()==CHANNEL_WORLD)
	{
		chan = CHANNEL_AGENCY->fetch_channel_by_type(this, CHANNEL_WORLD);
	}

	if (NULL == chan)
	{
		return 0;
	}
	ChatRecord* r = chan->get_voice(proto->voice_id());

	Proto50200006 out;
	if(NULL!=r)
	{
		out.set_voice_id(r->__voice_id);
		int len = r->__buffer->readable_bytes();
		out.set_content(r->__buffer->get_read_ptr(),len);
		out.set_voice_length(r->__voice_len);

		MSG_USER("get private voice %ld id[%ld] %d %d %d", this->role_id(), r->__voice_id, len, out.content().length(), r->__buffer->readable_bytes());
	}
	return this->respond_to_client_error(RETURN_CHAT_GET_VOICE,0,&out);
}

int ChatPlayer::chat_get_flaunt_record(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto10200007*, request, -1);

	Int64 flaunt_id = request->flaunt_id();
	FlauntRecord* record = NULL;
	if(CHAT_MONITOR->find_flaunt_record(flaunt_id, record) == 0)
	{
		JUDGE_RETURN(record != NULL, -1);

		Proto50200007 respond;
		respond.set_flaunt_id(flaunt_id);
		respond.set_type(record->__flaunt_type);
		respond.set_msg(record->__buffer->get_read_ptr(), record->__len);

//		MSG_DEBUG(display Proto50200007: %s, respond.Utf8DebugString().c_str());
//		MSG_DEBUG(len: %d, record->__len);

		return this->respond_to_client_error(RETURN_CHAT_GET_FLAUNT_RECORD, 0, &respond);
	}
	return 0;
}

int ChatPlayer::chat_private_display_offline_message(int64_t peer_id, int time_offset)
{
	if(this->role_detail().__league_id<=0)
	{
		return 0;
	}
	PrivateChat* chan= CHANNEL_AGENCY->find_private_channel(this->role_id());
	if(NULL==chan)
	{
		return 0;
	}

	std::list<ChatRecord*> list;
	chan->get_history(list, peer_id, time_offset);

	Block_Buffer buffer;
	if(list.empty())
	{
		return 0;
	}
	else
	{
//		Proto50200005 out;
//		std::list<ChatRecord*>::iterator iter=list.begin();
//		for(;iter!=list.end();iter++)
//		{
//			ChatRecord* record = *iter;
//			ProtoChatInfo* info = out.add_content();
//			info->set_channel(CHANNEL_PRIVATE);
//			info->set_type(record->__type);
//			info->set_voice_id(record->__voice_id);
//			info->set_voice_len(record->__voice_len);
//			info->set_role_id(this->role_id());
//			info->set_name(this->role_detail().__name);
//			info->set_sex(this->role_detail().__sex);
//			info->set_vip(this->role_detail().__vip_type);
//			info->set_label(this->label_id());
//			info->set_team(this->team_id());
//			info->set_time(record->__time);
////			string content();
////			info->set_content(content);
//			info->set_target_id(record->__dst_role_id);
//
//			uint32_t  byte_size = out.ByteSize();
//
//			Block_Buffer* buffer = record->__buffer;
//			buffer->ensure_writable_bytes(byte_size + sizeof(uint32_t) * 4);
//			out.SerializeToArray(buffer->get_write_ptr(), buffer->writable_bytes());
//			buffer->set_write_idx(buffer->get_write_idx() + byte_size);
//		}
//
//		CHAT_MONITOR->dispatch_to_client(this, &out);
//	}
//	return 0;


		int sid = this->client_sid();

		ProtoClientHead head;
		head.__error = 0;
//		head.__recogn = RETURN_CHAT_GET_PRIVATE_HISTORY;
		head.__recogn = ACTIVE_CHAT_PRIVATE;

		std::list<ChatRecord*>::iterator iter=list.begin();
		for(;iter!=list.end();iter++)
		{
			uint32_t len = sizeof(ProtoClientHead), byte_size =
					(*iter)->__buffer->readable_bytes();
			len += byte_size;

			buffer.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
			buffer.write_uint32(len);
			buffer.copy(&head,sizeof(ProtoClientHead));
			buffer.copy((*iter)->__buffer);
//			this->respond_to_client(&buffer);
			this->monitor()->dispatch_to_client(sid, &buffer);
			buffer.reset();
		}
	}
	return 0;
}

int ChatPlayer::sync_role_info(Message *msg_proto)
{
	return 0;
}


int ChatPlayer::establish_league(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto30200104*,proto,msg_proto,-1);
	CHANNEL_AGENCY->create_league_channel(proto->league_id());
	return 0;
}

int ChatPlayer::dismiss_league(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto30200105*,proto,msg_proto,-1);
	return CHANNEL_AGENCY->del_league_channel(this);
}

int ChatPlayer::join_league(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto30200106*,proto,msg_proto,-1);
	if (this->league_id() > 0)
		CHANNEL_AGENCY->unregister_from_league_channel(this);

	MSG_DEBUG(CHAT join_league --league:%ld -- player:%ld, proto->league_id(), this->role_id());
	this->role_detail_.__league_id = proto->league_id();
	this->role_detail_.__league_name = proto->league_name();
	return CHANNEL_AGENCY->register_to_league_channel(this);
}
int ChatPlayer::leave_league(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200107*,proto,-1);
	CHANNEL_AGENCY->unregister_from_league_channel(this);
	this->role_detail_.__league_id = 0;
	this->role_detail_.__league_name.clear();
	MSG_DEBUG(CHAT leave_league --league:%ld -- player:%ld, proto->league_id(), proto->role_id());
	return 0;
}

int ChatPlayer::establish_team(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200108*,proto,-1);
	int prev_team_id = this->team_id();
	if (prev_team_id > 0)
		CHANNEL_AGENCY->unregister_from_team_channel(this);

	this->role_detail_.__team_id = proto->team_id();

	return CHANNEL_AGENCY->create_team_channel(proto->team_id());
}

int ChatPlayer::dismiss_team(Message* msg)
{
//	MSG_DYNAMIC_CAST_RETURN(Proto30200109*,proto, -1);
	return CHANNEL_AGENCY->del_team_channel(this);
}

int ChatPlayer::join_team(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200110*,proto,-1);

	if(this->team_id() > 0)
		CHANNEL_AGENCY->unregister_from_team_channel(this);

	MSG_USER("CHAT join team :%d", proto->team_id());
	this->role_detail_.__team_id = proto->team_id();

	if(CHANNEL_AGENCY->register_to_team_channel(this)<0)
	{
		this->role_detail_.__team_id =0;
	}
	return 0;
}

int ChatPlayer::leave_team(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200111*,proto,-1);
	CHANNEL_AGENCY->unregister_from_team_channel(this);
	this->role_detail_.__team_id = 0;
	return 0;
}

int ChatPlayer::add_black_list(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto30200113*,proto,msg_proto,-1);
	PrivateChat* chat = CHANNEL_AGENCY->find_private_channel(this->role_id());
	if(NULL!=chat)
	{
		for(int i = 0; i < (int)proto->role_id_set_size(); ++i)
			chat->add_black_list(proto->role_id_set(i));
		return 0;
	}
	return 0;
}

int ChatPlayer::remove_black_list(Message *msg_proto)
{
	DYNAMIC_CAST_RETURN(Proto30200114*,proto,msg_proto,-1);
	PrivateChat* chat = this->monitor()->channel_agency()->find_private_channel(this->role_id());
	if(NULL!=chat)
	{
		for(int i = 0; i < (int)proto->role_id_set_size(); ++i)
			chat->remove_black_list(proto->role_id_set(i));
		return 0;
	}
	return 0;
}

int ChatPlayer::add_friend_list(Message *msg_proto)//加名单
{
	DYNAMIC_CAST_RETURN(Proto30200120*, proto, msg_proto, -1);
	PrivateChat* chat = CHANNEL_AGENCY->find_private_channel(this->role_id());
	if(NULL!=chat)
	{
		for(int i = 0; i < (int)proto->role_id_set_size(); ++i)
			chat->add_friend_list(proto->role_id_set(i));
		return 0;
	}
	return 0;
}

int ChatPlayer::remove_friend_list(Message *msg_proto)//删除名单
{
	DYNAMIC_CAST_RETURN(Proto30200120*,proto,msg_proto,-1);
	PrivateChat* chat = CHANNEL_AGENCY->find_private_channel(this->role_id());
	if(NULL!=chat)
	{
		for(int i = 0; i < (int)proto->role_id_set_size(); ++i)
			chat->remove_friend_list(proto->role_id_set(i));
		return 0;
	}
	return 0;
}

int ChatPlayer::refrsh_vip_status(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200122*, request, -1);
	this->role_detail().__vip_type = request->vip_type();
	return 0;
}

int ChatPlayer::sync_update_player_name(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400020 *, request, -1);
    this->role_detail_.set_name(request->role_name());
    return 0;
}

int ChatPlayer::sync_update_player_sex()
{
	this->role_detail_.set_sex();
	return 0;
}

int ChatPlayer::sync_role_level(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200126*, request, -1);
    JUDGE_RETURN(this->role_id() == request->role_id(), -1);

    this->role_detail_.__level = request->role_level();
    JUDGE_RETURN(this->scene_id() != request->scene_id(), -1);
    CHANNEL_AGENCY->unregister_from_nearby_channel(this);

    this->role_detail_.__scene_id = request->scene_id();
    CHANNEL_AGENCY->register_to_nearby_channel(this);

	return 0;
}

int ChatPlayer::sync_role_speak_state(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30200127*, request, -1);

	this->role_detail_.__forbid_type = request->speak_state();
	this->role_detail_.__forbid_time = request->expired_time();
	return 0;
}

int ChatPlayer::speak_state()
{
	if(this->role_detail_.__forbid_type == GameEnum::OPER_BAN_NONE)
		return GameEnum::OPER_BAN_NONE;

	if(this->role_detail_.__forbid_time < ::time(0))
		return GameEnum::OPER_BAN_NONE;

	return this->role_detail_.__forbid_type;
}
