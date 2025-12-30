#include "Giant.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;
USING_NS_CC;

/**
 * 创建巨人实例 (工厂方法).
 * 初始化巨人的特有配置（高血量、高攻击、移动慢、占人口多）并创建对象.
 * * @param spawnPos 出生位置坐标.
 * @param mapScale 地图缩放比例.
 * @return 创建成功的 GiantTroop 对象指针，失败返回 nullptr.
 */
GiantTroop* GiantTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) GiantTroop();
    if (troop) {
        // 【修改后】从全局配置表读取
        TroopConfig config = g_troopTrainConfig[TroopType::GIANT];

        // 巨人占5个人口，这里可以做个兜底，也可以完全信赖配置表
        config.spaceCost = 5;

        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    CC_SAFE_DELETE(troop);
    return nullptr;
}
/**
 * 初始化巨人.
 * 调用父类通用初始化逻辑，并设置特有的视觉缩放（体型巨大）.
 * * @param config 兵种配置结构体.
 * @param spawnPos 出生位置.
 * @param mapScale 地图缩放比例.
 * @return 初始化成功返回 true.
 */
bool GiantTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    // 巨人特有初始化：体型默认比普通兵种大1.5倍
    this->setScale(mapScale * 1.5f);
    return true;
}

/**
 * 执行巨人特有攻击行为.
 * 播放缓慢放大（蓄力）、闪黄、震动动画，模拟重拳攻击效果.
 */
void GiantTroop::doSpecialAttack() {
    //if (_attackTarget == nullptr) return;

    // 1. 攻击动作1：缓慢放大（体现巨人笨重的攻击节奏）
    auto scaleUp = ScaleTo::create(0.2f, _mapScale * 1.8f); // 放大到1.8倍
    auto scaleDown = ScaleTo::create(0.2f, _mapScale * 1.5f); // 恢复巨人基础大小
    auto attackScale = Sequence::create(scaleUp, scaleDown, nullptr);

    // 2. 攻击动作2：闪黄（区别于野蛮人闪红）
    auto tintYellow = TintTo::create(0.1f, 255, 200, 50);   // 闪黄
    auto tintWhite = TintTo::create(0.1f, 255, 255, 255);   // 恢复白色
    auto attackTint = Sequence::create(tintYellow, tintWhite, nullptr);

    // 3. 攻击动作3：轻微震动（体现重拳砸地的效果）
    auto shake1 = MoveBy::create(0.05f, Vec2(5, 0));        // 右移
    auto shake2 = MoveBy::create(0.05f, Vec2(-10, 0));      // 左移
    auto shake3 = MoveBy::create(0.05f, Vec2(5, 0));        // 回正
    auto attackShake = Sequence::create(shake1, shake2, shake3, nullptr);
    SimpleAudioEngine::getInstance()->playEffect("audio/giant_hit.mp3", false);
    // 4. 同时播放缩放+闪黄+震动动画
    this->runAction(Spawn::create(attackScale, attackTint, attackShake, nullptr));
}

/**
 * 获取巨人特有描述.
 * * @return 描述字符串 (强调肉盾、高人口、优先攻击防御建筑).
 */
std::string GiantTroop::getSpecialDesc() {
    return "重型近战单位，血量极高、攻击力强但攻速和移速缓慢，优先攻击防御建筑，占用5人口";
}