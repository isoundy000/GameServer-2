/*
 * PubStruct.cpp
 *
 * Created on: 2012-12-31 19:26
 *     Author: glendy
 */

#include "PubStruct.h"
#include "ProtoPublic.pb.h"
#include "ProtoInnerPublic.pb.h"
#include <google/protobuf/descriptor.h>
#include <mongo/client/dbclient.h>
#include <openssl/md5.h>
#include <openssl/des.h>
#include <sys/syscall.h>
#include <signal.h>
#include "PoolMonitor.h"
#include "SessionManager.h"

#include "GameCommon.h"
#include "GamePackage.h"
#include "MapLogicStruct.h"
#include "MapLogicPlayer.h"

int string_utf8_len(const char *str, int *p_asccii_amount, int *p_chinese_amount)
{
	int ascii_amount = 0, chinese_amount = 0, i = 0;
	if (str == 0)
		return 0;

	const unsigned char *u_str = (const unsigned char *)str;
	while (u_str[i] != '\0')
	{
		if ((u_str[i] & 0x080) == 0)
			++ascii_amount;
		else if ((u_str[i] & 0x0C0) != 0x080)
			++chinese_amount;
		++i;
	}
	if (p_asccii_amount != NULL)
		*p_asccii_amount = ascii_amount;
	if (p_chinese_amount != NULL)
		*p_chinese_amount = chinese_amount;

	return (ascii_amount + chinese_amount * 2);
}

size_t string_utf8_len_to_raw_size(const char *str, const int limit_len)
{
	int ascii_amount = 0, chinese_amount = 0, i = 0;
	if (str == 0)
		return 0;

	const unsigned char *u_str = (const unsigned char *)str;
	while (u_str[i] != '\0')
	{
		if ((u_str[i] & 0x080) == 0)
			++ascii_amount;
		else if ((u_str[i] & 0x0C0) != 0x080)
			++chinese_amount;
		++i;

		JUDGE_BREAK(ascii_amount + chinese_amount < limit_len);
	}

	return (size_t)(ascii_amount + chinese_amount * 2);
}

int chinese_utf8_len(const char *str)
{
	int ascii_amount = 0, chinese_amount = 0, i = 0;
	if (str == 0)
		return 0;

	const unsigned char *u_str = (const unsigned char *)str;
	while (u_str[i] != '\0')
	{
		if ((u_str[i] & 0x080) == 0)
			++ascii_amount;
		else if ((u_str[i] & 0x0C0) != 0x080)
			++chinese_amount;
		++i;
	}

	return (ascii_amount + chinese_amount);
}

int string_remove_black_char(char *dest, const int max_len, const char *src, const int src_len)
{
    if (dest == 0 || max_len <= 0)
        return -1;

    int dest_idx = 0;
    for (int i = 0; i < src_len; ++i)
    {
        if (dest_idx >= max_len)
            break;

        char c = src[i];
        if ((0 <= c && c <= 32) || c == '\"' || c == '\\' || c == '\'' || c == '_' ||
        		c == '%' || c == '/' || c == '`')
            continue;
        dest[dest_idx++] = c;
    }
    if (dest_idx >= max_len)
        dest_idx = max_len - 1;
    dest[dest_idx] = '\0';
    return 0;
}

uint64_t des_encrypt(const uint64_t data, const char *key)
{
	DES_cblock key_block;
	DES_string_to_key(key, &key_block);

	DES_key_schedule key_schedule;
	DES_set_key_checked(&key_block, &key_schedule);

	DES_cblock input;
	memcpy(input, &data, sizeof(input));
	DES_cblock output;
	DES_ecb_encrypt(&input, &output, &key_schedule, DES_ENCRYPT);

	uint64_t dest_data = 0;
	memcpy(&dest_data, output, sizeof(output));
	return dest_data;
}

uint64_t des_decrypt(const uint64_t data, const char *key)
{
	DES_cblock key_block;
	DES_string_to_key(key, &key_block);

	DES_key_schedule key_schedule;
	DES_set_key_checked(&key_block, &key_schedule);

	DES_cblock input;
	memcpy(input, &data, sizeof(input));
	DES_cblock output;
	DES_ecb_encrypt(&input, &output, &key_schedule, DES_DECRYPT);

	uint64_t dest_data = 0;
	memcpy(&dest_data, output, sizeof(output));
	return dest_data;
}

int generate_md5(const char *src, std::string &str_md5)
{
    unsigned char tmp_md5[MD5_DIGEST_LENGTH + 1], dest_md5[MD5_DIGEST_LENGTH*2 + 1];
    ::memset(tmp_md5, 0, sizeof(tmp_md5));
    ::memset(dest_md5, 0, sizeof(dest_md5));

    MD5((const unsigned char *)src, strlen(src), tmp_md5);
    for (uint i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        ::sprintf((char *)(dest_md5 + (i * 2)), "%.2x", tmp_md5[i]);
    }
    str_md5 = (char *)dest_md5;
    return 0;
}

int generate_md5(const char *src, const int len, std::string &str_md5)
{
    unsigned char tmp_md5[MD5_DIGEST_LENGTH + 1], dest_md5[MD5_DIGEST_LENGTH*2 + 1];
    ::memset(tmp_md5, 0, sizeof(tmp_md5));
    ::memset(dest_md5, 0, sizeof(dest_md5));

    MD5((const unsigned char *)src, len, tmp_md5);
    for (uint i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        ::sprintf((char *)(dest_md5 + (i * 2)), "%.2x", tmp_md5[i]);
    }
    str_md5 = (char *)dest_md5;
    return 0;
}

time_t convert_to_whole_hour(const Time_Value &tick)
{
    Date_Time tick_date(tick);
    tick_date.minute(0);
    tick_date.second(0);
    tick_date.microsec(0);
    return tick_date.time_sec();
}

time_t convert_to_midnight(const Time_Value &tick)
{
    Date_Time tick_date(tick);
    tick_date.hour(0);
    tick_date.minute(0);
    tick_date.second(0);
    tick_date.microsec(0);
    return tick_date.time_sec();
}

Message* create_message(const std::string& type_name)
{
	Message *message = 0;
    const ::google::protobuf::Descriptor* descriptor = ::google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor != 0)
    {   
        const ::google::protobuf::Message *prototype = ::google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype != 0)
        {   
            message = prototype->New();	// remember delete
        }   
    }   
    return message;
}

Message* parse_message(const std::string &type_name, Block_Buffer *data_buff)
{
    Message *proto_msg = create_message(type_name);
    JUDGE_RETURN(proto_msg != NULL, NULL);

    if (proto_msg->ParseFromArray(data_buff->get_read_ptr(), data_buff->readable_bytes()) == false)
    {
        delete proto_msg;
        return 0;
    }
    return proto_msg;
}

Message* parse_message(int recogn, Block_Buffer *data_buff)
{
	char type_name[GameEnum::MAX_MSG_NAME_LEN];
    ::sprintf(type_name, "Proto%d", recogn);
    return parse_message(type_name, data_buff);
}

int string_to_block_len(const std::string &data)
{
    return (sizeof(uint16_t) + data.length());
}

void split_long_to_int(const int64_t value, int32_t &i_low, int32_t &i_high)
{
    i_low = (int32_t)value;
    i_high = (int32_t)(value >> 32);
}

int64_t merge_int_to_long(const int32_t i_low, const int32_t i_high)
{
    int64_t value = i_high;
    value <<= 32;
    value += i_low;
    return value;
}

int type_name_to_recogn(const std::string &type_name)
{
    size_t type_name_len = type_name.length();
    const char *sz_type_name = type_name.c_str();
    const char *sz_recogn = 0;
    for (size_t i = 0; i < type_name_len; ++i)
    {
        if ('0' <= sz_type_name[i] && sz_type_name[i] <= '9')
        {
            sz_recogn = sz_type_name + i;
            break;
        }
    }
    if (sz_recogn == 0)
        return 0;
    return ::atoi(sz_recogn);
}

pthread_t gettid(void)
{
	return ::syscall(SYS_gettid);
}

int register_signal(int signo, SigHandler handler)
{
	if (signal(signo, handler) == SIG_ERR)
		return -1;
	return 0;
}

Time_Value current_day(const int hour, const int minute, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.second(0);
    now_date.hour(hour);
    now_date.minute(minute);
    time_t next_tick = now_date.time_sec();
    return Time_Value(next_tick, 0);
}

Time_Value current_week(const int week, const int hour, const int minute, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.hour(hour);
    now_date.minute(minute);
    now_date.second(0);


    int inc_day = (week - now_date.weekday() % 7);
    Time_Value next_tick(now_date.time_sec() + inc_day * (24 * 3600), 0);
    return next_tick;

}

Time_Value current_month(const int day, const int hour, const int minute, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.hour(hour);
    now_date.minute(minute);
    now_date.second(0);
    now_date.day(day);
    return Time_Value(now_date.time_sec(), 0);
}

Time_Value current_year(const int month, const int day, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.hour(0);
    now_date.minute(0);
    now_date.second(0);
    now_date.day(day);
    now_date.month(month);

    return Time_Value(now_date.time_sec(), 0);
}

int diff_day(const Time_Value &tick, const Time_Value &nowtime)
{
    Date_Time open_date(tick), now_date(nowtime);
    open_date.hour(0);
    open_date.minute(0);
    open_date.second(0);

    now_date.hour(0);
    now_date.minute(0);
    now_date.second(0);

    int diff_sec = std::abs(now_date.time_sec() - open_date.time_sec());
    return (diff_sec / 86400 + (diff_sec % 86400 > 0 ? 1 : 0));
}

Time_Value next_day(const int hour, const int minute, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.second(0);
    now_date.hour(hour);
    now_date.minute(minute);
    time_t next_tick = now_date.time_sec();
    if (next_tick <= nowtime.sec())
        next_tick += (24 * 3600);
    return Time_Value(next_tick);
}

Time_Value next_week(const int week, const int hour, const int minute, const Time_Value &nowtime)
{
    Time_Value next_tick = current_week(week, hour, minute, nowtime);
    while (next_tick < nowtime)
        next_tick += Time_Value(7 * 24 * 3600, 0);
    return next_tick;
}

Time_Value next_month(const int day, const int hour, const int minute, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.hour(hour);
    now_date.minute(minute);
    now_date.second(0);
    now_date.day(day);
    now_date.month(now_date.month() + 1);
    if (now_date.month() > 12)
    {
        now_date.month(1);
        now_date.year(now_date.year() + 1);
    }
    return Time_Value(now_date.time_sec(), 0);
}

Time_Value next_year(const int month, const int day, const Time_Value &nowtime)
{
    Date_Time now_date(nowtime);
    now_date.hour(0);
    now_date.minute(0);
    now_date.second(0);
    now_date.day(day);
    now_date.month(month);
    now_date.year(now_date.year() + 1);

    return Time_Value(now_date.time_sec(), 0);
}

InnerRouteHead::InnerRouteHead(const int recogn, const Int64 role_id, const int broad_type, const int scene_id, const int inner_req) :
    __recogn(recogn), __role_id(role_id), __broad_type(broad_type), __scene_id(scene_id), __inner_req(inner_req)
{
	this->__line_id = -1;
}

void InnerRouteHead::reset(void)
{
    ::memset(this, 0, sizeof(InnerRouteHead));
    this->__line_id = -1;
}

ProtoClientHead::ProtoClientHead(void) :
    __recogn(0), __error(0)
{ /*NULL*/ }

void ProtoClientHead::reset(void)
{
    ::memset(this, 0, sizeof(ProtoClientHead));
}

ProtoHead::ProtoHead(const int recogn, const int error, const int trans_id, const int scene_id, const Int64 role_id) :
    __recogn(recogn), __error(error), __trans_id(trans_id), __scene_id(scene_id), __role_id(role_id)
{
	this->__src_line_id = -1;
	this->__src_scene_id = 0;
}

void ProtoHead::reset(void)
{
    ::memset(this, 0, sizeof(ProtoHead));
}

void UnitMessage::set_msg_proto(Message *msg_proto)
{
    this->__type = TYPE_PROTO_MSG;
    this->__data.__proto_msg = msg_proto;
}

