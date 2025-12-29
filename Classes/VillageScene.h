#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__
#include "Building.h"
#include "Troop.h"
#include "cocos2d.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 
#include "EnumType.h" 
#include <unordered_set>

using namespace cocos2d;
extern std::unordered_map<TroopType, TroopConfig> g_troopTrainConfig;
std::vector<std::string> split(const std::string& s, const std::string& delim);
enum class Mode {
    NONE,       // 无建造模式
    PLACE_BUILDING,  // 放置建筑模式
    SPAWN_TROOP,    // 放置兵种模式
    FIGHT,     // 战斗模式（预留）
    MOVE, 	// 移动建筑模式
    PROTECT, // 保护模式(过渡模式，放置误触)
    LEVEL1,
    LEVEL2,
    LEVEL3
};
//存档相关
namespace SaveData {
    // 单栋建筑的存档数据
    struct Building {
        BuildingType type;          // 建筑类型（TOWN_HALL/GOLD_MINE等）
        cocos2d::Vec2 tilePos;      // 建筑所在格子坐标
        BuildingState state;        // 建筑状态（IDLE/BUILDING等）
        int level = 1;              // 建筑等级（如果有升级逻辑）
        bool immediatelyBuild;       //是否立即建造完成
        float progressTimer;         //进度
        // 序列化：将数据转为字符串（方便存储）
        std::string toString() const {
            std::stringstream ss;
            ss << (int)type << ","
                << tilePos.x << "," << tilePos.y << ","
                << (int)state << "," << level << ","
                << immediatelyBuild << "," << progressTimer;
            return ss.str();
        }

        // 反序列化：从字符串恢复数据
        static Building fromString(const std::string& str) {
            Building data;
            std::vector<std::string> parts = split(str, ","); // 按逗号分割
            if (parts.size() >= 5) {
                data.type = (BuildingType)std::stoi(parts[0]);
                data.tilePos.x = std::stof(parts[1]);
                data.tilePos.y = std::stof(parts[2]);
                data.state = (BuildingState)std::stoi(parts[3]);
                data.level = std::stoi(parts[4]);
                data.immediatelyBuild = std::stoi(parts[5]);
                data.progressTimer = std::stof(parts[6]);
            }
            return data;
        }
    };

    // 整个村庄的存档数据
    struct Village {
        std::vector<Building> buildings;       // 所有建筑数据
        cocos2d::Vec2 mapSize;                 // 地图尺寸（可选）
        std::vector<Vec2> occupiedTiles; // 已占用格子（可选，可通过建筑数据推导）
        Mode currentMode = Mode::NONE;         // 当前模式（可选）
        int gold; 						   // 金币
        int elixir;					   // 圣水
        // 序列化整个村庄数据
        std::string toString() const {
            std::stringstream ss;
            // 1. 先存地图尺寸
            ss << mapSize.x << "," << mapSize.y << "\n";
            // 2. 存当前模式
            ss << (int)currentMode << "\n";
            // 3. 存已占用格子（可选）
            for (const auto& tile : occupiedTiles) {
                ss << tile.x << "," << tile.y << ";";
            }
            if (occupiedTiles.empty()) {
                ss << ','; // 结尾加分号
            }
            ss << "\n";
            ss << gold << "," << elixir << "\n";
            // 4. 存所有建筑（每行一个建筑）
            for (const auto& b : buildings) {
                ss << b.toString() << "\n";
            }
            return ss.str();
        }

