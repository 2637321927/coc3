#include "Giant.h"
#include "cocos2d.h"
USING_NS_CC;

// 实现create方法（核心：初始化巨人配置）
GiantTroop* GiantTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) GiantTroop();
    if (troop) {
        TroopConfig config;
        config.id = 1003;                  // 巨人唯一ID（区别于野蛮人1001/弓箭手1002）
        config.type = TroopType::GIANT;    // 兵种类型为巨人
        config.name = "Giant";             // 兵种名称
        config.imgPath = "troops/giant.png"; // 巨人纹理路径（替换为你的实际资源）
        config.hp = 1500;                  // 高血量（核心特征：肉盾）
        config.attackPower = 150;          // 高攻击力（近战重拳）
        config.attackRange = 60.0f;        // 攻击范围略大于野蛮人（体型大）
        config.attackSpeed = 2.5f;         // 低攻速（2.5秒/次，体现笨重）
        config.moveSpeed = 80.0f;          // 移动速度慢（体现体型大）
        config.elixirCost = 200; // 高训练成本（圣水200）
        config.trainingTime = 8.0f;        // 训练时长更长（8秒）
        config.level = 1;                  // 初始等级
        config.spaceCost = 5;              // 占用更多人口（5人口，体现体型）

        // 调用初始化方法
        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    delete troop;
    return nullptr;
}

// 重写初始化方法（可扩展巨人特有初始化）
bool GiantTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    // 巨人特有初始化
    this->setScale(mapScale * 1.5f); // 巨人默认比普通兵种大1.5倍
    return true;
}

// 重写特有攻击行为（巨人：近战重拳攻击，适配笨重特征的动画）
void GiantTroop::doSpecialAttack() {
    if (_attackTarget == nullptr) return;

    // 1. 攻击动作1：缓慢放大（体现巨人笨重的攻击节奏）
    auto scaleUp = ScaleTo::create(0.2f, _mapScale * 1.8f); // 放大到1.8倍（比野蛮人更大）
    auto scaleDown = ScaleTo::create(0.2f, _mapScale * 1.5f); // 恢复巨人基础大小
    auto attackScale = Sequence::create(scaleUp, scaleDown, nullptr);

    // 2. 攻击动作2：闪黄（区别于野蛮人闪红，体现巨人特征）
    auto tintYellow = TintTo::create(0.1f, 255, 200, 50);   // 闪黄
    auto tintWhite = TintTo::create(0.1f, 255, 255, 255);   // 恢复白色
    auto attackTint = Sequence::create(tintYellow, tintWhite, nullptr);

    // 3. 攻击动作3：轻微震动（体现重拳砸地的效果）
    auto shake1 = MoveBy::create(0.05f, Vec2(5, 0));        // 右移5像素
    auto shake2 = MoveBy::create(0.05f, Vec2(-10, 0));      // 左移10像素
    auto shake3 = MoveBy::create(0.05f, Vec2(5, 0));        // 回正
    auto attackShake = Sequence::create(shake1, shake2, shake3, nullptr);

    // 4. 同时播放缩放+闪黄+震动动画（贴合巨人笨重的攻击特征）
    this->runAction(Spawn::create(attackScale, attackTint, attackShake, nullptr));
}

// 重写特有描述（体现巨人肉盾+高伤特征）
std::string GiantTroop::getSpecialDesc() {
    return "重型近战单位，血量极高、攻击力强但攻速和移速缓慢，优先攻击防御建筑，占用5人口";
}