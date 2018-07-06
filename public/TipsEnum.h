/*
 * TipsEnum.h
 *
 *  Created on: Jun 16, 2016
 *      Author: peizhibi
 */

#ifndef TIPSENUM_H_
#define TIPSENUM_H_

namespace GameEnum
{
	enum TipsFormatMsgID
	{	// brocast/tips_message.json
		FORMAT_MSG_SHUSAN_KILLER 				= 1001,
		FORMAT_MSG_LEVEL_LIMIT 					= 1002,
		FORMAT_MSG_BEAST_SKILL 					= 1003,
		FORMAT_MSG_ITEM_SEND_MAIL 				= 1004,
		FORMAT_MSG_BLOOD_ITEM_FINISH 			= 1005,    	// 生命药已用完
		FORMAT_MSG_MAGIC_ITEM_FINISH 			= 1006,    	// 法力药已用完
		FORMAT_MSG_EXPLOIT_SINGLE_ITEM 			= 1007,  	// 爆一个道具
		FORMAT_MSG_EXPLOIT_ITEMS 				= 1008,   	// 爆多个道具
		FORMAT_MSG_PICKUP_SINGLE_ITEM 			= 1009,   	// 拾取一个道具
		FORMAT_MSG_PICKUP_ITEMS 				= 1010,    	// 拾取多个道具
		FORMAT_MSG_REDUCE_LUCKY 				= 1011,    	// 恶意杀人扣幸运
		FORMAT_MSG_AUTO_TRANSFER 				= 1012,     // 自动使用卷轴
		FORMAT_MSG_BOSS_OWNER 					= 1013,     // 获得BOSS拾取权
		FORMAT_MSG_ADD_KILLING_VALUE 			= 1014,		// 增加杀戮值提示
		FORMAT_MSG_SCRIPT_ONE_LEVEL_LIMIT 		= 1015,   	// 副本一个玩家等级不足
		FORMAT_MSG_SCRIPT_TWO_LEVEL_LIMIT 		= 1016,   	// 副本两个玩家等级不足
		FORMAT_MSG_SCRIPT_READY_LEVEL_LIMIT 	= 1017, 	// 副本准备等级不足提示
		FORMAT_MSG_SCRIPT_ONE_SPECIAL_SPACE 	= 1018,		// 副本一个玩家处于异空间
		FORMAT_MSG_SCRIPT_TWO_SPECIAL_SPACE 	= 1019,		// 副本两个玩家处于异空间
		FORMAT_MSG_SELF_ATTEND_TRAVTEAM 		= 1020,     // 成功加入战队
		FORMAT_MSG_TEAMER_ATTEND_TRAVTEAM 		= 1021,   	// 队员接受邀请
		FORMAT_MSG_TEAMER_REJECT_TRAVTEAM 		= 1022,   	// 队员拒绝邀请
		FORMAT_MSG_TRAVPEAK_SHUTDOWN 			= 1023,    	// 该赛区巅峰对决临时关闭
		FORMAT_MSG_ENTER_SAFE_AREA 				= 1024,		// 夺宝奇兵进入安全区域
		FORMAT_MSG_EXIT_SAFE_AREA 				= 1025,		// 夺宝奇兵离开安全区域
		FORMAT_MSG_LOSE_CAPTURE_JEWEL 			= 1026,		// 夺宝遗失宝珠
		FORMAT_MSG_GET_ST_SCORE 				= 1027, 	// 秘藏积点
		FORMAT_MSG_ST_SCENE_SCORE 				= 1028, 	// 秘藏积点
		FORMAT_MSG_NEED_LIEVE_TIME				= 1029,		// 复活需要的时间
		FORMAT_MSG_END
	};
}



#endif /* TIPSENUM_H_ */
