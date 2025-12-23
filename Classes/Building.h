#ifndef __BUILDING_H__
#define __BUILDING_H__

#include "cocos2d.h"
#include <string>
#include <unordered_map>
#include <functional>
using namespace cocos2d;
// 前置声明
class VillageScene;

// 建筑类型枚举
enum class BuildingType {
    TOWN_HALL,   // 大本营
	GOLD_MINE,   // 金矿
    BARRACKS,    // 兵营
    UNKNOWN
};

// 建筑状态枚举
enum class BuildingState {
    IDLE,        // 正常
    BUILDING,    // 建造中
    UPGRADING,   // 升级中
    DESTROYED,   // 摧毁
    UNKNOWN
};
// 通用建筑配置（所有建筑都有的属性）
//特有属性在子类内部praivate
struct BuildingConfig {
    int id;                     // 唯一ID
    BuildingType type;          // 建筑类型
    std::string name;           // 名称
    std::string imgPath;        // 纹理路径
	int hp;                     // 生命值
    int tileWidth;              // 占用瓦片宽
    int tileHeight;             // 占用瓦片高
    std::unordered_map<std::string, int> cost; // 建造消耗（金币/圣水）
    float buildTime;            // 建造时长（秒）
    int level = 1;              // 初始等级
};

// 抽象基类（不可实例化，只能继承）
class BaseBuilding : public cocos2d::Node
{
public:
    // 工厂方法（创建子类实例，基类指针接收）
    static BaseBuilding* create(BuildingType type, const cocos2d::Vec2& tilePos, float mapScale);

    // ========== 通用初始化（子类可重写） ==========
    bool init(const BuildingConfig& config, const cocos2d::Vec2& tilePos, float mapScale);
	bool BaseBuilding::loadBuildingSprite(); // 加载建筑图片精灵（通用）
    // ========== 通用状态管理（所有建筑都有） ==========
    virtual void startBuild();          // 开始建造（通用逻辑）
    virtual void finishBuild();         // 完成建造（通用逻辑）
    virtual void startUpgrade();        // 开始升级（通用逻辑）
    virtual void destroy();             // 摧毁建筑（通用逻辑）
    void setState(BuildingState state); // 设置状态（通用）
    BuildingState getState() const { return _state; }

    // ========== 通用属性接口（所有建筑都有） ==========
    BuildingType getType() const { return _config.type; }
    Vec2 getTilePos() const { return _tilePos; }
    int getLevel() const { return _config.level; }
    const BuildingConfig& getConfig() const { return _config; }
    void syncScale(float mapScale);     // 同步地图缩放（通用）

    // ========== 交互接口（通用） ==========
    void bindClickCallback(const std::function<void(BaseBuilding*)>& callback);

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
	void BaseBuilding::initTouchListener(); // 初始化触摸监听器
    void updateProgress();              // 更新建造/升级进度
    // 点击回调函数（参数为当前建筑指针）
    std::function<void(BaseBuilding*)> _clickCallback;
    // 触摸事件监听器
    EventListenerTouchOneByOne* _touchListener;
    // 通用成员（子类可访问）
    BuildingConfig _config;             // 通用配置
	//需要在public加一些接口供外部访问
	BuildingState _state = BuildingState::UNKNOWN;  // 当前状态
    Vec2 _tilePos;             // 瓦片坐标
    float _mapScale = 1.0f;             // 地图缩放比例
    float _progressTimer = 0.0f;        // 进度计时器
    ProgressTimer* _progressBar = nullptr; // 通用进度条
	Sprite*  _buildingSprite;        // 建筑图片精灵
};
class GoldMine : public BaseBuilding {
public:
    static GoldMine* create(const cocos2d::Vec2& tilePos, float mapScale);
    virtual bool init(const cocos2d::Vec2& tilePos, float mapScale) ;

    // 重写专属行为：生产金币
    void doSpecialAction() override;
    // 重写专属描述
    std::string getSpecialDesc() override;
    // 收集金币
	int collectGold(); 
private:
    void produceGold(float dt);
    float _goldTimer = 0.0f;    // 金币生产计时器
    int _goldPerInterval = 10;  // 每次生产金币数量
    float _produceInterval = 2.0f; // 生产间隔（秒）
	int _goldStored = 0;      // 已生产但未收集的金币数量
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
#endif // __BUILDING_H__
