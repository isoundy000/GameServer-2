/*
 * GameField.cpp
 *
 * Created on: 2013-02-20 16:30
 *     Author: glendy
 */

#include "GameField.h"

/////////////////DBPairObj/////////////////////
DBField DBPairObj::KEY = "key";
DBField DBPairObj::VALUE = "value";
DBField DBPairObj::TICK = "tick";
DBField DBPairObj::NAME = "name";

/////////////////DBItemObj/////////////////////
DBField DBItemObj::ID = "item_id";
DBField DBItemObj::AMOUNT = "item_amount";
DBField DBItemObj::BIND = "item_bing";
DBField DBItemObj::INDEX = "item_index";

///////////////////Global////////////////
DBField Global::COLLECTION = "mmo.global";
DBField Global::KEY = "key";
DBField Global::ROLE = "role";
DBField Global::GID = "id";
DBField Global::CONTENT = "content";
DBField Global::MAIL = "mail";
DBField Global::FRIENDSHIP = "friendship";
DBField Global::LEAGUE = "league";
DBField Global::VOICE_ID = "voice_id";
DBField Global::BEAST = "beast";
DBField Global::CUSTOMER_SERVICE = "customer_service";
DBField Global::SCRIPT_PROGRESS = "script_progress";
DBField Global::WEDDING = "wedding";
DBField Global::LSTORE_APPLY = "lstore_apply";
DBField Global::LSTORE_ITEM_ID = "lstore_item_id";
DBField Global::BROTHER = "brother";
DBField Global::TRAVEL_TEAM = "travel_team";
DBField Global::AVERAGE_ROLE_LEVEL = "average_role_level";

///////////////////Role//////////////////
const std::string Role::COLLECTION = "mmo.role";
const std::string Role::IS_ACTIVE = "is_active";
const std::string Role::ACCOUNT = "account";
const std::string Role::SERVER_FLAG = "server_flag";
const std::string Role::AGENT = "agent";
const std::string Role::AGENT_CODE = "agent_code";
const std::string Role::PLATFORM = "platform";
const std::string Role::PLATFORM_CODE = "platform_code";
const std::string Role::MARKET_CODE = "market_code";
const std::string Role::IS_NEW = "is_new";
const std::string Role::IS_FIRST_RENAME = "is_first_rename";
const std::string Role::BAN_TYPE = "ban_type";
const std::string Role::BAN_EXPIRED = "ban_expired";
const std::string Role::ID = "id";
const std::string Role::NAME = "name";
const std::string Role::SRC_NAME = "src_name";
const std::string Role::FULL_NAEM = "full_name";
const std::string Role::LEVEL = "level";
const std::string Role::EXP = "exp";
const std::string Role::SEX = "sex";
const std::string Role::CAREER = "career";
const std::string Role::VIP_TYPE = "vip_type";
const std::string Role::FORCE = "force";
const std::string Role::PREV_FORCE_MAP = "prev_force_map";
const std::string Role::WATCH_STATUS = "watch_status";
const std::string Role::IS_TRANSFER_WATCH = "is_transfer_watch";
const std::string Role::FIRST_WBOSS_TIME = "first_wboss_time";
const std::string Role::ALREADY_LEAGUE = "already_league";
const std::string Role::LEAGUE_ID = "league_id";
const std::string Role::LEAGUE_NAME = "league_name";
const std::string Role::PARTNER_ID = "partner_id";
const std::string Role::PARTNER_NAME = "partner_name";
const std::string Role::WEDDING_ID = "wedding_id";
const std::string Role::WEDDING_TYPE = "wedding_type";
const std::string Role::SAVE_TRVL_SCENE = "save_trvl_scene";
const std::string Role::DAY_RESET_TICK = "day_reset";
const std::string Role::WEEK_RESET_TICK = "week_reset";
const std::string Role::ML_DAY_RESET_TICK = "ml_day_reset";
const std::string Role::VIEW_TICK = "view_tick";
const std::string Role::NOTICE_DRAW_TICK = "notice_draw_tick";
const std::string Role::SHAPE_DETAIL = "shape_detail";
const std::string Role::LABEL_INFO = "label_info";
const std::string Role::BUY_MAP = "buy_map";
const std::string Role::BUY_TOTAL_MAP = "buy_total_map";
const std::string Role::PANIC_BUY_NOTIFY = "panic_buy_notify";
const std::string Role::MOUNT_INFO = "mount_info2";

const std::string Role::WEDDING_SELF = "wedding_self";
const std::string Role::WEDDING_SIDE = "wedding_side";

const std::string Role::SERVER_TICK = "server_tick";
const std::string Role::COMBINE_TICK = "combine_tick";
const std::string Role::IS_YELLOW = "is_yellow";
const std::string Role::KILL_NUM = "kill_num";
const std::string Role::KILL_NORMAL = "kill_normal";
const std::string Role::KILL_EVIL = "kill_evil";
const std::string Role::IS_BROCAST = "is_brocast";
const std::string Role::ONLINE_TICKS = "online_ticks";
const std::string Role::KILL_VALUE = "kill_value";
const std::string Role::BROTHER_REWARD_INDEX = "brother_reward_index";
const std::string Role::TRANSLATE_TO_ENEMY_TIMES = "translate_to_enemy_times";
const std::string Role::IS_WORSHIP = "is_worship";
const std::string Role::TODAY_RECHARGE_GOLD = "today_recharge_gold";
const std::string Role::TODAY_CONSUME_GOLD = "today_consume_gold";
const std::string Role::TODAY_BUY_TIMES = "today_buy_times";
const std::string Role::TODAY_CAN_BUY_TIMES = "today_can_buy_times";
const std::string Role::GASHAPON_BUY_TIMES = "gashapon_buy_times";
const std::string Role::CONTINUITY_LOGIN_DAY = "continuity_login_day";
const std::string Role::CONTINUITY_LOGIN_FLAG = "continuity_login_flag";
const std::string Role::CREATE_TIME = "create_time";
const std::string Role::IP = "ip";
const std::string Role::LAST_SIGN_IN = "last_sign_in";
const std::string Role::LAST_SIGN_OUT = "last_sign_out";
const std::string Role::LOGIN_TICK = "login_tick";
const std::string Role::LOGIN_DAYS = "login_day";
const std::string Role::LOGIN_COUNT = "login_count";
const std::string Role::PERMISSION = "permission";
const std::string Role::DRAW_DAY = "draw_day";
const std::string Role::DRAW_GIFT = "draw_gift";
const std::string Role::DRAW_VIP = "draw_vips";
const std::string Role::RAND_USE_TIMES = "rand_use_times";
const std::string Role::RECHARGE_TOTAL_GOLD = "recharge_total_gold";
const std::string Role::SCENE_PK_STATE = "scene_pk_state";
const std::string Role::WEDDING_GIFTBOX_TICK = "wedding_giftbox_tick";
const std::string Role::WEDDING_GIFTBOX_TIMES = "wedding_giftbox_times";
const std::string Role::FRESH_FREE_RELIVE_TICK = "fresh_free_relive_tick";
const std::string Role::USED_FREE_RELIVE = "used_free_relive";
const std::string Role::SACREDSTONE_END_TICK = "sacredstone_end_tick";
const std::string Role::COLLECT_CHEST_AMOUNT = "collect_chest_amount";
const std::string Role::SACREDSTONE_EXP = "sacredstone_exp";
const std::string Role::CHANGE_NAME_TICK = "change_name_tick";
const std::string Role::CHANGE_SEX_TICK = "change_sex_tick";
const std::string Role::OPEN_GIFT_CLOSE = "open_gift_close";
const std::string Role::LAST_ACT_TYPE = "last_act_type";
const std::string Role::LAST_ACT_END_TIME = "last_act_end_time";

const std::string Role::LOCATION = "location";
const std::string Role::Location::SCENE_ID = "scene_id";
const std::string Role::Location::PIXEL_X = "pixel_x";
const std::string Role::Location::PIXEL_Y = "pixel_y";
const std::string Role::Location::TOWARD = "toward";
const std::string Role::Location::MODE = "mode";
const std::string Role::Location::SPACE_ID = "space_id";
const std::string Role::Location::TEMP_PIXEL_X = "temp_pixel_x";
const std::string Role::Location::TEMP_PIXEL_Y = "temp_pixel_y";
const std::string Role::Location::PREV_SCENE_ID = "prev_scene_id";
const std::string Role::Location::PREV_PIXEL_X = "prev_pixel_x";
const std::string Role::Location::PREV_PIXEL_Y = "prev_pixel_y";
const std::string Role::Location::PREV_MODE = "prev_mode";
const std::string Role::Location::PREV_SPACE_ID = "prev_space_id";
const std::string Role::Location::PREV_TOWN_SCENE = "prev_town_scene";
const std::string Role::Location::PREV_TOWN_X = "prev_town_x";
const std::string Role::Location::PREV_TOWN_Y = "prev_town_y";
const std::string Role::Location::SCENE_HISTORY = "scene_history";

const std::string Role::SAVE_INFO = "save_info";
const std::string Role::SaveInfo::SCENE_ID = "scene_id";
const std::string Role::SaveInfo::PK_STATE = "pk_state";
const std::string Role::SaveInfo::BLOOD = "blood";
const std::string Role::SaveInfo::MAGIC = "magic";

const std::string Role::PRACTICE_TIMES = "practice_times";
const std::string Role::PRACTICE_GOLD_TIMES = "practice_gold_times";
const std::string Role::PRACTICE_CLOUDS = "practice_clouds";
const std::string Role::ESCORT_TIMES = "escort_times";
const std::string Role::PROTECT_TIMES = "protect_times";
const std::string Role::ROB_TIMES = "rob_times";

const std::string Role::SPECIAL_BOX_INFO = "special_box_info";
const std::string Role::SpecialBoxInfo::BUY_TIMES = "buy_times";
const std::string Role::SpecialBoxInfo::SCORE = "score";
const std::string Role::SpecialBoxInfo::REFRESH_TIMES = "refresh_times";

//}}}


////////////////////RoleEx////////////////////////////
//{{{
const std::string RoleEx::COLLECTION = "mmo.role_ex";
const std::string RoleEx::ID = "id";
const std::string RoleEx::BOX_OPEN_COUNT_ONE = "box_open_count_one";
const std::string RoleEx::BOX_OPEN_COUNT_TEN = "box_open_count_ten";
const std::string RoleEx::BOX_OPEN_COUNT_FIFTY = "box_open_count_fifty";
const std::string RoleEx::SAVVY = "savvy";
const std::string RoleEx::BOX_IS_OPEN = "bos_is_open";
const std::string RoleEx::SECOND_DECOMPOSE = "second_decompose";
const std::string RoleEx::LTABLE_LEFT_TIMES = "ltable_left_times";
const std::string RoleEx::LTABLE_EXEC_TIMES = "ltable_exec_times";
const std::string RoleEx::LTABLE_GOLD = "ltable_gold";
const std::string RoleEx::LTABLE_KEY = "ltable_key";
//}}}
////////////////////VIP////////////////////////////
//{{{
const std::string Vip::COLLECTION = "mmo.vip";
const std::string Vip::ID = "id";
const std::string Vip::TYPE = "vip_type";
const std::string Vip::EXPIRED_TIME = "vip_expired_time";
const std::string Vip::START_TIME = "vip_start_time";
const std::string Vip::CHECK_FLAG = "check_flag";
const std::string Vip::IS_GIVEN = "isGiven";
const std::string Vip::WEEKLY_GIVEN = "weekly_given";
const std::string Vip::IS_GIVEN_WEEKLY = "is_given_weekly";
const std::string Vip::WEEKLY_TICK = "weekly_tick";
const std::string Vip::SUPER_VIP_TYPE = "super_vip_type";
//}}}

//////////////////DBCopyPlayer////////////////////
const std::string DBCopyPlayer::COLLECTION = "mmo.copy_player";

const std::string DBCopyPlayer::ID = "id";
const std::string DBCopyPlayer::NAME = "name";
const std::string DBCopyPlayer::LEVEL = "level";
const std::string DBCopyPlayer::VIP_TYPE = "vip";
const std::string DBCopyPlayer::SEX = "sex";
const std::string DBCopyPlayer::FORCE = "force";
const std::string DBCopyPlayer::CAREER = "career";
const std::string DBCopyPlayer::LEAGUE_ID = "league_id";
const std::string DBCopyPlayer::LEAGUE_NAME = "league_name";
const std::string DBCopyPlayer::LEAGUE_POS = "league_pos";

const std::string DBCopyPlayer::PARTNER_ID = "partner_id";
const std::string DBCopyPlayer::PARTNER_NAME = "partner_name";
const std::string DBCopyPlayer::WEDDING_ID = "wedding_id";
const std::string DBCopyPlayer::WEDDING_TYPE = "wedding_type";
const std::string DBCopyPlayer::FASHION_ID = "fashion_id";
const std::string DBCopyPlayer::FASHION_COLOR = "fashion_color";

const std::string DBCopyPlayer::ATTACK_LOWER = "attack_lower";
const std::string DBCopyPlayer::ATTACK_UPPER = "attack_upper";
const std::string DBCopyPlayer::DEFENCE_LOWER = "defence_lower";
const std::string DBCopyPlayer::DEFENCE_UPPER = "defence_upper";
const std::string DBCopyPlayer::HIT = "hit";
const std::string DBCopyPlayer::DODGE = "dodge";
const std::string DBCopyPlayer::CRIT = "crit";
const std::string DBCopyPlayer::TOUGHNESS = "toughness";
const std::string DBCopyPlayer::LUCKY = "lucky";
const std::string DBCopyPlayer::BLOOD = "blood";
const std::string DBCopyPlayer::MAGIC = "magic";
const std::string DBCopyPlayer::SPEED = "speed";
const std::string DBCopyPlayer::DAMAGE = "damage";	//伤害加成
const std::string DBCopyPlayer::REDUCTION = "reduction";	//伤害减免
const std::string DBCopyPlayer::WING_LEVEL = "wing_level";
const std::string DBCopyPlayer::SOLIDER_LEVEL = "solider_level";
const std::string DBCopyPlayer::MAGIC_LEVEL = "magic_level";
const std::string DBCopyPlayer::BEAST_LEVEL = "beast_level";
const std::string DBCopyPlayer::MOUNT_LEVEL = "mount_level";
const std::string DBCopyPlayer::WEAPON_LEVEL = "weapon_level";
const std::string DBCopyPlayer::EQUIP_REFINE_LVL = "equip_refine_lvl";

const std::string DBCopyPlayer::SKILL_SET = "skill_set";
const std::string DBCopyPlayer::SHAPE_SET = "shape_set";

//////////////////Fight////////////////////
const std::string Fight::COLLECTION = "mmo.fight";
const std::string Fight::ID = "id";
const std::string Fight::PK = "pk";
const std::string Fight::SAVE_PK = "save_pk";
const std::string Fight::CAMP_ID = "camp_id";
const std::string Fight::LEVEL = "level";
const std::string Fight::EXPERIENCE = "experience";
const std::string Fight::PROP_POINT = "prop_point";
const std::string Fight::BLOOD_BASIC = "blood_basic";
const std::string Fight::ATTACK_LOWER_BASIC = "attack_lower_basic";
const std::string Fight::ATTACK_UPPER_BASIC = "attack_upper_basic";
const std::string Fight::DEFENCE_LOWER_BASIC = "defence_lower_basic";
const std::string Fight::DEFENCE_UPPER_BASIC = "defence_upper_basic";
const std::string Fight::HIT_BASIC = "hit_basic";
const std::string Fight::AVOID_BASIC = "avoid_basic";
const std::string Fight::CRIT_BASIC = "crit_basic";
const std::string Fight::TOUGHNESS_BASIC = "toughness_basic";
const std::string Fight::MAGIC_BASIC = "magic_basic";
const std::string Fight::LUCKY_BASIC = "lucky_basic";

const std::string Fight::BLOOD = "blood";
const std::string Fight::MAGIC = "magic";
const std::string Fight::ANGRY = "angry";
const std::string Fight::GLAMOUR = "glamour";
const std::string Fight::JUMP = "jump";
const std::string Fight::PK_TICK = "pk_tick";
//

//////////////////DBMapTiny////////////////////
const std::string DBMapTiny::COLLECTION = "mmo.map_tiny";
const std::string DBMapTiny::ID = "id";

const std::string DBMapTiny::CUR_BLOOD = "cur_blood";
const std::string DBMapTiny::NON_TIPS = "non_tips";
const std::string DBMapTiny::CLIENT_GUIDE = "client_guide";
const std::string DBMapTiny::EVERYDAY_TICK = "everyday_tick";
const std::string DBMapTiny::LAST_RECHARGE_TICK = "last_recharge_tick";
const std::string DBMapTiny::DAILY_TOTAL_RECHARGE = "daily_total_recharge";


//////////////////Package////////////////////
const std::string Package::COLLECTION = "mmo.package";
const std::string Package::ID = "id";

const std::string Package::RECHARGE_FIRST_TICK = "recharge_first_tick";
const std::string Package::RECHARGE_GOLD = "recharge_gold";
const std::string Package::GAME_RESOURCE = "game_resource";
const std::string Package::USER_GAME_RESOURCE = "use_game_resource";

