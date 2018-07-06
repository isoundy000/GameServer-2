/*
 * Scene.h
 *
 * Created on: 2013-01-23 10:46
 *     Author: glendy
 */

#ifndef _SCENE_H_
#define _SCENE_H_

#include "MapMapStruct.h"
#include "HashMap.h"
#include "GameTimer.h"
#include "GameMover.h"
#include "GameConfig.h"
#include "Sender.h"

class FloatAI;

class Scene
{
public:
    static const int chase_point[][2];
    static const int SELECT_POINT[12][2];
    static const int max_chase_point_size_;

public:
    typedef boost::unordered_set<BlockIndexType> BlockIndexSet;
    typedef HashMap<int64_t, GameMover *, NULL_MUTEX> MoverMap;
    typedef HashMap<int64_t, GameFighter *, NULL_MUTEX> FighterMap;

    class SceneTimer : public GameTimer
    {
    public:
        Scene *scene(Scene *scene = 0);
        virtual int type(void);

    protected:
        virtual int handle_timeout(const Time_Value &tv);

    protected:
        Scene *scene_;
    };

    class SceneControlTimer : public GameTimer
    {
    public:
        Scene *scene(Scene *scene = 0);
        virtual int type(void);

    protected:
        virtual int handle_timeout(const Time_Value &tv);

    protected:
        Scene *scene_;
    };

public:
    Scene(void);
    virtual ~Scene(void);

    MapMonitor *monitor(void);
    SceneDetail& scene_detail(void);
    Block_Buffer *pop_block(void);
    int push_block(Block_Buffer *pbuff);
#ifndef NO_BROAD_PORT
    Sender *client_sender(const int index = 0);
#endif

    virtual void reset(void);
    virtual int init_scene(const int space_id, const int scene_id);
    bool is_inited_scene(void);

    virtual void start_scene();
    virtual void run_scene();
    bool is_started_scene(void);

    int hash_id();
    int camp_split();
    int has_event_cut();
    int has_quit_team();
    int has_safe_area();
    int find_status_value(Int64 role, int id, int type = 1);

    int scene_id(void);
    int space_id(void);
    int mover_amount(void);
    int player_amount(void);
    int room_amount(void);

    virtual int handle_timeout(const Time_Value &nowtime);
    virtual int process_broad(const Time_Value &nowtime);

    virtual int register_mover(GameMover *mover, const int64_t block_index = -1);
    virtual int unregister_mover(GameMover *mover, const int64_t block_index = -1, const bool is_exit_scene = false);

    int near_block_x_from_pos_x(const int pos_x);
    int near_block_y_from_pos_y(const int pos_y);
    Int64 near_block_index_by_block_coord(const int block_x, const int block_y);
    int register_near_mover(GameMover *mover);
    int unregister_near_mover(GameMover *mover);
    int process_broad_near_player(const Time_Value &nowtime);

    virtual int start_timer(void);
    virtual int stop_timer(void);
    virtual int is_fighting_time();
    virtual int is_validate_attack_player(GameFighter* attacker, GameFighter* target);
    virtual int is_validate_attack_monster(GameFighter* attacker, GameFighter* target);

    virtual int refresh_mover_location(GameMover *mover, const MoverCoord &target, const bool is_send_move = true);
    virtual int refresh_mover_location_for_relive(GameMover *mover, const MoverCoord &target);
    virtual int refresh_mover_location_for_watch(GameMover *mover, const MoverCoord &target);
    virtual int notify_appear(GameMover *mover);
    virtual int notify_disappear(GameMover *mover);
    virtual int notify_area_info(GameMover *mover, Block_Buffer &buff, const int room_scene_index = -1);
    virtual int notify_fullscene(Block_Buffer &buff, const int room_scene_index = -1);
    int notify_fullscene(Message *msg, const int room_scene_index = -1);

    virtual int fetch_around_appear_info(GameMover *mover, Block_Buffer &buff);
    virtual int fetch_around_disappear_info(GameMover *mover, Block_Buffer &buff);

