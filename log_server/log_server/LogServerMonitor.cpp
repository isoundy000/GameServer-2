/*
 * LogServerMonitor.cpp
 *
 * Created on: 2013-01-10 15:06
 *     Author: glendy
 */

#include "LogServerMonitor.h"
#include "PoolMonitor.h"
#include "GameConfig.h"
#include "LogStruct.h"
#include "Date_Time.h"
#include <sstream>
#include "DaemonServer.h"
#include "ProtoDefine.h"
#include <sys/stat.h>

int LogServerMonitor::init(const int mysql_thr)
{
    this->server_acceptor_.init();
    this->server_receiver_.init();
    this->server_config_index_ = -1;
    this->mysql_unit_index_ = 0;
    this->mysql_thr_ = mysql_thr;
    if (this->mysql_thr_ <= 0)
    	this->mysql_thr_ = 1;
    this->mysql_unit_list_ = new MysqlUnit[mysql_thr];

    this->log_service_size_ = 0;

    return 0;
}

int LogServerMonitor::start(void)
{
    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][this->server_config_index_];
    int inner_port = server_json["inner_port"].asInt();

    const Json::Value &machine_json = CONFIG_INSTANCE->global()["mysql_machine"];
    int mysql_is_on = server_json["mysql_on"].asInt(),
        mysql_port = machine_json["port"].asInt();
    std::string address = machine_json["host"].asString();
    std::string usr = machine_json["usr"].asString();
    std::string passwd = machine_json["pwd"].asString();

    this->server_acceptor_.set(inner_port);

    this->log_unit_.thr_create();
    for (int i = 0; i < this->mysql_thr_; ++i)
    {
        this->mysql_unit_list_[i].mysql_connector().init(address, mysql_port,
                usr, passwd, (mysql_is_on == 1));
        this->mysql_unit_list_[i].thr_create();
    }
    this->server_packer_.thr_create();
    this->server_receiver_.thr_create();
    this->server_acceptor_.thr_create();

    MSG_USER("start log server");

    return 0;
}

int LogServerMonitor::stop(void)
{
    for (int i = 0; i < this->mysql_thr_; ++i)
        this->mysql_unit_list_[i].stop_wait();
    this->log_unit_.stop_wait();
    
    this->server_acceptor_.thr_cancel_join();
    this->server_receiver_.thr_cancel_join();
    this->server_packer_.thr_cancel_join();
    return 0;
}

void LogServerMonitor::fina(void)
{
    this->server_acceptor_.fini();
    this->server_receiver_.fini();
    
    delete [] this->mysql_unit_list_;

    this->config_to_logfile_map_.clear();
    this->config_to_logtime_map_.clear();

    std::vector<LogServerService *> used_list = this->log_server_service_list_.erase_all_used();
    for (std::vector<LogServerService *>::iterator iter = used_list.begin();
            iter != used_list.end(); ++iter)
    {
        (*iter)->close_fd();
        this->log_server_service_pool_.push(*iter);
    }

    this->log_server_service_pool_.clear();
    this->sql_cache_pool_.clear();
}

bool LogServerMonitor::is_running(void)
{
    for (int i = 0; i < this->mysql_thr_; ++i)
    {
        if (this->mysql_unit_list_[i].is_running() == true)
            return true;
    }
    if (this->log_unit_.is_running() == true)
        return true;

    return false;
}

LogServerAcceptor &LogServerMonitor::server_acceptor(void)
{
    return this->server_acceptor_;
}

LogServerReceiver &LogServerMonitor::server_receiver(void)
{
    return this->server_receiver_;
}

LogServerPacker &LogServerMonitor::server_packer(void)
{
    return this->server_packer_;
}

LogUnit &LogServerMonitor::log_unit(void)
{
    return this->log_unit_;
}

MysqlUnit &LogServerMonitor::mysql_unit(void)
{
    int index = this->mysql_unit_index_;
    this->mysql_unit_index_ = (++this->mysql_unit_index_) % this->mysql_thr_;

    index = index % this->mysql_thr_;
    return this->mysql_unit_list_[index];
}

Block_Buffer *LogServerMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int LogServerMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

LogServerMonitor::UnitMessagePool *LogServerMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

