/*
 * DaemonServer.h
 *
 * Created on: 2013-01-17 10:58
 *     Author: glendy
 */

#ifndef _DAEMONSERVER_H_
#define _DAEMONSERVER_H_

#include "GameDefine.h"
#include "Singleton.h"
#include "Event_Handler.h"
#include <map>
#include <set>
#include <vector>

class DaemonServer
{
public:
    enum {
    	MAX_ARG_ACCOUNT = 10,
        MAX_ARG_LENGTH  = 512,
        SIG_START_SERVER = 34,
        SIG_UPDATE_CONFIG = 35,
        SIG_RESTART_LOGIC = 36,
        SIG_NORMAL_EXIT	= 37,

        P_STATUS_RUNNING = 1,
        P_STATUS_DEATH = 2
    };

    class ChildHandler : public Event_Handler
    {
    public:
        virtual ~ChildHandler(void);
        virtual int handle_timeout(const Time_Value &tv);
    protected:
        Time_Value pool_report_tick_;
    };

    //typedef boost::unordered_map<int, int> Child_Map;
    typedef std::map<int, int> ChildCoreMap;
    typedef std::map<int, int> ChildCoreTickMap;
    typedef std::set<int> GatePidSet;

public:
    int init(int argc, char *argv[]);
    int start(int argc, char *argv[]);

    static void sigcld_handle(int signo);
    static void sig_control_handle(int signo);

    Epoll_Watcher *watcher(void);

    void restart_process(const int index, const int pid, bool is_force_start = false);

    bool is_running(void);
    int server_index(void);
    int server_type(void);
    bool is_server_log(void);
    bool is_server_logic(void);
    bool is_server_sandbox(void);
    bool is_server_map(void);
    bool is_server_chat(void);
    bool is_server_daemon(void);
    bool is_server_auth(void);
    bool is_server_gate(void);
    bool is_normal_exit(void);
    void set_is_normal_exit(const bool flag);

    int force_restart_process(const int index);
    int request_update_config(void);
    int check_log_can_stop(void);

    const char *parent_path(void);

    // 定时检查子进程是否已死亡
    int check_process_state(const int pid);
    void set_check_child_start(const bool flag);
    void check_child_need_restart(void);
    // 热更动态添加新进程
    void check_update_child_list(void);

protected:
    int fork_process_by_index(const int index);
    int fork_process(const int index, char *file, char * const *args);

    int start_server(int argc, char *argv[]);
    int start_as_daemon(void);
    int start_as_log(void);
    int start_as_logic(void);
    int start_as_sandbox(void);
    int start_as_map(void);
    int start_as_chat(void);
    int start_as_auth(void);
    int start_as_gate(void);

    int stop_server(void);
    int stop_as_daemon(void);
    int stop_as_log(void);
    int stop_as_logic(void);
    int stop_as_sandbox(void);
    int stop_as_map(void);
    int stop_as_chat(void);
    int stop_as_auth(void);
    int stop_as_gate(void);

    void fina_as_log(void);
    void fina_as_logic(void);
    void fina_as_sanbox(void);
    void fina_as_map(void);
    void fina_as_chat(void);
    void fina_as_auth(void);
    void fina_as_gate(void);

    void report_pool_info(void);

    void fork_server_list(void);

protected:
    int ppid_;
    char pppath_[1024];
    int server_index_;
    int server_type_;
    bool is_running_;
    bool is_normal_exit_;

    Epoll_Watcher *watcher_;
//    Child_Map child_map_;     // key: pid, value: server_index;
    ChildHandler child_handler_;
    std::vector<int> child_list_;      // index: server_index, value: pid;
    bool check_child_start_;

    std::string machine_name_;
    std::string machine_num_;

    char process_files_[50][MAX_ARG_LENGTH + 1];
    char process_args_[50][MAX_ARG_ACCOUNT][MAX_ARG_LENGTH + 1];

    ChildCoreMap child_core_map_;       // key: server_index, value: core times
    ChildCoreTickMap child_coretick_map_;   // key: server_index, value: core times update tick
    GatePidSet gate_pid_set_;
};

typedef Singleton<DaemonServer> DaemonServerSingle;
#define DAEMON_SERVER   (DaemonServerSingle::instance())

#endif //_DAEMONSERVER_H_
