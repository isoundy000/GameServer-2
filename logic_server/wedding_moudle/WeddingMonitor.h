/*
 * WeddingMonitor.h
 *
 * Created on: 2015-06-03 16:55
 *     Author: lyz
 */

#ifndef _WEDDINGMONITOR_H_
#define _WEDDINGMONITOR_H_

#include "GameHeader.h"
#include "GameTimer.h"

class LogicMonitor;
class WeddingDetail;

class WeddingMonitor
{
public:
    typedef boost::unordered_map<Int64, WeddingDetail *> WeddingMap;
    typedef boost::unordered_map<Int64, int> WeddingTypeMap;

	class MailTimer : public GameTimer
	{
	public:
		virtual int type(void);
		virtual int handle_timeout(const Time_Value &tv);

	};
public:
    int init(void);
    int stop(void);

    LogicMonitor *monitor(void);

    bool is_has_wedding(const Int64 role_id);

    WeddingMap &wedding_by_id_map(void);
    WeddingMap &wedding_by_role_map(void);

    WeddingDetail *fetch_wedding_detail(const Int64 role_id);
    WeddingDetail *find_wedding_detail(const Int64 wedding_id);

    int insert_new_wedding(const Int64 partner_1, const Int64 partner_2, const int wedding_type);
    int update_wedding(const Int64 partner_1, const Int64 partner_2, const int wedding_type);

    int remove_wedding(const Int64 wedding_id);

    int process_other_update_intimacy(Message *msg);
    int check_and_update_intimacy(const int type, const int id, LongSet &role_set, const Json::Value &intimacy_json);

    int process_handle_timeout(const Time_Value &nowtime);
    int update_wedding_cache(const Time_Value &nowtime);
    int check_and_update_cruise_icon_list(const Time_Value &nowtime);
    int check_and_update_cartoon_list(const Time_Value &nowtime);

    void insert_update_id(const Int64 id);
    void insert_remove_id(const Int64 id);

    void set_cruise_state(WeddingDetail *wedding_info);
    int notify_all_cruise_icon(void);
    int make_up_all_cruise_icon(Message *msg);
    int process_cruise_finish(Message *msg);

    void set_cartoon_state(WeddingDetail *wedding_info, const int wedding_type);
    int make_up_cartoon_info(const Int64 role_id, Message *msg);

    int notify_to_cruise(WeddingDetail *wedding_info, const int wedding_type);
	int send_mail_to_player();
    int request_save_all_wedding_info(void);

protected:
    int shout_player_cruise_start(WeddingDetail *wedding_info, const int wedding_type);


private:
	MailTimer mail_timer_;
    LogicMonitor *monitor_;
    WeddingMap wedding_by_id_map_;      // key: wedding_id
    WeddingMap wedding_by_role_map_;    // key: role_id

    Time_Value update_cache_tick_;
    LongSet remove_id_set_;
    LongSet update_id_set_;

    Time_Value cruise_icon_check_tick_;
    LongSet cruise_id_set_;

    WeddingTypeMap cartoon_play_map_;
};

typedef Singleton<WeddingMonitor> WeddingMonitorSingle;
#define WEDDING_MONITOR     (WeddingMonitorSingle::instance())

#endif //_WEDDINGMONITOR_H_
