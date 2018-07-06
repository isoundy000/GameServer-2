/*
 * MLMagicWeapon.h
 *
 *  Created on: 2015-12-14
 *      Author: xu
 */

#ifndef MLMAGICWEAPON_H_
#define MLMAGICWEAPON_H_

#include "MLPacker.h"

class ProtoMagicWeapon;

struct MagicWeaponDetail
{
	int mw_id_;  //罗摩id
	int mw_is_activate_; //罗摩是否激活
	bool mw_is_adorn_;  //罗摩是否被佩戴
	int mw_skill_id_;   //罗摩对应的技能id
	int mw_skill_lvl_;  //罗摩对应技能的等级

	struct MagicRank
	{
		//法宝品阶即法宝淬炼  //罗摩信息
		int rank_star_grade_; //当前淬炼星级(共70星) //罗摩等级
		int rank_curr_star_progress_; //升级到下一星的当前进度
		MagicRank();
		void reset();
	};

	struct MagicQuality
	{
		int qua_star_grade_; //罗摩注灵星级(共20星)
		int qua_curr_star_progress_; //注灵要升级到下一星的当前进度
		MagicQuality();
		void reset();
	};

	MagicRank mw_rank_;
	MagicQuality mw_quality_;

	MagicWeaponDetail();
	void reset();

	bool validate_id() const;
};

//typedef std::vector<MagicWeaponDetail> MagicWeaponVec;
typedef std::map<int,MagicWeaponDetail> MagicWeaponMap;

class MLMagicWeapon:virtual public MLPacker
{
public:
	enum
	{
		NOT_ACTIVE = 0,
		ALREADY_ACTIVE = 1,
		CAN_ACTIVE = 2,
		END_ACTIVE,
	};

public:
	 MLMagicWeapon();
	 virtual ~MLMagicWeapon();

	 void reset_magicweapon();
	 MagicWeaponMap &magicweapon_list();
	 void make_up_detail(int id, int skill_id, int skill_lvl, bool is_adorn,
			 int is_active, int rank_grade, int qua_grade, MagicWeaponDetail& ref_mwd);

public:
	void magic_weapon_init_when_actived(int mw_id,int rank_grade = 1,int quality_grade = 1);
	 MagicWeaponDetail &magicweapon_set_by_id(int mw_id);

	 int check_open_rama(int task_id);
	 int magic_weapon_fetch_info();

	 int change_magic_weapon_status(const int type, const int ext_num);//不同方式获得新的罗摩
	 int init_magicweapon_list();  //初始化罗摩
	 int magic_weapon_active(Message *msg);
	 int magic_weapon_promote_rank(Message *msg);
	 int magic_weapon_promote_quality(Message *msg);
	 int magic_weapon_adorn(Message *msg);
	 int magic_weapon_promote_common(int mw_id,bool auto_buy,int recogn,Message *msg);

	 int process_magic_weapon_levelup_rank(MagicWeaponDetail &curr_magic_weapon,int next_lvl);
	 int process_magic_weapon_levelup_quality(MagicWeaponDetail &curr_magic_weapon,int next_lvl);
	 int process_magic_weapon_progress_rank(int mw_id);
	 int process_magic_weapon_progress_quality(int mw_id,int inc_progress);

	 int refresh_magic_weapon_skill(MagicWeaponDetail &curr_magic_weapon);
	 int sync_magic_weapon_info(const int curr_skill_id,const int curr_skill_lvl);
	 int sync_magic_weapon_rank_lvlup(int enter_type=0);
	 int sync_transfer_magic_weapon_info(const int scene_id);
	 int read_transfer_magic_weapon_info(Message *msg);
	 int set_talisman_id(int id);
	 int set_talisman_rank_lvl(int lvl);
	 int talisman_id();
	 int talisman_rank_lvl();
	 int player_reiki_num();
	 int make_up_property(const Json::Value &inc_prop,int grade,IntMap &prop_map);

	 bool magic_weapon_open_lvl();
	 bool validate_money_when_active(const Json::Value &json,Money &cost);
	 bool is_open_quality();
	 bool judge_can_active(int item_id);

	 int player_get_reiki(Message *msg);//玩家获得灵气
	 int test_max_rama();

	 int get_rama_open();
	 int set_rama_open(int open);

protected:
	 void magic_weapon_serialize(ProtoMagicWeapon *proto,MagicWeaponDetail *detail);
	 int refresh_magic_weapon_property(int enter_type=0);

private:
	 MagicWeaponDetail null_magic_weapon_;
	 MagicWeaponMap magicweapon_list_; //所有罗摩列表,0未激活，1已激活，2，可激活
	 int open_;	//模块是否开启
	 int talisman_id_; //当前佩戴罗摩id
	 int talisman_rank_lvl_; //当前佩戴罗摩品阶等级
	 int player_reiki_num_; //当前玩家灵气
};



#endif /* MLMAGICWEAPON_H_ */
