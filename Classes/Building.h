#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <unordered_map>
#include <functional>
#include "ui/CocosGUI.h" 
#include "EnumType.h" 
USING_NS_CC;
// 前置声明
class VillageScene;
extern std::unordered_map<TroopType, TroopConfig> g_troopTrainConfig;
class BaseTroop;
// 通用建筑配置（所有建筑都有的属性）
//特有属性在子类内部praivate

// 抽象基类（不可实例化，只能继承）
class BaseBuilding : public cocos2d::Node
{
public:
    // 工厂方法（创建子类实例，基类指针接收）
    static BaseBuilding* create(BuildingType type, const cocos2d::Vec2& tilePos, float mapScale);

    // ========== 通用初始化（子类可重写） ==========
    bool init(const BuildingConfig& config, const cocos2d::Vec2& tilePos, float mapScale);
    bool loadBuildingSprite(); // 加载建筑图片精灵（通用）
    // ========== 通用状态管理（所有建筑都有） ==========
    virtual void startBuild();          // 开始建造（通用逻辑）
    virtual void finishBuild();         // 完成建造（通用逻辑）
    virtual void startUpgrade();        // 开始升级（通用逻辑）
    virtual void destroy();             // 摧毁建筑（通用逻辑）
    virtual void takeDamage(int damage); // 受到伤害（通用逻辑）
    virtual void updateBuildingSprite(); // 更新建筑精灵图片（等级变化时调用）
    virtual void refreshBuildingAttributes();    // 刷新建筑属性（生命值/产量/容量等）
    //virtual bool checkUpgradeCondition(int& outErrCode); // 检查升级条件（通用）
    int getLevel() const { return _currentLevel; }
    // 设置等级（内部调用，用于加载存档/升级完成）
    void setLevel(int level) {
        _currentLevel = level;
        updateBuildingSprite();
        updateLevelLabel();
        refreshBuildingAttributes();
    }
    // 升级前置校验（资源是否足够、是否在升级中）
    //bool checkUpgradeCondition(int& outErrCode); // outErrCode：1=等级已满，2=资源不足，3=升级中
    // 开始升级（扣除资源，设置升级状态，启动倒计时）
    // 升级完成（更新等级、属性、图片，恢复状态）
    void finishUpgrade();
    void setState(BuildingState state); // 设置状态（通用）
    BuildingState getState() const { return _state; }
    // ========== 通用属性接口（所有建筑都有） ==========
    BuildingType getType() const { return _config.type; }
    Vec2 getTilePos() const {
        if (_tilePos.empty()) {
            CCLOG("empty:_tilePos ！");
            return Vec2::ZERO; // 返回一个默认值，避免崩溃
        }
        // 语义：返回建筑的第一个瓦片坐标（通常是起始/左下角瓦片）
        return _tilePos[0];
    }
    std::vector<Vec2> getTilePositions() const { return _tilePos; }
    void setTilePos(Vec2 tilePos) {
        _tilePos.push_back(tilePos);
    }
    void clearTilePos() {
        _tilePos.clear();
    }
    int getCurrentHp() { return _currentHp; }
    // int getLevel() const { return _config.level; }
    void buildImmediately() { immediatelyBuild = 1; }
    const BuildingConfig& getConfig() const { return _config; }
    void setBuildTime(float time) { _config.buildTime = time; }
    void syncScale(float mapScale);     // 同步地图缩放（通用）
    void setProgressTimer(float progressTimer) { _progressTimer = progressTimer; }
    float getProgressTimer() { return _progressTimer; }
    // ========== 交互接口（通用） ==========
    void bindClickCallback(const std::function<void(BaseBuilding*)>& callback);
    std::function<void(BaseBuilding*)> _buildFinishCallback;
    void bindBuildFinishCallback(const std::function<void(BaseBuilding*)>& callback);
    // ========== 纯虚接口（子类必须实现：差异化逻辑） ==========
    virtual void doSpecialAction() = 0; // 建筑特有行为（如农场产粮、城镇中心造兵）
    virtual std::string getSpecialDesc() = 0; // 特有描述（如“每10秒产10粮食”）

