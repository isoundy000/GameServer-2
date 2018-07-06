/*
 * MapSkiller.cpp
 *
 * Created on: 2013-12-05 10:31
 *     Author: lyz
 */

#include "MapSkiller.h"
#include "PubStruct.h"
#include "MapPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"


MapSkiller::MapSkiller(void)
{
}

MapSkiller::~MapSkiller(void)
{ /*NULL*/ }

MapPlayer *MapSkiller::skill_player(void)
{
    return dynamic_cast<MapPlayer *>(this);
}

void MapSkiller::reset_map_skiller(void)
{
    this->skiller_detail_.current_scheme_ = 1;
    this->skiller_detail_.rama_skill_ = 0;
    this->skiller_detail_.scheme_list_.clear();
    this->skiller_detail_.rama_skill_list_.clear();
    this->skiller_detail_.scheme_list_.reserve(MAX_SKILL_SCHEME);
    this->adjust_map_skiller();
}

void MapSkiller::adjust_map_skiller(void)
{
	this->skiller_detail_.current_scheme_ = std::max(1, this->skiller_detail_.current_scheme_);

	SkillerDetail::SkillIdList null_list(MAX_SCHEME_SKILL_SIZE, 0);
    for (int i = this->skiller_detail_.scheme_list_.size(); i < MAX_SKILL_SCHEME; ++i)
    {
        this->skiller_detail_.scheme_list_.push_back(null_list);
    }

    if (this->skiller_detail_.scheme_list_.size() > MAX_SKILL_SCHEME)
    {
    	this->skiller_detail_.scheme_list_.resize(MAX_SKILL_SCHEME);
    }

    for (SkillerDetail::SchemeList::iterator iter = this->skiller_detail_.scheme_list_.begin();
    		iter != this->skiller_detail_.scheme_list_.end(); ++iter)
    {
    	JUDGE_CONTINUE(iter->size() != MAX_SCHEME_SKILL_SIZE);
    	iter->resize(MAX_SCHEME_SKILL_SIZE);
    }
}

int MapSkiller::fetch_skill_list(Message *proto)
{
	this->fetch_noraml_skill_list();
	this->fetch_rama_skill_list();
	return 0;
}

int MapSkiller::fetch_noraml_skill_list()
{
    MapPlayer *player = this->skill_player();
    SkillMap &skill_map = player->fight_detail().__skill_map;

    Proto50400701 respond;
    respond.set_part(0);
    respond.set_scheme(this->current_scheme());

    for (SkillMap::iterator iter = skill_map.begin(); iter != skill_map.end(); ++iter)
    {
    	FighterSkill* skill = iter->second;
        JUDGE_CONTINUE(skill != NULL);

        ProtoSkill* proto_skill = respond.add_skill_list();
        skill->serialize(proto_skill, true);
    }

    SkillerDetail::SkillIdList &id_list = this->fetch_cur_scheme_skill();
    for (SkillerDetail::SkillIdList::iterator iter = id_list.begin();
    		iter != id_list.end(); ++iter)
    {
    	respond.add_scheme_skill_list(*iter);
    }

    FINER_PLAYER_PROCESS_RETURN(RETURN_FETCH_SKILL_LIST, &respond);
}

int MapSkiller::fetch_rama_skill_list()
{
	Proto50400701 respond;
	respond.set_part(1);
	respond.set_rama_skill(this->skiller_detail_.rama_skill_);

	for (IntMap::iterator iter = this->skiller_detail_.rama_skill_list_.begin();
			iter != this->skiller_detail_.rama_skill_list_.end(); ++iter)
	{
		respond.add_rama_skill_list(iter->second);
	}

	MapPlayer *player = this->skill_player();
	FINER_PLAYER_PROCESS_RETURN(RETURN_FETCH_SKILL_LIST, &respond);
}