        // 反序列化整个村庄数据
        static Village fromString(const std::string& str) {
            Village data;
            std::vector<std::string> lines = split(str, "\n"); // 按行分割
            if (lines.empty()) return data;

            // 解析地图尺寸
            std::vector<std::string> mapParts = split(lines[0], ",");
            if (mapParts.size() >= 2) {
                data.mapSize.x = std::stof(mapParts[0]);
                data.mapSize.y = std::stof(mapParts[1]);
            }

            // 解析当前模式
            if (lines.size() >= 2) {
                data.currentMode = (Mode)std::stoi(lines[1]);
            }

            // 解析已占用格子
            if (lines.size() >= 3 && !lines[2].empty()) {

                std::vector<std::string> tileParts = split(lines[2], ";");
                for (const auto& tileStr : tileParts) {
                    if (tileStr.empty()) continue;
                    std::vector<std::string> pos = split(tileStr, ",");
                    if (pos.size() >= 2) {
                        data.occupiedTiles.emplace_back(std::stof(pos[0]), std::stof(pos[1]));
                    }
                }
            }

            // 解析金币+圣水（第3行）
            if (lines.size() >= 4 && !lines[3].empty()) {
                std::vector<std::string> resParts = split(lines[3], ",");
                if (resParts.size() >= 2) {
                    // 异常捕获：避免存档数据错误导致崩溃
                    try {
                        data.gold = std::stoi(resParts[0]);
                        data.elixir = std::stoi(resParts[1]);
                    }
                    catch (const std::invalid_argument& e) {
                        CCLOG("解析金币234324/圣水失败：%s", e.what());
                        data.gold = 0;
                        data.elixir = 0;
                    }
                    catch (const std::out_of_range& e) {
                        CCLOG("金币/圣234水数值超出范围：%s", e.what());
                        data.gold = 0;
                        data.elixir = 0;
                    }
                }
            }

            // 建筑从第4行开始解析
            for (int i = 4; i < lines.size(); i++) {
                if (lines[i].empty()) continue;
                data.buildings.push_back(Building::fromString(lines[i]));
            }

            return data;
        }

    private:
        // 辅助分割字符串
        static std::vector<std::string> split(const std::string& s, const std::string& delim) {
            std::vector<std::string> res;
            size_t pos = 0;
            std::string token;
            std::string str = s;
            while ((pos = str.find(delim)) != std::string::npos) {
                token = str.substr(0, pos);
                if (!token.empty()) res.push_back(token);
                str.erase(0, pos + delim.length());
            }
            if (!str.empty()) res.push_back(str);
            return res;
        }
    };
}
class VillageScene : public Scene
{
public:
    void addMapChild(Sprite* child) {
        _mapContainer->addChild(child, 10000);
    }
    // 新增：提供坐标转换接口
    Vec2 screenToIsoTilePublic(const Vec2& screenPos) {
        return screenToIsoTile(screenPos);  // 调用原有私有方法
    }
    Vec2 isoTileToContainerPosPublic(const Vec2& tilePos) {
        return isoTileToContainerPos(tilePos);  // 调用原有私有方法
    }

    // 新增：提供地图数据接口
    cocos2d::TMXLayer* getPathLayer() const { return _pathLayer; }  // 返回路径层
    cocos2d::Size getMapSize() const { return _mapSize; }        // 返回地图尺寸
    const std::vector<Vec2>& getOccupiedTiles() const {    // 返回占用瓦片
        return _occupiedTiles;
    }
    BaseBuilding* VillageScene::findNearestEnemyBuilding(const Vec2& troopPos) {
        BaseBuilding* nearestBuilding = nullptr;
        float minDistance = FLT_MAX;

        // 遍历所有建筑（假设场景中有存储建筑的容器 _buildings）
        for (auto& building : _buildings) {
            float distance = troopPos.distance(building->getPosition());
            if (distance < minDistance) {
                minDistance = distance;
                nearestBuilding = building;
            }
        }
        return nearestBuilding;
    }
    void VillageScene::init_level_Btns(BaseMode baseMode);
    static VillageScene* getInstance() { // 提供外部访问接口
        auto currentScene = Director::getInstance()->getRunningScene();
        if (!currentScene) return nullptr;
        // 找到场景中 Tag=100 的子节点
        auto scene_100 = dynamic_cast<VillageScene*>(currentScene->getChildByTag(100));
        if (scene_100 && scene_100->isRunning()) {
            return dynamic_cast<VillageScene*>(currentScene->getChildByTag(100));
        }
        auto scene_25 = dynamic_cast<VillageScene*>(currentScene->getChildByTag(25));
        if (scene_25 && scene_25->isRunning()) {
            return dynamic_cast<VillageScene*>(currentScene->getChildByTag(25));
        }
        return nullptr;
    }
    // Cocos2d-x 标准创建方法（必须）
    static cocos2d::Scene* createScene(BaseMode baseMode = BaseMode::CREATING);
    // 初始化方法
    virtual bool init();

