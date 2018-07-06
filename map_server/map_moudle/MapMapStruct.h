/*
 * MapMapStruct.h
 *
 *  Created on: Feb 15, 2014
 *      Author: peizhibi
 */

#ifndef MAPMAPSTRUCT_H_
#define MAPMAPSTRUCT_H_

#include "Heap.h"
#include "FightStruct.h"
#include <queue>

class LWarField;
class AreaField;
class LeagueBonfire;

class Proto30400006;
class Proto30400427;
class Proto50400306;
class Proto50400307;
class Proto50400318;
class Proto80400111;
class Proto80400336;
class Proto80400337;
class Proto80400351;
class Proto80400102;
class ProtoTravelTeam;
class ProtoTeamer;
class ProtoQualityRank;
class ProtoMilitaryExploits;
class ProtoSMBattleRankRec;
class ProtoLeagueRankInfo;

//可触发技能队列，按技能使用时间来排序
typedef Heap<FighterSkill, FighterSkillCmp> 	SkillQueue;
typedef std::queue<FighterSkill *> 				SkillSequence;

namespace GameEnum
{
	//>=0为属性类型，< 0为调用类型
	enum UPDATE_FIGHT_PROP_TYPE
	{
		UT_PROP_KILL_VALUE 	= -2,
		UT_PROP_GLAMOUR		= -3,
		UT_PROP_END
	};
}

struct MapRoleDetail : public BaseRoleInfo
{
    // load data from database
    string __league_name;
    int __league_pos;

    std::string __trvl_server_flag;

    int escort_times;		// 运镖次数
	int protect_times;		// 保护次数
	int rob_times;			// 打劫次数

    int fb_flag_;
    int team_flag_;
    int sign_trvl_team_;	// 跨服组队
    int prev_force_;
    int client_type_;		//客户端传送类型
    IntMap prev_force_map_;

    int drop_act_;		//掉落活动ID
    int is_big_act_time_;	//是否处于大型活动期间
    int notify_trvl_chat_;
    int save_trvl_scene_;	//保存的跨服场景id

    IntSet __scene_history;			//进入过场景
    SaveSceneInfo __save_scene;		//保存场景信息

    Time_Value __refresh_free_relive_tick;  // 刷新每天免费复活次数的时间
    int __used_free_relive;

    // 采集礼盒次数
    Time_Value __wedding_giftbox_tick;
    int __wedding_giftbox_times;

    int __collect_chest_amount;      // 活动期间内采集宝箱数
    int __inc_sacredstone_exp;       // 累计获得经验
    int hickty_id_;
    int health_;

    void reset(void);
};

struct LWarRoleInfo
{
	Int64 id_;
	Int64 tick_;

	int sex_;
	string name_;
	Int64 league_index_;
	Int64 leader_index_;
	string league_name_;
	string leader_name_;

	int state_;	// 1:prepare, 2:enter scene, 3:exit scene
	int space_;
	int camp_index_;
	int score_;

	IntMap reward_map_; // 获得的积分奖励
	IntMap record_map_; // 获得的积分奖励记录

	int camp_id();
	void reset();
};

struct LWarLeagueInfo
{
	Int64 league_index_;
	Int64 leader_index_;
	string league_name_;
	string leader_name_;
	Int64 tick_;

	int score_;
	int rank_;
	int space_id_;

	LongMap lwar_player_;			//prepare
	LongMap enter_lwar_player_;		//enter

	void reset();
	void serialize(ProtoLeagueRankInfo *proto);
};

struct LWarLeagueHurt
{
	Int64 league_index_;
	string league_name_;
	Int64 tick_;

	int rank_;

	int hurt_value_;

	void reset();
};

// 上一期帮派争霸排名
struct LeagueWarInfo
{
	int total_num_;	//第几期
	Int64 tick_;

	struct LeagueWarRank
	{
		Int64 league_index_;
		string league_name_;
		string leader_;
		int force_;
		int rank_;
		int score_;
		int flag_lvl_;

