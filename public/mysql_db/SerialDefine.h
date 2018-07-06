/*
 * SerialDefine.h
 *
 * Created on: 2013-05-04 11:26
 *     Author: lyz
 */

#ifndef _SERIALDEFINE_H_
#define _SERIALDEFINE_H_

enum GameSerial
{
	//增加流水
	ADD_FROM_OPEN_ACTIV                 = 1001,		//开服活动
	ADD_FROM_LOCAL_TEST	     			= 1002,     //本地测试添加
	ADD_FROM_RETURN_ACTIV				= 1003,		//返利活动
	ADD_FROM_MAIL_ITEM                	= 1004,    	//邮件获得
    ADD_FROM_TASK_MONEY             	= 1005,     //任务获得货币
    ADD_RROM_TASK_ITEM                	= 1006,    	//任务获得物品
    ADD_FROM_BACK_RECHARGE         		= 1007,     //后台充值
    ADD_FROM_TAKE_OFF_JEWEL            	= 1008,    	//宝石替换
    ADD_FORM_USE_ITEM					= 1009,		//使用道具
    ADD_FROM_SM_BATTLE					= 1010,		//杀戮战场积分奖励
    ADD_FROM_SM_BATTLE_FINISH			= 1011,		//杀戮战场结束奖励
    ADD_FROM_MOUNT_GRADE				= 1012,		//战骑类进阶奖励
    ADD_FROM_LEAGUE_SALARY         		= 1013,     //帮派俸禄
    ADD_FROM_FROM_WB_POCKET				= 1015,		//世界boss抢红包
    ADD_FROM_MONSTER_DROP               = 1016,    	//怪物掉落
    ADD_FROM_SCRIPT_CLEAN               = 1017,     //副本扫荡奖励，子类型：副本ID
    ADD_FROM_SCRIPT_FIRST_PASS          = 1018,     //副本首次通关奖励，子类型：副本ID
    ADD_FROM_SCRIPT_PASS                = 1019,     //副本通关奖励
    ADD_FROM_SUPER_SUMMON 				= 1020,		//超级召唤
    ADD_FROM_LEAGUE_BOSS_DIRECT 		= 1021,		//帮派boss直接奖励
    ADD_FROM_MONSTER_ATTACK_KILL		= 1022,		//怪物攻城击杀小怪
    ADD_FROM_MONSTER_ATTACK_WAVE_AWARD 	= 1023,		//怪物攻城波数奖励
    ADD_FROM_WBOSS_DICE_MAX				= 1024,		//世界boss投最大数字
    ADD_FROM_LWAR_ROLE_SCORE			= 1025,		//帮派争霸个人积分
    ADD_FROM_SEVEN_DAY_AWARD			= 1026,		//七天奖励
    ADD_FROM_SWORD_POOL_UP				= 1027,		//剑池升级
    ADD_FROM_VIP_RETURN					= 1029,		//VIP返还
    ADD_FROM_LEAGUE_WELFARE				= 1030,		//帮派福利
    ADD_FROM_ITEM_COMPOSE        		= 1031,    	//装备合成
    ADD_FROM_ITEM_GIFT_PACK             = 1032,    	//使用礼包
    ADD_FROM_ITEM_GATHER                = 1033,    	//采集获得
    ADD_FROM_ITEM_MALL_BUY              = 1034,    	//商城购买
    ADD_FROM_ITEM_DAILY_RECHARGE		= 1035,		//每日充值
    ADD_FROM_ITEM_REBATE_RECHARGE		= 1036,		//百倍返利
    ADD_FROM_ITEM_INVEST_RECHARGE		= 1037,		//投资计划
    ADD_FROM_TRAVEL_ARENA_WIN			= 1038,		//跨服竞技赢
    ADD_FROM_TRAVEL_ARENA_LOSE			= 1039,		//跨服竞技输
    ADD_FROM_TRAVEL_ARENA_DAY			= 1040,		//跨服竞技天
    ADD_FROM_GATHER_REWARD				= 1041,		//采集宝箱
    ADD_FROM_RESTORE					= 1042,		//资源找回
    ADD_FROM_VIP_GIFT                   = 1043,    	//VIP奖励
    ADD_FROM_ONLINE_REWARDS             = 1044,    	//在线奖励
    ADD_FROM_CHECKI_IN_AWARD            = 1045,    	//签到奖励
    ADD_FROM_GAME_NOTICE                = 1046,    	//公告奖励
    ADD_FROM_COLLECT_CHESTS				= 1047,    	//采集宝箱额外奖励
    ADD_FROM_ANSWER_ACTIVITY			= 1048,	   	//答题活动
    ADD_FROM_DAILY_ROUTINE_TASK			= 1049,	   	//日常环式任务
    ADD_FROM_ARENA						= 1050,	   	//江湖榜
    ADD_FROM_ESCORT_MOST_LEVEL			= 1051,	   	//护送最高等级奖励
    ADD_FROM_TARENA_WIN_TIMES			= 1052,		//跨服竞技赢次数
    ADD_FROM_OPINION_RECORD				= 1053,		//意见反馈
    ADD_FROM_RECHARGE_ITEM         		= 1054,		//充值道具
    ADD_FROM_UPDAET_RES                 = 1055,    	//资源包下载完成
    ADD_FROM_GIFT_DRAW                 	= 1056,    	//礼包领取
    ADD_FROM_ACHIEVE_REWARD				= 1057,		//成就奖励
    ADD_FROM_RED_CLOTHES_EXCHANGE		= 1058,		//红装碎片兑换
    ADD_FROM_RETURN_ACTIVITY_BUY		= 1059,		//返利活动兑换/购买获得
    ADD_FROM_HIDDEN_TREASURE_GET		= 1060,		//藏宝阁奖励
    ADD_FROM_HIDDEN_TREASURE_BUY		= 1061,		//藏宝阁购买
    ADD_FROM_BACK_RECHARGE_EXT			= 1062,		//充值额外奖励
    ADD_FROM_BACK_RECHARGE_ACT			= 1063,		//开服活动充值奖励
    ADD_FROM_TREASURES_GAME				= 1064,		//旋即宝匣
    ADD_FROM_LUCKY_WHEEL_REWARD			= 1065,		//幸运大轮盘抽奖获得
    ADD_FROM_LUCKY_WHEEL_EXCHANGE		= 1066,		//幸运大轮盘兑换获得
    ADD_FROM_MAIN_TASK					= 1067,		//主线任务
    ADD_FROM_LEAGUE_ROUTINE_TASK		= 1068,	   	//帮派环式任务
    ADD_FROM_OFFER_ROUTINE_TASK			= 1069,	   	//悬赏环式任务
    ADD_FROM_GOLD_BOX_REWARD			= 1070,		//元宝宝匣抽奖获得
    ADD_FROM_ADVANCE_BOX_REWARD			= 1071,		//进阶宝匣抽奖获得
    ADD_FROM_TIME_LIMIT_ACTIVITY 		= 1072,		//限时秒杀购买获得
    ADD_FROM_TIME_LIMIT_REWARD			= 1073,		//限时秒杀奖励获得
    ADD_FROM_CABINET_ACTIVITY			= 1074,		//藏珍阁物品获得
    ADD_FROM_WEDDING					= 1075, 	//结婚奖励
    ADD_FROM_WEDDING_TREASURES			= 1076,		//爱情宝匣
    ADD_FROM_COUPLE_ACT					= 1077,		//结婚活动获得
    ADD_FROM_WEDDING_FASHION			= 1078,		//结婚礼服相同增加
    ADD_FROM_WEDDING_LABEL				= 1079,		//结婚称号
    ADD_FROM_MAZE_TREASURE				= 1080,		//迷宫寻宝
    ADD_FROM_NINE_WORD_ALL_OPEN			= 1081,		//密宗九字一键点亮奖励
    ADD_FROM_NINE_WORD_OPEN_SLOT 		= 1082,		//密宗九字点开格子奖励
    ADD_FROM_LIGHTEN_SIX_WORD			= 1083,		//密宗九字点亮6字奖励
    ADD_FROM_NINE_WORD_ACT_REWARD		= 1084,		//密宗九字真言奖励
    ADD_FROM_IMMORTAL_TREASURES			= 1085,		//神仙鉴宝奖励
    ADD_FROM_LUCKY_EGG_OPEN				= 1086,		//幸运砸蛋开蛋获得
    ADD_FROM_LUCKY_EGG_REWARD			= 1087,		//幸运砸蛋奖励获得
    ADD_FROM_IMMORTAL_TREASURES_RETURN	= 1088,		//神仙鉴宝返还奖励
    ADD_FROM_OPEN_GIFT_REWARD			= 1089,		//开服豪礼领取奖励
    ADD_FROM_LEAGUE_FB_BOX				= 1090,		//帮派副本宝箱奖励
    ADD_FROM_COBMINE_ACTIV				= 1091,		//合服活动
    ADD_FROM_RETURN_COMBINE				= 1092,		//合服返利活动
    ADD_FROM_FESTIVE_ACTIV				= 1093,		//节日活动
    ADD_FROM_SECRET_EXCHANGE			= 1094,		//密卷兑换
    ADD_FROM_LEGEND_EXCHANGE			= 1095,		//传说图谱兑换
    ADD_FROM_OFFLINE_HOOK				= 1096,		//离线挂机
    ADD_FROM_GASHAPON_ACTIV				= 1097, 	//扭蛋活动领取奖励
    ADD_FROM_SPIRIT_UP_LVL				= 1098,		//英魂升级获得
    ADD_FROM_SPIRIT_UP_STAGE			= 1099,		//英魂升阶获得
    ADD_FROM_BACK_ACTIVITY				= 1100,		//后台活动获得
    ADD_FROM_TRANSFER_OPEN				= 1101,		//变身系统开启奖励
    ADD_FROM_MAY_ACTIVITY				= 1102,		//五一活动获得
    ADD_FROM_MAY_ACT_BUY				= 1103,		//五一活动购买获得
    ADD_FROM_ESCORT_PROTECT				= 1104,		//护送保护
    ADD_FROM_DAILY_RUN_RUN				= 1105,		//进行天天跑酷增加
    ADD_FROM_DAILY_RUN_BUY				= 1106,		//天天跑酷购买增加
    ADD_FROM_DAILY_RUN_SEND				= 1107,		//天天跑酷赠送增加
    ADD_FROM_LRF_FLAG					= 1108,		//城池战斩旗奖
    ADD_FROM_DAILY_RUN_DIRECT			= 1109,		//天天跑酷直线奖励
    ADD_FROM_DAILY_RUN_JUMP				= 1110,		//天天跑酷跳跃奖励
    ADD_FROM_FASHION_TAKE_REWARD		= 1111,		//绝版时装抽到奖励
    ADD_FROM_FASHION_LIVENESS_REWARD	= 1112, 	//绝版时装活跃读奖励
    ADD_FROM_RED_PACKET_MONEY			= 1113, 	//抢红包金额
    ADD_FROM_LRF_SCORE					= 1114,		//城池战积分奖励
    ADD_FROM_LRF_FINISH					= 1115,		//城池战结束奖励
    ADD_FROM_LRF_LEADER					= 1116,		//城池福利帮主奖励
    ADD_FROM_LRF_DAILY					= 1117,		//城池福利日常奖励
    ADD_FROM_EQUIP_JEWEL_SUBLIME		= 1118,		//宝石升华奖励
    ADD_FROM_EQUIP_JEWEL_REMOVE			= 1119,		//卸下镶嵌的宝石
    ADD_FROM_TBATTLE_FINISH             = 1120,     //华山论战的历练奖励
    ADD_FROM_TBATTLE_FLOOR              = 1121,     //华山论战的下层奖励
    ADD_FROM_TBATTLE_SCORE              = 1122,     //华山论战的积分奖励
    ADD_FROM_TBATTLE_TREASURE           = 1123,     //华山论战的秘宝奖励
    ADD_FROM_TMARENA_FINISH				= 1124,		//争霸天下结束奖励
    ADD_FROM_CHANGE						= 1125,		//神秘兑换奖励
    ADD_FROM_FISH_SCORE_CHANGE			= 1126,		//积分兑换
    ADD_FROM_GET_FISH_REWARD			= 1127, 	//捕鱼奖励
    ADD_FROM_SPECIAL_BOX_REWARD			= 1128,		//神秘宝箱
    ADD_FROM_SPECIAL_BOX_CHANGE_REWARD  = 1129,		//神秘宝箱兑换奖励