void UnitMessage::set_data_buff(Block_Buffer *buff)
{
    this->__type = TYPE_BLOCK_BUFF;
    this->__data.__buf = buff;
}

void UnitMessage::set_ivalue(const int value)
{
    this->__type = TYPE_IVALUE;
    this->__data.__i_val = value;
}

bool UnitMessage::need_adjust_player_scene()
{
	if (this->__msg_head.__scene_id <= 0)
	{
		return true;
	}

//    if (this->__msg_head.__recogn / 10000000 == 1 && this->__route_head.__inner_req == 0)
//    {
//    	return true;
//    }

    return false;
}

UnitMessage::UnitMessage(void) :
    __type(0), __sid(0), __len(0)
{
    ::memset(&(this->__data), 0, sizeof(this->__data));
}

void UnitMessage::reset(void)
{
    this->__type = 0;
    this->__sid = 0;
    this->__len = 0;
    this->__route_head.reset();
    this->__msg_head.reset();
    ::memset(&(this->__data), 0, sizeof(this->__data));
}

Block_Buffer *UnitMessage::data_buff(void)
{
    if (this->__type == TYPE_BLOCK_BUFF)
        return this->__data.__buf;
    return 0;
}

int UnitMessage::ivalue(void)
{
    if (this->__type == TYPE_IVALUE)
        return this->__data.__i_val;
    return 0;
}

Message *UnitMessage::proto_msg(void)
{
    if (this->__type == TYPE_PROTO_MSG)
        return this->__data.__proto_msg;
    return 0;
}

BaseServerInfo::BaseServerInfo()
{
	BaseServerInfo::reset();
}

void BaseServerInfo::reset()
{
	this->__server_id = 0;
	this->__server_flag.clear();
	this->__server_prev.clear();
	this->__server_name.clear();
}

void BaseServerInfo::set_cur_server_flag()
{
	if (this->__server_flag.empty() == true)
	{
		string flag = CONFIG_INSTANCE->global()["server_flag"].asString();
		this->set_server_flag(flag);
	}
	else
	{
		string flag = this->__server_flag;
		this->set_server_flag(flag);
	}
}

void BaseServerInfo::set_server_flag(const string& server_flag)
{
	JUDGE_RETURN(server_flag.empty() == false, ;);

	this->__server_flag = server_flag;
	this->__server_id = GameCommon::server_flag_to_server_id(server_flag);
}

void BaseServerInfo::serialize(ProtoServer* proto, int include_cur)
{
	proto->set_id(this->__server_id);
	proto->set_name(this->__server_name);
	proto->set_prev(this->__server_prev);
	proto->set_flag(this->__server_flag);

	JUDGE_RETURN(include_cur == true, ;)
	proto->set_cur_flag(CONFIG_INSTANCE->server_flag());
}

void BaseServerInfo::unserialize(const ProtoServer& proto)
{
	this->__server_id = proto.id();
	this->__server_name = proto.name();
	this->__server_prev = proto.prev();
	this->__server_flag = proto.flag();
	this->__cur_server_flag = proto.cur_flag();
}

void BaseServerInfo::unserialize(const BaseServerInfo& server_info)
{
	JUDGE_RETURN(this != &server_info, ;);
	*this = server_info;
}

string BaseServerInfo::fetch_flag_by_trvl()
{
	if (this->__cur_server_flag.empty() == false)
	{
		return this->__cur_server_flag;
	}
	else
	{
		return this->__server_flag;
	}
}

ServerInfo::ServerInfo(int in_use)
{
	ServerInfo::reset();
    this->__is_in_use = in_use;
}

void ServerInfo::reset(void)
{
	BaseServerInfo::reset();

    this->__is_in_use = 1;
    this->__open_server = Time_Value::zero;

    this->__combine_to_server_id = 0;
    this->__has_collect_combine_set = false;

    this->__combine_set.clear();
    this->__back_server_name.clear();
}

int GlobalIndex::global_team_index_ = 1;
int GlobalIndex::global_market_index_ = 1;
int GlobalIndex::global_scene_hash_id_ = 1;
int GlobalIndex::global_item_hash_id_ = 1;

std::string GameName::SPEED = "speed";		//速度

std::string GameName::POWER	= "power";
std::string GameName::STAMINA = "stamina";
std::string GameName::PHYSICA = "physica";
std::string GameName::AGILITY = "agility";
std::string GameName::BODYACT = "bodyact";

std::string GameName::BLOOD_MAX = "health";//生命上限
std::string GameName::MAGIC_MAX	= "magic";	//魔法上限
std::string GameName::ATTACK = "attack";	//攻击（会同时增加攻击上限、下限）
std::string GameName::DEFENSE = "defence";	//防御（会同时增加防御上限、下限）
std::string GameName::HIT = "hit";	//命中
std::string GameName::AVOID = "dodge";	//闪避
std::string GameName::CRIT = "crit";	//暴击
std::string GameName::TOUGHNESS = "toughness";	//韧性
std::string GameName::PK_VALUE = "pk";			//pk值
std::string GameName::GLAMOUR = "glamour";		//魅力值
std::string GameName::LUCK = "luck";		//幸运值
std::string GameName::ATTACK_MAX  = "attack_max";	//攻击上限
std::string GameName::ATTACK_MIN = "attack_min";	//攻击下限
std::string GameName::DEFENCE_MAX  = "defence_max";	// 防御上限
std::string GameName::DEFENCE_MIN  = "defence_min";	// 防御下限
std::string GameName::CRIT_HURT_MULTI = "crit_multi";	//暴击伤害倍数(万分比)
std::string GameName::DAMAGE_MULTI = "damage_multi";	//伤害加成倍数(万分比)
std::string GameName::REDUCTION_MULTI = "reduction_multi";	//伤害减免倍数(万分比)
std::string GameName::DAMAGE = "damage";	//伤害加成
std::string GameName::REDUCTION = "reduction";	//伤害减免
std::string GameName::HIT_KLV = "hit_klv";				//命中KLV
std::string GameName::AVOID_KLV = "dodge_klv";			//闪避KLV
std::string GameName::CRIT_KLV = "crit_klv";			//暴击KLV
std::string GameName::TOUGHNESS_KLV = "toughness_klv";		//韧性KLV

std::string GameName::WELCONTRI = "contribute";
std::string GameName::WELGOODS = "goods";
std::string GameName::WELCOPPER = "copper";

std::string GameName::CENTER_BORN = "center";
std::string GameName::FIXED_BORN = "fixed";
std::string GameName::AREA_BORN = "area";
std::string GameName::UNLIMITED_BORN = "unlimited";

std::string GameName::ER_STAGE_PASS_THROUGHT = "pass_throught";
std::string GameName::ER_STAGE_PASS_CHAPTER = "pass_chapter";
std::string GameName::ER_STAGE_PASS_FLOOR = "pass_floor";


void SessionDetail::reset(void)
{
	if (this->is_registered() == true)
	{
		this->cancel_timer();
	}

    this->__account.clear();
    this->__address.clear();
    this->__port = 0;
    this->__timeout_tick = Time_Value::zero;
    this->__role_id = 0;
    this->__session.clear();
    this->__client_sid = 0;
    this->__manager = 0;
}

int SessionDetail::type(void)
{
    return GTT_GATE_TRANS;
}

int SessionDetail::handle_timeout(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->__manager != NULL, 0);

    this->cancel_timer();

    SessionDetail *session = 0;
    if (this->__manager->unbind_account_session(this->__account, session) == 0)
    {
        if (session == this)
            this->__manager->session_pool()->push(this);
        else
            this->__manager->bind_account_session(session->__account, session);
    }

    return 0;
}

Equipment::Equipment(void)
{
	Equipment::reset();
}

void Equipment::fetch_refine(IntMap& refine_map)
{
	for(ItemObjMap::const_iterator it = this->__good_refine.begin();
			it != this->__good_refine.end(); ++it)
	{
		JUDGE_CONTINUE(it->second.validate() == true);
		refine_map[it->second.id_] = it->second.amount_;
	}
}

void Equipment::set_refine(IntMap& refine_map)
{
	for (IntMap::iterator iter = refine_map.begin();
			iter != refine_map.end(); ++iter)
	{
		ItemObj& obj = this->__good_refine[iter->first];
		obj.id_ = iter->first;
		obj.amount_ = iter->second;
	}
}

void Equipment::serialize(ProtoEquip *msg_proto, PackageItem *package_item)
{
	msg_proto->set_refine_level(this->strengthen_level_);
	msg_proto->set_refine_degree(this->__refine_degree);

	for(IntMap::const_iterator it = this->__jewel_detail.begin();
			it != this->__jewel_detail.end(); ++it)
	{
		ProtoPairObj* lists = msg_proto->add_jewel_lists();
		lists->set_obj_id(it->first);
		lists->set_obj_value(it->second);
	}

	for(ItemObjMap::const_iterator it = this->__good_refine.begin();
			it != this->__good_refine.end(); ++it)
	{
		ProtoPairObj* lists = msg_proto->add_refine_lists();
		lists->set_obj_id(it->second.id_);
		lists->set_obj_value(it->second.amount_);
	}

	string nature_name = "";
	for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
	{
		MoldingNature *molding_nature = msg_proto->add_molding_nature();
		int cur_level = this->__molding_detail.fetch_nature_level(i);
		int cur_schedule = this->__molding_detail.fetch_nature_schedule(i);
		molding_nature->set_level(cur_level);
		molding_nature->set_cur_schedule(cur_schedule);
		int nature_id = CONFIG_INSTANCE->molding_spirit_return_sys_nature_id(i);
		molding_nature->set_nature_id(nature_id);
		const Json::Value &molding_info = CONFIG_INSTANCE->molding_spirit_info(i, cur_level, package_item->red_grade() + GameEnum::ORANGE);
		int value = 0;
		if(i == GameEnum::MOLDING_SPIRIT_ALL_NATURE)
		{
			int nature_index = CONFIG_INSTANCE->molding_spirit_equip_nature(package_item->__index + 1)["nature"].asInt();
			nature_name = CONFIG_INSTANCE->molding_spirit_return_sys_nature_name(nature_index);
			value = molding_info[nature_name].asInt();
			molding_nature->set_value(value);

		}
		else
		{
			nature_name = "nature";
			value = molding_info[nature_name].asInt();
			molding_nature->set_value(value);
		}
		//MSG_DEBUG("type:%d, level:%d, schedule:%d, value:%d", i, cur_level, cur_schedule, value);
	}
}

void Equipment::unserialize(const ProtoEquip *msg_proto, PackageItem *package_item)
{
	this->strengthen_level_ = msg_proto->refine_level();
	this->__refine_degree = msg_proto->refine_degree();

	for(int i = 0; i < msg_proto->jewel_lists_size(); ++i)
	{
		const ProtoPairObj& tmp = msg_proto->jewel_lists(i);
		this->__jewel_detail[tmp.obj_id()] = tmp.obj_value();
	}

	for(int i = 0; i < msg_proto->refine_lists_size(); ++i)
	{
		const ProtoPairObj& tmp = msg_proto->refine_lists(i);
		ItemObj& obj = this->__good_refine[tmp.obj_id()];
		obj.id_ 	= tmp.obj_id();
		obj.amount_ = tmp.obj_value();
	}

	MoldingSpiritDetail &molding_detail = this->__molding_detail;
	molding_detail.reset();
	molding_detail.__red_grade = package_item->red_grade() + GameEnum::ORANGE;
	for(int i = 0; i < msg_proto->molding_nature_size(); ++i)
	{
		const MoldingNature &molding_nature = msg_proto->molding_nature(i);
		molding_detail.update_nature_level(i, molding_nature.level(), true);
		molding_detail.update_nature_schedule(i, molding_nature.cur_schedule(), true);
	}
}

void Equipment::reset()
{
	this->__refine_degree = 0;
	this->strengthen_level_ = 0;
	this->__luck_value = 0;

	this->__good_refine.clear();
	this->__jewel_detail.clear();
	this->__special_jewel.clear();
	this->__molding_detail.reset();
}

