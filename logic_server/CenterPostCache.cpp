/*
 * CenterPostCache.cpp
 *
 *  Created on: 2014年1月20日
 *      Author: xing
 */

#include <CenterPostCache.h>
#include <SimpleHTTPClient.h>
#include "json/json.h"
#include "GameHeader.h"
#include "GameConfig.h"
#include "LogicMonitor.h"
#include "CenterUnit.h"
#include "ProtoDefine.h"

void CenterRoleDetail::reset(void)
{
	this->__name.clear();
	this->__account.clear();
    this->__agent.clear();
    this->__platform.clear();
	this->__id = 0;
	this->__level = 0;
	this->__sex = 0;
    this->__server_flag.clear();
}

CenterPostCache::CenterPostCache() :
		post_once_limit_(128), initialize_(false)
{
	// TODO Auto-generated constructor stub
}

int CenterPostCache::init()
{
	const Json::Value &server_json = CONFIG_INSTANCE->global()["php_center"];
	const Json::Value &limit_json = CONFIG_INSTANCE->php_center_limit();

	if(limit_json == Json::Value::null)
	{
		MSG_EXIT("php_center_limit config error");
	}

	httpcli_.init( server_json["php_center_host"].asString(),
			server_json["php_center_port"].asUInt(),
			limit_json["php_center_timeout"].asInt() );

//	post_url_ = server_json["php_center_url"].asString();
	php_url_ = server_json["php_url"].asString();

	post_once_limit_ = limit_json["php_center_post_limit"].asInt();

	server_flag_ = CONFIG_INSTANCE->global()["server_flag"].asString();
	generate_md5( (server_json["php_center_key"].asString()+server_flag_).c_str(),
				post_sign_ );

	initialize_ = true;
	return 0;
}

CenterPostCache::~CenterPostCache()
{
	// TODO Auto-generated destructor stub
}

bool CenterPostCache::is_need_post(void)
{

#ifdef LOCAL_DEBUG
	// test code
	const Json::Value &limit_json = CONFIG_INSTANCE->php_center_limit();

	if( limit_json.isMember("php_center_insert_test") &&
			limit_json["php_center_insert_test"].asInt() == 1 &&
			role_detail_map_.size() == 0)
	{
		Proto32101101 req;
		char buf[128] =
		{	0};

		int insert_num = limit_json["php_center_insert_test_num"].asInt();
		std::string role_name_prefix = limit_json["php_center_insert_test_name_prefix"].asString();
		MSG_DEBUG("insert test data %d",insert_num );

		for(Int64 i=0; i<insert_num; i++)
		{
			req.Clear();
			snprintf(buf, 128, "account_%lld", i<<32);
			req.set_account(buf);
			snprintf(buf, 128, "%s%lld", role_name_prefix.c_str(), i);
			req.set_role_name(buf);
			req.set_role_id(i);
			req.set_level(i%100);
			req.set_agent("devtest");
			req.set_sex((i%2)+1);
			CENTER_POST_CACHE->insert_role_detail(&req);
		}
	}
	// test code end
#endif

	return (role_detail_map_.size() >= post_once_limit_);
}

int CenterPostCache::insert_role_detail(CenterRoleDetail *detail)
{
	JUDGE_RETURN(initialize_, -1);
	JUDGE_RETURN(detail != NULL, -1);
	role_detail_map_[detail->__id] = detail;
	return 0;
}

int CenterPostCache::insert_role_detail(Message *msg)
{
	JUDGE_RETURN(initialize_, -1);
	MSG_DYNAMIC_CAST_RETURN(Proto32101101*, request, -1);

	CenterRoleDetail *detail = role_detail_pool_.pop();
	JUDGE_RETURN(detail != NULL, -1);

//	MSG_DEBUG("%s", request->Utf8DebugString().c_str());
	detail->__account = request->account();
	detail->__name = request->role_name();
	detail->__id = request->role_id();
	detail->__level = request->level();
	detail->__sex = request->sex();
	detail->__agent = request->agent();
	detail->__platform = request->platform();
    detail->__server_flag = request->server_flag();

	return insert_role_detail(detail);
}

int CenterPostCache::post2center_role_all(bool retry)
{
	JUDGE_RETURN(initialize_, -1);
	JUDGE_RETURN(this->post_once_limit_ > 0, -1);

	int max_loop = (role_detail_map_.size() / post_once_limit_) * 2; // 限制循环次数
	for (int i = 0; !role_detail_map_.empty() && i <= max_loop &&
	LOGIC_MONITOR->center_unit()->is_running(); i++)
	{
		if(post2center_role_once(retry) != 0)
			break;
//		MSG_DEBUG("role_detail_map_.size() %d", role_detail_map_.size());
	}
	return 0;
}

