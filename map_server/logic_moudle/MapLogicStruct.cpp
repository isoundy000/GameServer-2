/*
 * MapLogicStruct.cpp
 *
 *  Created on: Dec 23, 2013
 *      Author: peizhibi
 */

#include "MapLogicStruct.h"
#include "GamePackage.h"
#include "FlowControl.h"
#include "ProtoDefine.h"
#include "MapLogicPlayer.h"
#include "MapMonitor.h"
#include "GameField.h"
#include "MLBackDailyRechargeSys.h"

const string PropName::ADD_EXP = "add_exp";
const string PropName::ADD_BLOOD = "add_blood";
const string PropName::ADD_CULTIVATION = "add_cultivation";
const string PropName::REDUCE_KILL = "reduce_kill";
const string PropName::ADD_LUCKY = "add_lucky";
const string PropName::ADD_GOLD = "gold";
const string PropName::ADD_BIND_GOLD = "bind_gold";
const string PropName::ADD_REPUTATION = "add_reputation";
const string PropName::ADD_EXPLOIT = "add_exploit";
const string PropName::ADD_CONTRI = "add_contri";
const string PropName::ADD_HONOR = "add_honor";
const string PropName::ADD_REIKI = "add_reiki";
const string PropName::ADD_PRACTICE = "add_practice";
const string PropName::ADD_SPIRIT = "add_spirit";

const string PropName::HEALTH = "health";
const string PropName::BEAST_EGG = "beast_egg";
const string PropName::EXP_PERCENT = "exp_percent";	//经验卡
const string PropName::MOUNT_LEVEL = "mount_level";
const string PropName::XIANYU_LEVEL = "xianyu_level";
const string PropName::MAGIC_LEVEL = "magic_level";
const string PropName::BEAST_LEVEL = "beast_level";
const string PropName::GODSOLIDER_LEVEL = "godsolider_level";
const string PropName::BEAST_EQUIP_LEVEL = "beast_equip_level";
const string PropName::BEAST_MOUNT_LEVEL = "beast_mount_level";
const string PropName::BEAST_WING_LEVEL	= "beast_wing_level";
const string PropName::BEAST_WING_ACT	= "beast_wing_act";
const string PropName::SWORD_POOL_EXP	= "sword_pool_exp";

const string PropName::ONE_OFFLINE_PERCENT = "one_offline_percent";
const string PropName::TWO_OFFLINE_PERCENT = "two_offline_percent";
const string PropName::VIP_OFFLINE_PERCENT = "vip_offline_percent";

const string PropName::GIFT_PACK = "gift_pack";
const string PropName::FIX_GIF_PACK = "fix_gift_pack";
const string PropName::RESP_GIF_PACK = "resp_gift_pack";
const string PropName::TOTAL_GIF_PACK = "total_gift_pack";
const string PropName::RAND_GIF_PACK = "rand_gift_pack";
const string PropName::ROTARY_TABLE = "rotary_table";
const string PropName::MAGIC_ACT_PACK = "magic_act_pack";

const string PropName::VIP_CARD = "vip_card";
const string PropName::LEVEL_UP = "level_up";
const string PropName::TRANSFER = "transfer";
const string PropName::TRANSFER_RANDOM = "transfer_random";
const string PropName::ROUTINE_FINISH = "routine_finish";
const string PropName::OPEN_BOX_BY_KEY = "open_box_by_key";
const string PropName::ADD_LABEL = "add_label";
const string PropName::CHANGE_PERMISSION = "change_permission";
const string PropName::ADD_FASHION = "add_fashion";
const string PropName::ADD_TRANSFER = "add_transfer";

const string PropName::RECHARGE = "recharge";
const string PropName::SCRIPT_COMPACT = "script_compact";
const string PropName::ADD_BEAST_SKILL = "add_beast_skill";

const string PropName::ADD_SAVVY = "add_savvy";
const string PropName::MAGIC = "magic";
const string PropName::ACTIVE_WING = "active_wing";

//离线时间丹药
const string PropName::OFFLINEHOOK_DRUG = "offlinehook_drug";
const string PropName::ADD_FISH_SCORE = "fish_score";

