#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__
#include "Building.h"
#include "Troop.h"
#include "cocos2d.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 
using namespace cocos2d;

std::vector<std::string> split(const std::string& s, const std::string& delim);
enum class Mode {
    NONE,       // 无建造模式
    PLACE_BUILDING,  // 放置建筑模式
    SPAWN_TROOP,    // 放置兵种模式
};
//存档相关
namespace SaveData {
    // 单栋建筑的存档数据
    struct Building {
        BuildingType type;          // 建筑类型（TOWN_HALL/GOLD_MINE等）
        cocos2d::Vec2 tilePos;      // 建筑所在格子坐标
        BuildingState state;        // 建筑状态（IDLE/BUILDING等）
        int level = 1;              // 建筑等级（如果有升级逻辑）

        // 序列化：将数据转为字符串（方便存储）
        std::string toString() const {
            std::stringstream ss;
            ss << (int)type << ","
                << tilePos.x << "," << tilePos.y << ","
                << (int)state << ","
                << level;
            return ss.str();
        }

        // 反序列化：从字符串恢复数据
        static Building fromString(const std::string& str) {
            Building data;
            std::vector<std::string> parts = split(str, ","); // 需实现split函数
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

    // 整个村庄的存档数据
    struct Village {
        std::vector<Building> buildings;       // 所有建筑数据
        cocos2d::Vec2 mapSize;                 // 地图尺寸（可选）
        std::vector<cocos2d::Vec2> occupiedTiles; // 已占用格子（可选，可通过建筑数据推导）
        Mode currentMode = Mode::NONE;         // 当前模式（可选）
		int gold=0; 						   // 金币
		int elixir=0;					   // 圣水
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
class VillageScene : public Scene
{
public:
    // Cocos2d-x 标准创建方法（必须）
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // CREATE_FUNC 宏：自动生成 create() 方法
    CREATE_FUNC(VillageScene);
    // 存档（保存到本地文件）
    bool saveGame(const std::string& savePath = "village_save.txt");
    // 读档（从本地文件恢复）
    bool loadGame(const std::string& savePath = "village_save.txt");
    // 按钮点击回调
    void onSaveBtnClicked(cocos2d::Ref* sender);
    void onLoadBtnClicked(cocos2d::Ref* sender);
private:
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
    // 建筑放置相关
    Sprite* _buildPreview;       // 建筑放置预览图
    BuildingType _selectedBuildingType;      // 选中的建筑类型
    std::vector<Vec2> _occupiedTiles;        // 已占用的瓦片
    bool _isBuildBarShow = false; // 建筑栏是否显示
    Layer* _buildBarLayer = nullptr; // 建筑栏容器层
    // 存储所有建筑（基类指针，兼容所有建筑类型）
    ui::Button* _saveBtn;
    ui::Button* _loadBtn;
    std::vector<BaseBuilding*> _buildings;
    // 按类型拆分存储（便于快速查找）
    std::vector<GoldMine*> _goldMines;
    std::vector<TownHall*> _townHalls;
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
	// 金矿生产控制(如切换到其他场景，战争模式时暂停生产)
	void pauseAllGoldMines();// 暂停所有金矿生产
	void resumeAllGoldMines();// 恢复所有金矿生产
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
	// 获取建筑配置
    const BuildingConfig& VillageScene::getBuildingConfigByType(BuildingType type);
	// 处理建筑弹窗按钮点击回调
    void handleBuildingBtnClick(BaseBuilding* building, BuildingPopup::ButtonType type);
    // 摧毁建筑核心函数
    void destroyBuilding(BaseBuilding* building);
    // 辅助：释放建筑占用的瓦片
    void releaseBuildingTiles(BaseBuilding* building);
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
    // -------------------------- 整体控制--------------------------
	void hideModeBtn();//TODO: 隐藏模式切换按钮，每次切换模式时调用，防止模式重叠，或者，另外一个思路，在按钮上加限制，只有主模式才能按按钮
    //存档相关
    // 辅助：将当前场景数据转为存档结构
    SaveData::Village packSaveData();
    // 辅助：从存档结构恢复场景数据
    void unpackSaveData(const SaveData::Village& saveData);
    void initSaveLoadButtons(); // 辅助：创建UI按钮（新增）
  
};
#endif // __VILLAGE_SCENE_H__
