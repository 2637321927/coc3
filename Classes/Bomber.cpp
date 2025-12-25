#include "Bomber.h"
#include "cocos2d.h"
USING_NS_CC;

// 实现create方法（核心：初始化炸弹人配置）
BomberTroop* BomberTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) BomberTroop();
    if (troop) {
        TroopConfig config;
        config.id = 1004;                  // 炸弹人唯一ID（区别于其他兵种）
        config.type = TroopType::BOMBER;   // 兵种类型为炸弹人
        config.name = "Bomber";            // 兵种名称
        config.imgPath = "troops/bomber.png"; // 炸弹人纹理路径（替换为你的实际资源）
        config.hp = 100;                   // 极低血量（核心特征：脆皮，一碰就炸）
        config.attackPower = 500;          // 超高范围伤害（只对城墙生效）
        config.attackRange = 80.0f;        // 攻击范围（自爆范围）
        config.attackSpeed = 0.0f;         // 无攻速（自爆一次就消失）
        config.moveSpeed = 90.0f;          // 移动速度中等（向城墙冲锋）
        config.elixirCost = 100; // 训练成本（圣水100）
        config.trainingTime = 5.0f;        // 训练时长（5秒）
        config.level = 1;                  // 初始等级
        config.spaceCost = 2;              // 占用人口（2人口）

        // 调用初始化方法
        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    delete troop;
    return nullptr;
}

// 重写初始化方法（可扩展炸弹人特有初始化）
bool BomberTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    // 炸弹人特有初始化
    this->setScale(mapScale * 0.8f);
    return true;
}

// 重写特有攻击行为（炸弹人：自爆攻击，范围伤害，攻击后自身消失）
void BomberTroop::doSpecialAttack() {
    if (_attackTarget == nullptr) return;

    // 1. 自爆前预热动画：快速闪烁（提醒玩家即将爆炸）
    auto blink = Blink::create(0.5f, 3); // 0.5秒内闪烁3次

    // 2. 自爆核心动画：放大+变橙红（爆炸视觉效果）
    auto scaleUp = ScaleTo::create(0.3f, _mapScale * 2.0f); // 放大到2倍（爆炸扩散）
    auto tintRed = TintTo::create(0.1f, 255, 80, 0);        // 橙红色（爆炸色）

    // 3. 爆炸后消失+回调（炸弹人自爆后死亡）
    auto explode = Sequence::create(
        blink,                                  // 预热闪烁
        Spawn::create(scaleUp, tintRed, nullptr),// 爆炸放大+变色
        CallFunc::create([=]() {
            // 自爆后触发死亡逻辑（直接调用die，无需等血量）
            this->die();
            }),
        nullptr
    );

    // 执行自爆动画
    this->runAction(explode);
}

// 重写特有描述（体现炸弹人自爆+拆墙特征）
std::string BomberTroop::getSpecialDesc() {
    return "自爆型单位，血量极低但对城墙造成超高范围伤害，攻击后自身消失，优先攻击城墙建筑";
}