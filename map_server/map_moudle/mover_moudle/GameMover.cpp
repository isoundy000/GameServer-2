/*
 * GameMover.cpp
 *
 * Created on: 2013-02-02 11:48
 *     Author: glendy
 */

#include "GameMover.h"
#include "Scene.h"
#include "MapMonitor.h"
#include "GameConfig.h"
#include "ProtoDefine.h"
#include "GameFighter.h"
#include "GameAI.h"

GameMover::GameMover(void)
{
    this->is_active_ = false;
    this->is_enter_scene_ = false;
    
    this->scene_ = 0;
    this->mover_type_ = 0;
    this->is_send_disappear_ = 0;

    this->monitor_ = MAP_MONITOR;
    this->room_scene_index_ = 0;
}

GameMover::~GameMover(void)
{ /*NULL*/ }

void GameMover::reset_mover(void)
{
	EntityCommunicate::reset_entity();

	this->scene_ = 0;
    this->mover_type_ = 0;

    this->is_active_ = false;
    this->is_enter_scene_ = false;
    this->is_send_disappear_ = false;

	this->mover_detial_.reset();
	this->room_scene_index_ = -1;
}

void GameMover::set_cur_location(const MoverCoord& coord)
{
	this->set_cur_toward(coord);
	this->mover_detial_.__location = coord;
}

void GameMover::set_cur_toward(const MoverCoord& coord)
{
	this->mover_detial_.__toward = this->calculate_toward(coord);
}

MapMonitor *GameMover::monitor(void)
{
    return this->monitor_;
}

const Json::Value& GameMover::scene_set_conf()
{
	return CONFIG_INSTANCE->scene_set(this->scene_id());
}

const Json::Value& GameMover::scene_conf()
{
	return CONFIG_INSTANCE->scene(this->scene_id());
}

int GameMover::respond_from_broad_client(Block_Buffer *buff)
{
    return 0;
}

int GameMover::respond_from_broad_client(const int recogn, const int error, Block_Buffer *msg_body)
{
    return 0;
}

int GameMover::respond_from_broad_client(const int recogn, const int error, const Message *msg_proto)
{
    return 0;
}

int GameMover::respond_to_broad_area(const Message *msg, const int is_full_screen)
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

	Scene *scene = this->fetch_scene();
	if (scene != 0 && scene->is_inited_scene() == true)
	{
		if (is_full_screen == 1)
		{
			return scene->notify_fullscene(buff, this->room_scene_index());
		}
		else
		{
			return scene->notify_area_info(this, buff, this->room_scene_index());
		}
	}

	return this->respond_from_broad_client(&buff);
}

int GameMover::respond_to_broad_area(const int recogn, const int is_full_screen)
{
    ProtoClientHead head;
    head.__recogn = recogn;

    uint32_t len = sizeof(ProtoClientHead);

	Block_Buffer buff;
	buff.ensure_writable_bytes(len + sizeof(uint32_t) * 4);
	buff.write_uint32(len);
	buff.copy((char *) &head, sizeof(ProtoClientHead));

	Scene *scene = this->fetch_scene();
	if (scene != 0 && scene->is_inited_scene() == true)
	{
		if (is_full_screen == 1)
		{
			return scene->notify_fullscene(buff, this->room_scene_index());
		}
		else
		{
			return scene->notify_area_info(this, buff, this->room_scene_index());
		}
	}

	return this->respond_from_broad_client(&buff);
}

int64_t GameMover::mover_id(void)
{
    return this->entity_id();
}

int GameMover::mover_id_low(void)
{
    return (int)(this->mover_id());
}

int GameMover::mover_id_high(void)
{
    return (int)(this->mover_id() >> 32);
}

int GameMover::fetch_mover_volume()
{
	static int default_volume = CONFIG_INSTANCE->const_set("role_volume");
	return default_volume;
}

void GameMover::set_active(const bool flag)
{
    this->is_active_ = flag;
}

bool GameMover::is_active(void)
{
    return this->is_active_;
}

bool GameMover::is_enter_scene(void)
{
    return this->is_enter_scene_;
}

bool GameMover::is_player(void)
{
	return this->check_mover_type(MOVER_TYPE_PLAYER);
}

bool GameMover::is_beast(void)
{
	return this->check_mover_type(MOVER_TYPE_BEAST);
}

bool GameMover::is_monster(void)
{
	return this->check_mover_type(MOVER_TYPE_MONSTER);
}

void GameMover::set_scene_mode(const int scene_mode)
{
    this->mover_detial_.__scene_mode = scene_mode;
}

int GameMover::scene_id(void)
{
    return this->mover_detial_.__scene_id;
}

MoverCoord &GameMover::location(void)
{
    return this->mover_detial_.__location;
}

int GameMover::scene_mode(void)
{
    return this->mover_detial_.__scene_mode;
}

int GameMover::space_id(void)
{
    return this->mover_detial_.__space_id;
}

