/*
 * BackField.cpp
 *
 * Created on: 2013-07-31 16:30
 *     Author: lyz
 */

#include "BackField.h"

////////serial/////////////
const std::string DBBackSerial::COLLECTION = "backstage.serial";
const std::string DBBackSerial::ID = "id";

const std::string DBBackSerial::MONEY_SERIAL = "money_serial";
const std::string DBBackSerial::MoneySerial::SERIAL = "serial";
const std::string DBBackSerial::MoneySerial::FRESH_TICK = "fresh_tick";
const std::string DBBackSerial::MoneySerial::COPPER = "copper";
const std::string DBBackSerial::MoneySerial::GOLD = "gold";
const std::string DBBackSerial::MoneySerial::BIND_GOLD = "bind_bold";
const std::string DBBackSerial::MoneySerial::BIND_COPPER = "bind_copper";

const std::string DBBackSerial::ITEM_SERIAL = "item_serial";
const std::string DBBackSerial::ItemSerial::SERIAL = "serial";
const std::string DBBackSerial::ItemSerial::ITEM_ID = "item_id";
const std::string DBBackSerial::ItemSerial::FRESH_TICK = "fresh_tick";
const std::string DBBackSerial::ItemSerial::VALUE = "value";

const std::string DBBackSerial::EXP_SERIAL = "exp_serial";
const std::string DBBackSerial::ExpSerial::SERIAL = "serial";
const std::string DBBackSerial::ExpSerial::FRESH_TICK = "fresh_tick";
const std::string DBBackSerial::ExpSerial::VALUE = "value";
//}}}

////////////FlowControl
const std::string DBFlowControl::COLLECTION = "backstage.flow_control";
const std::string DBFlowControl::ID = "id";
const std::string DBFlowControl::SERVER_INDEX_SET = "server_index_set";
const std::string DBFlowControl::NEED_LOAD_DATA = "need_load_data";

const std::string DBFlowControl::IS_FORBIT_LOGIN = "is_forbit_login";
const std::string DBFlowControl::FORBIT_CHANNEL = "channel";
const std::string DBFlowControl::SERIAL_RECORD = "serial_record";
const std::string DBFlowControl::MONEY_SERIAL_RECORD = "money_serial";
const std::string DBFlowControl::ITEM_SERIAL_RECORD = "item_serial";
const std::string DBFlowControl::EQUIP_SERIAL_RECORD = "equip_serial";
const std::string DBFlowControl::MOUNT_SERIAL_RECORD = "mount_serial";
const std::string DBFlowControl::PET_SERIAL_RECORD = "pet_serial";
const std::string DBFlowControl::SKILL_SERIAL_RECORD = "skill_serial";
const std::string DBFlowControl::MAIL_SERIAL_RECORD = "mail_serial";
const std::string DBFlowControl::MARKET_SERIAL_RECORD = "market_serial";
const std::string DBFlowControl::ACHIEVE_SERIAL_RECORD = "achieve_serial";
const std::string DBFlowControl::FORCE_REFRSH_RANK_TPYE = "force_refrsh_rank_tpye";
const std::string DBFlowControl::PLAYER_LEVEL_SERIAL = "player_level_serial";
const std::string DBFlowControl::OTHER_SERIAL = "other_serial";
const std::string DBFlowControl::ONLINE_USER_SERIAL = "online_user_serial";
const std::string DBFlowControl::LOGIN_SERIAL = "login_serial";
const std::string DBFlowControl::TASK_SERIAL = "task_serial";
const std::string DBFlowControl::RANK_SERIAL = "rank_serial";
const std::string DBFlowControl::CHAT_SERIAL = "chat_serial";

