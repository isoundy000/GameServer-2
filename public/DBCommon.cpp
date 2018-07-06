/*
 * DBCommon.cpp
 *
 *  Created on: Jun 28, 2016
 *      Author: peizhibi
 */

#include "GameField.h"
#include "BackField.h"
#include "MapPlayer.h"
#include "RankStruct.h"

#include "DBCommon.h"
#include <mongo/client/dbclient.h>


void DBCommon::bson_to_customer_service_record(CustomerServiceRecord* record, const BSONObj& res)
{
	record->__record_id = res[BackCustomerSVCRecord::ID].numberLong();
	record->__sender_id = res[BackCustomerSVCRecord::SENDER_ID].numberLong();
	record->__send_tick = res[BackCustomerSVCRecord::SEND_TICK].numberLong();
	record->__sender_name = res[BackCustomerSVCRecord::SENDER_NAME].String();
	record->__content = res[BackCustomerSVCRecord::CONTENT].String();
	record->__title = res[BackCustomerSVCRecord::TITLE].String();
	record->__replay_content = res[BackCustomerSVCRecord::REPLAY_CONTENT].String();
	record->__record_type = res[BackCustomerSVCRecord::RECORD_TYPE].numberInt();
	record->__has_replay = res[BackCustomerSVCRecord::HAS_REPLAY].numberInt();
	record->__has_read = res[BackCustomerSVCRecord::HAS_READ].numberInt();
	record->__sender_level = res[BackCustomerSVCRecord::SENDER_LEVEL].numberInt();
	record->__server_code = res[BackCustomerSVCRecord::SERVER_CODE].numberInt();
	record->__platform = res[BackCustomerSVCRecord::PLATFORM].numberInt();
	record->__agent = res[BackCustomerSVCRecord::AGENT].numberInt();
	record->__evaluate_level = res[BackCustomerSVCRecord::EVALUATE_LEVEL].numberInt();
	record->__evaluate_tick = res[BackCustomerSVCRecord::EVALUATE_TICK].numberLong();
	record->__evaluate_star = res[BackCustomerSVCRecord::EVALUATE_STAR].numberInt();
	record->__opinion_index = res[BackCustomerSVCRecord::OPINION_INDEX].numberInt();
}

void DBCommon::mail_info2mail_serial_obj(const MailInformation* mail_info,
		MailDetailSerialObj& obj)
{
	obj.__title         = mail_info->mail_title_;
	obj.__content       = mail_info->mail_content_;
	obj.__sender_name   = mail_info->sender_name_;
	obj.__receiver_name = mail_info->receiver_name_;
	obj.__read_tick     = mail_info->read_tick_;
	obj.__send_tick     = mail_info->send_time_;
	obj.__sender_id     = mail_info->sender_id_;
	obj.__receiver_id   = mail_info->receiver_id_;
	obj.__mail_index    = mail_info->mail_index_;
    obj.__mail_type     = mail_info->mail_type_;
    obj.__mail_format_  = mail_info->mail_format_;
    obj.__has_read      = mail_info->has_read_;
//    obj.__attach_money  = mail_info->money_;

    MailDetailSerialObj::AttachItem& attach_item_set = obj.__attach_item;
    const ItemListMap& attach_map = mail_info->goods_map_;
    ItemListMap::const_iterator it = attach_map.begin();
    for(int i = 0; it != attach_map.end(); ++it, ++i)
    {
    	PackageItem* pack_item = it->second;
    	JUDGE_CONTINUE(NULL != pack_item);

		ThreeObj obj;
		obj.id_    = pack_item->__id;
		obj.value_ = pack_item->__amount;
		obj.tick_  = i + 1;

		attach_item_set.push_back(obj);
    }
}

BSONObj DBCommon::system_setting_to_bson(SysSetting* detail)
{
	return BSON(DBSysSetting::SetDetail::IS_SHOCK << detail->__is_shock
			<< DBSysSetting::SetDetail::SCREEN_TYPE << detail->__screen_type
			<< DBSysSetting::SetDetail::FLUENCY_TYPE << detail->__fluency_type
			<< DBSysSetting::SetDetail::SHIELD_TYPE << detail->__shield_type
			<< DBSysSetting::SetDetail::TURNOFF_ACT_NOTIFY << detail->__turnoff_act_notify
			<< DBSysSetting::SetDetail::AUTO_ADJUST_EXPRESS << detail->__auto_adjust_express
			<< DBSysSetting::SetDetail::MusicEffect::MUSIC_ON << detail->__music_effect.id_
			<< DBSysSetting::SetDetail::MusicEffect::MUSIC_VOLUME << detail->__music_effect.value_
			<< DBSysSetting::SetDetail::SoundEffect::SOUND_ON << detail->__sound_effect.id_
			<< DBSysSetting::SetDetail::SoundEffect::SOUND_VOLUME << detail->__sound_effect.value_);
}