	//扣除金钱和资源流水
	SUB_MONEY_BUY_TRANSFER				= 3001,		//传送自动购买
	SUB_MONEY_MOUNT_EVA             	= 3002,    	//坐骑进化自动购买
	SUB_MONEY_FAST_BUY_REFINE       	= 3003,     //强化自动购买
	SUB_MONEY_LOCAL_TEST	     		= 3004,     //本地测试删除
    SUB_MONEY_LEAGUE_CREATE         	= 3006,     //创建宗派
    SUB_MONEY_LEAGUE_DONATE         	= 3007,     //帮派捐献
    SUB_MONEY_SCRIPT_ADD_TIMES      	= 3008,     //副本购买次数
    SUB_MONEY_ESCORT_CAR            	= 3009,     //仙盟运镖
    SUB_MONEY_PESCORT_CAR           	= 3010,     //自动购买
    SUB_MONEY_SUMMON_BOSS 				= 3011, 	//帮派超级召唤boss消耗
    SUB_MONEY_SPOOL_FIND_BACK_COST 		= 3012,		//剑池经验找回消耗
    SUB_MONEY_ENTER_WBOSS_USE_FLY		= 3013,		//用钱飞进世界boss
    SUB_MONEY_FAST_FINISH_ROUTINE		= 3014,		//快速完成环式任务
    SUB_MONEY_MALL_BUY              	= 3015,     //商城购买
    SUB_MONEY_REBATE_RECHARGE			= 3016,		//百倍返利
    SUB_MONEY_INVEST_RECHARGE			= 3017,		//投资计划
    SUB_MONEY_RESTORE           		= 3018,     //找回资源
    SUB_MONEY_TASK_FAST_FINISH      	= 3019,     //快速完成任务
    SUB_MONEY_EXP_RESTORE           	= 3020,     //找回资源
    SUB_MONEY_AREA_TIMES            	= 3021,     //购买江湖榜次数
    SUB_MONEY_AREA_COOL             	= 3022,     //购买江湖榜冷却时间
    SUB_MONEY_PROMOTE_MAGICWEAPON   	= 3023,     //罗摩提升
    SUB_MONEY_TREASURES					= 3024,		//旋即宝匣
    SUB_MONEY_RETURN_ACTIVITY_BUY		= 3025,		//商店类活动购买消耗
    SUB_MONEY_HIDDEN_TREASURE_BUY		= 3026,		//藏宝阁购买
    SUB_MONEY_LUCKY_WHEEL_ACTIVITY		= 3027,		//幸运大转盘抽奖
    SUB_MONEY_ADVANCE_BOX_ACTIVITY		= 3028,		//进阶宝匣抽奖
    SUB_MONEY_GOLD_BOX_ACTIVITY			= 3029,		//元宝宝匣抽奖
    SUB_MONEY_ADVANCE_BOX_RESET			= 3030,		//进阶宝匣重置
    SUB_MONEY_TIME_LIMIT_ACTIVITY		= 3031, 	//限时秒杀购买消耗
    SUB_MONEY_REFRESH_CABINET			= 3032,		//藏珍阁刷新
    SUB_MONEY_BUY_CABINET				= 3033,		//藏珍阁购买
    SUB_MONEY_BUY_FASHION_COLOR 		= 3034,		//购买染色材料消耗
    SUB_MONEY_BUY_FLOWER				= 3035,		//鲜花购买
    SUB_MONEY_BUY_WEDDING_TREASURES		= 3036,		//购买爱情宝匣
    SUB_MONEY_WEDDING               	= 3037,     //举办婚礼消耗
    SUB_MONET_MAZE_TREASURE_ACTIVITY	= 3038,		//迷宫寻宝
    SUB_MONEY_NINE_WORD_ACTIVITY		= 3039,		//密宗九字开启格子消耗
    SUB_MONEY_NINE_WORD_RESET			= 3040,		//密宗九字重置消耗
    SUB_MONEY_DRAW_IMMORTAL				= 3041,		//神仙鉴宝鉴宝消耗
    SUB_MONEY_RAND_IMMORTAL				= 3042,		//神仙鉴宝换宝消耗
    SUB_MONEY_LUCKY_EGG_ACTIVITY		= 3043,		//幸运砸蛋进行砸蛋消耗
    SUB_MONEY_LUCKY_EGG_RESET			= 3044,		//幸运砸蛋重置消耗
    SUB_MONEY_MARKET_BUY            	= 3045,     //市场购买
    SUB_MONEY_SPIRIT_UP_LVL				= 3046,		//英魂聚精消耗
    SUB_MONEY_BUY_TRANSFER_ID 			= 3047,		//购买变身消耗
    SUB_MONEY_MAY_ACT_BUY				= 3048,		//五一活动购买消耗
    SUB_MONEY_HICKTY_BUY				= 3049,		//成池战购买
    SUB_MONEY_ENTER_TRVL_WBOSS_USE_FLY	= 3050,		//用钱飞进跨服世界boss
    SUB_MONEY_CREATE_TRAVTEAM       	= 3051,     //巅峰对决创建战队消耗
    SUB_MONEY_REFRESH_CABINET_DISCOUNT  = 3052,		//折扣商店刷新
    SUB_MONEY_BUY_CABINET_DISCOUNT		= 3053,		//折扣商店购买
    SUB_MONEY_FISH_REFRESH				= 3054,		//捕鱼刷新
    SUB_MONEY_GET_FISH					= 3055,		//捕鱼消耗
    SUB_MONEY_SPECIAL_BOX_OPEN_BOX		= 3056,		//神秘宝箱开箱消耗金币
    SUB_MONEY_GODDESS_BLESS				= 3057,		//女神赐福消耗
    SUB_MONEY_SPECIAL_BOX_BUY_KEY		= 3058,		//神秘宝箱购买钥匙消耗金币

