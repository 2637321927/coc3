#include "LevelScene.h"

/**
 * 创建关卡场景实例 (工厂方法).
 * 根据传入的关卡文件路径创建并初始化战斗场景.
 * * @param levelPath 关卡配置文件的路径 (如 "level1.txt").
 * @return 创建成功的 Scene 对象指针.
 */
Scene* LevelScene::createWithLevel(const std::string& levelPath)
{
    // 1. 创建 LevelScene 实例
    auto pRet = new (std::nothrow) LevelScene();
    // 调用特有的 level_init 初始化 (假设这是 VillageScene 中定义的 protected 方法)
    if (pRet && pRet->level_init())
    {
        pRet->_currentLevelPath = levelPath;
        pRet->loadGame(levelPath); // 调用父类的 loadGame 读取关卡配置
        pRet->setupLevelUI();      // 设置关卡专属UI
        pRet->autorelease();
        return pRet; // 直接返回，因为 LevelScene 是 Scene 子类
    }
    delete pRet;
    return nullptr;
}

/**
 * 初始化关卡场景.
 * 调用父类初始化，并进行关卡特有的设置（如隐藏主界面按钮、设置Tag）.
 * * @return 初始化成功返回 true.
 */
bool LevelScene::init()
{
    // 关键：调用父类 VillageScene 的 init() 以加载地图和基础层
    // 注意：父类 init 必须是 public 或 protected 才能在此调用
    if (!VillageScene::init())
    {
        CCLOGERROR("LevelScene 父类初始化失败！");
        return false;
    }

    // 隐藏父类中不需要在关卡显示的按钮 (如关卡选择、商店)
    // 需确保父类中这些节点已通过 setName 设置了名称
    auto levelSelectBtn = this->getChildByName("LevelSelectBtn");
    if (levelSelectBtn) levelSelectBtn->setVisible(false);

    auto shopBtn = this->getChildByName("ShopBtn");
    if (shopBtn) shopBtn->setVisible(false);

    // 设置 Tag 为 100，以便攻击类建筑通过 VillageScene::getInstance() 获取场景实例
    this->setTag(100);
    return true;
}

/**
 * 设置关卡专属UI.
 * 添加返回村庄按钮、战斗结算信息等.
 */
void LevelScene::setupLevelUI()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 2. 创建返回村庄按钮
    // 注意：MenuItemImage 需要放入 Menu 中才能响应点击，或者使用 ui::Button
    auto returnBtn = MenuItemImage::create("ui/btn_ble.png", "ui/btn_blue_pressed.png");
    returnBtn->setPosition(Vec2(visibleSize.width - 120, 40));

    // 建议：将其包装到 Menu 中或改用 ui::Button 以便绑定回调
    // auto menu = Menu::create(returnBtn, nullptr);
    // menu->setPosition(Vec2::ZERO);
    // this->addChild(menu, 300);
    this->addChild(returnBtn, 300);
}

/**
 * 战斗结束按钮回调.
 * 处理战斗结算逻辑.
 * * @param sender 触发事件的对象.
 */
void LevelScene::onEndBattleClicked(Ref* sender)
{
    CCLOG("战斗结束：计算得分...");
}

/**
 * 返回村庄按钮回调.
 * 切换场景回到主村庄界面.
 * * @param sender 触发事件的对象.
 */
void LevelScene::onReturnToVillageClicked(Ref* sender)
{
    // 切换回村庄场景（用父类的 createScene）
    auto villageScene = VillageScene::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(0.5f, villageScene));
}