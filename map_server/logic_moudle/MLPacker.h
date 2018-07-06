/*
 * MLPacker.h
 *
 *  Created on: Jul 5, 2013
 *      Author: peizhibi
 */

#ifndef MLPACKER_H_
#define MLPACKER_H_

#include "MapLogicStruct.h"
#include "GamePackage.h"

class LogicOnline;
class Proto51400101;
class Proto11400136;
class MapLogicSerial;

/*
 * MLPacker: Map Logic Packer
 * */
class MLPacker : virtual public EntityCommunicate
{
public:
	MLPacker();
	virtual ~MLPacker();

	MapLogicPlayer* map_logic_player();

	// database
    int request_save_mmo_begin(const string& table_name, const BSONObj& query,
    		const BSONObj& content, int upgrade_type = true,
    		int trans_recogn = TRANS_TRADE_MODE_DEFAULT);
    int request_remove_mmo_begin(const string& table_name, const BSONObj& query,
    		int just_one = true, int trans_recogn = TRANS_TRADE_MODE_REMOVE);

    // send to map thread
    int send_to_map_thread(int recogn);
	int send_to_map_thread(Message& msg);
	int send_to_map_thread(int cmd, Block_Buffer* buf);

	// send to other scene logic thread
	int send_to_other_logic_thread(int scene_id, Message& msg);

	// enter_type用于进入场景时指定ENTER_SCENE_TRANSFER, 否则进入场景时会不满血
	void refresh_fight_property(int offset, const IntMap& prop_map, const SubObj& obj = SubObj());
	// 此接口不能用来刷血量，可能会出现进入场景时不满血
	void refresh_fight_property(int offset, int prop_type, int prop_value);

	// other
    void test_add_pack_item();
    void sync_today_consume_gold(int consum_gold, int consum_bind_gold);

    int handle_operate_result(int recogn, int result);
    int request_map_use_goods(PackageItem* pack_item, int &use_num);

    int request_direct_add_blood(int add_blood);
    int request_direct_add_magic(int add_magic);
    int request_use_lucky_prop(PackageItem* pack_item, int& use_num);
    int process_kill_reduce_lucky(Message *msg);

    int request_level_up_player(int type);
    int request_add_exp_per_buff(int id, int percent, int last);
    int request_add_exp(int add_exp, const SerialObj &serial_obj);

public:
    // Package Message
    int fetch_pack_info(Message* msg);

    int fetch_rotary_table_result(Message* msg);
    int use_rotary_table_goods(Message* msg);
    int tip_pack_goods(Message* msg);	 // 去除new标签

    int role_create_days();
    int is_arrive_max_level(int lvl_up_num);

    //7天登录
    int fetch_login_reward_flag();
    int fetch_login_reward_info();
    int draw_login_reward(Message* msg);
    int login_reward_red_point();

    //开服豪礼
    int fetch_open_gift_info();
    int draw_open_gift_reward(Message* msg);
    bool check_is_open_gift_day();
    int fetch_open_gift_left_tick();
    int check_send_open_gift_mail();
    int check_is_in_travel_scene();
    int test_reset_open_gift();

    //检测是否需要删除的物品
    int check_del_item(RewardInfo &reward_info);

    //红装碎片兑换
    int red_clothes_exchange(Message* msg);
    //传说/密卷兑换
    int request_exchange(Message* msg);
    int exchange_detail(Proto11400136 *request, const Json::Value& exchange_json);

    int open_pack_grid(Message* msg);
    int pack_sort_and_merge(Message* msg);

    int pack_move(Message* msg);
    int fetch_item_detail_info(Message* msg);

