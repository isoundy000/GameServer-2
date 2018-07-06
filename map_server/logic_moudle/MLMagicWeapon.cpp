/*
 * MLMagicWeapon.cpp
 *
 *  Created on: 2015-12-14
 *      Author: xu
 */

#include "MLMagicWeapon.h"
#include "MapLogicPlayer.h"
#include "PubStruct.h"
#include "MapMonitor.h"
#include "MLGameSwither.h"
#include "GameHeader.h"
#include "ProtoDefine.h"

MagicWeaponDetail::MagicWeaponDetail()
{
	MagicWeaponDetail::reset();
}

void MLMagicWeapon::make_up_detail(int id, int skill_id, int skill_lvl, bool is_adorn,
		int is_active, int rank_grade, int qua_grade, MagicWeaponDetail& ref_tmp)
{
	ref_tmp.mw_id_ = id;
	ref_tmp.mw_skill_id_ = skill_id;
	ref_tmp.mw_skill_lvl_ = skill_lvl;
	ref_tmp.mw_is_adorn_  = is_adorn;
	ref_tmp.mw_is_activate_ = is_active;
	ref_tmp.mw_rank_.rank_star_grade_ = rank_grade;
	ref_tmp.mw_quality_.qua_star_grade_ = qua_grade;
}

void MagicWeaponDetail::reset()
{
	this->mw_id_ = 0;
	this->mw_is_activate_ = 0;
	this->mw_is_adorn_ = 0;
	this->mw_skill_id_ = 0;
	this->mw_skill_lvl_ = 0;
	this->mw_rank_.reset();
	this->mw_quality_.reset();
}

bool MagicWeaponDetail::validate_id() const
{
	return this->mw_id_ > 0 && this->mw_id_ < GameEnum::RAMA_CLASS_ID_END;
}

MagicWeaponDetail::MagicRank::MagicRank()
{
	 this->rank_star_grade_ = 1;
	 this->rank_curr_star_progress_ = 0;
}

void MagicWeaponDetail::MagicRank::reset()
{
	 this->rank_star_grade_ = 1;
	 this->rank_curr_star_progress_ = 0;
}

MagicWeaponDetail::MagicQuality::MagicQuality()
{
	this->qua_star_grade_ = 1;
	this->qua_curr_star_progress_ = 0;
}

void MagicWeaponDetail::MagicQuality::reset()
{
	this->qua_star_grade_ = 1;
	this->qua_curr_star_progress_ = 0;
}

MLMagicWeapon::MLMagicWeapon()
{
	this->open_ = 0;
	this->player_reiki_num_ = 0;
	this->talisman_id_ = 0;
	this->talisman_rank_lvl_ = 0;
	this->magicweapon_list_.clear();
}

MLMagicWeapon::~MLMagicWeapon()
{
}

void MLMagicWeapon::reset_magicweapon()
{
	this->open_ = false;
	this->talisman_id_ = 0;
	this->talisman_rank_lvl_ = 0;
	this->player_reiki_num_ = this->reiki();
	this->magicweapon_list_.clear();
}

 MagicWeaponMap &MLMagicWeapon::magicweapon_list()
 {
	 return this->magicweapon_list_;
 }

void MLMagicWeapon::magic_weapon_init_when_actived(int mw_id,int rank_grade,int quality_grade)
{
	MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
	JUDGE_RETURN(curr_magic_weapon.mw_id_ == mw_id,;);

	curr_magic_weapon.mw_is_activate_ = MLMagicWeapon::ALREADY_ACTIVE;
	curr_magic_weapon.mw_is_adorn_ = false;
}

MagicWeaponDetail &MLMagicWeapon::magicweapon_set_by_id(int mw_id)
{
	MagicWeaponMap::iterator iter = this->magicweapon_list_.find(mw_id);
	if (iter != this->magicweapon_list_.end())
	{
		 return iter->second;
	}
	else
	{
	 return this->null_magic_weapon_;
	}
}

