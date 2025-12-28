#ifndef __GIANT_TROOP_H__
#define __GIANT_TROOP_H__

#include "Troop.h"

/**
 * 巨人兵种类.
 * 继承自 BaseTroop，实现高血量、优先攻击建筑的肉盾逻辑.
 */
class GiantTroop : public BaseTroop {
public:
    /**
     * 创建巨人实例.
     * 工厂方法的核心实现.
     * * @param spawnPos 出生位置坐标.
     * @param mapScale 地图缩放比例.
     * @return 创建成功的 GiantTroop 对象指针.
     */
    static GiantTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 重写纯虚函数 ==========

    /**
     * 执行特有攻击逻辑.
     * 实现巨人的重拳攻击表现（缓慢放大、震动）.
     */
    virtual void doSpecialAttack() override;

    /**
     * 获取特有描述.
     * * @return 描述字符串 (如 "肉盾单位，优先攻击防御建筑...").
     */
    virtual std::string getSpecialDesc() override;

protected:
    /**
     * 初始化方法.
     * 调用父类初始化并设置特定属性（如巨大的体型缩放）.
     * * @param config 兵种配置数据.
     * @param spawnPos 出生坐标.
     * @param mapScale 地图缩放.
     * @return 初始化成功返回 true.
     */
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale) override;

private:
    /** 私有构造函数. */
    GiantTroop() = default;

    /** 私有析构函数. */
    ~GiantTroop() override = default;
};

#endif // __GIANT_TROOP_H__