Equipment* Equipment::operator=(const Equipment& rhs)
{
	JUDGE_RETURN(this != &rhs, this);

	this->__refine_degree 		= rhs.__refine_degree;
	this->strengthen_level_ 	= rhs.strengthen_level_;
	this->__good_refine 		= rhs.__good_refine;
	this->__luck_value 			= rhs.__luck_value;
	this->__jewel_detail 		= rhs.__jewel_detail;
	this->__molding_detail   = rhs.__molding_detail;

	return this;
}

void RotaryTableInfo::reset()
{
	this->prop_index_ = 0;
}

FightProperty::FightProperty(int type)
{
	FightProperty::reset();
	this->type_ = type;
}

void FightProperty::reset()
{
	int src_type = this->type_;
	ZeroMemory(this, sizeof(FightProperty));

	this->type_ = src_type;
}

void FightProperty::set_type(int type)
{
	this->type_ = type;
}

void FightProperty::add_multi_times(int multi_times, int type)
{
	JUDGE_RETURN(multi_times != 0, ;);
	double times = 1 + GameCommon::div_percent(multi_times);

	this->blood_max_ 	*= times;
	this->attack_ 		*= times;
	this->defence_ 		*= times;
	this->avoid_ 		*= times;
	this->hit_			*= times;
	this->crit_ 		*= times;
	this->toughness_ 	*= times;
	this->damage_		*= times;
	this->reduction_	*= times;

	this->crit_hurt_multi_	*= times;
	this->damage_multi_ 	*= times;
	this->reduction_multi_ 	*= times;
}

int FightProperty::caculate_force(int sub)
{
	this->force_ = this->attack_ * 5 + this->defence_ * 5 + this->blood_max_ * 0.5
			+ this->hit_ * 15 + this->avoid_ * 15 + this->crit_ * 15 + this->toughness_ * 15 + sub;
	return this->force_;
}

void FightProperty::serialize(IntMap& prop_map)
{
	prop_map[GameEnum::BLOOD_MAX] 	+= this->blood_max_;
	prop_map[GameEnum::ATTACK]		+= this->attack_;
	prop_map[GameEnum::DEFENSE]		+= this->defence_;
	prop_map[GameEnum::AVOID]		+= this->avoid_;
	prop_map[GameEnum::HIT]			+= this->hit_;
	prop_map[GameEnum::CRIT]		+= this->crit_;
	prop_map[GameEnum::TOUGHNESS]	+= this->toughness_;
	prop_map[GameEnum::DAMAGE]		+= this->damage_;
	prop_map[GameEnum::REDUCTION]	+= this->reduction_;

	prop_map[GameEnum::CRIT_HURT_MULTI]	+= this->crit_hurt_multi_;
	prop_map[GameEnum::DAMAGE_MULTI]	+= this->damage_multi_;
	prop_map[GameEnum::REDUCTION_MULTI]	+= this->reduction_multi_;
}

void FightProperty::serialize(ProtoFightPro* proto)
{
	proto->set_force(this->force_);
	proto->set_attack(this->attack_);
	proto->set_defence(this->defence_);
	proto->set_blood_max(this->blood_max_);

	if (this->blood_ > 0)
	{
		proto->set_blood(this->blood_);
	}

	if (this->avoid_ > 0)
	{
		proto->set_avoid(this->avoid_);
	}

	if (this->hit_ > 0)
	{
		proto->set_hit(this->hit_);
	}

	if (this->crit_ > 0)
	{
		proto->set_crit(this->crit_);
	}

	if (this->toughness_ > 0)
	{
		proto->set_toughness(this->toughness_);
	}

	if (this->crit_hurt_multi_ > 0)
	{
		proto->set_crit_multi(this->crit_hurt_multi_);
	}

	if (this->damage_multi_ > 0)
	{
		proto->set_damage_multi(this->damage_multi_);
	}

	if (this->reduction_multi_ > 0)
	{
		proto->set_reduction_multi(this->reduction_multi_);
	}
}

void FightProperty::unserialize(IntMap& prop_map)
{
	this->blood_max_ 		= prop_map[GameEnum::BLOOD_MAX];
	this->attack_ 			= prop_map[GameEnum::ATTACK];
	this->defence_ 			= prop_map[GameEnum::DEFENSE];
	this->avoid_ 			= prop_map[GameEnum::AVOID];
	this->hit_ 				= prop_map[GameEnum::HIT];
	this->crit_ 			= prop_map[GameEnum::CRIT];
	this->toughness_ 		= prop_map[GameEnum::TOUGHNESS];
	this->damage_ 			= prop_map[GameEnum::DAMAGE];
	this->reduction_ 		= prop_map[GameEnum::REDUCTION];
	this->caculate_force();
}

void FightProperty::unserialize(const ProtoFightPro& proto)
{
	this->attack_ 		= proto.attack();
	this->defence_ 		= proto.defence();
	this->blood_ 		= proto.blood();
	this->blood_max_ 	= proto.blood_max();
	this->avoid_ 		= proto.avoid();
	this->hit_ 			= proto.hit();
	this->crit_ 		= proto.crit();
	this->toughness_ 	= proto.toughness();

	this->crit_hurt_multi_	= proto.crit_multi();
	this->damage_multi_		= proto.damage_multi();
	this->reduction_multi_ 	= proto.reduction_multi();

	this->caculate_force();
}

//属性名，如果配置属性值大于0，增加额外value
void FightProperty::make_up_name_prop(const Json::Value& prop_json, int times, int value)
{
	JUDGE_RETURN(times != 0, ;);
	JUDGE_RETURN(prop_json.empty() == false, ;);

	IntPair value_pair(times, value);
	IntPair multi_pair(times, value);

	int conf_value = prop_json[GameName::BLOOD_MAX].asInt();
	add_fight_property(this->blood_max_, conf_value, value_pair);

//	conf_value = prop_json[GameName::MAGIC_MAX].asInt();
//	add_fight_property(this->magic_max_, conf_value, value_pair);

	conf_value = prop_json[GameName::ATTACK].asInt();
	add_fight_property(this->attack_, conf_value, value_pair);

	conf_value = prop_json[GameName::DEFENSE].asInt();
	add_fight_property(this->defence_, conf_value, value_pair);

	conf_value = prop_json[GameName::HIT].asInt();
	add_fight_property(this->hit_, conf_value, value_pair);

	conf_value = prop_json[GameName::AVOID].asInt();
	add_fight_property(this->avoid_, conf_value, value_pair);

	conf_value = prop_json[GameName::CRIT].asInt();
	add_fight_property(this->crit_, conf_value, value_pair);

	conf_value = prop_json[GameName::TOUGHNESS].asInt();
	add_fight_property(this->toughness_, conf_value, value_pair);

	conf_value = prop_json[GameName::DAMAGE].asInt();
	add_fight_property(this->damage_, conf_value, value_pair);

	conf_value = prop_json[GameName::REDUCTION].asInt();
	add_fight_property(this->reduction_, conf_value, value_pair);

	conf_value = prop_json[GameName::CRIT_HURT_MULTI].asInt();
	add_fight_property(this->crit_hurt_multi_, conf_value, multi_pair);

	conf_value = prop_json[GameName::DAMAGE_MULTI].asInt();
	add_fight_property(this->damage_multi_, conf_value, multi_pair);

	conf_value = prop_json[GameName::REDUCTION_MULTI].asInt();
	add_fight_property(this->reduction_multi_, conf_value, multi_pair);
}

//属性ID
void FightProperty::make_up_id_prop(const Json::Value& prop_json)
{
}

//单个属性
void FightProperty::make_up_one_prop(const string& prop_name, int prop_add, int times, int value)
{
	JUDGE_RETURN(times != 0 && prop_add != 0, ;);

	IntPair value_pair(times, value);
	IntPair multi_pair(times, value);

	if (prop_name == GameName::BLOOD_MAX)
		add_fight_property(this->blood_max_, prop_add, value_pair);
	else if (prop_name == GameName::ATTACK)
		add_fight_property(this->attack_, prop_add, value_pair);
	else if (prop_name == GameName::DEFENSE)
		add_fight_property(this->defence_, prop_add, value_pair);
	else if (prop_name == GameName::HIT)
		add_fight_property(this->hit_, prop_add, value_pair);
	else if (prop_name == GameName::AVOID)
		add_fight_property(this->avoid_, prop_add, value_pair);
	else if (prop_name == GameName::CRIT)
		add_fight_property(this->crit_, prop_add, value_pair);
	else if (prop_name == GameName::TOUGHNESS)
		add_fight_property(this->toughness_, prop_add, value_pair);
	else if (prop_name == GameName::DAMAGE)
		add_fight_property(this->damage_, prop_add, value_pair);
	else if (prop_name == GameName::REDUCTION)
		add_fight_property(this->reduction_, prop_add, value_pair);
	else if (prop_name == GameName::CRIT_HURT_MULTI)
		add_fight_property(this->crit_hurt_multi_, prop_add, multi_pair);
	else if (prop_name == GameName::DAMAGE_MULTI)
		add_fight_property(this->damage_multi_, prop_add, multi_pair);
	else if (prop_name == GameName::REDUCTION_MULTI)
		add_fight_property(this->reduction_multi_, prop_add, multi_pair);
}

void FightProperty::add_fight_property(const FightProperty& fight_prop)
{
	this->attack_ 		+= fight_prop.attack_;
	this->defence_ 		+= fight_prop.defence_;
	this->blood_ 		+= fight_prop.blood_;
	this->blood_max_ 	+= fight_prop.blood_max_;
	this->avoid_ 		+= fight_prop.avoid_;
	this->hit_ 			+= fight_prop.hit_;
	this->crit_ 		+= fight_prop.crit_;
	this->toughness_ 	+= fight_prop.toughness_;
	this->damage_		+= fight_prop.damage_;
	this->reduction_ 	+= fight_prop.reduction_;

	this->crit_hurt_multi_ 	+= fight_prop.crit_hurt_multi_;
	this->damage_multi_ 	+= fight_prop.damage_multi_;
	this->reduction_multi_ 	+= fight_prop.reduction_multi_;
}

void FightProperty::add_fight_property(int& cur_prop, int conf_value, const IntPair& pair)
{
	JUDGE_RETURN(conf_value > 0, ;);
	cur_prop += conf_value * pair.first + pair.second;
}

void PackageItem::serialize(ProtoItem *msg_proto, int new_index, int set_num)
{
	JUDGE_RETURN(msg_proto != NULL, ;);

	if (new_index != -1)
	{
		msg_proto->set_index(new_index);
	}
	else
	{
		msg_proto->set_index(this->__index);
	}

	// 此函数,不进行数量合法性判断
    if (set_num > 0)
    {
    	msg_proto->set_amount(set_num);
    }
    else
    {
		msg_proto->set_amount(this->__amount);
    }

    msg_proto->set_id(this->__id);
    msg_proto->set_unique_id(this->__unique_id);
    msg_proto->set_bind(this->__bind);
    msg_proto->set_use_tick(this->__use_tick);
    msg_proto->set_use_times(this->__use_times);
    msg_proto->set_timeout(this->__timeout);
    msg_proto->set_tips_level(this->__tips_level);

    if (this->is_equipment() == true)
    {
    	ProtoEquip* proto_equip = msg_proto->mutable_equipment();
    	this->__equipment.serialize(proto_equip, this);
    }

    for (uint i = 0; i < this->__tips_time_map.size(); ++i)
    {
    	msg_proto->add_tips_time_map(this->__tips_time_map[i]);
    	msg_proto->add_tips_status_map(this->__tips_status_map[i]);
    }

}

void PackageItem::unserialize(const ProtoItem *msg_proto)
{
    this->__index = msg_proto->index();
    this->__id = msg_proto->id();
    this->__amount = msg_proto->amount();
    this->__bind = msg_proto->bind();
    this->__unique_id = msg_proto->unique_id();

    this->__use_tick = msg_proto->use_tick();
    this->__use_times = msg_proto->use_times();
    this->__timeout = msg_proto->timeout();
    this->__tips_level = msg_proto->tips_level();

    if (this->is_equipment() == true)
    {
    	this->__equipment.unserialize(&(msg_proto->equipment()), this);
    }

    for (int i = 0; i < msg_proto->tips_time_map_size(); ++i)
    {
    	this->__tips_time_map[i] = msg_proto->tips_time_map(i);
    	this->__tips_status_map[i] = msg_proto->tips_status_map(i);
    }

}

