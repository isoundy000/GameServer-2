/*
 * MapMailPlayer.cpp
 *
 *  Created on: 2013-7-12
 *      Author: xie
 */

#include "MapMailPlayer.h"
#include "MapMonitor.h"
#include "ProtoDefine.h"
#include "MongoConnector.h"
#include "SerialRecord.h"
#include "Transaction.h"
#include "MLGameSwither.h"
#include "GameFont.h"
#include "MMOMailOffline.h"
#include "MongoDataMap.h"
#include "MapLogicPlayer.h"
#include "DBCommon.h"

MailBox::MailBox(void)
{ /*NULL*/ }

void MailBox::reset(void)
{
    for (MailMap::iterator iter = this->__mail_map.begin();
    		iter != this->__mail_map.end(); ++iter)
    {
    	iter->second->reset();
    	POOL_MONITOR->mail_info_pool()->push(iter->second);
    }
    this->__mail_map.clear();
}


MapMailPlayer::MapMailPlayer(void)
{
    this->send_mail_count_ = 0;
    this->send_mail_cool_tick_ = 0;
}

MapMailPlayer::~MapMailPlayer(void)
{ /*NULL*/ }

void MapMailPlayer::reset(void)
{
    this->map_logic_mail_box_reset();
}

MailBox& MapMailPlayer::mail_box(void)
{
	return this->mail_box_;
}

int MapMailPlayer::obtain_mail_list(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400301*, request, RETURN_OBTAIN_MAIL_LIST);

	Proto51400301 respond;
	MailBox::MailMap::iterator iter = this->mail_box_.__mail_map.begin();
	for (; iter != this->mail_box_.__mail_map.end(); ++iter)
	{
		MailInformation* mail_info = iter->second;

		int left_min = MapMailPlayer::calc_mail_left_minute(mail_info);
		JUDGE_CONTINUE(left_min > 0);

		ProtoMailInfo* proto_info = respond.add_mail_list();
		proto_info->set_mail_id(iter->first);
		proto_info->set_has_read(mail_info->has_read_);
		proto_info->set_mail_type(mail_info->mail_type_);
		proto_info->set_mail_time(mail_info->send_time_);
		proto_info->set_role_name(mail_info->sender_name_);
		proto_info->set_mail_title(mail_info->mail_title_);
		proto_info->set_has_attach(MapMailPlayer::check_has_attached(mail_info));
		proto_info->set_left_minute(left_min);
	}

	int deposit_mail_amount = this->calc_deposit_mail_num();
	respond.set_deposit_mail_amount(deposit_mail_amount);

	FINER_PROCESS_RETURN(RETURN_OBTAIN_MAIL_LIST, &respond);
}

int MapMailPlayer::obtain_mail_info(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400302*, request, RETURN_OBTAIN_MAIL_INFO);

	Int64 mail_id = request->mail_id();
	CONDITION_NOTIFY_RETURN(this->mail_box_.__mail_map.count(mail_id) > 0,
			RETURN_OBTAIN_MAIL_INFO, ERROR_MAIL_ID);

	MailInformation* mail_info = this->mail_box_.__mail_map[mail_id];
	if(mail_info->has_read_ == 0)
	{
		mail_info->has_read_ = 1;
		mail_info->read_tick_ = Time_Value::gettimeofday().sec();
		this->map_logic_player()->update_new_mail_amount_event();
	}

	Proto51400302 respond;
	respond.set_mail_id(mail_id);
	respond.set_mail_type(mail_info->mail_type_);
	respond.set_role_name(mail_info->sender_name_);
	respond.set_mail_title(mail_info->mail_title_);
	respond.set_mail_content(mail_info->mail_content_);
	respond.set_sender_vip(mail_info->sender_vip_);

	int index = 0;
	for (ItemListMap::iterator iter_goods = mail_info->goods_map_.begin();
			iter_goods != mail_info->goods_map_.end(); ++iter_goods)
	{
		PackageItem* pack_item = iter_goods->second;
		pack_item->__index = index++;
		pack_item->serialize(respond.add_goods_list());
	}

	respond.set_left_minute(MapMailPlayer::calc_mail_left_minute(mail_info));
	FINER_PROCESS_RETURN(RETURN_OBTAIN_MAIL_INFO, &respond);
}