int GameMover::prev_scene_id(void)
{
    return this->mover_detial_.__prev_scene_id;
}

MoverCoord &GameMover::prev_location(void)
{
    return this->mover_detial_.__prev_location;
}

int GameMover::prev_scene_mode(void)
{
    return this->mover_detial_.__prev_scene_mode;
}

int GameMover::prev_space_id(void)
{
    return this->mover_detial_.__prev_space_id;
}

void GameMover::set_room_scene_index(const int index)
{
    this->room_scene_index_ = index;
}

int GameMover::room_scene_index(void)
{
    return this->room_scene_index_;
}

bool GameMover::is_in_normal_mode(void)
{
    return this->mover_detial_.__scene_mode == SCENE_MODE_NORMAL;
}

bool GameMover::is_in_league_mode(void)
{
    return this->mover_detial_.__scene_mode == SCENE_MODE_LEAGUE;
}

bool GameMover::is_in_script_mode(void)
{
    return this->mover_detial_.__scene_mode == SCENE_MODE_SCRIPT;
}

bool GameMover::is_in_world_boss_mode(void)
{
	return this->mover_detial_.__scene_mode == SCENE_MODE_WORLD_BOSS;
}

int GameMover::init_mover_scene(Scene* scene)
{
	JUDGE_RETURN(scene != NULL, -1);

	this->mover_detial_.__space_id = scene->space_id();
	this->mover_detial_.__scene_id = scene->scene_id();

	this->scene_ = scene;
	return 0;
}

MoverDetail &GameMover::mover_detail(void)
{
    return this->mover_detial_;
}

int GameMover::notify_mover_cur_location()
{
	Proto50400203 respond;
	respond.set_scene_id(this->scene_id());
	respond.set_space_id(this->space_id());
	respond.set_pixel_x(this->location().pixel_x());
	respond.set_pixel_y(this->location().pixel_y());

	GameFighter* fighter = dynamic_cast<GameFighter*>(this);
	if (fighter != NULL)
	{
		respond.set_cur_blood(fighter->cur_blood());
	}
	else
	{
		respond.set_cur_blood(1);
	}
	FINER_PROCESS_RETURN(RETURN_RELIVE, &respond);
}

bool GameMover::is_npc_nearby(int npc_id)
{
	JUDGE_RETURN(npc_id > 0, true);

    const Json::Value &npc_json = CONFIG_INSTANCE->npc(this->scene_id(), npc_id);
    JUDGE_RETURN(npc_json != Json::Value::null, false);

	if (npc_json.isMember("pos"))
	{
		MoverCoord npc_coord(npc_json["pos"][0u].asInt(), npc_json["pos"][1u].asInt());
		return this->check_cur_distance(npc_coord, 8);
	}
	else
	{
		MoverCoord npc_coord;
		npc_coord.set_pixel(npc_json["posX"].asInt(), npc_json["posY"].asInt());
		return this->check_cur_distance(npc_coord, 8);
	}
}

bool GameMover::is_in_league_region()
{
	return this->scene_id() == GameEnum::LEAGUE_REGION_FIGHT_ID;
}

int GameMover::check_cur_distance(const MoverCoord& aim_coord, int max_distance)
{
	return ::coord_offset_grid(this->location(), aim_coord) <= max_distance;
}

int GameMover::make_up_appear_info(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
	int ret = this->make_up_appear_info_base(buff, send_by_gate);
	this->make_up_buff_info(buff, send_by_gate);
	this->make_up_appear_other_info(buff, send_by_gate);
	return ret;
}

int GameMover::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
	MSG_USER("ERROR CAN'T RUN HERE %d %d %d", this->mover_id(), this->mover_id_high(), this->mover_id_low());
    // TODO;
    return -1;
}

int GameMover::make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
	return 0;
}

int GameMover::make_up_buff_info(Block_Buffer *buff, const bool send_by_gate)
{
    GameFighter* p_fighter = dynamic_cast<GameFighter*>(this);
    JUDGE_RETURN(p_fighter != NULL, -1);
    return p_fighter->make_up_status_msg(buff);
}

int GameMover::make_up_disappear_info(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
	this->make_up_disappear_other_info(buff, send_by_gate);
	this->make_up_disappear_info_base(buff, send_by_gate);
	return 0;
}

int GameMover::make_up_disappear_info_base(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
    Proto80400104 respond;
    respond.set_mover_id(this->mover_id());

    if (send_by_gate == false)
    {
        ProtoClientHead head;
        head.__recogn = ACTIVE_MOVE_DISAPPEAR;
        return this->make_up_client_block(buff, &head, &respond);
    }
    else
    {
        ProtoHead head;
        head.__recogn = ACTIVE_MOVE_DISAPPEAR;
        head.__role_id = this->mover_id();
        head.__scene_id = this->scene_id();
        return this->make_up_gate_block(buff, &head, &respond);
    }
}