void PackageItem::unserialize(const ProtoItem& msg_proto)
{
	this->unserialize(&msg_proto);
}

void PackageItem::unserialize(const ItemObj& obj, int index)
{
	this->__id 		= obj.id_;
	this->__bind 	= obj.bind_;
	this->__amount 	= obj.amount_;
	this->__index 	= index;
}

int PackageItem::red_grade()
{
	return this->conf()["color"].asInt() - GameEnum::ORANGE;
}

IntPair PackageItem::equip_part()
{
	int conf_part = this->conf()["part"].asInt();

	IntPair pair;
	pair.first  = (conf_part / 100) * 10;		//背包类型Index
	pair.second = conf_part % 100;				//部位

	return pair;
}

const Json::Value& PackageItem::conf()
{
	return CONFIG_INSTANCE->item(this->__id);
}

PackageItem &PackageItem::operator=(const PackageItem &item)
{
    this->__index = item.__index;
    this->__id = item.__id;
    this->__amount = item.__amount;
    this->__bind = item.__bind;

    this->__use_tick = item.__use_tick;
    this->__use_times = item.__use_times;
    this->__timeout = item.__timeout;

    this->__equipment = item.__equipment;
    this->__unique_id = item.__unique_id;
    this->__tips_level = item.__tips_level;

    for (int i = 0; i < (int)item.__tips_time_map.size(); ++i)
    {
    	int temp = item.__tips_time_map.find(i)->second;
    	this->__tips_time_map[i] = temp;

    	temp = item.__tips_status_map.find(i)->second;
    	this->__tips_status_map[i] = temp;
    }

    return (*this);
}

PackageItem::PackageItem(void)
{
	this->__hash_id = GlobalIndex::global_scene_hash_id_++;
	PackageItem::reset();
}

void PackageItem::reset(void)
{
    this->__index = 0;
    this->__id = 0;
    this->__amount = 0;
    this->__unique_id = 0;
    this->__bind = 0;
    this->__use_tick = 0;
    this->__new_tag = 0;
    this->__use_times = 0;
    this->__force_flag = 0;
    this->__timeout = 0;
    this->__tips_level = 0;

    this->__tips_time_map.clear();
    this->__tips_status_map.clear();

    this->__prop_info.reset();
    this->__equipment.reset();
    this->__rotary_table.reset();
}

bool PackageItem::validate_item()
{
	return this->__id > 0;
}

bool PackageItem::time_item()
{
	return this->__timeout > 0;
}

bool PackageItem::expire_item()
{
	JUDGE_RETURN(this->time_item() == true, false);
	return GameCommon::left_time(this->__timeout) <= 0;
}

bool PackageItem::is_equipment()
{
	return GameCommon::item_is_equipment(this->__id);
}

void PackageItem::set(int id, int amount, int bind, int index)
{
	this->__id = id;
	this->__amount = amount;
	this->__bind = bind;
	this->__index = index;
	this->__new_tag = 0;
    this->__unique_id = 0;
}

void PackageItem::caculate_fight_prop()	//战骑类装备
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, ;);

	this->__prop_info.reset();
	this->__prop_info.make_up_name_prop(conf);

	const Json::Value& add_prop = conf["add_prop"];
	for (uint i = 0; i < add_prop.size(); ++i)
	{
		this->__prop_info.make_up_name_prop(add_prop[i]);
	}

	this->__prop_info.caculate_force();
}

void PackageItem::caculate_fight_prop(int role_lvl, int strength_lvl)
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, ;);

	const Json::Value strengthen_conf = CONFIG_INSTANCE->equip_strengthen(
			this->__index, strength_lvl);

	int color = std::max<int>(1, conf["color"].asInt());
	string color_name = GameCommon::make_up_color_name(color);

	int grade_prop = conf["grade_inc"].asInt() * role_lvl;
	int strengthen_prop = strengthen_conf[color_name].asInt();

	this->__prop_info.reset();
	this->__prop_info.make_up_name_prop(conf, 1, grade_prop + strengthen_prop);

	for (IntMap::iterator iter = this->__equipment.__jewel_detail.begin();
			iter != this->__equipment.__jewel_detail.end(); ++iter)
	{
		const Json::Value& gewel_conf = CONFIG_INSTANCE->prop_unit(iter->second);
		this->__prop_info.make_up_name_prop(gewel_conf);
	}

	for (ItemObjMap::iterator iter = this->__equipment.__good_refine.begin();
			iter != this->__equipment.__good_refine.end(); ++iter)
	{
		const Json::Value& refine_conf = CONFIG_INSTANCE->prop_unit(iter->first);
		this->__prop_info.make_up_name_prop(refine_conf, iter->second.amount_);
	}

	MoldingSpiritDetail &molding_detail = this->__equipment.__molding_detail;
	molding_detail.__red_grade = this->red_grade() + GameEnum::ORANGE;
	string nature_name;
	for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
	{
		int cur_nature_schedule = molding_detail.fetch_nature_schedule(i);
		int cur_nature_level = molding_detail.fetch_nature_level(i);
		const Json::Value &config_value = CONFIG_INSTANCE->molding_spirit_info(i, cur_nature_level, molding_detail.__red_grade);

		int value = 0;
		if(i == GameEnum::MOLDING_SPIRIT_ALL_NATURE)
		{
			int nature_index = CONFIG_INSTANCE->molding_spirit_equip_nature(this->__index + 1)["nature"].asInt();
			nature_name = CONFIG_INSTANCE->molding_spirit_return_sys_nature_name(nature_index);
			value = config_value[nature_name].asInt();
		}
		else
		{
			nature_name = CONFIG_INSTANCE->molding_spirit_return_sys_nature_name(i);
			//当进度满的话，就读取full_nature属性
			if(cur_nature_schedule == config_value["cond"].asInt())
				value = config_value["full_nature"].asInt();
			else
				value = config_value["nature"].asInt();
		}

		this->__prop_info.make_up_one_prop(nature_name, value);
		//MSG_DEBUG("nature:%s, value:%d", nature_name.c_str(), value);
	}

	this->__prop_info.caculate_force();
}

ShopItem::ShopItem(void)
{
	ShopItem::reset();
}

void ShopItem::reset(void)
{
	this->item_id_ = 0;
	this->item_pos_ = 0;
	this->item_bind_ = 0;

	this->item_type_.clear();
	this->content_.clear();
	this->shop_type_ = 0;

	this->money_type_ = 0;
	this->src_price_ = 0;
	this->cur_price_ = 0;

	this->start_tick_ = 0;
	this->end_tick_ = 0;

	this->max_item_ = 0;
	this->max_total_ = 0;

	this->need_item_.first = 0;
	this->need_item_.second = 0;
}

bool ShopItem::check_time()
{
	Int64 now_tick = Time_Value::gettimeofday().sec();
	if(this->start_tick_ != 0  && now_tick < this->start_tick_)
		return false;
	if(this->end_tick_ != 0 && now_tick > this->end_tick_)
		return false;

	return true;
}

PageInfo::PageInfo(void)
{
	PageInfo::reset();
}

void PageInfo::reset(void)
{
	this->cur_page_ = 0;
	this->total_page_ = 0;

	this->start_index_ = 0;
	this->total_count_ = 0;

	this->add_count_ = 0;
}


SerialObj::SerialObj(const ProtoSerialObj& proto_serial) :
		type_(0), sub_(0), value_(0)
{
	this->unserialize(proto_serial);
}

SerialObj::SerialObj(int serial_type, int sub_serial, int serial_value)
{
	this->type_ = serial_type;
	this->sub_ =  sub_serial;
	this->value_ = serial_value;
}

void SerialObj::serialize(ProtoSerialObj* proto_serial) const
{
	proto_serial->set_serial_type(this->type_);
	proto_serial->set_sub_type(this->sub_);
	proto_serial->set_serial_value(this->value_);
}

void SerialObj::unserialize(const ProtoSerialObj& proto_serial)
{
	this->type_ = proto_serial.serial_type();
	this->sub_ = proto_serial.sub_type();
	this->value_ = proto_serial.serial_value();
}

ItemObj::ItemObj(int id, int amount,
		int bind, int index)
{
	ItemObj::reset();
	this->id_ 		= id;
	this->amount_ 	= amount;
	this->bind_ 	= bind;
	this->index_ 	= index;
}

ItemObj::ItemObj(const ProtoItem& proto_item)
{
	ItemObj::reset();
	this->unserialize(proto_item);
}

void ItemObj::reset(void)
{
	this->id_ 		= 0;
	this->amount_ 	= 0;

	this->bind_ 	= 0;
	this->index_ 	= 0;
    this->rand_ 	= 0;

    this->rand_start_times_ = 0;
    this->no_rand_times_ 	= 0;
    this->refine_level_ 	= 0;
}

void ItemObj::serialize(ProtoItem* proto_item, int type) const
{
	JUDGE_RETURN(this->id_ > 0, ;);
	JUDGE_RETURN(proto_item != NULL, ;);

	proto_item->set_id(this->id_);
	proto_item->set_amount(this->amount_);
	proto_item->set_bind(this->bind_);
	proto_item->set_index(this->index_);
}

void ItemObj::unserialize(const ProtoItem& proto_item)
{
	this->id_ 		= proto_item.id();
	this->amount_ 	= proto_item.amount();

	this->bind_ 	= proto_item.bind();
	this->index_ 	= proto_item.index();
}

void ItemObj::unserialize(PackageItem* item)
{
	this->id_ 		= item->__id;
	this->amount_ 	= item->__amount;
	this->bind_ 	= item->__bind;
}

bool ItemObj::validate() const
{
	return this->id_ > 0;
}

RecordEquipObj::RecordEquipObj()
{
	RecordEquipObj::reset();
}

RecordEquipObj::RecordEquipObj(int type, PackageItem* item)
{
	RecordEquipObj::reset();
	this->pack_type_ = type;
	this->set_attr_data(item);
}

void RecordEquipObj::reset()
{
	this->equip_id_ = 0;
	this->pack_type_ = 0;
	this->equip_index_ = 0;
	this->equip_bind_ = 0;
	this->strengthen_level_ = 0;
	this->refine_degree_ = 0;
	this->luck_value = 0;

	this->jewel_list.clear();
	this->good_refine.clear();
	this->molding_detail_.reset();
}

void RecordEquipObj::set_attr_data(PackageItem* item)
{
	JUDGE_RETURN(item != NULL, ;);

	this->equip_id_ = item->__id;
	this->equip_bind_ = item->__bind;
	this->equip_index_ = item->__index;

	this->luck_value = item->__equipment.__luck_value;
	this->refine_degree_ = item->__equipment.__refine_degree;
	this->strengthen_level_ = item->__equipment.strengthen_level_;
	this->jewel_list = item->__equipment.__jewel_detail;

	item->__equipment.fetch_refine(this->good_refine);

	this->molding_detail_.copy(item->__equipment.__molding_detail);
}

SerialInfo::SerialInfo(void) :
    __fresh_tick(0), __value(0), __copper(0), __gold(0),
    __bind_gold(0),__bind_copper(0)
{ /*NULL*/ }


Money::Money(const Int64 gold, const Int64 bind_gold, const Int64 copper, const Int64 bind_copper)
	: __gold(gold), __bind_gold(bind_gold), __copper(copper), __bind_copper(bind_copper)

{ /*NULL*/ }

Money::Money(const ProtoMoney& proto_money)
{
	Money::reset();
	Money::unserialize(&proto_money);
}

void Money::serialize(ProtoMoney* proto_money) const
{
	proto_money->set_gold(this->__gold);
	proto_money->set_copper(this->__copper);
	proto_money->set_bind_gold(this->__bind_gold);
	proto_money->set_bind_copper(this->__bind_copper);
}

void Money::unserialize(const ProtoMoney *proto_money)
{
	this->__gold = proto_money->gold();
	this->__copper = proto_money->copper();
	this->__bind_gold = proto_money->bind_gold();
	this->__bind_copper = proto_money->bind_copper();
}

