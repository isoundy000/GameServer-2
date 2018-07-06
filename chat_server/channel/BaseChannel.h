/*
 * BaseChannel.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef BASECHANNEL_H_
#define BASECHANNEL_H_

#include "ChatStruct.h"
#include "GameTimer.h"

class ChannelAgency;
class Block_Buffer;

class BaseChannel:public GameTimer
{
public:
	typedef std::list<ChatRecord*> RecordList;

	BaseChannel();
	virtual ~BaseChannel();

	virtual int type(void);
	virtual int handle_timeout(const Time_Value &tv);
	virtual int schedule_timer(const Time_Value &interval);
	virtual int cancel_timer(void);
	virtual int channel_type(void)=0;

	virtual int init(ChannelAgency *agency,int64_t channel_id=0);
	virtual int stop(void);

	virtual int send_record(ChatRecord *record);

	/*定时发送*/
	virtual int notify(void);

	int push_record(ChatRecord *record);
	ChatRecord* pop_record(void);

	int bind_sid(int sid);
	int unbind_sid(int sid);
	int find_sid(int sid);
    int channel_client_amount(void);

	void set_channel_id(int64_t id);
	const int64_t channel_id(void);

	void set_agency(ChannelAgency *agency);
	ChannelAgency *agency(void);

	virtual void reset(void);

	virtual int push_voice(ChatRecord* record,bool history=false);
	virtual ChatRecord* get_voice(int64_t id);

protected:
	RecordList record_list_;
	RecordList voice_list_;

	BIntSet player_sid_set_;
	Time_Value interval_tick_;

protected:
	ChannelAgency *agency_;
	int64_t channel_id_;

};

#endif /* BASECHANNEL_H_ */