    // CREATE_FUNC 宏：自动生成 create() 方法
    CREATE_FUNC(VillageScene);
    // 存档（保存到本地文件）
    bool saveGame(const std::string& savePath = "village_save.txt");
    // 读档（从本地文件恢复）

    bool VillageScene::loadGame(const std::string& savePath, bool fallbackToResources);
    // 按钮点击回调
    void onSaveBtnClicked(cocos2d::Ref* sender);
    void onLoadBtnClicked(cocos2d::Ref* sender);
    // 容量管理相关方法
    int getTotalTroopCapacity() const { return _maxPopulation; }
    void addTroopCapacity(int bonus); // 增加容量
    void removeTroopCapacity(int bonus); // 移除容量（比如兵营被摧毁）
    // 训练营弹窗相关
    void showTrainingCampPopup(TrainingCamp* camp); // 显示训练弹窗
    void hideTrainingCampPopup(); // 隐藏训练弹窗
    //获取敌军接口（用于给攻击建筑）
    std::vector<BaseTroop*>& getAllEnemyTroops() {
        if (_enemyTroops.capacity() > 10000 || _enemyTroops.size() > 10000) {
            CCLOGERROR("敌方士兵列表异常，长度：%zu，容量：%zu", _enemyTroops.size(), _enemyTroops.capacity());
            _enemyTroops.clear();
            return _enemyTroops;
        }
        return _enemyTroops;
    }
    //移除敌军军队指针（用于敌军士兵死亡后移除）
    void removeEnemyTroop(BaseTroop* troop);
    // 添加/移除敌方兵种
    void addEnemyTroop(BaseTroop* troop) {
        _enemyTroops.push_back(troop);
    }
    //设置基础模式（关卡，创造，普通）
    void setBaseMode(const BaseMode& baseMode) {
        _baseMode = baseMode;
    }
    BaseMode getBaseMode() const { return _baseMode; }
    void setTroopModeBtnInvisible() {
        _troopModeBtn->setVisible(true);
    }
    // 摧毁建筑核心函数
    void destroyBuilding(BaseBuilding* building);
    // 显示无法放置提示
    void VillageScene::showCannotPlaceTip(Vec2 pos);
    // 放置建筑