int GameMover::make_up_disappear_other_info(Block_Buffer *buff, const bool send_by_gate/* = false*/)
{
	return 0;
}

int GameMover::make_up_move_info(Block_Buffer *buff, Int64 mover_id /*= 0*/)
{
    if (mover_id == 0)
    {
    	mover_id = this->mover_id();
    }

    Proto80400105 respond;
    respond.set_mover_id(mover_id);
    respond.set_toward(this->mover_detial_.__toward);

    for (MoverDetail::MoverStepList::iterator iter = this->mover_detial_.__step_list.begin();
            iter != this->mover_detial_.__step_list.end(); ++iter)
    {
        ProtoCoord *coord = respond.add_step_list();
        coord->set_pixel_x(iter->pixel_x());
        coord->set_pixel_y(iter->pixel_y());
    }

    ProtoClientHead head;
    head.__recogn = ACTIVE_MOVE_ACTION;
    return this->make_up_client_block(buff, &head, &respond);
}

int GameMover::sign_in(void)
{
    this->mover_detial_.__scene_id = this->correct_scene(this->scene_id());
    this->mover_type_ = GameCommon::fetch_mover_type(this->mover_id_low());
    this->set_active(true);
    return 0;
}

int GameMover::enter_scene(const int type)
{
	JUDGE_RETURN(this->is_enter_scene_ == false, -1);

    Scene* scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    this->check_and_set_mover_cur_block();
    scene->enter_scene(this, type);

	this->is_enter_scene_ = true;
	return 0;
}

int GameMover::exit_scene(const int type)
{
	JUDGE_RETURN(this->is_enter_scene_ == true, -1);
    this->is_enter_scene_ = false;

    Scene* scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, -1);

    return scene->exit_scene(this, type);
}

int GameMover::sign_out(void)
{
    this->set_active(false);
    return 0;
}

bool GameMover::is_movable_coord(const MoverCoord &coord)
{
	return GameCommon::is_movable_coord(this->scene_id(), coord);
}

bool GameMover::is_movable_coord_skill_push(const MoverCoord &coord)
{
	return true;
}

bool GameMover::is_movable_path_coord(const DynamicMoverCoord &coord)
{
	MoverCoord mover_coord;
	mover_coord.set_pixel(coord.pixel_x(), coord.pixel_y());
	return this->is_movable_coord(mover_coord);
}

int GameMover::generate_move_path(const MoverCoord &target, const int max_step)
{
    JUDGE_RETURN(this->location() != target, -1);

    if (this->mover_detial_.__move_path.rbegin() != this->mover_detial_.__move_path.rend())
    {
        MoverCoord &tail_coord = *(this->mover_detial_.__move_path.rbegin());
        if (tail_coord == target)
        {
            for (MoverDetail::MovePath::iterator iter = this->mover_detial_.__move_path.begin();
                    iter != this->mover_detial_.__move_path.end();)
            {
                if (*iter == this->location())
                {
                    iter = this->mover_detial_.__move_path.erase(iter);
                    this->generate_move_step_list();
                    return 0;
                }
                iter = this->mover_detial_.__move_path.erase(iter);
            }
        }
    }

    // 直线寻路
    {
		int ret = this->generate_path_by_direct(target, max_step);
		if (ret == 0)
		{
			this->generate_move_step_list();
			return 0;
		}
    }

    {
    	int ret = this->generate_path_by_astar(target);
		if (ret == 0)
		{
			this->generate_move_step_list();
			return 0;
		}
		return ret;
    }
}

int GameMover::generate_move_path_no_dynamic_mpt(const MoverCoord &target, const int max_step)
{
    if (this->location() == target)
        return -1;

    if (this->mover_detial_.__move_path.rbegin() != this->mover_detial_.__move_path.rend())
    {
        MoverCoord &tail_coord = *(this->mover_detial_.__move_path.rbegin());
        if (tail_coord == target)
        {
            for (MoverDetail::MovePath::iterator iter = this->mover_detial_.__move_path.begin();
                    iter != this->mover_detial_.__move_path.end();)
            {
                if (*iter == this->location())
                {
                    iter = this->mover_detial_.__move_path.erase(iter);
                    this->generate_move_step_list();
                    return 0;
                }
                iter = this->mover_detial_.__move_path.erase(iter);
            }
        }
    }


    int ret = this->generate_path_by_astar_no_dynamic_mpt(target);
    if (ret == 0)
    {
        this->generate_move_step_list_no_dynamic_mpt();
        return 0;
    }
    return ret;
}

