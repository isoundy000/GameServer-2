/*
 * GameCommon.h
 *
 *  Created on: Jun 30, 2013
 *      Author: peizhibi
 */

#ifndef GAMECOMMON_H_
#define GAMECOMMON_H_

#include "PubStruct.h"

class Scene;
class RechargeRankItem;
class RechargeRankExtraAwarder;

class ProtoPairMap;
class Proto30400432;
class Proto31401501;
class Proto80200006;
class ProtoBrocastNewInfo;
class ServerItemRecord;

namespace GameEnum
{
	enum GAME_ITEM_SUB_TYPE
	{
		ITEM_SUB_TYPE_JEWAL 		= 11,	//宝石
		ITEM_SUB_TYPE_END
	};

	enum GAME_SKILL_FUN_TYPE
	{
		SKILL_FUN_PASSIVE			= 1,	//玩家被动属性技能
		SKILL_FUN_MOUNT				= 2,	//战骑类技能
		SKILL_FUN_RAMA				= 3,	//罗摩技能
		SKILL_FUN_SHEN_LUO			= 4,	//神罗天征技能
		SKILL_FUN_TRANSFER			= 5,	//变身技能
		SKILL_FUN_END
	};
}

/*
 * 共用的静态函数
 * */
class GameCommon
{
public:
	/*
	 * null string or empty string
	 * */
	static std::string NullString;

	/*
	 * null item obj
	 * */
	static ItemObj NullItemObj;
	static const uint16_t crc16_table[];

public:
	/*
	 * 通用比较函数
	 * */
	static bool pair_comp_by_asc(const PairObj& first, const PairObj& second);
	static bool pair_comp_by_desc(const PairObj& first, const PairObj& second);

	static bool three_comp_by_asc(const ThreeObj& first, const ThreeObj& second);
	static bool three_comp_by_desc(const ThreeObj& first, const ThreeObj& second);
	static bool three_comp_by_desc_b(const ThreeObj& first, const ThreeObj& second);

	static bool four_comp_by_desc(const FourObj& first, const FourObj& second);

	/*
	 * three max
	 * */
	template <class T>
	static T max(const T& fir_value, const T& sec_value, const T& thi_value)
	{
		T max_value = std::max(fir_value, sec_value);
		max_value = std::max(max_value, thi_value);
		return max_value;
	}

	/*
	 * find first
	 * */
	template <class S, class T>
	static bool find_first(S& data_set, const T& value)
	{
		typename S::iterator iter = std::find(data_set.begin(),
				data_set.end(), value);
		return iter != data_set.end();
	}

	/*
	 * find first
	 * */
	template <class S, class T>
	static IntPair find_first_pair(S& data_set, const T& value)
	{
		typename S::iterator iter = std::find(data_set.begin(),
				data_set.end(), value);

		IntPair pair;
		pair.first = (iter != data_set.end());

		if (pair.first == true)
		{
			pair.second = iter - data_set.begin();
		}

		return pair;
	}

	template <class S, class T>
	static bool remove_first(S& data_set, const T& value)
	{
		for (typename S::iterator iter = data_set.begin();
				iter != data_set.end(); ++iter)
		{
			JUDGE_CONTINUE(*iter == value);
			data_set.erase(iter);
			return true;
		}

		return false;
	}

	/*
	 * replace first, or push back
	 * */
	template <class S, class T>
	static void replace_first_or_push(S& data_set, const T& src_value,
			const T& dest_value)
	{
		for (typename S::iterator iter = data_set.begin();
				iter != data_set.end(); ++iter)
		{
			JUDGE_CONTINUE(*iter == src_value);

			*iter = dest_value;
			return;
		}

		data_set.push_back(dest_value);
	}

	template <class S>
	static void json_to_t_vec(S& data_vec, const Json::Value& conf)
	{
		if (conf.isArray() == true)
		{
			int total = conf.size();
			data_vec.reserve(total);

			for (int i = 0; i < total; ++i)
			{
				data_vec.push_back(conf[i].asInt());
			}
		}
		else
		{
			data_vec.push_back(conf.asInt());
		}
	}

