/*
 * ChannelAgency.h
 *
 *  Created on: 2013-6-27
 *      Author: root
 */

#ifndef CHANNELAGENCY_H_
#define CHANNELAGENCY_H_

#include "ChatStruct.h"
#include "ObjectPoolEx.h"
#include "LeagueChannel.h"
#include "NearbyChannel.h"
#include "TeamChannel.h"
#include "PrivateChat.h"
#include "HashMap.h"
#include "Thread_Mutex.h"
#include "SystemChannel.h"

class ChatPlayer;
class ChatMonitor;
class WorldChannel;
class LoudSpeaker;

class Proto10200003;
class Proto10200002;
class Proto30200117;

class ChannelAgency
{
public:
	typedef ObjectPoolEx<LeagueChannel> LeagueChannelPool;
	typedef ObjectPoolEx<NearbyChannel> NearbyChannelPool;
	typedef ObjectPoolEx<TeamChannel> TeamChannelPool;
	typedef ObjectPoolEx<PrivateChat> PrivateChatPool;
	typedef ObjectPoolEx<ChatRecord> ChatRecordPool;

	typedef HashMap<int64_t,LeagueChannel*,Null_Mutex> LeagueChannelMap;
	typedef HashMap<int64_t,NearbyChannel*,Null_Mutex> NearbyChannelMap;
	typedef HashMap<int64_t,TeamChannel*,Null_Mutex> TeamChannelMap;
	typedef HashMap<int64_t,PrivateChat*,Null_Mutex> PrivateChatMap;

	struct RoutineTimer:public GameTimer
	{
		virtual int type(void)
		{
			return GTT_CHAT_CHANNEL;
		}
		virtual int handle_timeout(const Time_Value &tv)
		{
			return this->instance->routine_timeout();
		}
		ChannelAgency* instance;
	};
	ChannelAgency(ChatMonitor *monitor);
	virtual ~ChannelAgency();

	int init(void);
	int start(void);
	int stop(void);

	BaseUnit *logic_unit(void);

	int sign_in(ChatPlayer *player);
	int sign_out(ChatPlayer *player);

	int register_to_all_channel(ChatPlayer *player);
	int register_to_world_channel(ChatPlayer *player);
	int register_to_system_channel(ChatPlayer *player);
	int register_to_nearby_channel(ChatPlayer *player);
	int register_to_league_channel(ChatPlayer *player);
	int register_to_team_channel(ChatPlayer *player);
	int register_to_loudspeaker(ChatPlayer *player);
    int register_to_travel_channel(ChatPlayer *player);

	int unregister_from_all_channel(ChatPlayer *player);
	int unregister_from_world_channel(ChatPlayer *player);
	int unregister_from_system_channel(ChatPlayer *player);
	int unregister_from_nearby_channel(ChatPlayer *player);
	int unregister_from_league_channel(ChatPlayer *player);
	int unregister_from_team_channel(ChatPlayer *player);
	int unregister_from_loudspeaker(ChatPlayer *player);
    int unregister_from_travel_channel(ChatPlayer *player);

	WorldChannel* world_channel(void);
	SystemChannel* system_channel(void);
	LoudSpeaker* loud_speaker(void);
	NearbyChannel* find_nearby_channel(int scene_id);
	LeagueChannel* find_league_channel(Int64 league_id);
	TeamChannel* find_team_channel(int team_id);
	PrivateChat* find_private_channel(int64_t role_id);
	WorldChannel* find_travel_channel(int area_id);

	int create_nearby_channel(int scene_id);
	LeagueChannel* create_league_channel(Int64 league_id);
	int create_team_channel(int team_id);
	int create_private_channel(int64_t role_id,bool is_offline=false);

	int del_league_channel(ChatPlayer *player);
	int del_team_channel(ChatPlayer *player);
	int del_private_channel(int64_t role_id);

    int fetch_client_channel(const int channel_type);
	int chat_in_normal_channel(int type, ChatPlayer *player, Proto10200002 *proto, bool trick=false);
	int chat_private(ChatPlayer *player,Proto10200003 *proto, bool trick=false);
	int chat_loudspeaker(ChatPlayer *player, const string& content, bool trick=false);
	int channel_notify(int type,ChatRecord *record,bool offline = false);
	int display_trick_chat_message_to_sender(int recogn, ChatRecord *record);
	int display_private_chat_message_to_sender(ChatRecord *record);

	int flaunt_in_normal_chanel(Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto);
	int flaunt_private(Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto);
	int flaunt_loudspeaker(Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto);

	Block_Buffer *pop_block(void);
	int push_block(Block_Buffer *buff, int cid = 0);

	ChatRecord *pop_record(void);
	int push_record(ChatRecord *record);

	void serial_flaunt(Int64 flaunt_id, ChatPlayer *player,
			ChatRecord *record, Proto30200117* proto);
	void serial_record(int channel,ChatPlayer *player,
			ChatRecord *record, const string& content);
	void brocast_serial_record(ChatRecord *record, const ProtoBrocastNewInfo* brocast_info);
	void backstage_brocast_serial_record(ChatRecord *record,
			const std::string& content, int type);

	LeagueChannelPool &league_channel_pool(void);
	ChatRecordPool &chat_record_pool(void);

	int load_league_history(Int64 league_id);
	int load_private_history(int64_t roleid);
	int load_loudspeaker(void);
	int save_loudspeaker(void);

	int64_t generate_voice_id(void);
	int64_t max_voice_id(void);

	BaseChannel* fetch_channel_by_type(ChatPlayer *player, int type = CHANNEL_WORLD);
	BaseChannel* fetch_channel_by_groud_id(Int64 group_id, int type);

	int notify_league_offline_message(ChatPlayer *player);

	int trick_player_chat_in_normal_channel(int type, ChatPlayer *player, Proto10200002 *proto);
	int trick_player_chat_in_private_channel(ChatPlayer *player, Proto10200003 *proto);
	int trick_flaunt(int channel, Int64 flaunt_id, ChatPlayer *player, Proto30200117 *proto);

private:
	int routine_timeout(void);


	ChatMonitor *monitor_;

	WorldChannel *world_channel_;
	SystemChannel *system_channel_;
	WorldChannel *trvl_channel_;
	LoudSpeaker  *loud_speaker_;
	LeagueChannelPool league_channel_pool_;
	NearbyChannelPool nearby_channel_pool_;
	TeamChannelPool team_channel_pool_;
	PrivateChatPool private_channel_pool_;
	ChatRecordPool chat_record_pool_;

	LeagueChannelMap league_channel_map_;
	NearbyChannelMap nearby_channel_map_;
	TeamChannelMap team_channel_map_;
	PrivateChatMap private_channel_map_;

	Thread_Mutex mutex_;
	int64_t global_voice_id_;
	int latest_save_time_;
};

#endif /* CHANNELAGENCY_H_ */
