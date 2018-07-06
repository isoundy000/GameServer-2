/*
 * MMORole.cpp
 *
 * Created on: 2013-03-21 11:38
 *     Author: lyz
 */

#include "GameField.h"
#include "BackField.h"

#include "ArenaSys.h"
#include "GateMonitor.h"

#include "MMORole.h"
#include "MMOLeague.h"
#include "MMOWedding.h"
#include "MongoConnector.h"

#include "LogicPlayer.h"
#include "MapPlayerEx.h"
#include "GatePlayer.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"

#include "LogicStruct.h"
#include "DBCommon.h"
#include "MongoData.h"
#include "MongoDataMap.h"
#include "MongoException.h"
#include <mongo/client/dbclient.h>
using namespace mongo;

MMORole:: ~MMORole(void)
{ /*NULL*/ }

int MMORole::load_player(GatePlayer *player)
{
BEGIN_CATCH

	this->set_role_permission_info(player->detail());
	this->set_gm_permission(player);
	this->set_ban_ip_info(player);
	this->set_mac_ban_info(player);

    GateRoleDetail &detail = player->detail();
    BSONObj ret_fields = BSON(Role::IS_ACTIVE << 1
            << Role::SERVER_FLAG << 1
            << Role::ACCOUNT << 1 << Role::ID << 1
            << Role::NAME << 1 << Role::LEVEL << 1
            << Role::SEX << 1 << Role::CAREER << 1
            << Role::LOCATION << 1
            << Role::LAST_SIGN_OUT << 1
            << Role::PERMISSION << 1
            << Role::AGENT << 1
            << Role::AGENT_CODE << 1
            << Role::PLATFORM_CODE << 1
            << Role::MARKET_CODE << 1
            << Role::BAN_TYPE << 1
            << Role::BAN_EXPIRED << 1);
    
    BSONObj res;
    if (this->fetch_player(res, player, ret_fields) == false)
    {
        player->set_active(false);
        return ERROR_ROLE_NOT_EXISTS;
    }

    player->set_active(true);
    player->set_role_id(res[Role::ID].numberLong());

    if(player->is_ban_ip() == false && player->is_ban_mac() == false)
    {
        player->set_ban_state(res[Role::BAN_TYPE].numberInt(),
        		res[Role::BAN_EXPIRED].numberLong());
    }

    MMORole::init_base_role_info(detail, res);

    BSONObj move_obj = res.getObjectField(Role::LOCATION.c_str());
    detail.__pixel_x = move_obj[Role::Location::PIXEL_X].numberInt();
    detail.__pixel_y = move_obj[Role::Location::PIXEL_Y].numberInt();
    detail.__scene_mode = move_obj[Role::Location::MODE].numberInt();
    detail.__space_id = move_obj[Role::Location::SPACE_ID].numberInt();

//    if (CONFIG_INSTANCE->scene(detail.__scene_id) == Json::Value::null)
//    {
//        detail.__scene_id = GameEnum::BHYK_SCENE_ID;
//        this->conection().update(Role::COLLECTION, QUERY(Role::ID << (player->role_id())),
//                BSON("$set" << BSON((Role::LOCATION + "." + Role::Location::SCENE_ID) << detail.__scene_id)), false);
//    }

    this->save_account_phone_info(player);
    return 0;

END_CATCH
    return -1;
}

int MMORole::fetch_player(BSONObj& res, GatePlayer* player, const BSONObj& fields)
{
	if (player->detail().__server_flag.empty() == false)
	{
		res = this->conection().findOne(Role::COLLECTION, QUERY(Role::ACCOUNT
				<< player->account() << Role::IS_ACTIVE << 1
				<< Role::SERVER_FLAG << player->detail().__server_flag), &fields);
	}

	if (res.isEmpty() == false)
	{
		return true;
	}

	auto_ptr<DBClientCursor> cursor = this->conection().query(Role::COLLECTION,
			QUERY(Role::ACCOUNT	<< player->account() << Role::IS_ACTIVE << 1));

	ThreeObjVec three_set;
	while (cursor->more())
	{
		BSONObj obj = cursor->next();

		ThreeObj three_obj;
		three_obj.id_ = obj[Role::ID].numberLong();
		three_obj.value_ = obj[Role::LEVEL].numberInt();
		three_obj.tick_ = obj[Role::EXP].numberLong();

		three_set.push_back(three_obj);
	}

	int total_size = three_set.size();
	if (total_size > 0)
	{
		std::sort(three_set.begin(), three_set.end(), GameCommon::three_comp_by_asc);
		res = this->conection().findOne(Role::COLLECTION, QUERY(Role::ID
				<< three_set[total_size - 1].id_), &fields);

		return res.isEmpty() == false;
	}
	else
	{
		return false;
	}
}

int MMORole::save_new_back_role(GatePlayer *player)
{
BEGIN_CATCH
	GateRoleDetail& detail = player->detail();
	BSONObj ret_field = BSON(DBBackRole::ROLE_ID << 1);

	int is_new_mac = 1;
	if (this->conection().findOne(DBBackRole::COLLECTION,
			QUERY(DBBackRole::MAC << detail.__mac), &ret_field).isEmpty() == false)
	{
		is_new_mac = 0;
	}

    BSONObjBuilder builder;
    builder << DBBackRole::SERVER_FLAG << detail.__server_flag
        << DBBackRole::MARKET << player->market_code()
        << DBBackRole::AGENT << player->agent()
        << DBBackRole::AGENT_CODE << player->agent_code()
        << DBBackRole::NET_TYPE << detail.__net_type
		<< DBBackRole::SYS_VERSION << detail.__sys_version
		<< DBBackRole::SYS_MODEL << detail.__sys_model
		<< DBBackRole::MAC << detail.__mac
		<< DBBackRole::IP << player->client_ip()
		<< DBBackRole::IMEI << detail.__imei
	    << DBBackRole::CREATE_MARKET_CODE << player->market_code()
    	<< DBBackRole::CREATE_AGENT << player->agent()
    	<< DBBackRole::CREATE_AGENT_CODE << player->agent_code()
    	<< DBBackRole::CREATE_NET_TYPE << detail.__net_type
    	<< DBBackRole::CREATE_SYS_VERSION << detail.__sys_version
    	<< DBBackRole::CREATE_SYS_MODEL << detail.__sys_model
    	<< DBBackRole::CREATE_MAC << detail.__mac
    	<< DBBackRole::CREATE_IMEI << detail.__imei
    	<< DBBackRole::CREATE_IP << player->client_ip()
    	<< DBBackRole::IS_NEW_MAC << is_new_mac
        << DBBackRole::ROLE_ID << player->role_id()
        << DBBackRole::ROLE_NAME << detail.name()
        << DBBackRole::ACCOUNT << detail.__account
        << DBBackRole::CAREER << detail.__career
        << DBBackRole::PERMISSION << player->detail().__permission
        << DBBackRole::LEVEL << 1
        << DBBackRole::SEX << player->detail().__sex
        << DBBackRole::CREATE_TIME << Int64(::time(0))
        << DBBackRole::LOGIN_COUNT << 0;

    this->conection().update(DBBackRole::COLLECTION,
            QUERY(DBBackRole::ROLE_ID << player->role_id()),
            BSON("$set" << builder.obj()), true);
    return 0;
END_CATCH
    return -1;
}

int MMORole::save_account_phone_info(GatePlayer *player)
{
	{
		GateRoleDetail& detail = player->detail();

		BSONObjBuilder builder;
		builder << DBBackRole::NET_TYPE << detail.__net_type
				<< DBBackRole::SYS_VERSION << detail.__sys_version
				<< DBBackRole::SYS_MODEL << detail.__sys_model
				<< DBBackRole::MAC << detail.__mac
				<< DBBackRole::IMEI << detail.__imei
				<< DBBackRole::IP << player->client_ip()
				<< DBBackRole::MARKET << player->market_code()
				<< DBBackRole::AGENT << player->agent()
				<< DBBackRole::AGENT_CODE << player->agent_code();

		this->conection().update(DBBackRole::COLLECTION,
				QUERY(DBBackRole::ROLE_ID << player->role_id()),
				BSON("$set" << builder.obj()), true);
	}

	{
		BSONObjBuilder builder;
		builder << Role::AGENT << player->agent()
				<< Role::AGENT_CODE << player->agent_code()
				<< Role::MARKET_CODE << player->market_code();

		this->conection().update(Role::COLLECTION, QUERY(Role::ID << player->role_id()),
				BSON("$set" << builder.obj()), true);
	}

	return 0;
}

int MMORole::is_can_create_role(const string agent_code,const string ip,const string mac)
{
	unsigned long long ip_n = this->conection().count(DBBackRole::COLLECTION,
			BSON(DBBackRole::CREATE_AGENT << agent_code << DBBackRole::CREATE_IP << ip));
	unsigned long long mac_n = this->conection().count(DBBackRole::COLLECTION,
			BSON(DBBackRole::CREATE_AGENT << agent_code << DBBackRole::CREATE_MAC << mac));

	const Json::Value &agent_json = CONFIG_INSTANCE->agent_of_ip_mac();
	if (agent_json == Json::Value::null)
	{
		return true;
	}

	if (agent_json.isMember(agent_code.c_str()) == false)
	{
		return true;
	}

	const Json::Value &one_agent_json = agent_json[agent_code.c_str()];
	if (one_agent_json.isMember("ip_unlimit"))
	{
		for(uint i = 0; i < one_agent_json.size();++i)
		{
			if(strcmp(ip.c_str(),one_agent_json["ip_unlimit"][i].asString().c_str()) == 0)
			{
				return true;
			}
		}
	}

	if (one_agent_json.isMember("ip_limit"))
	{
		int ip_max = one_agent_json["ip_limit"].asInt();
		if((int)ip_n >= ip_max)
			return -1;
	}

	if (one_agent_json.isMember("mac_limit"))
	{
		int mac_max = one_agent_json["mac_limit"].asInt();
		if((int)mac_n >= mac_max)
			return -2;
	}

	return true;
}