const std::string DBFlowControl::ForceRefreshRankType::RANK_FIGHT_LEVEL = "fight_level";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FIGHT_FORCE = "fight_force";
const std::string DBFlowControl::ForceRefreshRankType::RANK_KILL_VALUE = "kill_value";
const std::string DBFlowControl::ForceRefreshRankType::RANK_KILL_NUM = "killing_num";
const std::string DBFlowControl::ForceRefreshRankType::RANK_KILL_NORMAL = "kill_normal";
const std::string DBFlowControl::ForceRefreshRankType::RANK_KILL_EVIL = "kill_evil";
const std::string DBFlowControl::ForceRefreshRankType::RANK_PET = "rank_pet";
const std::string DBFlowControl::ForceRefreshRankType::RANK_MOUNT = "rank_mount";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_MOUNT = "rank_fun_mount";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_GOD_SOLIDER = "rank_fun_god_solider";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_MAGIC_EQUIP = "rank_fun_magic_equip";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_XIAN_WING = "rank_fun_xian_wing";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_LING_BEAST = "rank_fun_ling_beast";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_BEAST_EQUIP = "rank_fun_beast_equip";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_BEAST_MOUNT = "rank_fun_beast_mount";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_BEAST_WING = "rank_fun_beast_wing";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_BEAST_MAO = "rank_fun_beast_mao";
const std::string DBFlowControl::ForceRefreshRankType::RANK_FUN_TIAN_GANG = "rank_fun_tian_gang";

// DBSceneLine////////////////
// {{{
const std::string DBSceneLine::COLLECTION = "backstage.scene_line";
const std::string DBSceneLine::FLAG = "flag";
const std::string DBSceneLine::SERVER_INDEX_SET = "server_index_set";
const std::string DBSceneLine::SERVER_INDEX = "server_index";
const std::string DBSceneLine::SCENE = "scene";
const std::string DBSceneLine::Scene::SCENE_ID = "scene_id";
const std::string DBSceneLine::Scene::TYPE = "type";
const std::string DBSceneLine::Scene::MAX_LINE = "max_line";
const std::string DBSceneLine::Scene::PER_PLAYER = "per_player";
// }}}

// DBBackBroDetail////////////////
// {{{
const std::string DBBackBroDetail::COLLECTION = "backstage.brocast";
const std::string DBBackBroDetail::ID = "id";
const std::string DBBackBroDetail::DB_OP_TYPE = "db_op_type";
const std::string DBBackBroDetail::DATA_CHANGE = "data_change";
const std::string DBBackBroDetail::BRO_RECORD = "bro_record";

const std::string DBBackBroDetail::Bro_record::BRO_TYPE = "bro_type";
const std::string DBBackBroDetail::Bro_record::BRO_TICK = "bro_tick";
const std::string DBBackBroDetail::Bro_record::BRO_TIMES = "bro_times";
const std::string DBBackBroDetail::Bro_record::REPEAT_TIMES = "repeat_times";
const std::string DBBackBroDetail::Bro_record::INTERVAL_SEC = "interval_sec";
const std::string DBBackBroDetail::Bro_record::CONTENT = "content";
// }}}


// DBBackNotice////////////////
const std::string DBBackNotice::COLLECTION = "backstage.notice";
const std::string DBBackNotice::ID = "id";
const std::string DBBackNotice::NOTIFY = "notify";
const std::string DBBackNotice::TITLE = "title";
const std::string DBBackNotice::CONTENT = "content";
const std::string DBBackNotice::TICK = "tick";

const std::string DBBackNotice::START_TICK = "start_tick";
const std::string DBBackNotice::ITEM_SET = "item_set";

// DBBackRole/////////////////////
const std::string DBBackRole::COLLECTION = "backstage.role";
const std::string DBBackRole::SERVER_FLAG = "server_flag";
const std::string DBBackRole::MARKET = "market";
const std::string DBBackRole::PLATFORM = "platform";
const std::string DBBackRole::AGENT = "agent";
const std::string DBBackRole::PLATFORM_CODE = "platform_code";
const std::string DBBackRole::AGENT_CODE = "agent_code";
const std::string DBBackRole::NET_TYPE = "net_type";
const std::string DBBackRole::SYS_VERSION = "sys_version";
const std::string DBBackRole::SYS_MODEL = "sys_model";
const std::string DBBackRole::MAC = "mac";
const std::string DBBackRole::IP = "ip";
const std::string DBBackRole::IMEI = "imei";
const std::string DBBackRole::CREATE_MARKET_CODE = "create_market_code";
const std::string DBBackRole::CREATE_AGENT = "create_agent";
const std::string DBBackRole::CREATE_AGENT_CODE = "create_agent_code";
const std::string DBBackRole::CREATE_NET_TYPE = "create_net_type";
const std::string DBBackRole::CREATE_SYS_VERSION = "create_sys_version";
const std::string DBBackRole::CREATE_SYS_MODEL = "create_sys_model";
const std::string DBBackRole::CREATE_MAC = "create_mac";
const std::string DBBackRole::CREATE_IP = "create_ip";
const std::string DBBackRole::CREATE_IMEI = "create_imei";
const std::string DBBackRole::IS_NEW_MAC = "is_new_mac";
const std::string DBBackRole::ROLE_ID = "role_id";
const std::string DBBackRole::ACCOUNT = "account";
const std::string DBBackRole::ROLE_NAME = "role_name";
const std::string DBBackRole::CAREER = "career";
const std::string DBBackRole::SEX = "sex";
const std::string DBBackRole::LEVEL = "level";
const std::string DBBackRole::FIGHT_FORCE = "fight_force";
const std::string DBBackRole::SCENE_ID = "scene_id";
const std::string DBBackRole::COORD_X = "coord_x";
const std::string DBBackRole::COORD_Y = "coord_y";
const std::string DBBackRole::CREATE_TIME = "create_time";
const std::string DBBackRole::EXPERIENCE = "experience";
const std::string DBBackRole::LEAGUE_ID = "league_id";
const std::string DBBackRole::LEAGUE_NAME = "league_name";
const std::string DBBackRole::FIGHTER_PROP = "fighter_prop";

