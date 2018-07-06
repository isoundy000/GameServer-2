/*
 * LogicSocialer.h
 *
 *  Created on: 2013-10-31
 *      Author: louis
 */

#ifndef LOGICSOCIALER_H_
#define LOGICSOCIALER_H_

#include "BaseLogicPlayer.h"

class ProtoFriendInfo;
class LogicSocialer : virtual public BaseLogicPlayer
{
public:
	LogicSocialer();
	virtual ~LogicSocialer();

	int fetch_friend_list(Message *msg);
	int append_to_friend_list(Message *msg);
	int send_friend_apply(Message *msg);	//申请好友
	int accept_friend_apply(Message *msg);	//接受好友申请
	int remove_apply_info(Message *msg);	//删除好友申请
	int request_fetch_apply_list();			//打开好友申请列表
	int send_friend_to_black(Message *msg); //加入黑名单
	int remove_black_friend(Message *msg); 	//从黑名单变为好友
	int remove_from_friend_list(Message *msg);
	int remove_player_friend_list(Int64 role_id, int src_type);
	int search_friend_by_name(Message *msg);
	int recommend_friend(Message *msg);
	int fetch_friend_info_by_role_id(Message *msg);
	int fetch_single_player_all_by_role_id(Message *msg);
	int fetch_nearby_player(Message* msg);
    int translate_to_enemy_position(Message* msg);
    int after_fetch_to_enemy_position(Message* msg);

	virtual int after_fetch_friend_list(Transaction* trans);
	virtual int after_accept_friend_apply(Transaction* trans);
	virtual int after_fetch_friend_info_by_role_id(Transaction* trans);
	virtual int after_fetch_single_player_all_info(Transaction* trans);

	virtual int record_be_added_as_stranger_friend(Message *msg);

	LogicSocialerDetail &socialer_detail(void);

	int request_load_other_master(Int64 role_id, const int query_type = 0);
	int after_fetch_other_master(Transaction* trans);

	int friend_sign_in();	//登录后检测好友更新
	int add_friend_to_apply_list(Int64 role_id);
	int remove_apply_player(Int64 role_id);

	int is_friend(Int64 role_id);	//检测是否好友

protected:
	void socialer_handle_player_levelup();

    LogicSocialerDetail socialer_detail_;

private:

    int modify_friend_type(const int64_t role_id);
    int fetch_friend_info(LogicPlayer *player, FriendInfo& info);
    int load_friend_info_offine(const int recogn, const int friend_type = FRIEND_TYPE_CLOSE);

    int insert_to_list(const int64_t role_id, const int des_type);
    int delete_from_list(const int64_t role_id, const int des_type);
    int handle_be_added_as_friend(const int64_t target_id);
    int active_notify_be_added_to_friend(LogicPlayer* player);
    int record_be_added_as_stranger_friend(LogicPlayer* player, Int64 maker_id);
    int stranger_list_limit_size(void);
    int enemy_list_limit_size(void);
    int recommend_friend_ex(int dir, int cur_cnt, int need_num, LongSet& result);
    int recommend_friend_calc_limit(void);

    static int socialer_system_open_level(void);

    static bool cmp_close_friend(const FriendInfo& lhs, const FriendInfo& rhs);
    static bool cmp_stranger_friend(const FriendInfo& lhs, const FriendInfo& rhs);
    static bool cmp_black_friend(const FriendInfo& lhs, const FriendInfo& rhs);
    static bool cmp_enemy(const FriendInfo& lhs, const FriendInfo& rhs);

	int serialize(ProtoFriendInfo *msg_proto, FriendInfo& info);
	int unserialize(ProtoFriendInfo *msg_proto, FriendInfo& info);

	int socialer_detail_info(void);

	int request_load_player_detail(Int64 role_id);
	int bson2player_info(BSONObj& res, Proto50100156& respond);
};

#endif /* LOGICSOCIALER_H_ */
