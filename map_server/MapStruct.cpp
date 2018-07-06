/*
 * MapStruct.cpp
 *
 * Created on: 2013-01-18 11:50
 *     Author: glendy
 */

#include "MapStruct.h"
#include "GameMover.h"
#include "MapLogicPlayer.h"
#include "GamePackage.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "LeagueMonitor.h"
#include "SceneLineManager.h"
#include "MLBackDailyRechargeSys.h"
//#include "MapPlayer.h"
#include "MapPlayerEx.h"
#include "TrvlScriptMonitor.h"


SceneBlock::SceneBlock(void) : __block_index(-1)
{
	this->reset();
}

SceneBlock::SceneBlock(const SceneBlock &obj)
{
    *this = obj;
}

SceneBlock &SceneBlock::operator= (const SceneBlock &obj)
{
    this->__block_index = obj.__block_index;
    this->__data_buff.copy(const_cast<Block_Buffer *>(&(obj.__data_buff)));
    
    this->__mover_map.unbind_all();
    for (MoverMap::const_iterator iter = obj.__mover_map.begin();
            iter != obj.__mover_map.end(); ++iter)
    {
        this->__mover_map.bind(iter->first, iter->second);
    }

    this->__player_map.unbind_all();
    for (MoverMap::const_iterator iter = obj.__player_map.begin();
            iter != obj.__player_map.end(); ++iter)
    {
        this->__player_map.bind(iter->first, iter->second);
    }
    this->__other_mover_map.unbind_all();
    for (MoverMap::const_iterator iter = obj.__other_mover_map.begin();
    		iter != obj.__other_mover_map.end(); ++iter)
    {
    	this->__other_mover_map.bind(iter->first, iter->second);
    }

    this->__mover_offset_map.unbind_all();
    for (MoverOffsetMap::const_iterator iter = obj.__mover_offset_map.begin();
            iter != obj.__mover_offset_map.end(); ++iter)
    {
        this->__mover_offset_map.bind(iter->first, iter->second);
    }
    return *this;
}

void SceneBlock::reset(void)
{
    this->__block_index = -1;
    this->__data_buff.reset();
    this->__mover_map.unbind_all();
    this->__player_map.unbind_all();
    this->__other_mover_map.unbind_all();
    this->__mover_offset_map.unbind_all();
}

int SceneBlock::push_data(Block_Buffer &buff)
{
    if (this->__player_map.empty() == false)
    {
    	if (this->__data_buff.writable_bytes() < buff.readable_bytes())
    	{
    		int need_len = std::max(size_t(4096), buff.readable_bytes());
    		this->__data_buff.ensure_writable_bytes(need_len);
    	}
        this->__data_buff.copy(&buff);
    }
    return 0;
}

int SceneBlock::flush_to_mover(GameMover *mover)
{
    if (this->__data_buff.readable_bytes() <= 0)
        return 0;

    size_t offset = 0;
    if (this->__mover_offset_map.find(mover->mover_id(), offset) != 0)
    {
        mover->respond_from_broad_client(&(this->__data_buff));
    }
    else
    {
        size_t org_idx = this->__data_buff.get_read_idx();

        this->__data_buff.set_read_idx(offset);
        if (this->__data_buff.readable_bytes() > 0)
            mover->respond_from_broad_client(&(this->__data_buff));

        this->__data_buff.set_read_idx(org_idx);
    }
    return 0;
}

int SceneBlock::make_up_all_disappear_info(Block_Buffer &buff, GameMover *mover)
{
    for (MoverMap::iterator iter = this->__mover_map.begin();
    		iter != this->__mover_map.end(); ++iter)
    {
    	GameMover *p_mover = iter->second;
        JUDGE_CONTINUE(p_mover != mover);
        p_mover->make_up_disappear_info(&buff);
    }

    return 0;
}

int SceneBlock::make_up_all_appear_info(Block_Buffer &buff, GameMover *mover, bool send_by_gate)
{
    for (MoverMap::iterator iter = this->__player_map.begin();
    		iter != this->__player_map.end(); ++iter)
    {
    	GameMover *p_mover = iter->second;
        JUDGE_CONTINUE(p_mover != mover);

    	if (p_mover->is_enter_scene() == false)
        {
            MSG_USER("ERROR player no enter scene: %ld %x", p_mover->mover_id(), p_mover);
            continue;
        }

    	p_mover->make_up_appear_info(&buff, send_by_gate);
    }

    for (MoverMap::iterator iter = this->__other_mover_map.begin();
    		iter != this->__other_mover_map.end(); ++iter)
    {
    	GameMover *p_mover = iter->second;
        JUDGE_CONTINUE(p_mover != mover);

        if (p_mover->is_enter_scene() == false)
        {
            MSG_USER("ERROR mover no enter scene: %ld %x", p_mover->mover_id(), p_mover);
        }

        p_mover->make_up_appear_info(&buff, send_by_gate);
    }

    return 0;
}