int MapMailPlayer::pick_up_single_mail(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400304*, request, RETURN_PICK_UP_ATTACH_A);

	Int64 mail_id = request->mail_id();
	CONDITION_NOTIFY_RETURN(this->mail_box_.__mail_map.count(mail_id) > 0,
			RETURN_PICK_UP_ATTACH_A, ERROR_MAIL_ID);

	MailInformation* mail_info = this->mail_box_.__mail_map[mail_id];

	int ret = this->pick_up_single_mail(mail_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PICK_UP_ATTACH_A, ret);

	//db
	this->map_logic_player()->update_new_mail_amount_event();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAIL);

//	Proto51400304 respond;
//	respond.set_mail_id(mail_id);
	FINER_PROCESS_NOTIFY(RETURN_PICK_UP_ATTACH_A);
}

int MapMailPlayer::pick_up_all_mail(void)
{
	IntMap goods_index;
	int attach_num = 0, mail_count = 0;
	MailBox::MailMap::iterator iter_mail = this->mail_box_.__mail_map.begin();
	for (; iter_mail != this->mail_box_.__mail_map.end() &&
	             mail_count < this->player_mail_box_max_size(); ++iter_mail, ++mail_count)
	{
		MailInformation* mail_info = iter_mail->second;
		JUDGE_CONTINUE(MapMailPlayer::check_has_attached(mail_info) == true);

		mail_info->fetch_goods_index(goods_index);
		attach_num += (int)goods_index.size();
		goods_index.clear();
	}

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	CONDITION_NOTIFY_RETURN(package->left_capacity() >= attach_num,
			RETURN_PICK_UP_ATTACH_A, ERROR_PACKAGE_NO_CAPACITY);

	mail_count = 0;
    iter_mail = this->mail_box_.__mail_map.begin();
	for (; iter_mail != this->mail_box_.__mail_map.end() &&
	             mail_count < this->player_mail_box_max_size(); ++iter_mail, ++mail_count)
	{
		MailInformation* mail_info = iter_mail->second;
		JUDGE_CONTINUE(MapMailPlayer::check_has_attached(mail_info) == true);

		int ret = this->pick_up_single_mail(mail_info);
		CONDITION_NOTIFY_RETURN(ret == 0, RETURN_PICK_UP_ATTACH_A, ret);
	}

	this->map_logic_player()->update_new_mail_amount_event();
	this->cache_tick().update_cache(MapLogicPlayer::CACHE_MAIL);

	FINER_PROCESS_NOTIFY(RETURN_PICK_UP_ALL_ATTACH);
}

int MapMailPlayer::delete_mail(Message* msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto11400306*, request, RETURN_DELETE_MAIL);

	LongVec delete_set;
	switch(request->del_type())
	{
	case GameEnum::MAIL_DELETE_ALL:
	{
		for (MailBox::MailMap::iterator iter = this->mail_box_.__mail_map.begin();
				iter != this->mail_box_.__mail_map.end(); ++iter)
		{
			delete_set.push_back(iter->first);
		}
		break;
	}
	case GameEnum::MAIL_DELETE_READ:
	{
		for (MailBox::MailMap::iterator iter = this->mail_box_.__mail_map.begin();
				iter != this->mail_box_.__mail_map.end(); ++iter)
		{
			JUDGE_CONTINUE(iter->second != NULL && iter->second->has_read_ == 1);
			delete_set.push_back(iter->first);
		}
		break;
	}
	case GameEnum::MAIL_DELETE_SINGLE:
	{
		for (int i = 0; i < request->del_list_size(); ++i)
		{
			Int64 mail_id = request->del_list(i);
			delete_set.push_back(mail_id);
		}
		break;
	}
	case GameEnum::MAIL_DELETE_NO_ATTACHED:
	{
		for (MailBox::MailMap::iterator iter = this->mail_box_.__mail_map.begin();
				iter != this->mail_box_.__mail_map.end(); ++iter)
		{
			JUDGE_CONTINUE(MapMailPlayer::check_has_attached(iter->second) == false);
			delete_set.push_back(iter->first);
		}
		break;
	}
	default:
	{
		return -1;
	}
	}

	for (LongVec::iterator iter = delete_set.begin(); iter != delete_set.end(); ++iter)
	{
		Int64 mail_id = *iter;
		JUDGE_CONTINUE(this->mail_box_.__mail_map.count(mail_id) > 0);

		MailInformation* mail_info = this->mail_box_.__mail_map[mail_id];
		this->mail_box_.__mail_map.erase(mail_id);
		JUDGE_CONTINUE(mail_info != NULL);

		//mail serial record to backstage
	    this->make_up_mail_serial_detail(MAIL_DELETE, mail_info);
		POOL_MONITOR->mail_info_pool()->push(mail_info);
	}

	this->map_logic_player()->update_new_mail_amount_event();
	FINER_PROCESS_NOTIFY(RETURN_DELETE_MAIL);
}

