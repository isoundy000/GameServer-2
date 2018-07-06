/*
 * LogicOfflineHookPlayer.cpp
 *
 *  Created on: Mar 21, 2017
 *      Author: root
 */

#include "LogicOfflineHookPlayer.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"

Offline_Hookdetial::Offline_Hookdetial()
{
	Offline_Hookdetial::reset();
}

void Offline_Hookdetial::reset()
{
	this->soul_limit = 0;
	this->offhook_playerid = 0;
	this->offline_surplustime.sec(0);
	this->last_offhook_exp = 1;
	this->last_offhook_starttime.sec(0);
	this->last_offhook_endtime.sec(0);
	this->last_offhook_time.sec(0);
	this->offhook_costProp.clear();
	this->offline_reward.clear();
	this->one_point_five_plue_time.sec(0);
	this->two_plus_time.sec(0);
	this->vip_plus_time.sec(0);
	this->today_offline_reward.clear();
}

int Offline_Hookdetial::max_surplus_time()
{
	return Time_Value::HOUR * 20;
}


LogicOfflineHookPlayer::LogicOfflineHookPlayer():m_isApplyHook(false)
{
}

Offline_Hookdetial& LogicOfflineHookPlayer::offline_hook_detail()
{
	return this->m_offdetial;
}

int LogicOfflineHookPlayer::player_request_applyofflineHook(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11408002 *, request, -1);

//消耗某物品
	int drug_id = request->offlinehook_drug_id();
	int count = request->offlinehook_drug_count();

//查看背包是否有物品
	int have_amount = this->pack_count(drug_id);
	CONDITION_NOTIFY_RETURN(count <= have_amount, RETURN_OFFLINEHOOK_DRUG_USE,
			ERROR_INVALID_PARAM);

	const Json::Value& effect_prop = CONFIG_INSTANCE->item(drug_id)["effect_prop"];
	CONDITION_NOTIFY_RETURN(effect_prop["name"].asString() == "out_off_line",
			RETURN_OFFLINEHOOK_DRUG_USE, ERROR_INVALID_PARAM);

	int add_time = count * effect_prop["time"].asInt();
	int total_time = this->m_offdetial.offline_surplustime.sec() + add_time;
	int max_time = Offline_Hookdetial::max_surplus_time() + Time_Value::HOUR;
	CONDITION_NOTIFY_RETURN(total_time < max_time, RETURN_OFFLINEHOOK_DRUG_USE, ERROR_OFFLINE_HOOK_MAX);

	this->m_isApplyHook = true;
	this->pack_remove(ITEM_PLAYER_USE, drug_id, count);
	this->m_offdetial.offline_surplustime.sec(total_time);

	Proto51408002 respond;
	respond.set_reply(1);
	FINER_PROCESS_RETURN(RETURN_OFFLINEHOOK_DRUG_USE, &respond);
}

int LogicOfflineHookPlayer::player_request_offlineHook(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11408001 *, request, -1);
	Proto51408001 inner;
	inner.set_surplus_time(this->m_offdetial.offline_surplustime.sec());
	if(this->m_offdetial.last_offhook_exp == 1)
	{
		inner.set_offlinehook_exp(0);
	}
	else
	{
		inner.set_offlinehook_exp(this->m_offdetial.last_offhook_exp);

	}
	inner.set_offlinehook_time(this->m_offdetial.last_offhook_time.sec() );

	for (IntMap::iterator iter = this->m_offdetial.offline_reward.begin();
			iter!=this->m_offdetial.offline_reward.end(); iter++)
	{
		ProtoItemId* item = inner.add_offlinehook_reward();
		item->set_id(iter->first);
		item->set_amount(iter->second) ;
	}

	FINER_PROCESS_RETURN(RETURN_OFFLINEHOOK_INFO, &inner);
}

int LogicOfflineHookPlayer::player_replay_offlinereward(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11408003 *, request, -1);

	for( IntMap::iterator iter = this->m_offdetial.offline_reward.begin();
			iter!=this->m_offdetial.offline_reward.end(); iter++)
	{
		this->insert_package(ADD_FROM_OFFLINE_HOOK, iter->first, iter->second, 0);
	}

	if (this->m_offdetial.last_offhook_exp > 1)
	{
		this->request_add_exp(this->m_offdetial.last_offhook_exp, EXP_FROM_OFFLINE_HOOK);
	}