void SceneDetail::reset(void)
{
    this->__scene_id = 0;
    this->__space_id = 0;

    this->__block_width = 0;
    this->__block_height = 0;
    this->__mpt_x_len = 0;
    this->__mpt_y_len = 0;
    this->__block_x_amount = 0;
    this->__block_y_amount = 0;

    this->__scene_block_list.clear();
    this->__mover_amount_list.clear();
    this->__player_amount_list.clear();
    this->__player_block_set.clear();
    this->__has_block_set.clear();
    
    {
        this->__dynamic_grid_pixel = 50;
        this->__dynamic_x_len = 0;
        this->__dynamic_y_len = 0;
        MptCoordList tmp_vec;
        this->__dynamic_mpt.swap(tmp_vec);
    }

    this->ai_sort_map_.clear();
    this->boss_sort_set_.clear();
    this->safe_area_set_.clear();

    for (SceneDetail::TeamSceneBlockMap::iterator iter = this->team_scene_block_map_.begin();
            iter != this->team_scene_block_map_.end(); ++iter)
    {
        MAP_MONITOR->team_scene_block_pool()->push(iter->second);
    }
    this->team_scene_block_map_.clear();

    this->__room_amount = 0;
    for(int i = 0; i < MAX_ROOM_SCENE_SIZE; i++)
    {
    	this->__room_player_amount_list[i] = 0;
    	this->__scene_block_vec[i].clear();
    	this->__mover_amount_vec[i].clear();
    	this->__player_amount_vec[i].clear();
    	this->__player_map_vec[i].clear();

        MptCoordList tmp_vec;
        this->__dynamic_mpt_vec[i].swap(tmp_vec);
    }
    this->ai_group_map_.clear();

    this->__near_block_width = 0;
    this->__near_block_height = 0;
    this->__near_block_x_amount = 0;
    this->__near_block_y_amount = 0;
    this->__near_block_list.clear();
    this->__near_player_block_set.clear();
    this->__near_notify_tick = Time_Value::zero;
}

MoverCoord::MoverCoord(int pos_x, int pos_y) :
    __pos_x(0), __pos_y(0), __pixel_x(0), __pixel_y(0)
{
	this->set_pos(pos_x, pos_y);
}

void MoverCoord::reset(void)
{
    this->__pos_x = 0;
    this->__pos_y = 0;
    this->__pixel_x = 0;
    this->__pixel_y = 0;
}

int MoverCoord::pos_x(void) const
{
    return this->__pos_x;
}

int MoverCoord::pos_y(void) const
{
    return this->__pos_y;
}

int MoverCoord::pixel_x(void) const
{
    return this->__pixel_x;
}

int MoverCoord::pixel_y(void) const
{
    return this->__pixel_y;
}

void MoverCoord::serialize(ProtoCoord* proto_coord) const
{
	proto_coord->set_pixel_x(this->__pixel_x);
	proto_coord->set_pixel_y(this->__pixel_y);
}

void MoverCoord::unserialize(ProtoCoord* proto_coord)
{
	this->set_pixel(proto_coord->pixel_x(), proto_coord->pixel_y());
}

Int64 MoverCoord::coord_index()
{
	return merge_int_to_long(this->__pos_x, this->__pos_y);
}

void MoverCoord::set_pos(const int pos_x, const int pos_y, const int cell_width, const int cell_height)
{
    this->__pos_x = pos_x;
    this->__pos_y = pos_y;
    this->__pixel_x = pos_to_pixel(pos_x, cell_width);
    this->__pixel_y = pos_to_pixel(pos_y, cell_height);
}

void MoverCoord::set_pixel(const int pixel_x, const int pixel_y, const int cell_width, const int cell_height)
{
    this->__pixel_x = pixel_x;
    this->__pixel_y = pixel_y;
    this->__pos_x = pixel_to_pos(pixel_x, cell_width);
    this->__pos_y = pixel_to_pos(pixel_y, cell_height);
}

int MoverCoord::pixel_to_pos(const int pixel, const int cell)
{
    return (pixel / cell);
}

int MoverCoord::pos_to_pixel(const int pos, const int cell)
{
    return (pos * cell + cell / 2);
}

bool operator==(const MoverCoord &left, const MoverCoord &right)
{
    return (left.pos_x() == right.pos_x()) && (left.pos_y() == right.pos_y());
}

bool operator!=(const MoverCoord &left, const MoverCoord &right)
{
    return !(left == right);
}

bool operator<(const MoverCoord &left, const MoverCoord &right)
{
    if (left.pixel_x() < right.pixel_x())
    {
        return true;
    }
    else if (left.pixel_x() == right.pixel_x())
    {
        if (left.pixel_y() < right.pixel_y())
            return true;
    }
    return false;
}

int coord_offset_grid(const MoverCoord &mover_coord, const MoverCoord &target_coord)
{
    int offset_x = abs(mover_coord.pixel_x() - target_coord.pixel_x()) / 30;
    int offset_y = abs(mover_coord.pixel_y() - target_coord.pixel_y()) / 30;

    return std::max<int>(offset_x, offset_y);
}

int coord_offset_grid_min(const MoverCoord &mover_coord, const MoverCoord &target_coord)
{
    int offset_x = abs(mover_coord.pixel_x() - target_coord.pixel_x()) / 30;
    int offset_y = abs(mover_coord.pixel_y() - target_coord.pixel_y()) / 30;

    return std::min<int>(offset_x, offset_y);
}

