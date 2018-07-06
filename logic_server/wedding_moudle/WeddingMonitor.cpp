/*
 * WeddingMonitor.cpp
 *
 * Created on: 2015-06-04 21:43
 *     Author: lyz
 */

#include "WeddingMonitor.h"
#include "LogicMonitor.h"
#include "GameField.h"
#include "LogicStruct.h"
#include "LogicPlayer.h"
#include "ProtoDefine.h"
#include "MMOWedding.h"
#include "MongoDataMap.h"
#include "SendActReward.h"

int WeddingMonitor::MailTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int WeddingMonitor::MailTimer::handle_timeout(const Time_Value &tv)
{
	return WEDDING_MONITOR->send_mail_to_player();
}

int WeddingMonitor::init(void)
{
	this->monitor_ = LOGIC_MONITOR;
    MMOWedding::load_all_wedding_detail(this);

	MSG_USER("WEDDING_SYS %d %d %d", this->wedding_by_id_map_.size(), this->wedding_by_role_map_.size(),
			this->remove_id_set_.size());
    this->update_cache_tick_ = Time_Value::zero;
    this->cruise_icon_check_tick_ = Time_Value::zero;

	this->mail_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,0).sec() - Time_Value::gettimeofday().sec();
	this->mail_timer_.schedule_timer((int)left_time);

	MSG_USER("WEDDING_SYS %d %d %d %d", this->wedding_by_id_map_.size(), this->wedding_by_role_map_.size(),
			this->remove_id_set_.size(), this->mail_timer_.left_second());
    return 0;
}

int WeddingMonitor::send_mail_to_player(void)
{
	int mail_id = CONFIG_INSTANCE->wedding_base()["treasures_mail_id"].asInt();
	for (WeddingMap::iterator it = this->wedding_by_id_map_.begin(); it != this->wedding_by_id_map_.end(); ++it)
	{
		Proto30103001 reward_info;
		reward_info.set_act_type(mail_id);
		reward_info.set_mail_id(mail_id);
		WeddingDetail* detail = it->second;

		//partner_1
		int reward_num = 0;
		if (detail->__partner_1.__tick > 0)
		{
			Int64 left_time = Time_Value::gettimeofday().sec() - detail->__partner_1.__fetch_tick;
			if (left_time > Time_Value::DAY)
			{
				Proto30103001 reward_info;
				reward_info.set_act_type(mail_id);
				reward_info.set_mail_id(mail_id);
				WeddingDetail* detail = it->second;

			    int id = CONFIG_INSTANCE->wedding_treasures(1)["keep_time"].asInt() - detail->__partner_1.__left_times + 1;
			    const Json::Value &info_json = CONFIG_INSTANCE->wedding_treasures(id);
		        int reward_id = info_json["day_reward"].asInt();
		        reward_num++;
		        reward_info.add_reward_list(reward_id);
				detail->__partner_1.__fetch_tick = Time_Value::gettimeofday().sec() - 60;

		        //最后一天领完清空
				detail->__partner_1.__left_times--;
		        if (detail->__partner_1.__left_times == 0)
		        {
		        	detail->__partner_1.__fetch_tick = 0;
		        	detail->__partner_1.__tick = 0;
		        	detail->__partner_1.__once_reward = 0;
		        }

				reward_info.set_role_id(detail->__partner_1.__role_id);
				if (reward_num > 0) SEND_ACTREWARD->send_player_act_reward(&reward_info);
			}
		}


		//partner_2
		reward_num = 0;
		if (detail->__partner_2.__tick > 0)
		{
			Int64 left_time = Time_Value::gettimeofday().sec() - detail->__partner_2.__fetch_tick;
			if (left_time > Time_Value::DAY)
			{
				Proto30103001 reward_info;
				reward_info.set_act_type(mail_id);
				reward_info.set_mail_id(mail_id);
				WeddingDetail* detail = it->second;

			    int id = CONFIG_INSTANCE->wedding_treasures(1)["keep_time"].asInt() - detail->__partner_2.__left_times + 1;
			    const Json::Value &info_json = CONFIG_INSTANCE->wedding_treasures(id);
		        int reward_id = info_json["day_reward"].asInt();
		        reward_num++;
		        reward_info.add_reward_list(reward_id);
				detail->__partner_2.__fetch_tick = Time_Value::gettimeofday().sec() - 60;

		        //最后一天领完清空
				detail->__partner_2.__left_times--;
		        if (detail->__partner_2.__left_times == 0)
		        {
		        	detail->__partner_2.__fetch_tick = 0;
		        	detail->__partner_2.__tick = 0;
		        	detail->__partner_2.__once_reward = 0;
		        }

				reward_info.set_role_id(detail->__partner_2.__role_id);
				if (reward_num > 0) SEND_ACTREWARD->send_player_act_reward(&reward_info);
			}

		}


		//刷新面板
		LogicPlayer* partner1 = NULL;
		LogicPlayer* partner2 = NULL;
		if (this->monitor_->find_player(detail->__partner_1.__role_id, partner1) == 0 && partner1 != NULL)
		{
			partner1->request_wedding_pannel();
		}
		if (this->monitor_->find_player(detail->__partner_2.__role_id, partner2) == 0 && partner2 != NULL)
		{
			partner2->request_wedding_pannel();
		}
	}

	this->mail_timer_.cancel_timer();
	Int64 left_time = ::next_day(0,0).sec() - Time_Value::gettimeofday().sec();
	this->mail_timer_.schedule_timer((int)left_time);
	return 0;
}

