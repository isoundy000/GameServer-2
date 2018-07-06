/*
 * GameConfig.cpp
 *
 * Created on: 2012-11-16 10:27
 *     Author: glendy
 */

#include "GameConfig.h"
#include "PubStruct.h"
#include "BTCFactory.h"
#include "GameCommon.h"

#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <zlib.h>
#include <dirent.h>
#include <netdb.h>
#include <fstream>
#include "MapStruct.h"
#include "ActivityTipsSystem.h"
#include "LogicMonitor.h"
#include "DaemonServer.h"
#include "LeagueSystem.h"
#include "MMOServerInfo.h"

GameConfig::BasicConfig::BasicConfig(void) :
	__cur_version(0)
{ /*NULL*/ }

Json::Value &GameConfig::BasicConfig::json(void)
{
    return this->__json[this->__cur_version];
}

GameConfig::ConfigMap &GameConfig::BasicConfig::map(void)
{
    return this->__map[this->__cur_version];
}

GameConfig::DoubleKeyConfigMap &GameConfig::BasicConfig::double_map(void)
{
	return this->__double_map[this->__cur_version];
}

Json::Value &GameConfig::BasicConfig::revert_json(void)
{
    return this->__json[((this->__cur_version + 1) % MAX_VERSION_CNT)];
}

GameConfig::ConfigMap &GameConfig::BasicConfig::revert_map(void)
{
    return this->__map[((this->__cur_version + 1) % MAX_VERSION_CNT)];
}

GameConfig::DoubleKeyConfigMap &GameConfig::BasicConfig::revert_double_map(void)
{
	return this->__double_map[((this->__cur_version + 1) % MAX_VERSION_CNT)];
}

const Json::Value& GameConfig::BasicConfig::find(int key)
{
   Json::Value *json = 0;
	if (this->map().find(key, json) == 0)
	{
		return *json;
	}
	else
	{
		return Json::Value::null;
	}
}

const Json::Value& GameConfig::BasicConfig::find_limit_max(int key)
{
	int adjust_key = std::min<int>(this->map().size(), key);
	return this->find(adjust_key);
}

Json::Value& GameConfig::BasicConfig::json_name(const char* item_name)
{
	if (item_name != NULL)
	{
		return this->json()[item_name];
	}
	else
	{
		return this->json();
	}
}

int GameConfig::BasicConfig::current_version(void)
{
    return this->__cur_version;
}

int GameConfig::BasicConfig::prev_version(void)
{
    return ((this->__cur_version + 1) % MAX_VERSION_CNT);
}

int GameConfig::BasicConfig::validate(int key)
{
	Json::Value *json = 0;
	return this->map().find(key, json) == 0;
}

void GameConfig::BasicConfig::update_version(void)
{
    this->__cur_version = (this->__cur_version + 1) % MAX_VERSION_CNT;

    Time_Value nowtime = Time_Value::gettimeofday();
    Date_Time now_date(nowtime);
    char version_no[64 + 1];
    ::snprintf(version_no, 64, "%04ld%02ld%02ld%02ld%02ld%02ld",
            now_date.year(), now_date.month(), now_date.day(),
            now_date.hour(), now_date.minute(), now_date.second());
    version_no[64] = '\0';
    this->__version_no[this->__cur_version] = version_no;
}

void GameConfig::BasicConfig::revert_version(void)
{
    this->__cur_version = (this->__cur_version + MAX_VERSION_CNT - 1) % MAX_VERSION_CNT;
}

void GameConfig::BasicConfig::convert_json_to_map(int key_flag)
{
	this->revert_map().unbind_all();
    for (Json::Value::iterator iter = this->revert_json().begin();
            iter != this->revert_json().end(); ++iter)
    {
        Json::Value* json = &(*iter);

        IntPair pair = GameCommon::to_int_number(iter.key().asString());
        JUDGE_CONTINUE(pair.first == true);

        if (key_flag == true)
        {
        	(*json)["id"] = pair.second;
        }

        this->revert_map().rebind(pair.second, json);
    }
}

void GameConfig::BasicConfig::load_combine_file(const string& path)
{
	ConfigMap& act_map = this->revert_map();
	for (ConfigMap::iterator iter = act_map.begin(); iter != act_map.end(); ++iter)
	{
		char path_file[1024] = {0};
		::sprintf(path_file, path.c_str(), iter->first);

		Json::Value& act_conf = *(iter->second);
		CONFIG_INSTANCE->combine_json_file_b(act_conf["reward"], path_file);
	}
}

string GameConfig::BasicConfig::version_no()
{
	return this->__version_no[this->__cur_version];
}

MapBlockDetail &GameConfig::SceneConfig::revert_map_block_detail(void)
{
    return this->__map_block_detail[this->__map_block.prev_version()];
}

GameConfig::MptDetailMap &GameConfig::SceneConfig::revert_mpt_detail_map(void)
{
    return this->__mpt[this->__scene.prev_version()];
}

GameConfig::MptMap &GameConfig::SceneConfig::revert_mpt_coord_map(void)
{
    return this->__mpt_coord_map[this->__scene.prev_version()];
}

GameConfig::CoordIndexListMap &GameConfig::SceneConfig::revert_move_coord_map(void)
{
    return this->__move_coord_map[this->__scene.prev_version()];
}

GameConfig::CoordIndexSetMap &GameConfig::SceneConfig::revert_border_coord_map(void)
{
    return this->__border_coord_map[this->__scene.prev_version()];
}

GameConfig::SceneSceneJumpMap &GameConfig::SceneConfig::revert_scene_scene_jump_map(void)
{
    return this->__scene_scene_jump_map[this->__scene.prev_version()];
}

MapBlockDetail &GameConfig::SceneConfig::map_block_detail(void)
{
    return this->__map_block_detail[this->__map_block.current_version()];
}

GameConfig::MptDetailMap &GameConfig::SceneConfig::mpt_detail_map(void)
{
    return this->__mpt[this->__scene.current_version()];
}

GameConfig::MptMap &GameConfig::SceneConfig::mpt_coord_map(void)
{
    return this->__mpt_coord_map[this->__scene.current_version()];
}

GameConfig::CoordIndexListMap &GameConfig::SceneConfig::move_coord_map(void)
{
    return this->__move_coord_map[this->__scene.current_version()];
}

GameConfig::CoordIndexSetMap &GameConfig::SceneConfig::border_coord_map(void)
{
    return this->__border_coord_map[this->__scene.current_version()];
}

GameConfig::SceneSceneJumpMap &GameConfig::SceneConfig::scene_scene_jump_map(void)
{
    return this->__scene_scene_jump_map[this->__scene.current_version()];
}

void GameConfig::SceneConfig::update_version(void)
{
    this->__map_block.update_version();
    this->__scene.update_version();
    this->__npc.update_version();
}

void GameConfig::SceneConfig::revert_version(void)
{
    this->__map_block.revert_version();
    this->__scene.revert_version();
    this->__npc.revert_version();
}

GameConfig::GameConfig(void) :
    is_map_server_(false)
{
	this->server_info_ = NULL;
	this->server_ip_ = "127.0.0.1";
}

GameConfig::~GameConfig(void)
{
	SAFE_DELETE(this->server_info_);
}

int GameConfig::init(const char *server_name)
{
	this->travel_config_.__is_updated_travel = false;

    this->load_server_config();
    this->load_map_config();
    this->load_fight_config();
    this->load_monster_config();

    this->load_item_config();
    this->load_equip_config();
    this->load_role_config();
    this->load_tiny_config();

    this->load_market_config();
    this->load_arena_config();
    this->load_task_config();
    this->load_league_config();
    this->load_fashion_config();
    this->load_sword_pool_config();
    this->load_transfer_config();
    this->load_mount_config();
    this->load_beast_config();

    this->load_relax_play_config();
    this->load_achieve_config();
    this->load_brocast_config();
    this->load_activity_config();
    this->load_welfare_config();
    this->load_limit_config();
    this->load_wedding_config();
    this->load_travel_config();
    this->load_world_boss_config();
    this->load_cornucopia_config();
    this->load_fashion_act_config();
    this->load_labour_act_config();
    this->load_offline_vip_config();
    this->load_molding_spirit_config();
    this->load_fish_type_config();
    this->load_goddess_bless_config();
    this->load_special_box_config();

    this->is_map_server_ = false;
    this->update_tick_ = Time_Value::gettimeofday() + Time_Value(5);

#ifndef LOCAL_DEBUG
    if (::strcmp(server_name, SERVICE_NAME_MAP) == 0)
#endif
    {
        this->is_map_server_ = true;
        // map server
        BTCFACTORY->init();
    }

    return 0;
}

const Json::Value &GameConfig::global()
{
    return this->server_config_.__global;
}

void GameConfig::load_server_ip()
{
#ifndef LOCAL_DEBUG
    std::ifstream in("../../common_cron/server_ip", std::ios::in);
    if (in.is_open() == false)
    {
    	MSG_USER("ERROR file open server ip");
    	return;
    }

    char buff[256] = {0};
    in.getline(buff, 255);

    this->server_ip_ = buff;
    in.close();

    MSG_USER("server ip %s", this->server_ip_.c_str());
#endif
}

void GameConfig::load_version_config()
{
	if (this->load_json_config("config/server/version.json",
			this->server_config_.__version.revert_json()) == 0)
	{
		this->server_config_.__version.convert_json_to_map();
		this->server_config_.__version.update_version();
	}
}

