/*
 * AuthMonitor.cpp
 *
 * Created on: 2013-04-13 15:35
 *     Author: lyz
 */

#include "PubStruct.h"
#include "AuthMonitor.h"
#include "ProtoClient001.pb.h"

AuthMonitor::HistoryTick::HistoryTick(void) : __tick_one(0), __tick_two(0)
{ /*NULL*/ }

int AuthMonitor::init_game_timer_handler(void)
{
    return 0;
}

int AuthMonitor::start_game_timer_handler(void)
{
	return 0;
}

int AuthMonitor::init(void)
{
    Time_Value client_send_timeout(0, 100 * 1000);
    Time_Value inner_send_timeout(0, 100 * 1000);
    {
    	Time_Value recv_timeout(10000, 0);
#ifndef LOCAL_DEBUG
    	if (CONFIG_INSTANCE->tiny("heart").asInt() == 1)
    		recv_timeout.set(30, 0);
#endif
        this->client_receiver_.set(&recv_timeout);
    }
    {
        Time_Value send_timeout(0, 100 * 1000);
        this->connect_sender_.set(send_timeout);
    }
    this->client_monitor_.set_svc_max_list_size(400);
    this->client_monitor_.set_svc_max_pack_size(10 * 1024);
    this->connect_monitor_.set_svc_max_list_size(20000);
    this->connect_monitor_.set_svc_max_pack_size(1024 * 1024);

    this->client_packer_.monitor(&(this->client_monitor_));
    this->client_monitor_.packer(&(this->client_packer_));

    this->inner_packer_.monitor(&(this->inner_monitor_));
    this->inner_monitor_.packer(&(this->inner_packer_));

    this->connect_receiver_.monitor(&(this->connect_monitor_));
    this->connect_monitor_.receiver(&(this->connect_receiver_));
    this->connect_packer_.monitor(&(this->connect_monitor_));
    this->connect_monitor_.packer(&(this->connect_packer_));

    this->gate_to_amount_map_.unbind_all();

    return SUPPER::init(client_send_timeout, inner_send_timeout);
}

int AuthMonitor::start(void)
{
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];
    int limit_accept = servers_json[this->server_config_index_]["limit_accept"].asInt();
    this->client_monitor_.acceptor()->set_limit_connect(limit_accept);

    const Json::Value &connect_scene_json = servers_json[this->server_config_index_]["connect_scene"];
    for (uint i = 0; i < connect_scene_json.size(); ++i)
    {
        if ((connect_scene_json[i].asInt() / 100) != 6)
            continue;
        this->gate_to_amount_map_.rebind(connect_scene_json[i].asInt(), 0);
    }

    SUPPER::start();

    MSG_USER("start auth server");
    return 0;
}

int AuthMonitor::start_game(void)
{
	return 0;
}

void AuthMonitor::fina(void)
{
    this->connect_receiver_.fini();
    this->gate_to_amount_map_.unbind_all();

    SUPPER::fina();
}

BaseUnit *AuthMonitor::logic_unit(void)
{
    return &(this->logic_unit_);
}

Block_Buffer *AuthMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int AuthMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

AuthMonitor::UnitMessagePool *AuthMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

int AuthMonitor::process_init_scene(const int scene_id, const int config_index, const int space_id)
{
    return 0;
}

int AuthMonitor::dispatch_to_all_gate(const int recogn, Block_Buffer *body)
{
    ProtoClientHead head;
    head.__recogn = recogn;

    Block_Buffer buff;

    uint32_t len = sizeof(ProtoClientHead);
    if (body != 0)
    {
        len += body->readable_bytes();
    }

    buff.write_uint32(len);
    buff.copy((char *)(&head), sizeof(ProtoClientHead));
    if (body != 0)
    {
        buff.copy(body);
    }

    for (SceneSet::iterator iter = this->connect_scene_set_.begin();
            iter != this->connect_scene_set_.end(); ++iter)
    {
    	JUDGE_CONTINUE(CONFIG_INSTANCE->is_gate_scene(*iter) == true);

        int sid = this->fetch_sid_of_scene(*iter);
        JUDGE_CONTINUE(sid >= 0);

        this->connect_sender()->push_data_block_with_len(sid, buff);
    }
    return 0;
}

int AuthMonitor::refresh_gate_roleamount(Block_Buffer *msg)
{
    int scene_id = 0, amount = 0;
    msg->read_int32(scene_id);
    msg->read_int32(amount);

    this->gate_to_amount_map_[scene_id] = amount;
    return 0;
}

