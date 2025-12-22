#ifndef __ARCHER_TROOP_H__
#define __ARCHER_TROOP_H__

#include "Troop.h"

// 弓箭手子类，继承自BaseTroop抽象基类
class ArcherTroop : public BaseTroop {
public:
    // 创建方法（工厂方法调用的核心）
    static ArcherTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // 重写纯虚函数（弓箭手特有逻辑）
    virtual void doSpecialAttack() override;
    virtual std::string getSpecialDesc() override;

protected:
    // 重写初始化方法
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale) override;

private:
    // 私有化构造/析构，遵循cocos2d的内存管理规范
    ArcherTroop() = default;
    ~ArcherTroop() override = default;
};

#endif // __ARCHER_TROOP_H__
#pragma once