int GameConfig::load_json_config(const char *doc, Json::Value &conf)
{
    int file = ::open(doc, O_RDONLY);
    if (file < 0)
    {
        return -1;
    }

    struct stat statbuf;
    if (::fstat(file, &statbuf) < 0)
    {
        LOG_SYS("fstat");
        return -1;
    }

    void *src = 0;
    if ((src = ::mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) 
    {
        LOG_SYS("mmap");
        return -1;
    }

    int skip_src_size = statbuf.st_size;
    const unsigned char *skip_src = skipBOM((const unsigned char *)src, &skip_src_size);

    if (this->parse((const char *)skip_src, ((const char *)skip_src) + skip_src_size, conf) == false)
    {
        MSG_USER("Failt to parse file %s\nerror:%s\ncontext:%s",
                doc, this->getFormatedErrorMessages().c_str(), src);

        return -1;
    }

    if (::munmap(src, statbuf.st_size) < 0) 
    {
        LOG_SYS("munmap");
        return -1;
    }   

    ::close(file);

    return 0;
}

void GameConfig::update_open_server_date(void)
{
    int open_date = this->global()["open_date"].asInt();
    int open_time = this->global()["open_time"].asInt();

    int year 	= open_date / 10000;
    int month 	= (open_date % 10000) / 100;
    int day 	= open_date % 100;
    int hour 	= open_time / 10000;
    int minute 	= (open_time % 10000) / 100;
    int second 	= open_time % 100;

    Date_Time now_date(Time_Value::gettimeofday());
    now_date.hour(hour);
    now_date.minute(minute);
    now_date.second(second);
    now_date.day(day);
    now_date.month(month);
    now_date.year(year);

    this->server_config_.__open_sever.set_tick(now_date.time_sec());
}

void GameConfig::update_combine_server_date(void)
{
	int combine_date = 0;
	string server_flag = this->global()["server_flag"].asString();

	if (this->global().isMember("combine_date"))
	{
		combine_date = this->global()["combine_date"].asInt();
	}
	else if (GameCommon::is_combine_server_flag(server_flag) == true)
	{
		combine_date = this->global()["open_date"].asInt();
	}

	int year 	= combine_date / 10000;
	int month 	= (combine_date % 10000) / 100;
	int day 	= combine_date % 100;
	Date_Time now_date(day, month, year);
	this->server_config_.__combine_server.set_tick(now_date.time_sec());
}

int GameConfig::load_server_config(void)
{
#ifdef LOCAL_DEBUG
    {
        if (this->load_json_config("config/server/machine.json", this->server_config_.__global) != 0)
            return -1;
    }
#else
    {
        if (this->load_json_config("../server-machine/machine.json", this->server_config_.__global) != 0)
            return -1;
    }
#endif

    this->update_open_server_date();
    this->update_combine_server_date();
    this->init_server_info();
    this->init_combine_server_flag();
    this->load_servers_list();
    this->load_version_config();

    {
        char server_path[256];
        const Json::Value &special_json = this->server_config_.__global["special"];
        if (special_json != Json::nullValue)
        {
            ::sprintf(server_path, "config/server/special-%s.json", 
                    special_json.asCString());
        }
        else
        {
            ::sprintf(server_path, "config/server/server-%d.json",
                    int(this->server_config_.__global["machine_list"].size()));
        }

        Json::Value server;
        if (this->load_json_config(server_path, server) != 0)
        {
            return -1;
        }

        this->server_config_.__global["server_list"] = server["server_list"];
    }
    {
        Json::Value version;
        if (this->load_json_config("config/server/version.json", version) != 0)
        {
        	MSG_USER("ERROR load server/version.json");
            return -1;
        }

        this->server_config_.__global["version"] = version["version"];
        this->server_config_.__global["vs_no"] = version["vs_no"];
        this->server_config_.__global["svn_vs"] = version["svn_vs"]; 
    }

    this->server_config_.__server_list.clear();
    this->server_config_.__scene_convert_to_map.clear();
    this->server_config_.__convert_scene_to_map.clear();

    const Json::Value &server_list = this->server_config_.__global["server_list"];
    for (uint i = 0; i < server_list.size(); ++i)
    {
        const Json::Value &server_json = server_list[i];

        ServerDetail detail;
        detail.__index = i;
        detail.__is_travel = server_json["travel"].asInt();
        detail.__service_name = server_json["service"].asString();
        detail.__service_machine = server_json["machine"].asString();
        detail.__server_type = this->convert_server_type(detail.__service_name);
      	detail.__address = this->machine_address(detail.__service_machine, &detail.__domain,
      			detail.__is_travel);

        if (detail.__is_travel == 1)
        {
        	detail.__inner_port = this->travel_server_port(detail.__service_machine);
        }
        else if (detail.__is_travel == 2)
        {
        	detail.__inner_port = this->travel_server_port_channel(detail.__service_machine);
        }
        else
        {
            detail.__inner_port = server_json["inner_port"].asInt();
            detail.__outer_port = server_json["outer_port"].asInt();
        }

        for (uint j = 0; j < server_json["scene"].size(); ++j)
        {
        	int scene_id = server_json["scene"][j].asInt();
        	detail.__scene_list.insert(scene_id);

#ifdef LOCAL_DEBUG
            JUDGE_CONTINUE(GameCommon::is_travel_scene(scene_id) == true);
            detail.__is_travel = 1;
#endif
        }

        this->server_config_.__server_list.push_back(detail);
    }

    return 0;
}

string GameConfig::server_ip()
{
	return this->server_ip_;
}

string GameConfig::machine_address(void)
{
    std::string machine = this->global()["run_machine"].asString();
    return this->machine_address(machine);
}

string GameConfig::machine_address(const string &machine, string *domain, int travel_type)
{
    const Json::Value &machine_list = this->global()["machine_list"];
    for (uint i = 0 ; i < machine_list.size(); ++i)
    {
        JUDGE_CONTINUE(machine_list[i]["name"].asString() == machine);

        string local_address = machine_list[i]["host"].asString();
        string nic_name = "eth0";

        if (machine_list[i].isMember("domain") == true)
		{
			std::string domain_address = machine_list[i]["domain"].asString();
			if (domain != 0)
			{
				*domain = domain_address;
			}

			struct hostent *domain_ht = ::gethostbyname(domain_address.c_str());
			if (domain_ht == 0)
			{
				LOG_USER_INFO("domain [%s] error, use local ip[%s]", domain_address.c_str(), local_address.c_str());
				return local_address;
			}

			if (domain_ht->h_addr_list[0] != 0)
			{
				char buf[65];
				::memset(buf, 0, sizeof(buf));
				::inet_ntop(domain_ht->h_addrtype, domain_ht->h_addr_list[0], buf, sizeof(buf));
				return buf;
			}
		}
        else if (local_address == "127.0.0.1")
        {
            int sfd, intr;
            struct ifreq buf[16];
            struct ifconf ifc;
            sfd = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (sfd < 0)
                return std::string("127.0.0.1");

            ifc.ifc_len = sizeof(buf);
            ifc.ifc_buf = (caddr_t)buf;
            if (::ioctl(sfd, SIOCGIFCONF, (char *)&ifc))
                return std::string("127.0.0.1");
            intr = ifc.ifc_len / sizeof(struct ifreq);
            while (intr-- > 0)
            {
                if (buf[intr].ifr_name == nic_name)
                    break;
            }
            ::close(sfd);
            return ::inet_ntoa(((struct sockaddr_in*)(&buf[intr].ifr_addr))->sin_addr);
        }
        else
        {
        	return local_address;
        }
    }

    const Json::Value& conf = this->cur_servers_list();
    if (travel_type == 2)
    {
    	string agent_name = conf["agent_name"].asString();
    	const Json::Value& json = this->tiny_config_.agent_.json();
    	if (json.isMember(agent_name) == false)
    	{
    		return GameCommon::NullString;
    	}

    	if (json[agent_name].isMember(machine) == true)
    	{
    		return json[agent_name][machine][0u].asString();
    	}
    }
    else if (conf.isMember(machine) == true)
    {
		return conf[machine][0u].asString();
    }

    return GameCommon::NullString;
}

string GameConfig::full_role_name(int server_id, const string& name)
{
//	char full_name[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0};
//	std::snprintf(full_name, GameEnum::DEFAULT_MAX_NAME_LENGTH, "s%d. %s",
//			server_id, name.c_str());
//	return full_name;
	return name;
}

ServerInfo* GameConfig::server_info()
{
	return this->server_info_;
}

const ServerDetail& GameConfig::cur_map_server()
{
#ifdef LOCAL_DEBUG
	const ServerList& server_list = this->server_list();
	for (ServerList::const_iterator iter = server_list.begin();
			iter != server_list.end(); ++iter)
	{
		JUDGE_CONTINUE(iter->__server_type == SERVER_MAP);
		return *iter;
	}
#else
	const ServerDetail& server_detail = this->server_list(DAEMON_SERVER->server_index());
	if (server_detail.__server_type == SERVER_MAP)
	{
		return server_detail;
	}
#endif

	return this->server_config_.__null_server_detail;
}

const Json::Value& GameConfig::server(int index)
{
    return this->global()["server_list"][index];
}

const Json::Value& GameConfig::cur_server()
{
	return this->server(DAEMON_SERVER->server_index());
}

const Json::Value& GameConfig::cur_servers_list()
{
	string flag = this->server_flag();
	JUDGE_RETURN(this->travel_config_.__flag_index_map.count(flag) > 0, Json::Value::null);

	int index = this->travel_config_.__flag_index_map[flag];
	return this->travel_config_.__servers_list.find(index);
}

string GameConfig::main_version()
{
	return this->server_config_.__version.json()["main"].asString();
}

string GameConfig::date_version()
{
	return this->server_config_.__version.json()["version"].asString();
}

bool GameConfig::is_same_main_version(const string& main_version)
{
	if (this->server_config_.__version.json()["no_check"].asInt() == 1)
	{
		return true;
	}
	else
	{
		return this->main_version() == main_version;
	}
}

int GameConfig::travel_server_port(const string& machine)
{
    const Json::Value& conf =this->cur_servers_list();
    JUDGE_RETURN(conf.isMember(machine) == true, 0);
    return conf[machine][1u].asInt();
}

int GameConfig::travel_server_port_channel(const string& machine)
{
	const Json::Value& conf =this->cur_servers_list();
	string agent_name = conf["agent_name"].asString();

	const Json::Value& json = this->tiny_config_.agent_.json();
	if (json.isMember(agent_name) == false)
	{
		MSG_USER("no agent code: %s", agent_name.c_str());
		return 0;
	}

	if (json[agent_name].isMember(machine) == true)
	{
		return json[agent_name][machine][1u].asInt();
	}
	return 0;
}

int GameConfig::convert_server_type(const string &service_name)
{
    if (service_name == SERVICE_NAME_GATE)
        return SERVER_GATE;
    if (service_name == SERVICE_NAME_AUTH)
        return SERVER_AUTH;
    if (service_name == SERVICE_NAME_SAND)
        return SERVER_SAND;
    if (service_name == SERVICE_NAME_CHAT)
        return SERVER_CHAT;
    if (service_name == SERVICE_NAME_LOGIC)
        return SERVER_LOGIC;
    if (service_name == SERVICE_NAME_LOG)
        return SERVER_LOG;
    if (service_name == SERVICE_NAME_MAP)
        return SERVER_MAP;
    return -1;
}

GameConfig::ServerList &GameConfig::server_list(void)
{
    return this->server_config_.__server_list;
}

const ServerDetail &GameConfig::server_list(uint index)
{
    if (index < 0 || index >= this->server_config_.__server_list.size())
    {
        return this->server_config_.__null_server_detail;
    }

    return this->server_config_.__server_list[index];
}

bool GameConfig::is_convert_scene(const int scene)
{
	return this->server_config_.__convert_scene_to_map.count(scene) > 0;
}

int GameConfig::convert_scene_to(const int convert_scene)
{
    if (this->is_convert_scene(convert_scene) == false)
    {
        return convert_scene;
    }
    return this->server_config_.__convert_scene_to_map[convert_scene];
}

bool GameConfig::is_gate_scene(int scene_id)
{
	return (scene_id / 100) == SERVER_GATE;
}

const BIntSet &GameConfig::convert_scene_set(const int scene)
{
	IntSetMap::iterator iter = this->server_config_.__scene_convert_to_map.find(scene);
    if (iter == this->server_config_.__scene_convert_to_map.end())
    {
        return this->server_config_.__null_scene_set;
    }
    return iter->second;
}

Int64 GameConfig::open_day_tick(int days)
{
	return this->server_config_.__open_sever.create_day_tick() + days * Time_Value::DAY;
}

Int64 GameConfig::open_server_date(void)
{
    return this->server_config_.__open_sever.create_tick_;
}

int GameConfig::open_server_days(void)
{
	return this->server_config_.__open_sever.passed_days();
}

int GameConfig::open_server_hours(void)	//开服小时数
{
	Int64 open_time = this->server_config_.__open_sever.passed_time();
	return open_time / Time_Value::HOUR;
}

int GameConfig::left_open_activity_time(int adjust_time)
{
	static int open_days = this->const_set("open_activity_days");
#ifdef TEST_COMMAND
	Int64 open_end_tick = this->open_day_tick(open_days) - 1;
#else
	static Int64 open_end_tick = this->open_day_tick(open_days) - 1;
#endif
	return GameCommon::left_time(open_end_tick + adjust_time);
}

int GameConfig::during_open_activity(int day)
{
	JUDGE_RETURN(this->left_open_activity_time() > 0, false);
	return this->open_server_days() == day;
}

int GameConfig::main_activity_type()
{
	if (this->do_combine_server() == true)	// 是合服
	{
		if (this->left_combine_activity_time() > 0)
		{
			return GameEnum::MAIN_ACT_COMBINE;
		}
		else
		{
			return GameEnum::MAIN_ACT_C_RETURN;
		}
	}
	else
	{
		if (this->left_open_activity_time() > 0)
		{
			return GameEnum::MAIN_ACT_OPEN;
		}
		else
		{
			return GameEnum::MAIN_ACT_RETURN;
		}
	}
}

int GameConfig::client_open_days()
{
	if (this->do_combine_server() == true)
	{
		return this->combine_server_days();
	}
	else
	{
		return this->open_server_days();
	}
}

int GameConfig::fetch_server_index_list(const int scene_id, IntVec &index_list)
{
    const GameConfig::ServerList &server_list= this->server_list();
    for (GameConfig::ServerList::const_iterator iter = server_list.begin();
            iter != server_list.end(); ++iter)
    {
        const ServerDetail &server_detail = *iter;
        JUDGE_CONTINUE(server_detail.__scene_list.count(scene_id) > 0);
        index_list.push_back(iter - server_list.begin());
    }
    return 0;
}

int GameConfig::test_client_set_open_day(int test_day)
{
	if (this->do_combine_server())
	{
		this->test_set_server_combine_day(test_day);
	}
	else
	{
		this->test_set_server_open_day(test_day);
	}
	return 0;
}

int GameConfig::test_set_server_open_day(int test_day)
{
	Int64 open_time = ::time(NULL) - (test_day - 1) * Time_Value::DAY;
	this->server_config_.__open_sever.set_tick(open_time);
	this->server_config_.__is_combine_server = false;
	return 0;
}

bool GameConfig::do_combine_server(void)
{
	return this->server_config_.__is_combine_server;
}

Int64 GameConfig::combine_day_tick(int days)	// 合服后几天
{
	return this->server_config_.__combine_server.create_day_tick() + days * Time_Value::DAY;
}

Int64 GameConfig::combine_server_date()		// 合服时间
{
	return this->server_config_.__combine_server.create_tick_;
}

int GameConfig::combine_server_days()	// 合服天数，从1开始
{
	return this->server_config_.__combine_server.passed_days();
}

int GameConfig::combine_server_hours()			// 合服小时数
{
	Int64 open_time = this->server_config_.__combine_server.passed_time();
	return open_time / Time_Value::HOUR;
}

int GameConfig::left_combine_activity_time(int adjust_time)	// 剩余合服活动时间
{
	static int combine_days = this->const_set("combine_activity_days");
#ifdef TEST_COMMAND
	Int64 combine_end_tick = this->combine_day_tick(combine_days) - 1;
#else
	static Int64 combine_end_tick = this->combine_day_tick(combine_days) - 1;
#endif
	return GameCommon::left_time(combine_end_tick + adjust_time);
}
bool GameConfig::is_during_combine_activity(int day)	//是否是第几天的合服活动
{
	JUDGE_RETURN(this->left_combine_activity_time() > 0, false);
	return this->combine_server_days() == day;
}

void GameConfig::test_set_server_combine_day(int test_day)
{
	Int64 open_time = ::time(NULL) - (test_day - 1) * Time_Value::DAY;
	this->server_config_.__combine_server.set_tick(open_time);
	this->server_config_.__is_combine_server = true;
}

int GameConfig::load_map_config(void)
{
	if (this->load_json_config("config/limit/scene_set.json",
			this->limit_config_.__scene_set.revert_json()) == 0)
	{
		this->limit_config_.__scene_set.convert_json_to_map();
		this->limit_config_.__scene_set.update_version();
	}

	this->load_json_config("config/map/scene.json",
			this->scene_config_.__scene.revert_json());
	this->scene_config_.__scene.convert_json_to_map();

    for (GameConfig::ConfigMap::iterator iter = this->scene_config_.__scene.revert_map().begin();
            iter != this->scene_config_.__scene.revert_map().end(); ++iter)
    {
    	int scene_id = iter->first;
    	JUDGE_CONTINUE(GameCommon::is_normal_scene(scene_id) == true);	//普通场景
    	this->load_scene_config(scene_id, "map");
    }

    const ServerDetail& server_detail = this->cur_map_server();
    for (BIntSet::iterator iter = server_detail.__scene_list.begin();
    		iter != server_detail.__scene_list.end(); ++iter)
    {
		int scene_id = *iter;
		JUDGE_CONTINUE(GameCommon::is_script_scene(scene_id) == false);	//副本场景另外加载
		JUDGE_CONTINUE(GameCommon::is_normal_scene(scene_id) == false);	//普通场景另外加载
//		JUDGE_CONTINUE(this->is_convert_scene(scene_id) == false);   	//处理分线的场景
		this->load_scene_config(scene_id, "map");
    }

	this->load_script_config();

	// 处理分线的场景
	for (BIntMap::iterator iter = this->server_config_.__convert_scene_to_map.begin();
			iter != this->server_config_.__convert_scene_to_map.end(); ++iter)
	{
        int scene_id = iter->first;
        int from_scene = iter->second;

		char scene_key[32] = {0};
		::sprintf(scene_key, "%d", scene_id);

		this->scene_config_.__scene.revert_json()[scene_key] = *(this->scene_config_.__scene.revert_map()[from_scene]);
		Json::Value &scene_json = this->scene_config_.__scene.revert_json()[scene_key];
		this->scene_config_.__scene.revert_map().rebind(scene_id, &scene_json);

		MptDetailMap &mpt_detail_map = this->scene_config_.revert_mpt_detail_map();
		mpt_detail_map[scene_id] = mpt_detail_map[from_scene];
		MptMap &mpt_map = this->scene_config_.revert_mpt_coord_map();
		CoordIndexListMap &coord_index_map = this->scene_config_.revert_move_coord_map();
		CoordIndexSetMap &border_set_map = this->scene_config_.revert_border_coord_map();
		mpt_map[scene_id] = mpt_map[from_scene];
		coord_index_map[scene_id] = coord_index_map[from_scene];
		border_set_map[scene_id] = border_set_map[from_scene];
	}

	this->scene_config_.update_version();
    this->load_map_block_config();
    return 0;
}

int GameConfig::convert_to_border_coord(const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2, CoordIndexSet &border_set)
{
    int dx = x2 - x1, dy = y2 - y1, next_x, next_y;
    double k = 0, k_dlta = 0.0;
    int dlta = 0, is_use_dlta_x = 1, inc_unit = 5;
    if (std::abs(dy) > std::abs(dx))
    {
        is_use_dlta_x = 0;
        dlta = (dy < 0 ? -inc_unit : inc_unit);
        if (dy != 0)
            k = double(dx) / dy * dlta;
        k_dlta += k;
        next_x = x1 + k_dlta;
        next_y = y1 + dlta;
    }
    else
    {
        is_use_dlta_x = 1;
        dlta = (dx < 0 ? -inc_unit : inc_unit);
        if (dx != 0)
            k = double(dy) / dx * dlta;
        k_dlta += k;
        next_x = x1 + dlta;
        next_y = y1 + k_dlta;
    }
    int pos_x = MoverCoord::pixel_to_pos(x1), pos_y = MoverCoord::pixel_to_pos(y1);
    int64_t pos_index = merge_int_to_long(pos_x, pos_y);
    border_set.insert(pos_index);
    while ((std::abs(dx) > 0 && std::abs(next_x - x1) <= std::abs(dx)) || (std::abs(dy) > 0 && std::abs(next_y - y1) <= std::abs(dy)))
    {
        pos_x = MoverCoord::pixel_to_pos(next_x);
        pos_y = MoverCoord::pixel_to_pos(next_y);
        pos_index = merge_int_to_long(pos_x, pos_y);
        border_set.insert(pos_index);
        k_dlta += k;
        if (is_use_dlta_x == 1)
        {
            next_x += dlta;
            next_y = y1 + k_dlta;
        }
        else
        {
            next_x = x1 + k_dlta;
            next_y += dlta;
        }
    }
    return 0;
}

int GameConfig::load_scene_config(const int scene_id, const char *path)
{
    char scene_key[32], config_doc[1024];

    ::sprintf(scene_key, "%d", scene_id);
    ::sprintf(config_doc, "config/%s/s%d.spt", path, scene_id);
    if (this->load_mpt_config(config_doc, scene_id, this->scene_config_) != 0)
        return -1;

    ::sprintf(config_doc, "config/%s/s%s.json", path, scene_key);
    Json::Value json;
    if (this->load_json_config(config_doc, json) != 0)
    {
        MSG_USER("ERROR load scene detail json failed [%s]", config_doc);
        return -1;
    }

    Json::Value &scene_json = this->scene_config_.__scene.revert_json()[scene_key];

    Json::Value::Members json_member = json.getMemberNames();
    for (Json::Value::Members::iterator iter = json_member.begin();
    		iter != json_member.end(); ++iter)
    {
    	JUDGE_CONTINUE(*iter != "scene_id");

    	MSG_DEBUG("load scene %d %s", scene_id, (*iter).c_str());
    	scene_json[*iter] = json[*iter];
    }

    for (Json::Value::iterator json_iter = scene_json["npc"].begin();
            json_iter != scene_json["npc"].end(); ++json_iter)
    {
        Json::Value &npc_json = *json_iter;
        npc_json["scene_id"] = scene_id;
        this->scene_config_.__npc.revert_double_map().rebind(scene_id, atoi(json_iter.key().asCString()), &npc_json);
    }

    this->scene_config_.__scene.revert_map().rebind(scene_id, &scene_json);

    {
        SceneSceneJumpMap &scene_jump_map = this->scene_config_.revert_scene_scene_jump_map();
        const Json::Value &exits_json = scene_json["exits"];

        IntVec less_scene_vc, all_sceen_vc;
        for (uint i = 0; i < exits_json.size(); ++i)
        {
            int exit_scene_id = ::atoi(exits_json[i]["id"].asCString());
            if (exit_scene_id <= 0 || exit_scene_id == scene_id)
                continue;

            scene_jump_map.rebind(scene_id, exit_scene_id, 1);
            scene_jump_map.rebind(exit_scene_id, scene_id, 1);
            all_sceen_vc.push_back(exit_scene_id);
        }

        for (IntVec::iterator left_iter = all_sceen_vc.begin();
                left_iter != all_sceen_vc.end(); ++left_iter)
        {
            for (IntVec::iterator right_iter = left_iter; right_iter != all_sceen_vc.end(); ++right_iter)
            {
                if (*left_iter == *right_iter)
                    continue;

                this->calc_scene_jump(*left_iter, scene_id, *right_iter);
            }
        }
    }

    MSG_USER("INFO load scene %s", scene_json["name"].asCString());
    return 0;
}

int GameConfig::calc_scene_jump(const int start_scene, const int mid_scene, const int target_scene)
{
    SceneSceneJumpMap &scene_jump_map = this->scene_config_.revert_scene_scene_jump_map();
    int jump_step = 0, left_jump = 0, right_jump = 0;
    scene_jump_map.find(start_scene, mid_scene, left_jump);
    scene_jump_map.find(mid_scene, target_scene, right_jump);
    if (scene_jump_map.find(start_scene, target_scene, jump_step) == 0 && jump_step <= (left_jump + right_jump))
        return 0;

    scene_jump_map.rebind(start_scene, target_scene, left_jump + right_jump);
    scene_jump_map.rebind(target_scene, start_scene, left_jump + right_jump);
    
    SceneSceneJumpMap::KeyValueMap *sub_scene_map = 0;
    if (scene_jump_map.find_object_map(start_scene, sub_scene_map) == 0)
    {
        for (SceneSceneJumpMap::KeyValueMap::iterator iter = sub_scene_map->begin(); iter != sub_scene_map->end(); ++iter)
        {
            if (iter->first == target_scene || iter->first == mid_scene)
                continue;
            this->calc_scene_jump(iter->first, start_scene, target_scene);
        }
    }
    return 0;
}

int GameConfig::load_mpt_config(const char *doc, const int mpt_id, SceneConfig &scene_conf)
{
    int file = ::open(doc, O_RDONLY);
    if (file < 0)
        return -1;

    struct stat statbuf;
    if (::fstat(file, &statbuf) < 0)
    {
        LOG_SYS("fstat");
        return -1;
    }

    void *src = 0;
    if ((src = ::mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) 
    {
        LOG_SYS("mmap");
        return -1;
    }

    Block_Buffer dst_buf;
    //dst_buf.ensure_writable_bytes(statbuf.st_size * 4);
    dst_buf.ensure_writable_bytes(statbuf.st_size + sizeof(int) * 4);
    ::memcpy(dst_buf.get_write_ptr(), src, statbuf.st_size);
    dst_buf.set_write_idx(dst_buf.get_write_idx() + statbuf.st_size);

    if (::munmap(src, statbuf.st_size) < 0) 
    {
        LOG_SYS("munmap");
        return -1;
    }
    ::close(file);

    int16_t cell_width, cell_height, x_len, y_len, map_width, map_height, i_point_y;
    dst_buf.read_int16_big_endian(cell_width);
    dst_buf.read_int16_big_endian(cell_height);
    dst_buf.read_int16_big_endian(x_len);
    dst_buf.read_int16_big_endian(y_len);
    dst_buf.read_int16_big_endian(map_width);
    dst_buf.read_int16_big_endian(map_height);
    dst_buf.read_int16_big_endian(i_point_y);

    dst_buf.set_read_idx(dst_buf.get_read_idx() + sizeof(int) * 10);
    MptMap &mpt_map = scene_conf.revert_mpt_coord_map();
    CoordIndexListMap &coord_index_map = scene_conf.revert_move_coord_map();
    CoordIndexSetMap &border_set_map = scene_conf.revert_border_coord_map();

    MptDetail &mpt_detail = scene_conf.revert_mpt_detail_map()[mpt_id];
    mpt_detail.__cell_width = cell_width;
    mpt_detail.__cell_height = cell_height;
    mpt_detail.__x_len = x_len;
    mpt_detail.__y_len = y_len;
    mpt_detail.__map_width = map_width;
    mpt_detail.__map_height = map_height;
    mpt_detail.__i_point_y = i_point_y;

    LOG_DEBUG_INFO("mpt range %d cell(%d,%d) grid(%d,%d) pixel(%d,%d)",
    		mpt_id, cell_width, cell_height, x_len, y_len, map_width, map_height);

    MptCoordList &mpt_coord_list = mpt_map[mpt_id];
    CoordIndexList &coord_list = coord_index_map[mpt_id];
    CoordIndexSet &border_set = border_set_map[mpt_id];

    mpt_coord_list.clear();
    coord_list.clear();
    border_set.clear();
    int8_t coord = 0;
    size_t max_size = x_len * y_len;
    for (size_t i = 0; i < max_size; ++i)
    {
        dst_buf.read_int8(coord);
        mpt_coord_list.push_back(coord);

        if (GameCommon::is_movable_mpt(coord) == true)
        {
        	coord_list.push_back(GridIndexType(i));
        }
    }

    int32_t polygon_size = 0, vertex_size = 0, begin_vertex_x = 0, begin_vertex_y = 0,
            prev_vertex_x = 0, prev_vertex_y = 0, vertex_x = 0, vertex_y = 0;
    dst_buf.read_int32_big_endian(polygon_size);
    if (polygon_size > 100000)
    	MSG_USER("ERROR load mpt polygon %d %d", mpt_id, polygon_size);

    for (int i = 0; i < polygon_size; ++i)
    {
        dst_buf.read_int32_big_endian(vertex_size);
        dst_buf.read_int32_big_endian(begin_vertex_x);
        dst_buf.read_int32_big_endian(begin_vertex_y);
        prev_vertex_x = begin_vertex_x;
        prev_vertex_y = begin_vertex_y;
        for (int j = 1; j < vertex_size; ++j)
        {
            dst_buf.read_int32_big_endian(vertex_x);
            dst_buf.read_int32_big_endian(vertex_y);
            this->convert_to_border_coord(prev_vertex_x, prev_vertex_y, vertex_x, vertex_y, border_set);
            prev_vertex_x = vertex_x;
            prev_vertex_y = vertex_y;
        }
    }
    if (polygon_size > 0)
        this->convert_to_border_coord(vertex_x, vertex_y, begin_vertex_x, begin_vertex_y, border_set);

    return 0;
}

int GameConfig::load_map_block_config(void)
{
    this->load_json_config("config/server/map_block.json",
    		this->scene_config_.__map_block.revert_json());

    Json::Value &map_block_json = this->scene_config_.__map_block.revert_json();
    MapBlockDetail &revert_map_block_detail = this->scene_config_.revert_map_block_detail();
    revert_map_block_detail.__max_step = map_block_json["step"].asInt();
    revert_map_block_detail.__monster_step_tick = map_block_json["monster_step_tick"].asDouble();
    revert_map_block_detail.__script_stop_times = map_block_json["script_stop"].asInt();
    revert_map_block_detail.__script_monster_max = map_block_json["script_monster"].asInt();
    if (revert_map_block_detail.__script_monster_max <= 0)
        revert_map_block_detail.__script_monster_max = 50;
    revert_map_block_detail.__base_block_config.__block_width = map_block_json["block_width"].asInt();
    revert_map_block_detail.__base_block_config.__block_height = map_block_json["block_height"].asInt();

    const Json::Value &flush_tick_json = map_block_json["flush_tick"];
    for (uint i = 0; i < flush_tick_json.size(); ++i)
        revert_map_block_detail.__flush_tick_vc.push_back(flush_tick_json[i].asInt());

    const Json::Value &scene_block_json = map_block_json["scene"];
    for (uint i = 0; i < scene_block_json.size(); ++i)
    {
        int scene_id = scene_block_json[i]["scene_id"].asInt();
        MptDetailMap::iterator iter = this->scene_config_.mpt_detail_map().find(scene_id);
        if (iter == this->scene_config_.mpt_detail_map().end())
            continue;
        iter->second.__block_width = scene_block_json[i]["block_width"].asInt();
        iter->second.__block_height = scene_block_json[i]["block_height"].asInt();

        BlockConfigDetail block_conf_detail;
        block_conf_detail.__block_width = scene_block_json[i]["block_width"].asInt();
        block_conf_detail.__block_height = scene_block_json[i]["block_height"].asInt();
        revert_map_block_detail.__scene_block_config_map.bind(scene_id, block_conf_detail);
    }
    this->scene_config_.__map_block.update_version();
    return 0;
}

int GameConfig::load_activity_config(void)
{
	//活动大厅
	if (this->load_json_config("config/activity/common_activity.json",
				this->activity_config_.__common_activity.revert_json()) == 0)
	{
		this->activity_config_.__common_activity.convert_json_to_map();
		this->activity_config_.__common_activity.update_version();
	}

	//开服活动
	if (this->load_json_config("config/activity/open_activity.json",
				this->activity_config_.__open_activity.revert_json()) == 0)
	{
		this->activity_config_.__open_activity.convert_json_to_map();
		this->activity_config_.__open_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__open_activity.update_version();
	}

	//返利活动
	if (this->load_json_config("config/activity/return_activity.json",
				this->activity_config_.__return_activity.revert_json()) == 0)
	{
		this->activity_config_.__return_activity.convert_json_to_map();
		this->activity_config_.__return_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__return_activity.update_version();
	}

	//节日活动
	if (this->load_json_config("config/activity/festival_activity.json",
				this->activity_config_.__festival_activity.revert_json()) == 0)
	{
		this->activity_config_.__festival_activity.convert_json_to_map();
		this->activity_config_.__festival_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__festival_activity.update_version();
	}

	//合服活动
	if (this->load_json_config("config/activity/combine_activity.json",
				this->activity_config_.__combine_activity.revert_json()) == 0)
	{
		this->activity_config_.__combine_activity.convert_json_to_map();
		this->activity_config_.__combine_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__combine_activity.update_version();
	}

	//合服返利活动
	if (this->load_json_config("config/activity/combine_return_activity.json",
				this->activity_config_.__combine_return_activity.revert_json()) == 0)
	{
		this->activity_config_.__combine_return_activity.convert_json_to_map();
		this->activity_config_.__combine_return_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__combine_return_activity.update_version();
	}

	//五一活动
	if (this->load_json_config("config/activity/may_activity.json",
			this->activity_config_.__may_activity.revert_json()) == 0)
	{
		this->activity_config_.__may_activity.convert_json_to_map();
		this->activity_config_.__may_activity.load_combine_file("config/activity/act%d.json");
		this->activity_config_.__may_activity.update_version();
	}

	if (this->load_json_config("config/activity/daily_run_item.json",
			this->activity_config_.__daily_run_item.revert_json()) == 0)
	{
		this->activity_config_.__daily_run_item.convert_json_to_map();
		this->activity_config_.__daily_run_item.update_version();
	}

	if (this->load_json_config("config/activity/daily_run_extra.json",
			this->activity_config_.__daily_run_extra.revert_json()) == 0)
	{
		this->activity_config_.__daily_run_extra.convert_json_to_map();
		this->activity_config_.__daily_run_extra.update_version();
	}

	if (this->load_json_config("config/activity/no_send_act.json",
			this->activity_config_.__no_send_act.revert_json()) == 0)
	{
		this->activity_config_.__no_send_act.convert_json_to_map();
		this->activity_config_.__no_send_act.update_version();
	}

	if (this->load_json_config("config/activity/recharge.json",
			this->welfare_config_.__recharge.revert_json()) == 0)
	{
		this->welfare_config_.__recharge.convert_json_to_map();
		this->welfare_config_.__recharge.update_version();
	}

	if (this->load_json_config("config/activity/daily_recharge.json",
			this->welfare_config_.__daily_recharge.revert_json()) == 0)
	{
		this->welfare_config_.__daily_recharge.convert_json_to_map();
		this->welfare_config_.__daily_recharge.update_version();
	}

	if (this->load_json_config("config/activity/rebate_recharge.json",
			this->welfare_config_.__rebate_recharge.revert_json()) == 0)
	{
		this->welfare_config_.__rebate_recharge.convert_json_to_map();
		this->welfare_config_.__rebate_recharge.update_version();
	}

	if (this->load_json_config("config/activity/invest_recharge.json",
			this->welfare_config_.__invest_recharge.revert_json()) == 0)
	{
		this->welfare_config_.__invest_recharge.convert_json_to_map();
		this->welfare_config_.__invest_recharge.update_version();
	}

	if (this->load_json_config("config/activity/hidden_treasure.json",
			this->welfare_config_.__hidden_treasure.revert_json()) == 0)
	{
		this->welfare_config_.__hidden_treasure.convert_json_to_map();
		this->welfare_config_.__hidden_treasure.update_version();
	}

	if (this->load_json_config("config/activity/operate_activity.json",
			this->welfare_config_.__operate_activity.revert_json()) == 0)
	{
		this->welfare_config_.__operate_activity.convert_json_to_map();
		this->welfare_config_.__operate_activity.load_combine_file("config/activity/act%d.json");
		this->welfare_config_.__operate_activity.update_version();
	}

	if (this->load_json_config("config/activity/score_exchange.json",
			this->welfare_config_.__score_exchange.revert_json()) == 0)
	{
		this->welfare_config_.__score_exchange.convert_json_to_map();
		this->welfare_config_.__score_exchange.update_version();
	}

	if (this->load_json_config("config/activity/time_limit_reward.json",
			this->welfare_config_.__time_limit_reward.revert_json()) == 0)
	{
		this->welfare_config_.__time_limit_reward.convert_json_to_map();
		this->welfare_config_.__time_limit_reward.update_version();
	}

	if (this->load_json_config("config/activity/word_reward.json",
			this->welfare_config_.__word_reward.revert_json()) == 0)
	{
		this->welfare_config_.__word_reward.convert_json_to_map();
		this->welfare_config_.__word_reward.update_version();
	}

	if (this->load_json_config("config/activity/egg_reward.json",
			this->welfare_config_.__egg_reward.revert_json()) == 0)
	{
		this->welfare_config_.__egg_reward.convert_json_to_map();
		this->welfare_config_.__egg_reward.update_version();
	}

	if (this->load_json_config("config/activity/open_gift.json",
			this->welfare_config_.__open_gift.revert_json()) == 0)
	{
		this->welfare_config_.__open_gift.convert_json_to_map();
		this->welfare_config_.__open_gift.update_version();
	}

	if (this->load_json_config("config/activity/daily_recharge_rank.json",
			this->welfare_config_.__daily_recharge_rank.revert_json()) == 0)
	{
		this->welfare_config_.__daily_recharge_rank.convert_json_to_map();
		this->welfare_config_.__daily_recharge_rank.update_version();
	}

	if (this->load_json_config("config/activity/daily_activity.json",
			this->welfare_config_.__daily_activity.revert_json()) == 0)
	{
		this->welfare_config_.__daily_activity.convert_json_to_map();
		this->welfare_config_.__daily_activity.load_combine_file("config/activity/act%d.json");
		this->welfare_config_.__daily_activity.update_version();
	}

	if (this->load_json_config("config/activity/total_double.json",
			this->welfare_config_.__total_double.revert_json()) == 0)
	{
		this->welfare_config_.__total_double.convert_json_to_map();
		this->welfare_config_.__total_double.update_version();
	}

    return 0;
}

int GameConfig::load_welfare_config(void)
{
	if (this->load_json_config("config/welfare/check_model.json",
			this->welfare_config_.__check_model.revert_json()) == 0)
	{
		this->welfare_config_.__check_model.convert_json_to_map();
		this->welfare_config_.__check_model.update_version();
	}

	if (this->load_json_config("config/welfare/day_check.json",
			this->welfare_config_.__day_check.revert_json()) == 0)
	{
		this->welfare_config_.__day_check.convert_json_to_map();
		this->welfare_config_.__day_check.update_version();
	}

	if (this->load_json_config("config/welfare/online_check.json",
			this->welfare_config_.__online_check.revert_json()) == 0)
	{
		this->welfare_config_.__online_check.convert_json_to_map();
		this->welfare_config_.__online_check.update_version();
	}

	if (this->load_json_config("config/welfare/total_check.json",
			this->welfare_config_.__total_check.revert_json()) == 0)
	{
		this->welfare_config_.__total_check.convert_json_to_map();
		this->welfare_config_.__total_check.update_version();
	}

//	if (this->load_json_config("config/welfare/welfare_config.json",
//			this->welfare_config_.__welfare_elements.revert_json()) == 0)
//	{
//		this->welfare_config_.__welfare_elements.convert_json_to_map();
//		this->welfare_config_.__welfare_elements.update_version();
//	}

	if (this->load_json_config("config/welfare/restore.json",
			this->welfare_config_.__restore.revert_json()) != 0)
	{
		this->welfare_config_.__restore.convert_json_to_map();
		this->welfare_config_.__restore.update_version();
	}

	return 0;
}

int GameConfig::load_brocast_config(void)
{
	if (this->load_json_config("config/brocast/brocast.json",
			this->brocast_config_.__brocast.revert_json()) == 0)
	{
		this->brocast_config_.__brocast.convert_json_to_map();
		this->brocast_config_.__brocast.update_version();
	}

	if (this->load_json_config("config/brocast/tips_message.json",
			this->brocast_config_.__tips_msg.revert_json()) == 0)
	{
		this->brocast_config_.__tips_msg.convert_json_to_map();
		this->brocast_config_.__tips_msg.update_version();
	}

	return 0;
}

int GameConfig::load_script_config(void)
{
    if (this->load_json_config("config/script/script.json", this->script_config_.__script.revert_json()) != 0)
    {
        this->script_config_.__script.revert_json() = this->script_config_.__script.json();
        MSG_USER("ERROR load script.json");
    }
    else
    {
        this->script_config_.__script.convert_json_to_map();
    }

	if (this->load_json_config("config/script/clean_out.json",
			this->script_config_.__clean_out.revert_json()) == 0)
	{
		this->script_config_.__clean_out.convert_json_to_map();
		this->script_config_.__clean_out.update_version();
	}

	if (this->load_json_config("config/script/sword_skill.json",
			this->script_config_.__sword_skill.revert_json()) == 0)
	{
		this->script_config_.__sword_skill.convert_json_to_map();
		this->script_config_.__sword_skill.update_version();
	}

    this->load_script_map();
    this->script_config_.__script.update_version();

    return 0;
}

int GameConfig::load_script_map(void)
{
	int script_flag = false;
    const ServerDetail& server_detail = this->cur_map_server();

    for (BIntSet::iterator iter = server_detail.__scene_list.begin();
    		iter != server_detail.__scene_list.end(); ++iter)
    {
    	JUDGE_CONTINUE(GameCommon::is_script_scene(*iter) == true);
    	script_flag = true;
    	break;
    }
    JUDGE_RETURN(script_flag == true, -1);

    char config_doc[1024];
    for (GameConfig::ConfigMap::iterator iter = this->script_config_.__script.revert_map().begin();
            iter != this->script_config_.__script.revert_map().end(); ++iter)
    {
        Json::Value &script_json = *(iter->second);
        for (uint i = 0; i < script_json["scene"].size(); ++i)
        {
            Json::Value &script_scene_json = script_json["scene"][i];
            int scene_id = script_scene_json["scene_id"].asInt();

            if (this->load_scene_config(scene_id, "script") != 0)
            {
            	MSG_USER("ERROR load script scene config %d", scene_id);
            }

            Json::Value *scene_json = 0;
            if (this->scene_config_.__scene.revert_map().find(scene_id, scene_json) != 0)
            {
                this->scene_config_.__scene.revert_map().rebind(scene_id, &script_scene_json);
                continue;
            }

            (*scene_json)["floor"] = script_scene_json["floor"];
            (*scene_json)["exec"] = script_scene_json["exec"];
            (*scene_json)["award"] = script_scene_json["award"];
            (*scene_json)["script_sort"] = iter->first;

            if ((*scene_json)["exec"].isMember("wave_spt_map"))
            {
                Json::Value &wave_spt_json = (*scene_json)["exec"]["wave_spt_map"];
                for (uint j = 0; j < wave_spt_json.size(); ++j)
                {
                    int spt_id = wave_spt_json[j][1u].asInt();

                    ::sprintf(config_doc, "config/%s/s%d_%d.spt", "script", spt_id % 1000000, spt_id / 1000000);
                    if (this->load_mpt_config(config_doc, spt_id, this->scene_config_) != 0)
                        MSG_USER("ERROR load %s", config_doc);
                }
            }
        }
    }

    return 0;
}

const Json::Value &GameConfig::map_block(void)
{
    return this->scene_config_.__map_block.json();
}

int GameConfig::max_move_step(void)
{
    return this->scene_config_.map_block_detail().__max_step;
}

const double &GameConfig::monster_step_tick(void)
{
	return this->scene_config_.map_block_detail().__monster_step_tick;
}

int GameConfig::script_stop_times(void)
{
    return this->scene_config_.map_block_detail().__script_stop_times;
}

int GameConfig::script_monster_max(void)
{
    return this->scene_config_.map_block_detail().__script_monster_max;
}

MapBlockDetail &GameConfig::map_block_detail(void)
{
    return this->scene_config_.map_block_detail();
}

const Json::Value &GameConfig::scene(int scene_id)
{
	return this->scene_config_.__scene.find(scene_id);
}

int GameConfig::scene_set_type(int scene_id)
{
	const Json::Value& scene_set = this->scene_set(scene_id);
	return scene_set["type"].asInt();
}

const MptDetail &GameConfig::mpt(const int mpt_id)
{
    MptDetailMap::iterator iter = this->scene_config_.mpt_detail_map().find(mpt_id);
    if (iter == this->scene_config_.mpt_detail_map().end())
    {
        return this->scene_config_.__null_mpt_detail;
    }
    return iter->second;
}

const GameConfig::CoordIndexList &GameConfig::move_coord_list(const int mpt_id)
{
    return this->scene_config_.move_coord_map()[mpt_id];
}

int GameConfig::coord_mpt_value(const int mpt_id, const int coord_x, const int coord_y)
{
	JUDGE_RETURN(coord_x >= 0 && coord_y >= 0, -1);

    int x_len = this->mpt(mpt_id).__x_len;
    MptCoordList &list = this->scene_config_.mpt_coord_map()[mpt_id];

    size_t index = coord_y * x_len + coord_x;
    return (index < list.size() ? list[index] : -1);
}

int GameConfig::fetch_coord_by_mpt_value(CoordVec& coord_set, int mpt_id, int mpt_value)
{
	const MptDetail &mpt_detail = CONFIG_INSTANCE->mpt(mpt_id);

	MptCoordList &list = this->scene_config_.mpt_coord_map()[mpt_id];
	size_t total_size = list.size();

	for (int i = 0; i < mpt_detail.__x_len; ++i)
	{
		for (int j = 0; j < mpt_detail.__y_len; ++j)
		{
			size_t index = j * mpt_detail.__x_len + i;

			JUDGE_CONTINUE(index < total_size);
			JUDGE_CONTINUE(list[index] < mpt_value);

			coord_set.push_back(MoverCoord(i, j));
		}
	}

	return 0;
}

//是否是没有阵营区分
bool GameConfig::is_no_camp_scene(const int scene_id)
{
    const Json::Value &scene_json = this->scene(scene_id);
    return scene_json["camp_split"].asInt() == 0;
}

bool GameConfig::is_border_coord_for_rand(const int scene_id, const int32_t pos_x, const int32_t pos_y)
{
    CoordIndexSet &border_set = this->scene_config_.border_coord_map()[scene_id];

    int64_t pos_index = merge_int_to_long(pos_x, pos_y);
	return border_set.find(pos_index) != border_set.end();
}

bool GameConfig::is_border_coord(const int scene_id, const int32_t pos_x, const int32_t pos_y)
{
    CoordIndexSet &border_set = this->scene_config_.border_coord_map()[scene_id];

    int64_t pos_index = merge_int_to_long(pos_x, pos_y);
	if (border_set.find(pos_index) != border_set.end())
		return true;
	MoverCoord coord;
	coord.set_pos(pos_x, pos_y - 1);
	if (GameCommon::is_movable_coord(scene_id, coord))
		return true;
	coord.set_pos(pos_x, pos_y + 1);
	if (GameCommon::is_movable_coord(scene_id, coord))
		return true;
	coord.set_pos(pos_x - 1, pos_y);
	if (GameCommon::is_movable_coord(scene_id, coord))
		return true;
	coord.set_pos(pos_x + 1, pos_y);
	if (GameCommon::is_movable_coord(scene_id, coord))
		return true;
    return false;
}

const Json::Value &GameConfig::npc(const int scene_id, const int npc_id)
{
    Json::Value *json = 0;
    if (this->scene_config_.__npc.double_map().find(scene_id, npc_id, json) == 0)
        return *json;
    return Json::Value::null;
}

const Json::Value &GameConfig::script(const int script_sort)
{
	return this->script_config_.__script.find(script_sort);
}

const GameConfig::ConfigMap &GameConfig::script_map(void)
{
    return this->script_config_.__script.map();
}

const Json::Value &GameConfig::script_clean_out(int script_sort)
{
	return this->script_config_.__clean_out.find(script_sort);
}

const Json::Value &GameConfig::script_clean_order(void)
{
    return this->script_config_.__clean_out.json()["order"];
}

int GameConfig::script_clean_fast_gold(void)
{
    return this->script_config_.__clean_out.json()["fast_gold"].asInt();
}

const Json::Value &GameConfig::script_monster_tower(void)
{
    return this->script_config_.__monster_tower.json();
}

const Json::Value &GameConfig::script_sword_skill(int id)
{
	return this->script_config_.__sword_skill.find(id);
}

int GameConfig::load_fight_config(void)
{
    MSG_USER("load fight config begin");

	if (this->load_json_config("config/fight/player_skill.json",
			this->fight_config_.__role_skill.revert_json()) == 0)
	{
		this->fight_config_.__role_skill.convert_json_to_map();
		for (ConfigMap::iterator iter = this->fight_config_.__role_skill.revert_map().begin();
				iter != this->fight_config_.__role_skill.revert_map().end(); ++iter)
		{
			JUDGE_CONTINUE(iter->first > 0);
			this->fight_config_.__skill.revert_map().rebind(iter->first, iter->second);
		}
	}

	if (this->load_json_config("config/fight/monster_skill.json",
			this->fight_config_.__monster_skill.revert_json()) == 0)
	{
		this->fight_config_.__monster_skill.convert_json_to_map();
		for (ConfigMap::iterator iter = this->fight_config_.__monster_skill.revert_map().begin();
				iter != this->fight_config_.__monster_skill.revert_map().end(); ++iter)
		{
			JUDGE_CONTINUE(iter->first > 0);
			this->fight_config_.__skill.revert_map().rebind(iter->first, iter->second);
		}
	}

	GameConfig::DoubleKeyConfigMap &detail_map = this->fight_config_.__skill_detail.revert_double_map();
	detail_map.unbind_all();
	ConfigMap &skill_map = this->fight_config_.__skill.revert_map();
	for (ConfigMap::iterator iter = skill_map.begin(); iter != skill_map.end(); ++iter)
	{
		int skill_id = iter->first;

		Json::Value& skill_conf = *(iter->second);
		JUDGE_CONTINUE(skill_conf.isArray() == false);

		if (skill_conf["level_type"].asInt() == 1)
		{
			char path_file[1024] = {0};
			::sprintf(path_file, "config/fight/skill%d.json", skill_id);
			this->combine_json_file_a(skill_conf, path_file);
		}

		for (uint i = 0; i < skill_conf["detail"].size(); ++i)
		{
			detail_map.rebind(skill_id, i + 1, &(skill_conf["detail"][i]));
		}
	}

	this->fight_config_.__skill.update_version();
	this->fight_config_.__skill_detail.update_version();
	this->fight_config_.__role_skill.update_version();
	this->fight_config_.__monster_skill.update_version();

    MSG_USER("load fight config finish");
    return 0;
}


const Json::Value &GameConfig::skill(const int skill_id)
{
	return this->fight_config_.__skill.find(skill_id);
}

const GameConfig::ConfigMap &GameConfig::role_skill_map(void)
{
    return this->fight_config_.__role_skill.map();
}

const Json::Value &GameConfig::skill_detail(int skill_id, int skill_level)
{
    if (skill_level <= 0)
    {
        return Json::Value::null;
    }

    Json::Value *json = 0;
    if (this->fight_config_.__skill_detail.double_map().find(skill_id, skill_level, json) != 0)
    {
        return Json::Value::null;
    }

    return *json;
}

int GameConfig::load_item_config(void)
{
	if (this->load_json_config("config/item/item.json",
			this->item_config_.item_.revert_json()) == 0)
	{
		this->item_config_.item_.convert_json_to_map();
		this->item_config_.item_.update_version();
	}

	if (this->load_json_config("config/item/buff.json",
			this->item_config_.buff_.revert_json()) == 0)
	{
		this->item_config_.buff_.convert_json_to_map();
		this->item_config_.buff_.update_version();
	}

	if (this->load_json_config("config/item/shop.json",
			this->item_config_.shop_.revert_json()) == 0)
	{
		this->item_config_.shop_.update_version();
	}

	if (this->load_json_config("config/item/server_reward.json",
		this->item_config_.reward_.revert_json()) == 0)
	{
		this->item_config_.reward_.convert_json_to_map(true);
		this->item_config_.reward_.update_version();
	}

	if (this->load_json_config("config/item/magicweapon.json",
	   this->item_config_.magic_weapon_.revert_json()) == 0)
	{
		this->item_config_.magic_weapon_.convert_json_to_map();
		this->item_config_.magic_weapon_.update_version();
	}

	if (this->load_json_config("config/item/rama_info_list.json",
	   this->item_config_.rama_list_.revert_json()) == 0)
	{
		this->item_config_.rama_list_.convert_json_to_map();
		this->item_config_.rama_list_.update_version();
	}

	if (this->load_json_config("config/item/rama_open.json",
	   this->item_config_.rama_open_.revert_json()) == 0)
	{
		this->item_config_.rama_open_.convert_json_to_map();
		this->item_config_.rama_open_.update_version();
	}

    return 0;
}

int GameConfig::load_equip_config(void)
{
	if (this->load_json_config("config/equip/item_smelt_exp.json",
				this->equip_config_.smelt_exp_.revert_json()) == 0)
	{
		this->equip_config_.smelt_exp_.convert_json_to_map();
		this->equip_config_.smelt_exp_.update_version();
	}

	if (this->load_json_config("config/equip/smelt_level.json",
			this->equip_config_.smelt_level_.revert_json()) == 0)
	{
		this->equip_config_.smelt_level_.convert_json_to_map();
		this->equip_config_.smelt_level_.update_version();
	}

	if (this->load_json_config("config/equip/equip_refine.json",
			this->equip_config_.refine_.revert_json()) == 0)
	{
		this->equip_config_.refine_.convert_json_to_map();
		this->equip_config_.refine_.update_version();
	}

	if (this->load_json_config("config/equip/equip_compse.json",
			this->equip_config_.compose_.revert_json()) == 0)
	{
		this->equip_config_.compose_.convert_json_to_map();
		this->equip_config_.compose_.update_version();
	}

	if (this->load_json_config("config/equip/jewal_upgrade.json",
			this->equip_config_.jewel_upgrade_.revert_json()) == 0)
	{
		this->equip_config_.jewel_upgrade_.convert_json_to_map();
		this->equip_config_.jewel_upgrade_.update_version();
	}

	if (this->load_json_config("config/equip/jewal_sublime.json",
			this->equip_config_.jewel_sublime_.revert_json()) == 0)
	{
		this->equip_config_.jewel_sublime_.convert_json_to_map();
		this->equip_config_.jewel_sublime_.update_version();
	}

	for (int i = 0; i < GameEnum::EQUIP_MAX_INDEX; ++i)
	{
		//强化
		char file_name[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0};
		::snprintf(file_name, GameEnum::DEFAULT_MAX_NAME_LENGTH,
				"config/equip/equip_strengthen%d.json", i + 1);

		if (this->load_json_config(file_name,
				this->equip_config_.strengthen_[i].revert_json()) == 0)
		{
			this->equip_config_.strengthen_[i].convert_json_to_map();
			this->equip_config_.strengthen_[i].update_version();
		}

		//红装升阶
		::snprintf(file_name, GameEnum::DEFAULT_MAX_NAME_LENGTH,
				"config/equip/red_uprising%d.json", i + 1);

		if (this->load_json_config(file_name,
				this->equip_config_.red_uprising[i].revert_json()) == 0)
		{
			this->equip_config_.red_uprising[i].convert_json_to_map();
			this->equip_config_.red_uprising[i].update_version();
		}
	}

	if (this->load_json_config("config/equip/prop_suit.json",
			this->equip_config_.prop_suit_.revert_json()) == 0)
	{
		this->equip_config_.prop_suit_.convert_json_to_map();
		this->equip_config_.prop_suit_.update_version();
	}

	if (this->load_json_config("config/equip/red_clothes_exchange.json",
			this->equip_config_.red_clothes_exchange_.revert_json()) == 0)
	{
		this->equip_config_.red_clothes_exchange_.convert_json_to_map();
		this->equip_config_.red_clothes_exchange_.update_version();
	}

	if (this->load_json_config("config/equip/secret_exchange.json",
			this->equip_config_.secret_exchange_.revert_json()) == 0)
	{
		this->equip_config_.secret_exchange_.convert_json_to_map();
		this->equip_config_.secret_exchange_.update_version();
	}

	if (this->load_json_config("config/equip/legend_exchange.json",
			this->equip_config_.legend_exchange_.revert_json()) == 0)
	{
		this->equip_config_.legend_exchange_.convert_json_to_map();
		this->equip_config_.legend_exchange_.update_version();
	}

	return 0;
}

int GameConfig::load_market_config(void)
{
    if (this->load_json_config("config/market/market.json",
            this->market_cfg_.market_.revert_json()) == 0)
    {
		this->market_cfg_.market_.convert_json_to_map();
		this->market_cfg_.market_.update_version();
    }

    if (this->load_json_config("config/market/market_const.json",
            this->market_cfg_.market_const_.revert_json()) == 0)
    {
		this->market_cfg_.market_const_.update_version();
    }

    return 0;
}

int GameConfig::insert_total_task(ConfigMap &task_map, IntSetMap &npc_task_map)
{
    for (GameConfig::ConfigMap::iterator iter = task_map.begin(); 
            iter != task_map.end(); ++iter)
    {
        Json::Value *json = iter->second;
        int accept_npc = (*json)["before_accept"]["npc"].asInt();
        int submit_npc = (*json)["finish"]["npc"].asInt();
        int accepted_npc = (*json)["after_accept"]["npc"].asInt();
        if (accept_npc > 0)
            npc_task_map[accept_npc].insert(iter->first);
        if (submit_npc > 0)
            npc_task_map[submit_npc].insert(iter->first);
        if (accepted_npc > 0)
            npc_task_map[accepted_npc].insert(iter->first);
    
        this->task_cfg_.__total_task.revert_map().rebind(iter->first,iter->second);
    }
    return 0;
}

int GameConfig::load_task_config(void)
{
    {
        if (this->load_json_config("config/task/task_setting.json",
        		this->task_cfg_.__task_setting.revert_json()) == 0)
        {
        }
    
        if (this->load_json_config("config/task/main.json",
        		this->task_cfg_.__main.revert_json()) != 0)
        {
            MSG_USER("load main.json error");
            return -1;
        }

        this->task_cfg_.__main.convert_json_to_map();
        IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.prev_version()];
        npc_task_map.clear();
        this->insert_total_task(this->task_cfg_.__main.revert_map(), npc_task_map);
    }

    if (this->load_json_config("config/task/branch_task.json",this->task_cfg_.__branch.revert_json()) == 0)
    {
        this->task_cfg_.__branch.convert_json_to_map();
        IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.prev_version()];
        this->insert_total_task(this->task_cfg_.__branch.revert_map(), npc_task_map);
    }

	if (this->load_json_config("config/task/offer_routine.json", this->task_cfg_.__offer_routine.revert_json()) == 0)
	{
		this->task_cfg_.__offer_routine.convert_json_to_map();
		IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.prev_version()];
		this->insert_total_task(this->task_cfg_.__offer_routine.revert_map(), npc_task_map);
	}

	if (this->load_json_config("config/task/new_routine.json", this->task_cfg_.__new_routine.revert_json()) == 0)
	{
		this->task_cfg_.__new_routine.convert_json_to_map();
		IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.prev_version()];
		this->insert_total_task(this->task_cfg_.__new_routine.revert_map(), npc_task_map);
	}

	if (this->load_json_config("config/task/league_routine.json", this->task_cfg_.__league_routine.revert_json()) == 0)
	{
		this->task_cfg_.__league_routine.convert_json_to_map();
		IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.prev_version()];
		this->insert_total_task(this->task_cfg_.__league_routine.revert_map(), npc_task_map);
	}

    this->task_cfg_.__main.update_version();
    this->task_cfg_.__branch.update_version();
    this->task_cfg_.__total_task.update_version();
    this->task_cfg_.__task_setting.update_version();
    this->task_cfg_.__new_routine.update_version();
    this->task_cfg_.__offer_routine.update_version();
    return 0;
}