int MMORole::save_new_role(GatePlayer *player)
{
BEGIN_CATCH

	this->set_role_permission_info(player->detail());
    Int64 role_id = this->conector_->update_global_key(Global::ROLE);
    if (role_id < 0)
    {
        MSG_USER("ERROR global role key");
        return -1;
    }
//    int64_t agent_code = (((int64_t)(player->agent_code() * 100000)) << 32);
//    role_id += agent_code;
    player->set_role_id(role_id);
    GateRoleDetail &detail = player->detail();
    detail.__scene_id = GATE_MONITOR->scene_convert_to(detail.__scene_id, role_id);

    int ret = this->is_can_create_role(player->agent(), player->client_ip(), player->client_mac());
    if(ret != true)
    {
    	return ERROR_ROLE_TOO_MORE;
    }

    if (detail.__name.empty() == true)
    {
    	return ERROR_INVALID_PARAM;
    }

    {
		BSONObj res = this->conection().findOne(Role::COLLECTION, QUERY(Role::ACCOUNT
				<< player->account() << Role::IS_ACTIVE << 1));
		if (res.isEmpty() == false)
			return ERROR_ROLE_EXISTS;
    }
    {
    	BSONObj res = this->conection().findOne(Role::COLLECTION, QUERY(Role::NAME
    			<< detail.__name << Role::IS_ACTIVE << 1));
		if (res.isEmpty() == false)
			return ERROR_ROLE_EXISTS;
    }
    
    BSONObjBuilder builder;
    builder << Role::ACCOUNT << player->account()
            << Role::SERVER_FLAG << player->detail().__server_flag
    		<< Role::AGENT << player->agent()
            << Role::AGENT_CODE << player->agent_code()
            << Role::MARKET_CODE << player->market_code()
            << Role::IS_NEW << 1
            << Role::IS_FIRST_RENAME << 1
    		<< Role::ID << role_id
            << Role::IS_ACTIVE << 1
    		<< Role::NAME << detail.__name
    		<< Role::SRC_NAME << detail.__src_name
    		<< Role::LEVEL << detail.__level
    		<< Role::SEX << detail.__sex
    		<< Role::CAREER << detail.__career
            << Role::CREATE_TIME << Int64(::time(0))
            << Role::IP << player->client_ip()
            << Role::PERMISSION << player->detail().__permission
            << Role::LOGIN_COUNT << 0
    		<< Role::LOCATION << BSON(Role::Location::SCENE_ID << detail.__scene_id
                << Role::Location::PIXEL_X << detail.__pixel_x
                << Role::Location::PIXEL_Y << detail.__pixel_y);

    this->conection().update(Role::COLLECTION, QUERY(Role::ID << role_id),
    		BSON("$set" << builder.obj()), true);

    this->save_new_back_role(player);


    return 0;
END_CATCH
    return -1;
}

void MMORole::set_role_permission_info(BaseRoleInfo& role_info)
{
	BSONObj account_res = this->conection().findOne(DBBackAccount::COLLECTION,
			QUERY(DBBackAccount::ACCOUNT << role_info.__account));
	JUDGE_RETURN(account_res.isEmpty() == false, ;);

	role_info.__permission = account_res[DBBackAccount::PERMISSION].numberInt();
}

void MMORole::set_ban_ip_info(GatePlayer *player)
{
	JUDGE_RETURN(player->detail().__permission != GameEnum::PERT_GM, ;);

//	uint ip_value = GameCommon::format_ip_string_to_int(player->client_ip());

	BSONObj ban_ip_res = this->conection().findOne(DBBanIpInfo::COLLECTION,
			QUERY(DBBanIpInfo::IP_STRING << player->client_ip()));
	JUDGE_RETURN(ban_ip_res.isEmpty() == false, ;);

	Int64 expired_time = ban_ip_res[DBBanIpInfo::EXPIRED_TIME].numberLong();
	JUDGE_RETURN(expired_time > ::time(0), ;);

	player->set_ban_state(GameEnum::OPER_BAN_IP, expired_time);
	MSG_USER("ban ip->ban_type:%d,expired_time:%d", player->ban_type(), player->ban_expried_time());
}

void MMORole::set_gm_permission(GatePlayer *player)
{
	BSONObj white_ip_res = this->conection().findOne(DBWhiteIpInfo::COLLECTION,
			QUERY(DBWhiteIpInfo::IP_STRING << player->client_ip()));
	JUDGE_RETURN(white_ip_res.isEmpty() == false, ;);

	player->detail().__permission = GameEnum::PERT_GM;
}

void MMORole::set_mac_ban_info(GatePlayer *player)
{
	string mac_str = player->client_mac();
	JUDGE_RETURN(mac_str.empty() == false, ;);

	JUDGE_RETURN(player->detail().__permission != GameEnum::PERT_GM, ;);
	JUDGE_RETURN(player->ban_type()  == GameEnum::OPER_BAN_NONE
			|| player->ban_expried_time() < ::time(0), ;);

	BSONObj mac_res = this->conection().findOne(DBBanMacInfo::COLLECTION,
			QUERY(DBBanMacInfo::MAC_STRING << mac_str));
	JUDGE_RETURN(mac_res.isEmpty() == false, ;);

	Int64 expired_time = mac_res[DBBanMacInfo::EXPIRED_TIME].numberLong();
	JUDGE_RETURN(expired_time > ::time(0), ;);

	player->set_ban_state(GameEnum::OPER_BAN_MAC, expired_time);
	MSG_USER("ban_mac->ban_type:%d,expired_time:%d", player->ban_type(), player->ban_expried_time());
}

