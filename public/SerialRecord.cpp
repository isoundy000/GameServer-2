/*
 * SerialRecord.cpp
 *
 *  Created on: 2013-7-12
 *      Author: root
 */

#include <stdio.h>

#include "SerialRecord.h"
#include "Log.h"
#include "ProtoInnerPublic.pb.h"
#include "ProtoInner010.pb.h"
#include "ProtoInner009.pb.h"

#include "GameEnum.h"
#include "MapMonitor.h"
#include "MapLogicPlayer.h"

SerialRecord::SerialRecord()
{

}

SerialRecord::~SerialRecord()
{

}

inline std::string SerialRecord::get_table_name(Int64 role_id,
		std::string base_table_name)
{
//	int index = role_id % 100 + 1;
//	char sub_name[32] = { 0 };
//	snprintf(sub_name, sizeof(sub_name), "_%03d", index);
//	return base_table_name + sub_name;
	return base_table_name;
}

// role_id,serial_type,sub_serial_type,sub_agent,money_type,money,bind_money,remain_money,remain_bind_money,time
int SerialRecord::record_money(EntityCommunicate* entity, int agent_code, int market_code,
		const SerialObj &serial,const Money &money, const Money &remain_money)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::MONEY_SERIAL_RECORD), -1);

	RecordMoney record;
	Int64 role_id = entity->entity_id();

	int result = 0;
	if (money.__gold != 0 || money.__bind_gold != 0)
	{
		record.set_table_name(this->get_table_name(role_id, "money_serial"));
		record.set_role_id(role_id);
		record.set_serial_type(serial.type_);
		record.set_sub_serial_type(serial.sub_);
		record.set_sub_agent(agent_code);
		record.set_platform(0);
		record.set_market(market_code);

		record.set_money_type(GameEnum::MONEY_GOLD);
		record.set_money(money.__gold);
		record.set_bind_money(money.__bind_gold);
		record.set_remain_money(remain_money.__gold);
		record.set_remain_bind_money(remain_money.__bind_gold);
		record.set_time(Time_Value::gettimeofday().sec());

		result += this->request_save_serial(&record, true, entity);
	}

	if (money.__copper != 0 || money.__bind_copper != 0)
	{
		record.set_table_name(this->get_table_name(role_id, "money_serial"));
		record.set_role_id(role_id);
		record.set_serial_type(serial.type_);
		record.set_sub_serial_type(serial.sub_);
		record.set_sub_agent(agent_code);
		record.set_platform(0);
		record.set_market(market_code);

		record.set_money_type(GameEnum::MONEY_COPPER);
		record.set_money(money.__copper);
		record.set_bind_money(money.__bind_copper);
		record.set_remain_money(remain_money.__copper);
		record.set_remain_bind_money(remain_money.__bind_copper);
		record.set_time(Time_Value::gettimeofday().sec());

		result += this->request_save_serial(&record, true, entity);
	}

	return result;
}

int SerialRecord::record_item(MLPacker* player, int agent_code, int market_code,
		const SerialObj& serial, const ItemObj& item_obj, Int64 src_role_id)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::ITEM_SERIAL_RECORD), -1);

	RecordItem record;
	record.set_table_name(this->get_table_name(player->role_id(), "item_serial"));
	record.set_role_id(player->role_id());
	record.set_role_level(player->role_level());
	record.set_src_role_id(src_role_id);
	record.set_serial_type(serial.type_);
	record.set_sub_serial_type(serial.sub_);
	record.set_sub_agent(agent_code);
	record.set_platform(0);
	record.set_market(market_code);

	record.set_item_id(item_obj.id_);
	record.set_amount(item_obj.amount_);
	record.set_bind(item_obj.bind_);
	record.set_time(::time(NULL));

	return this->request_save_serial(&record, true, player);
}

int SerialRecord::record_player_level(EntityCommunicate* entity, int agent_code,
		int market_code, int serial_type, int level)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::PLAYER_LEVEL_RECORD), -1);

	RecordPlayerLevel record;
	record.set_table_name(this->get_table_name(entity->entity_id(), "level_serial"));
	record.set_role_id(entity->entity_id());
	record.set_serial_type(serial_type);
	record.set_sub_agent(agent_code);
	record.set_platform(market_code);
	record.set_level(level);
	record.set_time(::time(NULL));

	return this->request_save_serial(&record, true, entity);
}

