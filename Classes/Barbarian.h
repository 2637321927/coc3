#ifndef __BARBARIAN_TROOP_H__
#define __BARBARIAN_TROOP_H__

#include "Troop.h"
#include "cocos2d.h"

class BarbarianTroop : public BaseTroop {
public:
    // 创建方法（工厂方法调用的核心）
    static BarbarianTroop* create(const cocos2d::Vec2& spawnPos, float mapScale);

    // 重写纯虚函数（野蛮人特有逻辑）
    virtual void doSpecialAttack() override;
    virtual std::string getSpecialDesc() override;

protected:
    // 重写初始化方法
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale) override;

private:
    BarbarianTroop() = default;
    ~BarbarianTroop() override = default;
};

#endif // __BARBARIAN_TROOP_H__
#pragma once