	//扣除物品流水
	ITEM_SYSTEM_AUTO_USE				= 4001,		//系统自动使用
	ITEM_MOUNT_EVOLUATE                 = 4002,    	//坐骑进化
	ITEM_EQUIP_REFINE_USE           	= 4003,    	//装备强化
	ITEM_PLAYER_USE                     = 4004,    	//物品使用
	ITEM_INSERT_REMOVE_JEWEL        	= 4005,    	//装备镶嵌宝石
	ITEM_PUT_ON_EQUIP               	= 4006,    	//穿上装备
	ITEM_RELIVE_USE                 	= 4007,    	//复活使用
	ITEM_TASK_REMOVE                	= 4008,    	//任务失去
	ITEM_PLAYER_DROP                    = 4009,    	//丢弃物品
	ITEM_SKILL_UPGRADE_LEVEL           	= 4010,    	//技能升级
	ITEM_LEAGUE_DONATE 	 				= 4011,		//帮派捐献
	ITEM_FEED_LEAGUE_BOSS 				= 4012,		//喂养帮派boss消耗
	ITEM_ENTER_WBOSS_USE_FLY			= 4013,		//飞鞋进入世界boss
    ITEM_EQUIP_COMPOSE_USE         		= 4014,		//装备合成
    ITEM_MOUNT_ABILITY_GROWTH			= 4015,		//资质成长提升
    ITEM_GOOD_REFINE_USE				= 4016,		//装备精炼
    ITEM_TASK_FAST_FINISH           	= 4017,    	//快速完成任务
    ITEM_MAGICWEAPON_PROMOTE        	= 4018,    	//罗摩提升
    ITEM_UPGRADE_ILLUS					= 4019,	    //升级图鉴
    ITEM_EQUIP_SMELT					= 4020,	   	//装备熔炼
    ITEM_UPGRADE_RAMA					= 4021,	   	//升级罗摩
    ITEM_UPGRADE_ESCORT					= 4022, 	//升级护送
    ITEM_REMOVE_FROM_TEST           	= 4023,    	//测试删除
    ITEM_MAP_TRANSFER					= 4024,    	//地图传送
    ITEM_RED_UPRISING					= 4025,		//红装升阶
    ITEM_RED_CLOTHES_EXCHANGE			= 4026,		//红装碎片兑换
    ITEM_TREASURES_DISCOUNT				= 4027,		//宝匣礼卷
    ITEM_RETURN_ACTIVITY_BUY			= 4028,		//商店类活动购买消耗
    ITEM_FASHION_ADD_COLOR_COST			= 4029,		//时装染色消耗
    ITEM_FLOWER							= 4030,		//玩家送花
    ITEM_UPDATE_WEDDING_RING			= 4031,		//升级戒指
    ITEM_UPDATE_WEDDING_TREE			= 4032,		//升级爱情树
    ITEM_REMOVE_RING_FIRST				= 4033,		//第一次购买戒指
    ITEM_REMOVE_TIMEOUT					= 4034,		//过期删除
    ITEM_SECRET_EXCHANGE				= 4035,		//密卷兑换
    ITEM_LEGEND_EXCHANGE				= 4036,		//传说图谱兑换
    ITEM_TRANSFER_UP_STAGE				= 4037,		//英魂升阶消耗
    ITEM_MAY_ACT_COST					= 4038,		//五一活动购买消耗
    ITEM_ENTER_TRVL_WBOSS_USE_FLY		= 4039,		//飞鞋进入跨服世界boss
    ITEM_EQUIP_JEWEL_UPGRADE			= 4040,		//宝石升级消耗
    ITEM_GOOD_MOLDING_SPIRIT			= 4041,		//铸魂消耗
    ITEM_EQUIP_JEWEL_SUBLIME			= 4042,		//宝石升华消耗
    ITEM_SPECIAL_BOX_COST_ITEM			= 4043,		//神秘宝箱消耗道具
    ITEM_USE_GODER_UPGRADE				= 4044,		//化神升阶
    ITEM_SPECIAL_CHANGE_ITEM			= 4045,		//神秘宝箱兑换道具所消耗