    // ========== 生命周期（通用） ==========
    virtual void update(float dt) override; // 帧更新（处理进度）
    bool immediatelyBuild = 0; // 是否立即建造完成
protected:
    BaseBuilding() = default;
    ~BaseBuilding() override = default;
    Label* _levelLabel;
    // 通用辅助方法（子类可调用）
    void initCommonUI();                // 初始化通用UI（进度条、等级标签）
    void initTouchListener(); // 初始化触摸监听器
    void updateProgress();              // 更新建造/升级进度
    void updateLevelLabel();
    // 点击回调函数（参数为当前建筑指针）
    std::function<void(BaseBuilding*)> _clickCallback;
    // 触摸事件监听器
    EventListenerTouchOneByOne* _touchListener;
    // 通用成员（子类可访问）
    BuildingConfig _config;             // 通用配置
    //需要在public加一些接口供外部访问
    BuildingState _state = BuildingState::UNKNOWN;  // 当前状态
    std::vector<Vec2> _tilePos;             // 占用的瓦片坐标(左上角)
    int _currentHp;          // 当前生命值
    int _currentLevel = 1;		  // 当前等级
    float _mapScale = 1.0f;             // 地图缩放比例
    float _progressTimer = 0.0f;        // 进度计时器
    float _upgradeRemainingTime; // 升级剩余时间（秒）
    ProgressTimer* _progressBar = nullptr; // 通用进度条
    ProgressTimer* _hpBar = nullptr; // 通用血条
    Sprite* _buildingSprite;        // 建筑图片精灵
};
class GoldMine : public BaseBuilding {
public:
    static GoldMine* create(const cocos2d::Vec2& tilePos, float mapScale);
    bool init(const cocos2d::Vec2& tilePos, float mapScale);