int AuthMonitor::close_gate_role(Block_Buffer *msg)
{
    int scene_id = 0;
    msg->read_int32(scene_id);

    --this->gate_to_amount_map_[scene_id];
    return 0;
}

#ifndef NO_USE_PROTO
int AuthMonitor::validate_auth_session(const int sid, Message *msg)
#else
int AuthMonitor::validate_auth_session(const int sid, Block_Buffer *msg)
#endif
{
    Svc *svc = NULL;
    JUDGE_RETURN(this->client_monitor_.find_service(sid, svc) == 0, -1);

#ifndef NO_USE_PROTO
    MSG_DYNAMIC_CAST_RETURN(Proto10700101*, request, -1);

    string account = request->account();
    string session = request->session();
    string tick = request->tick();

#else
    std::string account, session, tick;
    (*msg) >> account >> session >> tick;
#endif

    MSG_USER("validate auth %s %s %s", account.c_str(), session.c_str(), tick.c_str());

#ifndef LOCAL_DEBUG
    if (CONFIG_INSTANCE->tiny("auth_validate").asInt() == 1)
    {
		if (account.empty() == true || session.empty() == true || tick.empty() == true)
		{
			MSG_USER("ERROR client auth session %s %s %s",
					account.c_str(), session.c_str(), tick.c_str());

			svc->handle_close();
			return -1;
		}

		int check_tick = ::atoi(tick.c_str());
		time_t nowtime_t = ::time(0);
		AuthMonitor::HistoryTickMap::iterator iter = this->history_tick_map_.find(account);
		if (iter == this->history_tick_map_.end())
		{
			if ((check_tick + AUTH_SESSION_TIMEOUT) < nowtime_t)
			{
				MSG_USER("ERROR session 1 timeout %s %ld %ld", account.c_str(), check_tick, nowtime_t);

				this->dispatch_to_client(sid, RETURN_AUTH_SESSION, ERROR_SESSION_TIMEOUT);
				return -1;
			}
			HistoryTick &h_tick = this->history_tick_map_[account];
			h_tick.__tick_one = check_tick;
		}
		else
		{
			HistoryTick &h_tick = iter->second;
			if (h_tick.__tick_one != check_tick && h_tick.__tick_two != check_tick)
			{
				if ((check_tick + AUTH_SESSION_TIMEOUT) < nowtime_t)
				{
					MSG_USER("ERROR session 2 timeout %s %ld %ld", account.c_str(), check_tick, nowtime_t);

					this->dispatch_to_client(sid, RETURN_AUTH_SESSION, ERROR_SESSION_TIMEOUT);
					return -1;
				}
				if (h_tick.__tick_one <= h_tick.__tick_two)
					h_tick.__tick_one = check_tick;
				else
					h_tick.__tick_two = check_tick;
			}
		}


		std::string src_string(account), dest_md5;
		src_string.append(tick);

		const std::string md5_key = CONFIG_INSTANCE->global()["md5_key"].asString();
		src_string.append(md5_key);

		if (generate_md5(src_string.c_str(), dest_md5) != 0)
		{
			this->dispatch_to_client(sid, RETURN_AUTH_SESSION, ERROR_SESSION_ILLEGAL);
			return -1;
		}
		if (dest_md5 != session)
		{
			MSG_USER("ERROR validate session %s gen_flag:%s flag:%s", src_string.c_str(), dest_md5.c_str(), session.c_str());
			this->dispatch_to_client(sid, RETURN_AUTH_SESSION, ERROR_SESSION_ILLEGAL);
			return -1;
		}
    }
#endif

    int scene_id = 0, role_amount = INT_MAX;
    for (SceneToAmountMap::iterator iter = this->gate_to_amount_map_.begin();
            iter != this->gate_to_amount_map_.end(); ++iter)
    {
        JUDGE_CONTINUE(iter->second < role_amount);

		scene_id = iter->first;
		role_amount = iter->second;
    }

    int config_index = -1;
    if (this->scene_to_index_map_.find(scene_id, 0, config_index) != 0)
    {
        MSG_USER("no connect gate %d", scene_id);
        return -1;
    }


	// force all gate logout;
	Block_Buffer body;
	body << account << 0 << "";
	this->dispatch_to_all_gate(INNER_FORCE_GATE_LOGOUT, &body);
    MSG_USER("auth sync session[%d] %s %s %s %d %d", scene_id, account.c_str(),
    		session.c_str(), tick.c_str(), sid, config_index);

    ++this->gate_to_amount_map_[scene_id];
    Block_Buffer req_buff;
    req_buff << account << session << tick << sid << config_index;
    return this->dispatch_to_gate(scene_id, INNER_SYNC_SEESION, &req_buff);
}