bool check_coord_distance(const MoverCoord &mover_coord, const MoverCoord &target_coord,
		const int distance, const int mapping_factor/* = 1*/, const int grid_dis)
{
    int pixel_distance = (distance + 1) * grid_dis;	//由整数相除取整，导致有一格之差， 如[240,269]都属于8格
    int offset_x = mover_coord.pixel_x() - target_coord.pixel_x();
    int offset_y = mapping_factor * (mover_coord.pixel_y() - target_coord.pixel_y());
    return ((offset_x * offset_x + offset_y * offset_y) < (pixel_distance * pixel_distance));
}

bool check_coord_pixel_distance(const MoverCoord &mover_coord, const MoverCoord &target_coord,
        const int pixel_radius, const double mapping_factor/* = 1*/)
{
    int offset_x = (mover_coord.pixel_x() - target_coord.pixel_x());
    int offset_y = mapping_factor * (mover_coord.pixel_y() - target_coord.pixel_y());
    return ((offset_x * offset_x + offset_y * offset_y) <= pixel_radius * pixel_radius);
}

int vector_to_angle(const MoverCoord &start, const MoverCoord &end)
{
	double radian = vector_to_radian(start, end);
	return radian * 180 / PI;
}

int toward_to_angle(const int toward_index)
{
    static int toward_angle[] = {
    		90,
    		-90,
    		-45,
    		0,
    		45,
    		90,
    		135,
    		180,
    		-135
    };
    if (toward_index < 0 || toward_index >= (int)(sizeof(toward_angle) / sizeof(int)))
        return toward_angle[0];
    return toward_angle[toward_index];
}

double vector_to_radian(const MoverCoord &start, const MoverCoord &end)
{
    double dx = end.pixel_x() - start.pixel_x(),
           dy = end.pixel_y() - start.pixel_y();
    if (dx == 0)
    {
    	if (dy < 0)
    		return -PI / 2.0;
        return PI / 2.0;
    }

    double angle = ::atan(dy / dx);
    if (dx < 0)
    {
        if (angle > 0)
            angle -= PI;
        else
            angle += PI;
    }
    return angle;
}

int center_to_rect(MoverCoord &pointA, MoverCoord &pointB, MoverCoord &pointC, MoverCoord &pointD,
        const MoverCoord &center, const double angle, const double width, const double height)
{
	double cos_a = ::cos(angle);
	double sin_a = ::sin(angle);

	MoverCoord point1;
	point1.set_pixel(center.pixel_x() - height / 2.0 * cos_a, center.pixel_y() - height / 2.0 * sin_a);

    int point_x, point_y;
    {
		point_x = point1.pixel_x() + width / height * (center.pixel_y() - point1.pixel_y());
		point_y = point1.pixel_y() - width / height * (center.pixel_x() - point1.pixel_x());
        pointA.set_pixel(point_x, point_y);
    }
    {
		point_x = point1.pixel_x() - width / height * (center.pixel_y() - point1.pixel_y());
		point_y = point1.pixel_y() + width / height * (center.pixel_x() - point1.pixel_x());
        pointB.set_pixel(point_x, point_y);
    }
    {
		point_x = pointB.pixel_x() + height * cos_a;
		point_y = pointB.pixel_y() + height * sin_a;
        pointC.set_pixel(point_x, point_y);
    }
    {
		point_x = pointA.pixel_x() + height * cos_a;
		point_y = pointA.pixel_y() + height * sin_a;
        pointD.set_pixel(point_x, point_y);
    }
    return 0;
}

SaveSceneInfo::SaveSceneInfo()
{
	SaveSceneInfo::reset();
}

void SaveSceneInfo::reset()
{
	this->scene_id_ 	= 0;
	this->scene_mode_ 	= 0;
	this->blood_ 		= 0;
	this->pk_state_ 	= 0;
	this->space_id_ 	= 0;
	this->coord_.reset();
}

void MoverDetail::reset(void)
{
    this->__scene_id = 0;
    this->__location.reset();
    this->__scene_mode = 0;
    this->__space_id = 0;

    this->__prev_scene_id = 0;
    this->__prev_location.reset();
    this->__prev_scene_mode = 0;
    this->__prev_space_id = 0;

    this->__cur_block_index = -1;
    this->__temp_location.reset();

    this->__step_arrive_tick = Time_Value::zero;
    this->__step_time = Time_Value::zero;
    this->__toward = 0;
    this->__step_list.clear();
    this->__move_path.clear();

    this->__speed.reset();
    this->__speed_multi.reset();

    this->__speed.set_single(CONFIG_INSTANCE->base_role_speed(), BasicElement::BASIC);
    this->__speed_multi.set_single(1, BasicElement::BASIC);
}


MapTeamInfo::MapTeamInfo(void)
{
	MapTeamInfo::reset();
}

void MapTeamInfo::reset(void)
{
	this->team_index_ = 0;
	this->leader_id_ = 0;
	this->teamer_set_.clear();
	this->replacement_set_.clear();
}