LogServerMonitor::LogServerServicePool *LogServerMonitor::server_service_pool(void)
{
    return &(this->log_server_service_pool_);
}

LogServerMonitor::SqlTableCachePool *LogServerMonitor::sql_cache_pool(void)
{
    return &(this->sql_cache_pool_);
}

int LogServerMonitor::bind_server_service(LogServerService *svc)
{
    int sid = this->log_server_service_list_.record_svc(svc);
    if (sid == -1)
    {
        Lib_Log::instance()->msg_user("cid == -1");
        return -1;
    }
    
    svc->set_cid(sid);
    ++this->log_service_size_;
    return 0;
}

int LogServerMonitor::unbind_server_service(const int sid)
{
    int ret = this->log_server_service_list_.erase_svc(sid);
    if (ret == 0)
    	--this->log_service_size_;
    return ret;
}

int LogServerMonitor::find_server_service(const int sid, LogServerService *&svc)
{
    if (this->log_server_service_list_.get_used_svc(sid, svc) == true)
        return 0;
    return -1;
}

void LogServerMonitor::set_server_config_index(const int index)
{
    this->server_config_index_ = index;
}

int LogServerMonitor::logging(::google::protobuf::Message *msg_proto)
{
    if (msg_proto == 0)
        return -1;

    int32_t config_index = 0, log_sub_type = 0, pid, time;

    Proto30310001 *log_info = dynamic_cast<Proto30310001 *>(msg_proto);
    config_index = log_info->log_type();
    log_sub_type = log_info->log_sub_type();
    pid = log_info->pid();
    time = log_info->nowtime();
    const std::string &log_text = log_info->log_text();

    time_t log_tick = convert_to_whole_hour(Time_Value(time));
    ConfigToTimeMap::iterator iter = this->config_to_logtime_map_.find(config_index);
    FILE *log_fp = 0;
    if (iter == this->config_to_logtime_map_.end() ||
            iter->second != log_tick)
    {
        this->config_to_logtime_map_[config_index] = log_tick;

        std::string log_path = this->make_log_dir(config_index);
        std::string file_path = this->make_file_path(log_path, log_tick);
        if ((log_fp = ::fopen(file_path.c_str(), "a")) == 0)
        {
            Lib_Log::instance()->msg_user("can't open log file %s", file_path.c_str());
            return -1;
        }

        FILE *old_fp = 0;
        ConfigToFileMap::iterator file_iter = this->config_to_logfile_map_.find(config_index);
        if (file_iter == this->config_to_logfile_map_.end())
        {
            this->config_to_logfile_map_[config_index] = log_fp;
        }
        else
        {
            old_fp = file_iter->second;
            ::fclose(old_fp);
            file_iter->second = log_fp;
        }
    }
    else
    {
        log_fp = this->config_to_logfile_map_[config_index];
    }
    if (log_fp == 0)
        return -1;

    ::fputs(log_text.c_str(), log_fp);
    ::fflush(log_fp);
    return 0;
}

std::string LogServerMonitor::make_log_dir(const int config_index)
{
    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][config_index];
    if (server_json == Json::nullValue)
        return std::string();
    
    std::string path;
    path.append("./log/").append(server_json["service"].asString());
    char sz_config_index[64];
    ::sprintf(sz_config_index, "_%d", config_index);
    path.append(sz_config_index); 

    ::mkdir("./log", 0777);
    ::mkdir(path.c_str(), 0777);
    return path;
}

std::string LogServerMonitor::make_file_path(const std::string &path, const int time)
{
	Time_Value time_tick(time);
    Date_Time time_date(time_tick);
    char sz_file_name[64];
    ::sprintf(sz_file_name, "/%04ld-%02ld-%02ld-%02ld.log", 
            time_date.year(),
            time_date.month(),
            time_date.day(),
            time_date.hour());

    return std::string(path).append(sz_file_name);
}

int LogServerMonitor::check_log_can_stop(void)
{
	if (this->log_service_size_ > 0)
	{
		LOG_USER_INFO("self log %d %d\n", this->log_service_size_, DAEMON_SERVER->server_index());
		return -1;
	}
	return 0;
}

