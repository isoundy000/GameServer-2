/*
 * EffectAI.cpp
 *
 * Created on: 2014-02-12 17:55
 *     Author: lyz
 */

#include "EffectAI.h"
#include "ProtoDefine.h"
#include "AIManager.h"
#include "Scene.h"
#include "MapMonitor.h"

EffectAI::EffectAI(void) : effect_sort_(0), safe_point_effect_sort_(0), effect_monster_type_(0)
{
	this->ai_id_ = this->ai_id_ - BASE_OFFSET_MONSTER + BASE_OFFSET_EFFECT_MONSTER;
}

EffectAI::~EffectAI(void)
{ /*NULL*/ }

void EffectAI::reset(void)
{
    this->effect_sort_ = 0;
    this->safe_point_effect_sort_ = 0;
    this->effect_monster_type_ = 0;
    GameAI::reset();
}

bool EffectAI::is_area_hurt_effect(void)
{
    return this->effect_monster_type_ == EMT_AREA_HURT_EFFECT;
}

bool EffectAI::is_safe_area_effect(void)
{
    return this->effect_monster_type_ == EMT_SAFE_AREA_EFFECT;
}

void EffectAI::set_effect_sort(const int effect_sort)
{
    this->effect_sort_ = effect_sort;
}

int EffectAI::effect_sort(void)
{
    return this->effect_sort_;
}

int EffectAI::make_up_appear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
    JUDGE_RETURN(this->effect_sort() > 0, this->effect_sort());

    Proto80400106 appear_info;
    appear_info.set_effect_id(this->ai_id());
    appear_info.set_effect_sort(this->effect_sort());
    appear_info.set_pixel_x(this->location().pixel_x());
    appear_info.set_pixel_y(this->location().pixel_y());

    ProtoClientHead head;
    head.__recogn = ACTIVE_EFFECT_APPEAR;

    //    MSG_DEBUG("Proto80400106: %s", appear_info.Utf8DebugString().c_str());
    return this->make_up_client_block(buff, &head, &appear_info);
}

int EffectAI::make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate)
{
	GameAI::make_up_appear_other_info(buff, send_by_gate);

    ProtoClientHead head;
    head.__recogn = ACTIVE_EFFECT_APPEAR;

    // safe points appear
    {
        int point_id_n = this->ai_detail_.__safe_point_ids.size();
        int point_n = this->ai_detail_.__safe_points.size();

        point_n = std::max(point_n, point_id_n);

        for(int i=0; i< point_n; ++i)
        {
            int point_id = this->ai_detail_.__safe_point_ids[i];
            int point_pixel_x = this->ai_detail_.__safe_points[i].pixel_x();
            int point_pixel_y = this->ai_detail_.__safe_points[i].pixel_y();


            Proto80400106 save_point_appear_info;
            save_point_appear_info.set_effect_id(point_id);
            save_point_appear_info.set_effect_sort(this->safe_point_effect_sort_);
            save_point_appear_info.set_pixel_x(point_pixel_x);
            save_point_appear_info.set_pixel_y(point_pixel_y);

            this->make_up_client_block(buff, &head, &save_point_appear_info);
        }
    }
    return 0;
}

int EffectAI::make_up_disappear_info_base(Block_Buffer *buff, const bool send_by_gate)
{
    JUDGE_RETURN(this->effect_sort() > 0, this->effect_sort());

    Proto80400107 respond;
    respond.set_effect_id(this->ai_id());
    respond.set_effect_sort(this->effect_sort());

	ProtoClientHead head;
	head.__recogn = ACTIVE_EFFECT_DISAPPEAR;
	return this->make_up_client_block(buff, &head, &respond);
}

int EffectAI::make_up_disappear_other_info(Block_Buffer *buff, const bool send_by_gate)
{
	GameAI::make_up_disappear_other_info(buff, send_by_gate);

	// safe points disappear
	{
		int point_id_n = this->ai_detail_.__safe_point_ids.size();
		for(int i=0; i< point_id_n; ++i)
		{
			int point_id = this->ai_detail_.__safe_point_ids[i];

			Proto80400107 save_point_disappear_info;
			save_point_disappear_info.set_effect_id(point_id);
			save_point_disappear_info.set_effect_sort(this->safe_point_effect_sort_);

			ProtoClientHead head;
			head.__recogn = ACTIVE_EFFECT_DISAPPEAR;
			this->make_up_client_block(buff, &head, &save_point_disappear_info);
		}
	}
	return 0;
}

int EffectAI::recycle_self(void)
{
    return AIMANAGER->effect_ai_package()->unbind_and_push(this->ai_id(), this);
}

void EffectAI::init_base_property(void)
{
    GameAI::init_base_property();

    this->effect_monster_type_ = EMT_AREA_HURT_EFFECT;
    const Json::Value &prop_monster_json = CONFIG_INSTANCE->prop_monster(this->ai_sort());
    if (prop_monster_json.isMember("monster_type"))
    {
        const std::string &monster_type_str = prop_monster_json["monster_type"].asString();
        if (monster_type_str == "safe_area")
            this->effect_monster_type_ = EMT_SAFE_AREA_EFFECT;
    }
}

