/*
 * MMOMailOffline.cpp
 *
 *  Created on: 2013年7月15日
 *      Author: xie
 */

#include "GameField.h"
#include "MMOMailOffline.h"
#include "MongoConnector.h"
#include "MongoException.h"
#include "MapLogicPlayer.h"
#include "MongoDataMap.h"
#include "PoolMonitor.h"

#include <mongo/client/dbclient.h>

MMOMailOffline::MMOMailOffline(void)
{ }

MMOMailOffline::~MMOMailOffline(void)
{ }

int MMOMailOffline::load_mail_offline(Int64 role_id, MailBox* mail_box)
{
	mail_box->reset();

    BSONObj query = BSON(MailOffline::ROLE_ID << role_id << MailOffline::FLAG << 0);
    auto_ptr<DBClientCursor> cursor = this->conection().query(MailOffline::COLLECTION,
    		query);

    int flag = false;
    while (cursor->more())
    {
    	flag = true;
        BSONObj info_obj = cursor->next();

        MailInformation* mail_info = POOL_MONITOR->mail_info_pool()->pop();
        JUDGE_CONTINUE(mail_info != NULL);

        mail_info->receiver_id_ = role_id;
        mail_info->mail_index_ = info_obj[MailOffline::MAIL_ID].numberLong();
        mail_info->mail_type_ = info_obj[MailOffline::TYPE].numberInt();
        mail_info->mail_format_ = info_obj[MailOffline::FORMAT].numberInt();
        mail_info->send_time_ = info_obj[MailOffline::TIME].numberLong();
        mail_info->sender_name_ = info_obj[MailOffline::SENDER_NAME].str();
        mail_info->mail_title_ = info_obj[MailOffline::TITLE].str();
        mail_info->mail_content_ = info_obj[MailOffline::CONTENT].str();
        mail_info->sender_id_ = info_obj[MailOffline::SENDER_ID].numberLong();
        mail_info->label_id_ = info_obj[MailOffline::LABEL].numberInt();
        mail_info->sender_vip_ = info_obj[MailOffline::SENDER_VIP].numberInt();

        if (info_obj.hasField(Package::MONEY.c_str()))
        {
        	BSONObj money_obj = info_obj.getObjectField(Package::MONEY.c_str());
        	GameCommon::bson_to_money(mail_info->money_, money_obj);
        }

        BSONObjIterator iter_item(info_obj.getObjectField(MailOffline::GOODS.c_str()));
        while (iter_item.more())
        {
            BSONObj item_obj = iter_item.next().embeddedObject();

            int id = item_obj[Package::PackItem::ID].numberInt();
            PackageItem* pack_item = GamePackage::pop_item(id);
            JUDGE_CONTINUE(pack_item != NULL);

            GameCommon::bson_to_item(pack_item, item_obj);
            mail_info->goods_map_[pack_item->__index] = pack_item;
        }

        mail_box->__mail_map[mail_info->mail_index_] = mail_info;
        MSG_USER("mail recv: %ld %s sender(%ld %s) receive(%ld %s)", mail_info->mail_index_,
        		mail_info->mail_title_.c_str(), mail_info->sender_id_, mail_info->sender_name_.c_str(),
        		mail_info->receiver_id_, mail_info->receiver_name_.c_str());
    }

    JUDGE_RETURN(flag == true, -1);

    Time_Value nowtime = Time_Value::gettimeofday();
    int del_tick = nowtime.sec() - Time_Value::DAY * 15;
    this->conection().remove(MailOffline::COLLECTION, QUERY(MailOffline::TIME << BSON("$lt" << del_tick)), false);

    return 0;
}

int MMOMailOffline::save_mail_offline(MailInformation* mail_info)
{
	if (mail_info->receiver_id_ == 0)
	{
		mail_info->receiver_id_ = this->conector_->search_player_name(
				mail_info->receiver_name_);
		JUDGE_RETURN(mail_info->receiver_id_ > 0, ERROR_ROLE_NOT_EXISTS);
	}

	BSONVec vc_goods;
	for (ItemListMap::iterator iter_goods = mail_info->goods_map_.begin();
			iter_goods != mail_info->goods_map_.end(); ++iter_goods)
	{
		PackageItem *item = iter_goods->second;
		vc_goods.push_back(GameCommon::item_to_bson(item));
	}
	return 0;
}

int MMOMailOffline::remove_loaded_mail_offline(const Int64 role_id, LongVec &id_list, MongoDataMap *data_map)
{
	data_map->push_remove(MailOffline::COLLECTION,
			BSON(MailOffline::ROLE_ID << role_id << MailOffline::MAIL_ID << BSON("$in" << id_list)));
	return 0;
}

void MMOMailOffline::ensure_all_index(void)
{
	this->conection().ensureIndex(MailOffline::COLLECTION,
			BSON(MailOffline::MAIL_ID << 1), true);
	this->conection().ensureIndex(MailOffline::COLLECTION,
			BSON(MailOffline::ROLE_ID << 1 << MailOffline::FLAG << 1), false);
	this->conection().ensureIndex(MailOffline::COLLECTION,
			BSON(MailOffline::ROLE_ID << 1 << MailOffline::FLAG << 1
					<< MailOffline::MAIL_ID << 1), false);
	this->conection().ensureIndex(MailOffline::COLLECTION,
				BSON(MailOffline::TIME << -1), false);
}


MMOMail::MMOMail(void)
{ }