void DBCommon::bson_to_system_setting(SysSetting* detail, const BSONObj& res)
{
	detail->__is_shock = res[DBSysSetting::SetDetail::IS_SHOCK].numberInt();
	detail->__screen_type = res[DBSysSetting::SetDetail::SCREEN_TYPE].numberInt();
	detail->__fluency_type = res[DBSysSetting::SetDetail::FLUENCY_TYPE].numberInt();
	DBCommon::bson_to_int_vec(detail->__shield_type,res.getObjectField(DBSysSetting::SetDetail::SHIELD_TYPE.c_str()));
	detail->__turnoff_act_notify = res[DBSysSetting::SetDetail::TURNOFF_ACT_NOTIFY].numberInt();
	detail->__auto_adjust_express = res[DBSysSetting::SetDetail::AUTO_ADJUST_EXPRESS].numberInt();
	detail->__music_effect.id_ = res[DBSysSetting::SetDetail::MusicEffect::MUSIC_ON].numberInt();
	detail->__music_effect.value_ = res[DBSysSetting::SetDetail::MusicEffect::MUSIC_VOLUME].numberInt();
	detail->__sound_effect.id_ = res[DBSysSetting::SetDetail::SoundEffect::SOUND_ON].numberInt();
	detail->__sound_effect.value_ = res[DBSysSetting::SetDetail::SoundEffect::SOUND_VOLUME].numberInt();
}

BSONObj DBCommon::rank_record_to_bson(RankRecord* detail)
{
	return BSON(DBRank::RankDetail::RANK_TYPE
			<< detail->__single_player_info.__rank_type
			<< DBRank::RankDetail::RANK_VALUE
			<< detail->__single_player_info.__rank_value
			<< DBRank::RankDetail::CUR_RANK
			<< detail->__single_player_info.__cur_rank
			<< DBRank::RankDetail::LAST_RANK
			<< detail->__single_player_info.__last_rank
			<< DBRank::RankDetail::ACHIEVE_TICK
			<< detail->__single_player_info.__achive_tick
			<< DBRank::RankDetail::PLAYER_ID
			<< detail->__single_player_info.__role_id
			<< DBRank::RankDetail::DISPLAYER_CONTENT
			<< detail->__single_player_info.__role_name
			<< DBRank::RankDetail::LEAGUE_NAME
			<< detail->__single_player_info.__league_name
			<< DBRank::RankDetail::ADDIITION_ID
			<< detail->__single_player_info.__additional_id
            << DBRank::RankDetail::EXT_VALUE
            << detail->__single_player_info.__ext_value
			<< DBRank::RankDetail::VIP_TYPE
			<< detail->__single_player_info.__vip_type
			<< DBRank::RankDetail::WORSHIP_NUM
			<< detail->__single_player_info.__worship_num
			<< DBRank::RankDetail::IS_WORSHIP
			<< detail->__single_player_info.__is_worship);

}

void DBCommon::bson_to_rank_record(RankRecord* detail, const BSONObj& res)
{
	detail->__single_player_info.__cur_rank = res[DBRank::RankDetail::CUR_RANK].numberInt();
	detail->__single_player_info.__last_rank = res[DBRank::RankDetail::LAST_RANK].numberInt();
	detail->__single_player_info.__rank_type = res[DBRank::RankDetail::RANK_TYPE].numberInt();
	detail->__single_player_info.__rank_value = res[DBRank::RankDetail::RANK_VALUE].numberInt();
	detail->__single_player_info.__role_id = res[DBRank::RankDetail::PLAYER_ID].numberLong();
	detail->__single_player_info.__role_name = res[DBRank::RankDetail::DISPLAYER_CONTENT].str();
	detail->__single_player_info.__league_name = res[DBRank::RankDetail::LEAGUE_NAME].str();
	detail->__single_player_info.__achive_tick = res[DBRank::RankDetail::ACHIEVE_TICK].numberLong();
	detail->__single_player_info.__additional_id = res[DBRank::RankDetail::ADDIITION_ID].numberLong();
    detail->__single_player_info.__ext_value = res[DBRank::RankDetail::EXT_VALUE].numberLong();
    detail->__single_player_info.__vip_type = res[DBRank::RankDetail::VIP_TYPE].numberInt();
    detail->__single_player_info.__worship_num = res[DBRank::RankDetail::WORSHIP_NUM].numberLong();
    detail->__single_player_info.__is_worship = res[DBRank::RankDetail::IS_WORSHIP].numberInt();
}

