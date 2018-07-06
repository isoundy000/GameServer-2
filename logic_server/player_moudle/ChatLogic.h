/*
 * ChatLogic.h
 *
 *  Created on: 2013-7-8
 *      Author: root
 */

#ifndef CHATLOGIC_H_
#define CHATLOGIC_H_
#include <stdlib.h>
#include "LogicStruct.h"

class ChatLogic
{
public:
	enum OPRA_STATUS
	{
		JOIN_STATUS                  =    1,
		QUIT_STATUS                  =    2,

		OPRA_STATUS_END
	};

public:
	ChatLogic();
	virtual ~ChatLogic();

    int chat_establish_team(void);
    int chat_dismiss_team(void);
    int chat_join_team(int team_id);
    int chat_leave_team(void);

    int chat_establish_league(Int64 league_id);
    int chat_join_league(Int64 league_id, string league_name);
    int chat_dismiss_league(Int64 league_id);
    int chat_leave_league(Int64 league_id);

    int chat_add_black_list(LongVec& role_id_set);
    int chat_remove_black_list(LongVec& role_id_set);
    int chat_add_friend_list(LongVec& role_id_set);
    int chat_remove_friend_list(LongVec& role_id_set);
};

#endif /* CHATLOGIC_H_ */
