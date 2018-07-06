/*
 * GameFont.h
 *
 *  Created on: 2013年7月26日
 *      Author: xie
 */

#ifndef GAMEFONT_H_
#define GAMEFONT_H_

enum Font
{
	FONT_MARKET_SELL_MAIL 			= 1001,		//拍卖行出售
	FONT_MARKET_BUY_MAIL 			= 1002,		//拍卖行购买
	FONT_MALL_DONATE_GOODS          = 1005,     //商城赠送物品
	FONT_SYSTEM_MAIL                = 1006,     //系统邮件
	FONT_WORLD_BOSS_AWARDS_FINAL	= 1014,		//世界BOSS最后一击奖励邮件
	FONT_WORLD_BOSS_AWARDS_HURT		= 1015,		//世界BOSS伤害奖励邮件
	FONT_WORLD_BOSS_AWARDS_RANK		= 1016,		//世界BOSS排行奖励邮件
    FONT_WORLD_BOSS_AWARDS_FIRST	= 1020,		//世界BOSS第一滴血奖励邮件
    FONT_LEAGUE_ESCORT_REWARD		= 1054,		//仙盟运镖
    FONT_LOTTERY_DRAW               = 1055,     //限时神装抽中奖池奖金
    FONT_LOTTERY_RANK               = 1056,     //限时神装排行奖励
    FONT_LOTTERY_ATTEND				= 1057,		//限时神装参与奖
    FONT_NO_PACKAGE					= 1058,		//背包已满
    FONT_LSTORE_GET_LEADER          = 1069,     //仙盟仓库取出物资提示(盟主)
    FONT_LSTORE_GET                 = 1070,     //仙盟仓库取出物资提示(副盟主)
    FONT_EQUIP_DECOMPOSE_BACK_STONE = 1072,     //装备分解返还宝石
    FONT_GIVA_UP_BROTHER			= 1076,		//分道扬镳不包含装备）
    FONT_BROTHER_TASK_REWARD		= 1077,		//结义任务奖励
    FONT_LABEL_TIME_UP				= 1100,		//称号过期
    FONT_TBATTLE_REWARD_MAIL        = 70001,    // 九华之巅（华山论剑）奖励
    FONT_TBATTLE_PRACTICE_MAIL      = 70002,    // 九华之巅历练奖励
	FONT_END
};

#define FONT2(X) CONFIG_INSTANCE->font(X)

#endif /* GAMEFONT_H_ */
