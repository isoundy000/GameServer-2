/*
 * CenterPlay800.cpp
 *
 *  Created on: May 6, 2016
 *      Author: root
 */

#include <SimpleHTTPClient.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "json/json.h"
#include "GameHeader.h"
#include "GameConfig.h"
#include "LogicMonitor.h"
#include "CenterUnit.h"
#include "ProtoDefine.h"

#include "CenterPlay800.h"
#include "MMORole.h"
CenterPlay800::CenterPlay800()
{
	this->time_out_ = 0;
	this->init_ = false;
	this->port_ = 80;
}
int CenterPlay800::init()
{
	const Json::Value &channel_json = CONFIG_INSTANCE->channel_ext("play800");

	if(channel_json == Json::Value::null)
	{
		MSG_USER("tiny/channel_txt.json[\"play800\"] config error");
		return 0;
	}

	std::string address = channel_json["address"].asString();

	struct hostent *ht = ::gethostbyname(address.c_str());
	if (ht == 0)
	{
		LOG_USER_INFO("connect [%s] error!!!", address.c_str());
		return 0;
	}
	if (ht->h_addr_list[0] != 0)
	{
		char buf[65];
		::memset(buf, 0, sizeof(buf));
		::inet_ntop(ht->h_addrtype, ht->h_addr_list[0], buf, sizeof(buf));
		this->host_ = buf;
	}else
	{
		LOG_USER_INFO("connect [%s] error!!!", address.c_str());
		return 0;
	}

	this->port_ = channel_json["port"].asInt();
	this->time_out_ = channel_json["time_out"].asInt();
	this->url_ = channel_json["url"].asString();
	httpcli_.init(host_,port_,time_out_);

	this->init_ = true;
	return 0;
}
CenterPlay800::~CenterPlay800() {
	// TODO Auto-generated destructor stub
}


int CenterPlay800::income_log_get(Message *msg)
{
	JUDGE_RETURN(init_ == true,-1);
	DYNAMIC_CAST_RETURN(Proto32101107 *, request, msg, -1);
	const Json::Value &channel_json = CONFIG_INSTANCE->channel_ext("play800");
	char buf[2048] = {0};
	char *query_args = buf;

	time_t cur_time = ::time(NULL);
	char time_str[64] = {0};
	::sprintf(time_str, "%ld", cur_time);

	string sign_str;
	generate_md5((channel_json["site"].asString()+time_str+channel_json["key"].asString()).c_str(), sign_str );

	string sys_model;
	string market;
	MMORole::load_sys_model(request->role_id(),sys_model,market);
	string aid ;
	if(channel_json["aid"][market] != Json::Value::null)
		aid = channel_json["aid"][market].asString();
	int gold = request->own_gold();
	int income_gold = request->income_gold();
	snprintf(query_args, sizeof(buf), "site=%s&vtime=%s&sign=%s&gid=%s&sid=%d&aid=%s&uid=%s&username=%s"
			"&roleid=%ld&rolename=%s&level=%d&device_type=%s"
			"&order_id=%s&income_channel=%s"
			"&income_currency=%s&income_money=%s&income_gold=%d"
			"&own_gold=%d&income_status=%s&time=%ld",
			channel_json["site"].asCString(),time_str,
			sign_str.c_str(),channel_json["gid"].asCString(), request->sid(), aid.c_str(), request->uid().c_str(),request->username().c_str(),
			request->role_id(),request->role_name().c_str(),request->level(),sys_model.c_str(),
			request->order_id().c_str(),request->income_channel().c_str(),
			channel_json["income_currency"].asCString(),request->income_money().c_str(),income_gold,
			gold,request->income_status()==0?"success":"error",request->time());

	std::string http_respond;
	MSG_USER("http_get:%s",query_args);
	httpcli_.get(channel_json["url"].asString(), query_args, http_respond);
	MSG_USER("http_respond:%s",http_respond.c_str());
	return 0;
}