int WeddingMonitor::stop(void)
{
    this->request_save_all_wedding_info();
    return 0;
}

LogicMonitor *WeddingMonitor::monitor(void)
{
    return this->monitor_;
}

bool WeddingMonitor::is_has_wedding(const Int64 role_id)
{
    if (this->wedding_by_role_map_.find(role_id) != this->wedding_by_role_map_.end())
        return true;
    return false;
}

WeddingMonitor::WeddingMap &WeddingMonitor::wedding_by_id_map(void)
{
    return this->wedding_by_id_map_;
}

WeddingMonitor::WeddingMap &WeddingMonitor::wedding_by_role_map(void)
{
    return this->wedding_by_role_map_;
}

WeddingDetail *WeddingMonitor::fetch_wedding_detail(const Int64 role_id)
{
    WeddingMap::iterator iter = this->wedding_by_role_map_.find(role_id);
    if (iter != this->wedding_by_role_map_.end())
        return iter->second;
    return NULL;
}

WeddingDetail *WeddingMonitor::find_wedding_detail(const Int64 wedding_id)
{
    WeddingMap::iterator iter = this->wedding_by_id_map_.find(wedding_id);
    if (iter != this->wedding_by_id_map_.end())
        return iter->second;
    return NULL;
}

int WeddingMonitor::insert_new_wedding(const Int64 partner_1, const Int64 partner_2, const int wedding_type)
{
    WeddingDetail *detail1 = this->fetch_wedding_detail(partner_1),
                  *detail2 = this->fetch_wedding_detail(partner_2);
    JUDGE_RETURN(detail1 == NULL, ERROR_HAS_WEDDING);
    JUDGE_RETURN(detail2 == NULL, ERROR_TARGET_HAS_WEDDING);

    WeddingDetail *detail = this->monitor()->wedding_detail_pool()->pop();

    int64_t wedding_id = 0;
    this->monitor()->fetch_global_value(Global::WEDDING, wedding_id);
    detail->__wedding_id = wedding_id;
    if (detail->__wedding_id <= 0)
    {
        this->monitor()->wedding_detail_pool()->push(detail);
        return ERROR_SERVER_INNER;
    }
    detail->__wedding_tick = Time_Value::gettimeofday();
    detail->__wedding_type = wedding_type;

    int init_intimacy = 0;
    LogicPlayer *player = NULL;
    {
        WeddingDetail::WeddingRole &wedding_role = detail->__partner_1;
        wedding_role.__role_id = partner_1;
        if (this->monitor()->find_player(partner_1, player) == 0)
        {
            wedding_role.__role_name = player->role_detail().__name;
            wedding_role.__sex = player->role_detail().__sex;
            wedding_role.__career = player->role_detail().__career;
            init_intimacy = player->intimacy(partner_2);

            // 更新玩家的wedding_id
//            player->self_wedding_info().__intimacy_map[partner_2] = 1;
            player->self_wedding_info().__wedding_id = wedding_id;
            player->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
        }
    }
    {
        WeddingDetail::WeddingRole &wedding_role = detail->__partner_2;
        wedding_role.__role_id = partner_2;
        if (this->monitor()->find_player(partner_2, player) == 0)
        {
            wedding_role.__role_name = player->role_detail().__name;
            wedding_role.__sex = player->role_detail().__sex;
            wedding_role.__career = player->role_detail().__career;
            init_intimacy = std::max(init_intimacy, player->intimacy(partner_1));

            // 更新玩家的wedding_id
//            player->self_wedding_info().__intimacy_map[partner_1] = 1;
            player->self_wedding_info().__wedding_id = wedding_id;
            player->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
        }
    }

    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &wedding_type_json = wedding_json["wedding_type"][wedding_type - 1];
    detail->__keepsake_id = wedding_type_json["keepsake"].asInt();
    detail->__day_refresh_tick = next_day(0, 0, Time_Value::gettimeofday());
    detail->__day_wedding_times = 1;
    detail->__intimacy = init_intimacy;
    detail->__history_intimacy = init_intimacy;

    this->wedding_by_id_map_[detail->__wedding_id] = detail;
    this->wedding_by_role_map_[detail->__partner_1.__role_id] = detail;
    this->wedding_by_role_map_[detail->__partner_2.__role_id] = detail;

    this->insert_update_id(detail->__wedding_id);

    MSG_USER("wedding[%ld] %ld %s | %ld %s", detail->__wedding_id,
    		detail->__partner_1.__role_id, detail->__partner_1.__role_name.c_str(),
    		detail->__partner_2.__role_id, detail->__partner_2.__role_name.c_str());

    //结婚邮件
    int mail_id = CONFIG_INSTANCE->wedding_base()["wed_mail"].asInt();
	MailInformation *mail_info_1 = GameCommon::create_sys_mail(mail_id);
	MailInformation *mail_info_2 = GameCommon::create_sys_mail(mail_id);

	::snprintf(mail_info_1->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info_1->mail_content_.c_str(), detail->__partner_2.__role_name.c_str());
	::snprintf(mail_info_2->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info_2->mail_content_.c_str(), detail->__partner_1.__role_name.c_str());

	GameCommon::request_save_mail_content(detail->__partner_1.__role_id, mail_info_1);
	GameCommon::request_save_mail_content(detail->__partner_2.__role_id, mail_info_2);

    return 0;
}

