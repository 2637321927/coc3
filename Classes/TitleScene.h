#ifndef __TITLE_SCENE_H__
#define __TITLE_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;

/**
 * 游戏标题场景类.
 * 管理游戏启动后的主界面，包含背景显示、模式选择（创造/普通）等功能入口.
 */
class TitleScene : public cocos2d::Scene
{
public:
    /** Cocos2d-x 标准宏，自动实现 create() 方法. */
    CREATE_FUNC(TitleScene);

    /**
     * 初始化场景.
     * 调用父类初始化，并加载背景与按钮.
     *
     * @return 初始化成功返回 true.
     */
    virtual bool init() override;

    /**
     * 创建场景实例 (静态工厂).
     * 创建一个通用 Scene 容器，并将 TitleScene 作为子节点添加进去.
     *
     * @return 包含 TitleScene 的 Scene 对象指针.
     */
    static Scene* createScene()
    {
        auto scene = Scene::create();
        auto layer = TitleScene::create();
        scene->addChild(layer);
        _instance = layer;
        layer->setTag(6); // 设置Tag便于通过 getChildByTag 获取
        return scene;
    }

    /**
     * 获取全局单例.
     *
     * @return TitleScene 的当前实例指针.
     */
    static TitleScene* getInstance() { return _instance; }

private:
    /**
     * 初始化背景 UI.
     * 加载背景图片并根据屏幕尺寸进行适配.
     */
    void initBackground();

    /**
     * 初始化功能按钮.
     * 创建开始游戏、模式选择等菜单按钮并绑定回调.
     */
    void initButtons();

    /** 全局静态变量，用于保存当前场景实例. */
    static TitleScene* _instance;

    // ========== 按钮点击回调 ==========

    /**
     * 创造模式按钮点击回调.
     *
     * @param pSender 触发事件的对象.
     */
    void onCreateGameClick(Ref* pSender);

    /**
     * 关卡选择按钮点击回调 (预留).
     *
     * @param pSender 触发事件的对象.
     */
    void onLevelSelectClick(Ref* pSender);

    /**
     * 普通模式按钮点击回调.
     *
     * @param pSender 触发事件的对象.
     */
    void onNormalGameClick(Ref* pSender);

    /**
     * 设置按钮点击回调 (预留).
     *
     * @param pSender 触发事件的对象.
     */
    void onSettingClick(Ref* pSender);

    /**
     * 场景进入生命周期回调.
     * 当场景即将显示时调用，常用于调试或播放入场动画.
     */
    void onEnter();

    /**
     * 获取按钮矩形区域 (辅助函数).
     * 用于自定义触摸检测逻辑.
     *
     * @param btnSprite 按钮精灵对象.
     * @return 按钮的包围盒 Rect.
     */
    Rect getButtonRect(Sprite* btnSprite);
};

#endif // __TITLE_SCENE_H__