void MapTeamInfo::serialize(ProtoTravelTeam* team)
{
	team->set_team_id(this->team_index_);
	team->set_leader_id(this->leader_id_);

	for (LongMap::iterator iter = this->teamer_set_.begin();
			iter != this->teamer_set_.end(); ++iter)
	{
		team->add_teamer_id(iter->first);
	}
}

void MapTeamInfo::unserialize(const ProtoTravelTeam& team)
{
	this->team_index_ = team.team_id();
	this->leader_id_ = team.leader_id();

	for (int i = 0; i < team.teamer_id_size(); ++i)
	{
		Int64 role_id = team.teamer_id(i);
		this->teamer_set_[role_id] = true;
	}
}

int MapTeamInfo::team_index()
{
	return this->team_index_;
}

void HookDetail::reset(void)
{
	::memset(this, 0, sizeof(HookDetail));
	this->__stop_hook = 1;
}

TransferDetail::TransferInfo::TransferInfo(int id)
{
	TransferInfo::reset();
	this->transfer_id_ = id;
}

void TransferDetail::TransferInfo::reset()
{
	this->delete_skill();

	this->transfer_id_ 	= 0;
	this->transfer_lv_ 	= 0;
	this->is_permanent_ = 0;
	this->is_active_ 	= 0;
	this->active_tick_ 	= 0;
	this->end_tick_ 	= 0;
	this->transfer_skill_ = 0;
	this->skill_map_.clear();

	this->one_prop_.reset();
}

const Json::Value& TransferDetail::TransferInfo::conf()
{
	return CONFIG_INSTANCE->transfer_total(this->transfer_id_);
}

const Json::Value& TransferDetail::TransferInfo::detail_conf()
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, Json::Value::null);

	const Json::Value& change_conf = conf["reward"];
	for (uint i = 0; i < change_conf.size(); ++i)
	{
		int level = change_conf[i]["level"].asInt();
		JUDGE_CONTINUE(this->transfer_lv_ == level);

		return change_conf[i];
	}

	return Json::Value::null;
}

bool TransferDetail::TransferInfo::is_active_time()
{
	JUDGE_RETURN(this->is_permanent_ == false, true);

	Int64 cur_tick = ::time(NULL);
	return this->end_tick_ >= cur_tick;
}

void TransferDetail::TransferInfo::add_skill()
{
	const Json::Value& conf = this->conf();
	JUDGE_RETURN(conf.empty() == false, ;);

	if (this->transfer_skill_ <= 0)
	{
		this->transfer_skill_ = conf["transfer_skill"].asInt();
	}

	this->skill_map_.clear();
	int normal_attack = conf["normal_attack"].asInt();
	this->skill_map_[normal_attack] = 1;

	const Json::Value& normal_skill = conf["normal_skill"];
	for (uint i = 0; i < normal_skill.size(); ++i)
	{
		int skill_id = normal_skill[i].asInt();
		this->skill_map_[skill_id] = 1;
	}

	const Json::Value& special_skill = conf["special_skill"];
	for (uint i = 0; i < special_skill.size(); ++i)
	{
		int skill_id = special_skill[i].asInt();
		this->skill_map_[skill_id] = 1;
	}
}

void TransferDetail::TransferInfo::create_skill()
{
	if (this->skill_map_.size() <= 0)
	{
		this->add_skill();
	}

	for (IntMap::iterator iter = this->skill_map_.begin();
			iter != this->skill_map_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first > 0);

		iter->second = iter->second > 0 ? iter->second : 1;

		FighterSkill* skill = NULL;
		if (this->skill_info_.count(iter->first) == 0)
		{
			skill = MAP_MONITOR->pop_skill(iter->first, iter->second);
		}
		else
		{
			skill = this->skill_info_[iter->first];
			MAP_MONITOR->update_skill_level(skill, iter->second);
		}

		JUDGE_CONTINUE(skill != NULL);
		this->skill_info_[iter->first] = skill;
	}
}

void TransferDetail::TransferInfo::delete_skill()
{
	for (SkillMap::iterator iter = this->skill_info_.begin();
			iter != this->skill_info_.end(); ++iter)
	{
		MAP_MONITOR->skill_pool()->push(iter->second);
	}
	this->skill_info_.clear();
}

void TransferDetail::TransferInfo::serialize(ProtoTransferInfo *proto)
{
	proto->set_transfer_id(this->transfer_id_);
	proto->set_transfer_lv(this->transfer_lv_);
	proto->set_is_permanent(this->is_permanent_);
	proto->set_is_active(this->is_active_);
	proto->set_active_tick(this->active_tick_);
	proto->set_end_tick(this->end_tick_);
	proto->set_transfer_skill(this->transfer_skill_);

	for (IntMap::iterator iter = this->skill_map_.begin();
			iter != this->skill_map_.end(); ++iter)
	{
		ProtoPairObj* pair = proto->add_skill();
		pair->set_obj_id(iter->first);
		pair->set_obj_value(iter->second);
	}
}

void TransferDetail::TransferInfo::unserialize(ProtoTransferInfo proto)
{
	this->transfer_id_ 	= proto.transfer_id();
	this->transfer_lv_ 	= proto.transfer_lv();
	this->is_permanent_ = proto.is_permanent();
	this->is_active_ 	= proto.is_active();
	this->active_tick_ 	= proto.active_tick();
	this->end_tick_ 	= proto.end_tick();
	this->transfer_skill_ = proto.transfer_skill();

	for (int j = 0; j < proto.skill_size(); ++j)
	{
		const ProtoPairObj &obj = proto.skill(j);
		this->skill_map_[obj.obj_id()] = obj.obj_value();
	}
}

