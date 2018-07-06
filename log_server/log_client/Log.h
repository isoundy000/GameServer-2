/*
 * Log.h
 *
 *  Created on: May 10, 2012
 *      Author: ChenLong
 */

#ifndef LOG_H_
#define LOG_H_

#include <cstdarg>
#include <string>
#include "Time_Value.h"
namespace google
{
	namespace protobuf
	{
		class Message;
	}
}
using ::google::protobuf::Message;

class Log {
public:
	enum {
		F_SYS_ABORT 	= 0x40,
		F_ABORT 		= 0x20,
		F_EXIT 			= 0x10,
		F_SYS 			= 0x8,
		F_USER			= 0x4,
		F_USER_TRACE	= 0x2,
		F_DEBUG			= 0x1,

		M_SYS_ABORT 	= 0,
		M_ABORT 		= 1,
		M_EXIT 			= 2,
		M_SYS 			= 3,
		M_USER			= 4,
		M_USER_TRACE	= 5,
		M_DEBUG			= 6,

		NULL_STUB		= 11,
	};
	static int msg_buf_size;
	static int backtrace_size;
	static std::string msg_head[];

	static Log *instance(void);
    static void destroy(void);

	void msg_sys_abort(const char *fmt, ...);
	void msg_abort(const char *fmt, ...);
	void msg_exit(const char *fmt, ...);
	void msg_sys(const char *fmt, ...);
	void msg_user(const char *fmt, ...);
	void msg_user_trace(const char *fmt, ...);
	void msg_debug(const char *fmt, ...);

	void set_log_type(int type, int sub_type = 0);
	void set_switcher(int switcher);

    int logging_mysql(const std::string& name, const std::string& content);

    int pid(void);

private:
	Log(void);
	virtual ~Log(void);

	void assembly_msg(int log_flag, const char *fmt, va_list ap);
	void logging_file(std::ostringstream &msg_stream);
    
private:
	static Log *instance_;
	int switcher_;
	int log_type_;
	int log_sub_type_;
	bool is_connected_;
	int pid_;
};

////////////////////////////////////////////////////////////////////////////////

inline void Log::set_log_type(int type, int sub_type) {
	log_type_ = type;
	log_sub_type_ = sub_type;
}

inline void Log::set_switcher(int switcher) {
	switcher_ &= switcher;
}

////////////////////////////////////////////////////////////////////////////////
/// 调用abort产生core文件, 结束程序
#define MSG_ABORT(FMT, ...) do {					\
		Log::instance()->msg_abort("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

/// 消息内包含errno和对应的错误描述, 调用abort产生core文件结束程序
#define MSG_SYS_ABORT(FMT, ...) do {					\
		Log::instance()->msg_sys_abort("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

/// 调用exit结束程序
#define MSG_EXIT(FMT, ...) do {						\
		Log::instance()->msg_exit("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

/// 消息内包含errno和对应的错误描述
#define MSG_SYS(FMT, ...) do {						\
		Log::instance()->msg_sys("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)

/// 用户层代码错误信息
#define MSG_USER(FMT, ...) do {						\
		Log::instance()->msg_user("in function %s: "#FMT, __func__, ##__VA_ARGS__); \
	} while (0)

/// 用户层代码错误信息
#define MSG_USER_TRACE(FMT, ...) do {						\
		Log::instance()->msg_user_trace("in function %s: "#FMT, __func__, ##__VA_ARGS__); \
	} while (0)

/// 调试信息
#ifdef LOCAL_DEBUG
#define MSG_DEBUG(FMT, ...) do {					\
		Log::instance()->msg_debug("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)
#else
#ifdef LOG_DEBUG_MSG
#define MSG_DEBUG(FMT, ...) do {					\
		Log::instance()->msg_debug("in %s:%d function %s: "#FMT, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
	} while (0)
#else
#define MSG_DEBUG(FMT, ...)
#endif
#endif

class Perf_Mon {
public:
	Perf_Mon(int msg_id)
		: msg_id_(msg_id), tv_(Time_Value::gettimeofday()) { }

	~Perf_Mon(void) {
		Time_Value res_tv, intv(0, 10);
		if ((res_tv = Time_Value::gettimeofday() - tv_) > intv) {
			MSG_DEBUG("[Poor Performance] msg_id = %d, use time [sec = %ld, usec = %ld]",
				  msg_id_, res_tv.sec(), res_tv.usec());
		}
	}

private:
	int msg_id_;
	Time_Value tv_;
};

#define PERF_MON do {				\
		Perf_Mon perf_mon;		\
	} while (0);

#ifdef LOCAL_DEBUG
#define MSG_PERF_MON(msg_id) do {		\
		Perf_Mon perf_mon(msg_id);	\
	} while (0);
#else
#define MSG_PERF_MON(msg_id)
#endif

#endif /* LOG_H_ */
