/*
 * MapWedding.h
 *
 * Created on: 2015-06-10 16:41
 *     Author: lyz
 */

#ifndef _MAPWEDDING_H_
#define _MAPWEDDING_H_

#include <vector>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
}
using ::google::protobuf::Message;
class MapPlayer;
class MoverCoord;

class MapWedding
{
public:
	typedef long long int Int64;
public:
    virtual ~MapWedding(void);

    virtual MapPlayer *wedding_player(void);

    void reset_wedding(void);

    void set_wedding_id(const Int64 id);
    Int64 wedding_id(void);
    void set_partner_id(const Int64 id);
    Int64 partner_id(void);
    void set_float_ai_id(const Int64 id);
    Int64 float_ai_id(void);

    int process_get_wedding_role_coord(Message *msg);
    int enter_float_cruise_state(const bool is_notify = false);
    int exit_float_cruise_state(void);
    int correct_player_coord_when_float(void);
    bool is_float_cruise(void);
    int follow_float_move_action(std::vector<MoverCoord> &step_list);

    int process_sync_wedding_info(Message *msg);

    int check_wedding_giftbox_times(const int wedding_type);

    int notify_send_flower_effect(Message *msg);

protected:
    int sync_update_intimacy_by_kill(const Int64 fighter_id);

protected:
    Int64 wedding_id_;     // 姻缘ID
    Int64 partner_id_;     // 伴侣ID
    Int64 float_ai_id_; // 花车巡游状态, 大于0为在巡游中
};

#endif //_MAPWEDDING_H_
