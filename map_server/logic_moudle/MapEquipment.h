/*
 * MapEquipment.h
 *
 *  Created on: 2013-11-15
 *      Author: louis
 */

#ifndef MAPEQUIPMENT_H_
#define MAPEQUIPMENT_H_

#include "MLPacker.h"

class MapEquipment  : virtual public MLPacker
{
public:
	MapEquipment();
	virtual ~MapEquipment();

	static int equip_red_item_pair();
	static string equip_red_item_name(int index);

	static int equip_refine_item_pair();
	static string equip_refine_item_name(int index);

	SmeltInfo& smelt_info(void);

	void map_logic_equipment_reset(void);

	int fetch_equip_refine_panel_info(Message* msg);/*客户端获取下次强化的材料和金钱消耗*/
	int equip_strengthen(Message* msg);/*强化*/
	int equip_red_uprising(Message* msg);/*红装升阶*/
    int equip_orange_uprising(Message *msg); /*橙炼*/
	int equip_brighten(Message* msg);/*铸光*/

	void check_smelt_pa_event();
	void open_smelt(const int task_id);
	int calc_item_prop(int equip_id);
	int equip_smelt(Message* msg);/*熔炼*/
	int equip_smelt(int num);
	int fetch_equip_smelt_panel_info();/*熔炼信息*/
	int change_smelt_recommend(Message* msg);
	void get_smelt_item(IntVec &equip);
	int calc_prop_map(int equip_id);
	void refresh_smelt_prop(int enter_type = 0);

	int equip_god_refine(Message* msg);//神炼
	int equip_god_refine_diff(Message* msg);//differ value

	int put_on_equip(Message* msg);/*穿装备*/
	int put_on_equip(int pkg_index, int pkg_type = GameEnum::INDEX_PACKAGE);
	int take_off_equip(Message* msg);
	int fetch_magical_panel_info(void);/*client获取神兵界面信息:已开发/未开发的神兵，神兵加成的属性*/
	int activate_magical_item(Message* msg);/*激活神兵（按照等级段激活）*/
	int fetch_magical_detail_info(Message* msg);/*点开神兵时，获取神兵的四个属性*/
	int magical_polish(Message* msg);/*神兵洗练*/
	int magical_polish_select_result(Message* msg);/**替换神兵属性**/
	int magical_polish_clear_single_record(Message* msg);

	int equip_inherit(Message* msg);	//装备继承
	int equip_tempered(Message* msg);	//装备淬练
	int equip_insert_jewel(Message* msg);	//装备镶嵌宝石
	int equip_good_refine(Message* msg);	//装备精炼
	int equip_polish(Message* msg);			//装备洗练
	int equip_polish_fetch_attr_list(Message* msg);	//装备洗练时获取需要进行洗练的属性列表
	int equip_polish_replace_attr(Message* msg);	//装备洗练：属性替换

	int equip_compose(Message* msg);	//宝石合成
    int equip_decompose(Message *msg);  //装备分解

    int equip_remove_jewel(Message *msg);				//镶嵌的宝石卸下
    int equip_upgrade_jewel(Message *msg);				//镶嵌的宝石升级
    int validate_open_sublime_jewel(void);				//效验宝石升华功能是否可以开启
    int fetch_equip_sublime_jewel_info(void);			//获取宝石升华信息
    int refresh_jewel_sublime_prop(int enter_type = 0);	//刷新宝石升华属性
    int equip_sublime_jewel(void);						//镶嵌的宝石升华操作
    bool calc_equip_attached_property(Int64 &blood_max, Int64 &attack, Int64 &defense); //计算身上的装备镶嵌后的附加属性

    int fetch_molding_spirit_info(Message *msg);		//获取铸魂信息
    int fetch_molding_spirit_info(int index);			//获取铸魂信息
    int fetch_molding_spirit_all_equip_info();			//获取所有装备铸魂信息
	int equip_molding_spirit(Message *msg);				//装备铸魂
	int molding_spirit_rand_nature();					//随机属性
	int test_command_clean_molding_nature();			//清空属性


	int sync_transfer_equipment_info(int scene_id); // 角色切换场景时, 同步神兵洗练信息、熔炼信息到其他地图进程
	int read_transfer_equipment_info(Message *msg);	// 角色切换场景时,读取来自其他进程的同步信息

	/*实时查询在线玩家的装备时调用，打包玩家穿戴的装备信息*/
	int make_up_equipment_list(Proto50100156* &respond);
	int make_up_equipment_detail(Proto50100156* &respond, GamePackage* package);

	bool equip_strengthen_is_operate();		//身上是否有可强化的装备
	bool equip_red_uprising_is_operate();	//身上是否有可升阶的装备
	bool equip_good_refine_is_operate(int type);
    bool equip_orange_uprising_is_operate(int pack_type = GameEnum::INDEX_EQUIP);
	bool equip_tempered_is_operate();	//背包是否有可淬练的装备
	bool equip_polish_is_operate();		//身上是否有可洗练的装备
	bool equip_insert_jewel_is_operate(int item_id);	//身上是否有可镶嵌宝石的装备
	bool equip_jewel_combine_is_operate(int item_id);	//身上是否有可合成宝石

	void check_pa_event_equip_strengthen();	//强化
	void check_pa_event_equip_red_uprising(); //红装
	void check_pa_event_equip_good_refine(int item_id); //精炼
	void check_pa_event_jewel_all(int check = 0x03);
	void check_pa_event_equip_insert_jewel(int item_id);
	void check_pa_event_jewel_combine(int item_id);

	void refresh_equip_prop_to_map();
	void sync_equip_prop_to_map(int enter_type = 0);

	/*
	 * @para set_zero = true, means the fashion id would be set to sero -- player must be shown with equip id instead
	 *
	 * 玩家同时有装备和时装，展示的造型是 ( 时装 > 装备 )；
	 * 玩家造型有变化时，调用这个接口，发消息到地图线程进行区域广播
	 * */
	void refresh_player_equip_shape(int part, int item_id = -1, bool no_notify = false);

	void calc_equip_property();

private:
	static int fetch_jewel_prop_type(int item_id);
	static string fetch_jewel_prop_name(int prop_type);

protected:
	void calc_equip_property(IntMap &prop_map);

private:
	int __last_request_tick;

	SmeltInfo __smelt_info;
	BIntSet fashion_id_set;		//时装宝箱
};

#endif /* MAPEQUIPMENT_H_ */
