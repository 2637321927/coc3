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
    void setState(BuildingState state); // 设置状态（通用）
    virtual void takeDamage(int damage); // 受到伤害（通用逻辑）
    BuildingState getState() const { return _state; }

    // ========== 通用属性接口（所有建筑都有） ==========
    BuildingType getType() const { return _config.type; }
    Vec2 getTilePos() const { return _tilePos; }
    int getLevel() const { return _config.level; }
    const BuildingConfig& getConfig() const { return _config; }
    void syncScale(float mapScale);     // 同步地图缩放（通用）
    void setLevel(int level) { _config.level = level; }
    // ========== 交互接口（通用） ==========
    void bindClickCallback(const std::function<void(BaseBuilding*)>& callback);
    std::function<void(BaseBuilding*)> _buildFinishCallback;
    void bindBuildFinishCallback(const std::function<void(BaseBuilding*)>& callback);
    // ========== 纯虚接口（子类必须实现：差异化逻辑） ==========
    virtual void doSpecialAction() = 0; // 建筑特有行为（如农场产粮、城镇中心造兵）
    virtual std::string getSpecialDesc() = 0; // 特有描述（如“每10秒产10粮食”）

    // ========== 生命周期（通用） ==========
    virtual void update(float dt) override; // 帧更新（处理进度）

protected:
    BaseBuilding() = default;
    ~BaseBuilding() override = default;

    // 通用辅助方法（子类可调用）
    void initCommonUI();                // 初始化通用UI（进度条、等级标签）
	void initTouchListener(); // 初始化触摸监听器
    void updateProgress();              // 更新建造/升级进度
    // 点击回调函数（参数为当前建筑指针）
    std::function<void(BaseBuilding*)> _clickCallback;
    // 触摸事件监听器
    EventListenerTouchOneByOne* _touchListener;
    // 通用成员（子类可访问）
    BuildingConfig _config;             // 通用配置
	//需要在public加一些接口供外部访问
	BuildingState _state = BuildingState::UNKNOWN;  // 当前状态
    int _currentHp;                   // 当前生命值
    Vec2  _tilePos ;             // 瓦片坐标(左上角)
    float _mapScale = 1.0f;             // 地图缩放比例
    float _progressTimer = 0.0f;        // 进度计时器
    ProgressTimer* _progressBar = nullptr; // 通用进度条
	Sprite*  _buildingSprite;        // 建筑图片精灵
};
class GoldMine : public BaseBuilding {
public:
    static GoldMine* create(const cocos2d::Vec2& tilePos, float mapScale);
    bool init(const cocos2d::Vec2& tilePos, float mapScale) ;

    // 重写专属行为：生产金币
    void doSpecialAction() override;
    // 重写专属描述
    std::string getSpecialDesc() override;
    // 收集金币
	int collectGold();
    void produceGold(float dt);
	void destroy() override;
private:
    float _goldTimer = 0.0f;    // 金币生产计时器
    int _goldPerInterval = 10;  // 每次生产金币数量
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
private:
	float _elixirTimer = 0.0f;    // 圣水生产计时器
	int _elixirPerInterval = 10;  // 每次生产圣水数量
	float _produceInterval = 2.0f; // 生产间隔（秒）
	int _elixirStored = 0;      // 已生产但未收集的圣水数量
};
// ========== 训练营类 ==========
class TrainingCamp : public BaseBuilding {
public:
	static TrainingCamp* create(const cocos2d::Vec2& tilePos, float mapScale);
	 bool init(const cocos2d::Vec2& tilePos, float mapScale);
     
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
     static Barracks* create(const cocos2d::Vec2& tilePos, float mapScale);
     bool init(const cocos2d::Vec2& tilePos, float mapScale) ;

     // 重写特殊行为
     void doSpecialAction() override;
     std::string getSpecialDesc() override;
	 // 训练士兵接口
     int getTroopSpace()const { return _maxTroopSpace; }; // 获取容量
     int getPulseSpace()const { return _pulseSpace; }; // 获取升级后新增容量
     //应重写升级函数，升级后增加总容量
    private:
        int _maxTroopSpace; // 总容量
		int _pulseSpace; // 升级增加的容量
        
  };
// ========== 大本营类 ==========
class TownHall : public BaseBuilding {
public:
    static TownHall* create(const cocos2d::Vec2& tilePos, float mapScale);
    virtual bool init(const cocos2d::Vec2& tilePos, float mapScale);

    // 实现基类虚函数
    virtual void doSpecialAction() override; // 升级解锁逻辑
    virtual std::string getSpecialDesc() override { return "村庄的核心建筑"; }
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
class Vault: public BaseBuilding {
public:
    static Vault* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale);
    // 实现基类虚函数
    void doSpecialAction() override; 
    std::string getSpecialDesc() override { return "用于储存金币的建筑"; }
    int getStorageCapacity() const { return _storageCapacity; }
    int getStoragePulse() const { return _storagePulse; }
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
    BaseTroop* _targetTroop=nullptr;     // 当前攻击目标
    std::string _attackEffectPath; // 攻击特效路径
    float _attackEffectDuration;  // 特效时长
	DrawNode* _rangeDraw = nullptr;//范围显示节点
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
    bool init(const Vec2& tilePos, float mapScale) ;

    // 实现攻击逻辑
    void attackTarget() ;
    void doSpecialAction();
    // 特殊描述（重写）
    std::string getSpecialDesc() ;
};
class ArrowTower : public BaseAttackBuilding {
public:
    static ArrowTower* create(const Vec2& tilePos, float mapScale);
    bool init(const Vec2& tilePos, float mapScale) ;
    void doSpecialAction() ;
    void attackTarget() ;
    std::string getSpecialDesc() ;
};

#endif // __BUILDING_H__




