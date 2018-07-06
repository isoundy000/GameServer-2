/*
 * Scene.cpp
 *
 * Created on: 2013-01-23 11:40
 *     Author: glendy
 */

#include <math.h>
#include "Scene.h"
#include "AIManager.h"
#include "MapMonitor.h"
#include "MapBeast.h"
#include "MapPlayerEx.h"
#include "FloatAI.h"
#include "ProtoDefine.h"

#define MAPPING_FACTOR 1.5

const int Scene::chase_point[][2] = {
    {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},
    {2,0},{2,1},{2,2},{1,2},{0,2},{-1,2},{-2,2},{-2,1},
    {-2,0},{-2,-1},{-2,-2},{-1,-2},{0,-2},{1,-2},{2,-2},{2,-1}
};

const int Scene::SELECT_POINT[][2] = {
    {1,0},  {0,1},   {-1,0}, {0,-1},
    {2,0},  {1,1},   {0,2},  {-1,1},
    {-2,0}, {-1,-1}, {0,-2}, {1,-1}
};
const int Scene::max_chase_point_size_ = 24;

Scene *Scene::SceneTimer::scene(Scene *scene)
{
    if (scene != 0)
        this->scene_ = scene;
    return this->scene_;
}

int Scene::SceneTimer::type(void)
{
    return GTT_MAP_BROAD;
}

int Scene::SceneTimer::handle_timeout(const Time_Value &tv)
{
    this->scene_->process_broad(tv);
    this->scene_->process_broad_teamer();
//    this->scene_->process_broad_near_player(tv);

    return 0;
}

Scene *Scene::SceneControlTimer::scene(Scene *scene)
{
    if (scene != 0)
        this->scene_ = scene;
    return this->scene_;
}

int Scene::SceneControlTimer::type(void)
{
    return GTT_MAP_ONE_SECOND;
}

int Scene::SceneControlTimer::handle_timeout(const Time_Value &tv)
{
	this->scene_->handle_timeout(tv);

    return 0;
}


Scene::Scene(void)
{
    this->monitor_ = MAP_MONITOR;
    this->hash_id_ = GlobalIndex::global_scene_hash_id_++;

    this->scene_timer_.scene(this);
    this->scene_trl_timer_.scene(this);

    Scene::reset();
}

Scene::~Scene(void)
{ /*NULL*/ }

MapMonitor *Scene::monitor(void)
{
    return this->monitor_;
}

SceneDetail& Scene::scene_detail(void)
{
	return this->scene_detail_;
}

void Scene::reset(void)
{
	this->scene_trl_timer_.cancel_timer();
	this->scene_timer_.cancel_timer();

    std::vector<GameAI *> ai_vc;
    std::vector<AIDropPack *> drop_vc;
    for (MoverMap::iterator iter = this->mover_map_.begin(); iter != this->mover_map_.end(); ++iter)
    {
        GameAI *game_ai = dynamic_cast<GameAI *>(iter->second);
        if (game_ai != NULL && game_ai->is_enter_scene() == true)
        {
            ai_vc.push_back(game_ai);
            continue;
        }
        AIDropPack *drop_pack = dynamic_cast<AIDropPack *>(iter->second);
        if (drop_pack != NULL && drop_pack->is_enter_scene() == true)
        {
        	drop_vc.push_back(drop_pack);
        	continue;
        }
    }

    for (std::vector<GameAI *>::iterator iter = ai_vc.begin(); iter != ai_vc.end(); ++iter)
    {
        (*iter)->sign_out();
    }

    for (std::vector<AIDropPack *>::iterator iter = drop_vc.begin(); iter != drop_vc.end(); ++iter)
    {
    	(*iter)->exit_scene();
    }

    this->scene_detail_.reset();
    this->mover_map_.unbind_all();
    this->player_map_.unbind_all();
    this->fighter_map_.unbind_all();
    this->monster_map_.unbind_all();
    this->float_ai_map_.unbind_all();

    this->event_cut_ = false;
    this->quit_team_ = false;
    this->camp_split_ = false;
    this->safe_area_ = false;

    this->start_scene_ = false;
    this->start_monster_ = false;
    this->is_inited_scene_ = false;
}

int Scene::init_scene(const int space_id, const int scene_id)
{
    this->scene_detail_.__space_id = space_id;
    this->scene_detail_.__scene_id = scene_id;

    const Json::Value& conf = this->conf();
    const Json::Value& set_conf = this->set_conf();
    if (conf.isMember("safe_area") == true)
    {
    	this->safe_area_ = true;
    }
    if (set_conf.isMember("camp_split") == true)
    {
    	this->camp_split_ = set_conf["camp_split"].asInt();
    }
    if (set_conf.isMember("event_cut") == true)
    {
    	this->event_cut_ = set_conf["event_cut"].asInt();
    }
    if (set_conf.isMember("quit_team") == true)
    {
    	this->quit_team_ = set_conf["quit_team"].asInt();
    }

    const MptDetail &mpt_detail = CONFIG_INSTANCE->mpt(scene_id);
    const Json::Value &map_block_json = CONFIG_INSTANCE->map_block();

    if (mpt_detail.__block_width > 0)
    {
        this->scene_detail_.__block_width = mpt_detail.__block_width;
        this->scene_detail_.__block_height = mpt_detail.__block_height;
    }
    else
    {
        this->scene_detail_.__block_width = map_block_json["block_width"].asInt();
        this->scene_detail_.__block_height = map_block_json["block_height"].asInt();
    }

    this->scene_detail_.__mpt_x_len = mpt_detail.__x_len;
    this->scene_detail_.__mpt_y_len = mpt_detail.__y_len;

    // TODO; read from config;
    this->scene_detail_.__dynamic_grid_pixel = GameEnum::DEFAULT_AI_PATH_GRID;
    this->scene_detail_.__dynamic_x_len = mpt_detail.__map_width / this->scene_detail_.__dynamic_grid_pixel + 
        (mpt_detail.__map_width % this->scene_detail_.__dynamic_grid_pixel > 0 ? 1 : 0);
    this->scene_detail_.__dynamic_y_len = mpt_detail.__map_height / this->scene_detail_.__dynamic_grid_pixel +
        (mpt_detail.__map_height % this->scene_detail_.__dynamic_grid_pixel > 0 ? 1 : 0);
    this->scene_detail_.__dynamic_mpt.resize(this->scene_detail_.__dynamic_x_len * this->scene_detail_.__dynamic_y_len, 0);

    int block_width = this->scene_detail_.__block_width,
        block_height = this->scene_detail_.__block_height,
        x_len = this->scene_detail_.__mpt_x_len,
        y_len = this->scene_detail_.__mpt_y_len;
    this->scene_detail_.__block_x_amount = x_len / block_width + ((x_len % block_width) > 0 ? 1 : 0);
    this->scene_detail_.__block_y_amount = y_len / block_height + ((y_len % block_height) > 0 ? 1 : 0);

    size_t max_block_size = this->scene_detail_.__block_x_amount * this->scene_detail_.__block_y_amount;
    this->scene_detail_.__scene_block_list.resize(max_block_size);
    this->scene_detail_.__mover_amount_list.resize(max_block_size);
    this->scene_detail_.__player_amount_list.resize(max_block_size);

    this->scene_detail_.__near_block_width = map_block_json["near_block_width"].asInt();
    this->scene_detail_.__near_block_height = map_block_json["near_block_height"].asInt();
    if (this->scene_detail_.__near_block_width <= 0)
    	this->scene_detail_.__near_block_width = 65;
    if (this->scene_detail_.__near_block_height <= 0)
    	this->scene_detail_.__near_block_height = 43;
    this->scene_detail_.__near_block_x_amount = x_len / this->scene_detail_.__near_block_width + ((x_len % this->scene_detail_.__near_block_width) > 0 ? 1 : 0);
    this->scene_detail_.__near_block_y_amount = y_len / this->scene_detail_.__near_block_height + ((y_len % this->scene_detail_.__near_block_height) > 0 ? 1 : 0);
    size_t max_near_block_size = this->scene_detail_.__near_block_x_amount * this->scene_detail_.__near_block_y_amount;
    this->scene_detail_.__near_block_list.resize(max_near_block_size);
    this->is_inited_scene_ = true;

    MSG_USER("scene init size: %d %d", scene_id, max_block_size);
    return 0;
}

bool Scene::is_inited_scene(void)
{
	return this->is_inited_scene_;
}

void Scene::start_scene()
{
	this->start_scene_ = true;
	this->scene_trl_timer_.schedule_timer(1);
}

void Scene::run_scene()
{
	JUDGE_RETURN(this->start_scene_ == true, ;);

	const Json::Value &layout = this->conf()["layout"];
	JUDGE_RETURN(layout.empty() == false, ;);

	for (uint i = 0; i < layout.size(); ++i)
	{
		if (this->start_monster_ == false)
		{
			this->init_scene_monster(i);
		}
		else
		{
			this->run_scene_monster(i);
		}
	}

	this->start_monster_ = true;
}

bool Scene::is_started_scene(void)
{
	return this->start_scene_;
}

int Scene::init_scene_monster(int layout_index)
{
	AIMANAGER->generate_ai(layout_index, this);
	return 0;
}

int Scene::run_scene_monster(int layout_index)
{
//	const Json::Value &layout_item = this->layout_index(layout_index);
//	JUDGE_RETURN(layout_item["born_type"].asString() == GameName::CENTER_BORN, -1);
//	JUDGE_RETURN(layout_item["born_times"].asString() == GameName::UNLIMITED_BORN, -1);
//
//	int monster_sort = layout_item["monster_sort"].asInt();
//	int center_size = layout_item["center_coordxy"].size();
//	int differ_count = layout_item["born_count"].asInt() * (center_size / 2)
//			- this->scene_detail_.ai_sort_map_[monster_sort].size();
//
//	JUDGE_RETURN(differ_count > 0, -1);
//	AIMANAGER->generate_ai_by_center(layout_index, this, differ_count, true);

	return 0;
}

int Scene::register_monster(GameMover *mover)
{
	GameAI *game_ai = dynamic_cast<GameAI *>(mover);
	JUDGE_RETURN(game_ai != NULL, -1);

    this->monster_map_.rebind(game_ai->ai_id(), game_ai);

    {
    	BLongSet & monster_set = this->scene_detail_.ai_sort_map_[game_ai->ai_sort()];
	    monster_set.insert(game_ai->mover_id());

        if (game_ai->is_boss())
		{
			this->scene_detail_.boss_sort_set_.insert(game_ai->ai_sort());
		}
    }

    if (game_ai->group_id() > 0)
    {
    	BLongSet &monster_set = this->scene_detail_.ai_group_map_[game_ai->group_id()];
        monster_set.insert(game_ai->mover_id());
    }

    EffectAI *effect_ai = dynamic_cast<EffectAI *>(mover);
    if (effect_ai != NULL)
    {
        if (effect_ai->is_safe_area_effect())
            this->scene_detail_.safe_area_set_.insert(effect_ai->ai_id());
    }

	return 0;
}

int Scene::unregister_monster(GameMover *mover)
{
	GameAI *game_ai = dynamic_cast<GameAI *>(mover);
	JUDGE_RETURN(game_ai != NULL, -1);

    this->monster_map_.unbind(game_ai->ai_id());

    {
    	BLongSet & monster_set = this->scene_detail_.ai_sort_map_[game_ai->ai_sort()];
	    monster_set.erase(game_ai->ai_id());
    }

    if (game_ai->group_id() > 0)
    {
    	BLongSet &monster_set = this->scene_detail_.ai_group_map_[game_ai->group_id()];
        monster_set.erase(game_ai->ai_id());
    }

    EffectAI *effect_ai = dynamic_cast<EffectAI *>(mover);
    if (effect_ai != NULL)
    {
        if (effect_ai->is_safe_area_effect())
            this->scene_detail_.safe_area_set_.erase(effect_ai->ai_id());
    }

	return 0;
}

int Scene::cal_block_index_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, BlockIndexType &block_index)
{
    block_index = this->scene_detail_.__block_x_amount * block_y + block_x;
    if (block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= block_index)
        return -1;
    return 0;
}

int Scene::hash_id()
{
	return this->hash_id_;
}

int Scene::camp_split()
{
	return this->camp_split_ == true;
}

int Scene::has_event_cut()
{
	return this->event_cut_ == true;
}

int Scene::has_quit_team()
{
	return this->quit_team_ == true;
}

int Scene::has_safe_area()
{
	return this->safe_area_ == true;
}

int Scene::find_status_value(Int64 role, int id, int type)
{
	GameFighter* fighter = this->find_fighter(role);
	JUDGE_RETURN(fighter != NULL, 0);
	return fighter->find_status_value(id, type);
}

int Scene::scene_id(void)
{
    return this->scene_detail_.__scene_id;
}

int Scene::space_id(void)
{
    return this->scene_detail_.__space_id;
}

int Scene::mover_amount(void)
{
    return this->mover_map_.size();
}

int Scene::player_amount(void)
{
    return this->player_map_.size();
}

int Scene::room_amount(void)
{
    return this->scene_detail_.__room_amount;
}

int Scene::handle_timeout(const Time_Value &nowtime)
{
	return 0;
}