void Money::unserialize(const ProtoMoney& proto_money)
{
	this->unserialize(&proto_money);
}

Money &Money::operator -=(const Money &money)
{
    __gold -= money.__gold;
    __copper -= money.__copper;
    __bind_gold -= money.__bind_gold;
    __bind_copper -= money.__bind_copper;
    return *this;
}

Money &Money::operator +=(const Money &money)
{
    __gold += money.__gold;
    __copper += money.__copper;
    __bind_gold += money.__bind_gold;
    __bind_copper += money.__bind_copper;

    return *this;
}

void Money::reset(void)
{
    ::memset(this, 0, sizeof(Money));
}

bool operator > (const Money &a, const Money &b)
{
	return a.__gold > b.__gold && a.__copper > b.__copper &&
			a.__bind_gold > b.__bind_gold && a.__bind_copper > b.__bind_copper;
}

bool operator >= (const Money &a, const Money &b)
{
	return a.__gold >= b.__gold && a.__copper >= b.__copper &&
			a.__bind_gold >= b.__bind_gold && a.__bind_copper >= b.__bind_copper;
}

bool operator < (const Money &a, const Money &b)
{
    return (a.__gold < b.__gold && a.__copper < b.__copper &&
        a.__bind_gold < b.__bind_gold && a.__bind_copper < b.__bind_copper);
}

bool operator <= (const Money &a, const Money &b)
{
    return (a.__gold <= b.__gold && a.__copper <= b.__copper &&
            a.__bind_gold <= b.__bind_gold && a.__bind_copper <= b.__bind_copper);
}

bool operator == (const Money &a, const Money &b)
{
	return a.__gold == b.__gold && a.__copper == b.__copper &&
			a.__bind_gold == b.__bind_gold && a.__bind_copper == b.__bind_copper;
}

bool operator != (const Money &a, const Money &b)
{
	return a.__gold != b.__gold || a.__copper != b.__copper ||
			a.__bind_gold != b.__bind_gold || a.__bind_copper != b.__bind_copper;
}

Money operator +(const Money &a, const Money &b)
{
    return Money(a.__gold + b.__gold, a.__bind_gold + b.__bind_gold,
    		a.__copper + b.__copper, a.__bind_copper + b.__bind_copper);
}

Money operator -(const Money &a, const Money &b)
{
    return Money(a.__gold - b.__gold, a.__bind_gold - b.__bind_gold,
    		a.__copper - b.__copper, a.__bind_copper - b.__bind_copper);
}

ActivityTimeInfo::ActivityTimeInfo(int day_check)
{
	ActivityTimeInfo::reset();
	this->day_check_ = day_check;
	this->set_freq_type(GameEnum::DAILY_ACTIVITY);
}

void ActivityTimeInfo::reset(void)
{
	/*
	 * freq_type and time_cycle not reset
	 * */
	this->cur_state_ = GameEnum::ACTIVITY_STATE_NO_START;
	this->time_set_.clear();
	this->week_day_.clear();

	this->time_index_ 	= 0;
	this->time_span_	= 0;
	this->refresh_time_	= 0;
	this->active_time_  = 0;
}

void ActivityTimeInfo::set_freq_type(int type)
{
	this->freq_type_ = type;

	if (this->freq_type_ == GameEnum::DAILY_ACTIVITY)
	{
		this->time_cycle_ = Time_Value::DAY;
	}
	else
	{
		this->time_cycle_ = Time_Value::WEEK;
	}
}

void ActivityTimeInfo::set_week_day(const IntVec& week_day)
{
	GameCommon::t_vec_to_map(this->week_day_, week_day);
}

void ActivityTimeInfo::set_next_stage()
{
	if (this->time_span_ == 1)
	{
		this->set_next_one_state();
	}
	else if (this->time_span_ == 2)
	{
		this->set_next_two_stage();
	}
	else
	{
		this->set_next_three_stage();
	}
}

void ActivityTimeInfo::set_next_one_state()
{
	this->time_index_ 	= GameCommon::mod_next(
			this->time_index_, this->fetch_time_group());
	this->refresh_time_ = this->fetch_next_cycle_time();
}

void ActivityTimeInfo::set_next_two_stage()
{
	if (this->cur_state_ == GameEnum::ACTIVITY_STATE_START)
	{
		// stop activity
		this->time_index_ 	= GameCommon::mod_next(
				this->time_index_, this->fetch_time_group());
		this->refresh_time_ = this->fetch_next_cycle_time();
		this->cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
	}
	else
	{
		// start activity
		int index 		= this->time_index_;
		int start_time 	= this->time_set_[index * this->time_span_];
		int end_time 	= this->time_set_[index * this->time_span_ + 1];

		this->refresh_time_ = end_time - start_time;
		this->cur_state_ 	= GameEnum::ACTIVITY_STATE_START;
	}
}

void ActivityTimeInfo::set_next_three_stage()
{
	int index 		= this->time_index_;
	int ahead_time 	= this->time_set_[index * this->time_span_];
	int start_time 	= this->time_set_[index * this->time_span_ + 1];
	int end_time 	= this->time_set_[index * this->time_span_ + 2];

	switch (this->cur_state_)
	{
	case GameEnum::ACTIVITY_STATE_NO_START:
	{
		this->refresh_time_ = start_time - ahead_time;
		this->cur_state_ 	= GameEnum::ACTIVITY_STATE_AHEAD;
		break;
	}

	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		this->refresh_time_ = end_time - start_time;
		this->cur_state_ 	= GameEnum::ACTIVITY_STATE_START;
		break;
	}

	case GameEnum::ACTIVITY_STATE_START:
	{
		this->time_index_ 	=  GameCommon::mod_next(this->time_index_,
				this->fetch_time_group());
		this->refresh_time_ = this->fetch_next_cycle_time();
		this->cur_state_ 	= GameEnum::ACTIVITY_STATE_NO_START;
		break;
	}
	}
}

int ActivityTimeInfo::fetch_time_group()
{
	return this->time_set_.size() / this->time_span_;
}

int ActivityTimeInfo::fetch_last_index()
{
	if (this->time_index_ > 0)
	{
		return (this->time_index_ - 1) % this->fetch_time_group();
	}
	else
	{
		return this->fetch_time_group() - 1;
	}
}

int ActivityTimeInfo::fetch_next_cycle_time()
{
	if (this->time_index_ == 0)
	{
		int total_size = this->time_set_.size();
		return this->time_cycle_ - this->time_set_[total_size - 1] + this->time_set_[0];
	}
	else
	{
		int cur_index = this->time_index_ * this->time_span_;
		return this->time_set_[cur_index] - this->time_set_[cur_index - 1];
	}
}

int ActivityTimeInfo::fetch_left_time()
{
	if (this->time_span_ == 2)
	{
		return this->fetch_two_left_time();
	}
	else
	{
		return this->fetch_three_left_time();
	}
}

int ActivityTimeInfo::fetch_two_left_time()
{
	if (this->cur_state_ == GameEnum::ACTIVITY_STATE_START)
	{
		int cur_sec = GameCommon::fetch_cur_sec(this->freq_type_);
		int cur_index = this->time_index_ * this->time_span_;
		return this->time_set_[cur_index + 1] - cur_sec;

	}
	else
	{
		return 0;
	}
}

int ActivityTimeInfo::fetch_three_left_time()
{
	int cur_sec = GameCommon::fetch_cur_sec(this->freq_type_);
	int cur_index = this->time_index_ * this->time_span_;

	switch (this->cur_state_)
	{
	case GameEnum::ACTIVITY_STATE_AHEAD:
	{
		return this->time_set_[cur_index + 1] - cur_sec;
	}

	case GameEnum::ACTIVITY_STATE_START:
	{
		return this->time_set_[cur_index + 2] - cur_sec;
	}
	}

	return 0;
}

int ActivityTimeInfo::fetch_start_time()
{
	if (this->time_span_ == 2)
	{
		return this->fetch_two_start_time();
	}
	else
	{
		return this->fetch_three_start_time();
	}
}

int ActivityTimeInfo::fetch_two_start_time()
{
	return this->time_set_[this->time_index_ * this->time_span_];
}

int ActivityTimeInfo::fetch_three_start_time()
{
	return this->time_set_[this->time_index_ * this->time_span_ + 1];
}

int ActivityTimeInfo::fetch_end_time()
{
	int last_index = this->fetch_last_index();
	return this->time_set_[last_index * this->time_span_];
}

int ActivityTimeInfo::fetch_after_end_left_time()
{
	JUDGE_RETURN(this->cur_state_ == GameEnum::ACTIVITY_STATE_NO_START, 0);

	int cur_sec = GameCommon::fetch_cur_day_sec();
	int end_sec = this->fetch_end_time();
	JUDGE_RETURN(cur_sec > end_sec, 0);

	return Time_Value::DAY - cur_sec;
}

int ActivityTimeInfo::client_state()
{
	if (this->cur_state_ != GameEnum::ACTIVITY_STATE_NO_START)
	{
		return this->cur_state_;
	}

	if (this->finish_type() == true)
	{
		return GameEnum::ACTIVITY_STATE_END;
	}

	return GameEnum::ACTIVITY_STATE_NO_START;
}

int ActivityTimeInfo::finish_type()
{
	JUDGE_RETURN(this->cur_state_ == GameEnum::ACTIVITY_STATE_NO_START, -1);

	int end_sec = this->fetch_end_time();
	int cur_sec = GameCommon::fetch_cur_day_sec();

	if (cur_sec > end_sec)
	{
		return true;	//已经结束
	}
	else
	{
		return false;	//未开始
	}
}

int ActivityTimeInfo::today_activity()
{
	tm cur_tm = GameCommon::fetch_cur_tm();
	return this->week_day_.count(cur_tm.tm_wday) > 0;
}

void DBInputArgv::reset()
{
	this->type_int_ = 0;
	this->type_void_ = NULL;

	this->type_int64_ = 0;
	this->type_string_.clear();

	this->extra_int_vec_.clear();
	this->extra_long_vec_.clear();
}

DBOutPutArgv::DBOutPutArgv():
		type_int_(0), type_void_(0),type_int64_(0)

{
	this->bson_obj_ = new BSONObj;
	this->bson_vec_ = new BSONVec;
}

DBOutPutArgv::~DBOutPutArgv(void)
{
	SAFE_DELETE(this->bson_obj_);
	SAFE_DELETE(this->bson_vec_);
}

void DBOutPutArgv::reset()
{
	this->type_int_ = 0;
	this->type_void_ = NULL;

	this->type_int64_ = 0;
	this->type_string_.clear();

	this->int_vec_.clear();
	this->void_vec_.clear();
	this->str_vec_.clear();

	this->long_vec_.clear();
	this->bson_vec_->clear();
}

DBShopMode::DBShopMode(void)
{
	DBShopMode::reset();
}

void DBShopMode::reset(void)
{
	this->recogn_ = 0;
	this->sub_value_ = 0;
	this->input_argv_.reset();
	this->output_argv_.reset();
}

DBTradeMode::DBTradeMode(void)
{
	this->query_ = new BSONObj;
	this->content_ = new BSONObj;
	this->operate_flag_ = true;
}

DBTradeMode::~DBTradeMode(void)
{
	SAFE_DELETE (this->query_);
	SAFE_DELETE(this->content_ );
}

void DBTradeMode::reset()
{
	this->operate_flag_ = true;
	*this->query_ = BSONObj();
	*this->content_ = BSONObj();
}

BaseRoleInfo::BaseRoleInfo(void)
{
	BaseRoleInfo::reset();
}

void BaseRoleInfo::reset(void)
{
	BaseServerInfo::reset();
	this->create_info_.reset();

    this->__name.clear();
    this->__src_name.clear();
    this->__account.clear();
    this->__agent.clear();

	this->__agent_code = 0;
	this->__market_code = 0;
	this->__sex = 0;
	this->__career = 0;
	this->__level = 0;
	this->__vip_type = 0;
	this->__fight_force = 0;

	this->__id = 0;
	this->__label_id = 0;
	this->__team_id = 0;
	this->__scene_id = 0;

	this->__login_tick = 0;
	this->__login_days = 0;
	this->__last_logout_tick = 0;
	this->__permission = GameEnum::PERT_NORMAL_PLAYER;
	this->__recharge_total_gold = 0;

	this->__level = 1;
	this->__career = 1;

	this->server_tick_ = 0;
	this->combine_tick_ = 0;

	this->__league_id = 0;
	this->__partner_id = 0;
	this->__partner_name.clear();
	this->__wedding_id = 0;
	this->__wedding_type = 0;
}

