/*
 * NormalScene.h
 *
 * Created on: 2013-02-28 10:11
 *     Author: glendy
 */

#ifndef _NORMALSCENE_H_
#define _NORMALSCENE_H_

#include "Scene.h"

class NormalScene : public Scene
{
public:
	class BornAITimer : public GameTimer
	{
	public:
		int type(void);
		int handle_timeout(const Time_Value &tv);

		NormalScene* scene_;
	};

public:
	NormalScene();
	virtual void reset();
	virtual void start_scene();

	void dump_info(Int64 cur_time);
	void check_and_run_scene_monster();
    void run_center_scene_monster(int layout_index, int differ_count);
    void run_area_scene_monster(int layout_index, int differ_count);

    int handle_reborn_info(GameAI* game_ai);
    int handle_festival_boss(GameAI* game_ai);

	virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理
	virtual int handle_boss_die_action(GameAI* game_ai);

protected:
	virtual int run_scene_monster(int layout_index);

private:
	Int64 next_time_;
	IntPairMap monster_map_;
    BornAITimer born_timer_;
};

#endif //_NORMALSCENE_H_