	template <class S>
	static void json_to_t_map(S& data_map, const Json::Value& conf)
	{
		JUDGE_RETURN(conf != Json::Value::null, ;);

		if (conf.isArray() == true)
		{
			int total = conf.size();
			for (int i = 0; i < total; ++i)
			{
				data_map[conf[i].asInt()] = true;
			}
		}
		else
		{
			data_map[conf.asInt()] = true;
		}
	}

	template <class S1, class S2>
	static void t_vec_to_map(S1& data_map, const S2& data_vec)
	{
		for (typename S2::const_iterator iter = data_vec.begin();
				iter != data_vec.end(); ++iter)
		{
			data_map[*iter]= 1;
		}
	}

	template <class S1, class S2>
	static void t_map_to_vec(S1& data_vec, const S2& data_map)
	{
		for (typename S2::const_iterator iter = data_map.begin();
				iter != data_map.end(); ++iter)
		{
			data_vec.push_back(iter->first);
		}
	}

	template <class S>
	static void shuffle_t_vec(S& vec)
	{
		int total_size = vec.size();
		JUDGE_RETURN(total_size > 0, ;);

		for (int i = 0; i < total_size; ++i)
		{
			int index = ::rand() % total_size;
			JUDGE_CONTINUE(i != index);
			std::swap(vec[i], vec[index]);
		}
	}

	/*
	 * 0到total_size - 1,随机排序
	 * */
	static void fetch_rand_index(IntVec& index_set, int total_size);

	static void stl_split(StringVec& str_set, const std::string& des_str,
			const std::string& delim_str = ",");

	static void string_hidden_replace(string& dst_str, const string& find_str);
	static void string_replace(string& dst_str, const string& find_str, const string& rep_str);

	/*
	 * 下一个，如果超过总数，取模
	 * */
	static int mod_next(int cur_value, int total_size);
	static int rand_value(int min_value, int max_value);
	static int rand_value(const Json::Value& range_conf);

	/*
	 * 转换成正整数
	 * */
	static int adjust_positive_integer(int cur, int total);

	static int up_number(double number);
	static int validate_name_length(const string& name, int max_length);
	static int validate_chinese_name_length(const string& name, int max_length);

	static int double_compare(double left, double right);
	static LongPair fetch_map_max_value(const LongMap& long_map);

	static bool is_zero(double number);
	static double div_percent(double number);

	/*
	 * skill detail
	 * */
	static const Json::Value& skill_detail(FighterSkill* fighter_skill);
	static const Json::Value& skill_detail(int skill_id, int skill_lvl);
    static const Json::Value& json_by_index(const Json::Value &json, uint index);
    static const Json::Value& json_by_level(const Json::Value &json, uint level);


    //检查是否是合法战斗者
    static int validate_fighter(GameFighter* fighter, int type = 0);
    //检查自己是否合法攻击者
    static int validate_attacker(GameFighter* fighter, int type = 0);

    static int is_copy_player_skill(FighterSkill* fighter_skill);
	static int validate_skill_usage(GameFighter* fighter, FighterSkill* fighter_skill);
	static int validate_skill_target(FighterSkill* skill, GameFighter* attacker,
			GameFighter* target);

	static int is_self_relation(GameFighter* attacker, GameFighter* target);
	static int is_enemy_relation(GameFighter* attacker, GameFighter* target, Scene* scene);
	static int is_ally_relation(GameFighter* attacker, GameFighter* target, Scene* scene);
	static int is_master_relation(GameFighter* attacker, GameFighter* target);
	static int is_monster_relation(GameFighter* attacker, GameFighter* target, Scene* scene);
	static int is_boss_relation(GameFighter* attacker, GameFighter* target, Scene* scene);

    static bool is_multi_target_skill(FighterSkill* fighter_skill);
    static bool is_base_skill(int skill_type);
    static bool is_jump_skill(int skill_type);
    static bool is_angry_skill(int skill_type);
    static bool is_base_skill(FighterSkill* skill);
    static bool is_jump_skill(FighterSkill* skill);
    static bool is_angry_skill(FighterSkill* skill);

    static bool is_passive_prop_skill(int skill_type);
    static bool is_base_skill(int skill_id, int mover_type);