    virtual int enter_scene(GameMover *mover, int type);
    virtual int exit_scene(GameMover *mover, int type);

    void register_fighter(GameFighter* fighter);
    void unregister_fighter(GameFighter* fighter);

    virtual int validate_ai_pickup(GameAI* game_ai, MapPlayer* player);
    virtual int modify_ai_hurt_value(GameAI* game_ai, int src_value, Int64 attackor_id);

    virtual int handle_ai_alive_recycle(GameAI* game_ai);//回收AI
    virtual int handle_ai_die(GameAI* game_ai, Int64 benefited_attackor);//AI死亡处理
    virtual int handle_boss_die_action(GameAI* game_ai);

    virtual int makeup_ai_appear_info(GameAI* game_ai, Proto80400111* appear_info);//场景额外AI信息，如剩余倒计时
    virtual int makeup_role_appear_info(MapPlayer* player, Proto80400102* appear_info);//

    virtual IntPair fetch_addition_exp();
    virtual int fetch_relive_protected_time(Int64 role);
    virtual int fetch_enter_buff(IntVec& buff_vec, Int64 role);

    virtual int handle_init_ai(GameAI* game_ai);
    virtual int is_ai_area_recycle(GameAI* game_ai);
    /*
     * 以下只能一个地方处理
     * */
    virtual int handle_fighter_hurt(GameFighter* fighter, Int64 benefited_attackor, int hurt_value);
    virtual int handle_player_hurt(MapPlayer* player, Int64 benefited_attackor, int hurt_value);
    virtual int handle_ai_hurt(GameAI* game_ai, Int64 benefited_attackor, int hurt_value);//AI伤害处理

    int find_fighter(const int64_t fighter_id, GameFighter *&fighter);
    GameFighter* find_fighter(Int64 fighter_id);

    int fetch_player_set(LongVec& player_set);
    int transfer_player_out(MapPlayerEx* player, int exit_type = GameEnum::EXIT_TYPE_SAVE);

    GameAI* find_ai(Int64 ai_id);
    GameMover* find_mover(Int64 mover_id);

    MapPlayerEx* find_player(Int64 role_id);
    MapPlayerEx* fetch_benefited_player(Int64 attackor);

    void recycle_one_mover(Int64 ai_id);
    virtual void recycle_all_monster(void);

    virtual int notify_shield_info(BasicStatus* status);

    void notify_all_player_msg(int recogn, Message* msg = NULL);
    void notify_all_player_msg(const LongVec& player_set, int recogn,
    		Message* msg = NULL);
    void notiy_player_msg(Int64 role_id, int recogn, Message* msg = NULL);

    void restore_all_player_blood_full();
    void notify_all_player_exit(int exit_type = GameEnum::EXIT_TYPE_SAVE,
    		int player_type = GameEnum::PLAYER_TYPE_BOTH);

    virtual FighterMap &fighter_map(void);
    virtual MoverMap &player_map(void);
    virtual MoverMap &mover_map(void);
    virtual MoverMap &monster_map(void);

    const Json::Value& conf();
    const Json::Value& set_conf();
    const Json::Value& layout_sort(int monster_sort);
    const Json::Value& layout_index(int layout_index);

    virtual MoverCoord rand_coord(const MoverCoord& center, int radius, GameFighter *fighter = NULL);
    MoverCoord rand_coord_by_pixel_radius(const MoverCoord& center, int pixel_radius);
    MoverCoord rand_coord_by_pixel_ring(const MoverCoord& center, int out_pixel_radius, int in_pixel_radius);
    MoverCoord rand_dynamic_coord(const MoverCoord& center, int radius);
    MoverCoord rand_toward(const MoverCoord& center, int offset);
    MoverCoord rand_dynamic_toward(const MoverCoord& center, int offset);
    MoverCoord rand_ai_move_coord(GameAI* game_ai);
    MoverCoord fetch_random_coord(void);
    MoverCoord fetch_beast_coord(const MoverCoord& location);

