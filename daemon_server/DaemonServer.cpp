/*
 * DaemonServer.cpp
 *
 * Created on: 2013-01-17 11:08
 *     Author: glendy
 */

#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "DaemonServer.h"
#include "Log.h"
#include "GameConfig.h"
#include "LogServerMonitor.h"
#include "LogClientMonitor.h"
#include "LogicMonitor.h"
#include "SandMonitor.h"
#include "MapMonitor.h"
#include "ChatMonitor.h"
#include "AuthMonitor.h"
#include "GateMonitor.h"
#include "TransactionMonitor.h"
#include "MongoConnector.h"
#include "GameCommon.h"

#define LOG_PROGRAM     "./log_server/log_server"
#define LOGIC_PROGRAM   "./logic/logic"
#define SANDBOX_PROGRAM "./sand/sand"
#define MAP_PROGRAM     "./map/map"
#define CHAT_PROGRAM    "./chat/chat"
#define AUTH_PROGRAM    "./auth/auth"
#define GATE_PROGRAM    "./gate/gate"
#define MONGO_THREAD_SIZE	1

DaemonServer::ChildHandler::~ChildHandler(void)
{ /*NULL*/ }

int DaemonServer::ChildHandler::handle_timeout(const Time_Value &tv)
{
	if (DAEMON_SERVER->is_running() == true && DAEMON_SERVER->is_normal_exit() == true)
	{
    	LOG_USER_INFO("self exit %d\n", DAEMON_SERVER->server_index());
//        DAEMON_SERVER->watcher()->remove(this);
        DAEMON_SERVER->stop_server();

        return 0;
	}

    if (DAEMON_SERVER->is_server_daemon() == false)
    {
        if (::access(DAEMON_SERVER->parent_path(), F_OK) != 0 && DAEMON_SERVER->is_running() == true)
        {
        	if (DAEMON_SERVER->is_server_log() == true)
        	{
        		if (DAEMON_SERVER->check_log_can_stop() != 0)
        		{
        			return 0;
        		}
        	}
        	LOG_USER_INFO("self exit %d\n", DAEMON_SERVER->server_index());
//            DAEMON_SERVER->watcher()->remove(this);
            DAEMON_SERVER->stop_server();
        }

        if (DAEMON_SERVER->is_running() == true)
        {
            Time_Value nowtime = Time_Value::gettimeofday();
            if (this->pool_report_tick_ < nowtime)
            {
                DAEMON_SERVER->report_pool_info();
                this->pool_report_tick_ = nowtime + Time_Value(300);
            }
        }
    }
    else
    {
        DAEMON_SERVER->check_child_need_restart();
    }
    return 0;
}

int DaemonServer::init(int argc, char *argv[])
{
    ::memset(this->process_files_, 0, sizeof(this->process_files_));
    ::memset(this->process_args_, 0, sizeof(this->process_args_));
    this->watcher_ = 0;
    this->server_index_ = -1;
    this->server_type_ = 0;
    this->is_running_ = false;
    this->is_normal_exit_ = false;
    this->check_child_start_ = false;

    if ((this->watcher_ = new Epoll_Watcher()) == 0)
        return -1;

    return 0;
}

int DaemonServer::start(int argc, char *argv[])
{
    this->is_running_ = true;
    if (this->start_server(argc, argv) < 0)
    {
        this->is_running_ = false;
        return -1;
    }

    this->watcher_->loop();
    return 0;
}

void DaemonServer::sigcld_handle(int signo)
{
//    pid_t pid = ::wait(0);
//    if (pid < 0)
//        return;
    DAEMON_SERVER->set_check_child_start(true);

    //DAEMON_SERVER->restart_process(pid);
}