int GameMover::generate_move_step_list(void)
{
    Time_Value tick;
    this->mover_detial_.__step_arrive_tick = Time_Value::gettimeofday();
    this->mover_detial_.__step_time = Time_Value::zero;
    this->mover_detial_.__step_list.clear();

    int i = 0, move_max_offset = 3, mover_spd = this->speed_total_i();	// TODO AI once broadcast step
    if (mover_spd <= 30)
    	move_max_offset = 1;
    else if (mover_spd <= 60)
    	move_max_offset = 2;
    MoverCoord cur_pos = this->location(), prev_pos;
    for (MoverDetail::MovePath::iterator iter = this->mover_detial_.__move_path.begin();
            iter != this->mover_detial_.__move_path.end(); ++iter)
    {
 		prev_pos = cur_pos;
        cur_pos = *iter;

        if (this->is_movable_path_coord(DynamicMoverCoord(cur_pos.pixel_x(), cur_pos.pixel_y())) == false)
            break;

        cur_pos.set_pos(cur_pos.pos_x(), cur_pos.pos_y());
        this->mover_detial_.__toward = vector_to_angle(prev_pos, cur_pos);

        tick = this->calculate_move_tick();
        this->mover_detial_.__step_arrive_tick += tick;
        this->mover_detial_.__step_time += tick;
        this->mover_detial_.__step_list.push_back(cur_pos);

        if (++i >= move_max_offset)
        {
            break;
        }
    }
    
	return 0;
}

int GameMover::generate_move_step_list_no_dynamic_mpt(void)
{
    Time_Value tick;
    this->mover_detial_.__step_arrive_tick = Time_Value::gettimeofday();
    this->mover_detial_.__step_time = Time_Value::zero;
    this->mover_detial_.__step_list.clear();

    int i = 0, move_max_offset = 3;	// TODO AI once broadcast step
    MoverCoord cur_pos = this->location(), prev_pos;
    for (MoverDetail::MovePath::iterator iter = this->mover_detial_.__move_path.begin();
            iter != this->mover_detial_.__move_path.end(); ++iter)
    {
 		prev_pos = cur_pos;
        cur_pos = *iter;
        if (this->is_movable_coord(cur_pos) == false)
            break;

        cur_pos.set_pos(cur_pos.pos_x(), cur_pos.pos_y());
        this->mover_detial_.__toward = vector_to_angle(prev_pos, cur_pos);

        tick = this->calculate_move_tick();
        this->mover_detial_.__step_arrive_tick += tick;
        this->mover_detial_.__step_time += tick;
        this->mover_detial_.__step_list.push_back(cur_pos);

        if (++i >= move_max_offset)
        {
            break;
        }
    }

	return 0;
}

int GameMover::generate_path_by_direct(const MoverCoord &org_target, const int max_step)
{
    DynamicMoverCoord mover_coord(this->location().pixel_x(), this->location().pixel_y()),
                      target(org_target.pixel_x(), org_target.pixel_y());
    MoverDetail::MovePath &move_path = this->mover_detial_.__move_path;
    move_path.clear();
    int dx = target.pixel_x() - mover_coord.pixel_x(),
        dy = target.pixel_y() - mover_coord.pixel_y();
    double k = 0, k_dlta = 0.0;
    int dlta = 0, is_use_dlta_x = 1, inc_unit = GameEnum::DEFAULT_AI_PATH_GRID;
    if (std::abs(dy) > std::abs(dx))
    {
        is_use_dlta_x = 0;
        dlta = (dy < 0 ? -inc_unit : inc_unit);
        if (dy != 0)
        {
            k = double(dx) / dy * inc_unit;
            k = (dlta < 0 ? -k : k);
        }
    }
    else
    {
        is_use_dlta_x = 1;
        dlta = (dx < 0 ? -inc_unit : inc_unit);
        if (dx != 0)
        {
            k = double(dy) / dx * inc_unit;
            k = (dlta < 0 ? -k : k);
        }
    }

    MoverCoord step_coord;
    DynamicMoverCoord next_step = mover_coord, prev_step;
    int i = 0;
    while (next_step.dynamic_pos_x() != target.dynamic_pos_x() || next_step.dynamic_pos_y() != target.dynamic_pos_y())
    {
        ++i;
        k_dlta += k;
        if (is_use_dlta_x == 1)
        {
            next_step.set_dynamic_pixel(next_step.pixel_x() + dlta, mover_coord.pixel_y() + int(k_dlta));
        }
        else
        {
        	next_step.set_dynamic_pixel(mover_coord.pixel_x() + int(k_dlta), next_step.pixel_y() + dlta);
        }
        if (this->is_movable_path_coord(next_step) == false)
            return -1;
        if (prev_step == next_step)
        	continue;

        step_coord.set_pixel(next_step.pixel_x(), next_step.pixel_y());
        move_path.push_back(step_coord);
        prev_step = next_step;

        if (i > 10000 || i >= max_step)
            break;
    }

    if ((next_step != target) || i >= max_step)
        return -1;

    MoverCoord last_step;
    last_step.set_pixel(next_step.pixel_x(), next_step.pixel_y());
    if (next_step == target && org_target != last_step)
    	 move_path.push_back(org_target);

    return 0;
}