    virtual int fetch_all_around_player(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center,
    		int radius, const int max_amount = 20);
    virtual int fetch_all_around_fighter(GameMover *mover, Scene::MoverMap& fighter_map, const MoverCoord& center,
    		int radius, const int max_amount = 20);
    virtual int fetch_all_around_player_mapping(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center,
    		int radius, const int mapping_factor=2, const int max_amount = 20);
    virtual int fetch_all_around_fighter_mapping(GameMover *mover, Scene::MoverMap& fighter_map, const MoverCoord& center,
    		int radius, const int mapping_factor=2, const int max_amount = 20);
    virtual int fetch_all_around_player_in_ring(GameMover *mover, Scene::MoverMap& player_map, const MoverCoord& center,
    		int inner_radius, int outer_radius, const int max_amount = 20);

    virtual int fetch_all_sector_fighter(GameMover *mover, Scene::MoverMap &fighter_map, 
            const MoverCoord& center, const MoverCoord &skill_coord, 
            const int radius, const double angle = 45, const int max_amount = 20);
    virtual int fetch_all_rect_fighter(GameMover *mover, Scene::MoverMap &fighter_map, const MoverCoord& center,
    		double angle, int width, int height, const SubObj& max_amount = SubObj(20));
    virtual int fetch_all_around_monster(GameMover *mover, Scene::MoverMap &monster_map, const MoverCoord &center,
    		int radius, const int max_amount = 20);
    int fetch_all_around_boss(GameMover *mover, Scene::MoverMap &boss_map, const MoverCoord &center,
    		int radius, const int max_amount = 20);

    virtual BLongSet *ai_sort_set(const int sort);
    virtual BLongSet *ai_group_set(const int group_id);
    virtual int summon_monster(GameFighter *caller, const Json::Value &effect_json);
    virtual int start_special_layout(const int layout_index);

    virtual void set_protect_npc(const int sort, const Int64 ai_id);
    virtual int protect_npc_sort(void);
    virtual Int64 protect_npc_id(void);

    virtual SceneDetail::MptCoordList &dynamic_mpt(GameMover *mover);
    int calc_mpt_index_by_coord(const MoverCoord &coord);
    int calc_grid_coord_by_mpt_index(int &grid_x, int &grid_y, const int mpt_index);
    int calc_dynamic_mpt_index_by_coord(const DynamicMoverCoord &coord);
    virtual bool is_movable_dynamic_coord(const DynamicMoverCoord &coord);
    virtual int increase_mpt_coord(GameMover *mover, const MoverCoord &coord);
    virtual int reduce_mpt_coord(GameMover *mover, const MoverCoord &coord);

    virtual int cal_block_index_by_grid_coord(const int grid_x, const int grid_y, BlockIndexType &block_index);

    int is_in_safe_area(GameFighter* fighter);
    int validate_in_safe_area(GameFighter *defender);

    bool is_ai_sort_alive(const int ai_sort);
    bool is_near_block(const MoverCoord &coord, const BlockIndexType block_index);

    FloatAI *find_float_ai(const Int64 role_id);
    int rebind_float_ai(const Int64 role_id, FloatAI *float_ai);
    int unbind_float_ai(const Int64 role_id);

    int bind_team_player(const int team_id, MapPlayer *player);
    int unbind_team_player(const int team_id, MapPlayer *player);
    int process_broad_teamer(void);
    int push_team_player_data(const int team_id, Block_Buffer &buff);
    int open_mini_map_pannel(const int team_id, const Int64 role_id);
    int close_mini_map_pannel(const int team_id, const Int64 role_id);
    int make_up_all_teamer_appear_info(const int team_id, Block_Buffer &buff);

public:
    static bool is_validate_around_mover(GameMover* mover,
    		const MoverCoord& center, int radius, const int mapping_factor=1);
    static bool is_validate_around_fighter(GameFighter* fighter,
    		const MoverCoord& center, int radius);
    static bool is_validate_rect_mover(GameMover *mover, const MoverCoord &attack_coord,
            const MoverCoord &center, const int width, const int height);
    static bool is_validate_rect_mover(GameMover *mover, const MoverCoord &pointA,
            const MoverCoord &pointB, const MoverCoord &pointC, const MoverCoord &pointD);

