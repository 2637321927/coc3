#include "VillageScene.h"
#include "cocos2d.h"
#include "Building.h"

bool VillageScene::init()
{
    if (!Scene::init()) return false;
    // 初始化流程
    _mapContainer = Node::create();
    this->addChild(_mapContainer);
    initMap();
    initBuildPreview();
    initBuildModeBtn(); 
    //TODO：建筑和触摸暂时屏蔽
 
    //initTouchEvent();
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
//设置瓦片颜色（测试坐标转换函数是否正确）
void VillageScene::setTileColor(Vec2 tilePos, Color3B color) {
    // 1校验瓦片坐标是否有效
    if (tilePos.x < 0 || tilePos.x >= _tileMap->getMapSize().width
        || tilePos.y < 0 || tilePos.y >= _tileMap->getMapSize().height) {
        return;
    }

    // 2. 获取瓦片对应的精灵（TMXLayer本质是SpriteBatchNode，每个瓦片是Sprite）
    Sprite* tileSprite = _bgLayer->getTileAt(tilePos);
    if (!tileSprite) { // 空瓦片（无精灵）
        return;
    }

    // 3. 记录原始颜色（仅第一次设置时记录）
    if (!_hasLastTile) {
        _originalTileColor = tileSprite->getColor();
    }

    // 4. 设置瓦片颜色（叠加色，白色为原始色）
    tileSprite->setColor(color);
}

// 新增：恢复上一个瓦片的原始颜色
void VillageScene::restoreLastTileColor() {
    if (!_hasLastTile) {
        return;
    }

    Sprite* lastTileSprite = _bgLayer->getTileAt(_lastTilePos);
    if (lastTileSprite) {
        lastTileSprite->setColor(_originalTileColor); // 恢复原始颜色
    }

    _hasLastTile = false; // 重置标记
}
// 鼠标按下：开始拖拽/记录位置
//TODO : 划分不可拖拽区域（放置建筑和一些按钮的位置）和拖拽区域
void VillageScene::onMouseDown(Event* event)
{
    //TODO: 划分不可拖拽区域（放置建筑和一些按钮的位置）和拖拽区域;
    // 只响应鼠标左键
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        _isDragging = true;
        _lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
        // 记录容器的位置
        _mapOriginPos = _mapContainer->getPosition();
        // 只有建筑栏显示且处于建造模式时，才处理放置逻辑
        if (_isBuildBarShow && _Mode == Mode::PLACE_BUILDING) {
            Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
            Vec2 tilePos = screenToIsoTile(currentPos);
            tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));
            if (checkCanPlace(tilePos)) {
				placeBuilding(currentPos, _selectedBuildingType);//应该传瓦片坐标更合适，不过瓦片转容器有误，先传入屏幕坐标，屏幕转容器和屏幕转瓦片坐标是正确的
                // 可选：放置后不清空建造模式，继续放置同类型建筑
                // _buildMode = BuildMode::NONE;
                // _buildPreview->setVisible(false);
            }
            else {
                showCannotPlaceTip(currentPos);
            }
        }
    }
}
// 鼠标移动：处理拖拽偏移/建筑预览跟随
void VillageScene::onMouseMove(Event* event)    
{
    EventMouse* e = (EventMouse*)event;
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    /*
    float mouseX = e->getCursorX(); // 屏幕X（原点左上角，向右为+）
    float mouseY = e->getCursorY(); // 屏幕Y（原点左上角，向下为+）
    Vec2 currentTilePos = screenToIsoTile(Vec2(mouseX, mouseY));
    // 转为整数（瓦片坐标是索引，必须是整数）
    currentTilePos = Vec2(floor(currentTilePos.x), floor(currentTilePos.y));
    // 先恢复上一个瓦片的颜色
    restoreLastTileColor();
    //  给当前瓦片设置高亮色（比如半透明红色/绿色）
    setTileColor(currentTilePos, Color3B::YELLOW); // 黄色高亮，可改为Color3B(255,0,0,180)（半透红）

    //记录当前瓦片为“上一个瓦片”，用于下次恢复
    _lastTilePos = currentTilePos;
    _hasLastTile = true;
    */
    if (_isDragging&& _Mode != Mode::PLACE_BUILDING) {
        Vec2 offset = currentMousePos - _lastMousePos;
        _mapContainer->setPosition(_mapOriginPos + offset);
        // 注意：不要在 Move 里累加 _mapOriginPos，除非你每一帧都重新赋值
    }
    else {
        // 建造预览跟随（磁吸效果）
        if (_Mode == Mode::PLACE_BUILDING && _buildPreview->isVisible()) {
            Vec2 tilePos = screenToIsoTile(currentMousePos);
            // 将预览图位置设为容器本地坐标
            Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(currentMousePos);
            _buildPreview->setPosition(containerLocalPos);
            _buildPreview->setColor(checkCanPlace(tilePos) ? Color3B::GREEN : Color3B::RED);
        }
    }
    
 
}
// 显示无法放置提示
void VillageScene::showCannotPlaceTip(Vec2 pos) {
    auto tip = Label::createWithTTF("无法放置在这里!", "fonts/Marker Felt.ttf", 20);
    tip->setColor(Color3B::RED);
    tip->setPosition(pos + Vec2(0, 30));
    this->addChild(tip, 20);

    // 2秒后自动消失
    tip->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}
