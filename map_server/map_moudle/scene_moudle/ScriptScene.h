/*
 * ScriptScene.h
 *
 * Created on: 2013-12-24 10:13
 *     Author: lyz
 */

#ifndef _SCRIPTSCENE_H_
#define _SCRIPTSCENE_H_

#include "Scene.h"
#include "AIStruct.h"
#include "ScriptStruct.h"

struct SceneAIRecord;
class ScriptAI;
class BaseScript;
class MapPlayerScript;

class ScriptScene : public Scene
{
public:
    class AIManagerTimer : public GameTimer
    {
    public:
        AIManagerTimer(void);
        virtual ~AIManagerTimer(void);

        void set_script_scene(ScriptScene *scene);
        ScriptScene *scene(void);

        virtual int type(void);
        virtual int handle_timeout(const Time_Value &tv);

    protected:
        ScriptScene *scene_;
    };

    typedef std::vector<SceneAIRecord *> SceneAIRecordVec;
    typedef HashMap<Int64, int, NULL_MUTEX> ScriptAIRecordMap;
    typedef HashMap<Int64, ScriptAI *, NULL_MUTEX> ScriptAIMap;

    typedef Heap<SceneAIRecord, AIRecordCmp> AIRecordQueue;
    typedef std::vector<AIRecordQueue> AIRecordWaveSet;

    typedef DoubleKeyMap<Int64, Int64, ScriptAI *, NULL_MUTEX> SortScriptAIMap;
public:
    ScriptScene(void);
    virtual ~ScriptScene(void);

    virtual void reset(void);
    virtual bool is_running(void);
    virtual void set_script_sort(const int script_sort);
    virtual int script_sort(void);
    
    virtual int init_scene(const int space_id, const int scene_id);

    // 启动副本场景
    virtual void run_scene(void);
    // 定时调用
    virtual int run_scene_record(const Time_Value &nowtime);
    // 挂起副本场景
    virtual void holdup_scene(void);

    virtual int fetch_all_around_player(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center, int radius, const int max_amount);

    virtual void recycle_all_monster(void);
    virtual int recycle_monster(ScriptAI *script_ai);

    virtual ScriptAI *pop_script_ai(const int scene_id, const int monster_sort);
    virtual int push_script_ai(ScriptAI *script_ai);

    virtual int update_generate_check_tick(SceneAIRecord *record, const int num);
    virtual int summon_monster(GameFighter *caller, const Json::Value &effect_json);

    virtual void set_protect_npc(const int sort, const Int64 ai_id);
    virtual int protect_npc_sort(void);
    virtual Int64 protect_npc_id(void);
    virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理

    ScriptSceneDetail &script_detail(void);
    int increase_monster_amount(int sort, int amount);	//增加AI数量
    int descrease_monster_amount(int sort, int amount);	//减少AI数量

    int call_puppet(const int puppet_sort, MapPlayerScript *player, MoverCoord &coord, const bool is_rand = true);

    int wave_ai_size(const int wave_id);
    int total_wave_ai_size(void);

    ScriptAIMap &script_ai_map(void);

    int force_kill_all_monster(void);

protected:
    virtual int init_scene_record(void);
    virtual int holdup_scene_record(void);
    int generage_scene_monster(SceneAIRecord *record, const Json::Value &layout_monster_json);

    virtual int register_monster(GameMover *mover);
    virtual int unregister_monster(GameMover *mover);

    int process_tick_record(const Json::Value &layout_json);
    int process_wave_record(const Json::Value &layout_json, BaseScript *script);

    void notify_add_buff(BaseScript *script);

private:
    bool s_is_inited_scene_;
    bool is_running_scene_;
    int script_sort_;
    int protect_npc_sort_;
    Int64 protect_npc_id_;

    Time_Value generate_check_tick_;

    SceneAIRecordVec record_vec_;
    ScriptAIMap script_ai_map_;

    int tick_record_times_;	// 怪物生成次数
    AIRecordQueue tick_record_queue_;
    AIRecordWaveSet wave_record_set_;

    AIManagerTimer ai_manager_timer_;
    int chase_index_;

    ScriptSceneDetail script_detail_;
};

#endif //_SCRIPTSCENE_H_