    static bool is_validate_sector_mover(GameMover *mover,
    		const MoverCoord &center, const MoverCoord &toward_point, const int range, const double angle);

public:
    //有体积
    static bool is_validate_around_mover(GameMover* target, GameMover* attack, int radius);

protected:
    virtual int init_scene_monster(int layout_index);
    virtual int run_scene_monster(int layout_index);

    virtual int register_monster(GameMover *mover);
    virtual int unregister_monster(GameMover *mover);

    virtual int flush_broad_interval(const Time_Value &nowtime, const size_t mover_amount);

    virtual int send_around_block(GameMover *mover, const BlockIndexType block_x, const BlockIndexType block_y, Block_Buffer *buff);
    virtual int send_diff_block(GameMover *mover, const MoverCoord &target,
            const BlockIndexType org_block_x, const BlockIndexType org_block_y, 
            const BlockIndexType target_block_x, const BlockIndexType target_block_y,
            Block_Buffer *disappear_buff, Block_Buffer *move_buff, Block_Buffer *appear_buff);
    virtual int send_diff_block_for_relive(GameMover *mover, const MoverCoord &target,
            const BlockIndexType org_block_x, const BlockIndexType org_block_y,
            const BlockIndexType target_block_x, const BlockIndexType target_block_y,
            Block_Buffer *disappear_buff, Block_Buffer *appear_buff);

    int cal_block_coord_by_grid_coord(const int grid_x, const int grid_y, BlockIndexType &block_x, BlockIndexType &block_y);
    int cal_block_coord_by_block_index(const BlockIndexType block_index, BlockIndexType &block_x, BlockIndexType &block_y);
    virtual int cal_block_index_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, BlockIndexType &block_index);
    int cal_grid_range_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, int &min_grid_x, int &max_grid_x, int &min_grid_y, int &max_grid_y);

    virtual int cal_around_block_by_block_index(const BlockIndexType block_index, BlockIndexSet &move_set);
    virtual int cal_around_block_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, BlockIndexSet &move_set);
    virtual int cal_diff_block_by_block_index(const BlockIndexType org_block_index, const BlockIndexType target_block_index,
                BlockIndexSet &disappear_set, BlockIndexSet &move_set, BlockIndexSet &appear_set);
    virtual int cal_diff_block_by_block_coord(const BlockIndexType org_block_x, const BlockIndexType org_block_y,
    		const BlockIndexType target_block_x, const BlockIndexType target_block_y,
    		BlockIndexSet &disappear_set, BlockIndexSet &move_set, BlockIndexSet &appear_set);

protected:
    bool start_scene_;
    bool start_monster_;
    bool is_inited_scene_;

    int hash_id_;
    int camp_split_;
    int event_cut_;	//连斩标识
    int quit_team_;	//退出队伍标识
    int	safe_area_;	//安全区域标识

    MapMonitor *monitor_;
    SceneDetail scene_detail_;
    SceneTimer scene_timer_;
    SceneControlTimer scene_trl_timer_;

    MoverMap mover_map_;
    MoverMap player_map_;
    MoverMap monster_map_;
    MoverMap float_ai_map_;    // key: role_id, value: FloatAI
    FighterMap fighter_map_;
};

inline int Scene::cal_block_coord_by_grid_coord(const int grid_x, const int grid_y, BlockIndexType &block_x, BlockIndexType &block_y)
{
	//如果这里coredump，基本是场景已回收到对象池
    block_x = grid_x / this->scene_detail_.__block_width;
    block_y = grid_y / this->scene_detail_.__block_height;
    return 0;
}

