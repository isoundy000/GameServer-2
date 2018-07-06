/*
 * File Name: TrvlBattleMonitor.cpp
 * 
 * Created on: 2017-04-18 21:56:47
 * Author: glendy
 * 
 * Last Modified: 2017-05-15 18:20:32
 * Description: 
 */

#include "TrvlBattleMonitor.h"
#include "ProtoDefine.h"
#include "GameConfig.h"
#include "GameCommon.h"
#include "TrvlBattleScene.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "GameFont.h"
#include "MMOTrvlBattle.h"

void TrvlBaseRole::reset(void)
{
    this->__role_id = 0;
    this->__role_name.clear();
    this->__prev.clear();
    this->__sex = 0;
    this->__career = 0;
    this->__level = 0;
    this->__force = 0;
    this->__weapon = 0;
    this->__clothes = 0;
    this->__wing_level = 0;
    this->__solider_level = 0;
    this->__vip_type = 0;
    this->__mount_sort = 0;
    this->__sword_pool = 0;
    this->__tian_gang = 0;
    this->__fashion_id = 0;
    this->__fashion_color = 0;
}

void TrvlBaseRole::serialize(ProtoTrvlBaseRole *msg)
{
    msg->set_role_id(this->__role_id);
    msg->set_role_name(this->__role_name);
    msg->set_prev(this->__prev);
    msg->set_sex(this->__sex);
    msg->set_career(this->__career);
    msg->set_level(this->__level);
    msg->set_force(this->__force);
    msg->set_weapon(this->__weapon);
    msg->set_clothes(this->__clothes);
    msg->set_fashion_clothes(this->__fashion_id);
    msg->set_wing_level(this->__wing_level);
    msg->set_solider_level(this->__solider_level);
    msg->set_vip_type(this->__vip_type);
    msg->set_mount_sort(this->__mount_sort);
    msg->set_sword_pool(this->__sword_pool);
    msg->set_tian_gang(this->__tian_gang);
    msg->set_fashion_id(this->__fashion_id);
    msg->set_fashion_color(this->__fashion_color);
}

void TrvlBaseRole::unserialize(const ProtoTrvlBaseRole &msg)
{
    this->__role_id = msg.role_id();
    this->__role_name = msg.role_name();
    this->__prev = msg.prev();
    this->__sex = msg.sex();
    this->__career = msg.career();
    this->__level = msg.level();
    this->__force = msg.force();
    this->__weapon = msg.weapon();
    this->__clothes = msg.clothes();
    this->__wing_level = msg.wing_level();
    this->__solider_level = msg.solider_level();
    this->__vip_type = msg.vip_type();
    this->__mount_sort = msg.mount_sort();
    this->__sword_pool = msg.sword_pool();
    this->__tian_gang = msg.tian_gang();
    this->__fashion_id = msg.fashion_id();
    this->__fashion_color = msg.fashion_color();
}

void TrvlBattleRanker::reset(void)
{
    TrvlBaseRole::reset();
    this->__rank = 0;
    this->__score = 0;
    this->__rank_score = 0;
    this->__total_kill_amount = 0;
    this->__tick = 0;
}

void TrvlBattleRanker::serialize(ProtoTrvlBattleRank *msg)
{
    msg->set_rank(this->__rank);
    msg->set_role_id(this->__role_id);
    msg->set_role_name(this->__role_name);
    msg->set_score(this->__rank_score);
    msg->set_kill_amount(this->__total_kill_amount);
    msg->set_force(this->__force);
    msg->set_tick(this->__tick);
    msg->set_role_sex(this->__sex);
}

bool trvl_battle_ranker_cmp(const TrvlBattleRanker *left, const TrvlBattleRanker *right)
{
    if (left->__rank_score != right->__rank_score)
        return left->__rank_score > right->__rank_score;
    if (left->__total_kill_amount != right->__total_kill_amount)
        return left->__total_kill_amount > right->__total_kill_amount;
    if (left->__force != right->__force)
        return left->__force > right->__force;
    if (left->__tick != right->__tick)
        return left->__tick < right->__tick;
    return left->__role_id < right->__role_id;
}

void TrvlBattleRole::reset(void)
{
    BaseServerInfo::reset();

    this->__role_id = 0;
    this->__league_id = 0;
    this->__role_name.clear();
    this->__max_floor = 0;

    this->__cur_floor = 0;
    this->__cur_floor_kill_amount = 0;
    this->__last_reward_score = 0;
    this->__next_reward_score = 0;
    this->__next_score_reward_id = 0;
    this->__even_kill_amount = 0;
}

int TrvlBattleRole::next_award_score(void)
{
    int last_score = this->__last_reward_score;
    const Json::Value &score_reward_json = CONFIG_INSTANCE->tbattle_reward()["score_reward"];
    for (uint i = 0; i < score_reward_json.size(); ++i)
    {
        if (this->__last_reward_score >= score_reward_json[i][0u].asInt())
            continue;

        last_score = score_reward_json[i][0u].asInt();
        break;
    }
    return last_score;
}

int TrvlBattleMonitor::BattleStageTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int TrvlBattleMonitor::BattleStageTimer::handle_timeout(const Time_Value &tv)
{
    TRVL_BATTLE_MONITOR->handle_battle_stage_timeout(tv);
    return 0;
}

int TrvlBattleMonitor::BattleSecTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int TrvlBattleMonitor::BattleSecTimer::handle_timeout(const Time_Value &tv)
{
    TRVL_BATTLE_MONITOR->handle_timeout(tv);
    return 0;
}

TrvlBattleMonitor::TrvlBattleMonitor(void)
{
    this->trvl_battle_ranker_pool_ = new TrvlBattleRankerPool();
    this->cur_ranker_map_ = new TrvlBattleRankerMap(4095);
    this->cur_ranker_list_ = new TrvlBattleRankList();

    this->trvl_battle_role_pool_ = new TrvlBattleRolePool();
    this->battle_role_map_ = new TrvlBattleRoleMap(4095);

    this->last_ranker_map_ = new TrvlBattleRankerMap(4095);
    this->last_ranker_list_ = new TrvlBattleRankList();

    this->history_hall_of_fame_list_ = new TrvlBattleRankList();

    this->indiana_list_ = new TrvlBattleRankList();

    this->trvl_battle_scene_pool_ = new TrvlBattleScenePool();
    this->scene_map_ = new TrvlBattleSceneMap();
    this->is_created_scene_ = false;

    this->is_sort_rank_  = false;
    this->sort_rank_tick_ = Time_Value::zero;
}