void DaemonServer::sig_control_handle(int signo)
{
    if (signo == SIG_START_SERVER)
    {
    	LOG_USER_INFO("Daemon catch start server signal");
        Json::Value json;
        if (CONFIG_INSTANCE->load_daemon_config(json) != 0)
        {
        	LOG_USER_INFO("Daemon ERROR no daemon.json");
            return;
        }
        int index = json["index"].asInt();
        DAEMON_SERVER->force_restart_process(index);
        ::unlink("config/daemon.json");

        LOG_USER_INFO("Daemon finish force start server");
    }
    else if (signo == SIG_UPDATE_CONFIG)
    {
        UnitMessage msg;
        msg.__msg_head.__recogn = INNER_UPDATE_CONFIG;
        msg.__type = UnitMessage::TYPE_BLOCK_BUFF;
        switch (DAEMON_SERVER->server_type())
        {
            case SERVER_LOGIC:
                LOGIC_MONITOR->logic_unit()->push_request(msg);
                break;
            case SERVER_MAP:
                MAP_MONITOR->map_unit()->push_request(msg);
                break;
            case SERVER_CHAT:
                CHAT_MONITOR->logic_unit()->push_request(msg);
                break;
            case SERVER_GATE:
                GATE_MONITOR->logic_unit()->push_request(msg);
                break;
            case SERVER_AUTH:
                AUTH_MONITOR->logic_unit()->push_request(msg);
                break;
            default:
                break;
        }
    }
    else if (signo == SIG_RESTART_LOGIC)
    {
        UnitMessage msg;
        msg.__msg_head.__recogn = INNER_RELOGIN_LOGIC;
        msg.__type = UnitMessage::TYPE_BLOCK_BUFF;
        GATE_MONITOR->logic_unit()->push_request(msg);
    }
    else if (signo == SIG_NORMAL_EXIT)
    {
    	DAEMON_SERVER->set_is_normal_exit(true);
    }
}

Epoll_Watcher *DaemonServer::watcher(void)
{
    return this->watcher_;
}

void DaemonServer::restart_process(const int index, const int pid, bool is_force_start)
{
    this->gate_pid_set_.erase(pid);

    time_t nowtime = ::time(0);

    if (is_force_start == true)
    {
		this->child_coretick_map_[index] = nowtime;
		this->child_core_map_[index] = 0;
    }
    else
    {
		if (this->child_coretick_map_[index] + 66 < nowtime)
		{
			this->child_coretick_map_[index] = nowtime;
			this->child_core_map_[index] = 0;
		}
		++this->child_core_map_[index];

		if (this->child_core_map_[index] > 10)
		{
			std::cout << "core times too more " << this->child_core_map_[index] << std::endl;
			MSG_USER("core times too more %d", this->child_core_map_[index]);
			return;
		}
    }

    std::cout << "restart process " << pid << " " << index << std::endl;
    MSG_USER("restart process %d %d", pid, index);

    this->fork_process_by_index(index);
}

bool DaemonServer::is_running(void)
{
    return this->is_running_;
}

int DaemonServer::server_index(void)
{
    return this->server_index_;
}

int DaemonServer::server_type(void)
{
    return this->server_type_;
}

bool DaemonServer::is_server_log(void)
{
    return this->server_type_ == SERVER_LOG;
}

bool DaemonServer::is_server_logic(void)
{
    return this->server_type_ == SERVER_LOGIC;
}

bool DaemonServer::is_server_sandbox(void)
{
    return this->server_type_ == SERVER_SAND;
}

bool DaemonServer::is_server_map(void)
{
    return this->server_type_ == SERVER_MAP;
}

bool DaemonServer::is_server_chat(void)
{
    return this->server_type_ == SERVER_CHAT;
}

bool DaemonServer::is_server_daemon(void)
{
    return this->server_type_ == SERVER_DAEMON;
}

bool DaemonServer::is_server_auth(void)
{
    return this->server_type_ == SERVER_AUTH;
}

bool DaemonServer::is_server_gate(void)
{
    return this->server_type_ == SERVER_GATE;
}

bool DaemonServer::is_normal_exit(void)
{
	return this->is_normal_exit_;
}

