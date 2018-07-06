/*
 * CenterUnit.cpp
 *
 * Created on: 2014-01-17 16:45
 *     Author: lyz
 */

#include <CenterPostCache.h>
#include "CenterPlay800.h"
#include "CenterUnit.h"
#include "LogicMonitor.h"
#include "ProtoDefine.h"
#include "GameFont.h"
#include "GameConfig.h"
#include "GameCommon.h"

int CenterUnit::type(void)
{
    return BaseUnit::CENTER_UNIT;
}

UnitMessage *CenterUnit::pop_unit_message(void)
{
    return LOGIC_MONITOR->unit_msg_pool()->pop();
}

int CenterUnit::push_unit_message(UnitMessage *msg)
{
    return LOGIC_MONITOR->unit_msg_pool()->push(msg);
}

int CenterUnit::process_block(UnitMessage *unit_msg)
{
    int32_t recogn = unit_msg->__msg_head.__recogn;
    int64_t role_id = unit_msg->__msg_head.__role_id;

    ::google::protobuf::Message *msg = unit_msg->proto_msg();
    MSG_USER("CenterUnit::process_block(), recogn: %d, php_url_: %s, before process",
    		recogn, CENTER_POST_CACHE->php_url().c_str());
    switch (recogn)
    {
    	case INNER_CENTER_UNIT_ROLE_DEAIL:
    		return CENTER_POST_CACHE->insert_role_detail(msg);

    	case INNER_CENTER_UNIT_QUERY_ACTI_CODE:
    		return this->logic_query_acti_code_ret(role_id, msg);

        case INNER_CENTER_SYNC_UCEXTEND:
            return CENTER_POST_CACHE->sync_sdk_info(msg);

        case INNER_CENTER_UNIT_SEND_ACT_MAIL:
        	return this->logic_notify_act_mail(msg);
        case INNER_CENTER_UNIT_SEND_ACT_REWARD_MAIL:
        	return this->logic_send_act_reward_mail(msg);
        case INNER_CENTER_DEL_RETURN_RECHARGE:
        	return this->logic_query_del_return_recharge(msg);
        case INNER_CENTER_DRAW_RANK_REWARD:
        	return this->logic_rank_send_mail(msg);
        case INNER_CENTER_PLAYER_INCOME_LOG:
        	return CENTER_PLAY_800->income_log_get(msg);
        default:
            MSG_USER("ERROR can't reconigze center unit recogn %d", recogn);
            return -1;
    }
    MSG_USER("CenterUnit::process_block(), recogn: %d, php_url_: %s, after process",
    		recogn, CENTER_POST_CACHE->php_url().c_str());
    return 0;
}

int CenterUnit::process_stop(void)
{
    // 正常停服时调用
    // 停服时推送处理
	CENTER_POST_CACHE->post2center_role_all(false); // 推送一次数据，失败不重试
    return 0;
}

int CenterUnit::prev_loop_init(void)
{
    this->interval_amount_ = 0;
    this->interval_tick_ = Time_Value::zero;
    CENTER_POST_CACHE->init();
    CENTER_PLAY_800->init();
    return 0;
}

int CenterUnit::interval_run(void)
{
    ++this->interval_amount_;
    if (this->interval_amount_ < 10000)
        return 0;
    this->interval_amount_ = 0;

    // 间隔调用
    Time_Value nowtime = Time_Value::gettimeofday();
    if (!CENTER_POST_CACHE->is_need_post() && // 没有其他推送条数满足
    		this->interval_tick_ > nowtime ) // 定时器也未到达
        return 0;
#ifdef LOCAL_DEBUG
    this->interval_tick_ = nowtime + Time_Value(1);    // 定时间隔
#else
    this->interval_tick_ = nowtime + Time_Value(10);    // 定时间隔
#endif
    // 定时推送处理
    CENTER_POST_CACHE->post2center_role_all();

    return 0;
}

int CenterUnit::logic_query_acti_code_ret(int64_t role_id, Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto32101201*, request, -1);
	int query_ret = CENTER_POST_CACHE->query_center_acti_code(request->acti_code());

	Proto30101102 req;
	req.set_code_id(request->code_id());
	req.set_query_ret(query_ret);

	return LOGIC_MONITOR->process_inner_logic_request(role_id, req);
}