const std::string Package::MONEY = "money";
const std::string Package::Money::GOLD = "gold";
const std::string Package::Money::COPPER = "copper";
const std::string Package::Money::BIND_GOLD = "bind_gold";
const std::string Package::Money::BIND_COPPER = "bind_copper";

const std::string Package::PACK = "pack";
const std::string Package::Pack::PACK_TYPE = "pack_type";
const std::string Package::Pack::PACK_SIZE = "pack_size";
const std::string Package::Pack::LAST_TICK = "last_tick";
const std::string Package::Pack::STRENGTHEN = "strengthen";
const std::string Package::Pack::PACK_ITEM = "pack_item";
const std::string Package::Pack::SUBLIME_LEVEL = "sublime_level";
const std::string Package::Pack::IS_OPEN_SUBLIME = "is_open_sublime";

const std::string Package::PackItem::INDEX = "index";
const std::string Package::PackItem::ID = "id";
const std::string Package::PackItem::AMOUNT = "amount";
const std::string Package::PackItem::BIND = "bind";
const std::string Package::PackItem::USE_TICK = "tick";
const std::string Package::PackItem::USE_TIMES = "times";
const std::string Package::PackItem::TIME_OUT = "timeout";;
const std::string Package::PackItem::NEW_TAG = "new_tag";
const std::string Package::PackItem::UNIQUE_ID = "unique_id";

const std::string Package::PackItem::TIPS_LEVEL = "tips_level";
const std::string Package::PackItem::TIPS_TIME_MAP = "tips_time_map";
const std::string Package::PackItem::TIPS_STATUS_MAP = "tips_status_map";
const std::string Package::PackItem::OUT_TIME_ITEM_ID = "out_time_item_id";
const std::string Package::PackItem::OUT_TIME_ITEM_AMOUNT = "out_time_item_amount";
const std::string Package::PackItem::OUT_TIME_ITEM_BIND = "out_time_item_bind";

const std::string Package::Pack::PACK_EQUIP = "equip";
const std::string Package::PackEquip::REFINE_LEVEL = "refine_level";
const std::string Package::PackEquip::REFINE_DEGREE = "refine_degree";
const std::string Package::PackEquip::BRIGHT_FLAG = "bright_flag";
const std::string Package::PackEquip::MOLDING_ATTACK_LEVEL = "molding_attack_level";
const std::string Package::PackEquip::MOLDING_DEFENCE_LEVEL = "molding_defence_level";
const std::string Package::PackEquip::MOLDING_HEALTH_LEVEL = "molding_health_level";
const std::string Package::PackEquip::MOLDING_ALL_LEVEL = "molding_all_level";
const std::string Package::PackEquip::MOLDING_ATTACK_SCHEDULE = "molding_attack_schedule";
const std::string Package::PackEquip::MOLDING_DEFENCE_SCHEDULE = "molding_defence_schedule";
const std::string Package::PackEquip::MOLDING_HEALTH_SCHEDULE = "molding_health_schedule";


const std::string Package::PackEquip::FASHION = "fashion";
const std::string Package::PackEquip::LUCK_VALUE = "luck_value";
const std::string Package::PackEquip::JEWEL_SETS = "jewel_sets";
const std::string Package::PackEquip::REFINE_SETS = "refine_sets";
const std::string Package::PackEquip::BASE_POLISH_ATTR = "base_polish_attr";
const std::string Package::PackEquip::EXTRAS_ATTR = "extras_attr";
const std::string Package::PackEquip::SPECIAL_JEWELS = "special_jewels";

const std::string Package::Fashion::USE_TYPE = "use_type";
const std::string Package::Fashion::USE_TICK = "use_tick";
const std::string Package::Fashion::EXPIRE_TICK = "expire_tick";
const std::string Package::Fashion::NOTIFED_MAP = "notifed_map";
const std::string Package::Fashion::IS_IN_USE = "is_in_use";

const std::string Package::PolishAttrDetial::ATTR_DETIAL_LOCK_INDEX = "lock_index";
const std::string Package::PolishAttrDetial::ATTR_DETIAL_ATTR_TYPE = "attr_type";
const std::string Package::PolishAttrDetial::ATTR_DETIAL_COLOR = "color";
const std::string Package::PolishAttrDetial::ATTR_DETIAL_CUR_VALUE = "cur_value";
const std::string Package::PolishAttrDetial::ATTR_DETIAL_MAX_VALUE = "max_value";

const std::string Package::BasePolishAttr::PROCESS_VALUE = "process_value";
const std::string Package::BasePolishAttr::CUR_POLISH_INFO = "cur_polish_info";
const std::string Package::BasePolishAttr::SINGLE_POLISH_INFO = "single_polish_info";
const std::string Package::BasePolishAttr::BATCH_POLISH_INFO = "batch_polish_info";

const std::string Package::JewelMapInfo::ID = "id";
const std::string Package::JewelMapInfo::BIND_STATUS = "bind_status";

/////////////////Escort/////////////////////
const std::string Escort::COLLECTION = "mmo.escort";
const std::string Escort::ID = "id";
const std::string Escort::CAR_INDEX = "car_index";
const std::string Escort::PROTECT_ID = "protect_id";
const std::string Escort::ESCORT_TYPE = "escort_type";
const std::string Escort::ESCORT_TIMES = "escort_times";
const std::string Escort::TOTAL_EXP = "total_exp";
const std::string Escort::START_TICK = "start_tick";
const std::string Escort::TILL = "till";
const std::string Escort::TARGET_LEVEL = "target_level";
const std::string Escort::PROTECT_LIST = "protect_list";
const std::string Escort::Protect_list::PROTECT_PLAYER = "protect_player";

/////////////////Skill//////////////////////
//{{{
const std::string Skill::COLLECTION = "mmo.skill";
const std::string Skill::ID = "id";

const std::string Skill::SKILL = "skill";
const std::string Skill::SSkill::SKILL_ID = "skill_id";
const std::string Skill::SSkill::LEVEL = "level";
const std::string Skill::SSkill::USED_TIMES = "used_times";

const std::string Skill::SSkill::USETICK = "usetick";
const std::string Skill::SSkill::Tick::SEC = "sec";
const std::string Skill::SSkill::Tick::USEC = "usec";

const std::string Skill::CUR_SCHEME = "cur_scheme";
const std::string Skill::SCHEME_LIST = "scheme_list";
const std::string Skill::CUR_RAMA = "cur_rama";
const std::string Skill::RAMA_LIST = "rama_list";
//}}}

/////////////////Status/////////////////////
//{{{
const std::string Status::COLLECTION = "mmo.status";
const std::string Status::ID = "id";

const std::string Status::STATUS = "status";
const std::string Status::SStatus::STATUS_TYPE = "status_type";
const std::string Status::SStatus::VALUE1 = "value1";
const std::string Status::SStatus::VALUE2 = "value2";
const std::string Status::SStatus::VALUE3 = "value3";
const std::string Status::SStatus::VALUE4 = "value4";
const std::string Status::SStatus::VALUE5 = "value5";

const std::string Status::SStatus::VIEW_TYPE = "view_type";
const std::string Status::SStatus::VIEW1 = "view1";
const std::string Status::SStatus::VIEW2 = "view2";
const std::string Status::SStatus::VIEW3 = "view3";

const std::string Status::SStatus::SKILL_ID = "skill_id";
const std::string Status::SStatus::SKILL_LEVEL = "skill_level";
const std::string Status::SStatus::ATTACK_ID = "attack_id";
const std::string Status::SStatus::ACCUMULATE = "accumulate";

const std::string Status::SStatus::CHECKTICK = "checktick";
const std::string Status::SStatus::INTERVAL = "interval";
const std::string Status::SStatus::LASTTICK = "lasttick";
const std::string Status::SStatus::Tick::SEC = "sec";
const std::string Status::SStatus::Tick::USEC = "usec";
//}}}

/////////////////Trade/////////////////////
const std::string DBTrade::COLLECTION = "mmo.trade";

const std::string DBTrade::ID = "id";
const std::string DBTrade::GOODS = "goods";
const std::string DBTrade::DBSTATE = "dbstate";

///////////////////DBShopItem/////////////////
const std::string DBShopItem::COLLECTION = "mmo.shop";

const std::string DBShopItem::SHOP_TYPE = "shop_type";
const std::string DBShopItem::CONTENT = "content";
const std::string DBShopItem::ITEM_ID = "item_id";
const std::string DBShopItem::ITEM_POS = "item_pos";
const std::string DBShopItem::ITEM_BIND = "item_bind";
const std::string DBShopItem::ITEM_TYPE = "item_type";
const std::string DBShopItem::MAX_ITEM = "max_item";
const std::string DBShopItem::MAX_TOTAL = "max_total";

const std::string DBShopItem::MONEY_TYPE = "money_type";
const std::string DBShopItem::SRC_PRICE = "src_price";
const std::string DBShopItem::CUR_PRICE = "cur_price";
const std::string DBShopItem::VIP_PRICE = "vip_price";

const std::string DBShopItem::START_TICK = "start_tick";
const std::string DBShopItem::END_TICK = "end_tick";

/////////////////DBBaseRole/////////////////////

const std::string DBBaseRole::INDEX = "id";
const std::string DBBaseRole::VIP = "vip";
const std::string DBBaseRole::SEX = "sex";
const std::string DBBaseRole::NAME = "name";
const std::string DBBaseRole::LVL = "lvl";
const std::string DBBaseRole::FORCE = "force";
const std::string DBBaseRole::CAREER = "career";

/////////////////DBLeague/////////////////////
const std::string DBLeague::COLLECTION = "mmo.league";

const std::string DBLeague::ID = "id";
const std::string DBLeague::LEAGUE_NAME = "name";
const std::string DBLeague::LEAGUE_INTRO = "intro";
const std::string DBLeague::LEAGUE_LVL = "lvl";
const std::string DBLeague::LEAGUE_RESOURCE = "resource";
const std::string DBLeague::REGION_RANK = "region_rank";
const std::string DBLeague::REGION_TICK = "region_tick";
const std::string DBLeague::REGION_LEADER_REWARD = "region_leader_reward";

const std::string DBLeague::CREATE_TICK = "create_tick";
const std::string DBLeague::LAST_LOGIN = "last_login";
const std::string DBLeague::LEADER_INDEX = "leader_index";
const std::string DBLeague::AUTO_ACCEPT = "auto_accept";
const std::string DBLeague::ACCEPT_FORCE = "accpt_force";
const std::string DBLeague::FLAG_LVL = "flag_lvl";
const std::string DBLeague::FLAG_EXP = "flag_exp";
const std::string DBLeague::LEAGUE_MEMBER = "member";
const std::string DBLeague::LEAGUE_APPLIER = "applier";
const std::string DBLeague::LEAGUE_LOG = "league_log";

const std::string DBLeague::Member::ROLE_INDEX = "id";
const std::string DBLeague::Member::JOIN_TICK = "tick";
const std::string DBLeague::Member::LEAGUE_POS= "pos";
const std::string DBLeague::Member::CUR_CONTRI = "cur_contri";
const std::string DBLeague::Member::TODAY_CONTRI = "today_contri";
const std::string DBLeague::Member::TOTAL_CONTRI = "total_contri";
const std::string DBLeague::Member::OFFLINE_CONTRI = "offline_contri";
const std::string DBLeague::Member::TODAY_RANK = "today_rank";
const std::string DBLeague::Member::TOTAL_RANK = "total_rank";
const std::string DBLeague::Member::LRF_BET_LEAGUE_ID  = "lrf_bet_league_id";

const std::string DBLeague::Log::LOG_TICK = "tick";
const std::string DBLeague::Log::LOG_CONTENT = "content";

const std::string DBLeague::LEAUGE_FB = "league_fb";
const std::string DBLeague::LeagueFB::OPEN_TICK = "tick";
const std::string DBLeague::LeagueFB::OPEN_MODE = "mode";
const std::string DBLeague::LeagueFB::OPEN_STATE = "open_state";
const std::string DBLeague::LeagueFB::FINISH_STATE = "finish_state";

const std::string DBLeague::LEAGUE_BOSS = "league_boss";
const std::string DBLeague::LeagueBoss::BOSS_INDEX = "boss_index";
const std::string DBLeague::LeagueBoss::BOSS_EXP = "boss_exp";
const std::string DBLeague::LeagueBoss::SUPER_SUMMON_ROLE = "super_summon_role";
const std::string DBLeague::LeagueBoss::RESET_TICK = "reset_tick";
const std::string DBLeague::LeagueBoss::NORMAL_SUMMON_TICK = "normal_summon_tick";
const std::string DBLeague::LeagueBoss::SUPER_SUMMON_TICK = "super_summon_tick";
const std::string DBLeague::LeagueBoss::NORMAL_SUMMON_TYPE = "normal_summon_type";
const std::string DBLeague::LeagueBoss::SUPER_SUMMON_TYPE = "super_summon_type";
const std::string DBLeague::LeagueBoss::NORMAL_DIE_TICK = "normal_die_tick";
const std::string DBLeague::LeagueBoss::SUPER_DIE_TICK = "super_die_tick";

const std::string DBLeague::LEAGUE_IMPEACH = "league_impeach";
const std::string DBLeague::LeagueImpeach::IMPEACH_ROLE = "impeach_role";
const std::string DBLeague::LeagueImpeach::IMPEACH_TICK = "impeach_tick";
const std::string DBLeague::VOTE_MAP = "vote_map";

const std::string DBLeague::LFB_PLAYER_SET = "lfb_player_set";
const std::string DBLeague::LFbPlayerSet::PLAYER_ID = "player_id";
const std::string DBLeague::LFbPlayerSet::TICK = "tick";
const std::string DBLeague::LFbPlayerSet::NAME = "name";
const std::string DBLeague::LFbPlayerSet::SEX = "sex";
const std::string DBLeague::LFbPlayerSet::WAVE = "wave";
const std::string DBLeague::LFbPlayerSet::LAST_WAVE = "last_wave";
const std::string DBLeague::LFbPlayerSet::CHEER = "cheer";
const std::string DBLeague::LFbPlayerSet::ENCOURAGE = "encourage";
const std::string DBLeague::LFbPlayerSet::BE_CHEER = "be_cheer";
const std::string DBLeague::LFbPlayerSet::BE_ENCOURAGE = "be_encourage";
const std::string DBLeague::LFbPlayerSet::RECORD_VEC = "record_vec";
const std::string DBLeague::LFbPlayerSet::RecordSet::ROLE_ID = "role_id";
const std::string DBLeague::LFbPlayerSet::RecordSet::TYPE = "type";
const std::string DBLeague::LFbPlayerSet::RecordSet::IS_ACTIVE = "is_active";
const std::string DBLeague::LFbPlayerSet::RecordSet::TIME = "time";
const std::string DBLeague::LFbPlayerSet::RecordSet::NAME = "name";

/////////////////DBLeaguer/////////////////////
const std::string DBLeaguer::COLLECTION = "mmo.leaguer";

const std::string DBLeaguer::ID = "id";
const std::string DBLeaguer::SHOP_BUY = "shop_buy";
const std::string DBLeaguer::LEAGUE_SKILL = "skill";
const std::string DBLeaguer::OPEN = "open";
const std::string DBLeaguer::DRAW_WELFARE = "draw_welfare";
const std::string DBLeaguer::WAND_DONATE = "wand_donate";
const std::string DBLeaguer::GOLD_DONATE = "gold_donate";
const std::string DBLeaguer::SEND_FLAG = "send_flag";
const std::string DBLeaguer::SIEGE_SHOP = "siege_shop";
const std::string DBLeaguer::SIEGE_SHOP_REFRESH = "siege_shop_refresh";
const std::string DBLeaguer::DAY_ADMIRE_TIMES = "day_admire_times_";

const std::string DBLeaguer::LEAVE_TYPE = "leave_type";
const std::string DBLeaguer::LEAVE_TICK = "leave_tick";
const std::string DBLeaguer::CUR_CONTRI	= "cur_contri";
const std::string DBLeaguer::SALARY_FLAG = "salary_flag";
const std::string DBLeaguer::FB_FLAG = "fb_flag";
const std::string DBLeaguer::STORE_TIMES = "store_times";
const std::string DBLeaguer::APPLY_LIST = "apply_list";
const std::string DBLeaguer::WAVE_REWARD_MAP = "wave_reward_map";
const std::string DBLeaguer::REGION_DRAW = "region_draw";

const std::string DBLeaguer::LV_REFRESH_TICK = "lv_refresh_tick";
const std::string DBLeaguer::LV_TASK_SET = "lv_task_set";
const std::string DBLeaguer::TID = "tid";
const std::string DBLeaguer::MID = "mid";
const std::string DBLeaguer::NEED = "need";
const std::string DBLeaguer::FINISH = "finish";

////////////////////////////////DBLeagueWar
const std::string DBLeagueWarInfo::COLLECTION = "mmo.league_war_db";
const std::string DBLeagueWarInfo::ID = "id";
const std::string DBLeagueWarInfo::TOTAL_NUM = "total_num";
const std::string DBLeagueWarInfo::LAST_TICK = "last_tick";

