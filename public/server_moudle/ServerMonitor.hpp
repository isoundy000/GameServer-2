/*
 * ServerMonitor.hpp
 *
 * Created on: 2013-04-09 11:04
 *     Author: lyz
 */

template <class ClientService, class InnerService, class ConnectService>
ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceMonitor::ConnectServiceMonitor(void) : monitor_(0)
{ /*NULL*/ }

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceMonitor::Monitor *ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceMonitor::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceMonitor::unbind_service(const int sid)
{
    this->monitor()->unbind_connect_scene(sid);
    return SUPPER::unbind_service(sid);
}

template <class ClientService, class InnerService, class ConnectService>
inline ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::InnerServiceMonitor(void) : monitor_(0)
{ /*NULL*/ }

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::Monitor *ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::monitor(Monitor *monitor)
{
    if (monitor != 0)
        this->monitor_ = monitor;
    return this->monitor_;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::bind_service(Svc *svc)
{
    if (SUPPER::bind_service(svc) != 0)
        return -1;

    {
        GUARD(RE_MUTEX, mon, this->sid_set_mutex_);
        this->sid_set_.insert(svc->get_cid());
    }
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::unbind_service(const int sid)
{
    {
        GUARD(RE_MUTEX, mon, this->sid_set_mutex_);
        this->sid_set_.erase(sid);
    }
    return SUPPER::unbind_service(sid);
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::sid_list(std::vector<int> &sid_vc)
{
    GUARD(RE_MUTEX, mon, this->sid_set_mutex_);
    for (SidSet::iterator iter = this->sid_set_.begin();
            iter != this->sid_set_.end(); ++iter)
    {
        sid_vc.push_back(*iter);
    }
	return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceMonitor::first_sid(void)
{
    GUARD(RE_MUTEX, mon, this->sid_set_mutex_);
    if (this->sid_set_.begin() != this->sid_set_.end())
        return *(this->sid_set_.begin());
	return -1;
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ClientServiceAcceptor *ServerMonitor<ClientService, InnerService, ConnectService>::client_acceptor(void)
{
    return this->client_monitor_.acceptor();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ClientServiceReceiver *ServerMonitor<ClientService, InnerService, ConnectService>::client_receiver(void)
{
    return this->client_monitor_.receiver();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ClientServiceSender *ServerMonitor<ClientService, InnerService, ConnectService>::client_sender(const int index)
{
    return this->client_monitor_.sender(index);
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ClientServicePacker *ServerMonitor<ClientService, InnerService, ConnectService>::client_packer(ClientServicePacker *packer)
{
    return this->client_monitor_.packer(packer);
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceAcceptor *ServerMonitor<ClientService, InnerService, ConnectService>::inner_acceptor(void)
{
    return this->inner_monitor_.acceptor();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceReceiver *ServerMonitor<ClientService, InnerService, ConnectService>::inner_receiver(void)
{
    return this->inner_monitor_.receiver();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::InnerServiceSender *ServerMonitor<ClientService, InnerService, ConnectService>::inner_sender(const int index)
{
    return this->inner_monitor_.sender(index);
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::InnerServicePacker *ServerMonitor<ClientService, InnerService, ConnectService>::inner_packer(InnerServicePacker *packer)
{
    return this->inner_monitor_.packer(packer);
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceConnector *ServerMonitor<ClientService, InnerService, ConnectService>::connect_connector(void)
{
    return this->connect_monitor_.connector();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceReceiver *ServerMonitor<ClientService, InnerService, ConnectService>::connect_receiver(ConnectServiceReceiver *receiver)
{
    return this->connect_monitor_.receiver(receiver);
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServiceSender *ServerMonitor<ClientService, InnerService, ConnectService>::connect_sender(void)
{
    return this->connect_monitor_.sender();
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::ConnectServicePacker *ServerMonitor<ClientService, InnerService, ConnectService>::connect_packer(ConnectServicePacker *packer)
{
    return this->connect_monitor_.packer(packer);
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::init(Time_Value &client_sender_timeout, Time_Value &inner_sender_timeout, const int client_sender, const int inner_sender)
{
	this->is_inited_ = true;
	this->init_game_timer_handler();

    this->client_sender_size_ = 1;
    if (client_sender > 0)
        this->client_sender_size_ = client_sender;
    this->client_sender_ = new ClientServiceSender[this->client_sender_size_];
    for (int i = 0; i < this->client_sender_size_; ++i)
    {
        this->client_sender_[i].monitor(&(this->client_monitor_));
        this->client_sender_[i].set(client_sender_timeout);
        this->client_sender_[i].init();
    }
    this->client_monitor_.set_senders(this->client_sender_, this->client_sender_size_);
    this->client_receiver_.monitor(&(this->client_monitor_));
    this->client_monitor_.receiver(&(this->client_receiver_));
    if (this->client_packer() == 0)
    {
        MSG_USER("ERROR client packer is null");
        return -1;
    }

    this->inner_receiver_.monitor(&(this->inner_monitor_));
    this->inner_monitor_.receiver(&(this->inner_receiver_));

    this->inner_sender_size_ = 1;
    if (inner_sender > 0)
        this->inner_sender_size_ = inner_sender;
    this->inner_sender_ = new InnerServiceSender[this->inner_sender_size_];
    for (int i = 0; i < this->inner_sender_size_; ++i)
    {
        this->inner_sender_[i].monitor(&(this->inner_monitor_));
        this->inner_sender_[i].set(inner_sender_timeout);
        this->inner_sender_[i].init();
    }
    this->inner_monitor_.set_senders(this->inner_sender_, this->inner_sender_size_);
    if (this->inner_packer() == 0)
    {
        MSG_USER("ERROR inner packer is null");
        return -1;
    }

    this->connect_monitor_.monitor(this);
    this->connect_sender_.monitor(&(this->connect_monitor_));
    this->connect_monitor_.set_senders(&(this->connect_sender_));

    this->server_config_index_ = -1;
    this->client_acceptor()->init();
    this->client_receiver()->init();

    this->inner_acceptor()->init();
    this->inner_receiver()->init();
    this->inner_sender()->init();

    if (this->connect_receiver() != 0)
        this->connect_receiver()->init();
    this->connect_sender()->init();

    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::start(void)
{
	IntMap scene_line_map;
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];

    for (uint i = 0; i < servers_json.size(); ++i)
    {
        IntSet processed_scene;

        const Json::Value &server_json = servers_json[i];
        int chat_scene = server_json["chat_scene"].asInt();

        for (uint j = 0; j < server_json["scene"].size(); ++j)
        {
            int scene_id = server_json["scene"][j].asInt();
            if (processed_scene.count(scene_id) == 0)
            {
            	processed_scene.insert(scene_id);

            	this->scene_to_index_map_.rebind(scene_id, scene_line_map[scene_id], i);
            	scene_line_map[scene_id] += 1;
            }

            if (chat_scene > 0)
            {
                this->scene_to_chat_map_.rebind(scene_id, chat_scene);
            }

            this->process_init_scene(scene_id, i);
        }
    }

    int out_port = 0, inner_port = 0;
    {
        const Json::Value &server_json = servers_json[this->server_config_index_];
        out_port = server_json["outer_port"].asInt();
        inner_port = server_json["inner_port"].asInt();
        for (uint i = 0; i < server_json["connect_scene"].size(); ++i)
        {
            if (server_json["connect_scene"][i].isInt() == true)
            {
                this->connect_scene_set_.insert(server_json["connect_scene"][i].asInt());
            }
            else
            {
                for (SceneToIndexMap::iterator iter = this->scene_to_index_map_.begin();
                        iter != this->scene_to_index_map_.end(); ++iter)
                {
                	//网关进程，连接所有地图进程
                    JUDGE_CONTINUE(iter->first >= 10000);
                    this->connect_scene_set_.insert(iter->first);
                }
            }
        }
    }

    this->check_all_connect_set();
    this->start_game();

    if (this->logic_unit() != 0)
    {
        this->logic_unit()->thr_create();
    }

    if (this->connect_scene_set_.empty() == false)
    {
        this->connect_sender()->thr_create();
        if (this->connect_packer() != 0)
        {
            this->connect_packer()->thr_create();
        }
        if (this->connect_receiver() != 0)
        {
            this->connect_receiver()->thr_create();
        }
    }

    if (inner_port > 0)
    {
        this->inner_acceptor()->set(inner_port);
        this->inner_packer()->thr_create();
        for (int i = 0; i < this->inner_sender_size_; ++i)
        {
            this->inner_sender_[i].thr_create();
        }
        this->inner_receiver()->thr_create();
        this->inner_acceptor()->thr_create();
    }

    if (out_port > 0)
    {
        this->client_acceptor()->set(out_port);
        this->client_packer()->thr_create();
        for (int i = 0; i < this->client_sender_size_; ++i)
        {
            this->client_sender_[i].thr_create();
        }
        this->client_receiver()->thr_create();
        this->client_acceptor()->thr_create();
    }

    POOL_MONITOR->global_timer_watcher()->start();
    this->start_game_timer_handler();

    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::stop(void)
{
#define THR_CACEL_JOIN(THR) \
	if (THR != 0) \
	{ \
		THR->thr_cancel_join(); \
	}

	POOL_MONITOR->global_timer_watcher()->stop_wait();
    if (this->logic_unit() != 0)
    {
        this->logic_unit()->stop_wait();
    }

    THR_CACEL_JOIN(this->client_acceptor());
    THR_CACEL_JOIN(this->client_receiver());

    for (int i = 0; i < this->client_sender_size_; ++i)
    {
        this->client_sender_[i].thr_cancel_join();
    }

    THR_CACEL_JOIN(this->client_packer());
    THR_CACEL_JOIN(this->inner_acceptor());
    THR_CACEL_JOIN(this->inner_receiver());

    for (int i = 0 ; i < this->inner_sender_size_; ++i)
    {
        this->inner_sender_[i].thr_cancel_join();
    }

    THR_CACEL_JOIN(this->inner_packer());
    THR_CACEL_JOIN(this->connect_receiver());
    THR_CACEL_JOIN(this->connect_sender());
    THR_CACEL_JOIN(this->connect_packer());

#undef THR_CACEL_JOIN
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline void ServerMonitor<ClientService, InnerService, ConnectService>::fina(void)
{
    this->client_monitor_.fini();
    this->inner_monitor_.fini();
    this->connect_monitor_.fini();

    this->client_receiver_.fini();
    for (int i = 0; i < this->client_sender_size_; ++i)
    {
        this->client_sender_[i].fini();
    }
    delete [] this->client_sender_;
    this->client_sender_ = 0;
    this->inner_receiver_.fini();
    for (int i = 0; i < this->inner_sender_size_; ++i)
    {
        this->inner_sender_[i].fini();
    }
    delete [] this->inner_sender_;
    this->inner_sender_ = 0;
    this->connect_sender_.fini();

    this->scene_to_index_map_.unbind_all();
    this->connect_scene_set_.clear();
    this->connect_scene_to_sid_map_.unbind_all();
    this->sid_line_map_.unbind_all();
    this->scene_to_chat_map_.unbind_all();
}

template <class ClientService, class InnerService, class ConnectService>
inline bool ServerMonitor<ClientService, InnerService, ConnectService>::is_running(void)
{
    if (this->logic_unit() != 0)
        return this->logic_unit()->is_running();
    return false;
}

template <class ClientService, class InnerService, class ConnectService>
int ServerMonitor<ClientService, InnerService, ConnectService>::check_all_connect_set(void)
{
    for (SceneSet::iterator iter = this->connect_scene_set_.begin();
            iter != this->connect_scene_set_.end(); ++iter)
    {
    	SceneToIndexMap::KeyValueMap *kv_map = 0;
    	JUDGE_CONTINUE(this->scene_to_index_map_.find_object_map(*iter, kv_map) == 0);

    	int total_size = std::max<int>(1, kv_map->size());
		for (int i = 0; i < total_size; ++i)
		{
    		JUDGE_CONTINUE(this->connect_scene_to_sid(*iter, i) < 0);
    		this->connect_server(*iter, i);
		}
    }
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline void ServerMonitor<ClientService, InnerService, ConnectService>::set_server_config_index(const int index)
{
    this->server_config_index_ = index;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::server_config_index(void)
{
    return this->server_config_index_;
}

template <class ClientService, class InnerService, class ConnectService>
inline bool ServerMonitor<ClientService, InnerService, ConnectService>::is_current_server_scene(const int scene_id)
{
	const ServerDetail &server_cfg = CONFIG_INSTANCE->server_list(this->server_config_index_);
	return server_cfg.__scene_list.count(scene_id) > 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::connect_scene_to_sid(const int scene_id, const int line_id)
{
    int sid = 0;
    if (this->connect_scene_to_sid_map_.find(scene_id, line_id, sid) != 0)
        return -1;
    Svc *svc = 0;
    if (this->connect_monitor_.find_service(sid, svc) != 0)
        return -1;
    return sid;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::connect_server(const int scene_id, const int line_id)
{
	JUDGE_RETURN(this->connect_scene_set_.count(scene_id) > 0, -1);

    int config_index = -1;
    if (this->scene_to_index_map_.find(scene_id, line_id, config_index) != 0)
        return -1;

    const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(config_index);
    if (server_detail.__address.empty() == true || server_detail.__inner_port <= 0)
        return -1;
    int port = server_detail.__inner_port;
    int sid = this->connect_connector()->connect(server_detail.__address.c_str(), port);
    if (sid < 0)
        return -1;

    this->sid_line_map_.rebind(sid, line_id);

    int other_scene_id = 0;
    for (BIntSet::const_iterator iter = server_detail.__scene_list.begin(); iter != server_detail.__scene_list.end(); ++iter)
    {
        other_scene_id = *iter;
        this->connect_scene_to_sid_map_.rebind(other_scene_id, line_id, sid);
    }

    return sid;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::unbind_connect_scene(const int sid)
{
	int line_id = 0;
	if (this->sid_line_map_.unbind(sid, line_id) != 0)
		line_id = 0;

    std::set<int> scene_set;
    {
        GUARD_READ(RW_Mutex, mon, this->connect_scene_to_sid_map_.mutex());

        for (SceneToSidMap::iterator iter = this->connect_scene_to_sid_map_.begin();
                iter != this->connect_scene_to_sid_map_.end(); ++iter)
        {
        	SceneToSidMap::KeyValueMap *kv_map = iter->second;
        	int map_sid = 0;
        	if (kv_map->find(line_id, map_sid) != 0)
        		continue;
            if (map_sid != sid)
                continue;

            scene_set.insert(iter->first);
        }
    }
    for (std::set<int>::iterator iter = scene_set.begin(); 
            iter != scene_set.end(); ++iter)
    {
        this->connect_scene_to_sid_map_.unbind(*iter, line_id);
    }
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline typename ServerMonitor<ClientService, InnerService, ConnectService>::SceneSet &ServerMonitor<ClientService, InnerService, ConnectService>::connect_scene_set(void)
{
    return this->connect_scene_set_;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::fetch_sid_of_scene(const int scene_id, const bool for_log, const int64_t src_line_id)
{
	SceneToIndexMap::KeyValueMap *kv_map = 0;
	if (this->scene_to_index_map_.find_object_map(scene_id, kv_map) != 0)
		return -1;

	int line_id = 0;
	if (kv_map->size() > 0)
		line_id = src_line_id % kv_map->size();

    int sid = this->connect_scene_to_sid(scene_id, line_id);
#ifndef LOCAL_DEBUG
    if (for_log == true && this->is_running() == false)
        return sid;
#endif
    if (sid < 0)
    {
        return this->connect_server(scene_id, line_id);
    }
    return sid;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::fetch_sid_of_all_line(const int scene_id, std::vector<int> &sid_list)
{
	SceneToSidMap::KeyValueMap *kv_map = 0;
	if (this->connect_scene_to_sid_map_.find_object_map(scene_id, kv_map) != 0)
		return -1;
	for (SceneToSidMap::KeyValueMap::iterator iter = kv_map->begin(); iter != kv_map->end(); ++iter)
	{
		sid_list.push_back(iter->second);
	}
	return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::fetch_scene_line_amount(const int scene_id)
{
	SceneToIndexMap::KeyValueMap *kv_map = 0;
	if (this->scene_to_index_map_.find_object_map(scene_id, kv_map) != 0)
		return 0;
    return kv_map->size();
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::fetch_sid(int scene_id, int line_id)
{
	SceneToSidMap::KeyValueMap *kv_map = 0;
	if (this->connect_scene_to_sid_map_.find_object_map(scene_id, kv_map) != 0)
	{
		return -1;
	}

	int sid = 0;
	if (kv_map->find(line_id, sid) == 0)
	{
		return sid;
	}

	return -1;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::scene_convert_to(const int scene_id, const int64_t split_id)
{
    int convert_scene = scene_id;
    const BIntSet &convert_scene_set = CONFIG_INSTANCE->convert_scene_set(scene_id);
    if (convert_scene_set.size() > 0)
    {
        int index = split_id % (convert_scene_set.size());
        BIntSet::const_iterator iter = convert_scene_set.begin();
        convert_scene = *iter;
        for (int i = 0; i <= index && iter != convert_scene_set.end(); ++i)
        {
            convert_scene = *iter;
            ++iter;
        }
    }

//    MSG_USER("convert scene (%ld) %d->%d", split_id, scene_id, convert_scene);

    return convert_scene;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::inner_sid_list(std::vector<int> &sid_vc)
{
    return this->inner_monitor_.sid_list(sid_vc);
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::first_inner_sid(void)
{
    return this->inner_monitor_.first_sid();
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::chat_scene(const int map_scene)
{
    int chat_scene = 0;
    if (this->scene_to_chat_map_.find(map_scene, chat_scene) != 0)
        return -1;
    return chat_scene;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::make_up_client_head(Block_Buffer *buff, const int recogn, const int error)
{
    ProtoClientHead head;
    head.__recogn = recogn;
    head.__error = error;

    buff->write_uint32(sizeof(ProtoClientHead));
    buff->copy((char *)&head, sizeof(ProtoClientHead));
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::make_up_gate_head(Block_Buffer *buff, const ProtoHead *head)
{
    buff->write_uint32(sizeof(ProtoHead));
    buff->copy((char *)head, sizeof(ProtoHead));
    return 0;
}

template <class ClientService, class InnerService, class ConnectService>
inline int ServerMonitor<ClientService, InnerService, ConnectService>::update_block_length(Block_Buffer *buff)
{
    uint32_t *len = (uint32_t *)(buff->get_read_ptr());
    *len = buff->readable_bytes() - sizeof(uint32_t);
    return 0;
}