int GameMover::generate_path_by_astar(const MoverCoord &org_target)
{
    static int diff[9][2] = {
        {0,0},  // NULL
        {0,-1}, // UP
        {1,-1}, // RIGHT_UP
        {1,0},  // RIGHT
        {1,1},  // RIGHT_DOWN
        {0,1},  // DOWN
        {-1,1}, // LEFT_DOWN
        {-1,0}, // LEFT
        {-1,-1} // LEFT_UP
    };

    typedef std::map<DynamicMoverCoord, PriorityCoord> CheckedMap;
    CheckedMap src_checked_map, target_checked_map;

    typedef std::priority_queue<PriorityCoord> CoordHeap;
    CoordHeap src_uncheck_heap, target_uncheck_heap;

    DynamicMoverCoord beg_coord(this->location().pixel_x(), this->location().pixel_y()), 
                      target(org_target.pixel_x(), org_target.pixel_y()), null_coord;
    int target_prio = this->calculate_target_priority(beg_coord, target);
    {
        PriorityCoord beg_coord_prio(target_prio, 0, beg_coord, beg_coord);
        src_uncheck_heap.push(beg_coord_prio);
    }
    {
        PriorityCoord target_coord_prio(target_prio, 0, target, target);
        target_uncheck_heap.push(target_coord_prio);
    }

    int cur_max_priority = 50000;
    if (MAP_MONITOR->is_has_travel_scene() == true)
    {
    	cur_max_priority = 5000;
    }

    bool is_in_src_checked_map = false, is_in_target_checked_map = false;
    while (src_uncheck_heap.empty() == false && target_uncheck_heap.empty() == false)
    {
        // init src cur pos;
        PriorityCoord src_cur_coord_prio = src_uncheck_heap.top();
        src_uncheck_heap.pop();
        DynamicMoverCoord src_cur_pos = src_cur_coord_prio.__cur_pos;
        if (src_cur_coord_prio.__source_priority > cur_max_priority)
        {
            MSG_USER("ERROR generate move path too big (%d,%d)->(%d,%d)",
                    beg_coord.pixel_x(), beg_coord.pixel_y(),
                    target.pixel_x(), target.pixel_y());
            return -1;
        }

        is_in_src_checked_map = true;
        CheckedMap::iterator src_iter = src_checked_map.find(src_cur_pos);
        if (src_iter == src_checked_map.end())
        {
            is_in_src_checked_map = false;
            src_checked_map[src_cur_pos] = src_cur_coord_prio;
        }

        // init target cur pos;
        PriorityCoord target_cur_coord_prio = target_uncheck_heap.top();
        target_uncheck_heap.pop();
        DynamicMoverCoord target_cur_pos = target_cur_coord_prio.__cur_pos;

        is_in_target_checked_map = true;
        CheckedMap::iterator target_iter = target_checked_map.find(target_cur_pos);
        if (target_iter == target_checked_map.end())
        {
            is_in_target_checked_map = false;
            target_checked_map[target_cur_pos] = target_cur_coord_prio;
        }

        // check path;
        CheckedMap::iterator check_target_iter = target_checked_map.find(src_cur_pos);
        if (check_target_iter != target_checked_map.end())
        {
//        	MSG_USER("path middle (%d,%d)->(%d,%d)->(%d,%d)",
//                    beg_coord.pixel_x(), beg_coord.pixel_y(),
//                    src_cur_pos.pixel_x(), src_cur_pos.pixel_y(),
//                    target.pixel_x(), target.pixel_y());
            MoverDetail::MovePath &path = this->mover_detial_.__move_path;
            path.clear();

            MoverCoord pos_coord;
            while (src_cur_coord_prio.__prev_pos != src_cur_pos)
            {
                pos_coord.set_pixel(src_cur_pos.pixel_x(), src_cur_pos.pixel_y());
                path.push_front(pos_coord);
                src_cur_pos = src_cur_coord_prio.__prev_pos;
                src_cur_coord_prio = src_checked_map[src_cur_pos];
            }

            target_cur_coord_prio = check_target_iter->second;
            target_cur_pos = target_cur_coord_prio.__prev_pos;
            target_cur_coord_prio = target_checked_map[target_cur_pos];
            while (target_cur_coord_prio.__prev_pos != target_cur_pos)
            {
                pos_coord.set_pixel(target_cur_pos.pixel_x(), target_cur_pos.pixel_y());
                path.push_back(pos_coord);
                target_cur_pos = target_cur_coord_prio.__prev_pos;
                target_cur_coord_prio = target_checked_map[target_cur_pos];
            }
            MoverCoord last_step;
            if (path.size() > 0)
            	last_step = path.back();
            if (last_step != org_target)
                path.push_back(org_target);
            return 0;
        }

        for (int i = 1; i <= 8; ++i)
        {
            if (is_in_src_checked_map == false)
            {
                DynamicMoverCoord src_next_step;
                src_next_step.set_dynamic_pos(src_cur_pos.dynamic_pos_x() + diff[i][0],
                        src_cur_pos.dynamic_pos_y() + diff[i][1]);
                CheckedMap::iterator src_iter = src_checked_map.find(src_next_step);
                if (this->is_movable_path_coord(src_next_step) == true &&
                        src_iter == src_checked_map.end())
                {
                    int target_prio = this->calculate_target_priority(src_next_step, target);
                    PriorityCoord next_coord_prio(target_prio, src_cur_coord_prio.__source_priority + 10, //
                            src_next_step, src_cur_pos);
                    if (i % 2 == 0)
                        next_coord_prio.__source_priority += 4;

                    src_uncheck_heap.push(next_coord_prio);
                }
            }
            if (is_in_target_checked_map == false)
            {
                DynamicMoverCoord target_next_step;
                target_next_step.set_dynamic_pos(target_cur_pos.dynamic_pos_x() + diff[i][0],
                        target_cur_pos.dynamic_pos_y() + diff[i][1]);
                CheckedMap::iterator target_iter = target_checked_map.find(target_next_step);
                if (this->is_movable_path_coord(target_next_step) == true &&
                        target_iter == target_checked_map.end())
                {
                    int target_prio = this->calculate_target_priority(target_next_step, beg_coord);
                    PriorityCoord next_coord_prio(target_prio, target_cur_coord_prio.__source_priority + 10, // 
                            target_next_step, target_cur_pos);
                    if (i % 2 == 0)
                        next_coord_prio.__source_priority += 4;

                    target_uncheck_heap.push(next_coord_prio);
                }
            }
        }
    }
    return -1;
}

