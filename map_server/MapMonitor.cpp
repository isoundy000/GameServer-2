/*
 * MapMonitor.cpp
 *
 * Created on: 2013-01-17 22:49
 *     Author: glendy
 */

#include <signal.h>

#include "PubStruct.h"
#include "MapCommunicate.h"
#include "MapMonitor.h"
#include "FightStruct.h"
#include "MapTaskStruct.h"
#include "ScriptStruct.h"
#include "AIStruct.h"
#include "MapStruct.h"
#include "MapLogicStruct.h"
#include "MapUnit.h"
#include "MapLogicUnit.h"
#include "MapTimerHandler.h"
#include "Scene.h"
#include "SceneFactory.h"
#include "MapPlayerEx.h"
#include "MapLogicPlayer.h"
#include "TaskImplement.h"
#include "TaskRoutineImp.h"
#include "TaskTrialImp.h"
#include "MapBeast.h"
#include "SessionManager.h"
#include "TransactionMonitor.h"
#include "LeagueMonitor.h"
#include "SMBattleSystem.h"
#include "LeagueWarSystem.h"
#include "MonsterAttackSystem.h"
#include "WorldBossSystem.h"
#include "FlowControl.h"
#include "ProtoDefine.h"
#include "MMOBeast.h"
#include "BaseScript.h"
#include "ScriptFactory.h"
#include "ScriptAI.h"
#include "GameCommon.h"
#include "MongoConnector.h"
#include "GlobalScriptHistory.h"
#include "SceneLineManager.h"
#include "MediaGiftConfig.h"
#include "TaskRoutineImp.h"
#include "Transaction.h"
#include "MLBackDailyRechargeSys.h"
#include "MLBackRebateRechargeSys.h"
#include "MLGameSwither.h"
#include "CollectChestsScene.h"
#include "AnswerActivityScene.h"
#include "HotspringActivityScene.h"
#include "BackActivityTick.h"
#include "AIManager.h"
#include "BackGameSwitcher.h"
#include "MMOGlobal.h"
#include "TMarenaMonitor.h"
#include "ScriptSystem.h"
#include "TrvlArenaMonitor.h"
#include "TrvlWeddingMonitor.h"
#include "TrvlRechargeMonitor.h"
#include "TrvlWbossMonitor.h"
#include "LeagueReginFightSystem.h"
#include "TrvlBattleMonitor.h"
#include "TrvlPeakMonitor.h"
#include "TrvlPeakActor.h"

MapMonitor::MapMonitor(void)
{
	this->line_id_ = 0;
	this->chat_port_ = 0;
	this->chat_scene_ = 0;
    this->average_level_ = 1;
    this->travel_scene_ = false;
    this->check_gate_state_ = false;

    this->player_pool_ = new MapPlayerPool();
    this->skill_pool_ = new SkillPool();
    this->status_pool_ = new StatusPool();
    this->history_status_pool_ = new HistoryStatusPool();
    this->status_queue_node_pool_ = new StatusQueueNodePool();
    this->logic_player_pool_ = new LogicPlayerPool();
    this->mail_box_pool_ = new MailBoxPool();
    this->task_info_pool_ = new TaskInfoPool();
    this->task_condition_pool_ = new TaskConditionPool();
    this->task_imp_pool_ = new TaskImplementPool();
    this->task_routine_imp_pool_ = new TaskRoutineImpPool();
    this->task_trial_imp_pool_ = new TaskTrialImpPool();
    this->delay_skill_pool_ = new DelaySkillPool();
    this->script_player_rel_pool_ = new ScriptPlayerRelPool();
    this->script_team_pool_ = new ScriptTeamDetailPool();
    this->scene_ai_record_pool_ = new SceneAIRecordPool();
    this->script_ai_pool_ = new ScriptAIPool();
    this->acti_code_detail_pool_ = new ActiCodeDetailPool();
    this->passive_skill_qn_pool_ = new PassiveSkillQNPool();
    this->team_scene_block_pool_ = new SceneBlockPool();

    this->script_factory_ = NULL;
    this->scene_factory_ = NULL;
    this->map_beast_package_ = NULL;

    this->map_one_sec_timer_ = NULL;
    this->ml_mid_night_timer_ = NULL;
    this->ml_int_min_timer_ = NULL;
    this->map_int_hour_timer_ = NULL;
    this->map_mid_night_timer_ = NULL;
    this->trvl_keep_alive_timer_ = NULL;

    this->map_unit_ = NULL;
    this->logic_unit_ = NULL;
    this->inner_packer_ = NULL;
    this->client_packer_ = NULL;
    this->session_manager_ = NULL;
    this->player_manager_ = NULL;
    this->timer_handler_list_ = NULL;
    this->scene_line_manager_ = NULL;
    this->ml_player_assist_package_ = NULL;

    this->generate_ai_id_ 		= 0;
    this->generate_drop_id_ 	= 0;
    this->generate_camp_id_ 	= 0;
    this->generate_beast_id_	= 0;

	this->generate_role_copy_id_ 	= 0;
	this->generate_beast_copy_id_ 	= 0;
	this->generate_effect_id_ 		= 0;
}

MapMonitor::~MapMonitor(void)
{
    SAFE_DELETE(this->player_manager_);
    SAFE_DELETE(this->scene_line_manager_);
    SAFE_DELETE(this->scene_factory_);
    SAFE_DELETE(this->script_factory_);

    SAFE_DELETE(this->player_pool_);
    SAFE_DELETE(this->skill_pool_);
    SAFE_DELETE(this->status_pool_);
    SAFE_DELETE(this->history_status_pool_);
    SAFE_DELETE(this->status_queue_node_pool_);
    SAFE_DELETE(this->logic_player_pool_);
    SAFE_DELETE(this->mail_box_pool_);
    SAFE_DELETE(this->task_info_pool_);
    SAFE_DELETE(this->task_condition_pool_);
    SAFE_DELETE(this->task_imp_pool_);
    SAFE_DELETE(this->task_routine_imp_pool_);
    SAFE_DELETE(this->task_trial_imp_pool_);
    SAFE_DELETE(this->delay_skill_pool_);
    SAFE_DELETE(this->script_player_rel_pool_);
    SAFE_DELETE(this->script_team_pool_);
    SAFE_DELETE(this->scene_ai_record_pool_);
    SAFE_DELETE(this->script_ai_pool_);
    SAFE_DELETE(this->acti_code_detail_pool_);
    SAFE_DELETE(this->map_beast_package_);
    SAFE_DELETE(this->skill_cool_package_);
    SAFE_DELETE(this->ml_player_assist_package_);
    SAFE_DELETE(this->passive_skill_qn_pool_);
    SAFE_DELETE(this->team_scene_block_pool_);

    SAFE_DELETE(this->map_one_sec_timer_);
    SAFE_DELETE(this->map_int_hour_timer_);
    SAFE_DELETE(this->map_mid_night_timer_);
    SAFE_DELETE(this->trvl_keep_alive_timer_);
    SAFE_DELETE(this->ml_int_min_timer_);
    SAFE_DELETE(this->ml_mid_night_timer_);

    SAFE_DELETE(this->map_unit_);
    SAFE_DELETE(this->logic_unit_);
    SAFE_DELETE(this->timer_handler_list_);
    SAFE_DELETE(this->client_packer_);
    SAFE_DELETE(this->inner_packer_);
}

int MapMonitor::init_game_timer_handler(void)
{
    const double inter_tick_list[] = {
        0.1,					//401
        0.3,					//402
        10,						//403
        MAP_BROAD_INTERVAL,		//404
        1,						//405
        1,						//406
        60,						//407
        60						//408
    };

    int timer_amount = sizeof(inter_tick_list) / sizeof(double);
    this->timer_handler_list_->resize(timer_amount);
    POOL_MONITOR->init_game_timer_list(timer_amount);

    for (int i = GTT_MAP_TYPE_BEG + 1; i < GTT_MAP_TYPE_END; ++i)
    {
    	int index = i - GTT_MAP_TYPE_BEG - 1;
        Time_Value interval = Time_Value::gettime(inter_tick_list[index]);

        (*(this->timer_handler_list_))[index].set_type(i);
        (*(this->timer_handler_list_))[index].set_interval(interval);
    }

    return 0;
}

int MapMonitor::start_game_timer_handler(void)
{
	for (TimerHandlerList::iterator iter = this->timer_handler_list_->begin();
			iter != this->timer_handler_list_->end(); ++iter)
	{
		iter->schedule_timer(iter->interval());
	}
	return 0;
}

int MapMonitor::init(const int config_index)
{
	this->is_inited_ = true;
    this->travel_scene_ = false;
    this->line_id_ = 0;
    this->client_packer_ = new MapClientPacker();
    this->inner_packer_ = new MapInnerPacker();
    this->map_unit_ = new MapUnit();
    this->logic_unit_ = new MapLogicUnit();
    this->timer_handler_list_ = new TimerHandlerList();
    this->session_manager_ = SESSION_MANAGER;
    this->player_manager_ = new PlayerManager();
    this->scene_line_manager_ = new SceneLineManager();
    this->scene_factory_ = new SceneFactory();
    this->script_factory_ = new ScriptFactory();
    this->map_beast_package_ = new MapBeastPackage;
    this->skill_cool_package_ = new SkillCoolPackage;
    this->ml_player_assist_package_ = new MLPlayerAssistPackage;

    this->map_one_sec_timer_ = new MapOneSecTimer;
    this->map_int_hour_timer_ = new MapIntHourTimer;
    this->map_mid_night_timer_ = new MapMidNightTimer;
    this->trvl_keep_alive_timer_ = new TrvlKeepAliveTimer;
    this->ml_int_min_timer_ = new MLIntMinTimer;
    this->ml_mid_night_timer_ = new MLMidNightTimer;

    Time_Value client_send_timeout(0, 50 * 1000);
    Time_Value inner_send_timeout(0, 1000);
    {
#ifdef LOCAL_DEBUG
        Time_Value recv_timeout(3000000, 0);
#else
        Time_Value recv_timeout(60, 0);
#endif
        this->client_receiver_.set(&recv_timeout);
    }
    {
        Time_Value send_timeout(0, 100 * 1000);
        this->connect_sender_.set(send_timeout);
    }
    this->client_monitor_.set_svc_max_list_size(1000);          // 收发队列最长200个消息
    this->client_monitor_.set_svc_max_pack_size(10 * 1024);    // 单个包最大10K
    this->inner_monitor_.set_svc_max_list_size(20000);
    this->inner_monitor_.set_svc_max_pack_size(1024 * 1024 * 10);
    this->connect_monitor_.set_svc_max_list_size(20000);

    this->client_packer_->monitor(&(this->client_monitor_));
    this->client_monitor_.packer(this->client_packer_);

    this->inner_packer_->monitor(&(this->inner_monitor_));
    this->inner_monitor_.packer(this->inner_packer_);

    const Json::Value &server_json = CONFIG_INSTANCE->global()["server_list"][config_index];
    int sender_thr = 2;
    if (server_json.isMember("send_thr") == true)
    {
        sender_thr = server_json["send_thr"].asInt();
    }

    this->fight_total_use_ = Time_Value::zero;
    this->move_total_use_ = Time_Value::zero;
    this->ai_total_use_ = Time_Value::zero;

    return SUPPER::init(client_send_timeout, inner_send_timeout, 1, sender_thr);
}

