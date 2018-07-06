/*
 * ScriptStruct.h
 *
 * Created on: 2013-12-13 14:33
 *     Author: lyz
 */

#ifndef _SCRIPTSTRUCT_H_
#define _SCRIPTSTRUCT_H_

#include "HashMap.h"
#include "PubStruct.h"
#include "DoubleKeyMap.h"

class ProtoLegendTopRank;

namespace GameEnum
{
    enum SCRIPT_SCENE_FLAG
    {
        SCRIPT_SF_STARTED_SCENE  = 0,	//启动副本
        SCRIPT_SF_FINISH_SCENE,
        SCRIPT_SF_FAILURE_SCENE,
        SCRIPT_SF_FINISH_SCRIPT,	//副本成功
        SCRIPT_SF_FAILURE_SCRIPT,	//副本失败

        SCRIPT_SF_USE_TICK_SCENE,		//场景用时
        SCRIPT_SF_PREPARING_SCENE,		//准备
        SCRIPT_SF_ONGOING_SCENE,		//正在进行

        SCRIPT_SF_STARTED_GEN_MONSTER,  // 是否已启动生成怪的定时器
        SCRIPT_SF_KILL_ALL,
        SCRIPT_SF_ALL_RELIVE,			// 玩家不死
        SCRIPT_SF_KILL_MONSTER,
        SCRIPT_SF_CHAPTER_MONSTER,
        SCRIPT_SF_COLLECT_TEXT,         // 收集诗句文字
        SCRIPT_SF_COUPLES,				// 夫妻副本

        SCRIPT_SF_EVENCUT,  // 连斩BUFF控制
        SCRIPT_SF_WAVE,     // 波数控制
        SCRIPT_SF_PROTECT_NPC,  // 保护NPC

        SCRIPT_SF_END
    };

    enum SCRIPT_TYPE
    {
        SCRIPT_T_EXP    		= 1,    // 经验副本
        SCRIPT_T_STORY			= 2,	// 剧情副本
        SCRIPT_T_RAMA			= 3,    // 罗摩副本
        SCRIPT_T_NEW_ADVANCE	= 4, 	// 新手进阶副本
        SCRIPT_T_NEW_STORY		= 5,    // 新手剧情副本
        SCRIPT_T_SWORD_TOP     	= 6,    // 论剑武林副本
        SCRIPT_T_COUPLES        = 7,    // 夫妻副本
        SCRIPT_T_SEAL           = 8,    // 封印BOSS副本
        SCRIPT_T_LEAGUE_FB		= 9,	// 帮派副本
        SCRIPT_T_MONSTER_TOWER	= 10,	// 镇魔塔副本
		SCRIPT_T_LEGEND_TOP 	= 12,	// 问鼎江湖
		SCRIPT_T_ADVANCE		= 13, 	// 进阶副本
		SCRIPT_T_VIP 			= 14,	// VIP副本
		SCRIPT_T_TRVL			= 15,	// 跨服副本

        SCRIPT_T_CLIMB_TOWER	= 100,  // 爬塔副本
        SCRIPT_T_UPCLASS		= 101,  // 寒冰炼狱副本
        SCRIPT_T_TOWER_DEFENSE	= 102, 	// 断魂桥副本
        SCRIPT_T_FOUR_ALTAR 	= 103,	// 四相祭坛副本

        SCRIPT_T_END
    };

    enum SCRIPT_FRESH_TIMES_TYPE
    {
        SCRIPT_FTT_ENTER_FRESH = 0, // 进入时增加次数
        SCRIPT_FTT_FINISH_FRESH = 1,// 完成时增加次数
        SCRIPT_FTT_END_FRESH = 2,   // 完成或失败时增加次数

        SCRIPT_FTT_END
    };

    // 怪物特定的阵营ID
    enum MONSTER_CAMP_TYPE
    {
        MONSTER_CAMP_BASE = 500000001,      // 怪物基本阵营
        MONSTER_CAMP_NPC = 500000002,       // NPC阵营

        MONSTER_CAMP_END
    };

    enum SCRIPT_EXIT_STATE
    {
    	SCRIPT_EXIT_RUNNING	= 1,
    	SCRIPT_EXIT_STOPED = 2,
    	SCRIPT_EXIT_END
    };
    
    enum SCRIPT_SORT_TYPE
    {
//        SCRIPT_SORT_CLIMB_TOWER = 20300,	// 无尽试炼
        SCRIPT_SORT_LEGEND_TOP 	= 20500,	// 问鼎江湖
        SCRIPT_SORT_COUPLE      = 20701    	// 夫妻副本ID
    };

    enum SCRIPT_STAR_TYPE
    {
    	SCRIPT_KILL_MOSTER 	= 1, 		// 击杀怪物数
    	SCRIPT_PASS_TICK 	= 2,		// 指定时间内通关
    	SCRIPT_DIE_NUM 		= 3,		// 死亡次数

