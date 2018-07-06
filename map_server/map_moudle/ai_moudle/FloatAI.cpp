/*
 * FloatAI.cpp
 *
 * Created on: 2015-06-09 12:03
 *     Author: lyz
 */

#include "FloatAI.h"
#include "Scene.h"
#include "AIManager.h"
#include "MapPlayerEx.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"

FloatAI::~FloatAI(void)
{ /*NULL*/ }

void FloatAI::reset(void)
{
    this->wedding_id_ = 0;
    this->wedding_type_ = 0;
    this->partner_id_1_ = 0;
    this->partner_career_1_ = 0;
    this->partner_id_2_ = 0;
    this->partner_career_2_ = 0;
    this->float_type_ = 0;
    this->cruise_timeout_tick_ = Time_Value::zero;
    this->giftbox_set_.clear();

    GameAI::reset();
}

void FloatAI::set_float_type(void)
{
    this->float_type_ = FTYPE_FLOAT;
}

void FloatAI::set_giftbox_type(void)
{
    this->float_type_ = FTYPE_GIFTBOX;
}

bool FloatAI::is_float(void)
{
    return this->float_type_ == FTYPE_FLOAT;
}

bool FloatAI::is_giftbox(void)
{
    return this->float_type_ == FTYPE_GIFTBOX;
}

void FloatAI::set_wedding_id(const Int64 id)
{
    this->wedding_id_ = id;
}

Int64 FloatAI::wedding_id(void)
{
    return this->wedding_id_;
}

void FloatAI::set_wedding_type(const int type)
{
    this->wedding_type_ = type;
}

int FloatAI::wedding_type(void)
{
    return this->wedding_type_;
}

void FloatAI::set_partner_id_1(const Int64 role_id)
{
    this->partner_id_1_ = role_id;
}

Int64 FloatAI::partner_id_1(void)
{
    return this->partner_id_1_;
}

void FloatAI::set_partner_career_1(const int career)
{
	this->partner_career_1_ = career;
}

int FloatAI::partner_career_1(void)
{
	return this->partner_career_1_;
}

void FloatAI::set_partner_id_2(const Int64 role_id)
{
    this->partner_id_2_ = role_id;
}

Int64 FloatAI::partner_id_2(void)
{
    return this->partner_id_2_;
}

void FloatAI::set_partner_career_2(const int career)
{
	this->partner_career_2_ = career;
}

int FloatAI::partner_career_2(void)
{
	return this->partner_career_2_;
}

bool FloatAI::is_float_owner(const Int64 role_id)
{
    if (this->partner_id_1() == role_id || this->partner_id_2() == role_id)
        return true;
    return false;
}

void FloatAI::generate_gift_box(void)
{
    const Json::Value &ai_rule_json = this->fetch_prop_config();

    int giftbox_sort = ai_rule_json["giftbox_sort"].asInt(),
        gen_range = std::max(ai_rule_json["giftbox_rule"][0u].asInt(), 1),
        gen_amount = ai_rule_json["giftbox_rule"][1u].asInt();

    JUDGE_RETURN(gen_amount > 0, ;);

    Scene *scene = this->fetch_scene();
    JUDGE_RETURN(scene != NULL, ;);

    MoverCoord coord;
    for (int i = 0; i < gen_amount; ++i)
    {
        coord = scene->rand_dynamic_coord(this->location(), gen_range);
        Int64 ai_id = AIMANAGER->generate_float_ai(giftbox_sort, coord, scene);

        FloatAI *giftbox_ai = AIMANAGER->float_ai_package()->find_object(ai_id);
        if (giftbox_ai == NULL)
            continue;

        giftbox_ai->set_caller(this->ai_id());
        giftbox_ai->set_giftbox_type();
        giftbox_ai->set_wedding_id(this->wedding_id());
        giftbox_ai->set_partner_id_1(this->partner_id_1());
        giftbox_ai->set_partner_id_2(this->partner_id_2());
        giftbox_ai->set_wedding_type(this->wedding_type());
        giftbox_ai->enter_scene();

        this->giftbox_set_.insert(giftbox_ai->ai_id());
    }
}

// 检查不是所有者才可以拾取
int FloatAI::gather_limit_collect_begin(Int64 role_id)
{
    JUDGE_RETURN(this->is_float_owner(role_id) == false, ERROR_GIFTBOX_OWNER);

    MapPlayerEx *player = this->find_player(role_id);
    JUDGE_RETURN(player != NULL && player->check_wedding_giftbox_times(this->wedding_type()) == 0, ERROR_WEDDING_GIFTBOX_AMOUNT);

    return GameAI::gather_limit_collect_begin(role_id);
}