TrvlBattleMonitor::~TrvlBattleMonitor(void)
{
    SAFE_DELETE(this->trvl_battle_ranker_pool_);
    SAFE_DELETE(this->cur_ranker_map_);
    SAFE_DELETE(this->cur_ranker_list_);

    SAFE_DELETE(this->trvl_battle_role_pool_);
    SAFE_DELETE(this->battle_role_map_);

    SAFE_DELETE(this->last_ranker_map_);
    SAFE_DELETE(this->last_ranker_list_);

    SAFE_DELETE(this->history_hall_of_fame_list_);

    SAFE_DELETE(this->indiana_list_);

    SAFE_DELETE(this->trvl_battle_scene_pool_);
    SAFE_DELETE(this->scene_map_);
}

int TrvlBattleMonitor::start(void)
{
    // load mongo;
    MMOTrvlBattle::load_trvl_battle_info(this);

    const Json::Value &scene_set_json = CONFIG_INSTANCE->scene_set(this->scene_id());
    this->activity_id_ = scene_set_json["activity_id"].asInt();
    this->enter_level_ = scene_set_json["enter_level"].asInt();
    this->max_floor_ = CONFIG_INSTANCE->tbattle_reward()["total_floor"].asInt();

    const Json::Value &activity_conf = CONFIG_INSTANCE->common_activity(this->activity_id_);

    GameCommon::cal_activity_info(this->time_info_, activity_conf);

    MSG_USER("TrvlBattleMonitor start %d %d %d %d", this->activity_id_, this->enter_level_, 
            this->time_info_.cur_state_, this->time_info_.refresh_time_);

    this->battle_stage_timer_.schedule_timer(this->time_info_.refresh_time_);
    this->battle_sec_timer_.schedule_timer(1);
    if (this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START)
    {
        this->activity_end_tick_ = Time_Value::gettimeofday() + Time_Value(this->time_info_.refresh_time_);
        this->start_event();
    }
    return 0;
}

int TrvlBattleMonitor::stop(void)
{
    // SAVE mongo;
    MMOTrvlBattle::save_trvl_battle_info(this);
	return 0;
}

int TrvlBattleMonitor::scene_id(void)
{
    return GameEnum::TRVL_BATTLE_SCENE_ID;
}

int TrvlBattleMonitor::activity_id(void)
{
    return this->activity_id_;
}

MoverCoord TrvlBattleMonitor::fetch_enter_pos(void)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
    const Json::Value &enter_pos_json = scene_json["enter_pos"];
    MoverCoord enter_coord;
    JUDGE_RETURN(enter_pos_json.size() > 0, enter_coord);
    
    int rand_idx = rand() % int(enter_pos_json.size());
    enter_coord.set_pixel(enter_pos_json[rand_idx][0u].asInt(), enter_pos_json[rand_idx][1u].asInt());
    return enter_coord;
}

int TrvlBattleMonitor::handle_battle_stage_timeout(const Time_Value &nowtime)
{
    int prev_stage = this->time_info_.cur_state_;
    this->time_info_.set_next_stage();
    return this->handle_battle_stage_change(prev_stage, this->time_info_.cur_state_);
}

int TrvlBattleMonitor::handle_battle_stage_change(const int prev_stage, const int cur_stage)
{
    if (cur_stage == GameEnum::ACTIVITY_STATE_AHEAD)
    {
        this->ahead_event();
    }
    else if (cur_stage == GameEnum::ACTIVITY_STATE_START)
    {
        this->start_event();
    }
    else
    {
        this->stop_event();
    }

    this->battle_stage_timer_.cancel_timer();
    this->battle_stage_timer_.schedule_timer(Time_Value(this->time_info_.refresh_time_));

    MSG_USER("TrvlBattleMonitor stage timeout %d %d %d %d %d", this->activity_id_, this->enter_level_,
            prev_stage, cur_stage, this->time_info_.refresh_time_);

    return 0;
}

bool TrvlBattleMonitor::is_started_battle_activity(void)
{
    return this->time_info_.cur_state_ == GameEnum::ACTIVITY_STATE_START;
}

int TrvlBattleMonitor::left_actvity_sec(void)
{
	if (this->is_started_battle_activity() == false)
		return 0;

    int left_sec = this->activity_end_tick_.sec() - Time_Value::gettimeofday().sec();
    if (left_sec < 0)
        left_sec = 0;
	return left_sec;
}

void TrvlBattleMonitor::ahead_event(void)
{ 
    if (this->is_created_scene_ == false)
    {
        this->generate_all_trvl_battle_scene();
    }
    GameCommon::map_sync_activity_tips_ahead(PairObj(this->activity_id(), true), this->time_info_.refresh_time_);
}

void TrvlBattleMonitor::start_event(void)
{
    this->first_top_id_ = 0;
    this->first_top_name_.clear();
    this->treasure_owner_ = 0;
    this->treasure_owner_name_.clear();
    this->treasure_timeout_tick_ = Time_Value::zero;

    this->activity_end_tick_ = Time_Value::gettimeofday() + Time_Value(this->time_info_.refresh_time_);

    // 启动九层的scene
    if (this->is_created_scene_ == false)
    {
        this->generate_all_trvl_battle_scene();
    }

    GameCommon::map_sync_activity_tips_start(PairObj(this->activity_id(), true), this->time_info_.refresh_time_);
}

void TrvlBattleMonitor::stop_event(void)
{
    GameCommon::map_sync_activity_tips_stop(PairObj(this->activity_id(), true), this->time_info_.refresh_time_);

    if (this->is_sort_rank_ == true)
    {
        this->handle_sort_rank();
        this->is_sort_rank_ = false;
    }

    // 处理历练奖励
    this->send_all_tbattle_res_reward();

	// 战场日志及名人堂
	this->copy_cur_rank_to_latest_rank();

    //// 踢出场景内所有玩家
    //this->kick_all_online_player();

    MMOTrvlBattle::save_trvl_battle_info(this);

    // 停止九层的scene并回收
    this->recycle_all_trvl_battle_scene();
}