int Scene::process_broad(const Time_Value &nowtime)
{
    if (this->player_amount() <= 0 && this->start_scene_ == true)
        this->stop_timer();

    JUDGE_RETURN(this->scene_detail_.__has_block_set.size() > 0, 0);

    size_t mover_offset = 0, mover_amount = 0;

    InnerRouteHead route_head(0, 0, BT_BROAD_IN_GATE, this->scene_id(), 1);
    ProtoHead head;

    for (BLongSet::iterator iter = this->scene_detail_.__has_block_set.begin();
            iter != this->scene_detail_.__has_block_set.end(); ++iter)
    {
        int64_t block_index = *iter;
        if (block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= block_index)
            continue;
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];

        if (scene_block.__data_buff.readable_bytes() <= 0)
        	continue;

        if (this->scene_detail_.__player_amount_list[block_index] <= 0)
        {
            scene_block.__data_buff.reset();
            scene_block.__mover_offset_map.clear();
            mover_amount += scene_block.__mover_map.size();
            continue;
        }

        if (scene_block.__player_map.empty() == false)
        {
        	int prev_len = sizeof(int32_t) * 2 + sizeof(InnerRouteHead) + sizeof(int32_t) + sizeof(ProtoHead),
        			data_buff_len = scene_block.__data_buff.readable_bytes();

        	boost::unordered_map<int, Block_Buffer*> gate_block_map;
			for (MoverMap::iterator player_iter = scene_block.__player_map.begin();
					player_iter != scene_block.__player_map.end(); ++player_iter)
			{
				GameMover *mover = player_iter->second;
                if (mover->is_enter_scene() == false)
                	continue;
                if (mover->gate_sid() <= 0)
                	continue;

                MapPlayerEx *player = dynamic_cast<MapPlayerEx *>(mover);
                if (player == NULL || player->is_online_player() == false)
                	continue;

                boost::unordered_map<int, Block_Buffer*>::iterator gate_iter = gate_block_map.find(mover->gate_sid());
                if (gate_iter == gate_block_map.end())
                {
                	Block_Buffer *buff = this->pop_block();
                	gate_block_map[mover->gate_sid()] = buff;

                	buff->ensure_writable_bytes(prev_len + sizeof(int) * 4 + data_buff_len + sizeof(int64_t) * 50);
                	buff->write_int32(mover->gate_sid());
                	buff->write_int32(0);
                	buff->copy(&route_head, sizeof(InnerRouteHead));
                	buff->write_int32(0);
                	buff->copy(&head, sizeof(ProtoHead));
                	buff->write_int32(data_buff_len);
                	buff->copy(&(scene_block.__data_buff));
                	buff->write_int32(1);
                	buff->write_int64(mover->mover_id());
					if (scene_block.__mover_offset_map.find(mover->mover_id(), mover_offset) != 0)
					{
						mover_offset = 0;
					}
					else
					{
						mover_offset -= scene_block.__data_buff.get_read_idx();
						mover_offset = (mover_offset < 0 ? 0 : mover_offset);
					}
					buff->write_int32(mover_offset);
                }
                else
                {
                	Block_Buffer *buff = gate_iter->second;
                	int *data_len = (int *)(buff->get_read_ptr() + prev_len);
                	int *mover_size = (int *)(buff->get_read_ptr() + prev_len + sizeof(int32_t) + *data_len);
                	++(*mover_size);
                	buff->write_int64(mover->mover_id());
					if (scene_block.__mover_offset_map.find(mover->mover_id(), mover_offset) != 0)
					{
						mover_offset = 0;
					}
					else
					{
						mover_offset -= scene_block.__data_buff.get_read_idx();
						mover_offset = (mover_offset < 0 ? 0 : mover_offset);
					}
                	buff->write_int32(mover_offset);
                }
			}
            for (boost::unordered_map<int, Block_Buffer*>::iterator block_iter = gate_block_map.begin();
            		block_iter != gate_block_map.end(); ++block_iter)
            {
            	Block_Buffer *buff = block_iter->second;
            	int *total_len = (int *)(buff->get_read_ptr() + sizeof(int32_t));
            	int *head_body_len = (int *)(buff->get_read_ptr() + sizeof(int32_t) * 2 + sizeof(InnerRouteHead));
            	*total_len = buff->readable_bytes() - sizeof(int32_t) * 2;
            	*head_body_len = buff->readable_bytes() - sizeof(int32_t) * 3 - sizeof(InnerRouteHead);
            	if (this->monitor()->inner_sender(block_iter->first)->push_pool_block_with_len(block_iter->second) != 0)
            		this->push_block(block_iter->second);
            }
        }
        scene_block.__data_buff.reset();
        scene_block.__mover_offset_map.clear();
        mover_amount += scene_block.__mover_map.size();
    }

    this->scene_detail_.__has_block_set.clear();
    this->flush_broad_interval(nowtime, mover_amount);

    return 0;
}

int Scene::flush_broad_interval(const Time_Value &nowtime, const size_t mover_amount)
{
//    int index = mover_amount / 200;
    int index = 0, total_player = this->monitor()->player_manager()->process_player_size();
    if (total_player <= 100)
    	index = 0;
    else if (total_player <= 200)
    	index = 1;
    else if (total_player <= 600)
    	index = 2;
    if (total_player > 600)
    {
        index = (total_player - 400) / 200 + 2;
    }

    // calculate interval by mover amount;
    MapBlockDetail &block_detail = CONFIG_INSTANCE->map_block_detail();
    if (index >= int(block_detail.__flush_tick_vc.size()))
        index = int(block_detail.__flush_tick_vc.size()) - 1;

    int inter = block_detail.__flush_tick_vc[index];
    Time_Value interval(0, inter * 1000);
    this->scene_timer_.refresh_tick(nowtime, interval);
    return 0;
}

int Scene::start_timer(void)
{
	JUDGE_RETURN(this->scene_timer_.is_registered() == false, -1);

    Time_Value interval(0, 1 * 1000);
    return this->scene_timer_.schedule_timer(interval);
}

int Scene::stop_timer(void)
{
    return this->scene_timer_.cancel_timer();
}

int Scene::is_fighting_time()
{
	return true;
}

int Scene::is_validate_attack_player(GameFighter* attacker, GameFighter* target)
{
	return true;
}

int Scene::is_validate_attack_monster(GameFighter* attacker, GameFighter* target)
{
	return true;
}

int Scene::refresh_mover_location(GameMover *mover, const MoverCoord &target, const bool is_send_move)
{
    MoverCoord &mover_coord = mover->location();
    if (mover_coord.pixel_x() == target.pixel_x() && mover_coord.pixel_y() == target.pixel_y())	// 修正战斗时移动坐标不一致
    {
    	return 0;
    }

    BlockIndexType org_block_index = mover->cur_block_index(), target_block_index = -1,
                   org_block_x = 0, org_block_y = 0, target_block_x = 0, target_block_y = 0;
    if (org_block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= org_block_index)
    {
        MSG_USER("ERROR mover cur_block_index error %ld %d %d[%d,%d]", mover->mover_id(), org_block_index,
                mover->scene_id(), mover_coord.pos_x(), mover_coord.pos_y());
        return ERROR_COORD_ILLEGAL;
    }
    this->cal_block_coord_by_block_index(org_block_index, org_block_x, org_block_y);

    this->cal_block_coord_by_grid_coord(target.pos_x(), target.pos_y(), target_block_x, target_block_y);
    if (this->cal_block_index_by_block_coord(target_block_x, target_block_y, target_block_index) != 0)
    {
        MSG_USER("ERROR target grid coord to index %ld %d[%d,%d]", mover->mover_id(),
                this->scene_id(), target.pos_x(), target.pos_y());
        return ERROR_COORD_ILLEGAL;
    }

    bool is_near_block = this->is_near_block(target, org_block_index);
    if (is_near_block == true)
    {
        // 未跨区块
        Block_Buffer move_buff;
        Block_Buffer *buff = &move_buff;
        if (is_send_move == true)
        {
        	mover->make_up_move_info(buff);
        }

        this->send_around_block(mover, org_block_x, org_block_y, buff);
    }
    else
    {
        // 跨区块
        // 将原来玩家所在区块的消息推送
        if (mover->is_player() == true)
        {
            this->scene_detail_.__scene_block_list[org_block_index].flush_to_mover(mover);
        }

        Block_Buffer disappear_buff;
        Block_Buffer mover_buff;
        Block_Buffer appear_buff;
        mover->make_up_disappear_info(&disappear_buff);
        if (is_send_move == true)
        {
        	mover->make_up_move_info(&mover_buff);
        }

        MoverCoord org_coord = mover_coord;
        mover_coord = target;
        mover->make_up_appear_info(&appear_buff);
        mover_coord = org_coord;
        this->send_diff_block(mover, target, org_block_x, org_block_y, target_block_x, target_block_y,
                &disappear_buff,
                &mover_buff,
                &appear_buff);
    }

    if (mover->is_monster())
    {
        this->reduce_mpt_coord(mover, mover->location());
    }

    if (is_near_block == false)
    {
        this->unregister_mover(mover, org_block_index);
    }

//    int prev_near_block_index = this->near_block_index_by_block_coord(this->near_block_x_from_pos_x(mover_coord.pos_x()), this->near_block_y_from_pos_y(mover_coord.pos_y())),
//    	target_near_block_index = this->near_block_index_by_block_coord(this->near_block_x_from_pos_x(target.pos_x()), this->near_block_y_from_pos_y(target.pos_y()));
//    if (prev_near_block_index != target_near_block_index)
//    {
//    	this->unregister_near_mover(mover);
//    }

    mover->set_cur_location(target);

//    if (prev_near_block_index != target_near_block_index)
//    {
//    	this->register_near_mover(mover);
//    }

    if (is_near_block == false)
    {
        this->register_mover(mover, target_block_index);
        mover->set_cur_block_index(target_block_index);
    }

    if (mover->is_monster())
    {
        this->increase_mpt_coord(mover, mover->location());
    }
    return 0;
}

int Scene::refresh_mover_location_for_relive(GameMover *mover, const MoverCoord &target)
{
    MoverCoord &mover_coord = mover->location();

    BlockIndexType org_block_index = mover->cur_block_index(), target_block_index = -1,
                   org_block_x = 0, org_block_y = 0, target_block_x = 0, target_block_y = 0;
    if (org_block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= org_block_index)
    {
        MSG_USER("ERROR mover cur_block_index error %ld %d %d[%d,%d]", mover->mover_id(), org_block_index,
                mover->scene_id(), mover_coord.pos_x(), mover_coord.pos_y());
        return ERROR_COORD_ILLEGAL;
    }
    this->cal_block_coord_by_block_index(org_block_index, org_block_x, org_block_y);

    this->cal_block_coord_by_grid_coord(target.pos_x(), target.pos_y(), target_block_x, target_block_y);
    if (this->cal_block_index_by_block_coord(target_block_x, target_block_y, target_block_index) != 0)
    {
        MSG_USER("ERROR target grid coord to index %ld %d[%d,%d]", mover->mover_id(),
                this->scene_id(), target.pos_x(), target.pos_y());
        return ERROR_COORD_ILLEGAL;
    }

    bool is_near_block = this->is_near_block(target, org_block_index);
    if (is_near_block == true)
    {
        // 未跨区块
        Block_Buffer disappear_buff, appear_buff;
        mover->make_up_disappear_info(&disappear_buff);
        MoverCoord org_coord = mover_coord;
        mover_coord = target;
        mover->make_up_appear_info(&appear_buff);
        mover_coord = org_coord;
        this->send_around_block(mover, org_block_x, org_block_y, &disappear_buff);
        this->send_around_block(mover, org_block_x, org_block_y, &appear_buff);
    }
    else
    {
        // 跨区块
        // 将原来玩家所在区块的消息推送
        if (mover->is_player() == true)
            this->scene_detail_.__scene_block_list[org_block_index].flush_to_mover(mover);

        Block_Buffer disappear_buff, appear_buff;
        mover->make_up_disappear_info(&disappear_buff);

        MoverCoord org_coord = mover_coord;
        mover_coord = target;
        mover->make_up_appear_info(&appear_buff);
        mover_coord = org_coord;
        this->send_diff_block_for_relive(mover, target, org_block_x, org_block_y, target_block_x, target_block_y,
                &disappear_buff, &appear_buff);
    }

    if (is_near_block == false)
        this->unregister_mover(mover, org_block_index);
//    this->unregister_near_mover(mover);

    mover_coord = target;

//    this->register_near_mover(mover);
    if (is_near_block == false)
    {
        this->register_mover(mover, target_block_index);
        mover->set_cur_block_index(target_block_index);
    }
    return 0;
}

int Scene::refresh_mover_location_for_watch(GameMover *mover, const MoverCoord &target)
{
	MoverDetail &detail = mover->mover_detail();

    BlockIndexType org_block_index = mover->cur_block_index(), target_block_index = -1,
                   org_block_x = 0, org_block_y = 0, target_block_x = 0, target_block_y = 0;
    if (org_block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= org_block_index)
    {
        MSG_USER("ERROR mover cur_block_index error %ld %d %d[%d,%d]", mover->mover_id(), org_block_index,
                mover->scene_id(), detail.__location.pos_x(), detail.__location.pos_y());
        return ERROR_COORD_ILLEGAL;
    }
    this->cal_block_coord_by_block_index(org_block_index, org_block_x, org_block_y);

    this->cal_block_coord_by_grid_coord(target.pos_x(), target.pos_y(), target_block_x, target_block_y);
    if (this->cal_block_index_by_block_coord(target_block_x, target_block_y, target_block_index) != 0)
    {
        MSG_USER("ERROR target grid coord to index %ld %d[%d,%d]", mover->mover_id(),
                this->scene_id(), target.pos_x(), target.pos_y());
        return ERROR_COORD_ILLEGAL;
    }

    bool is_near_block = this->is_near_block(target, org_block_index);
    if (is_near_block == false)
        this->unregister_mover(mover, org_block_index);
//    this->unregister_near_mover(mover);

    detail.__location = target;

//    this->register_near_mover(mover);
    if (is_near_block == false)
    {
        this->register_mover(mover, target_block_index);
        mover->set_cur_block_index(target_block_index);
    }
    return 0;
}

int Scene::notify_appear(GameMover *mover)
{
    Block_Buffer buff;
    JUDGE_RETURN(mover->make_up_appear_info(&buff) == 0, -1);

    return this->notify_area_info(mover, buff, mover->room_scene_index());
}

