#ifndef __ARCHER_TROOP_H__
#define __ARCHER_TROOP_H__

#include "Troop.h"

/**
 * 弓箭手兵种类.
 * 继承自 BaseTroop，实现远程攻击逻辑.
 */
class ArcherTroop : public BaseTroop {
public:
    /**
     * 创建弓箭手实例.
     * 工厂方法的核心实现.
     * * @param spawnPos 出生位置坐标.
     * @param mapScale 地图缩放比例.
     * @return 创建成功的 ArcherTroop 对象指针.
     */
    static ArcherTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 重写纯虚函数 ==========

    /**
     * 执行特有攻击逻辑.
     * 实现弓箭手的远程射击行为（发射箭矢）.
     */
    virtual void doSpecialAttack() override;

    /**
     * 获取特有描述.
     * * @return 描述字符串 (如 "远程攻击单位...").
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
    /** 私有构造函数，遵循 Cocos2d 内存管理规范. */
    ArcherTroop() = default;

    /** 私有析构函数. */
    ~ArcherTroop() override = default;
};

#endif // __ARCHER_TROOP_H__