int GameConfig::load_relax_play_config(void)
{
	{
		if(this->load_json_config("config/relax_play/collect_chests.json",
				this->relax_play_config_.__collect_chests.revert_json()) != 0)
		{
			MSG_USER("ERROR Load collect_chests configure");			}
		else
		{
			this->relax_play_config_.__collect_chests.convert_json_to_map(true);
			this->relax_play_config_.__collect_chests.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/chests_table.json",
				this->relax_play_config_.__chests_table.revert_json()) != 0)
		{
			MSG_USER("ERROR Load chests_table configure");			}
		else
		{
			this->relax_play_config_.__chests_table.convert_json_to_map(true);
			this->relax_play_config_.__chests_table.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/answer_activity.json",
				this->relax_play_config_.__answer_activity.revert_json()) != 0)
		{
			MSG_USER("ERROR Load chests_table configure");			}
		else
		{
			this->relax_play_config_.__answer_activity.convert_json_to_map(true);
			this->relax_play_config_.__answer_activity.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/convoy.json",
				this->relax_play_config_.__convoy.revert_json()) != 0)
		{
			MSG_USER("ERROR Load convoy configure");			}
		else
		{
			this->relax_play_config_.__convoy.convert_json_to_map(true);
			this->relax_play_config_.__convoy.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/treasures_base.json",
				this->relax_play_config_.__treasures_base.revert_json()) != 0)
		{
			MSG_USER("ERROR Load treasures_base configure");			}
		else
		{
			this->relax_play_config_.__treasures_base.convert_json_to_map(true);
			this->relax_play_config_.__treasures_base.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/treasures_grid.json",
				this->relax_play_config_.__treasures_grid.revert_json()) != 0)
		{
			MSG_USER("ERROR Load treasures_grid configure");			}
		else
		{
			this->relax_play_config_.__treasures_grid.convert_json_to_map(true);
			this->relax_play_config_.__treasures_grid.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/hotspring_activity.json",
				this->relax_play_config_.__hotspring_activity.revert_json()) != 0)
		{
			MSG_USER("ERROR Load chests_table configure");			}
		else
		{
			this->relax_play_config_.__hotspring_activity.convert_json_to_map(true);
			this->relax_play_config_.__hotspring_activity.update_version();
		}
	}
	{
		if(this->load_json_config("config/relax_play/topic_bank.json",
				this->relax_play_config_.__topic_bank.revert_json()) != 0)
		{
			MSG_USER("ERROR Load chests_table configure");			}
		else
		{
			this->relax_play_config_.__topic_bank.convert_json_to_map(true);
			this->relax_play_config_.__topic_bank.update_version();
		}
	}
	return 0;
}

