/*
 * GameAI.cpp
 *
 * Created on: 2013-04-02 11:18
 *     Author: lyz
 */

#include "GameAI.h"
#include "Scene.h"

#include "AIBTCName.h"
#include "AIManager.h"
#include "MapPlayerEx.h"
#include "MapMonitor.h"
#include "GameConfig.h"
#include "BTCFactory.h"
#include "ProtoDefine.h"
#include "ScriptAI.h"
#include "BaseScript.h"

int GameAI::GatherLimitTimer::type(void)
{
	return GTT_MAP_ONE_SECOND;
}

int GameAI::GatherLimitTimer::handle_timeout(const Time_Value &tv)
{
	return game_ai_->gather_limit_timeout();
}

GameAI::GameAI(void)
{
    this->ai_id_ = MAP_MONITOR->generate_ai_id();
    this->gather_timer_.game_ai_= this;

	this->behavior_tree_ 	= NULL;
    this->is_has_drop_ 		= false;
}

GameAI::~GameAI(void)
{
}

const Json::Value& GameAI::monster_conf()
{
	return CONFIG_INSTANCE->monster(this->ai_sort());
}

const Json::Value& GameAI::prop_monster()
{
	return CONFIG_INSTANCE->prop_monster(this->ai_sort());
}

int GameAI::auto_action_timeout(const Time_Value &nowtime)
{
    this->execute_ai_tree();
    JUDGE_RETURN(this->is_active() == true, -1);

	AutoMapFighter::auto_action_timeout(nowtime);
    this->monitor()->ai_total_use_ += (Time_Value::gettimeofday() - nowtime);

    return 0;
}

int GameAI::fetch_default_step()
{
	return 0;
}

Int64 GameAI::league_id(void)
{
	return this->ai_detail_.league_index_;
}

int GameAI::fighter_sort()
{
	return this->ai_detail_.__monster_type;
}

const char* GameAI::name()
{
	return this->ai_detail_.__name.c_str();
}

void GameAI::push_attack_interval()
{
	this->push_schedule_time(this->ai_detail_.__attack_interval);
}

void GameAI::trigger_left_blood_skill(Int64 attackor)
{
}

int GameAI::fetch_mover_volume()
{
	return this->ai_detail_.__volume;
}

int GameAI::fetch_max_chase_distance()
{
	int max_chase_distance = AutoMapFighter::fetch_max_chase_distance();

	const Json::Value &prop_json = this->fetch_prop_config();
	if (prop_json.isMember(AIBTCName::MAX_CHASE_RADIUS_FIELD))
	{
		max_chase_distance = prop_json[AIBTCName::MAX_CHASE_RADIUS_FIELD].asInt();
	}
	else if (max_chase_distance < this->fetch_select_distance())
	{
		max_chase_distance = this->fetch_select_distance();
	}

	return max_chase_distance;
}

int GameAI::fetch_auto_skill_id()
{
	int left_blood_skill = this->fetch_left_blood_skill();
	JUDGE_RETURN(left_blood_skill == 0, left_blood_skill);

	return AutoMapFighter::fetch_auto_skill_id();
}

int GameAI::auto_modify_around_blood(const Time_Value &now_time)
{
	JUDGE_RETURN(this->ai_detail_.around_blood_dis_ > 0, 0);
	JUDGE_RETURN(this->ai_detail_.around_use_tick_ <= now_time.sec(), 0);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, 0);

	Scene::MoverMap player_map;
	scene->fetch_all_around_player(this, player_map, this->location(),
			this->ai_detail_.around_blood_dis_);

	for (Scene::MoverMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
        GameFighter *fighter = dynamic_cast<GameFighter *>(iter->second);
        JUDGE_CONTINUE(GameCommon::validate_fighter(fighter) == true);
        JUDGE_CONTINUE(this->validate_attack_to_defender(fighter, this->base_skill_id()) == 0);
        this->direct_hurt_by_defender_max_blood(fighter, -1 * this->ai_detail_.around_blood_per_);
	}

	this->ai_detail_.around_use_tick_ = now_time.sec()
			+ this->ai_detail_.around_blood_span_;
	return 0;
}

int GameAI::fetch_left_blood_skill()
{
	JUDGE_RETURN(this->is_death() == false, 0);
	JUDGE_RETURN(this->is_blood_full() == false, 0);

	AIDetail& detail = this->ai_detail();
	JUDGE_RETURN(detail.left_blood_skill_.empty() == false, 0)

	int total_size = detail.left_blood_skill_.size();
	int left_percent = this->cur_blood_percent(GameEnum::DAMAGE_ATTR_PERCENT);

	for (int i = 0; i < total_size; ++i)
	{
		ThreeObj& obj = detail.left_blood_skill_[i];
		JUDGE_CONTINUE(obj.sub_ == 0);
		JUDGE_CONTINUE(left_percent < obj.value_);

		obj.sub_ = 1;
		return obj.id_;
	}

	return 0;
}

void GameAI::trigger_ice_inside_skill(FighterSkill* skill)
{
	JUDGE_RETURN(skill != NULL, ;);

	const Json::Value& trigger_conf = skill->detail()["effect"][0u];
	int index = std::rand() % trigger_conf["center"].size();

	int pixel_x = trigger_conf["center"][index][0u].asInt();
	int pixel_y = trigger_conf["center"][index][1u].asInt();

	BasicStatus status(trigger_conf["client_buff"].asInt());
	status.set_all(Time_Value(trigger_conf["last"].asInt()), Time_Value(1),	this->fighter_id(),
			skill->__skill_id, skill->__level, pixel_x, pixel_y, 0, trigger_conf["hurt_start"].asInt());

	this->insert_defender_status(this, status);
}

void GameAI::trigger_jian_drop_skill(FighterSkill* skill, Int64 attackor)
{
	JUDGE_RETURN(skill != NULL, ;);

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, ;);

	MapPlayerEx* player = scene->fetch_benefited_player(attackor);
	JUDGE_RETURN(player != NULL, ;);

	MoverCoord coord = this->location();
	float delta_x = player->location().pixel_x() - coord.pixel_x();
	float detla_y = player->location().pixel_y() - coord.pixel_y();

	const Json::Value& trigger_conf = skill->detail()["effect"][0u];
	JUDGE_RETURN(trigger_conf["coord"].size() > 0, ;);

	BasicStatus status(trigger_conf["client_buff"].asInt());
	int use_times = this->ai_detail_.__left_blood_skill_use[skill->__skill_id]++;
	int cur_index = use_times % trigger_conf["coord"].size();

	const Json::Value& coord_conf = trigger_conf["coord"][cur_index];
	for (uint j = 0; j < coord_conf.size(); ++j)
	{
		float length = coord_conf[j][0u].asInt() * 30;
		float angle = coord_conf[j][1u].asInt();
		float theta = std::atan2(detla_y, delta_x) + PI * angle / 180.0;

		float x = coord.pixel_x() + length * std::cos(theta);
		float y = coord.pixel_y() + length * std::sin(theta);

		status.__value6.push_back(x);
		status.__value6.push_back(y);
	}

//	status.set_client_status(trigger_conf["client_buff"].asInt());
	status.set_all(Time_Value(trigger_conf["last"].asInt()), Time_Value(1),
			this->fighter_id(), skill->__skill_id, skill->__level,
			trigger_conf["r"].asInt(), 0, 0, trigger_conf["hurt_start"].asInt());

	this->insert_defender_status(this, status);
}

Int64 GameAI::ai_id()
{
    return this->ai_id_;
}

AIDetail &GameAI::ai_detail(void)
{
    return this->ai_detail_;
}

AIDetail::PlayerHurtMap &GameAI::hurt_map(void)
{
    return this->ai_detail_.__hurt_map;
}

bool GameAI::is_gather_goods(void)
{
	return this->ai_detail_.__monster_type == GameEnum::FIGHTER_GATHER;
}

bool GameAI::has_back_action(void)
{
    return (this->behavior_tree_->test_ation_type_flag(GameEnum::AAT_BACK) == true ||
            this->behavior_tree_->test_ation_type_flag(GameEnum::AAT_IDLE_BACK) == true);
}

