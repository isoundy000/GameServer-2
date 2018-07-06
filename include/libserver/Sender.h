// -*- C++ -*-
/*
 * Sender.h
 *
 *  Created on: Apr 25, 2012
 *      Author: ChenLong
 */

#ifndef SENDER_H_
#define SENDER_H_

#include "Thread.h"
#include "Epoll_Watcher.h"
#include "Event_Handler.h"
#include "List.h"
#include "Block_List.h"
#include "boost/unordered_map.hpp"
#include "boost/unordered_set.hpp"

class Svc;

class Sender: public Thread, public Event_Handler {
	typedef Thread_Mutex Svc_Map_Lock;
	typedef boost::unordered_map<int, Svc *> Svc_Map;
	typedef List<int, Thread_Mutex> Drop_List;
	typedef Block_List<Thread_Mutex> Data_Block_List;
	typedef boost::unordered_set<int> Svc_Set;

public:
	Sender(void);

	virtual ~Sender(void);

    virtual int thr_cancel_join(void);

	void init(void);

	void fini(void);

	virtual void run_handler(void);

	/// 获取、释放一个buf
	virtual Block_Buffer *pop_block(int cid);

	virtual int push_block(int cid, Block_Buffer *buf);

	int push_data_block_with_len(int cid, Block_Buffer &rbuf);
    int push_pool_block_with_len(Block_Buffer *pbuf);

	int append_send_block(void);

	int push_drop(int cid);

	int process_drop(void);

	int register_svc(Svc *svc);

	int unregister_svc(Svc *svc);

	void set(Time_Value &interval);

	int register_self_timer(void);

	virtual int drop_handler(int cid);

	virtual Svc *find_svc(int cid);

	Epoll_Watcher *reactor(void);

	Time_Value &get_interval(void);

	void set_interval(Time_Value &tv);

	virtual int handle_timeout(const Time_Value &tv);


protected:
	Epoll_Watcher *reactor_;
	Data_Block_List append_list_;
	Drop_List drop_list_;

	Svc_Map_Lock svc_map_lock_;
	Svc_Map svc_map_;
	Svc_Set send_svc_set_;

	Time_Value interval_;
	bool is_register_self_;
	bool is_running_;
};


#endif /* SENDER_H_ */
