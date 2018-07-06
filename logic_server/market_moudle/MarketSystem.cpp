/*
 * MarketSystem.cpp
 *
 *  Created on: 2013-7-19
 *      Author: root
 */

#include "MarketSystem.h"
#include "MMOMarket.h"
#include "ArrTimeManager.h"

MarketSystem::MarketSystem()
{
	this->market_item_timer_ = new ArrTimeManager(
			MarketSystem::handle_market_item);
}

MarketSystem::~MarketSystem()
{

}

int MarketSystem::init(void)
{
	return 0;
}

int MarketSystem::fina(void)
{
    SAFE_DELETE(this->market_item_timer_);
	return 0;
}

int MarketSystem::start(void)
{
	MMOMarket::load_market_system();
	MMOMarket::load_all_market_item();

	MSG_USER("MarketSystem start...");
	return 0;
}

int MarketSystem::stop(void)
{
	MSG_USER("Market System stop");

	MMOMarket::remove_all_market_item();
	MMOMarket::save_market_system();

	this->undo_all_buy();
	this->save_market();

	return 0;
}

void MarketSystem::save_market(int direct_save)
{
	BIntMap item_map = this->market_item_package_.fetch_index_map();
	for (BIntMap::iterator iter = item_map.begin(); iter != item_map.end();
			++iter)
	{
		MarketItem* market_item = this->find_market_item(iter->first);
		MMOMarket::save_market_item(market_item, direct_save);
	}
}

void MarketSystem::undo_all_buy(void)
{
	BIntMap item_map = this->market_item_package_.fetch_index_map();
	for (BIntMap::iterator iter = item_map.begin(); iter != item_map.end();
			++iter)
	{
		MarketItem* market_item = this->find_market_item(iter->first);
		market_item->item_obj_.__amount += market_item->undo_amount_;
		market_item->undo_amount_ = 0;
	}
}

int MarketSystem::market_handle_time_out(Int64 now_tick)
{
	this->market_item_timer_->handle_time_out(now_tick);
	return 0;
}

MarketerInfo* MarketSystem::find_marketer(Int64 role_id, int flag)
{
	MarketerInfo* marketer = this->marketer_package()->find_object(role_id);
	JUDGE_RETURN(marketer == NULL, marketer);
	JUDGE_RETURN(flag == true, NULL);

	marketer = this->marketer_package()->pop_object();
	JUDGE_RETURN(marketer != NULL, NULL);

	marketer->role_index_ = role_id;
	this->marketer_package()->bind_object(marketer->role_index_, marketer);

	return marketer;
}

MarketItem* MarketSystem::find_market_item(int market_index)
{
	return this->market_item_package_.find_object(market_index);
}

MarketItemType* MarketSystem::find_market(int market_type)
{
	JUDGE_RETURN(market_type > 0, NULL);

	MarketItemType* market = this->market_type_package()->find_object(market_type);
	JUDGE_RETURN(market == NULL, market);

	market = this->market_type_package()->pop_object();
	JUDGE_RETURN(market != NULL, NULL);

	market->market_type_ = market_type;
	this->market_type_package()->bind_object(market->market_type_, market);

	return market;
}

MarketItemType* MarketSystem::find_market_by_item(int item_id)
{
	return this->find_market(this->fetch_main_type(item_id));
}

int MarketSystem::fetch_main_type(int item_id)
{
	const Json::Value& conf = CONFIG_INSTANCE->market(item_id);
	return conf["type"].asInt() / 100;
}

int MarketSystem::fetch_sub_type(int item_id)
{
	const Json::Value& conf = CONFIG_INSTANCE->market(item_id);
	return conf["type"].asInt() % 100;
}

MarketItem* MarketSystem::pop_market_item()
{
	MarketItem* market_item = this->market_item_package_.pop_object();
	JUDGE_RETURN(market_item != NULL, NULL);

	this->market_item_package_.bind_object(market_item->market_index_, market_item);
	return market_item;
}

int MarketSystem::push_market_item(MarketItem* market_item)
{
	return this->market_item_package_.unbind_and_push(
			market_item->market_index_,	market_item);
}

int MarketSystem::set_low_price(int item_id, int price)
{
	if (this->low_price_map_.count(item_id) == 0)
	{
		this->low_price_map_[item_id] = price;
	}
	else
	{
		int min_price = std::min<int>(price, this->low_price_map_[item_id]);
		this->low_price_map_[item_id] = min_price;
	}
	return 0;
}

int MarketSystem::fetch_low_price(int item_id)
{
	JUDGE_RETURN(this->low_price_map_.count(item_id) > 0, 0);
	return this->low_price_map_[item_id];
}

int MarketSystem::arrive_max_onsell(Int64 role_index)
{
	MarketerInfo* marketer = this->marketer_package()->find_object(role_index);
	JUDGE_RETURN(marketer != NULL, false);

	static uint MAX_ONSELL = CONFIG_INSTANCE->market_const("max_onsell").asInt();
	return marketer->onsell_map_.size() >= MAX_ONSELL;
}

int MarketSystem::add_market_item(MarketItem* market_item, bool save)
{
	JUDGE_RETURN(market_item != NULL, -1);

	MarketerInfo* marketer = this->find_marketer(market_item->role_id_);
	JUDGE_RETURN(marketer != NULL, -1);

	MarketItemType* market = this->find_market(market_item->main_type_);
	JUDGE_RETURN(market != NULL, -1);

	market->item_set_.push_back(market_item);
	marketer->onsell_map_[market_item->market_index_] = market_item;

	this->set_low_price(market_item->item_obj_.__id, market_item->price_);
	this->market_item_timer_->add_item(market_item);
	JUDGE_RETURN(save == true, 0);

	MMOMarket::save_market_item(market_item, false);
	return 0;
}

int MarketSystem::remove_market_item(MarketItem* market_item)
{
	JUDGE_RETURN(market_item != NULL, -1);
	this->market_item_timer_->remove_item(market_item);

	MarketItemType* market = this->find_market(market_item->main_type_);
	if (market != NULL)
	{
		market->erase_market_item(market_item);
	}

	MarketerInfo* marketer = this->find_marketer(market_item->role_id_);
	if (marketer != NULL)
	{
		marketer->onsell_map_.erase(market_item->market_index_);
	}

	MMOMarket::remove_market_item(market_item, false);
	return 0;
}

int MarketSystem::update_market_item(MarketItem* market_item)
{
	JUDGE_RETURN(market_item != NULL, -1);
	MMOMarket::save_market_item(market_item, false);
	return 0;
}

PoolPackage<MarketItemType>* MarketSystem::market_type_package()
{
	return &this->market_type_package_;
}

PoolPackage<MarketerInfo, Int64>* MarketSystem::marketer_package()
{
	return &this->marketer_package_;
}

int MarketSystem::handle_market_item(ArrTimeItem* arr_item)
{
	MarketItem* market_item = dynamic_cast<MarketItem*>(arr_item);
	JUDGE_RETURN(market_item != NULL, -1);

	MarketItemType* market = MARKET_SYS->find_market(market_item->main_type_);
	JUDGE_RETURN(market != NULL, -1);

	market->erase_market_item(market_item);
	return 0;
}