int FloatAI::gather_limit_collect_done(Int64 role_id, int result, ItemObj &gather_item)
{
	if (result == 0)
	{
		MapPlayerEx *player = this->find_player(role_id);
		if (player != NULL)
			++(player->role_detail().__wedding_giftbox_times);
	}

    return GameAI::gather_limit_collect_done(role_id, result, gather_item);
}

int FloatAI::recycle_self(void)
{
    return AIMANAGER->float_ai_package()->unbind_and_push(this->ai_id(), this);
}

int FloatAI::enter_scene(const int type)
{
    int ret = GameAI::enter_scene(type);
    if (this->is_float() && ret == 0)
    {
        const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
        int cruit_timeout_tick = wedding_json["cruise_tick"].asInt();
        if (cruit_timeout_tick <= 0)
            cruit_timeout_tick = 240;
        this->cruise_timeout_tick_ = Time_Value::gettimeofday() + Time_Value(cruit_timeout_tick);

        // 注册玩家对应的花车，以检查玩家状态
        Scene *scene = this->fetch_scene();
        if (scene != NULL)
        {
            if (this->partner_id_1() > 0)
                scene->rebind_float_ai(this->partner_id_1(), this);
            if (this->partner_id_2() > 0)
                scene->rebind_float_ai(this->partner_id_2(), this);
        }
        // 花车进入场景时修改玩家状态
        MapPlayerEx *partner = this->find_player(this->partner_id_1());
        if (partner != NULL)
            partner->enter_float_cruise_state(true);
        partner = this->find_player(this->partner_id_2());
        if (partner != NULL)
            partner->enter_float_cruise_state(true);
    }

    return ret;
}

int FloatAI::exit_scene(const int type)
{
    if (this->is_float())
    {
        // 花车退出场景时修改玩家状态
        MapPlayerEx *partner = this->find_player(this->partner_id_1());
        if (partner != NULL)
            partner->exit_float_cruise_state();
        partner = this->find_player(this->partner_id_2());
        if (partner != NULL)
            partner->exit_float_cruise_state();

        Scene *scene = this->fetch_scene();
        if (scene != NULL)
        {
            scene->unbind_float_ai(this->partner_id_1());
            scene->unbind_float_ai(this->partner_id_2());

            LongVec giftbox_list;
            for (LongSet::iterator iter = this->giftbox_set_.begin(); iter != this->giftbox_set_.end(); ++iter)
            {
            	giftbox_list.push_back(*iter);
            }
            for (LongVec::iterator iter = giftbox_list.begin(); iter != giftbox_list.end(); ++iter)
            {
                GameAI *giftbox_ai = scene->find_ai(*iter);
                if (giftbox_ai == NULL)
                	continue;

                if (giftbox_ai->caller() != this->ai_id())
                    continue;
                giftbox_ai->exit_scene();
                giftbox_ai->sign_out();
            }
        }
        this->giftbox_set_.clear();
        {
            // 通知公共服退出巡游状态
            Proto31101609 inner_req;
            inner_req.set_wedding_id(this->wedding_id());
            this->monitor()->dispatch_to_logic(&inner_req);
        }
    }
    if (this->is_giftbox())
    {
        Scene *scene = this->fetch_scene();
        if (scene != NULL)
        {
            FloatAI *float_ai = dynamic_cast<FloatAI *>(scene->find_ai(this->caller()));
            if (float_ai != NULL)
                float_ai->remove_giftbox_id(this->ai_id());
        }
    }

    return GameAI::exit_scene(type);
}

int FloatAI::schedule_move_fighter(void)
{
    if (this->cruise_timeout_tick_ <= Time_Value::gettimeofday())
    {
        // 防止花车一直不消失
        this->exit_scene();
        this->sign_out();
        return 0;
    }

    int ret = GameAI::schedule_move_fighter();
   
    // 让玩家跟随花车移动
    MapPlayerEx *partner = this->find_player(this->partner_id_1());
    if (partner != NULL)
        partner->follow_float_move_action(this->mover_detial_.__step_list);
    partner = this->find_player(this->partner_id_2());
    if (partner != NULL)
        partner->follow_float_move_action(this->mover_detial_.__step_list);

    return ret;
}

void FloatAI::remove_giftbox_id(const Int64 id)
{
    this->giftbox_set_.erase(id);
}

int FloatAI::make_up_appear_other_info(Block_Buffer *buff, const bool send_by_gate)
{
	GameAI::make_up_appear_other_info(buff, send_by_gate);

	Proto80101409 respond;
	respond.set_float_id(this->ai_id());
	respond.set_partner_id_1(this->partner_id_1());
	respond.set_partner_career_1(this->partner_career_1());
	respond.set_partner_id_2(this->partner_id_2());
	respond.set_partner_career_2(this->partner_career_2());

	ProtoClientHead head;
	head.__recogn = ACTIVE_FLOAT_PARTNER;
	return this->make_up_client_block(buff, &head, &respond);
}