const std::string DBLeagueWarInfo::RANK_SET = "rank_set";
const std::string DBLeagueWarInfo::LeagueWarRank::ID = "id";
const std::string DBLeagueWarInfo::LeagueWarRank::LEAGUE_INDEX = "league_index";
const std::string DBLeagueWarInfo::LeagueWarRank::LEAGUE_NAME = "league_name";
DBField DBLeagueWarInfo::LeagueWarRank::LEADER = "leader";
DBField DBLeagueWarInfo::LeagueWarRank::FORCE = "force";
DBField DBLeagueWarInfo::LeagueWarRank::SCORE = "score";
DBField DBLeagueWarInfo::LeagueWarRank::FLAG_LVL = "flag_lvl";

////////////////////////////////DBLWTicker
DBField DBLWTicker::COLLECTION = "mmo.league_war";
DBField DBLWTicker::ID = "id";

DBField DBLWTicker::Arena::TIMEOUT_TICK = "timeout_tick";
DBField DBLWTicker::Arena::RE_RANK = "re_rank";

DBField DBLWTicker::Shop::LIMITED_SET = "limited_set";
DBField DBLWTicker::Shop::LIMITED_TOTAL_SET = "limited_total_set";

DBField DBLWTicker::CombineFirst::COMB_FIRST = "combine_first";

////////////////////////////////DBArenaRole

const std::string DBArenaRole::COLLECTION = "mmo.arena_role";

const std::string DBArenaRole::ID = "id";
const std::string DBArenaRole::NAME = "name";
const std::string DBArenaRole::SEX = "sex";
const std::string DBArenaRole::CAREER = "career";
const std::string DBArenaRole::FORCE = "force";
const std::string DBArenaRole::LEVEL = "level";
const std::string DBArenaRole::WING_LEVEL = "wing_level";
const std::string DBArenaRole::SOLIDER_LEVEL = "solider_level";
const std::string DBArenaRole::REWARD_LEVEL = "reward_level";

const std::string DBArenaRole::REFRESH_TICK = "refresh_tick";
const std::string DBArenaRole::LEFT_TIMES = "left_times";
const std::string DBArenaRole::BUY_TIMES = "buy_times";

const std::string DBArenaRole::RANK = "rank";
const std::string DBArenaRole::IS_SKIP = "is_skip";
const std::string DBArenaRole::IS_OVER_LIMIT = "is_over_limit";
const std::string DBArenaRole::OPEN_FLAG = "open_flag";
const std::string DBArenaRole::FIGHT_SET = "fight_set";

const std::string DBArenaRole::LAST_RANK = "last_rank";
const std::string DBArenaRole::ADD_ANIMA = "add_anima";
const std::string DBArenaRole::ADD_MONEY = "add_money";
const std::string DBArenaRole::CONTINUE_WIN = "continue_win";

const std::string DBArenaRole::HIS_RECORD = "his_record";
const std::string DBArenaRole::FIGHT_TICK = "fight_tick";
const std::string DBArenaRole::FIGHT_TYPE = "fight_type";
const std::string DBArenaRole::FIGHT_STATE = "fight_state";
const std::string DBArenaRole::FIGHT_NAME = "fight_name";
const std::string DBArenaRole::FIGHT_RANK = "fight_rank";
const std::string DBArenaRole::RANK_CHANGE = "rank_change";

////////////////////////////////
const std::string DBChatRecord::SRC_ROLE_ID="src_roleid";
const std::string DBChatRecord::DST_ROLE_ID="dst_roleid";
const std::string DBChatRecord::TIME="time";
const std::string DBChatRecord::TYPE = "type";
const std::string DBChatRecord::VOICE_ID = "voice_id";
const std::string DBChatRecord::VOICE_LEN = "voice_len";
const std::string DBChatRecord::CONTENT="content";

/////////////////////////DBChatLeague/////////////////////
const std::string DBChatLeague::COLLECTION = "mmo.chatleague";
const std::string DBChatLeague::LEAGUE_ID = "league_id";
const std::string DBChatLeague::CHAT_LIST = "chat_list";

///////////////DBOnline//////////////////////////
const std::string DBOnline::COLLECTION = "mmo.online";
const std::string DBOnline::ID = "id";
const std::string DBOnline::IS_ONLINE = "is_online";
const std::string DBOnline::SIGN_IN_TICK = "sign_in_tick";
const std::string DBOnline::SIGN_OUT_TICK = "sign_out_tick";
const std::string DBOnline::TOTAL_ONLINE = "total_online";
const std::string DBOnline::DAY_ONLINE = "day_online";
const std::string DBOnline::WEEK_ONLINE = "week_online";
const std::string DBOnline::MONTH_ONLINE = "month_online";
const std::string DBOnline::YEAR_ONLINE = "year_online";

const std::string DBOnline::DAY_REFRESH = "day_refresh";
const std::string DBOnline::WEEK_REFRESH = "week_refresh";
const std::string DBOnline::MONTH_REFRESH = "month_refresh";
const std::string DBOnline::YEAR_REFRESH = "year_refresh";

////////////////////////Socialer//////////////////////////
const std::string SocialerInfo::COLLECTION = "mmo.socialer";
const std::string SocialerInfo::ID = "id";
const std::string SocialerInfo::OPEN = "open";

const std::string SocialerInfo::APPLY_LIST = "apply_list";
const std::string SocialerInfo::ApplyInfo::FRIEND_ID = "friend_id";
const std::string SocialerInfo::ApplyInfo::FRIEND_NAME = "friend_name";
const std::string SocialerInfo::ApplyInfo::LEAGUE_ID = "league_id";
const std::string SocialerInfo::ApplyInfo::LEAGUE_NAME = "league_name";
const std::string SocialerInfo::ApplyInfo::LEVEL = "level";
const std::string SocialerInfo::ApplyInfo::SEX = "sex";
const std::string SocialerInfo::ApplyInfo::TICK = "tick";

const std::string SocialerInfo::FRIEND_LIST = "friend_list";
const std::string SocialerInfo::STRANGER_LIST = "stranger_list";
const std::string SocialerInfo::BLACK_LIST = "black_list";
const std::string SocialerInfo::ENEMY_LIST = "enemy_list";
const std::string SocialerInfo::NEARBY_LIST = "nearby_list";
const std::string SocialerInfo::TICK = "tick";
const std::string SocialerInfo::STRENGTH_LIST = "strength_list";
const std::string SocialerInfo::STRENGTH_GIVE_LIST = "strength_give_list";
const std::string SocialerInfo::STRENGTH = "strength_value";
const std::string SocialerInfo::GIVE_TIMES = "strength_give";
const std::string SocialerInfo::GET_TIMES = "strength_get";
const std::string SocialerInfo::VIP_BUY = "vip_buy";

////////////////////////DBFriendPair//////////////////
const std::string DBFriendPair::COLLECTION = "mmo.friend_pair";
const std::string DBFriendPair::ID = "id";
const std::string DBFriendPair::PAIR_SET = "pair_set";
const std::string DBFriendPair::PairSet::NUMBER = "number";
const std::string DBFriendPair::PairSet::PLAYER_ONE = "player_one";
const std::string DBFriendPair::PairSet::PLAYER_TWO = "player_two";

////////////////////////mail//////////////////////////
const std::string MailInfo::COLLECTION = "mmo.mail_info";
const std::string MailInfo::ID = "id";
const std::string MailInfo::SEND_MAIL_COUNT = "send_mail_count";
const std::string MailInfo::SEND_MAIL_COOL_TICK = "send_mail_cool_tick";
const std::string MailInfo::COUNT = "mail_count";
const std::string MailInfo::INFO = "info";
const std::string MailInfo::Info::MAIL_ID = "mail_id";
const std::string MailInfo::Info::TYPE = "mail_type";
const std::string MailInfo::Info::FORMAT = "mail_format";
const std::string MailInfo::Info::HAS_READ = "has_read";
const std::string MailInfo::Info::TIME = "time";
const std::string MailInfo::Info::READ_TICK = "read_tick";
const std::string MailInfo::Info::SENDER_NAME = "sender_name";
const std::string MailInfo::Info::TITLE = "title";
const std::string MailInfo::Info::CONTENT = "content";
const std::string MailInfo::Info::GOODS = "goods";
const std::string MailInfo::Info::SENDER_ID = "sender_id";
const std::string MailInfo::Info::LABEL = "label";
const std::string MailInfo::Info::EXPLOIT = "exploit";
const std::string MailInfo::Info::SENDER_VIP = "sender_vip";
const std::string MailInfo::Info::ST_SCORE = "st_score";
const std::string MailInfo::Info::RESOURCE = "resource";

////////////////////////mail offline//////////////////////////
const std::string MailOffline::COLLECTION = "mmo.mail_offline";
const std::string MailOffline::MAIL_ID = "mail_id";
const std::string MailOffline::ROLE_ID = "receiver_id";
const std::string MailOffline::SENDER_ID = "sender_id";
const std::string MailOffline::FLAG = "flag";
const std::string MailOffline::TYPE = "mail_type";
const std::string MailOffline::FORMAT = "mail_format";
const std::string MailOffline::TIME = "time";
const std::string MailOffline::SENDER_NAME = "sender_name";
const std::string MailOffline::TITLE = "title";
const std::string MailOffline::CONTENT = "content";
const std::string MailOffline::GOODS = "goods";
const std::string MailOffline::LABEL = "label";
const std::string MailOffline::EXPLOIT = "exploit";
const std::string MailOffline::SENDER_VIP = "sender_vip";
const std::string MailOffline::ST_SCORE = "st_score";

////////////market///////////////////////
const std::string DBMarket::COLLECTION = "mmo.market";
const std::string DBMarket::ID = "id";
const std::string DBMarket::LAST_GOLD = "last_gold";
const std::string DBMarket::LAST_COPPER = "last_copper";

////////////market item///////////////////////
const std::string DBMarketItem::COLLECTION = "mmo.market_item";
const std::string DBMarketItem::ID = "id";

const std::string DBMarketItem::ROLE_ID = "role_id";
const std::string DBMarketItem::ON_TICK = "on_tick";
const std::string DBMarketItem::OFF_TICK = "off_tick";
const std::string DBMarketItem::ITEM_OBJ = "item_obj";
const std::string DBMarketItem::ITEM_ID = "item_id";
const std::string DBMarketItem::ITEM_AMOUNT = "item_amount";
const std::string DBMarketItem::MONEY_TYPE = "money_type";
const std::string DBMarketItem::PRICE = "price";

////////////DBBeastSkill///////////////////////
const std::string DBBeastSkill::ID = "id";
const std::string DBBeastSkill::LOCK = "lock";
const std::string DBBeastSkill::LEVEL = "level";
const std::string DBBeastSkill::INDEX = "index";
const std::string DBBeastSkill::TRANSFORM = "transform";

////////////DBMaster///////////////////////
const std::string DBMaster::COLLECTION = "mmo.master";

const std::string DBMaster::ID = "id";
const std::string DBMaster::MASTER_NAME = "master_name";
const std::string DBMaster::SKILL_SET = "skill_set";
const std::string DBMaster::BEAST_SET = "beast_set";
const std::string DBMaster::PACK_SIZE = "pack_size";
const std::string DBMaster::CUR_BESAT = "cur_beast";
const std::string DBMaster::CUR_BEAST_SORT = "cur_beast_sort";
const std::string DBMaster::SAVE_BESAT = "save_beast";
const std::string DBMaster::LAST_BEAST = "last_beast";
const std::string DBMaster::LAST_BEAST_SORT = "last_beast_sort";
const std::string DBMaster::GEN_SKILL_LUCKY = "gen_skill_lucky";
const std::string DBMaster::GEN_SKILL_BOOK = "gen_skill_book";
const std::string DBMaster::IS_OPEN_SKILL = "is_open_skill";

const std::string DBMaster::MOUNT_SET = "mount_set";
const std::string DBMaster::MOUNT_GRADE = "mount_grade";
const std::string DBMaster::OPEN = "open";
const std::string DBMaster::BLESS = "bless";
const std::string DBMaster::FAIL_TIMES = "fail_times";
const std::string DBMaster::FINISH_BLESS = "finish_bless";
const std::string DBMaster::ON_MOUNT = "on_mount";
const std::string DBMaster::MOUNT_SHAPE = "mount_shape";
const std::string DBMaster::ABILITY = "ability";
const std::string DBMaster::GROWTH = "growth";
const std::string DBMaster::ACT_SHAPE = "act_shape";
const std::string DBMaster::SPOOL_LEVEL = "spool_level";
const std::string DBMaster::SKILL = "skill";

////////////////////////friendship value//////////////////////////
const std::string FriendshipValue::COLLECTION = "mmo.friendship_value";
const std::string FriendshipValue::FRIEND_ID = "friend_id";
const std::string FriendshipValue::FriendValueDetail = "friend_value_detail";
const std::string FriendshipValue::FriendValueDetail::FIRST_ID = "first_id";
const std::string FriendshipValue::FriendValueDetail::SECOND_ID = "second_id";
const std::string FriendshipValue::FriendValueDetail::FRIEND_VALUE = "friend_value";

////////////////////////////////////////////////////
const std::string DBTask::COLLECTION="mmo.task";
const std::string DBTask::ID = "id";
const std::string DBTask::SUBMITED_TASK = "submited_task";
const std::string DBTask::NOVICE_STEP = "novice_step";
const std::string DBTask::LATEST_MAIN_TASK = "latest_main_task";
const std::string DBTask::UIOPEN_STEP = "uiopen_steps";

const std::string DBTask::ROUTINE_REFRESH_TICK = "routine_refresh_tick";
const std::string DBTask::ROUTINE_TASK_INDEX = "routine_task_index";
const std::string DBTask::ROUTINE_CONSUME_HISTORY = "routine_consume_history";
const std::string DBTask::ROUTINE_TOTAL_NUM	= "routine_total_num";
const std::string DBTask::IS_FINISH_ALL_ROUTINE = "is_finish_all_routine";
const std::string DBTask::IS_ROUTINE_TASK = "is_routine_task";
const std::string DBTask::IS_SECOND_ROUTINE = "is_second_routine";

const std::string DBTask::OFFER_ROUTINE_TASK_INDEX = "offer_routine_task_index";
const std::string DBTask::OFFER_ROUTINE_TOTAL_NUM	= "offer_routine_total_num";
const std::string DBTask::IS_FINISH_ALL_OFFER_ROUTINE = "is_finish_all_offer_routine";
const std::string DBTask::IS_OFFER_ROUTINE_TASK = "is_offer_routine_task";
const std::string DBTask::IS_SECOND_OFFER_ROUTINE = "is_second_offer_routine";

const std::string DBTask::LEAGUE_ROUTINE_REFRESH_TICK = "league_routine_refresh_tick";
const std::string DBTask::LEAGUE_ROUTINE_TASK_INDEX = "league_routine_task_index";
const std::string DBTask::LEAGUE_ROUTINE_TOTAL_NUM	= "league_routine_total_num";
const std::string DBTask::IS_FINISH_ALL_LEAGUE_ROUTINE = "is_finish_all_league_routine";
const std::string DBTask::IS_LEAGUE_ROUTINE_TASK = "is_league_routine_task";
const std::string DBTask::IS_SECOND_LEAGUE_ROUTINE = "is_second_league_routine";

const std::string DBTask::LCONTRI_DAY_TICK = "lcontri_day_tick";
const std::string DBTask::LCONTRI_DAY = "lcontri_day";
const std::string DBTask::OPEN_UI = "open_ui";
const std::string DBTask::UI_VERSION = "ui_version";
const std::string DBTask::TRIAL_FRESH_TICK = "trial_fresh_tick";
const std::string DBTask::USED_TRIAL_TIMES = "used_trial_times";
const std::string DBTask::TRIAL_TASK_SET = "trial_task_set";
const std::string DBTask::FINISH_TASK = "finish_task";
const std::string DBTask::TASK = "task";

const std::string DBTask::Task::TASK_ID = "task_id";
const std::string DBTask::Task::GAME_TYPE = "game_type";
const std::string DBTask::Task::ACCEPT_TICK_SEC = "accept_tick_sec";
const std::string DBTask::Task::ACCEPT_TICK_USEC = "accept_tick_usec";
const std::string DBTask::Task::REFRESH_TICK_SEC = "refresh_tick_sec";
const std::string DBTask::Task::REFRESH_TICK_USEC = "refresh_tick_usec";
const std::string DBTask::Task::PREV_TASK = "prev_task";
const std::string DBTask::Task::POST_TASK = "post_task";
const std::string DBTask::Task::STATUS = "status";
const std::string DBTask::Task::LOGIC_TYPE = "logic_type";
const std::string DBTask::Task::TASK_STAR = "task_star";
const std::string DBTask::Task::FAST_FINISH_RATE = "fast_finish_rate";
const std::string DBTask::Task::FRESH_STAR_TIMES = "fresh_star_times";

const std::string DBTask::Task::COND_LIST = "cond_list";
const std::string DBTask::Task::CondList::TYPE = "type";
const std::string DBTask::Task::CondList::CURRENT_VALUE = "current_value";
const std::string DBTask::Task::CondList::COND_INDEX = "cond_index";
const std::string DBTask::Task::CondList::ID_LIST_INDEX = "id_list_index";
const std::string DBTask::Task::CondList::COND_ID = "cond_id";
const std::string DBTask::Task::CondList::FINAL_VALUE = "final_value";
const std::string DBTask::Task::CondList::KILL_TYPE = "kill_type";
const std::string DBTask::Task::CondList::RANGE_LEVEL = "range_level";