int MapMonitor::start(void)
{
    const Json::Value &servers_json = CONFIG_INSTANCE->global()["server_list"];
	const Json::Value &server_json = servers_json[this->server_config_index_];

	this->chat_scene_ = server_json["chat_scene"].asInt();
	if (server_json.isMember("line"))
	{
		this->line_id_ = server_json["line"].asInt() - 1;
	}

	const GameConfig::ServerList &server_list = CONFIG_INSTANCE->server_list();
	for (GameConfig::ServerList::const_iterator iter = server_list.begin();
			iter != server_list.end(); ++iter)
	{
		const ServerDetail &server_detail = *iter;
		JUDGE_CONTINUE(server_detail.__scene_list.count(this->chat_scene_) > 0);

		this->chat_ip_ = server_detail.__address;
		this->chat_port_ = server_detail.__outer_port;
		break;
	}

    SUPPER::start();
    this->map_unit()->thr_create();

#ifndef LOCAL_DEBUG
	if (this->is_has_travel_scene())
	{
		CONFIG_INSTANCE->load_server_ip();
		this->chat_ip_ = CONFIG_INSTANCE->server_ip();
	}
#endif

    MSG_USER("start map server %d server ip:%s port:%d...",
    		this->average_level_, this->chat_ip_.c_str(), this->chat_port_);
    return 0;
}

// 此处放需要启动前加载或初始化的代码接口
int MapMonitor::start_game(void)
{
    this->load_script_progress_id_map();
    GLOBAL_SCRIPT_HISTORY->load_script_history_info();

	FLOW_INSTANCE->load_flow_detail_when_init();
	MEDIA_GIFT_CONFIG->media_gift_start();
    BACK_ACTIVITY_TICK->init();

	MMOGlobal::load_global_key_value("average_role_level",
			this->average_level_);

    this->map_one_sec_timer_->schedule_timer();
    this->map_int_hour_timer_->schedule_timer();
    this->map_mid_night_timer_->schedule_timer();
    this->ml_int_min_timer_->schedule_timer();
    this->ml_mid_night_timer_->schedule_timer();

    this->check_and_run_scene_monster();

    const Json::Value &server_json = CONFIG_INSTANCE->global(
    		)["server_list"][this->server_config_index_];

	ML_SWITCHER_SYS->init();

    for (uint i = 0; i < server_json["scene"].size(); ++i)
    {
    	int scene_id = server_json["scene"][i].asInt();

        if (GameCommon::is_travel_scene(scene_id))
        {
            this->travel_scene_ = true;
        }

    	switch (scene_id)
    	{
    	case GameEnum::RECHARGE_BASE_SCENE_ID:
    	{
    		BackDR_SYS->init();
    		break;
    	}
    	case GameEnum::LBOSS_SCENE_ID:
    	{
    		LEAGUE_MONITOR->star_league_boss();
    		break;
    	}
    	case GameEnum::LWAR_SCENE_ID:
    	{
    		LEAGUE_WAR_SYSTEM->start_league_war(scene_id);
    		break;
    	}
    	case GameEnum::MATTACK_SCENE_ID:
    	{
    		MONSTER_ATTACK_SYSTEM->start_monster_attack(scene_id);
    		break;
    	}
    	case GameEnum::SUN_MOON_BATTLE_ID:
    	{
    		SM_BATTLE_SYSTEM->start_sm_battle(scene_id);
    		break;
    	}
    	case GameEnum::ANSWER_ACTIVITY_SCENE_ID:
    	{
    		ANSWERACTIVITY_INSTANCE->init_answer_scene();
    		break;
    	}
    	case GameEnum::HOTSPRING_SCENE_ID:
    	{
    		HOTSPRING_INSTANCE->init_hotspring_scene();
    		break;
    	}
    	case GameEnum::COLLECT_CHESTS_SCENE_ID:
    	{
    		Scene *temp;
    		MAP_MONITOR->find_scene(0, GameEnum::COLLECT_CHESTS_SCENE_ID, temp);
    		COLLECTCHESTS_INSTANCE->init_collect_chests(temp);
    		break;
    	}
        case GameEnum::LEGEND_TOP_SCENE:
        {
        	SCRIPT_SYSTEM->star();
        	break;
        }
        case GameEnum::TRVL_WBOSS_SCENE_ID_READY:
        {
        	TRVL_WBOSS_MONITOR->start_world_boss();
        	break;
        }
        case GameEnum::TRVL_ARENA_PREP_SCENE_ID:
        {
        	TRVL_ARENA_MONITOR->start();
        	break;
        }
        case GameEnum::TRVL_MARENA_PREP_SCENE_ID:
        {
        	TRVL_MARENA_MONITOR->start();
        	break;
        }
        case GameEnum::TRVL_WEDDING_SCENE_ID:
        {
        	TRVL_WEDDING_MONITOR->start();
            break;
        }
        case GameEnum::TRVL_RECHARGE_SCENE_ID:
        {
        	TRVL_RECHARGE_MONITOR->start();
        	break;
        }
        case GameEnum::LEAGUE_REGION_FIGHT_ID:
        {
        	LRF_MONITOR->init();
        	break;
        }
        case GameEnum::TRVL_BATTLE_SCENE_ID:
        {
        	TRVL_BATTLE_MONITOR->start();
        	break;
        }
        case GameEnum::TRVL_PEAK_PRE_SCENE_ID:
        {
            TRVL_PEAK_MONITOR->start();
            break;
        }
    	}
    }

    if (this->is_has_travel_scene() == true)
    {
    	this->trvl_keep_alive_timer_->schedule_timer(30);
    }

    //启动世界boss
    WORLD_BOSS_SYSTEM->start_world_boss();

    return 0;
}

int MapMonitor::stop(void)
{
	LEAGUE_MONITOR->fina();
	MEDIA_GIFT_CONFIG->media_gift_end();

	const Json::Value &server_json = CONFIG_INSTANCE->global(
	    		)["server_list"][this->server_config_index_];
	for (uint i = 0; i < server_json["scene"].size(); ++i)
	{
		int scene_id = server_json["scene"][i].asInt();
		switch (scene_id)
		{
		case GameEnum::RECHARGE_BASE_SCENE_ID:
		{
		    BackDR_SYS->stop();
		    break;
		}
		case GameEnum::SUN_MOON_BATTLE_ID:
		{
			SM_BATTLE_SYSTEM->stop_sm_battle();
			break;
		}
		case GameEnum::MATTACK_SCENE_ID:
		{
			MONSTER_ATTACK_SYSTEM->stop_monster_attack();
		    break;
		}
        case GameEnum::LEGEND_TOP_SCENE:
        {
        	SCRIPT_SYSTEM->stop();
        	break;
        }
        case GameEnum::TRVL_WBOSS_SCENE_ID_READY:
        {
        	TRVL_WBOSS_MONITOR->stop_world_boss();
            break;
        }
        case GameEnum::TRVL_ARENA_SCENE_ID:
        {
        	TRVL_ARENA_MONITOR->stop();
        	break;
        }
        case GameEnum::TRVL_MARENA_SCENE_ID:
        {
        	TRVL_MARENA_MONITOR->stop();
        	break;
        }
        case GameEnum::TRVL_WEDDING_SCENE_ID:
        {
            TRVL_WEDDING_MONITOR->stop();
            break;
        }
        case GameEnum::TRVL_RECHARGE_SCENE_ID:
        {
            TRVL_RECHARGE_MONITOR->stop();
            break;
        }
        case GameEnum::LEAGUE_REGION_FIGHT_ID:
        {
        	LRF_MONITOR->stop();
        	break;
        }
        case GameEnum::TRVL_BATTLE_SCENE_ID:
        {
        	TRVL_BATTLE_MONITOR->stop();
        	break;
        }
        case GameEnum::TRVL_PEAK_PRE_SCENE_ID:
        {
            TRVL_PEAK_MONITOR->stop();
            break;
        }
		}
	}

	//保存世界boss
	WORLD_BOSS_SYSTEM->stop_world_boss();
	//进程关闭提示客户端
	PLAYER_MANAGER->notify_all_player_offline();

	this->map_one_sec_timer_->cancel_timer();
	this->map_int_hour_timer_->cancel_timer();
	this->map_mid_night_timer_->cancel_timer();
	this->trvl_keep_alive_timer_->cancel_timer();
	this->ml_int_min_timer_->cancel_timer();
	this->ml_mid_night_timer_->cancel_timer();

    this->map_unit()->stop_wait();
    SUPPER::stop();

    SceneMap::KeyValueMap *key_map;
    if (this->scene_map_.find_object_map(0, key_map) == 0)
    {
        for (SceneMap::KeyValueMap::iterator iter = key_map->begin();
                iter != key_map->end(); ++iter)
        {
            this->scene_factory_->push_scene(iter->second);
        }
    }

    this->scene_map_.unbind_all();
    this->scene_line_manager_->stop();

    return 0;
}

void MapMonitor::fina(void)
{
    this->timer_handler_list_->clear();

    this->player_pool_->clear();
    this->skill_pool_->clear();
    this->status_pool_->clear();
    this->history_status_pool_->clear();
    this->status_queue_node_pool_->clear();
    this->logic_player_pool_->clear();
    this->mail_box_pool_->clear();
    this->task_info_pool_->clear();
    this->task_condition_pool_->clear();
    this->task_imp_pool_->clear();
    this->task_routine_imp_pool_->clear();
    this->task_trial_imp_pool_->clear();
    this->delay_skill_pool_->clear();
    this->script_player_rel_pool_->clear();
    this->script_team_pool_->clear();
    this->scene_ai_record_pool_->clear();
    this->script_ai_pool_->clear();
    this->acti_code_detail_pool_->clear();
    this->map_beast_package_->clear();
    this->skill_cool_package_->clear();
    this->ml_player_assist_package_->clear();
    this->passive_skill_qn_pool_->clear();
    this->team_scene_block_pool_->clear();

    SUPPER::fina();
}

