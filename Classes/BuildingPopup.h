#ifndef _BUILDINGPOPUP_H_
#define _BUILDINGPOPUP_H_

#include "cocos2d.h"
#include "Building.h"
#include <functional>

USING_NS_CC;

class BuildingPopup : public LayerColor {
public:
    // 按钮类型枚举
    enum class ButtonType {
        INFO,       // 信息
        UPGRADE,    // 升级
        COLLECT,    // 收集资源（仅金矿）
        DESTROY     // 摧毁
    };

    // 创建弹窗（传入目标建筑、按钮点击回调）
    static BuildingPopup* create(BaseBuilding* building,
        const std::function<void(ButtonType)>& btnCallback);

    bool init(BaseBuilding* building,
        const std::function<void(ButtonType)>& btnCallback) ;

private:
    BaseBuilding* _targetBuilding; // 关联的建筑
    std::function<void(ButtonType)> _btnCallback; // 按钮点击回调

    // 根据建筑类型生成按钮（金矿显示4个，其他建筑按需调整）
    void createButtons();
    // 创建单个按钮（图片路径、按钮类型、位置）
    void createButton(const std::string& imgPath, ButtonType type, const Vec2& pos);
};

#endif // BUILDINGPOPUP_H#pragma once