		void reset();
		LeagueWarRank();
	};

	typedef std::map<int, LeagueWarRank> LWarRankMap;
	LWarRankMap lwar_rank_map_;

	Int64 find_camp_league(int rank_index);

	void reset();
	LeagueWarInfo();
};

struct MAttackInfo
{
	Int64 id_;
	Int64 tick_;

	int sex_;
	string name_;
	int camp_index_;

	int state_;
	int score_;

	IntMap reward_map_;

	int camp_id();

	MAttackInfo(void);
	void reset(void);
};

struct MAttackLabelRecord
{
	int label_id_;
	int role_sex_;
	Int64 role_id_;
	std::string role_name_;

	MAttackLabelRecord();
	void reset(void);
};

struct WorldBossInfo
{
	int scene_id_;
	int status_;
	Int64 die_tick_;
	double cur_blood_;
	Int64 killer_;
	string killer_name_;

	void reset(void);
};

//巅峰对决队伍信息
struct TravPeakTeam : public BaseServerInfo
{
	enum
	{
		STATE_NONE 		= 0,	//空闲
		STATE_ATTEND	= 1,	//参与战斗
		END
	};

    Int64 __team_id;
    std::string __team_name;
    Int64 __leader_id;

    int __quality_times;// 资格赛过程中挑战的次数
    int __score;		// 该次资格赛获得积分
    int __continue_win;	// 连胜场次
    int __rank;			// 积分排名
    Int64 __update_tick;// 积分更新时间

    int __space_id;   	// 用于查找战斗中的场景对象
    int __camp_id;		// 阵营id
    int __state;		// 状态
    Int64 __last_rival;	// 上次对手
    Int64 __start_tick;
    int __last_add_score; // 最近一次增加的积分
    int __last_reward_id; // 最近一次的奖励id

    struct TravPeakTeamer
    {
        Int64 __teamer_id;
        std::string __teamer_name;
        int __teamer_sex;
        int __teamer_career;
        int __teamer_level;
        int __teamer_force;

        Int64 __offline_teamer_id;
        MapPlayerEx *__offline_player;

        TravPeakTeamer(void);
        void reset(void);
        void serialize(ProtoTeamer *msg);
        void unserialize(const ProtoTeamer &msg);
    };
    typedef std::map<Int64, TravPeakTeamer> TeamerMap;
    TeamerMap __teamer_map;

    void reset(void);
    void serialize(ProtoTravelTeam *msg);
    void serialize_rank(ProtoQualityRank *msg);
    TravPeakTeamer *find_teamer(Int64 role_id);
    int total_force();
    int total_level();
};

// 跨服巅峰对决资格赛信息
struct TravPeakQualityInfo
{
	LongSet __signup_team_set;		// 已报名的战队ID集合
	ThreeObjVec __team_score_vec; 	// 队伍积分排名

	Time_Value __signup_start_tick;
	Time_Value __signup_end_tick;
	Time_Value __quality_start_tick;
	Time_Value __quality_end_tick;

	void reset(void);
};

struct TravPeakKnockoutInfo
{
    Time_Value __knockout_start_tick;
    Time_Value __knockout_end_tick;

    void reset(void);
};

struct SMBattlerInfo
{
	Int64 id_;
	int sex_;
	string name_;

	int state_;	// 1:prepare, 2:enter scene, 3:exit scene
	Int64 tick_;

	int space_;
	int camp_index_;
	int force_;

	int score_;
	int con_kill_num_;
	int total_kill_num_;
	int max_con_kill_num_;
	Int64 score_tick_;
	IntMap reward_map_;

	int camp_id();
	int con_score(int per, int max);

	void reset();
	void make_up_rank(int rank, ProtoSMBattleRankRec* proto);
};

struct SMCampInfo
{
	int rank_;
	int score_;
	Int64 tick_;

	LongMap sm_player_;			//prepare
	LongMap enter_sm_player_;	//enter