    static bool is_boss(int monster_sort);
    static bool check_config_value(const string& type_str, uint type_index);

    static void copy_player(OfflineRoleDetail* aim_detail, MapPlayerEx* player);
    static void copy_beast(OfflineBeastDetail* aim_detail, MapBeast* beast);

    static void copy_role_property(OfflineRoleDetail* aim_detail, MapPlayerEx* player);
    static void copy_fight_property(OfflineRoleDetail* aim_detail, MapPlayerEx* player);

    static void copy_player(Proto30400432* trans_info, const OfflineRoleDetail& offline_detail);
    static void copy_beast(Proto30400432* trans_info, const OfflineBeastDetail& offline_detail);

    static int fetch_name_color(int kill_num, int killing_value);
    static int fetch_kill_value(int name_color);

    static int fetch_base_skill(int mover_type, int flag = 1);

    static bool is_operate_activity(int activity_id);
    static bool is_daily_activity(int activity_id);

	/*
	 * array index and config
	 * */
	static int validate_array_index(const Json::Value &array_conf, int index);
	static int adjust_array_index(const Json::Value &array_conf, int index);
	static const Json::Value &fetch_index_config(const Json::Value &array_conf, int index);


	/*
	 * property id to property name
	 * */
	static string fight_prop_name(int prop_id);

	/*
	 * validate time span
	 * */
	static bool validate_time_span(Int64 last_tick, int span_time);

	/*
	 * fetch current tm
	 * */
	static tm fetch_cur_tm();
	static void fetch_cur_tm(tm& cur_tm);

	/*
	 * left time span
	 * */
	static int left_time(Int64 finish_tick);
	static int left_time_span(Int64 last_tick, int span_time);

	/*
	 * 到下一分钟的秒数
	 * */
	static int next_minute();
	/*
	 * 到下一小时的秒数
	 * */
	static int next_hour();
	/*
	 * 到下一天0点的秒数
	 * */
	static int next_day();
	/*
	 * 到下周一0点的秒数
	 * */
	static int next_week();

	/*
	 * 到这个月最后一天0点的秒数
	 * */
	static int next_month();

	/*
	 * 今天0点的tick
	 * */
	static time_t today_zero();
	/*
	 * 这周0点的tick
	 * */
	static time_t week_zero();
	/*
	 * 某天0点的tick
	 * */
	static time_t day_zero(time_t tick);
	/*
	 * 下个月初的tick
	 * */
	static time_t next_month_start_zero();
	/*
	 * 是否今天的Tick
	 * */
	static int is_current_day(Int64 check_tick);
	/*
	 * 两个时间点相差的天数
	 * */
	static int day_interval(Int64 first_tick, Int64 second_tick);
	/*
	 * 天数
	 * */
	static int day_interval(int second, int type = 0);

	/*
	 * Time transform
	 * */
	static int time_to_sec(const Time_Value &tick);
	static int time_to_mdate(const time_t tick);	//当月的天数
	static int time_to_ydate(const time_t tick);	//当年的天数
	static int time_to_date(const time_t tick); 	//公元1970年1月1日至今天数

	static time_t make_time_value(int year, int month, int day, int hour, int minute);
    static void make_time_value(Time_Value &time_value, int year,
    		int month, int day, int hour, int minute);

    static bool is_same_week(const Time_Value &tv1);
	static bool is_same_month(const Time_Value &tv1, const Time_Value &tv2);

	/*
	 * 是否是连续的天数
	 * */
	static int is_continue_day(Int64 check_tick);
	static int is_continue_day(Int64 first_tick, Int64 second_tick);

	static void make_up_reward_items(RewardInfo& reward_info, int reward_id);
	static void make_up_reward_items(RewardInfo& reward_info, const ThreeObjVec& obj_vec);
	static void make_up_reward_items(RewardInfo& reward_info, const Json::Value& reward_json);
	static void make_up_fix_reward_items(RewardInfo& reward_info, const Json::Value& reward_json);
	static int make_up_rand_reward_items(RewardInfo& reward_info, int rand_type,
			const Json::Value& reward_json);
	static int choose_goods_list(RewardInfo& reward_info, const Json::Value& choose_json);
	static int choose_goods_list_b(RewardInfo& reward_info, const Json::Value& choose_json);
	static int check_reward_open_condition(RewardInfo& reward_info, int rand_type,
			const Json::Value& reward_json);