int GameMover::calculate_target_priority(const DynamicMoverCoord &source, const DynamicMoverCoord &target)
{
    int dx = std::abs(target.dynamic_pos_x() - source.dynamic_pos_x()),
        dy = std::abs(target.dynamic_pos_y() - source.dynamic_pos_y());
    int diag = std::min(dx, dy), straight = dx + dy;
    return (14 * diag + 10 * (straight - 2 * diag));
}

int GameMover::generate_path_by_astar_no_dynamic_mpt(const MoverCoord &org_target)
{
    static int diff[9][2] = {
        {0,0},  // NULL
        {0,-1}, // UP
        {1,-1}, // RIGHT_UP
        {1,0},  // RIGHT
        {1,1},  // RIGHT_DOWN
        {0,1},  // DOWN
        {-1,1}, // LEFT_DOWN
        {-1,0}, // LEFT
        {-1,-1} // LEFT_UP
    };

    typedef std::map<MoverCoord, PriorityMptCoord> CheckedMap;
    CheckedMap src_checked_map, target_checked_map;

    typedef std::priority_queue<PriorityMptCoord> CoordHeap;
    CoordHeap src_uncheck_heap, target_uncheck_heap;

    MoverCoord beg_coord(this->location().pos_x(), this->location().pos_y()),
                      target(org_target.pos_x(), org_target.pos_y()), null_coord;
    int target_prio = this->calculate_target_priority_by_mpt(beg_coord, target);
    {
        PriorityMptCoord beg_coord_prio(target_prio, 0, beg_coord, beg_coord);
        src_uncheck_heap.push(beg_coord_prio);
    }
    {
        PriorityMptCoord target_coord_prio(target_prio, 0, target, target);
        target_uncheck_heap.push(target_coord_prio);
    }

    bool is_in_src_checked_map = false, is_in_target_checked_map = false;
    while (src_uncheck_heap.empty() == false && target_uncheck_heap.empty() == false)
    {
        // init src cur pos;
        PriorityMptCoord src_cur_coord_prio = src_uncheck_heap.top();
        src_uncheck_heap.pop();
        MoverCoord src_cur_pos = src_cur_coord_prio.__cur_pos;
        if (src_cur_coord_prio.__source_priority > 100000)
        {
            MSG_USER("ERROR generate move path too big (%d,%d)->(%d,%d)",
                    beg_coord.pixel_x(), beg_coord.pixel_y(),
                    target.pixel_x(), target.pixel_y());
            return -1;
        }

        is_in_src_checked_map = true;
        CheckedMap::iterator src_iter = src_checked_map.find(src_cur_pos);
        if (src_iter == src_checked_map.end())
        {
            is_in_src_checked_map = false;
            src_checked_map[src_cur_pos] = src_cur_coord_prio;
        }

        // init target cur pos;
        PriorityMptCoord target_cur_coord_prio = target_uncheck_heap.top();
        target_uncheck_heap.pop();
        MoverCoord target_cur_pos = target_cur_coord_prio.__cur_pos;

        is_in_target_checked_map = true;
        CheckedMap::iterator target_iter = target_checked_map.find(target_cur_pos);
        if (target_iter == target_checked_map.end())
        {
            is_in_target_checked_map = false;
            target_checked_map[target_cur_pos] = target_cur_coord_prio;
        }

        // check path;
        CheckedMap::iterator check_target_iter = target_checked_map.find(src_cur_pos);
        if (check_target_iter != target_checked_map.end())
        {
            MoverDetail::MovePath &path = this->mover_detial_.__move_path;
            path.clear();

            MoverCoord pos_coord;
            while (src_cur_coord_prio.__prev_pos != src_cur_pos)
            {
                pos_coord.set_pixel(src_cur_pos.pixel_x(), src_cur_pos.pixel_y());
                path.push_front(pos_coord);
                src_cur_pos = src_cur_coord_prio.__prev_pos;
                src_cur_coord_prio = src_checked_map[src_cur_pos];
            }

            target_cur_coord_prio = check_target_iter->second;
            target_cur_pos = target_cur_coord_prio.__prev_pos;
            target_cur_coord_prio = target_checked_map[target_cur_pos];
            while (target_cur_coord_prio.__prev_pos != target_cur_pos)
            {
                pos_coord.set_pixel(target_cur_pos.pixel_x(), target_cur_pos.pixel_y());
                path.push_back(pos_coord);
                target_cur_pos = target_cur_coord_prio.__prev_pos;
                target_cur_coord_prio = target_checked_map[target_cur_pos];
            }
            MoverCoord last_step;
            if (path.size() > 0)
            	last_step = path.back();
            if (last_step != org_target)
                path.push_back(org_target);
            return 0;
        }

        for (int i = 1; i <= 8; ++i)
        {
            if (is_in_src_checked_map == false)
            {
                MoverCoord src_next_step;
                src_next_step.set_pos(src_cur_pos.pos_x() + diff[i][0],
                        src_cur_pos.pos_y() + diff[i][1]);
                CheckedMap::iterator src_iter = src_checked_map.find(src_next_step);
                if (this->is_movable_coord(src_next_step) == true &&
                        src_iter == src_checked_map.end())
                {
                    int target_prio = this->calculate_target_priority_by_mpt(src_next_step, target);
                    PriorityMptCoord next_coord_prio(target_prio, src_cur_coord_prio.__source_priority + 10, //
                            src_next_step, src_cur_pos);
                    if (i % 2 == 0)
                        next_coord_prio.__source_priority += 4;

                    src_uncheck_heap.push(next_coord_prio);
                }
            }
            if (is_in_target_checked_map == false)
            {
                MoverCoord target_next_step;
                target_next_step.set_pos(target_cur_pos.pos_x() + diff[i][0],
                        target_cur_pos.pos_y() + diff[i][1]);
                CheckedMap::iterator target_iter = target_checked_map.find(target_next_step);
                if (this->is_movable_coord(target_next_step) == true &&
                        target_iter == target_checked_map.end())
                {
                    int target_prio = this->calculate_target_priority_by_mpt(target_next_step, beg_coord);
                    PriorityMptCoord next_coord_prio(target_prio, target_cur_coord_prio.__source_priority + 10, //
                            target_next_step, target_cur_pos);
                    if (i % 2 == 0)
                        next_coord_prio.__source_priority += 4;

                    target_uncheck_heap.push(next_coord_prio);
                }
            }
        }
    }
    return -1;
}

