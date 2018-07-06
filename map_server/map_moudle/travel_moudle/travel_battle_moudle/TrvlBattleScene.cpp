/*
 * File Name: TrvlBattleScene.cpp
 * 
 * Created on: 2017-04-19 19:38:51
 * Author: glendy
 * 
 * Last Modified: 2017-05-10 11:43:34
 * Description: 
 */

#include "TrvlBattleScene.h"
#include "GameAI.h"
#include "MapMonitor.h"
#include "AIManager.h"
#include "TrvlBattleMonitor.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"

void TrvlBattleScene::AIGenRecord::reset(void)
{
	this->__ai_sort = 0;
	this->__layout_index = 0;
	this->__birth_coord.reset();
	this->__gen_tick = Time_Value::zero;
}

TrvlBattleScene::TrvlBattleScene(void)
{
    this->scene_ai_map_ = new AIMap();
    this->ai_gen_rec_list_ = new AIGenRecordList();
}
TrvlBattleScene::~TrvlBattleScene(void)
{
    SAFE_DELETE(this->scene_ai_map_);
    SAFE_DELETE(this->ai_gen_rec_list_);
}

void TrvlBattleScene::reset(void)
{
    GameAI *game_ai = NULL;
    std::list<GameAI *> remove_ai_list;
    for (AIMap::iterator iter = this->scene_ai_map_->begin(); iter != this->scene_ai_map_->end(); ++iter)
    {
        remove_ai_list.push_back(iter->second);
    }
    for (std::list<GameAI *>::iterator iter = remove_ai_list.begin(); iter != remove_ai_list.end(); ++iter)
    {
        game_ai = *iter;
        game_ai->exit_scene();
        game_ai->sign_out();
    }
    this->scene_ai_map_->clear();
    this->ai_gen_rec_list_->clear();

    this->is_start_recycle_ = false;
    this->recycle_tick_ = Time_Value::zero;

    Scene::reset();
}

void TrvlBattleScene::set_scene_auto_recycle_tick(const Time_Value &delay_sec)
{
    this->is_start_recycle_ = true;
    this->recycle_tick_ = Time_Value::gettimeofday() + delay_sec;
}

void TrvlBattleScene::init_trvl_battle_scene(const int space_id)
{
    this->init_scene(space_id, GameEnum::TRVL_BATTLE_SCENE_ID);
    MAP_MONITOR->bind_scene(space_id, GameEnum::TRVL_BATTLE_SCENE_ID, this);
}

void TrvlBattleScene::init_generate_monster(void)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());

    const Json::Value &layout_list_json = scene_json["layout"];
    for (uint i = 0; i < layout_list_json.size(); ++i)
    {
        const Json::Value &layout_json = layout_list_json[i];
        int floor = layout_json["floor"].asInt();
        JUDGE_CONTINUE(floor == this->space_id());

        int ai_sort = layout_json["monster_sort"].asInt();

        IntPair monster_index(i, ai_sort);
        const Json::Value &born_coordxy_json = layout_json["born_coordxy"];
        uint born_coord_size = born_coordxy_json.size();
        for (uint j = 0; j < born_coord_size; ++j)
        {
            MoverCoord coord;
            coord.set_pixel(born_coordxy_json[j][0u].asInt(), born_coordxy_json[j][1u].asInt());
            GameAI *game_ai = AIMANAGER->generate_monster_by_scene(monster_index, coord, this);
            if (game_ai != NULL)
            {
                (*(this->scene_ai_map_))[game_ai->ai_id()] = game_ai;
                MSG_USER("trvl battle generate monster %ld %d %d %d(%d,%d)", game_ai->ai_id(), game_ai->ai_sort(), game_ai->space_id(),
                		game_ai->scene_id(),game_ai->location().pixel_x(), game_ai->location().pixel_y());
            }
        }
    }
}

void TrvlBattleScene::start_trvl_battle_scene(void)
{
    this->start_scene();
    // 初始化生成怪物
    this->init_generate_monster();
}

void TrvlBattleScene::stop_trvl_battle_scene(void)
{
    return ;
}

void TrvlBattleScene::recycle_scene(void)
{
    GameAI *game_ai = NULL;
    std::list<GameAI *> remove_ai_list;
    for (AIMap::iterator iter = this->scene_ai_map_->begin(); iter != this->scene_ai_map_->end(); ++iter)
    {
        remove_ai_list.push_back(iter->second);
    }
    for (std::list<GameAI *>::iterator iter = remove_ai_list.begin(); iter != remove_ai_list.end(); ++iter)
    {
        game_ai = *iter;
        game_ai->exit_scene();
        game_ai->sign_out();
    }
    this->scene_ai_map_->clear();
    this->ai_gen_rec_list_->clear();
    MAP_MONITOR->unbind_scene(this->space_id(), this->scene_id());
    TRVL_BATTLE_MONITOR->recycle_scene(this);
}