BaseUnit *MapMonitor::map_unit(void)
{
    return this->map_unit_;
}

BaseUnit *MapMonitor::logic_unit(void)
{
    return this->logic_unit_;
}

Block_Buffer *MapMonitor::pop_block(int cid)
{
    return POOL_MONITOR->pop_buf_block(cid);
}

int MapMonitor::push_block(Block_Buffer *buff, int cid)
{
    return POOL_MONITOR->push_buf_block(buff, cid);
}

MapMonitor::UnitMessagePool *MapMonitor::unit_msg_pool(void)
{
    return POOL_MONITOR->unit_msg_pool();
}

int MapMonitor::chat_scene(void)
{
    return this->chat_scene_;
}

int MapMonitor::chat_scene(const int map_scene)
{
	return SUPPER::chat_scene(map_scene);
}

std::string &MapMonitor::chat_ip(void)
{
	return this->chat_ip_;
}

int MapMonitor::chat_port(void)
{
	return this->chat_port_;
}

int MapMonitor::line_id(void)
{
    return this->line_id_;
}

int MapMonitor::average_level()
{
	return this->average_level_;
}

int MapMonitor::find_client_service(const int sid, MapClientService *&svc)
{
	Svc *tmp_svc = 0;
	if (this->client_monitor_.find_service(sid, tmp_svc) != 0)
		return -1;

	svc = dynamic_cast<MapClientService *>(tmp_svc);
	if (svc == 0)
		return -1;
    return 0;
}

int MapMonitor::add_player_label(Int64 role_id, int label_id)
{
	MapPlayerEx* player = NULL;
	this->find_player(role_id, player);

	if (player != NULL)
	{
		player->request_map_logic_add_label(label_id);
	}
	else
	{
		Proto30100227 label_info;

		label_info.set_role_id(role_id);
		label_info.set_label_id(label_id);

		MAP_MONITOR->dispatch_to_logic(&label_info);
	}

	return 0;
}

int MapMonitor::gate_sid_list(IntVec &gate_sid_vc)
{
    return this->inner_sid_list(gate_sid_vc);
}

int MapMonitor::first_gate_sid(void)
{
    return this->first_inner_sid();
}

int MapMonitor::dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, Block_Buffer *msg_buff)
{
    JUDGE_RETURN(gate_sid >= 0, -1);
    JUDGE_RETURN(msg_buff != NULL, -1);

    uint32_t total_len = sizeof(InnerRouteHead) + msg_buff->readable_bytes(); 

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)route_head, sizeof(InnerRouteHead));
    pbuff->copy(msg_buff);

    int ret = this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int MapMonitor::dispatch_to_scene(const int gate_sid, const InnerRouteHead *route_head, const ProtoHead *proto_head, const Message *msg_proto)
{
	JUDGE_RETURN(gate_sid >= 0, -1);

    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead), 
             len = sizeof(ProtoHead), byte_size = 0;
    if (msg_proto != 0)
    {
        byte_size = msg_proto->ByteSize();
    }

    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)route_head, sizeof(InnerRouteHead));
    pbuff->write_uint32(len);
    pbuff->copy((char *)proto_head, sizeof(ProtoHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
    }

    int ret = this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int MapMonitor::dispatch_to_scene(const int gate_sid, const InnerRouteHead &route_head, const ProtoHead &proto_head, Block_Buffer *msg_buff)
{
	JUDGE_RETURN(gate_sid >= 0, -1);

    uint32_t total_len = sizeof(InnerRouteHead) + sizeof(uint32_t) + sizeof(ProtoHead),
             len = sizeof(ProtoHead), byte_size = 0;
    if (msg_buff != 0)
    {
        byte_size = msg_buff->readable_bytes();
    }

    total_len += byte_size;
    len += byte_size;

    Block_Buffer *pbuff = this->pop_block(gate_sid);
    pbuff->ensure_writable_bytes(total_len + sizeof(uint32_t) * 4);
    pbuff->write_int32(gate_sid);
    pbuff->write_uint32(total_len);
    pbuff->copy((char *)(&route_head), sizeof(InnerRouteHead));
    pbuff->write_uint32(len);
    pbuff->copy((char *)(&proto_head), sizeof(ProtoHead));
    if (msg_buff != 0)
    {
        pbuff->copy(msg_buff);
    }

    int ret = this->inner_sender(gate_sid)->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int MapMonitor::dispatch_to_chat(const int64_t role_id, const int recogn, const int scene_id, const Message *msg_proto)
{
    int chat_sid = this->fetch_sid_of_scene(this->chat_scene_);
    JUDGE_RETURN(chat_sid >= 0, -1);

    ProtoHead head;
    head.__recogn = recogn;
    head.__role_id = role_id;
    head.__scene_id = scene_id;

    uint32_t len = sizeof(ProtoHead), byte_size = 0;
    if (msg_proto != 0)
    {
        byte_size = msg_proto->ByteSize();
    }

    len += byte_size;
    Block_Buffer *pbuff = this->pop_block(chat_sid);
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(chat_sid);
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoHead));
    if (msg_proto != 0)
    {
        msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
        pbuff->set_write_idx(pbuff->get_write_idx() + byte_size);
    }

    int ret = this->connect_sender()->push_pool_block_with_len(pbuff);
    if (ret != 0)
    	this->push_block(pbuff);
    return ret;
}

int MapMonitor::dispatch_to_chat(EntityCommunicate* entity, const int recogn)
{
	return this->dispatch_to_chat(entity->entity_id(), recogn, 0, 0);
}

int MapMonitor::dispatch_to_chat(EntityCommunicate* entity, const Message* msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    return this->dispatch_to_chat(entity->entity_id(), recogn, 0, msg_proto);
}

int MapMonitor::dispatch_to_chat(const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    return this->dispatch_to_chat(0, recogn, 0, msg_proto);
}

int MapMonitor::dispatch_to_logic(EntityCommunicate* entity, const int recogn)
{
	return this->dispatch_to_scene(entity, SCENE_LOGIC, recogn);
}

int MapMonitor::dispatch_to_logic(EntityCommunicate* entity, const Message* msg_proto)
{
    return this->dispatch_to_scene(entity, SCENE_LOGIC, msg_proto);
}

int MapMonitor::dispatch_to_logic(const int gate_sid, const int recogn)
{
    return this->dispatch_to_scene_by_noplayer(gate_sid, 0, SCENE_LOGIC, NULL, recogn);
}

int MapMonitor::dispatch_to_logic(const int gate_sid, const Message *msg_proto)
{
	return this->dispatch_to_scene_by_noplayer(gate_sid, 0, SCENE_LOGIC, msg_proto);
}

int MapMonitor::dispatch_to_gate(int gate_sid, int recogn, Message *msg_proto)
{
    InnerRouteHead route_head;
    ProtoHead proto_head;

    route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
    route_head.__recogn = recogn;
    route_head.__scene_id = SCENE_GATE;
    route_head.__inner_req = 1;

    proto_head.__recogn = recogn;
    proto_head.__scene_id = SCENE_GATE;

    return this->dispatch_to_scene(gate_sid, &route_head, &proto_head, msg_proto);
}

// 跨服中使用的no player接口
int MapMonitor::dispatch_to_logic_in_all_server(int recogn)
{
    IntVec gate_sid_vc;
    this->gate_sid_list(gate_sid_vc);

    for (IntVec::iterator iter = gate_sid_vc.begin(); iter != gate_sid_vc.end(); ++iter)
    {
    	this->dispatch_to_logic(*iter, recogn);
    }
    return 0;
}

int MapMonitor::dispatch_to_logic_in_all_server(Message *msg_proto)
{
	IntVec gate_sid_vc;
    this->gate_sid_list(gate_sid_vc);

    for (IntVec::iterator iter = gate_sid_vc.begin(); iter != gate_sid_vc.end(); ++iter)
    {
        this->dispatch_to_logic(*iter, msg_proto);
    }

    return 0;
}

int MapMonitor::dispatch_to_gate_in_all_server(int recogn, Message* msg_proto)
{
	IntVec gate_sid_vc;
	this->gate_sid_list(gate_sid_vc);

    for (IntVec::iterator iter = gate_sid_vc.begin(); iter != gate_sid_vc.end(); ++iter)
    {
    	this->dispatch_to_gate(*iter, recogn, msg_proto);
    }

	return 0;
}

int MapMonitor::dispatch_to_logic(const int recogn)
{
	int gate_sid = this->first_gate_sid();
	JUDGE_RETURN(gate_sid >= 0, -1);

	InnerRouteHead route_head;
	route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;
	route_head.__inner_req = 1;

	ProtoHead head;
	head.__recogn = recogn;
	head.__scene_id = SCENE_LOGIC;

	return this->dispatch_to_scene(gate_sid, &route_head, &head);
}

int MapMonitor::dispatch_to_logic(const Message *msg_proto, int gate_sid, Int64 role_id)
{
	if (gate_sid == -1)
	{
		gate_sid = this->first_gate_sid();
	}
	JUDGE_RETURN(gate_sid >= 0, -1);

	int recogn = type_name_to_recogn(msg_proto->GetTypeName());

	InnerRouteHead route_head;
	route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;
	route_head.__inner_req = 1;

	ProtoHead head;
	head.__recogn = recogn;
	head.__role_id = role_id;
	head.__scene_id = SCENE_LOGIC;

    const ServerDetail &server_detail = CONFIG_INSTANCE->server_list(this->server_config_index_);
    if (server_detail.__scene_list.begin() != server_detail.__scene_list.end())
    	head.__src_scene_id = *(server_detail.__scene_list.begin());
	head.__src_line_id = this->line_id();

	return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}


int MapMonitor::dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id,
		const int recogn, const Message *msg_proto)
{
    InnerRouteHead route_head;
    ProtoHead proto_head;

    route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = scene_id;
    route_head.__inner_req = 1;

    proto_head.__recogn = recogn;
    proto_head.__role_id = role_id;
    proto_head.__scene_id = scene_id;

    return this->dispatch_to_scene(gate_sid, &route_head, &proto_head, msg_proto);
}

int MapMonitor::dispatch_to_scene(const int gate_sid, const int64_t role_id, const int scene_id, const Message *msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());
    return this->dispatch_to_scene(gate_sid, role_id, scene_id, recogn, msg_proto);
}

int MapMonitor::dispatch_to_scene(EntityCommunicate *entity, const int scene_id, const int recogn)
{
    return this->dispatch_to_scene(entity->gate_sid(), entity->entity_id(), scene_id, recogn);
}

int MapMonitor::dispatch_to_scene(EntityCommunicate *entity, const int scene_id, const Message *msg_proto)
{
    return this->dispatch_to_scene(entity->gate_sid(), entity->entity_id(), scene_id, msg_proto);
}

