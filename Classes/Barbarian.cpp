#include "Barbarian.h"
#include "cocos2d.h"
USING_NS_CC;

// 实现create方法（核心：初始化野蛮人配置）
BarbarianTroop* BarbarianTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) BarbarianTroop();
    if (troop) {
        TroopConfig config;
        config.id = 1001;
        config.type = TroopType::BARBARIAN;
        config.name = "Barbarian";
        config.imgPath = "troops/barbarian.png";
        config.hp = 400;
        config.attackPower = 80;
        config.attackRange = 50.0f;
        config.attackSpeed = 1.0f;
        config.moveSpeed = 120.0f;
        config.elixirCost = 25;
        config.trainingTime = 2.0f;
        config.level = 1;
        config.spaceCost = 1;

        // 调用初始化方法
        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    delete troop;
    return nullptr;
}

// 重写初始化方法（可扩展野蛮人特有初始化）
bool BarbarianTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    return true;
}

// 重写特有攻击行为（野蛮人：近战攻击，优先攻击最近建筑）
void BarbarianTroop::doSpecialAttack() {
    if (_attackTarget == nullptr) return;

    // 1. 攻击动作1：短暂放大（体现攻击发力）
    auto scaleUp = ScaleTo::create(0.1f, _mapScale * 1.2f); // 放大到1.2倍
    auto scaleDown = ScaleTo::create(0.1f, _mapScale);      // 恢复原大小
    auto attackScale = Sequence::create(scaleUp, scaleDown, nullptr);

    // 2. 攻击动作2：闪红（视觉反馈攻击生效）
    auto tintRed = TintTo::create(0.05f, 255, 50, 50);     // 闪红
    auto tintWhite = TintTo::create(0.05f, 255, 255, 255); // 恢复白色
    auto attackTint = Sequence::create(tintRed, tintWhite, nullptr);

    // 3. 同时播放缩放+闪红动画
    this->runAction(Spawn::create(attackScale, attackTint, nullptr));

}

// 重写特有描述（保留，用于UI展示；如果不需要可直接返回空字符串）
std::string BarbarianTroop::getSpecialDesc() {
    return "近战攻击单位，优先攻击最近的敌方建筑，攻击速度快，训练成本低";
}