int Scene::notify_disappear(GameMover *mover)
{
    Block_Buffer buff;
    mover->make_up_disappear_info(&buff);

    return this->notify_area_info(mover, buff, mover->room_scene_index());
}

int Scene::notify_area_info(GameMover *mover, Block_Buffer &buff, const int room_scene_index)
{
	if (this->is_inited_scene() == false)
	{
		MSG_USER("ERROR scene obj not init_scene %d %d %x", this->scene_id(), this->space_id(), this);
		return -1;
	}

    BlockIndexType block_index = mover->cur_block_index();
    if (block_index < 0)
    {
		if (this->cal_block_index_by_grid_coord(mover->location().pos_x(), mover->location().pos_y(), block_index) != 0)
		{
			MSG_USER("ERROR mover grid coord to index %d[%d,%d]", this->scene_id(), mover->location().pos_x(), mover->location().pos_y());
			return -1;
		}
    }

    BlockIndexSet move_set, prev_move_set;
    this->cal_around_block_by_block_index(block_index, move_set);

    for (BlockIndexSet::iterator iter = move_set.begin(); iter != move_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        scene_block.push_data(buff);

        if (scene_block.__player_map.empty() == false)
        	this->scene_detail_.__has_block_set.insert(*iter);
    }

    return 0;
}

int Scene::notify_fullscene(Block_Buffer &buff, const int room_scene_index)
{
	Int64 i = 0;
	for (SceneDetail::SceneBlockList::iterator iter = this->scene_detail_.__scene_block_list.begin();
			iter != this->scene_detail_.__scene_block_list.end(); ++iter)
	{
		SceneBlock &scene_block = *iter;
		scene_block.push_data(buff);

		if (scene_block.__player_map.empty() == false)
			this->scene_detail_.__has_block_set.insert(i);
		++i;
	}
	return 0;
}

int Scene::notify_fullscene(Message *msg, const int room_scene_index)
{
    int recogn = type_name_to_recogn(msg->GetTypeName());

    ProtoClientHead head;
    head.__recogn = recogn;

    uint32_t len = sizeof(ProtoClientHead), byte_size = 0;
	if (msg != 0)
		byte_size = msg->ByteSize();

	len += byte_size;
	Block_Buffer buff;
	buff.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
	buff.write_uint32(len);
	buff.copy((char *) &head, sizeof(ProtoClientHead));

	if (msg != 0)
	{
		msg->SerializeToArray(buff.get_write_ptr(), buff.writable_bytes());
		buff.set_write_idx(buff.get_write_idx() + byte_size);
	}
    return this->notify_fullscene(buff, room_scene_index);
}

int Scene::fetch_around_appear_info(GameMover *mover, Block_Buffer &buff)
{
    BlockIndexSet move_set;
    BlockIndexType block_x = -1, block_y = -1, block_index = 0;

    this->cal_block_coord_by_grid_coord(mover->location().pos_x(),
    		mover->location().pos_y(), block_x, block_y);
    this->cal_around_block_by_block_coord(block_x, block_y, move_set);

    for (BlockIndexSet::iterator iter = move_set.begin(); iter != move_set.end(); ++iter)
    {
        block_index = *iter;
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
        scene_block.make_up_all_appear_info(buff, mover, false);
    }

    return 0;
}

int Scene::fetch_around_disappear_info(GameMover *mover, Block_Buffer &buff)
{
    BlockIndexSet move_set;
    BlockIndexType block_x = -1, block_y = -1, block_index = 0;

    this->cal_block_coord_by_grid_coord(mover->location().pos_x(),
    		mover->location().pos_y(), block_x, block_y);
    this->cal_around_block_by_block_coord(block_x, block_y, move_set);

    for (BlockIndexSet::iterator iter = move_set.begin(); iter != move_set.end(); ++iter)
    {
        block_index = *iter;
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
        scene_block.make_up_all_disappear_info(buff, mover);
    }

    return 0;
}

int Scene::enter_scene(GameMover *mover, int type)
{
	GameFighter *fighter = dynamic_cast<GameFighter *>(mover);
	this->register_fighter(fighter);

    if (this->mover_map_.bind(mover->mover_id(), mover) != 0)
    {
        MSG_USER("ERROR multi register mover %ld %d %d %d",
                mover->mover_id(), mover->scene_id(), mover->space_id(), this->space_id());
    }

    if (mover->is_player() == true)
    {
        if (this->player_map_.bind(mover->mover_id(), mover) != 0)
            MSG_USER("ERROR multi register player %ld %d %d %d",
                    mover->mover_id(), mover->scene_id(), mover->space_id(), this->space_id());

        MapPlayer *player = dynamic_cast<MapPlayer *>(mover);
        if (player != 0 && player->team_id() > 0)
            this->bind_team_player(player->team_id(), player);
    }
    else if (mover->is_monster() == true)
    {
        this->register_monster(mover);
    }

    if (this->register_mover(mover, -1) != 0)
    {
        MSG_USER("ERROR register to scene %ld %d %d[%d,%d]",
                mover->mover_id(), mover->space_id(), mover->scene_id(),
                mover->location().pixel_x(), mover->location().pixel_y());
    }

    //this->register_near_mover(mover);

    if (mover->is_monster() == true)
    {
        this->increase_mpt_coord(mover, mover->location());
    }

    switch (type)
    {
    case EXIT_SCENE_JUMP:
    {
    	break;
    }

    default:
    {
        this->notify_appear(mover);
    	break;
    }
    }

    return 0;
}

int Scene::exit_scene(GameMover *mover, int type)
{
	GameFighter *fighter = dynamic_cast<GameFighter *>(mover);
	this->unregister_fighter(fighter);

    if (mover->is_player() == true)
    {
    	BlockIndexType org_block_index = mover->cur_block_index();
        if (org_block_index < 0)
            this->cal_block_index_by_grid_coord(mover->location().pos_x(), mover->location().pos_y(), org_block_index);
		if (org_block_index >= 0)
		{
			this->scene_detail_.__scene_block_list[org_block_index].flush_to_mover(mover);
		}

        Block_Buffer buff;
        this->fetch_around_disappear_info(mover, buff);
        mover->respond_from_broad_client(&buff);

        MapPlayer *player = dynamic_cast<MapPlayer *>(mover);
        if (player != 0 && player->team_id() > 0)
        {
            this->unbind_team_player(player->team_id(), player);
        }

        this->player_map_.unbind(mover->mover_id());
    }
    this->mover_map_.unbind(mover->mover_id());

    if (mover->is_monster() == true)
    {
        this->unregister_monster(mover);
    }

    if (type != EXIT_SCENE_JUMP)
    {
    	this->notify_disappear(mover);
    }

    if (this->unregister_mover(mover, mover->cur_block_index(), true) != 0)
    {
        MSG_USER("ERROR unregister to scene %ld %d %d[%d,%d]",
                mover->mover_id(), mover->space_id(), mover->scene_id(),
                mover->location().pixel_x(), mover->location().pixel_y());
    }

    //this->unregister_near_mover(mover);

    mover->set_cur_block_index(-1);
    if (mover->is_monster() == true)
    {
        this->reduce_mpt_coord(mover, mover->location());
    }
    return 0;
}

void Scene::register_fighter(GameFighter* fighter)
{
	JUDGE_RETURN(fighter != NULL, ;);
	this->fighter_map_.bind(fighter->fighter_id(), fighter);
}

void Scene::unregister_fighter(GameFighter* fighter)
{
	JUDGE_RETURN(fighter != NULL, ;);
	this->fighter_map_.unbind(fighter->fighter_id());
}

int Scene::validate_ai_pickup(GameAI* game_ai, MapPlayer* player)
{
	return 0;
}

int Scene::modify_ai_hurt_value(GameAI* game_ai, int src_value, Int64 attackor_id)
{
	return src_value;
}

int Scene::handle_ai_alive_recycle(GameAI* game_ai)
{
	return 0;
}

int Scene::handle_ai_die(GameAI* game_ai, Int64 benefited_attackor)
{
	return 0;
}

int Scene::handle_boss_die_action(GameAI* game_ai)
{
	return 0;
}

int Scene::makeup_ai_appear_info(GameAI* game_ai, Proto80400111* appear_info)
{
	AIDetail& ai_detail = game_ai->ai_detail();
	appear_info->set_sub_name(ai_detail.league_name_);
	appear_info->set_sub_id(ai_detail.league_index_);
	return 0;
}

int Scene::makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info)
{
	return 0;
}

IntPair Scene::fetch_addition_exp()
{
	return IntPair(0, 0);
}

int Scene::fetch_relive_protected_time(Int64 role)
{
	static int last_time = CONFIG_INSTANCE->const_set("relive_protected_time");
	return last_time;
}

int Scene::fetch_enter_buff(IntVec& buff_vec, Int64 role)
{
	const Json::Value& set_conf = this->set_conf();
	JUDGE_RETURN(set_conf.isMember("buff_list") == true, -1);

	GameCommon::json_to_t_vec(buff_vec, set_conf["buff_list"]);
	return 0;
}

int Scene::handle_init_ai(GameAI* game_ai)
{
	return 0;
}

int Scene::is_ai_area_recycle(GameAI* game_ai)
{
	const Json::Value& patrol_route = CONFIG_INSTANCE->prop_monster(
			game_ai->ai_sort())["patrol_path"];

	int size = patrol_route.size();
	JUDGE_RETURN(size > 0, false);

	MoverCoord aim_coord;
	aim_coord.set_pixel(patrol_route[size - 1][0u].asInt(),
			patrol_route[size - 1][1u].asInt());

	return ::check_coord_distance(game_ai->location(), aim_coord, 3,
			GameEnum::DEFUALT_MAPPING_FACTOR, GameEnum::DEFAULT_AI_PATH_GRID);
}

int Scene::handle_fighter_hurt(GameFighter* fighter, Int64 benefited_attackor, int hurt_value)
{
	return 0;
}

int Scene::handle_player_hurt(MapPlayer* player, Int64 benefited_attackor, int hurt_value)
{
	return 0;
}

int Scene::handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value)
{
	return 0;
}

int Scene::find_fighter(const int64_t fighter_id, GameFighter *&fighter)
{
    return this->fighter_map_.find(fighter_id, fighter);
}

GameFighter* Scene::find_fighter(Int64 fighter_id)
{
    GameFighter* fighter = NULL;
    this->fighter_map_.find(fighter_id, fighter);
    return fighter;
}

int Scene::fetch_player_set(LongVec& player_set)
{
	player_set.reserve(this->player_map_.size());

	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		player_set.push_back(iter->first);
	}

	return 0;
}

int Scene::transfer_player_out(MapPlayerEx* player, int exit_type)
{
	if (player->is_need_send_message() == false)
	{
		// offline player
		player->offline_exit_scene();
		player->offline_sign_out();
	}
	else
	{
		switch (exit_type)
		{
		case GameEnum::EXIT_TYPE_PREVE:
		{
			return player->transfer_to_prev_scene();
		}
		case GameEnum::EXIT_TYPE_SAVE:
		{
			return player->transfer_to_save_scene();
		}
		}
	}

	return 0;
}

GameAI* Scene::find_ai(Int64 ai_id)
{
	GameMover* game_ai = NULL;
	JUDGE_RETURN(this->mover_map_.find(ai_id, game_ai) == 0, NULL);

	return dynamic_cast<GameAI*>(game_ai);
}

MapPlayerEx* Scene::find_player(Int64 role_id)
{
	GameMover* player = NULL;
	JUDGE_RETURN(this->player_map_.find(role_id, player) == 0, NULL);

	return dynamic_cast<MapPlayerEx*>(player);
}

MapPlayerEx* Scene::fetch_benefited_player(Int64 attackor)
{
	GameFighter* fighter = this->find_fighter(attackor);
	JUDGE_RETURN(fighter != NULL, NULL);

	if (fighter->is_beast())
	{
		MapBeast* beast = dynamic_cast<MapBeast *>(fighter);
		JUDGE_RETURN(beast != NULL, NULL);

		return this->find_player(beast->master_id());
	}
    else if (fighter->is_monster())
    {
        GameAI *game_ai = dynamic_cast<GameAI *>(fighter);
        JUDGE_RETURN(game_ai != NULL, NULL);
        return this->find_player(game_ai->caller());
    }

	return fighter->self_player();
}

GameMover* Scene::find_mover(Int64 mover_id)
{
	GameMover* mover = NULL;
	JUDGE_RETURN(this->mover_map_.find(mover_id, mover) == 0, NULL);

	return mover;
}

void Scene::recycle_one_mover(Int64 ai_id)
{
	GameMover* mover = this->find_mover(ai_id);
	JUDGE_RETURN(mover != NULL, ;);

	mover->exit_scene();
	mover->sign_out();
}

void Scene::recycle_all_monster(void)
{
	typedef std::vector<GameMover*> MonsterVec;

	MonsterVec mover_vc;
	for (MoverMap::iterator iter = this->mover_map_.begin();
			iter != this->mover_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second->is_player() == false);
		mover_vc.push_back(iter->second);
	}

    for (MonsterVec::iterator iter = mover_vc.begin();
    		iter != mover_vc.end(); ++iter)
    {
    	GameMover* monster = *iter;
    	JUDGE_CONTINUE(monster != NULL);

    	monster->exit_scene();
    	monster->sign_out();
    }
}

int Scene::notify_shield_info(BasicStatus* status)
{
	Proto80401024 respond;
	respond.set_scene_id(this->scene_id());
	respond.set_total_blood(status->__value4);
	respond.set_cur_blood(status->__value1);

	this->notify_all_player_msg(NOTIFY_CREATE_BOSS_SHIELD, &respond);
	return 0;
}

void Scene::notify_all_player_msg(int recogn, Message* msg)
{
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameMover* player = iter->second;
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(recogn, msg);
	}
}