int MapMailPlayer::send_mail(int sid, Message* msg)
{
//	MSG_DYNAMIC_CAST_NOTIFY(Proto11400307*, request, RETURN_SEND_MAIL);
//
//	// 后台开关检查
//	CONDITION_NOTIFY_RETURN(ML_SWITCHER_SYS->map_check_switcher(GameSwitcherName::mail),
//			RETURN_SEND_MAIL, ERROR_MODEL_CLOSED);
//
//	CONDITION_NOTIFY_RETURN(GameCommon::is_travel_scene(this->scene_id()) == false,
//			RETURN_SEND_MAIL, ERROR_OP_IN_ACTIVITY_TICK);
//
//	int level_limit = CONFIG_INSTANCE->tiny("send_mail_limit").asInt();
//	CONDITION_NOTIFY_RETURN(this->role_level() >= level_limit,
//				RETURN_SEND_MAIL, ERROR_SEND_MAIL_LEVEL_LIMIT);
//
//	CONDITION_NOTIFY_RETURN(this->is_leagal_interval_for_send_mail(),
//			RETURN_SEND_MAIL, ERROR_MAIL_COOL_TIME);
//
//	const string& receiver_name = request->receiver_name();
//	CONDITION_NOTIFY_RETURN(receiver_name.length() < MAX_NAME_LENGTH,
//			RETURN_SEND_MAIL, ERROR_MAIL_FORMAT);
//	CONDITION_NOTIFY_RETURN(receiver_name != this->role_name(),
//			RETURN_SEND_MAIL, ERROR_SEND_MAIL_TO_SELF);
//
//	const string& mail_title = request->mail_title();
//	const string& mail_content = request->mail_content();
//	CONDITION_NOTIFY_RETURN(mail_title.length() < GameEnum::MAX_MAIL_TITLE_LENGTH
//			&& mail_content.length() < GameEnum::MAX_MAIL_CONTENT_LENGTH,
//			RETURN_SEND_MAIL, ERROR_MAIL_FORMAT);
//
//	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
//	CONDITION_NOTIFY_RETURN(package != NULL, RETURN_SEND_MAIL, ERROR_CLIENT_OPERATE);
//
//	MailInformation *mail_info = POOL_MONITOR->mail_info_pool()->pop();
//
//	mail_info->receiver_name_ = receiver_name;
//	mail_info->mail_title_ = request->mail_title();
//	mail_info->mail_content_ = request->mail_content();
//	Money mail_money(request->mail_gold(), 0, request->mail_copper());
//	mail_info->money_ = mail_money;
//
//	int attach_size = std::min<int>(GameEnum::MAX_MAIL_GOODS_COUNT,
//			request->attach_list_size());
//	for (int i = 0; i < attach_size; ++i)
//	{
//		const MailAttach& mail_attach = request->attach_list(i);
//
//		PackageItem* pack_item = package->find_by_index(mail_attach.pack_index());
//		if(pack_item == NULL)
//		{
//			POOL_MONITOR->mail_info_pool()->push(mail_info);
//			return this->respond_to_client_error(RETURN_SEND_MAIL,
//				ERROR_CLIENT_OPERATE);
//		}
//
//		if(pack_item->__bind == GameEnum::ITEM_BIND)
//		{
//			POOL_MONITOR->mail_info_pool()->push(mail_info);
//			return this->respond_to_client_error(RETURN_SEND_MAIL, ERROR_PACKAGE_GOODS_BIND);
//		}
//
//		int pack_amount = mail_attach.pack_amount();
//		if(pack_item->__amount < pack_amount || pack_amount == 0)
//		{
//			POOL_MONITOR->mail_info_pool()->push(mail_info);
//			return this->respond_to_client_error(RETURN_SEND_MAIL, ERROR_PACKAGE_ITEM_AMOUNT);
//		}
//
//		mail_info->attach_map_[mail_attach.mail_index()] = pack_amount;
//		mail_info->goods_map_[mail_attach.mail_index()] = pack_item;
//	}
//
//	TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_LOAD_PLAYER_TO_SEND_MAIL,
//			DB_MAIL_INFO, mail_info, POOL_MONITOR->mail_info_pool(), MAP_MONITOR->logic_unit());
//
//    MSG_USER(" 11 send mail detail 1: sender_id[%ld]sender_name[%s]"
//    		"-recever_id[%ld]recever_name[%s]-attach_size[%d]-tick[%ld]"
//    		"-mail_index[%ld]-mail_content[%s]-address[%x]",
//    		this->role_id(), this->role_name().c_str(),
//    		mail_info->receiver_id_, mail_info->receiver_name_.c_str(),
//    		mail_info->goods_map_.size(), mail_info->send_time_,
//    		mail_info->mail_index_, mail_info->mail_content_.c_str(), mail_info);

	return 0;
}