int MMORole::load_player_base(LogicPlayer *player)
{
    BSONObj res = this->conection().findOne(Role::COLLECTION,
            QUERY(Role::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    this->conection().update(Role::COLLECTION, QUERY(Role::ID << player->role_id()),
            BSON("$set" << BSON(Role::LAST_SIGN_IN << (long long int)(nowtime.sec()))
            		<< "$inc" << BSON(Role::LOGIN_COUNT << 1)));

    this->conection().update(DBBackRole::COLLECTION, QUERY(DBBackRole::ROLE_ID << player->role_id()),
    		BSON("$set" << BSON(DBBackRole::LAST_SIGN_IN_TIME << int(nowtime.sec()) << DBBackRole::ONLINE << 1)
    				<< "$inc" << BSON(DBBackRole::LOGIN_COUNT << 1)));

    player->set_speak_state(res[Role::BAN_TYPE].numberInt(),
    	res[Role::BAN_EXPIRED].numberLong());

    LogicRoleDetail &role_detail = player->role_detail();
    MMORole::init_base_role_info(role_detail, res);
    player->set_new_role_flag(res[Role::IS_NEW].numberInt() == 1);

	BSONObjIterator special_iter = res.getObjectField(Role::SPECIAL_BOX_INFO.c_str());
	while (special_iter.more())
	{
		BSONObj obj = special_iter.next().embeddedObject();
	    player->special_box_set_buy_times(obj[Role::SpecialBoxInfo::BUY_TIMES].numberLong());
	    player->special_box_set_score(obj[Role::SpecialBoxInfo::SCORE].numberLong());
	    GameCommon::bson_to_map(player->special_box_get_refresh_times_map(),
	    		obj.getObjectField(Role::SpecialBoxInfo::REFRESH_TIMES.c_str()));
	}


    role_detail.day_reset_tick_ = GameCommon::day_zero(res[Role::DAY_RESET_TICK].numberLong());
    role_detail.week_reset_tick_.sec(res[Role::WEEK_RESET_TICK].numberLong());
    role_detail.view_tick_ = res[Role::VIEW_TICK].numberLong();
    role_detail.draw_tick_ = res[Role::NOTICE_DRAW_TICK].numberLong();
    role_detail.__recharge_total_gold = res[Role::RECHARGE_TOTAL_GOLD].numberInt();
    role_detail.panic_buy_notify_ = res[Role::PANIC_BUY_NOTIFY].numberInt();
    role_detail.kill_num_ = res[Role::KILL_NUM].numberInt();
    role_detail.killing_value_ = res[Role::KILL_VALUE].numberInt();
    role_detail.kill_normal_ = res[Role::KILL_NORMAL].numberInt();
    role_detail.kill_evil_ = res[Role::KILL_EVIL].numberInt();
    role_detail.today_recharge_gold_ = res[Role::TODAY_RECHARGE_GOLD].numberInt();
    role_detail.today_consume_gold_ = res[Role::TODAY_CONSUME_GOLD].numberInt();
    role_detail.today_market_buy_times_ = res[Role::TODAY_BUY_TIMES].numberInt();
    role_detail.today_can_buy_times_ = res[Role::TODAY_CAN_BUY_TIMES].numberInt();
    role_detail.today_total_buy_times_ = res[Role::GASHAPON_BUY_TIMES].numberInt();
    role_detail.continuity_login_day_ = res[Role::CONTINUITY_LOGIN_DAY].numberInt();
    role_detail.continuity_login_flag_ = res[Role::CONTINUITY_LOGIN_FLAG].numberInt();
    role_detail.last_act_end_time_ = res[Role::LAST_ACT_END_TIME].numberInt();
    role_detail.last_act_type_ = res[Role::LAST_ACT_TYPE].numberInt();


	BSONObjIterator iter6(res.getObjectField(Role::BROTHER_REWARD_INDEX.c_str()));
	while(iter6.more())
	{
		role_detail.brother_reward_index.insert(iter6.next().numberInt());
	}

	BSONObjIterator iter7(res.getObjectField(Role::IS_WORSHIP.c_str()));
	while(iter7.more())
	{
		role_detail.is_worship_.insert(iter7.next().numberLong());
	}

    GameCommon::bson_to_map(role_detail.buy_map_, res.getObjectField(
			Role::BUY_MAP.c_str()));
    GameCommon::bson_to_map(role_detail.buy_total_map_, res.getObjectField(
   			Role::BUY_TOTAL_MAP.c_str()));
    DBCommon::bson_to_threeobj_map(role_detail.mount_info_, res.getObjectField(
   			Role::MOUNT_INFO.c_str()));

	this->set_role_permission_info(player->role_detail());
    return 0;
}

int MMORole::update_data(LogicPlayer *player, MongoDataMap *data_map, const int recogn)
{
	LogicRoleDetail& role_detail = player->role_detail();

	int is_online = 1;
	if (recogn == TRANS_LOGOUT_LOGIC_PLAYER)
	{
		is_online = 0;
	}

	BSONVec buy_bson;
	GameCommon::map_to_bson(buy_bson, role_detail.buy_map_);

	BSONVec total_buy_bson;
	GameCommon::map_to_bson(total_buy_bson, role_detail.buy_total_map_);

	BSONVec refresh_vec_bson;
	GameCommon::map_to_bson(refresh_vec_bson, player->special_box_get_refresh_times_map());

	BSONVec special_box_bson;
	special_box_bson.push_back(BSON(Role::SpecialBoxInfo::BUY_TIMES << player->special_box_get_buy_times()
					 << Role::SpecialBoxInfo::SCORE << player->special_box_get_score()
					 << Role::SpecialBoxInfo::REFRESH_TIMES << refresh_vec_bson
					 ));


	BSONVec mount_info_bson;
	DBCommon::threeobj_map_to_bson(mount_info_bson, role_detail.mount_info_);

	{
		BSONObjBuilder builder;
		builder << Role::DAY_RESET_TICK << role_detail.day_reset_tick_
				<< Role::WEEK_RESET_TICK << Int64(role_detail.week_reset_tick_.sec())
				<< Role::VIEW_TICK << role_detail.view_tick_
				<< Role::LAST_SIGN_OUT << role_detail.__last_logout_tick
				<< Role::LOGIN_TICK << role_detail.__login_tick
				<< Role::LOGIN_DAYS << role_detail.__login_days
				<< Role::NOTICE_DRAW_TICK << role_detail.draw_tick_
				<< Role::IS_NEW << 0
				<< Role::BUY_MAP << buy_bson
				<< Role::BUY_TOTAL_MAP << total_buy_bson
				<< Role::MOUNT_INFO << mount_info_bson
				<< Role::PANIC_BUY_NOTIFY << role_detail.panic_buy_notify_
				<< Role::KILL_NORMAL << role_detail.kill_normal_
				<< Role::KILL_EVIL << role_detail.kill_evil_
				<< Role::BROTHER_REWARD_INDEX << role_detail.brother_reward_index
				<< Role::IS_WORSHIP << role_detail.is_worship_
				<< Role::TODAY_RECHARGE_GOLD << role_detail.today_recharge_gold_
				<< Role::TODAY_CONSUME_GOLD << role_detail.today_consume_gold_
				<< Role::TODAY_BUY_TIMES << role_detail.today_market_buy_times_
				<< Role::TODAY_CAN_BUY_TIMES << role_detail.today_can_buy_times_
				<< Role::GASHAPON_BUY_TIMES << role_detail.today_total_buy_times_
				<< Role::CONTINUITY_LOGIN_DAY << role_detail.continuity_login_day_
				<< Role::CONTINUITY_LOGIN_FLAG << role_detail.continuity_login_flag_
				<< Role::LAST_ACT_TYPE << role_detail.last_act_type_
				<< Role::LAST_ACT_END_TIME << role_detail.last_act_end_time_
				<< Role::SPECIAL_BOX_INFO << special_box_bson
				;
		data_map->push_update(Role::COLLECTION, BSON(Role::ID << player->role_id()),
				builder.obj(), false);
	}

	MSG_DEBUG("save act_type:%d, last_act_end:%d", player->role_detail().last_act_type_, player->role_detail().last_act_end_time_);

	if (recogn == TRANS_LOGOUT_LOGIC_PLAYER)
	{
		BSONObjBuilder builder;
		builder << DBBackRole::LAST_SIGN_OUT_TIME << int(role_detail.__last_logout_tick)
				<< DBBackRole::ONLINE << 0;
		data_map->push_update(DBBackRole::COLLECTION, BSON(DBBackRole::ROLE_ID
				<< player->role_id()), builder.obj(), false);
	}

    return 0;
}

int MMORole::load_player_base(MapPlayerEx *player)
{
    BSONObj res = this->conection().findOne(Role::COLLECTION,
            QUERY(Role::ID << player->role_id()));
	JUDGE_RETURN(res.isEmpty() == false, -1);

    MapRoleDetail &role_detail = player->role_detail();
    MoverDetail &move_detail = player->mover_detail();
    KilledInfo &killed_info = player->killed_info();
    MMORole::init_base_role_info(role_detail, res);

    if (role_detail.__league_id > 0)
    {
    	role_detail.__league_name = MMOLeague::fetch_league_name(role_detail.__league_id);
    	role_detail.__league_pos = MMOLeague::fetch_league_pos(role_detail.__league_id, player->role_id());
    }

    killed_info.kill_num_ = res[Role::KILL_NUM].numberInt();
    killed_info.is_brocast_ = res[Role::IS_BROCAST].numberInt();
    killed_info.online_ticks_ = res[Role::ONLINE_TICKS].numberInt();
    killed_info.killing_value_ = res[Role::KILL_VALUE].numberInt();

    role_detail.__wedding_giftbox_tick.sec(res[Role::WEDDING_GIFTBOX_TICK].numberInt());
    role_detail.__wedding_giftbox_times = res[Role::WEDDING_GIFTBOX_TIMES].numberInt();
    role_detail.__refresh_free_relive_tick.sec(res[Role::FRESH_FREE_RELIVE_TICK].numberInt());
    role_detail.__used_free_relive = res[Role::USED_FREE_RELIVE].numberInt();
    role_detail.__collect_chest_amount = res[Role::COLLECT_CHEST_AMOUNT].numberInt();
    role_detail.escort_times = res[Role::ESCORT_TIMES].numberInt();
    role_detail.protect_times = res[Role::PROTECT_TIMES].numberInt();
    role_detail.rob_times = res[Role::ROB_TIMES].numberInt();
    role_detail.save_trvl_scene_ = res[Role::SAVE_TRVL_SCENE].numberInt();
    role_detail.prev_force_ = role_detail.__fight_force;
    role_detail.drop_act_ = MAP_MONITOR->festival_icon_type();
    role_detail.is_big_act_time_ = MAP_MONITOR->is_in_big_act_time();

    GameCommon::bson_to_map(role_detail.prev_force_map_,
    		res.getObjectField(Role::PREV_FORCE_MAP.c_str()), true);
	this->set_role_permission_info(role_detail);

    //load shape
	if(res.hasField(Role::SHAPE_DETAIL.c_str()))
	{
		IntVec value_set;
		BSONObj shape = res.getObjectField(Role::SHAPE_DETAIL.c_str());
		BSONObjIterator it(shape);
		while(it.more())
		{
			value_set.push_back(it.next().numberInt());
		}

		if (value_set.empty() == false)
		{
			ShapeDetail& detail = player->shape_detail();

			for(int i = 0; i < (int)value_set.size() && i + 1 < int(value_set.size()); i += 2)
			{
				int equip_id = value_set[i + 1];
				JUDGE_CONTINUE(CONFIG_INSTANCE->item(equip_id) != Json::Value::null);

				int index = value_set[i];
				detail[index] = equip_id;
			}
		}
	}


    //load cur_label
	if (res.hasField(Role::LABEL_INFO.c_str()))
	{
		BSONObj obj = res.getObjectField(Role::LABEL_INFO.c_str());
		int label_id = obj[LabelInfo::LABEL_ID].numberInt();
		Int64 expire_tick = obj[LabelInfo::EXPIRE_TICK].numberLong();

		std::map<int,Int64>& label_info = player->label_info();
		if(expire_tick != -1)
		{
			if(GameCommon::left_time(expire_tick) <= 0)
				label_id = 0;
		}
		label_info.insert(std::pair<int,Int64>(label_id, expire_tick));
	}

    {
    	// save info
    	BSONObj save_obj = res.getObjectField(Role::SAVE_INFO.c_str());
    	role_detail.__save_scene.scene_id_ = save_obj[Role::SaveInfo::SCENE_ID].numberInt();
    	role_detail.__save_scene.blood_ = save_obj[Role::SaveInfo::BLOOD].numberInt();
    	role_detail.__save_scene.magic_ = save_obj[Role::SaveInfo::MAGIC].numberInt();
    	role_detail.__save_scene.pk_state_ = save_obj[Role::SaveInfo::PK_STATE].numberInt();
    	role_detail.__save_scene.coord_.set_pixel(save_obj[Role::Location::PIXEL_X].numberInt(),
    			save_obj[Role::Location::PIXEL_Y].numberInt());
    }

    {
    	// cur location
        BSONObj move_obj = res.getObjectField(Role::LOCATION.c_str());
        move_detail.__scene_id = move_obj[Role::Location::SCENE_ID].numberInt();
        int pixel_x = move_obj[Role::Location::PIXEL_X].numberInt(),
            pixel_y = move_obj[Role::Location::PIXEL_Y].numberInt();

        move_detail.__location.set_pixel(pixel_x, pixel_y);
        move_detail.__toward = move_obj[Role::Location::TOWARD].numberInt();
        move_detail.__scene_mode = move_obj[Role::Location::MODE].numberInt();
        move_detail.__space_id = move_obj[Role::Location::SPACE_ID].numberInt();

        move_detail.__temp_location.set_pixel(move_obj[Role::Location::TEMP_PIXEL_X].numberInt(),
        		move_obj[Role::Location::TEMP_PIXEL_Y].numberInt());

        // prev location
        move_detail.__prev_scene_id = move_obj[Role::Location::PREV_SCENE_ID].numberInt();
        move_detail.__prev_location.set_pixel(move_obj[Role::Location::PREV_PIXEL_X].numberInt(),
        		move_obj[Role::Location::PREV_PIXEL_Y].numberInt());
        move_detail.__prev_scene_mode = move_obj[Role::Location::PREV_MODE].numberInt();
        move_detail.__prev_space_id = move_obj[Role::Location::PREV_SPACE_ID].numberInt();

        BSONObjIterator his_iter(move_obj.getObjectField(Role::Location::SCENE_HISTORY.c_str()));
        while (his_iter.more())
        {
            role_detail.__scene_history.insert(his_iter.next().numberInt());
        }
    }

    return 0;
}

int MMORole::update_data(MapPlayerEx *player, MongoDataMap *data_map)
{
    MapRoleDetail &role_detail = player->role_detail();
    MoverDetail &move_detail = player->mover_detail();
    ShapeDetail &shape_detail = player->shape_detail();
    KilledInfo &killed_info = player->killed_info();

    //shape
    IntVec shape_set;
    shape_set.reserve(shape_detail.size() * 2);
    for(ShapeDetail::iterator it = shape_detail.begin(); it != shape_detail.end(); ++it)
    {
    	shape_set.push_back(it->first);
    	shape_set.push_back(it->second);
    }

    //scene history
    IntVec scene_his_vc;
    for (IntSet::iterator iter = role_detail.__scene_history.begin();
            iter != role_detail.__scene_history.end(); ++iter)
    {
        scene_his_vc.push_back(*iter);
    }

    BSONVec prev_force_vc;
    GameCommon::map_to_bson(prev_force_vc,
    		role_detail.prev_force_map_, true, true);

    int cur_scene_id = move_detail.__scene_id;
    int cur_space_id = move_detail.__space_id;
    int cur_scene_mode = move_detail.__scene_mode;
    MoverCoord location = move_detail.__location;

    int save_trvl_scene = CONFIG_INSTANCE->scene_set(cur_scene_id)["save_trvl_scene"].asInt();
    if (save_trvl_scene == true)
    {
        role_detail.save_trvl_scene_ = cur_scene_id;
    }

    int flag = CONFIG_INSTANCE->scene_set(cur_scene_id)["quit_to_save"].asInt();
    if (flag == 1 || cur_scene_mode == SCENE_MODE_SCRIPT)
    {
    	cur_scene_id = role_detail.__save_scene.scene_id_;
    	cur_space_id = role_detail.__save_scene.space_id_;
    	cur_scene_mode = role_detail.__save_scene.scene_mode_;
    	location = role_detail.__save_scene.coord_;
    }

    BSONObjBuilder builder;
    builder << Role::ACCOUNT << role_detail.__account
        << Role::NAME << role_detail.__name
        << Role::IS_ACTIVE << 1
        << Role::SERVER_TICK << role_detail.server_tick_
        << Role::COMBINE_TICK << role_detail.combine_tick_
        << Role::LEVEL << player->level()
        << Role::IS_YELLOW << killed_info.is_yellow_
        << Role::KILL_NUM << killed_info.kill_num_
        << Role::IS_BROCAST << killed_info.is_brocast_
        << Role::ONLINE_TICKS << killed_info.online_ticks_
        << Role::KILL_VALUE << killed_info.killing_value_
        << Role::EXP << player->fight_detail().__experience
        << Role::SEX << role_detail.__sex
        << Role::CAREER << role_detail.__career
        << Role::VIP_TYPE << player->vip_type()
        << Role::FORCE << role_detail.__fight_force
        << Role::LEAGUE_NAME << role_detail.__league_name
        << Role::PARTNER_ID << role_detail.__partner_id
        << Role::PARTNER_NAME << role_detail.__partner_name
        << Role::WEDDING_ID << role_detail.__wedding_id
        << Role::WEDDING_TYPE << role_detail.__wedding_type
        << Role::SAVE_TRVL_SCENE << role_detail.save_trvl_scene_
        << Role::SHAPE_DETAIL << shape_set
        << Role::WEDDING_GIFTBOX_TICK << int(role_detail.__wedding_giftbox_tick.sec())
        << Role::WEDDING_GIFTBOX_TIMES << role_detail.__wedding_giftbox_times
        << Role::FRESH_FREE_RELIVE_TICK << int(role_detail.__refresh_free_relive_tick.sec())
        << Role::USED_FREE_RELIVE << role_detail.__used_free_relive
        << Role::COLLECT_CHEST_AMOUNT << role_detail.__collect_chest_amount
		<< Role::ESCORT_TIMES << role_detail.escort_times
		<< Role::PROTECT_TIMES << role_detail.protect_times
		<< Role::ROB_TIMES << role_detail.rob_times
		<< Role::PREV_FORCE_MAP << prev_force_vc
		<< Role::SAVE_INFO << BSON(Role::SaveInfo::SCENE_ID << role_detail.__save_scene.scene_id_
				<< Role::Location::PIXEL_X << role_detail.__save_scene.coord_.pixel_x()
				<< Role::Location::PIXEL_Y << role_detail.__save_scene.coord_.pixel_y()
				<< Role::SaveInfo::PK_STATE << role_detail.__save_scene.pk_state_
				<< Role::SaveInfo::BLOOD << role_detail.__save_scene.blood_
				<< Role::SaveInfo::MAGIC << role_detail.__save_scene.magic_)
        << Role::LOCATION << BSON(Role::Location::SCENE_ID << cur_scene_id
                << Role::Location::PIXEL_X << location.pixel_x()
                << Role::Location::PIXEL_Y << location.pixel_y()
                << Role::Location::TOWARD << move_detail.__toward
                << Role::Location::MODE << cur_scene_mode
                << Role::Location::SPACE_ID << cur_space_id
                << Role::Location::TEMP_PIXEL_X << move_detail.__temp_location.pixel_x()
                << Role::Location::TEMP_PIXEL_Y << move_detail.__temp_location.pixel_y()
                << Role::Location::PREV_SCENE_ID << move_detail.__prev_scene_id
                << Role::Location::PREV_PIXEL_X << move_detail.__prev_location.pixel_x()
                << Role::Location::PREV_PIXEL_Y << move_detail.__prev_location.pixel_y()
                << Role::Location::PREV_MODE << move_detail.__prev_scene_mode
                << Role::Location::PREV_SPACE_ID << move_detail.__prev_space_id
                << Role::Location::SCENE_HISTORY << scene_his_vc);

    data_map->push_update(Role::COLLECTION,
            BSON(Role::ID << player->role_id()), builder.obj(), false);

    return 0;
}

int MMORole::update_back_role(MapPlayerEx *player, MongoDataMap *data_map)
{
    MapRoleDetail &role_detail = player->role_detail();
    FightDetail &fight_detail = player->fight_detail();
    KilledInfo &killed_info = player->killed_info();

    BSONObjBuilder builder;
    builder << DBBackRole::ACCOUNT << role_detail.__account
    	<< DBBackRole::ROLE_NAME << role_detail.__name
        << DBBackRole::CAREER << role_detail.__career
        << DBBackRole::PERMISSION << role_detail.__permission
        << DBBackRole::SEX << role_detail.__sex
        << DBBackRole::LEVEL << fight_detail.__level
        << DBBackRole::FIGHT_FORCE << role_detail.__fight_force
        << DBBackRole::SCENE_ID << player->scene_id()
        << DBBackRole::COORD_X << player->location().pixel_x()
        << DBBackRole::COORD_Y << player->location().pixel_y()
        << DBBackRole::EXPERIENCE << fight_detail.__experience
        << DBBackRole::LEAGUE_ID << player->league_id()
        << DBBackRole::KILL_NUM << killed_info.kill_num_
        << DBBackRole::KILL_VALUE << killed_info.killing_value_
        << DBBackRole::FIGHTER_PROP << DBCommon::fighter_property_to_bson(player);

    data_map->push_update(DBBackRole::COLLECTION, 
            BSON(DBBackRole::ROLE_ID << player->role_id()), builder.obj(), true);

    return 0;
}

int MMORole::load_player_wedding_info_by_wedding_id(Int64 wedding_id, Int64 role_id, IntMap& self_map, IntMap& side_map)
{
    BSONObj res = this->conection().findOne(DBGWedding::COLLECTION, QUERY(DBGWedding::ID << wedding_id));
    if (res.isEmpty() == true)
        return 0;

    int type = 0, level = 0, side_level = 0;

        if (role_id == res[DBGWedding::PARTNER_ONE].numberLong())
        {
        	type = WED_RING;
        	level = res[DBGWedding::RING_LEVEL_1].numberInt();
        	side_level = res[DBGWedding::RING_LEVEL_2].numberInt();
        	self_map[type] = level; side_map[type] = side_level;

        	type = WED_SYS;
        	level = res[DBGWedding::SYS_LEVEL_1].numberInt();
        	side_level = res[DBGWedding::SYS_LEVEL_2].numberInt();
        	self_map[type] = level; side_map[type] = side_level;

        	type = WED_TREE;
        	level = res[DBGWedding::TREE_LEVEL_1].numberInt();
        	side_level = res[DBGWedding::TREE_LEVEL_2].numberInt();
        	self_map[type] = level; side_map[type] = side_level;
        }
        else if (role_id == res[DBGWedding::PARTNER_TWO].numberLong())
        {
        	type = WED_RING;
        	level = res[DBGWedding::RING_LEVEL_2].numberInt();
        	side_level = res[DBGWedding::RING_LEVEL_1].numberInt();
        	self_map[type] = level; side_map[type] = side_level;

        	type = WED_SYS;
        	level = res[DBGWedding::SYS_LEVEL_2].numberInt();
        	side_level = res[DBGWedding::SYS_LEVEL_1].numberInt();
        	self_map[type] = level; side_map[type] = side_level;

        	type = WED_TREE;
        	level = res[DBGWedding::TREE_LEVEL_2].numberInt();
        	side_level = res[DBGWedding::TREE_LEVEL_1].numberInt();
        	self_map[type] = level; side_map[type] = side_level;
        }

	return 0;
}

int MMORole::load_player_wedding_info_by_role_id(Int64 role_id, IntMap& self_map, IntMap& side_map)
{
    BSONObj res = this->conection().findOne(DBPlayerWedding::COLLECTION, QUERY(DBPlayerWedding::ID << role_id));
    if (res.isEmpty() == true)
        return 0;

    BSONObjIterator iter(res.getObjectField(DBPlayerWedding::WEDDING_PROPERTY.c_str()));
    int type =	WED_RING;
    while (iter.more())
    {
    	BSONObj obj = iter.next().embeddedObject();

    	self_map[type] = obj[DBPlayerWedding::Wedding_detail::LEVEL].numberInt();
    	side_map[type] = obj[DBPlayerWedding::Wedding_detail::SIDE_LEVEL].numberInt();
    	type++;
    }

	return 0;
}


int MMORole::load_player_base(MapLogicPlayer *player)
{
    BSONObj res = this->conection().findOne(Role::COLLECTION,
            QUERY(Role::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, -1);

    MapLogicRoleDetail &role_detail = player->role_detail();
    MMORole::init_base_role_info(role_detail, res);

    role_detail.set_create_tick(res[Role::CREATE_TIME].numberLong());
//    role_detail.__is_first_rename = res[Role::IS_FIRST_RENAME].numberInt();
    role_detail.__ml_day_reset_tick = res[Role::ML_DAY_RESET_TICK].numberLong();
	role_detail.__league_name = MMOLeague::fetch_league_name(role_detail.__league_id);
    role_detail.change_name_tick_ = res[Role::CHANGE_NAME_TICK].numberLong();
    role_detail.change_sex_tick_ = res[Role::CHANGE_SEX_TICK].numberLong();
    role_detail.__open_gift_close = res[Role::OPEN_GIFT_CLOSE].numberInt();

    if (res.hasField(Role::WEDDING_SELF.c_str()))
    {
        GameCommon::bson_to_map(role_detail.wedding_self_, res.getObjectField(Role::WEDDING_SELF.c_str()));
        GameCommon::bson_to_map(role_detail.wedding_side_, res.getObjectField(Role::WEDDING_SIDE.c_str()));
    }
    else
    {
    	if (role_detail.__wedding_id > 0)
    	{
    		this->load_player_wedding_info_by_wedding_id(role_detail.__wedding_id, role_detail.__id,
    			role_detail.wedding_self_, role_detail.wedding_side_);
    	}
    	else
    	{
    		this->load_player_wedding_info_by_role_id(role_detail.__id,
    			role_detail.wedding_self_, role_detail.wedding_side_);
    	}
    }

	GameCommon::bson_to_map(role_detail.draw_days_,
			res.getObjectField(Role::DRAW_DAY.c_str()));
	GameCommon::bson_to_map(role_detail.draw_gift_,
			res.getObjectField(Role::DRAW_GIFT.c_str()));
	GameCommon::bson_to_map(role_detail.draw_vips_,
			res.getObjectField(Role::DRAW_VIP.c_str()));
	GameCommon::bson_to_map(role_detail.rand_use_times_,
			res.getObjectField(Role::RAND_USE_TIMES.c_str()));


	this->set_role_permission_info(role_detail);

    return 0;

}

int MMORole::update_data(MapLogicPlayer* player, MongoDataMap* data_map)
{
	MapLogicRoleDetail &role_detail = player->role_detail();

	BSONVec draw_bson_vec;
	GameCommon::map_to_bson(draw_bson_vec, role_detail.draw_days_);
	BSONVec draw_gift_vec;
	GameCommon::map_to_bson(draw_gift_vec, role_detail.draw_gift_);
	BSONVec draw_vips_vec;
	GameCommon::map_to_bson(draw_vips_vec, role_detail.draw_vips_);
	BSONVec rand_use_times_vec;
	GameCommon::map_to_bson(rand_use_times_vec, role_detail.rand_use_times_);

	int label_id = player->cur_label_id(),expire_tick = 0;
	{
		expire_tick = player->limit_time_label_list().count(label_id) > 0
						? player->limit_time_label_list()[label_id] : -1;
	}

	BSONVec wedding_self, wedding_side;
	GameCommon::map_to_bson(wedding_self, role_detail.wedding_self_, false, true);
	GameCommon::map_to_bson(wedding_side, role_detail.wedding_side_, false, true);

    BSONObjBuilder builder;
    builder << Role::CHANGE_NAME_TICK << role_detail.change_name_tick_
    		<< Role::CHANGE_SEX_TICK << role_detail.change_sex_tick_
    		<< Role::OPEN_GIFT_CLOSE << role_detail.__open_gift_close
    		<< Role::DRAW_DAY << draw_bson_vec
    		<< Role::DRAW_GIFT << draw_gift_vec
    		<< Role::DRAW_VIP << draw_vips_vec
    		<< Role::RAND_USE_TIMES << rand_use_times_vec
    		<< Role::RECHARGE_TOTAL_GOLD << player->total_recharge_gold()
    		<< Role::ML_DAY_RESET_TICK << role_detail.__ml_day_reset_tick
    		<< Role::WEDDING_SELF << wedding_self
    		<< Role::WEDDING_SIDE << wedding_side
	        << Role::LABEL_INFO << BSON(LabelInfo::LABEL_ID << label_id
	        					<< LabelInfo::EXPIRE_TICK << expire_tick);

    data_map->push_update(Role::COLLECTION,
            BSON(Role::ID << player->role_id()), builder.obj(), false);
	return 0;
}

int MMORole::update_back_role(MapLogicPlayer *player, MongoDataMap *data_map)
{
    PackageDetail &pack_detail = player->pack_detail();
    MLVipDetail &vip_detail = player->vip_detail();

	BSONObjBuilder builder;
    builder << DBBackRole::BIND_COPPER << pack_detail.__money.__bind_copper
    	<< DBBackRole::BIND_GOLD << pack_detail.__money.__bind_gold
    	<< DBBackRole::COPPER << pack_detail.__money.__copper
        << DBBackRole::GOLD << pack_detail.__money.__gold
        << DBBackRole::GOLD_USE << pack_detail.use_resource_map_[GameEnum::ITEM_MONEY_UNBIND_GOLD]
        << DBBackRole::COUPON_USE << pack_detail.use_resource_map_[GameEnum::ITEM_MONEY_BIND_GOLD]
        << DBBackRole::ON_HOOK << player->hook_detail().__is_hooking
        << DBBackRole::RECHARGE_FIRST << player->recharge_detail().__first_recharge_time
        << DBBackRole::RECHARGE_GOLD << player->total_recharge_gold()
        << DBBackRole::VIP << vip_detail.__vip_type
        << DBBackRole::VIP_DEADLINE << vip_detail.__expired_time
        << DBBackRole::VIP_START_TIME << vip_detail.__start_time;

    data_map->push_update(DBBackRole::COLLECTION,
            BSON(DBBackRole::ROLE_ID << player->role_id()), builder.obj());
    return 0;
}

int MMORole::load_offline_friend_info(DBFriendInfo *friend_detail)
{
	LongSet::iterator it = friend_detail->__offine_set.begin();
	int64_t role_id = 0;
	FriendInfo info;
	for(; it != friend_detail->__offine_set.end(); ++it)
	{
		role_id = *it;
		info.reset();
		info.__friend_type = friend_detail->__friend_type;
		if(this->load_offline_player_info(role_id, info) == 0)
			friend_detail->__friend_info_vec.push_back(info);
	}
	return 0;
}

int MMORole::load_offline_player_info(Int64 role_id, FriendInfo& info)
{
	BSONObj res = this->conection().findOne(Role::COLLECTION,
			QUERY(Role::ID << role_id));
	JUDGE_RETURN(res.isEmpty() == false, -1);

	info.__role_id = res[Role::ID].numberLong();
	info.__icon_id = res[Role::ID].numberLong();
	info.__league_id = res[Role::LEAGUE_ID].numberLong();
	info.__vip_status = res[Role::VIP_TYPE].numberInt();
	info.__sex = res[Role::SEX].numberInt();
	info.__career = res[Role::CAREER].numberInt();
	info.__level = res[Role::LEVEL].numberInt();
	info.__name = res[Role::NAME].str();
	info.__name[res[Role::NAME].str().length()] = '\0';
	info.__team_status = 0;
	info.__force = res[Role::FORCE].numberInt();

	int killing_value = res[Role::KILL_VALUE].numberInt();
	int kill_num = res[Role::KILL_NUM].numberInt();
	info.__name_color = GameCommon::fetch_name_color(kill_num,killing_value);

	return 0;
}

int MMORole::load_player_to_send_mail(MailInformation *mail_info)
{
	std::string &receiver_name = mail_info->receiver_name_;
	mail_info->receiver_id_ = this->search_player_name(receiver_name);
	return 0;
}

int MMORole::init_introduction_info()
{
//	const Json::Value &rpm_json = CONFIG_INSTANCE->replacement_cont("replacement");
//	MMORole::init_introduction_info(rpm_json);

//	const Json::Value &arena_json = CONFIG_INSTANCE->replacement_cont("arena");
//	MMORole::init_introduction_info(arena_json);

	const Json::Value &rank_json = CONFIG_INSTANCE->athletics_rank();
	MMORole::init_introduction_info(rank_json);

	MMOLeague::load_arena_guide(ARENA_SYS->fetch_detail());
	return 0;
}

int MMORole::init_introduction_info(const Json::Value &rpm_json)
{
//	JUDGE_RETURN(rpm_json.size() > 0, -1);

	Json::Value::iterator it = rpm_json.begin();
	for(; it != rpm_json.end(); ++it)
	{
		const Json::Value &rpm_role_json = *it;
		for (int id = 0 ; id < GameEnum::ARENA_BASE_RANK_NUM; id++)
		{
			int rpm_id = rpm_role_json["rpm_id"].asInt() + id * GameEnum::ARENA_BASE_ID_NUM;

			BSONObj query_obj = CACHED_CONNECTION.findOne(DBRpmFakeInfo::COLLECTION,
					QUERY(DBRpmFakeInfo::CONFIG_ID << rpm_id));
			int wing_level = 0, solider_level = 0, magic_level = 0, beast_level = 0,
				mount_level = 0;

			wing_level = GameCommon::rand_mount_level("wind_range", rpm_role_json);
			solider_level = GameCommon::rand_mount_level("solider_range", rpm_role_json);
			magic_level = GameCommon::rand_mount_level("magic_range", rpm_role_json);
			beast_level = GameCommon::rand_mount_level("beast_range", rpm_role_json);
			mount_level = GameCommon::rand_mount_level("mount_range", rpm_role_json);

			BSONObjBuilder builder;
			builder << DBRpmFakeInfo::NAME << rpm_role_json["name"].asString()
					<< DBRpmFakeInfo::FORCE << rpm_role_json["force"].asInt()
					<< DBRpmFakeInfo::LEVEL << rpm_role_json["level"].asInt()
					<< DBRpmFakeInfo::SEX << rpm_role_json["sex"].asInt()
					<< DBRpmFakeInfo::CAREER << rpm_role_json["career"].asInt()
					<< DBRpmFakeInfo::VIP_TYPE << rpm_role_json["vip_type"].asInt()
					<< DBRpmFakeInfo::WING_LEVEL << wing_level
					<< DBRpmFakeInfo::SOLIDER_LEVEL << solider_level
					<< DBRpmFakeInfo::CONFIG_ID << rpm_id;

			Int64 fake_role_id = 0;
			if(query_obj.isEmpty())
			{
				fake_role_id = CACHED_INSTANCE->update_global_key(Global::ROLE);
				builder << DBRpmFakeInfo::ID << fake_role_id;
			}
			else
			{
				fake_role_id = query_obj[DBRpmFakeInfo::ID].numberLong();
			}

			CACHED_CONNECTION.update(DBRpmFakeInfo::COLLECTION,
					QUERY(DBRpmFakeInfo::CONFIG_ID << rpm_id), BSON("$set" << builder.obj()), true);


			BSONVec bson_skill_set;
			uint skill_set_size = rpm_role_json["skill_set"].size();
			for(uint i = 0; i< skill_set_size; ++i)
			{
				int key = rpm_role_json["skill_set"][i][0u].asInt();
				int value = rpm_role_json["skill_set"][i][1u].asInt();
				bson_skill_set.push_back(BSON(Skill::SSkill::SKILL_ID << key << Skill::SSkill::LEVEL << value));
			}

			IntMap shape_map;
			uint shape_set_size = rpm_role_json["shape_set"].size();
			for(uint i = 0; i< shape_set_size; ++i)
			{
				int key = rpm_role_json["shape_set"][i][0u].asInt();
				int value = rpm_role_json["shape_set"][i][1u].asInt();
				shape_map[key] = value;
			}

			BSONVec bson_shape_set;
			GameCommon::map_to_bson(bson_shape_set, shape_map);

			BSONObjBuilder cp_builder;
			cp_builder << DBCopyPlayer::NAME << rpm_role_json["name"].asString()
				<< DBCopyPlayer::LEVEL << rpm_role_json["level"].asInt()
				<< DBCopyPlayer::SEX << rpm_role_json["sex"].asInt()
				<< DBCopyPlayer::VIP_TYPE << rpm_role_json["vip_type"].asInt()
				<< DBCopyPlayer::FORCE << rpm_role_json["force"].asInt()
				<< DBCopyPlayer::ATTACK_LOWER << rpm_role_json["attack"].asInt()
				<< DBCopyPlayer::ATTACK_UPPER << rpm_role_json["attack"].asInt()
				<< DBCopyPlayer::DEFENCE_LOWER << rpm_role_json["defence"].asInt()
				<< DBCopyPlayer::DEFENCE_UPPER << rpm_role_json["defence"].asInt()
				<< DBCopyPlayer::HIT << rpm_role_json["hit"].asInt()
				<< DBCopyPlayer::DODGE << rpm_role_json["dodge"].asInt()
				<< DBCopyPlayer::CRIT << rpm_role_json["crit"].asInt()
				<< DBCopyPlayer::TOUGHNESS << rpm_role_json["toughness"].asInt()
				<< DBCopyPlayer::BLOOD << rpm_role_json["blood"].asInt()
				<< DBCopyPlayer::MAGIC << rpm_role_json["magic"].asInt()
				<< DBCopyPlayer::SPEED << rpm_role_json["speed"].asInt()
				<< DBCopyPlayer::DAMAGE << rpm_role_json["damage"].asInt()
				<< DBCopyPlayer::REDUCTION << rpm_role_json["reduction"].asInt()
				<< DBCopyPlayer::WING_LEVEL << wing_level
				<< DBCopyPlayer::SOLIDER_LEVEL << solider_level
				<< DBCopyPlayer::MAGIC_LEVEL << magic_level
				<< DBCopyPlayer::BEAST_LEVEL << beast_level
				<< DBCopyPlayer::MOUNT_LEVEL << mount_level
				<< DBCopyPlayer::SKILL_SET << bson_skill_set
				<< DBCopyPlayer::SHAPE_SET << bson_shape_set;

			CACHED_CONNECTION.update(DBCopyPlayer::COLLECTION, QUERY(DBCopyPlayer::ID
					<< fake_role_id), BSON("$set" << cp_builder.obj()), true);
		}
	}
	return 0;
}

int MMORole::get_rpm_introduction_info(RpmRecomandInfo* rpm_recomand_info)
{
	JUDGE_RETURN(NULL != rpm_recomand_info, -1);
	int force_range = CONFIG_INSTANCE->tiny("recomand_force_range").asInt();

	int force_low = rpm_recomand_info->__leader_force - force_range;
	int force_high = rpm_recomand_info->__leader_force + force_range;
	//int mini_count = CONFIG_INSTANCE->tiny("recomand_mini_count").asInt();

	BSONObj fields_test = BSON(Role::ID << 1);
	for( int i = 0; i < 1000; ++i )
	{
		auto_ptr<DBClientCursor> cursor_test = this->conection().query(
				DBRpmFakeInfo::COLLECTION, QUERY(DBRpmFakeInfo::FORCE <<
				BSON("$gte" << force_low << "$lte" << force_high)),
				RPM_QUERY_LIST_LENGTH, 0, &fields_test);

		int count = 0;
		while (cursor_test->more())
		{
			BSONObj res = cursor_test->next();
			int64_t role_id = res[DBRpmFakeInfo::ID].numberLong();
			//排除队伍中的玩家
			JUDGE_CONTINUE(rpm_recomand_info->__teamates_id.count(role_id) <= 0);
			++count;
		}

		JUDGE_BREAK(2 > count);
		force_low = MIN(force_low, force_low - force_low / 10);
		force_high = MAX(force_high, force_high + force_high / 10);
	}

	BSONObj fields = BSON(DBRpmFakeInfo::ID << 1 << DBRpmFakeInfo::FORCE << 1
			<< DBRpmFakeInfo::LEVEL << 1 << DBRpmFakeInfo::VIP_TYPE << 1
			<< DBRpmFakeInfo::SEX << 1 << DBRpmFakeInfo::CAREER << 1 << DBRpmFakeInfo::NAME << 1);

	auto_ptr<DBClientCursor> cursor = this->conection().query(
			DBRpmFakeInfo::COLLECTION, QUERY(DBRpmFakeInfo::FORCE <<
			BSON("$gte" << force_low << "$lte" << force_high)).sort(BSON(DBRpmFakeInfo::FORCE << -1)),
			RPM_QUERY_LIST_LENGTH, 0, &fields);

	int n = 0;
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		int64_t role_id = res[Role::ID].numberLong();
		//排除队伍中的玩家
		ReplacementRoleInfo& info = rpm_recomand_info->__role_info_list[n];
		JUDGE_CONTINUE(rpm_recomand_info->__teamates_id.count(role_id) <= 0);
		GameCommon::fill_rpm_role_info(info, res);

		++n;
		JUDGE_BREAK(n < RPM_LIST_LENGTH);
	}

	rpm_recomand_info->__info_count = n;
	return 0;

}

int MMORole::get_rpm_recomand_info(RpmRecomandInfo* rpm_recomand_info)
{
	JUDGE_RETURN(NULL != rpm_recomand_info, -1);
	int force_range = CONFIG_INSTANCE->tiny("recomand_force_range").asInt();
	//int force_extand = CONFIG_INSTANCE->tiny("recomand_force_extand").asInt();

	int force_low = rpm_recomand_info->__leader_force - force_range;
	int force_high = rpm_recomand_info->__leader_force + force_range;
	int mini_count = CONFIG_INSTANCE->tiny("recomand_mini_count").asInt();

BEGIN_CATCH

	BSONObj fields_test = BSON(Role::ID << 1);
	for( int i = 0; i < 10000; ++i )
	{
		auto_ptr<DBClientCursor> cursor_test = this->conection().query(
				Role::COLLECTION, QUERY(Role::FORCE <<
				BSON("$gte" << force_low << "$lte" << force_high)),
				RPM_QUERY_LIST_LENGTH, 0, &fields_test);

		int count = 0;
		while (cursor_test->more())
		{
			BSONObj res = cursor_test->next();
			int64_t role_id = res[Role::ID].numberLong();
			//排除队伍中的玩家
			JUDGE_CONTINUE(rpm_recomand_info->__teamates_id.count(role_id) <= 0);
			++count;
		}

		JUDGE_BREAK(mini_count > count);
//		force_low -= force_extand;
//		force_high += force_extand;
		force_low = MIN(force_low, force_low - force_low / 10);
		force_high = MAX(force_high, force_high + force_high / 10);
	}

	BSONObj fields = BSON(Role::ID << 1 << Role::FORCE << 1
			<< Role::LEVEL << 1 << Role::VIP_TYPE << 1
			<< Role::SEX << 1 << Role::CAREER << 1 << Role::NAME << 1);

	auto_ptr<DBClientCursor> cursor = this->conection().query(
			Role::COLLECTION, QUERY(Role::FORCE <<
			BSON("$gte" << force_low << "$lte" << force_high)).sort(BSON(DBRpmFakeInfo::FORCE << -1)),
			RPM_QUERY_LIST_LENGTH, 0, &fields);

	int n = 0;
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		int64_t role_id = res[Role::ID].numberLong();
		//排除队伍中的玩家
		ReplacementRoleInfo& info = rpm_recomand_info->__role_info_list[n];
		JUDGE_CONTINUE(rpm_recomand_info->__teamates_id.count(role_id) <= 0);
		GameCommon::fill_rpm_role_info(info, res);

		++n;
		JUDGE_BREAK(n < RPM_LIST_LENGTH);
	}

	rpm_recomand_info->__info_count = n;
	return 0;

END_CATCH
	return -1;
}

Int64 MMORole::search_player_name(const string& role_name)
{
    BSONObj fields = BSON(Role::ID << 1);
    BSONObj res = this->conection().findOne(Role::COLLECTION, QUERY(Role::NAME << role_name), &fields);

    if (false == res.isEmpty())
    {
        return res[Role::ID].numberLong();
    }
    else
    {
        MSG_USER("target player not exist...\n");
        return ERROR_ROLE_NOT_EXISTS;
    }
}

string MMORole::search_player_account(const string& role_name)
{
    BSONObj fields = BSON(Role::ACCOUNT << 1);
    BSONObj res = this->conection().findOne(Role::COLLECTION, QUERY(Role::NAME << role_name), &fields);

    string account;
    if (false == res.isEmpty())
    {
        account = res[Role::ACCOUNT].str();
    }

    return account;
}

//修正数据
int MMORole::check_and_modify(MapPlayerEx *player)
{
	MapRoleDetail &role_detail = player->role_detail();

	if (role_detail.__league_id == 0 && role_detail.__league_name.empty() == false)
	{
		role_detail.__league_name.clear();
	}

	if (role_detail.__league_id != 0 && role_detail.__league_name.empty() == true)
	{
		role_detail.__league_name = this->fetch_league_name(role_detail.__league_id);
	}

	if (role_detail.__wedding_id <= 0)
	{
		role_detail.__partner_id = 0;
		role_detail.__partner_name.clear();
		role_detail.__wedding_type = 0;
	}
	return 0;
}

int MMORole::check_role(Int64 role_id)
{
	BSONObj fields = BSON(Role::ID << 1);
	BSONObj res = this->conection().findOne(Role::COLLECTION,
			QUERY(Role::ID << role_id), &fields);
	return res.isEmpty() == false;
}

int MMORole::check_validate_create_role(Int64 role_id)
{
	BSONObj fields = BSON(Role::ID << 1 << Role::CREATE_TIME << 1);
	BSONObj res = this->conection().findOne(Role::COLLECTION,
			QUERY(Role::ID << role_id), &fields);

	JUDGE_RETURN(res.isEmpty() == false, false);
	return res[Role::CREATE_TIME].numberLong() > 0;
}

bool MMORole::validate_player(Int64 role_id)
{
	BSONObj fields = BSON(Role::ID << 1);
	BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION,
			QUERY(Role::ID << role_id), &fields);

	return res.isEmpty() == false;
}

