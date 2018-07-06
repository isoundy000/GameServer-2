/*
 * BaseLogicPlayer.cpp
 *
 *  Created on: Jul 5, 2013
 *      Author: peizhibi
 */

#include "BaseLogicPlayer.h"
#include "ProtoDefine.h"

#include "LogicPlayer.h"
#include "LogicMonitor.h"
#include "SerialRecord.h"


BaseLogicPlayer::BaseLogicPlayer()
{
	// TODO Auto-generated constructor stub

}

BaseLogicPlayer::~BaseLogicPlayer()
{
	// TODO Auto-generated destructor stub
}

LogicPlayer* BaseLogicPlayer::logic_player()
{
	return dynamic_cast<LogicPlayer*>(this);
}

LogicPlayer* BaseLogicPlayer::find_player(Int64 role_id)
{
	LogicPlayer* player = NULL;
	LOGIC_MONITOR->find_player(role_id, player);
	return player;
}

LogicPlayer* BaseLogicPlayer::find_player(const std::string& role_name)
{
	LogicPlayer* player = NULL;
	this->monitor()->find_player(role_name, player);
	return player;
}

LogicPlayer* BaseLogicPlayer::validate_find_player(Int64 role_id, int recogn,
		int errorno)
{
	LogicPlayer* player = this->find_player(role_id);
	JUDGE_RETURN(player == NULL, player);

	this->respond_to_client_error(recogn, errorno);
	return player;
}

bool BaseLogicPlayer::online_flag(Int64 role_id)
{
	return this->find_player(role_id) != NULL;
}

int BaseLogicPlayer::request_add_exp(int add_exp, const SerialObj& serial_obj)
{
    Proto30400013 add_info;
    add_info.set_add_exp(add_exp);

    ProtoSerialObj* proto_serial = add_info.mutable_serial_obj();
    serial_obj.serialize(proto_serial);

    return LOGIC_MONITOR->dispatch_to_scene(this, &add_info);
}

int BaseLogicPlayer::request_add_savvy(int add_savvy, const SerialObj& serial_obj)
{
	Proto31400006 add_info;
    add_info.set_add_savvy(add_savvy);

    ProtoSerialObj* proto_serial = add_info.mutable_serial_obj();
    serial_obj.serialize(proto_serial);

    return LOGIC_MONITOR->dispatch_to_scene(this, &add_info);
}


int BaseLogicPlayer::request_add_anima(int add_anima, const SerialObj& serial_obj)
{
	JUDGE_RETURN(add_anima > 0, -1);

    Proto31400010 add_info;
    add_info.set_add_anima(add_anima);

    ProtoSerialObj* proto_serial = add_info.mutable_serial_obj();
    serial_obj.serialize(proto_serial);

    return LOGIC_MONITOR->dispatch_to_scene(this, &add_info);
}

int BaseLogicPlayer::request_add_money(const Money& money, const SerialObj& serial_obj, int is_notify)
{
	JUDGE_RETURN(GameCommon::validate_money(money) == true, -1);
	Proto31400011 money_info;

	money.serialize(money_info.mutable_add_money());
	serial_obj.serialize(money_info.mutable_serial_obj());
	money_info.set_is_notify(is_notify);
	return LOGIC_MONITOR->dispatch_to_scene(this, &money_info);
}

int BaseLogicPlayer::request_add_goods(int source_type, const ItemObj& item_obj,
		const SerialObj& serial_obj)
{
	JUDGE_RETURN(item_obj.amount_ > 0, -1);

	Proto31400012 item_info;
	item_info.set_source_type(source_type);

	item_obj.serialize(item_info.mutable_item_set());
	serial_obj.serialize(item_info.mutable_serial_obj());

	return LOGIC_MONITOR->dispatch_to_scene(this, &item_info);
}

int BaseLogicPlayer::request_remove_goods(PackageItem &item_obj, const SerialObj& serial_obj)
{
	JUDGE_RETURN(item_obj.__amount > 0, -1);

	Proto31400041 item_info;

	item_obj.serialize(item_info.mutable_item_set());
	serial_obj.serialize(item_info.mutable_serial());

	return LOGIC_MONITOR->dispatch_to_scene(this, &item_info);
}