//鼠标松开：结束拖拽
void VillageScene::onMouseUp(Event* event)
{
        EventMouse* e = (EventMouse*)event;
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _isDragging = false;
         }
    /*
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
    */
}
// 滚轮缩放核心函数
void VillageScene::onMouseScroll(Event* event)
{
    EventMouse* e = (EventMouse*)event;
    float currentScale = _mapContainer->getScale();
    float newScale = (e->getScrollY() < 0) ?
        MIN(currentScale + _scaleStep, _maxScale) :
        MAX(currentScale - _scaleStep, _minScale);

    Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 containerPos = _mapContainer->getPosition();

    // 围绕鼠标点进行缩放的算法
    Vec2 offset = mousePos - containerPos;
    Vec2 newPos = mousePos - offset * (newScale / currentScale);

    _mapContainer->setScale(newScale);
    _mapContainer->setPosition(newPos);
}
// 限制地图拖动范围（核心：避免地图拖出屏幕）
//TODO: 需要优化，保证不超过地图范围，但是目前地图还没做完
void VillageScene::clampMapPosition()
{
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 containerPos = _mapContainer->getPosition();
    float containerScale = _mapContainer->getScale();

    // 计算地图实际大小（考虑缩放）
    float mapWidth = _tileMap->getContentSize().width * containerScale;
    float mapHeight = _tileMap->getContentSize().height * containerScale;

    // 计算可移动的边界（确保地图不会移出屏幕）
    float minX = visibleSize.width - mapWidth / 2;
    float maxX = mapWidth / 2;
    float minY = visibleSize.height - mapHeight / 2;
    float maxY = mapHeight / 2;

    // 限制位置
    float clampedX = clampf(containerPos.x, minX, maxX);
    float clampedY = clampf(containerPos.y, minY, maxY);
    _mapContainer->setPosition(Vec2(clampedX, clampedY));
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
    _tileMap = TMXTiledMap::create("map/map1.tmx");
    // 关键：加到 _mapContainer，而不是 this
    _mapContainer->addChild(_tileMap, 0);

    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();
    _bgLayer = _tileMap->getLayer("bg_layer");

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 整个容器居中
    _mapContainer->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    // 地图在容器内部居中（相对于容器原点）
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.0f));
    _tileMap->setPosition(Vec2::ZERO);
    /*
    // 加载地图文件
    _tileMap = TMXTiledMap::create("map/map1.tmx");
    this->addChild(_tileMap, 0);

    // 记录地图参数
    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();

    // 获取关键图层（与Tiled中命名对应）
    _bgLayer = _tileMap->getLayer("bg_layer"); // 背景图层（有视觉纹理）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.5f)); // 锚点居中（拖拽/缩放都依赖）
    _tileMap->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));//初始位置居中
    _tileMap->setScale(1.0f);
    */
    //TODO
   // _pathLayer = _tileMap->getLayer("path_layer");
}
// 初始化建筑放置预览图
void VillageScene::initBuildPreview() {
// 确保 _mapContainer 已经创建
    if (!_mapContainer) return;

    _buildPreview = Sprite::create(); 
    if (_buildPreview) {
        _buildPreview->setVisible(false);
        _buildPreview->setOpacity(150);
        _buildPreview->setAnchorPoint(Vec2(0.5f, 0.5f)); // 建议设置底部锚点
        // 添加到地图容器，层级设高一点（比如 99），确保在建筑上方
        _buildPreview->setScale(_mapContainer->getScale());
        _mapContainer->addChild(_buildPreview, 99); 
    } else {
        CCLOG("Error: Could not create _buildPreview sprite!");
    }
}