	static void make_up_conf_items(ItemObjVec& items, const Json::Value& json);
	static void make_up_conf_items_b(ItemObjVec& items, const Json::Value& json);
	static void make_up_conf_items(RewardInfo& reward_info, const Json::Value& json);

	static void json_to_int_set(IntSet& i_set, const Json::Value& json);


	/*
	 * 物品对象与数据库对象转换
	 * */
    static void bson_to_item(PackageItem *item, const BSONObj& item_obj);
    static BSONObj item_to_bson(PackageItem *item);

    /*
     * money and bson
     * */
    static void bson_to_money(Money& money, const BSONObj& money_obj);
    static BSONObj money_to_bson(const Money& money);

    /*
     * int map and bson
     * */
    static void map_to_bson(BSONVec& bson_vec, const IntMap& obj_map,
    		bool value_except_zero = false, bool key_include_zero = false);
    static void bson_to_map(IntMap& obj_map, BSONObjIterator iter,
    		bool key_include_zero = false);

    static void map_to_proto(ProtoPairMap* proto_map, const IntMap& obj_map);
    static void proto_to_map(IntMap& obj_map, const ProtoPairMap& proto_map);

    static void vector_to_proto(ProtoPairMap* proto_map, const IntVec& obj_vec);
    static void protot_to_vector(IntVec& obj_vec, const ProtoPairMap& proto_map);

    /*
	 * check money is validate
	 * */
	static bool validate_money(const Money& money);

	/*
	 * check item id is validate
	 * */
	static bool validate_item_id(int item_id);

	/*
	 * 当前随机
	 * */
	static bool validate_cur_rand(int use_rate);
	static bool validate_range_rand(int key, const Json::Value& rand_conf);
	static int rand_list_num(const Json::Value& rand_conf);
	/*
	 * check coordinate is movable
	 * */
	static bool is_movable_coord(int scene_id, const MoverCoord &coord);
	static bool is_movable_coord_no_border(int scene_id, const MoverCoord& coord);
	static bool is_nearby_aim(const IntPair& cur_coord, const IntPair& aim_coord);

	static int fetch_rand_coord(MoverCoord& aim_coord, int scene_id);
	static MoverCoord fetch_rand_conf_pos(const Json::Value& pos_conf);
	static void fetch_rang_coord(LongSet& coord_set, const MoverCoord& location, int range = 1);

	static int fetch_area_id_by_coord(const int scene_id, const MoverCoord &coord);

	/*
	 * check mpt value is movable
	 * */
	static bool is_movable_mpt(int mpt_value);

    static bool is_normal_scene(const int scene_id);
    static bool is_travel_scene(const int scene_id);
    static bool is_script_scene(const int scene_id);
    static bool is_world_boss_scene(const int scene_id);
    static bool is_trvl_wboss_scene(const int scene_id);
    static bool is_hiddien_beast_model_scene(const int scene_id);

    static bool is_arr_scene_level_limit(const int scene_id, const int level);
    static bool is_value_in_config(const Json::Value& value_set, int value);

    static int fetch_skill_sex_bit(int skill);
    static int fetch_skill_fun_bit(int skill);
    static int fetch_skill_fun_type(int fun_id);
    static int fetch_skill_id_fun_type(int skill);

    /*
     * mover type
     * */
    static int fetch_mover_type(int mover_lower_id);

	/*
	 * game page size, the min page is 1
	 * */
	static int game_page_size(int total_size, int page_count);

	/*
	 * calculate page
	 * */
	static void game_page_info(PageInfo& page_info, int page_index,
			int total_count, int page_size);


	/*
	 * shop mode
	 * */
	static DBShopMode* pop_shop_mode(int trans_recogn = 0);

	/*
	 * 玩家性别
	 * */
	static bool is_male(int sex_type);
	static bool is_female(int sex_type);

	static bool is_max_level(int level);
	static bool is_rotary_table_goods(int item_id);

	static void set_rotary_table_index(PackageItem* pack_item);