void TransferDetail::reset()
{
	this->level_ = 0;
	this->exp_ 	 = 0;
	this->open_  = 0;
	this->stage_ = 0;
	this->gold_times_ = 0;

	this->transfer_tick_ = 0;
	this->last_  		 = 0;
	this->active_id_  	 = 0;
	this->open_reward_ 	 = 0;

	this->refresh_tick_ = Time_Value::zero;

	this->reduce_cool_ = 0;
	this->add_time_    = 0;

	this->spirit_prop_.reset();
	this->total_prop_.reset();

	for (TransferInfoMap::iterator iter = this->transfer_map_.begin();
			iter != this->transfer_map_.end(); ++iter)
	{
		TransferInfo &info = iter->second;
		info.reset();
	}
	this->transfer_map_.clear();
}

void TransferDetail::reset_every_day()
{
	this->gold_times_ = 0;

	Time_Value nowtime = Time_Value::gettimeofday();
	this->refresh_tick_ = next_day(0, 0, nowtime);
}

TransferDetail::TransferInfo* TransferDetail::fetch_transfer_info()
{
	return this->fetch_transfer_info(this->active_id_);
}

TransferDetail::TransferInfo *TransferDetail::fetch_transfer_info(int id)
{
	JUDGE_RETURN(this->transfer_map_.count(id) > 0, NULL);
	return &(this->transfer_map_[id]);
}

bool TransferDetail::is_in_cool()
{
	const Json::Value& conf = CONFIG_INSTANCE->transfer_total(this->active_id_);
	JUDGE_RETURN(conf.empty() == false, false);

	Int64 cur_tick = ::time(NULL);
	int pass_tick = cur_tick - this->transfer_tick_;
	if (pass_tick >= conf["cool"].asInt() * (1 - double(this->reduce_cool_) / 100))
	{
		this->transfer_tick_ = 0;
		this->last_ = 0;
		return false;
	}

	return true;
}

bool TransferDetail::is_in_transfer()
{
	const Json::Value& conf = CONFIG_INSTANCE->transfer_total(this->active_id_);
	JUDGE_RETURN(conf.empty() == false, false);

	Int64 cur_tick = ::time(NULL);
	return cur_tick < this->transfer_tick_ + this->last_;
}

int TransferDetail::transfer_cool()
{
	const Json::Value& conf = CONFIG_INSTANCE->transfer_total(this->active_id_);
	JUDGE_RETURN(conf.empty() == false, 0);

	Int64 cur_tick = ::time(NULL);
	int pass_tick = cur_tick - this->transfer_tick_;
	int left_cool_tick = conf["cool"].asInt() * (1 - double(this->reduce_cool_) / 100) - pass_tick;
	if (left_cool_tick < 0)
		return 0;

	return left_cool_tick;
}

const Json::Value& TransferDetail::spirit_level_json()
{
	return CONFIG_INSTANCE->spirit_level(this->level_);
}

const Json::Value& TransferDetail::spirit_stage_json()
{
	return CONFIG_INSTANCE->spirit_stage(this->stage_);
}

const Json::Value& TransferDetail::transfer_base_json()
{
	return CONFIG_INSTANCE->transfer_base();
}

RoleFashion::FashionInfo::FashionInfo()
{
	FashionInfo::reset();
}

void RoleFashion::FashionInfo::reset()
{
	this->fashion_id_ 	= 0;
	this->color_id_   	= 0;
	this->active_type_ 	= 0;
	this->is_permanent_ = 0;
	this->active_tick_	= 0;
	this->end_tick_ 	= 0;

	this->color_set_.clear();
	this->fight_prop_.reset();
	this->color_num_prop_.reset();
}

const Json::Value& RoleFashion::FashionInfo::fashion_json()
{
	return CONFIG_INSTANCE->fashion(this->fashion_id_);
}

int RoleFashion::FashionInfo::check_color_set_has_color(int color_id)
{
	for (IntVec::iterator iter = this->color_set_.begin();
			iter != this->color_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter == color_id);
		return true;
	}
	return false;
}

int RoleFashion::FashionInfo::check_json_has_color(int color_id)
{
	const Json::Value &fashion_json = this->fashion_json();
	const Json::Value &color_json = fashion_json["color"];
	for (uint i = 0; i < color_json.size(); ++i)
	{
		int id = color_json[i][0u].asInt();
		JUDGE_CONTINUE(id != 0);
		JUDGE_CONTINUE(id == color_id);
		return true;
	}
	return false;
}

RoleFashion::RoleFashion()
{
	RoleFashion::reset();
}

void RoleFashion::reset()
{
	this->level_ 	 = 0;
	this->exp_ 		 = 0;
	this->select_id_ = 0;
	this->open_ 	 = 0;

	this->send_set_.clear();
	this->fashion_map_.clear();
	this->total_prop_.reset();
}