int MapSkiller::fetch_transfer_skill_list()
{
	MapPlayer *player = this->skill_player();

	SkillMap *skill_map = player->fetch_transfer_skill();
	JUDGE_RETURN(skill_map != NULL, 0);

	Proto50400701 respond;
	respond.set_part(2);

	for (SkillMap::iterator iter = skill_map->begin();
			iter != skill_map->end(); ++iter)
	{
	    FighterSkill* skill = iter->second;
	    JUDGE_CONTINUE(skill != NULL);

	    ProtoSkill* proto_skill = respond.add_skill_list();
	    skill->serialize(proto_skill, true);
	}

	FINER_PLAYER_PROCESS_RETURN(RETURN_FETCH_SKILL_LIST, &respond);
}

int MapSkiller::fetch_transfer_skill_list(const IntMap& skill_map)
{
	Proto50400701 respond;
	respond.set_part(2);

	for (IntMap::const_iterator iter = skill_map.begin();
			iter != skill_map.end(); ++iter)
	{
	    ProtoSkill* proto_skill = respond.add_skill_list();
	    proto_skill->set_skill_id(iter->first);
	    proto_skill->set_skill_level(iter->second);
	}

	MapPlayer *player = this->skill_player();
	FINER_PLAYER_PROCESS_RETURN(RETURN_FETCH_SKILL_LIST, &respond);
}

int MapSkiller::fetch_skill_scheme(Message *proto)
{
    DYNAMIC_CAST_RETURN(Proto10400702 *, request, proto, -1);

    int scheme = request->scheme();
    MapPlayer *player = this->skill_player();
    CONDITION_PLAYER_NOTIFY_RETURN(0 < scheme && scheme <= MAX_SKILL_SCHEME,
    		RETURN_FETCH_SKILL_SCHEME, ERROR_CLIENT_OPERATE);

    int prev_scheme = this->skiller_detail_.current_scheme_;
    this->skiller_detail_.current_scheme_ = scheme;

    if (this->skiller_detail_.current_scheme_ != prev_scheme)
    {
        this->fetch_skill_shortcut();
    }

    Proto50400702 respond;
    respond.set_scheme(scheme);

    SkillerDetail::SkillIdList &id_list = this->fetch_cur_scheme_skill();
    for (SkillerDetail::SkillIdList::iterator iter = id_list.begin();
    		iter != id_list.end(); ++iter)
    {
    	respond.add_skill_list(*iter);
    }

    FINER_PLAYER_PROCESS_RETURN(RETURN_FETCH_SKILL_SCHEME, &respond);
}

int MapSkiller::update_skill_scheme(Message *msg)
{
    MapPlayer *player = this->skill_player();

    MSG_DYNAMIC_CAST_RETURN(Proto10400704 *, request, -1);
    int scheme = request->scheme(), skill_index = request->index(), skill_id = request->skill_id(); 
   
    int ret = this->insert_skill_shortcut(scheme, skill_index - 1, skill_id);
    if (ret != 0)
    {
    	return player->respond_to_client_error(RETURN_UPDATE_SKILL_SCHEME, ret);
    }

    this->fetch_skill_shortcut();
    FINER_PLAYER_PROCESS_NOTIFY(RETURN_UPDATE_SKILL_SCHEME);
}

int MapSkiller::check_passive_skill_pa_event(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400152 *, request, -1);

	int tips_value = this->check_passive_skill_levelup(request->tips_id(),
			request->item_amount());
	return MAP_MONITOR->inner_notify_player_assist_event(this->skill_player(),
			request->tips_id(), tips_value);
}

