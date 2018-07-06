/*
 * LogicWeddingPlayer.cpp
 *
 * Created on: 2015-06-03 19:40
 *     Author: lyz
 */

#include "LogicWeddingPlayer.h"
#include "LogicPlayer.h"
#include "WeddingMonitor.h"
#include "ProtoDefine.h"
#include "MapStruct.h"
#include "LogicMonitor.h"


LogicWeddingPlayer::LogicWeddingPlayer(void)
{ /*NULL*/ }

LogicWeddingPlayer::~LogicWeddingPlayer(void)
{ /*NULL*/ }

void LogicWeddingPlayer::reset_wedding(void)
{
    this->player_wedding_detail_.reset();
    this->wedding_req_tick_ = Time_Value::zero;
    this->is_login_ = true;
}

bool LogicWeddingPlayer::is_has_wedding(void)
{
    return WEDDING_MONITOR->is_has_wedding(this->role_id());
}

bool LogicWeddingPlayer::is_wedding_prepare(void)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    JUDGE_RETURN(wedding_info != NULL, false);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(wedding_info->__wedding_cartoon_tick > nowtime ||
            wedding_info->__cruise_tick > nowtime, false);

    return true;
}

Int64 LogicWeddingPlayer::wedding_id(void)
{
    return this->player_wedding_detail_.__wedding_id;
}

int LogicWeddingPlayer::day_wedding_times(void)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    JUDGE_RETURN(wedding_info != NULL, 0);

    return wedding_info->__day_wedding_times;
}

WeddingDetail* LogicWeddingPlayer::wedding_detail(void)
{
    return WEDDING_MONITOR->fetch_wedding_detail(this->role_id());
}

PlayerWeddingDetail &LogicWeddingPlayer::self_wedding_info(void)
{
    return this->player_wedding_detail_;
}

int LogicWeddingPlayer::RefreshTimer::type(void)
{
	return GTT_LOGIC_ONE_SEC;
}

int LogicWeddingPlayer::RefreshTimer::handle_timeout(const Time_Value &tv)
{
	return this->player_->wedding_online_work();
}

void LogicWeddingPlayer::RefreshTimer::set_father(LogicWeddingPlayer* player)
{
	this->player_ = player;
}

int LogicWeddingPlayer::wedding_online_work(void)
{
	WeddingDetail *wedding_info = this->wedding_detail();
	if (wedding_info == NULL)
	{
		this->refresh_timer_.cancel_timer();
		return 0;
	}

	LogicPlayer *partner = this->fetch_wedding_partner();
	if (partner == NULL)
	{
		this->refresh_timer_.cancel_timer();
		return 0;
	}

    WeddingDetail::WeddingRole *self_info = NULL;
    WeddingDetail::WeddingRole *side_info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	self_info = &wedding_info->__partner_1;
    	side_info = &wedding_info->__partner_2;
    }
    else
    {
    	self_info = &wedding_info->__partner_2;
    	side_info = &wedding_info->__partner_1;
    }

    if (self_info != NULL && side_info != NULL)
    {
    	int self_sweet_num = CONFIG_INSTANCE->wedding_base()["sweet_num"].asInt()
    			+ this->get_vip_sweet_num();
    	int side_sweet_num = CONFIG_INSTANCE->wedding_base()["sweet_num"].asInt()
    			+ partner->get_vip_sweet_num();

    	self_info->__sweet_degree += self_sweet_num;
    	side_info->__sweet_degree += side_sweet_num;

    	this->record_other_serial(SWEET_DEGREE_SERIAL, self_info->__sweet_degree, self_sweet_num);
    	partner->record_other_serial(SWEET_DEGREE_SERIAL, side_info->__sweet_degree, side_sweet_num);
    }

	this->refresh_timer_.cancel_timer();
	this->refresh_timer_.schedule_timer(CONFIG_INSTANCE->wedding_base()["sweet_time"].asInt());

	this->request_wedding_pannel();
	partner->request_wedding_pannel();
	return 0;
}

int LogicWeddingPlayer::refresh_player_wedding_info(void)
{
	WeddingDetail *wedding_info = this->wedding_detail();
	JUDGE_RETURN(wedding_info != NULL, 0);

    WeddingDetail::WeddingRole *self_info = NULL;
    WeddingDetail::WeddingRole *side_info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	self_info = &wedding_info->__partner_1;
    	side_info = &wedding_info->__partner_2;
    }
    else
    {
    	self_info = &wedding_info->__partner_2;
    	side_info = &wedding_info->__partner_1;
    }

    self_info->__ring_level = this->get_ring_level();
    self_info->__sys_level = this->get_sys_level();
    self_info->__tree_level = this->get_tree_level();

    LogicPlayer* partner = this->fetch_wedding_partner();
    if (partner != NULL)
    {
        side_info->__ring_level = partner->get_ring_level();
        side_info->__sys_level = partner->get_sys_level();
        side_info->__tree_level = partner->get_tree_level();

        partner->self_wedding_info().__wedding_pro_map[WED_RING].__side_level = self_info->__ring_level;
        partner->self_wedding_info().__wedding_pro_map[WED_SYS].__side_level = self_info->__sys_level;
        partner->self_wedding_info().__wedding_pro_map[WED_TREE].__side_level = self_info->__tree_level;
    }

    this->self_wedding_info().__wedding_pro_map[WED_RING].__side_level = side_info->__ring_level;
    this->self_wedding_info().__wedding_pro_map[WED_SYS].__side_level = side_info->__sys_level;
    this->self_wedding_info().__wedding_pro_map[WED_TREE].__side_level = side_info->__tree_level;

    this->logic_player()->role_detail().__wedding_id = wedding_info->__wedding_id;
	return 0;
}

int LogicWeddingPlayer::wedding_login_out(void)
{
	WeddingDetail *wedding_info = this->wedding_detail();
	JUDGE_RETURN(wedding_info != NULL, 0);

	LogicPlayer *partner = this->fetch_wedding_partner();
	JUDGE_RETURN (partner != NULL, 0);

	if (this->get_ring_level() > 0 && partner->get_ring_level() > 0)
	{
		int fina_level = std::min(this->get_ring_level(), partner->get_ring_level());
		int buff_id = CONFIG_INSTANCE->wedding_base()["ring_buff_id"].asInt() + fina_level / 10 + 1;
		int buff_time = Time_Value::DAY * 180;
		int status = 0;	//移除
		this->logic_player()->sync_update_buff_info(status, buff_id, buff_time);
		partner->sync_update_buff_info(status, buff_id, buff_time);
	}
	return 0;
}

int LogicWeddingPlayer::wedding_login_in(void)
{
	this->refresh_timer_.set_father(this);

	WeddingDetail* wedding_info = this->wedding_detail();
	if (wedding_info == NULL)
	{
		if (this->player_wedding_detail_.__wedding_id > 0)
		{
			this->player_wedding_detail_.__wedding_id = -1;
		}

	    this->divorce_sync_property();
		this->sync_wedding_info_to_map(this->player_wedding_detail_.__wedding_id, "");
		return 0;
	}

	//名字更新
	WeddingDetail::WeddingRole *self_role = NULL;
	WeddingDetail::WeddingRole *side_role = NULL;

    if (wedding_info->__partner_1.__role_id == this->logic_player()->role_id())
    {
    	self_role = &wedding_info->__partner_1;
    	side_role = &wedding_info->__partner_2;
    }
    else
    {
    	self_role = &wedding_info->__partner_2;
    	side_role = &wedding_info->__partner_1;
    }
    self_role->__role_name = this->logic_player()->role_detail().__name;
    string name = side_role->__role_name;
	LogicPlayer *partner = this->fetch_wedding_partner();
	if (partner != NULL)
	{
		side_role->__role_name = partner->role_detail().__name;

		//同步partner 信息
		Int64 teamer_id = 0;

		partner->refresh_player_wedding_info();
		partner->sync_wedding_property();
		partner->check_award_label(wedding_info);
		partner->sync_wedding_info_to_map(wedding_info->__wedding_type, this->role_detail().__name);
		name = partner->role_detail().__name;
		teamer_id = partner->role_id();
	}

    this->check_award_label(wedding_info);
    this->sync_wedding_info_to_map(wedding_info->__wedding_type, name);
	//同步partner 信息

	this->refresh_player_wedding_info();
	this->sync_wedding_property();
	this->refresh_timer_.cancel_timer();

	if (partner != NULL)
	{
		this->refresh_timer_.schedule_timer(CONFIG_INSTANCE->wedding_base()["sweet_time"].asInt());
		partner->refresh_player_wedding_info();
		partner->sync_wedding_property();
		partner->request_wedding_pannel();

		//戒指buff
		if (this->get_ring_level() > 0 && partner->get_ring_level() > 0)
		{
			int fina_level = std::min(this->get_ring_level(), partner->get_ring_level());
			int buff_id = CONFIG_INSTANCE->wedding_base()["ring_buff_id"].asInt() + fina_level / 10 + 1;
			int buff_time = Time_Value::DAY * 180;
			int status = 1;	//获得
			this->logic_player()->sync_update_buff_info(status, buff_id, buff_time);
			partner->sync_update_buff_info(status, buff_id, buff_time);
		}
	}

	return 0;
}

int LogicWeddingPlayer::get_vip_sweet_num()
{
	JUDGE_RETURN(this->is_vip() == false, 0);

	const Json::Value& conf = this->vip_detail().conf();
	return conf["sweet_num"].asInt();
}

int LogicWeddingPlayer::get_ring_level()
{
	return this->player_wedding_detail_.__wedding_pro_map[WED_RING].__level;
}

int LogicWeddingPlayer::get_sys_level()
{
	return this->player_wedding_detail_.__wedding_pro_map[WED_SYS].__level;
}

int LogicWeddingPlayer::get_tree_level()
{
	return this->player_wedding_detail_.__wedding_pro_map[WED_TREE].__level;
}

Int64 LogicWeddingPlayer::fetch_wedding_partner_id(void)
{
	WeddingDetail *wedding_info = this->wedding_detail();
	JUDGE_RETURN(wedding_info != NULL, 0);

	Int64 partner_id = wedding_info->__partner_1.__role_id;
	if (partner_id == this->role_id())
		partner_id = wedding_info->__partner_2.__role_id;
	return partner_id;
}

