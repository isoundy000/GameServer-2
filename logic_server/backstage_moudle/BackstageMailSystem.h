/*
 * BackstageMailSystem.h
 *
 *  Created on: Jun 30, 2014
 *      Author: louis
 */

#ifndef BACKSTAGEMAILSYSTEM_H_
#define BACKSTAGEMAILSYSTEM_H_

#include "PubStruct.h"

class BackstageMailSystem
{
public:
	BackstageMailSystem();
	virtual ~BackstageMailSystem();

	int request_load_back_mail_request(void);
	int after_load_back_mail_request(Transaction* trans);

private:
	int handle_back_mail_request(mongo::BSONObj& res);
	int request_update_back_mail_request_flag(int request_id);
};

typedef Singleton<BackstageMailSystem> BackMailMonitor;
#define BACK_MAIL_SYS                  BackMailMonitor::instance()

#endif /* BACKSTAGEMAILSYSTEM_H_ */
