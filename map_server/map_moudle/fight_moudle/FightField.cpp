/*
 * FightField.cpp
 *
 * Created on: 2013-03-28 15:27
 *     Author: lyz
 */

#include "FightField.h"

const std::string Effect::REST = "rest";
const std::string Effect::HURT = "hurt";
const std::string Effect::BUFF = "buff";
const std::string Effect::STAGE_BUFF = "stage_buff";
const std::string Effect::BLOOD = "blood";
const std::string Effect::MAXBLOOD = "max_blood";
const std::string Effect::DIRECTBLOOD = "direct_blood";
const std::string Effect::DIRECTMAXBLOOD = "direct_max_blood";
const std::string Effect::CURE = "cure";
const std::string Effect::OUTPUT_HURT  = "output_hurt";
const std::string Effect::MAGIC = "magic";
const std::string Effect::MAXMAGIC = "max_magic";
const std::string Effect::DIRECTMAGIC = "direct_magic";
const std::string Effect::DIRECTMAXMAGIC = "direct_max_magic";
const std::string Effect::SELFMAGIC = "self_magic";
const std::string Effect::SPEED = "speed";
const std::string Effect::SILENCE = "silence";
const std::string Effect::STAY = "stay";
const std::string Effect::DIZZY = "dizzy";
const std::string Effect::SUPPERMAN = "supperman";
const std::string Effect::ATTACK = "attack";
const std::string Effect::DEFENCE = "defence";
const std::string Effect::CRIT = "crit";
const std::string Effect::TOUGHNESS = "toughness";
const std::string Effect::HIT = "hit";
const std::string Effect::AVOID = "avoid";
const std::string Effect::HURTDEEP = "hurtdeep";
const std::string Effect::CRITHURTDEEP = "crithurtdeep";
const std::string Effect::EXEMPT = "exempt";
const std::string Effect::MAGICHURT = "magic_hurt";
const std::string Effect::DMAGICHURT = "d_magic_hurt";
const std::string Effect::REBOUNDHURT = "rebound_hurt";
const std::string Effect::REBOUNDDEFENCE = "rebound_defence";
const std::string Effect::REBOUNDMAXBLOOD = "rebound_maxblood";
const std::string Effect::SKILLDISTANCE = "skilldistance";
const std::string Effect::RELIVE = "relive";
const std::string Effect::DIRECTHURT = "direct_hurt";
const std::string Effect::AREAHURT = "area_hurt";
const std::string Effect::SUMMON = "summon";
const std::string Effect::PULL = "pull";
const std::string Effect::PUSH = "push";
const std::string Effect::JUMP = "jump";
const std::string Effect::FORWARD = "forward";
const std::string Effect::ASSAULT = "assault";
const std::string Effect::BACKWARD = "backward";
const std::string Effect::NSEAL = "nseal";
const std::string Effect::NSERIOUS = "nserious";
const std::string Effect::NRESIDUAL = "nresidual";
const std::string Effect::NANGRY = "nangry";
const std::string Effect::NCOVER = "ncover";
const std::string Effect::NSTAY = "nstay";
const std::string Effect::SELFBLOOD = "self_blood";
const std::string Effect::PEASANT = "peasant";
const std::string Effect::SELFSUPPERMAN = "self_supperman";
const std::string Effect::IMMUNE = "immune";
const std::string Effect::HURT_MAX_BLOOD = "hurt_max_blood";	// 根据受击者最大血量计算伤害
const std::string Effect::SELF_CRIT_RATE = "self_crit_rate";
const std::string Effect::REPEATHURT = "repeathurt";
const std::string Effect::BLOOD_BACK_REDUCE = "blood_back_reduce";
const std::string Effect::SELF_HURTDEEP = "self_hurtdeep";
const std::string Effect::SELF_DIZZY = "self_dizzy";
const std::string Effect::SELF_STAY = "self_stay";
const std::string Effect::CLEAN = "clean";
const std::string Effect::SELF_ATTACK = "self_attack";
const std::string Effect::SELF_DEFENCE = "self_defence";
const std::string Effect::SELF_INFMAGIC = "self_infmagic";
const std::string Effect::METEOR = "meteor";
const std::string Effect::PROP = "prop";
const std::string Effect::RECOVERT_MAGIC = "recovert_magic";
const std::string Effect::MAGIC_REDUCE_HURT = "mreduce_hurt";
const std::string Effect::ATTACK_NDEL = "attack_ndel";
const std::string Effect::ADDHURT_NDEL = "addhurt_ndel";
const std::string Effect::CRIT_ATTACK_NDEL = "crit_attack_ndel";
const std::string Effect::MAXBLOOD_NDEL = "max_blood_ndel";
const std::string Effect::SHIELD = "shield";
const std::string Effect::LIFE_SHIELD = "life_shield";
const std::string Effect::PERCY = "percy";
const std::string Effect::FLASH_AVOID = "flash_avoid";
const std::string Effect::MASTER_EXEMPT = "master_exempt";
const std::string Effect::STONE_PLAYER = "stone_player";
const std::string Effect::AREA_DDEFENCE	= "area_defence";
const std::string Effect::CRAZY_ANGRY = "crazy_angry";		// 狂怒
const std::string Effect::ICE_INSIDE = "ice_inside";
const std::string Effect::JIAN_DROP = "jian_drop";
const std::string Effect::QUIT_REGION_TRANSFER = "quit_region_transfer";