	/*
	 * 物品是否绑定
	 * */
	static bool item_is_bind(int item_id);
	static bool item_is_bind(PackageItem* pack_item);
	static bool check_item_effect_value(PackageItem* pack_item, int check_type);

	/*
	 * 物品是否是装备
	 * */
	static bool item_is_equipment(int item_id);

	/*
	 * 物品能否叠放
	 * */
	static bool item_can_overlap(int item_id);
	static int item_overlap_count(int item_id);
	/*
	 * 物品是否能出售
	 * */
	static bool item_can_onsell(PackageItem* pack_item);
	/*
	 * 获取物品类型
	 * */
	static int fetch_item_type(int item_id);

	static std::string item_name(int item_id);
	static std::string item_name_color(int item_id);
	static std::string make_up_color_name(int color_id);

    /*
     * 计算道具花费
     * */
    static Money item_cost(const int item_id, const int item_amount, const int money_type = 0);

	/*
	 * 获取称号类型 ：1限时称号，0永久称号
	 * */
	static int fetch_label_type(int label_id);

	/*
	 * 先判断json的field_name是否存在
	 * */
	static int json_value(const Json::Value& json_value,
			const std::string& field_name);
	/*
	 * 通过一个 int 类型的 key 查找 json 值， 返回找到的 json
	 * */
	static const Json::Value& json_value_by_int_key(const Json::Value& json_value,
			const int& key);
	/*
	 * 物品
	 * */
	static ItemObj make_up_itemobj(const Json::Value& conf, int times = 1);
	static ItemObj make_up_itemobj_b(const Json::Value& conf);

	/*
	 * 先判断json是否有money_type字段
	 * */
	static Money make_up_money(int amount, const Json::Value& json_value);
	static Money make_up_money(int amount, int money_type);

	static bool validate_money_type(int money_type);
	static void make_up_money_times(Money& money, int add_times);

	/*
	 * 根据已的Money，调整需要的Money(绑定不足时自动使用非绑定)
	 * */
	static void adjust_money(Money& need_money, const Money& own_money,
			int adjust_type = GameEnum::MONEY_ADJUST_DEFAUL);

	static void adjust_copper(Money& need_money, const Money& own_money);
	static void adjust_gold(Money& need_money, const Money& own_money);

	/*
	 * 判断是否需要使用元宝补充铜钱/绑铜
	 *
	 * 返回值为1，需要
     * 返回值为0，不需要
	 * */
	static int check_money_adjust_with_gold(Money& need_money, const Money& own_money);
	static int calc_pack_money_exchange_rate(int money_type);
	static int calc_pack_money_exchange_need_gold(Money& need_money, const Money& own_money, int money_rate);

	static int commercial_recogn_type(int recogn);
    static int check_copper_adjust_exchange_gold(const Money &need_money, const Money &own_money, int *use_gold, int *buy_bind_copper);


	/*
	 * 当前分秒数
	 * */
	static int fetch_cur_min_sec();

	/*
	 * 当前时秒数
	 * */
	static int fetch_cur_hour_sec();

	/*
	 * 当前类型的秒数
	 * */
	static int fetch_cur_sec(int type);
	static int fetch_cur_combine_time();

	/*
	 * 当前天的秒数
	 * */
	static int fetch_cur_day_sec();
	/*
	 * 格式：1930表示19点30分
	 * */
	static int fetch_day_sec(int day_time);

	/*
	 * 当前周的秒数
	 * */
	static int fetch_cur_week_sec();
	/*
	 * 格式：10910表示周一九点十分
	 * */
	static int fetch_week_sec(int week_time);

	/*
	 * 将double转为Time_Value
	 * */
	static Time_Value fetch_time_value(double time_tick);
	static Time_Value fetch_add_time_value(double time_tick);

	static int cal_activity_info(ActivityTimeInfo& time_info,
			const Json::Value& activity_json);
	static void cal_one_activity_time(ActivityTimeInfo& time_info,
			const Json::Value& activity_json, int type = 0);
	static void cal_two_activity_time(ActivityTimeInfo& time_info);
	static void cal_three_activity_time(ActivityTimeInfo& time_info);