const std::string DBBackRole::BIND_COPPER = "bind_copper";
const std::string DBBackRole::BIND_GOLD = "bind_gold";
const std::string DBBackRole::COPPER = "copper";
const std::string DBBackRole::GOLD = "gold";
const std::string DBBackRole::GOLD_USE = "gold_use";
const std::string DBBackRole::COUPON_USE = "coupon_use";
const std::string DBBackRole::COPPER_USE = "copper_use";
const std::string DBBackRole::BIND_COPPER_USE = "bind_copper_use";

const std::string DBBackRole::LAST_SIGN_IN_TIME = "last_sign_in_time";
const std::string DBBackRole::LAST_SIGN_OUT_TIME = "last_sign_out_time";
const std::string DBBackRole::LOGIN_COUNT = "login_count";
const std::string DBBackRole::ON_HOOK = "on_hook";
const std::string DBBackRole::ONLINE = "online";
const std::string DBBackRole::PERMISSION = "permission";
const std::string DBBackRole::RECHARGE_FIRST = "recharge_first";
const std::string DBBackRole::RECHARGE_GOLD = "recharge_gold";
const std::string DBBackRole::VIP = "vip";
const std::string DBBackRole::VIP_DEADLINE = "vip_deadline";
const std::string DBBackRole::VIP_START_TIME = "vip_start_time";
const std::string DBBackRole::KILL_NUM = "kill_num";
const std::string DBBackRole::KILL_VALUE = "kill_value";

// DBBackAccount/////////////////
const std::string DBBackAccount::COLLECTION = "backstage.account";
const std::string DBBackAccount::ACCOUNT = "account";
const std::string DBBackAccount::PERMISSION = "permission";
const std::string DBBackAccount::CREATE = "create";
const std::string DBBackAccount::CREATE_TIME = "create_time";


// DBBackRecharge////////////////
const std::string DBBackRecharge::COLLECTION = "backstage.order";
const std::string DBBackRecharge::ID = "id";
const std::string DBBackRecharge::FLAG = "flag";
const std::string DBBackRecharge::ORDER_NUM = "order_num";
const std::string DBBackRecharge::RECHARGE_CHANNEL = "channel_id";
const std::string DBBackRecharge::RECHARGE_MONEY = "money";
const std::string DBBackRecharge::RECHANGE_GOLD = "gold";
const std::string DBBackRecharge::ACCOUNT = "account";
const std::string DBBackRecharge::ROLE_ID = "role_id";
const std::string DBBackRecharge::RECHARGE_TICK = "time";
const std::string DBBackRecharge::RECHARGE_RANK_TAG = "rank_tag";

// DBBackMail////////////////
// {{{
const std::string DBBackMail::COLLECTION = "backstage.mail";
const std::string DBBackMail::ID = "id";
const std::string DBBackMail::READ = "read";
const std::string DBBackMail::TIME = "time";
const std::string DBBackMail::RECEIVER_SET = "receiver_set";
const std::string DBBackMail::SENDER = "sender";
const std::string DBBackMail::TITLE = "title";
const std::string DBBackMail::CONTENT = "content";

const std::string DBBackMail::MONEY = "money";
const std::string DBBackMail::Money::GOLD = "gold";
const std::string DBBackMail::Money::BIND_GOLD = "bind_gold";
const std::string DBBackMail::Money::COPPER = "copper";
const std::string DBBackMail::Money::BIND_COPPER = "bind_copper";

