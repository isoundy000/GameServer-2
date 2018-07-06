/*
 * ServerMonitor.h
 *
 * Created on: 2013-04-08 10:52
 *     Author: lyz
 */

#ifndef _SERVERMONITOR_H_
#define _SERVERMONITOR_H_

template <class ClientService, class InnerService, class ConnectService>
class ServerMonitor
{
public:
    class ConnectServiceMonitor : public ServiceMonitor<ConnectService>
    {
    public:
        typedef ServerMonitor<ClientService, InnerService, ConnectService> Monitor;
        typedef ServiceMonitor<ConnectService> SUPPER;

    public:
        ConnectServiceMonitor(void);
        Monitor *monitor(Monitor *monitor = 0);

        virtual int unbind_service(const int sid);

    protected:
        Monitor *monitor_;
    };

    class InnerServiceMonitor : public ServiceMonitor<InnerService>
    {
    public:
        typedef ServerMonitor<ClientService, InnerService, ConnectService> Monitor;
        typedef ServiceMonitor<InnerService> SUPPER;
        typedef boost::unordered_set<int> SidSet;

    public:
        InnerServiceMonitor(void);
        Monitor *monitor(Monitor *monitor = 0);

        virtual int bind_service(Svc *svc);
        virtual int unbind_service(const int sid);
        virtual int sid_list(std::vector<int> &sid_vc);
        virtual int first_sid(void);

    protected:
        Monitor *monitor_;

        SidSet sid_set_;
        RE_MUTEX sid_set_mutex_;
    };

public:
    typedef ServiceMonitor<ClientService> ClientServiceMonitor;

    typedef ServiceAcceptor<ClientService> ClientServiceAcceptor;
    typedef ServiceReceiver<ClientService> ClientServiceReceiver;
    typedef ServiceSender<ClientService> ClientServiceSender;
    typedef ServicePacker<ClientService> ClientServicePacker;

    typedef ServiceAcceptor<InnerService> InnerServiceAcceptor;
    typedef ServiceReceiver<InnerService> InnerServiceReceiver;
    typedef ServiceSender<InnerService> InnerServiceSender;
    typedef ServicePacker<InnerService> InnerServicePacker;

    typedef ServiceConnector<ConnectService> ConnectServiceConnector;
    typedef ServiceReceiver<ConnectService> ConnectServiceReceiver;
    typedef ServiceSender<ConnectService> ConnectServiceSender;
    typedef ServicePacker<ConnectService> ConnectServicePacker;

    typedef DoubleKeyMap<int, int, int, RW_MUTEX> SceneToIndexMap;
    typedef DoubleKeyMap<int, int, int, RW_MUTEX> SceneToSidMap;
    typedef boost::unordered_set<int> SceneSet;
    typedef HashMap<int, int, NULL_MUTEX> SceneToChatMap;
    typedef HashMap<int, int, RW_MUTEX> SidLineMap;

    const static int MAP_OBJECT_BUCKET = 8193;

public:
    ClientServiceAcceptor *client_acceptor(void);
    ClientServiceReceiver *client_receiver(void);
    ClientServiceSender *client_sender(const int index = 0);
    ClientServicePacker *client_packer(ClientServicePacker *packer = 0);

    InnerServiceAcceptor *inner_acceptor(void);
    InnerServiceReceiver *inner_receiver(void);
    InnerServiceSender *inner_sender(const int index = 0);
    InnerServicePacker *inner_packer(InnerServicePacker *packer = 0);

    ConnectServiceConnector *connect_connector(void);
    ConnectServiceReceiver *connect_receiver(ConnectServiceReceiver *receiver = 0);
    ConnectServiceSender *connect_sender(void);
    ConnectServicePacker *connect_packer(ConnectServicePacker *packer = 0);

    ServerMonitor() : client_sender_size_(0), client_sender_(0),
    		inner_sender_size_(0), inner_sender_(0),
    		server_config_index_(-1), is_inited_(false) //
    {/*NULL*/}
    virtual ~ServerMonitor(){}
    virtual BaseUnit *logic_unit(void) = 0;

    virtual int init(Time_Value &client_sender_timeout, Time_Value &inner_sender_timeout, const int client_sender = 1, const int inner_sender = 1);
    virtual int start(void);
    virtual int start_game(void) = 0;
    virtual int stop(void);
    virtual void fina(void);
    virtual bool is_running(void);
    bool is_inited(void) { return this->is_inited_; }
    virtual int check_all_connect_set(void);

    void set_server_config_index(const int index);
    int server_config_index(void);
    bool is_current_server_scene(const int scene_id);
    virtual int connect_scene_to_sid(const int scene_id, const int line_id = 0);
    virtual int unbind_connect_scene(const int sid);
    SceneSet &connect_scene_set(void);
    virtual int fetch_sid_of_scene(const int scene_id, const bool for_log = false, const int64_t line_id = 0);
    int fetch_sid_of_all_line(const int scene_id, std::vector<int> &sid_list);
    int fetch_scene_line_amount(const int scene_id);
    int fetch_sid(int scene_id, int line_id);
    int scene_convert_to(const int scene_id, const int64_t split_id);

    int inner_sid_list(std::vector<int> &sid_vc);
    int first_inner_sid(void);

    virtual int chat_scene(const int map_scene);

    int make_up_client_head(Block_Buffer *buff, const int recogn, const int error = 0);
    int make_up_gate_head(Block_Buffer *buff, const ProtoHead *head);
    int update_block_length(Block_Buffer *buff);

protected:
    virtual int connect_server(const int scene_id, const int line_id = 0);

    virtual int process_init_scene(const int scene_id, const int config_index, const int space_id = 0) = 0;
    virtual int init_game_timer_handler(void) = 0;
    virtual int start_game_timer_handler(void) = 0;

protected:
    ClientServiceMonitor client_monitor_;
    InnerServiceMonitor inner_monitor_;
    ConnectServiceMonitor connect_monitor_;

    ClientServiceReceiver client_receiver_;

    int client_sender_size_;
    ClientServiceSender *client_sender_;

    InnerServiceReceiver inner_receiver_;
    int inner_sender_size_;
    InnerServiceSender *inner_sender_;
    
    ConnectServiceSender connect_sender_;

    int server_config_index_;
    SceneToIndexMap scene_to_index_map_;		// key_1: scene_id, key_2: line_id, value: config_index
    SceneSet connect_scene_set_;
    SceneToSidMap connect_scene_to_sid_map_;	// key_1: scene_id, key_2: line_id, value: connect_sid
    SidLineMap sid_line_map_;			// key: sid, value: line_id

    SceneToChatMap scene_to_chat_map_;
    bool is_inited_;
};

#include "ServerMonitor.hpp"

#endif //_SERVERMONITOR_H_
