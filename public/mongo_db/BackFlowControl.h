/*
 * BackFlowControl.h
 *
 *  Created on: May 22, 2014
 *      Author: louis
 */

#ifndef BACKFLOWCONTROL_H_
#define BACKFLOWCONTROL_H_

#include "MongoTable.h"

class BackFlowControl : public MongoTable
{
public:
	BackFlowControl();
	virtual ~BackFlowControl();

	int request_load_flow_detail();
	static int load_flow_control_detail(FlowControl* flow_control);

private:
	int update_flow_detail_load_flag(FlowControl::FlowControlDetail* detail, std::set<int> &server_set, const int current_index);

protected:
	virtual void ensure_all_index(void);
};

#endif /* BACKFLOWCONTROL_H_ */