void PackageDetail::reset(void)
{
	for (PackageMap::iterator iter = this->__pack_map.begin();
			iter != this->__pack_map.end(); ++iter)
	{
		POOL_MONITOR->game_pack_pool()->push(iter->second);
	}

	this->__money.reset();
	this->__pack_map.clear();

	this->resource_map_.clear();
	this->use_resource_map_.clear();
    this->__auto_exchange_copper.clear();
}

UpgradeAmountInfo::UpgradeAmountInfo(int type)
{
	UpgradeAmountInfo::reset();
	this->type_ = type;
}

UpgradeAmountInfo::UpgradeAmountInfo(MLPacker* packer, const Json::Value& conf)
{
	UpgradeAmountInfo::reset();

	GamePackage* package = packer->find_package(GameEnum::INDEX_PACKAGE);
	this->set_amount_info(package, conf);
}

UpgradeAmountInfo::UpgradeAmountInfo(MLPacker* packer, int item_id, int need_amount)
{
	UpgradeAmountInfo::reset();

	GamePackage* package = packer->find_package(GameEnum::INDEX_PACKAGE);
	this->set_item_price(item_id);
	this->set_amount_info(package->count_by_id(item_id), need_amount);
}

UpgradeAmountInfo::UpgradeAmountInfo(int item_id, int need_amount, int have_amount)	//购买
{
	UpgradeAmountInfo::reset();
	this->set_item_price(item_id);
	this->set_amount_info(have_amount, need_amount);
}

void UpgradeAmountInfo::reset()
{
	this->have_amount_ 		= 0;
	this->need_amount_ 		= 0;
	this->total_use_amount_ = 0;
	this->buy_amount_ 		= 0;
	this->type_				= 0;
	this->init_flag_		= 0;
	this->buy_item_			= 0;

	this->item_price_.reset();
	this->item_id_.clear();
	this->use_amount_.clear();
}

void UpgradeAmountInfo::set_item_price(int item_id)
{
	this->buy_item_ = item_id;

	const Json::Value& item_conf = CONFIG_INSTANCE->item(this->buy_item_);
	JUDGE_RETURN(item_conf.isMember("gold_price") == true, ;);

	this->item_price_.__gold = item_conf["gold_price"][0u].asInt();
	this->item_price_.__bind_gold = item_conf["gold_price"][1u].asInt();
}

void UpgradeAmountInfo::set_amount_info(int have_amount, int need_amount)
{
	this->init_flag_ 	= true;
	this->have_amount_ 	= have_amount;
	this->need_amount_ 	= need_amount;

	this->buy_amount_ 		= std::max(need_amount - have_amount, 0);
	this->total_use_amount_ = need_amount - this->buy_amount_;
}

void UpgradeAmountInfo::set_amount_info(GamePackage* package, const Json::Value& conf)
{
	GameCommon::json_to_t_vec(this->item_id_, conf["item"]);
	JUDGE_RETURN(this->item_id_.empty() == false, ;);

	this->need_amount_ = conf["amount"].asInt();
	JUDGE_RETURN(this->need_amount_ > 0, ;);

	int left_need_amount = this->need_amount_;
	for (IntVec::iterator iter = this->item_id_.begin(); iter != this->item_id_.end(); ++iter)
	{
		int have_amount = package->count_by_id(*iter);
		this->have_amount_ += have_amount;

		if (have_amount >= left_need_amount)
		{
			this->use_amount_.push_back(left_need_amount);
			left_need_amount = 0;
			break;
		}
		else
		{
			this->use_amount_.push_back(have_amount);
			left_need_amount -= have_amount;
		}
	}

	this->set_amount_info(this->have_amount_, this->need_amount_);
	JUDGE_RETURN(this->buy_amount_ > 0, ;);

	int last_index = this->item_id_.size() - 1;
	this->set_item_price(this->item_id_[last_index]);
}