int MLMagicWeapon::change_magic_weapon_status(int type, int ext_num)
{
	static string CONDSTR[] = {"online_total", "vip_type", "task_id", "first_recharge"};

	for (int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; i++)
	{
		JUDGE_CONTINUE(this->magicweapon_list_[i].mw_is_activate_ == MLMagicWeapon::NOT_ACTIVE);

		const Json::Value &rama_json = CONFIG_INSTANCE->rama_open(i);
		JUDGE_CONTINUE(rama_json.isMember(CONDSTR[type - 1]) == true);
		JUDGE_CONTINUE(ext_num >= rama_json[CONDSTR[type - 1]].asInt());

		this->magicweapon_list_[i].mw_is_activate_ = MLMagicWeapon::CAN_ACTIVE;
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);

		JUDGE_CONTINUE(i != GameEnum::RAMA_CLASS_ID_BEGIN);
		this->map_logic_player()->update_player_assist_single_event(GameEnum::PA_EVENT_RAMA_GET , i);
	}

	return 0;
}

 int MLMagicWeapon::init_magicweapon_list()
 {
	 this->player_reiki_num_ = this->reiki();
	 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN ;i < GameEnum::RAMA_CLASS_ID_END; i++)
	 {
		int cfg_id = i * GameEnum::RAMA_ID_BASE_NUM +  GameEnum::RAMA_BASS_LEVEL;
		JUDGE_CONTINUE(cfg_id > 0);
		JUDGE_CONTINUE(this->magicweapon_list_[i].mw_id_ <= 0);

		const Json::Value &magic_weapon_detail = CONFIG_INSTANCE->rama_list(cfg_id);
		int skill_id = magic_weapon_detail["skill_id"].asInt();
		int skill_level = magic_weapon_detail["skill_level"].asInt();

		MagicWeaponDetail tmp;
		this->make_up_detail(i,skill_id,skill_level,false,MLMagicWeapon::NOT_ACTIVE,
				  GameEnum::RAMA_BASS_LEVEL,GameEnum::RAMA_BASS_LEVEL,tmp);

		this->magicweapon_list_[i] = tmp;
		this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);
	 }
	 return 0;
 }


int MLMagicWeapon::check_open_rama(int task_id)
{
	JUDGE_RETURN(this->open_ == false, -1);
	JUDGE_RETURN(CONFIG_INSTANCE->arrive_fun_open_task("fun_rama", task_id) == true, -1);

	this->open_ = true;
	this->map_logic_player()->update_player_assist_single_event(GameEnum::PA_EVENT_RAMA_OPEN, 1);
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);
	return 0;
}

 int MLMagicWeapon::magic_weapon_fetch_info()
 {
//	JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
//	CONDITION_NOTIFY_RETURN(this->magic_weapon_open_lvl() == true, RETURN_REQUEST_MAGIC_WEAPON, ERROR_MAGICWEAPON_OPEN_LEVEL);
	this->init_magicweapon_list();

	Proto51402501 respond;
	CONDITION_NOTIFY_RETURN(this->open_ == 1, RETURN_REQUEST_MAGIC_WEAPON,
			ERROR_MAGICWEAPON_OPEN_LEVEL);

	respond.set_adorn_id(0);
	respond.set_reiki_num(this->player_reiki_num_);
	 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
	 {
		 MagicWeaponDetail *temp = &this->magicweapon_list_[i];
		 if((temp->mw_id_ > 0) && (temp->mw_is_adorn_))
		 {
			 respond.set_adorn_id(temp->mw_id_ * GameEnum::RAMA_ID_BASE_NUM + temp->mw_rank_.rank_star_grade_);
		 }

		 ProtoMagicWeapon *proto = respond.add_mw_list();
		 this->magic_weapon_serialize(proto,temp);
	}

	return this->respond_to_client(RETURN_REQUEST_MAGIC_WEAPON, &respond);
 }

 bool MLMagicWeapon::judge_can_active(int item_id)//need change
 {
//	 const Json::Value &magicweapon_json = CONFIG_INSTANCE->magic_weapon(item_id);
//	 CONDITION_NOTIFY_RETURN(magicweapon_json != Json::Value::null, RETURN_REQUEST_MAGIC_WEAPON, ERROR_CONFIG_NOT_EXIST);

//	 GamePackage *package = this->find_package(GameEnum::INDEX_PACKAGE);
//	 JUDGE_RETURN(NULL != package,-1);
//	 if(package->count_by_id(magicweapon_json["item_id"].asInt()) >= magicweapon_json["need_amount"].asInt())
//	 {
//		 return true;
//	 }
	 return false;
 }

int MLMagicWeapon::get_rama_open()
{
	return this->open_;
}

int MLMagicWeapon::set_rama_open(int open)
{
	this->open_ = open;
	return 0;
}

int MLMagicWeapon::test_max_rama()
{
	for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
	{
		MagicWeaponDetail *temp = &this->magicweapon_list_[i];
		temp->mw_rank_.rank_star_grade_ = 300;
	}

	return 0;
}

