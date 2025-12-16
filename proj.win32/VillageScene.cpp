#include "VillageScene.h"
#include "cocos2d.h"
using namespace cocos2d;

bool VillageScene::init()
{
    if (!Scene::init()) return false;

    // 初始化流程
    initMap();
    initBuildPreview();
    initTouchEvent();

    return true;
}

// 加载等轴测地图
void VillageScene::initMap()
{
    // 1. 加载地图文件
    _tileMap = TMXTiledMap::create("map/map1.tmx");
    _tileMap->setAnchorPoint(Vec2::ZERO);
    _tileMap->setPosition(Vec2::ZERO);
    this->addChild(_tileMap, 0);

    // 2. 记录地图参数
    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();

    // 3. 获取关键图层（与Tiled中命名对应）
    _placeLayer = _tileMap->getLayer("place_layer");
    //TODO
   // _pathLayer = _tileMap->getLayer("path_layer");
}

// 初始化建筑放置预览图
void VillageScene::initBuildPreview()
{
    // 创建半透明预览图（示例：大本营）
    _buildPreview = Sprite::create("building/town_hall.png");
    _buildPreview->setOpacity(150); // 半透明
    _buildPreview->setVisible(false); // 默认隐藏
    this->addChild(_buildPreview, 2); // 层级高于地图
}

// 初始化触摸交互
void VillageScene::initTouchEvent()
{
    auto listener = EventListenerTouchOneByOne::create();

    // 触摸开始：显示预览
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        _buildPreview->setVisible(true);
        return true;
        };

    // 触摸移动：预览跟随+可放置检测
    listener->onTouchMoved = [this](Touch* touch, Event* event) {
        Vec2 screenPos = touch->getLocation();
        Vec2 tilePos = screenToIsoTile(screenPos);
        Vec2 buildPos = isoTileToScreen(tilePos);

        // 预览图跟随触摸
        _buildPreview->setPosition(buildPos);

        // 检测可放置，切换预览颜色
        if (checkCanPlace(tilePos)) {
            _buildPreview->setColor(Color3B::GREEN);
        }
        else {
            _buildPreview->setColor(Color3B::RED);
        }
        };

    // 触摸结束：放置建筑
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        Vec2 screenPos = touch->getLocation();
        Vec2 tilePos = screenToIsoTile(screenPos);

        if (checkCanPlace(tilePos)) {
            placeBuilding(tilePos); // 放置建筑
        }
        _buildPreview->setVisible(false); // 隐藏预览
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

// 屏幕坐标 → 等轴测瓦片坐标
Vec2 VillageScene::screenToIsoTile(Vec2 screenPos)
{
    float tileW = _tileSize.width;
    float tileH = _tileSize.height;

    // 等轴测核心转换公式
    int tileX = (screenPos.x / tileW + screenPos.y / tileH) / 2;
    int tileY = (screenPos.y / tileH - screenPos.x / tileW) / 2;

    // 限制在地图范围内
    tileX = clampf(tileX, 0, _mapSize.width - 1);
    tileY = clampf(tileY, 0, _mapSize.height - 1);

    return Vec2(tileX, tileY);
}

// 等轴测瓦片坐标 → 屏幕坐标
Vec2 VillageScene::isoTileToScreen(Vec2 tilePos)
{
    float tileW = _tileSize.width;
    float tileH = _tileSize.height;

    float x = (tilePos.x - tilePos.y) * (tileW / 2);
    float y = (tilePos.x + tilePos.y) * (tileH / 2);

    return Vec2(x, y);
}

// 检测瓦片是否可放置建筑
bool VillageScene::checkCanPlace(Vec2 tilePos)
{
    // 1. 校验瓦片坐标是否越界
    if (tilePos.x < 0 || tilePos.x >= _mapSize.width
        || tilePos.y < 0 || tilePos.y >= _mapSize.height) {
        return false;
    }

    // 2. 获取place_layer中该位置的瓦片GID
    unsigned int tileGID = _placeLayer->getTileGIDAt(tilePos);
    if (tileGID == 0) { // 空瓦片（无属性）
        return false;
    }

    // 3. 直接获取属性并转为ValueMap（4.0版本无需类型判断，强转即可）
    ValueMap tileProps = _tileMap->getPropertiesForGID(tileGID).asValueMap();

    // 4. 读取canPlace属性（容错：无该属性则返回false）
    if (tileProps.count("canPlace") == 0) {
        return false;
    }
    return tileProps["canPlace"].asBool();
}

// 放置建筑
void VillageScene::placeBuilding(Vec2 tilePos)
{
    Vec2 buildPos = isoTileToScreen(tilePos);

    // 创建建筑Sprite（示例：大本营）
    Sprite* townHall = Sprite::create("building/town_hall.png");
    townHall->setPosition(buildPos);
    townHall->setLocalZOrder(tilePos.y); // 等轴测层级遮挡（Y值越大越靠上）
    this->addChild(townHall, 1);
}

Scene* VillageScene::createScene()
{
    auto scene = Scene::create();
    auto layer = VillageScene::create();
    scene->addChild(layer);
    return scene;
}