#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__

#include "Building.h"
#include "Troop.h"
#include "cocos2d.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 
#include "EnumType.h" 
#include <unordered_set>

// 自定义Vec2哈希函数，用于unordered_map/set
struct Vec2Hash {
    size_t operator()(const Vec2& v) const {
        // 优化哈希：将浮点坐标放大取整，避免精度问题和对称坐标碰撞
        // (x * 1000) ^ ((y * 1000) << 1)
        return std::hash<int>()(static_cast<int>(v.x * 1000)) ^
            (std::hash<int>()(static_cast<int>(v.y * 1000)) << 1);
    }
};

using TileKey = uint32_t; // 瓦片键类型 (足够存储 100*100=10000 的键)

TileKey tileToKey(const Vec2& tilePos);

using namespace cocos2d;

extern std::unordered_map<TroopType, TroopConfig> g_troopTrainConfig;

std::vector<std::string> split(const std::string& s, const std::string& delim);

// 场景交互模式枚举
enum class Mode {
    NONE,           // 浏览模式 (无操作)
    PLACE_BUILDING, // 建筑放置模式 (拖拽建筑虚影)
    SPAWN_TROOP,    // 兵种投放模式
    FIGHT,          // 战斗模式 (预留)
    MOVE,           // 建筑移动模式 (调整已有建筑位置)
    PROTECT         // 保护模式 (防止误触的过渡状态)
};

// 存档数据结构命名空间
namespace SaveData {
    // 单栋建筑的存档数据结构
    struct Building {
        BuildingType type;      // 建筑类型
        cocos2d::Vec2 tilePos;  // 建筑所在的瓦片坐标
        BuildingState state;    // 当前状态 (建造中/闲置等)
        int level = 1;          // 建筑等级

        // 序列化：对象 -> 字符串 (格式: "type,x,y,state,level")
        std::string toString() const {
            std::stringstream ss;
            ss << (int)type << ","
                << tilePos.x << "," << tilePos.y << ","
                << (int)state << ","
                << level;
            return ss.str();
        }

        // 反序列化：字符串 -> 对象
        static Building fromString(const std::string& str) {
            Building data;
            std::vector<std::string> parts = split(str, ","); // 按逗号分割
            if (parts.size() >= 5) {
                data.type = (BuildingType)std::stoi(parts[0]);
                data.tilePos.x = std::stof(parts[1]);
                data.tilePos.y = std::stof(parts[2]);
                data.state = (BuildingState)std::stoi(parts[3]);
                data.level = std::stoi(parts[4]);
            }
            return data;
        }
    };

    // 完整村庄的存档数据结构
    struct Village {
        std::vector<Building> buildings;        // 所有建筑列表
        cocos2d::Vec2 mapSize;                  // 地图尺寸
        std::vector<Vec2> occupiedTiles;        // 被占用的瓦片列表
        Mode currentMode = Mode::NONE;          // 保存时的模式
        int gold;                               // 金币数量
        int elixir;                             // 圣水数量