inline int Scene::cal_block_index_by_grid_coord(const int grid_x, const int grid_y, BlockIndexType &block_index)
{
    BlockIndexType block_x = 0, block_y = 0;
    this->cal_block_coord_by_grid_coord(grid_x, grid_y, block_x, block_y);

    return this->cal_block_index_by_block_coord(block_x, block_y, block_index);
}

inline int Scene::cal_block_coord_by_block_index(const BlockIndexType block_index, BlockIndexType &block_x, BlockIndexType &block_y)
{
    block_x = block_index % this->scene_detail_.__block_x_amount;
    block_y = block_index / this->scene_detail_.__block_x_amount;
    return 0;
}

inline int Scene::cal_grid_range_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, int &min_grid_x, int &max_grid_x, int &min_grid_y, int &max_grid_y)
{
    min_grid_x = block_x * this->scene_detail_.__block_width;
    max_grid_x = min_grid_x + this->scene_detail_.__block_width;
    min_grid_y = block_y * this->scene_detail_.__block_height;
    max_grid_y = min_grid_y + this->scene_detail_.__block_height;

    return 0;
}

inline bool Scene::is_near_block(const MoverCoord &coord, const BlockIndexType block_index)
{
    BlockIndexType block_x, block_y;
    this->cal_block_coord_by_block_index(block_index, block_x, block_y);
    int min_grid_x, max_grid_x, min_grid_y, max_grid_y;
    this->cal_grid_range_by_block_coord(block_x, block_y, min_grid_x, max_grid_x, min_grid_y, max_grid_y);

    if ((min_grid_x - 3) <= coord.pos_x() && coord.pos_x() <= (max_grid_x + 3) && //
            (min_grid_y - 3) <= coord.pos_y() && coord.pos_y() <= (max_grid_y + 3))
        return true;
    return false;
}

inline int Scene::cal_around_block_by_block_index(const BlockIndexType center_index, BlockIndexSet &block_set)
{
    BlockIndexType block_x = 0, block_y = 0;
    this->cal_block_coord_by_block_index(center_index, block_x, block_y);

    return this->cal_around_block_by_block_coord(block_x, block_y, block_set);
}

inline int Scene::cal_around_block_by_block_coord(const BlockIndexType block_x, const BlockIndexType block_y, BlockIndexSet &block_set)
{
    int pos[9][2];
    pos[0][0] = block_x;            pos[0][1] = block_y;        /// 中心
    pos[1][0] = block_x + 1;        pos[1][1] = block_y;        /// 右
    pos[2][0] = block_x + 1;        pos[2][1] = block_y + 1;    /// 右下
    pos[3][0] = block_x;            pos[3][1] = block_y + 1;    /// 下
    pos[4][0] = block_x - 1,        pos[4][1] = block_y + 1;    /// 左下
    pos[5][0] = block_x - 1,        pos[5][1] = block_y;        /// 左
    pos[6][0] = block_x - 1,        pos[6][1] = block_y - 1;    /// 左上
    pos[7][0] = block_x,            pos[7][1] = block_y - 1;    /// 上
    pos[8][0] = block_x + 1,        pos[8][1] = block_y - 1;    /// 右上

    int block_x_amount = this->scene_detail_.__block_x_amount,
        block_y_amount = this->scene_detail_.__block_y_amount;
    for (int i = 0; i < 9; ++i)
    {
        if (pos[i][0] < 0 || pos[i][0] >= block_x_amount || pos[i][1] < 0 || pos[i][1] >= block_y_amount)
            continue;
		block_set.insert(pos[i][1] * block_x_amount + pos[i][0]);
	}
	return 0;
}

inline int Scene::cal_diff_block_by_block_index(const BlockIndexType org_block_index, const BlockIndexType target_block_index,
        BlockIndexSet &disappear_set, BlockIndexSet &move_set, BlockIndexSet &appear_set)
{
    BlockIndexSet org_block_set, target_block_set;
    BlockIndexType org_block_x, org_block_y, target_block_x, target_block_y;
    this->cal_block_coord_by_block_index(org_block_index, org_block_x, org_block_y);
    this->cal_block_coord_by_block_index(target_block_index, target_block_x, target_block_y);

    this->cal_diff_block_by_block_coord(org_block_x, org_block_y, target_block_x, target_block_y, disappear_set, move_set, appear_set);
    return 0;
}

