#include "TitleScene.h"
#include "VillageScene.h" // 后续要跳转的主村庄场景
#include "cocos2d.h"
#include "EnumType.h" 
//#include "SimpleAudioEngine.h" // 音效（可选）

USING_NS_CC;
//using namespace CocosDenshion;

// TitleScene.cpp
TitleScene* TitleScene::_instance = nullptr;

/**
 * 初始化标题场景.
 * 负责创建背景、功能按钮和必要的调试信息.
 * * @return 初始化成功返回 true，失败返回 false.
 */
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

/**
 * 初始化背景图片.
 * 加载标题背景图并进行全屏适配.
 */
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

/**
 * 递归打印节点树信息 (调试用).
 * 用于排查触摸穿透、层级遮挡等问题.
 * * @param node 当前遍历的节点指针.
 * @param depth 当前递归深度 (用于缩进显示).
 */
void dumpNodeRecursive(Node* node, int depth = 0) {
    if (!node) return;
    // 打印层级、节点名、类型、位置、大小
    std::string indent(depth * 2, ' ');
    /*
    CCLOG("%sNode: %s (Type: %s) | Pos: (%.1f, %.1f) | Size: (%.1f, %.1f)",
        indent.c_str(),
        node->getName().c_str(),
        typeid(*node).name(),
        node->getPositionX(), node->getPositionY(),
        node->getContentSize().width, node->getContentSize().height);
    */
    // 递归打印子节点
    for (auto& child : node->getChildren()) {
        dumpNodeRecursive(child, depth + 1);
    }
}

/**
 * 场景进入回调.
 * 场景被激活显示时调用，用于执行调试日志输出.
 */
void TitleScene::onEnter() {
    CCLOG("TitleScene::onEnter called");
    Scene::onEnter();
    // 打印当前场景的所有节点（含层级、位置、大小）
    auto node = Director::getInstance()->getRunningScene();
    // dumpNodeRecursive(node); // 调试时可解开注释
}

/**
 * 初始化功能按钮.
 * 创建开始游戏、普通模式等菜单按钮，并绑定点击回调.
 */
void TitleScene::initButtons()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // ========== 1. 按钮资源加载 ==========
    // 创造模式按钮
    Sprite* creatingModeBtnNormal = Sprite::create("btn_start_normal.png"); // 常态图
    Sprite* creatingModeBtnPressed = Sprite::create("btn_start_pressed.png"); // 按下图

    // 普通模式按钮
    Sprite* normalModeBtnNormal = Sprite::create("btn_start_normal.png"); // 常态图
    Sprite* normalModeBtnPressed = Sprite::create("btn_start_pressed.png"); // 按下图

    // 创建菜单项
    MenuItemSprite* startMenuItem = MenuItemSprite::create(
        creatingModeBtnNormal,
        creatingModeBtnPressed,
        CC_CALLBACK_1(TitleScene::onCreateGameClick, this)
    );

    MenuItemSprite* normalMenuItem = MenuItemSprite::create(
        normalModeBtnNormal,
        normalModeBtnPressed,
        CC_CALLBACK_1(TitleScene::onNormalGameClick, this)
    );

    // ========== 2. 设置按钮位置 ==========
    // 创造模式按钮居中偏下
    startMenuItem->setPosition(origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2 - 50);

    // 普通模式按钮在右侧偏移100像素
    normalMenuItem->setPosition(origin.x + visibleSize.width / 2 + 100,
        origin.y + visibleSize.height / 2 - 50);

    // ========== 3. 创建菜单容器 ==========
    // Cocos2d-x中按钮必须放在Menu里才能响应触摸
    Menu* menu = Menu::create(startMenuItem, normalMenuItem, nullptr);
    menu->setPosition(Vec2::ZERO); // Menu本身位置归零，按钮用自身position定位
    this->addChild(menu, 1); // 层级1：按钮（在背景上层）
}

/**
 * 普通模式按钮点击回调.
 * 切换到 VillageScene 并进入 NORMAL 模式.
 * * @param pSender 触发事件的对象.
 */
void  TitleScene::onNormalGameClick(Ref* pSender)
{
	// 创建目标场景（之前写的VillageScene）
	Scene* villageScene = VillageScene::createScene(BaseMode::NORMAL);
	//场景切换（加淡入淡出动画，提升体验）
	if (!villageScene) {
		CCLOG("VillageScene 创建f失败！");
		return;
	}
	CCLOG("VillageScene 创建s成功，准备切换");
	Director::getInstance()->pushScene(villageScene);//将新场景压入栈顶，旧场景暂停保留；
	//TransitionFade* transition = TransitionFade::create(0.5f, villageScene);
	//Director::getInstance()->replaceScene(transition);
   // 切换场景（使用过渡动画，可选）
}

/**
 * 创造模式按钮点击回调.
 * 切换到 VillageScene 并进入 CREATING 模式.
 * * @param pSender 触发事件的对象.
 */
void TitleScene::onCreateGameClick(Ref* pSender)
{
    /* // 可选：播放点击音效
     SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
     */

     // 创建目标场景（之前写的VillageScene）
    Scene* villageScene = VillageScene::createScene(BaseMode::CREATING);

    // 场景切换
    if (!villageScene) {
        CCLOG("VillageScene 创建失败！");
        return;
    }
    CCLOG("VillageScene 创建成功，准备切换");
    Director::getInstance()->pushScene(villageScene); // 将新场景压入栈顶，旧场景暂停保留
}

/**
 * 关卡选择按钮回调（预留）.
 * * @param pSender 触发事件的对象.
 */
void TitleScene::onLevelSelectClick(Ref* pSender)
{
    /*
    SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
    log("点击了关卡选择按钮，待开发");
    */
}

/**
 * 设置按钮回调（预留）.
 * * @param pSender 触发事件的对象.
 */
void TitleScene::onSettingClick(Ref* pSender)
{
    /*
    SimpleAudioEngine::getInstance()->playEffect("click_btn.mp3");
    log("点击了设置按钮，待开发");
    */
}

/**
 * 获取按钮碰撞区域.
 * 辅助函数，用于自定义触摸检测（默认Menu已处理，可忽略）.
 * * @param btnSprite 按钮精灵对象.
 * @return 按钮在父节点坐标系下的矩形区域.
 */
Rect TitleScene::getButtonRect(Sprite* btnSprite)
{
    Size size = btnSprite->getContentSize();
    Vec2 pos = btnSprite->getPosition();
    return Rect(pos.x - size.width / 2, pos.y - size.height / 2, size.width, size.height);
}