int64_t GameAI::entity_id(void)
{
    return this->ai_id();
}

MoverCoord &GameAI::birth_coord(void)
{
    return this->ai_detail_.__birth_coord;
}

bool GameAI::is_config_scene(void)
{
    return this->ai_detail_.__config_type == GameEnum::AI_CT_SCENE;
}

bool GameAI::is_config_monster(void)
{
    return this->ai_detail_.__config_type == GameEnum::AI_CT_MONSTER;
}

const Json::Value &GameAI::fetch_prop_config(void)
{
    if (this->is_config_scene() == false)
    {
        return this->prop_monster();
    }
    else
    {
        return this->fetch_layout_item();
    }
}

void GameAI::set_chase_index(const int index)
{
    this->ai_detail_.__chase_index = index;
}

int GameAI::chase_index(void)
{
    return this->ai_detail_.__chase_index;
}

MoverCoord GameAI::chase_coord(const MoverCoord src_coord)
{
    MoverCoord chase = src_coord, cur_coord = src_coord;
    DynamicMoverCoord d_src_coord(src_coord.pixel_x(), src_coord.pixel_y()), tmp_coord;

    const int MAX_DISTANCE = GameEnum::DEFAULT_USE_SKILL_DISTANCE + 1, MIN_DISTANCE = 1;

    int chase_distance = INT_MAX, cur_distance = 0, grid_size = GameEnum::DEFAULT_AI_PATH_GRID;

    for (int distance = MIN_DISTANCE; distance < MAX_DISTANCE; ++distance)
    {
    	for (int x = -distance; x <= distance; ++x)
    	{
			for (int y = -distance; y <= distance; ++y)
			{
	    		if (std::abs(x) < MIN_DISTANCE && std::abs(y) < MIN_DISTANCE)
	    			continue;
                tmp_coord.set_dynamic_pixel(d_src_coord.pixel_x() + x * grid_size, d_src_coord.pixel_y() + y * grid_size);
				if (this->is_movable_path_coord(tmp_coord) == true)
				{
					cur_coord.set_pixel(tmp_coord.pixel_x(), tmp_coord.pixel_y());
					if (GameCommon::is_movable_coord_no_border(this->scene_id(), cur_coord) == true)
					{
						cur_distance = coord_offset_grid(cur_coord, this->location());
						if (cur_distance < chase_distance)
						{
							chase = cur_coord;
							chase_distance = cur_distance;
						}
					}
				}
			}
    	}
    	if (chase_distance < INT_MAX)
    		return chase;
    }

    return chase;
}

void GameAI::set_patrol_index(const int index)
{
    this->ai_detail_.__patrol_index = index;
}

int GameAI::patrol_index(void)
{
    return this->ai_detail_.__patrol_index;
}

void GameAI::set_moveto_action_coord(const MoverCoord coord)
{
    this->ai_detail_.__moveto_action_coord.set_pixel(coord.pixel_x(), coord.pixel_y());
}

MoverCoord &GameAI::moveto_action_coord(void)
{
    return this->ai_detail_.__moveto_action_coord;
}

void GameAI::set_aim_object(Int64 aim_id, const bool is_from_group)
{
    Int64 prev_aim_object_id = this->aim_object_id();
    AutoMapFighter::set_aim_object(aim_id);

    JUDGE_RETURN(prev_aim_object_id != aim_id && is_from_group == false && aim_id > 0, ;);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, ;);

    BLongSet *ai_set = scene->ai_group_set(this->group_id());
    JUDGE_RETURN(ai_set != 0, ;);

    for (BLongSet::iterator iter = ai_set->begin(); iter != ai_set->end(); ++iter)
    {
        GameFighter *fighter = 0;
        JUDGE_CONTINUE(scene->find_fighter(*iter, fighter) == 0);
        JUDGE_CONTINUE(fighter->is_death() == false);

        GameAI *game_ai = dynamic_cast<GameAI *>(fighter);
        JUDGE_CONTINUE(game_ai != NULL);
        JUDGE_CONTINUE(game_ai->aim_object_id() == 0);

        game_ai->set_aim_object(aim_id, true);
    }
}

int GameAI::ai_sort()
{
	return this->ai_detail_.__sort;
}

bool GameAI::is_boss(void)
{
    return this->ai_detail_.__monster_type == GameEnum::FIGHTER_BOSS;
}

double GameAI::hit_stiff_last(void)
{
    return this->ai_detail_.__hit_stiff_last;
}

int GameAI::gate_sid(void)
{
	return -1;
}

int GameAI::client_sid(void)
{
	return -1;
}

void GameAI::reset(void)
{
	this->is_has_drop_ = false;
    this->ai_detail_.reset();
    this->reset_auto_fighter();
	this->gather_timer_.cancel_timer();

	BTCFACTORY->push_ai_tree(this->tree_name_, this->behavior_tree_);
	this->tree_name_.clear();
	this->behavior_tree_ = NULL;
}

int GameAI::team_id(void)
{
    return 0;
}

int GameAI::produce_drop_package()
{
    this->produce_drop_package_b();
    JUDGE_RETURN(this->is_has_drop_ == true, -1);

	// 杀死怪物的玩家会听到掉落声音
	MapPlayerEx* palyer = this->find_player(this->ai_detail_.__killed_id);
	JUDGE_RETURN(palyer != NULL, -1);

	Proto80400231 respond;
	respond.set_mover_id(this->mover_id());
	return palyer->respond_to_client(ACTIVE_AI_DROP, &respond);
}

int GameAI::produce_drop_package_a(const Json::Value& monster_drop_item, LongSet &coord_set, int *drop_size)
{
	return 0;
}

int GameAI::produce_drop_package_b()
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	const Json::Value &monster_conf = this->monster_conf();
	JUDGE_RETURN(monster_conf.isMember("drop_reward") == true, -1);

	int reward_id = monster_conf["drop_reward"].asInt();
	const Json::Value &reward_json = CONFIG_INSTANCE->reward(reward_id);
	JUDGE_RETURN(reward_json.empty() == false, -1);

	int scene_id = scene->scene_id();
    int drop_type = monster_conf["drop_type"].asInt();

    LongVec player_vec;
	switch (drop_type)
    {
    case GameEnum::DROP_TYPE_ALL_HAVE_PICKUP:
    {
        scene->fetch_player_set(player_vec);
    	break;
    }
    default:
    {
    	player_vec.push_back(this->ai_detail_.__killed_id);
    	break;
    }
    }

    for (LongVec::iterator player_iter = player_vec.begin();
    		player_iter != player_vec.end(); ++player_iter)
    {
    	MapPlayer* player = this->find_player(*player_iter);
    	JUDGE_CONTINUE(player != NULL);

    	RewardInfo reward_info(false, player);
    	GameCommon::make_up_reward_items(reward_info, reward_json);

		for (ItemObjVec::iterator item_iter = reward_info.item_vec_.begin();
				item_iter != reward_info.item_vec_.end(); ++item_iter)
		{
			const ItemObj& obj = *item_iter;
			JUDGE_CONTINUE(MAP_MONITOR->check_add_item_drop_limit(
					player->role_id(), scene_id, obj) == false);
			this->produce_drop_items_b(scene, *player_iter, obj);
		}
    }

	return 0;
}

