/*
 * DebugServer.h
 *
 * Created on: 2013-01-21 16:15
 *     Author: glendy
 */

#ifndef _DEBUGSERVER_H_
#define _DEBUGSERVER_H_

#include "Singleton.h"
#include "Event_Handler.h"

class Epoll_Watcher;

class DebugServer
{
public:
    enum {
        SIG_UPDATE_CONFIG = 35
    };
    struct TimerHandler : public Event_Handler
    {
    public:
        virtual int handle_timeout(const Time_Value &tv);
    };
public:
    DebugServer(void);
    virtual ~DebugServer(void);

    int init(void);
    int start(void);
    int stop(void);

    int start_as_logic(const int index);
    int start_as_map(const int index);
    int start_as_chat(const int index);
    int start_as_auth(const int index);
    int start_as_gate(const int index);
    int start_as_log(const int index);

    int stop_as_logic(void);
    int stop_as_map(void);
    int stop_as_chat(void);
    int stop_as_auth(void);
    int stop_as_gate(void);
    int stop_as_log(void);

    static void sigcld_handle(int signo);
    int request_update_config(void);

private:
    Epoll_Watcher *watcher_;
    bool is_stopped_;
    TimerHandler timer_handler_;
};

typedef Singleton<DebugServer> DebugServerSingle;
#define DEBUG_SERVER    (DebugServerSingle::instance())

#endif //_DEBUGSERVER_H_
