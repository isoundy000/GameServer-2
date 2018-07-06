/*
 * MapLogicLabel.cpp
 *
 *  Created on: 2013-12-1
 *      Author: louis
 */

#include "MapLogicLabel.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "GameFont.h"
#include "GameCommon.h"
#include "PubStruct.h"

MapLogicLabel::MapLogicLabel() 
{
    this->label_detail_.cur_label_id_ = 0;
    this->label_detail_.pre_label_id_ = -1;
}

MapLogicLabel::~MapLogicLabel() 
{
	// TODO Auto-generated destructor stub
}


int MapLogicLabel::fetch_label_panel_info(Message* msg)
{
	Proto51400701 respond;
	respond.set_cur_label_id(this->label_detail_.cur_label_id_);

	IntSet::iterator it = this->label_detail_.permant_label_list_.begin();
	for(; it != this->label_detail_.permant_label_list_.end(); ++it)
		respond.add_permant_label_list(*it);

	std::map<int, Int64>::iterator iter = this->label_detail_.limit_time_label_list_.begin();
	for(; iter != this->label_detail_.limit_time_label_list_.end(); ++iter)
	{
		ProtoLimitTimeLabel* proto_list = respond.add_limit_time_label_list();
		proto_list->set_label_id(iter->first);
		proto_list->set_left_time(GameCommon::left_time(iter->second));
	}

	IntMap prop_map;
	this->calc_label_total_prop(prop_map);
	FightProperty temp;
	temp.unserialize(prop_map);
	temp.serialize(respond.mutable_prop_list());

	const Json::Value& cfg = CONFIG_INSTANCE->label_json();
	if(cfg != Json::Value::null && (int)cfg["expire_unshown"].size() > 0)
	{
		for(int i = 0; i < (int)cfg["expire_unshown"].size(); ++i)
		{
			JUDGE_CONTINUE(this->label_detail_.expire_unshown_list_.count(cfg["expire_unshown"][i].asInt()) > 0);
			respond.add_unshown_list(cfg["expire_unshown"][i].asInt());
		}
	}

	FINER_PROCESS_RETURN(RETURN_REQUEST_LABEL_PANEL_INFO, &respond);
}

int MapLogicLabel::select_label(Message* msg)
{
	int label_id;

	MSG_DYNAMIC_CAST_RETURN(Proto11400702*, request, RETURN_SELECT_LABEL);
	label_id = request->label_id();

	const Json::Value& no_change_label = CONFIG_INSTANCE->tiny("no_change_label");
	CONDITION_NOTIFY_RETURN(GameCommon::is_value_in_config(no_change_label,
			this->label_detail_.cur_label_id_) == false, RETURN_SELECT_LABEL, ERROR_LABEL_SELECT_LIMITED);

	if (label_id > 0)
	{
		const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
		CONDITION_NOTIFY_RETURN(cfg != Json::Value::null, RETURN_SELECT_LABEL,
				ERROR_CONFIG_NOT_EXIST);
		CONDITION_NOTIFY_RETURN(cfg["force_put_when_insert"].asInt() != 1, RETURN_SELECT_LABEL,
				ERROR_CLIENT_OPERATE);

		CONDITION_NOTIFY_RETURN(this->label_detail_.permant_label_list_.count(label_id) > 0 ||
				this->label_detail_.limit_time_label_list_.count(label_id) > 0,
				RETURN_SELECT_LABEL, ERROR_CLIENT_OPERATE);
	}

	this->refresh_cur_label_shape(label_id);

	Proto51400702 respond;
	respond.set_label_id(label_id);
	FINER_PROCESS_RETURN(RETURN_SELECT_LABEL, &respond);
}