int GameAI::fetch_drop_player_map(int drop_type, LongMap& player_map, const Json::Value &monster_drop_item)
{
	JUDGE_RETURN(drop_type != GameEnum::DROP_TYPE_ALL, -1);

	switch (drop_type)
	{
	case GameEnum::DROP_TYPE_PERSON:
	{
		MapPlayerEx* player = this->find_player_with_offline(this->fetch_killed_id());
		JUDGE_RETURN(player != NULL, -1);

		if (monster_drop_item["team_share"].asInt() == 1)
		{
			MapTeamInfo& team_info = player->team_info();
			player_map = team_info.teamer_set_;

            for (LongMap::iterator iter = team_info.teamer_set_.begin(); 
                    iter != team_info.teamer_set_.end(); ++iter)
            {
                this->ai_detail_.__drop_owner_set.insert(iter->first);
            }
		}

        this->ai_detail_.__drop_owner_set.insert(player->role_id());
		player_map[player->role_id()] = player->role_id();
		break;
	}
	case GameEnum::DROP_TYPE_MAX_HURT_PERSON:
	{
		Int64 max_hurt_role_id = this->find_max_hurt_fighter();
		MapPlayerEx* player = this->find_player_with_offline(max_hurt_role_id);
		JUDGE_RETURN(player != NULL, -1);

		if (monster_drop_item["team_share"].asInt() == 1)
		{
			MapTeamInfo& team_info = player->team_info();
			player_map = team_info.teamer_set_;

            for (LongMap::iterator iter = team_info.teamer_set_.begin(); 
                    iter != team_info.teamer_set_.end(); ++iter)
            {
                this->ai_detail_.__drop_owner_set.insert(iter->first);
            }
		}

        this->ai_detail_.__drop_owner_set.insert(player->role_id());
		player_map[player->role_id()] = player->role_id();
		break;
	}
	case GameEnum::DROP_TYPE_TEAM:
	{
		MapPlayerEx* player = this->find_player_with_offline(this->fetch_killed_id());
		JUDGE_RETURN(player != NULL, -1);

		MapTeamInfo& team_info = player->team_info();
		player_map = team_info.teamer_set_;

        for (LongMap::iterator iter = team_info.teamer_set_.begin(); 
                iter != team_info.teamer_set_.end(); ++iter)
        {
            this->ai_detail_.__drop_owner_set.insert(iter->first);
        }

        this->ai_detail_.__drop_owner_set.insert(player->role_id());
		player_map[player->role_id()] = player->role_id();
		break;
	}
    case GameEnum::DROP_TYPE_RANDOM_HURT:
    {
    	LongVec role_vc;
        if (monster_drop_item.isMember("hurt_range") == false)
        	role_vc.push_back(this->find_max_hurt_fighter());
        else
        	this->fetch_hurt_range_role(role_vc, monster_drop_item["hurt_range"][0u].asInt(), monster_drop_item["hurt_range"][1u].asInt());

        if (role_vc.size() == 0)
            break;

        int loop = 0;
        MapPlayerEx* player = NULL;
        while (player == NULL && (++loop) < 50)
        {
			int index = std::rand() % role_vc.size();
			Int64 hurt_id = role_vc[index];
			player = this->find_player_with_offline(this->fetch_benefited_attackor_id(hurt_id));
			if (player != NULL)
				break;
        }
        JUDGE_RETURN(player != NULL, -1);

		if (monster_drop_item["team_share"].asInt() == 1)
		{
			MapTeamInfo& team_info = player->team_info();
			player_map = team_info.teamer_set_;

            for (LongMap::iterator iter = team_info.teamer_set_.begin(); 
                    iter != team_info.teamer_set_.end(); ++iter)
            {
                this->ai_detail_.__drop_owner_set.insert(iter->first);
            }
		}

        this->ai_detail_.__drop_owner_set.insert(player->role_id());
		player_map[player->role_id()] = player->role_id();
        break;
    }
	}
	return 0;
}

Int64 GameAI::fetch_killed_id(void)
{
	return this->ai_detail_.__killed_id;
}

std::string GameAI::tree_name()
{
	return this->tree_name_;
}

std::string GameAI::born_type()
{
	return this->ai_detail_.__born_type;
}

const Json::Value& GameAI::fetch_layout_item()
{
	if (this->ai_detail_.__layout_index >= 0)
	{
		int layout_index = this->ai_detail_.__layout_index;
		const Json::Value &scene_json = CONFIG_INSTANCE->scene(this->scene_id());
		if (layout_index < int(scene_json["layout"].size()))
			return scene_json["layout"][layout_index];
	}

	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, Json::Value::null);

	return scene->layout_sort(this->ai_sort());
}

int GameAI::enter_scene(const int type)
{
	if (AutoMapFighter::enter_scene(type) != 0)
	{
        return -1;
	}

	this->start_auto_action();
    return 0;
}

int GameAI::exit_scene(const int type)
{
	this->stop_auto_action();
	this->notify_exit_scene_cancel_info(type);
	AutoMapFighter::exit_scene(type);

    return 0;
}

int GameAI::sign_in_with_scene(int monster_sort, const MoverCoord &coord, Scene* scene)
{
	JUDGE_RETURN(scene != NULL, -1);

    this->ai_detail_.__config_type = GameEnum::AI_CT_SCENE;
	this->base_sign_in(monster_sort, coord, scene);
    this->init_base_property();

    AutoMapFighter::sign_in();
	this->init_scene_monster();

	return 0;
}

int GameAI::sign_in_with_sort(int monster_sort, const MoverCoord &coord, Scene* scene)
{
	JUDGE_RETURN(scene != NULL, -1);
    
    this->ai_detail_.__config_type = GameEnum::AI_CT_MONSTER;
	this->base_sign_in(monster_sort, coord, scene);
    this->init_base_property();

    AutoMapFighter::sign_in();
    this->init_prop_monster();

	return 0;
}

int GameAI::sign_out(void)
{
	AutoMapFighter::sign_out();
	this->clean_when_recycle();
    return this->recycle_self();
}

void GameAI::clean_when_recycle(void)
{
	this->gather_timer_.cancel_timer();

    GameFighter *caller = 0;
    if (this->caller() > 0 && this->find_fighter(this->caller(), caller) == 0)
    {
        caller->remove_summon_ai(this->ai_id());
    }
}


int GameAI::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
    Proto80400111 appear_info;
    appear_info.set_ai_id(this->ai_id());

    if (this->ai_detail_.__view_sort > 0)
    {
        appear_info.set_ai_sort(this->ai_detail_.__view_sort);
    }
    else
    {
        appear_info.set_ai_sort(this->ai_sort());
    }

    appear_info.set_speed(this->speed_total_i());
    appear_info.set_toward(this->mover_detial_.__toward);
    appear_info.set_ai_blood(this->fight_detail_.__blood);
    appear_info.set_fight_state((this->is_fight_state() ? 1 : 0));
    appear_info.set_scene_id(this->scene_id());
    appear_info.set_pixel_x(this->mover_detial_.__location.pixel_x());
    appear_info.set_pixel_y(this->mover_detial_.__location.pixel_y());
    appear_info.set_space_id(this->space_id());
    appear_info.set_max_blood(this->fight_detail_.__blood_total_i(this));

    Scene* scene = this->fetch_scene();
    if (scene != NULL)
    {
    	scene->makeup_ai_appear_info(this, &appear_info);
    }

    ProtoClientHead head;
    head.__recogn = ACTIVE_MONSTER_APPEAR;
    return this->make_up_client_block(buff, &head, &appear_info);
}

int GameAI::recycle_self(void)
{
	this->gather_timer_.cancel_timer();

    GameFighter *caller = 0;
    if (this->caller() > 0 && this->find_fighter(this->caller(), caller) == 0)
    {
        caller->remove_summon_ai(this->ai_id());
    }

	return AI_PACKAGE->unbind_and_push(this->ai_id(), this);
}

int GameAI::execute_ai_tree()
{
	JUDGE_RETURN(this->behavior_tree_ != NULL, -1);

	NodeOutputParam input;
	input.game_ai_ = this;

	JUDGE_RETURN(this->behavior_tree_->Evaluate(input) == true, -1);

	NodeOutputParam output;
	output.game_ai_ = this;
	this->behavior_tree_->Tick(output, input);

	return 0;
}

int GameAI::add_boss_avoid_buff()
{
	JUDGE_RETURN(this->is_boss() == true, -1);

    static int BOSS_AVOID_BUFF[] = {BasicStatus::DIZZY,
    		BasicStatus::DIRECTMAXBLOOD, BasicStatus::SPEED};
    static int TOTAL_BUFF = ARRAY_LENGTH(BOSS_AVOID_BUFF);

	for (int i = 0; i < TOTAL_BUFF; ++i)
	{
		this->ai_detail_.__avoid_buff_set.insert(BOSS_AVOID_BUFF[i]);
	}

	this->set_ai_falg(GameEnum::AI_CF_NO_PUSH);
	return 0;
}