int MLMagicWeapon::player_get_reiki(Message *msg)
{
	this->player_reiki_num_ += 100000; // test
	this->change_reiki(100000, ITEM_UPGRADE_RAMA);

	for (int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
	{
		MagicWeaponDetail *temp = &this->magicweapon_list_[i];
		JUDGE_CONTINUE(temp->mw_is_activate_ != MLMagicWeapon::ALREADY_ACTIVE);
		temp->mw_is_activate_ = MLMagicWeapon::CAN_ACTIVE;
	}

	return 0;
}

int MLMagicWeapon::magic_weapon_active(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto11402502*, request, -1);

	MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(request->magicweapon_id());
	CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_id_ > 0, RETURN_REQUEST_MAGIC_WEAPON,
			ERROR_MAGICWEAPON_NOT_FOUND);

	CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_is_activate_ != MLMagicWeapon::ALREADY_ACTIVE,
			RETURN_REQUEST_MAGIC_WEAPON, ERROR_MAGIC_WEAPON_CANNOT_ACTIVE);

	CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_is_activate_ == MLMagicWeapon::CAN_ACTIVE,
			RETURN_REQUEST_MAGIC_WEAPON, ERROR_MAGIC_WEAPON_CANNOT_ACTIVE);

	this->magic_weapon_init_when_actived(request->magicweapon_id());
	const Json::Value &rama_json = CONFIG_INSTANCE->rama_open(request->magicweapon_id());
	this->map_logic_player()->request_sync_map_skill(rama_json["skill_id"].asInt());

	this->refresh_magic_weapon_property();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);

	Proto51402502 respond;
	respond.set_is_active(true);

	ProtoMagicWeapon *proto = respond.mutable_mw_curr_id();
	this->magic_weapon_serialize(proto, &curr_magic_weapon);
	FINER_PROCESS_RETURN(RETURN_ACTIVE_MAGIC_WEAPON, &respond);
}

 int MLMagicWeapon::magic_weapon_promote_rank(Message *msg)
 {
//	JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
	DYNAMIC_CAST_RETURN(Proto11402503 *, request, msg, -1);
	int mw_id = request->mw_id();
	MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
	CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_id_ != 0,
			RETURN_PROMOTE_RANK_MAGIC_WEAPON, ERROR_MAGICWEAPON_NOT_FOUND);

//	int inc_process = this->magic_weapon_promote_common(mw_id,request->auto_buy(),RETURN_PROMOTE_RANK_MAGIC_WEAPON,request);
//	JUDGE_RETURN(inc_process > 0,0);
	int status = this->process_magic_weapon_progress_rank(mw_id);

	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);

	Proto51402503 respond;
	respond.set_mw_id(mw_id);
	respond.set_rank_star_grade(curr_magic_weapon.mw_rank_.rank_star_grade_);
	respond.set_rank_star_progress(curr_magic_weapon.mw_rank_.rank_curr_star_progress_);
	respond.set_status(status);

	return this->respond_to_client(RETURN_PROMOTE_RANK_MAGIC_WEAPON, &respond);
 }

 int MLMagicWeapon::magic_weapon_promote_quality(Message *msg)//do without
 {
//	 JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
	 DYNAMIC_CAST_RETURN(Proto11402504 *, request, msg, -1);
	 int mw_id = request->mw_id();
	 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
//	 int rama_id = mw_id * GameEnum::RAMA_ID_BASE_NUM + curr_magic_weapon.mw_quality_.qua_star_grade_;

	 CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_id_ > 0, RETURN_PROMOTE_QUALITY_MAGIC_WEAPON, ERROR_MAGICWEAPON_NOT_FOUND);
//	 CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_rank_.rank_star_grade_ >= 10, RETURN_PROMOTE_QUALITY_MAGIC_WEAPON, ERROR_MAGICW_QUALITY_NOT_OPEN);

	 int inc_process = this->magic_weapon_promote_common(mw_id,request->auto_buy(),RETURN_PROMOTE_QUALITY_MAGIC_WEAPON,request);
	 JUDGE_RETURN(inc_process > 0,0);
	 int ret = this->process_magic_weapon_progress_quality(mw_id, inc_process);
	 CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PROMOTE_QUALITY_MAGIC_WEAPON, ret);
	 this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);

	 Proto51402504 respond;
	 respond.set_mw_id(mw_id);
	 respond.set_qua_star_grade(curr_magic_weapon.mw_quality_.qua_star_grade_);