int MapLogicLabel::cancel_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400703*, request, RETURN_CANCEL_LABEL);
	int label_id = request->label_id();

	const Json::Value& no_change_label = CONFIG_INSTANCE->tiny("no_change_label");
	CONDITION_NOTIFY_RETURN(GameCommon::is_value_in_config(no_change_label,
			this->label_detail_.cur_label_id_) == false, RETURN_SELECT_LABEL, ERROR_LABEL_SELECT_LIMITED);

	const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
	CONDITION_NOTIFY_RETURN(cfg != Json::Value::null, RETURN_CANCEL_LABEL, ERROR_CONFIG_NOT_EXIST);

	CONDITION_NOTIFY_RETURN(this->label_detail_.permant_label_list_.count(label_id) > 0 ||
			this->label_detail_.limit_time_label_list_.count(label_id) > 0,
			RETURN_CANCEL_LABEL, ERROR_CLIENT_OPERATE);

	this->refresh_cur_label_shape();

	Proto51400703 respond;
	respond.set_label_id(label_id);
	FINER_PROCESS_RETURN(RETURN_CANCEL_LABEL, &respond);
}

int MapLogicLabel::check_label_pa_event()
{
	if (this->label_detail_.new_list_.size())
	{
		this->notify_new_label();
	}
	this->map_logic_player()->update_player_assist_single_event(GameEnum::PA_EVENT_LABEL,
			this->label_detail_.new_list_.size());
	return 0;
}

int MapLogicLabel::notify_new_label()
{
	Proto81400702 respond;
	for (IntSet::iterator it = this->label_detail_.new_list_.begin();
			it != this->label_detail_.new_list_.end(); ++it)
	{
		respond.add_new_list(*it);
	}

	FINER_PROCESS_RETURN(ACTIVE_NOTIFY_NEW_LABEL, &respond);
}

int MapLogicLabel::check_new_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400705*, request, RETURN_CHECK_NEW_LABEL);
	int label_id = request->label_id();

	Proto51400705 respond;
	respond.set_label_id(label_id);
	respond.set_status(0);

	if (this->label_detail_.new_list_.count(label_id))
	{
		this->label_detail_.new_list_.erase(label_id);
		respond.set_status(1);
	}

	this->check_label_pa_event();

	FINER_PROCESS_RETURN(RETURN_CHECK_NEW_LABEL, &respond);
}

int MapLogicLabel::fetch_single_label_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11400704*, request, RETURN_REQUEST_SINGLE_LABEL_INFO);
	int label_id = request->label_id();

	const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
	CONDITION_NOTIFY_RETURN(cfg != Json::Value::null,
			RETURN_REQUEST_SINGLE_LABEL_INFO, ERROR_CONFIG_NOT_EXIST);

	CONDITION_NOTIFY_RETURN(this->label_detail_.permant_label_list_.count(label_id) > 0 ||
			this->label_detail_.limit_time_label_list_.count(label_id) > 0,
			RETURN_REQUEST_SINGLE_LABEL_INFO, ERROR_SPECIAL_LABEL);

	int left_time = this->label_detail_.limit_time_label_list_.count(label_id) > 0 ?
			GameCommon::left_time(this->label_detail_.limit_time_label_list_[label_id]) : -1;

	if(this->label_detail_.limit_time_label_list_.count(label_id) > 0)
	{
		MSG_USER("- player_name:%s-id:%ld-label_id:%d-expire_tick:%d -left_time:%d-",
				this->name(), this->role_id(),label_id, this->label_detail_.limit_time_label_list_[label_id], left_time);
	}

	Proto51400704 respond;
	respond.set_label_id(label_id);
	respond.set_left_time(left_time);
	FINER_PROCESS_RETURN(RETURN_REQUEST_SINGLE_LABEL_INFO, &respond);
}

