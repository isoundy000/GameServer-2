/*
 * Log.cpp
 *
 *  Created on: May 10, 2012
 *      Author: ChenLong
 */

#include "Log.h"
#include "Block_Buffer.h"
#include "LogClientMonitor.h"
#include "LogStruct.h"
#include "PoolMonitor.h"
#include <execinfo.h>
#include <syslog.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "google/protobuf/message.h"
#include "DaemonServer.h"
#ifndef LOCAL_DEBUG
#include "ProtoDefine.h"
#endif

Log::Log(void)
: switcher_(F_SYS_ABORT | F_ABORT | F_EXIT | F_SYS | F_USER | F_USER_TRACE | F_DEBUG)
{
	this->is_connected_ = false;
	this->log_type_ = 0;
	this->log_sub_type_ = 0;
	this->pid_ = ::getpid();
}

Log::~Log(void) { }

Log *Log::instance_;

int Log::msg_buf_size = 4096;

std::string Log::msg_head[] = {
		"[MSG_SYS_ABORT] ",		/// M_SYS_ABORT 	= 0,
		"[MSG_ABORT] ",			/// M_ABORT 		= 1,
		"[MSG_EXIT] ",			/// M_EXIT 			= 2,
		"[MSG_SYS] ",			/// M_SYS 			= 3,
		"[MSG_USER] ",			/// M_USER			= 4,
		"[MSG_USER_TRACE] ",	/// M_USER_TRACE	= 5,
		"[MSG_DEBUG] ",			/// M_DEBUG			= 6,
		"[CHAT] ",				/// LOGGING_CHAT	= 7,
		"[LOGIC] ",				/// LOGGING_LOGIC	= 8,
		"[MAP]",				/// LOGGING_MAP		= 9,
		"[LOG]",				/// LOGGING_LOG		= 10,
		"[NULL]"				/// NULL_STUB		= 11,
};

Log *Log::instance(void) {
	if (! instance_)
		instance_ = new Log;
	return instance_;
}

void Log::destroy(void)
{
    if (instance_ != 0)
        delete instance_;
    instance_ = 0;
}

int Log::backtrace_size = 512;

void Log::msg_abort(const char *fmt, ...) {
	va_list	ap;

	va_start(ap, fmt);
	assembly_msg(M_ABORT, fmt, ap);
	va_end(ap);
}

void Log::msg_sys_abort(const char *fmt, ...) {
	if (switcher_ & F_SYS_ABORT) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_SYS_ABORT, fmt, ap);
		va_end(ap);
	}
}

void Log::msg_exit(const char *fmt, ...) {
	if (switcher_ & F_EXIT) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_EXIT, fmt, ap);
		va_end(ap);
	}
}

void Log::msg_sys(const char *fmt, ...) {
	if (switcher_ & F_SYS) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_SYS, fmt, ap);
		va_end(ap);
	}
}

void Log::msg_user(const char *fmt, ...) {
	if (switcher_ & F_USER) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_USER, fmt, ap);
		va_end(ap);
	}
}

void Log::msg_user_trace(const char *fmt, ...) {
	if (switcher_ & F_USER_TRACE) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_USER_TRACE, fmt, ap);
		va_end(ap);
	}
}

void Log::msg_debug(const char *fmt, ...) {
	if (switcher_ & F_DEBUG) {
		va_list	ap;

		va_start(ap, fmt);
		assembly_msg(M_DEBUG, fmt, ap);
		va_end(ap);
	}
}