LogicPlayer *LogicWeddingPlayer::fetch_wedding_partner(void)
{
	Int64 id = this->fetch_wedding_partner_id();
	JUDGE_RETURN(id > 0, NULL);
	return this->find_player(id);
}

int LogicWeddingPlayer::intimacy(const Int64 partner_id)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    if (wedding_info != NULL && (wedding_info->__partner_1.__role_id == partner_id || wedding_info->__partner_2.__role_id == partner_id))
        return wedding_info->__intimacy;

    PlayerWeddingDetail::RoleValueMap::iterator iter = this->player_wedding_detail_.__intimacy_map.find(partner_id);
    if (iter != this->player_wedding_detail_.__intimacy_map.end())
        return iter->second;
    return 0;
}

int LogicWeddingPlayer::set_intimacy(const Int64 partner_id, int value)
{
	JUDGE_RETURN(this->player_wedding_detail_.__intimacy_map.count(partner_id) > 0, 0);

	this->player_wedding_detail_.__intimacy_map[partner_id] = value;
	return 0;
}

int LogicWeddingPlayer::total_recv_flower(void)
{
    return this->player_wedding_detail_.__total_recv_flower;
}

int LogicWeddingPlayer::total_send_flower(void)
{
    return this->player_wedding_detail_.__total_send_flower;
}

int LogicWeddingPlayer::act_total_recv_flower(void)
{
	return this->player_wedding_detail_.__act_total_recv_flower;
}
int LogicWeddingPlayer::act_total_send_flower(void)
{
	return this->player_wedding_detail_.__act_total_send_flower;
}

int LogicWeddingPlayer::request_wedding_before(Message *msg)
{
    Proto50101401 respond;
    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();

    respond.set_wed_limit(1);
//    respond.set_sex_limit(1);
    respond.set_level_limit(1);
    respond.set_online_limit(1);
    respond.set_intimacy_limit(1);
    respond.set_ring_limit(1);
    respond.set_team_limit(1);
    respond.set_side_ring_level(-1);
    respond.set_self_ring_level(-1);
    if (player->is_has_ring())
    {
    	respond.set_self_ring_level(player->get_ring_level());
    }

    //是否结婚
    if (this->is_has_wedding() == false)
    {
        respond.set_wed_limit(0);
    }
    else
    {
//    	return this->respond_to_client(RETURN_WEDDING_BEFORE, &respond);
    }

    //是否组队
    if (team_info != NULL && team_info->teamer_set_.size() == 2)
    {
    	respond.set_team_limit(0);
    }
    else
    {
    	return this->respond_to_client(RETURN_WEDDING_BEFORE, &respond);
    }

    //CONDITION_NOTIFY_RETURN(team_info->leader_id_ == this->role_id(), RETURN_WEDDING_BEFORE, ERROR_NO_LEADER);

    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;
        
        partner_id = *iter;
        break;
    }

    // 是否在线
    LogicPlayer *partner = this->find_player(partner_id);
    if(partner != NULL)
    {
    	respond.set_online_limit(0);
    }
    else
    {
    	return this->respond_to_client(RETURN_WEDDING_BEFORE, &respond);
    }

	respond.set_sex_limit(partner->role_detail().__sex);
    respond.set_partner_id(partner->role_id());
    respond.set_partnet_name(partner->role_detail().__name);
    if (partner->is_has_ring())
    {
    	respond.set_side_ring_level(partner->get_ring_level());
    }
    //等级 及 异性
    int ret = this->validate_can_wedding(partner_id);
    if (ret == 0 || ret == ERROR_HAS_WEDDING || ret == ERROR_WRONG_PARTNER_ID)
    {
    	respond.set_level_limit(0);
    }

    // 结婚对象是否结婚
    if (partner->is_has_wedding() == false)
    {
    	respond.set_wed_limit(0);
    }
    else
    {
    	respond.set_wed_limit(1);
    }

    //亲密度
    if (this->intimacy(partner_id) >= 99)
    {
    	respond.set_intimacy_limit(0);
    }

    if (this->is_has_ring() && partner->is_has_ring())
    {
    	respond.set_ring_limit(0);
    }

    if (team_info->leader_id_ == this->role_id())
    	this->player_wedding_detail_.__wedding_reply.clear();
    else
    	partner->player_wedding_detail_.__wedding_reply.clear();

    ProtoThreeObj* other_info = respond.mutable_other_info();
    other_info->set_id(this->player_wedding_detail_.__wedding_id);
    other_info->set_tick(partner->player_wedding_detail_.__wedding_id);
    player->respond_to_client(RETURN_WEDDING_BEFORE, &respond);
    return 0;
}

int LogicWeddingPlayer::sync_wedding_remove_item(int type, IntMap &item_id, IntMap &item_num)
{
	Proto31400048 sync_info;
	for (int i = 0; i < (int)item_id.size(); ++i)
	{
		ProtoItem* item = sync_info.add_item_list();
		item->set_id(item_id[i]);
		item->set_amount(item_num[i]);
	}

	sync_info.mutable_other_info()->set_id(type);
	LOGIC_MONITOR->dispatch_to_scene(this, &sync_info);
	return 0;
}

int LogicWeddingPlayer::update_wedding_property(int type, int exp)
{
	PlayerWeddingDetail::wedding_property &info = this->player_wedding_detail_.__wedding_pro_map[type];

	int level = info.__level;

	int pre_buff_id = 0;
	if (type == WED_RING)
	{
		LogicPlayer* partner = this->fetch_wedding_partner();
		if (partner != NULL)
		{
			//戒指buff
			if (this->get_ring_level() > 0 && partner->get_ring_level() > 0)
			{
				int fina_level = std::min(this->get_ring_level(), partner->get_ring_level());
				pre_buff_id = CONFIG_INSTANCE->wedding_base()["ring_buff_id"].asInt() + fina_level / 10 + 1;
			}
		}
	}
	const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, level);
	JUDGE_RETURN(pro_json != Json::Value::null, ERROR_CLIENT_OPERATE);

	/*
	int item_size = (int)pro_json["update_item"].size();
	int item_exp = 0;
	for (int i = 0; i < item_size; ++i)
	{
		if (pro_json["update_item"][i][0u].asInt() == item_id)
		{
			item_exp = pro_json["update_item"][i][1u].asInt();
			break;
		}
	}

	JUDGE_RETURN(item_exp > 0, ERROR_CLIENT_OPERATE);
	info.__exp += item_exp * item_num;
	*/
	info.__exp += exp;
	int upgrade_exp = CONFIG_INSTANCE->wedding_property(type, level)["exp"].asInt();

	int is_most = 0;
	while (info.__exp >= upgrade_exp)
	{
		info.__exp -= upgrade_exp;
		level++;
		const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, level + 1);
		if (pro_json == Json::Value::null)
		{
			is_most = 1;
			break;
		}
		upgrade_exp = CONFIG_INSTANCE->wedding_property(type, level)["exp"].asInt();
	}
	info.__level = level;
	this->sync_wedding_property();

	LogicPlayer* partner = this->fetch_wedding_partner();
	if (partner != NULL)
	{
		partner->sync_wedding_property();
		partner->request_wedding_pannel();
	}

	this->sync_wedding_property();

	int now_buff_id = 0;
	if (type == WED_RING)
	{
		LogicPlayer* partner = this->fetch_wedding_partner();
		if (partner != NULL)
		{
			//戒指buff
			if (this->get_ring_level() > 0 && partner->get_ring_level() > 0)
			{
				int fina_level = std::min(this->get_ring_level(), partner->get_ring_level());
				now_buff_id = CONFIG_INSTANCE->wedding_base()["ring_buff_id"].asInt() + fina_level / 10 + 1;
			}
		}
	}

	if (pre_buff_id != now_buff_id && pre_buff_id > 0 && now_buff_id > 0)
	{
		int buff_time = Time_Value::DAY * 180;
		int status = 0;	//移除
		this->logic_player()->sync_update_buff_info(status, pre_buff_id, buff_time);
		partner->sync_update_buff_info(status, pre_buff_id, buff_time);

		status = 1;
		this->logic_player()->sync_update_buff_info(status, now_buff_id, buff_time);
		partner->sync_update_buff_info(status, now_buff_id, buff_time);
	}

	this->refresh_label_info();
	this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
	return this->request_wedding_pannel();
}

int LogicWeddingPlayer::update_couple_fb_times()
{
	LogicPlayer* player = this->fetch_wedding_partner();
	JUDGE_RETURN(player != NULL, 0);

	Proto30400915 inner;
	inner.set_prev_scene_id(player->scene_id());
	return LOGIC_MONITOR->dispatch_to_scene(player, 20501, &inner);
}

int LogicWeddingPlayer::request_update_ring_begin(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101410 *, request, msg, RETURN_UPDATE_RING);

 //   int item_id = request->item_id();
    int type = WED_RING;
 //   int item_num = request->item_num();

    IntMap id_list, num_list;

    for (int i = 0; i < request->item_id_size(); ++i)
    {
    	id_list[i] = request->item_id(i);
    	num_list[i] = request->item_num(i);
    }
    this->sync_wedding_remove_item(type, id_list, num_list);
//    this->update_wedding_property(type, item_id, item_num);

    return 0;
}

int LogicWeddingPlayer::request_buy_wedding_treasures_begin(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10101413 *, request, msg, RETURN_BUY_WEDDING_TREASURES);

    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, RETURN_BUY_WEDDING_TREASURES, ERROR_NO_WEDDING);
    WeddingDetail::WeddingRole *info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	info = &wedding_info->__partner_2;
    }
    else
    {
    	info = &wedding_info->__partner_1;
    }

    CONDITION_NOTIFY_RETURN(info->__tick == 0, RETURN_BUY_WEDDING_TREASURES, ERROR_CLIENT_OPERATE);

	Proto31400049 sync_info;
	sync_info.set_type(request->type());

	LOGIC_MONITOR->dispatch_to_scene(this, &sync_info);
	return 0;
}