void Scene::notify_all_player_msg(const LongVec& player_set, int recogn, Message* msg)
{
	for (LongVec::const_iterator iter = player_set.begin();
			iter != player_set.end(); ++iter)
	{
		MapPlayerEx* player = this->find_player(*iter);
		JUDGE_CONTINUE(player != NULL);
		player->respond_to_client(recogn, msg);
	}
}

void Scene::restore_all_player_blood_full()
{
	for (MoverMap::iterator iter = this->player_map_.begin();
			iter != this->player_map_.end(); ++iter)
	{
		GameFighter* player = this->find_fighter(iter->first);
		JUDGE_CONTINUE(player != NULL);
		JUDGE_CONTINUE(player->is_death() == true);
		player->fighter_restore_all();
		player->notify_mover_cur_location();
	}
}

void Scene::notify_all_player_exit(int exit_type, int player_type)
{
	LongVec player_set;
	this->fetch_player_set(player_set);

	for (LongVec::iterator iter = player_set.begin(); iter != player_set.end(); ++iter)
	{
		MapPlayerEx* player = NULL;
		JUDGE_CONTINUE(MAP_MONITOR->find_player_with_offline(*iter, player) == 0);

		switch(player_type)
		{
		case GameEnum::PLAYER_TYPE_BOTH:
		{
			// online and offline
			this->transfer_player_out(player, exit_type);
			break;
		}

		case GameEnum::PLAYER_TYPE_OFFLINE:
		{
			// offline
			JUDGE_CONTINUE(player->is_need_send_message() == false);
			this->transfer_player_out(player, exit_type);
			break;
		}

		case GameEnum::PLAYER_TYPE_ONLINE:
		{
			// offline
			JUDGE_CONTINUE(player->is_need_send_message() == true);
			this->transfer_player_out(player, exit_type);
			break;
		}
		}
	}
}

void Scene::notiy_player_msg(Int64 role_id, int recogn, Message* msg)
{
	GameMover* player = this->find_mover(role_id);
	JUDGE_RETURN(player != NULL, ;);

	player->respond_to_client(recogn, msg);
}

Scene::FighterMap &Scene::fighter_map(void)
{
    return this->fighter_map_;
}

Scene::MoverMap &Scene::player_map(void)
{
    return this->player_map_;
}

Scene::MoverMap &Scene::mover_map(void)
{
	return this->mover_map_;
}

Scene::MoverMap &Scene::monster_map(void)
{
    return this->monster_map_;
}

const Json::Value& Scene::conf()
{
	return CONFIG_INSTANCE->scene(this->scene_id());
}

const Json::Value& Scene::set_conf()
{
	return CONFIG_INSTANCE->scene_set(this->scene_id());
}

const Json::Value& Scene::layout_sort(int monster_sort)
{
	const Json::Value &layout = this->conf()["layout"];
	for (uint i = 0; i < layout.size(); ++i)
	{
		JUDGE_CONTINUE(layout[i]["monster_sort"].asInt() == monster_sort);
		return layout[i];
	}
	return Json::Value::null;
}

const Json::Value& Scene::layout_index(int layout_index)
{
	const Json::Value &layout = this->conf()["layout"];
	return layout[layout_index];
}

MoverCoord Scene::rand_coord(const MoverCoord& center, int radius, GameFighter *fighter)
{
	JUDGE_RETURN(radius > 0, center);

	int rand_times = 0;
	int diameter = 2 * radius;
	int scene_id = this->scene_id();

	while (rand_times  < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
		rand_times += 1;

		int offset_x = std::rand() % diameter;
		int offset_y = std::rand() % diameter;

		int pos_x = std::max(center.pos_x() - radius + offset_x, 1);
		int pos_y = std::max(center.pos_y() - radius + offset_y, 1);

		MoverCoord coord(pos_x, pos_y);
		JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(scene_id, coord) == true);

		return coord;
	}

	return center;
}

MoverCoord Scene::rand_coord_by_pixel_radius(const MoverCoord &center, int pixel_radius)
{
    JUDGE_RETURN(pixel_radius > 0, center);

    MoverCoord coord;
    int pixel_x_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_radius);

    int pixel_y_base = int(sqrt((pixel_radius * pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
    if (pixel_y_base > 0)
        pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base);

    int pixel_x = center.pixel_x() + pixel_x_base, pixel_y = center.pixel_y() + pixel_y_base;

    int rand_times = 0;
	while ((rand_times++) < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
        pixel_x_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_radius);

        pixel_y_base = int(sqrt((pixel_radius * pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
        if (pixel_y_base > 0)
            pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base);

        pixel_x = center.pixel_x() + pixel_x_base;
        pixel_y = center.pixel_y() + pixel_y_base;

        coord.set_pixel(pixel_x, pixel_y);
        JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(this->scene_id(), coord) == true);

        return coord;
    }
    return center;
}

MoverCoord Scene::rand_coord_by_pixel_ring(const MoverCoord &center, int out_pixel_radius, int in_pixel_radius)
{
    JUDGE_RETURN(out_pixel_radius > 0, center);

    MoverCoord coord;
    int pixel_x_base = 0;
    if (out_pixel_radius > 0)
        pixel_x_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % out_pixel_radius);

    int pixel_y_in_base = 0, pixel_y_out_base = 0, pixel_y_base = 0;
    if (std::abs(pixel_x_base) > in_pixel_radius)
    {
        pixel_y_base = int(sqrt((out_pixel_radius * out_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
        if (pixel_y_base > 0)
            pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base);
    }
    else
    {
		pixel_y_in_base = int(sqrt((in_pixel_radius * in_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR)),
		pixel_y_out_base = int(sqrt((out_pixel_radius * out_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
		pixel_y_base = pixel_y_out_base - pixel_y_in_base;
		if (pixel_y_base > 0)
			pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base + pixel_y_in_base);
    }

    int pixel_x = center.pixel_x() + pixel_x_base, pixel_y = center.pixel_y() + pixel_y_base;

    int rand_times = 0;
	while ((rand_times++) < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
        pixel_x_base = 0;
        if (out_pixel_radius > 0)
            pixel_x_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % out_pixel_radius);

        if (std::abs(pixel_x_base) > in_pixel_radius)
        {
            pixel_y_base = int(sqrt((out_pixel_radius * out_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
            if (pixel_y_base > 0)
                pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base);
        }
        else
        {
    		pixel_y_in_base = int(sqrt((in_pixel_radius * in_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR)),
    		pixel_y_out_base = int(sqrt((out_pixel_radius * out_pixel_radius - pixel_x_base * pixel_x_base) / MAPPING_FACTOR));
    		pixel_y_base = pixel_y_out_base - pixel_y_in_base;
    		if (pixel_y_base > 0)
    			pixel_y_base = (std::rand() % 2 == 0 ? 1 : -1) * (std::rand() % pixel_y_base + pixel_y_in_base);
        }

        pixel_x = center.pixel_x() + pixel_x_base;
        pixel_y = center.pixel_y() + pixel_y_base;

        coord.set_pixel(pixel_x, pixel_y);
        JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(this->scene_id(), coord) == true);

        return coord;
    }
    return center;
}

MoverCoord Scene::rand_dynamic_coord(const MoverCoord& center, int radius)
{
	JUDGE_RETURN(radius > 0, center);

	int rand_times = 0;
	int scene_id = this->scene_id();

	DynamicMoverCoord dynamic_center(center.pixel_x(), center.pixel_y());
	while (rand_times  < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
		++rand_times;

		int toward_x = std::rand() % 2 == 0 ? 1 : -1;
		int toward_y = std::rand() % 2 == 0 ? 1 : -1;

		int offset_x = (std::rand() % radius + 2) * toward_x;
		int offset_y = (std::rand() % radius + 2) * toward_y;

		int pos_x = center.pos_x() + offset_x;
		int pos_y = center.pos_y() + offset_y;

		DynamicMoverCoord dynamic_coord;
		dynamic_coord.set_dynamic_pos(pos_x, pos_y);
		JUDGE_CONTINUE(this->is_movable_dynamic_coord(dynamic_coord) == true);

		MoverCoord coord;
		coord.set_pixel(dynamic_coord.pixel_x(), dynamic_coord.pixel_y());
		JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(scene_id, coord) == true);

		// 处理边界点问题；
		coord.set_pos(coord.pos_x(), coord.pos_y());
		return coord;
	}

	return center;
}

MoverCoord Scene::rand_toward(const MoverCoord& center, int offset)
{
	int rand_times = 0;
	int scene_id = this->scene_id();

	while (rand_times  < GameEnum::DEFAULT_MAX_RT_TIMES)
	{
		rand_times += 1;

		int toward_x = std::rand() % 2 == 0 ? 1 : -1;
		int toward_y = std::rand() % 2 == 0 ? 1 : -1;

		int pos_x = std::max(center.pos_x() + toward_x * offset, 1);
		int pos_y = std::max(center.pos_y() + toward_y * offset, 1);

		MoverCoord coord(pos_x, pos_y);
		JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(scene_id, coord) == true);

		return coord;
	}

	return center;
}

MoverCoord Scene::rand_dynamic_toward(const MoverCoord& center, int offset)
{
	JUDGE_RETURN(offset > 0, center);

	int rand_times = 0;
	int scene_id = this->scene_id();

	DynamicMoverCoord dynamic_center(center.pixel_x(), center.pixel_y());
	while (rand_times  < GameEnum::DEFAULT_MAX_RT_TIMES)
	{
		++rand_times;

		int offset_x = (std::rand() % 2 == 0 ? -1 : 1) * offset;
		int offset_y = (std::rand() % 2 == 0 ? -1 : 1) * offset;

		int pos_x = dynamic_center.dynamic_pos_x() + offset_x;
		int pos_y = dynamic_center.dynamic_pos_y() + offset_y;

		DynamicMoverCoord dynamic_coord;
		dynamic_coord.set_dynamic_pos(pos_x, pos_y);
		JUDGE_CONTINUE(this->is_movable_dynamic_coord(dynamic_coord) == true);

		MoverCoord coord;
		coord.set_pixel(dynamic_coord.pixel_x(), dynamic_coord.pixel_y());
		JUDGE_CONTINUE(GameCommon::is_movable_coord(scene_id, coord) == true);

		return coord;
	}

	return center;
}

MoverCoord Scene::rand_ai_move_coord(GameAI* game_ai)
{
	int rand_times = 0;
	MoverCoord aim_coord = game_ai->location();

	while (rand_times < GameEnum::DEFAULT_MAX_RAND_TIMES)
	{
		++rand_times;

        // 保证至少距离有5格
        int toward_x = rand() % 3 - 1;
        int toward_y = 0;
        if (toward_x == 0)
            toward_y = ((rand() % 2) == 0 ? 1 : -1);
        else
            toward_y = rand() % 3 - 1;
        int offset_x = 0, offset_y = 0;
        if (toward_x != 0)
            offset_x = rand() % 6 + 1;
        if (offset_x >= 5)
            offset_y = rand() % 6 + 1;
        else
            offset_y = rand() % 2 + 5;

		aim_coord.set_pos(game_ai->location().pos_x() + offset_x * toward_x, game_ai->location().pos_y() + offset_y * toward_y);

        JUDGE_CONTINUE(GameCommon::is_movable_coord_no_border(game_ai->scene_id(), aim_coord) == true);

		int between_distance = ::coord_offset_grid(aim_coord, game_ai->birth_coord());
        JUDGE_CONTINUE(between_distance <= GameEnum::AI_BACK_DISTANCE);
		break;
	}

	return aim_coord;
}

MoverCoord Scene::fetch_random_coord(void)
{
    MoverCoord random_coord;
    int pos_x = 0, pos_y = 0;
    const GameConfig::CoordIndexList &coord_list = CONFIG_INSTANCE->move_coord_list(this->scene_id());

    JUDGE_RETURN(coord_list.size() > 0, random_coord);

    int index = std::rand() % coord_list.size();
    GridIndexType grid_index = coord_list[index];
    this->calc_grid_coord_by_mpt_index(pos_x, pos_y, grid_index);
    random_coord.set_pos(pos_x, pos_y);

    int loop = 0;
    while (++loop < 20)
    {
        if (GameCommon::is_movable_coord_no_border(this->scene_id(), random_coord) == true)
            break;

        index = std::rand() % coord_list.size();
        GridIndexType grid_index = coord_list[index];
        this->calc_grid_coord_by_mpt_index(pos_x, pos_y, grid_index);
        random_coord.set_pos(pos_x, pos_y);
    }
    return random_coord;
}

MoverCoord Scene::fetch_beast_coord(const MoverCoord& location)
{
	static int MASTER_BEAST_DISTANCE = 2;	//主人宠物间距
	return this->rand_toward(location, MASTER_BEAST_DISTANCE);
}

bool Scene::is_validate_around_fighter(GameFighter* fighter, const MoverCoord& center,
		int radius)
{
	JUDGE_RETURN(Scene::is_validate_around_mover(fighter,
			center, radius) == true, false);
	JUDGE_RETURN(fighter->is_death() == false, false);
	return true;
}

bool Scene::is_validate_around_mover(GameMover* mover,const MoverCoord& center,
		int radius, const int mapping_factor)
{
	JUDGE_RETURN(mover != NULL, false);
	JUDGE_RETURN(mover->is_enter_scene() == true, false);

	JUDGE_RETURN(check_coord_distance(center, mover->location(),
			radius, mapping_factor) == true, false);

	return true;
}

bool Scene::is_validate_rect_mover(GameMover *mover, const MoverCoord &pointA,
            const MoverCoord &pointB, const MoverCoord &pointC, const MoverCoord &pointD)
{
	int min_x = pointA.pixel_x(), max_x = pointA.pixel_x(), min_y = pointA.pixel_y(), max_y = pointA.pixel_y();
	min_x = (min_x > pointB.pixel_x() ? pointB.pixel_x() : min_x);
	min_x = (min_x > pointC.pixel_x() ? pointC.pixel_x() : min_x);
	min_x = (min_x > pointD.pixel_x() ? pointD.pixel_x() : min_x);
	max_x = (max_x < pointB.pixel_x() ? pointB.pixel_x() : max_x);
	max_x = (max_x < pointC.pixel_x() ? pointC.pixel_x() : max_x);
	max_x = (max_x < pointD.pixel_x() ? pointD.pixel_x() : max_x);
	min_y = (min_y > pointB.pixel_y() ? pointB.pixel_y() : min_y);
	min_y = (min_y > pointC.pixel_y() ? pointC.pixel_y() : min_y);
	min_y = (min_y > pointD.pixel_y() ? pointD.pixel_y() : min_y);
	max_y = (max_y < pointB.pixel_y() ? pointB.pixel_y() : max_y);
	max_y = (max_y < pointC.pixel_y() ? pointC.pixel_y() : max_y);
	max_y = (max_y < pointD.pixel_y() ? pointD.pixel_y() : max_y);
    if (mover->location().pixel_x() < min_x || mover->location().pixel_x() > max_x || 
            mover->location().pixel_y() < min_y || mover->location().pixel_y() > max_y)
        return false;

    int flag_ba = (pointB.pixel_x() - pointA.pixel_x()) * (mover->location().pixel_y() - pointA.pixel_y()) -
    		(pointB.pixel_y() - pointA.pixel_y()) * (mover->location().pixel_x() - pointA.pixel_x());

    int flag_cb = (pointC.pixel_x() - pointB.pixel_x()) * (mover->location().pixel_y() - pointB.pixel_y()) -
    		(pointC.pixel_y() - pointB.pixel_y()) * (mover->location().pixel_x() - pointB.pixel_x());

    int flag_dc = (pointD.pixel_x() - pointC.pixel_x()) * (mover->location().pixel_y() - pointC.pixel_y()) -
    		(pointD.pixel_y() - pointC.pixel_y()) * (mover->location().pixel_x() - pointC.pixel_x());

    int flag_ad = (pointA.pixel_x() - pointD.pixel_x()) * (mover->location().pixel_y() - pointD.pixel_y()) -
    		(pointA.pixel_y() - pointD.pixel_y()) * (mover->location().pixel_x() - pointD.pixel_x());

    if ((flag_ba >= 0 && flag_cb >= 0 && flag_dc >= 0 && flag_ad >= 0) ||
            (flag_ba <= 0 && flag_cb <= 0 && flag_dc <= 0 && flag_ad <= 0))
        return true;
    return false;
}

bool Scene::is_validate_sector_mover(GameMover *mover,
    		const MoverCoord &center, const MoverCoord &toward_point, const int range, const double angle)
{
	JUDGE_RETURN(mover->location() != center, true);
    JUDGE_RETURN(mover->is_enter_scene() == true, false);
    JUDGE_RETURN(check_coord_distance(center, mover->location(), range + 1) == true, false);

	double cos_angle = ::cos(PI / 180.0 * angle);
    double vet_a_x = toward_point.pixel_x() - center.pixel_x(),
    		vet_a_y = toward_point.pixel_y() - center.pixel_y(),
    		vet_b_x = mover->location().pixel_x() - center.pixel_x(),
    		vet_b_y = mover->location().pixel_y() - center.pixel_y();
    double vet_a_len = ::sqrt(vet_a_x * vet_a_x + vet_a_y * vet_a_y),
    		vet_b_len = ::sqrt(vet_b_x * vet_b_x + vet_b_y * vet_b_y);
    double cos_val = (vet_a_x * vet_b_x + vet_a_y * vet_b_y) / (vet_a_len * vet_b_len);
    JUDGE_RETURN(cos_val >= cos_angle, false);

    return true;
}

bool Scene::is_validate_around_mover(GameMover* target, GameMover* attack, int radius)
{
	int total_radius = radius + target->fetch_mover_volume() + attack->fetch_mover_volume();
	return Scene::is_validate_around_mover(target, attack->location(), total_radius);
}

int Scene::fetch_all_around_fighter(GameMover *mover, Scene::MoverMap& fighter_map, const MoverCoord& center, int radius, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator mover_iter = scene_block.__mover_map.begin();
                mover_iter != scene_block.__mover_map.end(); ++mover_iter)
        {
        	JUDGE_CONTINUE(dynamic_cast<GameFighter *>(mover_iter->second) != 0 && mover->mover_id() != mover_iter->first);
            JUDGE_CONTINUE(mover_iter->second->is_player() == true ||
                    mover_iter->second->is_monster() == true);
            JUDGE_CONTINUE(this->is_validate_around_mover(mover_iter->second, center, radius) == true);
            fighter_map[mover_iter->first] = mover_iter->second;
            if (int(fighter_map.size()) >= max_amount)
                break;
        }
    }
    if (fighter_map.size() <= 0)
        return -1;
    return 0;
}

int Scene::fetch_all_around_player(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center, int radius, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator player_iter = scene_block.__player_map.begin();
                player_iter != scene_block.__player_map.end(); ++player_iter)
        {
            JUDGE_CONTINUE(this->is_validate_around_mover(player_iter->second, center, radius) == true);
            GameFighter *fighter = dynamic_cast<GameFighter *>(player_iter->second);
            JUDGE_CONTINUE(fighter != 0 && fighter->is_death() == false);
            player_map[player_iter->first] = player_iter->second;
            if (int(player_map.size()) >= max_amount)
                break;
        }
    }
    if (player_map.size() <= 0)
        return -1;

    return 0;
}