int MapMailPlayer::send_test_mail()
{
    MailInformation *mail_info = GameCommon::create_sys_mail(20002);
    mail_info->add_goods(217020602, 1);
    mail_info->add_goods(217020603, 1);
    GameCommon::request_save_mail(this->role_id(), mail_info);
    return 0;
}

int MapMailPlayer::after_send_mail(Transaction *transaction)
{
	return 0;
}

int MapMailPlayer::after_load_mail_offline(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return transaction->detail().__error;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_MAIL_BOX);
    JUDGE_RETURN(trans_data != NULL, -1);

    bool new_mail = false;
    MailBox* mail_box = trans_data->__data.__mail_box;
    trans_data->reset();

    LongVec remove_id_list;
    for (MailBox::MailMap::iterator iter = mail_box->__mail_map.begin();
    		iter != mail_box->__mail_map.end(); ++iter)
    {
    	Int64 mail_id = iter->first;
    	remove_id_list.push_back(mail_id);
    	if (this->mail_box_.__mail_map.count(mail_id) > 0)
    	{
    		POOL_MONITOR->mail_info_pool()->push(iter->second);
    		continue;
    	}

    	MailInformation* mail_info = iter->second;
    	JUDGE_CONTINUE(mail_info != NULL);

    	new_mail = true;
    	this->mail_box_.__mail_map[mail_id] = mail_info;

        MSG_USER("receive mail detail : sender_id[%ld]sender_name[%s]"
        		"-recever_id[%ld]recever_name[%s]-attach_size[%d]"
        		"-tick[%ld]-mail_index[%ld-mail_content[%s]-address[%x]",
        		mail_info->sender_id_, mail_info->sender_name_.c_str(),
        		mail_info->receiver_id_, mail_info->receiver_name_.c_str(),
        		mail_info->goods_map_.size(), mail_info->send_time_,
        		mail_info->mail_index_, mail_info->mail_content_.c_str(), mail_info);

    	//mail serial record to backstage
        this->make_up_mail_serial_detail(MAIL_RECV, mail_info);
    }
    mail_box->__mail_map.clear();
    this->monitor()->mail_box_pool()->push(mail_box);

    if (new_mail == true)
    {
		this->map_logic_player()->update_new_mail_amount_event();
    }

    {
    	MongoDataMap *data_map = POOL_MONITOR->mongo_data_map_pool()->pop();
    	MMOMailOffline::remove_loaded_mail_offline(this->role_id(), remove_id_list, data_map);
    	if (TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(), TRANS_DELETE_LOAD_MAIL_OFFLINE, data_map) != 0)
    	{
    		POOL_MONITOR->mongo_data_map_pool()->push(data_map);
    	}
    }

    transaction->summit();
    return 0;
}