int LogicWeddingPlayer::request_buy_wedding_treasures_end(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400049 *, request, RETURN_BUY_WEDDING_TREASURES);

    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, RETURN_BUY_WEDDING_TREASURES, ERROR_NO_WEDDING);

    WeddingDetail::WeddingRole *info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	info = &wedding_info->__partner_2;
    }
    else
    {
    	info = &wedding_info->__partner_1;
    }

    CONDITION_NOTIFY_RETURN(info->__tick == 0, RETURN_BUY_WEDDING_TREASURES, ERROR_CLIENT_OPERATE);
    info->__tick = Time_Value::gettimeofday().sec();
    info->__once_reward = 0;
    info->__left_times = CONFIG_INSTANCE->wedding_treasures(1)["keep_time"].asInt();

    Proto50101413 respond;
    respond.set_type(1);

    this->request_wedding_pannel();
    LogicPlayer* player = this->fetch_wedding_partner();
    if (player != NULL)	player->request_wedding_pannel();

    //购买邮件
    int mail_id = CONFIG_INSTANCE->wedding_base()["buy_treasures_mail_id"].asInt();
    Int64 player_id = this->fetch_wedding_partner_id();
	MailInformation *mail_info = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info->mail_content_.c_str(), this->role_detail().__name.c_str());

	GameCommon::request_save_mail_content(player_id, mail_info);

    return this->respond_to_client(RETURN_BUY_WEDDING_TREASURES, &respond);
}

int LogicWeddingPlayer::request_fetch_wedding_treasures(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10101414 *, request, msg, RETURN_FETCH_WEDDING_TREASURES);

	int type = request->type();
    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, RETURN_FETCH_WEDDING_TREASURES, ERROR_NO_WEDDING);
    WeddingDetail::WeddingRole *info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	info = &wedding_info->__partner_1;
    }
    else
    {
    	info = &wedding_info->__partner_2;
    }

    CONDITION_NOTIFY_RETURN(info->__tick != 0, RETURN_FETCH_WEDDING_TREASURES, ERROR_CLIENT_OPERATE);

    if (type == 1)
    {
    	const Json::Value &info_json = CONFIG_INSTANCE->wedding_treasures(1);
    	CONDITION_NOTIFY_RETURN(info_json.empty() == false, RETURN_FETCH_WEDDING_TREASURES,
    			ERROR_CONFIG_NOT_EXIST);

        CONDITION_NOTIFY_RETURN(info->__once_reward == 0, RETURN_FETCH_WEDDING_TREASURES, ERROR_AWARD_HAS_GET);
        int item_id = info_json["once_reward"][0u].asInt();
        int item_num = info_json["once_reward"][1u].asInt();
        int item_bind = info_json["once_reward"][2u].asInt();

        this->logic_player()->request_add_item(ADD_FROM_WEDDING_TREASURES, item_id, item_num, item_bind);
        info->__once_reward = 1;
    }
    else
    {
    	int id = CONFIG_INSTANCE->wedding_treasures(1)["keep_time"].asInt() - info->__left_times + 1;
    	const Json::Value &info_json = CONFIG_INSTANCE->wedding_treasures(id);
    	CONDITION_NOTIFY_RETURN(info_json.empty() == false, RETURN_FETCH_WEDDING_TREASURES,
    	    	ERROR_CONFIG_NOT_EXIST);

        CONDITION_NOTIFY_RETURN(is_same_day(Time_Value(info->__fetch_tick), Time_Value::gettimeofday()) == false,
        		RETURN_FETCH_WEDDING_TREASURES, ERROR_AWARD_HAS_GET);
        int reward_id = info_json["day_reward"].asInt();
        this->logic_player()->request_add_reward(reward_id, ADD_FROM_WEDDING_TREASURES);

        info->__fetch_tick = Time_Value::gettimeofday().sec();

        //最后一天领完清空
        info->__left_times--;
        if (info->__left_times == 0)
        {
        	info->__fetch_tick = 0;
        	info->__tick = 0;
        	info->__once_reward = 0;
        }
    }

    Proto50101414 respond;
    respond.set_type(request->type());

    this->request_wedding_pannel();
    LogicPlayer* player = this->fetch_wedding_partner();
    if (player != NULL)	player->request_wedding_pannel();
	return this->respond_to_client(RETURN_FETCH_WEDDING_TREASURES, &respond);
}

int LogicWeddingPlayer::request_wedding_label_info()
{
	Proto50101415 respond;
	respond.set_ring_level(this->get_ring_level());
	respond.set_sys_level(this->get_sys_level());
	respond.set_tree_level(this->get_tree_level());

	int size = CONFIG_INSTANCE->wedding_base()["wed_label_num"].asInt();
	for (int i = 1; i <= size; ++i)
	{
		if (this->player_wedding_detail_.__wedding_label_map[i] == 0)
		{
			this->player_wedding_detail_.__wedding_label_map[i] = WED_NONE;
		}
		const Json::Value &wed_label = CONFIG_INSTANCE->wedding_label(i);

		ProtoWedLabel* info = respond.add_label_list();
		info->set_id(i);
		info->set_label_id(wed_label["label_id"].asInt());
		info->set_ring_level_limit(wed_label["ring_level"].asInt());
		info->set_sys_level_limit(wed_label["sys_level"].asInt());
		info->set_tree_level_limit(wed_label["tree_level"].asInt());
		info->set_status(this->player_wedding_detail_.__wedding_label_map[i]);
	}
	return this->respond_to_client(RETURN_WEDDING_LABEL_INFO, &respond);
}

int LogicWeddingPlayer::request_wedding_get_label(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10101416 *, request, msg, RETURN_WEDDING_GET_LABEL);

	int id = request->id();
	CONDITION_NOTIFY_RETURN(this->player_wedding_detail_.__wedding_label_map[id] == WED_HAVE,
			RETURN_WEDDING_GET_LABEL, ERROR_CLIENT_OPERATE);
	const Json::Value &wed_label = CONFIG_INSTANCE->wedding_label(id);
	int item_id = wed_label["label_item"][0u].asInt();
	int item_num = wed_label["label_item"][1u].asInt();
	int item_bind = wed_label["label_item"][2u].asInt();
	this->player_wedding_detail_.__wedding_label_map[id] = WED_GONE;
	this->logic_player()->request_add_item(ADD_FROM_WEDDING_LABEL, item_id, item_num, item_bind);

	Proto50101416 respond;
	respond.set_id(id);
	respond.set_label_id(wed_label["label_id"].asInt());
	respond.set_status(WED_GONE);

	this->refresh_label_info();
	return this->respond_to_client(RETURN_WEDDING_LABEL_INFO, &respond);
}

int LogicWeddingPlayer::refresh_label_info()
{
	int size = CONFIG_INSTANCE->wedding_base()["wed_label_num"].asInt();
	for (int i = 1; i <= size; ++i)
	{
		if (this->player_wedding_detail_.__wedding_label_map[i] == 0)
		{
			this->player_wedding_detail_.__wedding_label_map[i] = WED_NONE;
		}
		JUDGE_CONTINUE(this->player_wedding_detail_.__wedding_label_map[i] < WED_HAVE);
		const Json::Value &wed_label = CONFIG_INSTANCE->wedding_label(i);
		if (this->get_ring_level() >= wed_label["ring_level"].asInt() &&
			this->get_sys_level() >= wed_label["sys_level"].asInt() &&
			this->get_tree_level() >= wed_label["tree_level"].asInt())
		{
			this->player_wedding_detail_.__wedding_label_map[i] = WED_HAVE;
		}
	}

	return	this->request_wedding_label_info();
}

int LogicWeddingPlayer::request_update_ring_or_tree_end(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400048*, request, -1);

	int type = request->other_info().id();
	PlayerWeddingDetail::wedding_property &info = this->player_wedding_detail_.__wedding_pro_map[type];

	int level = info.__level + 1;
	const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, level);
	JUDGE_RETURN(pro_json != Json::Value::null, ERROR_CLIENT_OPERATE);

	int exp = 0;
	for (int i = 0; i < request->item_list_size(); ++i)
	{
		int item_size = (int)pro_json["update_item"].size();
		int item_exp = 0;
		int item_id = request->item_list(i).id();
		int item_num = request->item_list(i).amount();

		for (int i = 0; i < item_size; ++i)
		{
			if (pro_json["update_item"][i][0u].asInt() == item_id)
			{
				item_exp = pro_json["update_item"][i][1u].asInt();
				break;
			}
		}

		JUDGE_RETURN(item_exp > 0, ERROR_CLIENT_OPERATE);
		exp += item_exp * item_num;
	}

	this->update_wedding_property(type, exp);
	return 0;
}

int LogicWeddingPlayer::request_update_sys(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101411 *, request, msg, RETURN_UPDATE_SYS);

//    int item_id = request->item_id();
    int type = WED_SYS;
//    int item_num = request->item_num();

    WeddingDetail *wedding_info = this->wedding_detail();
    if (wedding_info != NULL)
    {
		PlayerWeddingDetail::wedding_property &info = this->player_wedding_detail_.__wedding_pro_map[type];
		const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, info.__level);
		CONDITION_NOTIFY_RETURN(pro_json != Json::Value::null,
		    	    		RETURN_UPDATE_SYS, ERROR_CONFIG_NOT_EXIST);
		int exp = pro_json["exp"].asInt();

    	if (this->role_id() == wedding_info->__partner_1.__role_id)
    	{
    	    CONDITION_NOTIFY_RETURN(wedding_info->__partner_1.__sweet_degree >= exp,
    	    		RETURN_UPDATE_SYS, ERROR_CLIENT_OPERATE);

    		wedding_info->__partner_1.__sweet_degree -= exp;
    		this->record_other_serial(SWEET_DEGREE_SERIAL, wedding_info->__partner_1.__sweet_degree, -exp);
    		this->update_wedding_property(type, exp);
    	}
    	else
    	{
    	    CONDITION_NOTIFY_RETURN(wedding_info->__partner_2.__sweet_degree >= exp,
    	    		RETURN_UPDATE_SYS, ERROR_CLIENT_OPERATE);

    		wedding_info->__partner_2.__sweet_degree -= exp;
    		this->record_other_serial(SWEET_DEGREE_SERIAL, wedding_info->__partner_2.__sweet_degree, -exp);
    		this->update_wedding_property(type, exp);
    	}
    }

    return 0;
}

int LogicWeddingPlayer::request_update_tree_begin(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101412 *, request, msg, RETURN_UPDATE_TREE);

//    int item_id = request->item_id();
    int type = WED_TREE;
//    int item_num = request->item_num();

    IntMap id_list, num_list;
    for (int i = 0; i < request->item_id_size(); ++i)
    {
    	id_list[i] = request->item_id(i);
    	num_list[i] = request->item_num(i);
    }

    this->sync_wedding_remove_item(type, id_list, num_list);