int MapSkiller::insert_rama_skill(int skill_id, int update)
{
	JUDGE_RETURN(this->skiller_detail_.rama_skill_ != skill_id, -1);
	JUDGE_RETURN(GameCommon::fetch_skill_id_fun_type(skill_id) == GameEnum::SKILL_FUN_RAMA, -1);

	const Json::Value& conf = CONFIG_INSTANCE->skill(skill_id);
	JUDGE_RETURN(conf.empty() == false, -1);

	int skill_type = conf["skill_type"].asInt();
	this->skiller_detail_.rama_skill_list_[skill_type] = skill_id;

	MapPlayer* player = this->skill_player();
	const Json::Value& cur_conf = CONFIG_INSTANCE->skill(this->skiller_detail_.rama_skill_);
	if (this->skiller_detail_.rama_skill_ == 0 || cur_conf.empty() == true)
	{
		//未设置
		this->skiller_detail_.rama_skill_ = skill_id;
		player->insert_skill(skill_id);
	}
	else if (update == true)
	{
		//客户端切换
		player->remove_skill(this->skiller_detail_.rama_skill_);
		this->skiller_detail_.rama_skill_ = skill_id;
		player->insert_skill(skill_id);
	}
	else if (cur_conf["skill_type"].asInt() == skill_type)
	{
		//获取更好技能
		JUDGE_RETURN(skill_id > this->skiller_detail_.rama_skill_, -1);
		player->remove_skill(this->skiller_detail_.rama_skill_);
		this->skiller_detail_.rama_skill_ = skill_id;
		player->insert_skill(skill_id);
	}
	else
	{
		//获取新技能
		this->skiller_detail_.rama_skill_list_[skill_type] = skill_id;
	}

	return this->fetch_rama_skill_list();
}

int MapSkiller::insert_skill_shortcut(const int scheme, const int index, const int skill_id)
{
    return 0;
}

int MapSkiller::sync_update_skill(Message *msg)
{
    MapPlayer *player = this->skill_player();
    MSG_DYNAMIC_CAST_RETURN(Proto30400017*, request, -1);

    ProtoSkill *proto_skill = request->mutable_skill();
    JUDGE_RETURN(proto_skill->skill_id() > 0 && proto_skill->skill_level() > 0, -1);

    if (proto_skill->skill_level() > 1)
    {
		player->record_other_serial(MAIN_PLAYER_SKILL_SERIAL,
				proto_skill->skill_id(), proto_skill->skill_level());
    }

    int type = GameCommon::fetch_skill_id_fun_type(proto_skill->skill_id());
    if (type == GameEnum::SKILL_FUN_RAMA)
    {
    	this->insert_rama_skill(proto_skill->skill_id());
    }
    else if (type == GameEnum::SKILL_FUN_TRANSFER)
    {
    	player->insert_skill(proto_skill->skill_id(), proto_skill->skill_level());
        player->init_skill_buff();
    }
    else
    {
        player->insert_skill(proto_skill->skill_id(), proto_skill->skill_level());
        player->init_skill_property();
        player->update_fight_property(BasicElement::PLAYER_SKILL);
        player->cache_tick().update_cache(MapPlayer::CACHE_SKILL);
    }

    return 0;
}

int MapSkiller::exchange_skill_shortcut(Message *msg)
{
	MapPlayer *player = this->skill_player();
	MSG_DYNAMIC_CAST_RETURN(Proto10400707 *, request, -1);
    
	int index_1 = request->index_1();
	int index_2 = request->index_2();

    CONDITION_PLAYER_NOTIFY_RETURN(1 < index_1 && index_1 <= MAX_SCHEME_SKILL_SIZE,
    		RETURN_EXCHANGE_SKILL_SHOTCUT, ERROR_CLIENT_OPERATE);
    CONDITION_PLAYER_NOTIFY_RETURN(1 < index_2 && index_2 <= MAX_SCHEME_SKILL_SIZE,
    		RETURN_EXCHANGE_SKILL_SHOTCUT, ERROR_CLIENT_OPERATE);

    SkillerDetail::SkillIdList &list = this->fetch_cur_scheme_skill();
    std::swap(list[index_1 - 1], list[index_2 - 1]);

    this->fetch_skill_shortcut();
    FINER_PLAYER_PROCESS_NOTIFY(RETURN_EXCHANGE_SKILL_SHOTCUT);
}

