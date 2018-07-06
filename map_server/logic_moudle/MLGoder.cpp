/*
 * MLGoder.cpp
 *
 *  Created on: May 23, 2017
 *      Author: peizhibi
 */

#include "MLGoder.h"
#include "ProtoDefine.h"

void GoderDetail::reset()
{
	this->open_ = 0;
	this->grade_ = 0;
	this->fight_prop_.reset();
}

const Json::Value& GoderDetail::conf()
{
	return CONFIG_INSTANCE->change_goder(this->grade_);
}

MLGoder::MLGoder()
{
	// TODO Auto-generated constructor stub
}

MLGoder::~MLGoder()
{
	// TODO Auto-generated destructor stub
}

void MLGoder::reset_goder()
{
	this->goder_detail_.reset();
}

void MLGoder::add_goder_grade()
{
	GoderDetail& detail = this->goder_detail_;
	JUDGE_RETURN(detail.open_ == true, ;);

	detail.grade_ += 1;
	this->calculate_goder_prop();
	this->refresh_goder_property();
}

void MLGoder::calculate_goder_prop()
{
	GoderDetail& detail = this->goder_detail_;
	JUDGE_RETURN(detail.open_ == true, ;);

	const Json::Value& conf = detail.conf();
	detail.fight_prop_.reset();
	detail.fight_prop_.make_up_name_prop(conf);

	GamePackage* package = this->find_package(GameEnum::INDEX_GODER);
	for (ItemListMap::iterator iter = package->item_map().begin();
			iter != package->item_map().end(); ++iter)
	{
		PackageItem* item = iter->second;
		JUDGE_CONTINUE(item != NULL);
		detail.fight_prop_.add_fight_property(item->__prop_info);
	}

	detail.fight_prop_.caculate_force();
}

void MLGoder::refresh_goder_property(int type)
{
	IntMap prop_map;
	this->goder_detail_.fight_prop_.serialize(prop_map);
	this->refresh_fight_property(BasicElement::GODER, prop_map, type);
}

int MLGoder::fetch_goder_info(int equip)
{
	Proto51400421 goder_info;

	GoderDetail& detail = this->goder_detail_;
	goder_info.set_open(detail.open_);
	goder_info.set_grade(detail.grade_);

	detail.fight_prop_.serialize(goder_info.mutable_prop());
	this->respond_to_client(RETURN_FETCH_GODER_INFO, &goder_info);

	JUDGE_RETURN(equip == true, 0);
	return this->notify_pack_info(GameEnum::INDEX_GODER);
}

int MLGoder::goder_operate()
{
	GoderDetail& detail = this->goder_detail_;
	if (detail.open_ == false)
	{
		return this->open_goder();
	}
	else
	{
		return this->goder_upgrade();
	}
}

int MLGoder::open_goder()
{
	GoderDetail& detail = this->goder_detail_;
	JUDGE_RETURN(detail.open_ == false, -1);

	detail.open_ = true;
	this->add_goder_grade();
	this->fetch_goder_info();
	FINER_PROCESS_NOTIFY(RETURN_GODER_UPGRADE);
}

int MLGoder::goder_upgrade()
{
	GoderDetail& detail = this->goder_detail_;
	CONDITION_NOTIFY_RETURN(detail.open_ == true,
			RETURN_GODER_UPGRADE, ERROR_CLIENT_OPERATE);

	const Json::Value& conf = detail.conf();
	CONDITION_NOTIFY_RETURN(conf.isMember("goods") == true,
			RETURN_GODER_UPGRADE, ERROR_CLIENT_OPERATE);

	UpgradeAmountInfo amount_info(this, conf["goods"][0u].asInt(),
			conf["goods"][1u].asInt());

	CONDITION_NOTIFY_RETURN(amount_info.buy_amount_ == 0,
			RETURN_GODER_UPGRADE, ERROR_CLIENT_OPERATE);

	this->pack_remove(ITEM_USE_GODER_UPGRADE, amount_info.buy_item_,
			amount_info.total_use_amount_);

	this->add_goder_grade();
	this->fetch_goder_info();
	FINER_PROCESS_NOTIFY(RETURN_GODER_UPGRADE);
}