//    this->update_wedding_property(type, item_id, item_num);
    return 0;
}

int LogicWeddingPlayer::request_reply_wedding(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101408 *, request, msg, RETURN_REPLY_WEDDING);
    int reply = request->reply();

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    CONDITION_NOTIFY_RETURN(team_info != NULL, RETURN_REPLY_WEDDING, ERROR_WEDDING_TEAMER_SIZE);
    CONDITION_NOTIFY_RETURN(team_info->teamer_set_.size() == 2, RETURN_REPLY_WEDDING, ERROR_WEDDING_TEAMER_SIZE);

    if (team_info->leader_id_ != this->role_id())
    {
        LogicPlayer *leader = this->find_player(team_info->leader_id_);
        if (leader != NULL)
        {
            leader->update_wedding_reply(this->role_id(), reply);
        }
    }
    else
    {
        this->update_wedding_reply(this->role_id(), reply);
    }

    return this->respond_to_client(RETURN_REPLY_WEDDING);
}

int LogicWeddingPlayer::is_has_ring()
{
	return this->player_wedding_detail_.__is_has_ring;
}

int LogicWeddingPlayer::refresh_ring_info()
{
	int change = 0;
	if (this->player_wedding_detail_.__is_has_ring == 0) change = 1;
	if (change)
	{
		PackageItem item;
		item.set(CONFIG_INSTANCE->wedding_base()["wedding_ring"].asInt(), 1, 1);
		this->request_remove_goods(item, ITEM_REMOVE_RING_FIRST);
	}

	this->player_wedding_detail_.__is_has_ring = 1;
	if (this->player_wedding_detail_.__wedding_pro_map[WED_RING].__level <= 0)
		this->player_wedding_detail_.__wedding_pro_map[WED_RING].__level = 1;

	this->request_wedding_pannel();
	this->request_wedding_before(NULL);

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    JUDGE_RETURN(team_info != NULL && team_info->teamer_set_.size() == 2, 0);
    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;

        partner_id = *iter;
        break;
    }
    LogicPlayer *partner = this->find_player(partner_id);

    if (partner != 0)
    {
    	partner->request_wedding_pannel();
    	partner->request_wedding_before(NULL);
    }
	return 0;
}

int LogicWeddingPlayer::request_wedding_invest(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10101420 *, request, msg, RETURN_WEDING_INVEST);
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding_base();

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_WEDING_INVEST, ERROR_OPERATE_TOO_FAST);

    int wedding_type = request->wedding_type();
    CONDITION_NOTIFY_RETURN(0 < wedding_type && wedding_type <= wedding_json["wedding_type_num"].asInt(), RETURN_WEDING_INVEST, ERROR_CLIENT_OPERATE);

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    CONDITION_NOTIFY_RETURN(team_info != NULL, RETURN_WEDING_INVEST, ERROR_WEDDING_TEAMER_SIZE);
    CONDITION_NOTIFY_RETURN(team_info->leader_id_ == this->role_id(), RETURN_WEDING_INVEST, ERROR_NO_LEADER);
    CONDITION_NOTIFY_RETURN(team_info->teamer_set_.size() == 2, RETURN_WEDING_INVEST, ERROR_WEDDING_TEAMER_SIZE);
//    CONDITION_NOTIFY_RETURN(this->is_has_wedding() == false, RETURN_WEDING_INVEST, ERROR_HAS_WEDDING);

    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;

        partner_id = *iter;
        break;
    }
    int intimacy = this->intimacy(partner_id);
    CONDITION_NOTIFY_RETURN(intimacy >= wedding_json["intimacy_limit"].asInt(), RETURN_WEDING_INVEST, ERROR_INTIMACY_LIMIT_FOR_WEDDING);

    int ret = this->validate_can_wedding(partner_id);
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_WEDING_INVEST, ret);

    LogicPlayer *partner = this->find_player(partner_id);
    CONDITION_NOTIFY_RETURN(player != NULL, RETURN_WEDING_INVEST, ERROR_PLAYER_OFFLINE);
//    CONDITION_NOTIFY_RETURN(partner->is_has_wedding() == false, RETURN_WEDING_INVEST, ERROR_TARGET_HAS_WEDDING);
    CONDITION_NOTIFY_RETURN(player->is_has_ring() && partner->is_has_ring(), RETURN_WEDING_INVEST, ERROR_NO_RING);

    Proto50101420 respond;
    respond.set_lead_id(this->role_id());
    respond.set_lead_name(this->role_detail().__name);
    respond.set_teamer_id(partner->role_id());
    respond.set_teamer_name(partner->role_detail().__name);
    respond.set_wedding_type(wedding_type);
    ProtoThreeObj* info = respond.mutable_other_info();
    info->set_id(0);

    partner->respond_to_client(RETURN_WEDING_INVEST, &respond);

    info->set_id(1);
    return this->respond_to_client(RETURN_WEDING_INVEST, &respond);
}

int LogicWeddingPlayer::request_wedding_reply(Message *msg)
{
	DYNAMIC_CAST_NOTIFY(Proto10101421 *, request, msg, RETURN_WEDING_REPLY);
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding_base();
    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_WEDING_REPLY, ERROR_OPERATE_TOO_FAST);

    int wedding_type = request->wedding_type();
    CONDITION_NOTIFY_RETURN(0 < wedding_type && wedding_type <= wedding_json["wedding_type_num"].asInt(), RETURN_WEDING_REPLY, ERROR_CLIENT_OPERATE);

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    CONDITION_NOTIFY_RETURN(team_info != NULL, RETURN_WEDING_REPLY, ERROR_WEDDING_TEAMER_SIZE);
//    CONDITION_NOTIFY_RETURN(team_info->leader_id_ == this->role_id(), RETURN_WEDING_REPLY, ERROR_NO_LEADER);
    CONDITION_NOTIFY_RETURN(team_info->teamer_set_.size() == 2, RETURN_WEDING_REPLY, ERROR_WEDDING_TEAMER_SIZE);
//    CONDITION_NOTIFY_RETURN(this->is_has_wedding() == false, RETURN_WEDING_REPLY, ERROR_HAS_WEDDING);

    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;

        partner_id = *iter;
        break;
    }
    int intimacy = this->intimacy(partner_id);
    CONDITION_NOTIFY_RETURN(intimacy >= wedding_json["intimacy_limit"].asInt(), RETURN_WEDING_REPLY, ERROR_INTIMACY_LIMIT_FOR_WEDDING);

    int ret = this->validate_can_wedding(partner_id);
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_WEDING_REPLY, ret);

    LogicPlayer *partner = this->find_player(partner_id);
    CONDITION_NOTIFY_RETURN(player != NULL, RETURN_WEDING_REPLY, ERROR_PLAYER_OFFLINE);
//    CONDITION_NOTIFY_RETURN(partner->is_has_wedding() == false, RETURN_WEDING_REPLY, ERROR_TARGET_HAS_WEDDING);
    CONDITION_NOTIFY_RETURN(player->is_has_ring() && partner->is_has_ring(), RETURN_WEDING_REPLY, ERROR_NO_RING);

	Proto50101421 respond;
	respond.set_lead_id(request->lead_id());
	respond.set_lead_name(request->lead_name());
	respond.set_is_ok(request->is_ok());
	respond.set_teamer_id(request->teamer_id());
	respond.set_teamer_name(request->teamer_name());
	partner->respond_to_client(RETURN_WEDING_REPLY, &respond);

    if (request->is_ok())
    {
    	Proto10101402 respond;
    	respond.set_type(wedding_type);
    	partner->request_wedding(&respond);
    }
    else
    {
//    	this->respond_to_client(RETURN_WEDING_REPLY, &respond);
    }
	return 0;
}

int LogicWeddingPlayer::request_wedding(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101402 *, request, msg, RETURN_WEDDING);

//    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding_base();

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_WEDDING, ERROR_OPERATE_TOO_FAST);

    int wedding_type = request->type();
    CONDITION_NOTIFY_RETURN(0 < wedding_type && wedding_type <= wedding_json["wedding_type_num"].asInt(), RETURN_WEDDING, ERROR_CLIENT_OPERATE);

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    CONDITION_NOTIFY_RETURN(team_info != NULL, RETURN_WEDDING, ERROR_WEDDING_TEAMER_SIZE);
    CONDITION_NOTIFY_RETURN(team_info->leader_id_ == this->role_id(), RETURN_WEDDING, ERROR_NO_LEADER);
    CONDITION_NOTIFY_RETURN(team_info->teamer_set_.size() == 2, RETURN_WEDDING, ERROR_WEDDING_TEAMER_SIZE);
//    CONDITION_NOTIFY_RETURN(this->is_has_wedding() == false, RETURN_WEDDING, ERROR_HAS_WEDDING);

    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;
        
        partner_id = *iter;
        break;
    }
    int intimacy = this->intimacy(partner_id);
    CONDITION_NOTIFY_RETURN(intimacy >= wedding_json["intimacy_limit"].asInt(), RETURN_WEDDING, ERROR_INTIMACY_LIMIT_FOR_WEDDING);

    int ret = this->validate_can_wedding(partner_id);
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_WEDDING, ret);

    LogicPlayer *partner = this->find_player(partner_id);
    CONDITION_NOTIFY_RETURN(player != NULL, RETURN_WEDDING, ERROR_PLAYER_OFFLINE);
//    CONDITION_NOTIFY_RETURN(partner->is_has_wedding() == false, RETURN_WEDDING, ERROR_TARGET_HAS_WEDDING);
    CONDITION_NOTIFY_RETURN(player->is_has_ring() && partner->is_has_ring(), RETURN_WEDDING, ERROR_NO_RING);

    // update req tick;
    this->wedding_req_tick_ = nowtime + GameCommon::fetch_time_value(CONFIG_INSTANCE->const_set("wedding_wait_time"));


    CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(player->scene_id()) &&
    		GameCommon::is_normal_scene(partner->scene_id()), RETURN_WEDDING, ERROR_NORMAL_SCENE);

    Proto30101601 inner_req;
    inner_req.set_res_recogn(RETURN_WEDDING);
    inner_req.set_type(request->type());
    inner_req.add_role_list(this->role_id());
    inner_req.add_role_list(partner_id);
    return this->dispatch_to_map_server(&inner_req);
}