int MapSkiller::set_current_rama(Message* msg)
{
	MapPlayer *player = this->skill_player();
	MSG_DYNAMIC_CAST_RETURN(Proto10400705*, request, -1);

	int skill_id = request->skill_id();
	this->insert_rama_skill(skill_id, true);

	Proto50400705 respond;
	respond.set_skill_id(skill_id);
	FINER_PLAYER_PROCESS_RETURN(RETURN_SET_CUR_RAMA, &respond);
}

int MapSkiller::set_current_scheme(const int scheme)
{
    this->skiller_detail_.current_scheme_ = scheme;
    return scheme;
}

int MapSkiller::current_scheme(void)
{
    return this->skiller_detail_.current_scheme_;
}

SkillerDetail& MapSkiller::skiller_detail()
{
	return this->skiller_detail_;
}

SkillerDetail::SchemeList& MapSkiller::scheme_list(void)
{
    return this->skiller_detail_.scheme_list_;
}

int MapSkiller::fetch_all_skill_scheme()
{
	Proto50400703 scheme_info;
	scheme_info.set_cur_scheme(this->skiller_detail_.current_scheme_);

	int cur_index = 0;
	for (SkillerDetail::SchemeList::iterator scheme_iter = this->skiller_detail_.scheme_list_.begin();
			scheme_iter != this->skiller_detail_.scheme_list_.end(); ++scheme_iter)
	{
		SkillerDetail::SkillIdList& id_list = *scheme_iter;

		for (SkillerDetail::SkillIdList::iterator id_iter = id_list.begin();
				id_iter != id_list.end(); ++id_iter)
		{
			if (cur_index == 0)
			{
				scheme_info.add_scheme_1(*id_iter);
			}
			else if (cur_index == 1)
			{
				scheme_info.add_scheme_2(*id_iter);
			}
			else
			{
				scheme_info.add_scheme_3(*id_iter);
			}
		}

		cur_index += 1;
	}

	return this->skill_player()->respond_to_client(
			RETURN_FETCH_ALL_SKILL_SCHEME, &scheme_info);
}

int MapSkiller::fetch_skill_shortcut(void)
{
//	Proto50400706 req;
//
//    SkillIdList &id_list = this->scheme_list_[this->current_scheme() - 1];
//    for (SkillIdList::iterator iter = id_list.begin();
//    		iter != id_list.end(); ++iter)
//    {
//    	req.add_skill_list(*iter);
//    }
//
//	return this->skill_player()->respond_to_client(RETURN_SKILL_SHOTCUT, &req);
	return 0;
}

int MapSkiller::fetch_skill_special_force()
{
	int total_force = 0;
	SkillMap& skill_map = this->skill_player()->fight_detail().__skill_map;

	for(SkillMap::iterator it = skill_map.begin(); it != skill_map.end(); ++it)
	{
		FighterSkill* skill = it->second;
		JUDGE_CONTINUE(it->second != NULL);
		total_force += skill->__server_force;
	}

	return total_force;
}

int MapSkiller::check_passive_skill_levelup(int tips_id, int have_amount)
{
	const Json::Value& conf = CONFIG_INSTANCE->red_tips(tips_id);
	JUDGE_RETURN(conf.empty() == false, false);

	FighterSkill* skill = NULL;
	JUDGE_RETURN(this->skill_player()->find_skill(conf["sub1"].asInt(), skill) == 0, false);

	uint skill_level = skill->__level + 1;
    const Json::Value &skill_json = skill->conf();
    JUDGE_RETURN(skill_level < skill_json["upgrade_goods"].size(), false);

    ItemObj obj = GameCommon::make_up_itemobj(skill_json["upgrade_goods"][skill_level - 1]);
    JUDGE_RETURN(obj.validate() == true, false);

    return have_amount >= obj.amount_;
}

SkillerDetail::SkillIdList& MapSkiller::fetch_cur_scheme_skill()
{
	if (this->skiller_detail_.current_scheme_ > int(this->skiller_detail_.scheme_list_.size()))
	{
		return this->skiller_detail_.nulll_scheme_;
	}
	return this->skiller_detail_.scheme_list_[this->skiller_detail_.current_scheme_ - 1];
}