int UpgradeAmountInfo::total_money(Money& need_money, const Money& own_money)
{
	int buy_amount 	= this->buy_amount_;
	int bind_gold 	= this->item_price_.__bind_gold;

	if (bind_gold > 0 && own_money.__bind_gold > 0)
	{
		int max_buy = own_money.__bind_gold / bind_gold;
		int cur_buy = std::min<int>(max_buy, buy_amount);

		buy_amount -= cur_buy;
		need_money.__bind_gold = cur_buy * bind_gold;
		JUDGE_RETURN(buy_amount > 0, 0);
	}

	int gold = this->item_price_.__gold;
	if (gold > 0 && own_money.__gold > 0)
	{
		int max_buy = own_money.__gold / gold;
		int cur_buy = std::min<int>(max_buy, buy_amount);

		buy_amount -= cur_buy;
		need_money.__gold = cur_buy * gold;
		JUDGE_RETURN(buy_amount > 0, 0);
	}

	return ERROR_PACKAGE_GOLD_AMOUNT;
}

int UpgradeAmountInfo::total_money()
{
	return this->buy_amount_ * this->item_price_.__gold;
}

void MapLogicRoleDetail::reset(void)
{
	BaseRoleInfo::reset();
	this->__league_name.clear();

    this->__camp_id = 0;
    this->__is_death = 0;
    this->__death_tick = 0;

    this->__fight_state = 0;
    this->__open_gift_close = 0;
    this->__drop_act = 0;
    this->__is_big_act_time = 0;
    this->__ml_day_reset_tick = 0;

    this->change_name_tick_ = 0;
    this->change_sex_tick_ = 0;

    this->draw_days_.clear();
    this->draw_vips_.clear();
    this->draw_gift_.clear();

    this->wedding_self_.clear();
    this->wedding_side_.clear();
}

//trade
TradeInfo::TradeInfo()
{
	TradeInfo::reset();
}

void TradeInfo::reset()
{
	 is_trade_open = 0;
	 trade_state = 0;
	 is_locked = 0;
	 is_trading = 0;
	 curr_role_id = 0;
	 requ_role_id = 0;
     accept_role_id_list.clear();
     ItemTempVector.clear();
}

Aillusion::Aillusion()
{
	 this->id_ = 0;
	 this->unlock_ = false;
	 this->unlocktype_ = 0;
	 this->type_ = 1;
	 this->shape_expire_ = Time_Value::zero;
}

int Aillusion::expire_sec()
{
	return this->shape_expire_.sec();
}

void Aillusion::reset()
{
	 this->id_ = 0;
	 this->unlock_ = false;
	 this->unlocktype_ = 0;
	 this->type_ = 1;
	 this->shape_expire_ = Time_Value::zero;
}

int MLIntMinTimer::schedule_timer()
{
	int next_minute = GameCommon::next_minute();
	MSG_USER("MLIntMinTimer %d", next_minute);

	return GameTimer::schedule_timer(Time_Value(next_minute));
}

int MLIntMinTimer::type(void)
{
	return GTT_ML_ONE_SECOND;
}

int MLIntMinTimer::handle_timeout(const Time_Value &tv)
{
	FLOW_INSTANCE->request_load_flow_detail();
	BackDR_SYS->request_load_DR_open_time();

	this->refresh_tick(tv, Time_Value(Time_Value::MINUTE));
	return 0;
}

int MLIntMinTimer::notify_first_recharge_acti(void)
{
	return 0;
}

int MLMidNightTimer::schedule_timer()
{
	int next_day = GameCommon::next_day();
	MSG_USER("MLMidNightTimer %d", next_day);

	return GameTimer::schedule_timer(Time_Value(next_day));
}

int MLMidNightTimer::type(void)
{
	return GTT_ML_ONE_MINUTE;
}

int MLMidNightTimer::handle_timeout(const Time_Value &tv)
{
	MSG_USER("MLMidNightTimer");

	// 调用函数
	this->midnight_refresh();

	this->cancel_timer();
	GameTimer::schedule_timer(Time_Value::DAY);

	return 0;
}