bool MMORole::validate_league_member(Int64 role_id, Int64 league_index)
{
	BSONObj fields = BSON(Role::ID << 1 << Role::LEAGUE_ID << 1);
	BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION,
			QUERY(Role::ID << role_id), &fields);

	JUDGE_RETURN(res.isEmpty() == false, false);
	return res[Role::LEAGUE_ID].numberLong() == league_index;
}

void MMORole::load_league_member(LeagueMember& league_member)
{
	BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION,
			QUERY(Role::ID << league_member.role_index_));
	JUDGE_RETURN(res.isEmpty() == false, ;);

	league_member.role_sex_ = res[Role::SEX].numberInt();
	league_member.role_career_ = res[Role::CAREER].numberInt();
	league_member.role_force_ = res[Role::FORCE].numberInt();
	league_member.new_role_force_ = league_member.role_force_;
	league_member.role_lvl_ = res[Role::LEVEL].numberInt();
	league_member.vip_type_ = res[Role::VIP_TYPE].numberInt();
	league_member.offline_tick_ = res[Role::LAST_SIGN_OUT].numberLong();
	league_member.role_name_ = res[Role::NAME].str();
}

void MMORole::ensure_all_index(void)
{
BEGIN_CATCH
	this->conection().ensureIndex(Role::COLLECTION, BSON(Role::ID << 1), true);
    this->conection().ensureIndex(DBCopyPlayer::COLLECTION, BSON(DBCopyPlayer::ID << 1), true);
    this->conection().ensureIndex(DBCopyPlayer::COLLECTION, BSON(DBCopyPlayer::NAME << 1));

    this->conection().ensureIndex(Role::COLLECTION, BSON(Role::ACCOUNT << 1 << Role::IS_ACTIVE << 1), false);
    this->conection().ensureIndex(Role::COLLECTION, BSON(Role::ACCOUNT << 1
    		<< Role::IS_ACTIVE << 1 << Role::SERVER_FLAG << 1), false);
    this->conection().ensureIndex(Role::COLLECTION, BSON(Role::NAME << 1 << Role::IS_ACTIVE << 1), false);
    this->conection().ensureIndex(Role::COLLECTION, BSON(Role::FORCE << 1), false); // 提高副本组队招募化身速度
    this->conection().ensureIndex(Role::COLLECTION, BSON(Role::SEX << 1), false);

    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::ROLE_ID << 1), true);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::ACCOUNT << 1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::ONLINE << 1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::CREATE_TIME << -1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::SEX << 1 << DBBackRole::CREATE_TIME << -1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::AGENT_CODE << 1 << DBBackRole::CREATE_TIME << -1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::MARKET << 1 << DBBackRole::CREATE_TIME << -1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::CREATE_TIME << -1 << DBBackRole::MAC << 1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::IS_NEW_MAC << 1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::CREATE_AGENT << 1 <<DBBackRole::CREATE_IP << 1), false);
    this->conection().ensureIndex(DBBackRole::COLLECTION, BSON(DBBackRole::CREATE_AGENT << 1 <<DBBackRole::CREATE_MAC << 1), false);

    this->conection().ensureIndex(DBRpmFakeInfo::COLLECTION, BSON(DBRpmFakeInfo::CONFIG_ID << 1), true);
    this->conection().ensureIndex(DBRpmFakeInfo::COLLECTION, BSON(DBRpmFakeInfo::FORCE << 1), false);
    this->conection().ensureIndex(DBBackAccount::COLLECTION, BSON(DBBackAccount::ACCOUNT << 1), true);