int GameMover::calculate_target_priority_by_mpt(const MoverCoord &source, const MoverCoord &target)
{
    int dx = std::abs(target.pos_x() - source.pos_x()),
        dy = std::abs(target.pos_y() - source.pos_y());
    return (dx + dy) * 10;
}

int GameMover::schedule_move(const MoverCoord &step, const int toward, const Time_Value &arrive_tick)
{
	JUDGE_RETURN(this->is_enter_scene() == true, ERROR_SCENE_NO_EXISTS);
	JUDGE_RETURN(this->location() != step, -1);

    int ret = this->validate_movable(step);
    JUDGE_RETURN(ret == 0, ret);

    // < nowtime, move need second;
    Time_Value tick = this->calculate_arrive_tick(this->location(), step);
    if (arrive_tick < tick)
    {
        this->mover_detial_.__step_arrive_tick = tick;
    }
    else
    {
        this->mover_detial_.__step_arrive_tick = arrive_tick;
    }

    Scene *scene = this->fetch_scene();
    if (scene == 0)
    {
        MSG_USER("no scene object %d %d", this->scene_id(), this->space_id());
        return ERROR_SCENE_NO_EXISTS;
    }

    ret = scene->refresh_mover_location(this, step);
    if(ret != 0)
    {
    	MSG_DEBUG("refresh_mover_location() ret %d", ret);
    }

    this->mover_detial_.__toward = toward;
    return ret;
}

int GameMover::validate_movable(const MoverCoord &step)
{
    JUDGE_RETURN(this->is_movable_coord(step) == true, ERROR_COORD_ILLEGAL);

    return 0;
}