    	SCRIPT_STAR_END
    };

    enum SUB_SCRIPT_ID
    {
    	LEGEND_TOP_SCENE		= 20501,
    	SWORD_TOP_SCENE			= 20901,

    	SUB_SCRIPT_END
    };

    enum SCRIPT_ENTER_TYPE
    {
    	TYPE_OPEN_SERVER_DAY 	= 1,	//开服天数

    	TYPE_SCRIPT_ENTER_END
    };
};

class BaseScript;
typedef std::bitset<GameEnum::SCRIPT_SF_END> ScriptSceneFlagSet;

// 进入副本前验证时创建；
// 副本对象回收时回收ScriptPlayerRel
struct ScriptPlayerRel
{
    int __gate_sid;     // 网关ID
    int64_t __role_id;
    int __script_sort;  // 副本配置ID
    int __script_id;    // 副本对象ID, 按副本对象分配
    Int64 __progress_id;  // 进度号, 记录副本完成的进度（杀怪）
    int __scene_id;		// 源场景ID
    int __team_id;      // 队伍ID
    int __level;		// 玩家等级
    int __cheer_num;	// 助威人数
    int __encourage_num;// 鼓舞人数

    LongMap teamer_set_;// temp teamer
    LongMap replacement_set_;// temp src copy player

    int __used_times;   // 当天已使用次数
    int __buy_times;	// 当天购买次数
    Time_Value __fresh_tick;    // 副本次数刷新时间

    int __pass_piece;	// 已通过的篇
    int __pass_chapter; // 已通过的章
    int __piece;		// 当前进入的篇
    int __chapter;		// 当前进入的章
    int __vip_type;		// vip类型
    int __day_online_sec;   // 当天在线时间

    BSONObj *__progress_obj;

    ScriptPlayerRel(void);
    ~ScriptPlayerRel(void);
    void reset(void);

    ScriptPlayerRel &operator=(const ScriptPlayerRel &obj);
};

typedef HashMap<int, int, NULL_MUTEX> MonsterMap;
typedef std::map<int, IntMap > WaveMonsterMap;
typedef WaveMonsterMap ConfigMonsterMap;

struct ScriptDetail
{
    int __script_sort;
    Int64 __progress_id;
    int __stop_times;
    int __exit_script_state;	// 1 退出副本时正在跑，２ 已停止
    int __cheer_num;		//助威人数
    int __encourage_num;	//鼓舞人数

    IntVec __star_lvl;		//副本结束，计算星级
    BLongSet __player_set;

    LongMap couple_sel_;		//夫妻副本选择的答案
    LongMap teamer_set_;
    LongMap replacements_set_;
    LongMap src_replacements_set_;	// src player role_id

    Time_Value __recycle_tick;
    int __max_monster_num;  // 副本失败的怪物最大数量

    int __total_used_tick;
    bool __is_single;   // 是否单人副本
    int __protect_npc_sort; // 保护的NPC模型
    Int64 __owner_id;	// 单人所属者ID
    struct Team
    {
        int __team_id;
        LongStrMap __teamer_map;

        BLongSet __history_role;    // 进入副本前验证时插入
        BLongMap __used_times_tick_map;  // 历史进入时的副本次数刷新时间

        void reset(void);
    };
    Team __team;

    struct Scene
    {
        int __cur_scene;
        int __cur_floor;

        int __last_scene;
        int __last_floor;

        int __next_scene;
        int __next_floor;

        int __passed_scene;
        int __passed_floor;
        
        BIntSet __started_scene;
        BIntSet __finish_scene;     // 已pass的场景集合
        ScriptSceneFlagSet __scene_flag_set;
        Int64 __notify_finish_tick;

        int __monster_level_index;

        Time_Value __monster_fail_notify_tick; // 怪物数量超过通知
        Time_Value __monster_failure_tick;  // 副本怪物数量达到指定数量后失败时间

        void reset(void);
    };
    Scene __scene;

    struct Tick
    {
        int __ready_sec;	//准备时间
        int __used_sec;
        int __total_sec;	//配置总时间
        int __inc_sec;

        Int64 __ready_tick;
        Int64 __begin_tick;
        Int64 __end_tick;

        void reset(void);
    };
    Tick __tick;

    struct MagicMonster
    {
    	Int64 _boss_ai_id;	//boss本体
    	Int64 _monster_ai_id;	//小怪幻体
    	Int64 _role_id;	//小怪继承的玩家id
    	string _name;	//小怪继承玩家名字（+幻体）
    	int _flag;	//1表示幻体出现，2表示幻体消失
    	void reset(void);
    };

