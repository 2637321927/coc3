#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__
#include "Building.h"
#include "Troop.h"
#include "cocos2d.h"

using namespace cocos2d;
class VillageScene : public Scene
{
public:
    // Cocos2d-x 标准创建方法（必须）
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // CREATE_FUNC 宏：自动生成 create() 方法
    CREATE_FUNC(VillageScene);

private:
    enum class Mode {
        NONE,       // 无建造模式
        PLACE_BUILDING,  // 放置建筑模式
        SPAWN_TROOP          // 放置兵种模式
 
    };
    // -------------------------- 成员变量 --------------------------
    // 地图核心对象
    TMXTiledMap* _tileMap;       // 等轴测地图对象
    Size _tileSize;              // 单个瓦片尺寸
    Size _mapSize;               // 地图总瓦片数（宽×高）
    TMXLayer* _placeLayer;       // 可放置建筑的图层（对应Tiled的place_layer）
     TMXLayer* _pathLayer;      // 后续可启用：可通行图层（暂注释）
     TMXLayer* _bgLayer;
	 Node* _mapContainer;	  // 地图容器节点（用于整体缩放/拖拽）
     Mode _Mode = Mode::NONE;  // 当前模式
     Sprite* _mousePosSprite;//测试用，显示鼠标位置
    // 建筑放置相关
    Sprite* _buildPreview;       // 建筑放置预览图
    BuildingType _selectedBuildingType;      // 选中的建筑类型
    std::vector<Vec2> _occupiedTiles;        // 已占用的瓦片
    bool _isBuildBarShow = false; // 建筑栏是否显示
    Layer* _buildBarLayer = nullptr; // 建筑栏容器层
    // 缩放相关
    float _minScale = 0.5f;      // 最小缩放比例（避免缩太小）
    float _maxScale = 2.0f;      // 最大缩放比例（避免缩太大）
    float _scaleStep = 0.1f;     // 滚轮每滚一次的缩放步长
    // 拖拽相关
    bool _isDragging = false;    // 是否正在拖拽
    Vec2 _lastMousePos;          // 上一帧鼠标位置
    Vec2 _mapOriginPos;          // 地图初始位置（用于计算偏移）
	// 瓦片高亮相关
    std::vector<Vec2> _lastTilePos;           // 上一个选中的瓦片坐标
    bool _hasLastTile = false;   // 是否有上一个瓦片需要恢复
    Color3B _originalTileColor;  // 瓦片原始颜色（用于恢复）

    // -------------------------- 兵种相关成员变量 --------------------------
    bool _isTroopBarShow = false;// 兵种栏是否显示
    Sprite* _troopPreview;               // 兵种放置预览图
    TroopType _selectedTroopType = TroopType::UNKNOWN; // 选中的兵种类型
    std::vector<BaseTroop*> _spawnedTroops; // 已生成的所有兵种（用于管理生命周期）
    Vec2 _troopSpawnTilePos;             // 兵种出生瓦片坐标

    //金币，圣水数量
    int _gold ;
    int _elixir ;
    cocos2d::Label* _goldLabel;      // 金币显示标签
    cocos2d::Label* _elixirLabel;    // 圣水显示标签
    cocos2d::Sprite* _goldIcon;      // 金币图标
    cocos2d::Sprite* _elixirIcon;    // 圣水图标
    cocos2d::Layer* _resourceLayer;  // 资源显示层（方便统一管理）
    // -------------------------- 方法声明 --------------------------
    // 滚轮回调函数
    // 事件回调（新增鼠标按下/移动/松开）
    void onMouseScroll(Event* event);
    void onMouseDown(Event* event);
    void onMouseMove(Event* event);
    void onMouseUp(Event* event);
	// 鼠标移动时高亮瓦片(测试坐标转换函数是否正确)
	void VillageScene::setTileColor(Vec2 tilePos, Color3B color, BuildingType type);//测试用
    void restoreLastTileColor();
    // 限制地图拖动范围
    void clampMapPosition();
    // 初始化地图
    void initMap();
    // 初始化建筑预览（暂时屏蔽）
    void initBuildPreview();
	// 显示无法放置提示
    void VillageScene::showCannotPlaceTip(Vec2 pos);
    // 初始化触摸事件（暂时屏蔽）(手机版用，如时间不够则放弃）
    void initTouchEvent();
    // 坐标转换：屏幕坐标 → 等轴测瓦片坐标
    Vec2 screenToIsoTile(Vec2 screenPos);
    // 坐标转换：等轴测瓦片坐标 → 屏幕坐标
    Vec2 isoTileToScreen(Vec2 tilePos);
	// 坐标转换：瓦片坐标 → 容器坐标
    Vec2 isoTileToContainerPos(Vec2 tilePos);
	// 初始化建筑放置按钮
    void VillageScene::initBuildModeBtn();
    // 检测瓦片是否可放置建筑
    bool checkCanPlace(Vec2 tilePos, BuildingType type);
    // 放置建筑
    void VillageScene::placeBuilding(Vec2 tilePos, BuildingType type);
	// 切换建筑栏显示/隐藏
    void VillageScene::toggleBuildBar();
    // 创建建筑栏（仅第一次调用时创建）
    void createBuildBar();  
    // 隐藏建筑栏
    void hideBuildBar();  
	// 检测瓦片是否被占用
    bool isTileOccupied(Vec2 tilePos);
    const BuildingConfig& VillageScene::getBuildingConfigByType(BuildingType type);
    // -------------------------- 兵种相关方法声明 --------------------------
    // 初始化兵种放置预览
    void initTroopPreview();
    // 创建兵种栏（训练按钮）
    void createTroopBar();
    // 切换兵种栏显示/隐藏
    void toggleTroopBar();
    // 隐藏兵种栏
    void hideTroopBar();
    // 放置（生成）兵种
    void spawnTroop(Vec2 screenPos, TroopType type);
    // 初始化兵种训练按钮
    void initTroopModeBtn();
    // 检测兵种可生成位置（空地/非建筑占用）
    bool checkCanSpawnTroop(Vec2 tilePos);
    // 兵种攻击回调（处理伤害结算）
    void onTroopAttack(BaseTroop* troop, BaseBuilding* target);
    void VillageScene::initResourceBar();
    void VillageScene::setGold(int gold);
    void VillageScene::setElixir(int elixir);
    bool VillageScene::addGold(int amount);
    bool VillageScene::spendGold(int amount);
    bool VillageScene::addElixir(int amount);
    bool VillageScene::spendElixir(int amount);
    void VillageScene::showResourceShortageTip(const std::string& message);

    // 关卡选择相关
    cocos2d::Layer* _levelSelectLayer;
    bool _isLevelSelectShow;
    void createLevelSelectMenu();
    void showLevelSelectMenu();
    void hideLevelSelectMenu();
    void toggleLevelSelectMenu();

    // 初始化关卡选择按钮
    void initLevelSelectBtn();

    // 跳转到关卡的函数
    void gotoLevel1();
    void gotoLevel2();
    void gotoLevel3();
    // -------------------------- 整体控制--------------------------
	void hideModeBtn();//TODO: 隐藏模式切换按钮，每次切换模式时调用，防止模式重叠，或者，另外一个思路，在按钮上加限制，只有主模式才能按按钮
};

#endif // __VILLAGE_SCENE_H__