int MapMailPlayer::after_load_player_to_send_mail(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return transaction->detail().__error;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_MAIL_INFO);
    if (trans_data == 0)
        return -1;

    MailInformation *mail_info = trans_data->__data.__mail_info;
    trans_data->reset();
    transaction->summit();

    MSG_USER("ADDRESS:%x--mail:%x",  trans_data->__data.__mail_info, mail_info);

    Int64 receiver_id = mail_info->receiver_id_;
    if(receiver_id <= 0)
    {
		POOL_MONITOR->mail_info_pool()->push(mail_info);
    	return this->respond_to_client_error(RETURN_SEND_MAIL, ERROR_ROLE_NOT_EXISTS);
    }

	Money mail_money(mail_info->money_.__gold, mail_info->money_.__bind_gold,
			mail_info->money_.__copper, CONFIG_INSTANCE->tiny("mail_copper").asInt());
	GameCommon::adjust_money(mail_money, this->own_money());
	int ret = this->pack_money_sub(mail_money, SUB_MONEY_MAIL);
    if(ret != 0)
    {
		POOL_MONITOR->mail_info_pool()->push(mail_info);
    	return this->respond_to_client_error(RETURN_SEND_MAIL, ret);
    }

    ItemListMap new_goods_map;
    ItemListMap::iterator it = mail_info->goods_map_.begin();
    for(; it != mail_info->goods_map_.end(); ++it)
    {
    	JUDGE_CONTINUE(it->second != NULL);
    	PackageItem* mail_item = it->second;
    	int mail_index = it->first;
    	int mail_amount = mail_info->attach_map_.count(mail_index) > 0 ? mail_info->attach_map_[mail_index] : 1;

    	PackageItem* new_mail_item = GamePackage::pop_item(mail_item->__id);
    	JUDGE_CONTINUE(new_mail_item != NULL);

    	*new_mail_item = *mail_item;
    	new_mail_item->__index = mail_index;
    	new_mail_item->__amount = mail_amount;
    	new_goods_map[mail_index] = new_mail_item;

    	this->pack_remove(ITEM_MAIL_REMOVE, mail_item, mail_amount, GameEnum::INDEX_PACKAGE, receiver_id);
    }
    mail_info->goods_map_.clear();
    mail_info->goods_map_.insert(new_goods_map.begin(), new_goods_map.end());

    mail_info->mail_type_ = GameEnum::MAIL_PRIVATE;
    mail_info->send_time_ = ::time(NULL);
    mail_info->sender_name_ = this->role_name();
    mail_info->sender_id_ = this->role_id();
    mail_info->sender_vip_ = this->vip_type();

	//mail serial record to backstage
    this->make_up_mail_serial_detail(MAIL_SEND, mail_info);

    MSG_USER(" 22 send mail detail 2 : sender_id[%ld]sender_name[%s]sender_vip[%d]"
    		"-recever_id[%ld]recever_name[%s]-attach_size[%d]"
    		"-tick[%ld]-mail_index[%ld]-mail_content[%s]-address[%x]",
    		mail_info->sender_id_, mail_info->sender_name_.c_str(),mail_info->sender_vip_,
    		mail_info->receiver_id_, mail_info->receiver_name_.c_str(),
    		mail_info->goods_map_.size(), mail_info->send_time_,
    		mail_info->mail_index_, mail_info->mail_content_.c_str(), mail_info);
    GameCommon::request_save_mail(mail_info);

    ++this->send_mail_count_;
	FINER_PROCESS_NOTIFY(RETURN_SEND_MAIL);
}


int MapMailPlayer::mail_time_up(const Time_Value &nowtime)
{
	this->delete_expired_time_mail();
	this->check_send_mail_interval();

	MailBox* mail_box = this->monitor()->mail_box_pool()->pop();
	if (0 != TRANSACTION_MONITOR->request_mongo_transaction(this->role_id(),
			TRANS_LOAD_MAIL_OFFLINE, DB_MAIL_BOX, mail_box,
			this->monitor()->mail_box_pool(), this->monitor()->logic_unit()))
	{
		this->monitor()->mail_box_pool()->push(mail_box);
	}

	return 0;
}

int MapMailPlayer::map_logic_mail_box_reset()
{
	this->mail_box_.reset();
	this->send_mail_count_ = 0;
	this->send_mail_cool_tick_ = 0;
	return  0;
}