	void reset();
	int total_force();
	int total_player();
};

struct MapMagicWeaponInfo
{
    int __weapon_id;
    int __weapon_level;

    void reset(void);
};

struct OfflineRoleDetail
{
	Int64 role_id_;
	MapRoleDetail role_info_;
    MapMagicWeaponInfo magic_weapon_info_;

    IntPair kill_info_;
    IntMap skill_map_;
    IntMap shape_map_;

    double attack_lower_;
    double attack_upper_;
    double defence_lower_;
    double defence_upper_;
    double total_hit_;
    double total_avoid_;
    double total_lucky_;

    double total_damage_;
    double total_reduction_;
    double total_crit_;
    double total_toughness_;
    int total_blood_;
    int total_magic_;
    double total_speed_;
    int cur_label_;
    int equie_refine_lvl_;

    OfflineRoleDetail();
	void reset(void);
};

struct OfflineBeastDetail
{
	Int64 beast_id_;

	int beast_sort_;
	string beast_name_;

	IntMap prop_map_;
	IntVec skill_set_;

	OfflineBeastDetail();
	void reset(void);
};

struct OfflineDetail
{
	int die_times_;
	bool last_is_move_;	// 行为树上一次是移动
	void reset(void);
};

struct BloodContainer
{
	enum
	{
		TIPS_SPAN 	= 300,
		BUFF_ID		= 30201,
		END
	};

	int max_blood_;		// 血池最大血量
	int cur_blood_;		// 当前血池血量
	int check_flag_;
	BasicStatus status_;

	int non_tips_;
	Int64 last_tips_tick_;
	Int64 everyday_tick_;

	int interval_;
	int check_time_;

	void reset(void);
	void copy_to_status();
	void adjust_cur_blood();
	double cur_left_percent();

	int left_capacity();
	int arrive_time_add();
};

struct AutoFighterDetail
{
	enum
	{
		SKILL_MODE_QUEUE 	= 0,
		SKILL_MODE_MAP		= 1,
		SKILL_MODE_BEAST	= 2,
		SKILL_MODE_COPY		= 3,
		SKILL_MOCE_SEQUENCE	= 4,
		END
	};

	//目标ID
	Int64 aim_id_;
	//最近攻击者ID
	Int64 last_attacker_id_;
	BLongSet __monster_history_defender_set_ ;// 记录历史攻击过怪物的对象

	//主人ID
	Int64 owner_id_;
	//目标坐标
	MoverCoord aim_coord_;
	//攻击与追捕交替切换
	int is_attack_;
	//施法距离
	int attack_distance_;
	//仇恨范围
	int select_distance_;

	//调度队列
	TimeValueList schedule_list_;
	//可触发技能队列
    SkillQueue skill_queue_;
    //固定触发顺序的技能队列
    SkillSequence skill_sequence_;

    //分组警界，组ID
    int group_id_;

    //选择技能模式
    int fetch_skill_mode_;

    // 用于定位长方形技能范围的左侧边中点位置
    MoverCoord rect_skill_coord_;

    // 技能引导时间
    Time_Value guide_tick_;
    // 正在引导的技能
    int guide_skill_;
    // 两屏内的特效范围ID
    Int64 guide_range_effect_sort_;

    // 死亡可以调用的技能
    IntVec die_skill_vec_;

    // 播放技能提示语倒数
    IntMap skill_note_countdown_;

	void reset(void);
};

struct LeagueFBDetail
{
	struct BOSSWave
	{
		BOSSWave();

		int wave_id_;
		int wave_state_;
		LongMap boss_map_;
	};

	typedef std::map<int, BOSSWave> BOSSWaveMap;

	Int64 league_index_;
	ScoreInfoMap score_map_;
	BOSSWaveMap boss_wave_map_;

	int fb_index_;
	int fb_state_;

	int league_lvl_;
	int open_mode_;

	int pass_monster_;
	int finish_state_;	//false:fail, true:win

	void reset();
};

struct MapEquipDetail
{
	int weapon_lvl_;