int LogicWeddingPlayer::request_wedding_make_up(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101403 *, request, msg, RETURN_WEDDING_MAKE_UP);

    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    // check req tick;
    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_WEDDING_MAKE_UP, ERROR_OPERATE_TOO_FAST);

    int wedding_type = request->type();
    CONDITION_NOTIFY_RETURN(0 < wedding_type && wedding_type <= int(wedding_json["wedding_type"].size()), RETURN_WEDDING_MAKE_UP, ERROR_CLIENT_OPERATE);

    LogicPlayer *player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    CONDITION_NOTIFY_RETURN(team_info != NULL, RETURN_WEDDING_MAKE_UP, ERROR_WEDDING_TEAMER_SIZE);
    CONDITION_NOTIFY_RETURN(team_info->leader_id_ == this->role_id(), RETURN_WEDDING_MAKE_UP, ERROR_NO_LEADER);
    CONDITION_NOTIFY_RETURN(team_info->teamer_set_.size() == 2, RETURN_WEDDING_MAKE_UP, ERROR_WEDDING_TEAMER_SIZE);

    Int64 partner_id = 0;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        if (*iter == this->role_id())
            continue;
        
        partner_id = *iter;
        break;
    }

    int intimacy = this->intimacy(partner_id);
    CONDITION_NOTIFY_RETURN(intimacy >= wedding_json["intimacy"].asInt(), RETURN_WEDDING, ERROR_INTIMACY_LIMIT_FOR_WEDDING);

    int ret = this->validate_can_wedding(partner_id);
    CONDITION_NOTIFY_RETURN(ret == 0, RETURN_WEDDING_MAKE_UP, ret);

    LogicPlayer *partner = this->find_player(partner_id);
    CONDITION_NOTIFY_RETURN(partner != NULL, RETURN_WEDDING_MAKE_UP, ERROR_PLAYER_OFFLINE);
   
    // check has wedding
    WeddingDetail *wedding_info = this->wedding_detail();
    if (wedding_info != NULL)
    {
        CONDITION_NOTIFY_RETURN(wedding_info->__partner_1.__role_id == partner_id || // NULL
                wedding_info->__partner_2.__role_id == partner_id, 
                RETURN_WEDDING_MAKE_UP, ERROR_TARGET_HAS_WEDDING);
        CONDITION_NOTIFY_RETURN(this->is_wedding_prepare() == false, 
                RETURN_WEDDING_MAKE_UP, ERROR_WEDDING_READY_START);

        // check wedding times;
        this->refresh_day_wedding_times();
        CONDITION_NOTIFY_RETURN(wedding_info->__day_wedding_times < wedding_json["each_day_times"].asInt(), RETURN_WEDDING_MAKE_UP, ERROR_WEDDING_DAY_TIMES);
    }
    else
    {
        CONDITION_NOTIFY_RETURN(partner->is_has_wedding() == false, RETURN_WEDDING, ERROR_TARGET_HAS_WEDDING);
    }

    // update req tick for repeat request;
    this->wedding_req_tick_ = nowtime + GameCommon::fetch_time_value(CONFIG_INSTANCE->const_set("wedding_wait_time"));

    Proto30101601 inner_req;
    inner_req.set_res_recogn(RETURN_WEDDING_MAKE_UP);
    inner_req.set_type(request->type());
    inner_req.add_role_list(this->role_id());
    inner_req.add_role_list(partner_id);
    return this->dispatch_to_map_server(&inner_req);
}

int LogicWeddingPlayer::process_wedding_after_pack_check(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31101602 *, request, -1);

    int res_recogn = request->res_recogn();
    int wedding_type = request->wedding_type();

    Int64 partner_id = 0;
    for (int i = 0; i < request->role_list_size(); ++i)
    {
        JUDGE_CONTINUE(request->role_list(i) != this->role_id());
        partner_id = request->role_list(i);
        break;
    }

    if (partner_id <= 0)
    {
        MSG_USER("ERROR wedding partner id is 0 %ld %s %d", this->role_id(), this->name(), wedding_type);
        return 0;
    }
   
    if (this->is_has_wedding() == false)
    {
        int ret = WEDDING_MONITOR->insert_new_wedding(this->role_id(), partner_id, wedding_type);
        CONDITION_NOTIFY_RETURN(ret == 0, res_recogn, ret);
    }
    else
    {
        int ret = WEDDING_MONITOR->update_wedding(this->role_id(), partner_id, wedding_type);
        CONDITION_NOTIFY_RETURN(ret == 0, res_recogn, ret);
    }

    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, res_recogn, ERROR_NO_WEDDING);

    this->refresh_player_wedding_info();
    LogicPlayer *partner = this->fetch_wedding_partner();
    string name = "";
    Int64 teamer_id = 0;
	if (partner != NULL)
    {
	   partner->refresh_player_wedding_info();
	   partner->sync_wedding_property();
       partner->check_award_label(wedding_info);
       partner->sync_wedding_info_to_map(wedding_info->__wedding_type, this->role_detail().__name);
       name = partner->role_detail().__name;
       teamer_id = partner->role_id();
    }

    this->refresh_player_wedding_info();
    this->sync_wedding_property();
    this->check_award_label(wedding_info);
    this->sync_wedding_info_to_map(wedding_info->__wedding_type, name);
    WEDDING_MONITOR->set_cartoon_state(wedding_info, wedding_type);

    //结婚奖励
    int reward_id = CONFIG_INSTANCE->wedding_base()["wedding_reward"][wedding_type - 1].asInt();
    this->request_add_reward(reward_id, ADD_FROM_WEDDING);
    this->refresh_label_info();
    this->finish_wedding_branch_task();

    if (partner != NULL)
    {
		partner->request_add_reward(reward_id, ADD_FROM_WEDDING);
		partner->refresh_label_info();
		partner->finish_wedding_branch_task();
    }

	this->send_wedding_info_to_travel(wedding_type, partner);

    //传送指定场景
    const Json::Value &wedding_base = CONFIG_INSTANCE->wedding_base();
	Proto10400111 req;
	req.set_scene_id(wedding_base["scene_id"].asInt());
	req.set_pixel_x(wedding_base["scene_pos"][0u][0u].asInt());
	req.set_pixel_y(wedding_base["scene_pos"][0u][1u].asInt());
	this->logic_player()->dispatch_to_map_server(&req);

	if (partner != NULL)
	{
		req.set_pixel_x(wedding_base["scene_pos"][1u][0u].asInt());
		req.set_pixel_y(wedding_base["scene_pos"][1u][1u].asInt());
		partner->dispatch_to_map_server(&req);
	}

	int shout_id = wedding_base["shout_id"][wedding_type - 1].asInt();
    if (shout_id > 0 && partner != NULL)
    {
    	BrocastParaVec para_vec;
    	GameCommon::push_brocast_para_string(para_vec, this->role_detail().__name);
    	GameCommon::push_brocast_para_string(para_vec, partner->role_detail().__name);
    	GameCommon::announce(shout_id, &para_vec);
    }

	//全服特效
	Proto80101405 respond;
	respond.set_wedding_type(wedding_type);

	LogicMonitor::PlayerMap &player_map = this->monitor()->player_map();
	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
			iter != player_map.end(); ++iter)
	{
		iter->second->respond_to_client(ACTIVE_WEDDING_FLOWER, &respond);
	}

	//私聊
	Proto50101402 talk_info;
	talk_info.set_leader_id(this->role_id());
	talk_info.set_leader_name(this->role_detail().__name);
	talk_info.set_teamer_id(teamer_id);
	talk_info.set_teamer_name(name);
	this->logic_player()->respond_to_client(RETURN_WEDDING, &talk_info);
	if (partner != NULL)
	{
		partner->respond_to_client(RETURN_WEDDING, &talk_info);
	}
	/*
    {
        Proto80101401 act_respond;
        if (WEDDING_MONITOR->make_up_cartoon_info(this->role_id(), &act_respond) == 0)
        {
            this->respond_to_client(ACTIVE_WEDDING_CARTOON, &act_respond);
            if (partner != NULL)
                partner->respond_to_client(ACTIVE_WEDDING_CARTOON, &act_respond);
        }
    }
	*/
	this->wedding_login_in();
    this->request_wedding_pannel();
    if (partner != NULL)
	{
    	partner->request_wedding_pannel();
	}

    return 0;
}

int LogicWeddingPlayer::request_divorce(Message *msg)
{
	Time_Value nowtime = Time_Value::gettimeofday();
	CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_DIVORCE, ERROR_OPERATE_TOO_FAST);
//    CONDITION_NOTIFY_RETURN(this->is_has_wedding() == true, RETURN_DIVORCE, ERROR_NO_WEDDING);

    WeddingDetail *wedding_detail = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_detail != NULL, RETURN_DIVORCE, ERROR_NO_WEDDING);
//    CONDITION_NOTIFY_RETURN(this->is_wedding_prepare() == false, RETURN_DIVORCE, ERROR_WEDDING_READY_START);

    this->wedding_req_tick_ = nowtime + GameCommon::fetch_time_value(CONFIG_INSTANCE->const_set("wedding_wait_time"));

    Proto31101603 inner_req;
    return this->dispatch_to_map_server(&inner_req);
}

int LogicWeddingPlayer::process_divorce_after_pack_check(Message *msg)
{
	WeddingDetail *wedding_info = this->wedding_detail();
	JUDGE_RETURN(wedding_info != NULL, 0);

    this->wedding_login_out();

    int mail_id = CONFIG_INSTANCE->wedding_base()["mail"].asInt();
	MailInformation *mail_info_1 = GameCommon::create_sys_mail(mail_id);
	MailInformation *mail_info_2 = GameCommon::create_sys_mail(mail_id);
	::snprintf(mail_info_1->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info_1->mail_content_.c_str(), wedding_info->__partner_2.__role_name.c_str());

	::snprintf(mail_info_2->makeup_content_, GameEnum::MAX_MAIL_CONTENT_LENGTH,
				mail_info_2->mail_content_.c_str(), wedding_info->__partner_1.__role_name.c_str());

	GameCommon::request_save_mail_content(wedding_info->__partner_1.__role_id, mail_info_1);
	GameCommon::request_save_mail_content(wedding_info->__partner_2.__role_id, mail_info_2);

	Int64 partner_id = wedding_info->__partner_1.__role_id;
	if (partner_id == this->role_id())
	{
		partner_id = wedding_info->__partner_2.__role_id;
	}

    WEDDING_MONITOR->remove_wedding(this->player_wedding_detail_.__wedding_id);

    this->self_wedding_info().__wedding_id = -1;
    this->divorce_sync_property();
    this->sync_wedding_property();
    this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);

    string name = "";
    LogicPlayer *player = this->find_player(partner_id);
    if (player != NULL)
    {
    	player->self_wedding_info().__wedding_id = -1;
    	player->divorce_sync_property();
    	player->sync_wedding_property();
    	player->sync_wedding_info_to_map(-1, this->role_detail().__name);
    	player->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
        player->respond_to_client(RETURN_DIVORCE);
        player->request_wedding_pannel();
        name = player->role_detail().__name;
    }

    this->sync_wedding_info_to_map(-1, name);
    this->request_wedding_pannel();
    return this->respond_to_client(RETURN_DIVORCE);
}