void BaseRoleInfo::set_name(const string& src_name)
{
	this->__src_name = src_name;
	this->__name = CONFIG_INSTANCE->full_role_name(
			this->__server_id, this->__src_name);
}

void BaseRoleInfo::set_sex()
{
	if (this->__sex == 1)
		this->__sex = 2;
	else if (this->__sex == 2)
		this->__sex = 1;

	this->__career = this->__sex;
}

void BaseRoleInfo::set_create_tick(time_t create_tick)
{
	this->create_info_.set_tick(create_tick);
}

void BaseRoleInfo::make_up_teamer_info(Int64 role_id, ProtoTeamer* teamer_info)
{
	teamer_info->set_role_id(role_id);
	teamer_info->set_role_name(this->name());
	teamer_info->set_role_level(this->__level);
	teamer_info->set_role_force(this->__fight_force);
	teamer_info->set_role_career(this->__career);
	teamer_info->set_role_sex(this->__sex);
	teamer_info->set_vip_type(this->__vip_type);
}

void BaseRoleInfo::inc_adjust_login_days()
{
	this->__login_days++;

	int open_day = CONFIG_INSTANCE->open_server_days();
	JUDGE_RETURN(this->__login_days > open_day, ;);

	this->__login_days = open_day;
}

void BaseRoleInfo::set_open_tick(Int64 tick)
{
	this->server_tick_ = tick;
	JUDGE_RETURN(this->server_tick_ == 0, ;);

	this->server_tick_ = CONFIG_INSTANCE->open_day_tick();
}

void BaseRoleInfo::set_combine_tick(Int64 tick)
{
	JUDGE_RETURN(CONFIG_INSTANCE->do_combine_server() == true, ;);

	this->combine_tick_ = tick;
	JUDGE_RETURN(this->combine_tick_ != CONFIG_INSTANCE->combine_day_tick(), ;);

	this->combine_tick_ = CONFIG_INSTANCE->combine_day_tick();
}


const char* BaseRoleInfo::name() const
{
	return this->__name.c_str();
}

bool BaseRoleInfo::is_validate_permi(int permi)
{
	if (permi == GameEnum::PERT_NORMAL_PLAYER
			&& this->__permission == 0)
	{
		return true;
	}

	return this->__permission == permi;
}

bool BaseRoleInfo::validate_server_open_day(int need_day)
{
	JUDGE_RETURN(this->server_tick_ > 0, false);
	return GameCommon::validate_time_span(this->server_tick_,
			need_day * Time_Value::DAY);
}

bool BaseRoleInfo::validate_max_combine_day(int max_day)
{
	JUDGE_RETURN(this->combine_tick_ > 0, false);

	Int64 span_time = ::time(NULL) - this->combine_tick_;
	return span_time < max_day * Time_Value::DAY;
}

CreateDaysInfo::CreateDaysInfo()
{
	CreateDaysInfo::reset();
}

void CreateDaysInfo::reset()
{
	this->init_ 			= 0;
	this->create_tick_ 		= 0;
	this->first_left_sec_	= 0;
}

void CreateDaysInfo::set_tick(time_t create_tick)
{
	tm create_tm;
	::localtime_r(&create_tick, &create_tm);

	this->create_tick_ = create_tick;
	this->first_left_sec_ = Time_Value::DAY - (create_tm.tm_hour * Time_Value::HOUR
			+ create_tm.tm_min * Time_Value::MINUTE + create_tm.tm_sec);
}

int CreateDaysInfo::passed_days()
{
	Int64 differ_sec = this->passed_time();

	if (differ_sec >= this->first_left_sec_)
	{
		differ_sec -= this->first_left_sec_;
		return 2 + differ_sec / Time_Value::DAY;
	}
	else
	{
		return 1;
	}
}

Int64 CreateDaysInfo::passed_time()
{
	return std::max<Int64>(::time(NULL) - this->create_tick_, 0);
}

Int64 CreateDaysInfo::create_day_tick()
{
	return this->create_tick_ + this->first_left_sec_ - Time_Value::DAY;
}

BaseMember::BaseMember(int type)
{
	BaseMember::reset();
	this->type_ = type;
}

void BaseMember::reset()
{
	this->id_ 		= 0;
	this->name_.clear();
	this->prev_.clear();

	this->sex_ 		= 0;
	this->force_ 	= 0;
	this->level_ 	= 0;
	this->vip_type_ = 0;
	this->fashion_ 	= 0;
	this->wing_ 	= 0;
}

void BaseMember::serialize(ProtoTeamer* proto)
{
	proto->set_role_id(this->id_);
	proto->set_role_name(this->name_);
	proto->set_role_sex(this->sex_);
	proto->set_role_level(this->level_);
	proto->set_role_force(this->force_);
	proto->set_vip_type(this->vip_type_);
}

void BaseMember::unserialize(const ProtoTeamer& proto)
{
	this->id_ 		= proto.role_id();
	this->name_ 	= proto.role_name();
	this->sex_		= proto.role_sex();
	this->level_ 	= proto.role_level();
	this->force_ 	= proto.role_force();
	this->vip_type_	= proto.vip_type();
}

void BaseMember::unserialize(const BaseRoleInfo& role, int fashion)
{
	this->id_ 		= role.__id;
	this->name_ 	= role.__name;
	this->sex_		= role.__sex;
	this->level_ 	= role.__level;
	this->force_ 	= role.__fight_force;
	this->prev_ 	= role.__server_prev;
	this->vip_type_ = role.__vip_type;
	this->fashion_ 	= fashion;
}

ReplacementRoleInfo::ReplacementRoleInfo(void)
{
	ReplacementRoleInfo::reset();
}

void ReplacementRoleInfo::reset(void)
{
	::memset(this, 0, sizeof(ReplacementRoleInfo));
	this->__career = 1;
}

RpmRecomandInfo::RpmRecomandInfo()
{
	RpmRecomandInfo::reset();
}

void RpmRecomandInfo::reset(void)
{
	__leader_force = 0;
	__teamates_id.clear();

	__info_count = 0;

	for(int i = 0; i< RPM_LIST_LENGTH; ++i)
	{
		__role_info_list[i].reset();
	}
}

RewardInfo::RewardInfo(int adjust, MapPlayer* player, MapLogicPlayer* logic_player)
{
	RewardInfo::reset();
	this->adjust_ = adjust;
	this->player_ = player;
	this->logic_player_ = logic_player;
}

const Json::Value& RewardInfo::conf() const
{
	return CONFIG_INSTANCE->reward(this->id_);
}

int RewardInfo::has_player()
{
	return this->player_ != NULL || this->logic_player_ != NULL;
}

int RewardInfo::history_use_times(int id)
{
	JUDGE_RETURN(this->logic_player_ != NULL, 0);

	MapLogicRoleDetail& detail = this->logic_player_->role_detail();
	return detail.rand_use_times_[id];
}

int RewardInfo::add_use_times(int id)
{
	JUDGE_RETURN(this->logic_player_ != NULL, 0);

	MapLogicRoleDetail& detail = this->logic_player_->role_detail();
	detail.rand_use_times_[id] += 1;

	return 0;
}

int RewardInfo::remve_use_times(int id)
{
	JUDGE_RETURN(this->logic_player_ != NULL, 0);

	MapLogicRoleDetail& detail = this->logic_player_->role_detail();
	detail.rand_use_times_[id] = 0;

	return 0;
}

void RewardInfo::reset()
{
	this->adjust_ = true;
	this->player_ = NULL;
	this->logic_player_ = NULL;

	this->id_ = 0;
	this->exp_ = 0;
	this->contri_ = 0;
	this->select_index_ = -1;

    this->money_.reset();
    this->item_vec_.clear();
    this->resource_map_.clear();

    this->item_vec_.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
}

void RewardInfo::set_player(MapLogicPlayer* player)
{
	this->logic_player_ = player;
}

void RewardInfo::add_rewards(const ItemObjVec& obj_vec)
{
	for (ItemObjVec::const_iterator iter = obj_vec.begin();
			iter != obj_vec.end(); ++iter)
	{
		this->add_rewards(*iter);
	}
}

void RewardInfo::add_rewards(const ProtoItem& proto_obj)
{
	ItemObj obj;
	obj.unserialize(proto_obj);
	this->add_rewards(obj);
}

void RewardInfo::add_rewards(const ItemObj& obj)
{
	JUDGE_RETURN(GameCommon::validate_item_id(obj.id_) == true, ;);

	if (this->adjust_ == true)
	{
		this->add_adjust_rewards(obj);
	}
	else
	{
		this->add_nonadjust_rewards(obj);
	}
}

void RewardInfo::add_adjust_rewards(const ItemObj& obj)
{
	if (GameCommon::is_money_item(obj.id_) == true)
	{
		this->money_ += GameCommon::money_item_to_money(obj);
	}
	else if (obj.id_ == GameEnum::ITEM_ID_PLAYER_EXP)
	{
		this->exp_ += obj.amount_;
	}
	else if (obj.id_ == GameEnum::ITEM_ID_LEAGUE_CONTRI)
	{
		this->contri_ += obj.amount_;
	}
	else if (GameCommon::is_resource_item(obj.id_) == true)
	{
		this->resource_map_[obj.id_] += obj.amount_;
	}
	else
	{
		this->add_pack_items(obj);
	}
}

void RewardInfo::add_nonadjust_rewards(const ItemObj& obj)
{
	this->item_vec_.push_back(obj);
}

void RewardInfo::add_pack_items(const ItemObj& obj)
{
	for (ItemObjVec::iterator iter = this->item_vec_.begin();
			iter != this->item_vec_.end(); ++iter)
	{
		JUDGE_CONTINUE(obj.id_ == iter->id_);
		iter->amount_ += obj.amount_;
		return;
	}

	this->item_vec_.push_back(obj);
}

MailInformation::MailInformation(void)
{
	MailInformation::reset();
}

void MailInformation::reset(void)
{
    this->mail_index_ = 0;
    this->send_time_ = 0;
    this->read_tick_ = 0;

    this->mail_type_ = 0;
    this->mail_format_ = 0;
    this->has_read_ = 0;

    this->sender_id_ = 0;
    this->sender_name_.clear();
    this->sender_vip_ = -1;

    this->receiver_id_ = 0;
    this->receiver_name_.clear();

    this->mail_title_.clear();
    this->mail_content_.clear();
    ZeroMemory(this->makeup_content_, sizeof(this->makeup_content_));

    this->recycle_goods();
    this->label_id_ = 0;
    this->money_.reset();
    this->attach_map_.clear();
    this->resource_map_.clear();
}

void MailInformation::fetch_goods_index(IntMap& goods_index)
{
	for (ItemListMap::iterator iter = this->goods_map_.begin();
			iter != this->goods_map_.end(); ++iter)
	{
		goods_index[iter->first] = true;
	}
}

void MailInformation::add_goods(const ItemObjVec& obj_vec)
{
	for (ItemObjVec::const_iterator iter = obj_vec.begin();
			iter != obj_vec.end(); ++iter)
	{
		this->add_goods(*iter);
	}
}

void MailInformation::add_goods(const ItemObj& obj)
{
	JUDGE_RETURN(GameCommon::validate_item_id(obj.id_) == true, ;);

	PackageItem* pack_item = GamePackage::pop_item(obj.id_);
	JUDGE_RETURN(pack_item != NULL, ;);

	pack_item->unserialize(obj, this->goods_map_.size());
	this->goods_map_[pack_item->__index] = pack_item;
}

void MailInformation::add_goods(int item_id, int item_amount, int item_bind)
{
	return this->add_goods(ItemObj(item_id, item_amount, item_bind));
}