//	 respond.set_qua_star_progress(curr_magic_weapon.mw_quality_.qua_curr_star_progress_);
	 respond.set_skill_level(curr_magic_weapon.mw_skill_lvl_);

	 respond.set_skill_id(curr_magic_weapon.mw_skill_id_);
	 return this->respond_to_client(RETURN_PROMOTE_QUALITY_MAGIC_WEAPON, &respond);
 }

 int MLMagicWeapon::magic_weapon_adorn(Message *msg)
 {
//	 JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
	 DYNAMIC_CAST_RETURN(Proto11402505 *, request, msg, -1);
	 int mw_id = request->mw_id();
	 bool put_on = request->put_on();
	 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
	 CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_id_ > 0, RETURN_ADORN_MAGIC_WEAPON, ERROR_MAGICWEAPON_NOT_FOUND);
	 CONDITION_NOTIFY_RETURN(curr_magic_weapon.mw_is_activate_ == MLMagicWeapon::ALREADY_ACTIVE,
			 RETURN_ADORN_MAGIC_WEAPON, ERROR_MAGIC_WEAPON_NOT_ACTIVE);
	 //this->curr_magic_weapon_->mw_is_adorn_ = true;
	 if(put_on)
	 {
		 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
		 {
			 MagicWeaponDetail *temp = &this->magicweapon_list_[i];
			 JUDGE_CONTINUE(temp->mw_id_!= mw_id);
			 if((temp->mw_is_activate_ == MLMagicWeapon::ALREADY_ACTIVE) && temp->mw_is_adorn_)
			 {
				temp->mw_is_adorn_ = false;
				this->sync_magic_weapon_rank_lvlup(GameEnum::ENTER_DEMOUNT); //卸下更新
				break;
			 }
		 }
		 this->sync_magic_weapon_info(curr_magic_weapon.mw_skill_id_,curr_magic_weapon.mw_skill_lvl_);
		 curr_magic_weapon.mw_is_adorn_ = true;
		 this->talisman_id_ = curr_magic_weapon.mw_id_;
		 this->talisman_rank_lvl_ =curr_magic_weapon.mw_rank_.rank_star_grade_;
		 this->sync_magic_weapon_rank_lvlup(); //佩戴更新
	 }
	 else
	 {
		 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
		 {
			 MagicWeaponDetail *temp = &this->magicweapon_list_[i];
			 if((temp->mw_is_activate_ == MLMagicWeapon::ALREADY_ACTIVE) && temp->mw_is_adorn_)
			 {
				 if(temp->mw_id_ == mw_id)
				 {
					 this->sync_magic_weapon_info(0,0);
				 }
				 temp->mw_is_adorn_ = false;
				 this->talisman_id_ = 0;
			     this->talisman_rank_lvl_ = 0;
			     this->sync_magic_weapon_rank_lvlup(GameEnum::ENTER_DEMOUNT); //卸下更新
			 }
		 }
	 }
	 this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAGIC_WEAPON, true);
	 Proto51402505 respond;
	 respond.set_pre_mw_id(curr_magic_weapon.mw_id_ * GameEnum::RAMA_ID_BASE_NUM + curr_magic_weapon.mw_rank_.rank_star_grade_);
	 respond.set_put_on(put_on);
	 respond.set_role_id(this->role_id());
	 respond.set_mw_rank_level(curr_magic_weapon.mw_rank_.rank_star_grade_);
	 if(put_on)
		respond.set_mw_id(curr_magic_weapon.mw_id_ * GameEnum::RAMA_ID_BASE_NUM + curr_magic_weapon.mw_rank_.rank_star_grade_);
	 else
		respond.set_mw_id(0);
	 return this->respond_to_client(RETURN_ADORN_MAGIC_WEAPON, &respond);
 }

 int MLMagicWeapon::sync_magic_weapon_info(const int curr_skill_id,const int curr_skill_lvl)
 {
	Proto31401002 request;
	request.set_cur_wm_skill_id(curr_skill_id);
	request.set_cur_wm_skill_level(curr_skill_lvl);
	return this->send_to_map_thread(request);
 }

 int MLMagicWeapon::sync_magic_weapon_rank_lvlup(int enter_type)
 {
	 Proto31401003 request;

	 if(enter_type == GameEnum::ENTER_DEMOUNT)
	 {//卸下
		 request.set_mw_id(0);
		 request.set_mw_rank_lvl(0);
		 request.set_mw_skill_id(0);
		 request.set_mw_skill_lvl(0);
	 }
	 else
	 if(enter_type == GameEnum::ENTER_ADORN_DEFAULT)
	 {//佩戴
		 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(talisman_id_);
		 request.set_mw_id(curr_magic_weapon.mw_id_);
		 request.set_mw_rank_lvl(curr_magic_weapon.mw_rank_.rank_star_grade_);
		 request.set_mw_skill_id(curr_magic_weapon.mw_skill_id_);
		 request.set_mw_skill_lvl(curr_magic_weapon.mw_skill_lvl_);
	 }
	 else if(enter_type == GameEnum::ENTER_LEVELUP)
	 {//当前佩戴法宝升级时同步形态
		 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(talisman_id_);
		 if(curr_magic_weapon.mw_is_adorn_)
		 {
			 request.set_mw_id(curr_magic_weapon.mw_id_);
			 request.set_mw_rank_lvl(curr_magic_weapon.mw_rank_.rank_star_grade_);
			 request.set_mw_skill_id(curr_magic_weapon.mw_skill_id_);
		     request.set_mw_skill_lvl(curr_magic_weapon.mw_skill_lvl_);
		 }
		 else
			 return 0;
	 }
	 return this->send_to_map_thread(request);
 }

 int MLMagicWeapon::magic_weapon_promote_common(int mw_id,bool auto_buy,int recogn,Message *request)
  {
	 MapLogicPlayer *player = this->map_logic_player();
	 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
	 JUDGE_RETURN(curr_magic_weapon.mw_id_ > 0,-1);

 	 const Json::Value &mw_id_json = CONFIG_INSTANCE->magic_weapon(mw_id);
 	 CONDITION_PLAYER_NOTIFY_RETURN(mw_id_json != Json::Value::null, recogn, ERROR_CONFIG_NOT_EXIST);

 	 int curr_lvl = 0;
 	 int inc_process = 0;
 	 Money cost;
 	 ItemObj item_obj;

 	 if(recogn == RETURN_PROMOTE_RANK_MAGIC_WEAPON)
 	 {
 		 curr_lvl = curr_magic_weapon.mw_rank_.rank_star_grade_;
 		 const Json::Value &cost_list_json = mw_id_json["rank_cost_item"];
 		 const Json::Value &curr_list_json = cost_list_json["item_amount"];
 		 CONDITION_PLAYER_NOTIFY_RETURN(curr_lvl < int(curr_list_json.size()), recogn, ERROR_MAGIC_WEAPON_RANK_TOP_LVL);

		 item_obj.id_ = curr_list_json[curr_lvl][0u].asInt();
		 item_obj.amount_ = curr_list_json[curr_lvl][1u].asInt();


		if (auto_buy)
		{
			player->calc_item_need_money(item_obj.id_, item_obj.amount_, cost);
		}
		else
		{
			CONDITION_PLAYER_NOTIFY_RETURN(item_obj.amount_ <= player->pack_count(item_obj.id_), recogn, ERROR_PACKAGE_ITEM_AMOUNT);
		}
		PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(cost, recogn, auto_buy, request);
		if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
		{
			GameCommon::adjust_money(cost, this->own_money());
			CONDITION_PLAYER_NOTIFY_RETURN(player->validate_money(cost), recogn, ERROR_PACKAGE_MONEY_AMOUNT);
		}
		player->pack_remove(SerialObj(ITEM_MAGICWEAPON_PROMOTE, mw_id * 100000 + 10000 + curr_magic_weapon.mw_rank_.rank_star_grade_), item_obj.id_, item_obj.amount_);
		if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
		{
			player->pack_money_sub(cost, SerialObj(SUB_MONEY_PROMOTE_MAGICWEAPON, mw_id * 100000 + 10000 + curr_magic_weapon.mw_rank_.rank_star_grade_));
		}
	    const Json::Value &refine_process_json = cost_list_json["rank_refine_process"][curr_lvl];
		int rate_range_rand = rand() % 10000, act_rate = 0;
		for (uint i = 0; i < refine_process_json.size(); ++i)
		{
			act_rate += int(refine_process_json[0u].asDouble() * 100);
			if (rate_range_rand < act_rate)
			{
				int process_1 = refine_process_json[1u].asInt(),
					process_2 = refine_process_json[2u].asInt();
				int min_process = std::min(process_1, process_2),
					max_process = std::max(process_1, process_2);
				inc_process = rand() % (max_process - min_process + 1) + min_process;
				break;
			}
		}
 	 }
 	 else
 	 if(recogn == RETURN_PROMOTE_QUALITY_MAGIC_WEAPON)
 	 {
 		 curr_lvl = curr_magic_weapon.mw_quality_.qua_star_grade_;
		 const Json::Value &cost_list_json = mw_id_json["quality_cost_item"];
		 const Json::Value &curr_list_json = cost_list_json["item_amount"];
		 CONDITION_PLAYER_NOTIFY_RETURN(curr_lvl < int(curr_list_json.size()), recogn, ERROR_MAGIC_WEAPON_QUALITY_TOP_LVL);

		 item_obj.id_ = curr_list_json[curr_lvl][0u].asInt();
		 item_obj.amount_ = curr_list_json[curr_lvl][1u].asInt();

		 if (auto_buy)
		 {
			player->calc_item_need_money(item_obj.id_, item_obj.amount_, cost);
		 }
		 else
		 {
			CONDITION_PLAYER_NOTIFY_RETURN(item_obj.amount_ <= player->pack_count(item_obj.id_), recogn, ERROR_PACKAGE_ITEM_AMOUNT);
		 }

		PROCESS_AUTO_BIND_COPPER_EXCHANGE_AUTO_REPEAT(cost, recogn, auto_buy, request);
		if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
		{
			GameCommon::adjust_money(cost, this->own_money());
			CONDITION_PLAYER_NOTIFY_RETURN(player->validate_money(cost), recogn, ERROR_PACKAGE_MONEY_AMOUNT);
		}
		player->pack_remove(SerialObj(ITEM_MAGICWEAPON_PROMOTE, mw_id * 100000 + 20000 + curr_magic_weapon.mw_quality_.qua_star_grade_), item_obj.id_, item_obj.amount_);
		if (cost.__bind_copper > 0 || cost.__copper > 0 || cost.__bind_gold > 0 || cost.__gold > 0)
		{
			player->pack_money_sub(cost, SerialObj(SUB_MONEY_PROMOTE_MAGICWEAPON, mw_id * 100000 + 20000 + curr_magic_weapon.mw_quality_.qua_star_grade_));
		}
	    const Json::Value &refine_process_json = cost_list_json["quality_refine_process"][curr_lvl];
		int rate_range_rand = rand() % 10000, act_rate = 0;
		for (uint i = 0; i < refine_process_json.size(); ++i)
		{
			act_rate += int(refine_process_json[0u].asDouble() * 100);
			if (rate_range_rand < act_rate)
			{
				int process_1 = refine_process_json[1u].asInt(),
					process_2 = refine_process_json[2u].asInt();
				int min_process = std::min(process_1, process_2),
					max_process = std::max(process_1, process_2);
				inc_process = rand() % (max_process - min_process + 1) + min_process;
				break;
			}
		}
 	 }
 	 return inc_process;
 }

 int MLMagicWeapon::process_magic_weapon_progress_rank(int mw_id)
 {
	 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
//	 JUDGE_RETURN(inc_progress >= 0 && inc_progress <= 100,-1);
	 int rama_id = mw_id * GameEnum::RAMA_ID_BASE_NUM + curr_magic_weapon.mw_rank_.rank_star_grade_;
	 const Json::Value rama_json = CONFIG_INSTANCE->rama_list(rama_id);
	 //灵气足够
	 int reiki_num = rama_json["reiki_num"].asInt();
	 if (this->player_reiki_num_ < reiki_num) return ERROR_RAMA_UPGRADE_REIKI;

	 int item_id = rama_json["item_id"].asInt();
	 int item_num = rama_json["item_num"].asInt();
	 //物品足够
	 GamePackage *package = this->find_package(GameEnum::INDEX_PACKAGE);
	 JUDGE_RETURN(NULL != package,-1);
	 if(package->count_by_id(item_id) < item_num)
	 {
		 return ERROR_RAMA_UPGRADE_GOODS;
	 }

	 IntMap temp_map;
	 temp_map[GameEnum::ITEM_ID_REIKI] = reiki_num;
	 this->sub_game_resource(temp_map,SerialObj(ITEM_UPGRADE_RAMA));
	 this->player_reiki_num_ -= reiki_num;
	 this->pack_remove(package,SerialObj(ITEM_UPGRADE_RAMA),item_id, item_num);
	 this->player_reiki_num_ = this->reiki();

	 int rand_num = std::rand() % GameEnum::RAMA_BASE_RAND_NUM;
	 if (rand_num > rama_json["success_num"].asInt())
	 {
		 return 0;
	 }
	 //升级成功

	 curr_magic_weapon.mw_rank_.rank_star_grade_ ++;
	 rama_id ++;
	 curr_magic_weapon.mw_skill_lvl_ = rama_json["skill_level"].asInt();//技能提升
	 this->refresh_magic_weapon_property();
	 this->sync_magic_weapon_rank_lvlup(GameEnum::ENTER_LEVELUP);

	 return 1;
 }

 int MLMagicWeapon::process_magic_weapon_progress_quality(int mw_id,int inc_progress)
 {
	 MagicWeaponDetail &curr_magic_weapon = this->magicweapon_set_by_id(mw_id);
	 JUDGE_RETURN(inc_progress >= 0 && inc_progress <= 100,-1);
	 if( (inc_progress + curr_magic_weapon.mw_quality_.qua_curr_star_progress_) >= 100)
	 {
		 int ret =  this->process_magic_weapon_levelup_quality(curr_magic_weapon,curr_magic_weapon.mw_quality_.qua_star_grade_ + 1);
		 JUDGE_RETURN(ret == 0,ret);
		 curr_magic_weapon.mw_quality_.qua_curr_star_progress_ = 0;
	 }
	 else
	 {
		 //this->process_magic_weapon_levelup_quality(this->curr_magic_weapon_->mw_quality_.qua_star_grade_);
		 curr_magic_weapon.mw_quality_.qua_curr_star_progress_ += inc_progress;
	 }
	 return 0;
 }