// 初始化触摸交互
/*void VillageScene::initTouchEvent()
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
*/
// 屏幕坐标 → 等轴测瓦片坐标
Vec2 VillageScene::screenToIsoTile(Vec2 screenPos)
{    
    // 首先：将屏幕坐标转换为地图层（_tileMap）的本地坐标
       // 这一步会自动处理地图的拖拽位置和缩放（Scale）
    Vec2 localPos = _tileMap->convertToNodeSpace(screenPos);

    /* 等轴测 45° 转换公式逻辑：
       mapWidth = 瓦片总宽 * 瓦片像素宽度
       tileWidth = _tileSize.width
       tileHeight = _tileSize.height
    */

    float tw = _tileSize.width;
    float th = _tileSize.height;
    float mw = _mapSize.width;
    float mh = _mapSize.height;

    // TMX 45度地图的坐标系原点在顶部中心，公式如下：
    // pos.x = (x - y) * (tw/2) + mapWidth/2
    // pos.y = (mw + mh - x - y) * (th/2)

    // 反推得出 Tile 坐标：
    float isox = localPos.x / tw;
    float isoy = localPos.y / th;
    float originShift = mw / 2.0f; // Tiled默认的X轴偏移

    // 核心转换公式（适用于标准 Tiled Isometric 格式）
    int tileX = (int)(mh - isoy + (isox - originShift));
    int tileY = (int)(mh - isoy - (isox - originShift));

    return Vec2(tileX, tileY);
}