    struct Monster
    {
        int __total_monster;
        int __left_monster;	//剩余怪物数量

        MonsterMap __monster_left_map;
        MonsterMap __monster_killed_map;
        MonsterMap __monster_total_map;

        ConfigMonsterMap __config_monster_map;

        int __evencut;
        int __max_evencut;
        Time_Value __clear_evencut_tick;

        int __boss_hurt;        // 统计BOSS的总扣血
        int __player_hurt;

        std::string __poem_text;
        int __text_size;        // 诗句的字数
        int __appear_text_max;  // 出现连续诗句文字的最大索引
        BIntSet __appear_text_set;  // 出现诗句字索引集合
        BIntSet __special_sort_set; // 必定出现未出现诗句的怪物
        MagicMonster __magic_monster;

        void reset(void);
        void reset_everytime();
    };
    Monster __monster;

    struct Wave
    {
        // 总共多少波完成
        int __total_wave;
        // 当前生成怪物的波数
        int __cur_wave;
        // 已完成的波数
        int __finish_wave;
        // 是否完成了一波以上
        int __pass_wave;
        // 开始波数
        int __begin_wave;

        IntMap __wave_monster_map;  // key: wave_id, value: left_monster;
        WaveMonsterMap __wave_killed_map; // key_1: wave_id, key_2: sort, value: killed_monster_amount;
        WaveMonsterMap __wave_total_map;  // key_1: wave_id, key_2: sort, value: total_monster_amount;

        // 阵灵值
        int __spirit_value;
        // 阵法对应的等级
        IntMap __matrix_level_map;

        IntVec __active_puppet_flag;
        IntMap __active_puppet;
        
        void reset(void);
        void reset_everytime();
    };
    Wave __wave;

    struct Relive
    {
        // 总的复活次数
        int __total_relive;
        // 剩余复活次数
        int __left_relive;
        // 已使用复活次数
        int __used_relive;
        // 已使用的道具复活次数
        int __used_item_relive;

        void reset(void);
    };
    Relive __relive;

    struct Piece
    {
        int __piece;    // 第几篇
        int __chapter;  // 第几章 
        IntMap __piece_chapter_map;    // 每篇最大的章节数

        void reset(void);
    };
    Piece __piece;

    typedef boost::unordered_map<int, ItemObj> ItemMap;
    struct PlayerAward
    {
    	int __additional_exp;	// 用于副本一些额外经验奖励
        int __exp;				// 副本内记录获得的总经验
        int __copper;
        ItemMap __item_map;

        int __dps;	//伤害输出

        PlayerAward(void);
        void reset(void);
    };
    typedef boost::unordered_map<Int64, PlayerAward> PlayerAwardMap;
    PlayerAwardMap __player_award_map;

    void reset(void);
};

// 进入副本前验证时创建
// 副本对象回收时删除__script_list，列表为空时回收ScriptTeamDetail
struct ScriptTeamDetail
{
    int __team_id;
    
    LongList __teamer_list; // 进入副本前验证时插入
    LongMap __replacements_map;	//化身玩家
    BLongSet __script_list; // 进入副本前验证时插入

    void reset(void);
};

struct ScriptPlayerDetail
{
    int __script_id;		//副本唯一ID
    int __script_sort;		//副本配置ID
    int __task_listen;		//任务监听

    int __prev_scene;
    MoverCoord __prev_coord;
    int __prev_blood;   // 进入副本前的血量
    int __prev_magic;   // 进入副本前的法力

    int __trvl_total_pass;	//跨服副本通过次数
    int __max_trvl_pass;	//跨服最多通关次数

    int __skill_id;		//论剑武林技能id

    typedef std::map<int, ItemObj> ItemMap;
    ItemMap __first_pass_item;

    struct PieceRecord
    {
        int __pass_piece;
        int __pass_chapter;

        struct PieceChapterInfo
        {
            int __chapter_key;      // 篇ID * 1000 + 章ID
            int __used_sec;         // 最好成绩
            int __used_times;       // 使用次数，每天刷新
            int __totay_pass_flag;	// 当天已通关标识，每天刷新

            void reset(void);
        };

        typedef boost::unordered_map<int, PieceChapterInfo> PassChapterMap;
        PassChapterMap __pass_chapter_map;          // key: piece * 1000 + chapter_id; value: PieceChapterInfo

        IntMap __piece_star_award_map;
        ScriptPlayerDetail::ItemMap __pass_chapter_item;

        void reset(void);
    };
    PieceRecord __piece_record;

    //问鼎江湖信息
    struct LegendTopInfo
    {
    	int __pass_floor; 			//总通关最大层
    	int __today_rank;			//当前排名
    	int __is_sweep;				//是否以扫荡

    	struct FloorInfo
    	{
    		int __floor_id;
    		Int64 __pass_tick;
    		int __totay_pass_flag;	// 当天已通关标识，每天刷新

