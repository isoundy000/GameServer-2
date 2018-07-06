/*
 * GatePlayer.cpp
 *
 * Created on: 2013-04-16 10:41
 *     Author: lyz
 */

#include "GatePlayer.h"
#include "GateMonitor.h"
#include "SessionManager.h"
#include "ProtoDefine.h"
#include "SerialRecord.h"
#include "GameCommon.h"

GatePlayer::CachedTimer::CachedTimer(void) : player_(0)
{ /*NULL*/ }

GatePlayer::CachedTimer::~CachedTimer(void)
{ /*NULL*/ }

int GatePlayer::CachedTimer::type(void)
{
	return GTT_GATE_PLAYER;
}

int GatePlayer::CachedTimer::handle_timeout(const Time_Value &nowtime)
{
//	this->player_->check_accelerate(nowtime);

	return 0;
}

GatePlayer::GatePlayer(void) : 
    role_id_(0), client_sid_(0), line_id_(0), is_active_(false),
    client_port_(0), alive_amount_(0), ban_ip_(0),
    ban_type_(0), ban_expire_time_(0),ban_mac_(0), is_loading_mongo_(false)
{
    this->monitor_ = GATE_MONITOR;
    this->cached_timer_.player_ = this;
    this->agent_ = "";
}

GatePlayer::~GatePlayer(void)
{ /*NULL*/ }

void GatePlayer::reset(void)
{
    this->role_id_ = 0;
    this->client_sid_ = 0;
    this->line_id_ = 0;

    this->is_active_ = false;

    this->role_detail_.reset();
    this->online_.reset();

    this->client_ip_.clear();
    this->client_port_ = 0;
    this->prev_alive_tick_ = Time_Value::zero;
    this->alive_amount_ = 0;
    this->ban_ip_ = 0;
    this->ban_type_ = 0;
    this->ban_expire_time_ = 0;
    this->ban_mac_ = 0;
    this->is_loading_mongo_ = false;
    this->agent_ = "";
}

GateMonitor *GatePlayer::monitor(void)
{
    return this->monitor_;
}

void GatePlayer::set_agent(const char *agent)
{
	char buff[MAX_AGENT_LENGTH] = {0};
    int len = std::min<int>(strlen(agent), MAX_AGENT_LENGTH);
    for (int i = 0; i < len; ++i)
    {
    	if ('A' <= agent[i] && agent[i] <= 'Z')
    		buff[i] = (agent[i] - 'A' + 'a');
    	else
    		buff[i] = agent[i];
    }

    this->role_detail_.__agent = buff;
    this->role_detail_.__agent_code = CONFIG_INSTANCE->agent_code(this->role_detail_.__agent);
}

const char *GatePlayer::agent(void)
{
	return this->role_detail_.__agent.c_str();
}

int GatePlayer::agent_code(void)
{
    return this->role_detail_.__agent_code;
}

void GatePlayer::set_market_code(int code)
{
	this->role_detail_.__market_code = code;
}

int GatePlayer::market_code(void)
{
	return this->role_detail_.__market_code;
}

void GatePlayer::set_account(const char *account)
{
	this->role_detail_.__account = account;
}

const char *GatePlayer::account(void)
{
    return this->role_detail_.__account.c_str();
}

const char* GatePlayer::name()
{
	return this->role_detail_.name();
}

void GatePlayer::set_session(const char *session)
{
    this->session_ = session;
}

std::string &GatePlayer::session(void)
{
    return this->session_;
}

void GatePlayer::set_active(const bool flag)
{
    this->is_active_ = flag;
}

bool GatePlayer::is_active(void)
{
    return this->is_active_;
}

void GatePlayer::set_role_id(const int64_t id)
{
    this->role_id_ = id;
}

Int64 GatePlayer::role_id(void)
{
    return this->role_id_;
}

int64_t GatePlayer::entity_id(void)
{
    return this->role_id_;
}

void GatePlayer::set_line_id(const int line_id)
{
	this->line_id_ = line_id;
}

int GatePlayer::line_id(void)
{
	return this->line_id_;
}

void GatePlayer::set_client_sid(const int sid)
{
    this->client_sid_ = sid;
}

int GatePlayer::client_sid(void)
{
    return this->client_sid_;
}

void GatePlayer::set_client_ip(const std::string &ip)
{
    this->client_ip_ = ip;
}

void GatePlayer::set_client_port(const int port)
{
    this->client_port_ = port;
}

const std::string &GatePlayer::client_ip(void)
{
    return this->client_ip_;
}