int WeddingMonitor::update_wedding(const Int64 partner_1, const Int64 partner_2, const int wedding_type)
{
    WeddingDetail *detail1 = this->fetch_wedding_detail(partner_1),
                  *detail2 = this->fetch_wedding_detail(partner_2);
    JUDGE_RETURN(detail1 != NULL && detail1 == detail2, ERROR_TARGET_HAS_WEDDING);
    JUDGE_RETURN(wedding_type > detail1->__wedding_type, 0);
    
    detail1->__wedding_type = wedding_type;
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &wedding_type_json = wedding_json["wedding_type"][wedding_type - 1];
    detail1->__keepsake_id = wedding_type_json["keepsake"].asInt();
    detail1->__keepsake_level = 0;
    detail1->__keepsake_sublevel = 0;
    detail1->__keepsake_progress = 0;
    detail1->__history_intimacy = detail1->__intimacy;
    ++detail1->__day_wedding_times;

    this->insert_update_id(detail1->__wedding_id);
    
    return 0;
}

int WeddingMonitor::remove_wedding(const Int64 wedding_id)
{
    WeddingDetail *wedding_info = this->find_wedding_detail(wedding_id);
    JUDGE_RETURN(wedding_info != NULL, ERROR_NO_WEDDING);

    this->wedding_by_id_map_.erase(wedding_id);
    this->wedding_by_role_map_.erase(wedding_info->__partner_1.__role_id);
    this->wedding_by_role_map_.erase(wedding_info->__partner_2.__role_id);
    this->insert_remove_id(wedding_id);

    this->monitor()->wedding_detail_pool()->push(wedding_info);
    return 0;
}

int WeddingMonitor::process_other_update_intimacy(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31101607 *, request, -1);
    return 0;
}