BSONObj DBCommon::achieve_detail_to_bson(AchieveDetail* detail)
{
	return BSON(AchieveInfo::AchieveList::ACHIEVE_ID << detail->achieve_id_
				<< AchieveInfo::AchieveList::ACH_INDEX << detail->ach_index_
				<< AchieveInfo::AchieveList::FINISH_TICK << detail->finish_tick_
				<< AchieveInfo::AchieveList::GET_STATUS << detail->get_status_
				<< AchieveInfo::AchieveList::FINISH_NUM << detail->finish_num_
				<< AchieveInfo::AchieveList::SPECIAL_VALUE << detail->special_value_);
}

void DBCommon::bson_to_achieve_detail(AchieveDetail* detail, const BSONObj& res)
{
	detail->achieve_id_ = res[AchieveInfo::AchieveList::ACHIEVE_ID].numberInt();
	detail->ach_index_ = res[AchieveInfo::AchieveList::ACH_INDEX].numberInt();
	detail->finish_tick_ = res[AchieveInfo::AchieveList::FINISH_TICK].numberLong();
	detail->get_status_ = res[AchieveInfo::AchieveList::GET_STATUS].numberInt();
	detail->finish_num_ = res[AchieveInfo::AchieveList::FINISH_NUM].numberInt();
	detail->special_value_ = res[AchieveInfo::AchieveList::SPECIAL_VALUE].numberLong();
}

BSONObj DBCommon::player_tip_to_bson(PlayerAssistTip* detail)
{
	return BSON(DBPlayerTip::TipDetail::EVENT_ID << detail->__event_id
			<< DBPlayerTip::TipDetail::TIPS_FLAG << detail->__tips_flag);
}

void DBCommon::bson_to_player_tip(PlayerAssistTip* detail, const BSONObj& res)
{
	detail->__event_id = res[DBPlayerTip::TipDetail::EVENT_ID].numberInt();
	detail->__tips_flag = res[DBPlayerTip::TipDetail::TIPS_FLAG].numberInt();
}

void DBCommon::bson_to_int_vec(IntVec& intvec, BSONObjIterator iter)
{
	intvec.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	while (iter.more())
	{
		intvec.push_back(iter.next().numberInt());
	}
}

void DBCommon::bson_to_long_vec(LongVec& long_vec, BSONObjIterator iter)
{
	long_vec.reserve(GameEnum::DEFAULT_VECTOR_SIZE);
	while (iter.more())
	{
		long_vec.push_back(iter.next().numberLong());
	}
}

void DBCommon::bson_to_int_set(IntSet& int_set, BSONObjIterator iter, int except_zero)
{
	while (iter.more())
	{
		int number = iter.next().numberInt();
		if (except_zero == true)
		{
			JUDGE_CONTINUE(number != 0);
		}
		int_set.insert(number);
	}
}

void DBCommon::bson_to_long_set(LongSet& long_set, BSONObjIterator iter)
{
	while (iter.more())
	{
		long_set.insert(iter.next().numberLong());
	}
}

void DBCommon::int_map_to_bson(BSONVec& bson_vec, const IntMap& obj_map)
{
	bson_vec.reserve(obj_map.size());
	for (IntMap::const_iterator iter = obj_map.begin();
			iter != obj_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first != 0);
//		JUDGE_CONTINUE(iter->second != 0);

		bson_vec.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}
}

void DBCommon::bson_to_int_map(IntMap& obj_map, BSONObjIterator iter)
{
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		int obj_key = obj[DBPairObj::KEY].numberInt();
		int obj_value = obj[DBPairObj::VALUE].numberInt();

		JUDGE_CONTINUE(obj_key != 0);
		obj_map[obj_key] = obj_value;
	}
}

void DBCommon::long_map_to_bson(BSONVec& bson_vec, const LongMap& obj_map)
{
	bson_vec.reserve(obj_map.size());

	for (LongMap::const_iterator iter = obj_map.begin();
			iter != obj_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first != 0);
//		JUDGE_CONTINUE(iter->second != 0);

		bson_vec.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}
}

