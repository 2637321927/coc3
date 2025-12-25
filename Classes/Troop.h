#ifndef __TROOP_H__
#define __TROOP_H__

#include "cocos2d.h"
#include <string>
#include "Building.h"
#include <unordered_map>
#include <functional>

// 前置声明
class VillageScene;

// 兵种类型枚举
enum class TroopType {
    BARBARIAN,    // 野蛮人
    ARCHER,       // 弓箭手
    GIANT,        // 巨人
    BOMBER,       // 炸弹人
    UNKNOWN
};

// 兵种状态枚举
enum class TroopState {
    IDLE,         // 闲置
    MOVING,       // 移动中
    ATTACKING,    // 攻击中
    DEAD,         // 死亡
    TRAINING,     // 训练中
    UNKNOWN
};

// 通用兵种配置（所有兵种都有的属性）
// 特有属性在子类内部private
struct TroopConfig {
    int id;                     // 唯一ID
    TroopType type;             // 兵种类型
    std::string name;           // 名称
    std::string imgPath;        // 纹理路径
    int hp;                     // 生命值
    int attackPower;            // 攻击力
    float attackRange;          // 攻击范围（像素）
    float attackSpeed;          // 攻击速度（攻击间隔，秒/次）
    float moveSpeed;            // 移动速度（像素/秒）
    int elixirCost; // 训练消耗（圣水）
    float trainingTime;         // 训练时长（秒）
    int level = 1;              // 初始等级
    int spaceCost;              // 占用人口空间
	int unlockCampLevel=1;        // 解锁所需兵营等级
};

// 抽象基类（不可实例化，只能继承）
class BaseTroop : public cocos2d::Sprite {
public:
    // 工厂方法（创建子类实例，基类指针接收）
    static BaseTroop* create(TroopType type, const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 通用初始化（子类可重写） ==========
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 通用状态管理（所有兵种都有） ==========
    virtual void startTrain();          // 开始训练（通用逻辑）
    virtual void finishTrain();         // 完成训练（通用逻辑）
    virtual void startAttack(BaseBuilding* target); // 开始攻击（通用逻辑）
    virtual void takeDamage(int damage); // 受到伤害（通用逻辑）
    virtual void die();                 // 死亡（通用逻辑）
    void setState(TroopState state);    // 设置状态（通用）
    TroopState getState() const { return _state; }
    void setTargetPos(const Vec2& targetPos) {
        _targetPos = targetPos;
        setState(TroopState::MOVING);
    }
    // ========== 通用属性接口（所有兵种都有） ==========
    TroopType getType() const { return _config.type; }
    cocos2d::Vec2 getSpawnPos() const { return _spawnPos; }
    int getLevel() const { return _config.level; }
    const TroopConfig& getConfig() const { return _config; }
    int getCurrentHp() const { return _currentHp; }
    void syncScale(float mapScale);     // 同步地图缩放（通用）

    // ========== 交互接口（通用） ==========
    void bindAttackCallback(const std::function<void(BaseTroop*, BaseBuilding*)>& callback);

    // ========== 纯虚接口（子类必须实现：差异化逻辑） ==========
    virtual void doSpecialAttack() = 0; // 兵种特有攻击行为（如炸弹人炸墙、弓箭手远程攻击）
    virtual std::string getSpecialDesc() = 0; // 特有描述（如“优先攻击城墙，造成范围伤害”）

    // ========== 生命周期（通用） ==========
    virtual void update(float dt) override; // 帧更新（处理移动、攻击冷却）

protected:
    BaseTroop() = default;
    ~BaseTroop() override = default;

    // 通用辅助方法（子类可调用）
    void initCommonUI();                // 初始化通用UI（血条、等级标签）
    void updateAttackCD();              // 更新攻击冷却
    void updateMovement(float dt);      // 更新移动逻辑

    // 通用成员（子类可访问）
    TroopConfig _config;                // 通用配置
    TroopState _state = TroopState::UNKNOWN;
    cocos2d::Vec2 _spawnPos;            // 出生坐标
    cocos2d::Vec2 _targetPos;           // 目标坐标
    BaseBuilding* _attackTarget = nullptr; // 攻击目标
    float _mapScale = 1.0f;             // 地图缩放比例
    float _trainingTimer = 0.0f;        // 训练计时器
    float _attackCDTimer = 0.0f;        // 攻击冷却计时器
    int _currentHp;                     // 当前生命值
    cocos2d::ProgressTimer* _hpBar = nullptr; // 通用血条

    // 交互回调
    std::function<void(BaseTroop*, BaseBuilding*)> _attackCallback;
};

#endif // __TROOP_H__#pragma once