//	this->m_offdetial.one_point_five_plue_time = this->m_offdetial.one_point_five_plue_time - this->m_offdetial.last_offhook_time;
//	this->m_offdetial.two_plus_time = this->m_offdetial.two_plus_time - this->m_offdetial.last_offhook_time;
//	this->m_offdetial.vip_plus_time = this->m_offdetial.vip_plus_time - this->m_offdetial.last_offhook_time;

	this->m_offdetial.offline_reward.clear();
	this->m_offdetial.last_offhook_exp = 1;
	this->m_offdetial.last_offhook_time.sec(0);


	Proto51408003 inner;
	inner.set_reply(1);
	FINER_PROCESS_RETURN(RETURN_OFFLINEHOOK_INFO, &inner);
}

void LogicOfflineHookPlayer::player_sigin()
{
	Int64 exp = 0;
	Int64 current_time = ::time(NULL);

	this->m_offdetial.offhook_playerid = this->role_id();

	Int64 passtime = current_time - this->m_offdetial.last_offhook_starttime.sec();
	if (passtime > this->m_offdetial.offline_surplustime.sec())
	{
		passtime = this->m_offdetial.offline_surplustime.sec();
	}

	if(passtime <= 0)
		passtime = 0;

	Int64 one_use_time = 0, two_use_time = 0, vip_use_time = 0;

	if(this->m_offdetial.one_point_five_plue_time.sec() > 0)
	{
		if(passtime < this->m_offdetial.one_point_five_plue_time.sec())
			one_use_time = passtime;
		else
			one_use_time = this->m_offdetial.one_point_five_plue_time.sec();

		if(one_use_time < 0)
			one_use_time = this->m_offdetial.one_point_five_plue_time.sec();
	}

	if(this->m_offdetial.two_plus_time.sec() > 0)
	{
		if(passtime < this->m_offdetial.two_plus_time.sec())
			two_use_time = passtime;
		else
			two_use_time = this->m_offdetial.two_plus_time.sec();

		if(two_use_time < 0)
			two_use_time = this->m_offdetial.two_plus_time.sec();
	}

	if(this->m_offdetial.vip_plus_time.sec() > 0)
	{
		if(passtime < this->m_offdetial.vip_plus_time.sec())
			vip_use_time = passtime;
		else
			vip_use_time = this->m_offdetial.vip_plus_time.sec();

		if(vip_use_time < 0)
			vip_use_time = this->m_offdetial.vip_plus_time.sec();
	}

	if (this->m_offdetial.last_offhook_exp < 1)
	{
		this->m_offdetial.last_offhook_exp = 1;
	}

	if( passtime > 0 && this->m_offdetial.last_offhook_endtime.sec() != this->m_offdetial.last_offhook_starttime.sec() && this->m_offdetial.last_offhook_exp  > 0)
	{
		//结束时间
		this->m_offdetial.last_offhook_endtime.sec(current_time);
		// surplus 时间
		Int64 surplu_time = this->m_offdetial.last_offhook_starttime.sec() + this->m_offdetial.offline_surplustime.sec() - current_time;
		if(surplu_time < 0)
		{
			surplu_time = 0;
		}

		surplu_time = std::min<int>(Offline_Hookdetial::max_surplus_time(), surplu_time);
		this->m_offdetial.offline_surplustime.sec(surplu_time);
		this->m_offdetial.last_offhook_starttime.sec(current_time);
		this->m_offdetial.last_offhook_time.sec(passtime);

		if(one_use_time != 0)
			this->m_offdetial.one_point_five_plue_time.sec(this->m_offdetial.one_point_five_plue_time.sec() - one_use_time);
		if(two_use_time != 0)
			this->m_offdetial.two_plus_time.sec(this->m_offdetial.two_plus_time.sec() - two_use_time);
		if(vip_use_time != 0)
			this->m_offdetial.vip_plus_time.sec(this->m_offdetial.vip_plus_time.sec() - vip_use_time);

		MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
		//玩家实际战力
		int fight_force = player->role_detail().__fight_force;
		int level = player->role_level();

		int per_exp = CONFIG_INSTANCE->role_level(0, player->role_level())["exp_per_min"].asInt();
		int power = CONFIG_INSTANCE->role_level(0, player->role_level())["power"].asInt();
 		//float rate = (fight_force*1.0/power)*10000;
		//float rate = (fight_force*1.0/power) + 1;

		int rate = (int)((fight_force*1.0/power)*10000);

		int rateTime = 60;
		const Json::Value& lvl_json = CONFIG_INSTANCE->role_level(0, level);
		string c_str = "reward_output";
		int size = (int)lvl_json[c_str].size();
		float power_rate = 0.00000f;
		CONFIG_INSTANCE->role_fightConfig(rate,power_rate,level);

		//每分钟经验基础值
		int base_exp = per_exp * power_rate;

		int one_point_five_exp = base_exp * (one_use_time / rateTime);
		int two_exp = base_exp * (two_use_time / rateTime);
		int vip_exp = base_exp * (vip_use_time / rateTime);

		//基础经验值（除去了加成的经验）
		int base_sec_exp = base_exp * (passtime / rateTime);

		double one_percent = CONFIG_INSTANCE->item(ONE_OFFLINE_ITEM_ID)["effect_prop"]["percent"].asInt() / 10000.0f;
		double two_percent = CONFIG_INSTANCE->item(TWO_OFFLINE_ITEM_ID)["effect_prop"]["percent"].asInt() / 10000.0f;
		double vip_percent = CONFIG_INSTANCE->offline_vip_info(player->vip_detail().__vip_level)["scale"].asInt() / 10000.0f;

		exp = one_point_five_exp * one_percent + two_exp * two_percent + vip_exp * vip_percent + base_sec_exp;

		int tmp = 0;
		int loop_time = 10;
		int need_rand_item_count = passtime / rateTime / 10;
		while(need_rand_item_count)
		{
			if(loop_time)
			{
				int rand_index = ::std::rand() % size;
				//int rand_index = this->handler_offhookgood_weight(lvl_json,size);
				int rewardid = lvl_json[c_str][rand_index][tmp].asInt();
				int reward_count = lvl_json[c_str][rand_index][1].asInt();
				const Json::Value& cfg = CONFIG_INSTANCE->item(rewardid);
				int drop_limit = cfg["drop_limit"].asInt();
				int &today_get_count = this->m_offdetial.today_offline_reward[rewardid];
				if(drop_limit > 0)
				{
					if(today_get_count + reward_count > drop_limit)
					{
						loop_time--;
						continue;
					}
				}

				this->m_offdetial.offline_reward[rewardid] += reward_count;
				this->m_offdetial.today_offline_reward[rewardid] += reward_count;
				loop_time = 10;
				need_rand_item_count--;
				continue;
			}

			//当随机不出物品的时候，按顺序补发给玩家
			if(loop_time == 0)
			{
				for(int i = 0; i < size; ++i)
				{
					int rewardid = lvl_json[c_str][i][tmp].asInt();
					int count = lvl_json[c_str][i][1].asInt();

					const Json::Value& cfg = CONFIG_INSTANCE->item(rewardid);
					int drop_limit = cfg["drop_limit"].asInt();
					if(drop_limit <= 0)
					{
						this->m_offdetial.today_offline_reward[rewardid] += count;
						this->m_offdetial.offline_reward[rewardid] += count;
						loop_time = 10;
						need_rand_item_count--;
						break;
					}
					else
					{
						if(this->m_offdetial.today_offline_reward[rewardid] + count <= drop_limit)
						{
							this->m_offdetial.today_offline_reward[rewardid] += count;
							this->m_offdetial.offline_reward[rewardid] += count;
							loop_time = 10;
							need_rand_item_count--;
							break;
						}
					}
				}
			}

			if( this->m_offdetial.soul_limit > 0 )
			{
				int soul_rewardid = lvl_json["immortal_soul_output"][0u].asInt();
				int soul_count = lvl_json["immortal_soul_output"][1u].asInt();
				IntMap::iterator iter = this->m_offdetial.offline_reward.find(soul_rewardid);
				if(iter!= this->m_offdetial.offline_reward.end())
				{
					this->m_offdetial.offline_reward[soul_rewardid] += soul_count;
				}
				else
				{
					this->m_offdetial.offline_reward[soul_rewardid] = soul_count;
				}
				this->m_offdetial.soul_limit--;
			}
		}

		int scale = CONFIG_INSTANCE->vip(player->vip_type())["offline_extra_exp"].asInt();

		exp *= (1 + scale/10000.0f);
		this->m_offdetial.last_offhook_exp += exp;

		Int64 limit_exp =  std::max<Int64>(CONFIG_INSTANCE->role_level(0, player->role_level())["exp"].asDouble(), 1);
		if( limit_exp < this->m_offdetial.last_offhook_exp )
		{
			this->m_offdetial.last_offhook_exp = limit_exp;
		}
	}
}

