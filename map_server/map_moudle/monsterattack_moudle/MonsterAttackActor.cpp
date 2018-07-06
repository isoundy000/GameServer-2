/*
 * MonsterAttackActor.cpp
 *
 *  Created on: 2016年9月27日
 *      Author: lyw
 */

#include "MonsterAttackActor.h"
#include "MonsterAttackSystem.h"
#include "ProtoDefine.h"
#include "MapMonitor.h"
#include "MonsterAttackScene.h"

MonsterAttackActor::MonsterAttackActor() {
	// TODO Auto-generated constructor stub

}

MonsterAttackActor::~MonsterAttackActor() {
	// TODO Auto-generated destructor stub
}

void MonsterAttackActor::reset()
{

}

int MonsterAttackActor::enter_scene(const int type)
{
	switch (this->mattack_enter_scene_type())
	{
	case GameEnum::ET_MONSTER_ATTACK:
	{
		return this->on_enter_mattack_scene(type);
	}
	}

	return 0;
}

int MonsterAttackActor::exit_scene(const int type)
{
	switch (this->mattack_enter_scene_type())
	{
	case GameEnum::ET_MONSTER_ATTACK:
	{
		this->on_exit_mattack_scene(type);
		break;
	}
	}

	return MapPlayer::exit_scene(type);
}

int MonsterAttackActor::die_process(const int64_t fighter_id)
{
	int ret = MapPlayer::die_process(fighter_id);
	JUDGE_RETURN(fighter_id > 0, ret);

	switch (this->mattack_enter_scene_type())
	{
	case GameEnum::ET_MONSTER_ATTACK:
	{
		this->mattack_die_process(fighter_id);
		break;
	}
	}

	return ret;
}

int MonsterAttackActor::on_enter_mattack_scene(const int type)
{
	MAttackInfo* mattack = MONSTER_ATTACK_SYSTEM->find_mattack(this->role_id());
	JUDGE_RETURN(mattack != NULL, ERROR_SCENE_NO_EXISTS);

	MonsterAttackScene* mattack_scene = MONSTER_ATTACK_SYSTEM->find_mattack_scene(this->space_id());
	JUDGE_RETURN(mattack_scene != NULL, ERROR_SCENE_NO_EXISTS);

	this->init_mover_scene(mattack_scene);
	MapPlayer::enter_scene(type);

	this->set_camp_id(mattack->camp_id());
	mattack->name_ 	= this->role_name();
	mattack->sex_  	= this->fight_sex();
	mattack_scene->enter_player(mattack);
	return 0;
}

int MonsterAttackActor::on_exit_mattack_scene(const int type)
{
	MAttackInfo* mattack = MONSTER_ATTACK_SYSTEM->find_mattack(this->role_id());
	JUDGE_RETURN(mattack != NULL, -1);

	MonsterAttackScene* mattack_scene = MONSTER_ATTACK_SYSTEM->find_mattack_scene(this->space_id());
	JUDGE_RETURN(mattack_scene != NULL, -1);

	mattack_scene->exit_player(mattack);
	return 0;
}

int MonsterAttackActor::mattack_scene()
{
	return GameEnum::MATTACK_SCENE_ID;
}

int MonsterAttackActor::mattack_enter_scene_type()
{
	switch (this->scene_id())
	{
	case GameEnum::MATTACK_SCENE_ID:
	{
		return GameEnum::ET_MONSTER_ATTACK;
	}
	}
	return -1;
}

int MonsterAttackActor::request_enter_player(void)
{
	CONDITION_NOTIFY_RETURN(GameCommon::is_normal_scene(this->scene_id()) == true,
			RETURN_MATTACK_REQUEST_ENTER, ERROR_NORMAL_SCENE);

	Proto30400051 enter_info;
	enter_info.set_enter_type(GameEnum::ET_MONSTER_ATTACK);

	int ret = this->send_request_enter_info(this->mattack_scene(), enter_info);
	CONDITION_NOTIFY_RETURN(ret == 0, RETURN_MATTACK_REQUEST_ENTER, ret);

	return 0;
}

int MonsterAttackActor::request_get_fighting_will()
{
	CONDITION_NOTIFY_RETURN(this->scene_id() == this->mattack_scene(),
			RETURN_FIGHTING_WILL_INFO, ERROR_USE_SCENE_LIMIT);

	Proto50400122 respond;
	const Json::Value& conf = CONFIG_INSTANCE->scene(this->scene_id());
	CONDITION_NOTIFY_RETURN(conf != Json::Value::null, RETURN_FIGHTING_WILL_INFO,
			ERROR_CONFIG_NOT_EXIST);

	const Json::Value& buff_wave = conf["buff_wave"];
	for (uint i = 0; i < buff_wave.size(); ++i)
	{
		int label_id = buff_wave[i][2u].asInt();
		MAttackLabelRecord* label_record = MONSTER_ATTACK_SYSTEM->find_label_record(label_id);

		ProtoMAttackLabel *label_info = respond.add_mattack_label();
		label_info->set_label_id(label_id);
		if (label_record != NULL)
		{
			label_info->set_role_id(label_record->role_id_);
			label_info->set_role_name(label_record->role_name_);
			label_info->set_role_sex(label_record->role_sex_);
		}
	}
	return this->respond_to_client(RETURN_FIGHTING_WILL_INFO, &respond);
}

int MonsterAttackActor::handle_exit_mattack_scene(void)
{
	switch (this->mattack_enter_scene_type())
	{
	case GameEnum::ET_MONSTER_ATTACK:
	{
		this->transfer_to_save_scene();
		break;
	}
	}

	return 0;
}

int MonsterAttackActor::mattack_die_process(Int64 fighter_id)
{
	Int64 real_fighter = this->fetch_benefited_attackor_id(fighter_id);
    JUDGE_RETURN(real_fighter != this->role_id(), -1);

    MonsterAttackScene* mattack_scene = MONSTER_ATTACK_SYSTEM->find_mattack_scene(this->space_id());
	JUDGE_RETURN(mattack_scene != NULL, -1);

	return 0;
}