int MLMidNightTimer::midnight_refresh(void)
{
	PlayerManager::LogicPlayerSet logic_player_set;
	MAP_MONITOR->get_logic_player_set(logic_player_set);

	for(PlayerManager::LogicPlayerSet::iterator it = logic_player_set.begin();
			it != logic_player_set.end(); it++)
	{
		MapLogicPlayer* player = *it;
		JUDGE_CONTINUE(player != NULL);

		player->check_and_update_exp_restore_info();
		player->exp_restore_notify_reflash();
		MapLogicPlayer::notify_player_welfare_status(player); // 这个调用会自动刷新签到信息

		// 在线奖励
//		player->logout_online_rewards();
		player->reset_today_stage();
		player->login_online_rewards();

		player->reset_every_day();
		player->cache_tick().update_cache(MapLogicPlayer::CACHE_ROLE);
	}

	return 0;
}

void MasterDetial::reset()
{
	this->beast_set_.clear();

	this->cur_beast_id_ = 0;
	this->last_beast_id_ = 0;
	this->save_beast_id_ = 0;

	this->cur_beast_sort_ = 0;
	this->last_beast_sort_ = 0;

	this->gen_skill_lucky_ = 0;
	this->gen_skill_book_.clear();
	this->beast_skill_open_ = 0;
}

bool MasterDetial::validate_beast(Int64 beast_id)
{
	JUDGE_RETURN(beast_id > 0, false);
	return GameCommon::find_first(this->beast_set_, beast_id);
}

void LBFightDetail::reset()
{
	this->attack_.reset();
	this->crit_.reset();
	this->hit_.reset();
	this->attack_mul_.reset();
	this->crit_mul_.reset();
	this->hit_mul_.reset();
}

SellOutDetail::SellOutDetail(void)
{ /*NULL*/ }

void SellOutDetail::reset(void)
{
	for (ItemList::iterator iter = this->sell_out_.begin();
			iter != this->sell_out_.end(); ++iter)
	{
		GamePackage::push_item(*iter);
	}

	this->sell_out_.clear();
}

void RealmDetail::reset(void)
{
	this->__realm_id = 1;
    this->realmInfo.reset();
}

RealmDetail::RealmDetail(void):
__realm_id(1)
{/*NULL*/}

void RealmDetail::RealmInfo::reset(void)
{
    this->__step = 1;
    this->__property_index = 0;
    this->__cur_lingqi = 0;
    this->__cur_xianqi = 0;
    this->__total_blood = 0;
    this->__total_defence = 0;
    this->__total_crit = 0;
    this->__total_toughness = 0;
    this->__total_hit = 0;
    this->__total_avoid = 0;
    this->__total_attack= 0;

}

RealmDetail::RealmInfo::RealmInfo(void):
 __step(1), __property_index(0), __cur_xianqi(0),__cur_lingqi(0),__total_blood(0),__total_defence(0),__total_crit(0),__total_toughness(0),
 __total_hit(0),__total_avoid(0),__total_attack(0)
{ /*NULL*/ }




void SellOutDetail::add_sell_out(PackageItem* pack_item, int sell_amount)
{
	JUDGE_RETURN(pack_item != NULL && sell_amount > 0, ;);

	while (this->sell_out_.size() >= GameEnum::MAX_KEEP_SELL_OUT)
	{
		PackageItem* push_item = this->sell_out_.front();
		GamePackage::push_item(push_item);
		this->sell_out_.pop_front();
	}

	PackageItem* pop_item = GamePackage::pop_item(pack_item->__id);
	JUDGE_RETURN(pop_item != NULL, ;);

	*pop_item = *pack_item;

	pop_item->__amount = sell_amount;
	this->sell_out_.push_back(pop_item);
}

SkillCombineResult::SkillCombineResult(void)
{
	this->combine_exp_ = 0;
	this->combined_exp_ = 0;
}

void SkillCombineResult::serialize(ProtoSkillCombine* proto_combine)
{
	JUDGE_RETURN(proto_combine != NULL, ;);

	proto_combine->set_combine_flag(false);
	proto_combine->set_combined_exp(this->combined_exp_);
}

StorageRecord::StorageRecord(void) :
	__storage_id(0), __finish_count(0),__storage_valid(true)
{
	this->__record_timestamp = Time_Value::zero;
}