int GameAI::init_left_blood_skill()
{
	const Json::Value &monster_json = this->monster_conf();
	JUDGE_RETURN(monster_json.isMember("left_blood_skill") == true, -1);

	int total_size = monster_json["left_blood_skill"].size();
	for (int i = 0; i < total_size; ++i)
	{
		const Json::Value& left_json = monster_json["left_blood_skill"][i];
		this->insert_skill(left_json[0u].asInt(), 1);
		this->ai_detail_.left_blood_skill_.push_back(ThreeObj(
				left_json[0u].asInt(), left_json[1u].asInt()));
	}

	return 0;
}

void GameAI::base_sign_in(int monster_sort, const MoverCoord &coord, Scene* scene)
{
	this->ai_detail_.__sort = monster_sort;
    this->ai_detail_.__birth_coord = coord;

	this->mover_detial_.__location = coord;
	this->init_mover_scene(scene);

    const Json::Value& prop_config = this->fetch_prop_config();
    this->ai_detail_.__born_type = prop_config["born_type"].asString();
}

void GameAI::init_base_property(void)
{
	const Json::Value &monster_json = this->monster_conf();

	int blood 		= monster_json["hp"].asInt();
	int magic 		= monster_json["magic"].asInt();
	int attack 		= std::max<int>(monster_json["attack"].asInt(), 1);
	int defence 	= std::max<int>(monster_json["defense"].asInt(), 1);
	int toughness	= monster_json["though"].asInt();
	int crit 	= monster_json["crit"].asInt();
	int hit 	= monster_json["hit"].asInt();
	int avoid  	= monster_json["avoid"].asInt();
	int level 	= monster_json["lvl"].asInt();
	int speed 	= monster_json["spd"].asDouble();

	this->fight_detail_.__level = (level > 0 ? level : 1);
	this->fight_detail_.__blood_max.set_single(blood > 0 ? blood : 1000);
	this->fight_detail_.__magic_max.set_single(magic > 0 ? magic : 1000);
	this->fight_detail_.__attack_upper.set_single(attack);
	this->fight_detail_.__attack_lower.set_single(attack);
	this->fight_detail_.__defence_upper.set_single(defence);
	this->fight_detail_.__defence_lower.set_single(defence);
    this->fight_detail_.__toughness.set_single(toughness > 0 ? toughness : 1);
    this->fight_detail_.__crit.set_single(crit > 0 ? crit : 1);
    this->fight_detail_.__hit.set_single(hit > 0 ? hit : 1);
    this->fight_detail_.__avoid.set_single(avoid > 0 ? avoid : 1);
	this->mover_detial_.__speed.set_single(speed > 0 ? speed : 100.0);
    this->mover_detial_.__toward = ::rand() % 180;

    this->fight_detail_.__blood = this->fight_detail_.__blood_total_i(this);
    this->fight_detail_.__magic = this->fight_detail_.__magic_total_i(this);
    this->ai_detail_.__name = monster_json["name"].asString();
    this->ai_detail_.__is_remote = monster_json["remote"].asInt();
    this->ai_detail_.__volume = monster_json["volume"].asInt();
    this->ai_detail_.__full_screen_chase = monster_json["full_screen_chase"].asInt();
    this->ai_detail_.__born_span = monster_json["born_span"].asInt();
    this->ai_detail_.__force_hurt = monster_json["force_hurt"].asInt();

    this->init_klv_property();
    this->set_fighter_sort();
    this->set_pk_state(PK_PLENARY);
    this->add_boss_avoid_buff();

    double attack_interval = monster_json["interval"].asInt() - 0.3;
    this->ai_detail_.__attack_interval = std::max<double>(0.7, attack_interval);

//    if (monster_json.isMember("hit_stiff"))
//    {
//        this->ai_detail_.__hit_stiff_last = monster_json["hit_stiff"].asDouble();
//    }

    if (monster_json.isMember("avoid_buff"))
    {
    	const Json::Value &avoid_buff_json = monster_json["avoid_buff"];
    	GameCommon::json_to_int_set(this->ai_detail_.__avoid_buff_set, avoid_buff_json);
    }

    if (monster_json["avoid_push"].asInt() == 1)
    {
        this->set_ai_falg(GameEnum::AI_CF_NO_PUSH);
    }

    //自动回血设置
    if (monster_json.isMember("recover_blood"))
    {
    	this->fight_detail_.__auto_recover_blood = 1;
    	this->fight_detail_.__recover_blood_per = monster_json["recover_blood"].asInt();
    	this->fight_detail_.__recover_blood_span = monster_json["recover_blood_span"].asInt();
    }

    //自动扣周围玩家血设置
    if (monster_json.isMember("around_blood_dis"))
    {
    	this->ai_detail_.around_blood_dis_ = monster_json["around_blood_dis"].asInt();
    	this->ai_detail_.around_blood_span_ = monster_json["around_blood_span"].asInt();
    	this->ai_detail_.around_blood_per_ = monster_json["around_blood_per"].asInt();
    }
}

void GameAI::init_ai_tree(const Json::Value& prop_monster)
{
	this->tree_name_ = prop_monster["monster_tree"].asString();
	JUDGE_RETURN(this->tree_name_.empty() == false, ;);

	this->behavior_tree_ = BTCFACTORY->pop_ai_tree(this->tree_name_);
	this->ai_detail_.__auto_attack_tick.clear();
	this->ai_detail_.__born_tick = Time_Value::gettimeofday();
	this->ai_detail_.__next_interval = Time_Value::gettimeofday();

	int interval = std::max<int>(prop_monster["special_interval"].asInt(), 1);
	this->ai_detail_.__interval = Time_Value(interval);
	JUDGE_RETURN(prop_monster.isMember("recycle_tick") == true, ;);

	int recycle_tick = prop_monster["recycle_tick"].asInt();
	this->ai_detail_.__recycle_tick = this->ai_detail_.__born_tick + Time_Value(recycle_tick);
}

void GameAI::init_flag_property(const Json::Value& prop_monster)
{
	JUDGE_RETURN(prop_monster.isMember("flag_set") == true, ;);

	for (uint i = 0; i < prop_monster["flag_set"].size(); ++i)
	{
		std::string flag_name = prop_monster["flag_set"][i].asString();
		this->set_ai_falg(AIMANAGER->fetch_ai_flag(flag_name));
	}
}

void GameAI::init_skill_property(const Json::Value& prop_monster)
{
	int attack_dis = 0;
	if (prop_monster.isMember("attack_distance") == true)
	{
		attack_dis = prop_monster["attack_distance"].asInt();
	}
	else if (this->ai_detail_.__is_remote == false)
	{
		attack_dis = CONFIG_INSTANCE->const_set("near_attack_dis");
	}
	else
	{
		attack_dis = CONFIG_INSTANCE->const_set("remote_attack_dis");
	}

	int select_dis = 0;
	const Json::Value& monster_conf = this->monster_conf();
	if (monster_conf.isMember(AIBTCName::SELECT_C_RADIUS_FIELD) == true)
	{
		select_dis = monster_conf[AIBTCName::SELECT_C_RADIUS_FIELD].asInt();
	}
	else if (prop_monster.isMember(AIBTCName::SELECT_C_RADIUS_FIELD) == true)
	{
		select_dis = prop_monster[AIBTCName::SELECT_C_RADIUS_FIELD].asInt();
	}
	else
	{
		select_dis = CONFIG_INSTANCE->const_set("ai_select_dis");
	}

	this->auto_detail_.attack_distance_ = attack_dis + this->fetch_mover_volume();
	this->auto_detail_.select_distance_ = select_dis + this->fetch_mover_volume();

	if (prop_monster.isMember("skill_set") == true)
	{
		for (uint i = 0; i < prop_monster["skill_set"].size(); ++i)
		{
			int skill_id = prop_monster["skill_set"][i].asInt();
			this->insert_skill(skill_id);
		}
	}
	else if (prop_monster.isMember("skill_sequence") == true)
	{
		this->auto_detail_.fetch_skill_mode_ = AutoFighterDetail::SKILL_MOCE_SEQUENCE;
		for (uint i = 0; i < prop_monster["skill_sequence"].size(); ++i)
		{
			int skill_id = prop_monster["skill_sequence"][i].asInt();
			this->insert_sequence_skill(skill_id);
		}
	}

	this->init_left_blood_skill();
}