int AuthMonitor::dispatch_to_client(const int sid, const int recogn, Block_Buffer *body)
{
    ProtoClientHead head;
    head.__recogn = recogn;

    Block_Buffer *pbuff = this->pop_block(sid);

    uint32_t len = sizeof(ProtoClientHead), body_len = 0;
    if (body != 0)
    {
        body_len = body->readable_bytes();
    }
    len += body_len;

    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)(&head), sizeof(ProtoClientHead));
    if (body != 0)
    {
        pbuff->copy(body);
    }

    return this->client_sender(sid)->push_pool_block_with_len(pbuff);
}

int AuthMonitor::dispatch_to_client(const int sid, const int recogn, const int error, Message *body)
{
    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = error;

    Block_Buffer *pbuff = this->pop_block(sid);

    uint32_t len = sizeof(ProtoClientHead), body_len = 0;
    if (body != 0)
    {
        body_len = body->ByteSize();
    }
    len += body_len;

    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)(&head), sizeof(ProtoClientHead));
    if (body != 0)
    {
        body->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + body_len);
    }

    return this->client_sender(sid)->push_pool_block_with_len(pbuff);
}

int AuthMonitor::dispatch_to_gate(const int scene_id, const int recogn, Block_Buffer *body)
{
    int sid = this->connect_scene_to_sid(scene_id);
    if (sid < 0)
    {
        sid = this->connect_server(scene_id);
        if (sid < 0)
            return -1;
    }

    ProtoClientHead head;
    head.__recogn = recogn;

    Block_Buffer *pbuff = this->pop_block(sid);

    uint32_t len = sizeof(ProtoClientHead);
    if (body != 0)
        len += body->readable_bytes();
    pbuff->write_int32(sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)(&head), sizeof(ProtoClientHead));
    if (body != 0)
        pbuff->copy(body);

    return this->connect_sender()->push_pool_block_with_len(pbuff);
}

int AuthMonitor::synced_gate_session(Block_Buffer *msg)
{
	std::string account, session, tick;
	int client_sid = 0, conf_index = 0;
	(*msg) >> account >> session >> tick >> client_sid >> conf_index;

    const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
    const ServerDetail &server_detail = server_list[conf_index];

    MSG_USER("auth synced session %s %s %s %d %d, %s(%s:%d)", account.c_str(), session.c_str(), tick.c_str(), client_sid, conf_index,
            server_detail.__domain.c_str(), server_detail.__address.c_str(), server_detail.__outer_port);

#ifndef NO_USE_PROTO 
    Proto50700101 respond;
    respond.set_gate_ip(server_detail.__address);
    respond.set_gate_port(server_detail.__outer_port);
    respond.set_domain(server_detail.__domain);

    return this->dispatch_to_client(client_sid, RETURN_AUTH_SESSION, 0, &respond);
#else
    Block_Buffer res;
    res << server_detail.__address
        << server_detail.__outer_port
        << server_detail.__domain;
    return this->dispatch_to_client(client_sid, RETURN_AUTH_SESSION, &res);
#endif
}

int AuthMonitor::update_server_config(void)
{
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];
    IntMap scene_line_map;
    for (uint i = 0; i < servers_json.size(); ++i)
    {
    	const Json::Value &server_json = servers_json[i];
    	int chat_scene = server_json["chat_scene"].asInt();
        for (uint j = 0; j < server_json["scene"].size(); ++j)
        {
            int scene_id = server_json["scene"][j].asInt();
            int line_cnt = scene_line_map[scene_id];
            this->scene_to_index_map_.rebind(scene_id, line_cnt, i);
            scene_line_map[scene_id] = line_cnt + 1;

            if (chat_scene > 0)
                this->scene_to_chat_map_.rebind(scene_id, chat_scene);
        }
    }

    const Json::Value &connect_scene_json = servers_json[this->server_config_index_]["connect_scene"];
    for (uint i = 0; i < connect_scene_json.size(); ++i)
    {
        if (connect_scene_json[i].isInt() == true)
        {
            int scene_id = connect_scene_json[i].asInt();
            this->connect_scene_set_.insert(scene_id);

            if ((scene_id / 100) == 6)
                this->gate_to_amount_map_.rebind(scene_id, 0);
        }
        else
        {
            GUARD_READ(RW_MUTEX, mon, this->scene_to_index_map_.mutex());        
            for (SceneToIndexMap::iterator iter = this->scene_to_index_map_.begin();
                    iter != this->scene_to_index_map_.end(); ++iter)
            {
                if (iter->first > 10000)
                    this->connect_scene_set_.insert(iter->first);
            }   
        }
    }

    return 0;
}