END_CATCH
}

void MMORole::init_base_role_info(BaseRoleInfo& detail, const BSONObj& res)
{
//    detail.__agent_code 	= CONFIG_INSTANCE->agent_code(detail.__agent);
	detail.__agent_code 	= res[Role::AGENT_CODE].numberInt();
    detail.__market_code 	= res[Role::MARKET_CODE].numberInt();
    detail.set_server_flag(res[Role::SERVER_FLAG].str());
    detail.set_open_tick(res[Role::SERVER_TICK].numberLong());
    detail.set_combine_tick(res[Role::COMBINE_TICK].numberLong());

    detail.__id 		= res[Role::ID].numberLong();
	detail.__name 		= res[Role::NAME].str();
	detail.__src_name	= res[Role::SRC_NAME].str();
	detail.__account 	= res[Role::ACCOUNT].str();
    detail.__level 		= res[Role::LEVEL].numberInt();
    detail.__sex 		= res[Role::SEX].numberInt();
    detail.__vip_type 	= res[Role::VIP_TYPE].numberInt();
    detail.__permission = res[Role::PERMISSION].numberInt();
    detail.__league_id 	= res[Role::LEAGUE_ID].numberLong();
    detail.__partner_id = res[Role::PARTNER_ID].numberLong();
    detail.__partner_name = res[Role::PARTNER_NAME].str();
    detail.__wedding_id = res[Role::WEDDING_ID].numberLong();
    detail.__wedding_type = res[Role::WEDDING_TYPE].numberInt();
    detail.__career 	= detail.__sex;
    detail.__login_tick = ::time(NULL);

    detail.__fight_force		= res[Role::FORCE].numberInt();
    detail.__league_id 			= res[Role::LEAGUE_ID].numberLong();
    detail.__last_logout_tick	= res[Role::LAST_SIGN_OUT].numberLong();
    detail.__login_days 		= res[Role::LOGIN_DAYS].numberLong();

    std::string str_scene = Role::LOCATION + "." + Role::Location::SCENE_ID;
    detail.__scene_id = res.getFieldDotted(str_scene.c_str()).numberInt();

    if (detail.__agent_code <= 0)
    {
    	detail.__agent_code = CONFIG_INSTANCE->agent_code(detail.__agent);
    }
}