    // 重写专属行为：生产金币
    void doSpecialAction() override;
    // 重写专属描述
    std::string getSpecialDesc() override;
    // 收集金币
    int collectGold();
    int  getProduce() { return _goldPerInterval; }
    void produceGold(float dt);
    int getProduceInterval() { return  _produceInterval; };
    int getStored() { return _goldStored; };
    void destroy() override;
    void upGrade() {
        if (_currentLevel != 1) {
            _goldPerInterval += 100;
            _produceInterval -= 0.1f;
        }
    }
private:
    float _goldTimer = 0.0f;    // 金币生产计时器
    int _goldPerInterval = 100;  // 每次生产金币数量
    float _produceInterval = 2.0f; // 生产间隔（秒）
    int _goldStored = 0;      // 已生产但未收集的金币数量
};
class ElixirCollector : public BaseBuilding {
public:
    static ElixirCollector* create(const cocos2d::Vec2& tilePos, float mapScale);
    bool init(const cocos2d::Vec2& tilePos, float mapScale);
    // 重写专属行为：生产圣水
    void doSpecialAction() override;
    // 重写专属描述
    std::string getSpecialDesc() override;
    // 收集圣水
    int collectElixir();
    void produceElixir(float dt);
    void destroy() override;
    int getProduce() { return _elixirPerInterval; };
    int getProduceInterval() { return  _produceInterval; };
    int getStored() { return _elixirStored; };
    void upGrade() {
        if (_currentLevel != 1) {
            _elixirPerInterval += 100;
            _produceInterval -= 0.1f;
        }
    }
private:
    float _elixirTimer = 0.0f;    // 圣水生产计时器
    int _elixirPerInterval = 100;  // 每次生产圣水数量
    float _produceInterval = 2.0f; // 生产间隔（秒）
    int _elixirStored = 0;      // 已生产但未收集的圣水数量
};
// ========== 训练营类 ==========
class TrainingCamp : public BaseBuilding {
public:
    static TrainingCamp* create(const cocos2d::Vec2& tilePos, float mapScale);
    bool init(const cocos2d::Vec2& tilePos, float mapScale);
    void setTrainFinishCallback(const  std::function<void(TroopType)>& callback) {
        _trainFinishCallback = callback;
    }
    // 获取当前训练队列
    std::vector<TroopType>& getTrainQueue() { return _trainQueue; }
    // 新增训练任务
    void addTrainTask(TroopType type);
    // 移除指定位置的训练任务
    void removeTrainTask(int index);
    // 获取队列倒计时（和队列一一对应）
    std::vector<float>& getQueueTimers() { return _queueTimers; }
    // 实现基类虚函数
    void doSpecialAction() override; // 训练士兵逻辑
    std::string getSpecialDesc() override;
    void destroy();
    void update(float dt);
    // 核心：获取当前训练状态（供弹窗显示）
    bool isTraining() const { return _state == BuildingState::TRAINING && !_trainQueue.empty(); }
    TroopType getCurrentTrainingTroop() const { return isTraining() ? _trainQueue.front() : TroopType::UNKNOWN; }
    float getCurrentTrainRemainingTime() const { return _currentRemainingTime; } // 当前队列头剩余时间

private:
    std::function<void(TroopType)> _trainFinishCallback;
    std::unordered_map<TroopType, float> _troopTrainTimeMap; // 各兵种训练时长（预配置）
    float _trainTimer = 0.0f;    // 训练计时器
    float _trainInterval = 5.0f; // 训练间隔（秒）
    int _troopsInTraining = 0;   // 正在训练的士兵数量
    float _currentRemainingTime = 0.0f;      // 当前队列头兵种的剩余训练时间
    std::vector<TroopType> _trainQueue; // 训练队列
    std::vector<float> _queueTimers; // 队列倒计时
    // 内部逻辑：当前任务训练完成
    void finishCurrentTrainTask();
    // 内部逻辑：启动下一个队列任务
    void startNextTrainTask();
    // 内部私有方法：处理训练完成逻辑
    void finishTrainTroop(TroopType type);
    // 初始化兵种训练时间配置
    void initTroopTrainTimeConfig();
};
//========== 兵营类 ==========
class Barracks : public BaseBuilding {
public:
    static Barracks* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);

    // 重写特殊行为
    void doSpecialAction() override;
    std::string getSpecialDesc() override;
    // 训练士兵接口
    int getTroopSpace()const { return _maxTroopSpace; }; // 获取容量
    int getPulseSpace()const { return _pulseSpace; }; // 获取升级后新增容量
    //应重写升级函数，升级后增加总容量
    void upGrade() {
        if (_currentLevel != 1) {
            _maxTroopSpace += 20;
            _pulseSpace = 20;
        }
    }
private:
    int _maxTroopSpace=20*_currentLevel; // 总容量
    int _pulseSpace=20; // 升级增加的容量

};
// ========== 大本营类 ==========
class TownHall : public BaseBuilding {
public:
    static TownHall* create(const cocos2d::Vec2& tilePos, float mapScale);
    virtual bool init(const cocos2d::Vec2& tilePos, float mapScale);

    // 实现基类虚函数
    virtual void doSpecialAction() override; // 升级解锁逻辑
    virtual std::string getSpecialDesc() override { return "村庄的核心建筑"; }
    std::vector<BuildingType>
        getCanPlaceBuilding() const { 
        std::vector<BuildingType> canPlaceBuilding;
        if (_currentLevel == 1) {
            canPlaceBuilding.push_back(BuildingType::GOLD_MINE);
            canPlaceBuilding.push_back(BuildingType::ELIXIR_BOTTLE);
            canPlaceBuilding.push_back(BuildingType::ELIXIR_COLLECTOR);
            canPlaceBuilding.push_back(BuildingType::VAULT);
            canPlaceBuilding.push_back(BuildingType::BARRACKS);
            canPlaceBuilding.push_back(BuildingType::TRAINING_CAMP);
            canPlaceBuilding.push_back(BuildingType::CANNON);
            canPlaceBuilding.push_back(BuildingType::WALL);
        }
        if (_currentLevel == 2) {
            canPlaceBuilding.push_back(BuildingType::ARROW_TOWER);
        }
        return canPlaceBuilding;
    }
private:

};
// ========== 城墙 ==========
class Wall : public BaseBuilding {
public:
    static Wall* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);
    // 实现基类虚函数
    void doSpecialAction() override; // 墙体无特殊行为
    std::string getSpecialDesc() override { return "用于防御敌人攻击的建筑"; }
};