	void reset();
};

struct DynamicMoverCoord
{
public:
    DynamicMoverCoord(const int pixel_x = 0, const int pixel_y = 0);
    void reset(void);

    int dynamic_pos_x(void) const;
    int dynamic_pos_y(void) const;
    int pixel_x(void) const;
    int pixel_y(void) const;

    void set_dynamic_pos(const int posX, const int posY);
    void set_dynamic_pixel(const int pixel_x, const int pixel_y);

    static int dynamic_pos_to_pixel(const int pos, const int grid = GameEnum::DEFAULT_AI_PATH_GRID);
    static int pixel_to_dynamic_pos(const int pixel, const int grid = GameEnum::DEFAULT_AI_PATH_GRID);

private:
    int pixel_x_;
    int pixel_y_;
    int pos_x_;
    int pos_y_;
};
extern bool operator==(const DynamicMoverCoord &left, const DynamicMoverCoord &right);
extern bool operator!=(const DynamicMoverCoord &left, const DynamicMoverCoord &right);
extern bool operator<(const DynamicMoverCoord &left, const DynamicMoverCoord &right);

struct PriorityCoord
{
    int __target_priority;
    int __source_priority;

    DynamicMoverCoord __cur_pos;
    DynamicMoverCoord __prev_pos;

    PriorityCoord(void);
    PriorityCoord(const int target_prio, const int source_prio,
    		const DynamicMoverCoord &cur, const DynamicMoverCoord &prev = DynamicMoverCoord(0,0));
};
extern bool operator<(const PriorityCoord &left, const PriorityCoord &right);

struct PriorityMptCoord
{
    int __target_priority;
    int __source_priority;

    MoverCoord __cur_pos;
    MoverCoord __prev_pos;

    PriorityMptCoord(void);
    PriorityMptCoord(const int targte_prio, const int source_prio,
    		const MoverCoord &cur, const MoverCoord &prev = MoverCoord(0, 0));
};
extern bool operator<(const PriorityMptCoord &left, const PriorityMptCoord &right);

struct HotspringDetail
{
	Int64 double_major_player;
	int player_npc;
	int cur_stage;
	int first_npc;
	int second_npc;
	int third_npc;
	int left_time;
	int auto_double_major;
	int is_right;

	HotspringDetail(){ HotspringDetail::reset(); }
	void reset()
	{
		this->double_major_player = 0;
		this->cur_stage = 0;
		this->first_npc = 0;
		this->second_npc = 0;
		this->third_npc = 0;
		this->left_time = 0;
		this->player_npc = 0;
		this->auto_double_major = 1; //默认开启
		this->is_right = 0;
	}
};

struct HotspringActivityDetail
{
	int cur_stage_;
	int total_stage_;
	int stage_time_;
	int exp_base_;
	int wait_time_;
	int play_time_;
	int cycle_id_;
	int cur_status; //0 wait 1 start
	int cur_answer; // win npc id
	int cur_second;
	int cur_third;
	int win_award_id;
	int comfort_award_id;

	//serial
	LongMap act_player_vip_info;

	HotspringActivityDetail(){ HotspringActivityDetail::reset();}
	void reset()
	{
		this->cur_stage_ = 0;
		this->exp_base_ = 0;
		this->play_time_ = 0;
		this->wait_time_ = 0;
		this->total_stage_ = 0;
		this->stage_time_ = 0;
		this->cur_answer = 0;
		this->cur_second = 0;
		this->cur_third = 0;
		this->cycle_id_ = 0;
		this->cur_status = 0;
		this->win_award_id = 0;
		this->comfort_award_id = 0;

		this->act_player_vip_info.clear();

		this->double_major_list_.clear();
		this->player_answer_list_.clear();
		this->cur_state_answer_list_.clear();
		this->right_player_.clear();
		this->exp_map_.clear();
	}

	LongMap cur_state_answer_list_;
	LongMap player_answer_list_;
	LongMap double_major_list_;
	LongMap right_player_;
	LongMap exp_map_;
};

