#ifndef __TROOP_H__
#define __TROOP_H__

#include "cocos2d.h"
#include <functional>
#include "EnumType.h" 
#include "Building.h"
// 前置声明
class VillageScene;
class BaseBuilding;
// 通用兵种配置（所有兵种都有的属性）

// 抽象基类（不可实例化，只能继承）
class BaseTroop : public cocos2d::Sprite {
protected:
    BaseTroop() = default;
    ~BaseTroop() override;

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
    float _attackRange;
    float _attackPower;

    // 寻路相关成员
    std::vector<Vec2> _pathPoints;      // 路径点列表（瓦片坐标）
    int _currentPathIndex;              // 当前路径点索引
    cocos2d::Vec2 _targetWorldPos;      // 目标坐标
    VillageScene* _villageScene;        // 场景指针

    // 交互回调
    std::function<void(BaseTroop*, BaseBuilding*)> _attackCallback;
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
    // 在 public 下添加：
    // Troop.h -> public 区域

    BaseBuilding* getAttackTarget() const {
        return _attackTarget;
    }
    void setAttackTarget(BaseBuilding* target);
    void setTargetTilePosition(const cocos2d::Vec2& targetTilePos);
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
    float getAttackRange() const { return _attackRange; }
    float getAttackPower() const { return _attackPower; }
    void findNewTarget();
    // ========== 交互接口（通用） ==========
    void bindAttackCallback(const std::function<void(BaseTroop*, BaseBuilding*)>& callback);

    // ========== 纯虚接口（子类必须实现：差异化逻辑） ==========
    virtual void doSpecialAttack() = 0; // 兵种特有攻击行为（如炸弹人炸墙、弓箭手远程攻击）
    virtual std::string getSpecialDesc() = 0; // 特有描述（如“优先攻击城墙，造成范围伤害”）

    // ========== 生命周期（通用） ==========
    virtual void update(float dt) override; // 帧更新（处理移动、攻击冷却）

    // ========== 设置目标点并触发寻路 ==========
    void setTargetWorldPosition(const cocos2d::Vec2& targetPos);

    // ========== 获取当前所在瓦片坐标 ==========
    cocos2d::Vec2 getCurrentTilePos() const;

    // ============ 初始化场景指针 ==============
    void setVillageScene(VillageScene* scene) {
        _villageScene = scene;
    }
};

#endif // __TROOP_H__#pragma once