void TrvlBattleMonitor::recycle_scene(TrvlBattleScene *scene)
{
    this->scene_map_->unbind(scene->space_id());

    this->trvl_battle_scene_pool_->push(scene);
}

TrvlBattleScene *TrvlBattleMonitor::trvl_battle_scene_by_floor(const int floor)
{
    TrvlBattleScene *scene = NULL;
    if (this->scene_map_->find(floor, scene) == 0)
        return scene;
    return NULL;
}

void TrvlBattleMonitor::generate_all_trvl_battle_scene(void)
{
    std::vector<TrvlBattleScene *> remove_scene_list;
    for(TrvlBattleSceneMap::iterator iter = this->scene_map_->begin();
            iter != this->scene_map_->end(); ++iter)
    {
        remove_scene_list.push_back(iter->second);
    }
    for (std::vector<TrvlBattleScene *>::iterator iter = remove_scene_list.begin();
            iter != remove_scene_list.end(); ++iter)
    {
        TrvlBattleScene *battle_scene = *iter;
        battle_scene->recycle_scene();
    }
    this->scene_map_->unbind_all();

    int floor = 9;
    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    if (tbattle_reward_json["total_floor"].asInt() > 0)
        floor = tbattle_reward_json["total_floor"].asInt();

    for (int i = 1; i <= floor; ++i)
    {
        TrvlBattleScene *battle_scene = this->trvl_battle_scene_pool_->pop();
        battle_scene->init_trvl_battle_scene(i);
        battle_scene->start_trvl_battle_scene();
        this->scene_map_->bind(battle_scene->space_id(), battle_scene);
    }
    this->is_created_scene_ = true;
}

void TrvlBattleMonitor::recycle_all_trvl_battle_scene(void)
{
#ifdef LOCAL_DEBUG
    Time_Value recycle_tick = Time_Value(120);
#else
    Time_Value recycle_tick = Time_Value(7200);
#endif
    for(TrvlBattleSceneMap::iterator iter = this->scene_map_->begin();
            iter != this->scene_map_->end(); ++iter)
    {
        TrvlBattleScene *battle_scene = iter->second;
        battle_scene->set_scene_auto_recycle_tick(recycle_tick);
    }
    this->is_created_scene_ = false;
}

int TrvlBattleMonitor::request_enter_battle(const int sid, Proto30400051 *request)
{
    JUDGE_RETURN(this->is_started_battle_activity() == true, ERROR_GUARD_TIME);
    JUDGE_RETURN(request->role_level() >= this->enter_level_, ERROR_PLAYER_LEVEL);

    std::string main_version = request->main_version();
    if (main_version.empty() == false)
    {
        JUDGE_RETURN(CONFIG_INSTANCE->is_same_main_version(main_version) == true, ERROR_TRVL_SAME_VERSION);
    }

    TrvlBattleScene *scene = this->trvl_battle_scene_by_floor(1);
    JUDGE_RETURN(scene != NULL, ERROR_GUARD_TIME);

    Int64 role_id = request->role_id();
    TrvlBattleRole *tb_role = this->find_tb_role(role_id);

    Proto30400052 enter_info;


    if (tb_role != NULL)
    {
    	int space_id = std::max(tb_role->__cur_floor, 1);
        enter_info.set_space_id(space_id);
    }
    else
    {
    	enter_info.set_space_id(1);
    }
    enter_info.set_scene_mode(SCENE_MODE_BATTLE_GROUND);

    MoverCoord enter_point = this->fetch_enter_pos();
    enter_info.set_pos_x(enter_point.pixel_x());
    enter_info.set_pos_y(enter_point.pixel_y());

    return MAP_MONITOR->respond_enter_scene_begin(sid, request, &enter_info);
}

TrvlBattleRole *TrvlBattleMonitor::fetch_tbattle_role(const Int64 role_id)
{
    TrvlBattleRole *tb_role = NULL;
    if (this->battle_role_map_->find(role_id, tb_role) != 0)
    {
        tb_role = this->trvl_battle_role_pool_->pop();
        tb_role->__role_id = role_id;
        this->battle_role_map_->bind(role_id, tb_role);
    }
    return tb_role;
}

TrvlBattleRanker *TrvlBattleMonitor::fetch_tbattle_ranker(const Int64 role_id)
{
    TrvlBattleRanker *tb_ranker = NULL;
    if (this->cur_ranker_map_->find(role_id, tb_ranker) != 0)
    {
        tb_ranker = this->trvl_battle_ranker_pool_->pop();
        tb_ranker->__role_id = role_id;
        this->cur_ranker_map_->bind(role_id, tb_ranker);
        this->cur_ranker_list_->push_back(tb_ranker);
    }
    return tb_ranker;
}

void TrvlBattleMonitor::update_info_by_kill_player(const Int64 killer_role_id, const Int64 die_role_id, TrvlBattleScene *scene)
{
    MSG_USER("update kill player %ld %ld", killer_role_id, die_role_id);

    JUDGE_RETURN(this->is_started_battle_activity() == true, ;);

    if (GameCommon::fetch_mover_type(killer_role_id) == MOVER_TYPE_PLAYER)
    {
        TrvlBattleRole *killer_tb_role = this->fetch_tbattle_role(killer_role_id);
        MapPlayerEx *killer_player = scene->fetch_benefited_player(killer_role_id);
        if (killer_tb_role != NULL)
        {
            this->update_tb_role_of_kill(scene, killer_tb_role, killer_player);
        }
    }

    if (GameCommon::fetch_mover_type(die_role_id) == MOVER_TYPE_PLAYER)
    {
        TrvlBattleRole *die_tb_role = this->fetch_tbattle_role(die_role_id);
        MapPlayerEx *die_player = scene->fetch_benefited_player(die_role_id);
        if (die_tb_role != NULL)
        {
            this->update_tb_role_of_die(scene, die_tb_role, die_player);
        }
    }
}