int GatePlayer::client_port(void)
{
    return this->client_port_;
}

void GatePlayer::set_alive_amount(const int amount)
{
	this->alive_amount_ = amount;
}

int GatePlayer::alive_amount(void)
{
	return this->alive_amount_;
}

void GatePlayer::set_ban_state(int ban_type, const int64_t expired_time)
{
	JUDGE_RETURN((ban_type == GameEnum::OPER_BAN_ACCOUNT) || (ban_type == GameEnum::OPER_BAN_ROLE)
			|| (ban_type == GameEnum::OPER_BAN_NONE) ||	(ban_type == GameEnum::OPER_BAN_IP)
			|| (ban_type == GameEnum::OPER_BAN_MAC), ;);

	if (expired_time < ::time(0))
	{
		ban_type = GameEnum::OPER_BAN_NONE;
	}

	this->ban_type_ = ban_type;
	this->ban_expire_time_ = expired_time;
}

int GatePlayer::ban_type(void)
{
	return this->ban_type_;
}

int64_t GatePlayer::ban_expried_time(void)
{
	return this->ban_expire_time_;
}

void GatePlayer::set_ban_ip(bool is_ban)
{
	this->ban_ip_ = (int)is_ban;
}

bool GatePlayer::is_ban_ip(void)
{
	return this->ban_type_ == GameEnum::OPER_BAN_IP;
}

bool GatePlayer::is_ban_login(void)
{
	return this->ban_type_ != GameEnum::OPER_BAN_NONE
			&& this->ban_expire_time_ > ::time(0);
}

GateRoleDetail &GatePlayer::detail(void)
{
    return this->role_detail_;
}

int GatePlayer::start_game(void)
{
    MSG_USER("start game %ld %s %d %d online[%d] %x", this->role_id(), this->name(),
    		this->client_sid(), this->detail().__scene_id, this->online_.online_detail().__is_online, this);

    JUDGE_RETURN(this->online_.online_detail().__is_online == 0, -1);

    if (this->monitor()->bind_player_by_role_id(this->role_id(), this) != 0)
    {
    	MSG_USER("ERROR player repeate bind %d %s %ld %s", this->client_sid(), this->account(), this->role_id(), this->name());
    	return -1;
    }
    this->monitor()->bind_sid_by_role_id(this->role_id(), this->client_sid());

    this->set_active(true);
    this->prev_alive_tick_ = Time_Value::gettimeofday();
    this->set_alive_amount(0);
    this->cached_timer_.schedule_timer(Time_Value(1));

    this->online_.sign_in();

    Proto30100101 logic_request;
    Proto30400004 map_request;

    GameCommon::make_up_session(logic_request.mutable_session_info(), this->account());
    GameCommon::make_up_session(map_request.mutable_session_info(), this->account());

    ProtoHead head;
    head.__src_line_id = 0;
    head.__role_id = this->role_id();
   
    head.__recogn = INNER_LOGIC_LOGIN;
    head.__scene_id = SCENE_LOGIC;
    logic_request.set_scene_id(this->detail().__scene_id);
    logic_request.set_uc_sid(this->role_detail_.__uc_sid);
    logic_request.set_client_ip(this->client_ip_);
    logic_request.set_client_mac(this->detail().__mac);
    this->monitor()->dispatch_to_scene(&head, &logic_request);

    head.__recogn = INNER_MAP_LOGIN;
    head.__scene_id = this->role_detail_.__scene_id;
    head.__src_line_id = this->monitor()->fetch_line_id_by_scene(head.__role_id, head.__scene_id);
    this->monitor()->dispatch_to_scene(&head, &map_request);
    
    return 0;
}

