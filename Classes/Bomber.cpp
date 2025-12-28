#include "Bomber.h"
#include "cocos2d.h"
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
        TroopConfig config;
        config.id = 1004;               // 炸弹人唯一ID
        config.type = TroopType::BOMBER;// 兵种类型
        config.name = "Bomber";         // 兵种名称
        config.imgPath = "troops/bomber.png"; // 纹理路径

        // --- 属性配置 ---
        config.hp = 100;                // 极低血量（核心特征：脆皮，一碰就炸）
        config.attackPower = 500;       // 超高范围伤害（主要针对城墙）
        config.attackRange = 80.0f;     // 攻击范围（自爆范围）
        config.attackSpeed = 0.0f;      // 无攻速（一次性攻击）
        config.moveSpeed = 90.0f;       // 移动速度中等

        // --- 训练消耗 ---
        config.elixirCost = 100;        // 训练成本（圣水100）
        config.trainingTime = 5.0f;     // 训练时长
        config.level = 1;               // 初始等级
        config.spaceCost = 2;           // 占用人口（2单位）

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
    if (_attackTarget == nullptr) return;

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