int CenterPostCache::post2center_role_once(bool retry)
{
	JUDGE_RETURN(initialize_, -1);

    std::map<std::string, Json::Value > post_json_map;
	Json::Value detail_json;
	static char buf[256] = { 0 };
	std::vector<Int64> post_id_list;

	if (role_detail_map_.empty())
		return 0;

	uint i = 0;
	for (ID_Detail_map::const_iterator it = role_detail_map_.begin();
			it != role_detail_map_.end() && i < post_once_limit_; it++, i++)
	{
		// 此处必须严格按照顺序
		post_id_list.push_back(it->first);
		CenterRoleDetail *detail = it->second;
		detail_json.clear();
		detail_json.append(detail->__account);
		detail_json.append((int) (detail->__id >> 32)); // 高32位
		detail_json.append((int) detail->__id); // 低32位
		detail_json.append(detail->__name);
		detail_json.append(detail->__level);
		detail_json.append(detail->__sex);
		detail_json.append(detail->__platform);
		detail_json.append(detail->__agent);

        Json::Value &post_json = post_json_map[detail->__server_flag];
		post_json.append(detail_json);
	}

	MSG_DEBUG("left role: %d, POST role: %d",
			role_detail_map_.size() - post_id_list.size(), post_id_list.size());

    int httpcode = 0;
    bool is_http_error = false;
    const Json::Value &server_json = CONFIG_INSTANCE->global()["php_center"];
    std::string php_center_key = server_json["php_center_key"].asString(), post_sign;
    for (std::map<std::string, Json::Value >::iterator iter = post_json_map.begin();
            iter != post_json_map.end(); ++iter)
    {
        generate_md5((php_center_key + iter->first).c_str(), post_sign);
        char *post_prev_args = buf;
	    snprintf(post_prev_args, sizeof(buf), "server=%s&sign=%s&data=",
                iter->first.c_str(), post_sign.c_str());

	    httpcode = httpcli_.post(php_url()+"?r=levelinfo",
                post_prev_args + Json::FastWriter().write(iter->second), retry);

        if (httpcode / 100 != 2 && httpcode != 0 && retry) // 成功 or 指定失败不重试
        {
            is_http_error = true;
            break;
        }
    }

	if (is_http_error == false) // 成功 or 指定失败不重试
	{
		for (std::vector<Int64>::iterator it = post_id_list.begin();
				it != post_id_list.end(); it++)
		{
			ID_Detail_map::iterator find_it = role_detail_map_.find(*it);
			if (find_it != role_detail_map_.end())
			{
				role_detail_pool_.push(find_it->second);
				role_detail_map_.erase(find_it);
			}
		}
		return 0;
	}
	else
	{
		MSG_USER("POST to center error, http status code: %d", httpcode);
		return -1;
	}
}

int CenterPostCache::query_center_acti_code(const std::string &acti_code)
{
	JUDGE_RETURN(initialize_, -1);
	static char buf[256] = {0};

	char *query_args = buf;

	int cur_time = ::time(0);
	char time_str[32] = {0};
	::sprintf(time_str, "%d", cur_time);

	string sign_str;
	const Json::Value &server_json = CONFIG_INSTANCE->global()["php_center"];
	generate_md5((time_str+server_json["php_center_key"].asString()).c_str(), sign_str );

	snprintf(query_args, sizeof(buf), "r=checkcode&type=%d&code=%s&time=%d&sign=%s",
			1, acti_code.c_str(), cur_time, sign_str.c_str());

	MSG_USER("center req: %s/%s", php_url().c_str(), query_args);

	std::string http_respond;
	int ret = httpcli_.get(php_url(), query_args, http_respond);
	JUDGE_RETURN(0 == ret, -1);

	if(http_respond.empty() == false && http_respond[0] >= '0' && http_respond[0] <= '9')
		return ::atoi(http_respond.c_str());
	else
		return 1;
}

int CenterPostCache::sync_sdk_info(Message *msg)
{
    DYNAMIC_CAST_RETURN(Proto32101102 *, request, msg, -1);

    const Json::Value &channel_ext_json = CONFIG_INSTANCE->channel_ext(request->channel());
    JUDGE_RETURN(channel_ext_json != Json::Value::null, -1);

    char buf[2048];
    ::memset(buf, 0, sizeof(buf));

    std::string zone_name = CONFIG_INSTANCE->global()["zone_name"].asString(),
        center_url = channel_ext_json["url"].asString();
    int zone_id = CONFIG_INSTANCE->global()["zone_id"].asInt();

    ::snprintf(buf, sizeof(buf), channel_ext_json["data"].asCString(),
            request->uc_sid().c_str(), request->role_id(), 
            request->role_name().c_str(), request->role_level(),
            zone_name.c_str(), zone_id);
    buf[sizeof(buf) - 1] = '\0';

    MSG_USER("sdk info. url://%s %s", center_url.c_str(), buf);

    int httpcode = this->httpcli_.post(center_url, buf, true);
    if (httpcode / 100 != 2)
    {
        MSG_USER("sdk info error. url://%s %s", center_url.c_str(), buf);

        return -1;
    }

    return 0;
}

std::string& CenterPostCache::php_url(void)
{
	return this->php_url_;
}

int CenterPostCache::query_center_del_return_recharge(Int64 role_id, const std::string& account)
{
	JUDGE_RETURN(initialize_, -1);
	static char buf[256] = {0};

	char *query_args = buf;

	time_t cur_time = ::time(NULL);
	char time_str[64] = {0};
	::sprintf(time_str, "%ld", cur_time);

	string sign_str;
	const Json::Value &server_json = CONFIG_INSTANCE->global()["php_center"];
	generate_md5((time_str+server_json["php_center_key"].asString()).c_str(), sign_str );

	snprintf(query_args, sizeof(buf), "r=rebate&account=%s&id=%lld&server=%s&time=%ld&sign=%s",
			account.c_str(), role_id, this->server_flag_.c_str(), cur_time, sign_str.c_str());

	std::string http_respond;
	int ret = httpcli_.get(php_url(), query_args, http_respond);
	MSG_DEBUG("http_respond:%s",http_respond.c_str());
	JUDGE_RETURN(0 == ret, -1);
	JUDGE_RETURN(http_respond.empty() == false, -1);
	return ::atoi(http_respond.c_str());
}