//根据权重返回掉落物品
int LogicOfflineHookPlayer::handler_offhookgood_weight(const Json::Value& lvl_json,const int size)
{
	int rand = (::std::rand()%90 + 10)* 100;
	int index = 0;
	for( int i = 0; i < size ; i++)
	{
		if(lvl_json[4u].asInt() > rand )
		{
			index = i;
			break;
		}
	}
	return index;
}

void LogicOfflineHookPlayer::player_sigout()
{
	Int64 current_time = ::time(NULL);
	if((m_isApplyHook || this->m_offdetial.offline_surplustime.sec() > 0 ))
	{
		this->m_offdetial.last_offhook_starttime.sec(current_time+3*60);
		this->m_offdetial.last_offhook_endtime.sec(current_time);
	}
	else
	{
		this->m_offdetial.last_offhook_starttime.sec(current_time);
		this->m_offdetial.last_offhook_endtime.sec(current_time);
	}
}

void LogicOfflineHookPlayer::hook_reset()
{
	this->m_offdetial.reset();
}

int LogicOfflineHookPlayer::hook_sync_transfer_scene(int sceneid)
{
	Proto31404011 inner;
	inner.set_isapply(this->m_isApplyHook);
	inner.set_hook_exp(this->m_offdetial.last_offhook_exp);
	inner.set_surplustime(this->m_offdetial.offline_surplustime.sec());
	inner.set_start_time(this->m_offdetial.last_offhook_starttime.sec());
	inner.set_end_time(this->m_offdetial.last_offhook_endtime.sec());
	inner.set_hook_time(this->m_offdetial.last_offhook_time.sec());

	inner.set_vip_plus_time(this->m_offdetial.vip_plus_time.sec());
	inner.set_one_point_five_plue_time(this->m_offdetial.one_point_five_plue_time.sec());
	inner.set_two_plus_time(this->m_offdetial.two_plus_time.sec());

	GameCommon::map_to_proto(inner.mutable_today_offline_reward(),
			this->m_offdetial.today_offline_reward);

	GameCommon::map_to_proto(inner.mutable_offline_reward(),
			this->m_offdetial.offline_reward);
	return this->send_to_other_logic_thread(sceneid, inner);
}