//////////////////////MallActivityInfo
const std::string MallActivityInfo::COLLECTION = "mmo.mall_activity";
const std::string MallActivityInfo::ID = "id";
const std::string MallActivityInfo::OPEN = "open";
const std::string MallActivityInfo::REFRESH_TYPE = "refresh_type";
const std::string MallActivityInfo::REFRESH_TICK = "refresh_tick";
const std::string MallActivityInfo::LIMIT_TYPE = "limit_type";
const std::string MallActivityInfo::NAME = "name";
const std::string MallActivityInfo::MEMO = "memo";
const std::string MallActivityInfo::TOTAL_LIMIT_AMOUNT = "total_limit_amount";
const std::string MallActivityInfo::SINGLE_LIMIT_AMOUNT = "single_limit_amount";
const std::string MallActivityInfo::GOODS_ID = "goods_id";
const std::string MallActivityInfo::GOODS_AMOUNT = "goods_amount";
const std::string MallActivityInfo::IS_CONFIG = "is_config";
const std::string MallActivityInfo::VISIBLE_START_TICK = "visible_start_tick";
const std::string MallActivityInfo::VISIBLE_END_TICK = "visible_end_tick";
const std::string MallActivityInfo::ACTIVITY_START_TICK = "activity_start_tick";
const std::string MallActivityInfo::ACTIVITY_END_TICK = "activity_end_tick";
const std::string MallActivityInfo::TOTAL_BUY_RECORD = "total_buy_record";
const std::string MallActivityInfo::SINGLE_BUY_RECORD = "single_buy_record";
const std::string MallActivityInfo::PLAYER_RECORD = "player_record";

//////////////////////LabelInfo
const std::string LabelInfo::COLLECTION = "mmo.label";
const std::string LabelInfo::ID = "id";
const std::string LabelInfo::LABEL_ID = "label_id";
const std::string LabelInfo::PRE_LABEL_ID = "pre_label_id";

const std::string LabelInfo::WAR_LABEL = "war_label";
const std::string LabelInfo::WAR_LABEL_TICK = "war_tick";
const std::string LabelInfo::MATRIAL_LABEL = "matrial_label";
const std::string LabelInfo::MARTIAL_LABEL_TICK = "martial_tick";

const std::string LabelInfo::EXPIRE_TICK = "expire_tick";
const std::string LabelInfo::PERMANT_LIST = "permant_list";
const std::string LabelInfo::NEW_LIST = "new_list";
const std::string LabelInfo::LIMIT_TIME_LIST = "limit_time_list";
const std::string LabelInfo::EXPIRE_UNSHOWN_LIST = "expire_unshown_list";

//////////////////////activity_tips_system
const std::string DBActivityTipsInfo::COLLECTION = "mmo.activity_notification";
const std::string DBActivityTipsInfo::ID = "id";
const std::string DBActivityTipsInfo::NOTIFY_REC = "notify_record";
const std::string DBActivityTipsInfo::NotifyTipsRec::ACTIVITY_ID = "activity_id";
const std::string DBActivityTipsInfo::NotifyTipsRec::START_TICK = "start_tick";
const std::string DBActivityTipsInfo::NotifyTipsRec::END_TICK = "end_tick";
const std::string DBActivityTipsInfo::NotifyTipsRec::REFRESH_TICK = "refresh_tick";
const std::string DBActivityTipsInfo::NotifyTipsRec::FINISH_COUNT = "finish_count";
const std::string DBActivityTipsInfo::NotifyTipsRec::IS_TOUCH = "is_touch";

/////////////////////script
const std::string DBScript::COLLECTION = "mmo.script";
const std::string DBScript::ID = "id";
const std::string DBScript::SCRIPT_ID = "script_id";
const std::string DBScript::SCRIPT_SORT = "script_sort";
const std::string DBScript::PREV_SCENE = "prev_scene";
const std::string DBScript::PREV_PIXEL_X = "prev_pixel_x";
const std::string DBScript::PREV_PIXEL_Y = "prev_pixel_y";
const std::string DBScript::PREV_BLOOD = "prev_blood";
const std::string DBScript::PREV_MAGIC = "prev_magic";
const std::string DBScript::TRVL_TOTAL_PASS = "trvl_total_pass";
const std::string DBScript::FIRST_SCRIPT = "first_script";
const std::string DBScript::SKILL_ID = "skill_id";

const std::string DBScript::TYPE_RECORD = "type_record";
const std::string DBScript::TypeRecord::SCRIPT_TYPE = "script_type";
const std::string DBScript::TypeRecord::MAX_SCRIPT_SORT = "max_script_sort";
const std::string DBScript::TypeRecord::PASS_WAVE = "pass_wave";
const std::string DBScript::TypeRecord::PASS_CHAPTER = "pass_chapter";
const std::string DBScript::TypeRecord::START_WAVE = "start_wave";
const std::string DBScript::TypeRecord::START_CHAPTER = "start_chapter";
const std::string DBScript::TypeRecord::NOTIFY_CHAPTER = "notify_chapter";
const std::string DBScript::TypeRecord::IS_SWEEP = "is_sweep";
const std::string DBScript::TypeRecord::USED_TIMES_TICK_SEC = "used_times_tick_sec";
const std::string DBScript::TypeRecord::USED_TIMES_TICK_USEC = "used_times_tick_usec";

const std::string DBScript::SCRIPT_WAVE_RECORD = "script_wave_record";
const std::string DBScript::ScriptWaveInfo::SCRIPT_WAVE_ID = "script_wave_id";
const std::string DBScript::ScriptWaveInfo::IS_GET = "is_get";

const std::string DBScript::RECORD = "record";
const std::string DBScript::Record::SCRIPT_SORT = "script_sort";
const std::string DBScript::Record::USED_TIMES = "used_times";
const std::string DBScript::Record::BUY_TIMES = "buy_times";
const std::string DBScript::Record::COUPLE_BUY_TIME = "couple_buy_times";
const std::string DBScript::Record::USED_TIMES_TICK_SEC = "used_times_tick_sec";
const std::string DBScript::Record::USED_TIMES_TICK_USEC = "used_times_tick_usec";
const std::string DBScript::Record::ENTER_SCRIPT_TICK = "enter_script_tick";
const std::string DBScript::Record::PROGRESS_ID = "progress_id";
const std::string DBScript::Record::BEST_USE_TICK = "best_use_tick";
const std::string DBScript::Record::IS_FIRST_PASS = "is_first_pass";
const std::string DBScript::Record::DAY_PASS_TIMES = "day_pass_times";
const std::string DBScript::Record::IS_EVEN_ENTER = "is_even_enter";
const std::string DBScript::Record::PROTECT_BEAST_INDEX = "protect_beast_index";

const std::string DBScript::PIECE_RECORD = "piece_record";
const std::string DBScript::PieceRecord::PASS_PIECE = "pass_piece";
const std::string DBScript::PieceRecord::PASS_CHAPTER = "pass_chapter";
const std::string DBScript::PieceRecord::PIECE_STAR_AWARD = "piece_star_award";

const std::string DBScript::PieceRecord::PIECE_CHAPTER = "piece_chapter";
const std::string DBScript::PieceRecord::PieceChapter::CHAPTER_KEY = "chapter_key";
const std::string DBScript::PieceRecord::PieceChapter::USED_SEC = "used_sec";
const std::string DBScript::PieceRecord::PieceChapter::USED_TIMES = "used_times";
const std::string DBScript::PieceRecord::PieceChapter::TODAY_PASS_FLAG = "today_pass_flag";
const std::string DBScript::PieceRecord::PieceChapter::AWARD_FLAG = "award_flag";
const std::string DBScript::PieceRecord::PieceChapter::CHAPTER_ITEM = "chapter_item";

const std::string DBScript::LEGEND_TOP_INFO = "legend_top_info";
const std::string DBScript::SWORD_TOP_INFO = "sword_top_info";
const std::string DBScript::LegendTopInfo::PASS_FLOOR = "pass_floor";
const std::string DBScript::LegendTopInfo::TODAY_RANK = "today_rank";
const std::string DBScript::LegendTopInfo::IS_SWEEP = "is_sweep";

const std::string DBScript::LegendTopInfo::FLOOR_INFO = "floor_info";
const std::string DBScript::LegendTopInfo::FloorInfo::FLOOR_ID = "floor_id";
const std::string DBScript::LegendTopInfo::FloorInfo::PASS_TICK = "pass_tick";
const std::string DBScript::LegendTopInfo::FloorInfo::TODAY_PASS_FLAG = "today_pass_flag";

/////////////////////sword_pool
const std::string DBSwordPool::COLLECTION = "mmo.sword_pool";
const std::string DBSwordPool::ID = "id";
const std::string DBSwordPool::LEVEL = "level";
const std::string DBSwordPool::EXP = "exp";
const std::string DBSwordPool::OPEN = "open";
const std::string DBSwordPool::STYLE_LV = "style_lv";
const std::string DBSwordPool::REFRESH_TICK = "refresh_tick";

const std::string DBSwordPool::LAST_POOL_TASK_INFO = "last_pool_task_info";
const std::string DBSwordPool::TODAY_POOL_TASK_INFO = "today_pool_task_info";
const std::string DBSwordPool::PoolTaskInfo::TASK_ID = "task_id";
const std::string DBSwordPool::PoolTaskInfo::TOTAL_NUM = "total_num";
const std::string DBSwordPool::PoolTaskInfo::LEFT_NUM = "left_num";

/////////////////////DBHiddenTreasure
const std::string DBHiddenTreasure::COLLECTION = "mmo.hidden_treasure";
const std::string DBHiddenTreasure::ID = "id";
const std::string DBHiddenTreasure::DAY = "day";
const std::string DBHiddenTreasure::OPEN = "open";
const std::string DBHiddenTreasure::GET_STATUS = "get_status";
const std::string DBHiddenTreasure::REFRESH_TICK = "refresh_tick";
const std::string DBHiddenTreasure::BUY_MAP = "buy_map";

/////////////////////DBFashion
const std::string DBFashion::COLLECTION = "mmo.fashion";
const std::string DBFashion::ID = "id";
const std::string DBFashion::LEVEL = "level";
const std::string DBFashion::EXP = "exp";
const std::string DBFashion::OPEN = "open";
const std::string DBFashion::SELECT_ID = "select_id";
const std::string DBFashion::SEL_COLOR_ID = "sel_color_id";
const std::string DBFashion::SEND_SET = "send_set";

const std::string DBFashion::FASHION_SET = "fashion_set";
const std::string DBFashion::FashionInfo::FASHION_ID = "fashion_id";
const std::string DBFashion::FashionInfo::COLOR_ID = "color_id";
const std::string DBFashion::FashionInfo::ACTIVE_TYPE = "active_type";
const std::string DBFashion::FashionInfo::IS_PERMANENT = "is_permanent";
const std::string DBFashion::FashionInfo::ACTIVE_TICK = "active_tick";
const std::string DBFashion::FashionInfo::END_TICK = "end_tick";
const std::string DBFashion::FashionInfo::COLOR_SET = "color_set";

///////////////////////DBTransfer
const std::string DBTransfer::COLLECTION = "mmo.transfer";
const std::string DBTransfer::ID = "id";
const std::string DBTransfer::LEVEL = "level";
const std::string DBTransfer::EXP = "exp";
const std::string DBTransfer::OPEN = "open";
const std::string DBTransfer::STAGE = "stage";
const std::string DBTransfer::TRANSFER_TICK = "transfer_tick";
const std::string DBTransfer::LAST = "last";
const std::string DBTransfer::ACTIVE_ID = "active_id";
const std::string DBTransfer::OPEN_REWARD = "open_reward";
const std::string DBTransfer::GOLD_TIMES = "gold_times";
const std::string DBTransfer::REFRESH_TICK = "refresh_tick";

const std::string DBTransfer::TRANSFER_SET = "transfer_set";
const std::string DBTransfer::TransferSet::TRANSFER_ID = "transfer_id";
const std::string DBTransfer::TransferSet::TRANSFER_LV = "transfer_lv";
const std::string DBTransfer::TransferSet::IS_PERMANENT = "is_permanent";
const std::string DBTransfer::TransferSet::IS_ACTIVE = "is_active";
const std::string DBTransfer::TransferSet::ACTIVE_TICK = "active_tick";
const std::string DBTransfer::TransferSet::END_TICK = "end_tick";
const std::string DBTransfer::TransferSet::TRANSFER_SKILL = "transfer_skill";
const std::string DBTransfer::TransferSet::SKILL_SET = "skill_set";

///////////////////////DBAchieve
const std::string DBAchieve::COLLECTION = "mmo.achieve";
const std::string DBAchieve::ID = "id";
const std::string DBAchieve::ACH_INDEX = "ach_index";
const std::string DBAchieve::BASE_TYPE = "base_type";
const std::string DBAchieve::CHILD_TYPE = "child_type";
const std::string DBAchieve::COMPARE = "compare";
const std::string DBAchieve::SORT = "sort";
const std::string DBAchieve::RED_EVENT = "red_point";

const std::string DBAchieve::DETAIL = "detail";
const std::string DBAchieve::AchieveDetail::ACHIEVE_ID = "achieve_id";
const std::string DBAchieve::AchieveDetail::NEED_AMOUNT = "need_amount";
const std::string DBAchieve::AchieveDetail::SORT = "sort";
const std::string DBAchieve::AchieveDetail::ACHIEVE_TYPE = "ach_type";
const std::string DBAchieve::AchieveDetail::NUMBER_TYPE = "number_type";
const std::string DBAchieve::AchieveDetail::REWARD_ID = "reward_id";
const std::string DBAchieve::AchieveDetail::ACH_AMOUNT = "ach_amount";


/////////////////////achievement
const std::string AchieveInfo::COLLECTION = "mmo.achievement";
const std::string AchieveInfo::ID = "id";
const std::string AchieveInfo::ACHIEVE_LEVEL = "achieve_level";
const std::string AchieveInfo::POINT_MAP = "point_map";

const std::string AchieveInfo::ACHIEVE_LIST = "achieve_list";
const std::string AchieveInfo::AchieveList::ACHIEVE_ID = "achieve_id";
const std::string AchieveInfo::AchieveList::ACH_INDEX = "ach_index";
const std::string AchieveInfo::AchieveList::FINISH_TICK = "finish_tick";
const std::string AchieveInfo::AchieveList::GET_STATUS = "get_status";
const std::string AchieveInfo::AchieveList::FINISH_NUM = "finish_num";
const std::string AchieveInfo::AchieveList::SPECIAL_VALUE = "special_value";
//const std::string AchieveInfo::DRAWED_LIST = "drawed_list";
//const std::string AchieveInfo::UNCHECK_LIST = "uncheck_list";
//const std::string AchieveInfo::ACHIEVE_PROP = "achieve_prop";
//const std::string AchieveInfo::OTHER_ACHIEVE_RECORD = "other_achieve_record";
//const std::string AchieveInfo::OTHER_CUR_VALUE = "other_cur_value";
//const std::string AchieveInfo::TYPE = "type";
//const std::string AchieveInfo::SUB_TYPE = "sub_type";
//const std::string AchieveInfo::IS_FINISH = "is_finish";
//const std::string AchieveInfo::REWARD_STATUS = "reward_status";
//const std::string AchieveInfo::PROP_ID = "prop_id";
//const std::string AchieveInfo::PROP_VALUE = "prop_value";

////////////////////equip_smelt
const std::string DBEquipSmelt::COLLECTION = "mmo.equip_smelt";
const std::string DBEquipSmelt::ID = "id";
const std::string DBEquipSmelt::SMELT_LEVEL = "smelt_level";
const std::string DBEquipSmelt::SMELT_EXP = "smelt_exp";
const std::string DBEquipSmelt::RECOMMEND = "recommend";
const std::string DBEquipSmelt::OPEN = "open";

////////////////////treasuers
const std::string DBTreasures::COLLECTION = "mmo.treasures_info";
const std::string DBTreasures::ID = "id";
const std::string DBTreasures::RESET_TICK = "reset_tick";
const std::string DBTreasures::RESET_TIMES = "reset_times";
const std::string DBTreasures::FREE_TIMES = "free_times";
const std::string DBTreasures::GAME_INDEX = "game_index";
const std::string DBTreasures::GAME_LENGTH = "game_length";
const std::string DBTreasures::ITEM_LIST = "item_list";
const std::string DBTreasures::Item_list::ID = "id";
const std::string DBTreasures::Item_list::AMOUNT = "amount";
const std::string DBTreasures::Item_list::BIND = "bind";
const std::string DBTreasures::Item_list::INDEX = "index";

////////////////////offline_rewards
const std::string DBOfflineRewards::COLLECTION = "mmo.offline_rewards";
const std::string DBOfflineRewards::ID = "id";
const std::string DBOfflineRewards::OFFLINE_SUM = "offline_sum";
const std::string DBOfflineRewards::LONGEST_TIME = "longest_time";
const std::string DBOfflineRewards::LOGOUT_TIME = "logout_time";
const std::string DBOfflineRewards::RECEIVED_MARK = "received_mark";