int MapLogicLabel::sync_add_label(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31401701*, request, -1);
	int label_id = request->label_id();

	switch(request->type())
	{
	case 0:
	{
		this->insert_label(label_id);
		break;
	}
	case 1:
	{
		this->delete_label(label_id);
		break;
	}
	case 2:
	{
		if( this->label_detail_.pre_label_id_ == -1 )
		{
			this->label_detail_.pre_label_id_ = this->label_detail_.cur_label_id_;
		}
		this->insert_label(label_id);
		break;
	}
	case 3:
	{
		if(this->pre_label_id() == -1)
		{
			this->refresh_cur_label_shape(0);
			break;
		}
		this->refresh_cur_label_shape(this->pre_label_id());
		this->label_detail_.pre_label_id_ = -1;
		break;
	}
	}
	return 0;
}

int MapLogicLabel::label_time_up(const Time_Value& now_time)
{
	IntVec remove_set;
	std::map<int, Int64>::iterator it = this->label_detail_.limit_time_label_list_.begin();
	for(; it != this->label_detail_.limit_time_label_list_.end(); ++it)
	{
		JUDGE_CONTINUE(GameCommon::left_time(it->second) <= 0);
		remove_set.push_back(it->first);
	}

	JUDGE_RETURN(remove_set.size() > 0, 0);

	for(IntVec::iterator iter = remove_set.begin(); iter != remove_set.end(); ++iter)
	{
	    //MailInformation *mail_info = GameCommon::create_sys_mail(FONT_NO_PACKAGE);
	    //GameCommon::request_save_mail(this->role_id(), mail_info);
		this->delete_label(*iter);

		/*
		IntMap prop_map;

		this->calc_label_total_prop_ex(*iter, prop_map);

		FightProperty temp;
		temp.unserialize(prop_map);

		char content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH + 1] = {0};
		FontPair join_font = FONT2(FONT_LABEL_TIME_UP);
		std::snprintf(content, GameEnum::DEFAULT_MAX_CONTENT_LEGNTH, join_font.second.c_str(),*iter, temp.force_);
		content[GameEnum::DEFAULT_MAX_CONTENT_LEGNTH] = '\0';

		MailInformation* join_reward_mail = GameCommon::create_sys_mail(join_font.first, content,FONT_LABEL_TIME_UP);
		GameCommon::request_save_mail(this->role_id(), join_reward_mail);
		*/

		int label_id = *iter;
		const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
		int mail_id = CONFIG_INSTANCE->const_set("label_time_up_mail_id");
		MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);

		string name = cfg["name"].asString();

		::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), name.c_str());

		GameCommon::request_save_mail_content(this->role_id(), mail_info);
	}

	return 0;
}

int MapLogicLabel::notify_label_list_update(int opera, int label_id, int left_time)
{
	Proto81400701 respond;
	respond.set_operate(opera);
	respond.set_label_id(label_id);
	if(left_time > 0)
		respond.set_left_time(left_time);

	IntMap prop_map;
	this->calc_label_total_prop(prop_map);

	FightProperty temp;
	temp.unserialize(prop_map);
	temp.serialize(respond.mutable_prop_list());
	FINER_PROCESS_RETURN(ACTIVE_UPDATE_LABEL_LIST, &respond);
}

int MapLogicLabel::refresh_cur_label_shape(const int label_id)
{
	int expire_tick = 0;
	if (label_id > 0)
	{
		expire_tick = this->label_detail_.limit_time_label_list_.count(label_id) > 0
				? this->label_detail_.limit_time_label_list_[label_id] : -1;
	}

	this->set_cur_label_id(label_id);

	Proto30400602 respond;
	respond.set_cur_label(label_id);
	respond.set_expire_tick(expire_tick);
	return this->send_to_map_thread(respond);
}

int MapLogicLabel::cur_label_id(void)
{
	return this->label_detail_.cur_label_id_;
}

int MapLogicLabel::set_cur_label_id(int label_id)
{
	this->label_detail_.cur_label_id_ = label_id;
	return 0;
}

int MapLogicLabel::pre_label_id(void)
{
	return this->label_detail_.pre_label_id_;
}

int MapLogicLabel::set_pre_label_id(int label_id)
{
	this->label_detail_.pre_label_id_ = label_id;
	return 0;
}