void TrvlBattleMonitor::update_info_by_kill_monster(const Int64 killer_id, const Int64 ai_id, const int ai_sort, TrvlBattleScene *scene)
{
    MSG_USER("update kill monster %ld %ld %d", killer_id, ai_id, ai_sort);

    JUDGE_RETURN(this->is_started_battle_activity() == true, ;);

    if (GameCommon::fetch_mover_type(killer_id) == MOVER_TYPE_PLAYER)
    {
        TrvlBattleRole *killer_tb_role = this->fetch_tbattle_role(killer_id);
        MapPlayerEx *killer_player = scene->fetch_benefited_player(killer_id);
        if (killer_tb_role != NULL)
        {
            this->update_tb_role_of_kill(scene, killer_tb_role, killer_player);
        }
    }
}

void TrvlBattleMonitor::update_tb_role_of_kill(TrvlBattleScene *scene, TrvlBattleRole *killer_tb_role, MapPlayerEx *killer_player)
{
    TrvlBattleRanker *killer_ranker = this->fetch_tbattle_ranker(killer_tb_role->__role_id);

    // 更新击杀者信息
    if (killer_tb_role->__max_floor < scene->space_id())
    {
        if (killer_player != NULL)
        {
            this->reinit_tb_role_base_info_by_player(killer_tb_role, killer_player);
            this->reinit_tb_ranker_base_info_by_player(killer_ranker, killer_player);
            killer_tb_role->__max_floor = scene->space_id();
            // 首次进入此层的奖励
            this->send_first_enter_floor_reward(killer_tb_role, killer_player);
        }
    }
    killer_tb_role->__cur_floor = scene->space_id();
    ++killer_tb_role->__cur_floor_kill_amount;
    ++killer_tb_role->__even_kill_amount;

    if (killer_tb_role->__even_kill_amount >= 5)
    {
        const Json::Value &event_kill_shout_json = CONFIG_INSTANCE->tbattle_reward()["event_kill_shout"];
        for (uint i = 0; i < event_kill_shout_json.size(); ++i)
        {
            int event_kill_amount = event_kill_shout_json[i][0u].asInt(), shout_id = event_kill_shout_json[i][1u].asInt();
            if (killer_tb_role->__even_kill_amount == event_kill_amount)
            {
                BrocastParaVec para_vec;
                GameCommon::push_brocast_para_string(para_vec, killer_tb_role->__role_name);
                GameCommon::announce(shout_id, &para_vec, this->scene_id());
            }
        }
    }

    int prev_score = 0;
    if (killer_ranker != NULL)
        prev_score = killer_ranker->__score;

    this->calc_event_kill_score(killer_ranker, killer_tb_role);
    this->calc_kill_score(killer_ranker, killer_tb_role);

    int new_score = 0;
    if (killer_ranker != NULL)
    {
        new_score = killer_ranker->__score;
    }
    if (new_score != prev_score)
    {
        // 设置排序标识
        this->is_sort_rank_ = true;

        this->check_and_send_score_reward(killer_ranker, killer_tb_role, killer_player);

        if (killer_player != NULL)
            killer_player->notify_tbattle_left_pannel();
    }

    MSG_USER("update travel tb role killer %ld %s %d %d", killer_tb_role->__role_id, killer_tb_role->__role_name.c_str(), prev_score, new_score);
}

void TrvlBattleMonitor::update_tb_role_of_die(TrvlBattleScene *scene, TrvlBattleRole *die_tb_role, MapPlayerEx *die_player)
{
    TrvlBattleRanker *die_ranker = this->fetch_tbattle_ranker(die_tb_role->__role_id);

    // 更新被杀者信息
    if (die_tb_role->__max_floor < scene->space_id())
    {
        if (die_player != NULL)
        {
            this->reinit_tb_role_base_info_by_player(die_tb_role, die_player);
            this->reinit_tb_ranker_base_info_by_player(die_ranker, die_player);
            die_tb_role->__max_floor = scene->space_id();
            // 首次进入此层的奖励
            this->send_first_enter_floor_reward(die_tb_role, die_player);
        }
    }
    die_tb_role->__cur_floor = scene->space_id();
    die_tb_role->__even_kill_amount = 0;

    int prev_score = 0;
    if (die_ranker != NULL)
        prev_score = die_ranker->__score;

    this->calc_bekill_score(die_ranker, die_tb_role);

    int new_score = 0;
    if (die_ranker != NULL)
        new_score = die_ranker->__score;

    if (new_score != prev_score)
    {
        // 设置排序标识
        this->is_sort_rank_ = true;

        this->check_and_send_score_reward(die_ranker, die_tb_role, die_player);

        if (die_player != NULL)
            die_player->notify_tbattle_left_pannel();
    }

    MSG_USER("update travel tb role die %ld %s %d %d", die_tb_role->__role_id, die_tb_role->__role_name.c_str(), prev_score, new_score);
}

void TrvlBattleMonitor::reinit_tb_role_base_info_by_player(TrvlBattleRole *tb_role, MapPlayerEx *player)
{
    JUDGE_RETURN(tb_role != NULL && player != NULL, ;);
    tb_role->__league_id = player->league_id();
    tb_role->__role_name = player->role_name();
    player->role_detail().unserialize(*tb_role);
    char sz_prev[513];
    ::snprintf(sz_prev, 512, "s%d", player->role_detail().__server_id);
    tb_role->__server_prev = sz_prev;

}