int LogicWeddingPlayer::request_keepsake_upgrade(Message *msg)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, RETURN_KEEPSAKE_UPGRADE, ERROR_NO_WEDDING);

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_KEEPSAKE_UPGRADE, ERROR_OPERATE_TOO_FAST);

    const Json::Value &wedding_item_json = CONFIG_INSTANCE->wedding_item(wedding_info->__keepsake_id);
    CONDITION_NOTIFY_RETURN(wedding_item_json != Json::Value::null, RETURN_KEEPSAKE_UPGRADE, ERROR_CONFIG_NOT_EXIST);

    const Json::Value &cost_item_json = wedding_item_json["cost_item"];
    int max_size = int(cost_item_json.size());
    int max_level = cost_item_json[max_size - 1][0u].asInt();
    CONDITION_NOTIFY_RETURN(wedding_info->__keepsake_level <= max_level, RETURN_KEEPSAKE_UPGRADE, ERROR_KEEPSAKE_TOPLEVEL);

    this->wedding_req_tick_ = nowtime + GameCommon::fetch_time_value(CONFIG_INSTANCE->const_set("wedding_wait_time"));

    Proto31101604 inner_req;
    inner_req.set_keepsake_id(wedding_info->__keepsake_id);
    inner_req.set_keepsake_level(wedding_info->__keepsake_level);
    inner_req.set_keepsake_sublevel(wedding_info->__keepsake_sublevel);
    inner_req.set_keepsake_progress(wedding_info->__keepsake_progress);
    return this->dispatch_to_map_server(&inner_req);
}

int LogicWeddingPlayer::process_keepsake_upgrade_after_pack_check(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31101604 *, request, -1);

    WeddingDetail *wedding_info = this->wedding_detail();
    CONDITION_NOTIFY_RETURN(wedding_info != NULL, RETURN_KEEPSAKE_UPGRADE, ERROR_NO_WEDDING);

    const Json::Value &wedding_item_json = CONFIG_INSTANCE->wedding_item(wedding_info->__keepsake_id);
    CONDITION_NOTIFY_RETURN(wedding_item_json != Json::Value::null, RETURN_KEEPSAKE_UPGRADE, ERROR_CONFIG_NOT_EXIST);

    const Json::Value &cost_item_json = wedding_item_json["cost_item"];
    int max_size = int(cost_item_json.size());
    int max_level = cost_item_json[max_size - 1][0u].asInt();
    CONDITION_NOTIFY_RETURN(wedding_info->__keepsake_level <= max_level, RETURN_KEEPSAKE_UPGRADE, ERROR_KEEPSAKE_TOPLEVEL);

    double inc_progress = 0.0;
    int next_level = 0, next_sublevel = 0;
    for (uint i = 0; i < cost_item_json.size(); ++i)
    {
        if (cost_item_json[i][0u].asInt() == wedding_info->__keepsake_level &&
                cost_item_json[i][1u].asInt() == wedding_info->__keepsake_sublevel)
        {
            int progress_begin = cost_item_json[i][2u].asDouble() * 100,
                progress_end = cost_item_json[i][3u].asDouble() * 100;
            if (progress_begin >= progress_end)
            {
                inc_progress = progress_begin / 100.0;
            }
            else
            {
                int rank_value = rand() % (progress_end - progress_begin + 1);
                inc_progress = progress_begin / 100.0 + rank_value / 100.0;
            }
            if (i + 1 < cost_item_json.size())
            {
                next_level = cost_item_json[i+1][0u].asInt();
                next_sublevel = cost_item_json[i+1][1u].asInt();
            }
            else
            {
                next_level = wedding_info->__keepsake_level + 1;
                next_sublevel = 0;
            }
            break;
        }
    }

    int prev_level = wedding_info->__keepsake_level;
    wedding_info->__keepsake_progress += inc_progress;
    if (wedding_info->__keepsake_progress >= 100.0)
    {
        wedding_info->__keepsake_level = next_level;
        wedding_info->__keepsake_sublevel = next_sublevel;
        wedding_info->__keepsake_progress = 0.0;
    }
    if (prev_level < wedding_info->__keepsake_level)
    {
        this->sync_wedding_property();

        LogicPlayer *partner = this->fetch_wedding_partner();
        if (partner != NULL)
        	partner->sync_wedding_property();
    }

    WEDDING_MONITOR->insert_update_id(wedding_info->__wedding_id);

    this->respond_to_client(RETURN_KEEPSAKE_UPGRADE);

    this->request_wedding_pannel();

    return 0;
}

int LogicWeddingPlayer::request_present_flower(Message *msg)
{
    DYNAMIC_CAST_NOTIFY(Proto10101406 *, request, msg, RETURN_PRESENT_FLOWER);

    Time_Value nowtime = Time_Value::gettimeofday();
    CONDITION_NOTIFY_RETURN(this->wedding_req_tick_ <= nowtime, RETURN_PRESENT_FLOWER, ERROR_OPERATE_TOO_FAST);

    int item_id = request->item_id(), item_num = request->item_num();
    CONDITION_NOTIFY_RETURN(item_id > 0 && item_num > 0, RETURN_PRESENT_FLOWER, ERROR_CLIENT_OPERATE);

    Int64 receiver_id = request->receiver_id();
    LogicPlayer *player = this->find_player(receiver_id);

    MSG_USER("request present flowe %ld %s %d %d %ld", this->role_id(), this->name(), item_id, item_num, receiver_id);

    CONDITION_NOTIFY_RETURN(player != NULL, RETURN_PRESENT_FLOWER, ERROR_PLAYER_OFFLINE);

    this->wedding_req_tick_ = nowtime + GameCommon::fetch_time_value(2);

    Proto31101605 inner_req;
    inner_req.set_item_id(item_id);
    inner_req.set_receiver_id(receiver_id);
    inner_req.set_item_num(item_num);
    inner_req.set_receiver_name(player->role_detail().__name);
    inner_req.set_auto_buy(request->auto_buy());
    return this->dispatch_to_map_server(&inner_req);
}

int LogicWeddingPlayer::process_present_flower_after_pack_check(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto31101605 *, request, msg, -1);
    int item_id = request->item_id(), item_num = request->item_num();
    Int64 receiver_id = request->receiver_id();

    int inc_intimacy = 0;
    const Json::Value &wedding_item_json = CONFIG_INSTANCE->wedding_flower(item_id);
    inc_intimacy = wedding_item_json["intimacy"].asInt() * item_num;

    if (wedding_item_json["is_effects"].asInt())
    {
    	//全服特效
    	Proto80101408 inner_req;
    	inner_req.set_item_id(item_id);
    	inner_req.set_sender_id(this->role_id());
    	inner_req.set_sender_name(this->role_detail().__name);
    	inner_req.set_receiver_id(request->receiver_id());
    	inner_req.set_receiver_name(request->receiver_name());
    	inner_req.set_item_num(item_num);

    	LogicMonitor::PlayerMap &player_map = this->monitor()->player_map();

    	for(LogicMonitor::PlayerMap::iterator iter = player_map.begin();
    			iter != player_map.end(); ++iter)
    	{
    		iter->second->respond_to_client(ACTIVE_SEND_FLOWER, &inner_req);
    	}
    }

    //私聊
    Proto50101406 talk_info;
    talk_info.set_item_id(item_id);
    talk_info.set_sender_id(this->role_id());
    talk_info.set_sender_name(this->role_detail().__name);
    talk_info.set_receiver_id(request->receiver_id());
    talk_info.set_receiver_name(request->receiver_name());
    talk_info.set_item_num(item_num);

    LogicPlayer *receiver = this->find_player(receiver_id);
    if (receiver != NULL)
    {
		/*
    	if (GameCommon::is_normal_scene(receiver->scene_id()))
    	{
			receiver->dispatch_to_map_server(&inner_req);
    	}
    	else
    	{
    		receiver->respond_to_client(ACTIVE_SEND_FLOWER, &inner_req);
    	}
    	*/
    	receiver->respond_to_client(RETURN_PRESENT_FLOWER, &talk_info);
	    Proto30402401 inner_info;
	    inner_info.set_item_id(item_id);
	    inner_info.set_receiver_id(receiver_id);
	    inner_info.set_item_num(item_num);
	    inner_info.set_receiver_name(receiver->role_detail().__name);
	    receiver->dispatch_to_map_server(&inner_info);
    }


    this->respond_to_client(RETURN_PRESENT_FLOWER, &talk_info);
    this->update_flower_amount(receiver_id, item_id, item_num);
    this->update_intimacy(inc_intimacy, receiver_id);


    int shout_id = wedding_item_json["cast_id"].asInt();
    if (shout_id > 0 && receiver != NULL)
    {
    	BrocastParaVec para_vec;

    	GameCommon::push_brocast_para_string(para_vec, this->role_detail().__name);
    	GameCommon::push_brocast_para_string(para_vec, receiver->role_detail().__name);
    	GameCommon::push_brocast_para_string(para_vec, wedding_item_json["flower_name"].asString());
    	GameCommon::push_brocast_para_int(para_vec, item_num);

    	GameCommon::announce(shout_id, &para_vec);
    }

	/*
	if (receiver != NULL)
	{
		Proto10200003 chat_info;
		char chat_str[2048];
		::sprintf(chat_str, wedding_item_json["chat_info"].asCString(),
				this->role_detail().__name.c_str(), wedding_item_json["flower_name"].asCString(), item_num);

		chat_info.set_dst_role_id(receiver->role_id());
		chat_info.set_type(1);
		chat_info.set_wcontent(chat_str);
		this->dispatch_to_chat_server(&chat_info);
	}
	*/

    WeddingDetail *detail = WEDDING_MONITOR->fetch_wedding_detail(this->role_id());