int MapMonitor::dispatch_to_scene_by_gate(int gate_sid, Int64 role_id, const Message* msg_proto)
{
	return this->dispatch_to_scene(gate_sid, role_id, 0, msg_proto);
}

int MapMonitor::dispatch_to_scene(int scene_id, const Message *msg_proto)
{
	int gate_sid = this->first_gate_sid();
	JUDGE_RETURN(gate_sid >= 0, -1);

	return this->dispatch_to_scene_by_noplayer(gate_sid, 0, scene_id, msg_proto);
}

int MapMonitor::dispatch_to_scene_by_noplayer(int scene_id, int recogn)
{
	int gate_sid = this->first_gate_sid();
	JUDGE_RETURN(gate_sid >= 0, -1);

	return this->dispatch_to_scene_by_noplayer(gate_sid, 0, scene_id, NULL, recogn);
}

int MapMonitor::dispatch_to_scene_by_noplayer(EntityCommunicate *entity, int scene_id,
    		const Message *msg_proto)
{
	return this->dispatch_to_scene_by_noplayer(entity->gate_sid(), entity->entity_id(), scene_id, msg_proto);
}

int MapMonitor::dispatch_to_scene_by_noplayer(int gate_sid, Int64 role_id, int scene_id,
		const Message* msg_proto, int recogn)
{
	if (msg_proto != NULL)
	{
		recogn = type_name_to_recogn(msg_proto->GetTypeName());
	}

	InnerRouteHead route_head;
	route_head.__broad_type = BT_NOPLAYER_TARGET_SCENE;
	route_head.__inner_req = 1;
	route_head.__recogn = recogn;

	ProtoHead head;
	head.__recogn = recogn;
	head.__role_id = role_id;
	head.__scene_id = scene_id;

	return this->dispatch_to_scene(gate_sid, &route_head, &head, msg_proto);
}

int MapMonitor::dispatch_to_client_from_gate(const int gate_sid, const Int64 role_id, const int recogn, const int error)
{
    InnerRouteHead route_head;
    ProtoHead proto_head;

    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    proto_head.__recogn = recogn;
    proto_head.__role_id = role_id;
    proto_head.__scene_id = 1;
    proto_head.__error = error;

    return this->dispatch_to_scene(gate_sid, &route_head, &proto_head);
}

int MapMonitor::dispatch_to_client_from_gate(const int gate_sid, const Int64 role_id, const Message *msg_proto, const int error)
{
    InnerRouteHead route_head;
    ProtoHead proto_head;

    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__recogn = recogn;
    route_head.__role_id = role_id;
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    proto_head.__recogn = recogn;
    proto_head.__role_id = role_id;
    proto_head.__scene_id = 1;
    proto_head.__error = error;

    return this->dispatch_to_scene(gate_sid, &route_head, &proto_head, msg_proto);
}

int MapMonitor::dispatch_to_client_from_gate(GameMover *mover, Block_Buffer *msg_buff)
{
    InnerRouteHead route_head;

    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__role_id = mover->mover_id();
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    return this->dispatch_to_scene(mover->gate_sid(), &route_head, msg_buff);
}

int MapMonitor::dispatch_to_client_from_gate(GameMover *mover, const int recogn, const int error)
{
    return this->dispatch_to_client_from_gate(mover->gate_sid(), mover->mover_id(), recogn, error);
}

int MapMonitor::dispatch_to_client_from_gate(GameMover *mover, const Message *msg_proto, const int error)
{
    return this->dispatch_to_client_from_gate(mover->gate_sid(), mover->mover_id(), msg_proto, error);
}

int MapMonitor::dispatch_to_client_from_gate(MapLogicPlayer *player, Block_Buffer *msg_buff)
{
    InnerRouteHead route_head;

    route_head.__broad_type = BT_DIRECT_CLIENT;
    route_head.__role_id = player->role_id();
    route_head.__scene_id = 1;
    route_head.__inner_req = 1;

    return this->dispatch_to_scene(player->gate_sid(), &route_head, msg_buff);
}

int MapMonitor::dispatch_to_client_from_gate(MapLogicPlayer *player, const int recogn, const int error)
{
    return this->dispatch_to_client_from_gate(player->gate_sid(), player->role_id(), recogn, error);
}

int MapMonitor::dispatch_to_client_from_gate(MapLogicPlayer *player, const Message *msg_proto, const int error)
{
    return this->dispatch_to_client_from_gate(player->gate_sid(), player->role_id(), msg_proto, error);
}

int MapMonitor::dispatch_to_client_direct(GameMover *mover, Block_Buffer *msg_buff)
{
#ifdef NO_BROAD_PORT
	if (mover->gate_sid() <= 0)
	{
        MSG_USER("ERROR mover no connect gate socket %ld", mover->mover_id());
        return -1;
	}
	if (msg_buff != 0)
	{
		int total_len = msg_buff->readable_bytes() + sizeof(InnerRouteHead) + sizeof(int) + sizeof(ProtoHead);
		if (total_len >= this->inner_monitor_.svc_max_pack_size())
		{
			LOG_USER_INFO("ERROR broad data too long %d %d", total_len, this->inner_monitor_.svc_max_pack_size());
		}
	}

	InnerRouteHead route_head;
	ProtoHead head;
	route_head.__broad_type = BT_BROAD_CLIENT;
	route_head.__role_id = mover->mover_id();
	route_head.__scene_id = mover->scene_id();
	route_head.__inner_req = 1;

	head.__role_id = mover->mover_id();
	head.__scene_id = mover->scene_id();
	return this->dispatch_to_scene(mover->gate_sid(), route_head, head, msg_buff);
#else
    if (mover->client_sid() <= 0)
    {
        MSG_USER("ERROR mover no connect broad socket %ld", mover->mover_id());
        return -1;
    }
    return this->client_sender(mover->client_sid())->push_data_block_with_len(mover->client_sid(), *msg_buff);
#endif
}

int MapMonitor::dispatch_to_client_direct(GameMover *mover, const int recogn, const int error)
{
#ifdef NO_BROAD_PORT

	Block_Buffer *buff = this->pop_block();
	int32_t len = sizeof(ProtoClientHead);
	buff->ensure_writable_bytes(len + sizeof(int) * 4);

	ProtoClientHead head;
	head.__recogn = recogn;
	head.__error = error;
	mover->make_up_client_block(buff, &head, 0);
    int ret = this->dispatch_to_client_direct(mover, buff);
    this->push_block(buff);
    return ret;
//	return this->dispatch_to_client_from_gate(mover, recogn, error);
#else
    if (mover->client_sid() <= 0)
    {
        MSG_USER("ERROR mover no connect broad socket %ld", mover->mover_id());
        return -1;
    }
    uint32_t len = sizeof(ProtoClientHead);
    ProtoClientHead head;

    head.__error = error;
    head.__recogn = recogn;

    Block_Buffer *pbuff = this->pop_block(mover->client_sid());
    pbuff->write_int32(mover->client_sid());
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));

    return this->client_sender(mover->client_sid())->push_pool_block_with_len(pbuff);
#endif
}

int MapMonitor::dispatch_to_client_direct(GameMover *mover, const Message *msg_proto, const int error)
{
#ifdef NO_BROAD_PORT
	Block_Buffer *buff = this->pop_block();
	int32_t len = sizeof(ProtoClientHead);
	buff->ensure_writable_bytes(len + sizeof(int) * 4);

	ProtoClientHead head;
	head.__recogn = type_name_to_recogn(msg_proto->GetTypeName());
	head.__error = error;
	mover->make_up_client_block(buff, &head, msg_proto);
    int ret = this->dispatch_to_client_direct(mover, buff);
    this->push_block(buff);
    return ret;
//	return this->dispatch_to_client_from_gate(mover, msg_proto, error);
#else
    if (mover->client_sid() <= 0)
    {
        MSG_USER("mover no connect broad socket %ld", mover->mover_id());
        return -1;
    }
    int recogn = type_name_to_recogn(msg_proto->GetTypeName());

    uint32_t len = sizeof(ProtoClientHead), bype_size = 0;
    bype_size = msg_proto->ByteSize();
    len += bype_size;

    ProtoClientHead head;
    head.__error = error;
    head.__recogn = recogn;

    Block_Buffer *pbuff = this->pop_block(mover->client_sid());
    pbuff->ensure_writable_bytes(len + sizeof(uint32_t) * 4);
    pbuff->write_int32(mover->client_sid());
    pbuff->write_uint32(len);
    pbuff->copy((char *)&head, sizeof(ProtoClientHead));

    msg_proto->SerializeToArray(pbuff->get_write_ptr(), pbuff->writable_bytes());
    pbuff->set_write_idx(pbuff->get_write_idx() + bype_size);

    return this->client_sender(mover->client_sid())->push_pool_block_with_len(pbuff);
#endif
}

int MapMonitor::process_inner_logic_request(const int64_t role_id, const int recogn)
{
    UnitMessage unit_msg;

    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;
    unit_msg.set_msg_proto(0);

    return this->logic_unit()->push_request(unit_msg);
}

int MapMonitor::process_inner_logic_request(const int64_t role_id, Message &msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto.GetTypeName());
    Message *request = create_message(msg_proto.GetTypeName());

    JUDGE_RETURN(request != NULL, -1);
    request->CopyFrom(msg_proto);

    UnitMessage unit_msg;
    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;
    unit_msg.set_msg_proto(request);

    if (this->logic_unit()->push_request(unit_msg) != 0)
    {
    	if (request != 0)
    		delete request;
    }
    return 0;
}

int MapMonitor::process_inner_map_request(const int64_t role_id, const int recogn)
{
    UnitMessage unit_msg;
    unit_msg.set_msg_proto(0);

    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;

    return this->map_unit()->push_request(unit_msg);
}

int MapMonitor::process_inner_map_request(const int64_t role_id, Message &msg_proto)
{
    int recogn = type_name_to_recogn(msg_proto.GetTypeName());
    Message *request = create_message(msg_proto.GetTypeName());

    JUDGE_RETURN(request != NULL, -1);
    request->CopyFrom(msg_proto);

    UnitMessage unit_msg;
    unit_msg.set_msg_proto(request);

    unit_msg.__msg_head.__recogn = recogn;
    unit_msg.__msg_head.__role_id = role_id;

    if (this->map_unit()->push_request(unit_msg) != 0)
    {
    	if (request != 0)
    		delete request;
    }
    return 0;
}

MapMonitor::MapPlayerPool *MapMonitor::player_pool(void)
{
    return this->player_pool_;
}