    		void reset(void);
    	};
    	typedef boost::unordered_map<int, FloorInfo> FloorMap;
    	FloorMap __piece_map;

    	void reset(void);
    };
    LegendTopInfo __legend_top_info;	//问鼎江湖
    LegendTopInfo __sword_top_info;		//武林论剑

    struct TypeRecord
    {
    	int __script_type;
    	int __max_script_sort;
    	int __pass_wave;		//通过波数
    	int __pass_chapter;		//通过章节
    	int __notify_wave;		//经验/帮派副本进行的副本波数
    	int __notify_chapter;	//经验/帮派副本进行的副本章节
    	int __start_wave;		//帮派副本开始波数
    	int __start_chapter;	//帮派副本开始章节
    	int __is_sweep;
    	Time_Value __used_times_tick;
    	IntMap __reward_map;

    	TypeRecord(void);
    	void reset(void);
    };
    typedef boost::unordered_map<int, TypeRecord> TypeRecordMap;
    TypeRecordMap __type_map;

    struct ScriptWaveRecord
    {
        int __script_wave_id; 	// 副本id波数联合id
        int __is_get;			// 是否可领取,0:不可领取 1:可领取 2:已领取

        ScriptWaveRecord(void);
        void reset(void);
    };
    typedef boost::unordered_map<int, ScriptWaveRecord> ScriptWaveMap;
    ScriptWaveMap __script_wave_map;          // key: script_sort * 1000 + wave; value: ScriptWaveRecord

    struct ScriptRecord
    {
        int __script_sort;	//副本种类
        int __used_times;
        int __buy_times;    // 购买副本次数
        int __couple_buy_times; // 夫妻赠送次数
        Time_Value __used_times_tick;   // 玩家使用次数刷新时间
        Time_Value __enter_script_tick;
        Int64 __progress_id;
        int __best_use_tick;    // 最好时间
        int __award_star;       // 奖励星级
        int __is_first_pass;    // 是否已首次通过副本
        int __day_pass_times;   // 当日通关次数
        int __is_even_enter;	// 是否进入过副本
        int __protect_beast_index;    // 成功保护的最高宠物配置索引
        ScriptRecord(void);
        void reset(void);
    };
    typedef std::map<int, ScriptRecord> ScriptRecordMap;
    ScriptRecordMap __record_map;
    IntVec __first_script_vc;

    void reset(void);
    bool is_have_trvl_red();
};

struct LegendTopPlayer
{
	Int64 refresh_tick_;

	struct PlayerInfo
	{
		Int64 player_id_;
		string name_;
		int fight_score_;
		int floor_;
		int rank_;
		Int64 tick_;

		void reset(void);
		void serialize(ProtoLegendTopRank *proto_rank);
	};
	typedef std::map<Int64, PlayerInfo> PlayerInfoMap;
	PlayerInfoMap player_map_;

	void reset(void);
};

struct HistoryChapterRecord
{
    Int64 __first_top_level_player;
    std::string __first_top_level_role_name;
    int __chapter_key;
    int __best_use_tick;

    void reset(void);
};

struct ScriptSceneDetail
{
    int __left_monster;
    int __killed_monster;
    int __total_monster;

    void reset(void);
};

struct FourAltarDetail
{
	LongSet __boss_id_set;
    
    struct EffectDetail
    {
        int __target;
        int __effect_type;
        double __effect_value;
        double __effect_percent;
        double __effect_interval;
        double __effect_last;
        double __skill_interval;
        int __text_id;

        EffectDetail(void);
        void reset(void);
    };
    typedef std::vector<EffectDetail> EffectList;
    struct SceneSkillDetail
    {
        EffectList __once_buff_list;
        EffectDetail __scene_skill;
        Time_Value __skill_check_tick;
        int __tip_id;   // 点亮图标索引
        void reset(void);
    };
    typedef std::map<int, SceneSkillDetail> BossSceneSkillMap;
    BossSceneSkillMap __scene_skill_map;    // key: boss sort, value: scene skill

    Time_Value __min_skill_check_tick;      // 下次场景技能触发时间
    Time_Value __min_interval_check_tick;   // 至少间隔10秒
    LongSet __active_boss_set;
    
    void reset(void);
};

struct DropDragonHole_Role{ //坠龙窟副本玩家信息
 	Int64 role_id_;   //玩家id
	int scirpt_skill_id_; //玩家在副本里携带的技能id
	int skill_buff_id_;  //技能触发时对应的buff id
	int gather_sort_;  //玩家采集技能物品id
	DropDragonHole_Role();
	void reset(void);
};

typedef std::vector<DropDragonHole_Role>  Role_Set;

#endif //_SCRIPTSTRUCT_H_
