/*
 * TrvlPeakScene.h
 *
 *  Created on: 2017年5月22日
 *      Author: lyw
 */

#ifndef TRVLPEAKSCENE_H_
#define TRVLPEAKSCENE_H_

#include "Scene.h"

class TravPeakTeam;

class TrvlPeakScene : public Scene
{
public:
	enum
	{
	    FIGHT_PREPARE   = 1,
	    FIGHT_BEING     = 2,
	    FIGHT_FINISH    = 3,

	    SIDE_LEFT 		= 1,
	    SIDE_RIGHT		= 2,

	    END
	};

	class PeakTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

		int state_;
		TrvlPeakScene* scene_;
	};

public:
	TrvlPeakScene();
	virtual ~TrvlPeakScene();
	virtual void reset();

	int init_peak_scene(int space_id, FourObj& obj);
	void notify_peak_fight();
	void notify_fight_result(TravPeakTeam *win_team, TravPeakTeam *loss_team);
	void check_and_handle_offline();

	void handle_prep_timeout();
	void handle_fighting_timeout();
	void handle_finish_timeout();

	TravPeakTeam *fetch_team(const int side);
	void set_team(TravPeakTeam *team, const int side);

	void insert_die_player(const Int64 role_id);
	bool is_has_die_player(const Int64 role_id);

	bool is_prepare_fight(void);
	bool is_fighting(void);
	bool is_finish_fight(void);

	void set_win_team(const Int64 team_id);
	Int64 calc_win_team_id(void);

	void finish_fight_earlier(void);

private:
	PeakTimer peak_timer_;

	TravPeakTeam *left_team_;
	TravPeakTeam *right_team_;

	int prep_time_;
	int left_time_;
	Int64 win_team_id_;

	LongSet die_rold_set_;
	LongSet offline_set_;

};

#endif /* TRVLPEAKSCENE_H_ */
