/*
 * DBCommon.h
 *
 *  Created on: Jun 28, 2016
 *      Author: peizhibi
 */

#ifndef DBCOMMON_H_
#define DBCOMMON_H_

#include "GameHeader.h"

class BaseMember;
class RechargeRankItem;
class CustomerServiceRecord;
class RechargeRankExtraAwarder;

class DBCommon
{
public:
    /*
     * CustomerServiceRecord and bson
     * */
    static void bson_to_customer_service_record(CustomerServiceRecord* record, const BSONObj& res);

	/*
	 * 后台邮件流水
	 * MailDetailSerialObj to MailInformation
	 * */
	static void mail_info2mail_serial_obj(const MailInformation* mail_info, MailDetailSerialObj& obj);


    /*
     * system setting and bson
     * */
    static BSONObj system_setting_to_bson(SysSetting* detail);
    static void bson_to_system_setting(SysSetting* detail, const BSONObj& res);

    /*
     * rank record and bson
     * */
    static BSONObj rank_record_to_bson(RankRecord* detail);
    static void bson_to_rank_record(RankRecord* detail, const BSONObj& res);

    /*
     * achieve detail and bson
     * */
    static BSONObj achieve_detail_to_bson(AchieveDetail* detail);
    static void bson_to_achieve_detail(AchieveDetail* detail, const BSONObj& res);

    /*
     * player tip and bson
     * */
    static BSONObj player_tip_to_bson(PlayerAssistTip* detail);
    static void bson_to_player_tip(PlayerAssistTip* detail, const BSONObj& res);

    /*
     * vec and bson
     * */
    static void bson_to_int_vec(IntVec& int_vec, BSONObjIterator iter);
    static void bson_to_long_vec(LongVec& long_vec, BSONObjIterator iter);

    /*
     * set and bson
     * */
    static void bson_to_int_set(IntSet& int_set, BSONObjIterator iter, int except_zero = false);
    static void bson_to_long_set(LongSet& long_set, BSONObjIterator iter);

    /*
     * IntMap and bson
     * */
    static void int_map_to_bson(BSONVec& bson_vec, const IntMap& obj_map);
    static void bson_to_int_map(IntMap& obj_map, BSONObjIterator iter);

    /*
     * LongMap and bson
     * */
    static void long_map_to_bson(BSONVec& bson_vec, const LongMap& obj_map);
    static void bson_to_long_map(LongMap& obj_map, BSONObjIterator iter);

    /*
     * IntPair vector and bson
     * */
    static void pair_vec_to_bson(BSONVec& bson_vec,const IntPairVec& obj_set);
    static void bson_to_pair_vec(IntPairVec& obj_set, BSONObjIterator iter);

    /*
     * ItemVec and bson
     * */
    static void item_vec_to_bson(BSONVec& bson_vec, const ItemObjVec& obj_vec);
    static void bson_to_item_vec(ItemObjVec& obj_vec, BSONObjIterator iter);

    /*
     * ThreeObj and bson
     * */
    static BSONObj threeobj_to_bson(const ThreeObj& obj);

    static void threeobj_map_to_bson(BSONVec& bson_vec, const ThreeObjMap& obj_map);
    static void bson_to_threeobj_map(ThreeObjMap& obj_map, BSONObjIterator iter);

    /*
     * BaseMember and bson
     * */
    static BSONObj base_member_to_bson(const BaseMember& member, int type = 0);
    static void bson_to_base_member(BaseMember& member, const BSONObj& res, int type = 0);

    /*
     * BaseServerInfo and bson
     * */
    static BSONObj base_server_to_bson(const BaseServerInfo& server, int type = 0);
    static void bson_to_base_server(BaseServerInfo& member, const BSONObj& res, int type = 0);

    /*
     * SkillMap and bson
     * */
    static BSONObj skill_to_bson(const FighterSkill* skill);
    static void skill_map_to_bson(BSONVec& bson_vec, const SkillMap& skill_map);

    /*
     * fighter property and bson
     * */
    static BSONObj fighter_property_to_bson(MapPlayer* player, int type = 0);
    static BSONObj fight_property_to_bson(FightProperty& prop, int type = 0);
    static void bson_to_fight_property(FightProperty& prop, const BSONObj& res, int type = 0);
};

#endif /* DBCOMMON_H_ */