struct AnswerActivityDetail
{
	struct player_info //玩家答题活动基本信息
	{
		int answer_num;
		int time_num;
		int right_num;
		int score_num;
		int rank_num;
		string name;
		player_info():answer_num(0), time_num(0), right_num(0), score_num(0), rank_num(0)
    	{ /*NULL*/ }
		void reset()
		{
			this->answer_num = 0;
			this->right_num = 0;
			this->score_num = 0;
			this->time_num = 0;
			this->rank_num = 0;
			this->name = "";
		}
	};

	int broad_times_;
	int start_tick_;
	int stop_tick_;
	int left_tick_;

	int cycle_id_;
	int cur_stage_;    //当前轮
	int total_stage_;  //总轮
	int cur_topic_id;

	int wait_time_;
	int answer_time_;

	int cur_year;
	int cur_month;
	int cur_day;

	int a_area[2][2]; // a area coordinate
	int b_area[2][2]; // b area coordinate

	Int64 open_time;
	typedef std::map<Int64, player_info> PlayerInfoMap;

	std::vector<std::pair<Int64 , int> > player_rank_list; //玩家排名
	PlayerInfoMap player_info_list; //所有玩家信息

	IntMap reward_list;

	IntVec total_topic; //question bank
};

struct CollectChestsDetail
{
	struct ChestsItem
	{
		int chest_id_;
		int coord_x_;
		int coord_y_;
		int award_id_;
		int monster_id_;
	};
	int broad_times_;
	int start_tick_;
	int stop_tick_;
	int left_tick_;

	typedef std::map<int, ChestsItem> ChestsMap;

	int cycle_id_;
	int cur_stage_;
	int total_stage_;
	int count_;
	int left_;
	int refresh_;

	int cur_year;
	int cur_month;
	int cur_day;

	LongMap player_num;
	LongMap alive_collect_;
	ChestsMap chest_map_;
};

struct AnswerRoleDetail
{
	Int64 open_time;
	int cur_topic_num;

	AnswerRoleDetail();
	void reset();
};

struct SkillExtInfo
{
	SkillExtInfo(GameFighter* fighter = NULL,
			const MoverCoord& coord = MoverCoord(0, 0))
	{
		this->fighter_ = fighter;
		this->coord_ = coord;

		if (this->coord_.pos_x() == 0
				&& this->coord_.pos_y() == 0)
		{
			this->have_coord_ = false;
		}
		else
		{
			this->have_coord_ = true;
		}
	}

	GameFighter* fighter_;
	MoverCoord coord_;
	bool have_coord_;
};

struct LEscortDetail
{
	struct RankItem
	{
		RankItem();

		int rank_index_;

		Int64 id_;
		string name_;

		Int64 tick_;
		int score_;
		int total_contri_;

		int escort_type_;
		int escort_times_;
		Int64 car_index_;
		Int64 start_tick_;
	};

	typedef std::map<Int64, RankItem> RankMap;
	RankMap league_rank_;
	LongVec league_set_;
};

struct PEscortDetail
{
	struct Item
	{
		Item()
		{
			this->role_id_ = 0;
			this->car_index_ = 0;
			this->escort_type_ = 0;
			this->escort_times_ = 0;
			this->escort_exp_ = 0;
			this->pre_exp_ = 0;
			this->start_tick_ = 0;
			this->protect_map.clear();
			this->protect_id = 0;
			this->till = 0;
			this->target_level = 0;
		}

		Int64 role_id_;
		Int64 car_index_;

		LongSet protect_map;
		Int64 protect_id;
		int escort_type_;
		int escort_times_;
		int escort_exp_;
		int pre_exp_;
		Int64 start_tick_;

		int till;
		int target_level;

	};

	typedef std::map<Int64, Item> ItemMap;

	int cycle_id_;
	ItemMap item_map_;
};


#endif /* MAPMAPSTRUCT_H_ */