	/*
	 * broadcast
	 * */
	static void make_up_chat_broadcast_info(Proto80200006* chat_info,
			const ProtoBrocastNewInfo* brocast_info);

	static int make_up_session(ProtoSession* proto_session, const string& account);


	/*
	 * new broadcast
	 * */
	static void announce(const ShoutInfo& shout, BrocastParaVec* para_vec = NULL, int scene_id = 0);
	static void announce(Int64 group_id, const ShoutInfo& shout, BrocastParaVec* para_vec = NULL);
	static void trvl_announce(const ShoutInfo& shout, BrocastParaVec* para_vec = NULL);
	static void make_up_broadcast_info(ProtoBrocastNewInfo* brocast_info,
			int shout_id, const BrocastParaVec& para_vec);

	static void push_brocast_para_int(BrocastParaVec& para_vec, int single_value);
	static void push_brocast_para_int64(BrocastParaVec& para_vec, Int64 single_value);
	static void push_brocast_para_item_name(BrocastParaVec& para_vec, int item_id);

	static void push_brocast_para_reward(BrocastParaVec& para_vec, RewardInfo *reward_info);
	static void push_brocast_para_reward_id(BrocastParaVec& para_vec, int reward_id);
	static void push_brocast_para_item_info(BrocastParaVec& para_vec, const int item_id,
				const int item_bind = GameEnum::ITEM_NO_BIND, const int item_amount = 1);

	static void push_brocast_para_skill(BrocastParaVec& para_vec, int skill_id);
	static void push_brocast_para_string(BrocastParaVec& para_vec, const std::string& content);


	/*
	 * 不受限制的数据库实例
	 * */
	static MongoConnector* mongodb_connection(pthread_t tid = ::pthread_self());

	/*
	 * 加载
	 * */
	static int db_load_mode_begin(int recogn, BaseUnit* base_unit,
			Int64 role_id = 0);
	static int db_load_mode_begin(DBShopMode* shop_mode, BaseUnit* base_unit,
			Int64 role_id = 0);

	/*
	 * save role field info
	 * */
	static void request_save_role_int(Int64 role_index,
			const string& field_name, int value);
	static void request_save_role_long(Int64 role_index,
			const string& field_name, Int64 value);

	/*
	 * save mongo db
	 * */
    static int request_save_mmo_begin(const string& table_name, const BSONObj& query,
    		const BSONObj& content, int upgrade_type = true,
    		int trans_recogn = TRANS_TRADE_MODE_DEFAULT, long index = 0);

    /*
     * remove mongo db
     * */
    static int request_remove_mmo_begin(const string& table_name, const BSONObj& query,
    		int just_one = true, int trans_recogn = TRANS_TRADE_MODE_REMOVE, long index = 0);


    /*
     * request save mail
     * */
    static int request_save_sys_mail(Int64 role_id, int mail_id);
    static int request_save_mail_content(Int64 role_id, MailInformation* mail_info,
    		int push_pool = true);
    static int request_save_mail(Int64 role_id, MailInformation* mail_info,
    		int push_pool = true);
    static int request_save_mail(MailInformation* mail_info, int push_pool = true);
    static BSONObj mail_info_to_bson(MailInformation *mail_info);

    /*
     * create mail from pool
     * */
    static MailInformation* create_base_mail(const string& mail_title,
    		const string& mail_content, int mail_type, int mail_format);
    static MailInformation* create_pri_mail(const string& mail_title,
    		const string& mail_content);
    static MailInformation* create_sys_mail(const string& mail_title,
    		const string& mail_content, int format);

    static MailInformation* create_sys_mail(const FontPair& font_pair, int format);
    static MailInformation* create_sys_mail(int font_index);

    // 参数: 玩家所在坐标, 区域的 json, "area":[[0,0], [100,100]]
    static bool mover_in_area_json(const MoverCoord &coord, const Json::Value& area_arr_json);
    // 参数: 玩家所在坐标, 区域的左上角 x, y, 右下角 x, y (像素)
    static bool mover_in_area(const MoverCoord &coord, int lt_x, int lt_y, int rb_x, int rb_y);