void StorageRecord::reset()
{
	this->__storage_valid	= true;
	this->__storage_id		= 0;
	this->__finish_count	= 0;
	this->__record_timestamp = Time_Value::zero;
}

ExpRestoreDetail::ExpRestoreDetail(void) :
    __vip_type_record(0), __vip_start_time(0), __vip_expried_time(0)
{
	this->__check_timestamp = Time_Value::zero;
	this->__timestamp_level_map.clear();
	this->__timestamp_vip_map.clear();
	this->__storage_record_set.clear();
	this->__storage_stage_info.clear();
	this->__pre_act_map.clear();
	this->__now_act_map.clear();
}

void ExpRestoreDetail::reset(void)
{
	this->__vip_type_record = 0;
	this->__vip_start_time = 0;
	this->__vip_expried_time = 0;
	this->__check_timestamp = Time_Value::zero;
	this->__timestamp_level_map.clear();
	this->__timestamp_vip_map.clear();
	this->__storage_record_set.clear();
	this->__storage_stage_info.clear();
	this->__pre_act_map.clear();
	this->__now_act_map.clear();
}

ScriptCleanDetail::ScriptInfo::ScriptInfo(void) :
    __script_sort(0), __chapter_key(0), __use_times(0), 
    __use_tick(0), __reset_times(0), __protect_beast_index(-1)
{ /*NULL*/ }

void ScriptCleanDetail::ScriptInfo::reset(void)
{
    ::memset(this, 0, sizeof(ScriptCleanDetail::ScriptInfo));
    this->__protect_beast_index = -1;
}

void ScriptCleanDetail::reset(void)
{
    this->__current_script_vc.clear();
    this->__finish_script_vc.clear();

    this->__item_map.clear();
    this->__drop_map.clear();

    this->__get_item_map.clear();
    this->__get_drop_map.clear();

    this->__script_item_map.clear();

    this->__drop_dragon_script_sort = 0;
    this->__drop_dragon_enter_teamers = 0;

    this->__exp = 0;
    this->__savvy = 0;
    this->__money = Money(0);
    this->__clean_sort_list.clear();
    this->__reset_times = 0;
    this->__top_floor = 0;
    this->__pass_wave = 0;
    this->__pass_chapter = 0;
    this->__start_wave = 0;
    this->__start_chapter = 0;
    this->__mult = 0;

    this->__state = 0;
    this->__begin_tick = Time_Value::zero;
    this->__end_tick = Time_Value::zero;
    this->__terminate_tick = Time_Value::zero;
    this->__next_check_tick = Time_Value::zero;
}

void ScriptCleanDetail::reset_award(void)
{
    this->__item_map.clear();
    this->__drop_map.clear();
    this->__get_item_map.clear();
    this->__get_drop_map.clear();
    this->__script_item_map.clear();
    this->__exp = 0;
    this->__savvy = 0;
    this->__money = Money(0);
    this->__clean_sort_list.clear();
    this->__reset_times = 0;
    this->__top_floor = 0;
    this->__pass_wave = 0;
    this->__pass_chapter = 0;
    this->__start_wave = 0;
    this->__start_chapter = 0;
    this->__mult = 0;
}

void ScriptCompactDetail::reset(void)
{
    this->__compact_type = 0;
    this->__start_tick = Time_Value::zero;
    this->__expired_tick = Time_Value::zero;
    this->__sys_notify = 0;
}

SmeltInfo::SmeltInfo():
	__smelt_level(1),__smelt_exp(0),__recommend(1),open_(0)
{ /*NULL*/ }

void SmeltInfo::reset(void)
{
	this->__smelt_level = 1;
	this->__smelt_exp = 0;
	this->__recommend = 1;
	this->open_ = 0;
}

void RamaDetail::reset()
{
	this->cur_reiki_num = 0;
	this->cur_rama_prop.clear();
	this->player_rama_map.clear();
}

void ReincarnationDetail::reset(void)
{
	this->cur_cultivation = 0;
	this->round = 0;
    this->extra_exp = 0;
    this->exchange_times = 0;
    this->tick.sec(0);
}


ActiCodeDetail::ActiCodeDetail(void)
{
	ActiCodeDetail::reset();
}