    GAME_SERIAL_END
};

//其他流水
enum OtherSerial
{
    //主类型
    MAIN_SERIAL_BEGIN         			= 50000,
    MAIN_SERIAL_COMMON             		= 50001,    //公共流水
    MAIN_ADD_VIP_STATUS					= 50002,	//增加VIP
    MAIN_MEDIA_GIFT                 	= 50003,    //媒体礼包
    SCRIPT_REL_OTHER_SERIAL         	= 50004,    //副本相关流水
    MAIN_INTIMACY_VALUE					= 50005,	//亲密度
    LEAGUE_MAIN_SERIAL              	= 50006,    //仙盟
    REPUTATION_SERIAL               	= 50010,    //声望流水
    LUCKY_SERIAL                    	= 50011,    //幸运值流水
    KILLING_SERIAL                  	= 50012,    //杀戳值流水
    ESCORT_SERIAL						= 50014,	//运镖流水
    LEAGUE_CONTRI_SERIAL				= 50015,	//帮贡
    ILLUS_UPGRADE_SERIAL				= 50016,	//图鉴
    ESCORT_START_SERIAL					= 50017,	//开始护送流水
    SWORD_POOL_EXP_ADD_SERIAL			= 50018,	//剑池灵气增加流水
    SWORD_POOL_EXP_DEL_SERIAL			= 50019,	//剑池灵气减少流水
    SWEET_DEGREE_SERIAL					= 50020,	//甜蜜值流水
    MAIN_FIGHT_FORCE					= 50021,	//战力流水
    MAIN_EXP_SERIAL						= 50022,	//经验流水
    MAIN_TARENA_STAGE					= 50023,	//竞技段位流水
    MAIN_PLAYER_SKILL_SERIAL			= 50024,	//技能流水
    MAIN_EXECEPTION_SERIAL				= 50025,	//异常信息
    SCRIPT_CLEAN_SERIAL					= 50026,	//扫荡副本id流水
    TRAVPEAK_SERIAL                 	= 50027,    //巅峰对决流水
    RESOURCE_CHANGE_SERIAL				= 50028,	//资源商店购买流水
    FISH_SERIAL							= 50029,	//捕鱼流水

