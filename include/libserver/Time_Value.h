// -*- C++ -*-
/*
 * aa.cpp
 *
 *  Created on: Mar 19, 2012
 *      Author: ChenLong
 *
 *  此类大部分代码源自ACE_Time_Value
 */

#ifndef TIME_VALUE_H_
#define TIME_VALUE_H_

#include <string>
#include <time.h>
#include <sys/time.h>
#include <cstdlib>

class Time_Value {
public:
	static const int USECS;
	static const int SECOND;
	static const int MINUTE;
	static const int HOUR;
	static const int DAY;
	static const int WEEK;

	friend bool operator > (const Time_Value &tv1, const Time_Value &tv2);
	friend bool operator < (const Time_Value &tv1, const Time_Value &tv2);
	friend bool operator >= (const Time_Value &tv1, const Time_Value &tv2);
	friend bool operator <= (const Time_Value &tv1, const Time_Value &tv2);
	friend Time_Value operator - (const Time_Value &tv1, const Time_Value &tv2);
	friend Time_Value operator + (const Time_Value &tv1, const Time_Value &tv2);
	friend bool operator == (const Time_Value &tv1, const Time_Value &tv2);
	friend bool operator != (const Time_Value &tv1, const Time_Value &tv2);

public:
	static const Time_Value zero;

	static Time_Value gettimeofday();
	static Time_Value gettime(double tm);

	static int sleep(double tm);
	static int sleep(const Time_Value &tv);

	explicit Time_Value(long sec = 0, long usec = 0);
	explicit Time_Value(const struct timeval &t);
	Time_Value(const Time_Value &tv);

	void set(long sec, long usec) {	this->tv_.tv_sec = sec; this->tv_.tv_usec = usec; }
	inline void set(const timeval &t) { this->tv_ = t; }
	inline void set(const Time_Value &tv) { this->tv_ = tv.get_tv(); }

	inline long sec(void) const { return this->tv_.tv_sec; }
	inline void sec(long sec)  { this->tv_.tv_sec = sec; }
	inline long usec(void) const  { return this->tv_.tv_usec; }
	inline void usec(long usec)  { this->tv_.tv_usec = usec; }

	inline const timeval & get_tv(void) const { return this->tv_;	}

	Time_Value & operator += (const Time_Value &tv);
	Time_Value & operator -= (const Time_Value &tv);
	operator timespec() const;

	void normalize(void);

	void debug_dump(void) const;
	void debug_dump(char *str, size_t *sl);

private:
	timeval tv_;
};

//inline bool is_same_week(const Time_Value &tv1, const Time_Value &tv2) {
//	tm tm1, tm2;
//	time_t t1 = tv1.sec(), t2 = tv2.sec();
//
//	if (std::abs(t1 - t2) > Time_Value::WEEK) {
//		return false;
//	}
//
//	if (tv1 > tv2) {
//		std::swap(t1, t2);
//	}
//
//	localtime_r(&t1, &tm1);
//	localtime_r(&t2, &tm2);
//
//	if (tm1.tm_wday < tm2.tm_wday) {
//		return true;
//	} else if (tm1.tm_wday == tm2.tm_wday) {
//		return tm1.tm_yday == tm2.tm_yday;
//	}
//
//	return false;
//}

inline bool is_same_day(const Time_Value &tv1, const Time_Value &tv2) {
	tm tm1, tm2;
	time_t t1 = tv1.sec(), t2 = tv2.sec();

	localtime_r(&t1, &tm1);
	localtime_r(&t2, &tm2);

	if (tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday)
		return true;
	else
		return false;
}

inline bool is_same_hour(const Time_Value &tv1, const Time_Value &tv2) {
	tm tm1, tm2;
	time_t t1 = tv1.sec(), t2 = tv2.sec();

	localtime_r(&t1, &tm1);
	localtime_r(&t2, &tm2);

	if (tm1.tm_year == tm2.tm_year
			&& tm1.tm_mon == tm2.tm_mon
			&& tm1.tm_mday == tm2.tm_mday
			&& tm1.tm_hour == tm2.tm_hour)
		return true;
	else
		return false;
}

//inline void get_zero_time(const Time_Value &tv1, Time_Value &tv2) {
//	int zeor_time = tv1.sec() + (86400 - (tv1.sec() + 28800) % 86400);
//	tv2.set(zeor_time, 0);
//}

inline int Time_Value::sleep(double tm) {
	Time_Value tv = Time_Value::gettime(tm);
	return Time_Value::sleep(tv);
}

inline int Time_Value::sleep(const Time_Value &tv) {
	timespec ts = tv;
	return ::nanosleep (&ts, 0);
}

#endif /* TIME_VALUE_H_ */
