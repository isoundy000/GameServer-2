/*
 * EntityCommunicate.cpp
 *
 * Created on: 2013-02-20 15:13
 *     Author: glendy
 */

#include "EntityCommunicate.h"
#include "GameCommon.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

EntityCommunicate::~EntityCommunicate(void)
{}

int EntityCommunicate::respond_to_client(const int recogn, const Message *msg_proto)
{
    return this->respond_to_client_error(recogn, 0, msg_proto);
}

int EntityCommunicate::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
    return 0;
}

int EntityCommunicate::respond_to_client(Block_Buffer *buff)
{
    return 0;
}

int EntityCommunicate::gate_sid(void)
{
	return -1;
}

int EntityCommunicate::make_up_client_block(Block_Buffer *buff, const ProtoClientHead *head, const Message *msg_proto)
{
    uint32_t len = sizeof(ProtoClientHead), byte_size = 0;
    if (msg_proto != 0)
        byte_size = msg_proto->ByteSize();

    len += byte_size;
    buff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    buff->write_uint32(len);
    buff->copy((char *)head, sizeof(ProtoClientHead));

    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(buff->get_write_ptr(), buff->writable_bytes());
        buff->set_write_idx(buff->get_write_idx() + byte_size);
    }
    return 0;
}

int EntityCommunicate::make_up_broad_block(Block_Buffer *buff, const ProtoHead *head, const Message *msg_proto)
{
    uint32_t len = sizeof(ProtoHead) + sizeof(InnerRouteHead), byte_size = 0;
    if (msg_proto != 0)
        byte_size = msg_proto->ByteSize();

    InnerRouteHead route_head;
    route_head.__recogn = head->__recogn;
    route_head.__role_id = head->__role_id;
    route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    len += byte_size;
    buff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    buff->write_uint32(len);
    buff->copy((char *)&route_head, sizeof(InnerRouteHead));
    buff->copy((char *)head, sizeof(ProtoHead));

    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(buff->get_write_ptr(), buff->writable_bytes());
        buff->set_write_idx(buff->get_write_idx() + byte_size);
    }
    return 0;
}

int EntityCommunicate::make_up_gate_block(Block_Buffer *buff, const ProtoHead *head, const Message *msg_proto)
{
    uint32_t len = sizeof(ProtoHead), byte_size = 0;
    if (msg_proto != 0)
        byte_size = msg_proto->ByteSize();

    len += byte_size;
    buff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    buff->write_uint32(len);
    buff->copy((char *)head, sizeof(ProtoHead));

    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(buff->get_write_ptr(), buff->writable_bytes());
        buff->set_write_idx(buff->get_write_idx() + byte_size);
    }
    return 0;
}

int EntityCommunicate::entry_id_low(void)
{
    return (int32_t)(this->entity_id());
}

int EntityCommunicate::entry_id_high(void)
{
    return (int32_t)(this->entity_id() >> 32);
}

const char* EntityCommunicate::name()
{
	static string NullString;
	return NullString.c_str();
}

int EntityCommunicate::set_last_error(int last_error)
{
	this->last_error_ = last_error;
	return 0;
}

int EntityCommunicate::get_last_error()
{
	return this->last_error_;
}

int EntityCommunicate::notify_player_skill(FighterSkill* skill)
{
	Proto80400109 respond;
	skill->serialize(respond.mutable_skill(), true);
	FINER_PROCESS_RETURN(ACTIVE_PLAYER_SKILL, &respond);
}

int EntityCommunicate::validate_operate_tick(int type)
{
	JUDGE_RETURN(type >= 0 && type < GameEnum::TOTAL_OPERATE, false);
	return this->operate_tick_[type] <= Time_Value::gettimeofday();
}

int EntityCommunicate::add_validate_operate_tick(double add_tick, int type)
{
    Time_Value now_tick = Time_Value::gettimeofday();
    JUDGE_RETURN(this->operate_tick_[type] <= now_tick, false);

    this->operate_tick_[type] = GameCommon::fetch_add_time_value(add_tick);
    return true;
}

void EntityCommunicate::notify_tips_info(int msg_id, ...)
{
	TipsPlayer tips(this);

	const int buf_size = 1024;
	char buf[buf_size] = {0};

	const Json::Value &msg_format_json = CONFIG_INSTANCE->tips_msg_format(msg_id);
	JUDGE_RETURN(msg_format_json.empty() == false, ;);

	const std::string &format_str = msg_format_json.asString();
	if(format_str.length() <= 0)
	{
		MSG_USER("format error!!! tips_msg_format(%d)", msg_id);
		return ;
	}

    va_list arg_list;
    va_start(arg_list, msg_id);

	::vsnprintf(buf, buf_size - 1, format_str.c_str(), arg_list);
	buf[buf_size - 1] = '\0';

	va_end(arg_list);
	tips.push_tips_str(buf);
}

void EntityCommunicate::notify_one_tips_info(int type, int id, int amount)
{
	TipsPlayer tips_player(this);
	tips_player.push_tips(type, id, amount);
}

void EntityCommunicate::notify_red_point(int even_id, int even_value)	//红点通知
{
	Proto81401703 respond;
    respond.set_even_id(even_id);
    respond.set_even_value(even_value);
    this->respond_to_client(ACTIVE_PLAYER_ASSIST_APPEAR, &respond);
}

void EntityCommunicate::reset_entity()
{
	this->last_error_ = 0;
	for (int i = 0; i < GameEnum::TOTAL_OPERATE; ++i)
	{
		this->operate_tick_[i] = Time_Value::zero;
	}
}

void EntityCommunicate::announce(int shout_id, BrocastParaVec& para_vec)
{
	GameCommon::announce(ShoutInfo(shout_id, this->entity_id()), &para_vec);
}

void EntityCommunicate::notify_server_activity_info()
{
	Proto80400389 open_info;
	open_info.set_main_type(CONFIG_INSTANCE->main_activity_type());
	open_info.set_open_days(CONFIG_INSTANCE->client_open_days());
	open_info.set_absolute_days(CONFIG_INSTANCE->open_server_days());
	this->respond_to_client(ACTIVE_OPEN_DAYS_INFO, &open_info);
}

void EntityCommunicate::sync_open_activity_info(int first_type, const SubObj& sub)	//开服活动
{
	JUDGE_RETURN(CONFIG_INSTANCE->left_open_activity_time() > 0, ;);

	Proto30100241 request;
	request.set_first_type(first_type);
	request.set_sub1(sub.val1_);
	request.set_sub2(sub.val2_);
	request.set_sub3(sub.val3_);
	MAP_MONITOR->dispatch_to_logic(this, &request);
}