void ActiCodeDetail::reset(void)
{
	this->__id = 0;
	this->__user_id = 0;
	this->__gift_sort = 0;
	this->__amount = 0;
	this->__start_time = 0;
	this->__end_time = 0;
	this->__used_time = 0;
	this->__batch_id = 0;
	this->__use_only_vip = 0;
	this->__use_only_vip_client = 0;
	this->__des_mail.clear();
	memset(this->__acti_code, 0, sizeof(this->__acti_code));
}

MediaGiftBatchDetail::MediaGiftBatchDetail(void):
	__batch_id(0), __gen_num(0), __gen_time(0),
	__gift_sort(0), __agent_id(0), __platform_id(0)
{	/*NULL*/	}

void MediaGiftBatchDetail::reset(void)
{
	this->__batch_id = 0;
	this->__gen_num = 0;
	this->__gen_time = 0;
	this->__gift_sort = 0;
	this->__agent_id = 0;
	this->__platform_id = 0;
}

MediaGiftDef::MediaGiftDef(void)
{
	MediaGiftDef::reset();
}

void MediaGiftDef::reset(void)
{
	this->__gift_sort = 0;
	this->__gift_type = 0;
	this->__gift_tag = 0;
	this->__use_times = 0;
	this->__show_icon = 0;
	this->__hide_used = 0;
	this->__is_share = 0;
	this->__expire_time = 0;

	this->__value_ext.clear();
	this->__gift_list.clear();
	this->__font_color.clear();

	memset(this->__gift_name, 0, sizeof(this->__gift_name));
	memset(this->__gift_desc, 0, sizeof(this->__gift_desc));
}

PlayerMediaGiftDetail::PlayerMediaGiftDetail(void):
	__last_query_tick(0)
{
	this->__gift_use_times.clear();
	this->__gift_use_tags.clear();
	this->__gift_use_tick.clear();
	this->__acti_code_map.clear();
}

void PlayerMediaGiftDetail::reset(void)
{
	this->__last_query_tick = 0;
	this->__gift_use_times.clear();
	this->__gift_use_tags.clear();
	this->__gift_use_tick.clear();
	this->__acti_code_map.clear();
}

RechargeDetail::RechargeDetail(void)
{
	RechargeDetail::reset();
}

void RechargeDetail::reset(void)
{
	this->__recharge_money = 0;
	this->__recharge_times = 0;
	this->__feedback_awards = 0;
	this->__recharge_type = 1;	//默认为1
	this->__first_recharge_time = 0;
	this->__last_recharge_time = 0;
	this->__recharge_map.clear();
	this->__recharge_awards.clear();
	this->__latest_order_set.clear();
	this->__prev_order_set.clear();
	this->__order_fresh_tick = Time_Value::zero;
	this->__love_gift_index = 0;
	this->__love_gift_recharge = 0;
}

void RechargeDetail::record_recharge(int gold)
{
	this->__recharge_money += gold;
	this->__recharge_times += 1;
	this->__last_recharge_time = ::time(0);
	JUDGE_RETURN(this->__first_recharge_time == 0, ;);

	this->__first_recharge_time = this->__last_recharge_time;
}

int RechargeDetail::has_recharge(int index)
{
	return this->__recharge_map.count(index) > 0;
}

DailyRechargeDetail::DailyRechargeDetail(void)
{
	DailyRechargeDetail::reset();
}

// 确定你不是需要 MLDailyRecharge::reset() ?
void DailyRechargeDetail::reset(void)
{
	this->__today_recharge_gold = 0;
	this->__last_recharge_time = 0;
	this->__first_recharge_gold = 0;
	this->__act_recharge_times = 0;
	this->__act_has_mail = 0;
	this->__daily_recharge_rewards.clear();
	for (int i = MLDailyRecharge::DAILY_FIRST; i < MLDailyRecharge::TYPE_NUM; ++i)
	{
		this->__daily_recharge_rewards[i] = 0;
	}
};

RebateRechargeDetail::RebateRechargeDetail(void):
__rebate_times(0), __last_buy_time(0)
{

}