void DBCommon::bson_to_long_map(LongMap& obj_map, BSONObjIterator iter)
{
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		long obj_key = obj[DBPairObj::KEY].numberLong();
		long obj_value = obj[DBPairObj::VALUE].numberLong();

		JUDGE_CONTINUE(obj_key != 0);
		obj_map[obj_key] = obj_value;
	}
}

void DBCommon::pair_vec_to_bson(BSONVec& bson_vec, const IntPairVec& obj_set)
{
	bson_vec.reserve(obj_set.size());
	for (IntPairVec::const_iterator iter = obj_set.begin();
			iter != obj_set.end(); ++iter)
	{
		bson_vec.push_back(BSON(DBPairObj::KEY << iter->first
				<< DBPairObj::VALUE << iter->second));
	}
}

void DBCommon::bson_to_pair_vec(IntPairVec& obj_vec, BSONObjIterator iter)
{
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		int obj_key = obj[DBPairObj::KEY].numberInt();
		int obj_value = obj[DBPairObj::VALUE].numberInt();

		obj_vec.push_back(std::make_pair(obj_key, obj_value));
	}
}

void DBCommon::item_vec_to_bson(BSONVec& bson_vec, const ItemObjVec& obj_vec)
{
	bson_vec.reserve(obj_vec.size());
	for (ItemObjVec::const_iterator iter = obj_vec.begin();
			iter != obj_vec.end(); ++iter)
	{
		bson_vec.push_back(BSON(DBItemObj::ID << iter->id_
				<< DBItemObj::AMOUNT << iter->amount_
				<< DBItemObj::BIND << iter->bind_
				<< DBItemObj::INDEX << iter->index_));
	}
}

void DBCommon::bson_to_item_vec(ItemObjVec& obj_vec, BSONObjIterator iter)
{
	while (iter.more())
	{
		BSONObj obj = iter.next().embeddedObject();

		int id = obj[DBItemObj::ID].numberInt();
		JUDGE_CONTINUE(GameCommon::validate_item_id(id));

		ItemObj item_obj;
		item_obj.id_ = id;
		item_obj.amount_ = obj[DBItemObj::AMOUNT].numberInt();
		item_obj.bind_ 	= obj[DBItemObj::BIND].numberInt();
		item_obj.index_ = obj[DBItemObj::INDEX].numberInt();

		obj_vec.push_back(item_obj);
	}
}

BSONObj DBCommon::threeobj_to_bson(const ThreeObj& obj)
{
	return BSON(DBPairObj::KEY << obj.id_
			<< DBPairObj::VALUE << obj.value_
			<< DBPairObj::TICK << obj.tick_
			<< DBPairObj::NAME << obj.name_);
}

void DBCommon::threeobj_map_to_bson(BSONVec& bson_vec, const ThreeObjMap& obj_map)
{
	for (ThreeObjMap::const_iterator iter = obj_map.begin();
			iter != obj_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->first != 0);
		bson_vec.push_back(DBCommon::threeobj_to_bson(iter->second));
	}
}

void DBCommon::bson_to_threeobj_map(ThreeObjMap& obj_map, BSONObjIterator iter)
{
	while (iter.more())
	{
		BSONObj res = iter.next().embeddedObject();

		ThreeObj obj;
		obj.id_ 	= res[DBPairObj::KEY].numberLong();
		obj.value_ 	= res[DBPairObj::VALUE].numberInt();
		obj.tick_ 	= res[DBPairObj::TICK].numberLong();
		obj.name_ 	= res[DBPairObj::NAME].str();
		obj_map[obj.id_] = obj;
	}
}

BSONObj DBCommon::base_member_to_bson(const BaseMember& member, int type)
{
	BSONObjBuilder builder;
	builder << Role::ID << member.id_
			<< Role::NAME << member.name_
			<< Role::SEX << member.sex_;
	return builder.obj();
}

void DBCommon::bson_to_base_member(BaseMember& member, const BSONObj& res, int type)
{
	JUDGE_RETURN(res.isEmpty() == false, ;);

	member.id_ = res[Role::ID].numberLong();
	member.name_ = res[Role::NAME].str();
	member.sex_ = res[Role::SEX].numberInt();
}

BSONObj DBCommon::base_server_to_bson(const BaseServerInfo& server, int type)
{
	BSONObjBuilder builder;
	builder << DBServerInfo::SERVER_ID << server.__server_id
			<< DBServerInfo::SERVER_FLAG << server.__server_flag
			<< DBServerInfo::SERVER_PREV << server.__server_prev
			<< DBServerInfo::SERVER_NAME << server.__server_name
			<< DBServerInfo::CUR_SERVER_FLAG << server.__cur_server_flag;
	return builder.obj();
}

