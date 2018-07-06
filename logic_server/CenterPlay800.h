/*
 * CenterPlay800.h
 *
 *  Created on: May 6, 2016
 *      Author: root
 */

#ifndef CENTERPLAY800_H_
#define CENTERPLAY800_H_

#include "SimpleHTTPClient.h"

class CenterPlay800 {
public:
	CenterPlay800();
	virtual ~CenterPlay800();
	int init();
	int income_log_get(Message *msg);
private:
	SimpleHTTPClient httpcli_;
	std::string host_;
	std::string url_;
	int port_;
	int time_out_;
	bool init_;
};


typedef Singleton<CenterPlay800> CenterPlay800Single;
#define CENTER_PLAY_800	CenterPlay800Single::instance()
#endif /* CENTERPLAY800_H_ */
