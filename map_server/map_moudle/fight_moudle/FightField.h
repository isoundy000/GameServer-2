/*
 * FightField.h
 *
 * Created on: 2013-03-28 15:26
 *     Author: lyz
 */

#ifndef _FIGHTFIELD_H_
#define _FIGHTFIELD_H_

#include <string>

struct Effect
{
	static const std::string REST;

    static const std::string HURT;
    static const std::string BUFF;
    static const std::string STAGE_BUFF;
    static const std::string CURE;
    static const std::string OUTPUT_HURT;
    static const std::string BLOOD;
    static const std::string MAXBLOOD;
    static const std::string DIRECTBLOOD;
    static const std::string DIRECTMAXBLOOD;
    static const std::string MAGIC;
    static const std::string MAXMAGIC;
    static const std::string DIRECTMAGIC;
    static const std::string DIRECTMAXMAGIC;
    static const std::string SELFMAGIC;
    static const std::string SPEED;
    static const std::string SILENCE;
    static const std::string STAY;
    static const std::string DIZZY;
    static const std::string SUPPERMAN;
    static const std::string ATTACK;
    static const std::string DEFENCE;
    static const std::string CRIT;
    static const std::string TOUGHNESS;
    static const std::string HIT;
    static const std::string AVOID;
    static const std::string HURTDEEP;
    static const std::string CRITHURTDEEP;
    static const std::string EXEMPT;
    static const std::string MAGICHURT;
    static const std::string DMAGICHURT;
    static const std::string REBOUNDHURT;
    static const std::string REBOUNDDEFENCE;
    /* 反弹伤害,按造成伤害者的最大血量计算百分比 */
    static const std::string REBOUNDMAXBLOOD;
    static const std::string SKILLDISTANCE;
    static const std::string RELIVE;
    static const std::string DIRECTHURT;
    static const std::string AREAHURT;
    static const std::string SUMMON;
    static const std::string PULL;
    static const std::string PUSH;
    static const std::string JUMP;
    static const std::string FORWARD;
    static const std::string ASSAULT;
    static const std::string BACKWARD;
    static const std::string NSEAL;
    static const std::string NSERIOUS;
    static const std::string NRESIDUAL;
    static const std::string NANGRY;
    static const std::string NCOVER;
    static const std::string NSTAY;
    static const std::string SELFBLOOD;
    static const std::string PEASANT;
    static const std::string SELFSUPPERMAN;
    static const std::string IMMUNE;
    static const std::string HURT_MAX_BLOOD;
    static const std::string SELF_CRIT_RATE;
    static const std::string REPEATHURT;
    static const std::string BLOOD_BACK_REDUCE;
    static const std::string SELF_HURTDEEP;
    static const std::string SELF_DIZZY;
    static const std::string SELF_STAY;
    static const std::string CLEAN;
    static const std::string SELF_ATTACK;
    static const std::string SELF_DEFENCE;
    static const std::string SELF_INFMAGIC;
    static const std::string METEOR;
    static const std::string PROP;
    static const std::string RECOVERT_MAGIC;
    static const std::string MAGIC_REDUCE_HURT;
    static const std::string ATTACK_NDEL;
    static const std::string ADDHURT_NDEL;
    static const std::string CRIT_ATTACK_NDEL;
    static const std::string MAXBLOOD_NDEL;
    static const std::string SHIELD;
    static const std::string LIFE_SHIELD;
    static const std::string PERCY;
    static const std::string FLASH_AVOID;
    static const std::string MASTER_EXEMPT;
    static const std::string STONE_PLAYER;
    static const std::string AREA_DDEFENCE;
    static const std::string CRAZY_ANGRY;
    static const std::string ICE_INSIDE;
    static const std::string JIAN_DROP;
    static const std::string QUIT_REGION_TRANSFER;
};

#endif //_FIGHTFIELD_H_