const std::string DBBackMail::ITEM = "item";
const std::string DBBackMail::Item::ITEM_ID = "item_id";
const std::string DBBackMail::Item::ITEM_AMOUNT = "item_amount";
const std::string DBBackMail::Item::ITEM_BIND = "item_bind";
const std::string DBBackMail::Item::ITEM_NAME = "item_name";
// }}}


const std::string DBBackActivity::COLLECTION = "backstage.activity";
const std::string DBBackActivity::AGENT = "agent";
const std::string DBBackActivity::ACT_INDEX = "act_index";
const std::string DBBackActivity::OPEN_FLAG = "open_flag";
const std::string DBBackActivity::COND_TYPE = "cond_type";
const std::string DBBackActivity::START_COND = "start_cond";
const std::string DBBackActivity::MAIL_ID = "mail";
const std::string DBBackActivity::RED_EVENT = "red_point";
const std::string DBBackActivity::SPECIAL_NOTIFY = "special_notify";
const std::string DBBackActivity::SORT = "sort";
const std::string DBBackActivity::REWARD_TYPE = "reward_type";
const std::string DBBackActivity::FIRST_TYPE = "first_type";
const std::string DBBackActivity::SECOND_TYPE = "second_type";
const std::string DBBackActivity::START_TICK = "act_start";
const std::string DBBackActivity::STOP_TICK = "act_end";
const std::string DBBackActivity::UPDATE_TICK = "update_tick";
const std::string DBBackActivity::REWARD_START = "reward_start";
const std::string DBBackActivity::REWARD_END = "reward_end";
const std::string DBBackActivity::CYCLE_TIMES = "cycle_times";
const std::string DBBackActivity::OPEN_TIME = "open_time";
const std::string DBBackActivity::LIMIT = "limit";
const std::string DBBackActivity::REDRAW = "redraw";
const std::string DBBackActivity::DAY_CLEAR = "day_clear";
const std::string DBBackActivity::RECORD_VALUE = "record_value";
const std::string DBBackActivity::ACT_TITLE = "act_title";
const std::string DBBackActivity::ACT_CONTENT = "act_content";
const std::string DBBackActivity::REWARD = "reward";
const std::string DBBackActivity::DRAWED = "drawed";
const std::string DBBackActivity::T_SUB_MAP = "t_sub_map";
const std::string DBBackActivity::F_RANK_INFO = "f_rank_info";
const std::string DBBackActivity::MAIL_TITLE = "mail_title";
const std::string DBBackActivity::MAIL_CONTENT = "mail_content";
const std::string DBBackActivity::PRIORITY = "priority";
const std::string DBBackActivity::ICON_TYPE = "icon_type";

const std::string DBBackActivity::CORNUCOPIA_RECHARGE = "cornucopia_recharge";
const std::string DBBackActivity::CornucopiaRecharge::ROLE_ID = "role_id";
const std::string DBBackActivity::CornucopiaRecharge::PLAYER_NAME = "player_name";
const std::string DBBackActivity::CornucopiaRecharge::GET_TIME = "get_time";
const std::string DBBackActivity::CornucopiaRecharge::CORNUCOPIA_GOLD ="cornucopia_gold" ;
const std::string DBBackActivity::CornucopiaRecharge::REWARD_MULT = "reward_mult";


const std::string DBBackActivity::Reward::TYPE = "type";
const std::string DBBackActivity::Reward::COND = "cond";
const std::string DBBackActivity::Reward::COST_ITEM = "cost_item";
const std::string DBBackActivity::Reward::PRE_COST = "pre_cost";
const std::string DBBackActivity::Reward::CONTENT = "content";
const std::string DBBackActivity::Reward::ITEM = "item";
const std::string DBBackActivity::Reward::BROCAST = "brocast";
const std::string DBBackActivity::Reward::TIMES = "times";
const std::string DBBackActivity::Reward::MUST_RESET = "must_reset";
const std::string DBBackActivity::Reward::HANDLE_TYPE = "handle_type";
const std::string DBBackActivity::Reward::CASH_COUPON = "cash_coupon";
const std::string DBBackActivity::Reward::REWARD_ID = "reward_id";
const std::string DBBackActivity::Reward::REWARD_TYPE = "reward_type";
const std::string DBBackActivity::Reward::REWARD_START_COND = "reward_start_cond";
const std::string DBBackActivity::Reward::EXCHANGE_TYPE = "exchange_type";
const std::string DBBackActivity::Reward::EXCHANGE_ITEM_NAME = "exchange_item_name";
const std::string DBBackActivity::Reward::SUB_MAP = "sub_map";
const std::string DBBackActivity::Reward::RECHARGE_MAP = "recharge_map";
const std::string DBBackActivity::Reward::DRAWED_MAP = "drawed_map";