void DaemonServer::set_is_normal_exit(const bool flag)
{
	this->is_normal_exit_ = flag;
}

int DaemonServer::fork_process_by_index(const int index)
{
    int i = 0;
    char *args[MAX_ARG_ACCOUNT] = {0, };
    for (i = 0; i < MAX_ARG_ACCOUNT && this->process_args_[index][i][0] != '\0'; ++i)
    {
        args[i] = this->process_args_[index][i];
    }

    args[i] = 0;
    int pid = this->fork_process(index, this->process_files_[index], args);
    if (pid > 0) 
    {
        while (int(this->child_list_.size()) <= index)
        {
            this->child_list_.push_back(0);
        }
        this->child_list_[index] = pid;
    }

    return pid;
}

int DaemonServer::fork_process(const int index, char *file, char * const *args)
{
    pid_t pid = ::fork();
    if (pid < 0)
    {
        return -1;
    }
    else if (pid == 0)
    {
        if (::execvp(file, args) < 0)
        {
            ::exit(-1);
        }
    }
    else
    {
        if (this->child_core_map_[index] > 10)
        {
            this->child_core_map_[index] = 0;
        }
    }

    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][index];
    const std::string service_name = server_json["service"].asString();
    if (service_name == SERVICE_NAME_LOGIC)
    {
        ::sleep(2);
        for (DaemonServer::GatePidSet::iterator iter = this->gate_pid_set_.begin();
        		iter != this->gate_pid_set_.end(); ++iter)
        {
            ::kill(*iter, SIG_RESTART_LOGIC);
        }
    }
    else if (service_name == SERVICE_NAME_GATE)
    {
        this->gate_pid_set_.insert(pid);
    }

    return pid;
}

int DaemonServer::start_server(int argc, char *argv[])
{
    ::signal(SIGPIPE, SIG_IGN);
    register_signal(SIG_UPDATE_CONFIG, DaemonServer::sig_control_handle);
    register_signal(SIG_NORMAL_EXIT, DaemonServer::sig_control_handle);

    if (argc <= 3)
    {
        if (argc >= 2)
        {
            this->machine_name_ = argv[1];
        }

        if (argc >= 3)
        {
            this->machine_num_ = argv[2];
        }

        this->server_type_ = SERVER_DAEMON;
        CONFIG_INSTANCE->init("daemon");

        Time_Value timeout_tv(5, 0);
        this->watcher_->add(&(this->child_handler_), Epoll_Watcher::EVENT_TIMEOUT, &timeout_tv);

        return this->start_as_daemon();
    }

    this->ppid_ = ::getppid();
    JUDGE_RETURN(this->ppid_ != 1, -1);

    ::sprintf(this->pppath_, "/proc/%d", this->ppid_);

    this->server_index_ = ::atoi(argv[2]);
    CONFIG_INSTANCE->init(argv[1]);

    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][this->server_index_];
    JUDGE_RETURN(server_json.empty() == false, -1);


    Time_Value timeout_tv(5, 0);
    this->watcher_->add(&(this->child_handler_), Epoll_Watcher::EVENT_TIMEOUT, &timeout_tv);

    // START CLIENT LOG
    std::string service_type_name = server_json["service"].asString();
    if (service_type_name != SERVICE_NAME_LOG)
    {
        LOG_CLIENT_MONITOR->init();
        LOG_CLIENT_MONITOR->set_server_config_index(this->server_index_);
        LOG_CLIENT_MONITOR->start();
    }

    if (service_type_name == SERVICE_NAME_LOGIC)
    {
        this->server_type_ = SERVER_LOGIC;
        this->start_as_logic();
    }
    else if (service_type_name == SERVICE_NAME_CHAT)
    {
        this->server_type_ = SERVER_CHAT;
        this->start_as_chat();
    }
    else if (service_type_name == SERVICE_NAME_LOG)
    {
        this->server_type_ = SERVER_LOG;
        this->start_as_log();
    }
    else if (service_type_name == SERVICE_NAME_SAND)
    {
        this->server_type_ = SERVER_SAND;
        this->start_as_sandbox();
    }
    else if (service_type_name == SERVICE_NAME_MAP)
    {
        this->server_type_ = SERVER_MAP;
        this->start_as_map();
    }
    else if (service_type_name == SERVICE_NAME_AUTH)
    {
        this->server_type_ = SERVER_AUTH;
        this->start_as_auth();
    }
    else if (service_type_name == SERVICE_NAME_GATE)
    {
        register_signal(SIG_RESTART_LOGIC, DaemonServer::sig_control_handle);
        this->server_type_ = SERVER_GATE;
        this->start_as_gate();
    }
    else
        return -1;

    return 0;
}