int CenterUnit::logic_notify_act_mail(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto32101103*, request, -1);
	MSG_USER("act end and send mail act_size:%d role_size:%d",(int)request->act_list_size(),(int)request->role_id_size());

	MSG_DEBUG(%s, request->Utf8DebugString().c_str());
	FontPair pair = CONFIG_INSTANCE->font(FONT_SYSTEM_MAIL);
	std::string sender_name = pair.first;
	std::string title;
	if(CONFIG_INSTANCE->open_activity_json().isMember("act_open_notify"))
	{
		title = CONFIG_INSTANCE->open_activity_json()["act_open_notify"].asString();
	}

	std::string content;
	for(int i = 0; i < (int)request->act_list_size(); ++i)
	{
		char num[10] = {0};
		::snprintf(num, sizeof(num)-sizeof(char), "[%d]", i+1);
		std::string str_index = std::string(num);

		char time_str[512] = {0};
		if(CONFIG_INSTANCE->open_activity_json().isMember("act_open_time"))
		{
			time_t open_time = (time_t)request->act_list(i).start_tick();
			tm tm_info;
			::localtime_r(&open_time, &tm_info);
			snprintf(time_str, sizeof(time_str), CONFIG_INSTANCE->open_activity_json()["act_open_time"].asCString(), tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday, tm_info.tm_hour, tm_info.tm_min);
		}
		content = content + str_index + request->act_list(i).title() + ":" + request->act_list(i).content() + ", " +string(time_str) + "\n";
	}

	for(int i = 0; i < (int)request->role_id_size(); ++i)
	{
		MailInformation *mail_info = GameCommon::create_sys_mail(title, content,FONT_SYSTEM_MAIL);
		mail_info->sender_name_ = sender_name;
		mail_info->receiver_id_ = request->role_id(i);
		GameCommon::request_save_mail(mail_info);
	}
	return 0;
}

int CenterUnit::logic_send_act_reward_mail(Message* msg)
{
	return 0;
}

int CenterUnit::logic_query_del_return_recharge(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto32101105*, request, -1);

	Int64 role_id = request->role_id();
	MSG_USER("centerUnit request get delete_return_recharge role_id:%ld account:%s",
			role_id, request->account().c_str());

	int ret = CENTER_POST_CACHE->query_center_del_return_recharge(role_id, request->account());
	MSG_USER("centerUnit get delete_return_recharge gold:%d",ret);

	return 0;
	//现在邮件由php发送，这里不用回调
//	JUDGE_RETURN(ret > 0, -1);
//	Proto30100605 send;
//	send.set_role_id(role_id);
//	send.set_money(ret);
//	return LOGIC_MONITOR->process_inner_logic_request(role_id, send);
}

int CenterUnit::logic_rank_send_mail(Message* msg)
{
	//排行榜活动结束时发送邮件
	MSG_DYNAMIC_CAST_RETURN(Proto32101106*, request, -1);

//	BackSetActDetail::ActTypeItem act;
//	const PActTypeItem& a_act = request->act_info();
//	act.unserialize(&a_act);
//	MSG_USER("rank act end and send mail act_index:%d",act.act_index_);
////	MSG_DEBUG("%s", request->Utf8DebugString().c_str());
//	int rank_index = 1;
//	for (BackSetActDetail::ActItemSet::iterator iter = act.act_item_set_.begin(); iter != act.act_item_set_.end(); ++iter)
//	{	//遍历所有的排名段奖励信息，如第1-3名、第4-6名等等
//		for(LongMap::const_iterator it = iter->sub_map_.begin(); it != iter->sub_map_.end(); ++it, ++rank_index)
//		{	//遍历每个排名段的奖励信息，如第1-3名
//			FontPair mail_font;
//			char rand_str[GameEnum::MAX_MAIL_CONTENT_LENGTH + 1] = {0};
//			try{//主要为了防止错误的格式化导致进程崩溃
//				std::string notify;
//				if(CONFIG_INSTANCE->open_activity_json().isMember("act_rank_end_send_mail"))
//				{
//					notify = CONFIG_INSTANCE->open_activity_json()["act_rank_end_send_mail"].asString();
//				}
//				::snprintf(rand_str, GameEnum::MAX_MAIL_CONTENT_LENGTH, notify.c_str(), act.act_title_.c_str(), rank_index);	//服务器写定邮件内容
//			}catch(...){
//				continue;
//			}
//			rand_str[GameEnum::MAX_MAIL_CONTENT_LENGTH] = '\0';
//
//			MailInformation *mail_info = GameCommon::create_sys_mail(mail_font,FONT_SYSTEM_MAIL);
//			mail_info->receiver_id_ = it->second;
//			std::string item_str;
//			int carrer = it->first & 0xff;
////			MSG_DEBUG("index:%d,role_id:%ld, carrer:%d",rank_index, it->second, carrer);
//			ItemObjVec adjust_set = GameCommon::adjust_item_by_carrer(iter->reward_, carrer);
//			for(ItemObjVec::const_iterator obj = adjust_set.begin(); obj != adjust_set.end(); ++obj)
//			{	//遍历所有的奖励item，并插入背包
//				const Json::Value& json = CONFIG_INSTANCE->item(obj->item_id_);
//				JUDGE_CONTINUE(json != Json::Value::null);
//				char str_cnt[10] = {0};
//				::snprintf(str_cnt, sizeof(str_cnt)-sizeof(char), "x%d; ", obj->item_amount_);
//				item_str = item_str + json["name"].asString() + string(str_cnt);
//				mail_info->add_goods(*obj);
//			}
//			mail_info->mail_title_ = act.act_title_;
//			mail_info->mail_content_ = std::string(rand_str) + item_str;
//			MSG_USER("receiver_id:%ld, mail info:%s", mail_info->receiver_id_, mail_info->mail_content_.c_str());
//			GameCommon::request_save_mail(mail_info);
//		}
//	}

	return 0;
}