//    JUDGE_RETURN(detail != NULL && detail->__keepsake_level >= 0, ;);
    if (detail != NULL)
    {
        WeddingDetail::WeddingRole *self_info = NULL;
        WeddingDetail::WeddingRole *side_info = NULL;
        if (this->role_id() == detail->__partner_1.__role_id)
        {
        	self_info = &detail->__partner_1;
        	side_info = &detail->__partner_2;
        }
        else
        {
        	self_info = &detail->__partner_2;
        	side_info = &detail->__partner_1;
        }

        if (self_info != NULL && side_info != NULL)
        {
        	self_info->__sweet_degree += wedding_item_json["self_sweet"].asInt() * item_num;
        	this->record_other_serial(SWEET_DEGREE_SERIAL, self_info->__sweet_degree, wedding_item_json["self_sweet"].asInt() * item_num);
        	side_info->__sweet_degree += wedding_item_json["opposite_sweet"].asInt() * item_num;
        	if (receiver != NULL)
			{
        		receiver->record_other_serial(SWEET_DEGREE_SERIAL, side_info->__sweet_degree,
        				wedding_item_json["opposite_sweet"].asInt() * item_num);
			}
        }
    }

    this->request_wedding_pannel();
    if (receiver != NULL)
	{
    	receiver->request_wedding_pannel();
	}

    FINER_PROCESS_NOTIFY(RETURN_PRESENT_FLOWER);
}

int LogicWeddingPlayer::update_intimacy(const int inc_intimacy, const Int64 receiver_id)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    if (wedding_info == NULL ||
            (wedding_info->__partner_1.__role_id != receiver_id && wedding_info->__partner_2.__role_id != receiver_id))
    {
        int max_intimacy = 0;
        LogicPlayer *partner = this->find_player(receiver_id);
        if (partner != NULL)
            max_intimacy = partner->self_wedding_info().__intimacy_map[this->role_id()];
        max_intimacy = std::max(this->player_wedding_detail_.__intimacy_map[receiver_id], max_intimacy);
        max_intimacy += inc_intimacy;
        this->player_wedding_detail_.__intimacy_map[receiver_id] = max_intimacy;
        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);

        if (partner != NULL)
        {
            partner->self_wedding_info().__intimacy_map[this->role_id()] = max_intimacy;
           	partner->record_other_serial(MAIN_INTIMACY_VALUE, SUB_INTIMACY_VALUE,
            			max_intimacy, inc_intimacy, 0);
            partner->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
        }

    	this->record_other_serial(MAIN_INTIMACY_VALUE, SUB_INTIMACY_VALUE,
    			max_intimacy, inc_intimacy, 0);
    }
    else
    {
        wedding_info->__intimacy += inc_intimacy;

        this->player_wedding_detail_.__intimacy_map[receiver_id] = wedding_info->__intimacy;
        this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);

        LogicPlayer *partner = this->find_player(receiver_id);
        if (partner != NULL)
        {
            partner->self_wedding_info().__intimacy_map[this->role_id()] = wedding_info->__intimacy;
            partner->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
        }

    	this->record_other_serial(MAIN_INTIMACY_VALUE, SUB_INTIMACY_VALUE,
    			wedding_info->__intimacy, inc_intimacy, 1);

        if (wedding_info->__history_intimacy < wedding_info->__intimacy)
        {
            wedding_info->__history_intimacy = wedding_info->__intimacy;

//            this->check_award_label(wedding_info);
            LogicPlayer *partner = this->find_player(receiver_id);
            if (partner != NULL)
            {
//            	partner->check_award_label(wedding_info);
            	partner->record_other_serial(MAIN_INTIMACY_VALUE, SUB_INTIMACY_VALUE,
                			wedding_info->__intimacy, inc_intimacy, 1);
            }
        }

        WEDDING_MONITOR->insert_update_id(wedding_info->__wedding_id);
    }
    return 0;
}

int LogicWeddingPlayer::update_flower_amount(const Int64 receiver_id, const int item_id, const int item_num)
{
    int flower_num = 0;
    const Json::Value &flower_json = CONFIG_INSTANCE->wedding()["flower"];
    for (uint i = 0; i < flower_json.size(); ++i)
    {
        if (flower_json[i][0u].asInt() == item_id)
        {
            flower_num = flower_json[i][1u].asInt() * item_num;
            break;
        }
    }

    if(item_num > 0)
    {
    	this->logic_player()->update_cornucopia_activity_value(GameEnum::CORNUCOPIA_TASK_FLOWERS,item_num);
    	MSG_USER("LogicWeddingPlayer, update_flower_amount");
    }

    JUDGE_RETURN(flower_num > 0, 0);

    this->self_wedding_info().__total_send_flower += flower_num;
    this->logic_player()->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);

    LogicPlayer *player = this->find_player(receiver_id);
    if (player != NULL)
    {
        player->self_wedding_info().__total_recv_flower += flower_num;
        player->cache_tick().update_cache(LogicPlayer::CACHE_WEDDING);
    }
    return 0;
}

int LogicWeddingPlayer::make_pannel_info(ProtoWeddingDetail* detail, int type)
{
	PlayerWeddingDetail::wedding_property &info = this->player_wedding_detail_.__wedding_pro_map[type];

	detail->set_exp(info.__exp);
	detail->set_level(info.__level);
	detail->set_order(info.__side_level);
	detail->set_side_level(info.__level / 10);
	detail->set_side_order(info.__side_level / 10);

	int open = 0;
	WeddingDetail *wedding_info = this->wedding_detail();
	switch (type)
	{
	case 1:
	{
	    if (this->is_has_ring() > 0) open = 1;
	    break;
	}
	case 2:
	case 3:
	{
		if (wedding_info != NULL) open = 1;
		break;
	}
	}

	detail->set_is_open(open);

    const Json::Value &pro_json = CONFIG_INSTANCE->wedding_property(type, info.__level);
    JUDGE_RETURN(pro_json != Json::Value::null, -1);
    IntMap prop_map;

    prop_map[GameEnum::ATTACK] += pro_json["attack"].asInt();
    prop_map[GameEnum::DEFENSE] += pro_json["defence"].asInt();
    prop_map[GameEnum::BLOOD_MAX] += pro_json["health"].asInt();
    prop_map[GameEnum::HIT] += pro_json["hit"].asInt();
    prop_map[GameEnum::AVOID] += pro_json["dodge"].asInt();
    prop_map[GameEnum::CRIT] += pro_json["crit"].asInt();
    prop_map[GameEnum::TOUGHNESS] += pro_json["toughness"].asInt();

    const Json::Value &min_json = CONFIG_INSTANCE->wedding_property(type, std::min(info.__side_level, info.__level));
    int inc_scale = 0;
	if (min_json.isMember("order_scale"))
	{
		inc_scale += min_json["order_scale"].asInt();
	}

	if (min_json.isMember("level_scale"))
	{
		inc_scale += min_json["level_scale"].asInt();
	}

    if (inc_scale > 0)
    {
        prop_map[GameEnum::ATTACK] += int((double)prop_map[GameEnum::ATTACK] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::DEFENSE] += int((double)prop_map[GameEnum::DEFENSE] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::BLOOD_MAX] += int((double)prop_map[GameEnum::BLOOD_MAX] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::HIT] += int((double)prop_map[GameEnum::HIT] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::AVOID] += int((double)prop_map[GameEnum::AVOID] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::CRIT] += int((double)prop_map[GameEnum::CRIT] * (double)inc_scale / double(10000.0));
        prop_map[GameEnum::TOUGHNESS] += int((double)prop_map[GameEnum::TOUGHNESS] * (double)inc_scale / double(10000.0));
    }

	FightProperty temp;
	temp.unserialize(prop_map);
	temp.serialize(detail->mutable_pro_list());

	return 0;
}

int LogicWeddingPlayer::request_wedding_pannel(void)
{
    Proto50101407 respond;
    
    ProtoWeddingDetail* ring_info = respond.mutable_ring_info();
    this->make_pannel_info(ring_info, WED_RING);

    ProtoWeddingDetail* sys_info = respond.mutable_sys_info();
    this->make_pannel_info(sys_info, WED_SYS);

    ProtoWeddingDetail* tree_info = respond.mutable_tree_info();
    this->make_pannel_info(tree_info, WED_TREE);

    LogicPlayer* player = this->logic_player();
    TeamPanel *team_info = player->team_panel();
    if (team_info != NULL)
    {
    	 respond.set_keepsake_id(team_info->is_leader(player->role_id()));
    }
    else
    {
    	respond.set_keepsake_id(-1);
    }
    respond.set_keepsake_level(this->wedding_id());

    WeddingDetail *wedding_info = this->wedding_detail();
    if (wedding_info == NULL)
    {
    	respond.set_wedding_tick(0);
        respond.set_intimacy(0);
        return this->dispatch_to_map_server(&respond);
    }
/*
    this->refresh_day_wedding_times();
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    int left_wedding_times = wedding_json["each_day_times"].asInt() - wedding_info->__day_wedding_times;
    if (left_wedding_times < 0)
        left_wedding_times = 0;

    int awarded_label_id = 0;
    {
        const Json::Value &award_label_json = wedding_json["award_label"];
        for (uint i = 0; i < award_label_json.size(); ++i)
        {
            if (wedding_info->__history_intimacy < award_label_json[i][0u].asInt())
                break;
            awarded_label_id = award_label_json[i][1u].asInt();
        }
    }
*/
    respond.set_wedding_tick(wedding_info->__wedding_tick.sec());

    if (wedding_info->__partner_1.__role_id == this->role_id())
    {
        WeddingDetail::WeddingRole &wedding_role = wedding_info->__partner_2;
        respond.set_partner_id(wedding_role.__role_id);
        respond.set_partner_name(wedding_role.__role_name);
        respond.set_partner_sex(wedding_role.__sex);
        respond.set_partner_career(wedding_role.__career);
    }
    else
    {
        WeddingDetail::WeddingRole &wedding_role = wedding_info->__partner_1;
        respond.set_partner_id(wedding_role.__role_id);
        respond.set_partner_name(wedding_role.__role_name);
        respond.set_partner_sex(wedding_role.__sex);
        respond.set_partner_career(wedding_role.__career);
    }
    respond.set_intimacy(wedding_info->__intimacy);

    WeddingDetail::WeddingRole *self_info = NULL;
    WeddingDetail::WeddingRole *side_info = NULL;
    if (this->role_id() == wedding_info->__partner_1.__role_id)
    {
    	self_info = &wedding_info->__partner_1;
    	side_info = &wedding_info->__partner_2;
    }
    else
    {
    	self_info = &wedding_info->__partner_2;
    	side_info = &wedding_info->__partner_1;
    }

    ProtoWedTreasures* self_trea = respond.mutable_self_treasure();
    ProtoWedTreasures* side_trea = respond.mutable_side_treasure();

    self_trea->set_is_fetch(is_same_day(Time_Value(self_info->__fetch_tick), Time_Value::gettimeofday()) == false);
    self_trea->set_type(self_info->__once_reward);
    self_trea->set_left_time(self_info->__left_times);
    self_trea->set_buy_tick(self_info->__tick);

    side_trea->set_is_fetch(is_same_day(Time_Value(side_info->__fetch_tick), Time_Value::gettimeofday()) == false);
    side_trea->set_type(side_info->__once_reward);
    side_trea->set_left_time(side_info->__left_times);
    side_trea->set_buy_tick(side_info->__tick);

    respond.set_sweet_degree(self_info->__sweet_degree);

    LogicPlayer* partner = this->fetch_wedding_partner();
    if (partner != NULL)
    {
    	this->player_wedding_detail_.__side_fashion_id = partner->role_detail().fashion_id_;
    	this->player_wedding_detail_.__side_fashion_color = partner->role_detail().fashion_color_;
    }
    respond.set_keepsake_sublevel(this->player_wedding_detail_.__side_fashion_id);
    respond.set_keepsake_progress(this->player_wedding_detail_.__side_fashion_color);
    return this->dispatch_to_map_server(&respond);
}

