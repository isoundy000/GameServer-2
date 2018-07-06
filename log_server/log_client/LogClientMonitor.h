/*
 * LogClientMonitor.h
 *
 * Created on: 2013-01-09 14:56
 *     Author: glendy
 */

#ifndef _LOGCLIENTMONITOR_H_
#define _LOGCLIENTMONITOR_H_

#include <sstream>
#include "GameHeader.h"

class LogClientMonitor
{
public:
    int init(void);
    int start(void);
    int stop(void);

    void set_server_config_index(const int index);
    int log_scene(void);

    int log_sid(void);
    void set_log_sid(const int sid);
    int get_log_sid(void);

    int push_data_block(Block_Buffer *buff);
    int logging_in_log_server(std::ostringstream &msg_stream);

protected:
    int server_config_index_;
    int server_type_;
    int log_scene_;
    int log_sid_;
};

typedef Singleton<LogClientMonitor> LogClientMonitorSingle;
#define LOG_CLIENT_MONITOR  (LogClientMonitorSingle::instance())

#endif //_LOGCLIENTMONITOR_H_