void MMORole::update_role_name(MapLogicPlayer *player, const char *src_name, MongoDataMap *data_map)
{
//	string full_name = CONFIG_INSTANCE->full_role_name(player->role_detail().__server_id, src_name);
//
//    BSONObjBuilder builder;
//    builder << Role::NAME << full_name
//    		<< Role::SRC_NAME << src_name;
//
//    data_map->push_update(Role::COLLECTION, BSON(Role::ID << player->role_id()), builder.obj());
}

int MMORole::update_player_name(MongoDataMap *data_map)
{
BEGIN_CATCH
    MongoData *mongo_data = 0;
    if (data_map->find_data(Role::COLLECTION, mongo_data) != 0)
    {
        return ERROR_SERVER_INNER;
    }

    string new_name = mongo_data->data_bson()[Role::NAME].str();

    if (this->search_player_name(new_name) > 0)
    {
        return ERROR_ROLE_EXISTS;
    }

    string src_name = mongo_data->data_bson()[Role::SRC_NAME].str();
    this->conection().update(mongo_data->table(), mongo::Query(mongo_data->condition()),
		BSON("$set" << BSON(Role::NAME << new_name
				<< Role::SRC_NAME << src_name
				<< Role::IS_FIRST_RENAME << 0)));

    return 0;
END_CATCH
    return -1;
}