void RebateRechargeDetail::reset(void)
{
	this->__last_buy_time = 0;
	this->__rebate_times = 0;
}

LevelRewardsDetail::LevelRewardsDetail(void):__awards_num(0)
{
	/*NULL*/
}

void LevelRewardsDetail::reset(void)
{
	this->__rewards_map.clear();
	this->__awards_num = 0;
}

TinyDetail::FundItem::FundItem()
{
	FundItem::reset();
}

void TinyDetail::FundItem::reset()
{
	this->buy_flag_ = 0;
	this->draw_times_ = 0;
	this->draw_flag_ = 0;
	this->total_times_ = 0;
	this->get_tick_ = 0;
}

void TinyDetail::reset()
{
	for (uint i = 0; i < TOTAL_FUND; ++i)
	{
		this->fund_set_[i].reset();
	}

	this->guide_map_.clear();
}

WingDetail::SingleWingInfo::SingleWingInfo(void) :
    __wing_id(0), __wing_level(0), __wing_process(0)
{ /*NULL*/ }

void WingDetail::SingleWingInfo::reset(void)
{
    this->__wing_id = 0;
    this->__wing_level = 0;
    this->__wing_process = 0;
}

void WingDetail::reset(void)
{
    this->__wing_map.clear();
    this->__curr_wing_id = 0;
    this->__curr_wing_skill = 0;
    this->__curr_wing_skill_level = 0;
    this->__is_first_active_wing = 0;
}

void TotalRechargeDetail::reset()
{
	this->__reward_states.clear();
}

Box::Box(): __box_open_count_one(0), __box_open_count_ten(0),__box_open_count_fifty(0),__box_is_open(0)
{
}
void Box::reset(void)
{
	this->__box_open_count_one = 0;
	this->__box_open_count_ten = 0;
	this->__box_open_count_fifty = 0;
    this->__box_is_open = 0;
}


Box::~Box()
{
}
RoleExDetail::RoleExDetail(void) :__savvy(0), __is_second_equip_decompose(0)
{
}
void RoleExDetail::reset(void)
{
    this->__savvy = 0;
	this->__box.reset();
    this->__is_second_equip_decompose = 0;
}

ItemColor::ItemColor(void)
{
    this->__item_id = 0;
    this->__item_amount = 0;
    this->__item_color = 0;
}

bool ItemColorCmp(const ItemColor &left, const ItemColor &right)
{
    if (left.__item_color == right.__item_color)
    {
        return left.__item_id < right.__item_id;
    }
    return left.__item_color > right.__item_color;
}

BaseAchieveInfo::AchieveInfo::AchieveInfo(void)
{
	AchieveInfo::reset();
}

void BaseAchieveInfo::AchieveInfo::reset(void)
{
	this->achieve_id_ 	= 0;
	this->need_amount_ 	= 0;
	this->sort_ 		= 0;
	this->achieve_type_ = 0;
	this->number_type_ 	= 0;
	this->reward_id_ 	= 0;
	this->ach_amount_ 	= 0;
}

BaseAchieveInfo::ChildAchieve::ChildAchieve(void)
{
	ChildAchieve::reset();
}

void BaseAchieveInfo::ChildAchieve::reset(void)
{
	this->ach_index_ 	= 0;
	this->base_type_ 	= 0;
	this->child_type_ 	= 0;
	this->compare_ 		= 0;
	this->sort_ 		= 0;
	this->red_point_ 	= 0;

	this->ach_info_set_.clear();
}

BaseAchieveInfo::BaseAchieveInfo(void)
{
	BaseAchieveInfo::reset();
}

void BaseAchieveInfo::reset(void)
{
	this->achieve_level_ = 0;
	this->achieve_point_map_.clear();
	this->child_achieve_map_.clear();
}

int BaseAchieveInfo::add_new_achieve(int index, const Json::Value& conf)
{
	ChildAchieve &achieve = this->child_achieve_map_[index];
	achieve.ach_index_ = index;

	this->update_achieve(&achieve, conf);
//	this->child_achieve_set_.push_back(achieve);

	return achieve.ach_index_;
}

