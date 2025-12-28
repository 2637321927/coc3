#ifndef __BUILDING_POPUP_H__
#define __BUILDING_POPUP_H__

#include "cocos2d.h"
#include "Building.h"
#include <functional>

USING_NS_CC;

/**
 * 建筑功能弹窗层.
 * 点击建筑后弹出的操作界面，包含升级、信息、摧毁等功能按钮.
 */
class BuildingPopup : public LayerColor {
public:
    /**
     * 按钮功能类型枚举.
     */
    enum class ButtonType {
        INFO,       ///< 查看详细信息
        UPGRADE,    ///< 升级建筑
        COLLECT,    ///< 收集资源（仅金矿/圣水收集器）
        DESTROY,    ///< 摧毁建筑
        TRAINING    ///< 训练兵种（仅训练营）
    };

    /**
     * 创建弹窗实例 (工厂方法).
     * * @param building 关联的目标建筑指针.
     * @param btnCallback 按钮点击时的回调函数.
     * @return 创建成功的 BuildingPopup 对象指针.
     */
    static BuildingPopup* create(BaseBuilding* building,
        const std::function<void(ButtonType)>& btnCallback);

    /**
     * 初始化弹窗.
     * 设置背景、触摸吞噬及生成按钮.
     * * @param building 关联的目标建筑.
     * @param btnCallback 回调函数.
     * @return 初始化成功返回 true.
     */
    bool init(BaseBuilding* building,
        const std::function<void(ButtonType)>& btnCallback);

private:
    BaseBuilding* _targetBuilding; ///< 当前弹窗关联的建筑实例
    std::function<void(ButtonType)> _btnCallback; ///< 按钮点击回调存储

    /**
     * 生成按钮布局.
     * 根据建筑的类型（如金矿、训练营）和状态（空闲、建造中）生成对应的按钮列表.
     */
    void createButtons();

    /**
     * 创建单个功能按钮.
     * * @param imgPath 按钮图标路径.
     * @param type 按钮对应的功能枚举.
     * @param pos 按钮在弹窗中的位置.
     */
    void createButton(const std::string& imgPath, ButtonType type, const Vec2& pos);
};

#endif // __BUILDING_POPUP_H__