int SerialRecord::record_other_serial(const SerialPlayer& player, int agent_code, int market_code,
		int main_serail, int sub_serail, Int64 value,
		Int64 ext1, Int64 ext2)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::OTHER_SERIAL_RECORD), -1);

	RecordOtherSerial record;
	record.set_table_name(this->get_table_name(player.id_, "other_serial"));
	record.set_role_id(player.id_);
	record.set_serial_type(main_serail);
	record.set_sub_serial_type(sub_serail);
	record.set_sub_agent(agent_code);
	record.set_platform(market_code);

	record.set_value(value);
	record.set_ext1(ext1);
	record.set_ext2(ext2);
	record.set_time(Time_Value::gettimeofday().sec());

    return this->request_save_serial(&record, true, player.entity_);
}

int SerialRecord::record_equipment(EntityCommunicate* entity, int agent_code, int market_code,
		int serial_type, int sub_serial_type, RecordEquipObj& equip_obj)
{
	Int64 role_id = entity->entity_id();
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::EQUIP_SERIAL_RECORD), -1);

	RecordEquipment record;
	record.set_table_name(this->get_table_name(role_id, "equipment_serial"));
	record.set_role_id(role_id);
	record.set_serial_type(serial_type);
	record.set_serial_sub_type(sub_serial_type);
	record.set_sub_agent(agent_code);
	record.set_platform(market_code);

	record.set_equip_id(equip_obj.equip_id_);
	record.set_amount(1);
	record.set_bind(equip_obj.equip_bind_);
	record.set_index(equip_obj.equip_index_);
	record.set_pack_type(equip_obj.pack_type_);
	record.set_refine_level(equip_obj.strengthen_level_);
	record.set_refine_degree(equip_obj.refine_degree_);
	record.set_luck_value(equip_obj.luck_value);

	Json::Value jewel;
	for(IntMap::const_iterator it = equip_obj.jewel_list.begin();
			it != equip_obj.jewel_list.end(); ++it)
	{
		Json::Value tmp;
		tmp["type"] = it->first;
		tmp["id"] = it->second;
		jewel["jewel_lists"].append(tmp);
	}

	Json::Value extern_attr;
	for(IntMap::const_iterator it = equip_obj.good_refine.begin();
			it != equip_obj.good_refine.end(); ++it)
	{
		Json::Value tmp;
		tmp["id"] = it->first;
		tmp["amount"] = it->second;
		extern_attr["good_refine"].append(tmp);
	}

	MoldingSpiritDetail &molding_detail = equip_obj.molding_detail_;
	Json::Value molding_json;
	for(int i = GameEnum::MOLDING_SPIRIT_BEGIN; i < GameEnum::MOLDING_SPIRIT_END; ++i)
	{
		Json::Value tmp;
		tmp["type"] = i;
		tmp["level"] = molding_detail.fetch_nature_level(i);
		tmp["schedule"] = molding_detail.fetch_nature_schedule(i);
		molding_json["molding"].append(tmp);
	}

	Json::FastWriter wjson;
	record.set_jewel_lists(wjson.write(jewel));
	record.set_extern_attr(wjson.write(extern_attr));
	record.set_molding(wjson.write(molding_json));
	record.set_time(Time_Value::gettimeofday().sec());
	return this->request_save_serial(&record, true, entity);
}