int DaemonServer::start_as_daemon(void)
{
    ::signal(SIGCLD, sigcld_handle);
    if (register_signal(SIG_START_SERVER, DaemonServer::sig_control_handle) != 0)
        std::cout << "ERROR register singal " << SIG_START_SERVER << ", " << errno << std::endl;
    else
        std::cout << "register singal " << SIG_START_SERVER << std::endl;

    CACHED_INSTANCE->update_offline_tick();

    this->child_list_.clear();
    this->fork_server_list();

    CACHED_INSTANCE->disconnect();

    return 0;
}

void DaemonServer::fork_server_list(void)
{
    std::string service_name;
    std::string run_machine = CONFIG_INSTANCE->global()["run_machine"].asString();

    char *args[MAX_ARG_ACCOUNT] = {0, };
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];

    for (uint i = this->child_list_.size(); i < servers_json.size(); ++i)
    {
        if (servers_json[i]["machine"].asString() != run_machine)
        {
            continue;
        }

        service_name = servers_json[i]["service"].asString();
        if (service_name == SERVICE_NAME_LOG)
        {
            ::strncpy(this->process_files_[i], LOG_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], LOG_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';
        }
        else if (service_name == SERVICE_NAME_LOGIC)
        {
            ::strncpy(this->process_files_[i], LOGIC_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], LOGIC_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';
        }
        else if (service_name == SERVICE_NAME_SAND)
        {
            ::strncpy(this->process_files_[i], SANDBOX_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], SANDBOX_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';
        }
        else if (service_name == SERVICE_NAME_MAP)
        {
            ::strncpy(this->process_files_[i], MAP_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], MAP_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';
        }
        else if (service_name == SERVICE_NAME_CHAT)
        {
            ::strncpy(this->process_files_[i], CHAT_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], CHAT_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';

            Time_Value::sleep(1);
        }
        else if (service_name == SERVICE_NAME_AUTH)
        {
            ::strncpy(this->process_files_[i], AUTH_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], AUTH_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';

            Time_Value::sleep(1);
        }
        else if (service_name == SERVICE_NAME_GATE)
        {
            ::strncpy(this->process_files_[i], GATE_PROGRAM, MAX_ARG_LENGTH);
            this->process_files_[i][MAX_ARG_LENGTH] = '\0';

            ::strncpy(this->process_args_[i][0], GATE_PROGRAM, MAX_ARG_LENGTH);
            this->process_args_[i][0][MAX_ARG_LENGTH] = '\0';

            Time_Value::sleep(1);
        }
        else
        {
            continue;
        }

        int j = 1;
        ::strncpy(this->process_args_[i][j], service_name.c_str(), MAX_ARG_LENGTH);
        this->process_args_[i][j][MAX_ARG_LENGTH] = '\0';
        ++j;

        ::sprintf(this->process_args_[i][j], "%d", i);
        ++j;

        ::strncpy(this->process_args_[i][j], servers_json[i]["name"].asString().c_str(), MAX_ARG_LENGTH);
        this->process_args_[i][j][MAX_ARG_LENGTH] = '\0';
        ++j;

        ::strncpy(this->process_args_[i][j], this->machine_name_.c_str(), MAX_ARG_LENGTH);
        this->process_args_[i][j][MAX_ARG_LENGTH] = '\0';
        ++j;

        ::strncpy(this->process_args_[i][j], this->machine_num_.c_str(), MAX_ARG_LENGTH);
        this->process_args_[i][j][MAX_ARG_LENGTH] = '\0';
        ++j;

        this->child_core_map_[i] = 1;
        this->child_coretick_map_[i] = ::time(0);

        int k = 0;
        for (k = 0; k < j; ++k)
        {
            args[k] = this->process_args_[i][k];
        }

        args[k] = 0;
        int pid = this->fork_process(i, this->process_files_[i], args);
        if (pid > 0) 
        {
            while (this->child_list_.size() <= i)
            {
                this->child_list_.push_back(0);
            }

            this->child_list_[i] = pid;
            if (service_name == SERVICE_NAME_CHAT)
            {
                Time_Value::sleep(1);
            }
        }
    }
}