int Scene::fetch_all_around_fighter_mapping(GameMover *mover, Scene::MoverMap& fighter_map, const MoverCoord& center, int radius, const int mapping_factor/*=2*/, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator mover_iter = scene_block.__mover_map.begin();
                mover_iter != scene_block.__mover_map.end(); ++mover_iter)
        {
        	JUDGE_CONTINUE(dynamic_cast<GameFighter *>(mover_iter->second) != 0 && mover->mover_id() != mover_iter->first);
            JUDGE_CONTINUE(mover_iter->second->is_player() == true ||
                    mover_iter->second->is_monster() == true);
            JUDGE_CONTINUE(this->is_validate_around_mover(mover_iter->second, center, radius, mapping_factor) == true);
            fighter_map[mover_iter->first] = mover_iter->second;
            if (int(fighter_map.size()) >= max_amount)
                break;
        }
    }
    if (fighter_map.size() <= 0)
        return -1;
    return 0;
}

int Scene::fetch_all_around_player_mapping(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center, int radius, const int mapping_factor/*=2*/, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator player_iter = scene_block.__player_map.begin();
                player_iter != scene_block.__player_map.end(); ++player_iter)
        {
            JUDGE_CONTINUE(this->is_validate_around_mover(player_iter->second, center, radius, mapping_factor) == true);
            GameFighter *fighter = dynamic_cast<GameFighter *>(player_iter->second);
            JUDGE_CONTINUE(fighter != 0 && fighter->is_death() == false);
            player_map[player_iter->first] = player_iter->second;
            if (int(player_map.size()) >= max_amount)
                break;
        }
    }
    if (player_map.size() <= 0)
        return -1;

    return 0;
}

int Scene::fetch_all_around_player_in_ring(GameMover *mover, Scene::MoverMap& player_map,
		const MoverCoord& center, int inner_radius, int outer_radius, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator player_iter = scene_block.__player_map.begin();
                player_iter != scene_block.__player_map.end(); ++player_iter)
        {
            JUDGE_CONTINUE(this->is_validate_around_mover(player_iter->second,
            		center, outer_radius, GameEnum::CLIENT_MAPPING_FACTOR) == true);

            JUDGE_CONTINUE(check_coord_distance(center, player_iter->second->location(),
            		std::max(inner_radius - 2, 0), GameEnum::CLIENT_MAPPING_FACTOR) == false);

            GameFighter *fighter = dynamic_cast<GameFighter *>(player_iter->second);
            JUDGE_CONTINUE(fighter != 0 && fighter->is_death() == false);
            player_map[player_iter->first] = player_iter->second;
            if (int(player_map.size()) >= max_amount)
                break;
        }
    }
    if (player_map.size() <= 0)
        return -1;

    return 0;
}

int Scene::fetch_all_around_monster(GameMover *mover, Scene::MoverMap &monster_map,
		const MoverCoord &center, int radius, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator other_iter = scene_block.__other_mover_map.begin();
                other_iter != scene_block.__other_mover_map.end(); ++other_iter)
        {
            JUDGE_CONTINUE(other_iter->second->is_monster() == true);
            JUDGE_CONTINUE(other_iter->second->mover_id() != mover->mover_id());
            JUDGE_CONTINUE(this->is_validate_around_mover(other_iter->second, center, radius) == true);
            GameFighter *fighter = dynamic_cast<GameFighter *>(other_iter->second);
            JUDGE_CONTINUE(fighter != 0 && fighter->is_death() == false);
            JUDGE_CONTINUE(fighter->is_enter_scene() == true);
            monster_map[other_iter->first] = other_iter->second;
            if (int(monster_map.size()) >= max_amount)
                break;
        }
    }
    if (monster_map.size() <= 0)
        return -1;

    return 0;
}

int Scene::fetch_all_around_boss(GameMover *mover, Scene::MoverMap &boss_map,
		const MoverCoord &center, int radius, const int max_amount)
{
    int boss_sort = 0;
    for (BLongSet::iterator boss_iter = this->scene_detail_.boss_sort_set_.begin();
            boss_iter != this->scene_detail_.boss_sort_set_.end(); ++boss_iter)
    {
        boss_sort = *boss_iter;
        SceneDetail::SortAIMap::iterator ai_iter = this->scene_detail_.ai_sort_map_.find(boss_sort);
        if (ai_iter == this->scene_detail_.ai_sort_map_.end())
            continue;
        BLongSet &id_set = ai_iter->second;
        for (BLongSet::iterator id_iter = id_set.begin();
                id_iter != id_set.end(); ++id_iter)
        {
            GameAI *game_ai = this->find_ai(*id_iter);
            JUDGE_CONTINUE(game_ai != NULL);
            JUDGE_CONTINUE(game_ai->is_enter_scene());
            JUDGE_CONTINUE(game_ai->is_death() == false);
            JUDGE_CONTINUE(this->is_validate_around_mover(game_ai, center, radius) == true);

            boss_map[game_ai->ai_id()] = game_ai;
        }
    }
    if (boss_map.size() <= 0)
        return -1;

    return 0;
}

int Scene::fetch_all_sector_fighter(GameMover *mover, Scene::MoverMap &fighter_map, 
            const MoverCoord &center, const MoverCoord &skill_coord, 
            const int radius, const double angle, const int max_amount)
{
	BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    double cos_angle = ::cos(PI / 180.0 * angle);
    double vet_a_x = skill_coord.pixel_x() - center.pixel_x(),
    		vet_a_y = skill_coord.pixel_y() - center.pixel_y();
    double vet_a_len = ::sqrt(vet_a_x * vet_a_x + vet_a_y * vet_a_y), vet_b_x = 0, vet_b_y = 0, vet_b_len = 0;

    GameFighter *fighter = 0;
    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator mover_iter = scene_block.__mover_map.begin();
                mover_iter != scene_block.__mover_map.end(); ++mover_iter)
        {
        	fighter = dynamic_cast<GameFighter *>(mover_iter->second);
        	JUDGE_CONTINUE(fighter != 0  && mover->mover_id() != mover_iter->first);
            JUDGE_CONTINUE(fighter->is_player() == true || fighter->is_monster() == true);
            JUDGE_CONTINUE(this->is_validate_around_mover(fighter, skill_coord, radius) == true);
            vet_b_x = fighter->location().pixel_x() - center.pixel_x();
            vet_b_y = fighter->location().pixel_y() - center.pixel_y();
            vet_b_len = ::sqrt(vet_b_x * vet_b_x + vet_b_y * vet_b_y);
            double cos_val = (vet_a_x * vet_b_x + vet_a_y * vet_b_y) / (vet_a_len * vet_b_len);

            MSG_DEBUG("sector %d %d(%d,%d) %f %f", fighter->fighter_id(), fighter->scene_id(),
            		fighter->location().pixel_x(), fighter->location().pixel_y(), cos_angle, cos_val);

            if (cos_val < cos_angle)
            	continue;
            fighter_map[mover_iter->first] = mover_iter->second;
            if (int(fighter_map.size()) >= max_amount)
                break;
        }
    }
    return 0;
}

int Scene::fetch_all_rect_fighter(GameMover *mover, Scene::MoverMap &fighter_map,
		const MoverCoord& center, double angle, int width, int height, const SubObj& sub)
{
	double radian = angle * PI / 180.0;

    MoverCoord pointA, pointB, pointC, pointD;
    center_to_rect(pointA, pointB, pointC, pointD, center, radian, width, height);
    MSG_DEBUG("rect calc (%f %f)(%d,%d)", radian, angle, center.pixel_x(), center.pixel_y());

    BlockIndexType center_block_index = -1;
    int ret = this->cal_block_index_by_grid_coord(center.pos_x(), center.pos_y(), center_block_index);
    JUDGE_RETURN(ret == 0, ret);

    GameFighter *fighter = 0;
    BlockIndexSet around_set;
    ret = this->cal_around_block_by_block_index(center_block_index, around_set);
    for (BlockIndexSet::iterator iter = around_set.begin(); iter != around_set.end(); ++iter)
    {
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[*iter];
        for (MoverMap::iterator mover_iter = scene_block.__mover_map.begin();
                mover_iter != scene_block.__mover_map.end(); ++mover_iter)
        {
        	fighter = dynamic_cast<GameFighter *>(mover_iter->second);
        	JUDGE_CONTINUE(fighter != 0  && mover->mover_id() != mover_iter->first);

        	if (sub.val2_ == 0)
        	{
        		JUDGE_CONTINUE(fighter->is_player() == true || fighter->is_monster() == true);
        	}
        	else if (sub.val2_ == 1)
        	{
        		JUDGE_CONTINUE(fighter->is_player() == true);
        	}
        	else if (sub.val2_ == 2)
        	{
        		JUDGE_CONTINUE(fighter->is_monster() == true);
        	}

            JUDGE_CONTINUE(this->is_validate_rect_mover(fighter, pointA, pointB, pointC, pointD) == true);

            fighter_map[mover_iter->first] = mover_iter->second;
            if (int(fighter_map.size()) >= sub.val1_)
            {
                return 0;
            }
        }
    }

    return 0;
}