int BaseLogicPlayer::request_add_goods(int source_type, PackageItem &item_obj, const SerialObj& serial_obj)
{
	JUDGE_RETURN(item_obj.__amount > 0, -1);

	Proto31400012 item_info;
	item_info.set_source_type(source_type);

	item_obj.serialize(item_info.mutable_item_set());
	serial_obj.serialize(item_info.mutable_serial_obj());

	return LOGIC_MONITOR->dispatch_to_scene(this, &item_info);
}

int BaseLogicPlayer::request_add_reward(int id, const SerialObj& obj)
{
	Proto31400323 inner;
	inner.set_id(id);
	obj.serialize(inner.mutable_serial());
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int BaseLogicPlayer::request_add_item(const SerialObj& obj,
		int item_id, int amount, int item_bind)
{
	Proto31400043 inner;
	inner.set_item_id(item_id);
	inner.set_amount(amount);
	inner.set_bind(item_bind);
	obj.serialize(inner.mutable_serial_obj());
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner);
}

int BaseLogicPlayer::request_save_mmo_begin(const string& table_name, const BSONObj& query,
		const BSONObj& content, int upgrade_type, int trans_recogn)
{
	return GameCommon::request_save_mmo_begin(table_name, query, content, upgrade_type,
			trans_recogn, this->role_id());
}

int BaseLogicPlayer::announce_with_player(int shout_id, const BrocastParaVec& para_vec)
{
	return 0;
}

int BaseLogicPlayer::set_brocast_role(ProtoBrocastRole* brocast_role)
{
	brocast_role->set_role_id(this->role_id());
	brocast_role->set_role_name(this->name());
	brocast_role->set_team_state(false);
	return 0;
}

int BaseLogicPlayer::sync_restore_event_info(int event_id, int value, int times)
{
	Proto31400401 sync_info;
	sync_info.set_event_id(event_id);
	sync_info.set_value(value);
	sync_info.set_times(times);

	return this->monitor()->dispatch_to_scene(this, &sync_info);
}

int BaseLogicPlayer::sync_to_ml_activity_finish(int activity_type, int sub_type, int value)
{
    Proto31400018 request;
    request.set_activity_type(activity_type);
    request.set_sub_type(sub_type);
    request.set_value(value);

    return this->monitor()->dispatch_to_scene(this, &request);
}

int BaseLogicPlayer::inner_notify_assist_event(int event_id, int event_value)
{
	Proto31401710 inner_event;
	inner_event.set_event_id(event_id);
	inner_event.set_event_value(event_value);
	return LOGIC_MONITOR->dispatch_to_scene(this, &inner_event);
}

void BaseLogicPlayer::record_other_serial(int main_serial, int sub_serial, int value,
		int ext1, int ext2)
{
	SERIAL_RECORD->record_other_serial(SerialPlayer(this->role_id(), this),
			this->role_detail().__agent_code, this->role_detail().__market_code,
			main_serial, sub_serial, value, ext1, ext2);
}

int BaseLogicPlayer::dispatch_to_scene_server(const int scene_id, Message *msg)
{
    return LOGIC_MONITOR->dispatch_to_scene(this, scene_id, msg);
}

int BaseLogicPlayer::dispatch_to_scene_server(const int scene_id, const int recogn)
{
    return LOGIC_MONITOR->dispatch_to_scene(this->gate_sid(), this->role_id(), scene_id, recogn);
}

int BaseLogicPlayer::dispatch_to_map_server(Message *msg)
{
    return this->dispatch_to_scene_server(this->scene_id(), msg);
}

int BaseLogicPlayer::dispatch_to_map_server(const int recogn)
{
    return this->dispatch_to_scene_server(this->scene_id(), recogn);
}

int BaseLogicPlayer::dispatch_to_chat_server(Message *msg)
{
    return LOGIC_MONITOR->dispatch_to_chat(this, msg);
}

int BaseLogicPlayer::dispatch_to_chat_server(const int recogn)
{
    return LOGIC_MONITOR->dispatch_to_chat(this, recogn);
}

void BaseLogicPlayer::refresh_fight_property(const int offset, const IntMap &prop_map, const int enter_type)
{
    Proto30400011 prop_info;
    prop_info.set_offset(offset);
    prop_info.set_enter_type(enter_type);

    for (IntMap::const_iterator iter = prop_map.begin();
            iter != prop_map.end(); ++iter)
    {
        ProtoPairObj *pair_obj = prop_info.add_prop_set();
        pair_obj->set_obj_id(iter->first);
        pair_obj->set_obj_value(iter->second);
    }

    this->dispatch_to_map_server(&prop_info);
}