int WeddingMonitor::check_and_update_intimacy(const int type, const int id, LongSet &role_set, const Json::Value &intimacy_json)
{
	const int /*TYPE_FINISH_SCRIPT = 1, */TYPE_KILL_PLAYER = 2;
    int inc_intimacy = 0;
    if (type == TYPE_KILL_PLAYER)
    {
        for (uint i = 1; i < intimacy_json.size(); ++i)
        {
            if (intimacy_json[i].asInt() == id)
                return 0;
        }
        inc_intimacy = intimacy_json[0u].asInt();
    }
    else
    {
        for (uint i = 0; i < intimacy_json.size(); ++i)
        {
            if (intimacy_json[i][0u].asInt() == id)
            {
                inc_intimacy = intimacy_json[i][1u].asInt();
                break;
            }
        }
    }
    JUDGE_RETURN(inc_intimacy != 0, 0);

    LogicPlayer *partner_1 = NULL, *partner_2 = NULL;
    for (LongSet::iterator first_iter = role_set.begin();
            first_iter != role_set.end(); ++first_iter)
    {
        if (this->monitor()->find_player(*first_iter, partner_1) != 0)
            continue;

        LongSet::iterator second_iter = first_iter;
        for (++second_iter; second_iter != role_set.end(); ++second_iter)
        {
            if (this->monitor()->find_player(*first_iter, partner_2) != 0)
                continue;

            partner_1->update_intimacy(inc_intimacy, *second_iter);
        }
    }

    return 0;
}

int WeddingMonitor::process_handle_timeout(const Time_Value &nowtime)
{
    this->update_wedding_cache(nowtime);
    this->check_and_update_cruise_icon_list(nowtime);
    this->check_and_update_cartoon_list(nowtime);

    return 0;
}

int WeddingMonitor::update_wedding_cache(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->update_cache_tick_ <= nowtime, 0);

    this->update_cache_tick_ = nowtime + Time_Value(30);
    
    this->request_save_all_wedding_info();
    return 0;
}

int WeddingMonitor::check_and_update_cruise_icon_list(const Time_Value &nowtime)
{
    JUDGE_RETURN(this->cruise_icon_check_tick_ <= nowtime, 0);
    this->cruise_icon_check_tick_ = nowtime + Time_Value(10);

    std::vector<Int64> remove_id;
    for (LongSet::iterator iter = this->cruise_id_set_.begin();
            iter != this->cruise_id_set_.end(); ++iter)
    {
        WeddingDetail *detail = this->find_wedding_detail(*iter);
        if (detail == NULL)
            remove_id.push_back(*iter);

        JUDGE_CONTINUE(detail != NULL && detail->__cruise_tick <= nowtime);

        remove_id.push_back(*iter);
    }
    for (std::vector<Int64>::iterator iter = remove_id.begin(); iter != remove_id.end(); ++iter)
        this->cruise_id_set_.erase(*iter);

    if (remove_id.size() > 0)
        this->notify_all_cruise_icon();
    return 0;
}

void WeddingMonitor::insert_update_id(const Int64 id)
{
    this->update_id_set_.insert(id);
}

void WeddingMonitor::insert_remove_id(const Int64 id)
{
    this->remove_id_set_.insert(id);
}

void WeddingMonitor::set_cruise_state(WeddingDetail *wedding_info)
{
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    int cruise_tick = wedding_json["cruise_tick"].asInt();
    if (cruise_tick <= 0)
        cruise_tick = 300;
    wedding_info->__cruise_tick = Time_Value::gettimeofday() + Time_Value(cruise_tick);
    this->cruise_id_set_.insert(wedding_info->__wedding_id);

    this->notify_all_cruise_icon();
}

int WeddingMonitor::notify_all_cruise_icon(void)
{
    Proto80101403 respond;
    this->make_up_all_cruise_icon(&respond);

    LogicMonitor::PlayerMap &player_map = this->monitor()->player_map();
    for (LogicMonitor::PlayerMap::iterator iter = player_map.begin(); 
            iter != player_map.end(); ++iter)
    {
        LogicPlayer *player = iter->second;
        if (player->role_level() < 30)
        	continue;

        player->respond_to_client(ACTIVE_WEDDING_ICON, &respond);
    }
    return 0;
}

int WeddingMonitor::make_up_all_cruise_icon(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto80101403 *, request, msg, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    for (LongSet::iterator iter = this->cruise_id_set_.begin();
            iter != this->cruise_id_set_.end(); ++iter)
    {
        WeddingDetail *detail = this->find_wedding_detail(*iter);
        JUDGE_CONTINUE(detail != NULL && detail->__cruise_tick > nowtime);

        ProtoWeddingIcon *proto_icon = request->add_wedding_icon_list();
        proto_icon->set_wedding_req_id(detail->__partner_1.__role_id);
        proto_icon->set_wedding_req_name(detail->__partner_1.__role_name);
        proto_icon->set_wedding_partner_id(detail->__partner_2.__role_id);
        proto_icon->set_wedding_partner_name(detail->__partner_2.__role_name);
    }
    return 0;
}

