#ifndef __BARBARIAN_TROOP_H__
#define __BARBARIAN_TROOP_H__

#include "Troop.h"
#include "cocos2d.h"

/**
 * 野蛮人兵种类.
 * 继承自 BaseTroop，实现近战攻击逻辑.
 */
class BarbarianTroop : public BaseTroop {
public:
    /**
     * 创建野蛮人实例.
     * 工厂方法的核心实现.
     * * @param spawnPos 出生位置坐标.
     * @param mapScale 地图缩放比例.
     * @return 创建成功的 BarbarianTroop 对象指针.
     */
    static BarbarianTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 重写纯虚函数 ==========

    /**
     * 执行特有攻击逻辑.
     * 实现野蛮人的近战攻击表现（如缩放、闪烁）.
     */
    virtual void doSpecialAttack() override;

    /**
     * 获取特有描述.
     * * @return 描述字符串 (如 "近战攻击单位...").
     */
    virtual std::string getSpecialDesc() override;

protected:
    /**
     * 初始化方法.
     * 调用父类初始化并设置特定属性.
     * * @param config 兵种配置数据.
     * @param spawnPos 出生坐标.
     * @param mapScale 地图缩放.
     * @return 初始化成功返回 true.
     */
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale) override;

private:
    /** 私有构造函数. */
    BarbarianTroop() = default;

    /** 私有析构函数. */
    ~BarbarianTroop() override = default;
};

#endif // __BARBARIAN_TROOP_H__