BLongSet *Scene::ai_sort_set(const int sort)
{
    SceneDetail::SortAIMap::iterator iter = this->scene_detail_.ai_sort_map_.find(sort);
    if (iter == this->scene_detail_.ai_sort_map_.end())
        return 0;
    return &(iter->second);
}

BLongSet *Scene::ai_group_set(const int group_id)
{
    SceneDetail::GroupAIMap::iterator iter = this->scene_detail_.ai_group_map_.find(group_id);
    if (iter == this->scene_detail_.ai_group_map_.end())
        return 0;
    return &(iter->second);
}

Block_Buffer *Scene::pop_block(void)
{
    return this->monitor_->pop_block();
}

int Scene::push_block(Block_Buffer *pbuff)
{
    return this->monitor_->push_block(pbuff);
}

#ifndef NO_BROAD_PORT
Sender *Scene::client_sender(const int index)
{
    return this->monitor_->client_sender(index);
}
#endif

int Scene::summon_monster(GameFighter *fighter, const Json::Value &effect_json)
{
    // 注意此接口召唤出来的怪必须有一个定时自动消失的功能，否则会一直存在到被死亡
    if (effect_json["sort"].isArray())
    {
        int monster_sort = 0;
        Int64 ai_id = 0;
        GameAI *game_ai = 0, *fighter_ai = 0;
        for (uint i = 0; i < effect_json["sort"].size(); ++i)
        {
            monster_sort = effect_json["sort"][i].asInt();
            if (effect_json.isMember("max_summon_amount"))
            {
            	int max_summon_amount = effect_json["max_summon_amount"].asInt();
            	if (fighter->alive_summon_ai_size() >= max_summon_amount)
            		return -1;
            }

            ai_id = AIMANAGER->generate_monster_by_sort(monster_sort, fighter->location(), this);

            game_ai = AIMANAGER->ai_package()->find_object(ai_id);
            if (game_ai == 0)
                return -1;

            if (effect_json["copy_blood"].asInt() == 1)
            {
            	game_ai->modify_blood_by_levelup(fighter->fight_detail().__blood);
            }

            fighter->insert_summon_ai(ai_id);
            game_ai->set_caller(fighter->fighter_id());
            game_ai->set_self_owner(fighter->fighter_id());
            game_ai->set_camp_id(fighter->camp_id());
            
            fighter_ai = dynamic_cast<GameAI *>(fighter);
            if (fighter_ai != 0)
            {
                game_ai->set_group_id(fighter_ai->group_id());
                if(effect_json["no_self_attack"].asInt() != 1)
                {
                	game_ai->set_aim_object(fighter_ai->aim_object_id());
                }
                else
                {
                	game_ai->set_aim_object(0);
                }
            }
        }
    }
    return 0;
}

int Scene::start_special_layout(const int layout_index)
{
	return 0;
}

void Scene::set_protect_npc(const int sort, const Int64 ai_id)
{
    return;
}

int Scene::protect_npc_sort(void)
{
    return 0;
}

Int64 Scene::protect_npc_id(void)
{
    return 0;
}

SceneDetail::MptCoordList &Scene::dynamic_mpt(GameMover *mover)
{
    return this->scene_detail_.__dynamic_mpt;
}

int Scene::calc_mpt_index_by_coord(const MoverCoord &coord)
{
    return this->scene_detail_.__mpt_x_len * coord.pos_y() + coord.pos_x();
}

int Scene::calc_grid_coord_by_mpt_index(int &grid_x, int &grid_y, const int mpt_index)
{
    grid_y = mpt_index / this->scene_detail_.__mpt_x_len;
    grid_x = mpt_index % this->scene_detail_.__mpt_x_len;
    return 0;
}

int Scene::calc_dynamic_mpt_index_by_coord(const DynamicMoverCoord &coord)
{
	return this->scene_detail_.__dynamic_x_len * coord.dynamic_pos_y() + coord.dynamic_pos_x();
}

bool Scene::is_movable_dynamic_coord(const DynamicMoverCoord &coord)
{
    SceneDetail::MptCoordList &mpt_coord_list = this->dynamic_mpt(0);
    int grid_index = this->calc_dynamic_mpt_index_by_coord(coord);
    if (0 <= grid_index && grid_index < int(mpt_coord_list.size()))
        return (mpt_coord_list[grid_index] == 0);
    return false;
}

int Scene::increase_mpt_coord(GameMover *mover, const MoverCoord &coord)
{
     int grid_index = this->calc_dynamic_mpt_index_by_coord(DynamicMoverCoord(coord.pixel_x(), coord.pixel_y()));
     SceneDetail::MptCoordList &mpt_coord_list = this->dynamic_mpt(mover);
     if (0 <= grid_index && grid_index < int(mpt_coord_list.size()))
     {
         if (mpt_coord_list[grid_index] > 0)
         {
             MSG_USER("WARNING monster mpt point[%d] %ld %d[%d,%d] %d", mpt_coord_list[grid_index], mover->mover_id(),
                     mover->scene_id(), coord.pixel_x(), coord.pixel_y(), grid_index);
         }

         ++mpt_coord_list[grid_index];
     }
     return 0;
}

int Scene::reduce_mpt_coord(GameMover *mover, const MoverCoord &coord)
{
     int grid_index = this->calc_dynamic_mpt_index_by_coord(DynamicMoverCoord(coord.pixel_x(), coord.pixel_y()));
     SceneDetail::MptCoordList &mpt_coord_list = this->dynamic_mpt(mover);
     if (0 <= grid_index && grid_index < int(mpt_coord_list.size()))
     {
         if (mpt_coord_list[grid_index] == 0)
         {
             MSG_USER("WARNING monster mpt point[0] %ld %d(%d,%d) %d", mover->mover_id(),
                     mover->scene_id(), coord.pos_x(), coord.pos_y(), grid_index);
         }
         --mpt_coord_list[grid_index];
     }
     return 0;
}

int Scene::send_around_block(GameMover *mover, const BlockIndexType block_x, const BlockIndexType block_y, Block_Buffer *buff)
{
    int pos[9][2];
    pos[0][0] = block_x;            pos[0][1] = block_y;        /// 中心
    pos[1][0] = block_x + 1;        pos[1][1] = block_y;        /// 右
    pos[2][0] = block_x + 1;        pos[2][1] = block_y + 1;    /// 右下
    pos[3][0] = block_x;            pos[3][1] = block_y + 1;    /// 下
    pos[4][0] = block_x - 1,        pos[4][1] = block_y + 1;    /// 左下
    pos[5][0] = block_x - 1,        pos[5][1] = block_y;        /// 左
    pos[6][0] = block_x - 1,        pos[6][1] = block_y - 1;    /// 左上
    pos[7][0] = block_x,            pos[7][1] = block_y - 1;    /// 上
    pos[8][0] = block_x + 1,        pos[8][1] = block_y - 1;    /// 右上

    int block_x_amount = this->scene_detail_.__block_x_amount,
        block_y_amount = this->scene_detail_.__block_y_amount;
    Int64 block_index = 0;
    for (int i = 0; i < 9; ++i)
    {
        if (pos[i][0] < 0 || pos[i][0] >= block_x_amount || pos[i][1] < 0 || pos[i][1] >= block_y_amount)
            continue;

        block_index = pos[i][1] * block_x_amount + pos[i][0];
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
        scene_block.push_data(*buff);

		if (scene_block.__player_map.empty() == false)
			this->scene_detail_.__has_block_set.insert(block_index);
	}
	return 0;
}

int Scene::register_mover(GameMover *mover, const int64_t c_block_index)
{
    MoverCoord &location = mover->location();
    BlockIndexType block_index = c_block_index;
    if (c_block_index < 0)
    {
		if (this->cal_block_index_by_grid_coord(location.pos_x(), location.pos_y(), block_index) != 0)
		{
			MSG_USER("ERROR grid coord to block index %ld %d %d [%d,%d]",
					mover->mover_id(), mover->scene_id(), mover->space_id(),
					location.pos_x(), location.pos_y());
			return -1;
		}
    }

    SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
    if (scene_block.__mover_map.bind(mover->mover_id(), mover) != 0)
    {
        MSG_USER("ERROR multi register block mover %ld %d %d %d %d",
                mover->mover_id(), mover->scene_id(), block_index, mover->space_id(), this->space_id());
        return -1;
    }
    ++(this->scene_detail_.__mover_amount_list[block_index]);

    mover->set_cur_block_index(block_index);
    if (mover->is_player() == true)
    {
        this->scene_detail_.__player_block_set.insert(block_index);

        if (scene_block.__player_map.bind(mover->mover_id(), mover) != 0)
        {
            MSG_USER("ERROR multi register block player %ld %d %d %d %d",
                    mover->mover_id(), mover->scene_id(), block_index, mover->space_id(), this->space_id());
        }
        else
        {
            ++(this->scene_detail_.__player_amount_list[block_index]);
        }
        scene_block.__mover_offset_map[mover->mover_id()] = scene_block.__data_buff.get_write_idx();

        this->start_timer();
    }
    else
    {
    	scene_block.__other_mover_map.bind(mover->mover_id(), mover);
    }

    return 0;
}

int Scene::unregister_mover(GameMover *mover, const int64_t c_block_index, const bool is_exit_scene)
{
    MoverCoord &location = mover->location();
    BlockIndexType block_index = c_block_index;
    if (c_block_index < 0 || int(this->scene_detail_.__scene_block_list.size()) <= c_block_index)
    {
		if (this->cal_block_index_by_grid_coord(location.pos_x(), location.pos_y(), block_index) != 0)
		{
			MSG_USER("ERROR grid coord to index %ld %d[%d,%d]", mover->mover_id(),
					this->scene_id(), location.pos_x(), location.pos_y());
			return -1;
		}
    }

    SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
    if (scene_block.__mover_map.unbind(mover->mover_id()) == 0)
        --(this->scene_detail_.__mover_amount_list[block_index]);
    if (mover->is_player() == true)
    {
        if (is_exit_scene == true)
            scene_block.flush_to_mover(mover);

        if (scene_block.__player_map.unbind(mover->mover_id()) == 0)
            --(this->scene_detail_.__player_amount_list[block_index]);
        scene_block.__mover_offset_map.unbind(mover->mover_id());

        if (scene_block.__player_map.size() <= 0)
            this->scene_detail_.__player_block_set.erase(block_index);
    }
    else
    {
    	scene_block.__other_mover_map.unbind(mover->mover_id());
    }

    return 0;
}

int Scene::near_block_x_from_pos_x(const int pos_x)
{
	return pos_x / this->scene_detail_.__near_block_width;
}

int Scene::near_block_y_from_pos_y(const int pos_y)
{
	return pos_y / this->scene_detail_.__near_block_height;
}

Int64 Scene::near_block_index_by_block_coord(const int block_x, const int block_y)
{
	return Int64(block_y) * this->scene_detail_.__near_block_x_amount + block_x;
}

int Scene::register_near_mover(GameMover *mover)
{
	JUDGE_RETURN(mover->is_player() == true, 0);

	int block_x = this->near_block_x_from_pos_x(mover->location().pos_x()),
		block_y = this->near_block_y_from_pos_y(mover->location().pos_y());
	Int64 near_block_index = this->near_block_index_by_block_coord(block_x, block_y);

	JUDGE_RETURN(0 <= near_block_index && near_block_index < int(this->scene_detail_.__near_block_list.size()), 0);

	SceneDetail::MoverSet &mover_set = this->scene_detail_.__near_block_list[near_block_index];
	mover_set.insert(mover);
	this->scene_detail_.__near_player_block_set.insert(near_block_index);
	return 0;
}

int Scene::unregister_near_mover(GameMover *mover)
{
	JUDGE_RETURN(mover->is_player() == true, 0);

	int block_x = this->near_block_x_from_pos_x(mover->location().pos_x()),
		block_y = this->near_block_y_from_pos_y(mover->location().pos_y());
	Int64 near_block_index = this->near_block_index_by_block_coord(block_x, block_y);

	JUDGE_RETURN(0 <= near_block_index && near_block_index < int(this->scene_detail_.__near_block_list.size()), 0);

	SceneDetail::MoverSet &mover_set = this->scene_detail_.__near_block_list[near_block_index];
	mover_set.erase(mover);

	if (mover_set.size() <= 0)
		this->scene_detail_.__near_player_block_set.erase(near_block_index);
	return 0;
}

