/*
 * DebugServer.cpp
 *
 * Created on: 2013-01-21 16:19
 *     Author: glendy
 */

#include <sys/wait.h>
#include <google/protobuf/message.h>
#include "DebugServer.h"
#include "Epoll_Watcher.h"
#include "GameConfig.h"
#include "LogicMonitor.h"
#include "MapMonitor.h"
#include "ChatMonitor.h"
#include "AuthMonitor.h"
#include "GateMonitor.h"
#include "LogServerMonitor.h"
#include "LogClientMonitor.h"
#include "Log.h"
#include "TransactionMonitor.h"
#include "MongoConnector.h"
#ifdef LUADEBUG
#include "LuaDebug.h"
#endif

#define SERVICE_NAME_LOG    "log"
#define SERVICE_NAME_LOGIC  "logic"
#define SERVICE_NAME_SAND   "sand"
#define SERVICE_NAME_MAP    "map"
#define SERVICE_NAME_CHAT   "chat"
#define SERVICE_NAME_AUTH   "auth"
#define SERVICE_NAME_GATE   "gate"

int DebugServer::TimerHandler::handle_timeout(const Time_Value &tv)
{
//	GATE_MONITOR->report_pool_info();
//    LOGIC_MONITOR->report_pool_info(true);
//    MAP_MONITOR->report_pool_info(true);

    return 0;
}

DebugServer::DebugServer(void) : watcher_(0), is_stopped_(false)
{ /*NULL*/ }

DebugServer::~DebugServer(void)
{
	SAFE_DELETE(this->watcher_);
}

int DebugServer::init(void)
{
    this->is_stopped_ = true;
    this->watcher_ = new Epoll_Watcher();
    return 0;
}

int DebugServer::start(void)
{
	this->is_stopped_ = false;
    CONFIG_INSTANCE->init(SERVICE_NAME_DEBUG);

    signal(SIGPIPE, SIG_IGN);
    ::signal(SIG_UPDATE_CONFIG, sigcld_handle);
    ::signal(SIGINT,sigcld_handle);
    ::signal(34, sigcld_handle);

    TRANSACTION_MONITOR->init(SERVER_LOGIC, 1);
    TRANSACTION_MONITOR->start();

    std::string service_name;
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];
    for (uint i = 0; i < servers_json.size(); ++i)
    {
        service_name = servers_json[i]["service"].asString();
        if (service_name == SERVICE_NAME_LOGIC)
        {
            this->start_as_logic(i);
            LOG_CLIENT_MONITOR->init();
            LOG_CLIENT_MONITOR->set_server_config_index(int(i));
            LOG_CLIENT_MONITOR->start();
#ifdef VALGRIND
            ::sleep(10);
#endif
        }
        else if (service_name == SERVICE_NAME_MAP)
        {
            this->start_as_map(i);
#ifdef VALGRIND
            ::sleep(10);
#endif
        }
        else if (service_name == SERVICE_NAME_CHAT)
        {
            this->start_as_chat(i);
#ifdef VALGRIND
            ::sleep(10);
#endif
        }
        else if (service_name == SERVICE_NAME_AUTH)
            this->start_as_auth(i);
        else if (service_name == SERVICE_NAME_GATE)
        {
            this->start_as_gate(i);
#ifdef VALGRIND
            ::sleep(10);
#endif
        }
        else if (service_name == SERVICE_NAME_LOG)
        {
        	this->start_as_log(i);
        }
    }

    TRANSACTION_MONITOR->start_timer();

#ifdef LUADEBUG
    LUA_DEBUG->start("config/script");
#endif

    Time_Value timeout_tv(10, 0);
    this->watcher_->add(&(this->timer_handler_), Epoll_Watcher::EVENT_TIMEOUT, &timeout_tv);

    return this->watcher_->loop();
}

int DebugServer::stop(void)
{
	if (this->is_stopped_ == true)
    {
        MSG_USER("direct exit server");
		exit(-1);
    }

    MSG_USER("stop");
    this->is_stopped_ = true;

    this->stop_as_logic();
    this->stop_as_map();
    this->stop_as_chat();
    this->stop_as_auth();
    this->stop_as_gate();
#ifdef TEST_SERIAL
	this->stop_as_log();
#endif
    TRANSACTION_MONITOR->stop();
    TRANSACTION_MONITOR->fina();

    ::google::protobuf::ShutdownProtobufLibrary();
    GameConfigSingle::destroy();
    POOL_MONITOR->fina();
    PoolMonitorSingle::destroy();
    
    LogClientMonitorSingle::destroy();
    TransactionMonitorSingle::destroy();
    Lib_Log::destroy();
    Log::destroy();

    this->watcher_->end_loop();
    return 0;
}