int GameConfig::load_achieve_config(void)
{
	{
		if(this->load_json_config("config/achieve/illus.json",
				this->achieve_config_.__illus.revert_json()) != 0)
		{
			MSG_USER("ERROR Load illus configure");
		}
		else
		{
			this->achieve_config_.__illus.convert_json_to_map(true);
			this->achieve_config_.__illus.update_version();
		}
	}
	{
		if(this->load_json_config("config/achieve/illus_class.json",
				this->achieve_config_.__illus_class.revert_json()) != 0)
		{
			MSG_USER("ERROR Load illus_class configure");
		}
		else
		{
			this->achieve_config_.__illus_class.convert_json_to_map(true);
			this->achieve_config_.__illus_class.update_version();
		}
	}
	{
		if(this->load_json_config("config/achieve/illus_group.json",
				this->achieve_config_.__illus_group.revert_json()) != 0)
		{
			MSG_USER("ERROR Load illus_group configure");
		}
		else
		{
			this->achieve_config_.__illus_group.convert_json_to_map(true);
			this->achieve_config_.__illus_group.update_version();
		}
	}
	{
		if(this->load_json_config("config/achieve/label.json",
				this->achieve_config_.__label.revert_json()) != 0)
		{
			MSG_USER("ERROR Load label configure");
		}
		else
		{
			this->achieve_config_.__label.convert_json_to_map(true);
			this->achieve_config_.__label.update_version();
		}
	}

	{
		if(this->load_json_config("config/achieve/achievement.json",
				this->achieve_config_.__achievement.revert_json()) != 0)
		{
			MSG_USER("ERROR Load achievement configure");
		}
		else
		{
			this->achieve_config_.__achievement.convert_json_to_map();
			this->achieve_config_.__achievement.update_version();
		}
	}
	{
		if(this->load_json_config("config/achieve/rank_pannel.json",
				this->achieve_config_.__rank_pannel.revert_json()) != 0)
		{
			MSG_USER("ERROR Load rank_pannel configure");
		}
		else
		{
			this->achieve_config_.__rank_pannel.convert_json_to_map();
			this->achieve_config_.__rank_pannel.update_version();
		}
	}

//	if (this->load_json_config("config/achieve/child_achieve.json",
//			this->achieve_config_.__new_achieve.revert_json()) == 0)
//	{
//		this->achieve_config_.__new_achieve.convert_json_to_map();
//		ConfigMap& ach_map = this->achieve_config_.__new_achieve.revert_map();
//
//		for (ConfigMap::iterator iter = ach_map.begin(); iter != ach_map.end(); ++iter)
//		{
//			char path_file[1024] = {0};
//			::sprintf(path_file, "config/achieve/ach%d.json", iter->first);
//
//			Json::Value& ach_conf = *(iter->second);
//			this->combine_json_file_b(ach_conf["detail"], path_file);
//		}
//
//		this->achieve_config_.__new_achieve.update_version();
//	}

	if (this->load_json_config("config/achieve/achieve_list.json",
			this->achieve_config_.__achieve_list.revert_json()) == 0)
	{
		this->achieve_config_.__achieve_list.convert_json_to_map();
		this->achieve_config_.__achieve_list.update_version();
	}

	if (this->load_json_config("config/achieve/child_achieve.json",
			this->achieve_config_.__new_achieve.revert_json()) == 0)
	{
		this->achieve_config_.__new_achieve.convert_json_to_map();
		this->achieve_config_.__new_achieve.update_version();
	}

	if (this->load_json_config("config/achieve/achieve_level.json",
			this->achieve_config_.__achieve_level.revert_json()) == 0)
	{
		this->achieve_config_.__achieve_level.convert_json_to_map();
		this->achieve_config_.__achieve_level.update_version();
	}

	return 0;
}

int GameConfig::load_tiny_config(void)
{
	if (this->load_json_config("config/tiny/opinion_record.json",
			this->tiny_config_.opinion_record_.revert_json()) == 0)
	{
		this->tiny_config_.opinion_record_.convert_json_to_map();
		this->tiny_config_.opinion_record_.update_version();
	}

    if (this->load_json_config("config/tiny/font.json",
            this->tiny_config_.font_.revert_json()) == 0)
    {
        this->tiny_config_.font_.convert_json_to_map();
        this->tiny_config_.font_.update_version();
    }

	if (this->load_json_config("config/tiny/tiny.json",
			this->tiny_config_.tiny_.revert_json()) == 0)
	{
		this->tiny_config_.tiny_.update_version();
	}

    if (this->load_json_config("config/tiny/team.json",
                this->tiny_config_.team_.revert_json()) == 0)
    {
    	this->tiny_config_.team_.convert_json_to_map();
        this->tiny_config_.team_.update_version();
    }

	if (this->load_json_config("config/tiny/channel_ext.json",
			this->tiny_config_.channel_ext_.revert_json()) == 0)
	{
		this->tiny_config_.channel_ext_.update_version();
	}

    return 0;
}