    void VillageScene::showText(std::string string, Vec2 pos = Vec2(1000, 1000), float duringTime = 2.0f);
    void setGold(int gold);
    void setElixir(int elixir);
    bool addGold(int amount);
    bool spendGold(int amount);
    bool addElixir(int amount);
    bool spendElixir(int amount);
    void showResourceShortageTip(const std::string& message);
    void addGoldStorageCapacity(int bonus);
    void addElixirStorageCapacity(int bonus);
    int getGold() const { return _gold; }
    int getElixir() const { return _elixir; }
    virtual void update(float dt) override;
protected:
    bool level_init();
    ui::Layout* _uiLayer;  // 资源显示层（方便统一管理）
private:
    BaseMode _baseMode; // 所有模式管理器
    //static VillageScene* _instance;// 单例实例指针
    // -------------------------- 成员变量 --------------------------
    // 地图核心对象
    TMXTiledMap* _tileMap;       // 等轴测地图对象
    Size _tileSize;              // 单个瓦片尺寸
    Size _mapSize;               // 地图总瓦片数（宽×高）
    TMXLayer* _placeLayer;       // 可放置建筑的图层（对应Tiled的place_layer）
    TMXLayer* _pathLayer;      // 后续可启用：可通行图层（暂注释）
    TMXLayer* _bgLayer;      // 背景图层（对应Tiled的bg_layer）
    Node* _mapContainer;	  // 地图容器节点（用于整体缩放/拖拽）
    Mode _Mode = Mode::NONE;  // 当前模式
    Mode _lastMode = Mode::NONE; // 上一次模式（用于切换回原模式）
    Sprite* _mousePosSprite;//测试用，显示鼠标位置
    int _maxLevel = 1;
    // 建筑放置相关
    Sprite* _buildPreview;       // 建筑放置预览图
    BuildingType _selectedBuildingType;      // 选中的建筑类型
    std::vector<Vec2> _occupiedTiles;        // 已占用的瓦片
    Vec2 _lastTile; 		   // 上一次鼠标所在瓦片
    bool _isContinuousPlace = false; // 是否连续放置建筑
    // 建筑栏背景
    Sprite* _barBg;
    // 建筑按钮 
    MenuItemImage* _townHallBtn;
    MenuItemImage* _goldMineBtn;
    MenuItemImage* _elixirCollectorBtn;
    MenuItemImage* _barracksBtn;
    MenuItemImage* _trainingCampBtn;
    MenuItemImage* _cannonBtn;
    MenuItemImage* _arrowTowerBtn;
    MenuItemImage* _wallBtn;
    MenuItemImage* _elixirBottleBtn;
    MenuItemImage* _vaultBtn;
    MenuItemImage* _cancelPlaceBtn;
    BaseBuilding* _movingBuilding;
    // 定义场景最大瓦片数（根据你的游戏场景调整，比如50x50、100x100，固定不变）
    static const int MAX_TILE_X = 400;  // 瓦片x轴最大索引（0 ~ 49）
    static const int MAX_TILE_Y = 400;  // 瓦片y轴最大索引（0 ~ 49）

    // 二维布尔数组，标记瓦片是否被占用（全局初始化默认false，未占用）
    bool _tileOccupiedGrid[MAX_TILE_X][MAX_TILE_Y] = { false };
    // 获取建筑所有占用的格子
    std::vector<Vec2> getBuildingAllTiles(BaseBuilding* building) {
        std::vector<Vec2> tiles;
        if (!building) return tiles;
        for (int x = 0; x < building->getConfig().tileWidth; x++) {
            for (int y = 0; y < building->getConfig().tileHeight; y++) {
                Vec2 tile = building->getTilePos() + Vec2(x, y);
                tiles.push_back(tile);
            }
        }
        return tiles;
    }
    bool _isBuildBarShow = false; // 建筑栏是否显示
    Layer* _buildBarLayer = nullptr; // 建筑栏容器层
    bool _isAnyBuildSelected = false; // 是否有建筑被选中（移动）
    // 存储所有建筑（基类指针，兼容所有建筑类型）
    std::vector<BaseBuilding*> _buildings;
    // 按类型拆分存储（便于快速查找,资源类一键停止/开启）
    std::vector<GoldMine*> _goldMines;
    BaseBuilding* _townHall;
    //TODO:实现一些特殊建筑子列便于兵种寻找
    std::vector<ElixirCollector*> _elixirCollectors;
    // 缩放相关
    float _minScale = 0.5f;      // 最小缩放比例（避免缩太小）
    float _maxScale = 2.0f;      // 最大缩放比例（避免缩太大）
    float _scaleStep = 0.1f;     // 滚轮每滚一次的缩放步长
    // 拖拽相关
    bool _isDragging = false;    // 是否正在拖拽
    Vec2 _lastMousePos;          // 上一帧鼠标位置
    Vec2 _mapOriginPos;          // 地图初始位置（用于计算偏移）
    // 瓦片高亮相关
    bool _isLastMouseLeftButtonDown = false; // 上一帧鼠标左键状态
    std::vector<Vec2> _lastTilePos;           // 上一个选中的瓦片坐标
    bool _hasLastTile = false;   // 是否有上一个瓦片需要恢复
    Color3B _originalTileColor;  // 瓦片原始颜色（用于恢复）