// 目前只有EffectAI才有该安全区域功能
void GameAI::init_safe_area(const Json::Value &prop_monster)
{
    return;
}

void GameAI::init_group_id(const Json::Value &prop_monster)
{
    this->set_group_id(prop_monster["group"].asInt());
}

void GameAI::fetch_drop_money_amount(int *money_type, int *money_amount, const Json::Value &money_json)
{
    JUDGE_RETURN(money_json.size() >= 2, ;);

	*money_type = money_json[0u].asInt();
	JUDGE_RETURN(GameCommon::validate_money_type(*money_type) == true, ;);

	*money_amount = money_json[1u].asInt();
}

void GameAI::produce_drop_money(const Json::Value &monster_drop, LongMap& player_map, LongSet &coord_set, int *drop_size)
{
    if (monster_drop.isMember("money_drop_rate"))
    {
        int rate = monster_drop["money_drop_rate"].asDouble() * 100;
        int rand_val = rand() % 10000;
        JUDGE_RETURN(rand_val < rate, ;);
    }

    std::vector<int> money_type_vc, money_amount_vc;

    int money_type = 0, money_amount = 0;
    if (monster_drop.isMember("money_more"))
    {
        for (uint i = 0; i < monster_drop["money_more"].size(); ++i)
        {
            money_type = 0;
            money_amount = 0;
            this->fetch_drop_money_amount(&money_type, &money_amount, monster_drop["money_more"][i]);
            if (money_amount > 0)
            {
                money_type_vc.push_back(money_type);
                money_amount_vc.push_back(money_amount);
            }
        }
    }
    else if (monster_drop.isMember("money"))
    {
        this->fetch_drop_money_amount(&money_type, &money_amount, monster_drop["money"]);
        if (money_amount > 0)
        {
            money_type_vc.push_back(money_type);
            money_amount_vc.push_back(money_amount);
        }
    }

	JUDGE_RETURN(money_amount_vc.size() > 0, ;);

    for (uint money_index = 0; money_index < money_amount_vc.size(); ++money_index)
    {
        JUDGE_CONTINUE(money_index < money_type_vc.size());

        money_type = money_type_vc[money_index];
        money_amount = money_amount_vc[money_index];
        if (monster_drop.isMember("money_carve"))
        {
            int left_amount = money_amount, rand_amount = 0, min_rate = 0, max_rate = 0, rand_base = 0;
            double rand_rate = 0.0;
            for (uint i = 0; i < monster_drop["money_carve"].size(); ++i)
            {
                min_rate = monster_drop["money_carve"][i][0u].asInt();
                max_rate = monster_drop["money_carve"][i][1u].asInt();
                rand_base = max_rate - min_rate + 1;
                rand_rate = ((rand() % rand_base) + min_rate) / 100.0;
                rand_amount = money_amount * rand_rate;
                if (rand_amount >= left_amount || i == (monster_drop["money_carve"].size() - 1))
                {
                    this->produce_drop_money(monster_drop, player_map, money_type, left_amount, coord_set, drop_size);
                    break;
                }
                else
                {
                    left_amount -= rand_amount;
                }
                this->produce_drop_money(monster_drop, player_map, money_type, rand_amount, coord_set, drop_size);
            }
        }
        else
        {
            this->produce_drop_money(monster_drop, player_map, money_type, money_amount, coord_set, drop_size);
        }
    }
}

void GameAI::produce_drop_money(const Json::Value &monster_drop, LongMap& player_map, 
            const int money_type, const int money_amount, LongSet &coord_set, int *drop_size)
{
//	Scene* scene = this->fetch_scene();
//	JUDGE_RETURN(scene != NULL, ;);
//
//    AIDropPack* drop_pack = AIDROP_PACKAGE->pop_object();
//	JUDGE_RETURN(drop_pack != NULL, ;);
//
//	AIDropDetail& drop_detail = drop_pack->drop_detail();
//	drop_detail.ai_sort_ = this->ai_sort();
//
//    drop_detail.team_share_ = monster_drop["team_share"].asInt();
//	drop_detail.player_map_ = player_map;
//	drop_detail.money_map_[money_type] = money_amount;
//	drop_detail.drop_type_ = monster_drop["drop_type"].asInt();
//    drop_pack->calc_pickup_protect_tick(monster_drop);
//    drop_pack->calc_recycle_tick(monster_drop, scene);
//
//    MoverCoord rand_coord;
//    Int64 coord_id = -1;
//    if (GameCommon::fetch_drop_coord(rand_coord, *drop_size, this->location()) != 0 ||
//    		GameCommon::is_movable_coord_no_border(this->scene_id(), rand_coord) == false)
//    {
//		rand_coord = scene->rand_dynamic_coord(this->location(), MAX_EXPLOSE_RANGE);
//		coord_id = merge_int_to_long(rand_coord.pos_x(), rand_coord.pos_y());
//		int loop = 0;
//		while (coord_set.find(coord_id) != coord_set.end() && (++loop) < 50 && coord_set.size() < 25)
//		{
//			rand_coord = scene->rand_dynamic_coord(this->location(), MAX_EXPLOSE_RANGE);
//			coord_id = merge_int_to_long(rand_coord.pos_x(), rand_coord.pos_y());
//		}
//    }
//    coord_id = merge_int_to_long(rand_coord.pos_x(), rand_coord.pos_y());
//    coord_set.insert(coord_id);
//    ++(*drop_size);
//
//	drop_pack->sign_and_enter_scene(rand_coord, scene);
//
//	this->is_has_drop_ = true;
}

void GameAI::produce_drop_items(const Json::Value &monster_drop, LongMap& player_map, LongSet &coord_set, int *drop_size)
{}

void GameAI::produce_drop_items(const Json::Value &monster_drop,
		LongMap& player_map, const ItemObj& item_obj, LongSet &coord_set, int *drop_size)
{}

void GameAI::produce_drop_items_b(Scene* scene, Int64 role, const ItemObj& item_obj)
{
	LongMap player_map;
	player_map[role] = true;

	MoverCoord rand_coord = scene->rand_coord_by_pixel_radius(this->location(), 5 * 30);
	AIMANAGER->generate_drop_item(item_obj, rand_coord, scene,
			GameEnum::DROP_TYPE_PERSON, player_map);

	this->is_has_drop_ = true;
}

void GameAI::init_prop_monster(void)
{
	const Json::Value* real_conf = NULL;
	const Json::Value& prop_monster = this->prop_monster();

	if (prop_monster.empty() == true)
	{
		real_conf = &this->monster_conf();
	}
	else
	{
		real_conf = &prop_monster;
	}

	this->init_ai_tree(*real_conf);
	this->init_flag_property(*real_conf);
	this->init_skill_property(*real_conf);
    this->init_safe_area(*real_conf);
    this->init_group_id(*real_conf);
}

void GameAI::init_scene_monster(void)
{
	const Json::Value& layout_item = this->fetch_layout_item();
	JUDGE_RETURN(layout_item.empty() == false, ;);

	this->init_ai_tree(layout_item);
    this->init_flag_property(layout_item);
    this->init_skill_property(layout_item);
    this->init_safe_area(layout_item);
    this->init_group_id(layout_item);
}

void GameAI::set_caller(const Int64 id)
{
    this->ai_detail_.__caller = id;
}

void GameAI::set_leader(const Int64 id)
{
    this->ai_detail_.__leader = id;
}

Int64 GameAI::caller(void)
{
    return this->ai_detail_.__caller;
}

Int64 GameAI::leader(void)
{
    return this->ai_detail_.__leader;
}

bool GameAI::validate_ai_flag(int condition_flag)
{
	JUDGE_RETURN(condition_flag > GameEnum::AI_CF_NONE
			&& condition_flag < GameEnum::AI_CF_END, false);
	return true;
}