    MAIN_SERIAL_END,

	//子类型
    SUB_SERIAL_BEGIN          			= 10000,
    SUB_USE_MEDIA_GIFT              	= 10001,    //使用媒体礼包
    SUB_SCRIPT_ENTER                	= 10002,    //副本进入流水,变化值:已使用次数，扩展１:购买次数，扩展２：副本ＩＤ
    SUB_SCRIPT_FINISH               	= 10003,    //副本完成流水,变化值:已使用次数，扩展１:购买次数，扩展２：副本ＩＤ
    SUB_LEAGUE_LEVEL                	= 10004,    //仙盟等级流水,变化值:等级, 扩展１:当前资源
    SUB_LEAGUE_MEMBER               	= 10005,    //仙盟成员流水,变化值:成员个数
    SUB_ADD_BEAST                   	= 10006,    //增加宠物
    SUB_DEL_BEAST						= 10007,	//删除宠物
    SUB_INTIMACY_VALUE              	= 10008,    //亲密度
    SUB_SAVVY_SCRIPT                	= 10009,    //副本悟性奖励
    SUB_CULTIVATION_ADD_BY_EXP      	= 10010,    //经验换修为
    SUB_CULTIVATION_ADD_BY_EQUIP    	= 10011,    //装备加修为
    SUB_CULTIVATION_ADD_BY_ITEM     	= 10012,    //道具加修为
    SUB_CULTIVATION_RELIVE          	= 10013,    //修为转生
    SUB_SAVVY_ITEM_USE              	= 10014,    //道具加悟性
    SUB_REPUTATION_ADD              	= 10015,    //获得声望
    SUB_BEAST_COMBINE_COST          	= 10016,    //宠物融合消耗副宠
    SUB_KILL_REDUCE_LUCKY           	= 10017,    //恶意杀人扣幸运值
    SUB_KILL_ADD                    	= 10018,    //杀戳值异常增加
    SUB_SECRET_TREASURE_SCORE       	= 10019,    //秘藏猎人积点流水
    SUB_ESCORT_OFFLINE					= 10020,	//离线运镖失败
    SUB_LEAGUE_CONTRI_ADD				= 10021,	//帮派捐献获得帮贡
    SUB_TRAVPEAK_CREATE_TEAM        	= 10022,    //巅峰对决创队
    SUB_FISH_SCORE_ADD					= 10023,	//捕鱼积分增加
    SUB_FISH_SCORE_SUB					= 10024,	//捕鱼积分消耗
    SUB_FISH_TYPE_SCORE_ADD				= 10025,	//不同捕鱼方法增加积分
    SUB_FISH_SCORE_USE_PROP				= 10026, 	//使用道具增加积分
    SUB_SERIAL_END
};