int RoleFashion::permanent_fashion_amount()
{
	int amount = 0;
	for (FashionInfoMap::iterator iter = this->fashion_map_.begin();
			iter != this->fashion_map_.end(); ++iter)
	{
		FashionInfo& fashion_info = iter->second;
		JUDGE_CONTINUE(fashion_info.active_type_ == true);
		JUDGE_CONTINUE(fashion_info.is_permanent_ == true);

		++amount;
	}
	return amount;
}

int RoleFashion::check_in_send_set(int fashion_id)
{
	for (IntVec::iterator iter = this->send_set_.begin();
			iter != this->send_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(*iter == fashion_id);
		return true;
	}
	return false;
}

RoleFashion::FashionInfo* RoleFashion::fetch_fashion_info(int fashion_id)
{
	JUDGE_RETURN(this->fashion_map_.count(fashion_id) > 0, NULL);

	return &(this->fashion_map_[fashion_id]);
}

const Json::Value& RoleFashion::fashion_level()
{
	return CONFIG_INSTANCE->fashion_level(this->level_ + 1);
}

const Json::Value& RoleFashion::fashion_num_conf()
{
	int amount = this->permanent_fashion_amount();
	return CONFIG_INSTANCE->fashion_num_add(amount);
}

HiddenTreasureDetail::HiddenTreasureDetail(void)
{
	HiddenTreasureDetail::reset();
}

void HiddenTreasureDetail::reset(void)
{
	this->day_ = 0;
	this->open_ = 0;
	this->get_status_ = 0;
	this->refresh_tick_ = 0;
	this->buy_map_.clear();
}

void HiddenTreasureDetail::set_next_day_info()
{
	if (this->refresh_tick_ > 0)
	{
		Int64 cur_time = ::time(NULL);
		int interval_day = GameCommon::day_interval(cur_time, this->refresh_tick_) + 1;
		this->day_ += interval_day;
	}
	else
	{
		++(this->day_);
	}
	this->get_status_ = 0;
	this->buy_map_.clear();
	this->refresh_tick_ = GameCommon::today_zero() + Time_Value::DAY;

	const Json::Value& treasure_json = CONFIG_INSTANCE->hidden_treasure_json(this->day_);
	if (treasure_json == Json::Value::null)
	{
		this->open_ = END_OPEN;
	}
	JUDGE_RETURN(this->open_ == 1, ;);

	const Json::Value& cost_item = treasure_json["cost_item"];
	for (uint i = 0; i < cost_item.size(); ++i)
	{
		this->buy_map_[int(i+1)] = false;
	}
}

SwordPoolDetail::SwordPoolDetail(void):
level_(0), exp_(0), open_(0), stype_lv_(0)
{
	/*NULL*/
}

void SwordPoolDetail::reset(void)
{
	this->level_ = 0;
	this->exp_ 	 = 0;
	this->open_  = 0;
	this->stype_lv_ = 0;
	this->refresh_tick_ = Time_Value::zero;
	this->last_task_map_.clear();
	this->today_task_map_.clear();
}

SwordPoolDetail::PoolTaskInfo::PoolTaskInfo(void)
{
	this->task_id_ 	 = 0;
	this->total_num_ = 0;
	this->left_num_  = 0;
}

const Json::Value& MountDetail::conf()
{
	return CONFIG_INSTANCE->mount(this->type_, this->mount_grade_);
}

const Json::Value& MountDetail::set_conf()
{
	return CONFIG_INSTANCE->mount_set(this->type_);
}

FighterSkill* MountDetail::find_skill(int id)
{
	JUDGE_RETURN(this->skill_map_.count(id) > 0, NULL);
	return this->skill_map_[id];
}

int MountDetail::is_all_skill_open()
{
	const Json::Value &mount_json = CONFIG_INSTANCE->mount(this->type_);
	for (uint i = this->mount_grade_ + 1; i < mount_json.size(); ++i)
	{
		const Json::Value &evolution_conf = CONFIG_INSTANCE->mount(this->type_, i);
		JUDGE_CONTINUE(evolution_conf != Json::Value::null);

		int skill_id = evolution_conf["add_skill"].asInt();
		JUDGE_RETURN(skill_id <= 0, false);
	}

	return true;
}

int MountDetail::is_one_level_skill(int skill_id)
{
	FighterSkill* skill = this->skill_map_[skill_id];
	const Json::Value &skill_json = skill->conf();
	JUDGE_RETURN(skill_json != Json::Value::null, true);
	JUDGE_RETURN(skill_json.isMember("upgrade_goods") == true, true);

	return false;
}

double MountDetail::left_blood_value(int skill_id, int left_blood_per, int type)
{
	FighterSkill* skill = this->find_skill(skill_id);
	JUDGE_RETURN(skill != NULL, 0);

	int blood_per = skill->detail()["blood_per"].asInt();
	JUDGE_RETURN(left_blood_per < blood_per, 0);

	if (type == 0)
	{
		return skill->detail()["value"].asInt();
	}
	else
	{
		return GameCommon::div_percent(skill->detail()["percent"].asInt());
	}
}