////////////////////online_rewards
const std::string DBOnlineRewards::COLLECTION = "mmo.online_rewards";
const std::string DBOnlineRewards::ID = "id";
const std::string DBOnlineRewards::STAGE = "stage";
const std::string DBOnlineRewards::PRE_STAGE = "pre_stage";
const std::string DBOnlineRewards::LOGIN_TIME = "login_time";
const std::string DBOnlineRewards::RECEIVED_MARK = "received_mark";
const std::string DBOnlineRewards::ONLINE_SUM = "online_sum";
const std::string DBOnlineRewards::AWARD_LIST = "award_list";
const std::string DBOnlineRewards::Award_List::AWARD_NUM = "award_num";

////////////////////collect_chests
const std::string DBCollectChests::COLLECTION = "mmo.collect_chests";
const std::string DBCollectChests::ID = "id";
const std::string DBCollectChests::COLLECT_NUM = "collect_num";
const std::string DBCollectChests::NEXT_TICK = "next_tick";

////////////////// DBIllus illustration
const std::string DBIllus::COLLECTION = "mmo.illus";
const std::string DBIllus::ILLUS_GROUP= "illus_group";
const std::string DBIllus::ILLUS = "illus_i";
const std::string DBIllus::ID = "id";
const std::string DBIllus::OPEN = "open";

const std::string DBIllus::Illus_group::GROUP_NUM = "group_num";
const std::string DBIllus::Illus_group::GROUP_ID = "group_id";
const std::string DBIllus::Illus_group::GROUP_TYPE = "group_type";

const std::string DBIllus::Illus::ILLUS_NUM = "illus_num";
const std::string DBIllus::Illus::ILLUS_ID = "illus_id";
const std::string DBIllus::Illus::ILLUS_LEVEL = "illus_level";


////////////////// DBWelfare welfare 福利
const std::string DBWelfare::COLLECTION = "mmo.welfare";
const std::string DBWelfare::ID = "id";
const std::string DBWelfare::CHECK_IN = "check_in";

const std::string DBWelfare::CheckIn::AWARD_INDEX = "award_index";
const std::string DBWelfare::CheckIn::CHECK_IN_POINT = "check_in_point";
const std::string DBWelfare::CheckIn::LAST_TIME = "last_time";
const std::string DBWelfare::CheckIn::SHOW_POINT = "show_point";

const std::string DBWelfare::CheckIn::CHECK_TOTAL_INDEX = "check_total_index";
const std::string DBWelfare::CheckIn::CHARGE_MONEY = "charge_money";
const std::string DBWelfare::CheckIn::TOTAL_LAST_TIME = "total_last_time";
const std::string DBWelfare::CheckIn::TOTAL_POPUP = "total_popup";

const std::string DBWelfare::LIVENESS = "liveness";
const std::string DBWelfare::Liveness::LIVENESS_POINT = "liveness_point";
const std::string DBWelfare::Liveness::RECEIVED_AWARD_INDEX = "received_award_index";

const std::string DBWelfare::Liveness::FINISH_RECORD = "finish_record";
const std::string DBWelfare::Liveness::FinishRecord::ID = "id";

const std::string DBWelfare::Liveness::FinishRecord::RECORDS = "records";
const std::string DBWelfare::Liveness::FinishRecord::Record::NUM = "num";
const std::string DBWelfare::Liveness::FinishRecord::Record::IS_PASSED = "is_passed";

const std::string DBWelfare::Liveness::FINISH_TIMES = "finish_times";
const std::string DBWelfare::Liveness::FinishTimes::ID = "id";
const std::string DBWelfare::Liveness::FinishTimes::TIMES = "times";

const std::string DBWelfare::Liveness::LAST_TIME = "last_time";

const std::string DBWelfare::ONCE_REWARDS = "once_rewards";
const std::string DBWelfare::OnceRewards::UPDATE_RES_FLAG = "update_res_flag";

const std::string DBWelfare::DAILY_LIVENESS = "daily_liveness";
const std::string DBWelfare::DailyLiveness::RECORDS = "records";
const std::string DBWelfare::DailyLiveness::Record::ID = "id";
const std::string DBWelfare::DailyLiveness::Record::TYPE = "type";
const std::string DBWelfare::DailyLiveness::Record::VALUE = "value";
const std::string DBWelfare::DailyLiveness::Record::REWARD_VALUE = "reward_value";
const std::string DBWelfare::DailyLiveness::Record::IS_OVER = "is_over";

////////////////// exp_restore 经验找回
const std::string DBExpRestore::COLLECTION = "mmo.exp_restore";
const std::string DBExpRestore::ID = "id";
const std::string DBExpRestore::CHECK_TIMESTAMP = "check_time";
const std::string DBExpRestore::VIP_TYPE_REC = "vip_type_rec";
const std::string DBExpRestore::VIP_START_TIME = "vip_start_time";
const std::string DBExpRestore::VIP_EXPRIED_TIME = "vip_expried_time";

const std::string DBExpRestore::PRE_ACT_MAP = "pre_act_map";
const std::string DBExpRestore::PreActMap::ACT_ID = "act_id";
const std::string DBExpRestore::PreActMap::EXT_TYPE = "pre_ext_type";
const std::string DBExpRestore::PreActMap::TIMES = "pre_times";

const std::string DBExpRestore::NOW_ACT_MAP = "now_act_map";
const std::string DBExpRestore::NowActMap::ACT_ID = "act_id";
const std::string DBExpRestore::NowActMap::EXT_TYPE = "now_ext_type";
const std::string DBExpRestore::NowActMap::TIMES = "now_times";

const std::string DBExpRestore::LEVEL_RECORD = "level_record";
const std::string DBExpRestore::LevelRecord::REC_TIMESTAMP = "rec_timestamp";
const std::string DBExpRestore::LevelRecord::LEVEL = "level";

const std::string DBExpRestore::VIP_RECORD = "vip_record";
const std::string DBExpRestore::VipRecord::REC_TIMESTAMP= "rec_timestamp";
const std::string DBExpRestore::VipRecord::VIP_TYPE = "vip_type";

const std::string DBExpRestore::STORAGE_RECORD= "storage_record";
const std::string DBExpRestore::ActivityExpRestore::STORAGE_ID = "storage_id";
const std::string DBExpRestore::ActivityExpRestore::REC_TIMESTAMP = "rec_timestamp";
const std::string DBExpRestore::ActivityExpRestore::FINISH_COUNT = "finish_count";
const std::string DBExpRestore::ActivityExpRestore::STORAGE_VALID = "storage_valid";

const std::string DBExpRestore::STORAGE_STAGE_INFO = "storage_stage_info";
const std::string DBExpRestore::StorageStageInfo::STORAGE_ID = "storage_id";
const std::string DBExpRestore::StorageStageInfo::TIMESTAMP_STAGE = "timestamp_stage";
const std::string DBExpRestore::StorageStageInfo::TimestampStage::REC_TIMESTAMP = "rec_timestamp";
const std::string DBExpRestore::StorageStageInfo::TimestampStage::STAGE = "stage";


//////////////////// DBOfflineData
const std::string DBOfflineData::COLLECTION = "mmo.offline_data";
const std::string DBOfflineData::ID = "id";

//////////////////// DBRank
const std::string DBRank::COLLECTION = "mmo.rank_pannel";
const std::string DBRank::ID = "rank_type";
//const std::string DBRank::RANK_DETAIL = "rank_detail";
const std::string DBRank::TOTAL_VALUE = "total_value";
const std::string DBRank::JANE_ITEM = "jane_item";
const std::string DBRank::RANK_RECORD = "rank_record";
const std::string DBRank::RankDetail::RANK_TYPE = "rank_type";
const std::string DBRank::RankDetail::RANK_NAME = "rank_name";
const std::string DBRank::RankDetail::LAST_REFRESH_TICK = "last_refresh_tick";

const std::string DBRank::RankDetail::CUR_RANK = "cur_rank";
const std::string DBRank::RankDetail::LAST_RANK = "last_rank";
const std::string DBRank::RankDetail::RANK_VALUE = "rank_value";
const std::string DBRank::RankDetail::ACHIEVE_TICK = "achieve_tick";
const std::string DBRank::RankDetail::ADDITIONAL_ID = "additional_id";
const std::string DBRank::RankDetail::PLAYER_ID = "player_id";
const std::string DBRank::RankDetail::DISPLAYER_CONTENT = "displayer_content";
const std::string DBRank::RankDetail::LEAGUE_NAME = "league_name";
const std::string DBRank::RankDetail::ADDIITION_ID = "addiition_id";
const std::string DBRank::RankDetail::RANK_FORCE = "rank_force";
const std::string DBRank::RankDetail::EXT_VALUE = "ext_value";
const std::string DBRank::RankDetail::VIP_TYPE = "vip_type";
const std::string DBRank::RankDetail::WORSHIP_NUM = "worship_num";
const std::string DBRank::RankDetail::IS_WORSHIP = "is_worship";

//////////////////// DBRanker
const std::string DBRanker::COLLECTION = "mmo.ranker";
const std::string DBRanker::ID = "id";
const std::string DBRanker::NAME = "name";
const std::string DBRanker::EXPERIENCE = "experience";
const std::string DBRanker::FIGHT_LEVEL = "fight_level";
const std::string DBRanker::FIGHT_FORCE = "fight_force";
const std::string DBRanker::KILL_VALUE = "killing_value";
const std::string DBRanker::KILL_NUM = "kill_num";
const std::string DBRanker::KILL_NORMAL = "kill_normal";
const std::string DBRanker::WEEK_TICK = "reset_week_tick";
const std::string DBRanker::KILL_EVIL = "kill_evil";
const std::string DBRanker::LABEL = "label";
const std::string DBRanker::WEAPON = "weapon";
const std::string DBRanker::CLOTHES = "clothes";
const std::string DBRanker::FASHION_WEAPON = "fashion_weapon";
const std::string DBRanker::FASHION_CLOTHES = "fashion_clothes";
const std::string DBRanker::VIP_STATUS = "vip_status";
const std::string DBRanker::SEX = "sex";
const std::string DBRanker::CAREER = "career";
const std::string DBRanker::LEAGUE_ID = "league_id";
const std::string DBRanker::LEAGUE_NAME = "league_name";
const std::string DBRanker::VIP_TYPE = "vip_type";
const std::string DBRanker::WORSHIP_NUM = "worship_num";
const std::string DBRanker::IS_WORSHIP = "is_worship";

const std::string DBRanker::FUN_MOUNT_GRADE = "fun_mount_grade";
const std::string DBRanker::FUN_GOD_SOLIDER_GRADE = "fun_god_solider_grade";
const std::string DBRanker::FUN_MAGIC_EQUIP_GRADE = "fun_magic_equip_grade";
const std::string DBRanker::FUN_XIAN_WING_GRADE = "fun_xian_wing_grade";
const std::string DBRanker::FUN_LING_BEAST_GRADE = "fun_ling_beast_grade";
const std::string DBRanker::FUN_BEAST_EQUIP_GRADE = "fun_beast_equip_grade";
const std::string DBRanker::FUN_BEAST_MOUNT_GRADE = "fun_beast_mount_grade";
const std::string DBRanker::FUN_BEAST_WING_GRADE = "fun_beast_wing_grade";
const std::string DBRanker::FUN_BEAST_MAO_GRADE = "fun_beast_mao_grade";
const std::string DBRanker::FUN_TIAN_GANG_GRADE = "fun_tian_gang_grade";

const std::string DBRanker::FUN_MOUNT_FORCE = "fun_mount_force";
const std::string DBRanker::FUN_GOD_SOLIDER_FORCE = "fun_god_solider_force";
const std::string DBRanker::FUN_MAGIC_EQUIP_FORCE = "fun_magic_equip_force";
const std::string DBRanker::FUN_XIAN_WING_FORCE = "fun_xian_wing_force";
const std::string DBRanker::FUN_LING_BEAST_FORCE = "fun_ling_beast_force";
const std::string DBRanker::FUN_BEAST_EQUIP_FORCE = "fun_beast_equip_force";
const std::string DBRanker::FUN_BEAST_MOUNT_FORCE = "fun_beast_mount_force";
const std::string DBRanker::FUN_BEAST_WING_FORCE = "fun_beast_wing_force";
const std::string DBRanker::FUN_BEAST_MAO_FORCE = "fun_beast_mao_force";
const std::string DBRanker::FUN_TIAN_GANG_FORCE = "fun_tian_gang_force";

const std::string DBRanker::MOUNT_SET = "mount_set";
const std::string DBRanker::OFF_MOUNT_TYPE = "off_mount_type";
const std::string DBRanker::OFF_MOUNT_OPEN = "off_mount_open";
const std::string DBRanker::OFF_MOUNT_GRADE = "off_mount_grade";
const std::string DBRanker::OFF_MOUNT_SHAPE = "off_mount_shape";
const std::string DBRanker::OFF_ACT_SHAPE = "off_act_shape";
const std::string DBRanker::OFF_MOUNT_SKILL = "off_mount_skill";
const std::string DBRanker::OFF_MOUNT_FORCE = "off_mount_force";
const std::string DBRanker::OFF_MOUNT_PROP = "off_mount_prop";
const std::string DBRanker::OFF_MOUNT_TEMP = "off_mount_temp";

const std::string DBRanker::ATTACK = "attack";
const std::string DBRanker::DEFENCE = "defence";
const std::string DBRanker::MAX_BLOOD = "max_blood";
const std::string DBRanker::ATTACK_UPPER = "attack_upper";
const std::string DBRanker::ATTACK_LOWER = "attack_lower";
const std::string DBRanker::DEFENCE_UPPER = "defence_upper";
const std::string DBRanker::DEFENCE_LOWER = "defence_lower";
const std::string DBRanker::CRIT = "crit";
const std::string DBRanker::TOUGHNESS = "toughness";
const std::string DBRanker::HIT = "hit";
const std::string DBRanker::DODGE = "dodge";
const std::string DBRanker::LUCKY = "lucky";
const std::string DBRanker::CRIT_HIT = "crit_hit";
const std::string DBRanker::DAMAGE = "damage";
const std::string DBRanker::REDUCTION = "reduction";
const std::string DBRanker::GLAMOUR = "glamour";

const std::string DBRanker::MOUNT_GRADE = "mount_grade";
const std::string DBRanker::MOUNT_SHAPE = "mount_shape";
const std::string DBRanker::MOUNT_SORT = "mount_sort";
const std::string DBRanker::IS_ON_MOUNT = "on_mount";

const std::string DBRanker::ZYFM_PASS_KEY = "zyfm_pass_key";
const std::string DBRanker::ZYFM_PASS_TICK = "zyfm_pass_tick";

const std::string DBRanker::WING_LEVEL = "wing_level";

const std::string DBRanker::SEND_FLOWER = "send_flower";
const std::string DBRanker::RECV_FLOWER = "recv_flower";
const std::string DBRanker::ACT_SEND_FLOWER = "act_send_flower";
const std::string DBRanker::ACT_RECV_FLOWER = "act_recv_flower";

const std::string DBRanker::OP_TYPE = "op_type";

const std::string DBRanker::PET_DETAIL="pet_detail";
const std::string DBRanker::Pet::PET_IS_REMOVE = "pet_is_remove";
const std::string DBRanker::Pet::PET_INDEX = "pet_index";
const std::string DBRanker::Pet::PET_FORCE = "pet_force";
const std::string DBRanker::Pet::PET_SORT = "pet_sort";
const std::string DBRanker::Pet::PET_CUR_SORT = "pet_cur_sort";
const std::string DBRanker::Pet::PET_NAME = "pet_name";
const std::string DBRanker::Pet::PET_LEVEL = "pet_level";

const std::string DBRanker::SKILL_LIST = "skill_list";
const std::string DBRanker::Skill_list::skill_id = "skill_id";
const std::string DBRanker::Skill_list::skill_level = "skill_level";

const std::string DBRanker::EQUIP_LIST = "equip_list";
const std::string DBRanker::EQUIP_SET = "equip_set";

const std::string DBRanker::FAIRY_ACT = "fairy_act";
const std::string DBRanker::Fairy_act::ID = "id";
const std::string DBRanker::Fairy_act::SELECT_ICON = "select_icon";

///////////////////////// DBMAttackLabelRecord
const std::string DBMAttackLabelRecord::COLLECTION = "mmo.mattack_label_record";
const std::string DBMAttackLabelRecord::KEY = "key";
const std::string DBMAttackLabelRecord::ROLE_ID = "role_id";
const std::string DBMAttackLabelRecord::ROLE_NAME = "role_name";
const std::string DBMAttackLabelRecord::ROLE_SEX = "role_sex";

///////////////////////// DBWorldBossNew
const std::string DBWorldBossNew::COLLECTION = "mmo.world_boss_new";
const std::string DBWorldBossNew::KEY = "key";
const std::string DBWorldBossNew::SCENE_ID = "scene_id";
const std::string DBWorldBossNew::STATUS = "status";
const std::string DBWorldBossNew::DIE_TICK = "die_tick";

