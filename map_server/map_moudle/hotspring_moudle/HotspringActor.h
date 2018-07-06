/*
 * HotspringActor.h
 *
 *  Created on: 2016年9月19日
 *      Author: lzy
 */

#ifndef HOTSPRINGACTOR_H_
#define HOTSPRINGACTOR_H_

#include "MapPlayer.h"

class HotspringActor : virtual public MapPlayer
{
public:
	class ActivityStartTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);
		ActivityStartTimer(void);
		~ActivityStartTimer(void);
		void set_parent(HotspringActor *parent);
		HotspringActor *parent_;
	};

	HotspringDetail &get_hotspring_detail();
	HotspringActor(void);
	virtual ~HotspringActor(void);
	void reset(void);
	bool is_on_hotspring_scene(void);
    virtual int sign_out(const bool is_save_player = true);
    int request_enter_hotspring_activity();

	int get_hotspring_exp(void);
	int hotspring_activity_start(void);
	int request_Hotspring_info(void);
	int request_double_major(Message* msg);
	int request_player_guess(Message* msg);
	int request_hotspring_near_player(Message* msg);
	int notify_player_info(void);
	int notify_player_get_hotspring_award(void);

private:
	HotspringDetail hotspring_detail_;
	ActivityStartTimer start_timer_;
};


#endif /* HOTSPRINGACTOR_H_ */