int GameConfig::load_league_config(void)
{
    if (this->load_json_config("config/league/server_league.json",
                this->league_config_.league_.revert_json()) == 0)
    {
    	this->league_config_.league_.update_version();
    }

    {
        this->load_json_config("config/league/league_war.json",
                this->league_config_.league_war_.revert_json());
        this->league_config_.league_war_.update_version();
    }

    {
        this->load_json_config("config/league/league_log.json",
                this->league_config_.league_log_.revert_json());

        this->league_config_.league_log_.convert_json_to_map();
        this->league_config_.league_log_.update_version();
    }

    {
    	if (this->load_json_config("config/league/boss_info.json",
    				this->league_config_.boss_info_.revert_json()) == 0)
    	{
    		this->league_config_.boss_info_.convert_json_to_map();
    		this->league_config_.boss_info_.update_version();
    	}
    }

	if (this->load_json_config("config/league/server_league_boss.json",
			this->league_config_.league_boss_.revert_json()) == 0)
	{
		this->league_config_.league_boss_.update_version();
	}

    {
        if (this->load_json_config("config/league/league_flag.json",
        		this->league_config_.league_flag_.revert_json()) == 0)
        {
        	this->league_config_.league_flag_.convert_json_to_map();
        	this->league_config_.league_flag_.update_version();
        }
    }
    {
        if (this->load_json_config("config/league/league_skill_new.json",
        		this->league_config_.league_skill_.revert_json()) == 0)
        {
            this->league_config_.league_skill_.convert_json_to_map();
            this->league_config_.league_skill_.update_version();
        }
    }
    {
        if (this->load_json_config("config/league/league_pos.json",
        		this->league_config_.league_pos_.revert_json()) == 0)
        {
            this->league_config_.league_pos_.convert_json_to_map();
            this->league_config_.league_pos_.update_version();
        }
    }
    {
        if (this->load_json_config("config/league/lfb_wave_reward.json",
            	this->league_config_.lfb_wave_reward_.revert_json()) == 0)
        {
            this->league_config_.lfb_wave_reward_.convert_json_to_map();
            this->league_config_.lfb_wave_reward_.update_version();
        }
    }
    {
        if (this->load_json_config("config/league/lfb_base.json",
                this->league_config_.lfb_base_.revert_json()) == 0)
        {
            this->league_config_.lfb_base_.convert_json_to_map();
            this->league_config_.lfb_base_.update_version();
        }
    }
    {
        if (this->load_json_config("config/league/lfb_cheer_attr.json",
                this->league_config_.lfb_cheer_attr_.revert_json()) == 0)
        {
            this->league_config_.lfb_cheer_attr_.convert_json_to_map();
            this->league_config_.lfb_cheer_attr_.update_version();
        }
    }

    if (this->load_json_config("config/league/lrf_weapon.json",
            this->league_config_.lrf_weapon_.revert_json()) == 0)
    {
        this->league_config_.lrf_weapon_.convert_json_to_map();
        this->league_config_.lrf_weapon_.update_version();
    }

    if (this->load_json_config("config/league/league_region.json",
            this->league_config_.league_region_.revert_json()) == 0)
    {
        this->league_config_.league_region_.convert_json_to_map();
        this->league_config_.league_region_.update_version();
    }

    return 0;
}

int GameConfig::load_fashion_config(void)
{
	if(this->load_json_config("config/fashion/fashion.json",
			this->fashion_config_.fashion_.revert_json()) == 0)
	{
		this->fashion_config_.fashion_.convert_json_to_map();
		this->fashion_config_.fashion_.update_version();
	}

	if(this->load_json_config("config/fashion/fashion_level.json",
			this->fashion_config_.fashion_level_.revert_json()) == 0)
	{
		this->fashion_config_.fashion_level_.convert_json_to_map();
		this->fashion_config_.fashion_level_.update_version();
	}

	if(this->load_json_config("config/fashion/fashion_num_add.json",
			this->fashion_config_.fashion_num_add_.revert_json()) == 0)
	{
		this->fashion_config_.fashion_num_add_.convert_json_to_map();
		this->fashion_config_.fashion_num_add_.update_version();
	}

	if(this->load_json_config("config/fashion/fashion_send.json",
			this->fashion_config_.fashion_send_.revert_json()) == 0)
	{
		this->fashion_config_.fashion_send_.convert_json_to_map();
		this->fashion_config_.fashion_send_.update_version();
	}

	if (this->load_json_config("config/fashion/fashion_const.json",
			this->fashion_config_.fashion_const_.revert_json()) == 0)
	{
		this->fashion_config_.__const_map.clear();
		for (Json::Value::iterator iter = this->fashion_config_.fashion_const_.revert_json().begin();
		        iter != this->fashion_config_.fashion_const_.revert_json().end(); ++iter)
		{
			Json::Value *json = &(*iter);
			string name = iter.key().asString();

			int value = (*json)["value"].asInt();
			JUDGE_CONTINUE(name.empty() == false);

			this->fashion_config_.__const_map[name] = value;
		}

		this->fashion_config_.fashion_const_.update_version();
	}

	return 0;
}

int GameConfig::load_sword_pool_config(void)
{
	if(this->load_json_config("config/sword_pool/sword_pool.json",
			this->sword_pool_config_.sword_pool_.revert_json()) == 0)
	{
		this->sword_pool_config_.sword_pool_.update_version();
	}

	if(this->load_json_config("config/sword_pool/sword_pool_set_up.json",
			this->sword_pool_config_.sword_pool_set_up_.revert_json()) == 0)
	{
		this->sword_pool_config_.sword_pool_set_up_.convert_json_to_map();
		this->sword_pool_config_.sword_pool_set_up_.update_version();
	}

	if(this->load_json_config("config/sword_pool/sword_pool_task_info.json",
			this->sword_pool_config_.sword_pool_task_info_.revert_json()) == 0)
	{
		this->sword_pool_config_.sword_pool_task_info_.convert_json_to_map();
		this->sword_pool_config_.sword_pool_task_info_.update_version();
	}

	return 0;
}

int GameConfig::load_transfer_config(void)
{
	if(this->load_json_config("config/transfer/spirit_level.json",
			this->transfer_config_.spirit_level_.revert_json()) == 0)
	{
		this->transfer_config_.spirit_level_.convert_json_to_map();
		this->transfer_config_.spirit_level_.update_version();
	}

	if(this->load_json_config("config/transfer/spirit_stage.json",
			this->transfer_config_.spirit_stage_.revert_json()) == 0)
	{
		this->transfer_config_.spirit_stage_.convert_json_to_map();
		this->transfer_config_.spirit_stage_.update_version();
	}

	if(this->load_json_config("config/transfer/transfer_base.json",
			this->transfer_config_.transfer_base_.revert_json()) == 0)
	{
		this->transfer_config_.transfer_base_.convert_json_to_map();
		this->transfer_config_.transfer_base_.update_version();
	}

	if(this->load_json_config("config/transfer/transfer_total.json",
			this->transfer_config_.transfer_total_.revert_json()) == 0)
	{
		this->transfer_config_.transfer_total_.convert_json_to_map();
		this->transfer_config_.transfer_total_.load_combine_file("config/transfer/change%d.json");
		this->transfer_config_.transfer_total_.update_version();
	}

	return 0;
}

int GameConfig::load_mount_config(void)
{
	for (int i = 0; i < GameEnum::FUN_TOTAL_MOUNT_TYPE; ++i)
	{
		char file_name1[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0};
		char file_name2[GameEnum::DEFAULT_MAX_NAME_LENGTH + 1] = {0};

		::snprintf(file_name1, GameEnum::DEFAULT_MAX_NAME_LENGTH,
				"config/mount/mount%d.json", i + 1);
		::snprintf(file_name2, GameEnum::DEFAULT_MAX_NAME_LENGTH,
				"config/mount/mount_level%d.json", i + 1);

		if (this->load_json_config(file_name1,
				this->mount_config_.mount_[i].revert_json()) == 0)
		{
			this->mount_config_.mount_[i].convert_json_to_map();
			this->mount_config_.mount_[i].update_version();
		}

		if (this->load_json_config(file_name2,
					this->mount_config_.mount_level_[i].revert_json()) == 0)
		{
			this->mount_config_.mount_level_[i].convert_json_to_map();
			this->mount_config_.mount_level_[i].update_version();
		}
	}

    if (this->load_json_config("config/mount/prop_unit.json",
    		this->mount_config_.prop_unit_.revert_json()) == 0)
    {
    	this->mount_config_.prop_unit_.convert_json_to_map();
        this->mount_config_.prop_unit_.update_version();
    }

    if (this->load_json_config("config/mount/mount_set.json",
    		this->mount_config_.mount_set_.revert_json()) == 0)
    {
    	this->mount_config_.mount_set_.convert_json_to_map();
        this->mount_config_.mount_set_.update_version();
    }

    if (this->load_json_config("config/mount/change_goder.json",
    		this->mount_config_.change_goder_.revert_json()) == 0)
    {
    	this->mount_config_.change_goder_.convert_json_to_map();
        this->mount_config_.change_goder_.update_version();
    }

    return 0;
}

int GameConfig::load_beast_config(void)
{
	return 0;
}

int GameConfig::load_arena_config(void)
{
	{
        this->load_json_config("config/area/arena.json",
                this->arena_cfg_.arena_config_.revert_json());
        this->arena_cfg_.arena_config_.update_version();
	}

	if (this->load_json_config("config/area/athletics_base_info.json",
			this->arena_cfg_.athletics_base_config_.revert_json()) == 0)
	{
		this->arena_cfg_.athletics_base_config_.convert_json_to_map();
		this->arena_cfg_.athletics_base_config_.update_version();
	}

	if (this->load_json_config("config/area/athletics_rank_info.json",
			this->arena_cfg_.athletics_rank_config_.revert_json()) == 0)
	{
		this->arena_cfg_.athletics_rank_config_.convert_json_to_map();
		this->arena_cfg_.athletics_rank_config_.update_version();
	}

	{
        this->load_json_config("config/area/escort.json",
                this->arena_cfg_.escort_config_.revert_json());
        this->arena_cfg_.escort_config_.update_version();
	}

	return 0;
}

int GameConfig::load_monster_config(void)
{
	if (this->load_json_config("config/monster/bt_factory.json",
			this->ai_bt_config_.bt_factory_.revert_json()) == 0)
	{
	    this->ai_bt_config_.bt_factory_.update_version();
	}

	const Json::Value &tree_type = this->bt_factory()["tree_type"];
	for (uint i = 0; i < tree_type.size(); ++i)
	{
		std::string tree_name = tree_type[i].asString();

		char path_name[MAX_COMMON_NAME_LENGTH] = { 0 };
		::sprintf(path_name, "config/monster/bev/%s", tree_name.c_str());

		BasicConfig* tree_config = NULL;
		if (this->ai_bt_config_.tree_map_.count(tree_name) == 0)
		{
			tree_config = new BasicConfig;
			this->ai_bt_config_.tree_map_[tree_name] = tree_config;
		}
		else
		{
			tree_config = this->ai_bt_config_.tree_map_[tree_name];
		}

		this->load_json_config(path_name, tree_config->revert_json());
		tree_config->update_version();
	}

	this->update_monster_config();
    return 0;
}

int GameConfig::update_monster_config(void)
{
	if (this->load_json_config("config/monster/robot.json",
			this->ai_bt_config_.monster_set_.revert_json()) == 0)
	{
        this->ai_bt_config_.monster_set_.convert_json_to_map();
        this->ai_bt_config_.monster_set_.update_version();
    }

    if (this->load_json_config("config/monster/prop_monster.json",
    		this->ai_bt_config_.prop_monster_.revert_json()) == 0)
    {
        this->ai_bt_config_.prop_monster_.convert_json_to_map();
        this->ai_bt_config_.prop_monster_.update_version();
    }

    if (this->load_json_config("config/monster/gather.json",
    		this->ai_bt_config_.gather_.revert_json()) == 0)
    {
        this->ai_bt_config_.gather_.convert_json_to_map();
        this->ai_bt_config_.gather_.update_version();
    }

    return 0;
}

int GameConfig::load_role_config(void)
{
	if (this->load_json_config("config/role/birth.json",
			this->role_config_.__birth.revert_json()) == 0)
	{
		this->role_config_.__birth.convert_json_to_map();
		this->role_config_.__birth.update_version();
	}

	if (this->load_json_config("config/role/player_level.json",
			this->role_config_.__player_level.revert_json()) == 0)
	{
		this->role_config_.__player_level.convert_json_to_map();
		this->role_config_.__player_level.update_version();
	}

	this->role_config_.sub_level_exp_.clear();
	this->role_config_.add_level_exp_.clear();
	for (GameConfig::ConfigMap::iterator iter = this->role_config_.__player_level.map().begin();
			iter != this->role_config_.__player_level.map().end(); ++iter)
	{
		const Json::Value& conf = *(iter->second);

		if (conf.isMember("sub_level_exp") == true)
		{
			int sub_percent = conf["sub_level_exp"].asInt();
			this->role_config_.sub_level_exp_[iter->first] = sub_percent;
		}

		if (conf.isMember("add_level_exp") == true)
		{
			int add_percent = conf["add_level_exp"].asInt();
			this->role_config_.add_level_exp_[iter->first] = add_percent;
		}
	}

	if (this->load_json_config("config/role/seven_day.json",
			this->role_config_.__seven_day.revert_json()) == 0)
	{
		this->role_config_.__seven_day.convert_json_to_map();
		this->role_config_.__seven_day.update_version();
	}

	if (this->load_json_config("config/activity/cumulative_login.json",
			this->role_config_.__cumulative_login.revert_json()) == 0)
	{
		this->role_config_.__cumulative_login.convert_json_to_map();
		this->role_config_.__cumulative_login.update_version();
	}

	if (this->load_json_config("config/role/vip.json",
			this->role_config_.__vip.revert_json()) == 0)
	{
		this->role_config_.__vip.convert_json_to_map();
		this->role_config_.__vip.update_version();
	}

    {
        this->load_json_config("config/role/blood_cont.json",
                this->role_config_.__blood_cont.revert_json());
        this->role_config_.__blood_cont.update_version();
    }

    {
        this->load_json_config("config/role/copy_player.json",
                this->role_config_.__copy_player.revert_json());
        this->role_config_.__copy_player.update_version();
    }

    {
    	this->load_json_config("config/role/random_name.json",
    			this->role_config_.__random_name.revert_json());
		this->role_config_.__random_name.update_version();
    }

    {
        this->load_json_config("config/role/replacement.json",
        		this->role_config_.__replacement.revert_json());
        this->role_config_.__replacement.update_version();
    }


    if (this->load_json_config("config/role/test_goods.json",
        		this->role_config_.__test_goods.revert_json()) == 0)
    {
        this->role_config_.__test_goods.convert_json_to_map();
		this->role_config_.__test_goods.update_version();
    }

    if (this->load_json_config("config/role/fight_rate.json",
            		this->role_config_.__fight_power.revert_json()) == 0)
	{
		this->role_config_.__fight_power.convert_json_to_map();
		this->role_config_.__fight_power.update_version();
	}

    return 0;
}

const Json::Value &GameConfig::role(int sex)
{
	return this->role_config_.__birth.find(sex);
}

void GameConfig::test_goods(IntMap& goods_map)
{
	for (ConfigMap::iterator iter = this->role_config_.__test_goods.map().begin();
			iter != this->role_config_.__test_goods.map().end(); ++iter)
	{
		const Json::Value& json = *(iter->second);

		int key = iter->first;
		int value = json["amount"].asInt();
		JUDGE_CONTINUE(key > 0 && value > 0);

		goods_map[key] = value;
	}
}

const Json::Value &GameConfig::seven_day(const int day)
{
	return this->role_config_.__seven_day.find(day);
}

const Json::Value &GameConfig::cumulative_login(const int day)
{
	return this->role_config_.__cumulative_login.find(day);
}

const Json::Value &GameConfig::role_level(int career, int level)
{
	return this->role_config_.__player_level.find(level);
}

const Json::Value &GameConfig::blood_cont(const char* item_name)
{
	return this->role_config_.__blood_cont.json_name(item_name);
}

const Json::Value &GameConfig::copy_player(const char* item_name)
{
	return this->role_config_.__copy_player.json_name(item_name);
}

const Json::Value &GameConfig::replacement_cont(const char* item_name)
{
	return this->role_config_.__replacement.json_name(item_name);
}

void GameConfig::role_fightConfig(const int fightnum, float &power_rate,const int level)
{
	ConfigMap& fight_power = this->role_config_.__fight_power.map();
	for (ConfigMap::iterator iter = fight_power.begin();iter != fight_power.end(); ++iter)
	{
		const Json::Value& json = *iter->second;
		int interval_down = json["interval_down"].asInt();
		int interval_up = json["interval_up"].asInt();
		int cfg_level = json["level"].asInt();
		if( level == cfg_level && fightnum >= interval_down && fightnum <= interval_up )
		{
			power_rate = (json["percent"].asInt())/10000.00000f;
			break;
		}
//		else if(  level == cfg_level && fightnum < interval_down )
//		{
//			power_rate = (json["percent"].asInt())/10000.00000f;
//			break;
//		}
	}
	//return this->role_config_.__fight_power.json();
}

int GameConfig::top_level(void)
{
	static int max_role_level = this->const_set("max_role_level");
	return max_role_level;
}

double GameConfig::pk_tick(void)
{
//    return this->role_config_.__level.json()["pk_tick"].asDouble();
	return 1800;
}

int GameConfig::base_role_speed(void)
{
//    return this->role_config_.__level.json()["base_spd"].asInt();
	return 100;
}

// diff = monster_level - player_level
double GameConfig::sub_level_exp(int diff)
{
	JUDGE_RETURN(diff > 0, 0);

	double percent_value = GameEnum::DAMAGE_ATTR_PERCENT / 2;
	if (this->role_config_.sub_level_exp_.count(diff) > 0)
	{
		percent_value = this->role_config_.sub_level_exp_[diff];
	}

	return percent_value / GameEnum::DAMAGE_ATTR_PERCENT;
}

// diff = average_level - player_level
double GameConfig::add_level_exp(int diff)
{
	JUDGE_RETURN(diff > 0, 0);

	double percent_value = GameEnum::DAMAGE_ATTR_PERCENT / 2;
	if (this->role_config_.add_level_exp_.count(diff) > 0)
	{
		percent_value = this->role_config_.add_level_exp_[diff];
	}

	return percent_value / GameEnum::DAMAGE_ATTR_PERCENT;
}

const Json::Value &GameConfig::vip(int vip_type_id)
{
	JUDGE_RETURN(vip_type_id > 0, Json::Value::null);
	return this->role_config_.__vip.find(vip_type_id - 99);
}

const Json::Value &GameConfig::vip(void)
{
    return this->role_config_.__vip.json();
}

const Json::Value &GameConfig::first_name(void)
{
	return this->role_config_.__random_name.json()["first_name"];
}

const Json::Value &GameConfig::man_second(void)
{
	return this->role_config_.__random_name.json()["man_second"];
}

const Json::Value &GameConfig::man_third(void)
{
	return this->role_config_.__random_name.json()["man_third"];
}

const Json::Value &GameConfig::woman_second(void)
{
	return this->role_config_.__random_name.json()["woman_second"];
}

const Json::Value &GameConfig::woman_third(void)
{
	return this->role_config_.__random_name.json()["woman_third"];
}

