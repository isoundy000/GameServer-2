/*
 * LoudSpeaker.h
 *
 *  Created on: 2013-7-1
 *      Author: root
 */

#ifndef LOUDSPEAKER_H_
#define LOUDSPEAKER_H_

#include "BaseChannel.h"

class LoudSpeaker: public BaseChannel
{
public:
	LoudSpeaker();
	virtual ~LoudSpeaker();

	virtual int init(ChannelAgency *agency,int64_t channel_id=0);
	virtual int stop(void);
	virtual int channel_type(void);

	virtual int notify(void);

	virtual void reset(void);

	int load_record(void);
	int save_record(void);

	int record_size(void);
private:
	int wr_index_;
	int rd_index_;
	bool data_change_;
	Time_Value last_save_;
	RecordList record_list_bak_;

};

#endif /* LOUDSPEAKER_H_ */