FighterSkill* MapMonitor::pop_skill(int skill_id, int level)
{
	FighterSkill* skill = this->skill_pool()->pop();
	skill->__skill_id = skill_id;
	this->update_skill_level(skill, level);
    return skill;
}

void MapMonitor::update_skill_level(FighterSkill* skill, int level)
{
	if (level == 0)
	{
		level = skill->__level;
	}
	else
	{
		skill->__level = level;
	}

	const Json::Value &skill_json = skill->conf();
    skill->__aoe_type = skill_json["aoeType"].asInt();
    skill->__target_type = skill_json["type"].asString();
    skill->__use_level = skill_json["useLvl"].asInt();
    skill->__skill_type = skill_json["server_type"].asInt();
    skill->__transfer_no_release = skill_json["transfer_no_release"].asInt();
    skill->__del_buff = skill_json["del_buff"].asInt();

	skill->__object = skill_json["object"].asInt();
	skill->__radius = skill_json["radius"].asInt();
	skill->__distance = skill_json["distance"].asInt();
	skill->__launch_way = skill_json["launch_way"].asInt();
	skill->__is_mutual = skill_json["is_mutual"].asInt();
	skill->__is_loop = skill_json["is_loop"].asInt();
	skill->__full_screen = skill_json["fullscreen"].asInt();
	skill->__effect_ai_skill = skill_json["effect_ai_skill"].asInt();
	skill->__passive_trigger = skill_json["passive_trigger"].asInt();
	skill->__max_times = skill_json["max_times"].asInt();

	skill->__need_used_times = 0;
	skill->__no_object_limit = skill_json["no_object_limit"].asInt();
	skill->__object_from_server = skill_json["object_from_server"].asInt();
	skill->__cool = GameCommon::fetch_time_value(skill_json["cool"].asDouble());

    if (skill_json.isMember("launch_once") == true)
    {
        skill->__is_launch_once = (skill_json["launch_once"].asInt() != 0);
    }

    if (skill_json.isMember("db_flag") == true)
    {
    	skill->__db_flag = skill_json["db_flag"].asInt();
    }

    if (skill_json.isMember("check_flag") == true)
    {
    	skill->__check_flag = skill_json["check_flag"].asInt();
    }

    if (skill_json.isMember("level_type") == true)
    {
    	skill->__level_type = skill_json["level_type"].asInt();
    }

    if (skill_json.isMember("sub_rate_skill") == true)
    {
    	skill->__sub_rate_skill = skill_json["sub_rate_skill"].asInt();
    }

    if (skill_json.isMember("sub_rate_skill_2") == true)
    {
    	skill->__sub_rate_skill_2 = skill_json["sub_rate_skill_2"].asInt();
    }

    if (skill_json.isMember("skilled_times") == true)
    {
    	skill->__need_used_times = GameCommon::json_by_level(
    			skill_json["skilled_times"], level).asInt();
    }

    if (skill_json.isMember("use_rate") == true)
    {
    	skill->__use_rate = GameCommon::json_by_level(
    			skill_json["use_rate"], level).asInt();
    }

    if (skill_json.isMember("server_force") == true)
    {
    	skill->__server_force = GameCommon::json_by_level(
    			skill_json["server_force"], level).asInt();
    }

    if (skill_json.isMember("max_step") == true)
    {
    	skill->__max_step = skill_json["max_step"].asInt();
    }

    if (skill_json.isMember("rand_step") == true)
    {
       	skill->__rand_step = skill_json["rand_step"].asInt();
    }
}

MapMonitor::SkillPool *MapMonitor::skill_pool(void)
{
    return this->skill_pool_;
}

MapMonitor::StatusPool *MapMonitor::status_pool(void)
{
    return this->status_pool_;
}

MapMonitor::HistoryStatusPool *MapMonitor::history_status_pool(void)
{
    return this->history_status_pool_;
}

MapMonitor::StatusQueueNodePool *MapMonitor::status_queue_node_pool(void)
{
    return this->status_queue_node_pool_;
}

MapMonitor::LogicPlayerPool *MapMonitor::logic_player_pool(void)
{
    return this->logic_player_pool_;
}

MapMonitor::MailBoxPool *MapMonitor::mail_box_pool(void)
{
    return this->mail_box_pool_;
}

MapMonitor::TaskInfoPool *MapMonitor::task_info_pool(void)
{
	return this->task_info_pool_;
}

MapMonitor::TaskConditionPool *MapMonitor::task_condition_pool(void)
{
    return this->task_condition_pool_;
}

MapMonitor::TaskImplementPool *MapMonitor::task_imp_pool(void)
{
    return this->task_imp_pool_;
}

MapMonitor::TaskRoutineImpPool *MapMonitor::task_routine_imp_pool(void)
{
    return this->task_routine_imp_pool_;
}

MapMonitor::TaskTrialImpPool *MapMonitor::task_trial_imp_pool(void)
{
    return this->task_trial_imp_pool_;
}

MapMonitor::DelaySkillPool *MapMonitor::delay_skill_pool(void)
{
    return this->delay_skill_pool_;
}

MapMonitor::ScriptPlayerRelPool *MapMonitor::script_player_rel_pool(void)
{
    return this->script_player_rel_pool_;
}

MapMonitor::ScriptTeamDetailPool *MapMonitor::script_team_detail_pool(void)
{
    return this->script_team_pool_;
}

MapMonitor::SceneAIRecordPool *MapMonitor::scene_ai_record_pool(void)
{
    return this->scene_ai_record_pool_;
}

MapMonitor::ScriptAIPool *MapMonitor::script_ai_pool(void)
{
    return this->script_ai_pool_;
}

MapMonitor::ActiCodeDetailPool* MapMonitor::acti_code_detail_pool(void)
{
	return this->acti_code_detail_pool_;
}

MapMonitor::PassiveSkillQNPool *MapMonitor::passive_skill_qn_pool(void)
{
    return this->passive_skill_qn_pool_;
}

MapMonitor::SceneBlockPool *MapMonitor::team_scene_block_pool(void)
{
    return this->team_scene_block_pool_;
}

MapMonitor::MapBeastPackage *MapMonitor::map_beast_package(void)
{
	return this->map_beast_package_;
}

MapMonitor::MLPlayerAssistPackage *MapMonitor::ml_player_assist_package(void)
{
	return this->ml_player_assist_package_;
}

void MapMonitor::reset_everyday()
{
	this->item_drop_limit_.clear();
}

void MapMonitor::report_pool_info(const bool report)
{
    {
        std::ostringstream msg_stream;
        POOL_MONITOR->report_pool_info(msg_stream);
        if (this->scene_factory_ != 0)
            this->scene_factory_->report_pool_info(msg_stream);
        if (this->script_factory_ != 0)
            this->script_factory_->report_pool_info(msg_stream);

        AIMANAGER->report_pool_info(msg_stream);
        MSG_USER("%s", msg_stream.str().c_str());
    }

    std::ostringstream msg_stream;

    char tick_buff[2048];
    ::snprintf(tick_buff, 2048, "fight _ick: %ld.%06ld; move_tick: %ld.%06ld; ai_tick:%ld.%06ld",
            this->fight_total_use_.sec(), this->fight_total_use_.usec(),
            this->move_total_use_.sec(), this->move_total_use_.usec(),
            this->ai_total_use_.sec(), this->ai_total_use_.usec());
    msg_stream << tick_buff << std::endl;
    msg_stream << "Map Pool Info:" << std::endl;

    msg_stream << "ClientServicePool:" << std::endl;
    this->client_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Inner Service Pool:" << std::endl;
    this->inner_monitor_.service_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Connect Service Pool:" << std::endl;
    this->connect_monitor_.service_pool()->dump_info_to_stream(msg_stream);

    msg_stream << "SessionDetailPool:" << std::endl;
    this->session_manager()->session_pool()->dump_info_to_stream(msg_stream);

    msg_stream << "Map Player Pool" << std::endl;
    this->player_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Skill Pool" << std::endl;
    this->skill_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Status Pool" << std::endl;
    this->status_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "HistoryStatus Pool" << std::endl;
    this->history_status_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "Status Queue Node Pool" << std::endl;
    this->status_queue_node_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "MapLogicPlayerPool" << std::endl;
    this->logic_player_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "MailBox Pool" << std::endl;
    this->mail_box_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "TaskInfo Pool" << std::endl;
    this->task_info_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "TaskCondition Pool" << std::endl;
    this->task_condition_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "TaskImplement Pool" << std::endl;
    this->task_imp_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "TaskRoutineImp Pool" << std::endl;
    this->task_routine_imp_pool()->dump_info_to_stream(msg_stream);
    msg_stream << "DelaySkill Pool" << std::endl;
    this->delay_skill_pool()->dump_info_to_stream(msg_stream);

    if (this->script_player_rel_pool() != 0)
    {
        msg_stream << "ScriptPlayerRel Pool" << std::endl;
        this->script_player_rel_pool()->dump_info_to_stream(msg_stream);
    }
    if (this->script_team_pool_ != 0)
    {
        msg_stream << "ScriptTeamDetail Pool" << std::endl;
        this->script_team_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->scene_ai_record_pool_ != 0)
    {
        msg_stream << "SceneAIRecord Pool" << std::endl;
        this->scene_ai_record_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->script_ai_pool_ != 0)
    {
        msg_stream << "ScriptAI Pool" << std::endl;
        this->script_ai_pool_->dump_info_to_stream(msg_stream);
    }
    if (this->acti_code_detail_pool_ != 0)
    {
        msg_stream << "ActiCodeDetail Pool" << std::endl;
        this->acti_code_detail_pool_->dump_info_to_stream(msg_stream);
    }
    if (report == false)
        return;
    MSG_USER("%s", msg_stream.str().c_str());
}

SessionManager *MapMonitor::session_manager(void)
{
    return this->session_manager_;
}

PlayerManager *MapMonitor::player_manager(void)
{
    return this->player_manager_;
}

SceneLineManager *MapMonitor::scene_line_manager(void)
{
    return this->scene_line_manager_;
}

int MapMonitor::notify_all_player_info(int recogn, Message* msg)
{
	Proto30100104 notify_info;
	notify_info.set_recogn(recogn);

	if (msg != NULL)
	{
		notify_info.set_msg(msg->SerializeAsString());
	}

	return this->dispatch_to_logic(&notify_info);
}

int MapMonitor::request_map_player_login(const int gate_sid, const int64_t role_id, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30400004*, request, -1);

    SESSION_MANAGER->update_session(gate_sid, request->session_info().account(),
    		request->session_info().session(), role_id);

    MSG_USER("MapMonitor begin load %ld", role_id);
    return this->player_manager()->request_map_player_login(gate_sid, role_id, msg);
}