int MLMagicWeapon::process_magic_weapon_levelup_rank(MagicWeaponDetail &curr_magic_weapon,int next_lvl)
{
	return 0;
}

int MLMagicWeapon::process_magic_weapon_levelup_quality(MagicWeaponDetail &curr_magic_weapon,int next_lvl)
{
	JUDGE_RETURN(curr_magic_weapon.mw_id_ > 0,-1);

	if((next_lvl > 0) && ((next_lvl - 1) == curr_magic_weapon.mw_quality_.qua_star_grade_))
	{
		curr_magic_weapon.mw_quality_.qua_star_grade_ = next_lvl;

	    this->refresh_magic_weapon_skill(curr_magic_weapon);
	    this->refresh_magic_weapon_property();
	}
	return 0;
}

int MLMagicWeapon::refresh_magic_weapon_skill(MagicWeaponDetail &curr_magic_weapon)
{
	JUDGE_RETURN(curr_magic_weapon.mw_id_ > 0,-1);

	const Json::Value &mw_id_json = CONFIG_INSTANCE->magic_weapon(curr_magic_weapon.mw_id_);
	CONDITION_NOTIFY_RETURN(mw_id_json != Json::Value::null, RETURN_REQUEST_MAGIC_WEAPON, ERROR_CONFIG_NOT_EXIST);
	int skill_id = mw_id_json["skill_lvl"][0u].asInt();
	int min_lvl = mw_id_json["skill_lvl"][1u].asInt();
	int max_lvl = mw_id_json["skill_lvl"][2u].asInt();
	JUDGE_RETURN(min_lvl >= 0 && max_lvl > 0 && max_lvl > min_lvl,ERROR_CONFIG_ERROR);
	JUDGE_RETURN(curr_magic_weapon.mw_skill_lvl_ < max_lvl,0);//ERROR_MAGIC_WEAPON_SKILL_TOP_LVL);//技能满级不飘字
	if(curr_magic_weapon.mw_skill_id_ <=0)
		curr_magic_weapon.mw_skill_id_ = skill_id;

	if((curr_magic_weapon.mw_is_activate_ == MLMagicWeapon::ALREADY_ACTIVE) && (curr_magic_weapon.mw_rank_.rank_star_grade_ >= 10))
		curr_magic_weapon.mw_skill_lvl_ += 1;
	else
		curr_magic_weapon.mw_skill_lvl_ = 0;
	if(curr_magic_weapon.mw_is_adorn_)
	{//当前佩戴升级时同步技能等级
		 this->sync_magic_weapon_info(curr_magic_weapon.mw_skill_id_,curr_magic_weapon.mw_skill_lvl_);
	}
	return 0;
}