void Log::assembly_msg(int log_flag, const char *fmt, va_list ap) {
	std::ostringstream msg_stream;

	struct tm tm_v;
	time_t time_v = time(NULL);

	localtime_r(&time_v, &tm_v);

//#ifndef LOCAL_DEBUG
//	msg_stream << "<pid=" << (int)this->pid_ << "|tid=" << pthread_self()
//			<< ">(" << (tm_v.tm_hour) << ":" << (tm_v.tm_min) << ":" << (tm_v.tm_sec) << ")";
//#else
	msg_stream << "(" << std::setfill('0') << std::setw(2) << (tm_v.tm_hour) << ":"
			<< std::setfill('0') << std::setw(2) << (tm_v.tm_min) << ":"
			<< std::setfill('0') << std::setw(2) << (tm_v.tm_sec) << ")";
//#endif

	msg_stream << msg_head[log_flag];

	char line_buf[msg_buf_size];
	memset(line_buf, 0, sizeof(line_buf));
	vsnprintf(line_buf, sizeof(line_buf), fmt, ap);

	msg_stream << line_buf;

	switch (log_flag) {
	case M_SYS_ABORT: {
		msg_stream << "errno = " << errno;

		memset(line_buf, 0, sizeof(line_buf));
		strerror_r(errno, line_buf, sizeof(line_buf));
		msg_stream << ", errstr=[" << line_buf << "]" << std::endl;
		logging_file(msg_stream);
		abort();

		break;
	}
	case M_ABORT: {
		msg_stream << std::endl;
		logging_file(msg_stream);
		abort();

		break;
	}
	case M_EXIT: {
		msg_stream << std::endl;
		logging_file(msg_stream);
		exit(1);

		break;
	}
	case M_SYS: {
		msg_stream << ", errno = " << errno;

		memset(line_buf, 0, sizeof(line_buf));
		strerror_r(errno, line_buf, sizeof(line_buf));
		msg_stream << ", errstr=[" << line_buf << "]" << std::endl;

		logging_file(msg_stream);

		break;
	}
	case M_USER: {
		msg_stream << std::endl;
		logging_file(msg_stream);

		break;
	}
	case M_USER_TRACE: {
		int nptrs;
		void *buffer[backtrace_size];
		char **strings;

		nptrs = backtrace(buffer, backtrace_size);
		strings = backtrace_symbols(buffer, nptrs);
		if (strings == NULL)
			return ;

		msg_stream << std::endl;

		for (int i = 0; i < nptrs; ++i) {
			msg_stream << (strings[i]) << std::endl;
		}

		free(strings);

		logging_file(msg_stream);

		break;
	}
	case M_DEBUG: {
		msg_stream << std::endl;
		logging_file(msg_stream);

		break;
	}
	default: {
		break;
	}
	}

	return ;
}

void Log::logging_file(std::ostringstream &msg_stream) {
#ifdef LOCAL_DEBUG
	std::cerr << msg_stream.str();
#else
	if (msg_stream.str().length() > 1024 * 1024 * 3)
	{
		LOG_USER_INFO("log too len %d %d %d", this->log_type_, this->pid_, msg_stream.str().length());
		return;
	}

    if (DAEMON_SERVER->is_server_log() == true)
    {
    	LOG_CLIENT_MONITOR->logging_in_log_server(msg_stream);
    }
    int log_sid = LOG_CLIENT_MONITOR->log_sid();
    if (log_sid < 0)
    {
    	Lib_Log::instance()->msg_user("%s", msg_stream.str().c_str());
    	return;
    }

    ProtoHead head;
    head.__recogn = INNER_LOG_WRITE;

    int32_t pid = this->pid_,/* tid = ::pthread_self(),*/ nowtime = ::time(0);
    Proto30310001 log_info;
    log_info.set_log_type(this->log_type_);
    log_info.set_log_sub_type(this->log_sub_type_);
    log_info.set_pid(pid);
    log_info.set_nowtime(nowtime);
    log_info.set_log_text(msg_stream.str());

    int byte_size = log_info.ByteSize();

    uint32_t length = sizeof(ProtoHead) + byte_size;

    Block_Buffer *buff = POOL_MONITOR->pop_buf_block();
    buff->ensure_writable_bytes(length + sizeof(int) * 4);
    buff->write_int32(log_sid);
    buff->write_uint32(length);
    buff->copy((char *)&head, sizeof(ProtoHead));
    log_info.SerializeToArray(buff->get_write_ptr(), buff->writable_bytes());
    buff->set_write_idx(buff->get_write_idx() + byte_size);

    if (LOG_CLIENT_MONITOR->push_data_block(buff) == -1)
    {
    	Lib_Log::instance()->msg_user("%s", msg_stream.str().c_str());
    	POOL_MONITOR->push_buf_block(buff);
    }

#endif
}

int Log::logging_mysql(const std::string& type_name, const std::string& content)
{
#ifdef LOCAL_DEBUG
#ifndef TEST_SERIAL
	return 0;
#endif
#endif

	int log_sid = LOG_CLIENT_MONITOR->log_sid();
    if (log_sid < 0)
    {
    	Lib_Log::instance()->msg_user("ERROR mysql request[%s]", type_name.c_str());
    	return -1;
    }

    ProtoHead head;
    head.__recogn = INNER_MYSQL_INSERT_WITH_TABLE_NAME;

    uint32_t byte_size = content.size();
    uint32_t length = sizeof(ProtoHead) + string_to_block_len(type_name) + byte_size;

    Block_Buffer *buff = POOL_MONITOR->pop_buf_block();
    buff->ensure_writable_bytes(length + sizeof(uint32_t) * 4);

    buff->write_int32(log_sid);
    buff->write_uint32(length);
    buff->copy((char *)&head, sizeof(ProtoHead));
    buff->write_string(type_name);
    buff->copy(content.c_str(), byte_size);

    if (LOG_CLIENT_MONITOR->push_data_block(buff) < 0)
    {
    	Lib_Log::instance()->msg_user("ERROR mysql request send[%s]", type_name.c_str());
    	POOL_MONITOR->push_buf_block(buff);
    	return -1;
    }

    return 0;
}

int Log::pid(void)
{
	return this->pid_;
}