///////////////////////// DBTrvlWboss
const std::string DBTrvlWboss::COLLECTION = "mmo.trvl_wboss";
const std::string DBTrvlWboss::KEY = "key";
const std::string DBTrvlWboss::STATUS = "status";
const std::string DBTrvlWboss::KILLER = "killer";
const std::string DBTrvlWboss::KILLER_NAME = "killer_name";

/////////////////////////DBScriptProgress/////////////////////
const std::string DBScriptProgress::COLLECTION = "mmo.script_progress";
const std::string DBScriptProgress::ID = "id";
const std::string DBScriptProgress::SCRIPT_SORT = "script_sort";
const std::string DBScriptProgress::IS_FINISH = "is_finish";
const std::string DBScriptProgress::OWNER = "owner";
const std::string DBScriptProgress::SAVE_TICK = "save_tick";

//DBScriptClean/////////////////////////////////////
const std::string DBScriptClean::COLLECTION = "mmo.script_clean";
const std::string DBScriptClean::ID = "id";

const std::string DBScriptClean::CURRENT_SCRIPT = "current_script";
const std::string DBScriptClean::FINISH_SCRIPT = "finish_script";
const std::string DBScriptClean::ScriptInfo::SCRIPT_SORT = "script_sort";
const std::string DBScriptClean::ScriptInfo::CHAPTER_KEY = "chapter_key";
const std::string DBScriptClean::ScriptInfo::USE_TIMES = "use_times";
const std::string DBScriptClean::ScriptInfo::USE_TICK = "use_tick";

const std::string DBScriptClean::EXP = "exp";
const std::string DBScriptClean::SAVVY = "savvy";
const std::string DBScriptClean::MONEY_COPPER = "money_copper";
const std::string DBScriptClean::MONEY_BIND_COPPER = "money_bind_copper";
const std::string DBScriptClean::MONEY_GOLD = "money_gold";
const std::string DBScriptClean::MONEY_BIND_GOLD = "money_bind_gold";
const std::string DBScriptClean::STATE = "state";
const std::string DBScriptClean::BEGIN_TICK = "begin_tick";
const std::string DBScriptClean::END_TICK = "end_tick";
const std::string DBScriptClean::TERMINATE_TICK = "terminate_tick";
const std::string DBScriptClean::NEXT_CHECK_TICK = "next_check_tick";

const std::string DBScriptClean::SCRIPT_COMPACT = "script_compact";
const std::string DBScriptClean::ScriptCompact::COMPACT_TYPE = "compact_type";
const std::string DBScriptClean::ScriptCompact::START_TICK = "start_tick";
const std::string DBScriptClean::ScriptCompact::EXPIRED_TICK = "expired_tick";
const std::string DBScriptClean::ScriptCompact::SYS_NOTIFY = "sys_notify";

//DBPlayerTip/////////////////////////////////////
const std::string DBPlayerTip::COLLECTION = "mmo.player_tip";
const std::string DBPlayerTip::ID = "id";
const std::string DBPlayerTip::TIP_DETAIL = "tip_detail";
const std::string DBPlayerTip::TipDetail::EVENT_ID = "event_id";
const std::string DBPlayerTip::TipDetail::FASHION_ID = "fashion_id";
const std::string DBPlayerTip::TipDetail::FASHION_FORCE = "fashion_force";
const std::string DBPlayerTip::TipDetail::FASHION_LEFT_SEC = "fashion_left_sec";
const std::string DBPlayerTip::TipDetail::MARTIAL_ID = "martial_id";
const std::string DBPlayerTip::TipDetail::VIP_TYPE = "vip_type";
const std::string DBPlayerTip::TipDetail::VIP_LEFT_SEC = "vip_left_sec";
const std::string DBPlayerTip::TipDetail::TIPS_FLAG = "tips_flag";

//////////////////////////////////////
const std::string DBScriptHistory::COLLECTION = "mmo.script_history";
const std::string DBScriptHistory::SCRIPT_SORT = "script_sort";

const std::string DBScriptHistory::CHAPTER_REC = "chapter_rec";
const std::string DBScriptHistory::ChapterRec::FIRST_ID = "first_id";
const std::string DBScriptHistory::ChapterRec::FIRST_NAME = "first_name";
const std::string DBScriptHistory::ChapterRec::CHAPTER_KEY = "chapter_key";
const std::string DBScriptHistory::ChapterRec::BEST_USE_TICK = "best_use_tick";

//////////////////////////////////////DBLegendTopPlayer
const std::string DBLegendTopPlayer::COLLECTION = "mmo.legend_top_player";
const std::string DBLegendTopPlayer::COLLECTION2 = "mmo.sword_top_player";
const std::string DBLegendTopPlayer::ID = "id";
const std::string DBLegendTopPlayer::REFRESH_TICK = "refresh_tick";

const std::string DBLegendTopPlayer::PLAYER_SET = "player_set";
const std::string DBLegendTopPlayer::PlayerSet::PLAYER_ID = "player_id";
const std::string DBLegendTopPlayer::PlayerSet::NAME = "name";
const std::string DBLegendTopPlayer::PlayerSet::FIGHT_SCORE = "fight_score";
const std::string DBLegendTopPlayer::PlayerSet::FLOOR = "floor";
const std::string DBLegendTopPlayer::PlayerSet::RANK = "rank";
const std::string DBLegendTopPlayer::PlayerSet::TICK = "tick";

//////////////////////////////////////DBCouplePlayer
const std::string DBCouplePlayer::COLLECTION = "mmo.couple_player";
const std::string DBCouplePlayer::ID = "id";
const std::string DBCouplePlayer::ROLE_MAP = "role_map";

//////////////////////////////////////DBSysSetting
const std::string DBSysSetting::COLLECTION = "mmo.system_setting";
const std::string DBSysSetting::ID = "id";
const std::string DBSysSetting::ACCOUNT = "account";
const std::string DBSysSetting::SET_DETAIL = "set_detail";

const std::string DBSysSetting::SetDetail::IS_SHOCK = "is_shock";
const std::string DBSysSetting::SetDetail::SCREEN_TYPE = "screen_type";
const std::string DBSysSetting::SetDetail::FLUENCY_TYPE = "fluency_type";
const std::string DBSysSetting::SetDetail::SHIELD_TYPE = "shield_type";
const std::string DBSysSetting::SetDetail::TURNOFF_ACT_NOTIFY = "turnoff_act_notify";
const std::string DBSysSetting::SetDetail::AUTO_ADJUST_EXPRESS = "auto_adjust_express";
const std::string DBSysSetting::SetDetail::MUSIC_EFFECT = "music_effect";
const std::string DBSysSetting::SetDetail::SOUND_EFFECT = "sound_effect";

const std::string DBSysSetting::SetDetail::MusicEffect::MUSIC_ON = "music_on";
const std::string DBSysSetting::SetDetail::MusicEffect::MUSIC_VOLUME = "music_volume";

const std::string DBSysSetting::SetDetail::SoundEffect::SOUND_ON = "sound_on";
const std::string DBSysSetting::SetDetail::SoundEffect::SOUND_VOLUME = "sound_volume";

//////////////////////////////////////DBCustomerSVCDetail
const std::string DBCustomerSVCDetail::COLLECTION = "mmo.customer_service_detail";
const std::string DBCustomerSVCDetail::ID = "id";
const std::string DBCustomerSVCDetail::LAST_SUMMIT_TYPE = "last_summit_type";
const std::string DBCustomerSVCDetail::CONTENT = "content";
const std::string DBCustomerSVCDetail::TITLE = "title";
const std::string DBCustomerSVCDetail::OPINION_REWARD = "opinion_reward_info";
const std::string DBCustomerSVCDetail::RewardInfo::OPINION_INDEX = "opinion_index";
const std::string DBCustomerSVCDetail::RewardInfo::REWARD_STATUS = "reward_status";

//MediaGiftPlayer//////////////////////////////////
const std::string DBMediaGiftPlayer::COLLECTION = "mmo.media_gift";
const std::string DBMediaGiftPlayer::ID = "id";
const std::string DBMediaGiftPlayer::USE_TIMES_LIST = "use_times_list";
const std::string DBMediaGiftPlayer::USE_TAGS_LIST = "use_tag_list";
const std::string DBMediaGiftPlayer::USE_TICK_LIST = "use_tick_list";

//DBRpmFakeInfo//////////////////////////////////
const std::string DBRpmFakeInfo::COLLECTION = "mmo.rpm_fake_info";
const std::string DBRpmFakeInfo::ID = Role::ID;
const std::string DBRpmFakeInfo::NAME = Role::NAME;
const std::string DBRpmFakeInfo::FORCE = Role::FORCE;
const std::string DBRpmFakeInfo::LEVEL = Role::LEVEL;
const std::string DBRpmFakeInfo::SEX = Role::SEX;
const std::string DBRpmFakeInfo::CAREER = Role::CAREER;
const std::string DBRpmFakeInfo::VIP_TYPE = Role::VIP_TYPE;
const std::string DBRpmFakeInfo::CONFIG_ID = "config_id";
const std::string DBRpmFakeInfo::WING_LEVEL = "wing_level";
const std::string DBRpmFakeInfo::SOLIDER_LEVEL = "solider_level";

///////////////////////////DBOpenActivity
DBField	DBOpenActivity::COLLECTION = "mmo.open_activity";
DBField DBOpenActivity::ID = "id";
DBField DBOpenActivity::OPEN_ACT = "open_act";

DBField DBOpenActivity::Act::SUB_VALUE = "sub_value";
DBField DBOpenActivity::Act::UPDATE_TICK = "update_tick";
DBField DBOpenActivity::Act::DRAWED_SET = "drawed_set";
DBField DBOpenActivity::Act::ID = "id";
DBField DBOpenActivity::Act::ARRIVE = "arrive";
DBField DBOpenActivity::Act::ARRIVE_MAP = "arrive_map";
DBField DBOpenActivity::Act::DRAWED = "drawed";
DBField DBOpenActivity::Act::SECOND_SUB = "second_sub";
DBField DBOpenActivity::Act::START_TICK = "start_tick";
DBField DBOpenActivity::Act::STOP_TICK = "stop_tick";

DBField DBOpenActivity::CORNUCOPIA_TASK =  "cornucopis_task";
DBField DBOpenActivity::CornucopiaTask::TASKID = "task_id";
DBField DBOpenActivity::CornucopiaTask::COMPLETION_TIMES = "completion_times";
DBField DBOpenActivity::CornucopiaTask::TOTAL_TIMES = "total_times";

DBField DBOpenActivity::CORNUCOPIA_STAGE = "cornucopia_stage";
DBField DBOpenActivity::CornucopiaStage::STAGEID = "stage_id";
DBField DBOpenActivity::CornucopiaStage::FLAG = "flag";

DBField DBOpenActivity::RED_PACKET_GROUP = "red_packet_group";

DBField DBOpenActivity::ACT_DATA_RESET_FLAG = "act_data_reset_flag";
DBField DBOpenActivity::LAST_ACT_TIME = "last_act_time";
DBField DBOpenActivity::LAST_ACT_TYPE = "last_act_type";

DBField DBOpenActivity::ACCU_MAP = "accu_map";
DBField DBOpenActivity::COMBINE_TICK = "combine_tick";

DBField DBOpenActivity::ReLogin::SINGLE = "single";
DBField DBOpenActivity::ReLogin::TEN = "ten";
DBField DBOpenActivity::ReLogin::HUNDRED = "hundred";
DBField DBOpenActivity::ReLogin::MULTIPLE = "multiple";
DBField DBOpenActivity::ReLogin::SINGLE_STATE = "single_state";
DBField DBOpenActivity::ReLogin::TEN_STATE = "ten_state";
DBField DBOpenActivity::ReLogin::HUNDRED_STATE = "hundred_state";
DBField DBOpenActivity::ReLogin::MULTIPLE_STATE = "multiple_state";
DBField DBOpenActivity::ReLogin::CUMULATIVE_DAY = "cumulative_day";

///////////////////////////DBLuckyWheelSys
DBField	DBLuckyWheelSys::COLLECTION = "mmo.lucky_wheel_sys";
DBField	DBLuckyWheelSys::ID = "id";
DBField	DBLuckyWheelSys::ACT_TYPE = "act_type";
DBField	DBLuckyWheelSys::SAVE_GOLD = "save_gold";
DBField	DBLuckyWheelSys::DATE_TYPE = "date_type";
DBField	DBLuckyWheelSys::FIRST_DATE = "first_date";
DBField	DBLuckyWheelSys::LAST_DATE = "last_date";
DBField	DBLuckyWheelSys::IS_COMBINE_RESET = "is_combine_reset";
DBField	DBLuckyWheelSys::RESET_TICK = "reset_tick";
DBField	DBLuckyWheelSys::SAVE_DAY = "save_day";
DBField	DBLuckyWheelSys::REFRESH_TIMES = "refresh_times";
DBField	DBLuckyWheelSys::ALL_FINISH_TIMES = "all_finish_times";

DBField	DBLuckyWheelSys::ITEM_SET = "item_set";
DBField	DBLuckyWheelSys::ServerItemSet::PLAYER_ID = "player_id";
DBField	DBLuckyWheelSys::ServerItemSet::PLAYER_NAME = "player_name";
DBField	DBLuckyWheelSys::ServerItemSet::GET_TIME = "get_time";
DBField	DBLuckyWheelSys::ServerItemSet::ITEM_BIND = "item_bind";
DBField	DBLuckyWheelSys::ServerItemSet::AMOUNT = "amount";
DBField	DBLuckyWheelSys::ServerItemSet::ITEM_ID = "item_id";
DBField	DBLuckyWheelSys::ServerItemSet::REWARD_MULT = "reward_mult";
DBField DBLuckyWheelSys::ServerItemSet::SUB_VALUE = "sub_value";

DBField	DBLuckyWheelSys::RANK_NUM_SET = "rank_num_set";
DBField	DBLuckyWheelSys::RANK_LUCKY_SET = "rank_lucky_set";
DBField	DBLuckyWheelSys::OneRankSet::RANK = "rank";
DBField	DBLuckyWheelSys::OneRankSet::TICK = "tick";
DBField	DBLuckyWheelSys::OneRankSet::NUM = "num";
DBField	DBLuckyWheelSys::OneRankSet::ROLE_ID = "role_id";
DBField	DBLuckyWheelSys::OneRankSet::NAME = "name";

DBField	DBLuckyWheelSys::ROLE_MAIL_SET = "role_mail_set";
DBField	DBLuckyWheelSys::RoleMailSet::ROLE_ID = "role_id";
DBField	DBLuckyWheelSys::RoleMailSet::REWARD_SET = "reward_set";

DBField	DBLuckyWheelSys::GEM_ROLE_MAIL_SET = "gem_role_mail_set";
DBField	DBLuckyWheelSys::GemRoleMailSet::ROLE_ID = "role_id";
DBField	DBLuckyWheelSys::GemRoleMailSet::REWARD_SET = "reward_set";

DBField	DBLuckyWheelSys::BLESS_REWARD_SET = "bless_reward_set";

DBField	DBLuckyWheelSys::SLOT_SET = "slot_set";
DBField	DBLuckyWheelSys::SlotSet::TIME_POINT = "time_point";
DBField	DBLuckyWheelSys::SlotSet::SLOT_ID = "slot_id";
DBField	DBLuckyWheelSys::SlotSet::SERVER_BUY = "server_buy";

///////////////////////////DBLuckyWheel
DBField	DBLuckyWheel::COLLECTION = "mmo.lucky_wheel_player";
DBField	DBLuckyWheel::ID = "id";

DBField	DBLuckyWheel::ACTIVITY_DETAIL = "activity_detail";
DBField	DBLuckyWheel::ActivityDetail::ACTIVITY_ID = "activity_id";
DBField	DBLuckyWheel::ActivityDetail::ACT_SCORE = "act_score";
DBField	DBLuckyWheel::ActivityDetail::WHEEL_TIMES = "wheel_times";
DBField	DBLuckyWheel::ActivityDetail::RESET_TIMES = "reset_times";
DBField	DBLuckyWheel::ActivityDetail::LOGIN_TICK = "login_tick";
DBField	DBLuckyWheel::ActivityDetail::USE_FREE = "use_free";
DBField	DBLuckyWheel::ActivityDetail::LABEL_GET = "label_get";
DBField	DBLuckyWheel::ActivityDetail::RANK_GET = "rank_get";
DBField	DBLuckyWheel::ActivityDetail::REWARD_GET = "reward_get";
DBField	DBLuckyWheel::ActivityDetail::NINE_WORD_REWARD = "nine_word_reward";
DBField	DBLuckyWheel::ActivityDetail::IS_FIRST = "is_first";
DBField	DBLuckyWheel::ActivityDetail::OPEN_TIMES = "open_times";
DBField	DBLuckyWheel::ActivityDetail::RESET_TICK = "reset_tick";
DBField	DBLuckyWheel::ActivityDetail::COMBINE_RESET = "combine_reset";
DBField	DBLuckyWheel::ActivityDetail::REWARD_LOCATION = "reward_location";
DBField	DBLuckyWheel::ActivityDetail::REBATE_MAP = "rebate_map";

DBField	DBLuckyWheel::ActivityDetail::ITEM_RECORD = "item_record";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_BIND = "item_bind";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::AMOUNT = "amount";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::GET_TIME = "get_time";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::ITEM_ID = "item_id";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::REWARD_MULT = "reward_mult";
DBField	DBLuckyWheel::ActivityDetail::ItemRecord::SUB_VALUE = "sub_value";

