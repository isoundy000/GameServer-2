/*
 * MonsterAttackSystem.h
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#ifndef MONSTERATTACKSYSTEM_H_
#define MONSTERATTACKSYSTEM_H_

#include "PubStruct.h"
#include "MapMapStruct.h"

class MonsterAttackScene;

struct MonsterAttackSysInfo
{
	MonsterAttackSysInfo(void);
	void reset(void);

	int __activity_id;
	int __enter_level;
};

class MonsterAttackSystem
{
public:
	class CheckStartTimer : public GameTimer
	{
	public:
		CheckStartTimer(void);
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void init(MonsterAttackSystem* parent);

	private:
		MonsterAttackSystem* mattack_system_;
	};

	typedef std::map<int, MAttackLabelRecord>LabelRecordMap;

public:
	MonsterAttackSystem();
	virtual ~MonsterAttackSystem();

	const Json::Value& scene_conf();
	int real_scene();

	void reset(void);
	void start_monster_attack(int real_scene);
	void stop_monster_attack();

	void recycle_mattack(MonsterAttackScene* scene);

public:
	int handle_mattack_timeout();
	int handle_mattack_i(int state);

	int ahead_mattack_event();
	int start_mattack_event();
	int stop_mattack_event();
	int start_mattack_scene();

	void test_mattack(int id, int set_time);

	int mattack_left_time();
	int is_activity_time();
	int validate_player_level(int role_level);
	int request_enter_mattack(int sid, Proto30400051* request);
	int update_label_record(Message* msg);

	MAttackInfo* find_mattack(Int64 role_id);
	MonsterAttackScene* find_mattack_scene(int space_id);
	MAttackLabelRecord* find_label_record(int label_id);

private:
	int real_scene_;
	MonsterAttackScene* mattack_scene_;

	MonsterAttackSysInfo mattack_detail_;
	ActivityTimeInfo time_info_;
	CheckStartTimer mattack_check_timer_;
	LabelRecordMap label_record_map_;

	PoolPackage<MonsterAttackScene>* mattack_scene_package_;
	PoolPackage<MAttackInfo, Int64>* player_map_;
};

typedef Singleton<MonsterAttackSystem> MonsterAttackSystemSingleton;
#define MONSTER_ATTACK_SYSTEM  MonsterAttackSystemSingleton::instance()

#endif /* MONSTERATTACKSYSTEM_H_ */
