#ifndef MONSTER_TOWER_SCRIPT_H
#define MONSTER_TOWER_SCRIPT_H

#include "BaseScript.h"


class MapPlayerScript;

class MonsterTowerScript : public BaseScript
{
public:
	MonsterTowerScript();
	virtual ~MonsterTowerScript();

protected:
	virtual void recycle_self_to_pool();
	virtual int broad_script_pass();
	virtual int make_up_special_script_finish_detail(MapPlayerScript *player, Message *msg);
	virtual int make_up_special_script_progress_detail(Message *msg);
	virtual int summon_ai_inherit_player_attr(ScriptAI *script_ai, const Json::Value &json);
	virtual int sync_kill_monster(ScriptAI *script_ai, const Int64 fighter_id);
private:
	int get_tower_floor();
	int get_task_difficulty();
};
#endif
