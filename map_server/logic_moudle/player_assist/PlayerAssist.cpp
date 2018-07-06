/*
 * PlayerAssist.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: louis
 */

#include "PlayerAssist.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"

PlayerAssist::PlayerAssist(void)  : ingore_show_pannel_(false)
{/*NULL*/}

PlayerAssist::~PlayerAssist(void) 
{ /*NULL*/ }

void PlayerAssist::reset_everyday()
{
	IntMap remove_map;
	for (PATipPannel::iterator iter = this->pas_tip_pannel_.begin();
			iter != this->pas_tip_pannel_.end(); ++iter)
	{
		const Json::Value& conf = CONFIG_INSTANCE->red_tips(iter->first);
		JUDGE_CONTINUE(conf["check"].asInt() == 1);
		remove_map[iter->first] = true;
	}

	for (IntMap::iterator iter = remove_map.begin();
			iter != remove_map.end(); ++iter)
	{
		this->update_player_assist_single_event(iter->first, 0);
	}
}

void PlayerAssist::reset_player_assist(void)
{
    this->pas_tip_pannel_.clear();
    this->ingore_show_pannel_ = false;
}

PlayerAssist::PATipPannel &PlayerAssist::assist_tip_pannel(void)
{
    return this->pas_tip_pannel_;
}

bool PlayerAssist::is_ingore_show_pannel(void)
{
    return this->ingore_show_pannel_;
}

void PlayerAssist::set_ingore_show_pannel(const bool flag)
{
    this->ingore_show_pannel_ = flag;
}

int PlayerAssist::cancel_player_assist(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11401701*, request, -1);

    this->pas_tip_pannel_.erase(request->event_id());
    FINER_PROCESS_NOTIFY(RETURN_CANCEL_RED_POINT);
}

int PlayerAssist::fetch_player_assist_pannel(Message* msg)
{
	Proto51401702 respond;
	for (PATipPannel::iterator iter = this->pas_tip_pannel_.begin();
			iter != this->pas_tip_pannel_.end(); ++iter)
	{
		ProtoPairObj* proto = respond.add_event_list();
		iter->second.serilize(proto);
	}

//	MapLogicPlayer *player = dynamic_cast<MapLogicPlayer *>(this);
//	player->check_label_pa_event();

	FINER_PROCESS_RETURN(RETURN_FETCH_PLAYER_ASSIST_PANNEL, &respond);
}

int PlayerAssist::fetch_player_assist_tips(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11401703*, request, RETURN_FETCH_PLAYER_ASSIST_TIPS);

    int event_id = request->event_id();
    switch (event_id)
    {
        case GameEnum::PLAYER_ASSIST_EVENT_EXPIRE_FASHION:
        case GameEnum::PLAYER_ASSIST_EVENT_LIMIT_FASHION:
        case GameEnum::PLAYER_ASSIST_EVENT_VIP_EXPIRE:
        case GameEnum::PA_EVENT_49_VIP:
        {
            this->update_player_assist_single_event(event_id, 0);
            break;
        }
        default:
        {
            this->notify_player_assist_event_disappear(event_id);
            break;
        }
    }
    
    FINER_PROCESS_NOTIFY(RETURN_FETCH_PLAYER_ASSIST_TIPS);
}

int PlayerAssist::request_ingore_show_pannel(Message* msg)
{
	this->ingore_show_pannel_ = true;
	FINER_PROCESS_NOTIFY(RETURN_REQUEST_INGORE_SHOW_PANNEL);
}

int PlayerAssist::process_inner_player_assist_tips_event(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401710*, req, -1);
	return this->update_player_assist_single_event(req->event_id(), req->event_value());
}