void TrvlBattleMonitor::reinit_tb_ranker_base_info_by_player(TrvlBattleRanker *tb_ranker, MapPlayerEx *player)
{
    JUDGE_RETURN(tb_ranker != NULL && player != NULL, ;);

    tb_ranker->__role_id = player->role_id();
    tb_ranker->__role_name = player->role_name();
    tb_ranker->__server_flag = player->role_detail().__trvl_server_flag;
    {
        char sz_prev[513];
        ::snprintf(sz_prev, 512, "s%d", player->role_detail().__server_id);
        tb_ranker->__prev = sz_prev;
    }
    tb_ranker->__sex = player->role_detail().__sex;
    tb_ranker->__career = player->role_detail().__career;
    tb_ranker->__level = player->role_detail().__level;
    tb_ranker->__force = player->force_total_i();
    tb_ranker->__weapon = player->get_shape_item_id(GameEnum::EQUIP_WEAPON);
    tb_ranker->__clothes = player->get_shape_item_id(GameEnum::EQUIP_YIFU);
    tb_ranker->__wing_level = player->fetch_mount_id(GameEnum::FUN_XIAN_WING);
    tb_ranker->__solider_level = player->fetch_mount_id(GameEnum::FUN_GOD_SOLIDER);
    tb_ranker->__vip_type = player->vip_type();
    tb_ranker->__mount_sort = player->fetch_mount_id(GameEnum::FUN_MOUNT);
    tb_ranker->__sword_pool = player->fetch_spool_style_lvl();
    tb_ranker->__tian_gang = player->fetch_mount_id(GameEnum::FUN_TIAN_GANG);
    tb_ranker->__fashion_id = player->fetch_fashion_id();
    tb_ranker->__fashion_color = player->fetch_fashion_color();
}

void TrvlBattleMonitor::calc_event_kill_score(TrvlBattleRanker *killer_tb_ranker, TrvlBattleRole *killer_tb_role)
{
    JUDGE_RETURN(killer_tb_ranker != NULL, ;);

    const Json::Value &event_kill_score_json = CONFIG_INSTANCE->tbattle_reward()["event_kill_score"];
    for (uint i = 0; i < event_kill_score_json.size(); ++i)
    {
        int kill_amount = event_kill_score_json[i][0u].asInt();
        if (killer_tb_role->__even_kill_amount == kill_amount)
        {
            killer_tb_ranker->__score += event_kill_score_json[i][1u].asInt();
            killer_tb_ranker->__tick = ::time(NULL);
            break;
        }
        if (killer_tb_role->__even_kill_amount < kill_amount)
        {
            break;
        }
    }
}

void TrvlBattleMonitor::calc_kill_score(TrvlBattleRanker *killer_tb_ranker, TrvlBattleRole *killer_tb_role)
{
    JUDGE_RETURN(killer_tb_ranker != NULL, ;);

    int kill_score = CONFIG_INSTANCE->tbattle_reward()["kill_score"].asInt();
    killer_tb_ranker->__score += kill_score;
    killer_tb_ranker->__tick = ::time(NULL);
    ++killer_tb_ranker->__total_kill_amount;
}

void TrvlBattleMonitor::calc_bekill_score(TrvlBattleRanker *die_tb_ranker, TrvlBattleRole *killer_tb_role)
{
    JUDGE_RETURN(die_tb_ranker != NULL, ;);

    int bekill_score = CONFIG_INSTANCE->tbattle_reward()["bekill_score"].asInt();
    die_tb_ranker->__score += bekill_score;
    die_tb_ranker->__tick = ::time(NULL);
}

void TrvlBattleMonitor::inc_score_only(BattleGroundActor *player, const int inc_score)
{
    JUDGE_RETURN(player != NULL, ;);
    JUDGE_RETURN(inc_score > 0, ;);
    JUDGE_RETURN(this->is_started_battle_activity() == true, ;);

    TrvlBattleRole *tb_role = this->fetch_tbattle_role(player->role_id());
    TrvlBattleRanker *tb_ranker = this->fetch_tbattle_ranker(player->role_id());
    JUDGE_RETURN(tb_ranker != NULL && tb_role != NULL, ;);

    this->reinit_tb_role_base_info_by_player(tb_role, dynamic_cast<MapPlayerEx *>(player));
    this->reinit_tb_ranker_base_info_by_player(tb_ranker, dynamic_cast<MapPlayerEx *>(player));

    tb_ranker->__score += inc_score;
    tb_ranker->__tick = ::time(NULL);
    this->is_sort_rank_ = true;

    if (inc_score > 0)
    {
        TrvlBattleRole *tb_role = this->find_tb_role(player->role_id());
        this->check_and_send_score_reward(tb_ranker, tb_role, dynamic_cast<MapPlayerEx *>(player));
    }

    player->notify_tbattle_left_pannel();
}

// 首次进入下一层的奖励
void TrvlBattleMonitor::send_first_enter_floor_reward(TrvlBattleRole *tb_role, MapPlayerEx *player)
{
    int reward_floor = tb_role->__max_floor;

    const Json::Value &first_enter_reward_json = CONFIG_INSTANCE->tbattle_reward()["first_enter_reward"];
    JUDGE_RETURN(0 < reward_floor && reward_floor <= int(first_enter_reward_json.size()), ;);
    
    int reward_id = first_enter_reward_json[reward_floor - 1].asInt();
    JUDGE_RETURN(reward_id > 0, ;);

    // 发到背包线程，不够空间才发到公共进程发邮件
    player->send_trvl_reward_id(reward_id, SerialObj(ADD_FROM_TBATTLE_FLOOR, reward_floor), int(FONT_TBATTLE_REWARD_MAIL));
}

// 积分奖励
void TrvlBattleMonitor::check_and_send_score_reward(TrvlBattleRanker *tb_ranker, TrvlBattleRole *tb_role, MapPlayerEx *player)
{
	JUDGE_RETURN(player != NULL, ;);
    if (tb_role->__next_reward_score == 0 || tb_role->__next_reward_score <= tb_role->__last_reward_score)
    {
        const Json::Value &score_reward_json = CONFIG_INSTANCE->tbattle_reward()["score_reward"];
        JUDGE_RETURN(score_reward_json.size() > 0, ;);

        int max_reward_score = score_reward_json[score_reward_json.size() - 1][0u].asInt();
        JUDGE_RETURN(tb_role->__next_reward_score < max_reward_score, ;);

        for (uint i = 0; i < score_reward_json.size(); ++i)
        {
            if (tb_role->__last_reward_score < score_reward_json[i][0u].asInt())
            {
                tb_role->__next_reward_score = score_reward_json[i][0u].asInt();
                tb_role->__next_score_reward_id = score_reward_json[i][1u].asInt();
                break;
            }
        }
        JUDGE_RETURN(tb_role->__next_reward_score > tb_role->__last_reward_score, ;);
    }

    if (tb_ranker->__score >= tb_role->__next_reward_score)
    {
        // 找出对应的reward_id 发奖励
        int reward_id = tb_role->__next_score_reward_id;
        if (reward_id <= 0)
        {
            MSG_USER("tbattle reward score no reward %ld %s %d %d %d", tb_role->__role_id, tb_role->__role_name.c_str(), tb_ranker->__score, tb_role->__next_reward_score, reward_id);

            return;
        }

        // 发到背包线程，不够空间才发到公共进程发邮件
        player->send_trvl_reward_id(reward_id, SerialObj(ADD_FROM_TBATTLE_SCORE, tb_role->__next_reward_score), int(FONT_TBATTLE_REWARD_MAIL));
        
        tb_role->__last_reward_score = tb_role->__next_reward_score;
    }
}