    /// map进程使用 活动状态提示
    static int map_sync_activity_tips_no_start(int event_id, int sub_value = 0);
    static int map_sync_activity_tips_ahead(const PairObj& event, int left_time, int sub_value = 0);
    static int map_sync_activity_tips_start(const PairObj& event, int end_time, int sub_value = 0);
    static int map_sync_activity_tips_stop(const PairObj& event, int left_times = 0, int sub_value = 0);
    static int map_sync_activity_tips_next(int event_id, int left_times, int sub_value = 0);

    static int script_vip_extra_use_times(int vip_type, int script_sort);

    static IntPair to_int_number(const string& str);

    static char* string_to_uper_case(char* string);
    static char* string_to_lower_case(char* string);

	inline static uint16_t crc16_byte(uint16_t crc, const uint8_t data);
	static uint16_t crc16(uint16_t crc, uint8_t const *buffer, size_t len);

	static void fill_rpm_role_info(ReplacementRoleInfo &info, const BSONObj &bson);
	static int rand_mount_level(string str_info, const Json::Value &info_json);

	static int int2str(const int i, std::string &str);
	static const std::string int2str(const int i);
	static Int64 str2long(const char *nptr);

	/*
	 * 返回 json 中所有小于等于 value 的 key
	 * */
	static Json::Value::Members le_json_keys(const int value, const Json::Value&);

	static int get_item_by_money_type(int money_item);

	static bool is_money_item(int item_id);
	static bool is_resource_item(int item_id);
	static Money money_item_to_money(const ItemObj &item);
	static Money money_item_to_money(int money_item, int amount, int times);

	static std::string monster_name(int monster_sort);
    static std::string scene_name(const int scene_id);

	/*
	 * 过滤 sql 特殊字符
	 * 1. 将 \ 转为 \\
	 * 2. 将 ' 转为 \'
	 * */
	static std::string sql_escape(const std::string &r);

	static uint format_ip_string_to_int(const std::string& ip_addr);
	static Int64 format_mac_string_to_int64(const std::string& mac);
	static Int64 hex_to_decimal(const char* szHex,int len);
	static int hex_char_to_int(char c);

	static IntVec random_int_vector_from_config(const Json::Value list_json, bool is_sub=false);
	static IntVec random_int_vector_from_json_vc(std::vector<Json::Value> &json_vc);

	static bool json_int_array_exists(const Json::Value array_json, int id);

    /*
     * 活动时间类型
     */
    static void activity_time_tick_by_time(int *begin_tick, int *end_tick, const Json::Value &json);
    static int activity_time_tick(const int type, const Json::Value &json);

    //初始化战力属性值
    static void init_property_map(IntMap& prop_map);
    //战力属性的增加方式
    static void add_prop_attr(IntMap& prop_map, int prop_id, int value);

    static int check_welfare_open_condition(const std::string &item_name, const int role_level, const int create_day);

    static bool is_advance_script(int script_id);
    static bool is_exp_script(int script_id);

    static void output_item_info(const ProtoItem& proto_item);
    static void output_item_info(PackageItem* item);

    static int qq_fetch_agent_vip(int agent_code);
    static int reset_prop_map(IntMap& prop_map);

    // server flag to id
    static int is_combine_server_flag(const string& server_flag);
    static int server_flag_to_server_id(const string &server_flag);

    static int update_49_qq(LongMap&,const std::string&);

    //这里随机的是位置,每个位置的概率不同
    static int rand_by_chance(std::vector<int>& multiple, int rand_count = 1);

    //param1:总金额 param2:派发个数 param3:最小保低金额 0.01 * 100
    static IntVec rand_red_packet(int total_money, int dispatch_count, int min_money);

    static int cal_day_time(int time_point);


    static bool comp_by_time_desc(const ServerItemRecord &first, const ServerItemRecord &second);

};

// 此处的数据库对象不能在逻辑线程里调用，会导致逻辑线程会间歇性的卡
#define CACHED_INSTANCE   GameCommon::mongodb_connection()
// 此处的数据库对象不能在逻辑线程里调用，会导致逻辑线程会间歇性的卡
#define CACHED_CONNECTION GameCommon::mongodb_connection()->conection()

#endif /* GAMECOMMON_H_ */