int MapSkiller::request_skill_level_up(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto10400711 *, request, -1);

    MapPlayer *player = this->skill_player();
    JUDGE_RETURN(player->add_validate_operate_tick() == true, 0);

    int skill_id = request->skill_id();
    CONDITION_PLAYER_NOTIFY_RETURN(skill_id > 0, RETURN_SKILL_LEVEL_UP,
    		ERROR_CLIENT_OPERATE);

    const Json::Value &skill_conf = CONFIG_INSTANCE->skill(skill_id);
    CONDITION_PLAYER_NOTIFY_RETURN(skill_conf.empty() == false,
    		RETURN_SKILL_LEVEL_UP, ERROR_SKILL_NOT_EXISTS);

    Proto31402204 inner_req;
    inner_req.set_skill_id(skill_id);

    int fun_type = GameCommon::fetch_skill_id_fun_type(skill_id);
    switch (fun_type)
    {
    case GameEnum::SKILL_FUN_PASSIVE:
    {
        FighterSkill *skill = 0;
        CONDITION_PLAYER_NOTIFY_RETURN(player->find_skill(skill_id, skill) == 0,
        		RETURN_SKILL_LEVEL_UP, ERROR_SKILL_NOT_EXISTS);

        CONDITION_PLAYER_NOTIFY_RETURN(skill->__level - 1 < player->level(),
        		RETURN_SKILL_LEVEL_UP, ERROR_ARRIVE_MAX_LEVEL);

        inner_req.set_skill_level(skill->__level);
        break;
    }
    case GameEnum::SKILL_FUN_MOUNT:
    {
    	inner_req.set_skill_level(1);
    	break;
    }
    default:
    {
    	return player->respond_to_client_error(RETURN_SKILL_LEVEL_UP, ERROR_CLIENT_OPERATE);
    }
    }

    return player->send_to_logic_thread(inner_req);
}

int MapSkiller::add_used_times_and_level_up(FighterSkill* skill)
{
	JUDGE_RETURN(skill->__need_used_times > 0, -1);

	bool notify = false;
	if (skill->__used_times < skill->__need_used_times)
	{
		notify = true;
		skill->__used_times += 1;
	}

	MapPlayer *player = this->skill_player();
	if (skill->__used_times >= skill->__need_used_times && skill->__level < player->level())
	{
		notify = false;
		skill->__level += 1;
		skill->__used_times = 0;
		player->insert_skill(skill->__skill_id, skill->__level);
		player->record_other_serial(MAIN_PLAYER_SKILL_SERIAL, skill->__skill_id, skill->__level);
	}

	JUDGE_RETURN(notify == true, -1);
	return player->notify_fight_update(FIGHT_UPDATE_SKILL, skill->__used_times, 0, skill->__skill_id);
}

bool MapSkiller::validate_init_insert_skill(int skill_id)
{
    const Json::Value &skill_json = CONFIG_INSTANCE->skill(skill_id);
    JUDGE_RETURN(skill_json.isMember("check_flag") == true, false);

	MapPlayer *player = this->skill_player();
    int sex = GameCommon::fetch_skill_sex_bit(skill_id);
    JUDGE_RETURN(sex == 0 || player->fight_sex() == sex, false);

	int use_lvl = skill_json["useLvl"].asInt();
	JUDGE_RETURN(player->level() >= use_lvl, false);

	int scene_id = skill_json["scene_id"].asInt();
	if (scene_id > 0)
	{
		JUDGE_RETURN(player->scene_id() == scene_id, false);
	}

	return true;
}

int MapSkiller::fetch_skill_amount_in_level(const int skill_level)
{
    int skill_amount = 0;
    MapPlayer *player = this->skill_player();
    for (SkillMap::iterator iter = player->fight_detail().__skill_map.begin();
            iter != player->fight_detail().__skill_map.end(); ++iter)
    {
        FighterSkill *skill = iter->second;
        JUDGE_CONTINUE(skill->__level >= skill_level);
        ++skill_amount;
    }
    return skill_amount;
}