void MountDetail::set_grade(int grade, int add)
{
	this->mount_grade_ = grade + add;

	const Json::Value& evolution_conf = this->conf();
	this->limit_flag_ = evolution_conf["clear"].asInt();
}

void MountDetail::add_new_skill(int id, int lvl)
{
	JUDGE_RETURN(id > 0, ;);

	FighterSkill* skill = NULL;
	lvl = std::max<int>(1, lvl);

	if (this->skill_map_.count(id) == 0)
	{
		skill = MAP_MONITOR->pop_skill(id, lvl);
	}
	else
	{
		skill = this->skill_map_[id];
		MAP_MONITOR->update_skill_level(skill, lvl);
	}

	JUDGE_RETURN(skill != NULL, ;);
	this->skill_map_[id] = skill;
}

void MountDetail::reset(int type, int flag)
{
	for (SkillMap::iterator iter = this->skill_map_.begin();
			iter != this->skill_map_.end(); ++iter)
	{
		MAP_MONITOR->skill_pool()->push(iter->second);
	}

	this->skill_map_.clear();

	this->type_ 		= type;
	this->open_ 		= false;
	this->on_mount_ 	= true;
	this->limit_flag_ 	= 0;
	this->mount_shape_ 	= 0;
	this->act_shape_ 	= 0;
	this->mount_grade_ 	= 1;

	this->bless_ 			= 0;
	this->fail_times_ 		= 0;
	this->finish_bless_ 	= 0;

	this->ability_amount_ 	= 0;
	this->growth_amount_ 	= 0;
	this->sword_pool_level_ = 0;

	this->temp_prop_.reset();
	this->fight_prop_.reset();
	this->total_prop_.reset();
	JUDGE_RETURN(flag == 1, ;);

	const Json::Value& set_conf = this->set_conf();
	this->prop_index_ 	= set_conf["prop"].asInt();
	this->equip_index_ 	= set_conf["equip"].asInt();
	this->try_task_id_	= set_conf["try_task_id"].asInt();
	this->open_activity_= set_conf["open_activity"].asInt();
	this->skill_fun_ 	= set_conf["skill_fun"].asInt();
	this->fun_str_		= set_conf["fun_str"].asString();
	this->shout_id_ 	= set_conf["shout"].asInt();
	this->shout_start_ 	= set_conf["shout_start"].asInt();

	this->evoluate_red_ = set_conf["evoluate_red"].asInt();
	this->ability_red_	= set_conf["ability_red"].asInt();
	this->growth_red_	= set_conf["growth_red"].asInt();
	this->skill_red_	= set_conf["skill_red"].asInt();
	this->equip_red_	= set_conf["equip_red"].asInt();
	this->no_tips_		= set_conf["no_tips"].asInt();
	this->tips_task_ 	= set_conf["tips_task"].asInt();
	this->ach_grade_ 	= set_conf["ach_grade"].asInt();
	this->ach_skill_	= set_conf["ach_skill"].asInt();
	this->force_open_ 	= set_conf["force_open"].asInt();
}

void MountDetail::set_bless_time()
{
	JUDGE_RETURN(this->finish_bless_ == 0, ;);
	this->finish_bless_ = ::time(NULL) + Time_Value::DAY;
}

void MountDetail::check_adjust_open(int level)
{
	JUDGE_RETURN(this->open_ == false, ;);
	JUDGE_RETURN(this->force_open_ > 0, ;);
	JUDGE_RETURN(level >= this->force_open_, ;);
	this->open_ = true;
}

int MountDetail::shape_id()
{
	JUDGE_RETURN(this->on_mount_ == true, 0);

	if (this->act_shape_ > 0 && this->mount_shape_ == 0)
	{
		return this->act_shape_;
	}
	else
	{
		return this->mount_shape_;
	}
}

int MountDetail::hidden_flag()
{
	return this->on_mount_ == false;
}

int MountDetail::prop_open()
{
	if (this->open_ == true)
	{
		return true;
	}

	if (this->act_shape_ > 0)
	{
		return true;
	}

	return 0;
}

int MountDetail::sword_pool_multi()
{
	JUDGE_RETURN(this->sword_pool_level_ > 0, 0);

	const Json::Value &set_up_info = CONFIG_INSTANCE->sword_pool_set_up(
			this->sword_pool_level_);
	if (this->type_ == GameEnum::FUN_MOUNT)
	{
		return set_up_info["battle_horse"].asInt();
	}
	else
	{
		return set_up_info["wing_prop"].asInt();
	}
}

int MountDetail::left_time()
{
	JUDGE_RETURN(this->limit_flag_ == true, 0);
	JUDGE_RETURN(this->bless_ > 0, 0);

	return GameCommon::left_time(this->finish_bless_);
}

int MountDetail::skill_force()
{
	int skill_force = 0;

	for (SkillMap::iterator iter = this->skill_map_.begin();
			iter != this->skill_map_.end(); ++iter)
	{
		FighterSkill* skill = iter->second;
		skill_force += skill->__server_force;
	}
	return skill_force;
}

BeastDetail::BeastDetail(void)
{
	BeastDetail::reset();
}

void BeastDetail::reset(void)
{
//	this->beast_id_ = 0;	not reset
	this->src_beast_id_ = 0;
	this->master_id_ = 0;

	this->type_ = 0;
	this->beast_sort_ = 0;
	this->beast_name_.clear();
}


