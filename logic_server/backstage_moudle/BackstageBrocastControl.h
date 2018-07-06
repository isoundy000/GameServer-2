/*
 * BackstageBrocastControl.h
 *
 *  Created on: Jun 7, 2014
 *      Author: louis
 */

#ifndef BACKSTAGEBROCASTCONTROL_H_
#define BACKSTAGEBROCASTCONTROL_H_

#include "GameHeader.h"

class BackstageBrocastRecord;

class BackstageBrocastControl
{
public:
	typedef std::map<int, BackstageBrocastRecord*> BrocastRecordMap;

public:
	BackstageBrocastControl();
	virtual ~BackstageBrocastControl();

	int init(void);
	int start(void);
	int stop(void);
	int fina(void);

	int request_load_data_from_db(void);
	int after_load_data_from_db(Transaction* trans);

	int reinit_backstage_brocast_timer(void);
	int backstage_brocast_handle_timeout(const Int64 now);
	int update_record(BackstageBrocastRecord* record);

	BackstageBrocastRecord* pop_record(void);
	void push_record(BackstageBrocastRecord& record);
	void push_record_by_id(const int record_id);

	BIntSet& modify_set();
	BIntSet& remove_set();
	BrocastRecordMap& brocast_record_map();

private:
	BIntSet modify_set_;
	BIntSet remove_set_;

	BrocastRecordMap brocast_record_map_;
};

typedef Singleton<BackstageBrocastControl> BBCMonitor;
#define BBC_INSTANCE BBCMonitor::instance()

#endif /* BACKSTAGEBROCASTCONTROL_H_ */
