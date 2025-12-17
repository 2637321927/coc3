#include "VillageScene.h"
#include "cocos2d.h"
using namespace cocos2d;

bool VillageScene::init()
{
    if (!Scene::init()) return false;
    //scsdcs
    // 初始化流程
    initMap();
	//TODO：建筑和触摸暂时屏蔽
    //initBuildPreview();
    //initTouchEvent();
        // 新增：屏幕居中+适配
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 让地图居中显示
    Vec2 mapCenter = Vec2(
        _tileMap->getContentSize().width / 2,
        _tileMap->getContentSize().height / 2
    );
    Vec2 screenCenter = Vec2(
        origin.x + visibleSize.width / 2,
        origin.y + visibleSize.height / 2
    );
    _tileMap->setPosition(screenCenter - mapCenter);
    //  监听鼠标滚轮事件
    auto  mouseListener = EventListenerMouse::create();
    // 绑定滚轮回调
    mouseListener->onMouseScroll = CC_CALLBACK_1(VillageScene::onMouseScroll, this);
    mouseListener->onMouseDown = CC_CALLBACK_1(VillageScene::onMouseDown, this);    // 鼠标按下
    mouseListener->onMouseMove = CC_CALLBACK_1(VillageScene::onMouseMove, this);    // 鼠标移动
    mouseListener->onMouseUp = CC_CALLBACK_1(VillageScene::onMouseUp, this);        // 鼠标松开
    // 添加监听到事件分发器
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
    return true;

}
// 鼠标按下：开始拖拽
void VillageScene::onMouseDown(Event* event)
{
	//TODO: 划分不可拖拽区域（放置建筑和一些按钮的位置）和拖拽区域
    EventMouse* e = (EventMouse*)event;
    // 只响应鼠标左键
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        _isDragging = true;
        _lastMousePos = Vec2(e->getCursorX(), e->getCursorY()); // 记录按下时鼠标位置
        _mapOriginPos = _tileMap->getPosition();                // 记录按下时地图位置
    }
}

// 鼠标移动：处理拖拽偏移
void VillageScene::onMouseMove(Event* event)
{
    if (!_isDragging) return; // 未拖拽则跳过

    EventMouse* e = (EventMouse*)event;
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    // 计算鼠标移动的偏移量（反向：鼠标右移→地图左移，符合直觉）
    Vec2 offset = currentMousePos - _lastMousePos;
    // 更新地图位置
    _tileMap->setPosition(_mapOriginPos + offset);
    _mapOriginPos += offset;
    // 限制地图不能拖出屏幕（关键：避免地图拖没）
    clampMapPosition();
    // 更新上一帧鼠标位置（用于下一帧计算）
    _lastMousePos = currentMousePos;
}

//鼠标松开：结束拖拽
void VillageScene::onMouseUp(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        // 计算鼠标移动距离，小于5像素则视为“点击”，否则是“拖拽”
        Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
        float moveDistance = currentPos.distance(_lastMousePos);
        if (moveDistance < 5.0f) {
            log("鼠标点击了地图，位置：%f, %f", currentPos.x, currentPos.y);
            // 这里可以加点击建筑/瓦片的逻辑
        }
        _isDragging = false;
    }
}

// 限制地图拖动范围（核心：避免地图拖出屏幕）
//TODO: 需要优化，保证不超过地图范围，但是目前地图还没做完
void VillageScene::clampMapPosition()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 mapPos = _tileMap->getPosition();
    float mapScale = _tileMap->getScale();

    // 计算地图缩放后的实际尺寸
    float mapWidth = _tileMap->getContentSize().width * mapScale;
    float mapHeight = _tileMap->getContentSize().height * mapScale;

    // 计算地图可拖动的边界（保证至少有一部分地图在屏幕内）
    float minX = visibleSize.width - mapWidth / 2;  // 左边界
    float maxX = mapWidth / 2;                      // 右边界
    float minY = visibleSize.height - mapHeight / 2;// 下边界
    float maxY = mapHeight / 2;                     // 上边界

    // 限制地图位置在边界内
    float clampedX = clampf(mapPos.x, minX, maxX);
    float clampedY = clampf(mapPos.y, minY, maxY);
    _tileMap->setPosition(Vec2(clampedX, clampedY));
}
// 滚轮缩放核心函数
void VillageScene::onMouseScroll(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    float scrollY = e->getScrollY();
    float currentScale = _tileMap->getScale();
    float newScale = currentScale;

    // 1. 计算新缩放比例（不变）
    if (scrollY <0) {
        newScale = MIN(currentScale + _scaleStep, _maxScale);
    }
    else {
        newScale = MAX(currentScale - _scaleStep, _minScale);
    }

    // 获取鼠标当前位置（相对于屏幕）
    Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());
    // 计算缩放前鼠标到地图中心的偏移
    Vec2 mapPos = _tileMap->getPosition();
    Vec2 offset = mousePos - mapPos;
    // 缩放偏移量（按缩放比例调整）
    offset = offset * (newScale / currentScale);
    // 重新设置地图位置（跟随鼠标）
    _tileMap->setScale(newScale);
    _tileMap->setPosition(mousePos - offset);
}

Scene* VillageScene::createScene()
{
    auto scene = Scene::create();
    auto layer = VillageScene::create();
    scene->addChild(layer);
    return scene;
}
// 加载等轴测地图
void VillageScene::initMap()
{
    // 加载地图文件
    _tileMap = TMXTiledMap::create("map/map1.tmx");
    _tileMap->setAnchorPoint(Vec2::ZERO);
    _tileMap->setPosition(Vec2::ZERO);
    this->addChild(_tileMap, 0);

    // 记录地图参数
    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();

    // 获取关键图层（与Tiled中命名对应）
    _bgLayer = _tileMap->getLayer("bg_layer"); // 背景图层（有视觉纹理）
    _placeLayer = _tileMap->getLayer("place_layer");
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.5f)); // 锚点居中（拖拽/缩放都依赖）
    _tileMap->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _tileMap->setScale(1.0f);
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