int MapMonitor::after_load_player(Transaction *transaction)
{
    return this->player_manager()->after_load_player(transaction);
}

int MapMonitor::bind_player(const int64_t role_id, MapPlayerEx *player)
{
    return this->player_manager()->bind_player(role_id, player);
}

int MapMonitor::unbind_player(const int64_t role_id)
{
    return this->player_manager()->unbind_player(role_id);
}

int MapMonitor::find_player(const int64_t role_id, MapPlayerEx *&player)
{
    JUDGE_RETURN(this->player_manager()->find_player(role_id, player) == 0, -1);

//	JUDGE_RETURN(player->is_same_thread() == true, -1);
	JUDGE_RETURN(player->is_need_send_message() == true, -1);

    return 0;
}

int MapMonitor::find_player_with_offline(const int64_t role_id, MapPlayerEx *&player)
{
    JUDGE_RETURN(this->player_manager()->find_player(role_id, player) == 0, -1);
//	JUDGE_RETURN(player->is_same_thread() == true, -1);
	return 0;
}

int MapMonitor::player_online_flag(Int64 role_id)
{
	MapPlayerEx* player = NULL;
	JUDGE_RETURN(this->find_player(role_id, player) == 0, false);
	return true;
}

MapPlayerEx* MapMonitor::find_map_player(Int64 role_id)
{
	MapPlayerEx* player = NULL;
	this->find_player(role_id, player);
	return player;
}

MapLogicPlayer* MapMonitor::find_logic_player(Int64 role_id)
{
	JUDGE_RETURN(role_id > 0, NULL);

	MapLogicPlayer* player = NULL;
	this->player_manager_->find_logic_player(role_id, player);

	return player;
}

int MapMonitor::get_logic_player_set(PlayerManager::LogicPlayerSet& logic_player_set)
{
	return this->player_manager()->get_logic_player_set(logic_player_set);
}

int MapMonitor::update_map_player_sid(int sid, Int64 role_id, Message* msg)
{
	MapPlayerEx* player = this->find_map_player(role_id);
	JUDGE_RETURN(player != NULL && player->is_enter_scene() == true, -1);

	player->set_gate_sid(sid);
	MSG_USER("update map player sid %d, %ld, %s", sid, player->role_id(), player->name());

	return 0;
}

int MapMonitor::update_logic_player_sid(int sid, Int64 role_id, Message* msg)
{
	MapLogicPlayer* player = this->find_logic_player(role_id);
	JUDGE_RETURN(player != NULL && player->is_enter_scene() == true, -1);

	player->set_gate_sid(sid);
	MSG_USER("update map loigc player sid %d, %ld, %s", sid, player->role_id(), player->name());
	return 0;
}

int64_t MapMonitor::connect_map_broad(const int broad_sid, Message *msg_proto)
{
    Proto10400001 *request = dynamic_cast<Proto10400001 *>(msg_proto);
    if (request == 0)
    {
        MSG_USER("ERROR validate broad msg null %d", broad_sid);
        return -1;
    }

    MapClientService *svc = 0;
    if (this->find_client_service(broad_sid, svc) != 0)
    {
        MSG_USER("ERROR validate broad no svc %d", broad_sid);
        return -1;
    }

    int64_t role_id = request->role_id();
    MapPlayerEx *player = 0;
    if (this->find_player(role_id, player) != 0)
    {
        MSG_USER("ERROR validate broad svc %ld %d", role_id, broad_sid);
        svc->handle_close();
        return -1;
    }

    std::string account = request->account(), 
        str_session = request->session();

    if (account != player->role_detail().__account)
    {
        MSG_USER("ERROR validate broad svc session account %s %s", account.c_str(), player->role_detail().__account.c_str());
        svc->handle_close();
        return -1;
    }
    SessionDetail *session = 0;
    if (this->session_manager()->find_account_session(account, session) != 0)
    {
        MSG_USER("ERROR validate broad svc no session %s", account.c_str());
        svc->handle_close();
        return -1;
    }

    if (session->__session != str_session)
    {
        MSG_USER("ERROR validate broad svc session flag %s %s %s", account.c_str(), str_session.c_str(), session->__session.c_str());
        svc->handle_close();
        return -1;
    }
    session->__client_sid = broad_sid;
    svc->get_local_addr(session->__address, session->__port);

    this->player_manager()->bind_sid_player(broad_sid, player);
    player->set_client_sid(broad_sid);
    return 0;
}

int MapMonitor::disconnect_map_broad(const int broad_sid)
{
    MapPlayerEx *player = 0;
    if (this->player_manager()->unbind_sid_player(broad_sid, player) == 0)
    {
        if (player->client_sid() == broad_sid && player->is_enter_scene() == true)
            player->set_client_sid(0);
    }
    return 0;
}

SceneFactory *MapMonitor::scene_factory(void)
{
    return this->scene_factory_;
}

Scene *MapMonitor::pop_scene(const int scene_id)
{
    return this->scene_factory()->pop_scene(scene_id);
}

int MapMonitor::push_scene(Scene *scene)
{
    return this->scene_factory()->push_scene(scene);
}

int MapMonitor::bind_scene(const int space_id, const int scene_id, Scene *scene)
{
    return this->scene_map_.bind(space_id, scene_id, scene);
}

int MapMonitor::unbind_scene(const int space_id, const int scene_id)
{
    return this->scene_map_.unbind(space_id, scene_id);
}

int MapMonitor::find_scene(const int space_id, const int scene_id, Scene *&scene)
{
    return this->scene_map_.find(space_id, scene_id, scene);
}

int MapMonitor::process_init_scene(const int scene_id, const int config_index, const int space_id)
{
    if (config_index == this->server_config_index_ && this->is_normal_scene(scene_id) == true)
    {
        // 没有分线的场景直接把space_id置为0;
        if (this->scene_line_manager_->init_line_scene(scene_id) != 0)
        {
            return this->init_scene(scene_id, 0);
        }
    }
    return 0;
}

int MapMonitor::init_scene(const int scene_id, const int space_id)
{
    if (this->is_normal_scene(scene_id) == true)
    {
        Scene *scene = this->pop_scene(scene_id);
        JUDGE_RETURN(scene != NULL, -1);

        if (scene->init_scene(space_id, scene_id) != 0)
        {
            this->push_scene(scene);
            return -1;
        }

        if (this->bind_scene(space_id, scene_id, scene) != 0)
        {
            this->push_scene(scene);
            return -1;
        }

        MSG_USER("bind normal scene %d %d %x", space_id, scene_id, scene);
        scene->start_scene();
    }

    return 0;
}

int MapMonitor::check_and_run_scene_monster()
{
	for (SceneMap::iterator scene_iter = this->scene_map_.begin();
			scene_iter != this->scene_map_.end(); ++scene_iter)
	{
		for (SceneMap::KeyValueMap::iterator space_iter = scene_iter->second->begin();
				space_iter != scene_iter->second->end(); ++space_iter)
		{
			int scene_id = space_iter->second->scene_id();

			if (this->is_normal_scene(scene_id) == true)
			{
				space_iter->second->run_scene();
			}
		}
	}

	MSG_USER("MapMonitor check_and_run_scene_monster...");
	return 0;
}

bool MapMonitor::is_normal_scene(const int scene_id)
{
    return GameCommon::is_normal_scene(scene_id);
}

bool MapMonitor::is_script_scene(const int scene_id)
{
	return GameCommon::is_script_scene(scene_id);
}

int MapMonitor::generate_ai_id(void)
{
	this->generate_ai_id_ += 1;
    return BASE_OFFSET_MONSTER + this->generate_ai_id_;
}

int MapMonitor::generate_drop_id(void)
{
	this->generate_drop_id_ += 1;
	return BASE_OFFSET_AIDROP + this->generate_drop_id_;
}

int MapMonitor::generate_camp_id(void)
{
	this->generate_camp_id_ += 1;
	return this->generate_camp_id_;
}

int MapMonitor::generate_beast_id(void)
{
	this->generate_beast_id_ += 1;
	return BASE_OFFSET_BEAST + this->generate_beast_id_;
}

int MapMonitor::generate_role_copy_id(void)
{
	this->generate_role_copy_id_ = (this->generate_role_copy_id_ + 1) % BASE_OFFSET_COPY_ROLE;
	return BASE_OFFSET_COPY_ROLE + this->generate_role_copy_id_;
}

int MapMonitor::generate_beast_copy_id(void)
{
	this->generate_beast_copy_id_ = (this->generate_beast_copy_id_ + 1) % BASE_OFFSET_COPY_BEAST;
	return BASE_OFFSET_COPY_BEAST + this->generate_beast_copy_id_;
}

int MapMonitor::generate_effect_id(void)
{
	this->generate_effect_id_ += 1;
    return BASE_OFFSET_MONSTER + this->generate_effect_id_;
}

