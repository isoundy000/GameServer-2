/*
 * LeagueChannel.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef LEAGUECHANNEL_H_
#define LEAGUECHANNEL_H_

#include "BaseChannel.h"
#include <list>

class ChatPlayer;
class Proto50200004;

class LeagueChannel: public BaseChannel
{
public:
	friend class MMOChatLeague;

	LeagueChannel();
	virtual ~LeagueChannel();

	virtual int init(ChannelAgency *agency,int64_t channel_id);
	int start(void);
	virtual int stop(void);
	virtual int channel_type(void);
	virtual int notify(void);
	int get_history(std::list<ChatRecord*>& list,int time_offset);
	virtual void reset(void);
	virtual int notify_offline(ChatPlayer *player);

	int push_history(ChatRecord *record);
private:
	int load_history(void);
	int save_history(void);
	RecordList history_;
	RecordList history_for_save_;//用于数据保存
	bool data_change_;
	Time_Value last_save_;
};

#endif /* LEAGUECHANNEL_H_ */