int MMORole::update_player_name(DBShopMode* shop_mode)
{
	DBInputArgv& input = shop_mode->input_argv_;

	Int64 role_id = input.type_int64_;
	string src_name = input.type_string_;
	string full_name = CONFIG_INSTANCE->full_role_name(input.type_int_, src_name);

    if (this->search_player_name(full_name) > 0)
    {
    	shop_mode->output_argv_.type_int_ = -1;
    	shop_mode->output_argv_.int_vec_.push_back(ERROR_ROLE_EXISTS);
    }
    else
    {
    	shop_mode->output_argv_.type_int_ = 1;
    	shop_mode->output_argv_.type_string_ = full_name;
        this->conection().update(Role::COLLECTION, BSON(Role::ID << role_id),
        		BSON("$set" << BSON(Role::NAME << full_name << Role::SRC_NAME << src_name)));
    }

    return 0;
}

std::string MMORole::fetch_role_name(BSONObj &obj)
{
	return obj[Role::NAME].str();
}

std::string MMORole::fetch_player_name(Int64 role_id)
{
	std::string name;
	BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION, QUERY(Role::ID << role_id));

	if (false == res.isEmpty())
	{
		return res[Role::NAME].str();
	}
	return name;
}

std::string MMORole::fetch_league_name(Int64 league_id)
{
	std::string league_name;
	BSONObj res = this->conection().findOne(DBLeague::COLLECTION,
			QUERY(DBLeague::ID << league_id));
	if (res.isEmpty() == false)
	{
		return res["name"].str();
	}

	return league_name;
}

void MMORole::load_copy_player(DBShopMode* shop_mode)
{
	Int64 role_id = shop_mode->input_argv_.type_int64_;
	*shop_mode->output_argv_.bson_obj_ = this->conection().findOne(
			DBCopyPlayer::COLLECTION, QUERY(DBCopyPlayer::ID << role_id)).copy();
}

int MMORole::update_copy_player(MapPlayerEx *player, MongoDataMap *data_map, const Int64 src_role_id)
{
	Int64 role_id = src_role_id;
	if (role_id <= 0)
	{
		role_id = player->role_id();
	}

    OfflineRoleDetail aim_detail;
    GameCommon::copy_player(&aim_detail, player);

    BSONVec bson_skill_set;
    for (IntMap::iterator iter = aim_detail.skill_map_.begin();
    		iter != aim_detail.skill_map_.end(); ++iter)
    {
        FighterSkill *skill = 0;
        JUDGE_CONTINUE(player->find_skill(iter->first, skill) == 0);

        bson_skill_set.push_back(BSON(Skill::SSkill::SKILL_ID << iter->first
        			<< Skill::SSkill::LEVEL << skill->__level
                    << Skill::SSkill::USETICK << BSON(
                        Skill::SSkill::Tick::SEC << int(skill->__use_tick.sec())
                        << Skill::SSkill::Tick::USEC << int(skill->__use_tick.usec()))));
    }

    BSONVec bson_shape_set;
    GameCommon::map_to_bson(bson_shape_set, aim_detail.shape_map_);


    BSONObjBuilder builder;
    builder << DBCopyPlayer::NAME << aim_detail.role_info_.__name
        << DBCopyPlayer::LEVEL << aim_detail.role_info_.__level
        << DBCopyPlayer::SEX << aim_detail.role_info_.__sex
        << DBCopyPlayer::LEAGUE_ID << aim_detail.role_info_.__league_id
        << DBCopyPlayer::LEAGUE_NAME << aim_detail.role_info_.__league_name
        << DBCopyPlayer::LEAGUE_POS << aim_detail.role_info_.__league_pos
        << DBCopyPlayer::PARTNER_ID << aim_detail.role_info_.__partner_id
        << DBCopyPlayer::PARTNER_NAME << aim_detail.role_info_.__partner_name
        << DBCopyPlayer::WEDDING_ID << aim_detail.role_info_.__wedding_id
        << DBCopyPlayer::WEDDING_TYPE << aim_detail.role_info_.__wedding_type
        << DBCopyPlayer::VIP_TYPE << aim_detail.role_info_.__vip_type
        << DBCopyPlayer::FORCE << aim_detail.role_info_.__fight_force
        << DBCopyPlayer::CAREER << aim_detail.role_info_.__career
        << DBCopyPlayer::ATTACK_LOWER << aim_detail.attack_lower_
        << DBCopyPlayer::ATTACK_UPPER << aim_detail.attack_upper_
    	<< DBCopyPlayer::DEFENCE_LOWER << aim_detail.defence_lower_
    	<< DBCopyPlayer::DEFENCE_UPPER << aim_detail.defence_upper_
    	<< DBCopyPlayer::HIT << aim_detail.total_hit_
    	<< DBCopyPlayer::DODGE << aim_detail.total_avoid_
    	<< DBCopyPlayer::CRIT << aim_detail.total_crit_
    	<< DBCopyPlayer::TOUGHNESS << aim_detail.total_toughness_
    	<< DBCopyPlayer::BLOOD << aim_detail.total_blood_
    	<< DBCopyPlayer::MAGIC << aim_detail.total_magic_
    	<< DBCopyPlayer::SPEED << aim_detail.total_speed_
    	<< DBCopyPlayer::DAMAGE << aim_detail.total_damage_
    	<< DBCopyPlayer::REDUCTION << aim_detail.total_reduction_
		<< DBCopyPlayer::EQUIP_REFINE_LVL << aim_detail.equie_refine_lvl_
		<< DBCopyPlayer::WING_LEVEL << player->mount_detail(GameEnum::FUN_XIAN_WING).mount_grade_
		<< DBCopyPlayer::SOLIDER_LEVEL << player->mount_detail(GameEnum::FUN_GOD_SOLIDER).mount_grade_
		<< DBCopyPlayer::MAGIC_LEVEL << player->mount_detail(GameEnum::FUN_MAGIC_EQUIP).mount_grade_
		<< DBCopyPlayer::BEAST_LEVEL << player->mount_detail(GameEnum::FUN_LING_BEAST).mount_grade_
		<< DBCopyPlayer::MOUNT_LEVEL << player->mount_detail(GameEnum::FUN_MOUNT).mount_grade_
		<< DBCopyPlayer::FASHION_ID << player->fashion_detail().select_id_
		<< DBCopyPlayer::FASHION_COLOR << player->fashion_detail().sel_color_id_
    	<< DBCopyPlayer::SKILL_SET << bson_skill_set
    	<< DBCopyPlayer::SHAPE_SET << bson_shape_set;


	{
		int weapon_id = aim_detail.magic_weapon_info_.__weapon_id;
		int weapon_level = aim_detail.magic_weapon_info_.__weapon_level;
		builder << DBCopyPlayer::WEAPON_LEVEL << (weapon_id * 1000 + weapon_level);
	}

    data_map->push_update(DBCopyPlayer::COLLECTION, BSON(DBCopyPlayer::ID << role_id), builder.obj(), true);
	return 0;
}

int MMORole::add_test_copy_player(LongMap& add_map, const LongSet& exist_map, uint differ)
{
	auto_ptr<DBClientCursor> cursor = CACHED_CONNECTION.query(DBCopyPlayer::COLLECTION);

	while (cursor->more())
	{
		BSONObj res = cursor->next();
		JUDGE_CONTINUE(res.isEmpty() == false);

		Int64 role_id = res[DBCopyPlayer::ID].numberLong();
		JUDGE_CONTINUE(exist_map.count(role_id) == 0);

		add_map[role_id] = true;
		JUDGE_CONTINUE(add_map.size() >= differ);

		return 0;
	}

	return 0;
}

