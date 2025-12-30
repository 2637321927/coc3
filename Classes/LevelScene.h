#ifndef __LEVEL_SCENE_H__
#define __LEVEL_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" 
#include "VillageScene.h" 

// 前置声明，防止循环引用
class VillageScene;

/**
 * 关卡场景类.
 * 继承自 VillageScene，用于加载和展示特定关卡的内容（如战斗关卡、副本）.
 */
class LevelScene : public VillageScene
{
public:
    /**
     * 创建关卡场景实例 (静态工厂方法).
     * 根据指定的关卡文件路径创建场景.
     *
     * @param levelPath 关卡配置文件的路径 (如 "level1.txt").
     * @return 创建成功的 Scene 对象指针.
     */
    static Scene* createWithLevel(const std::string& levelPath);

    /**
     * 初始化场景.
     * 重写父类初始化方法，进行关卡特有的设置（如屏蔽主界面按钮）.
     *
     * @return 初始化成功返回 true，失败返回 false.
     */
    virtual bool init() override;

private:
    std::string _currentLevelPath; ///< 当前加载的关卡文件路径

    /**
     * 设置关卡专属 UI.
     * 初始化返回按钮、战斗结算界面等关卡特有的 UI 元素.
     */
    void setupLevelUI();

    /**
     * 战斗结束按钮回调.
     *
     * @param sender 触发事件的对象指针.
     */
    void onEndBattleClicked(Ref* sender);

    /**
     * 返回村庄按钮回调.
     * 点击后切换回主村庄场景.
     *
     * @param sender 触发事件的对象指针.
     */
    void onReturnToVillageClicked(Ref* sender);
};

#endif // __LEVEL_SCENE_H__