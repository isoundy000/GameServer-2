/*
 * LogClientMonitor.cpp
 *
 * Created on: 2013-01-09 14:56
 *     Author: glendy
 */

#include "LogClientMonitor.h"
#include "Log.h"
#include "GameConfig.h"
#include "ProtoInner001.pb.h"
#include "MapCommunicate.h"
#include "LogicCommunicate.h"
#include "LogicMonitor.h"
#include "MapMonitor.h"
#include "ChatMonitor.h"
#include "AuthMonitor.h"
#include "GateMonitor.h"
#include "DaemonServer.h"
#include "LogServerMonitor.h"
#include <sstream>

int LogClientMonitor::init(void)
{
    this->server_config_index_ = -1;
    this->server_type_ = 0;
    this->log_scene_ = 0;
    this->log_sid_ = -1;
    return 0;
}

int LogClientMonitor::start(void)
{
    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][this->server_config_index_];
    Log::instance()->set_log_type(this->server_config_index_);

    int switcher = 0;
    const std::string &log_switcher = CONFIG_INSTANCE->global()["log"].asString();
    for (size_t i = 0; i < log_switcher.length(); ++i)
    {
        switcher = switcher << 1;
        if (log_switcher[i] == '1')
            switcher += 1;
    }
    Log::instance()->set_switcher(switcher);

    const std::string &server_name = server_json["service"].asString();
    if (server_name == "logic")
        this->server_type_ = SERVER_LOGIC;
    else if (server_name == "chat")
        this->server_type_ = SERVER_CHAT;
    else if (server_name == "log")
        this->server_type_ = SERVER_LOG;
    else if (server_name == "sand")
        this->server_type_ = SERVER_SAND;
    else if (server_name == "map")
        this->server_type_ = SERVER_MAP;
    else if (server_name == "auth")
        this->server_type_ = SERVER_AUTH;
    else if (server_name == "gate")
        this->server_type_ = SERVER_GATE;
    else
        return -1;

   const Json::Value &connect_scene_json = server_json["connect_scene"];
   for (uint i = 0; i < connect_scene_json.size(); ++i)
   {
       if (connect_scene_json[i].isInt() == false)
           continue;
       if ((connect_scene_json[i].asInt() / 100) != SERVER_LOG)
           continue;

       this->log_scene_ = connect_scene_json[i].asInt();
       break;
   }

   return 0;
}

int LogClientMonitor::stop(void)
{
    return 0;
}

void LogClientMonitor::set_server_config_index(const int index)
{
    this->server_config_index_ = index;
}

int LogClientMonitor::log_scene(void)
{
    return this->log_scene_;
}

int LogClientMonitor::log_sid(void) {
	switch (this->server_type_) {
		case SERVER_LOGIC:
			this->log_sid_ = LOGIC_MONITOR->fetch_sid_of_scene(this->log_scene(), true);
			return this->log_sid_;
		case SERVER_MAP:
			this->log_sid_ = MAP_MONITOR->fetch_sid_of_scene(this->log_scene(), true);
			return this->log_sid_;
		case SERVER_CHAT:
			this->log_sid_ = CHAT_MONITOR->fetch_sid_of_scene(this->log_scene(), true);
			return this->log_sid_;
		case SERVER_AUTH:
			this->log_sid_ = AUTH_MONITOR->fetch_sid_of_scene(this->log_scene(), true);
			return this->log_sid_;
		case SERVER_GATE:
			this->log_sid_ = GATE_MONITOR->fetch_sid_of_scene(this->log_scene(), true);
			return this->log_sid_;
		default:
		    return -1;
	}
	return -1;
}

void LogClientMonitor::set_log_sid(const int sid)
{
	this->log_sid_ = sid;
}

int LogClientMonitor::get_log_sid(void)
{
	return this->log_sid_;
}

int LogClientMonitor::push_data_block(Block_Buffer *buff)
{
    switch (this->server_type_)
    {
        case SERVER_LOGIC:
            return LOGIC_MONITOR->connect_sender()->push_pool_block_with_len(buff);
        case SERVER_MAP:
            return MAP_MONITOR->connect_sender()->push_pool_block_with_len(buff);
        case SERVER_CHAT:
            return CHAT_MONITOR->connect_sender()->push_pool_block_with_len(buff);
        case SERVER_AUTH:
            return AUTH_MONITOR->connect_sender()->push_pool_block_with_len(buff);
        case SERVER_GATE:
            return GATE_MONITOR->connect_sender()->push_pool_block_with_len(buff);
        default:
            return -1;
    }
    return 0;
}

int LogClientMonitor::logging_in_log_server(std::ostringstream &msg_stream)
{
    UnitMessage msg;
    msg.reset();

    msg.reset();
    msg.__type = UnitMessage::TYPE_PROTO_MSG;
    msg.__sid = 0;
    msg.__msg_head.__recogn = INNER_LOG_WRITE;

    int32_t pid = Log::instance()->pid(),/* tid = ::pthread_self(),*/ nowtime = ::time(0);
    Proto30310001 *log_info = new Proto30310001();
    log_info->set_log_type(DAEMON_SERVER->server_index());
    log_info->set_log_sub_type(0);
    log_info->set_pid(pid);
    log_info->set_nowtime(nowtime);
    log_info->set_log_text(msg_stream.str().c_str());

    msg.__len = sizeof(ProtoHead) + log_info->ByteSize();

    msg.__data.__proto_msg = log_info;
    if (LOG_SERVER_MONITOR->log_unit().push_request(msg) != 0)
    {
        delete log_info;
        return -1;
    }
    return 0;
}

//LogClientConnector &LogClientMonitor::client_connector(void)
//{
//    return this->client_connector_;
//}
//
//LogClientSender &LogClientMonitor::client_sender(void)
//{
//    return this->client_sender_;
//}
//
//Block_Buffer *LogClientMonitor::pop_block(int cid)
//{
//    return POOL_MONITOR->pop_buf_block(cid);
//}
//
//int LogClientMonitor::push_block(Block_Buffer *buff, int cid)
//{
//    return POOL_MONITOR->push_buf_block(buff, cid);
//}
//
//LogClientMonitor::ClientServicePool *LogClientMonitor::client_service_pool(void)
//{
//    return &(this->client_service_pool_);
//}
//
//int LogClientMonitor::bind_client_service(LogClientService *svc)
//{
//    int sid = this->client_service_list_.record_svc(svc);
//    if (sid == -1)
//        return -1;
//
//    svc->set_cid(sid);
//    return 0;
//}
//
//int LogClientMonitor::unbind_client_service(const int sid)
//{
//    return this->client_service_list_.erase_svc(sid);
//}
//
//int LogClientMonitor::find_client_service(const int sid, LogClientService *&svc)
//{
//    if (this->client_service_list_.get_used_svc(sid, svc) == true)
//        return 0;
//    return -1;
//}