int MapMailPlayer::pick_up_single_mail(MailInformation* mail_info)
{
	JUDGE_RETURN(mail_info != NULL, ERROR_CLIENT_OPERATE);

	IntMap goods_index;
	mail_info->fetch_goods_index(goods_index);

	GamePackage* package = this->find_package(GameEnum::INDEX_PACKAGE);
	JUDGE_RETURN(package->left_capacity() >= (int)goods_index.size(), ERROR_PACKAGE_NO_CAPACITY);

	int sub_serial = CONFIG_INSTANCE->font_serial(mail_info->mail_format_);
	for (IntMap::iterator iter = goods_index.begin(); iter != goods_index.end(); ++iter)
	{
		PackageItem* pack_item = mail_info->goods_map_[iter->first];
		JUDGE_CONTINUE(pack_item != NULL && pack_item->validate_item() == true);

		ItemObj item_obj;
		item_obj.unserialize(pack_item);

		this->pack_insert(package, SerialObj(ADD_FROM_MAIL_ITEM, sub_serial),
				item_obj, mail_info->sender_id_);
	}

	mail_info->recycle_goods();

	//称号
	MapLogicPlayer* player = this->map_logic_player();
	player->insert_label(mail_info->label_id_);
	mail_info->label_id_ = 0;

	mail_info->has_read_ = 1;
	mail_info->read_tick_ = ::time(NULL);
	return this->make_up_mail_serial_detail(MAIL_PICKUP, mail_info);
}

int MapMailPlayer::delete_expired_time_mail(void)
{
	LongVec delete_set;
	MailBox::MailMap::iterator it = this->mail_box_.__mail_map.begin();
	for(; it != this->mail_box_.__mail_map.end(); ++it)
	{
		JUDGE_CONTINUE(this->calc_mail_left_minute(it->second) <= 0)
		delete_set.push_back(it->first);
	}

	JUDGE_RETURN(delete_set.empty() == false, 0);

	for(uint i = 0; i < delete_set.size(); ++i)
	{
		Int64 mail_id = delete_set[i];
		MailInformation *mail_info = this->mail_box_.__mail_map[mail_id];
		this->mail_box_.__mail_map.erase(mail_id);
		POOL_MONITOR->mail_info_pool()->push(mail_info);
	}

	return 0;
}

bool MapMailPlayer::check_has_attached(MailInformation *mail_info)
{
	JUDGE_RETURN(mail_info != NULL, false);
	return mail_info->goods_map_.empty() == false
			|| mail_info->resource_map_.empty() == false;
}

int MapMailPlayer::calc_mail_left_minute(MailInformation *mail_info)
{
	JUDGE_RETURN(mail_info != NULL, 0);

	const Json::Value &cfg = CONFIG_INSTANCE->tiny("mail_disappear");
	int index = mail_info->has_read_;
	int limit_minute = cfg[index].asInt() * 60;

	int expire_start = mail_info->has_read_ ? mail_info->read_tick_ : mail_info->send_time_;
	if(check_has_attached(mail_info))
	{
		expire_start = mail_info->send_time_;
		limit_minute = cfg[0u].asInt() * 60;
	}

	int exist_minute = (Time_Value::gettimeofday().sec() - expire_start) / 60;

	int left_minute = limit_minute - exist_minute > 0 ? limit_minute - exist_minute : 0;
	return left_minute;
}


int MapMailPlayer::sync_transfer_mail_box_info(int scene_id)
{
	Proto31400124 request;
	request.set_send_mail_count(this->send_mail_count_);
	request.set_send_mail_cool_tick(this->send_mail_cool_tick_);

	MailBox::MailMap::iterator it = this->mail_box_.__mail_map.begin();
	for(; it != this->mail_box_.__mail_map.end(); ++it)
	{
		MailInformation *mail_info = it->second;
		JUDGE_CONTINUE(mail_info != NULL);

		ProtoMailInfo* proto_mail = request.add_mail_list();
		mail_info->serilize(proto_mail);
	}
	return this->send_to_other_logic_thread(scene_id, request);
}

int MapMailPlayer::read_transfer_mail_box_info(Message *msg)
{
	MSG_DYNAMIC_CAST_NOTIFY(Proto31400124*, request, -1);

	this->send_mail_cool_tick_ = request->send_mail_cool_tick();
	this->send_mail_count_     = request->send_mail_count();

	for(int i = 0; i < request->mail_list_size(); ++i)
	{
		MailInformation* mail_info = POOL_MONITOR->mail_info_pool()->pop();
		JUDGE_CONTINUE(NULL != mail_info);

		const ProtoMailInfo &proto_mail = request->mail_list(i);
		mail_info->unserilize(&proto_mail);

		this->mail_box_.__mail_map[mail_info->mail_index_] = mail_info;
	}

	return 0;
}

