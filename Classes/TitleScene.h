#ifndef TITLE_SCENE_H
#define TITLE_SCENE_H

#include "cocos2d.h"

class TitleScene : public cocos2d::Scene
{
public:
    // Cocos2d-x创建场景的标准宏
    CREATE_FUNC(TitleScene);
    virtual bool init() override;

private:
    // 初始化UI元素（背景、按钮）
    void initBackground();
    void initButtons();

    // 按钮点击回调（预留不同模式入口）
    void onStartGameClick(cocos2d::Ref* pSender);
    void onLevelSelectClick(cocos2d::Ref* pSender);
    void onSettingClick(cocos2d::Ref* pSender);

    // 适配按钮点击区域（可选，优化触摸体验）
    cocos2d::Rect getButtonRect(cocos2d::Sprite* btnSprite);
};

#endif // TITLE_SCENE_H#pragma once#pragma once