        // 序列化整个村庄数据
        std::string toString() const {
            std::stringstream ss;
            // 1. 存地图尺寸
            ss << mapSize.x << "," << mapSize.y << "\n";
            // 2. 存当前模式
            ss << (int)currentMode << "\n";
            // 3. 存已占用格子
            for (const auto& tile : occupiedTiles) {
                ss << tile.x << "," << tile.y << ";";
            }
            if (occupiedTiles.empty()) {
                ss << ','; // 结尾加分号
            }
            ss << "\n";
            // 4. 存资源数据
            ss << gold << "," << elixir << "\n";
            // 5. 存所有建筑（每行一个建筑）
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
                        CCLOG("解析金币/圣水失败：%s", e.what());
                        data.gold = 0;
                        data.elixir = 0;
                    }
                    catch (const std::out_of_range& e) {
                        CCLOG("金币/圣水数值超出范围：%s", e.what());
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

// 游戏主场景类
class VillageScene : public Scene
{
public:
    // 提供坐标转换接口 (公有)
    Vec2 screenToIsoTilePublic(const Vec2& screenPos) {
        return screenToIsoTile(screenPos);  // 调用原有私有方法
    }
    Vec2 isoTileToContainerPosPublic(const Vec2& tilePos) {
        return isoTileToContainerPos(tilePos);  // 调用原有私有方法
    }

    // 提供地图数据接口
    cocos2d::TMXLayer* getPathLayer() const { return _pathLayer; }  // 返回路径层
    cocos2d::Size getMapSize() const { return _mapSize; }        // 返回地图尺寸
    const std::vector<Vec2>& getOccupiedTiles() const {    // 返回占用瓦片列表
        return _occupiedTiles;
    }

    // 查找最近的敌方建筑
    BaseBuilding* findNearestEnemyBuilding(const Vec2& troopPos) {
        BaseBuilding* nearestBuilding = nullptr;
        float minDistance = FLT_MAX;

        // 遍历所有建筑
        for (auto& building : _buildings) {
            float distance = troopPos.distance(building->getPosition());
            if (distance < minDistance) {
                minDistance = distance;
                nearestBuilding = building;
            }
        }
        return nearestBuilding;
    }

    void init_level_Btns(BaseMode baseMode);

    // 获取全局单例实例
    static VillageScene* getInstance() {
        auto currentScene = Director::getInstance()->getRunningScene();
        if (!currentScene) return nullptr;
        // 尝试通过 Tag 获取场景实例
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

    // Cocos2d-x 标准创建方法
    static cocos2d::Scene* createScene(BaseMode baseMode = BaseMode::CREATING);

    // 初始化方法
    virtual bool init();

    // CREATE_FUNC 宏：自动生成 create() 方法
    CREATE_FUNC(VillageScene);

    // 存档（保存到本地文件）
    bool saveGame(const std::string& savePath = "village_save.txt");

    // 读档（从本地文件恢复）
    bool loadGame(const std::string& savePath);
    bool loadGame2(const std::string& savePath = "village_save.txt");

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

    // 获取敌军接口（用于给建筑寻找攻击目标）
    std::vector<BaseTroop*>& getAllEnemyTroops() {
        if (_enemyTroops.capacity() > 10000 || _enemyTroops.size() > 10000) {
            CCLOGERROR("敌方士兵列表异常，长度：%zu，容量：%zu", _enemyTroops.size(), _enemyTroops.capacity());
            _enemyTroops.clear();
            return _enemyTroops;
        }
        return _enemyTroops;
    }

    // 移除敌军军队指针（用于敌军士兵死亡后移除）
    void removeEnemyTroop(BaseTroop* troop);

    // 添加/移除敌方兵种
    void addEnemyTroop(BaseTroop* troop) {
        _enemyTroops.push_back(troop);
    }

    // 设置基础模式（关卡，创造，普通）
    void setBaseMode(const BaseMode& baseMode) {
        _baseMode = baseMode;
    }
    BaseMode getBaseMode() const { return _baseMode; }

    // 隐藏兵种模式按钮
    void setTroopModeBtnInvisible() {
        _troopModeBtn->setVisible(true);
    }

    // 摧毁建筑核心函数
    void destroyBuilding(BaseBuilding* building);

    // 显示无法放置提示
    void showCannotPlaceTip(Vec2 pos);
    void showText(std::string string, Vec2 pos = Vec2(1000, 1000), float duringTime = 2.0f);

    // 资源管理接口
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

protected:
    bool level_init();
    ui::Layout* _uiLayer;  // 资源显示层（方便统一管理）

private:
    BaseMode _baseMode; // 所有模式管理器
    //static VillageScene* _instance; // 单例实例指针

    // -------------------------- 成员变量 --------------------------
    // 地图核心对象
    TMXTiledMap* _tileMap;       // 等轴测地图对象
    Size _tileSize;              // 单个瓦片尺寸
    Size _mapSize;               // 地图总瓦片数（宽×高）
    TMXLayer* _placeLayer;       // 可放置建筑的图层
    TMXLayer* _pathLayer;        // 可通行图层
    TMXLayer* _bgLayer;          // 背景图层
    Node* _mapContainer;         // 地图容器节点（用于整体缩放/拖拽）
    Mode _Mode = Mode::NONE;     // 当前模式
    Mode _lastMode = Mode::NONE; // 上一次模式（用于切换回原模式）
    Sprite* _mousePosSprite;     // 测试用，显示鼠标位置

    // 建筑放置相关
    Sprite* _buildPreview;                   // 建筑放置预览图
    BuildingType _selectedBuildingType;      // 选中的建筑类型
    std::vector<Vec2> _occupiedTiles;        // 已占用的瓦片
    Vec2 _lastTile;                          // 上一次鼠标所在瓦片
    bool _isContinuousPlace = false;         // 是否连续放置建筑
    BaseBuilding* _movingBuilding;

    // 定义场景最大瓦片数（固定值）
    static const int MAX_TILE_X = 400;  // 瓦片x轴最大索引
    static const int MAX_TILE_Y = 400;  // 瓦片y轴最大索引

    // 核心：二维布尔数组，标记瓦片是否被占用（快速查询）
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

    bool _isBuildBarShow = false;       // 建筑栏是否显示
    Layer* _buildBarLayer = nullptr;    // 建筑栏容器层
    bool _isAnyBuildSelected = false;   // 是否有建筑被选中（移动）

    // 存储所有建筑（基类指针）
    std::vector<BaseBuilding*> _buildings;
    // 按类型拆分存储（便于快速查找）
    std::vector<GoldMine*> _goldMines;
    std::vector<ElixirCollector*> _elixirCollectors;

    // 缩放相关
    float _minScale = 0.5f;      // 最小缩放比例
    float _maxScale = 2.0f;      // 最大缩放比例
    float _scaleStep = 0.1f;     // 缩放步长

    // 拖拽相关
    bool _isDragging = false;    // 是否正在拖拽
    Vec2 _lastMousePos;          // 上一帧鼠标位置
    Vec2 _mapOriginPos;          // 地图初始位置（用于计算偏移）

    // 瓦片高亮相关
    bool _isLastMouseLeftButtonDown = false; // 上一帧鼠标左键状态
    std::vector<Vec2> _lastTilePos;          // 上一个选中的瓦片坐标
    bool _hasLastTile = false;               // 是否有上一个瓦片需要恢复
    Color3B _originalTileColor;              // 瓦片原始颜色（用于恢复）

    // 训练营相关
    ui::Layout* _trainingPopup = nullptr;    // 训练弹窗根节点
    TrainingCamp* _currentCamp = nullptr;    // 当前关联的训练营
    const int MAX_QUEUE_SIZE = 5;            // 最大训练队列数

    // -------------------------- 兵种相关成员变量 --------------------------
    bool _isTroopBarShow = false;            // 兵种栏是否显示
    Sprite* _troopPreview;                   // 兵种放置预览图
    TroopType _selectedTroopType = TroopType::UNKNOWN; // 选中的兵种类型
    std::vector<BaseTroop*> _spawnedTroops;  // 已生成的所有兵种
    Vec2 _troopSpawnTilePos;                 // 兵种出生瓦片坐标
    std::vector<BaseTroop*> _enemyTroops;    // 敌方兵种列表(预留)

    // 资源相关
    int _gold;
    int _elixir;
    int _maxGold = 10000;
    int _maxElixir = 10000;
    int _maxPopulation = 0; // 人口总数
    int _population = 0;    // 当前人口数
    std::unordered_map<TroopType, int> _troopStorage; // 兵营存储的兵种信息
    std::unordered_map<TroopType, int> _troopLevel;   // 兵种等级信息
    int _builder;           // 建造者数量

    cocos2d::Label* _goldLabel;      // 金币显示标签
    cocos2d::Label* _elixirLabel;    // 圣水显示标签
    cocos2d::Sprite* _goldIcon;      // 金币图标
    cocos2d::Sprite* _elixirIcon;    // 圣水图标

    // -------------------------- 方法声明 --------------------------
    // 战斗相关
    float _totalTime;       // 总倒计时时长
    float _remainingTime;   // 剩余时间

    // UI组件
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
    float destroyPercent;           // 建筑摧毁百分比
    int currentStars;               // 当前星级
    Label* percentLabel;            // 百分比显示标签
    Vector<cocos2d::Sprite*> starSprites; // 星级精灵数组
    Vec2 _starTargetPos;            // 星级最终停留位置
    float _totalBuildingCount;      // 总建筑数量
    float _destroyedBuildingCount;  // 已摧毁建筑数量

    // 事件回调
    void onMouseScroll(Event* event);
    void onMouseDown(Event* event);
    void onMouseMove(Event* event);
    void onMouseUp(Event* event);

    // 鼠标移动时高亮瓦片
    void setTileColor(Vec2 tilePos, Color3B color, BuildingType type);
    void restoreLastTileColor();

    // 限制地图拖动范围
    void clampMapPosition();

    // 初始化地图
    void initMap();
    void initBtns(BaseMode baseMode = BaseMode::CREATING); // 初始化所有按钮

    // 初始化建筑预览
    void initBuildPreview();
    void initFightScene();

    // 初始化触摸事件 (暂未使用)
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

    // 放置建筑
    void placeBuilding(Vec2 tilePos, BuildingType type);
    void moveBuilding(BaseBuilding* building, Vec2 newTilePos);

    // 切换建筑栏显示/隐藏
    void toggleBuildBar();
    // 创建建筑栏
    void createBuildBar();
    // 隐藏建筑栏
    void hideBuildBar();

    // 检测瓦片是否被占用
    bool isTileOccupied(Vec2 tilePos);

    // 处理建筑弹窗按钮点击回调
    void handleBuildingBtnClick(BaseBuilding* building, BuildingPopup::ButtonType type);

    // 辅助：释放建筑占用的瓦片
    void releaseBuildingTiles(BaseBuilding* building);
    void addOccupiedTiles(const std::vector<Vec2>& tiles);
    void addOccupiedTile(const Vec2& tile);

    // 金矿生产控制
    void pauseAllGoldMines();
    void resumeAllGoldMines();

    // 一键收集所有资源
    void collectAllResources();

    // 训练营相关
    void initTrainingPopupUI();
    void initTroopButtonsInPopup();
    void initTrainQueuePanelInPopup();
    void refreshPopupTrainInfo();
    void updatePopupTimer(float dt);
    void refreshTrainQueueUI();
    void updateTrainQueueTimer(float dt);
    bool checkTroopResourceEnough(TroopType type);
    void deductTroopResource(TroopType type);
    void refundTroopResource(TroopType type);
    void addTroopToQueue(TroopType type);
    void removeTroopFromQueue(int index);

    // -------------------------- 兵种相关方法声明 --------------------------
    // 初始化兵种放置预览
    void initTroopPreview();
    // 创建兵种栏
    void createTroopBar();
    // 切换兵种栏
    void toggleTroopBar();
    // 隐藏兵种栏
    void hideTroopBar();
    // 放置（生成）兵种
    void spawnTroop(Vec2 screenPos, TroopType type);
    // 初始化兵种训练按钮
    void initTroopModeBtn();
    // 检测兵种可生成位置
    bool checkCanSpawnTroop(Vec2 tilePos);
    // 兵种攻击回调
    void onTroopAttack(BaseTroop* troop, BaseBuilding* target);

    // 资源管控
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
    void go_back_Btn();
    void gotoLevel1(const std::string& savePath);


    // 战斗相关
    void gotoFight();
    void backfromFight();
    void beginFight();
    void updateCountDown(float dt);
    void onFightSettle();

    // 星星相关
    void initStarRatingUI();
    void updateDestroyPercent();
    void checkStarUnlock();
    void flyStarToTarget(int starIndex);
    void updateStarDisplay();

    // -------------------------- 整体控制 --------------------------
    void hideModeBtn(); // 隐藏模式切换按钮
    void destroyScene(); // 销毁场景
    void onExit(); // 重写退出
    void cleanup(); // 重写清理

    // 存档相关
    SaveData::Village packSaveData();
    void unpackSaveData(const SaveData::Village& saveData);
    void initSaveLoadButtons();
};

#endif // __VILLAGE_SCENE_H__