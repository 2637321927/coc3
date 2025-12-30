#include "Barbarian.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;
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


        // 【修改后】 直接使用全局配置表中的数据！
        // 这样当我们在升级按钮里修改了全局数据后，这里创建的新兵种就会自动拥有新属性
        TroopConfig config = g_troopTrainConfig[TroopType::BARBARIAN];

        // 依然保留手动设置人口，防止配置表漏填（可选）
        config.spaceCost = 1;

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
    //if (_attackTarget == nullptr) return;

    // 1. 攻击动作1：短暂放大（体现攻击发力）
    auto scaleUp = ScaleTo::create(0.1f, _mapScale * 1.2f); // 放大到1.2倍
    auto scaleDown = ScaleTo::create(0.1f, _mapScale);      // 恢复原大小
    auto attackScale = Sequence::create(scaleUp, scaleDown, nullptr);

    // 2. 攻击动作2：闪红（视觉反馈攻击生效）
    auto tintRed = TintTo::create(0.05f, 255, 50, 50);     // 闪红
    auto tintWhite = TintTo::create(0.05f, 255, 255, 255); // 恢复白色
    auto attackTint = Sequence::create(tintRed, tintWhite, nullptr);
    SimpleAudioEngine::getInstance()->playEffect("audio/barbarian_hit.mp3", false);
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