int MMORole::fetch_random_name(MongoDataMap *data_map)
{
	MongoData *mongo_data = 0;
	if (data_map->find_data(Role::COLLECTION, mongo_data) != 0)
	{
		return ERROR_SERVER_INNER;
	}

	string account = mongo_data->condition()[Role::ACCOUNT].str();
	if (account.empty() == false
			&& this->conection().findOne(DBBackAccount::COLLECTION,
					QUERY(DBBackAccount::ACCOUNT << account)).isEmpty() == true)
	{
		BSONObjBuilder builder;
		builder << DBBackAccount::CREATE << int(1)
				<< DBBackAccount::CREATE_TIME << Int64(::time(NULL));
		this->conection().update(DBBackAccount::COLLECTION,
				QUERY(DBBackAccount::ACCOUNT << account), BSON("$set" << builder.obj()), true);
	}

	int loop = 0;
	while (true)
	{
		const Json::Value &first_name_json = CONFIG_INSTANCE->first_name();
		std::string first_name;
		int rand_val = 0;
		if (first_name_json.size() > 0)
		{
			rand_val = ::rand() % first_name_json.size();
			first_name = first_name_json[rand_val].asString();
		}

		const Json::Value *p_second_name_json = 0, *p_third_name_json = 0;
		int sex = mongo_data->condition()[Role::SEX].numberInt();
		if (sex == GameEnum::SEX_MALE)
		{
			p_second_name_json = &(CONFIG_INSTANCE->man_second());
			p_third_name_json = &(CONFIG_INSTANCE->man_third());
		}
		else
		{
			p_second_name_json = &(CONFIG_INSTANCE->woman_second());
			p_third_name_json = &(CONFIG_INSTANCE->woman_third());
		}

		std::string second_name;
		const Json::Value *name_json[] = {p_second_name_json, p_third_name_json};
		int rand_name_size = ::rand() % 2 + 1;
		for (int i = 0; i < rand_name_size; ++i)
		{
			JUDGE_CONTINUE(name_json[i]->size() > 0);

			const Json::Value &json = *(name_json[i]);
			rand_val = ::rand() % json.size();
			second_name += json[rand_val].asString();
		}

		first_name += second_name;
		if (this->search_player_name(first_name) < 0)
		{
			BSONObjBuilder builder;
			builder << Role::NAME << first_name;
			mongo_data->set_data_bson(builder.obj());
			break;
		}

		if ((++loop) >= 1000)
		{
			BSONObjBuilder builder;
			builder << Role::NAME << first_name;
			mongo_data->set_data_bson(builder.obj());
			break;
		}
	}

	return 0;
}

void MMORole::update_sex_condition(MongoDataMap *data_map, int sex, const string& account)
{
    data_map->push_find(Role::COLLECTION, BSON(Role::SEX << sex << Role::ACCOUNT << account));
}

int MMORole::sync_recharge_info_to_all_role(RechargeOrder* recharge_order)
{
	JUDGE_RETURN(NULL != recharge_order, -1);

	std::string account = recharge_order->__account;
	Int64 exclusive_id = recharge_order->__role_id;

	auto_ptr<DBClientCursor> cursor = this->conection().query(Role::COLLECTION,
			QUERY(Role::ACCOUNT << account));
	while(cursor->more())
	{
		BSONObj res = cursor->next();
		Int64 role_id = res[Role::ID].numberLong();
		JUDGE_CONTINUE(exclusive_id != role_id);

		const std::string table_name = Package::MONEY + "." + Package::Money::GOLD;
		this->conection().update(Package::COLLECTION, BSON(Package::ID << role_id),
				BSON("$inc" << BSON(table_name << recharge_order->__gold)), true);

		this->conection().update(Package::COLLECTION, BSON(Package::ID << role_id),
				BSON("$set" << BSON(Package::RECHARGE_GOLD << recharge_order->__money
						<< Package::RECHARGE_FIRST_TICK << recharge_order->__tick)), true);
	}
	return 0;
}

int MMORole::update_back_role_offline(void)
{
	int now_t = ::time(0);
	this->conection().update(DBBackRole::COLLECTION, QUERY(DBBackRole::ONLINE << 1),
			BSON("$set" << BSON(DBBackRole::ONLINE << 0 << DBBackRole::LAST_SIGN_OUT_TIME << now_t)), false, true);
	return 0;
}

int MMORole::fetch_role_info_for_wedding(const Int64 role_id, std::string &name, int &sex, int &career)
{
    BSONObj ret_fields = BSON(Role::ID << 1 << Role::NAME << 1 << Role::SEX << 1 << Role::CAREER << 1);

    BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION, QUERY(Role::ID << role_id), &ret_fields);
    if (res.isEmpty() == true)
        return 0;

    name = res[Role::NAME].str();
    sex = res[Role::SEX].numberInt();
    career = res[Role::CAREER].numberInt();
    return 0;
}

int MMORole::fetch_role_info_for_brother(const Int64 role_id, std::string &name, int &sex, int &career, int& level)
{
    BSONObj ret_fields = BSON(Role::ID << 1 << Role::NAME << 1 << Role::SEX << 1 << Role::CAREER << 1 << Role::LEVEL << 1);

    BSONObj res = CACHED_CONNECTION.findOne(Role::COLLECTION, QUERY(Role::ID << role_id), &ret_fields);
    if (res.isEmpty() == true)
        return 0;

    name = res[Role::NAME].str();
    sex = res[Role::SEX].numberInt();
    career = res[Role::CAREER].numberInt();
    level = res[Role::LEVEL].numberInt();
    return 0;
}

int MMORole::load_all_role_id(DBShopMode* shop_mode)
{
	BSONObj show_field =  BSON(Role::ID << 1<< Role::AGENT_CODE << 1);
	auto_ptr<DBClientCursor> cursor = this->conection().query(Role::COLLECTION, Query(), 0, 0, &show_field);

	bool is_all_agent = shop_mode->input_argv_.extra_int_vec_.empty();
	while (cursor->more())
	{
		BSONObj res = cursor->next();
		bool find_agent = false;
		if(is_all_agent == false)
		{
			int agent = res[Role::AGENT_CODE].numberInt();
			for(IntVec::const_iterator it = shop_mode->input_argv_.extra_int_vec_.begin();
					it != shop_mode->input_argv_.extra_int_vec_.end(); ++it)
			{
				if(agent == *it)
				{
					find_agent = true;
					break;
				}
			}
		}
		if(is_all_agent || find_agent)
		{
			shop_mode->output_argv_.long_vec_.push_back(res[Role::ID].numberLong());
		}
	}
	return 0;
}

int MMORole::load_rank_roleid_carrer(DBShopMode* shop_mode)
{
	BSONObj show_field =  BSON(Role::ID << 1 << Role::CAREER << 1);
	BSONObj query = BSON(Role::ID << BSON("$in" << shop_mode->input_argv_.extra_long_vec_));

	auto_ptr<DBClientCursor> cursor = this->conection().query(Role::COLLECTION, Query(query), 0, 0, &show_field);
	while(cursor->more())
	{
		BSONObj res = cursor->next();
		BSONObj tmp = BSON(Role::ID << res[Role::ID].numberLong() << Role::CAREER << res[Role::CAREER].numberInt());
		shop_mode->output_argv_.bson_vec_->push_back(tmp);
	}
	return 0;
}

void MMORole::load_sys_model(const Int64 role_id,string& sys_model,string& market_code)
{

    BSONObj res = CACHED_CONNECTION.findOne(DBBackRole::COLLECTION,QUERY(DBBackRole::ROLE_ID << role_id));
	if (res.isEmpty() == true)
        return ;
	sys_model = res[DBBackRole::MAC].str();
	static char market[20]={0};
	snprintf(market, sizeof(market),"%d",res[DBBackRole::MARKET].Int());
	market_code = market;
}


MMORoleEx::~MMORoleEx()
{
}

int MMORoleEx::load_role_ex(MapLogicPlayer *player)
{
	BEGIN_CATCH
    	RoleExDetail &detail = player->role_ex_detail();

	    BSONObj res = this->conection().findOne(RoleEx::COLLECTION, QUERY(RoleEx::ID << player->role_id()));
	    if (res.isEmpty())
	    {
		    detail.reset();
	        return 0;
	    }
	    detail.__box.__box_open_count_one = res[RoleEx::BOX_OPEN_COUNT_ONE].numberInt();
	    detail.__box.__box_open_count_ten = res[RoleEx::BOX_OPEN_COUNT_TEN].numberInt();
	    detail.__box.__box_open_count_fifty = res[RoleEx::BOX_OPEN_COUNT_FIFTY].numberInt();
	    detail.__savvy = res[RoleEx::SAVVY].numberInt();
	    detail.__box.__box_is_open = res[RoleEx::BOX_IS_OPEN].numberInt();
	    detail.__is_second_equip_decompose = res[RoleEx::SECOND_DECOMPOSE].numberInt();

	    return 0;
	END_CATCH
	    return -1;
}
int MMORoleEx::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	RoleExDetail &detail = player->role_ex_detail();
    BSONObjBuilder builder;
    builder << RoleEx::BOX_OPEN_COUNT_ONE << detail.__box.__box_open_count_one
    		<< RoleEx::BOX_OPEN_COUNT_TEN << detail.__box.__box_open_count_ten
    		<< RoleEx::BOX_OPEN_COUNT_FIFTY << detail.__box.__box_open_count_fifty
    		<< RoleEx::BOX_IS_OPEN << detail.__box.__box_is_open
    		<< RoleEx::SAVVY << detail.__savvy
    		<< RoleEx::SECOND_DECOMPOSE << detail.__is_second_equip_decompose;

    data_map->push_update(RoleEx::COLLECTION,
            BSON(RoleEx::ID << player->role_id()), builder.obj(), true);
	return 0;
}

int MMORoleEx::load_role_ex(LogicPlayer *player)
{
	BEGIN_CATCH
	    BSONObj res = this->conection().findOne(RoleEx::COLLECTION, QUERY(RoleEx::ID << player->role_id()));
	    if (res.isEmpty())
	    {
	        return 0;
	    }
	    {
			LuckyTableDetial &ltable_detail = player->fetch_ltable_detail();
			int i = 0;

	    	BSONObjIterator iter = res.getObjectField(RoleEx::LTABLE_LEFT_TIMES.c_str());
			while (iter.more())
			{
				ltable_detail.left_times[i++] = iter.next().numberInt();
			}
			i = 0;
			iter = res.getObjectField(RoleEx::LTABLE_EXEC_TIMES.c_str());
			while (iter.more())
			{
				ltable_detail.exec_times[i++] = iter.next().numberInt();
			}
			i = 0;
			iter = res.getObjectField(RoleEx::LTABLE_GOLD.c_str());
			while (iter.more())
			{
				ltable_detail.gold[i++] = iter.next().numberInt();
			}
			ltable_detail.activity_key = res[RoleEx::LTABLE_KEY].numberLong();
			player->check_and_reset_ltable();
	    }

	    return 0;
	END_CATCH
	    return -1;
}
int MMORoleEx::update_data(LogicPlayer *player, MongoDataMap *mongo_data)
{
	LuckyTableDetial &ltable_detail = player->fetch_ltable_detail();
	IntVec left_times,exec_times,ltable_gold;
	{
		left_times.push_back(ltable_detail.left_times[0]);
		left_times.push_back(ltable_detail.left_times[1]);
		exec_times.push_back(ltable_detail.exec_times[0]);
		exec_times.push_back(ltable_detail.exec_times[1]);
		ltable_gold.push_back(ltable_detail.gold[0]);
		ltable_gold.push_back(ltable_detail.gold[1]);
	}
    BSONObjBuilder builder;
    builder << RoleEx::LTABLE_LEFT_TIMES << left_times
    		<< RoleEx::LTABLE_EXEC_TIMES << exec_times
    		<< RoleEx::LTABLE_GOLD << ltable_gold
    		<< RoleEx::LTABLE_KEY << ltable_detail.activity_key;

    mongo_data->push_update(RoleEx::COLLECTION,
            BSON(RoleEx::ID << player->role_id()), builder.obj(), true);
	return 0;
}

void MMORoleEx::ensure_all_index(void)
{
BEGIN_CATCH
    this->conection().ensureIndex(RoleEx::COLLECTION, BSON(RoleEx::ID << 1), true);
END_CATCH
}