int LogicOfflineHookPlayer::read_hook_sync_transfer_scen(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto31404011*, sync_msg, -1);

	this->m_isApplyHook = sync_msg->isapply();
	this->m_offdetial.last_offhook_exp = sync_msg->hook_exp();
	this->m_offdetial.last_offhook_starttime.sec(sync_msg->start_time());
	this->m_offdetial.last_offhook_endtime.sec(sync_msg->end_time());
	this->m_offdetial.last_offhook_time.sec(sync_msg->hook_time());
	this->m_offdetial.offline_surplustime.sec(sync_msg->surplustime());
	this->m_offdetial.one_point_five_plue_time.sec(sync_msg->one_point_five_plue_time());
	this->m_offdetial.two_plus_time.sec(sync_msg->two_plus_time());
	this->m_offdetial.vip_plus_time.sec(sync_msg->vip_plus_time());

	GameCommon::proto_to_map(this->m_offdetial.offline_reward, sync_msg->offline_reward());
	GameCommon::proto_to_map(this->m_offdetial.today_offline_reward, sync_msg->today_offline_reward());


	return 0;
}

int LogicOfflineHookPlayer::notify_godsoul_limit_info()
{
	// immortal_soul
	//MapLogicPlayer* player = dynamic_cast<MapLogicPlayer *>(this);
	//int level = player->role_level();
	//const Json::Value& lvl_json = CONFIG_INSTANCE->role_level(0, level);
	//int soul_rewardid = lvl_json["immortal_soul_output"][0u].asInt();
	//const Json::Value& cfg = CONFIG_INSTANCE->item(soul_rewardid);
	//this->m_offdetial.soul_limit =  cfg["time"].asInt();;
	this->m_offdetial.soul_limit =  0;

	return 0;
}