DBField	DBLuckyWheel::ActivityDetail::PERSON_SLOT_SET = "person_slot_set";
DBField	DBLuckyWheel::ActivityDetail::PersonSlotSet::TIME_POINT = "time_point";
DBField	DBLuckyWheel::ActivityDetail::PersonSlotSet::SLOT_ID = "slot_id";
DBField	DBLuckyWheel::ActivityDetail::PersonSlotSet::BUY_TIMES = "buy_time";
DBField	DBLuckyWheel::ActivityDetail::PersonSlotSet::IS_COLOR = "is_color";
DBField	DBLuckyWheel::ActivityDetail::PersonSlotSet::NINE_SLOT = "nine_slot";

DBField DBLuckyWheel::ActivityDetail::FISH_INFO_VEC = "fish_info_vec";
DBField DBLuckyWheel::ActivityDetail::FISH_INFO::TYPE = "type";
DBField DBLuckyWheel::ActivityDetail::FISH_INFO::LAYER = "layer";
DBField DBLuckyWheel::ActivityDetail::FISH_INFO::FLAG = "flag";
DBField DBLuckyWheel::ActivityDetail::FISH_INFO::POS_X = "pos_x";
DBField DBLuckyWheel::ActivityDetail::FISH_INFO::POS_Y = "pos_y";
DBField DBLuckyWheel::ActivityDetail::FISH_REWARD = "fish_reward";

DBField	DBLuckyWheel::ActivityDetail::SHOP_SLOT_MAP = "shop_slot_map";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::IS_RARITY = "is_rarity";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::IS_BUY = "is_buy";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::IS_CAST = "is_cast";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::SLOT_ID = "slot_id";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE = "item_price";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_PRICE_PRE = "item_price_pre";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::GROUP_ID = "group_id";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::DAY = "day";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_ID = "item_id";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_AMOUNT = "item_amount";
DBField	DBLuckyWheel::ActivityDetail::ShopSlot::ITEM_BIND = "item_bind";
DBField	DBLuckyWheel::ActivityDetail::REFRESH_REWARD_MAP = "refresh_reward_map";
DBField	DBLuckyWheel::ActivityDetail::REFRESH_TICK = "refresh_tick";
DBField	DBLuckyWheel::ActivityDetail::REFRESH_TIMES_MAP = "refresh_times_map";
DBField	DBLuckyWheel::ActivityDetail::SLOT_INDEX = "slot_index";
DBField	DBLuckyWheel::ActivityDetail::SLOT_SCALE = "slot_scale";
DBField	DBLuckyWheel::ActivityDetail::SHOW_TIMES_MAP = "show_times_map";
DBField	DBLuckyWheel::ActivityDetail::SHOW_TIMES_FINA_MAP = "show_times_fina_map";
DBField	DBLuckyWheel::ActivityDetail::REWARD_SCALE = "reward_scale";
DBField	DBLuckyWheel::ActivityDetail::MAZE_FREE = "maze_free";
DBField	DBLuckyWheel::ActivityDetail::BLESS = "bless";
DBField	DBLuckyWheel::ActivityDetail::FREE_TIMES_MAP = "free_times_map";
DBField	DBLuckyWheel::ActivityDetail::SLOT_ITEM_ID = "slot_item_id";
DBField	DBLuckyWheel::ActivityDetail::SLOT_ITEM_NUM = "slot_item_num";
DBField	DBLuckyWheel::ActivityDetail::REWARD_GEM_MAP = "reward_gem_id";
DBField	DBLuckyWheel::ActivityDetail::TWO_SHOW_MAP = "two_show_map";
DBField	DBLuckyWheel::ActivityDetail::THREE_SHOW_MAP = "three_show_map";
DBField	DBLuckyWheel::ActivityDetail::NOW_SLOT_MAP = "now_slot_map";
DBField	DBLuckyWheel::ActivityDetail::FINA_SLOT_MAP = "fina_slot_map";
DBField	DBLuckyWheel::ActivityDetail::BLESS_VALUE = "bless_value";
DBField	DBLuckyWheel::ActivityDetail::REWARD_RECORD_MAP = "reward_record_map";
DBField	DBLuckyWheel::ActivityDetail::EXCHANGE_ITEM_FREQUENCY = "exchange_item_frequency";
DBField	DBLuckyWheel::ActivityDetail::BLESS_REWARD_FREQUENCY = "bless_reward_frequency";
DBField	DBLuckyWheel::ActivityDetail::BLESS_REWARD_POSSESS = "bless_reward_possess";

//////////////////////////DBDailyActSys
DBField DBDailyActSys::COLLECTION = "mmo.daily_act_sys";
DBField DBDailyActSys::ACT_TYPE = "act_type";
DBField DBDailyActSys::ACTIVITY_ID = "activity_id";
DBField DBDailyActSys::REFRESH_TICK = "refresh_tick";
DBField DBDailyActSys::RESET_TICK = "reset_tick";
DBField DBDailyActSys::FIRST_DATE = "first_date";
DBField DBDailyActSys::LAST_DATE = "last_date";

//////////////////////////InvestRechargeSystem
DBField InvestRecharge::COLLECTION = "mmo.invest_recharge_system";
DBField InvestRecharge::VERSION = "version";
DBField InvestRecharge::INVEST_RECHARGE = "invest_recharge";

DBField InvestRecharge::InvestMap::ID = "id";
DBField InvestRecharge::InvestMap::IS_BUY = "is_buy";
DBField InvestRecharge::InvestMap::BUY_TIME = "buy_time";
DBField InvestRecharge::InvestMap::REWARD_TIME = "reward_time";
DBField InvestRecharge::InvestMap::GET_REWARD = "get_reward";
DBField InvestRecharge::InvestMap::INVEST_INDEX = "invest_index";
DBField InvestRecharge::InvestMap::INVEST_REWARDS = "invest_rewards";
DBField InvestRecharge::InvestMap::VIP_REWARDS = "vip_rewards";
DBField InvestRecharge::InvestMap::VIP_LEVEL = "vip_level";
DBField InvestRecharge::InvestMap::SAVE_FLAG = "save_flag";

DBField InvestRecharge::SERIAL_DAILY = "serial_daily";
DBField InvestRecharge::SERIAL_REBATE = "serial_rebate";
DBField InvestRecharge::SERIAL_INVEST = "serial_invest";

///////////////////////////
DBField InvestRecharger::COLLECTION = "mmo.invester";
DBField InvestRecharger::ID = "id";

///////////////////////////DBDailyRechargeRoleMail
DBField DBDailyRechargeRoleMail::COLLECTION = "mmo.daily_recharge_mail";
DBField DBDailyRechargeRoleMail::ID = "id";

DBField DBDailyRechargeRoleMail::MAIL_MAP = "mail_map";
DBField DBDailyRechargeRoleMail::ROLE_ID = "role_id";
DBField DBDailyRechargeRoleMail::FIRST_RECHARGE_TIMES = "first_recharge_times";
DBField DBDailyRechargeRoleMail::DAILY_RECHARGE = "daily_recharge";

///////////////////////////DBRechargeRewards
DBField DBRechargeRewards::COLLECTION = "mmo.recharge_rewards";
DBField DBRechargeRewards::ID = "id";

DBField DBRechargeRewards::RECHARGE_MONEY = "recharge_money";
DBField DBRechargeRewards::RECHARGE_TIMES = "recharge_times";
DBField DBRechargeRewards::RECHARGE_AWARDS = "recharge_awards";
DBField DBRechargeRewards::FEEDBACK_AWARDS = "feedback_awards";
DBField DBRechargeRewards::RECHARGE_MAP = "recharge_map";
DBField DBRechargeRewards::RECHARGE_TYPE = "recharge_type";
DBField DBRechargeRewards::FIRST_RECHARGE_TIME = "first_recharge_time";
DBField DBRechargeRewards::LAST_RECHARGE_TIME = "last_recharge_time";
DBField DBRechargeRewards::LOVE_GIFT_INDEX = "love_gift_index";
DBField DBRechargeRewards::LOVE_RECHARGE_MONEY = "love_recharge_money";

DBField DBRechargeRewards::DAILY_RECHARGE = "dayil_recharge";
DBField DBRechargeRewards::DailyRecharge::TODAY_RECHARGE_GOLD = "today_recharge_gole";
DBField DBRechargeRewards::DailyRecharge::LAST_RECHARGE_TIME = "last_recharge_time";
DBField DBRechargeRewards::DailyRecharge::DAILY_RECHARGE_REWARDS = "daily_recharge_rewards";
DBField DBRechargeRewards::DailyRecharge::FIRST_RECHARGE_GOLD = "first_recharge_gole";
DBField DBRechargeRewards::DailyRecharge::ACT_RECHARGE_TIMES = "act_recharge_times";
DBField DBRechargeRewards::DailyRecharge::ACT_HAS_MAIL = "act_has_mail";

DBField DBRechargeRewards::INVEST_RECHARGE = "invest_recharge";
DBField DBRechargeRewards::InvestRecharge::IS_BUY = "is_buy";
DBField DBRechargeRewards::InvestRecharge::BUY_TIME = "buy_time";
DBField DBRechargeRewards::InvestRecharge::REWARD_TIME = "reward_time";
DBField DBRechargeRewards::InvestRecharge::GET_REWARD = "get_reward";
DBField DBRechargeRewards::InvestRecharge::INVEST_INDEX = "invest_index";
DBField DBRechargeRewards::InvestRecharge::INVEST_REWARDS = "invest_rewards";
DBField DBRechargeRewards::InvestRecharge::VIP_REWARDS = "vip_rewards";

DBField DBRechargeRewards::REBATE_RECHARGE = "rebate_recharge";
DBField DBRechargeRewards::RebateRecharge::REBATE_TIMES = "rebate_times";
DBField DBRechargeRewards::RebateRecharge::LAST_BUY_TIME = "last_buy_time";

DBField DBRechargeRewards::TOTAL_RECHARGE = "total_recharge";
DBField DBRechargeRewards::TotalRecharge::REWARD_STATES = "reward_states";


//////////////////////////////////////////
// DBGWedding
DBField DBGWedding::COLLECTION = "mmo.gwedding";
DBField DBGWedding::ID = "id";
DBField DBGWedding::WEDDING_TICK = "wedding_tick";
DBField DBGWedding::PARTNER_ONE = "partner_one";
DBField DBGWedding::PARTNER_TWO = "partner_two";
DBField DBGWedding::DAY_WEDDING_TIMES = "day_wedding_times";
DBField DBGWedding::DAY_REFRESH_TICK = "day_refresh_tick";
DBField DBGWedding::INTIMACY = "intimacy";
DBField DBGWedding::HISTORY_INTIMACY = "history_intimacy";
DBField DBGWedding::WEDDING_TYPE = "wedding_type";
DBField DBGWedding::KEEPSAKE_ID = "keepsake_id";
DBField DBGWedding::KEEPSAKE_LEVEL = "keepsake_level";
DBField DBGWedding::KEEPSAKE_SUBLEVEL = "keepsake_sublevel";
DBField DBGWedding::KEEPSAKE_PROGRESS = "keepsake_progress";
DBField DBGWedding::SWEET_DEGREE_1 = "sweet_degree_1";
DBField DBGWedding::RING_LEVEL_1 = "ring_level_1";
DBField DBGWedding::SYS_LEVEL_1 = "sys_level_1";
DBField DBGWedding::TREE_LEVEL_1 = "tree_level_1";
DBField DBGWedding::TICK_1 = "tick_1";
DBField DBGWedding::FETCH_TICK_1 = "fetch_tick_1";
DBField DBGWedding::ONCE_REWARD_1 = "once_reward_1";
DBField DBGWedding::LEFT_TIMES_1 = "left_times_1";

DBField DBGWedding::SWEET_DEGREE_2 = "sweet_degree_2";
DBField DBGWedding::RING_LEVEL_2 = "ring_level_2";
DBField DBGWedding::SYS_LEVEL_2 = "sys_level_2";
DBField DBGWedding::TREE_LEVEL_2 = "tree_level_2";
DBField DBGWedding::TICK_2 = "tick_2";
DBField DBGWedding::FETCH_TICK_2 = "fetch_tick_2";
DBField DBGWedding::ONCE_REWARD_2 = "once_reward_2";
DBField DBGWedding::LEFT_TIMES_2 = "left_times_2";

//////////////////////////////////////////
// DBPlayerWedding
DBField DBPlayerWedding::COLLECTION = "mmo.player_wedding";
DBField DBPlayerWedding::ID = "id";
DBField DBPlayerWedding::INTIMACY = "intimacy";
DBField DBPlayerWedding::WEDDING_ID = "wedding_id";
DBField DBPlayerWedding::RECV_FLOWER = "recv_flower";
DBField DBPlayerWedding::SEND_FLOWER = "send_flower";
DBField DBPlayerWedding::IS_HAS_RING = "is_has_ring";
DBField DBPlayerWedding::SIDE_FASHION_ID = "side_fashion_id";
DBField DBPlayerWedding::SIDE_FASHION_COLOR = "side_fashion_color";
DBField DBPlayerWedding::WEDDING_PROPERTY = "wedding_property";
DBField DBPlayerWedding::Wedding_detail::LEVEL = "level";
DBField DBPlayerWedding::Wedding_detail::EXP = "exp";
DBField DBPlayerWedding::Wedding_detail::SIDE_LEVEL = "side_level";
DBField DBPlayerWedding::Wedding_detail::SIDE_ORDER = "side_order";
DBField DBPlayerWedding::WEDDING_LABEL = "wedding_label";

/////////////////////////////////////////
// DBPlayerWedding
DBField DBBrother::COLLECTION = "mmo.player_brother";
DBField DBBrother::ID = "id";
DBField DBBrother::EMOTION = "emotion";
DBField DBBrother::PROMISE = "promise";
DBField DBBrother::BROTHRES = "brothers";
DBField DBBrother::TASK_ID = "task_id";
DBField DBBrother::MONSTER_ID = "monster_id";
DBField DBBrother::MONSTER_NUM = "monster_num";
DBField DBBrother::FINISH_INFO = "finish_info";
DBField DBBrother::TASK_LIST = "task_list";
DBField DBBrother::CUR_SCORE = "cur_score";
DBField DBBrother::BROTHER_TASK = "brother_task";
//////////////////////////////////////////

/////////////////////////////////////////
// DBMagicWeapon
DBField DBMagicWeapon::COLLECTION = "mmo.magicweapon";
DBField DBMagicWeapon::OPEN = "open";
DBField DBMagicWeapon::ROLEID = "roleid";
DBField DBMagicWeapon::MAGICW_LIST = "magicw_list";
DBField DBMagicWeapon::MAGICW_ID = "magicw_id";
DBField DBMagicWeapon::MAGICW_SKILL_ID = "magicw_skill_id";
DBField DBMagicWeapon::MAGICW_SKILL_LVL = "magicw_skill_lvl";
DBField DBMagicWeapon::ACTIVED_STATE = "actived_state";
DBField DBMagicWeapon::IS_ADORN = "is_adorn";
DBField DBMagicWeapon::MAGICW_RANK_GRADE = "magicw_rank_grade";
DBField DBMagicWeapon::MAGICW_RANK_PROGRESS = "magicw_rank_progress";
DBField DBMagicWeapon::MAGICW_QUALITY_GRADE = "magicw_qua_grade";
DBField DBMagicWeapon::MAGICW_QUALITY_PROGRESS = "magicw_qua_progress";

//////////////////////////////////////////后台屏蔽排行榜
DBField DBRankHide::COLLECTION = "backstage.rank_hide";
DBField DBRankHide::ROLE_ID = "role_id";

/////////////////////////////////////////
DBField DBBattler::COLLECTION = "mmo.battler";
DBField DBBattler::ID = "id";
DBField DBBattler::TARENA = "tarena";
DBField DBBattler::TARENA_OFFLINE = "tarena_offline";

DBField DBBattler::Tarena::STAGE = "stage";
DBField DBBattler::Tarena::SCORE = "score";
DBField DBBattler::Tarena::ADJUST_TICK = "adjust_tick";
DBField DBBattler::Tarena::DRAW = "draw";
DBField DBBattler::Tarena::WIN_TIMES = "win_times";
DBField DBBattler::Tarena::GET_EXPLOIT = "get_exploit";
DBField DBBattler::Tarena::ATTEND_TIMES = "attend_times";
DBField DBBattler::Tarena::DRAW_WIN = "draw_win";

/////////////////////////////////////////
DBField DBTarenaRole::COLLECTION = "mmo.tarena_role";
DBField DBTarenaRole::ID = "id";

DBField DBTarenaRole::SERVER = "server";
DBField DBTarenaRole::ROLE = "role";

DBField DBTarenaRole::INIT = "init";
DBField DBTarenaRole::SYNC = "sync";
DBField DBTarenaRole::STAGE = "stage";
DBField DBTarenaRole::SCORE = "score";
DBField DBTarenaRole::UPDATE_TICK = "update_tick";