void MailInformation::add_money(int money_type, int amount)
{
	int money_goods = GameCommon::get_item_by_money_type(money_type);
	JUDGE_RETURN(money_goods > 0, ;);

	return this->add_goods(money_goods, amount);
}

RewardInfo MailInformation::add_goods(int reward_id)
{
	RewardInfo reward_info(false);

	const Json::Value& reward_json = CONFIG_INSTANCE->reward(reward_id);
	JUDGE_RETURN(reward_json.empty() == false, reward_info);

	GameCommon::make_up_reward_items(reward_info, reward_json);

	for (ItemObjVec::iterator item_iter = reward_info.item_vec_.begin();
			item_iter != reward_info.item_vec_.end(); ++item_iter)
	{
		ItemObj& item_obj = *item_iter;
		this->add_goods(item_obj);
	}

	for (IntMap::iterator iter = reward_info.resource_map_.begin();
			iter != reward_info.resource_map_.end(); ++iter)
	{
		this->resource_map_[iter->first] += iter->second;
	}

//	this->money_ += reward_info.money_;

	if (reward_info.exp_ > 0)
	{
		this->add_goods(GameEnum::ITEM_ID_PLAYER_EXP,
				GameEnum::ITEM_BIND, reward_info.exp_);
	}

	return reward_info;
}

void MailInformation::recycle_self()
{
	this->recycle_goods();
	POOL_MONITOR->mail_info_pool()->push(this);
}

void MailInformation::recycle_goods()
{
	for (ItemListMap::iterator iter = this->goods_map_.begin();
			iter != this->goods_map_.end(); ++iter)
	{
		PackageItem* pack_item = iter->second;
		GamePackage::push_item(pack_item);
	}

	this->goods_map_.clear();
}

void MailInformation::serilize(ProtoMailInfo* proto_mail)
{
	proto_mail->set_mail_id(this->mail_index_);
	proto_mail->set_mail_type(this->mail_type_);
	proto_mail->set_format(this->mail_format_);
	proto_mail->set_mail_time(this->send_time_);
	proto_mail->set_role_name(this->sender_name_);
	proto_mail->set_mail_title(this->mail_title_);
	proto_mail->set_mail_read_tick(this->read_tick_);
	proto_mail->set_sender_id(this->sender_id_);
	proto_mail->set_receiver_id(this->receiver_id_);
	proto_mail->set_receiver_name(this->receiver_name_);
	proto_mail->set_mail_content(this->mail_content_);
	proto_mail->set_has_read(this->has_read_);
	proto_mail->set_label(this->label_id_);
	proto_mail->set_sender_vip(this->sender_vip_);

	ItemListMap::iterator it = this->goods_map_.begin();
	for(; it != this->goods_map_.end(); ++it)
	{
		PackageItem* item = it->second;
		JUDGE_CONTINUE(NULL != item);

		ProtoItem* proto_item = proto_mail->add_attach_list();
		item->serialize(proto_item);
	}
}

void MailInformation::unserilize(const ProtoMailInfo* proto_mail)
{
	this->mail_index_ = proto_mail->mail_id();
	this->mail_type_  = proto_mail->mail_type();
	this->mail_format_  = proto_mail->format();
	this->send_time_  = proto_mail->mail_time();
	this->sender_id_  = proto_mail->sender_id();
	this->sender_name_= proto_mail->role_name();
	this->mail_title_ = proto_mail->mail_title();
	this->read_tick_  = proto_mail->mail_read_tick();
	this->receiver_id_= proto_mail->receiver_id();
	this->receiver_name_ = proto_mail->receiver_name();
	this->mail_content_  = proto_mail->mail_content();
	this->has_read_      = proto_mail->has_read();
	this->label_id_   = proto_mail->label();
    this->sender_vip_ = proto_mail->sender_vip();

	for(int i = 0; i < proto_mail->attach_list_size(); ++i)
	{
		const ProtoItem& proto_item = proto_mail->attach_list(i);

		PackageItem* item = POOL_MONITOR->pack_item_pool()->pop();
		JUDGE_CONTINUE(NULL != item);

		item->unserialize(proto_item);
		this->goods_map_.insert(ItemListMap::value_type(
				i, item));

	}
}

FixedTimer::FixedTimer(int timer_type, int schedule_sec)
{
	this->timer_type_ = timer_type;
	this->schedule_sec_ = schedule_sec;
}

int FixedTimer::schedule_timer()
{
	Time_Value interval(this->schedule_sec_);
	return GameTimer::schedule_timer(interval);
}

int FixedTimer::type(void)
{
	return this->timer_type_;
}

FriendInfo::FriendInfo(void)
{
	FriendInfo::reset();
}

void FriendInfo::reset(void)
{
    this->__role_id = 0;
    this->__icon_id = 0;
    this->__league_id = 0;
    this->__friend_type = 0;
    this->__vip_status = 0;
    this->__is_online = 0;

    this->__sex = 0;
    this->__career = 0;
    this->__level = 0;
    this->__team_status = 0;
    this->__stranger_tick = 0;
    this->__black_tick = 0;
    this->__force = 0;
    this->__intimacy = 0;
    this->__name.clear();
}

DBFriendInfo::DBFriendInfo(void) :
    __friend_type(0)
{ /*NULL*/ }

void DBFriendInfo::reset(void)
{
	this->__friend_type = 0;
	this->__friend_info_vec.clear();
	this->__offine_set.clear();
}

MallActivityDetail::MallActivityDetail(void)
{
	MallActivityDetail::reset();
}

void MallActivityDetail::reset()
{
	this->__activity_id = 0;
	this->__daily_refresh_type = 0;
	this->__open_activity = 0;
	this->__refresh_tick = 0;
	this->__last_save_tick = 0;
	this->__limit_type = 0;
    this->__server_limit_amount = 0;
    this->__single_limit_amount = 0;
	this->__data_change = false;
	this->__activity_name.clear();
	this->__activity_memo.clear();
	this->__server_buy_map.clear();

	MallBuyRecord::iterator it = this->__player_record.begin();
	for(; it != this->__player_record.end(); ++it)
	{
		it->second.clear();
	}
	this->__player_record.clear();
}

AchieveDetail::AchieveDetail(void)
{
	AchieveDetail::reset();
}

void AchieveDetail::reset()
{
	this->achieve_id_ 	= 0;
	this->ach_index_ 	= 0;
	this->get_status_ 	= 0;
	this->finish_num_ 	= 0;
	this->finish_tick_ 	= 0;
	this->special_value_ = 0;
}

BaseVipDetail::BaseVipDetail(void)
{
	BaseVipDetail::reset();
}

const Json::Value& BaseVipDetail::conf()
{
	return CONFIG_INSTANCE->vip(this->__vip_type);
}

bool BaseVipDetail::is_vip()
{
	return this->__vip_type >= VIP_1 && this->__vip_type <= VIP_MAX;
}

bool BaseVipDetail::is_max_vip()
{
	return this->__vip_type >= VIP_MAX;
}

void BaseVipDetail::set_vip_type(int vip_type)
{
	this->__vip_type = vip_type;

	if (this->__vip_type > VIP_NOT_VIP)
	{
		this->__vip_level = (this->__vip_type % VIP_1) + 1;
	}
	else
	{
		this->__vip_level = 0;
	}
}

void BaseVipDetail::reset()
{
	this->__vip_type = VIP_NOT_VIP;
	this->__vip_level = 0;
	this->__expired_time = 0;
    this->__start_time = 0;
}

InvestRechargeDetail::InvestRechargeDetail()
{
	InvestRechargeDetail::reset();
}

void InvestRechargeDetail::reset(void)
{
	this->__is_buy = 0;
	this->__vip_level = 0;
	this->__buy_time.reset();

	for (int i = DAILY_1; i < INVEST_TYPE_NUM; ++i)
	{
		this->__invest_rewards[i] = REWARD_NONE;
		this->__vip_rewards[i] = REWARD_NONE;
	}
}

int InvestRechargeDetail::cur_max_days()
{
	return std::min<int>(this->__buy_time.passed_days(), DAILY_7);
}

int InvestRechargeDetail::is_passed_max_days()
{
	return this->__buy_time.passed_days() > DAILY_7;
}

void BrocastRole::reset()
{
	::memset(this, 0, sizeof(BrocastRole));
}

BrocastPara::BrocastPara(void)
{
    this->__shout_item = new ProtoShoutItem();
    BrocastPara::reset();
}

BrocastPara::~BrocastPara(void)
{
    SAFE_DELETE(this->__shout_item);
}

void BrocastPara::reset()
{
    this->__parse_type = 0;
	::memset(&(this->__parse_data), 0, sizeof(this->__parse_data));
    this->__shout_item->Clear();
}

BrocastPara &BrocastPara::operator=(const BrocastPara &para)
{
    this->__parse_type = para.__parse_type;
    ::memcpy(&(this->__parse_data), &(para.__parse_data), sizeof(this->__parse_data));
    *(this->__shout_item) = *(para.__shout_item);
    return *this;
}

BrocastPara::BrocastPara(const BrocastPara &para)
{
	this->__shout_item = new ProtoShoutItem();
    this->__parse_type = para.__parse_type;
    ::memcpy(&(this->__parse_data), &(para.__parse_data), sizeof(this->__parse_data));
    *(this->__shout_item) = *(para.__shout_item);
}

PlayerAssistTip::PlayerAssistTip(void)
{
	PlayerAssistTip::reset();
}

void PlayerAssistTip::reset(void)
{
	::memset(this, 0, sizeof(PlayerAssistTip));
}

void PlayerAssistTip::serilize(ProtoPairObj* proto)
{
	proto->set_obj_id(this->__event_id);
	proto->set_obj_value(this->__tips_flag);
}

SysSetting::SysSetting()
{
	SysSetting::reset();
}

void SysSetting::reset()
{
	this->__is_shock = 0;              //是否振动
	this->__screen_type = 0;           //游戏画面：1流畅 smooth， 2 平衡 balance， 3精美 perfect
	this->__fluency_type = 0;           //游戏画面：1流畅 smooth， 2 平衡 balance， 3精美 perfect
	this->__shield_type.clear();           //屏蔽类型
	this->__turnoff_act_notify = 0;    //是否关闭活动提示，1关闭，0开启
	this->__auto_adjust_express = 0;	 //是否自动调节游戏表现 1关闭 0开启
	this->__music_effect.id_ = 0;      //音乐: obj_id：是否勾选，obj_value：音量
	this->__music_effect.value_ = 0;
	this->__sound_effect.id_ = 0;      //音效: obj_id：是否勾选，obj_value：音量
	this->__sound_effect.value_ = 0;
}

RechargeOrder::RechargeOrder()
{
	RechargeOrder::reset();
}

void RechargeOrder::reset(void)
{
	this->__order_id = 0;
	this->__money = 0;
	this->__gold = 0;
	this->__channel_id = 0;
	this->__tick = 0;
	this->__role_id = 0;

	this->__order_num.clear();
	this->__account.clear();
}


MailDetailSerialObj::MailDetailSerialObj()
{
	MailDetailSerialObj::reset();
}

void MailDetailSerialObj::reset()
{
	this->__title.clear();
	this->__content.clear();
	this->__sender_name.clear();
	this->__receiver_name.clear();

	this->__read_tick = 0;
	this->__send_tick = 0;
	this->__sender_id = 0;
	this->__receiver_id = 0;
	this->__mail_index = 0;
	this->__mail_type = 0;
	this->__mail_format_ = 0;
	this->__has_read = 0;

	this->__attach_money.reset();
	this->__attach_item.clear();
}

ScoreInfo::ScoreInfo()
{
	this->id_ = 0;
	this->name_.clear();

	this->score_ = 0;
	this->tick_ = 0;
}

