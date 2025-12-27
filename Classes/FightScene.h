#ifndef __FIGHT_SCENE_H__
#define __FIGHT_SCENE_H__
#include "cocos2d.h"	
#include "VillageScene.h"
USING_NS_CC;
//废稿
class FightScene : public VillageScene
{

public:
	static FightScene* getInstance() {
		auto currentScene = Director::getInstance()->getRunningScene();
		if (!currentScene) return nullptr;
		// 找到场景中 Tag=25 的子节点
		return dynamic_cast<FightScene*>(currentScene->getChildByTag(25));
	}
	static Scene* createScene();
	virtual bool init() override;
	void initFightScene(); // 初始化战斗场景
	void beginFight(); // 开始战斗
	void updateCountDown(float dt);//战斗计时
	void onFightSettle();//战斗结算
	CREATE_FUNC(FightScene);
private:
	Label* _countDownLabel; // 倒计时标签
	float _totalTime;        // 总时间
	float _remainingTime;    // 剩余时间
	MenuItemImage* _fightStartBtn; // 开始战斗按钮
};




#endif 
#pragma once