int BaseAchieveInfo::update_achieve(ChildAchieve* achieve, const Json::Value& conf)
{
	achieve->base_type_ 	= conf[DBAchieve::BASE_TYPE].asInt();
	achieve->child_type_ 	= conf[DBAchieve::CHILD_TYPE].asInt();
	achieve->compare_ 		= conf[DBAchieve::COMPARE].asInt();
	achieve->sort_ 			= conf[DBAchieve::SORT].asInt();
	achieve->red_point_ 	= conf[DBAchieve::RED_EVENT].asInt();

	GameConfig::ConfigMap& ach_list_map = CONFIG_INSTANCE->achieve_list_map();
	GameConfig::ConfigMap::iterator iter = ach_list_map.begin();
	for (; iter != ach_list_map.end(); ++iter)
	{
		const Json::Value& conf = *(iter->second);
		JUDGE_CONTINUE(achieve->ach_index_ == conf[DBAchieve::ACH_INDEX].asInt());

		AchieveInfo achieve_info;
		achieve_info.achieve_id_ 	= iter->first;
		achieve_info.need_amount_ 	= conf[DBAchieve::AchieveDetail::NEED_AMOUNT].asInt();
		achieve_info.sort_ 			= conf[DBAchieve::AchieveDetail::SORT].asInt();
		achieve_info.achieve_type_ 	= conf[DBAchieve::AchieveDetail::ACHIEVE_TYPE].asInt();
		achieve_info.number_type_	= conf[DBAchieve::AchieveDetail::NUMBER_TYPE].asInt();
		achieve_info.reward_id_ 	= conf[DBAchieve::AchieveDetail::REWARD_ID].asInt();
		achieve_info.ach_amount_ 	= conf[DBAchieve::AchieveDetail::ACH_AMOUNT].asInt();
		achieve->ach_info_set_.push_back(achieve_info);
	}

	JUDGE_RETURN(this->achieve_point_map_.count(achieve->base_type_) <= 0, 0);
	this->achieve_point_map_[achieve->base_type_] = 0;

	return 0;
}

BaseAchieveInfo::ChildAchieve* BaseAchieveInfo::find_child_achieve(int ach_index)
{
//	BaseAchieveInfo::ChildAchieveSet::iterator iter = this->child_achieve_set_.begin();
//	for (; iter != this->child_achieve_set_.end(); ++iter)
//	{
//		JUDGE_CONTINUE(iter->ach_index_ == ach_index);
//		return &(*iter);
//	}
	JUDGE_RETURN(this->child_achieve_map_.count(ach_index) > 0, NULL);
	return &(this->child_achieve_map_[ach_index]);
}

BaseAchieveInfo::AchieveInfo* BaseAchieveInfo::find_achieve_info(int ach_index, int achieve_id)
{
	BaseAchieveInfo::ChildAchieve* child_achieve = this->find_child_achieve(ach_index);
	return this->find_achieve_info(child_achieve, achieve_id);
}

BaseAchieveInfo::AchieveInfo* BaseAchieveInfo::find_achieve_info(BaseAchieveInfo::ChildAchieve* child_achieve, int achieve_id)
{
	BaseAchieveInfo::AchieveInfoSet::iterator iter = child_achieve->ach_info_set_.begin();
	for (; iter != child_achieve->ach_info_set_.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->achieve_id_ == achieve_id);
		return &(*iter);
	}
	return NULL;
}

BaseAchieveInfo::AchieveInfo* BaseAchieveInfo::find_achieve_info(int achieve_id)
{
//	BaseAchieveInfo::ChildAchieveSet::iterator iter = this->child_achieve_set_.begin();
//	for (; iter != this->child_achieve_set_.end(); ++iter)
//	{
//		BaseAchieveInfo::ChildAchieve& child_achieve = *iter;
//		BaseAchieveInfo::AchieveInfoSet::iterator it = child_achieve.ach_info_set_.begin();
//		for (; it != child_achieve.ach_info_set_.end(); ++it)
//		{
//			JUDGE_CONTINUE(it->achieve_id_ == achieve_id);
//			return &(*it);
//		}
//	}
	return NULL;
}