// 秘宝时间到时给持有者发奖励
void TrvlBattleMonitor::send_treasure_timeout_reward(BattleGroundActor *player)
{
    int reward_id = CONFIG_INSTANCE->tbattle_reward()["treasure_reward_id"].asInt();
    JUDGE_RETURN(reward_id > 0, ;);

    MSG_USER("treasure timeout %ld %s", player->role_id(), player->role_detail().__name.c_str());

    // 发到背包线程，不够空间才发到公共进程发邮件
    player->send_trvl_reward_id(reward_id, SerialObj(ADD_FROM_TBATTLE_TREASURE), int(FONT_TBATTLE_REWARD_MAIL));

    TrvlBattleRanker *tb_ranker = this->find_tb_ranker(player->role_id());
    if (tb_ranker != NULL)
    {
        TrvlBattleRanker *indiana_ranker = this->trvl_battle_ranker_pool_->pop();
        *indiana_ranker = *tb_ranker;
        this->indiana_list_->push_back(indiana_ranker);
        indiana_ranker->__rank = this->indiana_list_->size();
    }


    int shout_id = CONFIG_INSTANCE->tbattle_reward()["treasure_shout"].asInt();
    if (shout_id > 0)
    {
        BrocastParaVec para_vec;
        GameCommon::push_brocast_para_string(para_vec, player->role_name());
        GameCommon::announce(shout_id, &para_vec);
    }
}

TrvlBattleRole *TrvlBattleMonitor::find_tb_role(const Int64 role_id)
{
    TrvlBattleRole *tb_role = NULL;
    if (this->battle_role_map_->find(role_id, tb_role) == 0)
        return tb_role;
    return NULL;
}

TrvlBattleRanker *TrvlBattleMonitor::find_tb_ranker(const Int64 role_id)
{
    TrvlBattleRanker *tb_ranker = NULL;
    if (this->cur_ranker_map_->find(role_id, tb_ranker) == 0)
        return tb_ranker;
    return NULL;
}

TrvlBattleRanker *TrvlBattleMonitor::find_tb_ranker_by_rank(const int rank)
{
    if (0 < rank && rank <= int(this->cur_ranker_list_->size()))
        return (*(this->cur_ranker_list_))[rank - 1];
    return NULL;
}

int TrvlBattleMonitor::max_floor(void)
{
    return this->max_floor_;
}

Int64 TrvlBattleMonitor::first_top_id(void)
{
    return this->first_top_id_;
}

std::string &TrvlBattleMonitor::first_top_name(void)
{
    return this->first_top_name_;
}

void TrvlBattleMonitor::set_top_player(BattleGroundActor *player)
{
    this->first_top_id_ = player->role_id();
    this->first_top_name_ = player->role_name();

    int shout_id = CONFIG_INSTANCE->tbattle_reward()["first_top_shout"].asInt();
    if (shout_id > 0)
    {
        BrocastParaVec para_vec;
        GameCommon::push_brocast_para_string(para_vec, player->role_name());
        GameCommon::announce(shout_id, &para_vec);
    }
}

Int64 TrvlBattleMonitor::treasure_owner_id(void)
{
    return this->treasure_owner_;
}

std::string &TrvlBattleMonitor::treasure_owner_name(void)
{
    return this->treasure_owner_name_;
}

int TrvlBattleMonitor::treasure_left_sec(void)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    int left_sec = this->treasure_timeout_tick_.sec() - nowtime.sec();
    if (left_sec < 0)
    	left_sec = 0;
    return left_sec;
}

void TrvlBattleMonitor::set_treasure_info(const Int64 role_id, const std::string &name, const Time_Value &check_tick)
{
    this->treasure_owner_ = role_id;
    this->treasure_owner_name_ = name;
    this->treasure_timeout_tick_ = check_tick;
}

int TrvlBattleMonitor::handle_timeout(const Time_Value &nowtime)
{
    if (this->sort_rank_tick_ <= nowtime)
    {
        this->sort_rank_tick_ = Time_Value::gettimeofday() + Time_Value(10);

        if (this->is_sort_rank_ == true)
        {
            this->handle_sort_rank();
            this->is_sort_rank_ = false;
        }
    }
    return 0;
}

void TrvlBattleMonitor::handle_sort_rank(void)
{
    std::sort(this->cur_ranker_list_->begin(), this->cur_ranker_list_->end(), trvl_battle_ranker_cmp);
    int rank = 0;
    for (TrvlBattleRankList::iterator iter = this->cur_ranker_list_->begin();
            iter != this->cur_ranker_list_->end(); ++iter)
    {
        TrvlBattleRanker *tb_ranker = *iter;
        tb_ranker->__rank = (++rank);
        tb_ranker->__rank_score = tb_ranker->__score;
    }
}

int TrvlBattleMonitor::process_fetch_every_list(const int sid, const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402501 *, request, msg, -1);

    int type = request->type();
    switch (type)
    {
        case 1: // 战场日志
        {
            Proto50401103 respond;
            this->make_up_last_rank_list(&respond, role_id, request->refresh());
            return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
        }
        case 2: // 名人堂
        {
            Proto50401104 respond;
            this->make_up_history_top_list(&respond, role_id, request->refresh());
            return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
        }
        case 3: // 当前排行
        {
            Proto50401106 respond;
            this->make_up_cur_rank_list(&respond, role_id, request->refresh());
            return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond); 
        }
        case 4: // 夺宝记录
        {
            Proto50401107 respond;
            this->make_up_indiana_list(&respond, role_id, request->refresh());
            return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
        }
        default:
            return -1;
    }
    return 0;
}