enum ContributeSerial
{
    CONTRI_FROM_ITEM                = 1,        //item
    CONTRI_FROM_MONSTER             = 2,        //monster
    CONTRI_FROM_MARTIAL             = 3,        //武道会
    CONTRI_FROM_WAR                 = 4,        //war
    CONTRI_FROM_BONFIRE             = 5,        //bonfire
    CONTRI_FROM_DONATE              = 6,        //donate
    CONTRI_END
};

//经验流水
enum ExpSerial
{
    EXP_TASK                        = 1,    //任务获得
    ADD_EXP_FROM_TEST               = 4,    //本地测试
    EXP_FROM_PROP                   = 6,    //物品
    EXP_FROM_MONSTER                = 7,    //普通怪物
    EXP_SCRIPT_ADDITION             = 8,    //副本额外经验奖励
    EXP_FROM_SCRIPT_MONSTER         = 11,   //副本杀怪经验
    EXP_FROM_OFFLINE_REWARDS		= 22,	//离线奖励
    EXP_FROM_ANSWER_ACTIVITY		= 23,	//答题活动经验
    EXP_FROM_HOTSPRING_ACTIVITY		= 24,	//温泉活动经验
    EXP_FROM_LEAGUE_BOSS			= 25,	//帮派boss经验
    EXP_FROM_ESCORT					= 26,	//护送经验
    EXP_FROM_OFFLINE_HOOK			= 27,	//离线挂机
    EXP_FROM_DAILY_RUN				= 28,	//天天跑酷
    EXP_FROM_TRVL_PEAK				= 29,	//巅峰对决增加经验

    EXP_END
};

//装备流水
enum EquipmentSerial
{
    EQUIP_PUT_ON					= 1,   	//穿上装备
    EQUIP_GOOD_REFINE				= 2,	//装备精练
    EQUIP_RED_UPRADE				= 3,	//装备传说
    FASHION_EXIPRE_DISAPPEAR        = 4,   	//限时时装到期消失
    FASHION_EXIPRE_EXIST            = 5,   	//限时时装到期不消失
    FASHION_APPEND                  = 6,   	//限时时装延长时间
    EQUIP_TAKE_PLACE                = 7,   	//穿装备时替换
    EQUIP_STRENGTHEN            	= 8,   	//装备强化
    EQUIP_JEWEL_INSERT              = 9,  	//装备镶嵌宝石
    EQUIP_UPGRADE                   = 10,  	//装备升阶
    EQUIP_JEWEL_REMOVE              = 11,  	//装备卸下宝石
    EQUIP_MOLDING_SPIRIT			= 12, 	//装备铸魂
    EQUIP_JEWEL_UPGRADE				= 13,   //镶嵌的宝石升级
    EQUIP_END
};

//战骑类流水
enum MountSerial
{
	MOUNT_SER_UPGRADE		= 1,	//升阶
	MOUNT_SER_EQUIP			= 2,	//装备
	MOUNT_SER_SKILL			= 3,	//技能
	MOUNT_SERIAL_END
};