int GameConfig::load_limit_config(void)
{
    {
        if (this->load_json_config("config/limit/experience.json", this->limit_config_.__exp_limit.revert_json()) != 0)
        {
            MSG_USER("ERROR Load experience configure");
        }
        Json::Value &serial_json = this->limit_config_.__exp_limit.revert_json()["serial"];
        this->limit_config_.__exp_limit.revert_map().clear();
        for (uint i = 0; i < serial_json.size(); ++i)
        {
            Json::Value &json = serial_json[i];
            for (uint j = 0; j < json["type"].size(); ++j)
                this->limit_config_.__exp_limit.revert_map().bind(json["type"][j].asInt(), &json);
        }
        this->limit_config_.__exp_limit.update_version();
    }
    {
        if (this->load_json_config("config/limit/limit_item.json", this->limit_config_.__item_limit.revert_json()) != 0)
        {
            MSG_USER("ERROR Load item_limit configure");
        }
        Json::Value &serial_json = this->limit_config_.__item_limit.revert_json()["serial"];
        this->limit_config_.__item_limit.revert_map().clear();
        for (uint i = 0; i < serial_json.size(); ++i)
        {
            Json::Value &json = serial_json[i];
            for (uint j = 0; j < json["type"].size(); ++j)
                this->limit_config_.__item_limit.revert_map().bind(json["type"][j].asInt(), &json);
        }
        this->limit_config_.__item_limit.update_version();
    }
    {
        if (this->load_json_config("config/limit/limit_money.json", this->limit_config_.__money_limit.revert_json()) != 0)
        {
            MSG_USER("ERROR Load money_limit configure");
        }
        Json::Value &serial_json = this->limit_config_.__money_limit.revert_json()["serial"];
        this->limit_config_.__money_limit.revert_map().clear();
        for (uint i = 0; i < serial_json.size(); ++i)
        {
            Json::Value &json = serial_json[i];
            for (uint j = 0; j < json["type"].size(); ++j)
                this->limit_config_.__money_limit.revert_map().bind(json["type"][j].asInt(), &json);
        }
        this->limit_config_.__money_limit.update_version();
    }
    {
        if (this->load_json_config("config/limit/php_center_limit.json",
        		this->limit_config_.__php_center_limit.revert_json()) != 0)
        {
            MSG_USER("ERROR Load php_center_limit configure");
        }

		this->limit_config_.__php_center_limit.update_version();
    }

	if (this->load_json_config("config/limit/limit_line.json",
			this->limit_config_.__scene_line_limit.revert_json()) == 0)
	{
		this->limit_config_.__scene_line_limit.convert_json_to_map();
		this->limit_config_.__scene_line_limit.update_version();
	}

	if (this->load_json_config("config/limit/limit_create_role.json",
			this->limit_config_.__ip_mac_limit.revert_json()) == 0)
	{
		this->limit_config_.__ip_mac_limit.convert_json_to_map();
		this->limit_config_.__ip_mac_limit.update_version();
	}

	if (this->load_json_config("config/limit/correct_agent.json",
			this->limit_config_.__correct_agent.revert_json()) == 0)
	{
		this->limit_config_.__correct_agent.update_version();
	}

	if (this->load_json_config("config/limit/const_set.json",
			this->limit_config_.__const_set.revert_json()) == 0)
	{
		this->limit_config_.__const_map.clear();
	    for (Json::Value::iterator iter = this->limit_config_.__const_set.revert_json().begin();
	            iter != this->limit_config_.__const_set.revert_json().end(); ++iter)
	    {
	        Json::Value *json = &(*iter);
	        string name = iter.key().asString();

	        int value = (*json)["value"].asInt();
	        JUDGE_CONTINUE(name.empty() == false);

	        this->limit_config_.__const_map[name] = value;
	    }

		this->limit_config_.__const_set.update_version();
	}

	if (this->load_json_config("config/limit/function_set.json",
			this->limit_config_.__function_set.revert_json()) == 0)
	{
		this->limit_config_.__fun_task_map.clear();
		this->limit_config_.__fun_level_map.clear();

	    for (Json::Value::iterator iter = this->limit_config_.__function_set.revert_json().begin();
	            iter != this->limit_config_.__function_set.revert_json().end(); ++iter)
	    {
	        string name = iter.key().asString();
	        JUDGE_CONTINUE(name.empty() == false);

	        int value = 0;
	        Json::Value *json = &(*iter);

	        if (json->isMember("open_level") == true)
	        {
	        	value = (*json)["open_level"].asInt();
	        	this->limit_config_.__fun_level_map[name] = value;
	        }

	        if (json->isMember("task_id") == true)
	        {
	        	value = (*json)["task_id"].asInt();
	        	this->limit_config_.__fun_task_map[name] = value;
	        }
	    }

		this->limit_config_.__function_set.update_version();
	}

	if (this->load_json_config("config/limit/red_tips.json",
			this->limit_config_.__red_tips.revert_json()) == 0)
	{
		this->limit_config_.__red_tips.convert_json_to_map();
		this->limit_config_.__red_tips.update_version();
	}

	if (this->load_json_config("config/limit/serial_set.json",
			this->limit_config_.__serial_set.revert_json()) == 0)
	{
		this->limit_config_.__serial_set.convert_json_to_map();
		this->limit_config_.__serial_set.update_version();
	}

    return 0;
}

const Json::Value &GameConfig::exp_limit(const int serial_type)
{
	return this->limit_config_.__exp_limit.find(serial_type);
}

const Json::Value &GameConfig::item_limit(const int serial_type)
{
	return this->limit_config_.__item_limit.find(serial_type);
}

const int64_t GameConfig::item_num_limit(const int serial_type, const int item_id)
{
    const Json::Value &json = this->item_limit(serial_type);
    if (json == Json::Value::null)
        return -1;

    for (uint i = 0; i < json["limit"].size(); ++i)
    {
        if (json["limit"][i][0u].asInt() != item_id)
            continue;
        return json["limit"][i][0u].asDouble();
    }
    return -1;
}

const Json::Value &GameConfig::money_limit(const int serial_type)
{
	return this->limit_config_.__money_limit.find(serial_type);
}

const Json::Value &GameConfig::php_center_limit()
{
    return this->limit_config_.__php_center_limit.json();
}

const Json::Value &GameConfig::scene_line(const int scene_id)
{
	return this->limit_config_.__scene_line_limit.find(scene_id);
}

const Json::Value &GameConfig::agent_of_ip_mac()
{
    return this->limit_config_.__ip_mac_limit.json();
}

const Json::Value &GameConfig::correct_agent(const std::string &agent)
{
	return this->limit_config_.__correct_agent.json_name(agent.c_str());
}

int GameConfig::serial_no_tips(int id)
{
	return this->limit_config_.__serial_set.find(id)["no_tips"].asInt();
}

int GameConfig::serial_exp_tips(int id)
{
	return this->limit_config_.__serial_set.find(id)["exp_tips"].asInt();
}

int GameConfig::validate_red_tips(int id)
{
	return this->limit_config_.__red_tips.validate(id);
}

int GameConfig::const_set(const string& key)
{
	JUDGE_RETURN(this->limit_config_.__const_map.count(key) > 0, 0);
	return this->limit_config_.__const_map[key];
}

int GameConfig::arrive_fun_open_level(const string& key, int level)
{
	JUDGE_RETURN(this->limit_config_.__fun_level_map.count(key) > 0, false);
	return level >= this->limit_config_.__fun_level_map[key];
}

int GameConfig::arrive_fun_open_task(const string& key, int task)
{
	JUDGE_RETURN(this->limit_config_.__fun_task_map.count(key) > 0, false);
	return task == this->limit_config_.__fun_task_map[key];
}

BStrIntMap& GameConfig::task_fun_open_map()
{
	return this->limit_config_.__fun_task_map;
}

const Json::Value& GameConfig::const_set_conf(const string& key)
{
	return this->limit_config_.__const_set.json_name(key.c_str());
}

const Json::Value& GameConfig::scene_set(int scene_id)
{
	return this->limit_config_.__scene_set.find(scene_id);
}

const Json::Value& GameConfig::function_set(const string& key)
{
	return this->limit_config_.__function_set.json()[key];
}

const Json::Value& GameConfig::red_tips(int id)
{
	return this->limit_config_.__red_tips.find(id);
}

int GameConfig::load_daemon_config(Json::Value &json)
{
    if (this->load_json_config("config/daemon.json", json) != 0)
    {
        MSG_USER("ERROR no daemon.json");
        return -1;
    }
    return 0;
}

int GameConfig::load_update_config(Json::Value &json)
{
    if (this->load_json_config("config/update_config.json", json) != 0)
    {
        MSG_USER("ERROR no update_config.json");
        return -1;
    }

    this->server_config_.__update_conf = json;
    return 0;
}

int GameConfig::update_config(const std::string &folder)
{
    Time_Value nowtime = Time_Value::gettimeofday();
    JUDGE_RETURN(this->update_tick_ < nowtime, -1);

    this->update_tick_ = nowtime + Time_Value(5);

    if (folder == "server")
    {
        return this->load_server_config();
    }

    if (folder == "role")
    {
        return this->load_role_config();
    }

    if (folder == "monster")
    {
        return this->update_monster_config();
    }

    if (folder == "map")
    {
        return this->load_map_config();
    }

    if (folder == "limit")
    {
        return this->load_limit_config();
    }

    if (folder == "fight")
    {
        return this->load_fight_config();
    }

    if (folder == "item")
    {
        return this->load_item_config();
    }

    if (folder == "equip")
    {
    	return this->load_equip_config();
    }

    if (folder == "tiny")
    {
    	this->load_tiny_config();
    	this->update_game_ram(folder);
        return 0;
    }

    if (folder == "market")
    {
        return this->load_market_config();
    }

    if (folder == "league")
    {
    	this->load_league_config();
    	this->update_game_ram(folder);
    	return 0;
    }

    if (folder == "fashion")
    {
        return this->load_fashion_config();
    }

    if (folder == "sword_pool")
    {
    	return this->load_sword_pool_config();
    }

    if (folder == "transfer")
    {
        return this->load_transfer_config();
    }

    if (folder == "mount")
    {
    	return this->load_mount_config();
    }

    if (folder == "beast")
    {
    	return this->load_beast_config();
    }

    if (folder == "task")
    {
        return this->load_task_config();
    }

    if (folder == "achieve")
    {
    	return this->load_achieve_config();
    }

    if (folder == "relax_play")
    {
    	return this->load_relax_play_config();
    }

    if (folder == "brocast")
    {
    	return this->load_brocast_config();
    }

    if (folder == "activity")
    {
    	this->load_activity_config();
    	this->update_game_ram(folder);
    	return 0;
    }

    if (folder == "welfare")
    {
    	return this->load_welfare_config();
    }

    if (folder == "area")
    {
    	return this->load_arena_config();
    }

    if (folder == "wedding")
    {
    	this->load_wedding_config();
    	return 0;
    }

    if (folder == "travel")
    {
    	this->load_travel_config();
    	this->load_servers_list();
    	this->load_version_config();
    	return 0;
    }

    if (folder == "all")
    {
        this->load_item_config();
        this->load_equip_config();
        this->load_role_config();
        this->update_monster_config();
        this->load_travel_config();
        this->load_servers_list();
        this->load_version_config();

        this->load_map_config();
        this->load_limit_config();
        this->load_fight_config();
        this->load_tiny_config();
        this->load_market_config();
        this->load_league_config();
        this->load_mount_config();
        this->load_fashion_config();
        this->load_sword_pool_config();
        this->load_transfer_config();
        this->load_beast_config();
        this->load_task_config();
        this->load_achieve_config();
        this->load_relax_play_config();
        this->load_brocast_config();
        this->load_activity_config();
        this->load_welfare_config();
        this->load_wedding_config();
        this->update_game_ram(folder);
        return 0;
    }

    return -1;
}

bool GameConfig::check_update_sub_value(int value)
{
	/*
	 * value
	 * 0: nothing
	 * 1: travel arena
	 * 2: travel channel
	 * */
	JUDGE_RETURN(this->server_config_.__update_conf.isMember(
			"sub_value") == true, false);

	int sub_value = this->server_config_.__update_conf["sub_value"].asInt();
	return sub_value == value;
}

void GameConfig::update_game_ram(const std::string &folder)
{
#ifndef LOCAL_DEBUG
	if (DAEMON_SERVER->is_server_logic())
#endif
	{
		if (folder == "activity" || folder == "all")
		{
			ACTIVITY_TIPS_SYSTEM->init_activity_map_from_cfg();
		}
    }

}

//将json文件合并到原来Json里
void GameConfig::combine_json_file_a(Json::Value& src_json, const string& path_file)
{
	BasicConfig file_conf;
	JUDGE_RETURN(this->load_json_config(path_file.c_str(), file_conf.revert_json()) == 0, ;);

	file_conf.convert_json_to_map();
	file_conf.update_version();

	int key = 0;
	for (ConfigMap::iterator iter = file_conf.map().begin(); iter != file_conf.map().end(); ++iter)
	{
		const Json::Value& item_json = *(iter->second);
		Json::Value::Members json_member = item_json.getMemberNames();

		for (Json::Value::Members::iterator member_iter = json_member.begin();
				member_iter != json_member.end(); ++member_iter)
		{
			const string& member_name = *member_iter;
			JUDGE_CONTINUE(member_name != "name");
			JUDGE_CONTINUE(member_name != "id");
			src_json[member_name][key] = item_json[member_name];
		}

		++key;
	}
}

//将json文件合成一个字段
void GameConfig::combine_json_file_b(Json::Value& src_json, const string& path_file)
{
	BasicConfig file_conf;
	JUDGE_RETURN(this->load_json_config(path_file.c_str(), file_conf.revert_json()) == 0, ;);

	file_conf.convert_json_to_map();
	file_conf.update_version();

	int key = 0;
	for (ConfigMap::iterator iter = file_conf.map().begin(); iter != file_conf.map().end(); ++iter)
	{
		const Json::Value& item_json = *(iter->second);
		Json::Value::Members json_member = item_json.getMemberNames();

		for (Json::Value::Members::iterator member_iter = json_member.begin();
				member_iter != json_member.end(); ++member_iter)
		{
			const string& member_name = *member_iter;
			JUDGE_CONTINUE(member_name != "name");
			JUDGE_CONTINUE(member_name != "id");
			src_json[key][member_name] = item_json[member_name];
		}

		++key;
	}
}

const Json::Value &GameConfig::bt_factory()
{
    return this->ai_bt_config_.bt_factory_.json();
}

const Json::Value &GameConfig::monster(int monster_sort)
{
    return this->ai_bt_config_.monster_set_.find(monster_sort);
}

const Json::Value &GameConfig::prop_monster(int monster_sort)
{
	return this->ai_bt_config_.prop_monster_.find(monster_sort);
}

const Json::Value &GameConfig::gather(int monster_sort)
{
	return this->ai_bt_config_.gather_.find(monster_sort);
}

const Json::Value &GameConfig::behavior(const std::string& tree_name)
{
    JUDGE_RETURN(this->ai_bt_config_.tree_map_.count(tree_name) > 0, Json::Value::null);
    return this->ai_bt_config_.tree_map_[tree_name]->json();
}

const Json::Value &GameConfig::opinion_record()
{
	return this->tiny_config_.opinion_record_.json();
}

const Json::Value &GameConfig::opinion_record(int id)
{
	return this->tiny_config_.opinion_record_.find(id);
}

const Json::Value &GameConfig::normal_team()
{
	return this->tiny_config_.team_.json();
}

const Json::Value &GameConfig::normal_team(int id)
{
	return this->tiny_config_.team_.find(id);
}

const Json::Value &GameConfig::tiny(const char* item_name)
{
	return this->tiny_config_.tiny_.json_name(item_name);
}

string GameConfig::fetch_shop_verison()
{
	return this->item_config_.shop_.version_no();
}

const Json::Value &GameConfig::killing(int color_id)
{
	return Json::Value::null;
}

int GameConfig::server_id()
{
	return this->server_info_->__server_id;
}

string GameConfig::server_flag()
{
	return this->server_info_->__server_flag;
}

int GameConfig::agent_code(const std::string &agent_str)
{
	const Json::Value& json = this->tiny_config_.agent_.json();
	if (json.isMember(agent_str) == false)
	{
		MSG_USER("no agent code: %s", agent_str.c_str());
		return 0;
	}

	return json[agent_str]["code"].asInt();
}

const Json::Value &GameConfig::channel_ext(const std::string &agent_str)
{
	if (this->tiny_config_.channel_ext_.json().isMember(agent_str) == false)
	{
		return Json::Value::null;
	}
	return this->tiny_config_.channel_ext_.json()[agent_str];
}

const Json::Value &GameConfig::league_log(int log_type)
{
	return this->league_config_.league_log_.find(log_type);
}

const Json::Value &GameConfig::league(const char* item_name)
{
    return this->league_config_.league_.json_name(item_name);
}

const Json::Value &GameConfig::league_war(const char* item_name)
{
	return this->league_config_.league_war_.json_name(item_name);
}

const Json::Value &GameConfig::boss_info(const int boss_index)
{
	return this->league_config_.boss_info_.find(boss_index);
}

int GameConfig::boss_attr(const int boss_index, const string name)
{
	const Json::Value& boss_info = this->boss_info(boss_index);
	JUDGE_RETURN(boss_info != Json::Value::null, -1);

	return boss_info[name].asInt();
}

const Json::Value &GameConfig::league_boss(const char* item_name)
{
	return this->league_config_.league_boss_.json_name(item_name);
}

const Json::Value &GameConfig::league_flag(const int flag_lvl)
{
	return this->league_config_.league_flag_.find(flag_lvl);
}

const Json::Value &GameConfig::league_skill()
{
	return this->league_config_.league_skill_.json();
}
const Json::Value &GameConfig::league_skill_info(int id)
{
	return this->league_config_.league_skill_.find(id);
}

const Json::Value &GameConfig::lrf_weapon(int hickty_id, int level)
{
	for (ConfigMap::iterator iter = this->league_config_.lrf_weapon_.map().begin();
			iter != this->league_config_.lrf_weapon_.map().end(); ++iter)
	{
		const Json::Value& weapon_json = *(iter->second);
		JUDGE_CONTINUE(weapon_json["hickty_id"].asInt() == hickty_id
				&& level == weapon_json["lv"].asInt());
		return weapon_json;
	}

	return Json::Value::null;
}

const Json::Value &GameConfig::league_region(int id)
{
	return this->league_config_.league_region_.find(id);
}

string GameConfig::league_pos_name(int pos)
{
	for (ConfigMap::iterator iter = this->league_config_.league_pos_.map().begin();
			iter != this->league_config_.league_pos_.map().end(); ++iter)
	{
		const Json::Value &pos_json = *(iter->second);
		JUDGE_CONTINUE(pos == pos_json["pos_id"].asInt());

		return pos_json["pos_name"].asString();
	}

	return GameCommon::NullString;
}

const Json::Value &GameConfig::lfb_wave_reward(int wave)
{
	return this->league_config_.lfb_wave_reward_.find(wave);
}

GameConfig::ConfigMap &GameConfig::lfb_wave_reward_map()
{
	return this->league_config_.lfb_wave_reward_.map();
}

const Json::Value &GameConfig::lfb_cheer_attr(int num)
{
	return this->league_config_.lfb_cheer_attr_.find(num);
}

const Json::Value &GameConfig::lfb_base()
{
	return this->league_config_.lfb_base_.find(1);
}

int GameConfig::lfb_base_value(const string& key)
{
	const Json::Value &conf = this->league_config_.lfb_base_.find(1);
	JUDGE_RETURN(conf != Json::Value::null, 0);

	return conf[key].asInt();
}

const Json::Value &GameConfig::fashion(int fashion_id)
{
	return this->fashion_config_.fashion_.find(fashion_id);
}

const Json::Value &GameConfig::fashion_level(int level)
{
	return this->fashion_config_.fashion_level_.find(level);
}

const Json::Value &GameConfig::fashion_num_add(int amount)
{
	return this->fashion_config_.fashion_num_add_.find(amount);
}

GameConfig::ConfigMap &GameConfig::fashion_num_add()
{
	return this->fashion_config_.fashion_send_.map();
}

int GameConfig::fashion_const(const string& key)
{
	JUDGE_RETURN(this->fashion_config_.__const_map.count(key) > 0, 0);
	return this->fashion_config_.__const_map[key];
}

const Json::Value &GameConfig::sword_pool(const char* item_name)
{
	return this->sword_pool_config_.sword_pool_.json_name(item_name);
}

const Json::Value &GameConfig::sword_pool_set_up(int level)
{
	return this->sword_pool_config_.sword_pool_set_up_.find(level);
}

const Json::Value &GameConfig::sword_pool_task(int task_id)
{
	return this->sword_pool_config_.sword_pool_task_info_.find(task_id);
}

const Json::Value &GameConfig::sword_pool_total_task()
{
	return this->sword_pool_config_.sword_pool_task_info_.json();
}

const Json::Value &GameConfig::spirit_level(int level)
{
	return this->transfer_config_.spirit_level_.find(level);
}

const Json::Value &GameConfig::spirit_stage(int stage)
{
	return this->transfer_config_.spirit_stage_.find(stage);
}

GameConfig::ConfigMap &GameConfig::spirit_stage_map()
{
	return this->transfer_config_.spirit_stage_.map();
}

const Json::Value &GameConfig::transfer_base()
{
	return this->transfer_config_.transfer_base_.find(1);
}

