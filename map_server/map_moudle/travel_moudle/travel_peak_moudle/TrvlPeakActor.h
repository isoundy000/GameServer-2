/*
 * TrvlPeakActor.h
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#ifndef TRVLPEAKACTOR_H_
#define TRVLPEAKACTOR_H_

#include "MapPlayer.h"

class TrvlPeakActor : virtual public MapPlayer
{
public:
	TrvlPeakActor();
	virtual ~TrvlPeakActor();
	void reset();

	virtual int enter_scene(const int type = ENTER_SCENE_TRANSFER);
	virtual int exit_scene(const int type = EXIT_SCENE_TRANSFER);
	virtual int die_process(const int64_t fighter_id = 0);

	int trvl_peak_scene();
	int trvl_peak_prep_scene();

	int on_enter_prep_peak_scene(int type);
	int on_exit_prep_peak_scene(int type);
	int on_enter_trvl_peak_scene(int type);
	int on_exit_trvl_peak_scene(int type);

	int request_enter_trvl_peak_scene();
	int request_match_trvl_peak_quality();
	int request_unmatch_trvl_peak_quality();
	int request_trvl_peak_scene_info();
	int request_trvl_peak_rank_info(Message *msg);

	int handle_exit_trvl_peak_scene(void);

	int sync_offline_hook_to_travel_scene(Message *msg);
	static int process_redirect_to_trvl_scene(DBShopMode *shop_mode);

};

#endif /* TRVLPEAKACTOR_H_ */