bool GameAI::ai_is_back()
{
	return this->ai_detail_.__is_back;
}

bool GameAI::ai_is_timeout()
{
	JUDGE_RETURN(this->ai_detail_.__recycle_tick != Time_Value::zero, false);
	return Time_Value::gettimeofday() > this->ai_detail_.__recycle_tick;
}

bool GameAI::ai_is_area_recycle()
{
	return 0;
}

bool GameAI::test_ai_flag(int condition_flag)
{
	JUDGE_RETURN(this->validate_ai_flag(condition_flag) == true, false);
	return this->ai_detail_.__condition_flag.test(condition_flag);
}

void GameAI::set_ai_falg(int condition_flag)
{
	JUDGE_RETURN(this->validate_ai_flag(condition_flag) == true, ;);
	this->ai_detail_.__condition_flag.set(condition_flag);
}

void GameAI::reset_ai_flag(int condition_flag)
{
	JUDGE_RETURN(this->validate_ai_flag(condition_flag) == true, ;);
	this->ai_detail_.__condition_flag.reset(condition_flag);
}

void GameAI::reset_ai_flag()
{
	this->ai_detail_.__condition_flag.reset();
}

void GameAI::set_fighter_sort()
{
	const Json::Value &monster_json = this->monster_conf();

	int monster_type = monster_json["monster_type"].asInt();
	if (monster_type == 0)
	{
		this->ai_detail_.__monster_type = GameEnum::FIGHTER_MONSTER;
	}
	else if (monster_type < 10)
	{
		this->ai_detail_.__monster_type = GameEnum::FIGHTER_GATHER;
	}
	else
	{
		this->ai_detail_.__monster_type = GameEnum::FIGHTER_BOSS;
	}
}

void GameAI::set_ai_back(bool back_flag)
{
	this->ai_detail_.__is_back = back_flag;
}

void GameAI::ai_die_reward(Int64 benefited_attackor)
{
    MapPlayerEx* player = this->find_player(benefited_attackor);
    JUDGE_RETURN(player != NULL, ;);

    this->update_task_monster(player);
    this->update_player_exp(player);
    this->update_player_angry(player);
}

int GameAI::update_task_monster(MapPlayerEx* player)
{
	player->inc_kill_monster(this->ai_sort(), 1);

	for (AIDetail::PlayerHurtMap::iterator iter = this->ai_detail_.__hurt_map.begin();
			iter != this->ai_detail_.__hurt_map.end(); ++iter)
    {
        MapPlayerEx* cur_player = this->find_player(iter->first);
        JUDGE_CONTINUE(cur_player != NULL);

        cur_player->inc_task_monster(this->ai_sort(), 1);
    }

    return 0;
}

int GameAI::update_player_exp(MapPlayerEx* player)
{
    //经验分配模式
    if (this->ai_detail_.reward_exp_mode_ == 0)
    {
        return this->reward_exp_mode_b(player);
    }
    else
    {
        return this->reward_exp_mode_a(player);
    }
}

int GameAI::update_player_angry(MapPlayerEx* player)
{
	int add_angry = this->monster_conf()["angry"].asInt();
	JUDGE_RETURN(add_angry > 0, -1);

    int type = GameCommon::fetch_skill_id_fun_type(player->fight_detail().__last_skill);
    JUDGE_RETURN(type != GameEnum::SKILL_FUN_SHEN_LUO, -1);

    return player->add_and_notify_angry_value(add_angry);
}

int GameAI::reward_exp_mode_a(MapPlayerEx* player)
{
	int add_exp = this->monster_conf()["exp"].asInt();
	JUDGE_RETURN(add_exp > 0, -1);

	// add experience
	Scene* scene = this->fetch_scene();
	if (scene != 0)
	{
		IntPair pair = scene->fetch_addition_exp();
		add_exp = pair.first + add_exp * (100.0 + pair.second) / 100.0;
	}

	return player->modify_element_experience(add_exp, SerialObj(EXP_FROM_MONSTER, this->ai_sort()));
}

int GameAI::reward_exp_mode_b(MapPlayerEx* player)
{
	const Json::Value &monster_config = this->monster_conf();

	int add_exp = monster_config["exp"].asInt();
	JUDGE_RETURN(add_exp > 0, -1);

	int monster_level = this->level();
	int total_blood = this->fight_detail_.__blood_total_i(this);

	for (AIDetail::PlayerHurtMap::iterator iter = this->ai_detail_.__hurt_map.begin();
			iter != this->ai_detail_.__hurt_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second > 0);

		MapPlayerEx* player = this->find_player(iter->first);
		JUDGE_CONTINUE(player != NULL);

		double percent = iter->second * 1.0 / total_blood;
		int reward_exp = std::max<double>(percent, 0.3) * add_exp ;
		JUDGE_CONTINUE(reward_exp > 0);

		int level_diff = monster_level - player->level();
		double sub_percent = CONFIG_INSTANCE->sub_level_exp(level_diff);
		double total_percent = (1 - sub_percent) * (1 + player->fetch_addition_exp_percent());
		player->modify_element_experience(reward_exp * total_percent, SerialObj(EXP_FROM_MONSTER, this->ai_sort()));
	}

	return 0;
}


Time_Value &GameAI::auto_attack_tick(const std::string &field)
{
	return this->ai_detail_.__auto_attack_tick[field];
}

void GameAI::set_layout_index(int layout_index)
{
	this->ai_detail_.__layout_index = layout_index;
}

int GameAI::layout_index(void)
{
    return this->ai_detail_.__layout_index;
}

void GameAI::set_auto_attack_tick(const std::string &field, const Time_Value &tick)
{
	this->ai_detail_.__auto_attack_tick[field] = tick;
}

int GameAI::fetch_left_alive_time()
{
	Time_Value diff = Time_Value::gettimeofday() - this->ai_detail_.__recycle_tick;
	return diff.sec();
}

int GameAI::die_process(const int64_t org_fighter_id)
{
    Int64 benefited_attackor = this->fetch_benefited_attackor_id(org_fighter_id);
    this->ai_detail_.__killed_id = benefited_attackor;

    this->produce_drop_package();
    this->send_festival_kill_info();
	this->ai_die_reward(benefited_attackor);
    this->auto_detail_.schedule_list_.clear();

	this->action_timer_.cancel_timer();
	this->action_timer_.schedule_timer(0.2);

    GameFighter *caller = 0;
    if (this->caller() > 0 && this->find_fighter(this->caller(), caller) == 0)
    {
        caller->remove_summon_ai(this->ai_id());
    }

    Scene* scene = this->fetch_scene();
    if (scene != NULL)
    {
    	scene->handle_ai_die(this, benefited_attackor);
    	this->update_labour_task_info(benefited_attackor);
    }

    return AutoMapFighter::die_process(org_fighter_id);
}

int GameAI::copy_fighter_property(GameFighter *fighter)
{
    this->fight_detail_.__camp_id = fighter->camp_id();
    this->fight_detail_.__attack_lower.set_single(fighter->fight_detail().__attack_lower_total(fighter));
    this->fight_detail_.__attack_upper.set_single(fighter->fight_detail().__attack_upper_total(fighter));
    this->fight_detail_.__defence_lower.set_single(fighter->fight_detail().__defence_lower_total(fighter));
    this->fight_detail_.__defence_upper.set_single(fighter->fight_detail().__defence_upper_total(fighter));
    this->fight_detail_.__hit.set_single(fighter->fight_detail().__hit_total(fighter));
    this->fight_detail_.__avoid.set_single(fighter->fight_detail().__avoid_total(fighter));
    this->fight_detail_.__crit.set_single(fighter->fight_detail().__crit_total(fighter));
    this->fight_detail_.__toughness.set_single(fighter->fight_detail().__toughness_total(fighter));
    this->fight_detail_.__blood_max.set_single(fighter->fight_detail().__blood_total(fighter));
    this->fight_detail_.__magic_max.set_single(fighter->fight_detail().__magic_total(fighter));
    this->fight_detail_.__blood = this->fight_detail_.__blood_total_i(this);
    this->fight_detail_.__magic = this->fight_detail_.__magic_total_i(this);
    this->fight_detail_.__lucky.set_single(fighter->fight_detail().__lucky_total(fighter));

    return 0;
}