int TrvlBattleScene::handle_timeout(const Time_Value &nowtime)
{
    if (this->is_start_recycle_ == true)
    {
        if (this->recycle_tick_ <= nowtime)
        {
            this->is_start_recycle_ = false;
            this->recycle_tick_ = Time_Value::zero;
            this->recycle_scene();
        }
        return 0;
    }

    this->check_and_generate_monster(nowtime);

    return 0;
}

int TrvlBattleScene::check_and_generate_monster(const Time_Value &nowtime)
{
    int loop = 0;
    while (this->ai_gen_rec_list_->size() > 0 && (loop++ < 50))
    {
        AIGenRecord &record = this->ai_gen_rec_list_->front();
        if (record.__gen_tick > nowtime)
            break;

        IntPair monster_index(record.__layout_index, record.__ai_sort);
        GameAI *game_ai = AIMANAGER->generate_monster_by_scene(monster_index, record.__birth_coord, this);
        if (game_ai != NULL)
        {
            (*(this->scene_ai_map_))[game_ai->ai_id()] = game_ai;
            this->ai_gen_rec_list_->pop_front();

            MSG_USER("trvl battle generate monster %ld %d %d %d(%d,%d)", game_ai->ai_id(), game_ai->ai_sort(), game_ai->space_id(),
            		game_ai->scene_id(),game_ai->location().pixel_x(), game_ai->location().pixel_y());
        }
    }
    return 0;
}

int TrvlBattleScene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)//AI死亡处理
{
    JUDGE_RETURN(game_ai != NULL, 0);
    JUDGE_RETURN(game_ai->is_config_scene(), 0);

    MSG_USER("trvl battle kill monster %ld %d %d %d(%d,%d)", game_ai->ai_id(), game_ai->ai_sort(), game_ai->space_id(),
    		game_ai->scene_id(),game_ai->location().pixel_x(), game_ai->location().pixel_y());

    this->insert_ai_reborn_record(game_ai);

    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    int treasure_sort = tbattle_reward_json["treasure_sort"].asInt();

    if (treasure_sort != game_ai->ai_sort())
    {
        // 处理AI积分奖励和击杀数统计
        TRVL_BATTLE_MONITOR->update_info_by_kill_monster(benefited_attackor, game_ai->ai_id(), game_ai->ai_sort(), this);
    }

    MapPlayerEx *killer_player = this->find_player(benefited_attackor);
    if (killer_player != NULL)
    {
        this->handle_enter_new_floor_when_kill(killer_player);
        this->check_and_insert_treasure_buff(killer_player, game_ai);
    }
    return 0;
}

int TrvlBattleScene::handle_trvl_battle_die_player(const Int64 die_role_id, const Int64 killer_role_id)
{
    // 处理玩家积分奖励和击杀数统计,更新排行信息
    TRVL_BATTLE_MONITOR->update_info_by_kill_player(killer_role_id, die_role_id, this);

//    if (die_player != NULL)
//    {
//        // 处理退层概率
//        this->handle_back_floor_when_die(die_player);
//    }

    MapPlayerEx *killer_player = this->find_player(killer_role_id);
    if (killer_player != NULL)
    {
        // 处理进层概率
        this->handle_enter_new_floor_when_kill(killer_player);
    } 

    MapPlayerEx *die_player = this->find_player(die_role_id);
    if (die_player != NULL && killer_player != NULL)
    {
        this->check_and_transfer_treasure_buff_to_killer(killer_player, die_player);
    }
    return 0;
}

void TrvlBattleScene::insert_ai_reborn_record(GameAI *game_ai)
{
    const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());

    int reborn_sec = 300;
    if (0 <= game_ai->layout_index() && game_ai->layout_index() < int(scene_json["layout"].size()))
    {
        reborn_sec = scene_json["layout"][game_ai->layout_index()]["reborn_tick"].asInt();
    }

    AIGenRecord record;
    record.reset();
    record.__ai_sort = game_ai->ai_sort();
    record.__layout_index = game_ai->layout_index();
    record.__birth_coord = game_ai->birth_coord();
    record.__gen_tick = Time_Value::gettimeofday() + Time_Value(reborn_sec);
    this->ai_gen_rec_list_->push_back(record);
}

void TrvlBattleScene::handle_enter_new_floor_when_kill(MapPlayerEx *player)
{
    const Json::Value &killamount_up_json = CONFIG_INSTANCE->tbattle_reward()["killamount_up"];
    int cur_floor = player->space_id();
    JUDGE_RETURN(0 < cur_floor && cur_floor <= int(killamount_up_json.size()), ;);
    int need_kill_amount = killamount_up_json[cur_floor - 1].asInt();
       
    TrvlBattleRole *tb_role = TRVL_BATTLE_MONITOR->find_tb_role(player->role_id());
    JUDGE_RETURN(tb_role != NULL, ;);
    int kill_amount = tb_role->__cur_floor_kill_amount;
    JUDGE_RETURN(kill_amount >= need_kill_amount, ;);

    this->handle_player_change_floor(player, cur_floor, cur_floor + 1);
}