// 等轴测瓦片坐标 → 屏幕坐标
Vec2 VillageScene::isoTileToScreen(Vec2 tilePos)
{
// 使用 TMXLayer 自带的 getPositionAt 可以获得瓦片的顶点
    // 如果你想手动计算以便更灵活地控制偏移：
    float tw = _tileSize.width;
    float th = _tileSize.height;
    float mw = _mapSize.width;
    
    // 计算该瓦片在地图本地坐标系下的位置
    float posX = (tilePos.x - tilePos.y) * (tw / 2.0f) + (mw * tw / 2.0f);
    float posY = (_mapSize.width + _mapSize.height - tilePos.x - tilePos.y - 2) * (th / 2.0f);

    // 将地图本地坐标转回世界坐标（屏幕坐标）
    return _tileMap->convertToWorldSpace(Vec2(posX, posY));
}
// 瓦片逻辑坐标 → 容器本地坐标
Vec2 VillageScene::isoTileToContainerPos(Vec2 tilePos) {
    float tileW_half = _tileSize.width / 2.0f; // 32
    float tileH_half = _tileSize.height / 2.0f; // 16
    // 等轴核心公式：逻辑坐标转容器本地像素坐标
    float x = (tilePos.x - tilePos.y) * tileW_half;
    float y = (tilePos.x + tilePos.y) * tileH_half;
    return Vec2(x, y);
}
// 检测瓦片是否可放置建筑
//用途：判断某个瓦片是否允许放置建筑，在建筑放置预览和实际放置时调用，根据这个建筑覆盖的所有瓦片依次调用
bool VillageScene::checkCanPlace(Vec2 tilePos)
{
    // 校验瓦片坐标是否越界
    if (tilePos.x < 0 || tilePos.x >= _mapSize.width
        || tilePos.y < 0 || tilePos.y >= _mapSize.height) {
        return false;
    }

    // 获取bg_layer中该位置的瓦片GID 
    unsigned int tileGID = _bgLayer->getTileGIDAt(tilePos);
    if (tileGID == 0) { // 空瓦片（无属性）
        return false;
    }
    Value propValue = _tileMap->getPropertiesForGID(tileGID);
    if (propValue.getType() != cocos2d::Value::Type::MAP) {
        // 属性不是MAP类型（比如空值、字符串），视为不可放置
        return false;
    }
    // 直接获取属性并转为ValueMap
    ValueMap tileProps = _tileMap->getPropertiesForGID(tileGID).asValueMap();

    // 读取canPlace属性（容错：无该属性则返回false）
    if (tileProps.count("canPlace") == 0) {
        return false;
    }

    return tileProps["canPlace"].asBool();
}
// 初始化建筑栏
void VillageScene::createBuildBar() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建建筑栏容器层（方便整体隐藏/显示）
    _buildBarLayer = Layer::create();
    this->addChild(_buildBarLayer, 99); // 层级低于开关按钮，高于地图

    // 建筑栏背景
    auto barBg = Sprite::create("ui/build_bar_bg.png");
    barBg->setPosition(Vec2(visibleSize.width / 2, 50));
    barBg->setScaleX(visibleSize.width / barBg->getContentSize().width * 0.8f);
    _buildBarLayer->addChild(barBg);

    // 建筑按钮 - 大本营
    auto townHallBtn = MenuItemImage::create(
        "building/town_hall_icon.png",
        "building/town_hall_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::TOWN_HALL;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/town_hall_preview.png");
        }
    );

    // 建筑按钮 - 金矿
    auto goldMineBtn = MenuItemImage::create(
        "building/gold_mine_icon.png",
        "building/gold_mine_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::GOLD_MINE;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/gold_mine_preview.png");
        }
    );

    // 取消放置按钮（仅退出当前建造模式，不隐藏建筑栏）
    auto cancelPlaceBtn = MenuItemImage::create(
        "ui/cancel_place_btn.png",
        "ui/cancel_place_btn_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::NONE;
            _buildPreview->setVisible(false);
        }
    );

    // 排列按钮
    auto menu = Menu::create(townHallBtn, goldMineBtn, cancelPlaceBtn, nullptr);
    menu->alignItemsHorizontallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width / 2, 50));
    _buildBarLayer->addChild(menu);
}
// 隐藏建筑栏
void VillageScene::hideBuildBar() {
    if (_buildBarLayer) {
        _buildBarLayer->setVisible(false);
    }
}
// 放置建筑
void VillageScene::placeBuilding(Vec2 tilePos, BuildingType type) {
    Vec2 tilePos1 = screenToIsoTile(tilePos);
    auto building = BaseBuilding::create(type, tilePos1, 1.0f);
    if (building) {
        auto config = building->getConfig();

        // 记录该建筑占用的所有瓦片
        for (int x = 0; x < config.tileWidth; ++x) {
            for (int y = 0; y < config.tileHeight; ++y) {
                _occupiedTiles.push_back(Vec2(tilePos1.x + x, tilePos1.y + y));
            }
        }
        Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(tilePos);
        building->setPosition(containerLocalPos);
        _mapContainer->addChild(building);

        // Z-Order 排序建议：使用底部中心点的 Y 坐标
        building->setLocalZOrder(1000 - (tilePos1.x + tilePos1.y));
        // 记录占用
        _occupiedTiles.push_back(tilePos1);
    }
}
// 初始化建筑模式切换按钮
void VillageScene::initBuildModeBtn() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建建筑模式开关按钮（右上角悬浮）
    auto buildModeBtn = MenuItemImage::create(
        "ui/build_mode_btn_normal.png",  // 正常状态图片
        "ui/build_mode_btn_selected.png",// 按下状态图片
        [this](Ref* sender) {
            this->toggleBuildBar(); // 点击切换建筑栏
        }
    );
    // 设置按钮大小和位置（可根据需求调整）
    buildModeBtn->setScale(0.8f);
    buildModeBtn->setPosition(Vec2(visibleSize.width - 50, visibleSize.height - 50));

    auto menu = Menu::create(buildModeBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100); // 最高层级，确保不被遮挡
}
// 切换建筑栏显示/隐藏
void VillageScene::toggleBuildBar() {
    if (!_isBuildBarShow) {
        // 显示建筑栏：首次显示则创建，非首次则显示已有层
        if (!_buildBarLayer) {
            this->createBuildBar();
        }
        else {
            _buildBarLayer->setVisible(true);
        }
        _isBuildBarShow = true;
    }
    else {
        // 隐藏建筑栏：同时退出建造模式，隐藏预览
        this->hideBuildBar();
        _isBuildBarShow = false;
        _Mode = Mode::NONE;
        if (_buildPreview) {
            _buildPreview->setVisible(false);
        }
    }
}
// 校验瓦片是否已占用
bool VillageScene::isTileOccupied(Vec2 tilePos) {
    for (const auto& pos : _occupiedTiles) {
        if (pos.equals(tilePos)) {
            return true;
        }
    }
    return false;
}