int WeddingMonitor::process_cruise_finish(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101609 *, request, msg, -1);

    Int64 wedding_id = request->wedding_id();

    WeddingDetail *wedding_info = this->find_wedding_detail(wedding_id);
    if (wedding_info != NULL)
        wedding_info->__cruise_tick = Time_Value::zero;
    this->cruise_id_set_.erase(wedding_id);

    this->notify_all_cruise_icon();
    return 0;
}

void WeddingMonitor::set_cartoon_state(WeddingDetail *wedding_info, const int wedding_type)
{
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    int cartoon_tick = wedding_json["cartoon_tick"].asInt();
    if (cartoon_tick <= 0)
    {
        this->notify_to_cruise(wedding_info, wedding_type);
        return;
    }

    wedding_info->__wedding_cartoon_tick = Time_Value::gettimeofday() + Time_Value(cartoon_tick);

    this->cartoon_play_map_[wedding_info->__wedding_id] = wedding_type;
}

int WeddingMonitor::make_up_cartoon_info(const Int64 role_id, Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto80101401 *, request, msg, -1);

    WeddingDetail *wedding_info = this->fetch_wedding_detail(role_id);
    JUDGE_RETURN(wedding_info != NULL, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(wedding_info->__wedding_cartoon_tick > nowtime, -1);

    WeddingTypeMap::iterator iter = this->cartoon_play_map_.find(wedding_info->__wedding_id);
    JUDGE_RETURN(iter != this->cartoon_play_map_.end(), -1);
    
    request->set_type(iter->second);
    request->set_cartoon_left_tick(wedding_info->__wedding_cartoon_tick.sec() - nowtime.sec());

    if (wedding_info->__partner_1.__sex == GameEnum::SEX_MALE)
    {
    	request->set_partner_id_1(wedding_info->__partner_1.__role_id);
    	request->set_partner_name_1(wedding_info->__partner_1.__role_name);
    	request->set_partner_career_1(wedding_info->__partner_1.__career);
    	request->set_partner_id_2(wedding_info->__partner_2.__role_id);
    	request->set_partner_name_2(wedding_info->__partner_2.__role_name);
    	request->set_partner_career_2(wedding_info->__partner_2.__career);
    }
    else
    {
    	request->set_partner_id_1(wedding_info->__partner_2.__role_id);
    	request->set_partner_name_1(wedding_info->__partner_2.__role_name);
    	request->set_partner_career_1(wedding_info->__partner_2.__career);
    	request->set_partner_id_2(wedding_info->__partner_1.__role_id);
    	request->set_partner_name_2(wedding_info->__partner_1.__role_name);
    	request->set_partner_career_2(wedding_info->__partner_1.__career);
    }
    return 0;
}

int WeddingMonitor::notify_to_cruise(WeddingDetail *wedding_info, const int wedding_type)
{
    // 通知退出动画播放
    LogicPlayer *player = NULL;
    if (this->monitor()->find_player(wedding_info->__partner_1.__role_id, player) == 0)
    {
        wedding_info->__partner_1.__role_name = player->role_detail().__name;
        player->respond_to_client(ACTIVE_WEDDING_CRUISE);
    }
    if (this->monitor()->find_player(wedding_info->__partner_2.__role_id, player) == 0)
    {
        wedding_info->__partner_2.__role_name = player->role_detail().__name;
        player->respond_to_client(ACTIVE_WEDDING_CRUISE);
    }

    this->shout_player_cruise_start(wedding_info, wedding_type);

    // 设置为花车巡游状态
    this->set_cruise_state(wedding_info);

    Proto30101608 inner_req;
    inner_req.set_wedding_id(wedding_info->__wedding_id);
    inner_req.set_wedding_type(wedding_type);
    if (wedding_info->__partner_1.__sex == GameEnum::SEX_MALE)
    {
    	inner_req.set_partner_id_1(wedding_info->__partner_1.__role_id);
    	inner_req.set_partner_career_1(wedding_info->__partner_1.__career);
    	inner_req.set_partner_id_2(wedding_info->__partner_2.__role_id);
    	inner_req.set_partner_career_2(wedding_info->__partner_2.__career);
    }
    else
    {
    	inner_req.set_partner_id_1(wedding_info->__partner_2.__role_id);
    	inner_req.set_partner_career_1(wedding_info->__partner_2.__career);
    	inner_req.set_partner_id_2(wedding_info->__partner_1.__role_id);
    	inner_req.set_partner_career_2(wedding_info->__partner_1.__career);
    }

    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    int float_sort = wedding_json["wedding_type"][wedding_type - 1]["float_sort"].asInt();
    inner_req.set_monster_sort(float_sort);

    const Json::Value &npc_loc_json = wedding_json["npc_locate"];
    int scene_id = npc_loc_json[0u].asInt();
    inner_req.set_scene_id(scene_id);

    MSG_USER("logic notify to start float[%ld] %ld %s <-> %ld %s", wedding_info->__wedding_id,
            wedding_info->__partner_1.__role_id, wedding_info->__partner_1.__role_name.c_str(),
            wedding_info->__partner_2.__role_id, wedding_info->__partner_2.__role_name.c_str());

    return this->monitor()->dispatch_to_scene(scene_id, &inner_req);
}