void TrvlBattleScene::handle_back_floor_when_die(BattleGroundActor *player)
{
    const Json::Value &die_fallback_json = CONFIG_INSTANCE->tbattle_reward()["die_fallback"];
    int cur_floor = player->space_id();
    JUDGE_RETURN(0 < cur_floor && cur_floor <= int(die_fallback_json.size()), ;);
    
    const Json::Value &die_floor_rate_json = die_fallback_json[cur_floor - 1];
    int rand_val = rand() % 10000, fall_floor = 0, total_val = 0;
    for (uint i = 0; i < die_floor_rate_json.size(); ++i)
    {
    	total_val += die_floor_rate_json[i][1u].asInt();
        if (rand_val < total_val)
        {
            fall_floor = die_floor_rate_json[i][0u].asInt();
            break;
        }
    }

    int next_floor = cur_floor - fall_floor;
    if (next_floor <= 0)
        next_floor = 1;
//    JUDGE_RETURN(next_floor != cur_floor, ;);

    this->handle_player_change_floor(player, cur_floor, next_floor, true);
}

void TrvlBattleScene::handle_player_change_floor(BattleGroundActor *player, const int cur_floor, const int change_floor, const bool is_relive)
{
    MoverCoord enter_coord = TRVL_BATTLE_MONITOR->fetch_enter_pos();

    Scene *scene = NULL;
    JUDGE_RETURN(MAP_MONITOR->find_scene(change_floor, this->scene_id(), scene) == 0, ;);

    if (TRVL_BATTLE_MONITOR->first_top_id() <= 0 && change_floor >= TRVL_BATTLE_MONITOR->max_floor())
    {
        TRVL_BATTLE_MONITOR->set_top_player(player);
    }

    TrvlBattleRole *tb_role = TRVL_BATTLE_MONITOR->find_tb_role(player->role_id());
    if (tb_role != NULL)
    {
        tb_role->__cur_floor = change_floor;
        if (cur_floor != change_floor)
        	tb_role->__cur_floor_kill_amount = 0;
        if (tb_role->__max_floor < tb_role->__cur_floor)
        {
            tb_role->__max_floor = tb_role->__cur_floor;
            TRVL_BATTLE_MONITOR->send_first_enter_floor_reward(tb_role, dynamic_cast<MapPlayerEx *>(player));
        }
    }

    if (cur_floor != change_floor)
    {
//		player->notify_exit_scene_cancel_info(EXIT_SCENE_TRANSFER);
		player->exit_scene();

		player->mover_detail().__space_id = change_floor;
		player->mover_detail().__location = enter_coord;

		if (is_relive == true)
		{
			Proto50400203 respond;
			respond.set_scene_id(player->scene_id());
			respond.set_pixel_x(enter_coord.pixel_x());
			respond.set_pixel_y(enter_coord.pixel_y());
			respond.set_space_id(player->space_id());
			respond.set_cur_blood(player->fight_detail().__blood);
			player->respond_to_client(RETURN_RELIVE, &respond);
		}
		else
		{
			Proto50400110 respond;
			respond.set_scene_id(player->scene_id());
			respond.set_pixel_x(player->location().pixel_x());
			respond.set_pixel_y(player->location().pixel_y());
			respond.set_space_id(player->space_id());
			player->respond_to_client(RETURN_TRANSFER_SCENE, &respond);
		}

		player->enter_scene();
    }
    else
    {
    	scene->refresh_mover_location_for_relive(player, enter_coord);
    }

//    player->notify_exit_scene_cancel_info(EXIT_SCENE_TRANSFER);
//    player->insert_protect_buff();

    player->notify_mover_cur_location();
    player->obtain_area_info();

    if (change_floor > cur_floor)
        player->notify_tbattle_enter_next_floor();
    else if (change_floor < cur_floor)
        player->notify_tbattle_fallback_floor();


    //player->all_beast_enter_scene();
    //player->sync_fighter_status_to_map_logic();
}

