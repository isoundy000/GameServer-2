/*
 * RankSystem.h
 *
 *  Created on: Feb 26, 2014
 *      Author: louis
 */

#ifndef RANKSYSTEM_H_
#define RANKSYSTEM_H_

#include "RankStruct.h"

class Proto31401601;
class RankSystem
{
public:

    typedef PoolPackage<PlayerOfflineData, Int64> PlayerOfflineDataPoolPack;

	enum {
		// 需要定时保存到数据库的数据
		RANK_TYPE_SINGLE_SCRIPT_ZYFM			= 1,     //斩妖除魔录
		RANK_CACHE_TYPE_END,

		// 内部定时器: 在默认构造函数里设置定时器间隔
		TIMEUP_SAVE								= 1,
		TIMEUP_END
	};

	struct CachedTimer : public GameTimer
	{
		CachedTimer();
		virtual ~CachedTimer();

		virtual int type(void);
		virtual int handle_timeout(const Time_Value&);

		RankSystem *rank_system_;
	};


public:
	RankSystem();
	~RankSystem();

	int init(void);
    int fina(void);

    int start(void);
    int stop(void);

    int init_rank_data(void);
    int init_rank_manager(void);

    int refresh_rank_manager(void);
    int fix_rank_manager_next_refresh(void);
    int save_rank_manager_last_refresh(void);
    int save_rank_manager_last_refresh(int rank_type, int refresh_tick);

    RankRecordPool* rank_record_pool();
    RankPannel& rank_pannel();
    RankShowPannel& rank_show_pannel();
    PoolPackage<PlayerOfflineData, Int64>& offline_data_map();
    GameCache& cache_tick();

    RankRecord* pop_rank_record();
    int push_rank_record(RankRecord* record);

    void reset(void);

public:
    //player offline data
    int update_player_offline(Message* msg);
    int fetch_player_offline(LogicPlayer* player, Message* msg);

    int map_fetch_rank_info(const ProtoHead& head, Message* msg);

    PlayerOfflineData* find_player_offline(Int64 role_id);
    PlayerOfflineData* check_and_pop_player_offline(Int64 role_id);

public:
    /*
     * 实时排行调用的接口，目前只在斩妖除魔录副本排行-> 现统一为整点排行
     * */
	int update_rank_data(Message* msg);
	int request_save_player_script_zyfm_date(Proto31401601* pass_info);
    int update_realtime_rank_data(Message *msg);
    void sort_realtime_rank_data(RankRecord *rank_rec);
    void clear_realtime_rank(const int rank_type);

	int rank_manager_maintenance();
	int refresh_rank_manager_at_midnight(void);
	int refresh_rank_data_at_hour(void);


	/*
	 * 根据排行榜类型来刷新重新生成排行数据
	 * */
    int request_refresh_rank_data(const int rank_type);
    int after_db_refresh_rank_data(Transaction* transaction);


    int request_save_rank_pannel_data(void);
    int request_save_rank_data(const int rank_type);
    int request_save_rank_record_detail(const int rank_type);

    //find opra
    Int64 calc_rank_player_pet_id(const Int64 player_id);
    RankRecord* fetch_rank_record(const int rank_type, const int rank_index);
    RankRecord* fetch_rank_record(const int rank_type, Int64 player_id);
    int fetch_player_rank(const int rank_type, Int64 player_id);
    bool is_player_on_rank(const int rank_type, Int64 player_id);
    int map_act_fetch_level_fight_rank_info(Message* msg);
    int map_act_fetch_level_fight_rank_info(int type, int size);

    int update_player_name(const Int64 role_id, const std::string &name);

    //后台排行榜中屏蔽玩家
    int req_get_hide_player();
    int after_get_hide_player(Transaction* transaction);
    LongSet& get_hide_player();
    int update_act_flower_rank(Message* msg);
    int reset_act_flower_rank(Transaction* transaction);
private:
    int check_rank_refresh(RankRefreshDetail& refresh_detail);
    int modify_rank_refesh_tick(RankRefreshDetail& refresh_detail);

    int bson2rank_data(auto_ptr<TQueryCursor> &cursor, const int rank_type, RankRecordVec& record_vec);
    int make_up_rank_pannel(const int rank_type, RankRecordVec& record_vec);
    int make_up_rank_pannel_info(const int rank_type, RankRecordVec& rank_set);
	int sort_rank_data(const int rank_type, RankRecordVec& rank_set);

    int clear_rank_data_by_rank_type(const int rank_type);
    int clear_rank_show_data_by_rank_type(const int rank_type);

    static int rank_system_open_level(void);

    //{{{ 实时排行调用的接口
    bool is_better_single_script_record(RankRecord* target_record, int rank_value, Int64 achieve_tick);
    bool is_better_single_script_record(RankRecord* target_record, RankRecord* src_record);
    int push_new_rank_record_to_show_pannel(RankRecord* record);
    int show_pannel_refresh_data(const int rank_type);
    void recycle_single_rank_record(RankRecord* record);
    void display_rank_show_pannel(const int rank_type);
    //}}}

    int rank_cache_time_up(const Time_Value& now_time);
    int check_and_clear_offline_data(void);

private:
    RankPannel rank_pannel_;
    RankShowPannel rank_show_pannel_;

    RankRecordPool rank_record_pool_;
    RankRefreshManager rank_manager_;

    PlayerOfflineDataPoolPack offline_data_map_;

    GameCache rank_cache_tick_;
    CachedTimer rank_cache_timer_;

    Time_Value realtime_save_tick_;
    IntSet cache_rank_type_;

    int average_role_level_;	//排行榜平均等级
    LongSet hide_player_;
};

typedef Singleton<RankSystem> RankSystemSingle;
#define RANK_SYS              (RankSystemSingle::instance())
#endif /* RANKSYSTEM_H_ */