MapOneSecTimer::MapOneSecTimer() : FixedTimer(GTT_MAP_ONE_SECOND, Time_Value::SECOND)
{
}

int MapOneSecTimer::handle_timeout(const Time_Value &tv)
{
//	MAP_MONITOR->check_and_run_scene_monster();
//	RoomScene::wboss_check_time_and_start();
//    MAP_MONITOR->scene_line_manager()->check_scene_line_config(tv);
//    BackDR_SYS->check_and_notify_state_to_all();
//    SHUSAN_MONITOR->check_and_born_scene_boss(tv);
//    GONGCHENG_MONITOR->check_for_born_or_clear_sm(tv);
//    GONGCHENG_MONITOR->check_for_born_or_clear_qixi(tv);
//    LSIEGE_MONITOR->timeout_check_tick(tv);
	return 0;
}


MLVipDetail::MLVipDetail(void)
{
	MLVipDetail::reset();
}

void MLVipDetail::reset(void)
{
	BaseVipDetail::reset();

	this->__check_flag = false;
	this->__is_given.clear();
	this->__weekly_given.clear();
	this->__super_get_type = 0;
	this->__des_mail.clear();
	this->__weekly_tick = Time_Value::zero;
};

bool MLVipDetail::is_has_reward(int vip_type, int reward_type)
{
	JUDGE_RETURN(this->__vip_type >= vip_type, false);

	if (reward_type == 0)
	{
		return this->__is_given[vip_type] == 0;
	}
	else
	{
		return this->__weekly_given[vip_type] == 0;
	}
}


OfflineRewardsDetail::OfflineRewardsDetail(void)
{
	OfflineRewardsDetail::reset();
}

void OfflineRewardsDetail::reset(void)
{
	__received_mark = 0;
	__logout_time = 0;
	__offline_sum = 0;
	__exp_num = 0;
	__longest_time = 43200;
}

OnlineRewardsDetail::OnlineRewardsDetail(void) :
    __stage(0), __pre_stage(0), __received_mark(1), __login_time(0), __online_sum(0)
{ /*NULL*/ }

void OnlineRewardsDetail::reset(void)
{
	__stage = 0;
	__pre_stage = 0;
	__received_mark = 1;
	__login_time = 0;
	__online_sum = 0;
	__reward.reset();
	award_list.clear();
}

CheckInDetail::CheckInDetail(void)
{
	CheckInDetail::reset();
}

void CheckInDetail::reset(void)
{
	__month_day = 0;
	__award_index = 0;
	__check_in_point = 0;
	__last_time = 0;
	__show_point = 0;

	__popup = 0;
    __charge_money = -1;
    __check_total_index = 0;
    __total_last_time = 0;
}

void BlessDetail::reset(void)
{
	__buff_start = 0;
	__buff_left = 0;
	__next_reward_time = 0;
	__reward_times = 0;
	__reward_amount = 0;
}

int MapIntHourTimer::schedule_timer()
{
	int next_hour = GameCommon::next_hour();
	MSG_USER("MapIntHourTimer %d", next_hour);
	return GameTimer::schedule_timer(next_hour);
}

int MapIntHourTimer::type(void)
{
	return GTT_MAP_ONE_MINUTE;
}

int MapIntHourTimer::handle_timeout(const Time_Value &tv)
{
	int next_hour = GameCommon::next_hour();
	MSG_USER("MapIntHourTimer %d", next_hour);

	if (MAP_MONITOR->is_has_travel_scene())
	{
		TRVL_SCRIPT_MONITOR->handle_team_timeout();
	}
	else
	{

	}

	this->cancel_timer();
	this->GameTimer::schedule_timer(next_hour);

	return 0;
}

int TrvlKeepAliveTimer::type()
{
	return GTT_MAP_ONE_SECOND;
}

int TrvlKeepAliveTimer::handle_timeout(const Time_Value &tv)
{
	JUDGE_RETURN(MAP_MONITOR->is_has_travel_scene() == true, 0);
	return MAP_MONITOR->check_keep_alvie_timeout();
}

int MapMidNightTimer::schedule_timer()
{
	int next_day = GameCommon::next_day();
	MSG_USER("MapMidNightTimer %d", next_day);
	return GameTimer::schedule_timer(Time_Value(next_day));
}

int MapMidNightTimer::type(void)
{
	return GTT_MAP_ONE_MINUTE;
}

int MapMidNightTimer::handle_timeout(const Time_Value &tv)
{
	MAP_MONITOR->reset_everyday();
	PLAYER_MANAGER->reset_map_player_everyday();

	int next_day = GameCommon::next_day();
	MSG_USER("MapMidNightTimer %d", next_day);

	this->cancel_timer();
	this->GameTimer::schedule_timer(next_day);

	return 0;
}

void SkillCoolInfo::reset()
{
	this->role_ = 0;
	this->skill_cool_.clear();
}

Time_Value SkillCoolInfo::fetch_time(int skill)
{
	JUDGE_RETURN(this->skill_cool_.count(skill) > 0, Time_Value::zero);
	return this->skill_cool_[skill];
}