int DaemonServer::start_as_log(void)
{
    LOG_SERVER_MONITOR->init(1);
    LOG_SERVER_MONITOR->set_server_config_index(this->server_index_);
    LOG_SERVER_MONITOR->start();

    return 0;
}

int DaemonServer::start_as_logic(void)
{
	LogicMonitorSingle::instance()->init();
    TRANSACTION_MONITOR->init(this->server_type_, MONGO_THREAD_SIZE);
    TRANSACTION_MONITOR->start();

    LOGIC_MONITOR->set_server_config_index(this->server_index_);
    LOGIC_MONITOR->start();
    TRANSACTION_MONITOR->start_timer();

    return 0;
}

int DaemonServer::start_as_sandbox(void)
{
    SAND_MONITOR->init();
    SAND_MONITOR->set_server_config_index(this->server_index_);
    SAND_MONITOR->start();
    return 0;
}

int DaemonServer::start_as_map(void)
{
	MapMonitorSingle::instance()->init(this->server_index_);
    TRANSACTION_MONITOR->init(this->server_type_, MONGO_THREAD_SIZE);
    TRANSACTION_MONITOR->start();

    MAP_MONITOR->set_server_config_index(this->server_index_);
    MAP_MONITOR->start();
    TRANSACTION_MONITOR->start_timer();

    return 0;
}

int DaemonServer::start_as_chat(void)
{
    CHAT_MONITOR->init();
    TRANSACTION_MONITOR->init(this->server_type_, MONGO_THREAD_SIZE);
    TRANSACTION_MONITOR->start();

    CHAT_MONITOR->set_server_config_index(this->server_index_);
    TRANSACTION_MONITOR->start_timer();
    CHAT_MONITOR->start();

    return 0;
}

int DaemonServer::start_as_auth(void)
{
    AUTH_MONITOR->init();

    AUTH_MONITOR->set_server_config_index(this->server_index_);
    AUTH_MONITOR->start();
    return 0;
}

int DaemonServer::start_as_gate(void)
{
    GATE_MONITOR->init(this->server_index_);
    TRANSACTION_MONITOR->init(this->server_type_, MONGO_THREAD_SIZE);
    TRANSACTION_MONITOR->start();

    GATE_MONITOR->set_server_config_index(this->server_index_);
    GATE_MONITOR->start();
    TRANSACTION_MONITOR->start_timer();
    return 0;
}