int PlayerAssist::update_player_assist_single_event(int event_id, int event_value)
{
	JUDGE_RETURN(event_id > 0, -1);

    if (event_value == 0)
    {
    	JUDGE_RETURN(this->pas_tip_pannel_.count(event_id) > 0, 0);

        this->pas_tip_pannel_.erase(event_id);
        this->notify_player_assist_event_disappear(event_id);
    }
    else if (this->pas_tip_pannel_.count(event_id) > 0)
    {
    	PlayerAssistTip &assist_tip = this->pas_tip_pannel_[event_id];
    	JUDGE_RETURN(assist_tip.__tips_flag != event_value, 0);

        assist_tip.__tips_flag = event_value;
    	this->notify_player_assist_event_appear(event_id);
    }
    else
    {
    	PlayerAssistTip &assist_tip = this->pas_tip_pannel_[event_id];
        assist_tip.__event_id = event_id;
        assist_tip.__tips_flag = event_value;
    	this->notify_player_assist_event_appear(event_id);
    }

    return 0;
}

//sync info when tranfer scene
int PlayerAssist::sync_transfer_tip_pannel(int scene_id)
{
	Proto31400125 request;
	request.set_ingore_flag(this->ingore_show_pannel_);

	for(PATipPannel::iterator iter = this->pas_tip_pannel_.begin();
            iter != this->pas_tip_pannel_.end(); ++iter)
	{
		ProtoPlayerTipSyncInfo* proto_info = request.add_tip_list();
		proto_info->set_event_id(iter->second.__event_id);
		proto_info->set_arena_reward(iter->second.__tips_flag);
	}

	return this->send_to_other_logic_thread(scene_id, request);
}

int PlayerAssist::read_transfer_tip_pannel(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400125*, request, -1);

	this->ingore_show_pannel_ = request->ingore_flag();
	for(int i = 0; i < request->tip_list_size(); ++i)
	{
		const ProtoPlayerTipSyncInfo* proto_info = request->mutable_tip_list(i);
		PlayerAssistTip&tip = this->pas_tip_pannel_[proto_info->event_id()];
		tip.__event_id = proto_info->event_id();
		tip.__tips_flag = proto_info->arena_reward();
	}

	return 0;
}

int PlayerAssist::notify_player_assist_event_appear(int event_id)
{
    JUDGE_RETURN(this->ingore_show_pannel_ == false, 0);

    PlayerAssistTip &assist_tip = this->pas_tip_pannel_[event_id];
    this->notify_red_point(assist_tip.__event_id, assist_tip.__tips_flag);

    return 0;
}

int PlayerAssist::notify_player_assist_event_disappear(int event_id)
{
    Proto81401705 respond;
    respond.set_even_id(event_id);
    FINER_PROCESS_RETURN(ACTIVE_PLAYER_ASSIST_DISAPPEAR, &respond);
}

int PlayerAssist::update_new_mail_amount_event(void)
{
    MapLogicPlayer *player = this->map_logic_player();

    int new_mail_amount = player->fetch_new_mail_amount();
    return this->update_player_assist_single_event(GameEnum::PA_EVENT_MAIL, new_mail_amount);
}

void PlayerAssist::check_pa_event_game_resource(int id, int value, const SerialObj& serial_obj)
{
	int tips_id = CONFIG_INSTANCE->item(id)["red_point_event"].asInt();
	JUDGE_RETURN(tips_id > 0, ;);

	int tips_value = 0;
	if (value >= CONFIG_INSTANCE->red_tips(tips_id)["sub1"].asInt())
	{
		tips_value = 1;
	}

	this->update_player_assist_single_event(tips_id, tips_value);
}

void PlayerAssist::check_pa_event_when_savvy_update(void)
{
//    MapLogicPlayer *player = this->map_logic_player();
//    Proto31401711 inner_req;
//    inner_req.set_savvy(player->role_ex_detail().__savvy);
//    player->own_money().serialize(inner_req.mutable_money());
//    inner_req.add_check_events(GameEnum::PA_EVENT_SKILL_LVLUP);
//
//    this->send_to_map_thread(inner_req);
}