int DebugServer::start_as_logic(const int index)
{
	LogicMonitorSingle::instance()->init();
    LOGIC_MONITOR->set_server_config_index(index);
    LOGIC_MONITOR->start();
    return 0;
}

int DebugServer::start_as_map(const int index)
{
	MapMonitorSingle::instance()->init(index);
    MAP_MONITOR->set_server_config_index(index);
    MAP_MONITOR->start();
    return 0;
}

int DebugServer::start_as_chat(const int index)
{
    CHAT_MONITOR->init();
    CHAT_MONITOR->set_server_config_index(index);
    CHAT_MONITOR->start();
    return 0;
}

int DebugServer::start_as_auth(const int index)
{
    AUTH_MONITOR->init();
    AUTH_MONITOR->set_server_config_index(index);
    AUTH_MONITOR->start();
    return 0;
}

int DebugServer::start_as_gate(const int index)
{
    GATE_MONITOR->init(index);
    GATE_MONITOR->set_server_config_index(index);
    GATE_MONITOR->start();
    return 0;
}

int DebugServer::start_as_log(const int index)
{
#ifdef TEST_SERIAL
    LOG_SERVER_MONITOR->init(1);
    LOG_SERVER_MONITOR->set_server_config_index(index);
    LOG_SERVER_MONITOR->start();
#endif

    return 0;
}

int DebugServer::stop_as_logic(void)
{
	MSG_USER("stop logic");
    LOGIC_MONITOR->stop();
    while (LOGIC_MONITOR->is_running() == true)
        ::sleep(1);

    LOGIC_MONITOR->fina();
    LogicMonitorSingle::destroy();
    return 0;
}

int DebugServer::stop_as_map(void)
{
	MSG_USER("stop map");
    MAP_MONITOR->stop();
    while (MAP_MONITOR->is_running() == true)
        ::sleep(1);

    MAP_MONITOR->fina();
    MapMonitorSingle::destroy();
    return 0;
}

int DebugServer::stop_as_chat(void)
{
	MSG_USER("stop chat");
    CHAT_MONITOR->stop();
    while (CHAT_MONITOR->is_running() == true)
        ::sleep(1);

    CHAT_MONITOR->fina();
    ChatMonitorSingle::destroy();
    return 0;
}

int DebugServer::stop_as_auth(void)
{
	MSG_USER("stop auth");
    AUTH_MONITOR->stop();
    while (AUTH_MONITOR->is_running() == true)
        ::sleep(1);

    AUTH_MONITOR->fina();
    AuthMonitorSingle::destroy();
    return 0;
}

int DebugServer::stop_as_gate(void)
{
	MSG_USER("stop gate");
    GATE_MONITOR->stop();
    while (GATE_MONITOR->is_running() == true)
        ::sleep(1);

    GATE_MONITOR->fina();
    GateMonitorSingle::destroy();
    return 0;
}

int DebugServer::stop_as_log(void)
{
	MSG_USER("stop log");
    LOG_SERVER_MONITOR->stop();
    while (LOG_SERVER_MONITOR->is_running() == true)
        ::sleep(1);

    LOG_SERVER_MONITOR->fina();
    LogServerMonitorSingle::destroy();
    return 0;
}

void DebugServer::sigcld_handle(int signo)
{
    if (signo == SIG_UPDATE_CONFIG)
    {
//        UnitMessage msg;
//        msg.reset();
//        msg.__sid = 0;
//        msg.__msg_head.__recogn = INNER_UPDATE_CONFIG;
//        msg.__type = UnitMessage::TYPE_BLOCK_BUFF;
//        msg.__data.__buf = 0;
//        LOGIC_MONITOR->logic_unit()->push_request(msg);
    	DEBUG_SERVER->request_update_config();
        return;
    }

    MSG_USER("catch signno %d", signo);
    DEBUG_SERVER->stop();
}

int DebugServer::request_update_config(void)
{
    Json::Value json;
    if (CONFIG_INSTANCE->load_update_config(json) != 0)
    {
        std::cout << "ERROR no update_config.json" << std::endl;
        return -1;
    }
    std::string folder = json["folder"].asString();
    MSG_USER("begin update config %s", folder.c_str());
    if (CONFIG_INSTANCE->update_config(folder) != 0)
        MSG_USER("ERROR update config %s", folder.c_str());
    else
        MSG_USER("finish update config %s", folder.c_str());

    if (folder == "server")
    	AUTH_MONITOR->update_server_config();

    return 0;
}