//邮件流水
enum MailDetailSerial
{
    MAIL_SEND      	= 1,  //发送邮件
    MAIL_DELETE     = 2,  //删除邮件
    MAIL_RECV       = 3,  //收到邮件
    MAIL_RESEND    	= 4,  //接收失败重新发送
    MAIL_READ		= 5,  //读取邮件[暂时不加]
    MAIL_PICKUP     = 6,  //提取附件
    MAIL_SERIAL_END
};

//任务流水
enum TaskSerial
{
    TASK_SERIAL_VISIBLE 		= 1,    //任务可见但未接
    TASK_SERIAL_ACCEPTABLE 		= 2, 	//任务可接
    TASK_SERIAL_ACCEPTED 		= 3,   	//任务已接
    TASK_SERIAL_FINISH 			= 4,    //任务已完成
    TASK_SERIAL_SUBMIT 			= 5,    //任务已提交
    TASK_SERIAL_ABANDON 		= 6,    //任务放弃
    TASK_SERIAL_FAST_FINISH 	= 7,	//快速完成

    TASK_STATUS_END
};

enum NoUseSerial
{
	/********************no use**************************/
    //增加金钱流水
    ADD_MONEY_SERIAL_BEGIN          	= 10000,
    ADD_MONEY_FROM_SELL             	= 10004,        //卖出商品获得
    ADD_MONEY_ACHIEVEMENT           	= 10010,        //成就奖励
    ADD_MONEY_OPEN_ACT_RECHARGE     	= 10019,        //开服活动，冲值返礼卷
    ADD_MONEY_SCRIPT_TOTAL_STAR     	= 10023,        //斩妖总星级奖励
    ADD_MONEY_CANG_QIONG_BET        	= 10026,        //1v多下注
    ADD_MONEY_FROM_FUND             	= 10027,        //基金
    ADD_MONEY_LSIEGE_ADMIRE         	= 10030,        //敬仰奖励
    ADD_MONEY_DECOMPOSE             	= 10031,        //装备分解获得
    ADD_MONEY_BROTHER_FAILED        	= 10033,        //结义失败返回
    ADD_MONEY_BOX_GET               	= 10034,        //藏宝库获得

    //扣除金钱流水
    SUB_MONEY_SERIAL_BEGIN          	= 20000,
    SUB_MONEY_MAIL                  	= 20002,        //邮件邮寄货币
    SUB_MONEY_MARKET_ONSELL         	= 20004,        //拍卖行上架
    SUB_MONEY_MARKET_CONSELL        	= 20005,        //拍卖行续期
    SUB_MONEY_LEAGUE_SHOP_BUY      	 	= 20017,        //宗派商店购买
    SUB_MONEY_MALL_DONATE           	= 20033,        //商城赠送
    SUB_MONEY_BUY_BLOOD_PACK        	= 20039,        //自动购买
    SUB_MONEY_SCRIPT_RELIVE         	= 20041,        //自动购买 -- 副本内复活
    SUB_MONEY_FASHION_FAST_USE      	= 20051,        //时装快速购买并使用
    SUB_MONEY_BIND_COPPER_ADJ       	= 20058,        //自动兑换绑铜
    SUB_MONEY_FAST_BUY_TOWN         	= 20059,        //自动购买 -- 回主城
    SUB_MONEY_CANG_QIONG_BET        	= 20069,        //1v多下注
    SUB_MONEY_PANIC_BUY             	= 20072,        //开服抢购
    SUB_MONEY_LOTTERY               	= 20073,        //限时神装抽奖
    SUB_MONEY_PRACTICE              	= 20075,        //修行-运功消耗
    SUB_MONEY_DIVORCE               	= 20077,        //离婚消耗
    SUB_MONEY_PRACTICE_DIRECT       	= 20079,        //修行-指点消耗
    SUB_MONEY_LSIEGE_SIGNUP         	= 20080,        //攻城报名消耗
    SUB_MONEY_LSIEGE_SHOP_BUY       	= 20081,        //攻城城内商店购买
    SUB_MONEY_FAST_BUY              	= 20082,        //转生快捷购买
    SUB_MONEY_EXCHANGE              	= 20083,        //转生兑换
    SUB_MONEY_TASK_STAR             	= 20088,        //任务刷新星级
    SUB_MONEY_BOSS_ENTER            	= 20090,        // 进入BOSS大厅
    SUB_MONEY_HOOK_BUY              	= 20091,        // 挂机自动购买
    SUB_MONEY_TRANSLATE_TO_ENEMY    	= 20093,        // 传送到仇人处扣钱
    SUB_MONEY_BUY_TYPHON            	= 20094,        // 自动购买大喇叭
    SUB_MONEY_BROTHER               	= 20095,        // 结义消耗
    SUB_MONEY_BEAST_CONBINE         	= 20099,        // 自动购买资质丹
    SUB_MONEY_BUY_STRENGTH          	= 20104,        // 购买体力消耗
    SUB_MONEY_LUCKY_TABLE_COST      	= 20105,        // 幸运转盘消耗


