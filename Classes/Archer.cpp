#include "Archer.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;
USING_NS_CC;

/**
 * 创建弓箭手实例 (工厂方法).
 * 初始化弓箭手的特有配置（血量、攻击力、攻击范围等）并创建对象.
 * * @param spawnPos 出生位置坐标.
 * @param mapScale 地图缩放比例.
 * @return 创建成功的 ArcherTroop 对象指针，失败返回 nullptr.
 */
 // [Archer.cpp]



ArcherTroop* ArcherTroop::create(const Vec2& spawnPos, float mapScale) {
    auto troop = new (std::nothrow) ArcherTroop();
    if (troop) {
        // 【修改后】从全局配置表读取，支持动态升级
        TroopConfig config = g_troopTrainConfig[TroopType::ARCHER];

        // 保留必要的硬性修正（防止配置表漏填）
        config.spaceCost = 1;

        if (troop->init(config, spawnPos, mapScale)) {
            troop->autorelease();
            return troop;
        }
    }
    CC_SAFE_DELETE(troop);
    return nullptr;
}
// ... 其他代码不变
/**
 * 初始化弓箭手.
 * 调用父类通用初始化逻辑，设置UI和状态.
 * * @param config 兵种配置结构体.
 * @param spawnPos 出生位置.
 * @param mapScale 地图缩放比例.
 * @return 初始化成功返回 true.
 */
bool ArcherTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    // 调用父类通用初始化（血条、等级标签、状态等）
    if (!BaseTroop::init(config, spawnPos, mapScale)) {
        return false;
    }

    return true;
}

/**
 * 执行弓箭手特有攻击逻辑.
 * 创建箭矢精灵，播放从当前位置飞向目标的动画.
 */
void ArcherTroop::doSpecialAttack() {
    //if (_attackTarget == nullptr) return;

    // 1. 播放射箭动画
    auto arrow = Sprite::create("troops/arrow.png");
    if (arrow) {
        arrow->setPosition(this->getPosition());
        // 注意：将箭矢添加到父节点（地图层），以保证层级正确，而不是加在弓箭手身上
        if (this->getParent()) {
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
    }
    SimpleAudioEngine::getInstance()->playEffect("audio/archer_hit.mp3", false);
}

/**
 * 获取弓箭手特有描述.
 * * @return 描述字符串 (包含攻击偏好、范围特点等).
 */
std::string ArcherTroop::getSpecialDesc() {
    return "远程攻击单位，优先攻击最远的敌方建筑，攻击范围大但血量较低，训练成本中等";
}