void DBCommon::bson_to_base_server(BaseServerInfo& server, const BSONObj& res, int type)
{
	JUDGE_RETURN(res.isEmpty() == false, ;);

	server.__server_id = res[DBServerInfo::SERVER_ID].numberInt();
	server.__server_flag = res[DBServerInfo::SERVER_FLAG].str();
	server.__server_prev = res[DBServerInfo::SERVER_PREV].str();
	server.__server_name = res[DBServerInfo::SERVER_NAME].str();
	server.__cur_server_flag = res[DBServerInfo::CUR_SERVER_FLAG].str();
}

BSONObj DBCommon::skill_to_bson(const FighterSkill* skill)
{
    return BSON(Skill::SSkill::SKILL_ID << skill->__skill_id
    			<< Skill::SSkill::LEVEL << skill->__level
    			<< Skill::SSkill::USED_TIMES << skill->__used_times);
}

void DBCommon::skill_map_to_bson(BSONVec& bson_vec, const SkillMap& skill_map)
{
	bson_vec.reserve(skill_map.size());

	for (SkillMap::const_iterator iter = skill_map.begin();
			iter != skill_map.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->second->__db_flag == 0);
		bson_vec.push_back(DBCommon::skill_to_bson(iter->second));
	}
}

BSONObj DBCommon::fighter_property_to_bson(MapPlayer* player, int type)
{
	FightDetail& detail = player->fight_detail();

	int cirt_hurt_rate 	= detail.__crit_hurt_multi_total(player) * 100;
	int damage_rate		= detail.__damage_rate_total(player) * 100;
	int reduction_rate 	= detail.__reduction_rate_total(player) * 100;

	BSONObjBuilder builder;
	builder << GameName::ATTACK << detail.__attack_total_i(player)
		<< GameName::DEFENSE << detail.__defence_total_i(player)
		<< GameName::BLOOD_MAX << detail.__blood_total_i(player)
		<< GameName::HIT << detail.__hit_total_i(player)
		<< GameName::AVOID << detail.__avoid_total_i(player)
		<< GameName::CRIT << detail.__crit_total_i(player)
		<< GameName::TOUGHNESS << detail.__toughness_total_i(player)
		<< GameName::CRIT_HURT_MULTI << cirt_hurt_rate
		<< GameName::DAMAGE_MULTI << damage_rate
		<< GameName::REDUCTION_MULTI << reduction_rate
		<< GameName::PK_VALUE << detail.__pk_value
		<< GameName::GLAMOUR << detail.__glamour;

	return builder.obj();
}

BSONObj DBCommon::fight_property_to_bson(FightProperty& prop, int type)
{
	BSONObjBuilder builder;
	builder << GameName::ATTACK << prop.attack_
			<< GameName::DEFENSE << prop.defence_
			<< GameName::BLOOD_MAX << prop.blood_max_
			<< GameName::HIT << prop.hit_
			<< GameName::AVOID << prop.avoid_
			<< GameName::CRIT << prop.crit_
			<< GameName::TOUGHNESS << prop.toughness_
			<< GameName::CRIT_HURT_MULTI << prop.crit_hurt_multi_
			<< GameName::DAMAGE_MULTI << prop.damage_multi_
			<< GameName::REDUCTION_MULTI << prop.reduction_multi_;

	return builder.obj();
}

void DBCommon::bson_to_fight_property(FightProperty& prop, const BSONObj& res, int type)
{
	JUDGE_RETURN(res.isEmpty() == false, ;);

	prop.attack_ = res[GameName::ATTACK].numberInt();
	prop.defence_ = res[GameName::DEFENSE].numberInt();
	prop.blood_max_ = res[GameName::BLOOD_MAX].numberInt();
	prop.hit_ = res[GameName::HIT].numberInt();
	prop.avoid_ = res[GameName::AVOID].numberInt();
	prop.crit_ = res[GameName::CRIT].numberInt();
	prop.toughness_ = res[GameName::TOUGHNESS].numberInt();
	prop.crit_hurt_multi_ = res[GameName::CRIT_HURT_MULTI].numberInt();
	prop.damage_multi_ = res[GameName::DAMAGE_MULTI].numberInt();
	prop.reduction_multi_ = res[GameName::REDUCTION_MULTI].numberInt();
}