int DaemonServer::stop_server(void)
{
    this->is_running_ = false;
    switch (this->server_type_)
    {
        case SERVER_LOG:
            this->stop_as_log();
            break;
        case SERVER_LOGIC:
            this->stop_as_logic();
            break;
        case SERVER_SAND:
            this->stop_as_sandbox();
            break;
        case SERVER_MAP:
            this->stop_as_map();
            break;
        case SERVER_CHAT:
            this->stop_as_chat();
            break;
        case SERVER_AUTH:
            this->stop_as_auth();
            break;
        case SERVER_GATE:
            this->stop_as_gate();
            break;
        default:
            this->stop_as_daemon();
            break;
    }
    TRANSACTION_MONITOR->stop();

    this->watcher_->end_loop();

    TRANSACTION_MONITOR->fina();

    switch (this->server_type_)
    {
        case SERVER_LOG:
            this->fina_as_log();
            break;
        case SERVER_LOGIC:
            this->fina_as_logic();
            break;
        case SERVER_SAND:
            this->fina_as_sanbox();
            break;
        case SERVER_MAP:
            this->fina_as_map();
            break;
        case SERVER_CHAT:
            this->fina_as_chat();
            break;
        case SERVER_AUTH:
            this->fina_as_auth();
            break;
        case SERVER_GATE:
            this->fina_as_gate();
            break;
        default:
            break;
    }

    ::google::protobuf::ShutdownProtobufLibrary();
    GameConfigSingle::destroy();
    POOL_MONITOR->fina();
    PoolMonitorSingle::destroy();

//    SAFE_DELETE(this->watcher_);

    LogClientMonitorSingle::destroy();
    TransactionMonitorSingle::destroy();
    Lib_Log::destroy();
    Log::destroy();

    exit(0);
}

int DaemonServer::stop_as_daemon(void)
{
    return 0;
}