int MLMagicWeapon::make_up_property(const Json::Value &inc_prop,int grade,IntMap &prop_map)
{
	 if (inc_prop.isMember("health"))
	 {
		 prop_map[GameEnum::BLOOD_MAX]	+= inc_prop["health"].asInt();
	 }
	 if (inc_prop.isMember("defence"))
	 {
		 prop_map[GameEnum::DEFENSE] += inc_prop["defence"].asInt();
	 }
	 if (inc_prop.isMember("attack"))
	 {
		 prop_map[GameEnum::ATTACK] += inc_prop["attack"].asInt();
	 }
	 if (inc_prop.isMember("crit"))
	 {
		 prop_map[GameEnum::CRIT] += inc_prop["crit"].asInt();
	 }
	 if (inc_prop.isMember("toughness"))
	 {
		 prop_map[GameEnum::TOUGHNESS] += inc_prop["toughness"].asInt();
	 }

	 if (inc_prop.isMember("hit"))
	 {
		 prop_map[GameEnum::HIT] += inc_prop["hit"].asInt();
	 }
	 if (inc_prop.isMember("dodge"))
	 {
		 prop_map[GameEnum::AVOID] += inc_prop["dodge"].asInt();
	 }
	 return 0;
}

int MLMagicWeapon::refresh_magic_weapon_property(int enter_type)
{
//	JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
//	JUDGE_RETURN(this->magic_weapon_open_lvl() == true,-1);
	IntMap prop_map;
	GameCommon::reset_prop_map(prop_map);

	if(this->magicweapon_list_.empty())
	{
		this->init_magicweapon_list();
	}
	 for(int id = GameEnum::RAMA_CLASS_ID_BEGIN; id < GameEnum::RAMA_CLASS_ID_END; ++id)
	 {
		 MagicWeaponDetail *temp = &this->magicweapon_list_[id];
		if(temp->mw_is_activate_ == MLMagicWeapon::ALREADY_ACTIVE)
		{
			for (int i = 1; i <= temp->mw_rank_.rank_star_grade_; ++i)
			{
				int rama_id = id * GameEnum::RAMA_ID_BASE_NUM + i;
				const Json::Value &rama_json = CONFIG_INSTANCE->rama_list(rama_id);
				CONDITION_NOTIFY_RETURN(rama_json != Json::Value::null, RETURN_REQUEST_MAGIC_WEAPON, ERROR_CONFIG_NOT_EXIST);
//				JUDGE_CONTINUE(temp->mw_rank_.rank_star_grade_ >= 0);

				this->make_up_property(rama_json,temp->mw_rank_.rank_star_grade_,prop_map);
			}
		}
	}
	this->refresh_fight_property(BasicElement::MAGICWEAPON, prop_map,enter_type);
	return 0;
}

