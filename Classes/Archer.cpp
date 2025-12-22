#include "Archer.h"
#include "cocos2d.h"
USING_NS_CC;

// 实现create方法（核心：初始化弓箭手配置）
ArcherTroop* ArcherTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) ArcherTroop();
    if (troop) {
        TroopConfig config;
        config.id = 1002;                  // 弓箭手唯一ID（区别于野蛮人1001）
        config.type = TroopType::ARCHER;   // 兵种类型为弓箭手
        config.name = "Archer";            // 兵种名称
        config.imgPath = "troops/archer.png"; // 弓箭手纹理路径（需替换为你的实际资源）
        config.hp = 200;                   // 生命值（比野蛮人低，远程脆皮）
        config.attackPower = 60;           // 攻击力（远程攻击，略低于野蛮人）
        config.attackRange = 200.0f;       // 攻击范围（远程核心，远大于野蛮人）
        config.attackSpeed = 1.5f;         // 攻击速度（比野蛮人慢，远程平衡）
        config.moveSpeed = 100.0f;         // 移动速度（比野蛮人稍慢）
        config.trainingCost = { {"Elixir", 50} }; // 训练消耗（圣水50，比野蛮人高）
        config.trainingTime = 3.0f;        // 训练时长（3秒）
        config.level = 1;                  // 初始等级
        config.spaceCost = 1;              // 占用人口（和野蛮人一致）

        // 调用初始化方法
        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    delete troop;
    return nullptr;
}

// 重写初始化方法（可扩展弓箭手特有初始化）
bool ArcherTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    return true;
}

// 重写特有攻击行为（弓箭手：远程射箭，优先攻击最远的建筑）
void ArcherTroop::doSpecialAttack() {
    if (_attackTarget == nullptr) return;

    // 1. 播放射箭动画
    auto arrow = Sprite::create("troops/arrow.png");
    arrow->setPosition(this->getPosition());
    this->getParent()->addChild(arrow);
    // 箭头向目标建筑移动
    arrow->runAction(Sequence::create(
        MoveTo::create(0.5f, _attackTarget->getPosition()),
        CallFunc::create([=]() {
            arrow->removeFromParent(); // 箭到达后消失
            }),
        nullptr
    ));

}

// 重写特有描述（保留，用于UI展示；如果不需要可直接返回空字符串）
std::string ArcherTroop::getSpecialDesc() {
    return "远程攻击单位，优先攻击最远的敌方建筑，攻击范围大但血量较低，训练成本中等";
}