int MapMonitor::request_enter_scene_begin(int sid, Int64 role_id, Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400051*, request, -1);

	IntPair pair;
	switch (request->enter_type())
	{
	case GameEnum::ET_LEAGUE_WAR:
	{
		pair.first = RETURN_ENTER_LEAGUE_WAR;
		pair.second = LEAGUE_WAR_SYSTEM->request_enter_lwar(sid, request);
		break;
	}

	case GameEnum::ET_MONSTER_ATTACK:
	{
		pair.first = RETURN_MATTACK_REQUEST_ENTER;
		pair.second = MONSTER_ATTACK_SYSTEM->request_enter_mattack(sid, request);
		break;
	}

	case GameEnum::ET_SM_BATTLE:
	{
		pair.first = RETURN_JOIN_SM_BATTLE;
		pair.second = SM_BATTLE_SYSTEM->request_enter_battle(sid, request);
		break;
	}

	case GameEnum::ET_WORLD_BOSS:
	{
		pair.first = RETURN_ENTER_WORLD_BOSS;
		pair.second = WORLD_BOSS_SYSTEM->request_enter_wboss(sid, request);
		break;
	}

	case GameEnum::ET_TRVL_WBOSS:
	{
		pair.first = RETURN_ENTER_TRVL_WBOSS_PRE;
		pair.second = TRVL_WBOSS_MONITOR->request_enter_trvl_wboss(sid, request);
		break;
	}

	case GameEnum::ET_LEAGUE_BOSS:
	{
		pair.first = RETURN_ENTER_LEAGUE_BOSS;
		pair.second = LEAGUE_MONITOR->request_enter_league_boss(sid, request);
		break;
	}

	case GameEnum::ET_COLLECT_CHESTS:
	{
		pair.first = RETURN_COLLECT_CHESTS_ENTER;
		pair.second = COLLECTCHESTS_INSTANCE->request_enter_collect_chests(sid, request);
		break;
	}

	case GameEnum::ET_ANSWER_ACTIVITY:
	{
		pair.first = RETURN_ANSWER_ACTIVITY_ENTER;
		pair.second = ANSWERACTIVITY_INSTANCE->request_enter_answer_activity(sid, request);
		break;
	}

	case GameEnum::ET_HOTSPRING_ACTIVITY:
	{
		pair.first = RETURN_HOTSPRING_ACTIVITY_ENTER;
		pair.second = HOTSPRING_INSTANCE->request_enter_hotspring_activity(sid, request);
		break;
	}

	case GameEnum::ET_TRAVEL_ARENA:
	{
		pair.first = RETURN_TRVL_ARENA_ENTER;
		pair.second = TRVL_ARENA_MONITOR->request_enter_tarena(sid, request);
		break;
	}

	case GameEnum::ET_TM_ARENA:
	{
		pair.first = RETURN_TRVL_MARENA_ENTER;
		pair.second = TRVL_MARENA_MONITOR->request_enter_tmarena(sid, request);
		break;
	}

	case GameEnum::ET_LEAGUE_REGION_WAR:
	{
		pair.first = RETURN_REQUEST_ENTER_WAR;
		pair.second = LRF_MONITOR->request_enter_lrf(sid, request);
		break;
	}
    case GameEnum::ET_TRVL_BATTLE:
    {
        pair.first = RETURN_TRVL_BATTLE_ENTER;
        pair.second = TRVL_BATTLE_MONITOR->request_enter_battle(sid, request);
        break;
    }
    case GameEnum::ET_TRVL_PEAK:
    {
    	pair.first = RETURN_TRVL_PEAK_ENTER;
    	pair.second = TRVL_PEAK_MONITOR->request_enter_travel_peak(sid, request);
    	break;
    }

	default:
	{
		break;
	}
	}

	JUDGE_RETURN(pair.second != 0, 0);
	return this->dispatch_to_client_from_gate(sid, role_id, pair.first, pair.second);
}

int MapMonitor::respond_enter_scene_begin(int sid, Message* req, Message* info)
{
	Proto30400051 *request = dynamic_cast<Proto30400051 *>(req);
	Proto30400052 *enter_info = dynamic_cast<Proto30400052 *>(info);

	enter_info->set_enter_type(request->enter_type());
	enter_info->set_scene_id(request->request_scene());

	InnerRouteHead route_head;
	route_head.__broad_type = BT_DIRECT_TARGET_SCENE;
	route_head.__scene_id = request->scene_id();

	ProtoHead proto_head;
	proto_head.__role_id = request->role_id();
	proto_head.__scene_id = request->scene_id();
	proto_head.__recogn =  INNER_MAP_RES_ENTER_SCENE;

	return this->dispatch_to_scene(sid, &route_head, &proto_head, enter_info);
}

int MapMonitor::check_add_item_drop_limit(Int64 role, int scene_id, const ItemObj& obj)
{
	JUDGE_RETURN(this->is_normal_scene(scene_id) == true, false);

	const Json::Value& item_conf = CONFIG_INSTANCE->item(obj.id_);
	JUDGE_RETURN(item_conf.isMember("drop_limit") == true, false);

	ThreeObj& three_obj = this->item_drop_limit_[role];
	int max_amount = item_conf["drop_limit"].asInt();

	if (three_obj.sub_map_[obj.id_] < max_amount)
	{
		three_obj.sub_map_[obj.id_] += obj.amount_;
		return false;
	}
	else
	{
		return true;
	}
}

int MapMonitor::inner_notify_player_assist_event(MapPlayer *player, int event_id, int event_value)
{
    JUDGE_RETURN(player != NULL, -1);

    Proto31401710 inner_req;
    inner_req.set_event_id(event_id);
    inner_req.set_event_value(event_value);
    return player->send_to_logic_thread(inner_req);
}

int MapMonitor::db_load_mode_begin(int trans_recogn, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(trans_recogn, this->logic_unit(), role_id);
}

int MapMonitor::db_load_mode_begin(DBShopMode* shop_mode, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(shop_mode, this->logic_unit(), role_id);
}

int MapMonitor::db_load_mode_done(Transaction* trans)
{
	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
	JUDGE_RETURN(trans_data != NULL, -1);

	DBShopMode* shop_mode = trans_data->__data.__shop_mode;

	MapLogicPlayer* player = this->find_logic_player(trans->role_id());
	if(trans->role_id() != 0 && player == NULL)
	{
		MSG_USER("ERROR!!! find_logic_player(%ld)", trans->role_id());
		return trans->summit();;
	}

	switch(shop_mode->recogn_)
	{
	case TRANS_PLAYER_ROLE_RENAME:
	{
		player->request_rename_role_done(shop_mode);
		break;
	}

	default:
	{
		break;
	}
	}

    return trans->summit();
}

int MapMonitor::db_map_load_mode_begin(DBShopMode* shop_mode, Int64 role_id)
{
	return GameCommon::db_load_mode_begin(shop_mode, this->map_unit(), role_id);
}

int MapMonitor::db_map_load_mode_done(Transaction* trans)
{
	TransactionData *trans_data = trans->fetch_data(DB_SHOP_LOAD_MODE);
	JUDGE_RETURN(trans_data != NULL, -1);

	DBShopMode* shop_mode = trans_data->__data.__shop_mode;
	switch(shop_mode->recogn_)
	{
	case TRANS_LOAD_COPY_PLAYER:
	{
		BaseScript::start_copy_player(shop_mode);
		break;
	}
	case TRANS_LOAD_AREA_COPY_PLAYER:
	{
		BaseScript::start_copy_player(shop_mode);
		break;
	}
	case TRANS_LOAD_COPY_TRAV_TEAMER:
	{
		TrvlPeakActor::process_redirect_to_trvl_scene(shop_mode);
		break;
	}
	case TRANS_LOAD_LFR_WAR_INFO:
	{
		LRF_MONITOR->load_league_war_info_done(shop_mode);
		break;
	}
	default:
	{
		break;
	}
	}

    return trans->summit();
}

int MapMonitor::logout_map_all_player(void)
{
    return this->player_manager()->logout_map_all_player();
}

int MapMonitor::logout_logic_all_player(void)
{
    return this->player_manager()->logout_logic_all_player();
}

int MapMonitor::bind_script_player_rel(const int script_sort, const int64_t role_id, ScriptPlayerRel *rel)
{
    return this->script_player_rel_map_.bind(script_sort, role_id, rel);
}

int MapMonitor::unbind_script_player_rel(const int script_sort, const int64_t role_id)
{
    return this->script_player_rel_map_.unbind(script_sort, role_id);
}

ScriptPlayerRel *MapMonitor::find_script_player_rel(const int script_sort, const int64_t role_id)
{
    ScriptPlayerRel *rel = 0;
    if (this->script_player_rel_map_.find(script_sort, role_id, rel) == 0)
        return rel;
    return 0;
}

int MapMonitor::bind_script_team_rel(const int team_id, ScriptTeamDetail *rel)
{
    return this->script_team_rel_map_.bind(team_id, rel);
}

int MapMonitor::unbind_script_team_rel(const int team_id)
{
    return this->script_team_rel_map_.unbind(team_id);
}

ScriptTeamDetail *MapMonitor::find_script_team_rel(const int team_id)
{
    ScriptTeamDetail *script_team = 0;
    if (this->script_team_rel_map_.find(team_id, script_team) == 0)
        return script_team;
    return 0;
}

int MapMonitor::bind_script(const int script_id, BaseScript *script)
{
    return this->script_map_.bind(script_id, script);
}

int MapMonitor::unbind_script(const int script_id)
{
    return this->script_map_.unbind(script_id);
}

BaseScript *MapMonitor::find_script(const int script_id)
{
    BaseScript *script = 0;
    if (this->script_map_.find(script_id, script) == 0)
        return script;
    return 0;
}

ScriptFactory *MapMonitor::script_factory(void)
{
    return this->script_factory_;
}

int MapMonitor::load_script_progress_id_map(void)
{
    const GameConfig::ConfigMap &script_map = CONFIG_INSTANCE->script_map();

    IntVec script_vc;
    for (GameConfig::ConfigMap::const_iterator iter = script_map.begin();
            iter != script_map.end(); ++iter)
    {
        script_vc.push_back(iter->first);
    }

    if (CACHED_INSTANCE->load_global_script_progress(&(this->script_progressid_map_), script_vc) != 0)
    {
        return -1;
    }

    MSG_USER("%d %d ...", script_vc.size(), this->script_progressid_map_.size());
    return 0;
}

Int64 MapMonitor::script_progress_id(const int script_sort)
{
    Int64 progress = 0;
    if (this->script_progressid_map_.find(script_sort, progress) == 0)
        return progress;
    return -1;
}

void MapMonitor::update_script_progress_id(const int script_sort, const Int64 progress)
{
    this->script_progressid_map_.rebind(script_sort, progress);
}

int MapMonitor::back_force_close_activity(Message *msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400447*, request, -1);

//	int type = request->activity_type();
//	switch(type)
//	{
//	case 1:
//	{
//		return LEAGUE_MONITOR->close_league_fb(msg);
//	}
//	}

	return 0;
}

int MapMonitor::is_has_travel_scene(void)
{
	return this->travel_scene_;
}

int MapMonitor::fetch_gate_sid(const string& server_flag)
{
	JUDGE_RETURN(this->all_sid_.count(server_flag) > 0, -1);
	return this->all_sid_[server_flag];
}

int MapMonitor::process_fetch_travel_area(int sid, Message *msg)
{
    MSG_DYNAMIC_CAST_RETURN(Proto30402018*, request, -1);
    JUDGE_RETURN(request->server_flag().empty() == false, -1);

    this->all_sid_[request->server_flag()] = sid;

    Proto30402018 respond;
    return this->dispatch_to_gate(sid, INNER_FETCH_TRAVEL_AREA, &respond);
}

int MapMonitor::keep_alive_msg_begin()
{
	this->check_gate_sid_map_.clear();
	JUDGE_RETURN(this->is_has_travel_scene() == true, -1);

	const ServerDetail& detail = CONFIG_INSTANCE->cur_map_server();
	JUDGE_RETURN(detail.__scene_list.empty() == false, -1);

	IntVec gate_sid_vc;
	this->gate_sid_list(gate_sid_vc);

	Proto30400027 respond;
	respond.set_index(detail.__index);

	int first_scene = (*detail.__scene_list.begin());
	respond.set_scene(first_scene);

	for (IntVec::iterator iter = gate_sid_vc.begin(); iter != gate_sid_vc.end(); ++iter)
	{
		int sid = *iter;
		this->check_gate_sid_map_[sid] = true;
		this->dispatch_to_gate(sid, INNER_MAP_TRVL_KEEP_ALIVE, &respond);
	}

	MSG_USER("trvl sid start check %d %d", first_scene, gate_sid_vc.size());
	return 0;
}