int TrvlBattleMonitor::process_fetch_view_player_info(const int sid, const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402502 *, request, msg, -1);

    Int64 rank_role_id = request->role_id();
    TrvlBattleRanker *ranker_info = NULL;
    if (request->type() == 1)
    {
        this->last_ranker_map_->find(rank_role_id, ranker_info);
    }
    else if (request->type() == 2)
    {
        int rank = request->rank();
        if (0 < rank && rank <= int(this->history_hall_of_fame_list_->size()))
        {
            ranker_info = (*(this->history_hall_of_fame_list_))[rank - 1];
        }
    }
    else
    {
        this->cur_ranker_map_->find(rank_role_id, ranker_info);
    }
    JUDGE_RETURN(ranker_info != NULL, -1);

    Proto50401105 respond;
    ranker_info->TrvlBaseRole::serialize(respond.mutable_trvl_role());

    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
}

int TrvlBattleMonitor::process_test_activity(const int sid, const Int64 role_id, Message *msg)
{
#ifndef TEST_COMMAND
    return 0;
#endif
	DYNAMIC_CAST_RETURN(Proto30402503 *, request, msg, -1);

    this->time_info_.cur_state_ = GameEnum::ACTIVITY_STATE_NO_START;
    this->time_info_.time_span_ = 3;
    int index = this->time_info_.time_index_;
    Date_Time now_date(Time_Value::gettimeofday());
    int ahead_time = now_date.weekday() * Time_Value::DAY + now_date.hour() * Time_Value::HOUR + now_date.minute() * Time_Value::MINUTE;
    this->time_info_.time_set_[index * this->time_info_.time_span_] = ahead_time;
    this->time_info_.time_set_[index * this->time_info_.time_span_ + 1] = ahead_time + request->ahead();
    this->time_info_.time_set_[index * this->time_info_.time_span_ + 2] = ahead_time + request->ahead() + request->last();

    Time_Value nowtime = Time_Value::gettimeofday();
    this->handle_battle_stage_timeout(nowtime);

    return 0;
}

int TrvlBattleMonitor::make_up_last_rank_list(Message *msg, const Int64 self_role, const int refresh)
{
    DYNAMIC_CAST_RETURN(Proto50401103 *, respond, msg, -1);

    int min_number = 0, page_size = 10;
    if (refresh == 0)
    {
        min_number = this->last_rank_list_page_[self_role];
    }
    int max_number = min_number + page_size;
    if (max_number > int(this->last_ranker_list_->size()))
        max_number = this->last_ranker_list_->size();
    this->last_rank_list_page_[self_role] = max_number;

    for (int i = min_number; i < int(this->last_ranker_list_->size()) && i < max_number; ++i)
    {
        ProtoTrvlBattleRank *proto_ranker = respond->add_rank_list();
        TrvlBattleRanker *ranker_info = (*(this->last_ranker_list_))[i];
        ranker_info->serialize(proto_ranker);
    }

    TrvlBattleRanker *self_ranker = NULL;
    if (this->last_ranker_map_->find(self_role, self_ranker) == 0)
    {
        self_ranker->serialize(respond->mutable_self_rank());
    }
    return 0;
}

int TrvlBattleMonitor::make_up_history_top_list(Message *msg, const Int64 self_role, const int refresh)
{
    DYNAMIC_CAST_RETURN(Proto50401104 *, respond, msg, -1);
    
    int min_number = 0, page_size = 10;
    if (refresh == 0)
    {
        min_number = this->history_top_list_page_[self_role];
    }
    int max_number = min_number + page_size;
    if (max_number > int(this->history_hall_of_fame_list_->size()))
        max_number = this->history_hall_of_fame_list_->size();
    this->history_top_list_page_[self_role] = max_number;

    for (int i = min_number; i < int(this->history_hall_of_fame_list_->size()) && i < max_number; ++i)
    {
        ProtoTrvlBattleRank *proto_ranker = respond->add_hall_of_fame_list();
        TrvlBattleRanker *ranker_info = (*(this->history_hall_of_fame_list_))[i];
        ranker_info->serialize(proto_ranker);
    }
    return 0;
}

int TrvlBattleMonitor::make_up_cur_rank_list(Message *msg, const Int64 self_role, const int refresh)
{
    DYNAMIC_CAST_RETURN(Proto50401106 *, respond, msg, -1);

    int min_number = 0, page_size = 10;
    if (refresh == 0)
    {
        min_number = this->cur_rank_list_page_[self_role];
    }
    int max_number = min_number + page_size;
    if (max_number > int(this->cur_ranker_list_->size()))
        max_number = this->cur_ranker_list_->size();

    for (int i = min_number; i < int(this->cur_ranker_list_->size()) && i < max_number; ++i)
    {
        TrvlBattleRanker *ranker_info = (*(this->cur_ranker_list_))[i];
        if (ranker_info->__rank == 0)
        {
            ++max_number;
            continue;
        }
        ProtoTrvlBattleRank *proto_ranker = respond->add_rank_list();
        ranker_info->serialize(proto_ranker);
    }

    this->cur_rank_list_page_[self_role] = max_number;

    TrvlBattleRanker *self_ranker = NULL;
    if (this->cur_ranker_map_->find(self_role, self_ranker) == 0)
    {
        self_ranker->serialize(respond->mutable_self_rank());
    }
    return 0;
}

int TrvlBattleMonitor::make_up_indiana_list(Message *msg, const Int64 self_role, const int refresh)
{
    DYNAMIC_CAST_RETURN(Proto50401107 *, respond, msg, -1);

    int min_number = 0, page_size = 10;
    if (refresh == 0)
    {
        min_number = this->indiana_list_page_[self_role];
    }
    int max_number = min_number + page_size;
    if (max_number > int(this->indiana_list_->size()))
        max_number = this->indiana_list_->size();
    this->indiana_list_page_[self_role] = max_number;

    for (int i = min_number; i < int(this->indiana_list_->size()) && i < max_number; ++i)
    {
        ProtoTrvlBattleRank *proto_ranker = respond->add_record_list();
        TrvlBattleRanker *ranker_info = (*(this->indiana_list_))[i];
        ranker_info->serialize(proto_ranker);
        proto_ranker->set_rank(i + 1);
    }
    return 0;
}

