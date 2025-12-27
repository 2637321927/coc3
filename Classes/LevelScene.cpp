// LevelScene.cpp 完整代码
#include "LevelScene.h"

Scene* LevelScene::createWithLevel(const std::string& levelPath)
{
    // 1. 创建 LevelScene 实例（依赖父类的 CREATE_FUNC）
    auto pRet = new (std::nothrow) LevelScene();
    if (pRet && pRet->level_init())
    {
        pRet->_currentLevelPath = levelPath;
        pRet->loadGame(levelPath); // 调用父类的 loadGame
        pRet->setupLevelUI();      // 设置关卡UI
        pRet->autorelease();
        return pRet; // 直接返回，因为 LevelScene 是 Scene 子类
    }
    delete pRet;
    return nullptr;
}

bool LevelScene::init() 
{
    if (!VillageScene::init()) return false;
    // 关键：调用父类 VillageScene 的 init()（必须是 public 才能调用）
    if (!VillageScene::init())
    {
        CCLOGERROR("LevelScene 父类初始化失败！");
        return false;
    }

    // 隐藏父类的关卡选择/商店按钮（安全判空）
    auto levelSelectBtn = this->getChildByName("LevelSelectBtn");
    if (levelSelectBtn) levelSelectBtn->setVisible(false);

    auto shopBtn = this->getChildByName("ShopBtn");
    if (shopBtn) shopBtn->setVisible(false);
    this->setTag(100);
    return true;
}

void LevelScene::setupLevelUI()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 2. 返回村庄按钮
    auto returnBtn = MenuItemImage::create("ui/btn_ble.png", "ui/btn_blue_pressed.png");
    returnBtn->setPosition(Vec2(visibleSize.width - 120, 40));
    this->addChild(returnBtn, 300);
}

void LevelScene::onEndBattleClicked(Ref* sender)
{
    CCLOG("战斗结束：计算得分...");
}

void LevelScene::onReturnToVillageClicked(Ref* sender)
{
    // 切换回村庄场景（用父类的 createScene）
    auto villageScene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, villageScene));
}