int SerialRecord::record_mount(MapLogicPlayer* player, int agent, int serial, int type)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::MOUNT_SERIAL_RECORD), -1);

	MountDetail& mount = player->mount_detail(type);
	GamePackage* package = player->find_package(MLMounter::mount_to_index(type));

	RecordMount record;
	record.set_table_name(this->get_table_name(player->role_id(), "mount_serial"));
	record.set_role_id(player->role_id());
	record.set_sub_agent(agent);
	record.set_serial_type(serial);
	record.set_mount_type(mount.type_);
	record.set_grade(mount.mount_grade_);
	record.set_bless(mount.bless_);
	record.set_time(::time(NULL));

	int index = 0;
	Json::Value skill_json;
	for(SkillMap::const_iterator iter = mount.skill_map_.begin();
			iter != mount.skill_map_.end(); ++iter)
	{
		Json::Value tmp;
		tmp["id"] = iter->second->__skill_id;
		tmp["level"] = iter->second->__level;

		skill_json[index] = tmp;
		++index;
	}

	index = 0;
	Json::Value equip_json;
	for (ItemListMap::iterator iter = package->item_map().begin();
			iter != package->item_map().end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item != NULL);
		equip_json[index] = item->__id;
		++index;
	}

	Json::FastWriter wjson;
	record.set_skill(wjson.write(skill_json));
	record.set_equip(wjson.write(equip_json));
	return this->request_save_serial(&record, true, player);
}

int SerialRecord::record_mail_detail(EntityCommunicate* entity, int agent_code, int market_code,
		int serial_type, const MailDetailSerialObj& mail_obj)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::MAIL_SERIAL_RECORD), -1);

	RecordMail record;
	record.set_table_name("mail_detail");
	record.set_mail_index(mail_obj.__mail_index);
	record.set_sender_id(mail_obj.__sender_id);
	record.set_receiver_id(mail_obj.__receiver_id);
	record.set_send_tick(mail_obj.__send_tick);
	record.set_read_tick(mail_obj.__read_tick);
	record.set_mail_type(mail_obj.__mail_type);
	record.set_mail_format(mail_obj.__mail_format_);
	record.set_has_read(mail_obj.__has_read);
	record.set_receiver_name(mail_obj.__receiver_name);
	record.set_sender_name(mail_obj.__sender_name);
	record.set_title(mail_obj.__title);
	record.set_content(mail_obj.__content);

	record.set_sub_agent(agent_code);
	record.set_platform(market_code);
	record.set_serial_type(serial_type);

	record.set_attach_gold(mail_obj.__attach_money.__gold);
	record.set_attach_copper(mail_obj.__attach_money.__copper);
	record.set_attach_bind_gold(mail_obj.__attach_money.__bind_gold);
	record.set_attach_bind_copper(mail_obj.__attach_money.__bind_copper);

	record.set_attach_id_1(0);
	record.set_attach_amount_1(0);
	record.set_attach_id_2(0);
	record.set_attach_amount_2(0);
	record.set_attach_id_3(0);
	record.set_attach_amount_3(0);
	record.set_attach_id_4(0);
	record.set_attach_amount_4(0);

	for(int i = 0; i < (int)mail_obj.__attach_item.size(); ++i)
	{
		const ThreeObj& obj = mail_obj.__attach_item[i];
		switch(obj.tick_)
		{
		case 1:
			record.set_attach_id_1((int) obj.id_);
			record.set_attach_amount_1(obj.value_);
			break;
		case 2:
			record.set_attach_id_2((int) obj.id_);
			record.set_attach_amount_2(obj.value_);
			break;
		case 3:
			record.set_attach_id_3((int) obj.id_);
			record.set_attach_amount_3(obj.value_);
			break;
		case 4:
			record.set_attach_id_4((int) obj.id_);
			record.set_attach_amount_4(obj.value_);
			break;
		}
	}

	return this->request_save_serial(&record, true, entity);
}

int SerialRecord::record_online_users(int agent_code, int market, int users, int hooking_users, int time)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::ONLINE_USER_RECORD), -1);

	RecordOnlineUsers record;
	record.set_table_name("online_serial");
	record.set_sub_agent(agent_code);
//	record.set_platform(platform_code);	// 统计不再区分平台, 不要写入该字段
	record.set_market(market);
	record.set_users(users);
	record.set_hooking_users(hooking_users);
	record.set_time(time);
	return this->request_save_serial(&record, true);
}