int GatePlayer::stop_game(int reason, const string& desc)
{
	this->set_active(false);
	this->cached_timer_.cancel_timer();
    this->online_.sign_out();

    // 判断是否要记录登录流水, 当前登录的玩家对象与当前断线的玩家对象相同时要记录流水
    GatePlayer *account_player = this->monitor()->find_account_player(this->account());
    if (account_player != NULL && account_player == this)
    {
	    SERIAL_RECORD->record_login_logout(this->role_id(), this->name(),
	    		reason, this->account(), this->client_ip(),
	    		this->online().online_detail().__sign_in_tick,
	    		this->online().online_detail().__sign_out_tick,
	    		this->agent_code(), this->market_code(),
	    		this->role_detail_.__sys_model,
	    		this->role_detail_.__sys_version,
	    		this->role_detail_.__mac);
    }

    // 当前帐号只有这个对象登录
    if (account_player == NULL || account_player == this)
    {
//        SessionDetail *session = 0;
//        if (SESSION_MANAGER->unbind_account_session(this->detail().__account, session) == 0)
//        {
//            session->cancel_timer();
//            SESSION_MANAGER->session_pool()->push(session);
//        }

        SESSION_MANAGER->update_session_time(this->account(), 3 * Time_Value::MINUTE);

        ProtoHead head;
        head.__role_id = this->role_id();
   
        head.__recogn = INNER_LOGIC_LOGOUT;
        head.__scene_id = SCENE_LOGIC;
        this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

        head.__recogn = INNER_MAP_LOGOUT;
        head.__scene_id = this->role_detail_.__scene_id;
        this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

        head.__recogn = INNER_MAP_LOGIC_LOGOUT;
        head.__scene_id = this->role_detail_.__scene_id;
        this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

        head.__recogn = INNER_CHAT_LOGOUT;
        head.__scene_id = this->monitor()->chat_scene(this->role_detail_.__scene_id);
        this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

        {
            Block_Buffer body;
            body << this->monitor()->gate_scene();
            this->monitor()->dispatch_to_auth(this->monitor()->auth_sid(), INNER_GATE_CLOSE_ROLE, &body);
        }

        // 注销role_id与player对象的绑定关系
        this->monitor()->unbind_player_by_role_id(this->role_id());
        // 注销帐号与player对象的绑定关系
        this->monitor()->unbind_account_player(this->account());

        // 注销client sid 与 此帐号的绑定关系, 判断防止被回收的socket在回收前就被重新使用注册
        std::string account;
        if (this->monitor()->find_sid_account(this->client_sid(), account) == 0 && account == this->account())
        {
            this->monitor()->unbind_sid_account(this->client_sid());
        }

        account = this->account();
        if (this->monitor()->find_account_sid(account) == this->client_sid())
        {
            this->monitor()->unbind_account_sid(account);
        }
    }

    IntPair error_tips;
    Proto50999999 respond;

    if (reason == GateMonitor::STOP_SERVER)
	{
		//关服
		error_tips.second = 2;
		respond.set_drop_reason(error_tips.second);
		this->monitor()->dispatch_to_client(this->client_sid(),
						RETURN_SERVER_KEEP_ALIVE, 0, &respond);
	}
    else if (reason > 0 && reason < GateMonitor::SOCK_DISCOUNT)
    {
    	//GM封禁
    	error_tips.second = 3;
        respond.set_ban_type(reason);
        respond.set_ban_str(desc);

        respond.set_drop_reason(error_tips.second);
		this->monitor()->dispatch_to_client(this->client_sid(),
				RETURN_SERVER_KEEP_ALIVE, 0, &respond);
    }
    else
    {
    	//顶号和网络断开，不提示
    	error_tips.second = 1;
    }

    // 注销角色ID与client sid的绑定关系
    if (this->monitor()->find_sid_by_role_id(this->role_id()) == this->client_sid())
    {
        this->monitor()->unbind_sid_by_role_id(this->role_id());
    }

    // 注销client sid与player对象的绑定关系
    GatePlayer *sid_player = NULL;
    if (this->monitor()->find_player_by_sid(this->client_sid(), sid_player) == 0 && sid_player == this)
    {
        this->monitor()->unbind_sid_player(this->client_sid());
    }
    
    MSG_USER("stop game %ld name:%s account:%s level:%d scene:%d tips:%d reason:%d sid:%d addr:%x",
    		this->role_id(), this->name(), this->account(), this->role_detail_.__level,
    		this->role_detail_.__scene_id, error_tips.first, reason, this->client_sid(), this);
    return this->monitor()->player_pool()->push(this);
}

int GatePlayer::update_map_sid()
{
	JUDGE_RETURN(this->is_active() == true, -1);

    ProtoHead head;
    head.__role_id = this->role_id();

    head.__recogn = INNER_MAP_UPDATE_SID;
    head.__scene_id = this->role_detail_.__scene_id;
    this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

    head.__recogn = INNER_ML_UPDATE_SID;
    head.__scene_id = this->role_detail_.__scene_id;
    this->monitor()->dispatch_to_scene(&head, (Block_Buffer *)0);

    MSG_USER("%ld, %s, %d", head.__role_id, this->name(), head.__scene_id);
    return 0;
}

