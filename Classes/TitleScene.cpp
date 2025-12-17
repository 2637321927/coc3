#include "TitleScene.h"
#include "VillageScene.h" // 后续要跳转的主村庄场景（之前定义的）
#include "cocos2d.h"
//#include "SimpleAudioEngine.h" // 音效（可选）


bool TitleScene::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 1. 初始化背景
    initBackground();
    // 2. 初始化功能按钮
    initButtons();

    return true;
}

void TitleScene::initBackground()
{
    // 获取屏幕尺寸（适配不同分辨率）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 创建背景精灵（替换为你的标题背景图路径）
    Sprite* bgSprite = Sprite::create("title_bg.png");
    if (bgSprite)
    {
        // 全屏适配（按屏幕缩放）
        bgSprite->setPosition(origin + visibleSize / 2);
        bgSprite->setScaleX(visibleSize.width / bgSprite->getContentSize().width);
        bgSprite->setScaleY(visibleSize.height / bgSprite->getContentSize().height);
        this->addChild(bgSprite, 0); // 层级0：背景
    }
}

void TitleScene::initButtons()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ========== 1. 开始游戏按钮 ==========
    Sprite* startBtnNormal = Sprite::create("btn_start_normal.png"); // 常态图
    Sprite* startBtnPressed = Sprite::create("btn_start_pressed.png"); // 按下图
    MenuItemSprite* startMenuItem = MenuItemSprite::create(
        startBtnNormal,
        startBtnPressed,
        CC_CALLBACK_1(TitleScene::onStartGameClick, this)
    );
    // 按钮位置（屏幕中偏下，可自定义）
    startMenuItem->setPosition(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 50);

    /*// ========== 2. 关卡选择按钮（预留） ==========
    Sprite* levelBtnNormal = Sprite::create("btn_level_normal.png");
    Sprite* levelBtnPressed = Sprite::create("btn_level_pressed.png");
    MenuItemSprite* levelMenuItem = MenuItemSprite::create(
        levelBtnNormal,
        levelBtnPressed,
        CC_CALLBACK_1(TitleScene::onLevelSelectClick, this)
    );
    levelMenuItem->setPosition(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 120);
        */
        /*
    // ========== 3. 设置按钮（预留） ==========
    Sprite* settingBtnNormal = Sprite::create("btn_setting_normal.png");
    Sprite* settingBtnPressed = Sprite::create("btn_setting_pressed.png");
    MenuItemSprite* settingMenuItem = MenuItemSprite::create(
        settingBtnNormal,
        settingBtnPressed,
        CC_CALLBACK_1(TitleScene::onSettingClick, this)
    );
    settingMenuItem->setPosition(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 190);
        */

        // 创建菜单（Cocos2d-x中按钮必须放在Menu里才能响应触摸）
       // Menu* menu = Menu::create(startMenuItem, levelMenuItem, settingMenuItem, nullptr);
    Menu* menu = Menu::create(startMenuItem, nullptr);
    menu->setPosition(Vec2::ZERO); // Menu本身位置归零，按钮用自身position定位
    this->addChild(menu, 1); // 层级1：按钮（在背景上层）

}

// 开始游戏按钮回调（跳转到主村庄场景）
void TitleScene::onStartGameClick(Ref* pSender)
{
    /* // 可选：播放点击音效
     SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
     */
     // 创建目标场景（之前写的VillageScene）
      Scene* villageScene = VillageScene::createScene();
     //场景切换（加淡入淡出动画，提升体验）
      if (!villageScene) {
          log("VillageScene 创建失败！");
          return;
      }
      log("VillageScene 创建成功，准备切换");
      Director::getInstance()->replaceScene(villageScene);
     //TransitionFade* transition = TransitionFade::create(0.5f, villageScene);
     //Director::getInstance()->replaceScene(transition);
    
    // 切换场景（使用过渡动画，可选）

    
}

// 关卡选择按钮回调（预留，先打印日志，后续实现）
void TitleScene::onLevelSelectClick(Ref* pSender)
{
    /*
    SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
    log("点击了关卡选择按钮，待开发");
    */
}

// 设置按钮回调（预留）
void TitleScene::onSettingClick(Ref* pSender)
{
    /*
    SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
    log("点击了设置按钮，待开发");
    */
}

// 可选：获取按钮碰撞区域（用于自定义触摸检测，默认Menu已处理，可忽略）
Rect TitleScene::getButtonRect(Sprite* btnSprite)
{
    Size size = btnSprite->getContentSize();
    Vec2 pos = btnSprite->getPosition();
    return Rect(pos.x - size.width / 2, pos.y - size.height / 2, size.width, size.height);
}