int LogicOfflineHookPlayer::cmd_hooktime(int hooktimes)
{
	this->m_offdetial.last_offhook_starttime.sec(::time(NULL));
	this->m_offdetial.last_offhook_starttime.sec( this->m_offdetial.last_offhook_starttime.sec() - hooktimes*3600);
	//this->m_offdetial.last_offhook_exp = 1;
	this->player_sigin();
	return 0;
}

int LogicOfflineHookPlayer::fetch_offline_plus_info()
{
	Proto51408005 respond;
	respond.add_left_time(this->m_offdetial.vip_plus_time.sec());
	respond.add_left_time(this->m_offdetial.one_point_five_plue_time.sec());
	respond.add_left_time(this->m_offdetial.two_plus_time.sec());
	FINER_PROCESS_RETURN(RETURN_OFFLINE_PLUS_INFO, &respond);
}

int LogicOfflineHookPlayer::use_offline_plus_item(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11408004*, request, -1);
	int item_amount = request->item_amount();
	int item_id = request->item_id();
	return this->use_offline_plus_item(item_id, item_amount);
}

int LogicOfflineHookPlayer::use_offline_plus_item(int item_id, int item_amount)
{
	//查看背包是否有物品
	int have_amount = this->pack_count(item_id);
	CONDITION_NOTIFY_RETURN(item_amount <= have_amount, RETURN_OFFLINE_PLUS_REWARD,
			ERROR_INVALID_PARAM);

	const Json::Value &use_info = CONFIG_INSTANCE->item(item_id)["effect_prop"];
	JUDGE_RETURN(!use_info.empty(), -1);

	Offline_Hookdetial &detail = this->offline_hook_detail();
	int plus = 0;
	int add_offline_time = 0;

	Time_Value one_time;
	Time_Value two_time;
	Time_Value vip_time;
	//这里是使用道具,获取加成数
	if(use_info["name"].asString() == "one_offline_percent")
	{
		plus = use_info["percent"].asInt();
		add_offline_time = item_amount * use_info["last"].asInt();
		one_time.sec(detail.one_point_five_plue_time.sec() + add_offline_time);
	}
	else if(use_info["name"].asString() == "two_offline_percent")
	{
		plus = use_info["percent"].asInt();
		add_offline_time = item_amount * use_info["last"].asInt();
		two_time.sec(detail.two_plus_time.sec() + add_offline_time);
	}
	else if(use_info["name"].asString() == "vip_offline_percent")
	{
		add_offline_time = item_amount * use_info["last"].asInt();
		vip_time.sec(detail.vip_plus_time.sec() + add_offline_time);
	}
	else
	{
		this->set_last_error(ERROR_INVALID_PARAM);
		return this->respond_to_client_error(RETURN_OFFLINE_PLUS_REWARD, ERROR_INVALID_PARAM);
	}

	if(one_time.sec() == 0)
		one_time = detail.one_point_five_plue_time;
	if(two_time.sec() == 0)
		two_time = detail.two_plus_time;
	if(vip_time.sec() == 0)
		vip_time = detail.vip_plus_time;

	Time_Value limit_time = one_time;
	if(limit_time < two_time)
		limit_time = two_time;
	if(limit_time < vip_time)
		limit_time = vip_time;

	//加成经验超出
	CONDITION_NOTIFY_RETURN(limit_time <= detail.offline_surplustime,
			RETURN_OFFLINE_PLUS_REWARD, ERROR_OFFLINE_HOOK_MAX);


	if(one_time.sec() > 0)
		detail.one_point_five_plue_time = one_time;

	if(two_time.sec() > 0)
		detail.two_plus_time = two_time;

	if(vip_time.sec() > 0)
		detail.vip_plus_time = vip_time;

	Time_Value left_time = vip_time + two_time + one_time;
	//使用道具
	this->pack_remove(ITEM_PLAYER_USE, item_id, item_amount);
	Proto51408004 respond;
	respond.set_item_id(item_id);
	respond.set_item_amount(this->pack_count(item_id));
	respond.set_left_time(left_time.sec());

	//重新刷新界面
	this->fetch_offline_plus_info();
	FINER_PROCESS_RETURN(RETURN_OFFLINE_PLUS_REWARD, &respond);
}

int LogicOfflineHookPlayer::reset_every_day_offline()
{
	this->offline_hook_detail().today_offline_reward.clear();
	return 0;
}