int MapMailPlayer::fetch_new_mail_amount(void)
{
	int new_mail_amount = 0;
	MailBox::MailMap &mail_map = this->mail_box_.__mail_map;
	for (MailBox::MailMap::iterator iter = mail_map.begin();
			iter != mail_map.end(); ++iter)
	{
		MailInformation *mail_info = iter->second;
		JUDGE_CONTINUE(mail_info != NULL);

		if (mail_info->has_read_ == false)
		{
			++new_mail_amount;
			continue;
		}

		if (MapMailPlayer::check_has_attached(mail_info) == true)
		{
			++new_mail_amount;
			continue;
		}
	}

	return new_mail_amount;
}

bool MapMailPlayer::is_leagal_interval_for_send_mail()
{
	static int mail_send_limit_amount = CONFIG_INSTANCE->tiny("mail_send_limit_amount").asInt();
	return this->send_mail_count_ < mail_send_limit_amount;
}

int MapMailPlayer::check_send_mail_interval()
{
	if(Time_Value::gettimeofday().sec() > this->send_mail_cool_tick_)
	{
		this->send_mail_count_ = 0;
		this->modify_send_mail_interval();
	}
	return 0;
}

int MapMailPlayer::modify_send_mail_interval()
{
	int mail_interval_hour = CONFIG_INSTANCE->tiny("mail_interval_hour").asInt();
	if(mail_interval_hour == 0)
		mail_interval_hour = GameEnum::DEFAULT_MAIL_INTERVAL_HOUR;

	time_t now = Time_Value::gettimeofday().sec();
	struct tm now_t;
	::localtime_r(&now, &now_t);
	int cur_hour = now_t.tm_hour;

	int interval = mail_interval_hour - cur_hour % mail_interval_hour;
	int refresh_hour = ( cur_hour + interval ) % 24;

	this->send_mail_cool_tick_ = ::current_day(refresh_hour, 0).sec();

	return 0;
}

int MapMailPlayer::player_mail_box_max_size()
{
	return GameEnum::DEFAULT_MAIL_BOX_SIZE;
}

int MapMailPlayer::calc_deposit_mail_num()
{
	return this->mail_box_.__mail_map.size();
}

int MapMailPlayer::make_up_mail_serial_detail(int opra_type, MailInformation* mail_info)
{
	JUDGE_RETURN(mail_info != NULL, -1);

    MailDetailSerialObj mail_obj;
    DBCommon::mail_info2mail_serial_obj(mail_info, mail_obj);

    return SERIAL_RECORD->record_mail_detail(this, this->agent_code(),
    		this->market_code(), opra_type, mail_obj);
}

int MapMailPlayer::handle_mail_no_reciever(Transaction *transaction)
{
	JUDGE_RETURN(transaction != NULL, -1);
    if (transaction->detail().__error != 0)
    {
        transaction->rollback();
        return transaction->detail().__error;
    }

    TransactionData *trans_data = transaction->fetch_data(DB_MAIL_BOX);
    JUDGE_RETURN(trans_data != NULL, -1);

    MailBox* mail_box = trans_data->__data.__mail_box;
    trans_data->reset();

    for (MailBox::MailMap::iterator iter = mail_box->__mail_map.begin();
    		iter != mail_box->__mail_map.end(); ++iter)
    {
    	MailInformation* mail_info = iter->second;
    	JUDGE_CONTINUE(mail_info != NULL);

        MSG_USER("mail no reciever detail : sender_id[%ld]sender_name[%s]"
        		"-recever_id[%ld]recever_name[%s]-attach_size[%d]-tick[%ld]"
        		"-mail_index[%ld-mail_content[%s]-address[%x]",
        		mail_info->sender_id_, mail_info->sender_name_.c_str(),
        		mail_info->receiver_id_, mail_info->receiver_name_.c_str(),
        		mail_info->goods_map_.size(), mail_info->send_time_,
        		mail_info->mail_index_, mail_info->mail_content_.c_str(), mail_info);
    }

    mail_box->__mail_map.clear();
    MAP_MONITOR->mail_box_pool()->push(mail_box);

    transaction->summit();
    return 0;
}