int GameAI::modify_blood_by_fight(const double inc_val, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	//被设置成不能被攻击的怪物不扣血
    JUDGE_RETURN(this->test_ai_flag(GameEnum::AI_CF_NO_BE_ATTACKED) == false, ERROR_TARGET_NO_ATTACK);

	switch (this->ai_detail_.modify_blood_mode_)
	{
	case 1:
	{
		return  this->modify_blood_mode_b(inc_val, fight_tips, attackor, skill_id);
	}

	default:
	case 0:
	{
		return this->modify_blood_mode_a(inc_val, fight_tips, attackor, skill_id);
	}
	}
}

int GameAI::modify_blood_mode_a(const double inc_val, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	// 副本组队在怪物死亡时需要获取怪物的受伤害列表来分配经验,在执行die_process()前记录血量
	int modify_val = inc_val;
	Int64 benefited_attackor_id = this->fetch_benefited_attackor_id(attackor);

	if (modify_val > 0 && this->ai_detail_.__force_hurt > 0)
	{
		modify_val = this->ai_detail_.__force_hurt;
	}

	GameFighter* fighter = NULL;
	if (this->ai_detail_.__force_hurt > 0
			&& this->find_fighter(attackor, fighter) == 0
			&& fighter->is_beast() == true)
	{
		//固定伤害，宠物减半
		modify_val = modify_val / 2;
	}

	Scene* scene = this->fetch_scene();
	if (modify_val > 0 && scene != NULL)
	{
		modify_val = scene->modify_ai_hurt_value(this, modify_val, attackor);
	}

	int correct_blood = std::min<int>(this->fight_detail().__blood, modify_val);
	if (correct_blood > 0)
	{
		this->update_hurt_map(benefited_attackor_id, correct_blood);
	}

    int real_value = AutoMapFighter::modify_blood_by_fight(modify_val,
    		fight_tips, attackor, skill_id);
    JUDGE_RETURN(real_value > 0 && attackor > 0, real_value);

    this->set_last_attacker(attackor);
    this->keep_and_set_aim_object(attackor);

    if (scene != NULL)
    {
    	scene->handle_fighter_hurt(this, benefited_attackor_id, real_value);
    	scene->handle_ai_hurt(this, benefited_attackor_id, real_value);
    }

    return real_value;
}

int GameAI::modify_blood_mode_b(const double inc_val, const int fight_tips,
		const int64_t attackor, const int skill_id)
{
	return 0;
}

bool GameAI::is_movable_path_coord(const DynamicMoverCoord &coord)
{
    if (GameMover::is_movable_path_coord(coord) == false)
    {
        return false;
    }

    MoverCoord test_coord;
    test_coord.set_pixel(coord.pixel_x(), coord.pixel_y());

    GameFighter *fighter = this->fetch_aim_object();
    if (fighter != 0 && coord_offset_grid(test_coord, fighter->location()) <= 1)
    {
    	return false;
    }

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != 0, true);

    return scene->is_movable_dynamic_coord(coord);
}

int GameAI::process_skill_post_launch(int skill_id, int source)
{
//	const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
//	if (this->test_ai_flag(GameEnum::AI_CF_RECYCLE_ON_ATTACKED))
//	{
//		this->ai_detail().__recycle_tick = Time_Value::gettimeofday();
//	}
//
//	int combine_skill = skill_json["combine_skill"].asInt();
//	if (combine_skill > 0)
//	{
//		FighterSkill *fight_skill = NULL;
//		if (this->find_skill(combine_skill, fight_skill) != 0)
//		{
//			this->insert_skill(combine_skill);
//			this->find_skill(combine_skill, fight_skill);
//		}
//
//		if (fight_skill != NULL)
//		{
//			// 怪物身上如果已触发有引导或互斥的技能，则些技能将不会触发；
//			this->auto_fighter_launch_skill(combine_skill);
//		}
//	}

	return 0;
}

int GameAI::refresh_fight_state(const Time_Value &nowtime)
{
	bool prev_state = this->fight_detail_.__fight_state > 0;
	GameFighter::refresh_fight_state(nowtime);
	bool after_state = this->is_fight_state();
	if (prev_state != after_state)
		this->ai_fight_state_to_spd(after_state);
	return 0;
}

void GameAI::update_fight_state(const Int64 fighter_id, const int state, const Time_Value &last_tick)
{
	bool prev_state = this->is_fight_state();
	GameFighter::update_fight_state(fighter_id, state, last_tick);

	bool after_state = this->is_fight_state();
	if (prev_state != after_state)
	{
		this->ai_fight_state_to_spd(after_state);
	}

//    if (state == GameEnum::FIGHT_STATE_PASSIVE)
//    {
//        if (this->hit_stiff_last() > 0.000001)
//            this->insert_defender_status(this, BasicStatus::STIFF, 0.0, this->hit_stiff_last());
//    }
        
}

void GameAI::ai_fight_state_to_spd(const bool new_is_fight)
{
	const Json::Value &robot_json = CONFIG_INSTANCE->monster(this->ai_sort());
	int base_spd = robot_json["spd"].asInt();
	if (base_spd <= 0)
	{
		base_spd = 100;
	}

	if (new_is_fight == true)
	{
		base_spd += robot_json["fight_spd"].asInt();
	}

	this->update_fighter_speed(GameEnum::SPEED, base_spd);
}

int GameAI::auto_fighter_attack(void)
{
	// 首次攻击时先停一下，让怪物播放完移动
	if (this->ai_detail_.__last_is_move == true)
	{
		this->ai_detail_.__last_is_move = false;
		Time_Value tick = this->calculate_move_tick();
		this->push_schedule_time(tick + tick);
		return 0;
	}
	return AutoMapFighter::auto_fighter_attack();
}

int GameAI::schedule_move_fighter(void)
{
    JUDGE_RETURN(this->guide_skill() == 0, -1);
	JUDGE_RETURN(this->validate_fighter_movable() == 0, -1);

	// 攻击完首次移动时先停一下，让怪物播放完攻击
	if (this->ai_detail_.__last_is_move == false)
	{
		this->ai_detail_.__last_is_move = true;
	}
	return AutoMapFighter::schedule_move_fighter();
}

int GameAI::schedule_move_fighter_no_dynamic_mpt(void)
{
    JUDGE_RETURN(this->guide_skill() == 0, -1);
	JUDGE_RETURN(this->validate_fighter_movable() == 0, -1);

	GameFighter *aim_fighter = this->fetch_aim_object();
	JUDGE_RETURN(aim_fighter != 0, -1);

	MoverCoord target = this->chase_coord(aim_fighter->location());
	if (target == aim_fighter->location())
	{
		int rand_x = rand() % 4 - 2, rand_y = rand() % 4 - 2;
		rand_x = (rand_x < 0 ? rand_x - 2 : rand_x + 2);
		rand_y = (rand_y < 0 ? rand_y - 2 : rand_y + 2);
		target.set_pos(target.pos_x() + rand_x, target.pos_y() + rand_y);
	}

    {
    	// 防止跨区域的寻路死循环
    	int fighter_area_id = GameCommon::fetch_area_id_by_coord(this->scene_id(), this->location()),
    			target_area_id = GameCommon::fetch_area_id_by_coord(this->scene_id(), target);
    	JUDGE_RETURN(fighter_area_id == target_area_id, -1);
    }

	Time_Value nowtime = Time_Value::gettimeofday();
	this->generate_move_path_no_dynamic_mpt(target);
	this->monitor()->move_total_use_ += (Time_Value::gettimeofday() - nowtime);

	JUDGE_RETURN(this->mover_detial_.__step_list.empty() == false, -1);

	MoverCoord &step = *(this->mover_detail().__step_list.rbegin());
	JUDGE_RETURN(step != this->location(), -1);

	Scene *scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	this->push_schedule_time(this->mover_detial_.__step_time);
	return scene->refresh_mover_location(this, step);
}

