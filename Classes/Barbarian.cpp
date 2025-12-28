#include "Barbarian.h"
#include "cocos2d.h"
USING_NS_CC;

/**
 * 创建野蛮人实例 (工厂方法).
 * 初始化野蛮人的基础属性配置（高血量、近战、移动快）并创建对象.
 * * @param spawnPos 出生位置坐标.
 * @param mapScale 地图缩放比例.
 * @return 创建成功的 BarbarianTroop 对象指针，失败返回 nullptr.
 */
BarbarianTroop* BarbarianTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) BarbarianTroop();
    if (troop) {
        TroopConfig config;
        config.id = 1001;               // 野蛮人唯一ID
        config.type = TroopType::BARBARIAN; // 兵种类型
        config.name = "Barbarian";      // 兵种名称
        config.imgPath = "troops/barbarian.png"; // 纹理路径

        // --- 属性配置 ---
        config.hp = 400;                // 生命值（较高，肉盾）
        config.attackPower = 80;        // 攻击力
        config.attackRange = 50.0f;     // 攻击范围（近战）
        config.attackSpeed = 1.0f;      // 攻击速度（较快）
        config.moveSpeed = 120.0f;      // 移动速度（快）

        // --- 训练消耗 ---
        config.elixirCost = 25;         // 训练消耗
        config.trainingTime = 2.0f;     // 训练时长
        config.level = 1;               // 初始等级
        config.spaceCost = 1;           // 占用人口

        // 调用初始化方法
        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    CC_SAFE_DELETE(troop);
    return nullptr;
}

/**
 * 初始化野蛮人.
 * 调用父类通用初始化逻辑.
 * * @param config 兵种配置结构体.
 * @param spawnPos 出生位置.
 * @param mapScale 地图缩放比例.
 * @return 初始化成功返回 true.
 */
bool BarbarianTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    return true;
}

/**
 * 执行野蛮人特有攻击表现.
 * 播放 "放大-缩小" 动画模拟挥砍动作，并闪烁红色模拟发力.
 */
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

/**
 * 获取野蛮人特有描述.
 * * @return 描述字符串 (近战、优先攻击最近建筑等).
 */
std::string BarbarianTroop::getSpecialDesc() {
    return "近战攻击单位，优先攻击最近的敌方建筑，攻击速度快，训练成本低";
}