void TrvlBattleScene::check_and_insert_treasure_buff(MapPlayerEx *killer_player, GameAI *game_ai)
{
    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    int treasure_sort = tbattle_reward_json["treasure_sort"].asInt();
    JUDGE_RETURN(treasure_sort == game_ai->ai_sort(), ;);

    int treasure_get_score = tbattle_reward_json["treasure_buff_score"].asInt();
    double treasure_interval = tbattle_reward_json["treasure_buff_interval"].asDouble(),
        treasure_last = tbattle_reward_json["treasure_buff_last"].asDouble();
    if (treasure_interval < 0.000001)
        treasure_interval = 1.0;

    killer_player->refresh_treasure_prop_buff(treasure_interval, treasure_last);
    killer_player->insert_defender_status(killer_player, BasicStatus::TBATTLE_TREASURE, treasure_interval, treasure_last);


    Time_Value check_tick = Time_Value::gettimeofday() + Time_Value(treasure_last);
    TRVL_BATTLE_MONITOR->set_treasure_info(killer_player->role_id(), killer_player->role_name(), check_tick);

    TRVL_BATTLE_MONITOR->inc_score_only(killer_player, treasure_get_score);
}

void TrvlBattleScene::check_and_transfer_treasure_buff_to_killer(MapPlayerEx *killer_player, MapPlayerEx *die_player)
{
    BasicStatus *status = NULL;
    JUDGE_RETURN(die_player->find_status(BasicStatus::TBATTLE_TREASURE, status) == 0, ;);

    MSG_USER("tbattle treasure %ld %s to %ld %s, %d", die_player->role_id(), die_player->role_detail().__name.c_str(),
    		killer_player->role_id(), killer_player->role_detail().__name.c_str(), status->__last_tick.sec());

    killer_player->refresh_treasure_prop_buff(status->__interval.sec(), status->__last_tick.sec());
    killer_player->insert_defender_status(killer_player, BasicStatus::TBATTLE_TREASURE, status->__interval.sec(), status->__last_tick.sec());

    Time_Value check_tick = Time_Value::gettimeofday() + status->__last_tick;
    TRVL_BATTLE_MONITOR->set_treasure_info(killer_player->role_id(), killer_player->role_name(), check_tick);

    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    int treasure_get_score = tbattle_reward_json["treasure_buff_score"].asInt();
    TRVL_BATTLE_MONITOR->inc_score_only(killer_player, treasure_get_score);

    IntSet remove_set = die_player->tbattle_treasure_buff_set();
    die_player->tbattle_treasure_buff_set().clear();
    die_player->remove_status(status);
    for (IntSet::iterator iter = remove_set.begin();
            iter != remove_set.end(); ++iter)
    {
        die_player->remove_status(*iter);
    }

}

void TrvlBattleScene::generate_treasure_sort_immediate(void)
{
    JUDGE_RETURN(this->ai_gen_rec_list_->size() > 0, ;);

    AIGenRecord &record = this->ai_gen_rec_list_->front();

    const Json::Value &tbattle_reward_json = CONFIG_INSTANCE->tbattle_reward();
    int treasure_sort = tbattle_reward_json["treasure_sort"].asInt();
    JUDGE_RETURN(record.__ai_sort == treasure_sort, ;);

    IntPair monster_index(record.__layout_index, record.__ai_sort);
    GameAI *game_ai = AIMANAGER->generate_monster_by_scene(monster_index, record.__birth_coord, this);
    if (game_ai != NULL)
    {
        (*(this->scene_ai_map_))[game_ai->ai_id()] = game_ai;
        this->ai_gen_rec_list_->pop_front();

        MSG_USER("trvl battle generate monster %ld %d %d %d(%d,%d)", game_ai->ai_id(), game_ai->ai_sort(), game_ai->space_id(),
                game_ai->scene_id(),game_ai->location().pixel_x(), game_ai->location().pixel_y());
    }
}

int TrvlBattleScene::makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info)
{
    ProtoRoleShape *proto_shap = appear_info->mutable_shape_info();
    int mount_sort = proto_shap->mount_sort(),
        wing = proto_shap->wing(),
        god_weapon = proto_shap->god_weapon(),
        tian_gang = proto_shap->tian_gang(),
        sword_pool = proto_shap->sword_pool();
    proto_shap->Clear();
    if (mount_sort > 0)
        proto_shap->set_mount_sort(1);
    if (wing > 0)
        proto_shap->set_wing(1);
    if (god_weapon > 0)
        proto_shap->set_god_weapon(1);
    if (tian_gang > 0)
        proto_shap->set_tian_gang(1);
    if (sword_pool > 0)
        proto_shap->set_sword_pool(1);

    proto_shap->set_sex(player->role_detail().__sex);
    proto_shap->set_career(player->role_detail().__career);
    appear_info->set_league_id(0);
    appear_info->set_league_name("");
    appear_info->set_partner_id(0);
    appear_info->set_partner_name("");
    appear_info->set_wedding_id(0);
    appear_info->set_wedding_type(0);
    appear_info->set_mw_id(0);
    appear_info->set_mw_rank_level(0);

    appear_info->set_name(CONFIG_INSTANCE->const_set_conf("secret_name")["strValue"].asString());
    appear_info->set_full_name(appear_info->name());

    return 0;
}