IntSet& MapLogicLabel::new_list(void)
{
	return this->label_detail_.new_list_;
}

IntSet& MapLogicLabel::permant_label_list(void)
{
	return this->label_detail_.permant_label_list_;
}

IntSet& MapLogicLabel::expire_unshown_list(void)
{
	return this->label_detail_.expire_unshown_list_;
}

std::map<int, Int64>& MapLogicLabel::limit_time_label_list(void)
{
	return this->label_detail_.limit_time_label_list_;
}

bool MapLogicLabel::is_has_label(const int label_id)
{
    if (this->permant_label_list().find(label_id) != this->permant_label_list().end())
        return true;
    if (this->expire_unshown_list().find(label_id) != this->expire_unshown_list().end())
        return true;
    return false;
}

int MapLogicLabel::insert_label(int label_id)
{
	JUDGE_RETURN(label_id > 0, 0);

	const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
	JUDGE_RETURN(cfg != Json::Value::null, -1);

	int label_type = GameCommon::fetch_label_type(label_id);
	int left_time = -1;
	switch(label_type)
	{
		case GameEnum::LABEL_TYPE_PERMANT:
		{
				this->label_detail_.permant_label_list_.insert(label_id);
				break;
		}
		case GameEnum::LABEL_TYPE_LIMIT_TIME:
		{
				int time = cfg["time"].asInt();

				if(this->label_detail_.limit_time_label_list_.count(label_id) > 0)
				{
					this->label_detail_.limit_time_label_list_[label_id] += time;
				}
				else
				{
					this->label_detail_.limit_time_label_list_[label_id] = ::time(NULL) + time;
					this->modify_expire_unshown_list(GameEnum::LABEL_INSERT, label_id);
				}

				left_time = GameCommon::left_time(this->label_detail_.limit_time_label_list_[label_id]);
				break;
		}
		default:
			break;
	}

	this->map_logic_player()->sync_fight_property_to_map();

	if (cfg["force_put_when_insert"].asInt() == 1)
	{
		if (this->cur_label_id() != label_id)
		{
			this->refresh_cur_label_shape(label_id);
//			Proto51400702 respond;
//			respond.set_label_id(label_id);
//			this->respond_to_client(RETURN_SELECT_LABEL, &respond);
		}
	}
	else
	{
		// 通知是否替换新的称号
		this->notify_label_list_update(GameEnum::LABEL_INSERT, label_id, left_time);
	}

	this->check_label_pa_event();
	return 0;
}

int MapLogicLabel::insert_label_i(int label_id, Int64 start_tick)
{
	const Json::Value& cfg = CONFIG_INSTANCE->label(label_id);
	JUDGE_RETURN(cfg != Json::Value::null, -1);

	int label_type = GameCommon::fetch_label_type(label_id);
	switch(label_type)
	{
	case GameEnum::LABEL_TYPE_PERMANT:
	{
		this->label_detail_.permant_label_list_.insert(label_id);
		break;
	}

	case GameEnum::LABEL_TYPE_LIMIT_TIME:
	{
		if (start_tick > 0)
		{
			int left_time = start_tick + cfg["time"].asInt() - ::time(NULL);
			JUDGE_RETURN(left_time > 0, -1);

			this->label_detail_.limit_time_label_list_[label_id] = ::time(NULL) + left_time;
		}
		else
		{
			this->label_detail_.limit_time_label_list_[label_id] = ::time(NULL) + cfg["time"].asInt();
		}

		this->modify_expire_unshown_list(GameEnum::LABEL_INSERT, label_id);
		break;
	}

	default:
	{
		break;
	}
	}

	return 0;
}

