/*
 * SMBattleSystem.h
 *
 *  Created on: Mar 18, 2014
 *      Author: lijin
 */

#ifndef SMBATTLESYSTEM_H_
#define SMBATTLESYSTEM_H_

#include "PubStruct.h"

class SMBattleScene;

struct SMBattleSysInfo
{
	SMBattleSysInfo(void);
	void reset(void);

	int __activity_id;
	int __enter_level;
};

class SMBattleSystem
{
public:
	typedef std::vector<SMBattleScene*> SMBattleSceneSet;

	class CheckStartTimer : public GameTimer
	{
	public:
		CheckStartTimer(void);
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		void init(SMBattleSystem* parent);

	private:
	    SMBattleSystem* sm_system_;
	};

public:
	SMBattleSystem(void);
	~SMBattleSystem(void);

	void reset(void);
	void recycle_battle(SMBattleScene* scene);

	int client_scene();
	int real_scene();

	const Json::Value& scene_conf();
	SMBattleSysInfo& sm_battle_sys_info(void);

	void start_sm_battle(int real_scene);
	void stop_sm_battle();
	void test_sm_battle(int id, int last);

	int ahead_battle_event();
	int start_battle_event();
	int stop_battle_event();

	int sm_battle_left_time();
	int start_sm_battle_scene();
	int handle_sm_battle_timeout();
	int handle_sm_battle_i(int state);

	int is_activity_time();
	int validate_player_level(int role_level);
	int request_enter_battle(int sid, Proto30400051* request);

	SMBattlerInfo* find_battler(Int64 role_id);
	SMBattleScene* fetch_battle_scene(Int64 role_id);
	SMBattleScene* pop_battle_scene(int space_id);
	SMBattleScene* find_battle_scene(int space_id);

private:
	int real_scene_;

    ActivityTimeInfo time_info_;
	SMBattleSysInfo battle_info_;

	SMBattleSceneSet sm_scene_set_;
	CheckStartTimer sm_check_timer_;

	PoolPackage<SMBattleScene>* sm_scene_package_;
	PoolPackage<SMBattlerInfo, Int64>* player_map_;
};

typedef Singleton<SMBattleSystem> SMBattleSystemSingleton;
#define SM_BATTLE_SYSTEM  SMBattleSystemSingleton::instance()

#endif /* SMBATTLESYSTEM_H_ */