int LogicWeddingPlayer:: notify_is_has_wedding(void)
{
	Proto80101407 respond;
	if(this->is_has_wedding())
	{
		respond.set_is_has_wedding(1);
	}
	else
	{
		respond.set_is_has_wedding(0);
	}

	return this->respond_to_client(ACTIVE_IS_WEDDING, &respond);
}

int LogicWeddingPlayer::sync_wedding_property(const int enter_type)
{
	this->refresh_player_wedding_info();

    Proto31403102 info;
    info.set_wedding_id(this->wedding_id());

    for (int type = WED_RING; type < WED_TYPE_END; ++type)
    {
    	ProtoWeddingDetail* info_detail = info.add_wed_info();
    	this->make_pannel_info(info_detail, type);
    }

	return LOGIC_MONITOR->dispatch_to_scene(this, &info);
}

int LogicWeddingPlayer::divorce_sync_property(const int enter_type)
{
    for (int type = WED_RING; type < WED_TYPE_END; ++type)
    {
    	PlayerWeddingDetail::wedding_property &info = this->player_wedding_detail_.__wedding_pro_map[type];
    	info.__side_level = 0;
    	info.__side_order = 0;
    }

    return 0;
}

int LogicWeddingPlayer::update_wedding_reply(const Int64 reply_id, const int reply)
{
    const int REPLAY_DISAGREE = 0, REPLAY_AGREE = 1;
    if (reply == REPLAY_DISAGREE)
    {
        this->player_wedding_detail_.__wedding_reply[reply_id] = reply; 
        this->notify_all_wedding_reply(REPLAY_DISAGREE);
        return 0;
    }

    this->player_wedding_detail_.__wedding_reply[reply_id] = reply;
    if (this->player_wedding_detail_.__wedding_reply.size() >= 2)
    {
        for (PlayerWeddingDetail::RoleValueMap::iterator iter = this->player_wedding_detail_.__wedding_reply.begin();
                iter != this->player_wedding_detail_.__wedding_reply.end(); ++iter)
        {
            if (iter->second == 0)
                return 0;
        }
    }
    else
    {
    	return 0;
    }

    this->notify_all_wedding_reply(REPLAY_AGREE);
    return 0;
}

int LogicWeddingPlayer::validate_can_wedding(const Int64 partner_id)
{
    LogicPlayer *partner = this->find_player(partner_id);
    JUDGE_RETURN(partner != NULL, ERROR_PLAYER_OFFLINE);

//    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding()["wedding"];
    const Json::Value &wedding_json = CONFIG_INSTANCE->wedding_base();

    JUDGE_RETURN(this->role_level() >= wedding_json["wedding_level"].asInt(), ERROR_PLAYER_LEVEL_LIMIT);
    JUDGE_RETURN(partner->role_level() >= wedding_json["wedding_level"].asInt(), ERROR_PLAYER_LEVEL_LIMIT);

//    JUDGE_RETURN(this->scene_id() == wedding_json["npc_locate"][0u].asInt(), ERROR_NOT_IN_WEDDING_SCENE);
//    JUDGE_RETURN(partner->scene_id() == wedding_json["npc_locate"][0u].asInt(), ERROR_NOT_IN_WEDDING_SCENE);

    JUDGE_RETURN(this->role_detail().__sex != partner->role_detail().__sex, ERROR_WED_SEX_SAME);


    if (this->is_has_wedding())
    {
    	JUDGE_RETURN(this->fetch_wedding_partner_id() == partner->role_id(), ERROR_WRONG_PARTNER_ID);
    }
    else
    {
    	JUDGE_RETURN(partner->is_has_wedding() == false, ERROR_HAS_WEDDING);
    }

    return 0;
}

int LogicWeddingPlayer::notify_all_wedding_reply(const int reply)
{
    TeamPanel *team_info = this->logic_player()->team_panel();
    JUDGE_RETURN(team_info != NULL, 0);

    Proto80101404 respond;
    respond.set_reply(reply);

    LogicPlayer *player = NULL;
    for (LongList::iterator iter = team_info->teamer_set_.begin();
            iter != team_info->teamer_set_.end(); ++iter)
    {
        player = this->find_player(*iter);
        if (player == NULL)
            continue;
        player->respond_to_client(ACTIVE_REPLY_WEDDING, &respond);
    }
    return 0;
}

int LogicWeddingPlayer::send_wedding_info_to_travel(int wedding_type, LogicPlayer* partner)
{
	JUDGE_RETURN(partner != NULL && wedding_type == 3, 0);

	Proto30400507 inner;
	ProtoWeddingRank *rank_info = inner.mutable_rank_info();
	rank_info->set_server_id(this->role_detail().__scene_id);
	rank_info->set_server_flag(this->role_detail().__server_flag);
	rank_info->set_server_prev(this->role_detail().__server_prev);
	rank_info->set_server_name(this->role_detail().__server_name);

	ProtoWeddingRole* player1 = rank_info->mutable_player1();
	ProtoWeddingRole* player2 = rank_info->mutable_player2();
	player1->set_role_id(this->role_id());
	player1->set_role_name(this->name());
	player1->set_sex(this->role_detail().__sex);
	player2->set_role_id(partner->role_id());
	player2->set_role_name(partner->name());
	player2->set_sex(partner->role_detail().__sex);
	return LOGIC_MONITOR->dispatch_to_scene(GameEnum::TRVL_WEDDING_SCENE_ID, &inner);
}

void LogicWeddingPlayer::refresh_day_wedding_times(void)
{
    WeddingDetail *wedding_info = this->wedding_detail();
    JUDGE_RETURN(wedding_info != NULL, ;);

    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(wedding_info->__day_refresh_tick <= nowtime, ;);
    
    wedding_info->__day_refresh_tick = next_day(0, 0, nowtime);
    wedding_info->__day_wedding_times = 0;

    WEDDING_MONITOR->insert_update_id(wedding_info->__wedding_id);
}

void LogicWeddingPlayer::finish_wedding_branch_task()
{
	Proto31400023 inner_info;
	inner_info.set_id(GameEnum::BRANCH_WEDDING);
	inner_info.set_num(1);
	this->dispatch_to_map_server(&inner_info);
}

void LogicWeddingPlayer::check_award_label(WeddingDetail *wedding_info)
{
    Proto31101606 inner_req;
    inner_req.set_intimacy(wedding_info->__wedding_type);
    this->dispatch_to_map_server(&inner_req);
}

void LogicWeddingPlayer::check_adjust_wedding_id()
{
	WeddingDetail* wedding_detail = this->wedding_detail();

	if (wedding_detail == NULL && this->player_wedding_detail_.__wedding_id > 0)
	{
		this->player_wedding_detail_.__wedding_id = -1;
		return;
	}

	if (wedding_detail != NULL && this->player_wedding_detail_.__wedding_id != wedding_detail->__wedding_id)
	{
		this->player_wedding_detail_.__wedding_id = wedding_detail->__wedding_id;
		return;
	}
}

int LogicWeddingPlayer::notify_cruise_icon(void)
{
	JUDGE_RETURN(this->role_level() >= 30, 0);
    JUDGE_RETURN(this->is_login_ == true, 0);

    this->is_login_ = false;

    Proto80101403 respond;
    WEDDING_MONITOR->make_up_all_cruise_icon(&respond);
    return this->respond_to_client(ACTIVE_WEDDING_ICON, &respond);
}

int LogicWeddingPlayer::notify_wedding_cartoon_play(void)
{
    Proto80101401 respond;
    JUDGE_RETURN(WEDDING_MONITOR->make_up_cartoon_info(this->role_id(), &respond) == 0, -1);

    return this->respond_to_client(ACTIVE_WEDDING_CARTOON, &respond);
}

int LogicWeddingPlayer::sync_wedding_info_to_map(int wedding_type, string partner_name)
{
//    if (this->is_has_wedding() == false)
//        this->dispatch_to_map_server(INNER_SYNC_DIVORCE_LABEL);

//    JUDGE_RETURN(this->wedding_id() > 0 && this->wedding_detail() != NULL, 0);

    Proto30101610 inner_req;
    inner_req.set_wedding_id(this->wedding_id());
    inner_req.set_partner_id(this->fetch_wedding_partner_id());
    inner_req.set_partner_name(partner_name);
    inner_req.set_wedding_type(wedding_type);
    return this->dispatch_to_map_server(&inner_req);
}