int MapMonitor::keep_alive_msg_done(int sid)
{
	this->check_gate_sid_map_.erase(sid);
	return 0;
}

int MapMonitor::handle_remove_illegal_sid()
{
	for (IntMap::iterator iter = this->check_gate_sid_map_.begin();
			iter != this->check_gate_sid_map_.end(); ++iter)
	{
		Svc* svc = NULL;
		JUDGE_CONTINUE(this->inner_monitor_.find_service(iter->first, svc) == 0);

		MSG_USER("trvl sid close svc %d %d %d %d %d", svc->get_cid(), svc->get_fd(),
				svc->is_closed(), svc->get_reg_recv(), svc->get_reg_send());
		svc->set_closed(false);
		svc->handle_close();
	}

	MSG_USER("trvl sid remove %d", this->check_gate_sid_map_.size());
	this->check_gate_sid_map_.clear();

	return 0;
}

int MapMonitor::check_keep_alvie_timeout()
{
	if (this->check_gate_state_ == 0)
	{
		this->check_gate_state_ = 1;
		this->keep_alive_msg_begin();
	}
	else
	{
		this->check_gate_state_ = 0;
		this->handle_remove_illegal_sid();
	}

	return 0;
}

int MapMonitor::fetch_average_level_done(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400025*, request, -1);
	this->average_level_ = request->average_level();
	return 0;
}

int MapMonitor::festival_generate_boss(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400040*, request, -1);

	Scene* scene = NULL;
	JUDGE_RETURN(this->find_scene(0, request->scene_id(), scene) == 0, -1);

	MoverCoord born_coord;
	born_coord.unserialize(request->mutable_coord());

	Int64 ai_id = AIMANAGER->generate_monster_by_sort(request->boss_id(), born_coord, scene);
	MSG_USER("%d, %d, %ld", request->scene_id(), request->boss_id(), ai_id);

	GameAI* game_ai = AI_PACKAGE->find_object(ai_id);
	JUDGE_RETURN(game_ai != NULL, -1);

	game_ai->ai_detail().league_index_ = 1;
	return 0;
}

int MapMonitor::set_festival_activity_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400041*, request, -1);

	this->festival_info_.act_state_ = request->drop_act();
	this->festival_info_.icon_type_ = request->icon_type();
	this->festival_info_.start_tick_ = request->start_tick();
	this->festival_info_.end_tick_ = request->end_tick();

	return 0;
}

int MapMonitor::set_big_activity_info(Message* msg)
{
	MSG_DYNAMIC_CAST_RETURN(Proto30400042*, request, -1);

	this->big_act_info_.type_ = request->type();
	this->big_act_info_.start_tick_ = request->start_tick();
	this->big_act_info_.end_tick_ = request->end_tick();

	return 0;
}

int MapMonitor::festival_icon_type()
{
	JUDGE_RETURN(this->festival_info_.act_state_ == true, 0);
	JUDGE_RETURN(this->festival_info_.icon_type_ > 0, 0);

	Int64 now_tick = ::time(NULL);
	JUDGE_RETURN(now_tick >= this->festival_info_.start_tick_
			&& now_tick < this->festival_info_.end_tick_, 0);

	return this->festival_info_.icon_type_;
}

int MapMonitor::is_in_big_act_time()
{
	JUDGE_RETURN(this->big_act_info_.type_ > 0, 0);

	Int64 now_tick = ::time(NULL);
	JUDGE_RETURN(now_tick >= this->big_act_info_.start_tick_
			&& now_tick < this->big_act_info_.end_tick_, 0);

	return true;
}

int MapMonitor::fetch_double_activity(int type)
{
	return 1;
}

int MapMonitor::add_skill_cool(GameFighter* player, FighterSkill* skill)
{
	JUDGE_RETURN(player->is_player() == true, -1);

	SkillCoolInfo* cool_info = this->skill_cool_package_->find_pop_bind_object(
			player->fighter_id());
	JUDGE_RETURN(cool_info != NULL, -1);

	cool_info->skill_cool_[skill->__skill_id] = skill->__use_tick;
	return 0;
}

Time_Value MapMonitor::fetch_skill_cool(Int64 role, int skill)
{
	SkillCoolInfo* cool_info = this->skill_cool_package_->find_object(role);
	JUDGE_RETURN(cool_info != NULL, Time_Value::zero);
	return cool_info->fetch_time(skill);
}

int MapMonitor::map_test_command(Message *msg)
{
#ifdef TEST_COMMAND
	MSG_DYNAMIC_CAST_RETURN(Proto11499999*, request, -1);

    if (request->cmd_name() == "SMBattle")
    {
    	if (this->is_current_server_scene(GameEnum::SUN_MOON_BATTLE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::SUN_MOON_BATTLE_ID, request);
    	}
    	else
    	{
    		SM_BATTLE_SYSTEM->test_sm_battle(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "league_region")
    {
    	if (this->is_current_server_scene(GameEnum::LEAGUE_REGION_FIGHT_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::LEAGUE_REGION_FIGHT_ID, request);
    	}
    	else
    	{
    		LRF_MONITOR->test_league_region(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "tarena")
    {
    	if (this->is_current_server_scene(GameEnum::TRVL_ARENA_SCENE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_ARENA_SCENE_ID, request);
    	}
    	else
    	{
    		TRVL_ARENA_MONITOR->test_arena(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "tmarena")
    {
    	if (this->is_current_server_scene(GameEnum::TRVL_MARENA_PREP_SCENE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_MARENA_PREP_SCENE_ID, request);
    	}
    	else
    	{
    		TRVL_MARENA_MONITOR->test_marena(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "lwar")
    {
        if (this->is_current_server_scene(GameEnum::LWAR_SCENE_ID) == false)
        {
        	return MAP_MONITOR->dispatch_to_scene(GameEnum::LWAR_SCENE_ID, request);
        }
        else
        {
        	LEAGUE_WAR_SYSTEM->test_lwar(request->param1(), request->param2());
        }
    }
    else if (request->cmd_name() == "wboss")
    {
        if (this->is_current_server_scene(GameEnum::WORLD_BOSS_SCENE_ID_1) == false)
        {
          	return MAP_MONITOR->dispatch_to_scene(GameEnum::WORLD_BOSS_SCENE_ID_1, request);
        }
        else
        {
        	WORLD_BOSS_SYSTEM->test_open_wboss(request->param1(), request->param2(), request->param3());
        }
    }
    else if (request->cmd_name() == "trvl_wboss")
    {
    	if (this->is_current_server_scene(GameEnum::TRVL_WBOSS_SCENE_ID_1) == false)
    	{
        	return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_WBOSS_SCENE_ID_1, request);
        }
        else
        {
        	TRVL_WBOSS_MONITOR->test_trvl_wboss(request->param1(), request->param2(), request->param3());
        }
    }
    else if (request->cmd_name() == "open_space")
	{
		if (this->is_current_server_scene(GameEnum::LWAR_SCENE_ID) == false)
		{
			return MAP_MONITOR->dispatch_to_scene(GameEnum::LWAR_SCENE_ID, request);
		}
		else
		{
			LEAGUE_WAR_SYSTEM->test_open_space();
		}
	}
    else if (request->cmd_name() == "test_trvl_wboss")
	{
    	for (int scene_id = (GameEnum::TRVL_WBOSS_SCENE_ID_READY + 1);
    			scene_id < GameEnum::TRVL_WBOSS_SCENE_ID_END; ++scene_id)
    	{
    		if (this->is_current_server_scene(scene_id) == false)
    		{
    			MAP_MONITOR->dispatch_to_scene(scene_id, request);
    		}
    		else
    		{
    			TRVL_WBOSS_MONITOR->test_relive_boss(scene_id);
    		}
    	}
	}
    else if (request->cmd_name() == "trvl_peak")
    {
    	if (this->is_current_server_scene(GameEnum::TRVL_PEAK_SCENE_ID) == false)
    	{
    		return MAP_MONITOR->dispatch_to_scene(GameEnum::TRVL_PEAK_SCENE_ID, request);
    	}
    	else
    	{
    		TRVL_PEAK_MONITOR->test_trvl_peak(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "test_wboss")
    {
        for (int scene_id = (GameEnum::WORLD_BOSS_SCENE_ID_READY + 1);
        		scene_id < GameEnum::WORLD_BOSS_SCENE_ID_END; ++scene_id)
        {
        	if (this->is_current_server_scene(scene_id) == false)
        	{
        		MAP_MONITOR->dispatch_to_scene(scene_id, request);
        	}
        	else
        	{
        		WORLD_BOSS_SYSTEM->test_relive_boss(scene_id);
        	}
        }
    }
    else if (request->cmd_name() == "mattack")
	{
		if (this->is_current_server_scene(GameEnum::MATTACK_SCENE_ID) == false)
		{
			return MAP_MONITOR->dispatch_to_scene(GameEnum::MATTACK_SCENE_ID, request);
		}
		else
		{
			MONSTER_ATTACK_SYSTEM->test_mattack(request->param1(), request->param2());
		}
	}
    else if (request->cmd_name() == "test_chests")
    {
    	if (this->is_current_server_scene(GameEnum::COLLECT_CHESTS_SCENE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::COLLECT_CHESTS_SCENE_ID, request);
    	}
    	else
    	{
    		COLLECTCHESTS_INSTANCE->test_chests(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "answer")
    {
    	if (this->is_current_server_scene(GameEnum::ANSWER_ACTIVITY_SCENE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::ANSWER_ACTIVITY_SCENE_ID, request);
    	}
    	else
    	{
    		ANSWERACTIVITY_INSTANCE->test_answer(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "hotspring")
    {
    	if (this->is_current_server_scene(GameEnum::HOTSPRING_SCENE_ID) == false)
    	{
    	    return MAP_MONITOR->dispatch_to_scene(GameEnum::HOTSPRING_SCENE_ID, request);
    	}
    	else
    	{
    		HOTSPRING_INSTANCE->test_hotspring(request->param1(), request->param2());
    	}
    }
    else if (request->cmd_name() == "legend_top_rank")
    {
    	if (this->is_current_server_scene(GameEnum::LEGEND_TOP_SCENE) == false)
    	{
    		return MAP_MONITOR->dispatch_to_scene(GameEnum::LEGEND_TOP_SCENE, request);
    	}
    	else
    	{
    		SCRIPT_SYSTEM->test_rank_reset();
    	}
    }

#endif

	return 0;
}