const std::string GameSwitcherDetail::Names::market = "market";
const std::string GameSwitcherDetail::Names::shop = "shop";
const std::string GameSwitcherDetail::Names::mail = "mail";
const std::string GameSwitcherDetail::Names::mall_gift = "mall_gift";
const std::string GameSwitcherDetail::Names::first_recharge = "first_recharge";
const std::string GameSwitcherDetail::Names::trade = "trading";
const std::string GameSwitcherDetail::Names::box = "box";
const std::string GameSwitcherDetail::Names::rank = "rank";
const std::string GameSwitcherDetail::Names::first_double_return = "first_double_return";
const std::string GameSwitcherDetail::Names::download_box_gift = "download_box_gift";
const std::string GameSwitcherDetail::Names::love_gift = "love_gift";
const std::string GameSwitcherDetail::Names::equip_red = "equip_red";
const std::string GameSwitcherDetail::Names::treasures = "treasures";
const std::string GameSwitcherDetail::Names::gift = "gift";
const std::string GameSwitcherDetail::Names::transfer = "transfer";
const std::string GameSwitcherDetail::Names::molding_spirit = "molding_spirit";
const std::string GameSwitcherDetail::Names::jewel_sublime = "jewel_sublime";
const std::string GameSwitcherDetail::Names::special_box = "special_box";

void GameSwitcherDetail::reset(void)
{
	this->switcher_map_[Names::shop] 		= true;
	this->switcher_map_[Names::market] 		= true;
	this->switcher_map_[Names::equip_red] 	= false;
	this->switcher_map_[Names::treasures] 	= false;
	this->switcher_map_[Names::gift] 		= false;
#ifdef LOCAL_DEBUG
	this->switcher_map_[Names::transfer] 	= true;
#else
	this->switcher_map_[Names::transfer] 	= false;
#endif

	this->switcher_map_[Names::molding_spirit] 		= false;
	this->switcher_map_[Names::jewel_sublime] 		= false;
	this->switcher_map_[Names::first_double_return] = false;
	this->switcher_map_[Names::special_box] = false;
}

bool GameSwitcherDetail::has_update(GameSwitcherDetail &other, const char* name)
{
	for (BStrIntMap::iterator iter = this->switcher_map_.begin();
			iter != this->switcher_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second != other.switcher_map_[iter->first]);
		return true;
	}

	return false;
}

const std::string BackGameModify::Names::league_fb = "league_fb";
const std::string BackGameModify::Names::gongcheng = "gongcheng";
const std::string BackGameModify::Names::wildqixi = "wildqixi";
const std::string BackGameModify::Names::you49_qq = "49you_qq";
const std::string BackGameModify::Names::fashion_box = "fashion_box";
const std::string BackGameModify::Names::lucky_table = "lucky_table";

void BackGameModify::reset()
{
	this->name.clear();
	this->is_update = 0;
	this->role_id = 0;
	this->league_id = 0;
	this->value = 0;
	this->value_str.clear();
}
void BackGameModify::LuckyTable::reset()
{
	this->activity_id = 0;
	this->open_state = 0;
	this->start_tick[0] = 0;
	this->start_tick[1] = 0;
	this->end_tick[0] = 0;
	this->end_tick[1] = 0;
}

void BackGameModify::SuperVipInfo::reset()
{
	this->qq_num.clear();
	this->des_content.clear();
	this->des_mail.clear();
	this->vip_level_limit = 0;
	this->need_recharge = 0;
}

FighterSkill::FighterSkill(int skill, int level)
{
	FighterSkill::reset(skill, level);
}

void FighterSkill::reset(int skill, int level)
{
    this->__level = level;
    this->__used_times = 0;
    this->__use_rate = 0;
    this->__server_force = 0;
    this->__need_used_times = 0;
    this->__fight_use_times = 0;

    this->__skill_id 	= skill;
    this->__aoe_type 	= 0;
    this->__lend_skill 	= 0;
    this->__launch_way 	= 0;
    this->__rand_step 	= 0;
    this->__max_step 	= 0;
    this->__use_level 	= 1;
    this->__full_screen = 0;
    this->__db_flag 	= 0;
    this->__check_flag 	= 0;
    this->__level_type 	= 0;

	this->__object 		= 0;
	this->__radius 		= 0;
	this->__distance 	= 0;
    this->__is_loop 	= 0;
    this->__is_mutual 	= 0;
    this->__max_times   = 0;
    this->__effect_ai_skill = 0;
    this->__sub_rate_skill 	= 0;
    this->__sub_rate_skill_2= 0;
    this->__passive_trigger = 0;

    this->__skill_type 			= 0;
    this->__transfer_no_release	= 0;
    this->__del_buff 			= 0;
    this->__is_launch_once 		= 0;
    this->__no_object_limit 	= 0;
    this->__object_from_server 	= 0;

    this->__target_type.clear();

    this->__cool = Time_Value::zero;
    this->__use_tick = Time_Value::zero;
}

void FighterSkill::add_use_tick()
{
	this->__use_tick = Time_Value::gettimeofday() + this->__cool;
}

void FighterSkill::serialize(ProtoSkill* proto, int client)
{
	proto->set_skill_id(this->__skill_id);
	proto->set_skill_level(this->skill_level(client));
	proto->set_use_times(this->__used_times);
	proto->set_use_sec(this->__use_tick.sec());
	proto->set_use_usec(this->__use_tick.usec());
}

void FighterSkill::unserialize(const ProtoSkill& proto)
{
    this->__use_tick = Time_Value(proto.use_sec(), proto.use_usec());
	this->__used_times = proto.use_times();
}

void FighterSkill::serialize(ProtoPairObj* proto)
{
	proto->set_obj_id(this->__skill_id);
	proto->set_obj_value(this->__level);
}

void FighterSkill::unserialize(const ProtoPairObj& proto)
{
}

bool FighterSkill::is_cool_finish()
{
	return this->__use_tick <= Time_Value::gettimeofday();
}

bool FighterSkill::is_active_skill()
{
	return (this->__skill_type / 100) == 1;
}

bool FighterSkill::is_passive_skill()
{
	return (this->__skill_type / 100) == 2;
}

bool FighterSkill::is_passive_prop_skill()
{
	return (this->__skill_type / 100) == 3;
}

bool FighterSkill::is_passive_buff_skill()
{
	return (this->__skill_type / 100) == 4;
}

bool FighterSkill::arrive_fight_max_times()
{
	JUDGE_RETURN(this->__max_times > 0, false);
	return this->__fight_use_times >= this->__max_times;
}

int FighterSkill::fun_bit()
{
	return GameCommon::fetch_skill_fun_bit(this->__skill_id);
}

int FighterSkill::skill_id()
{
	if (this->__lend_skill > 0)
	{
		return this->__lend_skill;
	}
	else
	{
		return this->__skill_id;
	}
}

int FighterSkill::skill_level(int client)
{
	if (client == false)
	{
		return this->__level;
	}

    int fun_bit = GameCommon::fetch_skill_id_fun_type(this->__skill_id);
    if (fun_bit != GameEnum::SKILL_FUN_PASSIVE)
    {
    	return this->__level;
    }

  	//人物被动技能
    return this->__level - 1;
}

int FighterSkill::level_conf(const Json::Value& json)
{
	return GameCommon::json_by_level(json, this->__level).asInt();
}

const Json::Value& FighterSkill::conf()
{
	return CONFIG_INSTANCE->skill(this->__skill_id);
}

const Json::Value& FighterSkill::detail()
{
	return CONFIG_INSTANCE->skill_detail(this->__skill_id, this->__level);
}

SecondTimeout::SecondTimeout()
{
	this->start_ 	= 0;
	this->going_ 	= 0;
	this->interval_ = 0;
	this->sub1_ 	= 0;
	this->sub2_		= 0;
	this->index_ 	= 0;
}

void SecondTimeout::stop_time()
{
	this->start_ = false;
}

void SecondTimeout::reset_time()
{
	this->going_ = 0;
}

void SecondTimeout::start_interval(int interval)
{
	this->going_ 	= 0;
	this->start_ 	= true;
	this->interval_ = interval;
}

bool SecondTimeout::update_time()
{
	JUDGE_RETURN(this->start_ == true, false);

	this->going_ += 1;
	return this->is_timeout();
}

bool SecondTimeout::is_timeout()
{
	return this->going_ >= this->interval_;
}

LeftTimeOut::LeftTimeOut()
{
	this->left_time_ = 0;
}

void LeftTimeOut::set_time(int left_time)
{
	JUDGE_RETURN(left_time > 0, ;);
	this->left_time_ = left_time;
}

bool LeftTimeOut::reduce_time()
{
	JUDGE_RETURN(this->left_time_ > 0, false);
	this->left_time_ -= 1;
	return true;
}

bool LeftTimeOut::is_zero()
{
	return this->left_time_ <= 0;
}

MoldingSpiritDetail::MoldingSpiritDetail()
{
	MoldingSpiritDetail::reset();
}

void MoldingSpiritDetail::reset()
{
	this->__nature_level_map.clear();
	this->__nature_schedule_map.clear();
	__red_grade = 0;
}

int MoldingSpiritDetail::fetch_nature_level(int type)
{
	if(this->__nature_level_map.count(type) == 0)
	{
		this->__nature_level_map[type] = 0;
	}
	return this->__nature_level_map[type];
}

int MoldingSpiritDetail::update_nature_level(int type, int level, bool is_set)
{
	//GameEnum::MOLDING_SPIRIT_ALL_NATURE 没有进度的
	if(is_set)
		this->__nature_level_map[type] = level;
	else
		this->__nature_level_map[type] += level;
	this->__nature_schedule_map[type] = 0;
//	MSG_USER("molding type:%d, update level:%d", type, this->__nature_level_map[type]);
	return 0;
}

int MoldingSpiritDetail::fetch_nature_schedule(int type)
{
	if(this->__nature_schedule_map.count(type) == 0)
	{
		this->__nature_schedule_map[type] = 0;
	}
	return this->__nature_schedule_map[type];
}

int MoldingSpiritDetail::update_nature_schedule(int type, int schedule, bool is_set)
{
	const string str_cond = "cond", str_red_grade = "red_grade";
	int cur_level = this->fetch_nature_level(type);
	const Json::Value &molding_config = CONFIG_INSTANCE->molding_spirit_info(type, cur_level);
	int need_schedule = molding_config[str_cond].asInt();
	int need_red_grade = molding_config[str_red_grade].asInt();
	JUDGE_RETURN(need_schedule > 0, -1);

	int cur_schedule = this->fetch_nature_schedule(type);
	if(cur_schedule + schedule > need_schedule)
	{
		//获取铸魂总属性等级，属性等级不能超过总属性等级和需要达到对应属性的阶数条件
		if(cur_level < this->fetch_nature_level(GameEnum::MOLDING_SPIRIT_ALL_NATURE)
				&& this->__red_grade >= need_red_grade)
		{
			this->update_nature_level(type, 1);
			int sub = cur_schedule + schedule - need_schedule;
			if(sub > 0)
				this->update_nature_schedule(type, sub);
		}
		else
			return -1;
	}
	else
	{
		if(is_set)
			this->__nature_schedule_map[type] = schedule;
		else
			this->__nature_schedule_map[type] += schedule;

		if(this->__nature_schedule_map[type] > need_schedule)
			this->__nature_schedule_map[type] = need_schedule;

//		MSG_USER("update schedule_map type:%d schedule:%d", type, this->__nature_schedule_map[type]);
	}
	return 0;
}

int MoldingSpiritDetail::check_up_all_nature()
{
	int save_level = this->__nature_level_map[GameEnum::MOLDING_SPIRIT_BEGIN];
	for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_ALL_NATURE; ++i)
	{
		const Json::Value value = CONFIG_INSTANCE->molding_spirit_info(i, this->__nature_level_map[i]);
		JUDGE_RETURN(Json::Value::null != value, - 1);
		int config_cond = value["cond"].asInt();
		//所有属性等级必须相等和进度都为最大值的时候
		if(this->__nature_level_map[i] != save_level
				|| this->fetch_nature_schedule(i) != config_cond)
			return -1;
	}
	return 0;
}

MoldingSpiritDetail* MoldingSpiritDetail::copy(const MoldingSpiritDetail& detail)
{
	this->__red_grade = detail.__red_grade;
	this->__nature_schedule_map = detail.__nature_schedule_map;
	this->__nature_level_map = detail.__nature_level_map;
	return this;
}

MoldingSpiritDetail* MoldingSpiritDetail::operator =(const MoldingSpiritDetail& rhs)
{
	return this->copy(rhs);
}
