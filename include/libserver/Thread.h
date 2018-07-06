// -*- C++ -*-
/*
 * Thread.h
 *
 *  Created on: Mar 21, 2012
 *      Author: ChenLong
 */

#ifndef THREAD_H_
#define THREAD_H_

#include "Lib_Log.h"
#include <pthread.h>

class Thread {
public:
	Thread(void) : tid(0) { }

	virtual ~Thread(void) { }

	static void *create_func(void *arg) {
		Thread *self = (Thread *)arg;
		pthread_cleanup_push(cleanup_func, arg);
		self->run_handler();
		pthread_cleanup_pop(1);
		return self;
	}

	static void cleanup_func(void *arg) {
		Thread *self = (Thread *)arg;
		self->exit_handler();
		return ;
	}

	virtual void run_handler(void) {
		LOG_USER_TRACE("SHOULD NOT HERE");
	}

	virtual void exit_handler(void) { }

	int thr_create(void) {
		return ::pthread_create(&tid, NULL, create_func, this);
	}

	int thr_cancel(void) {
		if (tid == 0)
			return 0;
		return ::pthread_cancel(tid);
	}

	int thr_join(void) {
		if (tid == 0)
			return 0;
		pthread_t ttid = this->tid;
		this->tid = 0;
		return ::pthread_join(ttid, NULL);
	}

	virtual int thr_cancel_join(void) {
		int ret;
		if ((ret = thr_cancel()) == 0) {
			ret = thr_join();
		} else {
			LOG_USER_TRACE("thr_cancel return ret = ", ret);
		}
		return ret;
	}

	void thr_exit(void *rval_ptr) {
		::pthread_exit(rval_ptr);
	}

	pthread_t thread_id() {
		return tid;
	}


private:
	pthread_t tid;
};

#endif /* THREAD_H_ */