// ========== 金库 ==========
class Vault : public BaseBuilding {
public:
    static Vault* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);
    // 实现基类虚函数
    void doSpecialAction() override;
    std::string getSpecialDesc() override { return "用于储存金币的建筑"; }
    int getStorageCapacity() const { return _storageCapacity; }
    int getStoragePulse() const { return _storagePulse; }
    void upGrade() {
        if (_currentLevel != 1) {
            _storageCapacity += 10000;
            _storagePulse = 10000;
        }
    }
private:
    int _storageCapacity = 10000; // 储存容量
    int _storagePulse; // 升级增加的容量
};

//================ 圣水瓶 =================
class ElixirBottle : public BaseBuilding {
public:
    static ElixirBottle* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);
    // 实现基类虚函数
    void doSpecialAction() override;
    std::string getSpecialDesc() override { return "用于储存圣水的建筑"; }
    int getStorageCapacity() const { return _storageCapacity; }
    int getStoragePulse() const { return _storagePulse; }
    void upGrade() {
        if (_currentLevel != 1) {
            _storageCapacity += 10000;
            _storagePulse = 10000;
        }
    }
private:
    int _storageCapacity = 10000; // 储存容量
    //每次升级同样增加储存容量，给village增加升级容量，摧毁时village调用储存容量减少
    int _storagePulse; // 升级增加的容量
};

// ========== 攻击型建筑基类 ==========
class BaseAttackBuilding : public BaseBuilding {
protected:
    // 核心攻击属性
    float _attackRange;          // 攻击范围（像素）
    float _attackDamage;         // 单次伤害
    float _attackCooldown;       // 攻击冷却（秒/次）
    float _currentCooldown;      // 当前冷却计时
    BaseTroop* _targetTroop = nullptr;     // 当前攻击目标
    std::string _attackEffectPath; // 攻击特效路径
    float _attackEffectDuration;  // 特效时长
    DrawNode* _rangeDraw = nullptr;//范围显示节点
    bool _isShow=0;
    // 纯虚函数：子类实现具体攻击逻辑
    virtual void attackTarget() = 0;
    // 目标检测：找攻击范围内的敌方兵种
    BaseTroop* findTargetInRange();
public:

    // 初始化攻击属性（子类 init 中调用）
    void initAttackProps(float range, float damage, float cooldown, const std::string& effectPath = "");
    // 重写 update 逻辑（攻击核心）
    virtual void update(float dt) override;

    // 攻击范围可视化（调试用）
    void showAttackRange(bool isShow);
    bool getIsShow() { return _isShow; };
    // 重写销毁逻辑
    virtual void destroy() override;

    // 攻击属性接口
    float getAttackRange() const { return _attackRange; }
    float getAttackDamage() const { return _attackDamage; }
    float getAttackCooldown() const { return _attackCooldown; }
};

class Cannon : public BaseAttackBuilding {
public:
    static Cannon* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);

    // 实现攻击逻辑
    void attackTarget();
    void doSpecialAction();
    // 特殊描述（重写）
    std::string getSpecialDesc();
    void upGrade() {
        if (_currentLevel != 1) {
            _attackRange += 8;
            _attackCooldown -= 0.02;
            _attackDamage += 75;
        }
    }
};
class ArrowTower : public BaseAttackBuilding {
public:
    static ArrowTower* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);
    void doSpecialAction();
    void attackTarget();
    std::string getSpecialDesc();
    void upGrade() {
        if (_currentLevel != 1) {
            _attackRange += 10;
            _attackCooldown -= 0.02;
            _attackDamage += 50;
        }
    }
};

#endif // __BUILDING_H__