MMOMail::~MMOMail(void)
{ }

int MMOMail::update_data(MapLogicPlayer *player, MongoDataMap *data_map)
{
	BSONVec mail_set;
	MailBox::MailMap &mail_map = player->mail_box().__mail_map;

	for (MailBox::MailMap::iterator iter_mail = mail_map.begin();
			iter_mail != mail_map.end(); ++iter_mail)
	{
		MailInformation* mail_info = iter_mail->second;

		BSONVec goods_vec;
		for (ItemListMap::iterator iter_goods = mail_info->goods_map_.begin();
				iter_goods != mail_info->goods_map_.end(); ++iter_goods)
		{
			PackageItem* pack_item = iter_goods->second;
			goods_vec.push_back(GameCommon::item_to_bson(pack_item));
		}

		BSONVec resource_vec;
		GameCommon::map_to_bson(resource_vec, mail_info->resource_map_);

		mail_set.push_back(BSON(MailInfo::Info::MAIL_ID << mail_info->mail_index_
				<< MailInfo::Info::TYPE << mail_info->mail_type_
				<< MailInfo::Info::FORMAT << mail_info->mail_format_
				<< MailInfo::Info::HAS_READ << mail_info->has_read_
				<< MailInfo::Info::TIME << mail_info->send_time_
				<< MailInfo::Info::READ_TICK << mail_info->read_tick_
				<< MailInfo::Info::SENDER_NAME << mail_info->sender_name_
				<< MailInfo::Info::SENDER_ID << mail_info->sender_id_
				<< MailInfo::Info::TITLE << mail_info->mail_title_
				<< MailInfo::Info::CONTENT << mail_info->mail_content_
				<< Package::MONEY << GameCommon::money_to_bson(mail_info->money_)
				<< MailInfo::Info::GOODS << goods_vec
				<< MailInfo::Info::RESOURCE << resource_vec
                << MailInfo::Info::LABEL << mail_info->label_id_
                <<MailInfo::Info::SENDER_VIP << mail_info->sender_vip_));
	}

	BSONObjBuilder builder;
	builder << MailInfo::INFO << mail_set;

    data_map->push_update(MailInfo::COLLECTION, BSON(MailInfo::ID << player->role_id()),
    		builder.obj(), true);
    return 0;
}

int MMOMail::load_player_mail(MapLogicPlayer * player)
{
    BSONObj res = this->conection().findOne(MailInfo::COLLECTION,
    		QUERY(MailInfo::ID << player->role_id()));
    JUDGE_RETURN(res.isEmpty() == false, 0);

    MailBox& mail_box = player->mail_box();

    int test_cnt = 0;
    BSONObj info_obj = res.getObjectField(MailInfo::INFO.c_str());

    BSONObjIterator info_iter(info_obj);
    while (info_iter.more())
    {
    	BSONObj obj = info_iter.next().embeddedObject();

        MailInformation* mail_info = POOL_MONITOR->mail_info_pool()->pop();
        JUDGE_CONTINUE(mail_info != NULL);

        mail_info->mail_index_ = obj[MailInfo::Info::MAIL_ID].numberLong();
        mail_info->mail_type_ = obj[MailInfo::Info::TYPE].numberInt();
        mail_info->mail_format_ = obj[MailInfo::Info::FORMAT].numberInt();
        mail_info->has_read_ = obj[MailInfo::Info::HAS_READ].numberInt();
        mail_info->send_time_ = obj[MailInfo::Info::TIME].numberLong();
        mail_info->read_tick_ = obj[MailInfo::Info::READ_TICK].numberLong();
        mail_info->sender_name_ = obj[MailInfo::Info::SENDER_NAME].str();
        mail_info->mail_title_ = obj[MailInfo::Info::TITLE].str();
        mail_info->mail_content_ = obj[MailInfo::Info::CONTENT].str();
        mail_info->sender_id_ = obj[MailInfo::Info::SENDER_ID].numberLong();
        mail_info->label_id_ = obj[MailInfo::Info::LABEL].numberInt();
        mail_info->sender_vip_ = obj[MailInfo::Info::SENDER_VIP].numberInt();

    	GameCommon::bson_to_map(mail_info->resource_map_,
    			res.getObjectField(MailInfo::Info::RESOURCE.c_str()));

//    	BSONObj money_obj = obj.getObjectField(Package::MONEY.c_str());
//        GameCommon::bson_to_money(mail_info->money_, money_obj);

        BSONObjIterator iter_item(obj.getObjectField(MailInfo::Info::GOODS.c_str()));
        while(iter_item.more())
        {
        	BSONObj item_obj = iter_item.next().embeddedObject();

        	int id = item_obj[Package::PackItem::ID].numberInt();
        	PackageItem* item  = GamePackage::pop_item(id);
        	JUDGE_CONTINUE(item != NULL);

        	GameCommon::bson_to_item(item, item_obj);
        	mail_info->goods_map_[item->__index] = item;
        }

        mail_box.__mail_map[mail_info->mail_index_] = mail_info;
    	++test_cnt;
    }

    MSG_USER("-hyk_load_player_mail- player_id : %ld %ld, mail_amount : %d", player->role_id(), res[MailInfo::ID].numberLong(), test_cnt);
	return 0;
}

void MMOMail::ensure_all_index(void)
{
	BEGIN_CATCH
	this->conection().ensureIndex(MailInfo::COLLECTION, BSON(Role::ID << 1), true);
	END_CATCH
}


