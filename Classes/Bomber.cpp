#include "Bomber.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;
USING_NS_CC;

/**
 * 创建炸弹人实例 (工厂方法).
 * 初始化炸弹人的特有配置（低血量、极高对墙伤害、自爆特性）并创建对象.
 * * @param spawnPos 出生位置坐标.
 * @param mapScale 地图缩放比例.
 * @return 创建成功的 BomberTroop 对象指针，失败返回 nullptr.
 */
BomberTroop* BomberTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) BomberTroop();
    if (troop) {
        // 【修改后】从全局配置表读取
        TroopConfig config = g_troopTrainConfig[TroopType::BOMBER];

        config.spaceCost = 2; // 炸弹人通常占2人口

        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    CC_SAFE_DELETE(troop);
    return nullptr;
}

/**
 * 初始化炸弹人.
 * 调用父类通用初始化逻辑，并设置特有的视觉缩放.
 * * @param config 兵种配置结构体.
 * @param spawnPos 出生位置.
 * @param mapScale 地图缩放比例.
 * @return 初始化成功返回 true.
 */
bool BomberTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    // 炸弹人特有初始化：体型稍小
    this->setScale(mapScale * 0.8f);
    return true;
}

/**
 * 执行炸弹人特有攻击行为（自爆）.
 * 包含三个阶段：快速闪烁预热 -> 放大变色爆炸 -> 自身销毁.
 */
void BomberTroop::doSpecialAttack() {
    //if (_attackTarget == nullptr) return;

    // 1. 自爆前预热动画：快速闪烁（提醒玩家即将爆炸）
    auto blink = Blink::create(0.5f, 3); // 0.5秒内闪烁3次

    // 2. 自爆核心动画：放大+变橙红（爆炸视觉效果）
    auto scaleUp = ScaleTo::create(0.3f, _mapScale * 2.0f); // 放大到2倍（爆炸扩散）
    auto tintRed = TintTo::create(0.1f, 255, 80, 0);        // 橙红色（爆炸色）

    // 3. 爆炸后消失+回调（炸弹人自爆后死亡）
    auto explode = Sequence::create(
        blink,                                   // 预热闪烁
        Spawn::create(scaleUp, tintRed, nullptr),// 爆炸放大+变色
        CallFunc::create([=]() {
            // 自爆后触发死亡逻辑（直接调用die，无需等血量扣减）
            this->die();
            }),
        nullptr
    );
    SimpleAudioEngine::getInstance()->playEffect("audio/bomber.mp3", false);
    // 执行自爆动画
    this->runAction(explode);
}

/**
 * 获取炸弹人特有描述.
 * * @return 描述字符串 (强调自爆和对墙伤害).
 */
std::string BomberTroop::getSpecialDesc() {
    return "自爆型单位，血量极低但对城墙造成超高范围伤害，攻击后自身消失，优先攻击城墙建筑";
}