void PlayerAssist::check_pa_event_role_pass_skill_a(int skill_id, int skill_level)
{
    MapLogicPlayer* player = this->map_logic_player();
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);

    ItemObj obj = GameCommon::make_up_itemobj(skill_json["upgrade_goods"][0u]);
    int tips_id = CONFIG_INSTANCE->item(obj.id_)["red_point_event"].asInt();
    JUDGE_RETURN(tips_id > 0, ;);

    int ret = player->skill_upgrade_use_goods(skill_id, skill_level, true);
    this->update_player_assist_single_event(tips_id, ret == 0);
}

void PlayerAssist::check_pa_event_role_pass_skill(int item_id, int tips_id)
{
	int pack_count = this->pack_count(item_id);
	if (pack_count <= 0)
	{
		this->update_player_assist_single_event(tips_id, 0);
	}
	else
	{
		Proto30400152 inner;
		inner.set_item_id(item_id);
		inner.set_tips_id(tips_id);
		inner.set_item_amount(pack_count);
		this->send_to_map_thread(inner);
	}
}

void PlayerAssist::check_pa_event_when_item_update(int item_id, int item_amount)
{
	int tips_id = CONFIG_INSTANCE->item(item_id)["red_point_event"].asInt();
	JUDGE_RETURN(tips_id > 0, ;);

	const Json::Value& conf = CONFIG_INSTANCE->red_tips(tips_id);
	JUDGE_RETURN(conf.empty() == false, ;);

	MapLogicPlayer *player = this->map_logic_player();
	switch (conf["type"].asInt())
	{
	case 1:	//进阶
	{
		player->check_pa_event_mount_evoluate(conf["sub1"].asInt());
		break;
	}
	case 2:	//资质
	{
		player->check_pa_event_mount_ability(conf["sub1"].asInt(), item_id);
		break;
	}
	case 3:	//成长
	{
		player->check_pa_event_mount_growth(conf["sub1"].asInt(), item_id);
		break;
	}
	case 4: //技能
	{
		player->check_pa_event_mount_skill(conf["sub1"].asInt());
		break;
	}
	case 5:	//装备
	{
		player->check_pa_event_mount_equip(conf["sub1"].asInt(), item_id);
		break;
	}
	case 6:	//强化,红装
	{
		JUDGE_BREAK(player->check_finish_task(conf["sub2"].asInt()) == true);
		player->check_pa_event_equip_strengthen();
		player->check_pa_event_equip_red_uprising();
		break;
	}
	case 7:	//宝石
	{
		JUDGE_BREAK(player->check_finish_task(conf["sub2"].asInt()) == true);

		//镶嵌
		player->check_pa_event_equip_insert_jewel(item_id);
		//合成
		player->check_pa_event_jewel_combine(item_id);
		break;
	}
	case 8:	//玩家被动技能
	{
		player->check_pa_event_role_pass_skill(item_id, tips_id);
		break;
	}
	case 9: //精炼
	{
		player->check_pa_event_equip_good_refine(item_id);
		break;
	}
	case 10: //红装
	{
		player->check_pa_event_equip_red_uprising();
		break;
	}
	case 11: //天天跑酷物品
	{
		player->check_pa_event_daily_run_item(tips_id, 1);
		break;
	}
	}
}

void PlayerAssist::check_pa_event_when_level_up(void)
{
	this->send_to_map_thread(INNER_UPDATE_SCRIPT_RED_POINT);
}

void PlayerAssist::check_pa_event_when_transfer(void)
{
//    MapLogicPlayer *player = this->map_logic_player();
//    player->check_pa_event_script_times();
}

void PlayerAssist::check_pa_event_daily_run_item(int tips_id, int value)
{
	MapLogicPlayer *player = this->map_logic_player();
	if (player->logic_validate_big_act_time())
	{
		this->update_player_assist_single_event(tips_id, value);
	}
	else
	{
		this->update_player_assist_single_event(tips_id, 0);
	}
}