int GameMover::calculate_toward(const MoverCoord &step)
{
	if (this->location() == step)
	{
		return this->mover_detial_.__toward;
	}

	return vector_to_angle(this->location(), step);
}

Time_Value GameMover::calculate_arrive_tick(const MoverCoord &pos, const MoverCoord &step)
{
    int step_amount = coord_offset_grid(pos, step);

	double tick = CONFIG_INSTANCE->monster_step_tick() * step_amount * 100.0 / this->speed_total();
	return Time_Value::gettime(tick);
}

Time_Value GameMover::calculate_move_tick()
{
	double tick = CONFIG_INSTANCE->monster_step_tick() * 100.0 / this->speed_total();
	return Time_Value::gettime(tick);
}

Scene* GameMover::fetch_scene(void)
{
    if (this->scene_ != 0
    		&& this->scene_->space_id() == this->space_id()
    		&& this->scene_->scene_id() == this->scene_id())
    {
        return this->scene_;
    }

    if (this->monitor()->find_scene(this->space_id(),
			this->scene_id(), this->scene_) == 0)
    {
    	return this->scene_;
    }

    return NULL;
}

void GameMover::set_cur_block_index(const BlockIndexType index)
{
    this->mover_detial_.__cur_block_index = index;
}

BlockIndexType GameMover::cur_block_index(void)
{
    return this->mover_detial_.__cur_block_index;
}

int GameMover::correct_scene(const int scene_id)
{
//    if (CONFIG_INSTANCE->is_convert_scene(scene_id) == false)
//    {
//        const ServerDetail &detail = CONFIG_INSTANCE->server_list(this->monitor()->server_config_index());
//        BIntMap::const_iterator iter = detail.__scene_convert_to_map.find(scene_id);
//        if (iter != detail.__scene_convert_to_map.end())
//        {
//        	MSG_USER("convert scene (%ld) %d->%d", this->mover_id(), scene_id, iter->second);
//
//            return iter->second;
//        }
//    }
    return scene_id;
}

double GameMover::speed_total(const int buff) const
{
	const BasicElement& speed_elem = this->mover_detial_.__speed;

	double multi_times = this->mover_detial_.__speed_multi.__total(buff, true);
    double speed = speed_elem.basic() * (multi_times - 1) + speed_elem.__total(buff);

    return std::max(speed, 0.0);
}

int GameMover::speed_total_i(const int buff) const
{
    return this->speed_total(buff);
}

double GameMover::speed_base_total(const int buff) const
{
    double speed = this->mover_detial_.__speed.__total(buff);
    return std::max(speed, 0.0);
}

int GameMover::speed_base_total_i(const int buff) const
{
    return this->speed_base_total(buff);
}

int GameMover::fetch_mover_type()
{
//	return GameCommon::fetch_mover_type(this->mover_id_low());
	return this->mover_type_;
}

int GameMover::check_mover_type(int mover_type)
{
	return this->mover_type_ == mover_type;
}

int GameMover::adjust_mover_coord()
{
	int ret = this->validate_movable(this->location());
	JUDGE_RETURN(ret != 0, -1);

	MSG_USER("ERROR %s, %ld, %d, %d [%d, %d]", this->name(), this->mover_id(), ret,
			this->scene_id(), this->location().pixel_x(), this->location().pixel_y());
	this->location() = this->fetch_config_relive_coord();
	return 0;
}

int GameMover::set_mover_cur_block()
{
	Scene* scene = this->fetch_scene();
	JUDGE_RETURN(scene != NULL, -1);

	BlockIndexType block_index = -1;
	scene->cal_block_index_by_grid_coord(this->location().pos_x(),
			this->location().pos_y(), block_index);

	this->set_cur_block_index(block_index);
	return 0;
}

int GameMover::check_and_set_mover_cur_block()
{
    JUDGE_RETURN(this->cur_block_index() < 0, -1);
    return this->set_mover_cur_block();
}

MoverCoord GameMover::fetch_config_relive_coord(int camp)
{
	MoverCoord relive_coord;
	const Json::Value& relive_json = this->scene_conf()["relive"];

    if (relive_json.isMember("fixed_pos") == true)
    {
    	relive_coord.set_pixel(relive_json["fixed_pos"][0u].asInt(),
    			relive_json["fixed_pos"][1u].asInt());
    }
    else if (relive_json.isMember("posX") == true)
    {
    	relive_coord.set_pixel(relive_json["posX"].asInt(),
    	        relive_json["posY"].asInt());
    }
    else if (relive_json.isMember("rand_pos") == true)
    {
    	relive_coord = GameCommon::fetch_rand_conf_pos(relive_json["rand_pos"]);
    }
    else if (relive_json.isMember("camp_pos") == true)
    {
    	relive_coord.set_pixel(relive_json["camp_pos"][camp - 1][0u].asInt(),
    			relive_json["camp_pos"][camp - 1][1u].asInt());
    }

    return relive_coord;
}