    //增加物品流水
    ADD_ITEM_SERIAL_BEGIN           	= 30000,
    ITEM_LEAGUE_SHOP_BUY            	= 30006,    // 宗派商店购买
    ITEM_MARKET_GETBACK             	= 30012,    // 拍卖行取回
    ITEM_FAST_BUY                   	= 30014,    // 快速购买
    ITEM_ACHIEVEMENT_REWARD         	= 30018,    // 成就奖励
    ITEM_LIVENESS_AWARD             	= 30022,    // 活跃度奖励获得
    ITEM_USE_FAILED_BACK            	= 30026,    // 使用失败返回
    ITEM_SCRIPT_CARD                	= 30031,    // 副本抽牌奖励，子类型：副本ID
    ITEM_ROTARY_TABLE_GET           	= 30033,    // 转盘
    ITEM_DAILY_LIVENESS             	= 30044,    // 日常功能奖励
    ITEM_LEVEL_REWARDS              	= 30045,    // 等级奖励
    ITEM_SCRIPT_TOTAL_STAR          	= 30047,    // 斩妖总星级奖励
    ITEM_TOTAL_RECHARGE_REWARD      	= 30050,    // 累积冲值奖励
    ITEM_PANIC_BUY                  	= 30052,    // 开服抢购
    ITEM_DRAW_LOTTERY               	= 30053,    // 限时神装抽奖
    ITEM_DRAW_LOTTERY_MORETIMES     	= 30057,    // 限时神装多次抽奖
    ITEM_LSIEGE_SHOP_BUY            	= 30060,    //攻城商店购买
    ITEM_HOOK_BUY                   	= 30063,    //挂机自动购买
    ITEM_REMOVE_STORE               	= 30065 ,   //物品移出仓库
    ITEM_GET_LSTORE                 	= 30066,    //从仙盟仓库取出物品
    ITEM_DECOMPOSE_GET              	= 30067,    //装备分解获得
    ITEM_BOX_INSERT_ONE             	= 30069,    //藏宝库抽一次获得
    ITEM_BOX_INSERT_TEN             	= 30070,    //藏宝库抽十次获得
    ITEM_BOX_INSERT_FIFTY           	= 30071,    //藏宝库抽五十次获得
    ITEM_BROTHER_REWARD             	= 30075,    //结义任务奖励
    ITEM_BEAST_GEN_SKILL_BOOK       	= 30077,    //生成技能书获得
    ITEM_BEAST_MERGE_SKILL          	= 30079,    //合成技能书获得
    ITEM_LSTORE_INSTER_FAILED       	= 30080,    //仙盟仓库插入物品失败返回
    ITEM_BEAST_SKILL_MERGE_FAILE    	= 30085,    //宠物技能书合成失败
    ITEM_FASHION_BOX_FAILED_RETURN  	= 30087,    //时装宝箱开启失败返回
    ITEM_FASHION_BOX_ADD            	= 30088,    //时装宝箱开启获得
    ITEM_LUCKY_TABLE_ADD            	= 30089,    //幸运转盘
    ITEM_LOVA_GIFT			        	= 30091,    //爱心礼包


    //扣除物品流水
    SUB_ITEM_SERIAL_BEGIN           	= 40000,
    ITEM_MAIL_REMOVE                	= 40004,    //邮件邮寄物品
    ITEM_SELL_REMOVE                	= 40005,    //出售
    ITEM_SHOP_BUY_REMOVE            	= 40013,    //预扣购买物品
    ITEM_REMOVE_ONSELL              	= 40015,    //拍卖上架
    ITEM_FASHION_CONVERT_USE        	= 40028,    //时装转化
    ITEM_FASHION_EXPIRE_TIME_DEL    	= 40029,    //时装时装转化
    ITEM_KEEPSAKE_UPGRADE           	= 40037,    //信物升级
    ITEM_LSIEGE_SIGNUP              	= 40040,    //攻城报名消耗
    ITEM_BOSS_USE_ITEM              	= 40044,    //打BOSS需要消耗的道具
    ITEM_INSERT_STORE               	= 40049,    //物品插入到仓库
    ITEM_USE_TYPHON                 	= 40050,    //自动使用大喇叭
    ITEM_INSERT_LSTORE              	= 40051,    //物品插入到仙盟仓库
    ITEM_DECOMPOSE_EQUIP            	= 40052,    //装备分解
    ITEM_BROTHER_WINE               	= 40056,    //赠送美酒
    ITEM_BEAST_CONBINE_BY_PROP      	= 40059,    //使用资质丹提升资质
    ITEM_BEAST_MERGE_SKILL_COST     	= 40062,    //合成技能书消耗
    ITEM_BEAST_LEARN_SKILL_COST     	= 40064,    //学习技能书消耗
    ITEM_FASHION_BOX_COST           	= 40067,    //时装扣除
    ITEM_LSTORE_DELETE_FAILED       	= 40069,    //仙盟仓库删除物品失败扣除

    NO_USE_SERIAL_END
};

#endif //_SERIALDEFINE_H_
