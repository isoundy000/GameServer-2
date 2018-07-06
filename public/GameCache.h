/*
 * GameCache.h
 *
 * Created on: 2013-04-28 17:17
 *     Author: lyz
 */

#ifndef _GAMECACHE_H_
#define _GAMECACHE_H_

#include "GameEnum.h"
#include "GameHeader.h"

class GameCache
{
public:
    typedef std::vector<bool> CacheUpdateList;
    typedef std::vector<Time_Value> TimeUpList;
    typedef std::vector<double> TimeIntervalList;

public:
    int init(const int cache_update_size, const int timeup_size, const double *timeup_interval, const int interval_size);

    void update_cache(const int type, const bool value = true);
    bool check_cache(const int type);
    void reset_cache_flag(void);

    bool check_timeout(const int type, const Time_Value &nowtime);
    void update_timeup_tick(const int type, const Time_Value &nowtime);

protected:
    CacheUpdateList cache_update_map_;
    TimeUpList timeup_map_;
    TimeIntervalList timeup_interval_;
    int interval_size_;
};

inline int GameCache::init(const int cache_update_size, const int timeup_size, const double *timeup_interval, const int interval_size)
{
    this->cache_update_map_.resize(cache_update_size);
    this->timeup_map_.resize(timeup_size);
    this->timeup_interval_.resize(timeup_size);

    for (int i = 0; i < cache_update_size; ++i)
        this->cache_update_map_[i] = false;

    for (int i = 0; i < timeup_size; ++i)
    {
        if (i < interval_size)
            this->timeup_interval_[i] = timeup_interval[i];
        else
            this->timeup_interval_[i] = GameEnum::DEFAULT_TIME_OUT;
        this->timeup_map_[i] = Time_Value::zero;
    }
    this->interval_size_ = interval_size;
    return 0;
}

inline void GameCache::update_cache(const int type, const bool value)
{
    if (type < int(this->cache_update_map_.size()))
        this->cache_update_map_[type] = value;
}

inline bool GameCache::check_cache(const int type)
{
    if (type >= int(this->cache_update_map_.size()))
        return false;

    return this->cache_update_map_[type];
}

inline void GameCache::reset_cache_flag(void)
{
    for (CacheUpdateList::iterator iter = this->cache_update_map_.begin(); iter != this->cache_update_map_.end(); ++iter)
        *iter = false;
}

inline bool GameCache::check_timeout(const int type, const Time_Value &nowtime)
{
    if (type >= int(this->timeup_map_.size()))
        return false;

    return (this->timeup_map_[type] < nowtime);
}

inline void GameCache::update_timeup_tick(const int type, const Time_Value &nowtime)
{
    if (type >= int(this->timeup_map_.size()))
        return;

    double inter = GameEnum::DEFAULT_TIME_OUT;
    if (0 <= type && type < this->interval_size_)
        inter = this->timeup_interval_[type];
    double sec = 0;
    double usec = modf(inter, &sec) * 1e6;

//    this->timeup_map_[type] = nowtime + Time_Value(long(sec), long(usec));

    Time_Value &check_time = this->timeup_map_[type];
	long long int check_tick = check_time.sec() * 1000000 + check_time.usec(),
			now_tick = nowtime.sec() * 1000000 + nowtime.usec(),
			interval_tick = sec * 1000000 + usec;
	long long int next_check = interval_tick;
	if (interval_tick > 0)
		 next_check -= ((now_tick - check_tick) % interval_tick);
	check_time = nowtime + Time_Value(next_check / 1000000, next_check % 1000000);
}

#endif //_GAMECACHE_H_
