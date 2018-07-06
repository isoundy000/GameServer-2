/*
 * SerialRecord.h
 *
 *  Created on: 2013-7-12
 *      Author: root
 */

#ifndef SERIALRECORD_H_
#define SERIALRECORD_H_

#include "PubStruct.h"
//#include "GameCommon.h"

enum SERIAL_RANK_TYPE
{
	SERIAL_RANK_TYPE_WORLD_BOSS 				= 1,	// BOSS no use
	SERIAL_RANK_TYPE_LEAGUE_MARTIAL_PLAYER 		= 3,	// 仙盟武道会, 个人  no use
	SERIAL_RANK_TYPE_FIGHT_FORCE 				= 6,	// 战力
	SERIAL_RANK_TYPE_FIGHT_LEVEL 				= 7,	// 等级
	SERIAL_RANK_TYPE_PET 						= 8,	// 宠物 no use
	SERIAL_RANK_TYPE_MOUNT	 					= 9,	// 坐骑 no use
	SERIAL_RANK_TYPE_LEAGUE						= 10, 	// 仙盟

	SERIAL_RANK_TYPE_SCRIPT_ZYFM 				= 13,	// 副本 斩妖封魔 no use
    SERIAL_RANK_TYPE_SEND_FLOWER 				= 16,   // 送花排行 no use
    SERIAL_RANK_TYPE_RECV_FLOWER 				= 17,   // 收花排行 no use
    SERIAL_RANK_TYPE_KILL_VALUE 				= 18, 	// 恶人榜排行 no use
    SERIAL_RANK_TYPE_HERO 						= 19, 	// 英雄榜排行 no use
    SERIAL_RANK_TYPE_AREA 						= 20,	// 江湖榜

    SERIAL_RANK_FUN_MOUNT	 	 				= 22,	// 骑宠
    SERIAL_RANK_FUN_GOD_SOLIDER	 				= 23,	// 魔刃
    SERIAL_RANK_FUN_MAGIC_EQUIP					= 24,	// 宝器
    SERIAL_RANK_FUN_XIAN_WING		 			= 25,	// 神翼
    SERIAL_RANK_FUN_LING_BEAST		 			= 26,	// 仙童
    SERIAL_RANK_FUN_BEAST_EQUIP	 				= 27,	// 仙武
    SERIAL_RANK_FUN_BEAST_MOUNT	 				= 28,	// 仙兽
    SERIAL_RANK_FUN_BEAST_WING					= 29,	// 灵羽
    SERIAL_RANK_FUN_BEAST_MAO					= 30,	// 灵兽
    SERIAL_RANK_FUN_TIAN_GANG					= 31,	// 元神
	SERIAL_RANK_TYPE_END,

	/*
	 * 所有的参加人数统计类型编号为该值加上对应的排行类型编号
	 * */
	SERIAL_RANK_TYPE_COUNT_BASE					= 1000
};

enum SERIAL_ACTIVITY_TYPE
{
	SERIAL_ACT_SMBATTLE							= 1,	//杀戮战场
	SERIAL_ACT_ANSWER							= 2, 	//答题
	SERIAL_ACT_CHESTS							= 3,	//宝箱
	SERIAL_ACT_ESCORT							= 4,	//护送
	SERIAL_ACT_DAILY_RECHARGE					= 5,	//每日充值
	SERIAL_ACT_REBATE_RECHARGE					= 6,	//百倍返利
	SERIAL_ACT_INVEST_RECHARGE					= 7,	//投资计划
	SERIAL_ACT_TREASURES						= 8,	//旋即宝匣
	SERIAL_ACT_GOLD_BOX							= 9,	//元宝宝匣
	SERIAL_ACT_HOTSPRING						= 10,	//温泉

	SERIAL_ACT_END
};

struct SerialPlayer
{
	SerialPlayer(Int64 id, EntityCommunicate* entity = NULL)
	{
		this->id_ = id;
		this->entity_ = entity;
	}

	Int64 id_;
	EntityCommunicate* entity_;
};

class SerialRecord
{
public:
	SerialRecord();
	~SerialRecord();

	/*
	 * 金钱流水
	 * */
	int record_money(EntityCommunicate* entity, int agent_code, int market_code,
			const SerialObj &serial_obj, const Money &money, const Money &remain_money);

	/*
	 * 物品流水
	 * */
	int record_item(MLPacker* player, int agent_code, int market_code,
			const SerialObj& serial_obj, const ItemObj& item_obj, Int64 src_role_id = 0);

	/*
	 * 玩家等级流水
	 * */
	int record_player_level(EntityCommunicate* entity, int agent_code, int market_code,
			int serial_type, int level);

	/*
	 * 其他流水
	 * */
	int record_other_serial(const SerialPlayer& player, int agent_code, int market_code,
			int main_serail, int sub_serail, Int64 value, Int64 ext1 = 0, Int64 ext2 = 0);

	/*
	 * 装备流水
	 * */
	int record_equipment(EntityCommunicate* entity, int agent_code, int market_code,
			int serial_type, int sub_serial_type, RecordEquipObj& equip_obj);

	/*
	 * 战骑类流水
	 * */
	int record_mount(MapLogicPlayer* player, int agent, int serial, int type);

	/*
	 * 邮件流水
	 * */
	int record_mail_detail(EntityCommunicate* entity, int agent_code, int market_code,
			int serial_type, const MailDetailSerialObj& mail_obj);

	/*
	 * 在线流水
	 * */
	int record_online_users(int agent_code, int market, int users, int hooking_users, int time);

	/*
	 * 登入流水
	 * */
	int record_login_logout(Int64 role_id, string role_name, int level,
			string account, string client_ip, Int64 login_time,
			Int64 logout_time, int sub_agent, int market_code,
			string sys_model, string sys_version, string mac);

	/*
	 * 任务流水
	 * */
	int record_task(EntityCommunicate* entity, int agent_code, int market_code,
			int serial_type, int task_id, int level);

	/*
	 * 排行流水
	 * */
	int record_rank(const PairObj& key, const string& name, int rank_type, int value, Int64 time,
			Int64 ext_int_1 = 0, Int64 ext_int_2 = 0, const string& ext_str_1 = string());

	/*
	 * 聊天流水
	 * */
	int record_chat(Int64 role_id, int serial_type, Int64 time, const string& content,
			int sub_agent, const string &server_flag, int market_code);

	/*
	 * 玩法流水
	 * */
	int record_activity(SERIAL_ACTIVITY_TYPE act_type, int total_attend,
			const PairObj& sub1 = PairObj(0, 0), const PairObj& sub2 = PairObj(0, 0),
			const string& sub3 = string());

	int request_save_serial(Message *data, bool with_table_name, EntityCommunicate* entity = NULL);

private:
	string get_table_name(Int64 role_id, string base_table_name);
};

#define SERIAL_RECORD Singleton<SerialRecord>::instance()

#endif /* SERIALRECORD_H_ */