int MapLogicLabel::delete_label(int label_id)
{
	if (this->label_detail_.limit_time_label_list_.count(label_id) > 0)
	{
		this->label_detail_.limit_time_label_list_.erase(label_id);
		this->notify_label_list_update(GameEnum::LABEL_DELETE, label_id);
		this->modify_expire_unshown_list(GameEnum::LABEL_DELETE, label_id);
	}
	else if (this->label_detail_.permant_label_list_.count(label_id) > 0)
	{
		this->label_detail_.permant_label_list_.erase(label_id);
	}

	if (this->label_detail_.cur_label_id_ == label_id)
	{
		this->refresh_cur_label_shape();
	}

	if (this->label_detail_.new_list_.count(label_id) > 0)
	{
		this->label_detail_.new_list_.erase(label_id);
	}

	this->map_logic_player()->sync_fight_property_to_map();
	return 0;
}

int MapLogicLabel::calc_label_total_prop(IntMap& prop_map)
{
	IntSet::iterator iter = this->label_detail_.permant_label_list_.begin();
	for(; iter != this->label_detail_.permant_label_list_.end(); ++iter)
	{
		this->calc_label_total_prop_ex(*iter, prop_map);
	}

	std::map<int, Int64>::iterator it = this->label_detail_.limit_time_label_list_.begin();
	for(; it != this->label_detail_.limit_time_label_list_.end(); ++it)
	{
		JUDGE_CONTINUE(GameCommon::left_time(it->second) > 0);
		this->calc_label_total_prop_ex(it->first, prop_map);
	}

	return 0;
}

int MapLogicLabel::calc_label_total_prop_ex(const int label_id, IntMap& prop_map)
{
    const Json::Value &label_json = CONFIG_INSTANCE->label(label_id);
    JUDGE_RETURN(label_json != Json::Value::null, -1);

//    if (label_json.isMember("second_prop") == true && (int)label_json["second_prop"].size() >= 8)
//    {
//    	for(int i = 0; i < (int)label_json["second_prop"].size(); ++i)
//    	{
//    		int value = label_json["second_prop"][i].asInt();
//        	JUDGE_CONTINUE(value > 0);
//        	prop_map[GameEnum::BLOOD_MAX + i] += value;
//    	}
//    }
    for(int prop_id = GameEnum::ATTR_BEGIN; prop_id <= GameEnum::ATTR_END; ++prop_id){
    	string prop_name = GameCommon::fight_prop_name(prop_id);
    	if(label_json.isMember(prop_name)){
    		if(prop_name == GameName::ATTACK){
    			prop_map[GameEnum::ATTACK] += label_json[prop_name].asInt();
    		}else if(prop_name == GameName::DEFENSE){
    			prop_map[GameEnum::DEFENSE] += label_json[prop_name].asInt();
    		}else{
    			prop_map[prop_id] += label_json[prop_name].asInt();
    		}
    	}
    }
	return 0;
}

int MapLogicLabel::modify_expire_unshown_list(const int opera, const int label_id)
{
	const Json::Value& cfg = CONFIG_INSTANCE->label_json();
	JUDGE_RETURN(cfg != Json::Value::null, -1);
	JUDGE_RETURN((int)cfg["expire_unshown"].size() > 0, 0);

	for(int i = 0; i < (int)cfg["expire_unshown"].size(); ++i)
	{
		JUDGE_CONTINUE(label_id == cfg["expire_unshown"][i].asInt());

		switch(opera)
		{
		{
			case GameEnum::LABEL_INSERT:
				if(this->label_detail_.expire_unshown_list_.count(label_id) > 0)
					this->label_detail_.expire_unshown_list_.erase(label_id);
				break;
		}
		{
			case GameEnum::LABEL_DELETE:
				this->label_detail_.expire_unshown_list_.insert(label_id);
				break;
		}
		default:
			break;
		}
	}

	return 0;
}

void LabelDetail::reset()
{
	cur_label_id_ = 0;
	pre_label_id_ = -1;
	permant_label_list_.clear();
	limit_time_label_list_.clear();
	expire_unshown_list_.clear();
	new_list_.clear();
}

void MapLogicLabel::reset(void)
{
	this->label_detail_.reset();
}