const Json::Value &GameConfig::transfer_total(int id)
{
	return this->transfer_config_.transfer_total_.find(id);
}

const Json::Value &GameConfig::mount(int type)
{
	return this->mount_config_.mount_[type - 1].json();
}

const Json::Value &GameConfig::mount(int type, int mount_id)
{
	return this->mount_config_.mount_[type - 1].find(mount_id);
}

const Json::Value &GameConfig::mount_level(int type, int role_level)
{
	return this->mount_config_.mount_level_[type - 1].find_limit_max(role_level);
}

const Json::Value &GameConfig::mount_set(int type)
{
	return this->mount_config_.mount_set_.find(type);
}

const Json::Value &GameConfig::prop_unit(int id)
{
	return this->mount_config_.prop_unit_.find(id);
}

const Json::Value &GameConfig::prop_unit(const char* item_name)
{
	return this->mount_config_.prop_unit_.json()[item_name];
}

const Json::Value &GameConfig::change_goder(int grade)
{
	return this->mount_config_.change_goder_.find(grade);
}

const Json::Value &GameConfig::arena(const char* item_name)
{
	return this->arena_cfg_.arena_config_.json_name(item_name);
}

const Json::Value &GameConfig::athletics_base()
{
	return this->arena_cfg_.athletics_base_config_.find(1);
}

const Json::Value &GameConfig::athletics_rank()
{
	return this->arena_cfg_.athletics_rank_config_.json();
}

const Json::Value &GameConfig::athletics_rank_by_id(int id)
{
	return this->arena_cfg_.athletics_rank_config_.find(id);
}

const Json::Value &GameConfig::athletics_rank(int rank_num)
{
	for (int i = GameEnum::ARENA_RANK_END; i >= GameEnum::ARENA_RANK_START; --i)
	{
		const Json::Value &rank_json = this->athletics_rank_by_id(i);
		if (rank_num >= rank_json["rank_num"].asInt())
			return rank_json;
	}
	return Json::Value::null;
}

const Json::Value &GameConfig::collect_chests(const char* item_name)
{
	return this->relax_play_config_.__collect_chests.json_name(item_name);
}

const Json::Value &GameConfig::answer_activity(const char* item_name)
{
	return this->relax_play_config_.__answer_activity.json_name(item_name);
}

void GameConfig::init_server_info()
{
	this->server_info_ = new ServerInfo;

	this->server_info_->__open_server = Time_Value(this->open_server_date());
	this->server_info_->set_server_flag(this->global()["server_flag"].asString());

	MMOServerInfo::load_server_info(this->server_info_);
}

void GameConfig::init_combine_server_flag()
{
	this->server_config_.__is_combine_server = false;

	const std::string& server_flag = this->global()["server_flag"].asString();
	JUDGE_RETURN(GameCommon::is_combine_server_flag(server_flag) == true, ;);

	this->server_config_.__is_combine_server = true;
}

void GameConfig::load_servers_list()
{
	if (this->load_json_config("config/travel/servers_list.json",
			this->travel_config_.__servers_list.revert_json()) == 0)
	{
		this->travel_config_.__servers_list.convert_json_to_map();
		this->travel_config_.__servers_list.update_version();
	}

	for (ConfigMap::iterator iter = this->travel_config_.__servers_list.map().begin();
			iter != this->travel_config_.__servers_list.map().end(); ++iter)
	{
		string flag = (*iter->second)["flag"].asString();
		this->travel_config_.__flag_index_map[flag] = iter->first;
	}

	if (this->load_json_config("config/tiny/channel.json",
			this->tiny_config_.agent_.revert_json()) == 0)
	{
		this->tiny_config_.agent_.update_version();
	}
}

const Json::Value &GameConfig::item(int item_id)
{
	return this->item_config_.item_.find(item_id);
}

const Json::Value &GameConfig::prop(int item_id)
{
	return Json::Value::null;
}

const Json::Value &GameConfig::equip_strengthen(int index, int lvl)
{
	return this->equip_config_.strengthen_[index].find(lvl);
}

const Json::Value &GameConfig::red_uprising(int index, int color_index)
{
	return this->equip_config_.red_uprising[index].find(color_index);
}

const Json::Value &GameConfig::prop_suit(int suit)
{
	return this->equip_config_.prop_suit_.find(suit);
}

const Json::Value &GameConfig::equip_refine(int color)
{
	return this->equip_config_.refine_.find(color);
}

const Json::Value &GameConfig::equip_compse(int id)
{
	return this->equip_config_.compose_.find(id);
}

const Json::Value &GameConfig::jewel_upgrade(int id)
{
	return this->equip_config_.jewel_upgrade_.find(id);
}

const Json::Value &GameConfig::jewel_sublime(int lvl)
{
	return this->equip_config_.jewel_sublime_.find(lvl);
}

GameConfig::ConfigMap &GameConfig::jewel_sublime_map()
{
	return this->equip_config_.jewel_sublime_.map();
}

const Json::Value &GameConfig::item_smelt_exp(int id)
{
	return this->equip_config_.smelt_exp_.find(id);
}

const Json::Value &GameConfig::equip_smelt_level(int id)
{
	return this->equip_config_.smelt_level_.find(id);
}

const Json::Value &GameConfig::red_clothes_exchange(int exchange_id)
{
	return this->equip_config_.red_clothes_exchange_.find(exchange_id);
}

const Json::Value &GameConfig::secret_exchange(int exchange_id)
{
	return this->equip_config_.secret_exchange_.find(exchange_id);
}

const Json::Value &GameConfig::legend_exchange(int exchange_id)
{
	return this->equip_config_.legend_exchange_.find(exchange_id);
}

const Json::Value &GameConfig::reward(int id)
{
	return this->item_config_.reward_.find(id);
}

const Json::Value &GameConfig::shop()
{
	return this->item_config_.shop_.json();
}

const Json::Value &GameConfig::market(int id)
{
    return this->market_cfg_.market_.find(id);
}

const Json::Value &GameConfig::market_const(const char* name)
{
	return this->market_cfg_.market_const_.json_name(name)["value"];
}

const Json::Value &GameConfig::task(void)
{
    return this->task_cfg_.__total_task.json();
}

const Json::Value &GameConfig::task_setting(void)
{
    return this->task_cfg_.__task_setting.json();
}

const Json::Value &GameConfig::find_task(int task_id)
{
	return this->task_cfg_.__total_task.find(task_id);
}

const GameConfig::ConfigMap &GameConfig::all_task_map(void)
{
    return this->task_cfg_.__total_task.map();
}

const GameConfig::ConfigMap &GameConfig::main_task_map(void)
{
    return this->task_cfg_.__main.map();
}

const GameConfig::ConfigMap &GameConfig::branch_task_map(void)
{
    return this->task_cfg_.__branch.map();
}

const BIntSet &GameConfig::npc_task_list(const int npc_id)
{
    IntSetMap &npc_task_map = this->task_cfg_.__npc_task_map[this->task_cfg_.__total_task.current_version()];
    IntSetMap::iterator iter = npc_task_map.find(npc_id);
    if (iter != npc_task_map.end())
        return iter->second;
    return this->null_int_set_;
}

const GameConfig::ConfigMap &GameConfig::league_routine_task_map(void)
{
    return this->task_cfg_.__league_routine.map();
}

const GameConfig::ConfigMap &GameConfig::offer_routine_task_map(void)
{
    return this->task_cfg_.__offer_routine.map();
}

const GameConfig::ConfigMap &GameConfig::new_routine_task_map(void)
{
    return this->task_cfg_.__new_routine.map();
}

const Json::Value &GameConfig::label(int label_id)
{
	return this->achieve_config_.__label.find(label_id);
}

const Json::Value &GameConfig::label_json(void)
{
	return this->achieve_config_.__label.json();
}

const Json::Value &GameConfig::illus(int illus_id)
{
	return this->achieve_config_.__illus.find(illus_id);
}

const Json::Value &GameConfig::illus_json(void)
{
	return this->achieve_config_.__illus.json();
}

const Json::Value &GameConfig::illus_class(int illus_class_id)
{
	return this->achieve_config_.__illus_class.find(illus_class_id);
}

const Json::Value &GameConfig::illus_class_json(void)
{
	return this->achieve_config_.__illus_class.json();
}

const Json::Value &GameConfig::illus_group(int illus_group_id)
{
	return this->achieve_config_.__illus_group.find(illus_group_id);
}

const Json::Value &GameConfig::illus_group_json(void)
{
	return this->achieve_config_.__illus_group.json();
}

const Json::Value &GameConfig::achievement(int achieve_type)
{
	return this->achieve_config_.__achievement.find(achieve_type);
}

const Json::Value &GameConfig::achievement_json(void)
{
	return this->achieve_config_.__achievement.json();
}

const Json::Value &GameConfig::achieve_json(void)
{
	return this->achieve_config_.__new_achieve.json();
}

const Json::Value &GameConfig::achieve_list_json(void)
{
	return this->achieve_config_.__achieve_list.json();
}

const Json::Value &GameConfig::achieve_level(int level)
{
	return this->achieve_config_.__achieve_level.find(level);
}

GameConfig::ConfigMap& GameConfig::achieve_map(void)
{
	return this->achieve_config_.__new_achieve.map();
}

GameConfig::ConfigMap& GameConfig::achieve_list_map(void)
{
	return this->achieve_config_.__achieve_list.map();
}

const Json::Value &GameConfig::rank_pannel(int rank_type)
{
	return this->achieve_config_.__rank_pannel.find(rank_type);
}

const Json::Value &GameConfig::rank_json(void)
{
	return this->achieve_config_.__rank_pannel.json();
}

int GameConfig::font_serial(int index)
{
	return index;
}

FontPair GameConfig::font(int index)
{
    const Json::Value& conf = this->tiny_config_.font_.find(index);

    FontPair pair;
    pair.first = conf["name"].asString();
    pair.second = conf["content"].asString();
    return pair;
}

const Json::Value &GameConfig::common_activity_json(void)
{
	return this->activity_config_.__common_activity.json();
}

const Json::Value &GameConfig::common_activity(int activity_id)
{
	return this->activity_config_.__common_activity.find(activity_id);
}

const Json::Value &GameConfig::brocast(int id)
{
	return this->brocast_config_.__brocast.find(id);
}

const Json::Value &GameConfig::tips_msg_format(int msg_id)
{
	return this->brocast_config_.__tips_msg.find(msg_id);
}

const Json::Value &GameConfig::buff(int id)
{
	return this->item_config_.buff_.find(id);
}

bool GameConfig::validate_buff(int id)
{
	return this->item_config_.buff_.validate(id);
}

const Json::Value &GameConfig::treasures_base_json(const int id)
{
	return this->relax_play_config_.__treasures_base.find(id);
}

const Json::Value &GameConfig::treasures_base_json()
{
	return this->relax_play_config_.__treasures_base.json();
}

const Json::Value &GameConfig::treasures_grid_json(const int id)
{
	return this->relax_play_config_.__treasures_grid.find(id);
}

const Json::Value &GameConfig::treasures_grid_json()
{
	return this->relax_play_config_.__treasures_grid.json();
}

const Json::Value &GameConfig::convoy_json(const int id)
{
	return this->relax_play_config_.__convoy.find(id);
}

const Json::Value &GameConfig::convoy_json()
{
	return this->relax_play_config_.__convoy.find(1);
}

const Json::Value &GameConfig::answer_activity_json(const int id)
{
	return this->relax_play_config_.__answer_activity.find(id);
}

const Json::Value &GameConfig::answer_activity_json()
{
	return this->relax_play_config_.__answer_activity.json();
}

const Json::Value &GameConfig::hotspring_activity_json(const int id)
{
	return this->relax_play_config_.__hotspring_activity.find(id);
}

const Json::Value &GameConfig::hotspring_activity_json()
{
	return this->relax_play_config_.__hotspring_activity.json();
}

const Json::Value &GameConfig::topic_bank_json(const int id)
{
	return this->relax_play_config_.__topic_bank.find(id);
}

const Json::Value &GameConfig::topic_bank_json()
{
	return this->relax_play_config_.__topic_bank.json();
}

const Json::Value &GameConfig::chests_table_json(const int id)
{
	return this->relax_play_config_.__chests_table.find(id);
}

const Json::Value &GameConfig::chests_table_json()
{
	return this->relax_play_config_.__chests_table.json();
}

const Json::Value &GameConfig::collect_chests_json(const int id)
{
	return this->relax_play_config_.__collect_chests.find(id);
}

const Json::Value &GameConfig::collect_chests_json()
{
	return this->relax_play_config_.__collect_chests.json();
}

const Json::Value &GameConfig::welfare_elements_json()
{
	return Json::Value::null;
}

const Json::Value &GameConfig::checkmodel_json(const int id)
{
	return this->welfare_config_.__check_model.find(id);
}

const Json::Value &GameConfig::daycheck_json(int id)
{
	return this->welfare_config_.__day_check.find(id);
}

const Json::Value &GameConfig::totalcheck_json(int id)
{
	return this->welfare_config_.__total_check.find(id);
}

const Json::Value &GameConfig::onlinecheck_json(int id)
{
	return this->welfare_config_.__online_check.find(id);
}

const Json::Value &GameConfig::onlinecheck_json()
{
	return this->welfare_config_.__online_check.json();
}

const Json::Value &GameConfig::restore_json(void)
{
	return this->welfare_config_.__restore.json();
}

const Json::Value &GameConfig::restore_json_by_act_id(int act_id)
{
	const Json::Value &json = this->restore_json();
	for (int i = 1; i <= (int)json.size(); ++i)
	{
		const Json::Value &temp = this->restore_json(i);
		if (temp["act_id"].asInt() == act_id)
			return temp;
	}

	return Json::Value::null;
}

const Json::Value &GameConfig::restore_json(int activity_id)
{
	return this->welfare_config_.__restore.find(activity_id);
}

const Json::Value &GameConfig::exp_restore_json(void)
{
	return Json::Value::null;
}

const Json::Value &GameConfig::exp_restore_json(int activity_id)
{
	return Json::Value::null;
}

int GameConfig::fetch_recharge_id(int gold, int type)
{
	ConfigMap& recharge_map = this->welfare_config_.__recharge.map();
	for (ConfigMap::iterator iter = recharge_map.begin();
			iter != recharge_map.end(); ++iter)
	{
		const Json::Value& json = *iter->second;
		JUDGE_CONTINUE(json["gold"].asInt() == gold && json["type"].asInt() == type);
		return iter->first;
	}
	return 0;
}

void GameConfig::recharge_id_map(IntMap& imap, int type)
{
	ConfigMap& recharge_map = this->welfare_config_.__recharge.map();
	for (ConfigMap::iterator iter = recharge_map.begin();
			iter != recharge_map.end(); ++iter)
	{
		const Json::Value& json = *iter->second;
		JUDGE_CONTINUE(json["type"].asInt() == type);
		imap[iter->first] = true;
	}
}

const Json::Value& GameConfig::recharge_json(int id)
{
	return this->welfare_config_.__recharge.find(id);
}

const Json::Value &GameConfig::rebate_recharge_json()
{
	return this->welfare_config_.__rebate_recharge.find(1);
}

const Json::Value &GameConfig::rebate_recharge_json(int id)
{
	return this->welfare_config_.__rebate_recharge.find(id);
}

const Json::Value &GameConfig::invest_recharge_json()
{
	return this->welfare_config_.__invest_recharge.find(1);
}

const Json::Value &GameConfig::invest_recharge_json(int id)
{
	return this->welfare_config_.__invest_recharge.find(id);
}

const Json::Value &GameConfig::hidden_treasure_json()
{
	return this->welfare_config_.__hidden_treasure.json();
}

const Json::Value &GameConfig::hidden_treasure_json(int day)
{
	return this->welfare_config_.__hidden_treasure.find(day);
}

const Json::Value &GameConfig::operate_activity(int activity_id)
{
	return this->welfare_config_.__operate_activity.find(activity_id);
}

GameConfig::ConfigMap &GameConfig::operate_activity_map()
{
	return this->welfare_config_.__operate_activity.map();
}

const Json::Value &GameConfig::score_exchange(int id)
{
	return this->welfare_config_.__score_exchange.find(id);
}

GameConfig::ConfigMap &GameConfig::score_exchange_map()
{
	return this->welfare_config_.__score_exchange.map();
}

const Json::Value &GameConfig::time_limit_reward(int id)
{
	return this->welfare_config_.__time_limit_reward.find(id);
}

const Json::Value &GameConfig::word_reward(int id)
{
	return this->welfare_config_.__word_reward.find(id);
}

const Json::Value &GameConfig::egg_reward(int id)
{
	return this->welfare_config_.__egg_reward.find(id);
}

GameConfig::ConfigMap &GameConfig::egg_reward_map()
{
	return this->welfare_config_.__egg_reward.map();
}

const Json::Value &GameConfig::open_gift(int location)
{
	return this->welfare_config_.__open_gift.find(location);
}

GameConfig::ConfigMap &GameConfig::open_gift_map()
{
	return this->welfare_config_.__open_gift.map();
}

const Json::Value &GameConfig::daily_recharge_rank(int type)
{
	return this->welfare_config_.__daily_recharge_rank.find(type);
}

GameConfig::ConfigMap &GameConfig::daily_activity_map()
{
	return this->welfare_config_.__daily_activity.map();
}

const Json::Value &GameConfig::total_double(int id)
{
	return this->welfare_config_.__total_double.find(id);
}

const Json::Value &GameConfig::daily_recharge_json()
{
	return this->welfare_config_.__daily_recharge.json();
}

const Json::Value &GameConfig::daily_recharge_json(int id)
{
	return this->welfare_config_.__daily_recharge.find(id);
}

const Json::Value &GameConfig::daily_recharge_by_weeknum(int weeknum)
{
	for (int i = 1; i < 9; ++i)
	{
		const Json::Value &dr_json = this->daily_recharge_json(i);
		if (dr_json["index"].asInt() == weeknum)
			return dr_json;
	}
	return Json::Value::null;
}

const Json::Value &GameConfig::total_recharge(void)
{
	return Json::Value::null;
}

const Json::Value& GameConfig::open_activity_json(void)
{
	return this->activity_config_.__open_activity.json();
}

GameConfig::ConfigMap& GameConfig::open_activity_map(void)
{
	return this->activity_config_.__open_activity.map();
}

GameConfig::ConfigMap& GameConfig::return_activity_map(void)
{
	return this->activity_config_.__return_activity.map();
}

GameConfig::ConfigMap& GameConfig::festival_activity_map(void)
{
	return this->activity_config_.__festival_activity.map();
}

GameConfig::ConfigMap& GameConfig::combine_activity_map(void)
{
	return this->activity_config_.__combine_activity.map();
}

GameConfig::ConfigMap& GameConfig::combine_return_activity_map(void)
{
	return this->activity_config_.__combine_return_activity.map();
}

GameConfig::ConfigMap& GameConfig::may_activity_map(void)
{
	return this->activity_config_.__may_activity.map();
}

const Json::Value &GameConfig::daily_run_item(int id)
{
	return this->activity_config_.__daily_run_item.find(id);
}

GameConfig::ConfigMap& GameConfig::daily_run_item_map(void)
{
	return this->activity_config_.__daily_run_item.map();
}

const Json::Value &GameConfig::daily_run_extra(int id)
{
	return this->activity_config_.__daily_run_extra.find(id);
}

const Json::Value &GameConfig::no_send_act(void)
{
	return this->activity_config_.__no_send_act.find(1);
}