inline int Scene::cal_diff_block_by_block_coord(const BlockIndexType org_block_x, const BlockIndexType org_block_y,
		const BlockIndexType target_block_x, const BlockIndexType target_block_y,
		BlockIndexSet &disappear_set, BlockIndexSet &move_set, BlockIndexSet &appear_set)
{
    int org_pos[9][2];
    org_pos[0][0] = org_block_x;            org_pos[0][1] = org_block_y;        /// 中心
    org_pos[1][0] = org_block_x + 1;        org_pos[1][1] = org_block_y;        /// 右
    org_pos[2][0] = org_block_x + 1;        org_pos[2][1] = org_block_y + 1;    /// 右下
    org_pos[3][0] = org_block_x;            org_pos[3][1] = org_block_y + 1;    /// 下
    org_pos[4][0] = org_block_x - 1,        org_pos[4][1] = org_block_y + 1;    /// 左下
    org_pos[5][0] = org_block_x - 1,        org_pos[5][1] = org_block_y;        /// 左
    org_pos[6][0] = org_block_x - 1,        org_pos[6][1] = org_block_y - 1;    /// 左上
    org_pos[7][0] = org_block_x,            org_pos[7][1] = org_block_y - 1;    /// 上
    org_pos[8][0] = org_block_x + 1,        org_pos[8][1] = org_block_y - 1;    /// 右上

    int target_pos[9][2];
    target_pos[0][0] = target_block_x;            target_pos[0][1] = target_block_y;        /// 中心
    target_pos[1][0] = target_block_x + 1;        target_pos[1][1] = target_block_y;        /// 右
    target_pos[2][0] = target_block_x + 1;        target_pos[2][1] = target_block_y + 1;    /// 右下
    target_pos[3][0] = target_block_x;            target_pos[3][1] = target_block_y + 1;    /// 下
    target_pos[4][0] = target_block_x - 1,        target_pos[4][1] = target_block_y + 1;    /// 左下
    target_pos[5][0] = target_block_x - 1,        target_pos[5][1] = target_block_y;        /// 左
    target_pos[6][0] = target_block_x - 1,        target_pos[6][1] = target_block_y - 1;    /// 左上
    target_pos[7][0] = target_block_x,            target_pos[7][1] = target_block_y - 1;    /// 上
    target_pos[8][0] = target_block_x + 1,        target_pos[8][1] = target_block_y - 1;    /// 右上

	BlockIndexType x_amount = this->scene_detail_.__block_x_amount,
				   y_amount = this->scene_detail_.__block_y_amount;
	for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(org_pos[i][0] - target_block_x)) <= 1 && std::abs(long(org_pos[i][1] - target_block_y)) <= 1)
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;
            move_set.insert(org_pos[i][1] * x_amount + org_pos[i][0]);
        }
        else
        {
            if (org_pos[i][0] < 0 || org_pos[i][0] >= x_amount || org_pos[i][1] < 0 || org_pos[i][1] >= y_amount)
                continue;
            disappear_set.insert(org_pos[i][1] * x_amount + org_pos[i][0]);
        }
    }

    for (int i = 0; i < 9; ++i)
    {
        if (std::abs(long(target_pos[i][0] - org_block_x)) <= 1 && std::abs(long(target_pos[i][1] - org_block_y)) <= 1)
            continue;

        if (target_pos[i][0] < 0 || target_pos[i][0] >= x_amount || target_pos[i][1] < 0 || target_pos[i][1] >= y_amount)
            continue;
        appear_set.insert(target_pos[i][1] * x_amount + target_pos[i][0]);
    }
    return 0;
}

#endif //_SCENE_H_