int MapLogicLabel::sync_transfer_label_info(int scene_id)
{
	Proto31400121 respond;
	respond.set_cur_label_id(this->label_detail_.cur_label_id_);
	respond.set_pre_label_id(this->label_detail_.pre_label_id_);

	IntSet::iterator it = this->label_detail_.permant_label_list_.begin();
	for(; it != this->label_detail_.permant_label_list_.end(); ++it)
		respond.add_permant_label_list(*it);

	IntSet::iterator it1 = this->label_detail_.new_list_.begin();
	for(; it1 != this->label_detail_.new_list_.end(); ++it1)
		respond.add_new_list(*it1);

	std::map<int, Int64>::iterator iter = this->label_detail_.limit_time_label_list_.begin();
	for(; iter != this->label_detail_.limit_time_label_list_.end(); ++iter)
	{
		ProtoFlaunt* proto_list = respond.add_limit_time_label_list();
		proto_list->set_color(iter->first);
		proto_list->set_flaunt_id(iter->second);
	}

	const Json::Value& cfg = CONFIG_INSTANCE->label_json();
	if(cfg != Json::Value::null && (int)cfg["expire_unshown"].size() > 0)
	{
		for(int i = 0; i < (int)cfg["expire_unshown"].size(); ++i)
		{
			JUDGE_CONTINUE(this->label_detail_.expire_unshown_list_.count(cfg["expire_unshown"][i].asInt()) > 0);
			respond.add_unshown_list(cfg["expire_unshown"][i].asInt());
		}
	}
//	MSG_DEBUG(SYNC_LABEL:%s, respond.Utf8DebugString().c_str());
	return this->send_to_other_logic_thread(scene_id, respond);
}

int MapLogicLabel::read_transfer_label_info(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31400121*, respond, -1);
//	MSG_DEBUG(get_SYNC_LABEL:%s, respond->Utf8DebugString().c_str());
	this->label_detail_.cur_label_id_ = respond->cur_label_id();
	this->label_detail_.pre_label_id_ = respond->pre_label_id();

	this->label_detail_.permant_label_list_.clear();
	for(int i = 0; i < respond->permant_label_list_size(); ++i)
	{
		int label_id = respond->permant_label_list(i);
		this->label_detail_.permant_label_list_.insert(label_id);
	}

	this->label_detail_.new_list_.clear();
	for(int i = 0; i < respond->new_list_size(); ++i)
	{
		int label_id = respond->new_list(i);
		this->label_detail_.new_list_.insert(label_id);
	}

	this->label_detail_.limit_time_label_list_.clear();
	for(int i = 0; i < respond->limit_time_label_list_size(); ++i)
	{
		ProtoFlaunt proto_limit_list = respond->limit_time_label_list(i);
		int label_id = proto_limit_list.color();
		Int64 expire_tick = proto_limit_list.flaunt_id();
		this->label_detail_.limit_time_label_list_.insert(std::pair<int, Int64>(label_id, expire_tick));
	}

	this->label_detail_.expire_unshown_list_.clear();
	for(int i = 0; i < respond->unshown_list_size(); ++i)
	{
		int label_id = respond->unshown_list(i);
		this->label_detail_.expire_unshown_list_.insert(label_id);
	}
	return 0;
}

int MapLogicLabel::insert_label_by_item(const Json::Value &effect, PackageItem *pack_item, int& use_num)
{
	int label_id = effect["label_id"].asInt();
	if(this->label_detail_.permant_label_list_.count(label_id))
	{
		return GameEnum::USE_GOODS_MUCH_TIMES;
	}
	if (this->label_detail_.new_list_.count(label_id) == 0)
	{
		this->label_detail_.new_list_.insert(label_id);
	}

	int ret = 0;
	for(int i = 0; i < use_num; ++i)
	{
		ret = this->insert_label(label_id);
		if(ret != 0)
		{
			if(i == 0)
			{
				return ret;
			}
			else
			{
				use_num = i;
				return 0;
			}
		}
	}
	return 0;
}