int DaemonServer::stop_as_log(void)
{
    LOG_SERVER_MONITOR->stop();
    while (LOG_SERVER_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

int DaemonServer::stop_as_logic(void)
{
    LOGIC_MONITOR->stop();
    while (LOGIC_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

int DaemonServer::stop_as_sandbox(void)
{
    SAND_MONITOR->stop();

    return 0;
}

int DaemonServer::stop_as_map(void)
{
    MAP_MONITOR->stop();
    while (MAP_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

int DaemonServer::stop_as_chat(void)
{
    CHAT_MONITOR->stop();
    while (CHAT_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

int DaemonServer::stop_as_auth(void)
{
    AUTH_MONITOR->stop();
    while (AUTH_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

int DaemonServer::stop_as_gate(void)
{
    GATE_MONITOR->stop();
    while (GATE_MONITOR->is_running() == true)
        ::sleep(1);

    return 0;
}

void DaemonServer::fina_as_log(void)
{
    LOG_SERVER_MONITOR->fina();
    LogServerMonitorSingle::destroy();
}

void DaemonServer::fina_as_logic(void)
{
    LOGIC_MONITOR->fina();
    LogicMonitorSingle::destroy();
}

void DaemonServer::fina_as_sanbox(void)
{

}

void DaemonServer::fina_as_map(void)
{
    MAP_MONITOR->fina();
    MapMonitorSingle::destroy();
}

void DaemonServer::fina_as_chat(void)
{
    CHAT_MONITOR->fina();
    ChatMonitorSingle::destroy();
}

void DaemonServer::fina_as_auth(void)
{
    AUTH_MONITOR->fina();
    AuthMonitorSingle::destroy();
}

void DaemonServer::fina_as_gate(void)
{
    GATE_MONITOR->fina();
    GateMonitorSingle::destroy();
}

void DaemonServer::report_pool_info(void)
{
    switch (this->server_type_)
    {
        case SERVER_LOGIC:
            LOGIC_MONITOR->report_pool_info();
            break;
        case SERVER_MAP:
            MAP_MONITOR->report_pool_info();
            break;
        case SERVER_CHAT:
            CHAT_MONITOR->report_pool_info();
            break;
        case SERVER_GATE:
            GATE_MONITOR->report_pool_info();
            break;
        default:
            break;
    }
}

int DaemonServer::force_restart_process(const int index)
{
//    if (this->child_core_map_[index] <= 10)
//    {
//        std::cout << "ERROR child core times " << this->child_core_map_[index] << std::endl;
//        return -1;
//    }

    std::cout << "child force restart " << index << std::endl;

    if (index >= int(this->child_list_.size()))
        return 0;

    int pid = this->child_list_[index];
    char pid_path[128];
    ::sprintf(pid_path, "/proc/%d", pid);
    int file_exsist = ::access(pid_path, F_OK);
    if (file_exsist != 0 || this->check_process_state(pid) == P_STATUS_DEATH)
    {
        if (file_exsist == 0)
            ::waitpid(pid, 0, 0);

        std::cout << index << " restart process in force" << pid << std::endl;

        this->restart_process(index, pid, true);
    }
    return 0;    
}

int DaemonServer::request_update_config(void)
{
    Json::Value json;
    if (CONFIG_INSTANCE->load_update_config(json) != 0)
    {
        std::cout << "ERROR no update_config.json" << std::endl;
        return -1;
    }
    std::string folder = json["folder"].asString();
    int server_type = this->server_type();
    if (folder != "server")
    {
        if (server_type == SERVER_DAEMON || server_type == SERVER_AUTH)
            return 0;
    }
    else
    {
        if (server_type != SERVER_AUTH && server_type != SERVER_DAEMON)
            return 0;
    }

    MSG_USER("begin update config %s", folder.c_str());
    if (CONFIG_INSTANCE->update_config(folder) != 0)
    {
        MSG_USER("ERROR update config %s", folder.c_str());
    }
    else
    {
        if (folder == "server")
        {
            if (this->server_type() == SERVER_DAEMON)
            {
                this->check_update_child_list();
                //this->fork_server_list();
            }
            if (this->server_type() == SERVER_AUTH)
            {
                AUTH_MONITOR->update_server_config();
            }
        }

        MSG_USER("finish update config %s", folder.c_str());
    }

    return 0;
}

int DaemonServer::check_log_can_stop(void)
{
    return LOG_SERVER_MONITOR->check_log_can_stop();
}

const char *DaemonServer::parent_path(void)
{
    return this->pppath_;
}

void DaemonServer::set_check_child_start(const bool flag)
{
    this->check_child_start_ = flag;
}

int DaemonServer::check_process_state(const int pid)
{
    FILE *ptr;  
    char buff[512];  
    char ps[128];
    int status = P_STATUS_RUNNING;
    sprintf(ps,"ps aux | grep \"\\<%d\\>\" | grep -v grep | awk '{print $8}'", pid);
    std::cout << ps << std::endl;
    strcpy(buff,"ABNORMAL");  
    if ((ptr=popen(ps, "r")) != NULL)  
    {  
        if (fgets(buff, 512, ptr) != NULL)  
        {
            std::cout << "process " << pid << " status " << buff << std::endl;
            if (buff[0] == 'Z')
            {
                status = P_STATUS_DEATH;
            }
        }  
    }  
    pclose(ptr);  
    return status;  
}

void DaemonServer::check_child_need_restart(void)
{
    if (this->check_child_start_ == false)
        return;

    if (this->is_server_daemon() == false)
        return;

    bool is_has_process_start = false;
    char pid_path[128];
    int i = 0;
    for (IntVec::iterator iter = this->child_list_.begin(); iter != this->child_list_.end(); ++iter)
    {
        ::sprintf(pid_path, "/proc/%d", *iter);
        int file_exsist = ::access(pid_path, F_OK);
        if (file_exsist != 0 || this->check_process_state(*iter) == P_STATUS_DEATH)
        {
            if (file_exsist == 0)
                ::waitpid(*iter, 0, 0);

            std::cout << i << "restart process in check " << *iter << std::endl;
            this->restart_process(i, *iter);
            is_has_process_start = true;
        }
        ++i;
    }

    if (is_has_process_start == false)
    {
        this->set_check_child_start(false);
    }
}

void DaemonServer::check_update_child_list(void)
{
    this->fork_server_list();
}