/////////////////////////////DBWeddingRank
DBField DBWeddingRank::COLLECTION = "mmo.wedding_act_rank";
DBField DBWeddingRank::ID = "id";
DBField DBWeddingRank::TICK = "tick";
DBField DBWeddingRank::SERVER = "server";
DBField DBWeddingRank::PLAYER1 = "player1";
DBField DBWeddingRank::PLAYER2 = "player2";

/////////////////////////////DBRechargeRank
DBField DBRechargeRank::COLLECTION = "mmo.recharge_rank";
DBField DBRechargeRank::ID = "id";

DBField DBRechargeRank::GET_RANK = "get_rank";
DBField DBRechargeRank::COST_RANK = "cost_rank";
DBField DBRechargeRank::RECHARGE_BACK_RANK = "recharge_back_rank";

DBField DBRechargeRank::RANK = "rank";
DBField DBRechargeRank::ROLE_ID = "role_id";
DBField DBRechargeRank::SERVER = "server";
DBField DBRechargeRank::ROLE = "role";
DBField DBRechargeRank::AMOUNT = "amount";
DBField DBRechargeRank::TICK = "tick";
DBField DBRechargeRank::ACTIVITY_ID = "activity_id";

//////////////////////////////////////// DBOFFLINE_HOOK
DBField DBPlayerOfflineHook::COLLECTION = "mmo.player_offlinehook";
DBField DBPlayerOfflineHook::ID = "id";
DBField DBPlayerOfflineHook::last_start_time = "last_start_time";
DBField DBPlayerOfflineHook::last_end_time = "last_end_time";
DBField DBPlayerOfflineHook::surplus_time = "surplus_time";
DBField DBPlayerOfflineHook::last_reward_exp = "last_reward_exp";
DBField DBPlayerOfflineHook::last_reward = "last_reward";
DBField DBPlayerOfflineHook::last_costProp = "last_costProp";
DBField DBPlayerOfflineHook::last_offhook_time = "last_offhook_time";
DBField DBPlayerOfflineHook::one_point_five_plue_time = "one_point_five_plue_time";
DBField DBPlayerOfflineHook::two_plus_time = "two_plus_time";
DBField DBPlayerOfflineHook::vip_plus_time = "vip_plus_time";
DBField DBPlayerOfflineHook::today_reward = "today_reward";


////////////////////////////////////////// 帮派领地
DBField DBLeagueRegionFight::COLLECTION = "mmo.league_regionfight";
DBField DBLeagueRegionFight::ID = "id";
DBField DBLeagueRegionFight::TOTAL_NUM = "total_num";
DBField DBLeagueRegionFight::LAST_TICK = "last_tick";
DBField DBLeagueRegionFight::FINISH = "finish";
DBField DBLeagueRegionFight::LRF_RESULT = "lrf_result_set";
DBField DBLeagueRegionFight::SUPP_RESULT = "supp_result";

DBField DBLeagueRegionFight::LRF_RES::LEAGUE_ID = "league_id";
DBField DBLeagueRegionFight::LRF_RES::LEAGUE_NAME = "league_name";
DBField DBLeagueRegionFight::LRF_RES::LEAGUE_RANK = "league_rank";
DBField DBLeagueRegionFight::LRF_RES::FORCE = "league_force";
DBField DBLeagueRegionFight::LRF_RES::LEADER = "leader";
DBField DBLeagueRegionFight::LRF_RES::WIN_OR_STATE = "league_winstate";
DBField DBLeagueRegionFight::LRF_RES::RESOURCE_LAND_ID = "league_reslandid";
DBField DBLeagueRegionFight::LRF_RES::ENEMY_LEAGUE_ID = "enemy_leagueid";
DBField DBLeagueRegionFight::LRF_RES::ENEMY_LEAGUE_NAME = "enamy_league_name";
DBField DBLeagueRegionFight::LRF_RES::ENEMY_RESOURCE_LAND_ID = "enamy_league_reslandid";

DBField DBLeagueRegionFight::SUPP_RES::ROLE_ID = "id";
DBField DBLeagueRegionFight::SUPP_RES::SUPPORT_LEAGUE = "league_id";
DBField DBLeagueRegionFight::SUPP_RES::RESULT = "result";

///////////////////后台活动奖励记录////////////////////////////////////
DBField DBJYBackActPlayerRec::COLLECTION = "mmo.jyback_act_player_rec";
DBField DBJYBackActPlayerRec::ID = "id";
DBField DBJYBackActPlayerRec::ACT_RECORD = "act_record";
DBField DBJYBackActPlayerRec::ActRecord::ACT_ID = "act_id";
DBField DBJYBackActPlayerRec::ActRecord::ACT_START = "act_start";
DBField DBJYBackActPlayerRec::ActRecord::ACT_END = "act_end";
DBField DBJYBackActPlayerRec::ActRecord::MAIL_TITLE = "mail_title";
DBField DBJYBackActPlayerRec::ActRecord::MAIL_CONTENT = "mail_content";
DBField DBJYBackActPlayerRec::ActRecord::DAILY_REFRESH_TICK = "daily_refresh_tick";
DBField DBJYBackActPlayerRec::ActRecord::DAILY_VALUE = "daily_value";
DBField DBJYBackActPlayerRec::ActRecord::SINGLE_COND_VALUE = "single_cond_value";
DBField DBJYBackActPlayerRec::ActRecord::REWARD_VALUE = "reward_value";
DBField DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_ID = "reward_id";
DBField DBJYBackActPlayerRec::ActRecord::RewardValue::REWARD_FLAG = "reward_flag";
DBField DBJYBackActPlayerRec::ActRecord::UPDATE_TICK = "update_tick";
DBField DBJYBackActPlayerRec::ActRecord::REWARD_ITEM = "reward_item";
DBField DBJYBackActPlayerRec::ActRecord::RewardItem::REWARD_ID = "reward_id";
DBField DBJYBackActPlayerRec::ActRecord::RewardItem::ITEM_LIST = "item_list";

//////////////////DBMayActivity
DBField DBMayActivity::COLLECTION = "mmo.may_activity";
DBField DBMayActivity::ID = "id";
DBField DBMayActivity::ACT_TYPE = "act_type";

DBField DBMayActivity::REWARD_SET = "reward_set";
DBField DBMayActivity::RewardSet::SUB_MAP = "sub_map";
DBField DBMayActivity::RewardSet::DRAW_MAP = "draw_map";
DBField DBMayActivity::RewardSet::ACT_DRAW_MAP = "act_draw_map";

DBField DBMayActivity::COUPLE_MAP = "couple_map";
DBField DBMayActivity::CoupleMap::WEDDING_ID = "wedding_id";
DBField DBMayActivity::CoupleMap::ONLINE_TICK = "online_tick";
DBField DBMayActivity::CoupleMap::BUY_TICK = "buy_tick";

DBField DBMayActivity::RUN_MAP = "run_map";
DBField DBMayActivity::RunInfo::ROLE_ID = "role_id";
DBField DBMayActivity::RunInfo::NAME = "name";
DBField DBMayActivity::RunInfo::SEX = "sex";
DBField DBMayActivity::RunInfo::VALUE = "value";
DBField DBMayActivity::RunInfo::TICK = "tick";

//////////////////DBMayActivityer
DBField DBMayActivityer::COLLECTION = "mmo.may_activityer";
DBField DBMayActivityer::ID = "id";
DBField DBMayActivityer::ACT_TYPE = "act_type";

DBField DBMayActivityer::MAY_ACT = "may_act";
DBField DBMayActivityer::Act::ID = "id";
DBField DBMayActivityer::Act::SUB_VALUE = "sub_value";
DBField DBMayActivityer::Act::DRAWED_SET = "drawed_set";
DBField DBMayActivityer::Act::ARRIVE = "arrive";
DBField DBMayActivityer::Act::ARRIVE_MAP = "arrive_map";
DBField DBMayActivityer::Act::DRAWED = "drawed";
DBField DBMayActivityer::Act::SECOND_SUB = "second_sub";
DBField DBMayActivityer::Act::SEND_MAP = "send_map";
DBField DBMayActivityer::Act::ROLE_MAP = "role_map";
DBField DBMayActivityer::Act::CUMULATIVE_TIMES = "cumulative_times";
DBField DBMayActivityer::Act::REFRESH_TICK = "refresh_tick";
DBField	DBMayActivityer::Act::REFRESH_TIMES_MAP = "refresh_times_map";

DBField DBMayActivityer::SHOP_SLOT_MAP = "shop_slot_map";
DBField DBMayActivityer::ShopSlot::IS_RARITY = "is_rarity";
DBField DBMayActivityer::ShopSlot::IS_CAST = "is_cast";
DBField DBMayActivityer::ShopSlot::SLOT_ID = "slot_id";
DBField DBMayActivityer::ShopSlot::IS_BUY = "is_buy";
DBField DBMayActivityer::ShopSlot::ITEM_PRICE = "item_price";
DBField DBMayActivityer::ShopSlot::GROUP_ID = "group_id";
DBField DBMayActivityer::ShopSlot::DAY = "day";
DBField DBMayActivityer::ShopSlot::ITEM_ID = "item_id";
DBField DBMayActivityer::ShopSlot::ITEM_AMOUNT = "item_amount";
DBField DBMayActivityer::ShopSlot::ITEM_BIND = "item_bind";
DBField DBMayActivityer::ShopSlot::ITEM_PRICE_PRE = "item_price_pre";

DBField DBMayActivityer::FASHION_ACT_INFO = "fashion_act_info";
DBField DBMayActivityer::FashionActInfo::LIVENESS = "liveness";
DBField DBMayActivityer::FashionActInfo::CUR_FASHION_TIMES = "cur_fashion_times";

DBField DBMayActivityer::FashionActInfo::FASHION_LIVENESS_MAP = "fashion_liveness_map";
DBField DBMayActivityer::FashionLivenessMap::TIMES = "times";

DBField DBMayActivityer::FashionLivenessMap::LIVENESS_REWARD_MAP = "liveness_reward_map";
DBField DBMayActivityer::LivenessRewardMap::SLOT_ID = "slot_id";
DBField DBMayActivityer::LivenessRewardMap::STATE = "state";

DBField DBMayActivityer::CORNUCOPIA_TASK =  "cornucopis_task";
DBField DBMayActivityer::CornucopiaTask::TASKID = "task_id";
DBField DBMayActivityer::CornucopiaTask::COMPLETION_TIMES = "completion_times";
DBField DBMayActivityer::CornucopiaTask::TOTAL_TIMES = "total_times";
DBField DBMayActivityer::CornucopiaTask::TASK_NAME = "task_name";

DBField DBMayActivityer::CORNUCOPIA_STAGE = "cornucopia_stage";
DBField DBMayActivityer::CornucopiaStage::STAGEID = "stage_id";
DBField DBMayActivityer::CornucopiaStage::FLAG = "flag";

////////////DBTrvlBattle///////////////////////
DBField DBTrvlBattle::COLLECTION = "mmo.trvl_battle";
DBField DBTrvlBattle::ID = "id";

// id == 0 的信息
DBField DBTrvlBattle::FIRST_TOP_ID = "first_top_id";
DBField DBTrvlBattle::FIRST_TOP_NAME = "first_top_name";

// id == 1 的当前排行信息
DBField DBTrvlBattle::CUR_RANK_LIST = "cur_rank_list";

// id == 2 的当前奖励层数信息
DBField DBTrvlBattle::CUR_ROLE_LIST = "cur_role_list";

// id == 3 的上一轮的战场日志信息
DBField DBTrvlBattle::LAST_RANK_LIST = "last_rank_list";

// id == 4 的名人堂列表
DBField DBTrvlBattle::HISTORY_FAME_LIST = "history_fame_list";

DBField DBTrvlBattle::BattleRole::ROLE_ID = "role_id";
DBField DBTrvlBattle::BattleRole::LEAGUE_ID = "league_id";
DBField DBTrvlBattle::BattleRole::ROLE_NAME = "role_name";
DBField DBTrvlBattle::BattleRole::MAX_FLOOR = "max_floor";
DBField DBTrvlBattle::BattleRole::LAST_REWARD_SCORE = "last_reward_score";
DBField DBTrvlBattle::BattleRole::NEXT_REWARD_SCORE = "next_reward_score";
DBField DBTrvlBattle::BattleRole::NEXT_SCORE_REWARD_ID = "next_score_reward_id";
DBField DBTrvlBattle::BattleRole::SERVER = "server";

DBField DBTrvlBattle::BattleRank::RANK = "rank";
DBField DBTrvlBattle::BattleRank::SCORE = "score";
DBField DBTrvlBattle::BattleRank::TOTAL_KILL_AMOUNT = "total_kill_amount";
DBField DBTrvlBattle::BattleRank::TICK = "tick";
DBField DBTrvlBattle::BattleRank::ROLE_ID = "role_id";
DBField DBTrvlBattle::BattleRank::ROLE_NAME = "role_name";
DBField DBTrvlBattle::BattleRank::PREV = "prev";
DBField DBTrvlBattle::BattleRank::SERVER_FLAG = "server_flag";
DBField DBTrvlBattle::BattleRank::SEX = "sex";
DBField DBTrvlBattle::BattleRank::CAREER = "career";
DBField DBTrvlBattle::BattleRank::LEVEL = "level";
DBField DBTrvlBattle::BattleRank::FORCE = "force";
DBField DBTrvlBattle::BattleRank::WEAPON = "weapon";
DBField DBTrvlBattle::BattleRank::CLOTHES = "clothes";
DBField DBTrvlBattle::BattleRank::WING_LEVEL = "wing_level";
DBField DBTrvlBattle::BattleRank::SOLIDER_LEVEL = "solider_level";
DBField DBTrvlBattle::BattleRank::VIP_TYPE = "vip_type";
DBField DBTrvlBattle::BattleRank::MOUNT_SORT = "mount_sort";
DBField DBTrvlBattle::BattleRank::SWORD_POOL = "sword_pool";
DBField DBTrvlBattle::BattleRank::TIAN_GANG = "tian_gans";
DBField DBTrvlBattle::BattleRank::FASHION_ID = "fashion_id";
DBField DBTrvlBattle::BattleRank::FASHION_COLOR = "fashion_color";

////////////DBLocalTravTeam///////////////////////
DBField DBLocalTravTeam::COLLECTION = "mmo.ltrav_team";
DBField DBLocalTravTeam::ID = "id";
DBField DBLocalTravTeam::TEAM_NAME = "team_name";
DBField DBLocalTravTeam::LEADER_ID = "leader_id";
DBField DBLocalTravTeam::AUTO_SIGNUP = "auto_signup";
DBField DBLocalTravTeam::AUTO_ACCEPT = "auto_accept";
DBField DBLocalTravTeam::NEED_FORCE = "need_force";
DBField DBLocalTravTeam::IS_SIGNUP = "is_signup";
DBField DBLocalTravTeam::REFRESH_SIGNUP_TICK = "refresh_signup_tick";
DBField DBLocalTravTeam::CREATE_TICK = "create_tick";
DBField DBLocalTravTeam::LAST_LOGOUT_TICK = "last_logout_tick";

DBField DBLocalTravTeam::TRAV_TEAMER = "trav_teamer";
DBField DBLocalTravTeam::APPLY_MAP = "apply_map";
DBField DBLocalTravTeam::TravTeamer::TEAMER_ID = "teamer_id";
DBField DBLocalTravTeam::TravTeamer::TEAMER_NAME = "teamer_name";
DBField DBLocalTravTeam::TravTeamer::TEAMER_SEX = "teamer_sex";
DBField DBLocalTravTeam::TravTeamer::TEAMER_CAREER = "teamer_career";
DBField DBLocalTravTeam::TravTeamer::TEAMER_LEVEl = "teamer_level";
DBField DBLocalTravTeam::TravTeamer::TEAMER_FORCE = "teamer_force";
DBField DBLocalTravTeam::TravTeamer::LOGOUT_TICK = "logout_tick";
DBField DBLocalTravTeam::TravTeamer::JOIN_TICK = "join_tick";

////////////DBRemoteTravTeam///////////////////////
DBField DBRemoteTravTeam::COLLECTION = "mmo.rtrav_team";
DBField DBRemoteTravTeam::ID = "id";
DBField DBRemoteTravTeam::TEAM_NAME = "team_name";
DBField DBRemoteTravTeam::LEADER_ID = "leader_id";
DBField DBRemoteTravTeam::SERVER = "server";

DBField DBRemoteTravTeam::QUALITY_TIMES = "quality_times";
DBField DBRemoteTravTeam::SCORE = "score";
DBField DBRemoteTravTeam::CONTINUE_WIN = "continue_win";
DBField DBRemoteTravTeam::UPDATE_TICK = "update_tick";

DBField DBRemoteTravTeam::TRAV_TEAMER = "trav_teamer";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_ID = "teamer_id";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_NAME = "teamer_name";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_SEX = "teamer_sex";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_CAREER = "teamer_career";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_LEVEL = "teamer_level";
DBField DBRemoteTravTeam::TravTeamer::TEAMER_FORCE = "teamer_force";

////////////DBQualityInfo///////////////////////
DBField DBQualityInfo::COLLECTION = "mmo.rtrav_team";
DBField DBQualityInfo::ID = "id";
DBField DBQualityInfo::SIGNUP_SET = "signup_set";