int GatePlayer::sync_role_info(Message *msg_proto)
{
    Proto30400201 *request = dynamic_cast<Proto30400201 *>(msg_proto);
    JUDGE_RETURN(request != NULL, -1);

//    if (request->prev_scene() > 0 && request->prev_scene() != this->detail().__scene_id)
//    {
//    	MSG_USER("sync scene error %ld %s %d %d %d", this->role_id(), this->detail().__name,
//    			this->detail().__scene_id, request->prev_scene(), request->scene_id());
//    	return -1;
//    }

    int prev_level = this->detail().__level;

    this->detail().__force = request->force();
    this->detail().__camp_id = request->camp_id();
    this->detail().__career = request->career();
    this->detail().__scene_id = request->scene_id();
    this->detail().__scene_mode = request->scene_mode();
    this->detail().__space_id = request->space_id();
    this->detail().__level = request->level();
 //   this->detail().__permission = request->permission();

    if(this->detail().__level > prev_level)
    {
    	this->level_upgrade(prev_level);
    }
    return 0;
}

int GatePlayer::reqeuest_login_logic(void)
{
    MSG_USER("logic restart %s %ld %d", this->name(), this->role_id(), this->detail().__scene_id);

    Proto30100101 logic_request;
    GameCommon::make_up_session(logic_request.mutable_session_info(), this->account());

    logic_request.set_scene_id(this->detail().__scene_id);
    logic_request.set_uc_sid(this->role_detail_.__uc_sid);
    logic_request.set_client_ip(this->client_ip());
    logic_request.set_client_mac(this->detail().__mac);

    ProtoHead head;
    head.__role_id = this->role_id(); 
    head.__recogn = INNER_LOGIC_LOGIN;
    head.__scene_id = SCENE_LOGIC;
    return this->monitor()->dispatch_to_scene(&head, &logic_request);
}

LogicOnline &GatePlayer::online(void)
{
    return this->online_;
}

int GatePlayer::respond_to_client_error(const int recogn, const int error, const Message *msg_proto)
{
    if (msg_proto == 0)
        return this->monitor()->dispatch_to_client(this->client_sid(), recogn, error);
    return this->monitor()->dispatch_to_client(this->client_sid(), msg_proto);
}

int GatePlayer::sync_update_player_name(Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto31400020 *, request, -1);
    this->role_detail_.set_name(request->role_name());
    return 0;
}

int GatePlayer::check_accelerate(const Time_Value &nowtime)
{
#ifndef LOCAL_DEBUG
	JUDGE_RETURN(this->prev_alive_tick_ < nowtime, -1);

	//心跳包5s一次
	int check_flag = CONFIG_INSTANCE->const_set("accelerate_validate");
	JUDGE_RETURN(check_flag == 1, 0);

	int alive_amount = this->alive_amount();
	JUDGE_RETURN(alive_amount > 2, 0);

	Time_Value diff_time = nowtime - this->prev_alive_tick_;

	double diff_sec = diff_time.sec() + diff_time.usec() / 1000000.0;
	double times = 5 * alive_amount / std::max<double>(diff_sec, 0.01);

	this->set_alive_amount(0);
	this->prev_alive_tick_ = nowtime;
	JUDGE_RETURN(times > 1.4, 0);

	// 检查到有加速
	Proto30400053 inner_req;
	inner_req.set_role_id(this->role_id());

	ProtoHead proto_head;
	proto_head.__role_id = this->role_id();
	proto_head.__recogn = INNER_MAP_ACCELERATE_FORBIT;
	proto_head.__scene_id = this->detail().__scene_id;
	proto_head.__src_line_id = this->monitor()->fetch_line_id_by_scene(
			proto_head.__role_id, proto_head.__scene_id);
	this->monitor()->dispatch_to_scene(&proto_head, &inner_req);
	this->prev_alive_tick_ = nowtime + Time_Value(30);

	MSG_USER("accelerate %d.%06d %d %.3f", diff_time.sec(), diff_time.usec(), alive_amount, times);
#endif
    return 0;
}

int GatePlayer::level_upgrade(int prev_level)
{
	return this->detail().__level;
}

const std::string &GatePlayer::client_mac(void)
{
	return this->detail().__mac;
}

bool GatePlayer::is_ban_mac(void)
{
	return this->ban_type_ == GameEnum::OPER_BAN_MAC;
}

void GatePlayer::set_ban_mac(bool is_ban)
{
   this->ban_mac_ = (int)is_ban;
}

void GatePlayer::set_is_loading_mongo(const bool is_loading)
{
    this->is_loading_mongo_ = is_loading;
}

bool GatePlayer::is_loading_mongo(void)
{
    return this->is_loading_mongo_;
}

void GatePlayer::set_agent_str(string agent)
{
	this->agent_ = agent;
}