void MLMagicWeapon::magic_weapon_serialize(ProtoMagicWeapon *proto,MagicWeaponDetail *detail)
{
	 proto->set_magicweapon_id(detail->mw_id_);
	 proto->set_rank_star_grade(detail->mw_rank_.rank_star_grade_);
	 proto->set_rank_star_progress(detail->mw_rank_.rank_curr_star_progress_);
	 proto->set_skill_id(detail->mw_skill_id_);
	 proto->set_skill_level(detail->mw_skill_lvl_);
	 proto->set_qua_star_grade(detail->mw_quality_.qua_star_grade_);
	 proto->set_qua_star_progress(detail->mw_quality_.qua_curr_star_progress_);
	 proto->set_activate_state(detail->mw_is_activate_);
}

int MLMagicWeapon::sync_transfer_magic_weapon_info(const int scene_id)
{
//	JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
	 Proto31400247 request;
	 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
	 {
		 MagicWeaponDetail *temp = &this->magicweapon_list_[i];
		 if((temp->mw_id_ > 0) && (temp->mw_is_adorn_))
		 {
			 request.set_adorn_id(temp->mw_id_);
		 }
		 ProtoMagicWeapon *proto = request.add_mw_list();
		 this->magic_weapon_serialize(proto,temp);
	 }
	 request.set_open(this->open_);
	 return this->send_to_other_logic_thread(scene_id, request);
}