int WeddingMonitor::check_and_update_cartoon_list(const Time_Value &nowtime)
{
    std::vector<Int64> remove_id;
    for (WeddingTypeMap::iterator iter = this->cartoon_play_map_.begin(); 
            iter != this->cartoon_play_map_.end(); ++iter)
    {
        Int64 wedding_id = iter->first;
        WeddingDetail *wedding_info = this->find_wedding_detail(wedding_id);
        if (wedding_info == NULL)
            remove_id.push_back(wedding_id);
        JUDGE_CONTINUE(wedding_info != NULL && wedding_info->__wedding_cartoon_tick <= nowtime);

        this->notify_to_cruise(wedding_info, iter->second);

        remove_id.push_back(wedding_id);
    }
    for (std::vector<Int64>::iterator iter = remove_id.begin(); iter != remove_id.end(); ++iter)
    {
        this->cartoon_play_map_.erase(*iter);
    }
    return 0;
}

int WeddingMonitor::shout_player_cruise_start(WeddingDetail *wedding_info, const int wedding_type)
{
	/*
	{
		Proto80101405 respond;
		respond.set_wedding_type(wedding_type);
		LogicMonitor::PlayerMap &player_map = this->monitor()->player_map();

		for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
				iter != player_map.end(); ++iter)
		{
			iter->second->respond_to_client(ACTIVE_WEDDING_FLOWER, &respond);
		}
	}
	*/

//    const Json::Value &wedding_name_json = CONFIG_INSTANCE->tiny("wedding_type");
//
//    BrocastParaVec para_vec;
//    GameCommon::push_brocast_para_role_detail(para_vec, wedding_info->__partner_1.__role_id, wedding_info->__partner_1.__role_name, true);
//    GameCommon::push_brocast_para_role_detail(para_vec, wedding_info->__partner_2.__role_id, wedding_info->__partner_2.__role_name, true);
//    GameCommon::push_brocast_para_string(para_vec, wedding_name_json[wedding_type - 1].asString());
//
//    return this->monitor()->announce_world(SHOUT_ALL_CRUISE, para_vec);
	return 0;
}

int WeddingMonitor::request_save_all_wedding_info(void)
{
    // update and remove set;
    WeddingDetail *wedding_info = NULL;
    MongoDataMap *data_map = NULL;
//    for (LongSet::iterator iter = this->update_id_set_.begin();
//            iter != this->update_id_set_.end(); ++iter)
    for (WeddingMap::iterator it = this->wedding_by_id_map_.begin();
    		it != this->wedding_by_id_map_.end(); ++it)
    {
        //wedding_info = this->find_wedding_detail(*iter);
    	wedding_info = it->second;
//        JUDGE_CONTINUE(wedding_info != NULL);

        data_map = POOL_MONITOR->mongo_data_map_pool()->pop();

        MMOWedding::update_data(wedding_info, data_map);

        TRANSACTION_MONITOR->request_mongo_transaction(wedding_info->__wedding_id, TRANS_SAVE_WEDDING_INFO, data_map);
    }

    for (LongSet::iterator iter = this->remove_id_set_.begin();
            iter != this->remove_id_set_.end(); ++iter)
    {
        data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
        MMOWedding::remove_data(*iter, data_map);
        TRANSACTION_MONITOR->request_mongo_transaction(*iter, TRANS_REMOVE_WEDDING_INFO, data_map);
    }
 
    this->update_id_set_.clear();
    this->remove_id_set_.clear();

    return 0;
}