void TrvlBattleMonitor::copy_cur_rank_to_latest_rank(void)
{
	for (TrvlBattleRankList::iterator iter = this->indiana_list_->begin();
			iter != this->indiana_list_->end(); ++iter)
	{
		this->trvl_battle_ranker_pool_->push(*iter);
	}
	this->indiana_list_->clear();

    for (TrvlBattleRankerMap::iterator iter = this->last_ranker_map_->begin();
            iter != this->last_ranker_map_->end(); ++iter)
    {
        this->trvl_battle_ranker_pool_->push(iter->second);
    }
    this->last_ranker_map_->unbind_all();
    this->last_ranker_list_->clear();

    SAFE_DELETE(this->last_ranker_map_);
    SAFE_DELETE(this->last_ranker_list_);

    this->last_ranker_map_ = this->cur_ranker_map_;
    this->last_ranker_list_ = this->cur_ranker_list_;

    // 名人堂只保留60条记录
    while (this->history_hall_of_fame_list_->size() >= 60)
    {
        TrvlBattleRanker *info = this->history_hall_of_fame_list_->back();
        this->trvl_battle_ranker_pool_->push(info);
        this->history_hall_of_fame_list_->pop_back();
    }

    if (this->last_ranker_list_->size() > 0)
    {
        TrvlBattleRanker *first_ranker = (*(this->last_ranker_list_))[0];
        TrvlBattleRanker *top_ranker = this->trvl_battle_ranker_pool_->pop();
        *top_ranker = *first_ranker;
        this->history_hall_of_fame_list_->insert(this->history_hall_of_fame_list_->begin(), top_ranker);
    }

    for (TrvlBattleRoleMap::iterator iter = this->battle_role_map_->begin();
            iter != this->battle_role_map_->end(); ++iter)
    {
        this->trvl_battle_role_pool_->push(iter->second);
    }
    this->battle_role_map_->unbind_all();
    
    this->cur_ranker_map_ = new TrvlBattleRankerMap();
    this->cur_ranker_list_ = new TrvlBattleRankList();
}

void TrvlBattleMonitor::send_all_tbattle_res_reward(void)
{
    for (TrvlBattleRankList::iterator iter = this->cur_ranker_list_->begin(); iter != this->cur_ranker_list_->end(); ++iter)
    {
        TrvlBattleRanker *tb_ranker = *iter;

        MapPlayerEx *player = NULL;
        if (MAP_MONITOR->find_player(tb_ranker->__role_id, player) != 0 ||
                player->is_enter_scene() == false || GameCommon::is_travel_scene(player->scene_id()) == false) 
        {
            this->send_practice_reward_mail(tb_ranker->__role_id, tb_ranker);
            continue;
        }

        BasicStatus *status = NULL;
        if (player->find_status(BasicStatus::TBATTLE_TREASURE, status) == 0)
        {
        	this->send_treasure_timeout_reward(player);
        }

        player->request_add_game_resource(GameEnum::ITEM_ID_PRACTICE, tb_ranker->__score, SerialObj(ADD_FROM_TBATTLE_FINISH, tb_ranker->__rank));
        player->notify_tbattle_finish_info();
    }
}

void TrvlBattleMonitor::kick_all_online_player(void)
{
    for (TrvlBattleSceneMap::iterator iter = this->scene_map_->begin(); iter != this->scene_map_->end(); ++iter)
    {
        TrvlBattleScene *scene = iter->second;
        Scene::MoverMap &player_map = scene->player_map();
        std::vector<Int64> online_id_list;
        for (Scene::MoverMap::iterator player_iter = player_map.begin(); player_iter != player_map.end(); ++player_iter)
        {
            MapPlayerEx *player = scene->find_player(player_iter->first);
            JUDGE_CONTINUE(player != NULL);

            online_id_list.push_back(player_iter->first);
        }
        for (std::vector<Int64>::iterator id_iter = online_id_list.begin(); id_iter != online_id_list.end(); ++id_iter)
        {
            MapPlayerEx *player = scene->find_player(*id_iter);
            JUDGE_CONTINUE(player != NULL);

            player->request_exit_cur_system();
        }
    }
}

int TrvlBattleMonitor::process_fetch_main_pannel(const int sid, const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto30402504 *, request, msg, -1);

    Proto51401901 respond;
    respond.set_state((this->is_started_battle_activity() == true ? 1 : 0));
    respond.set_tbattle_value(request->tbattle_value());

    if (this->last_ranker_list_->size() > 0)
    {
        TrvlBattleRanker *tb_ranker = (*(this->last_ranker_list_))[0];
        tb_ranker->TrvlBaseRole::serialize(respond.mutable_first_player());
    }

    return MAP_MONITOR->dispatch_to_client_from_gate(sid, role_id, &respond);
}

void TrvlBattleMonitor::send_practice_reward_mail(const Int64 role_id, TrvlBattleRanker *tb_ranker)
{
    Proto30402506 inner_req;
    inner_req.set_role_id(role_id);
    inner_req.set_practice(tb_ranker->__score);

#ifdef LOCAL_DEBUG
    int gate_sid = MAP_MONITOR->first_gate_sid();
#else
    int gate_sid = MAP_MONITOR->fetch_gate_sid(tb_ranker->__server_flag);
#endif
    if (gate_sid <= 0)
    {
        MSG_USER("tbattle practice reward no gate %ld %d %s %d", role_id, tb_ranker->__score, tb_ranker->__server_flag.c_str(), gate_sid);
        return;
    }
    MSG_USER("tbattle practice reward mail %ld %d %s %d", role_id, tb_ranker->__score, tb_ranker->__server_flag.c_str(), gate_sid);

    MAP_MONITOR->dispatch_to_logic(gate_sid, &inner_req);
}

