#include "FightScene.h"
#include "cocos2d.h"
Scene* FightScene::createScene()
{
	Scene* fightScene = VillageScene::createScene(BaseMode::FIGHT);
	VillageScene* it = dynamic_cast<VillageScene*>(fightScene->getChildByTag(25));
	it->loadGame("fight.txt");
	FightScene* layer = FightScene::create();
	//layer->setTag(25);
	fightScene->addChild(layer);
	return fightScene;
}

bool FightScene::init()
{
	// 初始化战斗场景特有内容
	initFightScene();
	return true;
}
void FightScene::initFightScene()
{
	_uiLayer = ui::Layout::create();
	// 初始化战斗场景特有UI和逻辑
	_countDownLabel = Label::createWithTTF("倒计时: 60", "fonts/Marker Felt.ttf", 32);
	if (_countDownLabel) {
		Size visibleSize = Director::getInstance()->getVisibleSize();
		_countDownLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
		_uiLayer->addChild(_countDownLabel, 300);
	}
	_totalTime = 60.0f; // 总战斗时间60秒
	_remainingTime = _totalTime;
	// 创建开始战斗按钮
	_fightStartBtn = MenuItemImage::create(
		"ui/fight_start_btn.png",
		"ui/fight_start_btn.png",
		[this](Ref* sender) {
			//VillageScene::getInstance()->
			setTroopModeBtnInvisible();
			beginFight();
		}
	);
	if (_fightStartBtn) {
		_fightStartBtn->setPosition(Vec2(100, 100)); // 左下角位置
		auto menu = Menu::create(_fightStartBtn, nullptr);
		menu->setPosition(Vec2::ZERO);
		this->addChild(menu, 300);
	}
}
void FightScene::beginFight()
{
	// 开始战斗逻辑
	this->schedule(CC_SCHEDULE_SELECTOR(FightScene::updateCountDown), 0.1f);
	_fightStartBtn->setVisible(false);
}
void FightScene::updateCountDown(float dt)
{
	// 扣除流逝的时间
	_remainingTime -= dt;
	// 处理时间小于0的情况
	if (_remainingTime <= 0)
	{
		_remainingTime = 0;
		this->unschedule(CC_SCHEDULE_SELECTOR(FightScene::updateCountDown));
		onFightSettle();
	}
	// 更新倒计时UI
	int minutes = (int)_remainingTime / 60;
	int seconds = (int)_remainingTime % 60;
	_countDownLabel->setString(StringUtils::format("倒计时: %02d:%02d", minutes, seconds));
}
void FightScene::onFightSettle()
{
	// 战斗结算逻辑
}