    int drop_goods(Message* msg);
    int add_gift_pack_money(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_fix_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_resp_gif_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_total_gift_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int open_box_by_key(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_rand_gif_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);
    int use_magic_act_pack(const Json::Value& effect, PackageItem* pack_item, const int amount);

    int notify_gift_pack_show(const ItemObjMap& items_map);
    int notify_item_map_tips(const ItemObjMap& items_map);
    int notify_pack_info(int pack_type = GameEnum::INDEX_PACKAGE);
    int make_up_other_equip_pack(GamePackage* package, Proto51400101* proto);

    int check_has_grid_open(void);
    int recharge(int gold, int serial_type = ADD_FROM_BACK_RECHARGE);
    int recharge_mail();
    int recharge_info_update(int gold = 0);
    int act_serial_info_update(int type, Int64 id, Int64 value, int sub_1 = 0, int sub_2 = 0);

    int pack_money_exchange_with_gold(Message* msg);
    int notify_lack_gold_suguest(const int gold, const int money_type);
    
    int notify_exchange_bind_copper(const int need_gold, const int exchange_copper, const int recogn);
    int bind_copper_exchange_or_notify(const Money &need_money, int recogn, const Message *req = 0, const bool force_auto_buy = false);
    int bind_copper_exchange_or_notify_i(Money &need_money, int recogn, bool force=false);
    int exchange_gold_to_bind_copper(const int gold);
    int calc_item_need_money(const int item_id, int &item_amount, Money &cost);

    int request_rename_role_begin(Message* msg);
    int request_rename_role_done(DBShopMode* shop_mode);

    int request_rename_league_begin(Message* msg);
    int request_rename_league_done(Message* msg);

    int request_resex_role_begin(Message* msg);
    int request_resex_role_done(Message* msg);

    int uprising_equip_item(int equip_id, int serial);
    int check_uprising_equip_item(int equip_id);
    int skill_upgrade_use_goods(uint id, uint level, int check = false);

    int add_reward(Message* msg);
    int add_reward(int id, const SerialObj& obj, int need_player = false);

    int add_game_resource(Message* msg);
    int add_mult_item(Message* msg);
    int add_single_item(Message* msg);

    int request_world_boss_enter(Message* msg);

    int update_labour_task_info(int type, int value);
public:
    // 清空背包
    void clean_all_pack();
    void clean_pack(const int pack_type = GameEnum::INDEX_PACKAGE);

    // 背包插入
    /*
     * 最常用,最好用背包插入
     * */
    int insert_package(const SerialObj& serial, int item_id,
    		int item_amount = 1, int item_bind = false, Int64 src_role = 0);
    int insert_package(const SerialObj& serial, const RewardInfo& reward_info,
    		Int64 src_role = 0);
    int insert_package(const SerialObj& serial, const ItemObjVec& item_obj_vec,
    		Int64 src_role = 0);

    /*
     * 常用背包道具插入
     * */
    int pack_insert(const SerialObj& serial, const ItemObj& item_obj,
    		int pack_type = GameEnum::INDEX_PACKAGE, Int64 src_role = 0);
    int pack_insert(const SerialObj& serial, const ProtoItem& proto_item,
    		int pack_type = GameEnum::INDEX_PACKAGE, Int64 src_role = 0);
    int pack_insert(const SerialObj& serial, PackageItem* pack_item,
     		int pack_type = GameEnum::INDEX_PACKAGE, Int64 src_role = 0); //自动寻找空格子

    int pack_insert(GamePackage* package, const SerialObj& serial,
    		const ItemObj& item_obj, Int64 src_role = 0);
    int pack_insert(GamePackage* package, const SerialObj& serial,
    		const ItemObjVec& item_obj_vec, Int64 src_role = 0);
    int pack_insert(GamePackage* package, const SerialObj& serial,
    		PackageItem* pack_item, Int64 src_role = 0);	//自动寻找空格子

    /*
     * 实际道具插入
     * */
    int pack_insert(GamePackage* package, const SerialObj& serial,
    		const RewardInfo& reward_info, Int64 src_role = 0);

    int pack_insert_with_notify(GamePackage* package, PackageItem* pack_item);
    int pack_insert_with_notify(GamePackage* package, int item_index, PackageItem* pack_item);

    int pack_insert_result(GamePackage* package, const SerialObj& serial, Int64 src_role = 0);
    int pack_insert_res_serial(GamePackage* package, const SerialObj& serial, Int64 src_role = 0);

    int pack_insert_res_notify(GamePackage* package);
    int pack_insert_res_task_listen(GamePackage* package);
    int pack_insert_res_tips(GamePackage* package, const SerialObj& serial);
    int pack_insert_res_other(GamePackage* package, const SerialObj& serial);
    int pack_insert_res_auto_use(GamePackage* package);

    // 背包删除
    /*
     * 如果知道索引，最常用背包删除
     * */
    int pack_remove_by_index(const SerialObj& serial, const ItemIndexObj& item_obj,
    		int pack_type = GameEnum::INDEX_PACKAGE, Int64 des_role = 0);
    /*
     * 如果不知道索引,最常用背包删除
     * */
    int pack_remove(const SerialObj& serial, int item_id, int item_amount,
    		int pack_type = GameEnum::INDEX_PACKAGE, Int64 des_role = 0);
    int pack_remove(const SerialObj& serial, PackageItem* pack_item,
    		int item_amount, int pack_type = GameEnum::INDEX_PACKAGE, Int64 des_role = 0);
    int pack_remove(GamePackage* package, const SerialObj& serial,
    		PackageItem* pack_item,	int item_amount, Int64 des_role = 0);
    int pack_remove(GamePackage* package, const SerialObj& serial,
    		int item_id, int item_amount, Int64 des_role = 0);
    int pack_remove(GamePackage* package, const SerialObj& serial,
    		const ItemObjVec& item_vec, Int64 des_role = 0);
    int pack_remove(GamePackage* package, const SerialObj& serial,
    		PackageItem* pack_item, Int64 des_role = 0);
    int pack_remove_with_notify(GamePackage* package, PackageItem* pack_item);

    int pack_remove_result(GamePackage* package, const SerialObj& serial, Int64 des_role = 0);
    int pack_remove_serial(GamePackage* package, const SerialObj& serial, Int64 des_role = 0);

    int pack_remove_notify(GamePackage* package);
    int pack_remove_notify(PackageItem* pack_item, int pack_type = GameEnum::INDEX_PACKAGE);
    int pack_remove_notify(int item_index, int item_amount,
    		int pack_type = GameEnum::INDEX_PACKAGE);

    // 背包其他操作
    int pack_check(GamePackage* package, const ItemObjVec& item_vec);
    int pack_count(GamePackage* package, int item_id);
	int pack_count(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
	int pack_bind_count(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
	int pack_unbind_count(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
    int pack_left_capacity(int pack_type = GameEnum::INDEX_PACKAGE);
    int pack_fetch_item_id(int index, int pack_type = GameEnum::INDEX_PACKAGE);

    int pack_transaction(const SerialObj& remove_serial, const ItemObj& remove_item, Int64 des_role,
    		const SerialObj& add_serial, const ItemObj& add_item, Int64 src_role);
    int pack_buy(const SerialObj& money_serial, Money& money,
    		const SerialObj& add_serial, const ItemObj& add_item);

    PackageItem* pack_find(int item_index, int pack_type = GameEnum::INDEX_PACKAGE);
    PackageItem* pack_find_first(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
    PackageItem* pack_find_by_unbind_id(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
    PackageItem* pack_find_by_bind_id(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);
    PackageItem* pack_find_by_id(int item_id, int pack_type = GameEnum::INDEX_PACKAGE);

    // 背包之间移动物品
    int pack_move_between_package(int src_index, int src_pkg_type,
    		int des_index, int des_pkg_type,
    		int item_serial_type, int equip_serial_type = 0);

    int validate_pack_amount(int id, int need_amount, int pack_type = GameEnum::INDEX_PACKAGE);

public:
    // 金钱
    int pack_money_add(const Money& money, const SerialObj& serial, int is_notify = true);
    int pack_money_add(const ProtoMoney& proto_money, const SerialObj& serial);
    int pack_money_add(const ProtoMoney& proto_money, const ProtoSerialObj& proto_serial, int is_notify = true);
    // 增加金钱，出错时回滚
    int pack_money_item_add(const ItemObjVec& item_obj_set, const SerialObj& serial);
    int pack_money_sub(const Money& money, const SerialObj& serial);
    int pack_money_serail(const Money& money, const SerialObj& serial);

    int notif_pack_money(int type = 0);
    int pack_money_set(const Money& money);

    Money& own_money();

    bool validate_money(const Money& money);
    bool pack_money_reset(const Money& set_money, const Money& cur_money,
    		const SerialObj& serial_obj);

public:
    int client_auto_use_item(GamePackage* package, int pkg_index);
    int check_auto_use_item(GamePackage* package, int pkg_index);
    int check_auto_goods_item(PackageItem* item);
    int check_auto_equip_item(PackageItem* item);

    int validate_game_resource(int item_id, int value);
    int validate_game_resource(const IntMap& need_resource);

    int add_game_resource(int item_id, int value, const SerialObj& obj);
    int add_game_resource(const IntMap& add_resource, const SerialObj& obj);

    int sub_game_resource(int item_id, int value, const SerialObj& obj);
    int sub_game_resource(const IntMap& sub_resource, const SerialObj& obj);

    int fetch_game_resource(int item_id);

    // 巅峰战功勋值
    int exploit(void);
    int exploit_sub(const int exploit, const SerialObj &serial_obj);

    // 帮贡
    int dispatch_to_logic_league(const int item_id, const int add_num);

    // 声望值
    int reputation(void);
    int change_reputation(const int reputation, const SerialObj &serial_obj);

    // 荣誉值
    int honour(void);
    int change_honour(const int honour, const SerialObj &serial_obj);

    //灵气
    int reiki(void);
    int change_reiki(const int reiki_num, const SerialObj &serial_obj);

    //精华
    int spirit(void);
    int change_spirit(const int spirit_num, const SerialObj &serial_obj);

    // 历练
    int practice(void);
    int change_practice(const int practice, const SerialObj &serial_obj);

public:
    // 流水
    int record_item_serial(const SerialObj& serial, const ItemObj& item,
    		Int64 src_role = 0);
    int record_other_serial(int main_serial, int sub_serial, Int64 value,
    		Int64 ext1 = 0, Int64 ext2 = 0);

    // 记录装备流水
    void record_equip_item(PackageItem* pack_item, int serial_type);

    // 道具传闻
    void pack_shout_item(const RewardInfo& reward_info);

public:
    virtual int scene_id(void) = 0;
	virtual int agent_code(void) = 0;
	virtual int market_code(void) = 0;
    virtual MapLogicRoleDetail &role_detail(void) = 0;

	virtual Int64 role_id(void) = 0;
	virtual string &role_name(void) = 0;

	virtual int role_level(void) = 0;
	virtual int role_career(void) = 0;
    virtual int total_recharge_gold() = 0;

	virtual MapMonitor* monitor(void) = 0;
	virtual GamePackage* find_package(int pack_type = GameEnum::INDEX_PACKAGE) = 0;

	virtual LogicOnline &online(void) = 0;
    virtual GameCache &cache_tick(void) = 0;

    virtual PackageDetail &pack_detail(void) = 0;
    virtual MapLogicSerial &serial_record(void) = 0;

    virtual bool is_vip(void) = 0;
    virtual int vip_type(void) = 0;
    virtual MLVipDetail& vip_detail(void) = 0;
    virtual int callback_after_exchange_copper(const int recogn) = 0;
    virtual void set_repeat_req(const int recogn, const Message *req) = 0;
};

#endif /* MLPACKER_H_ */