void EffectAI::init_safe_area(const Json::Value &prop_monster)
{
//    if (prop_monster.isMember("safe_radius"))
//    {
//        this->ai_detail_.__safe_radius_pixel = prop_monster["safe_radius"].asInt();
//        this->ai_detail_.__safe_radius = MoverCoord::pixel_to_pos(this->ai_detail_.__safe_radius_pixel);
//    }
//
//    int safe_type = prop_monster["safe_type"].asInt(),
//        safe_gen_radius = MoverCoord::pos_to_pixel(prop_monster["safe_area"][0u].asInt()),
//        safe_size = prop_monster["safe_area"][1u].asInt();
//
//    JUDGE_RETURN(safe_type > 0, ;);
//
//    this->ai_detail_.__safe_radius = prop_monster["safe_area"][2u].asInt();
//    this->ai_detail_.__safe_radius_pixel = MoverCoord::pos_to_pixel(this->ai_detail_.__safe_radius);
//    this->safe_point_effect_sort_ = prop_monster["safe_point_effect_sort"].asInt();
//
//    Proto80400907 res;
//    res.set_wait_tick(prop_monster["ready_safe_tick"].asInt());
//    res.set_effect_sort( this->safe_point_effect_sort_);
//    res.set_safe_radius(this->ai_detail_.__safe_radius);
//
//    CoordVec coord_set;
//    if (safe_type == GameEnum::AI_SAT_DEFENDER)
//    {
//        Scene *scene = this->fetch_scene();
//        JUDGE_RETURN(scene != 0, ;);
//
//        int pixel_x = 0, pixel_y = 0;
//        double angle = 0;
//        MoverCoord mover_coord, src_coord = this->location();
//        for (Scene::MoverMap::iterator iter = scene->player_map().begin();
//                iter != scene->player_map().end(); ++iter)
//        {
//            GameMover *mover = iter->second;
//            src_coord = mover->location();
//            for (int i = 0; i < safe_size; ++i)
//            {
//                int j = 0;
//                do {
//                    angle = (rand() % 360) * PI / 180;
//                    pixel_x = src_coord.pixel_x() + ::sin(angle) * safe_gen_radius;
//                    pixel_y = src_coord.pixel_y() + ::cos(angle) * safe_gen_radius;
//                    mover_coord.set_pixel(pixel_x, pixel_y);
//
//                    if (this->is_movable_coord(mover_coord) == true &&
//                            this->validate_point_not_overlap(coord_set, mover_coord,
//                                this->ai_detail_.__safe_radius) == true)
//                        break;
//                    ++j;
//                } while (j < 100);
//
//                if (j >= 100)
//                    continue;
//
//                int64_t safe_point_id = MAP_MONITOR->generate_ai_id();
//                this->ai_detail_.__safe_point_ids.push_back(safe_point_id);
//                res.set_effect_id(safe_point_id);
//
//                coord_set.insert(mover_coord);
//                this->ai_detail_.__safe_points.push_back(mover_coord);
//
//                ProtoCoord *proto_coord = res.mutable_safe_point();
//                proto_coord->set_pixel_x(mover_coord.pixel_x());
//                proto_coord->set_pixel_y(mover_coord.pixel_y());
//            }
//
//            //            int full_scene = 1;
//            //            this->respond_to_broad_area(&res, full_scene);
//        }
//    }
//    else if (safe_type == GameEnum::AI_SAT_ATTACK)
//    {
//        MoverCoord mover_coord, src_coord = this->location();
//        int pixel_x = 0, pixel_y = 0;
//        for (int i = 0; i < safe_size; ++i)
//        {
//            int j = 0;
//            do {
//                pixel_x = src_coord.pixel_x() - safe_gen_radius + rand() % safe_gen_radius;
//                pixel_y = src_coord.pixel_y() - safe_gen_radius + rand() % safe_gen_radius;
//                mover_coord.set_pixel(pixel_x, pixel_y);
//
//                if (this->is_movable_coord(mover_coord) == true &&
//                        this->validate_point_not_overlap(coord_set, mover_coord,
//                            this->ai_detail_.__safe_radius) == true)
//                    break;
//                ++j;
//            } while (j < 100);
//
//            if (j >= 100)
//                continue;
//
//            int64_t safe_point_id = MAP_MONITOR->generate_ai_id();
//            this->ai_detail_.__safe_point_ids.push_back(safe_point_id);
//            res.set_effect_id(safe_point_id);
//
//            coord_set.insert(mover_coord);
//            this->ai_detail_.__safe_points.push_back(mover_coord);
//
//            ProtoCoord *proto_coord = res.mutable_safe_point();
//            proto_coord->set_pixel_x(mover_coord.pixel_x());
//            proto_coord->set_pixel_y(mover_coord.pixel_y());
//
//        }
//
//        //        int full_scene = 1;
//        //        this->respond_to_broad_area(&res, full_scene);
//    }
//    else
//        return;

}

int EffectAI::validate_safe_area(GameFighter *defender)
{
    Scene *scene = this->fetch_scene();
    if (scene != NULL && scene->validate_in_safe_area(defender) == 0)
        return ERROR_IN_SAFE_AREA;


    for (AIDetail::SafePointList::iterator iter = this->ai_detail_.__safe_points.begin();
            iter != this->ai_detail_.__safe_points.end(); ++iter)
    {
        MoverCoord &point = *iter;
        //if (coord_offset_grid(point, defender->location()) <= this->ai_detail_.__safe_radius)
        if(check_coord_distance(point, defender->location(),
                    this->ai_detail_.__safe_radius, GameEnum::CLIENT_MAPPING_FACTOR))
            return ERROR_IN_SAFE_AREA;
    }
    return 0;
}

bool EffectAI::validate_point_not_overlap(CoordVec &coord_set, MoverCoord &move_coord, int radius)
{
    for(CoordVec::iterator iter = coord_set.begin();
            iter != coord_set.end(); ++iter)
    {
        // 如果距离小于 radius 返回 false
        if(check_coord_distance(*iter, move_coord, radius, GameEnum::CLIENT_MAPPING_FACTOR))
            return false;
    }

    return true;
}
