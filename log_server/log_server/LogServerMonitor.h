/*
 * LogServerMonitor.h
 *
 * Created on: 2013-01-10 14:43
 *     Author: glendy
 */

#ifndef _LOGSERVERMONITOR_H_
#define _LOGSERVERMONITOR_H_

#include "GameDefine.h"
#include "Singleton.h"
#include "ObjectPoolEx.h"
#include "Svc_Static_List.h"
#include "LogServerCommunicate.h"
#include "LogServerUnit.h"
#include <vector>
#include <map>
#include "SqlTableCache.h"

class UnitMessage;

class LogServerMonitor
{
public:
    typedef ObjectPoolEx<UnitMessage> UnitMessagePool;
    typedef ObjectPoolEx<LogServerService> LogServerServicePool;
    typedef ObjectPoolEx<SqlTableCache> SqlTableCachePool;

    typedef Svc_Static_List<LogServerService *, Thread_Mutex> LogServerServiceList;

    typedef std::vector<MysqlUnit> MysqlUnitList;
    typedef std::map<int, FILE *> ConfigToFileMap;
    typedef std::map<int, int32_t> ConfigToTimeMap;

public:
    int init(const int mysql_thr = 1);
    int start(void);
    int stop(void);
    void fina(void);
    bool is_running(void);

    LogServerAcceptor &server_acceptor(void);
    LogServerReceiver &server_receiver(void);
    LogServerPacker &server_packer(void);

    LogUnit &log_unit(void);
    MysqlUnit &mysql_unit(void);

    Block_Buffer *pop_block(int cid = 0);
    int push_block(Block_Buffer *buff, int cid = 0);

    UnitMessagePool *unit_msg_pool(void);
    LogServerServicePool *server_service_pool(void);
    SqlTableCachePool *sql_cache_pool(void);

    int bind_server_service(LogServerService *svc);
    int unbind_server_service(const int sid);
    int find_server_service(const int sid, LogServerService *&svc);

    void set_server_config_index(const int index);

    int logging(::google::protobuf::Message *msg_proto);

    int check_log_can_stop(void);

protected:
    std::string make_log_dir(const int config_index);
    std::string make_file_path(const std::string &path, const int time);

protected:
    LogServerAcceptor server_acceptor_;
    LogServerReceiver server_receiver_;
    LogServerPacker server_packer_;

    LogUnit log_unit_;
    int mysql_thr_;
    MysqlUnit *mysql_unit_list_;

    LogServerServicePool log_server_service_pool_;
    LogServerServiceList log_server_service_list_;
    int log_service_size_;

    int server_config_index_;
    int mysql_unit_index_;

    ConfigToFileMap config_to_logfile_map_;
    ConfigToTimeMap config_to_logtime_map_;

    SqlTableCachePool sql_cache_pool_;
};

typedef Singleton<LogServerMonitor> LogServerMonitorSingle;
#define LOG_SERVER_MONITOR  (LogServerMonitorSingle::instance())

#endif //_LOGSERVERMONITOR_H_