//////////////////////DBBackWonderfulActivity
const std::string DBBackWonderfulActivity::COLLECTION = "backstage.wonderful_activity";	//表名：wonderful_activity
const std::string DBBackWonderfulActivity::ACTIVITY_ID = "activity_id";	//活动id 例如50101 50201 50301
const std::string DBBackWonderfulActivity::DATE_TYPE = "date_type";
const std::string DBBackWonderfulActivity::FIRST_DATE = "first_date";
const std::string DBBackWonderfulActivity::LAST_DATE = "last_date";
const std::string DBBackWonderfulActivity::OPEN_FLAG = "open_flag";
const std::string DBBackWonderfulActivity::REFRESH_RICK = "fefresh_tick";
const std::string DBBackWonderfulActivity::AGENT = "agent";
const std::string DBBackWonderfulActivity::SORT = "sort";
const std::string DBBackWonderfulActivity::ACTIVITY_TYPE = "activity_type";
const std::string DBBackWonderfulActivity::ACT_CONTENT = "act_content";
const std::string DBBackWonderfulActivity::VALUE1 = "value1";
const std::string DBBackWonderfulActivity::VALUE2 = "value2";

//////////////////////DBFestActivity
const std::string DBFestActivity::COLLECTION = "backstage.fest_act";
const std::string DBFestActivity::ID = "id";	// id == 0
const std::string DBFestActivity::ICON_TYPE = "icon_type";
const std::string DBFestActivity::STATCK_TICK = "start_tick";
const std::string DBFestActivity::END_TICK = "end_tick";
const std::string DBFestActivity::UPDATE_TICK = "update_tick";

//////////////////////DBBackMayActivity
const std::string DBBackMayActivity::COLLECTION = "backstage.may_act";
const std::string DBBackMayActivity::ID = "id";
const std::string DBBackMayActivity::OPEN_FLAG = "open_flag";
const std::string DBBackMayActivity::REFRESH_TICK = "refresh_tick";
const std::string DBBackMayActivity::BEGIN_DATE = "begin_date";
const std::string DBBackMayActivity::END_DATE = "end_date";
const std::string DBBackMayActivity::ACT_TYPE = "act_type";
const std::string DBBackMayActivity::ACT_SET = "act_set";
const std::string DBBackMayActivity::AGENT = "agent";

//////////////////////ActiCode
const std::string DBBackActiCode::COLLECTION = "backstage.acti_code";
const std::string DBBackActiCode::BACKUP_COLLECTION = "backstage.acti_code_backup";
const std::string DBBackActiCode::ID = "id";
const std::string DBBackActiCode::USER_ID = "user_id";
const std::string DBBackActiCode::GIFT_SORT = "gift_sort";
const std::string DBBackActiCode::AMOUNT = "amount";
const std::string DBBackActiCode::START_TIME = "start_time";
const std::string DBBackActiCode::END_TIME = "end_time";
const std::string DBBackActiCode::USED_TIME = "used_time";
const std::string DBBackActiCode::BATCH_ID = "batch_id";
const std::string DBBackActiCode::USE_ONLY_VIP = "use_only_vip";
const std::string DBBackActiCode::ACTI_CODE = "acti_code";

