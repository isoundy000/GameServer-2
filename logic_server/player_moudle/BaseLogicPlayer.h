/*
 * BaseLogicPlayer.h
 *
 *  Created on: Jul 5, 2013
 *      Author: peizhibi
 */

#ifndef BASELOGICPLAYER_H_
#define BASELOGICPLAYER_H_

#include "LogicStruct.h"
#include "EntityCommunicate.h"

class BaseLogicPlayer : public EntityCommunicate
{
public:
	BaseLogicPlayer();
	virtual ~BaseLogicPlayer();

	virtual LogicPlayer* logic_player();

    static LogicPlayer* find_player(Int64 role_id);
    LogicPlayer* find_player(const std::string& role_name);

    LogicPlayer* validate_find_player(Int64 role_id, int recogn,
    		int errorno = ERROR_PLAYER_OFFLINE);

    bool online_flag(Int64 role_id);

    int request_add_exp(int add_exp, const SerialObj& serial_obj);
    int request_add_savvy(int add_savvy, const SerialObj& serial_obj);
    int request_add_anima(int add_anima, const SerialObj& serial_obj);
    int request_add_money(const Money& money, const SerialObj& serial_obj, const int is_notify = true);
    int request_add_goods(int source_type, const ItemObj& item_obj,
    		const SerialObj& serial_obj);
    int request_add_goods(int source_type, PackageItem &item_obj, const SerialObj& serial_obj);
    int request_remove_goods(PackageItem &item_obj, const SerialObj& serial_obj);

    int request_add_reward(int id, const SerialObj& obj);
    int request_add_item(const SerialObj& obj, int item_id,
    		int amount, int item_bind = false);


    int request_save_mmo_begin(const string& table_name, const BSONObj& query,
    		const BSONObj& content, int upgrade_type = true,
    		int trans_recogn = TRANS_TRADE_MODE_DEFAULT);

    // announce
    int set_brocast_role(ProtoBrocastRole* brocast_role);
    int announce_with_player(int shout_id, const BrocastParaVec& para_vec);

    // 通知资源找回系统事件更新
    int sync_restore_event_info(int event_id,  int value, int times = 0);

    // 通知活动完成事件
    int sync_to_ml_activity_finish(int activity_type, int sub_type = 0, int value = 1);
    int inner_notify_assist_event(int event_id, int event_value);

    void record_other_serial(int main_serial, int sub_serial, int value,
    		int ext1 = 0, int ext2 = 0);

    int dispatch_to_scene_server(const int scene_id, Message *msg);
    int dispatch_to_scene_server(const int scene_id, const int recogn);
    int dispatch_to_map_server(Message *msg);
    int dispatch_to_map_server(const int recogn);
    int dispatch_to_chat_server(Message *msg);
    int dispatch_to_chat_server(const int recogn);

    void refresh_fight_property(const int offset, const IntMap &prop_map, const int enter_type = 0);

public:
    virtual int scene_id() = 0;
    virtual int role_level() = 0;

	virtual Int64 role_id(void) = 0;
    virtual int role_id_low(void) = 0;
    virtual int role_id_high(void) = 0;

	virtual LogicMonitor *monitor(void) = 0;
    virtual const char *account(void) = 0;

    virtual LogicRoleDetail &role_detail(void) = 0;

    virtual bool is_vip(void) = 0;
    virtual bool notify_msg_flag() = 0;

    virtual int vip_type(void) = 0;
    virtual BaseVipDetail& vip_detail(void) = 0;
};

#endif /* BASELOGICPLAYER_H_ */
