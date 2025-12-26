#ifndef __LEVEL_SCENE_H__
#define __LEVEL_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h" // 必须包含，否则 ui::Button 会报错
#include "VillageScene.h" // 确保父类头文件路径正确

// 如果 VillageScene 引用了 LevelScene，会形成循环包含。
// 此时需要在这里使用 class LevelScene; 前置声明（本例通常不需要）
class VillageScene;
    class LevelScene : public VillageScene
    {
    public:
        // 自定义工厂函数（适配 Scene 类型）
        static Scene* createWithLevel(const std::string& levelPath);

        // 重写 init（必须 public + virtual）
        virtual bool init() override;

    private:
        std::string _currentLevelPath;

        // UI 设置
        void setupLevelUI();

        // 回调函数
        void onEndBattleClicked(Ref* sender);
        void onReturnToVillageClicked(Ref* sender);
    };

#endif