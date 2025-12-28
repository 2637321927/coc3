#ifndef __TROOP_H__
#define __TROOP_H__

#include "cocos2d.h"
#include <functional>
#include "EnumType.h" 
#include "Building.h"

// 前置声明，防止循环引用
class VillageScene;
class BaseBuilding;

/**
 * 兵种抽象基类.
 * 所有具体兵种（野蛮人、弓箭手等）的父类.
 * 负责通用逻辑：移动、状态管理、生命值管理、UI显示、寻路等.
 * 不可直接实例化，必须通过工厂方法 create 创建具体子类.
 */
class BaseTroop : public cocos2d::Sprite {
protected:
    /** 受保护的构造函数，确保只能通过工厂方法创建. */
    BaseTroop() = default;
    /** 虚析构函数，确保子类析构正确执行. */
    ~BaseTroop() override = default;

    // ========== 通用辅助方法（子类可调用） ==========

    /** 初始化通用 UI 组件 (血条、等级标签). */
    void initCommonUI();

    /** 更新攻击冷却计时器. */
    void updateAttackCD();

    /** * 更新移动逻辑.
     * 处理沿寻路路径点的移动插值.
     * @param dt 帧间隔时间.
     */
    void updateMovement(float dt);

    // ========== 通用成员变量（子类可访问） ==========

    TroopConfig _config;                ///< 兵种通用配置数据
    TroopState _state = TroopState::UNKNOWN; ///< 当前兵种状态
    cocos2d::Vec2 _spawnPos;            ///< 出生位置 (世界坐标)
    cocos2d::Vec2 _targetPos;           ///< 移动目标位置 (世界坐标)
    BaseBuilding* _attackTarget = nullptr; ///< 当前锁定的攻击目标建筑
    float _mapScale = 1.0f;             ///< 当前地图缩放比例
    float _trainingTimer = 0.0f;        ///< 训练进度计时器
    float _attackCDTimer = 0.0f;        ///< 攻击冷却计时器
    int _currentHp;                     ///< 当前剩余生命值
    cocos2d::ProgressTimer* _hpBar = nullptr; ///< 通用血条节点

    // ========== 寻路相关成员 ==========

    // 【修复关键点】加上 cocos2d:: 前缀
    std::vector<cocos2d::Vec2> _pathPoints;      ///< 当前路径点列表 (瓦片坐标序列)
    int _currentPathIndex;              ///< 当前正在前往的路径点索引
    cocos2d::Vec2 _targetWorldPos;      ///< 最终目标的精确世界坐标
    VillageScene* _villageScene;        ///< 所属场景指针 (用于坐标转换和获取地图数据)

    // ========== 交互回调 ==========

    /** 攻击发生时的回调函数 (用于通知场景层进行伤害结算). */
    std::function<void(BaseTroop*, BaseBuilding*)> _attackCallback;

public:
    /**
     * 兵种工厂方法.
     * 根据类型创建对应的子类实例 (如 BarbarianTroop, ArcherTroop).
     * * @param type 兵种类型枚举.
     * @param spawnPos 出生位置.
     * @param mapScale 地图缩放.
     * @return 创建成功的子类对象指针 (基类指针接收).
     */
    static BaseTroop* create(TroopType type, const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 通用初始化（子类可重写） ==========

    /**
     * 初始化兵种.
     * 加载纹理、设置属性、初始化UI.
     * * @param config 配置数据.
     * @param spawnPos 出生坐标.
     * @param mapScale 地图缩放.
     * @return 初始化成功返回 true.
     */
    virtual bool init(const TroopConfig& config, const cocos2d::Vec2& spawnPos, float mapScale);

    // ========== 通用状态管理（所有兵种都有） ==========

    /** 开始训练 (进入 TRAINING 状态). */
    virtual void startTrain();

    /** 完成训练 (进入 IDLE 状态). */
    virtual void finishTrain();

    /** * 开始攻击指定目标.
     * 切换到 ATTACKING 状态并锁定目标.
     * @param target 目标建筑.
     */
    virtual void startAttack(BaseBuilding* target);

    /** * 受到伤害.
     * 扣减 HP 并更新 UI，若 HP<=0 则死亡.
     * @param damage 伤害数值.
     */
    virtual void takeDamage(int damage);

    /** 死亡处理 (播放动画、移除自身). */
    virtual void die();

    /** * 设置当前状态.
     * @param state 新状态枚举.
     */
    void setState(TroopState state);

    /** 获取当前状态. */
    TroopState getState() const { return _state; }

    /** * 设置简单移动目标 (不寻路，直接设置状态).
     * @param targetPos 目标坐标.
     */
     // 【修复关键点】加上 cocos2d::
    void setTargetPos(const cocos2d::Vec2& targetPos) {
        _targetPos = targetPos;
        setState(TroopState::MOVING);
    }

    // ========== 通用属性接口（所有兵种都有） ==========

    /** 获取兵种类型. */
    TroopType getType() const { return _config.type; }

    /** 获取出生点坐标. */
    cocos2d::Vec2 getSpawnPos() const { return _spawnPos; }

    /** 获取兵种等级. */
    int getLevel() const { return _config.level; }

    /** 获取完整配置引用. */
    const TroopConfig& getConfig() const { return _config; }

    /** 获取当前生命值. */
    int getCurrentHp() const { return _currentHp; }

    /** * 同步地图缩放.
     * 调整自身及 UI 的缩放以匹配地图.
     * @param mapScale 新的缩放比例.
     */
    void syncScale(float mapScale);

    // ========== 交互接口（通用） ==========

    /** * 绑定攻击回调.
     * @param callback 回调函数.
     */
    void bindAttackCallback(const std::function<void(BaseTroop*, BaseBuilding*)>& callback);

    // ========== 纯虚接口（子类必须实现：差异化逻辑） ==========

    /** * 执行兵种特有攻击行为.
     * 子类需实现具体的视觉表现 (如近战挥砍、远程射箭、炸弹人自爆).
     */
    virtual void doSpecialAttack() = 0;

    /** * 获取兵种特有描述.
     * @return 描述字符串.
     */
    virtual std::string getSpecialDesc() = 0;

    // ========== 生命周期（通用） ==========

    /** * 帧更新函数.
     * 处理状态机逻辑 (移动、攻击冷却、训练进度).
     * @param dt 时间间隔.
     */
    virtual void update(float dt) override;

    // ========== 寻路与定位 ==========

    /** * 设置世界坐标目标并触发寻路.
     * 使用 A* 算法计算路径并开始移动.
     * @param targetPos 目标点的世界坐标.
     */
    void setTargetWorldPosition(const cocos2d::Vec2& targetPos);

    /** * 获取当前所在的瓦片坐标.
     * @return 瓦片逻辑坐标 (Grid Coord).
     */
    cocos2d::Vec2 getCurrentTilePos() const;

    /** * 注入场景指针.
     * 兵种需要访问场景以获取地图数据进行寻路.
     * @param scene 场景实例指针.
     */
    void setVillageScene(VillageScene* scene) {
        _villageScene = scene;
    }
};

#endif // __TROOP_H__