int Scene::process_broad_near_player(const Time_Value &nowtime)
{
	JUDGE_RETURN(this->scene_detail_.__near_notify_tick <= nowtime, 0);

	this->scene_detail_.__near_notify_tick = Time_Value::gettimeofday() + Time_Value(5);

    InnerRouteHead route_head(0, 0, BT_BROAD_IN_GATE, this->scene_id(), 1);
    ProtoHead head;
    ProtoClientHead client_head;
    client_head.__recogn = ACTIVE_NEAR_PLAYER;

    int prev_len = sizeof(int32_t) * 2 + sizeof(InnerRouteHead) + sizeof(int32_t) + sizeof(ProtoHead),
        data_buff_len = 0;

    Block_Buffer cache_buff;

	Proto80400233 respond;
	for (BLongSet::iterator iter = this->scene_detail_.__near_player_block_set.begin();
			iter != this->scene_detail_.__near_player_block_set.end(); ++iter)
	{
		Int64 near_block_index = *iter;
		JUDGE_CONTINUE(0 <= near_block_index && near_block_index < int(this->scene_detail_.__near_block_list.size()));

		respond.Clear();

		int block_x = near_block_index % this->scene_detail_.__near_block_x_amount,
			block_y = near_block_index / this->scene_detail_.__near_block_x_amount;

        boost::unordered_map<int, Block_Buffer*> gate_block_map;

		for (int x = block_x - 1; x <= block_x + 1 && x <= this->scene_detail_.__near_block_x_amount; ++x)
		{
			JUDGE_CONTINUE(x >= 0);
			for (int y = block_y - 1; y <= block_y + 1 && y <= this->scene_detail_.__near_block_y_amount; ++y)
			{
				JUDGE_CONTINUE(y >= 0);

				Int64 tmp_block_index = this->near_block_index_by_block_coord(x, y);
				JUDGE_CONTINUE(0 <= tmp_block_index && tmp_block_index < int(this->scene_detail_.__near_block_list.size()));

				SceneDetail::MoverSet &mover_set = this->scene_detail_.__near_block_list[tmp_block_index];
				for (SceneDetail::MoverSet::iterator mover_iter = mover_set.begin(); mover_iter != mover_set.end(); ++mover_iter)
				{
					GameMover *mover = *mover_iter;
					MapPlayerEx *player = dynamic_cast<MapPlayerEx *>(mover);
					JUDGE_CONTINUE(player != NULL && player->is_enter_scene() == true);

					ProtoNearRole *proto_near_role = respond.add_role_list();
					proto_near_role->set_role_id(player->role_id());
					proto_near_role->set_role_name(player->role_name());
					proto_near_role->set_level(player->level());
					player->location().serialize(proto_near_role->mutable_location());
					proto_near_role->set_name_color(player->fetch_name_color());
				}
			}
		}

		SceneDetail::MoverSet &broad_mover_set = this->scene_detail_.__near_block_list[near_block_index];
		for (SceneDetail::MoverSet::iterator mover_iter = broad_mover_set.begin(); mover_iter != broad_mover_set.end(); ++mover_iter)
		{
			GameMover *mover = *mover_iter;
			JUDGE_CONTINUE(mover != NULL && mover->is_enter_scene() == true);

            boost::unordered_map<int, Block_Buffer*>::iterator gate_iter = gate_block_map.find(mover->gate_sid());
            if (gate_iter == gate_block_map.end())
            {
            	Block_Buffer *buff = this->pop_block();
            	gate_block_map[mover->gate_sid()] = buff;

            	buff->ensure_writable_bytes(prev_len + sizeof(int) * 4 + data_buff_len + sizeof(int64_t) * 50);
            	buff->write_int32(mover->gate_sid());
            	buff->write_int32(0);
            	buff->copy(&route_head, sizeof(InnerRouteHead));
            	buff->write_int32(0);
            	buff->copy(&head, sizeof(ProtoHead));

                cache_buff.reset();
                mover->make_up_client_block(&cache_buff, &client_head, &respond);
                data_buff_len = cache_buff.readable_bytes();

                buff->write_int32(data_buff_len);
                buff->copy(&cache_buff);
            	buff->write_int32(1);
            	buff->write_int64(mover->mover_id());
				buff->write_int32(0);   // buff offset
            }
            else
            {
            	Block_Buffer *buff = gate_iter->second;
            	int *data_len = (int *)(buff->get_read_ptr() + prev_len);
            	int *mover_size = (int *)(buff->get_read_ptr() + prev_len + sizeof(int32_t) + *data_len);
            	++(*mover_size);
            	buff->write_int64(mover->mover_id());
            	buff->write_int32(0);   // buff offset
            }
		}

        for (boost::unordered_map<int, Block_Buffer*>::iterator block_iter = gate_block_map.begin();
            	block_iter != gate_block_map.end(); ++block_iter)
        {
        	Block_Buffer *buff = block_iter->second;
        	int *total_len = (int *)(buff->get_read_ptr() + sizeof(int32_t));
        	int *head_body_len = (int *)(buff->get_read_ptr() + sizeof(int32_t) * 2 + sizeof(InnerRouteHead));
        	*total_len = buff->readable_bytes() - sizeof(int32_t) * 2;
        	*head_body_len = buff->readable_bytes() - sizeof(int32_t) * 3 - sizeof(InnerRouteHead);
        	if (this->monitor()->inner_sender(block_iter->first)->push_pool_block_with_len(block_iter->second) != 0)
        		this->push_block(block_iter->second);
        }
	}
    return 0;
}

int Scene::send_diff_block(GameMover *mover, const MoverCoord &target,
        const BlockIndexType org_block_x, const BlockIndexType org_block_y, 
        const BlockIndexType target_block_x, const BlockIndexType target_block_y,
        Block_Buffer *disappear_buff, Block_Buffer *mover_buff, Block_Buffer *appear_buff)
{
    int org_pos[9][2];
    org_pos[0][0] = org_block_x;            org_pos[0][1] = org_block_y;        /// 中心
    org_pos[1][0] = org_block_x + 1;        org_pos[1][1] = org_block_y;        /// 右
    org_pos[2][0] = org_block_x + 1;        org_pos[2][1] = org_block_y + 1;    /// 右下
    org_pos[3][0] = org_block_x;            org_pos[3][1] = org_block_y + 1;    /// 下
    org_pos[4][0] = org_block_x - 1,        org_pos[4][1] = org_block_y + 1;    /// 左下
    org_pos[5][0] = org_block_x - 1,        org_pos[5][1] = org_block_y;        /// 左
    org_pos[6][0] = org_block_x - 1,        org_pos[6][1] = org_block_y - 1;    /// 左上
    org_pos[7][0] = org_block_x,            org_pos[7][1] = org_block_y - 1;    /// 上
    org_pos[8][0] = org_block_x + 1,        org_pos[8][1] = org_block_y - 1;    /// 右上

    int target_pos[9][2];
    target_pos[0][0] = target_block_x;            target_pos[0][1] = target_block_y;        /// 中心
    target_pos[1][0] = target_block_x + 1;        target_pos[1][1] = target_block_y;        /// 右
    target_pos[2][0] = target_block_x + 1;        target_pos[2][1] = target_block_y + 1;    /// 右下
    target_pos[3][0] = target_block_x;            target_pos[3][1] = target_block_y + 1;    /// 下
    target_pos[4][0] = target_block_x - 1,        target_pos[4][1] = target_block_y + 1;    /// 左下
    target_pos[5][0] = target_block_x - 1,        target_pos[5][1] = target_block_y;        /// 左
    target_pos[6][0] = target_block_x - 1,        target_pos[6][1] = target_block_y - 1;    /// 左上
    target_pos[7][0] = target_block_x,            target_pos[7][1] = target_block_y - 1;    /// 上
    target_pos[8][0] = target_block_x + 1,        target_pos[8][1] = target_block_y - 1;    /// 右上

	BlockIndexType x_amount = this->scene_detail_.__block_x_amount,
				   y_amount = this->scene_detail_.__block_y_amount;
//    BlockIndexType org_block_index = org_block_y * x_amount + org_block_x;

	int block_x = org_block_x, block_y = org_block_y;
	block_x *= this->scene_detail_.__block_width;
	block_y *= this->scene_detail_.__block_height;
//    const int MAX_STEP_AMOUNT = CONFIG_INSTANCE->max_move_step();

    Block_Buffer *other_buff = this->pop_block();
#ifndef NO_BROAD_PORT
    if (mover->is_player() == true)
        other_buff->write_int32(mover->client_sid());
#else
    int total_len_idx = -1, head_len_idx = -1, org_read_idx = -1;
    if (mover->is_player() == true)
    {
        InnerRouteHead route_head;
        route_head.__broad_type = BT_BROAD_CLIENT;
        route_head.__role_id = mover->mover_id();
        route_head.__inner_req = 1;
        route_head.__scene_id = mover->scene_id();
    
        ProtoHead head;
        head.__role_id = mover->mover_id();
        head.__scene_id = mover->scene_id();
    
        other_buff->ensure_writable_bytes(sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead) + sizeof(int) * 4);
        other_buff->write_int32(mover->gate_sid());
        total_len_idx = other_buff->get_write_idx();
        other_buff->write_int32(0);
        other_buff->copy((char *)(&route_head), sizeof(InnerRouteHead));
        head_len_idx = other_buff->get_write_idx();
        other_buff->write_int32(0);
        other_buff->copy((char *)(&head), sizeof(ProtoHead));
        org_read_idx = other_buff->get_read_idx();
        other_buff->set_read_idx(other_buff->get_write_idx());
    }
#endif

    Int64 block_index = 0;
    for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(org_pos[i][0] - target_block_x)) <= 1 && std::abs(long(org_pos[i][1] - target_block_y)) <= 1)
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;

            block_index = org_pos[i][1] * x_amount + org_pos[i][0];
            SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
            scene_block.push_data(*mover_buff);

            if (scene_block.__player_map.empty() == false)
            	this->scene_detail_.__has_block_set.insert(block_index);
        }
        else
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;

            block_index = org_pos[i][1] * x_amount + org_pos[i][0];
            SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
            scene_block.push_data(*disappear_buff);

            if (scene_block.__player_map.empty() == false)
            	this->scene_detail_.__has_block_set.insert(block_index);

            if (mover->is_player() == true)
                scene_block.make_up_all_disappear_info(*other_buff);
        }
    }

    for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(target_pos[i][0] - org_block_x)) <= 1 && std::abs(long(target_pos[i][1] - org_block_y)) <= 1)
            continue;

        if (target_pos[i][0] < 0 || target_pos[i][0] >= x_amount || target_pos[i][1] < 0 || target_pos[i][1] >= y_amount)
            continue;

        block_index = target_pos[i][1] * x_amount + target_pos[i][0];
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
        scene_block.push_data(*appear_buff);

        if (scene_block.__player_map.empty() == false)
        	this->scene_detail_.__has_block_set.insert(block_index);

        if (mover->is_player() == true)
            scene_block.make_up_all_appear_info(*other_buff);
    }

    if (mover->is_player() == true && other_buff->readable_bytes() > sizeof(int32_t))
    {
#ifdef NO_BROAD_PORT
        int data_len = other_buff->readable_bytes();
        other_buff->set_read_idx(total_len_idx);
        int32_t *total_len = (int32_t *)(other_buff->get_read_ptr());
        other_buff->set_read_idx(head_len_idx);
        int32_t *head_len  = (int32_t *)(other_buff->get_read_ptr());
        other_buff->set_read_idx(org_read_idx);
        *head_len = sizeof(ProtoHead) + data_len;
        *total_len = sizeof(InnerRouteHead) + sizeof(int32_t) + *head_len;
        if (MAP_MONITOR->inner_sender(mover->gate_sid())->push_pool_block_with_len(other_buff) != 0)
            this->push_block(other_buff);
#else
        this->client_sender(mover->client_sid())->push_pool_block_with_len(other_buff);
#endif
    }
    else
    {
    	this->push_block(other_buff);
    }
    return 0;
}

int Scene::send_diff_block_for_relive(GameMover *mover, const MoverCoord &target,
            const BlockIndexType org_block_x, const BlockIndexType org_block_y,
            const BlockIndexType target_block_x, const BlockIndexType target_block_y,
            Block_Buffer *disappear_buff, Block_Buffer *appear_buff)
{
    int org_pos[9][2];
    org_pos[0][0] = org_block_x;            org_pos[0][1] = org_block_y;        /// 中心
    org_pos[1][0] = org_block_x + 1;        org_pos[1][1] = org_block_y;        /// 右
    org_pos[2][0] = org_block_x + 1;        org_pos[2][1] = org_block_y + 1;    /// 右下
    org_pos[3][0] = org_block_x;            org_pos[3][1] = org_block_y + 1;    /// 下
    org_pos[4][0] = org_block_x - 1,        org_pos[4][1] = org_block_y + 1;    /// 左下
    org_pos[5][0] = org_block_x - 1,        org_pos[5][1] = org_block_y;        /// 左
    org_pos[6][0] = org_block_x - 1,        org_pos[6][1] = org_block_y - 1;    /// 左上
    org_pos[7][0] = org_block_x,            org_pos[7][1] = org_block_y - 1;    /// 上
    org_pos[8][0] = org_block_x + 1,        org_pos[8][1] = org_block_y - 1;    /// 右上

    int target_pos[9][2];
    target_pos[0][0] = target_block_x;            target_pos[0][1] = target_block_y;        /// 中心
    target_pos[1][0] = target_block_x + 1;        target_pos[1][1] = target_block_y;        /// 右
    target_pos[2][0] = target_block_x + 1;        target_pos[2][1] = target_block_y + 1;    /// 右下
    target_pos[3][0] = target_block_x;            target_pos[3][1] = target_block_y + 1;    /// 下
    target_pos[4][0] = target_block_x - 1,        target_pos[4][1] = target_block_y + 1;    /// 左下
    target_pos[5][0] = target_block_x - 1,        target_pos[5][1] = target_block_y;        /// 左
    target_pos[6][0] = target_block_x - 1,        target_pos[6][1] = target_block_y - 1;    /// 左上
    target_pos[7][0] = target_block_x,            target_pos[7][1] = target_block_y - 1;    /// 上
    target_pos[8][0] = target_block_x + 1,        target_pos[8][1] = target_block_y - 1;    /// 右上

	BlockIndexType x_amount = this->scene_detail_.__block_x_amount,
				   y_amount = this->scene_detail_.__block_y_amount;

	int block_x = org_block_x, block_y = org_block_y;
	block_x *= this->scene_detail_.__block_width;
	block_y *= this->scene_detail_.__block_height;

    Block_Buffer *other_buff = this->pop_block();
#ifndef NO_BROAD_PORT
    if (mover->is_player() == true)
        other_buff->write_int32(mover->client_sid());
#else
    int total_len_idx = -1, head_len_idx = -1, org_read_idx = -1;
    if (mover->is_player() == true)
    {
        InnerRouteHead route_head;
        route_head.__broad_type = BT_BROAD_CLIENT;
        route_head.__role_id = mover->mover_id();
        route_head.__inner_req = 1;
        route_head.__scene_id = mover->scene_id();
    
        ProtoHead head;
        head.__role_id = mover->mover_id();
        head.__scene_id = mover->scene_id();
    
        other_buff->ensure_writable_bytes(sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead) + sizeof(int) * 4);
        other_buff->write_int32(mover->gate_sid());
        total_len_idx = other_buff->get_write_idx();
        other_buff->write_int32(0);
        other_buff->copy((char *)(&route_head), sizeof(InnerRouteHead));
        head_len_idx = other_buff->get_write_idx();
        other_buff->write_int32(0);
        other_buff->copy((char *)(&head), sizeof(ProtoHead));
        org_read_idx = other_buff->get_read_idx();
        other_buff->set_read_idx(other_buff->get_write_idx());
    }
