/*
 * ArrTimeManager.h
 *
 *  Created on: Nov 8, 2013
 *      Author: peizhibi
 */

#ifndef ARRTIMEMANAGER_H_
#define ARRTIMEMANAGER_H_

#include "GameHeader.h"

class ArrTimeManager
{
	typedef std::list<ArrTimeItem*> ArrItemList;
	typedef int (*HandleTimeOut)(ArrTimeItem* arr_item);

public:
	ArrTimeManager(HandleTimeOut handle_timeout);
	~ArrTimeManager();

	void handle_time_out(Int64 now_tick);

	void add_item(ArrTimeItem* arr_item);
	void remove_item(ArrTimeItem* arr_item);

private:
	ArrItemList arr_item_list_;
	HandleTimeOut handle_timeout_;
};

#endif /* ARRTIMEMANAGER_H_ */
