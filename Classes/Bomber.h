#ifndef __BOMBER_TROOP_H__
#define __BOMBER_TROOP_H__

#include "Troop.h"

/**
 * 炸弹人兵种类.
 * 继承自 BaseTroop，实现自爆攻击逻辑.
 */
class BomberTroop : public BaseTroop {
public:
    /**
     * 创建炸弹人实例.
     * 工厂方法的核心实现.
     * * @param spawnPos 出生位置坐标.
     * @param mapScale 地图缩放比例.
     * @return 创建成功的 BomberTroop 对象指针.
     */
    static BomberTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 重写纯虚函数 ==========

    /**
     * 执行特有攻击逻辑.
     * 实现炸弹人的自爆攻击行为（闪烁、放大、变色）.
     */
    virtual void doSpecialAttack() override;

    /**
     * 获取特有描述.
     * * @return 描述字符串 (如 "自爆型单位...").
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
    BomberTroop() = default;

    /** 私有析构函数. */
    ~BomberTroop() override = default;
};

#endif // __BOMBER_TROOP_H__