// MediaGiftDetail////////////////////
// {{{
const std::string DBBackMediaGiftDef::COLLECTION = "backstage.media_gift_def";
const std::string DBBackMediaGiftDef::UPDATE_STATUS = "update_status";
const std::string DBBackMediaGiftDef::UPDATE_TICK = "update_tick";
const std::string DBBackMediaGiftDef::GIFT_SORT = "gift_sort";
const std::string DBBackMediaGiftDef::GIFT_TYPE = "gift_type";
const std::string DBBackMediaGiftDef::GIFT_TAG = "gift_tag";
const std::string DBBackMediaGiftDef::USE_TIMES = "use_times";
const std::string DBBackMediaGiftDef::SHOW_ICON = "show_icon";
const std::string DBBackMediaGiftDef::HIDE_USED = "hide_used";
const std::string DBBackMediaGiftDef::IS_SHARE = "is_share";
const std::string DBBackMediaGiftDef::EXPIRE_TIME = "expire_time";
const std::string DBBackMediaGiftDef::GIFT_NAME = "gift_name";
const std::string DBBackMediaGiftDef::GIFT_DESC = "gift_desc";
const std::string DBBackMediaGiftDef::VALUE_EXTS = "value_exts";
const std::string DBBackMediaGiftDef::FONT_COLOR = "font_color";
const std::string DBBackMediaGiftDef::ValueExts::VALUE_EXT = "value";
const std::string DBBackMediaGiftDef::GIFT_ITEMS = "gift_items";

const std::string DBBackMediaGiftDef::ItemObj::ITEM_ID = "item_id";
const std::string DBBackMediaGiftDef::ItemObj::ITEM_AMOUNT = "item_amount";
const std::string DBBackMediaGiftDef::ItemObj::ITEM_BIND = "item_bind";
const std::string DBBackMediaGiftDef::ItemObj::ITEM_INDEX = "item_index";
// }}}


//49you-luoshenyu-box//////////////////////
//{{{
const std::string DBBackDownLoadBoxGift::COLLECTION = "backstage.download_box_gift";
const std::string DBBackDownLoadBoxGift::AGENT_CODE = "agent_code";
const std::string DBBackDownLoadBoxGift::DOWNLOAD_URL = "download_url";
const std::string DBBackDownLoadBoxGift::ITEM_LIST = "item_list";
const std::string DBBackDownLoadBoxGift::ItemObj::ITEM_ID = "item_id";
const std::string DBBackDownLoadBoxGift::ItemObj::ITEM_AMOUNT = "item_amount";
const std::string DBBackDownLoadBoxGift::ItemObj::ITEM_BIND = "item_bind";
//}}}

// BackCustomerSVCRecord////////////////
// {{{
const std::string BackCustomerSVCRecord::COLLECTION = "backstage.customer_service";
const std::string BackCustomerSVCRecord::ID = "id";
const std::string BackCustomerSVCRecord::SENDER_ID = "sender_id";
const std::string BackCustomerSVCRecord::SEND_TICK = "send_tick";
const std::string BackCustomerSVCRecord::RECORD_TYPE = "record_type";
const std::string BackCustomerSVCRecord::HAS_REPLAY = "has_replay";
const std::string BackCustomerSVCRecord::HAS_READ = "has_read";
const std::string BackCustomerSVCRecord::NEED_LOAD_DATA = "need_load_data";
const std::string BackCustomerSVCRecord::SENDER_NAME = "sender_name";
const std::string BackCustomerSVCRecord::CONTENT = "content";
const std::string BackCustomerSVCRecord::TITLE = "title";
const std::string BackCustomerSVCRecord::REPLAY_CONTENT = "replay_content";
const std::string BackCustomerSVCRecord::SENDER_LEVEL = "sender_level";
const std::string BackCustomerSVCRecord::SERVER_CODE = "server_code";
const std::string BackCustomerSVCRecord::PLATFORM = "platform";
const std::string BackCustomerSVCRecord::AGENT = "agent";
const std::string BackCustomerSVCRecord::RECHARGE_GOLD = "recharge_gold";
const std::string BackCustomerSVCRecord::REMOVE_FLAG = "remove_flag";
const std::string BackCustomerSVCRecord::EVALUATE_TICK = "evaluate_tick";
const std::string BackCustomerSVCRecord::EVALUATE_LEVEL = "evaluate_level";
const std::string BackCustomerSVCRecord::EVALUATE_STAR = "evaluate_star";
const std::string BackCustomerSVCRecord::OPINION_INDEX = "opinion_index";
// }}}


// BackStageDailyRecharge
/// {{{
const int DBBackDailyRecharge::OPEN_TIME_ID = 1;

const std::string DBBackDailyRecharge::COLLECTION = "backstage.daily_charge";
const std::string DBBackDailyRecharge::ID = "id";
const std::string DBBackDailyRecharge::START_TIME = "start_time";
const std::string DBBackDailyRecharge::END_TIME = "end_time";
/// }}}