int GameConfig::load_wedding_config(void)
{
    if (this->load_json_config("config/wedding/wedding.json",
    		this->wedding_config_.__wedding.revert_json()) == 0)
    {
        this->wedding_config_.__wedding.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_item.json",
    		this->wedding_config_.__wedding_item.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_item.convert_json_to_map();
        this->wedding_config_.__wedding_item.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_flower.json",
    		this->wedding_config_.__wedding_flower.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_flower.convert_json_to_map();
        this->wedding_config_.__wedding_flower.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_base.json",
    		this->wedding_config_.__wedding_base.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_base.convert_json_to_map();
        this->wedding_config_.__wedding_base.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_property.json",
    		this->wedding_config_.__wedding_property.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_property.convert_json_to_map();
        this->wedding_config_.__wedding_property.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_treasures.json",
    		this->wedding_config_.__wedding_treasures.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_treasures.convert_json_to_map();
        this->wedding_config_.__wedding_treasures.update_version();
    }
    if (this->load_json_config("config/wedding/wedding_label.json",
    		this->wedding_config_.__wedding_label.revert_json()) == 0)
    {
        this->wedding_config_.__wedding_label.convert_json_to_map();
        this->wedding_config_.__wedding_label.update_version();
    }
    return 0;
}

const Json::Value &GameConfig::wedding(void)
{
    return this->wedding_config_.__wedding.json();
}

const Json::Value &GameConfig::wedding_item(const int item_id)
{
	return this->wedding_config_.__wedding_item.find(item_id);
}

const Json::Value &GameConfig::wedding_flower(const int item_id)
{
	return this->wedding_config_.__wedding_flower.find(item_id);
}

const Json::Value &GameConfig::wedding_treasures(int id)
{
	return this->wedding_config_.__wedding_treasures.find(id);
}

const Json::Value &GameConfig::wedding_label(int id)
{
	return this->wedding_config_.__wedding_label.find(id);
}

const Json::Value &GameConfig::wedding_property(int type, int level)
{
	const Json::Value &total_json = this->wedding_config_.__wedding_property.json();
	for (Json::Value::iterator it = total_json.begin(); it != total_json.end(); ++it)
	{
		const Json::Value &temp = (*it);
		JUDGE_CONTINUE(temp["property_type"].asInt() == type && temp["level"].asInt() == level);
		return temp;
	}

	return Json::Value::null;
}

const Json::Value &GameConfig::wedding_base()
{
	return this->wedding_config_.__wedding_base.find(1);
}

const Json::Value &GameConfig::magic_weapon()
{
    return this->item_config_.magic_weapon_.json();
}

const Json::Value &GameConfig::magic_weapon(const int id)
{
	return this->item_config_.magic_weapon_.find(id);
}

const Json::Value &GameConfig::rama_list()
{
    return this->item_config_.rama_list_.json();
}

const Json::Value &GameConfig::rama_list(const int id)
{
	return this->item_config_.rama_list_.find(id);
}

const Json::Value &GameConfig::rama_open()
{
    return this->item_config_.rama_open_.json();
}

const Json::Value &GameConfig::rama_open(const int id)
{
	return this->item_config_.rama_open_.find(id);
}

int GameConfig::load_travel_config(void)
{
	if (this->load_json_config("config/travel/tarena_stage.json",
			this->travel_config_.__tarena_stage.revert_json()) == 0)
	{
		this->travel_config_.__tarena_stage.convert_json_to_map();
		this->travel_config_.__tarena_stage.update_version();
	}

	if (this->load_json_config("config/travel/tarena_fight.json",
			this->travel_config_.__tarena_fight.revert_json()) == 0)
	{
		this->travel_config_.__tarena_fight.convert_json_to_map();
		this->travel_config_.__tarena_fight.update_version();
	}

	if (this->load_json_config("config/travel/tmarena_rank.json",
			this->travel_config_.__tmarena_rank.revert_json()) == 0)
	{
		this->travel_config_.__tmarena_rank.convert_json_to_map();
		this->travel_config_.__tmarena_rank.update_version();
	}

    if (this->load_json_config("config/travel/tbattle_buff.json",
                this->travel_config_.__tbattle_buff.revert_json()) == 0)
    {
        std::map<int, int> &amount_map = this->travel_config_.__tbattle_amount_map[this->travel_config_.__tbattle_buff.prev_version()];
        amount_map.clear();
        for (Json::Value::iterator iter = this->travel_config_.__tbattle_buff.revert_json().begin();
                iter != this->travel_config_.__tbattle_buff.revert_json().end(); ++iter)
        {
            Json::Value *json = &(*iter);
            IntPair pair = GameCommon::to_int_number(iter.key().asString());
            JUDGE_CONTINUE(pair.first == true);
            amount_map[(*json)["amount_range"][0u].asInt()] = pair.second;
        }

        this->travel_config_.__tbattle_buff.convert_json_to_map();
        this->travel_config_.__tbattle_buff.update_version();
    }
    
    if (this->load_json_config("config/travel/tbattle_reward.json",
                this->travel_config_.__tbattle_reward.revert_json()) == 0)
    {
        this->travel_config_.__tbattle_reward.convert_json_to_map();
        this->travel_config_.__tbattle_reward.update_version();
    }

    if (this->load_json_config("config/travel/trvl_peak_base.json",
            this->travel_config_.__peak_base.revert_json()) == 0)
        this->travel_config_.__peak_base.update_version();

    if (this->load_json_config("config/travel/trvl_peak_bet.json",
        	this->travel_config_.__peak_bet.revert_json()) == 0)
    {
        this->travel_config_.__peak_bet.convert_json_to_map();
        this->travel_config_.__peak_bet.update_version();
    }

    return 0;
}

int GameConfig::load_world_boss_config(void)
{
	if (this->load_json_config("config/world_boss/world_boss.json",
	    	this->world_boss_config_.__world_boss.revert_json()) == 0)
	{
		this->world_boss_config_.__world_boss.update_version();
	}
	if (this->load_json_config("config/world_boss/trvl_wboss.json",
		    this->world_boss_config_.__trvl_wboss.revert_json()) == 0)
	{
		this->world_boss_config_.__trvl_wboss.update_version();
	}
	return 0;
}

const Json::Value &GameConfig::tarena_stage(int index)
{
	return this->travel_config_.__tarena_stage.find(index);
}

const Json::Value &GameConfig::tarena_fight(int self, int rivial)
{
	int index = self * 1000 + rivial;
	return this->travel_config_.__tarena_fight.find(index);
}

int GameConfig::tarena_stage_size()
{
	return this->travel_config_.__tarena_stage.map().size();
}

string GameConfig::tarena_state_name(int index)
{
	return this->tarena_stage(index)["name"].asString();
}

GameConfig::ConfigMap &GameConfig::tmarena_rank_map()
{
	return this->travel_config_.__tmarena_rank.map();
}

bool GameConfig::is_updated_travel_list(void)
{
	return this->travel_config_.__is_updated_travel;
}

void GameConfig::reset_updated_travel_list(void)
{
	this->travel_config_.__is_updated_travel = false;
}

const Json::Value &GameConfig::world_boss(const char* item_name)
{
	return this->world_boss_config_.__world_boss.json_name(item_name);
}

const Json::Value &GameConfig::trvl_wboss(const char* item_name)
{
	return this->world_boss_config_.__trvl_wboss.json_name(item_name);
}

const Json::Value& GameConfig::cornucopia_info(int task_id)
{
	return this->cornucopia_config_.__cornucopia_info.find(task_id);
}

GameConfig::ConfigMap& GameConfig::cornucopia_info()
{
	return this->cornucopia_config_.__cornucopia_info.map();
}

int GameConfig::load_cornucopia_config()
{
	if (this->load_json_config("config/activity/cornucopia_info.json",
	    	this->cornucopia_config_.__cornucopia_info.revert_json()) == 0)
	{
		this->cornucopia_config_.__cornucopia_info.convert_json_to_map();
		this->cornucopia_config_.__cornucopia_info.update_version();
	}

	if (this->load_json_config("config/activity/cornucopia_reward.json",
	    	this->cornucopia_config_.__cornucopia_reward.revert_json()) == 0)
	{
		this->cornucopia_config_.__cornucopia_reward.convert_json_to_map();
		this->cornucopia_config_.__cornucopia_reward.update_version();
	}
	return 0;
}

int GameConfig::load_fashion_act_config()
{
	if (this->load_json_config("config/activity/fashion_act_info.json",
	    	this->fashion_act_config_.__faasion_act_info.revert_json()) == 0)
	{
		this->fashion_act_config_.__faasion_act_info.convert_json_to_map();
		this->fashion_act_config_.__faasion_act_info.update_version();
	}
	return 0;
}

int GameConfig::load_labour_act_config()
{
	if (this->load_json_config("config/activity/labour_act_info.json",
	    	this->labour_act_config_.__labour_act_config.revert_json()) == 0)
	{
		this->labour_act_config_.__labour_act_config.convert_json_to_map();
		this->labour_act_config_.__labour_act_config.update_version();
	}

	return 0;
}

const Json::Value& GameConfig::cornucopia_reward(int index)
{
	return this->cornucopia_config_.__cornucopia_reward.find(index);
}

GameConfig::ConfigMap& GameConfig::cornucopia_reward()
{
	return this->cornucopia_config_.__cornucopia_reward.map();
}

const Json::Value& GameConfig::fashion_act_info(int index)
{
	return this->fashion_act_config_.__faasion_act_info.find(index);
}

GameConfig::ConfigMap& GameConfig::fashion_act_info()
{
	return this->fashion_act_config_.__faasion_act_info.map();
}

const Json::Value& GameConfig::labour_act_info(int index)
{
	return this->labour_act_config_.__labour_act_config.find(index);
}

GameConfig::ConfigMap& GameConfig::labour_act_info()
{
	return this->labour_act_config_.__labour_act_config.map();
}

const Json::Value& GameConfig::offline_vip_info(int index)
{
	return this->offline_vip_config_.__offline_vip_config.find(index);
}

GameConfig::ConfigMap& GameConfig::offline_vip_info()
{
	return this->offline_vip_config_.__offline_vip_config.map();
}

int GameConfig::load_offline_vip_config()
{
	if (this->load_json_config("config/role/offine_vip_plus.json",
	    	this->offline_vip_config_.__offline_vip_config.revert_json()) == 0)
	{
		this->offline_vip_config_.__offline_vip_config.convert_json_to_map();
		this->offline_vip_config_.__offline_vip_config.update_version();
	}
	return 0;
}

int GameConfig::load_molding_spirit_config()
{
	if(this->load_json_config("config/equip/molding_attack.json",
	    	this->molding_spirit_config_.__attack.revert_json()) == 0)
	{
		this->molding_spirit_config_.__attack.convert_json_to_map();
		this->molding_spirit_config_.__attack.update_version();
	}

	if(this->load_json_config("config/equip/molding_defense.json",
	    	this->molding_spirit_config_.__defense.revert_json()) == 0)
	{
		this->molding_spirit_config_.__defense.convert_json_to_map();
		this->molding_spirit_config_.__defense.update_version();
	}

	if(this->load_json_config("config/equip/molding_blood.json",
	    	this->molding_spirit_config_.__blood.revert_json()) == 0)
	{
		this->molding_spirit_config_.__blood.convert_json_to_map();
		this->molding_spirit_config_.__blood.update_version();
	}

	if(this->load_json_config("config/equip/molding_all_nature.json",
			this->molding_spirit_config_.__all_nature.revert_json()) == 0)
	{
		this->molding_spirit_config_.__all_nature.convert_json_to_map();
		this->molding_spirit_config_.__all_nature.update_version();
	}

	if(this->load_json_config("config/equip/molding_equip_nature.json",
		this->molding_spirit_config_.__equip_nature.revert_json()) == 0)
	{
		this->molding_spirit_config_.__equip_nature.convert_json_to_map();
		this->molding_spirit_config_.__equip_nature.update_version();
	}

	return 0;
}

const Json::Value& GameConfig::molding_spirit_info(int type, int level, int red_grade)
{
	switch(type)
	{
		case GameEnum::MOLDING_SPIRIT_ATTACK:
		{
			return this->molding_spirit_config_.__attack.find(level);
		}
		case GameEnum::MOLDING_SPIRIT_DEFENSE:
		{
			return this->molding_spirit_config_.__defense.find(level);
		}
		case GameEnum::MOLDING_SPIRIT_BLOOD:
		{
			return this->molding_spirit_config_.__blood.find(level);
		}
		case GameEnum::MOLDING_SPIRIT_ALL_NATURE:
		{
			return this->molding_spirit_all_nature(level, red_grade);
		}
		default:
			return Json::Value::null;
	}
}

const Json::Value& GameConfig::molding_spirit_all_nature(int level, int red_grade)
{
	for(uint i = 1; i <= this->molding_spirit_config_.__all_nature.json().size(); ++i)
	{
		const Json::Value &value = this->molding_spirit_config_.__all_nature.find(i);
		int config_level = value["lv"].asInt();
		int config_grade = value["red_now"].asInt();
		if(config_level == level && config_grade == red_grade)
			return value;
	}
	return Json::Value::null;
}

const Json::Value& GameConfig::molding_spirit_equip_nature(int type)
{
	return this->molding_spirit_config_.__equip_nature.find(type);
}

int GameConfig::molding_spirit_return_sys_nature_id(int type)
{
	switch(type)
	{
		case GameEnum::MOLDING_SPIRIT_ATTACK:
		{
			return GameEnum::ATTACK;
			break;
		}
		case GameEnum::MOLDING_SPIRIT_BLOOD:
		{
			return GameEnum::BLOOD_MAX;
			break;
		}
		case GameEnum::MOLDING_SPIRIT_DEFENSE:
		{
			return GameEnum::DEFENSE;
			break;
		}
		case GameEnum::MOLDING_SPIRIT_ALL_NATURE:
		{
			return 1;
		}
		default:
			return 0;
	}
}

string GameConfig::molding_spirit_return_sys_nature_name(int type)
{
	switch(type)
	{
		case GameEnum::MOLDING_SPIRIT_ATTACK:
		{
			return GameName::ATTACK;

		}
		case GameEnum::MOLDING_SPIRIT_BLOOD:
		{
			return GameName::BLOOD_MAX;
			break;
		}
		case GameEnum::MOLDING_SPIRIT_DEFENSE:
		{
			return GameName::DEFENSE;
			break;
		}
		default:
			return "";
	}
}

const Json::Value &GameConfig::tbattle_buff(const int amount)
{
    return this->travel_config_.__tbattle_buff.find(amount);
}

const Json::Value &GameConfig::tbattle_reward(void)
{
    return this->travel_config_.__tbattle_reward.find(1);
}

int GameConfig::tbattle_player_amount_to_buff_idx(const int amount)
{
    std::map<int, int> &amount_map = this->travel_config_.__tbattle_amount_map[this->travel_config_.__tbattle_buff.current_version()];
    std::map<int, int>::iterator min_iter = amount_map.lower_bound(amount), max_iter = amount_map.upper_bound(amount);
    int key = 1;
    for (std::map<int, int>::iterator iter = min_iter; iter != max_iter; ++iter)
    {
        if (iter->first > amount)
            break;
        key = iter->second;
    }
    return key;
}


const Json::Value& GameConfig::fish_score_reward_info(int index)
{
	return this->fish_score_reward_config_.__fish_score_reward.find(index);
}

GameConfig::ConfigMap& GameConfig::fish_score_reward_info()
{
	return this->fish_score_reward_config_.__fish_score_reward.map();
}

const Json::Value* GameConfig::fish_score_reward_by_day(int index, int day)
{
	GameConfig::ConfigMap& config_map = this->fish_score_reward_info();
	Json::Value *new_config = NULL;
	for(GameConfig::ConfigMap::iterator iter = config_map.begin(); iter != config_map.end(); ++iter)
	{
		Json::Value &value = *(iter->second);
		int config_day = value["day"].asInt();
		JUDGE_CONTINUE(value["index"].asInt() == index);
		if(day >= config_day)
		{
			new_config = &value;
		}
	}
	return new_config;
}

int GameConfig::fish_type_max_count(int day)
{
	int cur_count = 0;
	for(int i = 0; i < 30; ++i)
	{
		const Json::Value *value = this->fish_score_reward_by_day(i, day);
		if(value == NULL)
			break;
		else
			cur_count++;
	}
	return cur_count;
}

int GameConfig::load_fish_type_config()
{
	if(this->load_json_config("config/activity/fish_score_reward.json",
			this->fish_score_reward_config_.__fish_score_reward.revert_json()) == 0)
	{
		this->fish_score_reward_config_.__fish_score_reward.convert_json_to_map();
		this->fish_score_reward_config_.__fish_score_reward.update_version();
	}
	return 0;
}

GameConfig::ConfigMap& GameConfig::goddess_bless_reward_item(void)
{
	return this->goddess_bless_config_.__bless_reward_item.map();
}

GameConfig::ConfigMap& GameConfig::goddess_bless_reward_blessing(void)
{
	return this->goddess_bless_config_.__bless_reward_blessing.map();
}

GameConfig::ConfigMap& GameConfig::goddess_bless_exchange_item(void)
{
	return this->goddess_bless_config_.__bless_exchange_item.map();
}

GameConfig::ConfigMap& GameConfig::goddess_bless_blessing_reward(void)
{
	return this->goddess_bless_config_.__blessing_reward.map();
}

int GameConfig::load_goddess_bless_config(void)
{
	if(this->load_json_config("config/activity/bless_exchange_item.json",
	    	this->goddess_bless_config_.__bless_reward_item.revert_json()) == 0)
	{
		this->goddess_bless_config_.__bless_reward_item.convert_json_to_map();
		this->goddess_bless_config_.__bless_reward_item.update_version();
	}

	if(this->load_json_config("config/activity/bless_reward_blessing.json",
	    	this->goddess_bless_config_.__bless_reward_blessing.revert_json()) == 0)
	{
		this->goddess_bless_config_.__bless_reward_blessing.convert_json_to_map();
		this->goddess_bless_config_.__bless_reward_blessing.update_version();
	}

//	if(this->load_json_config("config/activity/bless_exchange_item.json",
//	    	this->goddess_bless_config_.__bless_exchange_item.revert_json()) == 0)
//	{
//		this->goddess_bless_config_.__bless_exchange_item.convert_json_to_map();
//		this->goddess_bless_config_.__bless_exchange_item.update_version();
//	}

	if(this->load_json_config("config/activity/blessing_reward.json",
	    	this->goddess_bless_config_.__blessing_reward.revert_json()) == 0)
	{
		this->goddess_bless_config_.__blessing_reward.convert_json_to_map();
		this->goddess_bless_config_.__blessing_reward.update_version();
	}

	return 0;
}

const Json::Value &GameConfig::travel_peak_base(void)
{
	return this->travel_config_.__peak_base.json();
}

const Json::Value &GameConfig::travel_peak_bet(void)
{
    return this->travel_config_.__peak_bet.json();
}

const Json::Value &GameConfig::travel_peak_bet(const int promot_turns)
{
    Json::Value *json = NULL;
    if (this->travel_config_.__peak_bet.map().find(promot_turns, json) == 0)
        return *json;
    return Json::Value::null;
}

int GameConfig::load_special_box_config()
{
	if(this->load_json_config("config/special_box/box.json",
	    	this->special_box_config_.__special_box_item.revert_json()) == 0)
	{
		this->special_box_config_.__special_box_item.convert_json_to_map();
		this->special_box_config_.__special_box_item.update_version();
	}

	if(this->load_json_config("config/special_box/change_item.json",
	    	this->special_box_config_.__special_box_change_item.revert_json()) == 0)
	{
		this->special_box_config_.__special_box_change_item.convert_json_to_map();
		this->special_box_config_.__special_box_change_item.update_version();
	}

	return 0;
}

GameConfig::ConfigMap& GameConfig::special_box_item()
{
	return this->special_box_config_.__special_box_item.map();
}

GameConfig::ConfigMap& GameConfig::special_box_change_item()
{
	return this->special_box_config_.__special_box_change_item.map();
}