int MLMagicWeapon::read_transfer_magic_weapon_info(Message *msg)
{
//	JUDGE_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::magic_weapon),ERROR_MODEL_CLOSED);
	 MSG_DYNAMIC_CAST_RETURN(Proto31400247 *, request, -1);
	 if(!this->magicweapon_list_.empty())
		 this->magicweapon_list_.clear();
	 for(int i = 0; i < (int)request->mw_list_size(); ++i)
	 {
		const ProtoMagicWeapon& item = request->mw_list(i);
		if(item.magicweapon_id() > 0)
		{
			bool is_adorn = false;
			if(item.magicweapon_id() == request->adorn_id())
				is_adorn = true;
		MagicWeaponDetail tmp;
		this->make_up_detail(item.magicweapon_id(),item.skill_id(),item.skill_level(),
					is_adorn,item.activate_state(),item.rank_star_grade(),item.qua_star_grade(),tmp);
		this->magicweapon_list_[item.magicweapon_id()] = tmp;
		}
	 }
	if(request->adorn_id())
	{
		this->talisman_id_ = request->adorn_id();
	}
	this->open_ = request->open();
	 return 0;
}

bool MLMagicWeapon::validate_money_when_active(const Json::Value &mw_json,Money &cost)
{
	 if(mw_json.isMember("item_money"))
	 {
		int amount = mw_json["need_amount"].asInt();
		cost.__gold = mw_json["item_money"][0u].asInt() * amount ;
		cost.__bind_gold = mw_json["item_money"][1u].asInt() * amount ;
		cost.__copper = mw_json["item_money"][2u].asInt() * amount ;
		cost.__bind_copper = mw_json["item_money"][3u].asInt() * amount ;
		if(this->pack_detail().__money >= cost)
			return true;
	 }
	 return false;
}

bool MLMagicWeapon::magic_weapon_open_lvl()
 {
  	int level = 0;
  	const Json::Value& cfg = CONFIG_INSTANCE->tiny("level_limit");
  	if(cfg != Json::Value::null && cfg["magicweapon"].asInt() != 0)
  		level = cfg["magicweapon"].asInt();
  	if(this->role_level() >= level)
  	    return true;
  	return false;
}

bool MLMagicWeapon::is_open_quality()
{
	 const Json::Value &talisman_json = CONFIG_INSTANCE->task_setting()["novice_open_ui"]["current_version"]["talisman"];
	 if(talisman_json == Json::Value::null)
		 return false;
	 for(int i = GameEnum::RAMA_CLASS_ID_BEGIN; i < GameEnum::RAMA_CLASS_ID_END; ++i)
	 {
		 MagicWeaponDetail *temp = &this->magicweapon_list_[i];
		 if(temp->mw_rank_.rank_star_grade_ >= talisman_json[0u].asInt())
		 {
			 return true;
		 }
	 }
	return false;
}

int MLMagicWeapon::set_talisman_id(int id)
{
	this->talisman_id_ = id;
	return 0;
}

int MLMagicWeapon::set_talisman_rank_lvl(int lvl)
{
	this->talisman_rank_lvl_ = lvl;
	return 0;
}

int MLMagicWeapon::talisman_id()
{
	return this->talisman_id_;
}

int MLMagicWeapon::talisman_rank_lvl()
{
	return this->talisman_rank_lvl_;
}

int MLMagicWeapon::player_reiki_num()
{
	return this->player_reiki_num_;
}
