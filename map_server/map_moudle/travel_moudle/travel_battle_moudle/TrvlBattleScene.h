/*
 * File Name: TrvlBattleScene.h
 * 
 * Created on: 2017-04-19 16:41:16
 * Author: glendy
 * 
 * Last Modified: 2017-05-06 15:41:08
 * Description: 
 */

#ifndef _TRVLBATTLESCENE_H_
#define _TRVLBATTLESCENE_H_

#include "Scene.h"

class BattleGroundActor;
class MapPlayerEx;

class TrvlBattleScene : public Scene
{
public:
    friend class MapPlayerEx;

    struct AIGenRecord
    {
        int __ai_sort;
        int __layout_index;
        MoverCoord __birth_coord;
        Time_Value __gen_tick;

        void reset(void);
    };

    typedef std::map<Int64, GameAI *> AIMap;
    typedef std::list<AIGenRecord> AIGenRecordList;

public:
    TrvlBattleScene(void);
    virtual ~TrvlBattleScene(void);

    virtual void reset(void);

    void set_scene_auto_recycle_tick(const Time_Value &delay_sec);

    void init_trvl_battle_scene(const int space_id);
    void init_generate_monster(void);
    void start_trvl_battle_scene(void);
    void stop_trvl_battle_scene(void);
    void recycle_scene(void);

    virtual int handle_timeout(const Time_Value &nowtime);
 
    virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理
    int handle_trvl_battle_die_player(const Int64 die_role_id, const Int64 killer_role_id);

    void handle_back_floor_when_die(BattleGroundActor *player);

    void generate_treasure_sort_immediate(void);
protected:
    int check_and_generate_monster(const Time_Value &nowtime);
    void insert_ai_reborn_record(GameAI *game_ai);

    void handle_enter_new_floor_when_kill(MapPlayerEx *player);
    void handle_player_change_floor(BattleGroundActor *player, const int cur_floor, const int change_floor, const bool is_relive = false);

    void check_and_insert_treasure_buff(MapPlayerEx *killer_player, GameAI *game_ai);
    void check_and_transfer_treasure_buff_to_killer(MapPlayerEx *killer_player, MapPlayerEx *die_player);

    virtual int makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info);

protected:
    AIMap *scene_ai_map_;
    AIGenRecordList *ai_gen_rec_list_;

    bool is_start_recycle_;
    Time_Value recycle_tick_;
};

#endif //TRVLBATTLESCENE_H_