// BackRestriction /////////////////////////
const std::string DBBackRestriction::COLLECTION = "backstage.ban";
const std::string DBBackRestriction::ID = "id";
const std::string DBBackRestriction::ACCOUNT = "account";
const std::string DBBackRestriction::ROLE_NAME = "role_name";
const std::string DBBackRestriction::ROLE_ID = "role_id";
const std::string DBBackRestriction::IP_ADDR = "ip";
const std::string DBBackRestriction::DESC = "reason";
const std::string DBBackRestriction::MANAGER = "gm";
const std::string DBBackRestriction::OPERATION = "type";
const std::string DBBackRestriction::OPER_TYPE = "action";
const std::string DBBackRestriction::FLAG = "flag";
const std::string DBBackRestriction::EXPIRED_TIME = "time";
const std::string DBBackRestriction::CREATED_TIME = "created_time";
const std::string DBBackRestriction::MAC = "mac";

const std::string DBBanIpInfo::COLLECTION = "backstage.ban_ip";
const std::string DBBanIpInfo::IP_UINT = "ip_uint";
const std::string DBBanIpInfo::IP_STRING = "ip_str";
const std::string DBBanIpInfo::EXPIRED_TIME = "expired_time";

const std::string DBWhiteIpInfo::COLLECTION = "backstage.white_ip";
const std::string DBWhiteIpInfo::IP_UINT = "white_ip_uint";
const std::string DBWhiteIpInfo::IP_STRING = "white_ip_str";

const std::string DBBanMacInfo::COLLECTION = "backstage.ban_mac";
const std::string DBBanMacInfo::MAC_STRING = "mac_str";
const std::string DBBanMacInfo::MAC = "mac";
const std::string DBBanMacInfo::EXPIRED_TIME = "expired_time";


// DBBackSwitcher
const std::string DBBackSwitcher::COLLECTION = "backstage.game_switch";
const std::string DBBackSwitcher::NAME = "name";
const std::string DBBackSwitcher::VALUE = "value";

// DBBackModify
const std::string DBBackModify::COLLECTION = "backstage.game_modify";
const std::string DBBackModify::NAME = "name";
const std::string DBBackModify::IS_UPDATE = "is_update";
const std::string DBBackModify::ROLE_ID = "role_id";
const std::string DBBackModify::LEAGUE_ID = "league_id";
const std::string DBBackModify::VALUE = "value";
const std::string DBBackModify::ValueEle::QQ_NUM = "qq";
const std::string DBBackModify::ValueEle::DES_CONTENT = "des_content";
const std::string DBBackModify::ValueEle::DES_MAIL = "des_mail";
const std::string DBBackModify::ValueEle::VIP_LEVEL_LIMIT = "vip_level_limit";
const std::string DBBackModify::ValueEle::RECHARGE = "recharge";
const std::string DBBackModify::START_TICK = "start_tick";
const std::string DBBackModify::END_TICK = "end_tick";
const std::string DBBackModify::OPEN_STATE = "open_state";
const std::string DBBackModify::ACTIVITY_ID = "activity_id";

const std::string DBBackDraw::COLLECTION = "backstage.draw";
const std::string DBBackDraw::FLAG = "flag";
const std::string DBBackDraw::SERVER_INDEX = "server_index";
const std::string DBBackDraw::ACTIVITY_ID = "activity_id";
const std::string DBBackDraw::S_TICK = "s_tick";
const std::string DBBackDraw::E_TICK = "e_tick";

const std::string DBBackContactWay::COLLECTION = "backstage.contact_way";
const std::string DBBackContactWay::MARKET_CODE = "market_code";
const std::string DBBackContactWay::CONTACT_WAY = "contact_way";

/////////////////////DBServerInfo/////////////////////
const std::string DBServerInfo::COLLECTION = "backstage.server_info";
const std::string DBServerInfo::INDEX = "index";
const std::string DBServerInfo::SERVER_ID = "server_id";
const std::string DBServerInfo::IS_IN_USE = "is_in_use";
const std::string DBServerInfo::SERVER_FLAG = "server_flag";
const std::string DBServerInfo::CUR_SERVER_FLAG = "cur_server_flag";
const std::string DBServerInfo::SERVER_PREV = "server_prev";
const std::string DBServerInfo::SERVER_NAME = "server_name";
const std::string DBServerInfo::FRONT_SERVER_NAME = "front_server_name";
const std::string DBServerInfo::OPEN_SERVER = "open_server";
const std::string DBServerInfo::COMBINE_SERVER_SET = "combine_server_set";
const std::string DBServerInfo::COMBINE_TO_SERVER_ID = "combine_to_server_id";