int GameAI::gather_limit_collect_begin(Int64 role_id, int time)
{
	const Json::Value& gather_conf = CONFIG_INSTANCE->gather(this->ai_sort());
	JUDGE_RETURN(gather_conf != Json::Value::null, ERROR_SERVER_INNER);

	int gather_limit = gather_conf["gather_limit"].asInt();
	JUDGE_RETURN(gather_limit > 0, 0);

	JUDGE_RETURN(this->ai_detail_.__gather_count < gather_limit, ERROR_OTHER_USING);
	JUDGE_RETURN(this->ai_detail_.__toucher_map.count(role_id) == 0, ERROR_SERVER_INNER);

	this->ai_detail_.__gather_count += 1;
	this->ai_detail_.__toucher_map[role_id] = ::time(NULL) + time;

	if (this->gather_timer_.is_registered() == false)
	{
		this->gather_timer_.schedule_timer(Time_Value(1));
	}

	return 0;
}

int GameAI::gather_limit_collect_done(Int64 role_id, int result, ItemObj &gather_item)
{
	const Json::Value& gather_conf = CONFIG_INSTANCE->gather(this->ai_sort());

	int gather_limit = gather_conf["gather_limit"].asInt();
	JUDGE_RETURN(gather_limit > 0, 0);

	Scene* scene = this->fetch_scene();
	this->ai_detail_.__toucher_map.erase(role_id);

	// fail
	if (result != 0)
	{
		this->ai_detail_.__gather_count -= 1;
		return 0;
	}

	// success
	if (this->ai_detail_.__gather_count >= gather_limit)
	{
		this->ai_detail_.__killed_id = role_id;
		scene->handle_ai_die(this, role_id);

		this->exit_scene();
		this->sign_out();

		return 0;
	}

	return 0;
}

int GameAI::gather_limit_timeout()
{
	Int64 now_tick = ::time(NULL);
	LongMap toucher_map = this->ai_detail_.__toucher_map;

	for (LongMap::iterator iter = toucher_map.begin(); iter != toucher_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second >= now_tick);
		this->ai_detail_.__toucher_map.erase(iter->first);
		this->ai_detail_.__gather_count -= 1;
	}

	if (this->ai_detail_.__toucher_map.empty())
	{
		this->gather_timer_.cancel_timer();
	}

	return 0;
}


int GameAI::find_avoid_buff(const int status)
{
	if (this->ai_detail_.__avoid_buff_set.count(status) > 0)
	{
		return 0;
	}

	return -1;
}

Int64 GameAI::find_max_hurt_fighter(void)
{
	if (this->ai_detail_.__hurt_map.count(this->ai_detail_.__max_hurt_id) > 0)
	{
		return this->ai_detail_.__max_hurt_id;
	}

	int max_hurt = 0;
	Int64 max_fighter = NULL;

	AIDetail::PlayerHurtMap& hurt_map = this->ai_detail_.__hurt_map;
	for(AIDetail::PlayerHurtMap::iterator iter = hurt_map.begin();
			iter != hurt_map.end(); iter++)
	{
		JUDGE_CONTINUE(iter->second > max_hurt);

		max_hurt = iter->second;
		max_fighter = iter->first;
	}

	this->ai_detail_.__max_hurt_id = max_fighter;
	return this->ai_detail_.__max_hurt_id;
}

int GameAI::fetch_hurt_range_role(LongVec &role_set, const int max_begin, const int max_end)
{
    AIHurtSort hurt_sort;
    std::vector<AIHurtSort> hurt_vc;
    for (AIDetail::PlayerHurtMap::iterator iter = this->ai_detail_.__hurt_map.begin();
            iter != this->ai_detail_.__hurt_map.end(); ++iter)
    {
        hurt_sort.__role_id = iter->first;
        hurt_sort.__value = iter->second;
        hurt_vc.push_back(hurt_sort);
    }
    std::sort(hurt_vc.begin(), hurt_vc.end(), AIHurtSortCmp());

    if (hurt_vc.size() == 1)
    {
    	role_set.push_back(hurt_vc[0].__role_id);
    }
    else
    {
		for (int i = max_begin - 1; i < max_end && i < int(hurt_vc.size()); ++i)
		{
			role_set.push_back(hurt_vc[i].__role_id);
		}
    }
    return 0;
}

Int64 GameAI::aim_select(void)
{
	Int64 aim_id = find_max_hurt_fighter();
    JUDGE_RETURN(aim_id > 0, aim_id);

    GameFighter *aim_player = 0;
    this->find_fighter(aim_id, aim_player);
    JUDGE_RETURN(aim_player != NULL, 0);

	if (aim_player != 0 && !aim_player->is_death()
			&& this->is_movable_coord(aim_player->location()))
    {
    	this->set_aim_object(aim_id);
    }

	return aim_id;
}

void GameAI::update_hurt_map(const Int64 fighter_id, const int value)
{
	bool new_flag = this->ai_detail_.__hurt_map.count(fighter_id) == 0;
	this->ai_detail_.__hurt_map[fighter_id] += value;

	if (new_flag == true)
	{
		this->send_festival_hurt_info();
	}

	AIDetail::PlayerHurtMap::iterator iter = this->ai_detail_.__hurt_map.find(this->ai_detail_.__max_hurt_id);
	if (iter == this->ai_detail_.__hurt_map.end())
	{
		this->ai_detail_.__max_hurt_id = this->find_max_hurt_fighter();
	}
	else
	{
		if (iter->second < this->ai_detail_.__hurt_map[fighter_id])
		{
			this->ai_detail_.__max_hurt_id = fighter_id;
		}
	}
}

void GameAI::send_festival_hurt_info()
{
	JUDGE_RETURN(this->is_in_normal_mode() == true, ;);
	JUDGE_RETURN(MAP_MONITOR->festival_icon_type() > 0, ;);

	AIDetail& detail = this->ai_detail();
	JUDGE_RETURN(detail.league_index_ == 1, ;);

	Proto30100604 inner;
	inner.set_type(false);

	for (AIDetail::PlayerHurtMap::iterator iter = detail.__hurt_map.begin();
			iter != detail.__hurt_map.end(); ++iter)
	{
		inner.add_player_set(iter->first);
	}

	MAP_MONITOR->dispatch_to_logic(&inner);
}

int GameAI::update_labour_task_info(Int64 benefited_attackor)
{
	JUDGE_RETURN(MAP_MONITOR->is_in_big_act_time(), 0);
	MapPlayerEx* player = this->find_player(benefited_attackor);
	JUDGE_RETURN(player != NULL, 0);
	Proto31403201 msg;
	msg.set_task_finish_count(1);
	msg.set_task_id(GameEnum::LABOUR_TASK_KILL_MONSTER);
	const Json::Value &cond_value = CONFIG_INSTANCE->labour_act_info(GameEnum::LABOUR_TASK_KILL_MONSTER)["condition"];
	if(cond_value == Json::Value::null)
	{
		MSG_USER("No find task:%d", GameEnum::LABOUR_TASK_KILL_MONSTER);
		return 0;
	}
	if(cond_value.isArray())
	{
		int cond = cond_value[1].asInt();
		if(this->level() >= cond)
		{
			return MAP_MONITOR->dispatch_to_logic(player, &msg);
		}
	}
	else
	{
		if(this->level() >= 50)
		{
			return MAP_MONITOR->dispatch_to_logic(player, &msg);
		}
	}
	return 0;
}

void GameAI::send_festival_kill_info()
{
	JUDGE_RETURN(this->is_in_normal_mode() == true, ;);
	JUDGE_RETURN(MAP_MONITOR->festival_icon_type() > 0, ;);

	AIDetail& detail = this->ai_detail();
	JUDGE_RETURN(detail.league_index_ == 1, ;);

	Proto30100604 inner;
	inner.set_type(true);
	MAP_MONITOR->dispatch_to_logic(&inner);
}

void GameAI::generate_gift_box(void)
{
    return ;
}

void GameAI::shout_drop_goods(const PackageItem *drop_item)
{

}