    // 训练营相关
    ui::Layout* _trainingPopup = nullptr; // 训练弹窗根节点
    TrainingCamp* _currentCamp = nullptr; // 当前关联的训练营
    const int MAX_QUEUE_SIZE = 5; // 最大训练队列数


    // -------------------------- 兵种相关成员变量 --------------------------
    bool _isTroopBarShow = false;// 兵种栏是否显示
    Sprite* _troopPreview;               // 兵种放置预览图
    TroopType _selectedTroopType = TroopType::UNKNOWN; // 选中的兵种类型
    std::vector<BaseTroop*> _spawnedTroops; // 已生成的所有兵种（用于管理生命周期，不一定是敌军）
    Vec2 _troopSpawnTilePos;             // 兵种出生瓦片坐标
    std::vector<BaseTroop*> _enemyTroops; // 敌方兵种列表(预留，用于战斗模式)
    //资源相关
    int _gold;
    int _elixir;
    int _maxGold = 10000;
    int _maxElixir = 10000;
    int _maxPopulation = 0; //人口总数(兵营）
    int _population = 0; //当前人口数
    std::unordered_map<TroopType, int> _troopStorage;// 兵营存储的兵种信息：兵种类型 -> 数量
    std::unordered_map<TroopType, int> _troopLevel; // 兵种等级信息：兵种类型 -> 等级
    int _builder;    //建造者数量
    cocos2d::Label* _goldLabel;      // 金币显示标签
    cocos2d::Label* _elixirLabel;    // 圣水显示标签
    cocos2d::Sprite* _goldIcon;      // 金币图标
    cocos2d::Sprite* _elixirIcon;    // 圣水图标
    // -------------------------- 方法声明 --------------------------
    // 滚轮回调函数
    // 事件回调（新增鼠标按下/移动/松开）
    //战斗相关
    float _totalTime; // 总倒计时时长（150秒 = 2分30秒）
    float _remainingTime; // 剩余时间

    //ui组件
    ui::Button* _saveBtn;
    ui::Button* _loadBtn;
    ui::Button* _backBtn;
    ui::Button* _troopModeBtn;
    ui::Button* _buildModeBtn;
    ui::Button* _levelSelectBtn;
    ui::Button* _fightBtn;
    ui::Button* _fightStartBtn;
    ui::Button* _collectAllBtn;
    ui::Button* _moveBtn;
    Label* _countDownLabel;
    // 星级评定相关成员变量
    float destroyPercent; // 建筑摧毁百分比
    int currentStars;     // 当前星级
    Label* percentLabel; // 百分比显示标签
    Vector<cocos2d::Sprite*> starSprites; // 星级精灵数组
    Vec2 _starTargetPos; // 星级最终停留位置
    float _totalBuildingCount;    // 总建筑数量
    float _destroyedBuildingCount;// 已摧毁建筑数量

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
    void initMapContainer();
    bool loadMap(const std::string& mapPath);
    void initBtns(BaseMode baseMode = BaseMode::CREATING); //初始化一切按钮
    // 初始化建筑预览（暂时屏蔽）
    void initBuildPreview();
    void initFightScene();
    // 初始化触摸事件（暂时屏蔽）(手机版用，如时间不够则放弃）
    void initTouchEvent();
    // 坐标转换：屏幕坐标 → 等轴测瓦片坐标
    Vec2 screenToIsoTile(Vec2 screenPos);
    // 坐标转换：等轴测瓦片坐标 → 屏幕坐标
    Vec2 isoTileToScreen(Vec2 tilePos);
    // 坐标转换：瓦片坐标 → 容器坐标
    Vec2 isoTileToContainerPos(Vec2 tilePos);
    // 初始化建筑放置按钮
    void initBuildModeBtn();
    // 检测瓦片是否可放置建筑
    bool checkCanPlace(Vec2 tilePos, BuildingType type);