int SerialRecord::record_login_logout(Int64 role_id, string role_name,
		int level, string account, string client_ip, Int64 login_time,
		Int64 logout_time, int sub_agent, int market_code,
		string sys_model, string sys_version, string mac)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::LOGIN_RECORD), -1);
	JUDGE_RETURN(login_time > 0, -1);

	RecordLoginLogout record;
	record.set_table_name("loginout_serial");

	record.set_role_id(role_id);
	record.set_role_name(role_name);
	record.set_level(level);
	record.set_account(account);
	record.set_client_ip(client_ip);
	record.set_login_time(login_time);
	record.set_logout_time(logout_time);
	record.set_online_time(logout_time-login_time);
	record.set_sub_agent(sub_agent);
	record.set_platform(0);
	record.set_market(market_code);
	record.set_sys_model(sys_model);
	record.set_sys_version(sys_version);
	record.set_mac(mac);

	return this->request_save_serial(&record, true);
}

int SerialRecord::record_task(EntityCommunicate* entity, int agent_code, int market_code,
			int serial_type, int task_id, int level)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::TASK_RECORD), -1);

	RecordTask record;
	record.set_table_name("task_serial");
	record.set_role_id(entity->entity_id());
	record.set_serial_type(serial_type);
	record.set_platform(0);
	record.set_sub_agent(agent_code);
	record.set_market(market_code);
	record.set_task_id(task_id);
	record.set_level(level);
	record.set_time(Time_Value::gettimeofday().sec());

	return this->request_save_serial(&record, true, entity);
}

int SerialRecord::record_rank(const PairObj& key, const string& name,
		int rank_type, int value, Int64 time,
		Int64 ext_int_1, Int64 ext_int_2, const string& ext_str_1)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::RANK_RECORD), -1);

	RecordRank record;
	record.set_table_name("rank_serial");
	record.set_role_id(key.id_);
	record.set_vip(key.value_);
	record.set_role_name(name);
	record.set_rank_type(rank_type);
	record.set_value(value);
	record.set_time(time);	// 同一排行榜的多条数据时间要一样

	record.set_ext_int_1(ext_int_1);
	record.set_ext_int_2(ext_int_2);
	record.set_ext_str_1(ext_str_1);

	return this->request_save_serial(&record, true);
}

int SerialRecord::record_chat(Int64 role_id, int serial_type, Int64 time,
		const string& content, int sub_agent, const string &server_flag, int market_code)
{
	JUDGE_RETURN(FLOW_INSTANCE->is_need_serial_record(GameEnum::CHAT_RECORD), -1);

	RecordChat record;
	record.set_table_name("chat_serial");
	record.set_role_id(role_id);
	record.set_serial_type(serial_type);
	record.set_time(time);
	record.set_content(content);
	record.set_sub_agent(sub_agent);
	record.set_server_flag(server_flag);
	record.set_market(market_code);

	return this->request_save_serial(&record, true);
}

int SerialRecord::record_activity(SERIAL_ACTIVITY_TYPE act_type, int total_attend,
		const PairObj& sub1, const PairObj& sub2, const string& sub3)
{
	RecordActivity record;
	record.set_table_name("activity_serial");
	record.set_time(::time(NULL));
	record.set_act_type(act_type);
	record.set_total_attend(total_attend);

	record.set_sub1_key(sub1.id_);
	record.set_sub1_value(sub1.value_);
	record.set_sub2_key(sub2.id_);
	record.set_sub2_value(sub1.value_);
	record.set_sub3(sub3);

	return this->request_save_serial(&record, true);
}

int SerialRecord::request_save_serial(Message *data, bool with_table_name, EntityCommunicate* entity)
{
    if (entity != NULL
    		&& MapMonitorSingle::instance()->is_inited() == true
    		&& MAP_MONITOR->is_has_travel_scene() == true)
    {
    	Proto30600104 inner;
    	inner.set_flag(with_table_name);
    	inner.set_name(data->GetTypeName());
    	inner.set_content(data->SerializeAsString());
        return MAP_MONITOR->dispatch_to_scene(entity, SCENE_GATE, &inner);
    }
    else
    {
    	return Log::instance()->logging_mysql(data->GetTypeName(), data->SerializeAsString());
    }
}