/////////////////////DBCombineServer/////////////////////
const std::string DBCombineServer::COLLECTION = "backstage.combine_server";
const std::string DBCombineServer::ID = "id";

const std::string DBCombineServer::SERVER_FLAG = "server_flag";
const std::string DBCombineServer::IP = "ip";
const std::string DBCombineServer::PORT = "port";
const std::string DBCombineServer::SPECIAL_FILE = "special_file";
const std::string DBCombineServer::UPDATE_TICK = "update_tick";

/////////////////////////DBChatLimit/////////////////////
const std::string DBChatLimit::COLLECTION = "backstage.chatlimit";
const std::string DBChatLimit::ID = "id";
const std::string DBChatLimit::LIMIT_LEVEL = "limit_level";
const std::string DBChatLimit::CHAT_INTREVAL = "chat_interval";
const std::string DBChatLimit::UPDATE_TICK = "update_tick";

//////////////////////////DBCharCheck/////////////////////////
const std::string DBWordCheck::COLLECTION = "backstage.sensitive_words";
const std::string DBWordCheck::ID = "id";
const std::string DBWordCheck::LIST = "list";
const std::string DBWordCheck::TIME = "time";

//////////////////////////DBVipChat/////////////////////////
const std::string DBVipChat::COLLECTION = "backstage.vipchatlimit";
const std::string DBVipChat::ID = "id";
const std::string DBVipChat::UPDATE_TICK = "update_tick";
const std::string DBVipChat::TIME = "time";

const std::string DBVipChat::DETAIL = "detail";
const std::string DBVipChat::VipLimit::VIP_LV = "vip_lv";
const std::string DBVipChat::VipLimit::INFO = "info";

/////////////////////////DBJYBackActivity///////////////////
const std::string DBJYBackActivity::COLLECTION = "backstage.jyback_activity";
const std::string DBJYBackActivity::ACT_ID = "act_id";
const std::string DBJYBackActivity::UPDATE_FLAG = "update_flag";
const std::string DBJYBackActivity::UPDATE_TICK = "update_tick";
const std::string DBJYBackActivity::FIRST_TYPE = "first_type";
const std::string DBJYBackActivity::SECOND_TYPE = "second_type";
const std::string DBJYBackActivity::ACT_TITLE = "act_title";
const std::string DBJYBackActivity::ACT_CONTENT = "act_content";
const std::string DBJYBackActivity::ACT_START = "act_start";
const std::string DBJYBackActivity::ACT_END = "act_end";
const std::string DBJYBackActivity::IS_OPEN = "is_open";
const std::string DBJYBackActivity::ORDER = "order";
const std::string DBJYBackActivity::REWARD_MAIL_TITLE = "reward_mail_title";
const std::string DBJYBackActivity::REWARD_MAIL_CONTENT = "reward_mail_content";
const std::string DBJYBackActivity::NEED_GOLD = "need_gold";
const std::string DBJYBackActivity::REWARD = "reward";
const std::string DBJYBackActivity::Reward::COND_TYPE = "cond_type";
const std::string DBJYBackActivity::Reward::COND = "cond";
const std::string DBJYBackActivity::Reward::REWARD_TYPE = "reward_type";
const std::string DBJYBackActivity::Reward::REWARD_ITEM = "reward_item";
const std::string DBJYBackActivity::Reward::RETURN_GOLD_RATE = "return_gold_rate";

////////////////////////DBCorrectTrvlRank/////////////////
const std::string DBCorrectTrvlRank::COLLECTION = "backstage.correct_trvlrank";
const std::string DBCorrectTrvlRank::UPDATE_FLAG = "update_flag";
const std::string DBCorrectTrvlRank::ID = "id";
const std::string DBCorrectTrvlRank::TYPE = "type";
const std::string DBCorrectTrvlRank::OP_TYPE = "op_type";
const std::string DBCorrectTrvlRank::AMOUNT = "amount";
const std::string DBCorrectTrvlRank::TICK = "tick";
const std::string DBCorrectTrvlRank::ACTIVITY_ID = "activity_id";

const std::string DBCorrectTrvlRank::SERVER = "server";
const std::string DBCorrectTrvlRank::ROLE = "role";