    void moveBuilding(BaseBuilding* building, Vec2 newTilePos);
    // 切换建筑栏显示/隐藏
    void VillageScene::toggleBuildBar();
    // 创建建筑栏（仅第一次调用时创建）
    void createBuildBar();
    //根据大本营等级判断能放置的建筑
    void checkCanGetBuilding();
    // 隐藏建筑栏
    void hideBuildBar();
    //放置建筑
    void placeBuilding(Vec2 tilePos, BuildingType type);
    // 检测瓦片是否被占用
    bool isTileOccupied(Vec2 tilePos);
    // 处理建筑弹窗按钮点击回调
    void handleBuildingBtnClick(BaseBuilding* building, BuildingPopup::ButtonType type);
    // 辅助：释放建筑占用的瓦片
    void releaseBuildingTiles(BaseBuilding* building);
    void addOccupiedTiles(const std::vector<Vec2>& tiles);
    void addOccupiedTile(const Vec2& tile);
    // 金矿生产控制(如切换到其他场景，战争模式时暂停生产)
    // 暂停所有金矿生产
    void pauseAllGoldMines();
    // 恢复所有金矿生产
    void resumeAllGoldMines();
    //一键收集所有资源建筑的资源
    void collectAllResources();
    //训练营相关

    // 弹窗UI初始化
    void initTrainingPopupUI();
    // 初始化可训练兵种按钮
    void initTroopButtonsInPopup();
    // 初始化训练队列面板
    void initTrainQueuePanelInPopup();
    // 刷新弹窗内的队列和倒计时显示
    void refreshPopupTrainInfo();
    // 弹窗内的倒计时刷新调度（仅在弹窗打开时运行）
    void updatePopupTimer(float dt);
    // 刷新队列UI
    void refreshTrainQueueUI();
    // 更新队列倒计时
    void updateTrainQueueTimer(float dt);
    // 检查资源是否足够
    bool checkTroopResourceEnough(TroopType type);
    // 扣除训练资源
    void deductTroopResource(TroopType type);
    // 返还训练资源（取消训练）
    void refundTroopResource(TroopType type);
    // 添加兵种到训练队列
    void addTroopToQueue(TroopType type);
    // 移除队列指定位置的兵种
    void removeTroopFromQueue(int index);
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
    //资源管控
    void initResourceBar();
    // 关卡选择相关
    cocos2d::Layer* _levelSelectLayer;
    bool _isLevelSelectShow;
    void createLevelSelectMenu();
    void showLevelSelectMenu();
    void hideLevelSelectMenu();
    void toggleLevelSelectMenu();
    void initLevelSelectBtn();
    void go_back(const std::string& fileName);
    // 跳转到关卡的函数

    void go_back_Btn();
    void VillageScene::gotoLevel1(const std::string& levelFilename);



    //战斗相关
    void gotoFight();//跳转到战斗场景的函数
    void backfromFight();//从战斗场景退出
    void beginFight(); //开始战斗
    void updateCountDown(float dt);//战斗计时
    void onFightSettle();//战斗结算

    // 星星相关
    void initStarRatingUI(); // 初始化星级UI
    void updateDestroyPercent(); // 更新摧毁百分比
    void checkStarUnlock(); // 检查星级解锁
    void flyStarToTarget(int starIndex); // 星级飞行动画
    void updateStarDisplay(); // 更新星级显示状态
    // -------------------------- 整体控制--------------------------
    void hideModeBtn();//TODO: 隐藏模式切换按钮，每次切换模式时调用，防止模式重叠，或者，另外一个思路，在按钮上加限制，只有主模式才能按按钮
    void destroyScene(); // 销毁场景，释放资源
    void onExit(); // 重写退出方法，清理资源
    void cleanup(); // 重写清理方法，释放资源
    //存档相关
    // 辅助：将当前场景数据转为存档结构
    SaveData::Village packSaveData();
    // 辅助：从存档结构恢复场景数据
    void unpackSaveData(const SaveData::Village& saveData);
    void initSaveLoadButtons(); // 辅助：创建UI按钮（新增）

};
#endif // __VILLAGE_SCENE_H__