#endif

    Int64 block_index = 0;
    for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(org_pos[i][0] - target_block_x)) <= 1 && std::abs(long(org_pos[i][1] - target_block_y)) <= 1)
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;

            block_index = org_pos[i][1] * x_amount + org_pos[i][0];
            SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
            scene_block.push_data(*disappear_buff);
            scene_block.push_data(*appear_buff);

            if (scene_block.__player_map.empty() == false)
            	this->scene_detail_.__has_block_set.insert(block_index);
        }
        else
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;

            block_index = org_pos[i][1] * x_amount + org_pos[i][0];
            SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
            scene_block.push_data(*disappear_buff);

            if (scene_block.__player_map.empty() == false)
            	this->scene_detail_.__has_block_set.insert(block_index);

            if (mover->is_player() == true)
                scene_block.make_up_all_disappear_info(*other_buff);
        }
    }

    for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(target_pos[i][0] - org_block_x)) <= 1 && std::abs(long(target_pos[i][1] - org_block_y)) <= 1)
            continue;

        if (target_pos[i][0] < 0 || target_pos[i][0] >= x_amount || target_pos[i][1] < 0 || target_pos[i][1] >= y_amount)
            continue;

        block_index = target_pos[i][1] * x_amount + target_pos[i][0];
        SceneBlock &scene_block = this->scene_detail_.__scene_block_list[block_index];
        scene_block.push_data(*appear_buff);

        if (scene_block.__player_map.empty() == false)
        	this->scene_detail_.__has_block_set.insert(block_index);

        if (mover->is_player() == true)
            scene_block.make_up_all_appear_info(*other_buff);
    }

    if (mover->is_player() == true && other_buff->readable_bytes() > sizeof(int32_t))
    {
#ifdef NO_BROAD_PORT
        int data_len = other_buff->readable_bytes();
        other_buff->set_read_idx(total_len_idx);
        int32_t *total_len = (int32_t *)(other_buff->get_read_ptr());
        other_buff->set_read_idx(head_len_idx);
        int32_t *head_len  = (int32_t *)(other_buff->get_read_ptr());
        other_buff->set_read_idx(org_read_idx);
        *head_len = sizeof(ProtoHead) + data_len;
        *total_len = sizeof(InnerRouteHead) + sizeof(int32_t) + *head_len;
        if (MAP_MONITOR->inner_sender(mover->gate_sid())->push_pool_block_with_len(other_buff) != 0)
            this->push_block(other_buff);
#else
        this->client_sender(mover->client_sid())->push_pool_block_with_len(other_buff);
#endif
    }
    else
    {
    	this->push_block(other_buff);
    }
    return 0;
}

int Scene::is_in_safe_area(GameFighter* fighter)
{
	JUDGE_RETURN(this->has_safe_area() == true, false);

	const Json::Value& conf = this->conf();
	int total_size = conf["safe_area"].size();
	for (int i = 0; i < total_size; ++i)
	{
		const Json::Value& safe_conf = conf["safe_area"][i];

		MoverCoord center;
		center.set_pixel(safe_conf["center_x"].asInt(), safe_conf["center_y"].asInt());
        JUDGE_CONTINUE(::check_coord_pixel_distance(center, fighter->location(),
        		safe_conf["r"].asInt()) == true);

        return true;
	}

	return false;
}

int Scene::validate_in_safe_area(GameFighter *defender)
{
    EffectAI *effect_ai = NULL;
    for (BLongSet::iterator iter = this->scene_detail_.safe_area_set_.begin();
            iter != this->scene_detail_.safe_area_set_.end(); ++iter)
    {
        effect_ai = dynamic_cast<EffectAI *>(this->find_ai(*iter));
        if (effect_ai == NULL || effect_ai->is_enter_scene() == false)
            continue;

        if (check_coord_pixel_distance(effect_ai->location(), defender->location(),
        		effect_ai->ai_detail().__safe_radius_pixel, GameEnum::CLIENT_MAPPING_FACTOR) == true)
            return 0;
    }
    return -1;
}

bool Scene::is_ai_sort_alive(const int ai_sort)
{
	BLongSet *ai_set = this->ai_sort_set(ai_sort);
    if (ai_set == NULL || ai_set->size() <= 0)
        return false;
    return true;
}


FloatAI *Scene::find_float_ai(const Int64 role_id)
{
    GameMover *ai = NULL;
    if (this->float_ai_map_.find(role_id, ai) == 0)
        return dynamic_cast<FloatAI *>(ai);
    return NULL;
}

int Scene::rebind_float_ai(const Int64 role_id, FloatAI *float_ai)
{
    return this->float_ai_map_.rebind(role_id, float_ai);
}

int Scene::unbind_float_ai(const Int64 role_id)
{
    return this->float_ai_map_.unbind(role_id);
}

int Scene::bind_team_player(const int team_id, MapPlayer *player)
{
    JUDGE_RETURN(team_id > 0, 0);

    SceneBlock *scene_block = NULL;
    SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.find(team_id);
    if (iter == this->scene_detail_.team_scene_block_map_.end())
    {
        scene_block = this->monitor()->team_scene_block_pool()->pop();
        this->scene_detail_.team_scene_block_map_[team_id] = scene_block;
    }
    else
    {
        scene_block = iter->second;
    }

	// 处理队员进入场景通知
	Block_Buffer buff;
	player->make_up_teamer_appear(&buff);
	scene_block->push_data(buff);

    scene_block->__mover_map.rebind(player->role_id(), player);
    scene_block->__player_map.rebind(player->role_id(), player);
    scene_block->__mover_offset_map[player->role_id()] = scene_block->__data_buff.get_write_idx();

    return 0;
}

int Scene::unbind_team_player(const int team_id, MapPlayer *player)
{
    JUDGE_RETURN(team_id > 0, 0);

    SceneBlock *scene_block = NULL;
    SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.find(team_id);
    JUDGE_RETURN(iter != this->scene_detail_.team_scene_block_map_.end(), 0);
    
    scene_block = iter->second;
    JUDGE_RETURN(scene_block != NULL, 0);

    scene_block->__player_map.unbind(player->mover_id());
    scene_block->__mover_map.unbind(player->mover_id());

    if (scene_block->__player_map.size() <= 0)
    {
        this->scene_detail_.team_scene_block_map_.erase(iter);
        this->monitor()->team_scene_block_pool()->push(scene_block);
    }
    else
    {
        player->notify_teamer_disappear(this);
    }

    return 0;
}

int Scene::process_broad_teamer(void)
{
    SceneDetail::TeamSceneBlockMap &team_block_map = this->scene_detail_.team_scene_block_map_;
    JUDGE_RETURN(team_block_map.size() > 0, 0);

    size_t mover_offset = 0;
    
    InnerRouteHead route_head(0, 0, BT_BROAD_IN_GATE, this->scene_id(), 1);
    ProtoHead head;

    for (SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.begin();
            iter != this->scene_detail_.team_scene_block_map_.end(); ++iter)
    {
        JUDGE_CONTINUE(iter->second != NULL);
        SceneBlock &scene_block = *(iter->second);

        if (scene_block.__data_buff.readable_bytes() <= 0 || scene_block.__player_map.empty() == true)
        	continue;

        int prev_len = sizeof(int32_t) * 2 + sizeof(InnerRouteHead) + sizeof(int32_t) + sizeof(ProtoHead),
        	    data_buff_len = scene_block.__data_buff.readable_bytes();

        boost::unordered_map<int, Block_Buffer*> gate_block_map;
		for (MoverMap::iterator player_iter = scene_block.__player_map.begin();
				player_iter != scene_block.__player_map.end(); ++player_iter)
		{
			GameMover *mover = player_iter->second;
            if (mover->is_enter_scene() == false)
            	continue;
            if (mover->gate_sid() <= 0)
            	continue;

            boost::unordered_map<int, Block_Buffer*>::iterator gate_iter = gate_block_map.find(mover->gate_sid());
            if (gate_iter == gate_block_map.end())
            {
            	Block_Buffer *buff = this->pop_block();
            	gate_block_map[mover->gate_sid()] = buff;

            	buff->ensure_writable_bytes(prev_len + sizeof(int) * 4 + data_buff_len + sizeof(int64_t) * 50);
            	buff->write_int32(mover->gate_sid());
            	buff->write_int32(0);
            	buff->copy(&route_head, sizeof(InnerRouteHead));
            	buff->write_int32(0);
            	buff->copy(&head, sizeof(ProtoHead));
            	buff->write_int32(data_buff_len);
            	buff->copy(&(scene_block.__data_buff));
            	buff->write_int32(1);
            	buff->write_int64(mover->mover_id());
				if (scene_block.__mover_offset_map.find(mover->mover_id(), mover_offset) != 0)
				{
					mover_offset = 0;
				}
				else
				{
					mover_offset -= scene_block.__data_buff.get_read_idx();
					mover_offset = (mover_offset < 0 ? 0 : mover_offset);
				}
				buff->write_int32(mover_offset);
            }
            else
            {
            	Block_Buffer *buff = gate_iter->second;
            	int *data_len = (int *)(buff->get_read_ptr() + prev_len);
            	int *mover_size = (int *)(buff->get_read_ptr() + prev_len + sizeof(int32_t) + *data_len);
            	++(*mover_size);
            	buff->write_int64(mover->mover_id());
				if (scene_block.__mover_offset_map.find(mover->mover_id(), mover_offset) != 0)
				{
					mover_offset = 0;
				}
				else
				{
					mover_offset -= scene_block.__data_buff.get_read_idx();
					mover_offset = (mover_offset < 0 ? 0 : mover_offset);
				}
            	buff->write_int32(mover_offset);
            }
		}
        for (boost::unordered_map<int, Block_Buffer*>::iterator block_iter = gate_block_map.begin();
        		block_iter != gate_block_map.end(); ++block_iter)
        {
        	Block_Buffer *buff = block_iter->second;
        	int *total_len = (int *)(buff->get_read_ptr() + sizeof(int32_t));
        	int *head_body_len = (int *)(buff->get_read_ptr() + sizeof(int32_t) * 2 + sizeof(InnerRouteHead));
        	*total_len = buff->readable_bytes() - sizeof(int32_t) * 2;
        	*head_body_len = buff->readable_bytes() - sizeof(int32_t) * 3 - sizeof(InnerRouteHead);
        	if (this->monitor()->inner_sender(block_iter->first)->push_pool_block_with_len(block_iter->second) != 0)
        		this->push_block(block_iter->second);
        }
        
        scene_block.__data_buff.reset();
        scene_block.__mover_offset_map.clear();
    }
    return 0;
}

int Scene::push_team_player_data(const int team_id, Block_Buffer &buff)
{
    JUDGE_RETURN(team_id > 0, 0);

    SceneBlock *scene_block = NULL;
    SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.find(team_id);
    JUDGE_RETURN(iter != this->scene_detail_.team_scene_block_map_.end(), 0);
    
    scene_block = iter->second;
    JUDGE_RETURN(scene_block != NULL, 0);

    scene_block->push_data(buff);
    return 0;
}

int Scene::open_mini_map_pannel(const int team_id, const Int64 role_id)
{
    JUDGE_RETURN(team_id > 0, 0);

    SceneBlock *scene_block = NULL;
    SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.find(team_id);
    JUDGE_RETURN(iter != this->scene_detail_.team_scene_block_map_.end(), 0);
    
    scene_block = iter->second;
    JUDGE_RETURN(scene_block != NULL, 0);

    scene_block->__mover_offset_map[role_id] = scene_block->__data_buff.get_write_idx();
    return 0;
}

int Scene::close_mini_map_pannel(const int team_id, const Int64 role_id)
{
    return 0;
}

int Scene::make_up_all_teamer_appear_info(const int team_id, Block_Buffer &buff)
{
    JUDGE_RETURN(team_id > 0, 0);

    SceneBlock *scene_block = NULL;
    SceneDetail::TeamSceneBlockMap::iterator iter = this->scene_detail_.team_scene_block_map_.find(team_id);
    JUDGE_RETURN(iter != this->scene_detail_.team_scene_block_map_.end(), 0);
    
    scene_block = iter->second;
    JUDGE_RETURN(scene_block != NULL, 0);
    
    for (MoverMap::iterator player_iter = scene_block->__player_map.begin();
            player_iter != scene_block->__player_map.end(); ++player_iter)
    {
        MapPlayer *player = dynamic_cast<MapPlayer *>(player_iter->second);
        JUDGE_CONTINUE(player != NULL && player->is_enter_scene() == true);

        player->make_up_teamer_appear(&buff);
    }
    return 0;
}

