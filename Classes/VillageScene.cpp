#include "VillageScene.h"
#include "cocos2d.h"
#include "Building.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 

bool VillageScene::init()
{
    if (!Scene::init()) return false;
    // 初始化流程

    initResourceBar();
    _mapContainer = Node::create();
    this->addChild(_mapContainer);
    initMap();
    initBuildPreview();
    initBuildModeBtn();
    initTroopModeBtn();
    initTroopPreview();
    initSaveLoadButtons();
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
//设置瓦片颜色（放置预览）
void VillageScene::setTileColor(Vec2 tilePos, Color3B color, BuildingType type) {
    auto config = getBuildingConfigByType(_selectedBuildingType);
    // 1校验瓦片坐标是否有效
   // 校验瓦片坐标是否越界
    if (tilePos.x < 0 || tilePos.x + config.tileHeight - 1 >= _mapSize.width
        || tilePos.y < 0 || tilePos.y + config.tileWidth - 1 >= _mapSize.height) {
        return;
    }
    // 获取瓦片对应的精灵（TMXLayer本质是SpriteBatchNode，每个瓦片是Sprite）
    for (int i = tilePos.x;i <= tilePos.x + config.tileHeight - 1;i++) {
        for (int j = tilePos.y;j < tilePos.y + config.tileWidth - 1;j++) {
            Sprite* tileSprite = _bgLayer->getTileAt(Vec2(i, j));
            if (!tileSprite) { // 空瓦片（无精灵）
                return;
            }

            // 记录原始颜色（仅第一次设置时记录）
            if (!_hasLastTile) {
                _originalTileColor = tileSprite->getColor();
            }

            // 设置瓦片颜色（叠加色，白色为原始色）
            tileSprite->setColor(color);
            //记录当前瓦片为“上一个瓦片”，用于下次恢复
            _lastTilePos.push_back(Vec2(i, j));
        }
    }
}

// 新增：恢复上一个瓦片的原始颜色
void VillageScene::restoreLastTileColor() {
    if (!_hasLastTile) {
        return;
    }
    for (int i = 0;i < _lastTilePos.size();i++) {
        Sprite* lastTileSprite = _bgLayer->getTileAt(_lastTilePos[i]);
        if (lastTileSprite) {
            lastTileSprite->setColor(_originalTileColor); // 恢复原始颜色
        }
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
    /*
    float mouseX = e->getCursorX(); // 屏幕X（原点左上角，向右为+）
    float mouseY = e->getCursorY(); // 屏幕Y（原点左上角，向下为+）
    Vec2 currentTilePos = screenToIsoTile(Vec2(mouseX, mouseY));
    // 转为整数（瓦片坐标是索引，必须是整数）
    currentTilePos = Vec2(floor(currentTilePos.x), floor(currentTilePos.y));
    // 先恢复上一个瓦片的颜色
    restoreLastTileColor();
    //  给当前瓦片设置高亮色（比如半透明红色/绿色）
    unsigned int tileGID = _bgLayer->getTileGIDAt(currentTilePos);
    if (tileGID != 0) { // 空瓦片（无属性）
        setTileColor(currentTilePos, Color3B::YELLOW); // 黄色高亮，可改为Color3B(255,0,0,180)（半透红）
    }

    //记录当前瓦片为“上一个瓦片”，用于下次恢复
    _lastTilePos = currentTilePos;
    _hasLastTile = true;
    */
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
            if (checkCanPlace(tilePos, _selectedBuildingType)) {
                placeBuilding(tilePos, _selectedBuildingType);//应该传瓦片坐标更合适，不过瓦片转容器有误，先传入屏幕坐标，屏幕转容器和屏幕转瓦片坐标是正确的
                // 可选：放置后不清空建造模式，继续放置同类型建筑
                // _buildMode = BuildMode::NONE;
                // _buildPreview->setVisible(false);
            }
            else {
                showCannotPlaceTip(currentPos);
            }
        }
        // 兵种放置逻辑
        if (_isTroopBarShow && _Mode == Mode::SPAWN_TROOP) {
            Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
            spawnTroop(currentPos, _selectedTroopType);
            // 可选：放置后不退出模式，继续生成同类型兵种
            // _Mode = Mode::NONE;
            // _troopPreview->setVisible(false);
        }

    }
}
// 鼠标移动：处理拖拽偏移/建筑预览跟随
void VillageScene::onMouseMove(Event* event)
{
    restoreLastTileColor();
    EventMouse* e = (EventMouse*)event;
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
    if (_isDragging && _Mode != Mode::PLACE_BUILDING) {
        Vec2 offset = currentMousePos - _lastMousePos;
        _mapContainer->setPosition(_mapOriginPos + offset);
        // 注意：不要在 Move 里累加 _mapOriginPos，除非你每一帧都重新赋值
    }
    else {
        // 建造预览跟随（磁吸效果）
        if (_Mode == Mode::PLACE_BUILDING && _buildPreview->isVisible()) {
            Vec2 tilePos = screenToIsoTile(currentMousePos);
            currentMousePos.y += 50; // 微调Y轴位置，以便显示真实放置格子
            // 将预览图位置设为容器本地坐标
            Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(currentMousePos);
            _buildPreview->setPosition(containerLocalPos);
            float mouseX = e->getCursorX();
            float mouseY = e->getCursorY();
            // 转为整数（瓦片坐标是索引，必须是整数）
            //currentTilePos = Vec2(floor(currentTilePos.x), floor(currentTilePos.y));
            // 先恢复上一个瓦片的颜色
            restoreLastTileColor();
            //  给当前瓦片设置高亮色（比如半透明红色/绿色）
            setTileColor(tilePos, checkCanPlace(tilePos, _selectedBuildingType) ? Color3B::GREEN : Color3B::RED, _selectedBuildingType); // 黄色高亮，可改为Color3B(255,0,0,180)（半透红）
            //TODO：越界会报错，需要修复


            _hasLastTile = true;
            _buildPreview->setColor(checkCanPlace(tilePos, _selectedBuildingType) ? Color3B::GREEN : Color3B::RED);
        }
    }
    // 兵种预览跟随
    if (_Mode == Mode::SPAWN_TROOP && _troopPreview && _troopPreview->isVisible()) {
        Vec2 tilePos = screenToIsoTile(currentMousePos);
        // 核心：和建筑预览用同一个位置计算方式
        Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(currentMousePos);
        _troopPreview->setPosition(containerLocalPos);
        _troopPreview->setScale(_mapContainer->getScale());
        // 临时：和建筑用同一个检测函数，确保预览颜色正确
        _troopPreview->setColor(checkCanPlace(tilePos, _selectedBuildingType) ? Color3B::GREEN : Color3B::RED);
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
    }
    else {
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
    // 基础参数获取
    const float tileW = _tileSize.width;    // 单个瓦片宽度
    const float tileH = _tileSize.height;   // 单个瓦片高度
    const float mapHalfW = _mapSize.width * tileW / 2.0f; // 地图宽度的一半

    // 计算瓦片中心点在瓦片地图节点内的本地坐标（等距投影核心公式）
    // 等距地图坐标转换原理：
    // X = (tileX - tileY) * 瓦片半宽 + 地图半宽（居中偏移）
    // Y = (tileX + tileY) * 瓦片半高
    float tileMapLocalX = (tilePos.x - tilePos.y) * (tileW / 2.0f) + mapHalfW;
    float tileMapLocalY = (tilePos.x + tilePos.y) * (tileH / 2.0f);
    Vec2 tileCenterInTileMap(tileMapLocalX, tileMapLocalY);

    // 转换为世界坐标（屏幕坐标）
    // 步骤：
    // - 先将瓦片地图内的本地坐标转换为瓦片地图节点的世界坐标
    // - 再转换为屏幕坐标（cocos2d-x中世界坐标等价于屏幕坐标，除了UI坐标系）
    Vec2 tileMapWorldPos = _tileMap->convertToWorldSpaceAR(tileCenterInTileMap);

    // 4. 兼容多分辨率适配（可选，根据你的屏幕适配策略调整）
    // 如果项目使用了多分辨率适配（如FitWidth/FitHeight），可添加适配偏移
    // 示例：
    // Size visibleSize = Director::getInstance()->getVisibleSize();
    // Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // tileMapWorldPos += origin;

    return tileMapWorldPos;
}
// 瓦片逻辑坐标 → 容器本地坐标
Vec2 VillageScene::isoTileToContainerPos(Vec2 tilePos) {
    // 1. 获取该瓦片在 Layer 内部的局部坐标（官方函数）
        // 注意：getPositionAt 返回的是瓦片菱形的底端点
    Vec2 basePos = _bgLayer->getPositionAt(tilePos);

    // 2. 计算中心点偏移：向上移动半个瓦片高度
    // Cocos2d-x 坐标系 Y 轴向上，所以是 + height/2
    Vec2 localCenter = basePos + Vec2(0, _tileSize.height / 2.0f);

    // 3. 将 Layer 内部坐标转换为容器 (_mapContainer) 的坐标
    // 考虑到你可能有多个 Layer 或者 Layer 做了偏移，用转换函数最安全
    return _mapContainer->convertToNodeSpace(_bgLayer->convertToWorldSpace(localCenter));
}

// 检测瓦片是否可放置建筑
//用途：判断某个瓦片是否允许放置建筑，在建筑放置预览和实际放置时调用，根据这个建筑覆盖的所有瓦片依次调用
bool VillageScene::checkCanPlace(Vec2 tilePos, BuildingType type)
{
    auto config = getBuildingConfigByType(_selectedBuildingType);
    // 校验瓦片坐标是否越界
    if (tilePos.x < 0 || tilePos.x + config.tileHeight - 1 >= _mapSize.width
        || tilePos.y < 0 || tilePos.y + config.tileWidth - 1 >= _mapSize.height) {
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
    //多格建筑判断
    bool isOccupied = 0;
    for (int i = tilePos.x;i <= tilePos.x + config.tileHeight - 1;i++) {
        for (int j = tilePos.y;j < tilePos.y + config.tileWidth - 1;j++) {
            if (isTileOccupied(Vec2(i, j))) {
                isOccupied = 1;
            }
        }
    }
    // 读取canPlace属性（容错：无该属性则返回false）
    if (tileProps.count("canPlace") == 0 || isOccupied) {
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
// 处理按钮点击的核心逻辑
void VillageScene::handleBuildingBtnClick(BaseBuilding* building, BuildingPopup::ButtonType type) {
    switch (type) {
    case BuildingPopup::ButtonType::INFO:
        // 显示建筑信息（示例：打印日志/弹出信息框）
        log("建筑信息：类型=%d，等级=%d，血量=%d，位置=(%f,%f)",
            (int)building->getType(),
            building->getConfig().level,
            building->getConfig().hp,
            building->getTilePos().x,
            building->getTilePos().y);
        break;

    case BuildingPopup::ButtonType::UPGRADE:
        // 建筑升级逻辑（调用BaseBuilding的升级方法）
        if (building->getState() == BuildingState::IDLE) { // 仅闲置状态可升级
            building->startUpgrade();
            log("建筑开始升级！");
        }
        else {
            log("建筑非闲置状态，无法升级！");
        }
        break;

    case BuildingPopup::ButtonType::COLLECT:
        // 收集金矿资源（仅金矿有效）
        if (building->getType() == BuildingType::GOLD_MINE) {
            auto goldMine = dynamic_cast<GoldMine*>(building);
            if (goldMine) {
                // 调用金矿收集资源的方法（需在GoldMine中实现）
                addGold(goldMine->collectGold());
            }
        }
        break;

    case BuildingPopup::ButtonType::DESTROY:
        // 摧毁建筑
        destroyBuilding(building);
        break;

    default:
        break;
    }
}
// 放置建筑
void VillageScene::placeBuilding(Vec2 tilePos, BuildingType type) {
    auto building = BaseBuilding::create(type, tilePos, 1.0f);
    if (building) {
		_buildPreview->setVisible(false);      
        // 加入建筑总列表（核心：保存实例引用，避免内存泄漏/无法管理）
        _buildings.push_back(building);
        // 按类型加入细分列表
        if (type == BuildingType::GOLD_MINE) {
            _goldMines.push_back(dynamic_cast<GoldMine*>(building));
        }
        else if (type == BuildingType::TOWN_HALL) {
            _townHalls.push_back(dynamic_cast<TownHall*>(building));
        }
        auto config = building->getConfig();

        // 记录该建筑占用的所有瓦片
        for (int x = 0; x < config.tileWidth; ++x) {
            for (int y = 0; y < config.tileHeight; ++y) {
                _occupiedTiles.push_back(Vec2(tilePos.x + x, tilePos.y + y));
            }
        }
        // 计算建筑占用瓦片范围的中心点（瓦片坐标）
        // 对于2x2建筑：中心在 (tilePos.x + (2-1)/2, tilePos.y + (2-1)/2) = (x+0.5, y+0.5)
        float centerTileX = tilePos.x + (config.tileWidth) / 2.0f;
        float centerTileY = tilePos.y + (config.tileHeight - 1) / 2.0f;
        Vec2 centerTilePos(centerTileX, centerTileY);

        // 将中心点转换为容器坐标（替代原左上角坐标）
        Vec2 containerLocalPos = isoTileToContainerPos(centerTilePos);

        // 确保建筑锚点居中（关键：默认锚点可能不是中心，需显式设置）
        building->setAnchorPoint(Vec2(0.5f, 0.5f));
        building->setPosition(containerLocalPos);
        _mapContainer->addChild(building);

        // Z-Order 排序建议：使用底部中心点的 Y 坐标
        building->setLocalZOrder(1000 - (tilePos.x + tilePos.y));
        // 通用逻辑：添加到场景 + 绑定点击回调
        if (building) {

            // 统一绑定点击回调（弹窗逻辑）
            building->bindClickCallback([this](BaseBuilding* building) {
               if (_Mode!= Mode::NONE) {
                    // 非 NONE 模式，直接返回（不触发任何交互）
                    return;
                }
                // 弹出功能窗口
                auto popup = BuildingPopup::create(building, [this, building](BuildingPopup::ButtonType type) {
                    handleBuildingBtnClick(building, type);
                    });
                this->addChild(popup, 100); // 高层级显示弹窗
                });
        }
		// 2. 延迟0.1秒切换回NONE模式(放置点击触碰到其他建筑会触发弹窗)
        this->scheduleOnce([this](float delay) {
            _Mode = Mode::NONE;
            }, 0.1f, "delay_switch_to_none_mode"); // 0.1秒延迟，定时器标签用于防重复
    
    }
}
// 摧毁建筑
void VillageScene::destroyBuilding(BaseBuilding* building) {
    if (!building) return; // 空指针防护
    // 返还建造资源
    /*
        auto& cost = building->getConfig().cost;
        // 示例：返还80%建造资源（可自定义比例）
        for (auto& [resType, resValue] : cost) {
            int returnValue = resValue * 0.8f;
            if (resType == "gold") {
                _playerGold += returnValue;
            }
            else if (resType == "elixir") {
                _playerElixir += returnValue;
            }
        
        // 刷新资源UI（需自行实现，如更新金币标签）
        // updateResourceUI();
    }
    */
    // 释放建筑占用的瓦片（地图位置）
    releaseBuildingTiles(building);
	// 从渲染层移除建筑节点(basebuilding中已经调用过一次了，可以不调用)
    //building->removeFromParentAndCleanup(true);
    // removeFromParentAndCleanup(true)：
    // 从_mapContainer的节点树中移除建筑
    // 清理建筑的所有子节点（图片、进度条、UI）
    // 调用建筑的onExit()，自动停止所有定时器/监听器

    // 从管理列表中移除建筑引用（逻辑层清理）
    // 从总列表移除
    auto it = std::find(_buildings.begin(), _buildings.end(), building);
    if (it != _buildings.end()) {
        _buildings.erase(it);
    }
	// 酚类型列表移除
	if (building->getType() == BuildingType::TOWN_HALL) {
		auto it1 = std::find(_townHalls.begin(), _townHalls.end(), dynamic_cast<TownHall*>(building));
		if (it1 != _townHalls.end()) {
			_townHalls.erase(it1);
		}
	}    else if (building->getType() == BuildingType::GOLD_MINE) {
        auto it1 = std::find(_goldMines.begin(), _goldMines.end(), dynamic_cast<GoldMine*>(building));
        if (it1 != _goldMines.end()) {
            _goldMines.erase(it1);
        }
    }
    building->destroy();
    // 内存释放（Cocos2d-x 自动管理)
    // Cocos2d-x 用 autorelease 池管理内存，removeFromParentAndCleanup(true) 后
    // 建筑实例会在下次主循环被自动销毁，无需手动delete
}
// 辅助：释放建筑占用的瓦片（摧毁后该位置可重新建造）
void VillageScene::releaseBuildingTiles(BaseBuilding* building) {
    if (!building) return;
    auto& config = building->getConfig();
    Vec2 tileStart = building->getTilePos();
    // 遍历建筑占用的所有瓦片
    for (int x = 0; x < config.tileWidth; ++x) {
        for (int y = 0; y < config.tileHeight; ++y) {
            Vec2 tile = Vec2(tileStart.x + x, tileStart.y + y);
            // 从_occupiedTiles中移除该瓦片
            auto tileIt = std::find(_occupiedTiles.begin(), _occupiedTiles.end(), tile);
            if (tileIt != _occupiedTiles.end()) {
                _occupiedTiles.erase(tileIt);
            }
        }
    }
}
// 批量停止所有金矿生产
void VillageScene::pauseAllGoldMines() {
    for (auto goldMine : _goldMines) {
        if (goldMine) {
            goldMine->unschedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold));
        }
    }
}
// 批量重启所有金矿生产
void VillageScene::resumeAllGoldMines() {
    for (auto goldMine : _goldMines) {
        if (goldMine && goldMine->getState() == BuildingState::IDLE) {
            goldMine->doSpecialAction();
        }
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
    buildModeBtn->setPosition(Vec2(visibleSize.width - 100, visibleSize.height - 100));

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
//获取建筑的配置
const BuildingConfig& VillageScene::getBuildingConfigByType(BuildingType type)
{
    static BuildingConfig Config;
    switch (type) {
    case(BuildingType::TOWN_HALL):
        Config = {
            1,                      // id
            BuildingType::TOWN_HALL,// type
            "大本营",               // name
            "building/town_hall.png", // imgPath
            2000,                   // hp
            3,                      // tileWidth
            3,                      // tileHeight
            { {"gold", 1000}, {"wood", 500} }, // cost
            30.0f                   // buildTime
        };
        break;
    case(BuildingType::GOLD_MINE):

        Config = {
            2,                      // id
            BuildingType::GOLD_MINE,// type
            "金矿",                 // name
            "building/gold_mine.png", // imgPath
            500,                    // hp
            3,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 500}, {"wood", 200} }, // cost
            10.0f                   // buildTime
        };
        break;

    case(BuildingType::BARRACKS):
        Config = {
            3,                      // id
            BuildingType::BARRACKS, // type
            "兵营",                 // name
            "building/barracks.png", // imgPath
            800,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 800}, {"wood", 300} }, // cost
            20.0f                   // buildTime
        };
        break;
    default:
        static BuildingConfig Config = {
            -1,
            BuildingType::UNKNOWN,
            "未知建筑",
            "",
            0,
            0,
            0,
            {},
            0.0f
        };
        break;
    }
    return  Config;
}
// -------------------------- 兵种相关方法 --------------------------
// 初始化兵种放置预览图
void VillageScene::initTroopPreview() {
    if (!_mapContainer) return;

    _troopPreview = Sprite::create();
    if (_troopPreview) {
        _troopPreview->setVisible(false);
        _troopPreview->setOpacity(180); // 比建筑预览更亮一点
        _troopPreview->setAnchorPoint(Vec2(0.5f, 0.0f)); // 兵种锚点在底部（贴合地面）
        _troopPreview->setScale(_mapContainer->getScale());
        _mapContainer->addChild(_troopPreview, 100); // 层级高于建筑
    }
    else {
        CCLOG("Error: Could not create _troopPreview sprite!");
    }
}
// 初始化兵种训练/放置按钮
void VillageScene::initTroopModeBtn() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 兵种模式开关按钮（在建筑按钮左侧）
    auto troopModeBtn = MenuItemImage::create(
        "ui/troop_mode_btn_normal.png",  // 正常状态图片
        "ui/troop_mode_btn_selected.png",// 按下状态图片
        [this](Ref* sender) {
            this->toggleTroopBar(); // 点击切换兵种栏
        }
    );
    troopModeBtn->setScale(0.8f);
    troopModeBtn->setPosition(Vec2(visibleSize.width - 220, visibleSize.height - 100)); // 建筑按钮左侧

    auto menu = Menu::create(troopModeBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100); // 最高层级
}
// 创建兵种栏（训练/放置按钮）
void VillageScene::createTroopBar() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 兵种栏容器层（在建筑栏下方）
    auto troopBarLayer = Layer::create();
    this->addChild(troopBarLayer, 99);
    troopBarLayer->setTag(1001); // 用Tag标记，方便后续查找

    // 兵种栏背景
    auto barBg = Sprite::create("ui/build_bar_bg.png"); // 兵种栏背景
    barBg->setPosition(Vec2(visibleSize.width / 2, 50)); // 建筑栏同位置
    barBg->setScaleX(visibleSize.width / barBg->getContentSize().width * 0.8f);
    troopBarLayer->addChild(barBg);

    // 野蛮人训练按钮
    auto barbarianBtn = MenuItemImage::create(
        "troops/barbarian.png",  // 野蛮人图标
        "troops/barbarian.png",
        [this](Ref* sender) {
            _Mode = Mode::SPAWN_TROOP;
            _selectedTroopType = TroopType::BARBARIAN;
            // 设置预览图纹理
            _troopPreview->setTexture("troops/barbarian.png");
            _troopPreview->setVisible(true);
        }
    );
    barbarianBtn->setScale(0.4f);

    //弓箭手训练按钮
    auto archerBtn = MenuItemImage::create(
        "troops/archer.png",  // archer图标
        "troops/archer.png",
        [this](Ref* sender) {
            _Mode = Mode::SPAWN_TROOP;
            _selectedTroopType = TroopType::ARCHER;
            // 设置预览图纹理
            _troopPreview->setTexture("troops/archer.png");
            _troopPreview->setVisible(true);
        }
    );
    archerBtn->setScale(0.4f);

    //bomber训练按钮
    auto bomberBtn = MenuItemImage::create(
        "troops/bomber.png",  // bomber图标
        "troops/bomber.png",
        [this](Ref* sender) {
            _Mode = Mode::SPAWN_TROOP;
            _selectedTroopType = TroopType::BOMBER;
            // 设置预览图纹理
            _troopPreview->setTexture("troops/bomber.png");
            _troopPreview->setVisible(true);
        }
    );
    bomberBtn->setScale(0.4f);

    //giant训练按钮
    auto giantBtn = MenuItemImage::create(
        "troops/giant.png",  // giant图标
        "troops/giant.png",
        [this](Ref* sender) {
            _Mode = Mode::SPAWN_TROOP;
            _selectedTroopType = TroopType::GIANT;
            // 设置预览图纹理
            _troopPreview->setTexture("troops/giant.png");
            _troopPreview->setVisible(true);
        }
    );
    giantBtn->setScale(0.4f);

    // 取消放置按钮
    auto cancelTroopBtn = MenuItemImage::create(
        "ui/cancel_place_btn.png",
        "ui/cancel_place_btn_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::NONE;
            _troopPreview->setVisible(false);
        }
    );

    // 排列按钮
    auto menu = Menu::create(barbarianBtn, archerBtn, bomberBtn, giantBtn, cancelTroopBtn, nullptr);
    menu->alignItemsHorizontallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width / 2, 50));
    troopBarLayer->addChild(menu);
}
// 切换兵种栏显示/隐藏
void VillageScene::toggleTroopBar() {
    Layer* troopBarLayer = (Layer*)this->getChildByTag(1001);
    if (!_isTroopBarShow) {
        if (!troopBarLayer) {
            this->createTroopBar();
        }
        else {
            troopBarLayer->setVisible(true);
        }
        _isTroopBarShow = true;
        // 初始化兵种预览（首次调用）
        if (!_troopPreview) {
            initTroopPreview();
        }
    }
    else {
        // 隐藏兵种栏+退出放置模式
        if (troopBarLayer) {
            troopBarLayer->setVisible(false);
        }
        _isTroopBarShow = false;
        _Mode = Mode::NONE;
        if (_troopPreview) {
            _troopPreview->setVisible(false);
        }
    }
}
// 隐藏兵种栏
void VillageScene::hideTroopBar() {
    Layer* troopBarLayer = (Layer*)this->getChildByTag(1001);
    if (troopBarLayer) {
        troopBarLayer->setVisible(false);
    }
}
// 检测瓦片是否可生成兵种（空地+未被建筑占用）
bool VillageScene::checkCanSpawnTroop(Vec2 tilePos) {
    // 1. 坐标越界检测
    if (tilePos.x < 0 || tilePos.x >= _mapSize.width
        || tilePos.y < 0 || tilePos.y >= _mapSize.height) {
        return false;
    }

    // 2. 检测是否被建筑占用
    if (isTileOccupied(tilePos)) {
        return false;
    }

    // 3. 检测瓦片是否为可通行区域（复用建筑可放置属性）
    unsigned int tileGID = _bgLayer->getTileGIDAt(tilePos);
    if (tileGID == 0) {
        return false;
    }
    ValueMap tileProps = _tileMap->getPropertiesForGID(tileGID).asValueMap();

    // 优先级：canWalk > canPlace > 默认true（确保能看到预览）
    if (tileProps.count("canWalk") > 0) {
        return tileProps["canWalk"].asBool();
    }
    else if (tileProps.count("canPlace") > 0) {
        return tileProps["canPlace"].asBool();
    }
    else {
        return true; // 无属性时默认可放置
    }
}
// 生成兵种（放置到地图）
// 生成兵种（放置到地图）
void VillageScene::spawnTroop(Vec2 screenPos, TroopType type) {
    // ===== 第一步：计算瓦片坐标 =====
    Vec2 tilePos = screenToIsoTile(screenPos);
    tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));

    // 调试日志
    CCLOG("兵种生成：屏幕坐标(%.1f,%.1f) → 瓦片坐标(%.1f,%.1f)",
        screenPos.x, screenPos.y, tilePos.x, tilePos.y);

    // ===== 第二步：可放置检测 =====
    // 注意：兵种检测应该使用checkCanSpawnTroop而不是checkCanPlace
    if (!checkCanSpawnTroop(tilePos)) {
        showCannotPlaceTip(screenPos);
        CCLOG("失败：该瓦片不可放置兵种");
        return;
    }

    // ===== 第三步：计算兵种位置（复用建筑的坐标转换逻辑） =====
    // 对于单格兵种，中心点就是瓦片中心
    float centerTileX = tilePos.x - 0.5f;  // 单格兵种，中心在瓦片中心
    float centerTileY = tilePos.y - 2.0f;
    Vec2 centerTilePos(centerTileX, centerTileY);

    // 使用与建筑相同的坐标转换方法
    Vec2 containerLocalPos = isoTileToContainerPos(centerTilePos);

    // 调试坐标转换
    CCLOG("兵种坐标转换：瓦片(%.1f,%.1f) → 容器(%.1f,%.1f)",
        centerTilePos.x, centerTilePos.y, containerLocalPos.x, containerLocalPos.y);

    // ===== 第四步：创建兵种并设置位置 =====
    BaseTroop* troop = BaseTroop::create(type, tilePos, 1.0f);
    if (!troop) {
        // 兜底创建纯色占位
        CCLOG("BaseTroop创建失败，创建纯色占位");
        auto troopSprite = LayerColor::create(Color4B(255, 150, 0, 200), 40, 60);
        if (!troopSprite) {
            CCLOG("占位都创建失败！");
            return;
        }
        troopSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        troopSprite->setPosition(containerLocalPos);
        troopSprite->setScale(1.0f); // 固定缩放
        troopSprite->setLocalZOrder(2000 - (tilePos.x + tilePos.y)); // 层级比建筑高
        _mapContainer->addChild(troopSprite);
        _spawnedTroops.push_back(nullptr);
        CCLOG("成功：纯色占位已生成");
        return;
    }

    // ===== 第五步：设置兵种属性 =====
    // 关键：兵种也需要设置锚点居中，与建筑保持一致
    troop->setAnchorPoint(Vec2(0.5f, 0.5f));
    troop->setPosition(containerLocalPos);
    troop->setScale(1.0f); // 固定缩放
    troop->setLocalZOrder(2000 - (tilePos.x + tilePos.y)); // 层级比建筑高
    _mapContainer->addChild(troop);

    // ===== 第六步：记录兵种 =====
    _spawnedTroops.push_back(troop);
    CCLOG("成功：兵种已生成，容器位置(%.1f,%.1f)，总数=%zu",
        containerLocalPos.x, containerLocalPos.y, _spawnedTroops.size());
}

// 兵种攻击回调（处理伤害结算）
void VillageScene::onTroopAttack(BaseTroop* troop, BaseBuilding* target) {
    if (!troop || !target) return;

    // 简化：建筑受到伤害（需扩展BaseBuilding的takeDamage方法）
    // target->takeDamage(troop->getConfig().attackPower);

    CCLOG("野蛮人攻击了%s，造成%d点伤害",
        target->getConfig().name.c_str(),
        troop->getConfig().attackPower);
}



//金币/圣水条
// 初始化资源显示条
void VillageScene::initResourceBar() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建资源显示层
    _resourceLayer = Layer::create();
    this->addChild(_resourceLayer, 98); // 层级低于按钮，高于地图

    // 初始化资源数值（可以从存档或服务器获取）
    _gold = 1000;      // 示例：初始金币
    _elixir = 500;     // 示例：初始圣水

    // 金币显示
    // 金币图标
    _goldIcon = Sprite::create("ui/icon_gold.png");
    if (_goldIcon) {
        _goldIcon->setScale(0.8f);
        _goldIcon->setPosition(Vec2(70, visibleSize.height - 30));
        _resourceLayer->addChild(_goldIcon, 1);
    }

    // 金币标签
    _goldLabel = Label::createWithTTF(StringUtils::format("%d", _gold),
        "fonts/Marker Felt.ttf", 24);
    if (_goldLabel) {
        _goldLabel->setAnchorPoint(Vec2(0, 0.5f));
        _goldLabel->setPosition(Vec2(90, visibleSize.height - 30));
        _goldLabel->setColor(Color3B::YELLOW);
        _goldLabel->enableOutline(Color4B::BLACK, 2); // 添加黑色描边，提高可读性
        _resourceLayer->addChild(_goldLabel, 1);
    }

    // 圣水显示（在金币右侧）
    // 圣水图标
    _elixirIcon = Sprite::create("ui/icon_elixir.png");
    if (_elixirIcon) {
        _elixirIcon->setScale(0.8f);
        _elixirIcon->setPosition(Vec2(200, visibleSize.height - 30));
        _resourceLayer->addChild(_elixirIcon, 1);
    }

    // 圣水标签
    _elixirLabel = Label::createWithTTF(StringUtils::format("%d", _elixir),
        "fonts/Marker Felt.ttf", 24);
    if (_elixirLabel) {
        _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
        _elixirLabel->setPosition(Vec2(220, visibleSize.height - 30));
        _elixirLabel->setColor(Color3B::MAGENTA); // 紫色表示圣水
        _elixirLabel->enableOutline(Color4B::BLACK, 2);
        _resourceLayer->addChild(_elixirLabel, 1);
    }



    // 可选：添加资源获取按钮（点击可打开商店等）
    auto addGoldBtn = MenuItemImage::create(
        "ui/btn_add_gold.png",
        "ui/btn_add_selected.png",
        [this](Ref* sender) {
            // 点击增加金币按钮，可以打开商店或直接增加（测试用）
            this->addGold(100);
        }
    );
    addGoldBtn->setScale(0.6f);
    addGoldBtn->setPosition(Vec2(290, visibleSize.height - 30));

    auto addElixirBtn = MenuItemImage::create(
        "ui/btn_add_elixir.png",
        "ui/btn_add_selected.png",
        [this](Ref* sender) {
            // 点击增加圣水按钮
            this->addElixir(100);
        }
    );
    addElixirBtn->setScale(0.6f);
    addElixirBtn->setPosition(Vec2(330, visibleSize.height - 30));

    auto menu = Menu::create(addGoldBtn, addElixirBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    _resourceLayer->addChild(menu, 2);
}

// 更新金币数量
void VillageScene::setGold(int gold) {
    _gold = gold;
    if (_goldLabel) {
        // 播放数字变化动画（可选）
        auto fadeOut = FadeOut::create(0.2f);
        auto fadeIn = FadeIn::create(0.2f);
        auto updateText = CallFunc::create([this]() {
            _goldLabel->setString(StringUtils::format("%d", _gold));
            });

        _goldLabel->runAction(Sequence::create(
            fadeOut,
            updateText,
            fadeIn,
            nullptr
        ));

        // 简单方式：直接更新
        // _goldLabel->setString(StringUtils::format("%d", _gold));
    }
}

// 更新圣水数量
void VillageScene::setElixir(int elixir) {
    _elixir = elixir;
    if (_elixirLabel) {
        _elixirLabel->setString(StringUtils::format("%d", _elixir));

        // 如果需要动画效果，可以像setGold那样实现
    }
}

// 增加金币（带数量检查）
bool VillageScene::addGold(int amount) {
    if (amount <= 0) return false;

    int newGold = _gold + amount;
    // 可以设置金币上限
    int maxGold = 999999; // 最大金币数
    newGold = MIN(newGold, maxGold);

    setGold(newGold);
    return true;
}

// 减少金币（带数量检查）
bool VillageScene::spendGold(int amount) {
    if (amount <= 0 || _gold < amount) {
        // 金币不足的提示
        showResourceShortageTip("金币不足!");
        return false;
    }

    setGold(_gold - amount);
    return true;
}

// 增加圣水
bool VillageScene::addElixir(int amount) {
    if (amount <= 0) return false;

    int newElixir = _elixir + amount;
    int maxElixir = 999999;
    newElixir = MIN(newElixir, maxElixir);

    setElixir(newElixir);
    return true;
}

// 减少圣水
bool VillageScene::spendElixir(int amount) {
    if (amount <= 0 || _elixir < amount) {
        showResourceShortageTip("圣水不足!");
        return false;
    }

    setElixir(_elixir - amount);
    return true;
}

// 显示资源不足提示
void VillageScene::showResourceShortageTip(const std::string& message) {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto tip = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 20);
    tip->setColor(Color3B::RED);
    tip->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    this->addChild(tip, 100);

    // 添加闪烁效果
    auto blink = Blink::create(1.0f, 3);
    auto remove = RemoveSelf::create();
    tip->runAction(Sequence::create(blink, remove, nullptr));
}
//存档相关
// 1. 打包当前场景数据为存档结构
SaveData::Village VillageScene::packSaveData() {
    SaveData::Village saveData;
    // 填充地图尺寸
    saveData.mapSize = _mapSize;
    // 填充当前模式
    saveData.currentMode = _Mode;
    // 填充已占用格子
    saveData.occupiedTiles = _occupiedTiles;
	saveData.gold = _gold;
	saveData.elixir = _elixir;
    // 填充所有建筑数据
    for (const auto& building : _buildings) {
        SaveData::Building bData;
        bData.type = building->getType();
        bData.tilePos = building->getTilePos(); 
        bData.state = building->getState();
        bData.level = building->getLevel(); 
            saveData.buildings.push_back(bData);
    }
    return saveData;
}
// 辅助：字符串分割函数
std::vector<std::string> split(const std::string& s, const std::string& delim) {
    std::vector<std::string> result;
    if (delim.empty() || s.empty()) {
        result.push_back(s); // 分隔符为空/原字符串为空，直接返回原字符串
        return result;
    }

    std::string str = s; // 拷贝原字符串避免修改输入
    size_t pos = 0;
    std::string token;

    // 循环查找分隔符并截取子串
    while ((pos = str.find(delim)) != std::string::npos) {
        token = str.substr(0, pos);
        if (!token.empty()) { // 跳过空串（避免连续分隔符导致的空元素）
            result.push_back(token);
        }
        str.erase(0, pos + delim.length()); // 移除已处理的部分
    }

    // 处理最后一段剩余的字符串
    if (!str.empty()) {
        result.push_back(str);
    }

    return result;
}
// 从存档结构恢复场景数据
void VillageScene::unpackSaveData(const SaveData::Village& saveData) {
    // 清空当前场景的旧数据
	for (auto building : _buildings) {
		if (building) {
			building->destroy();
		}
	}
    _buildings.clear();
    _goldMines.clear();
    _townHalls.clear();
    _occupiedTiles.clear();
    // 恢复地图尺寸（可选，根据需求）
    _mapSize = saveData.mapSize;
    // 恢复当前模式
    _Mode = saveData.currentMode;
    // 恢复已占用格子
    _occupiedTiles = saveData.occupiedTiles;
	// 恢复资源数值
	setGold(saveData.gold);
	setElixir(saveData.elixir);
    // 重新创建所有建筑
    for (const auto& bData : saveData.buildings) {
        // 调用placeBuilding逻辑创建建筑（复用现有代码）
        auto building = BaseBuilding::create(bData.type, bData.tilePos, 1.0f);
        if (building) {
            // 恢复建筑状态/等级
            building->setState(bData.state);    // 需给BaseBuilding添加setState方法
            building->setLevel(bData.level);    // 需给BaseBuilding添加setLevel方法

            // 复用placeBuilding中的坐标/锚点/ZOrder逻辑
            auto config = building->getConfig();
            float centerTileX = bData.tilePos.x + (config.tileWidth) / 2.0f;
            float centerTileY = bData.tilePos.y + (config.tileHeight - 1) / 2.0f;
            Vec2 centerTilePos(centerTileX, centerTileY);
            Vec2 containerLocalPos = isoTileToContainerPos(centerTilePos);
            building->setAnchorPoint(Vec2(0.5f, 0.5f));
            building->setPosition(containerLocalPos);
            building->setLocalZOrder(1000 - (bData.tilePos.x + bData.tilePos.y));

            // 重新绑定点击回调
            building->bindClickCallback([this](BaseBuilding* building) {
                if (_Mode != Mode::NONE) return;
                auto popup = BuildingPopup::create(building, [this, building](BuildingPopup::ButtonType type) {
                    handleBuildingBtnClick(building, type);
                    });
                this->addChild(popup, 100);
                });

            // 重新加入容器和分类列表
            _mapContainer->addChild(building);
            _buildings.push_back(building);
            if (bData.type == BuildingType::GOLD_MINE) {
                _goldMines.push_back(dynamic_cast<GoldMine*>(building));
            }
            else if (bData.type == BuildingType::TOWN_HALL) {
                _townHalls.push_back(dynamic_cast<TownHall*>(building));
            }
        }
    }
}

//存档到文件
bool VillageScene::saveGame(const std::string& savePath) {
    // 打包数据
    SaveData::Village saveData = packSaveData();
    std::string saveStr = saveData.toString();

    // 获取Cocos2d-x可写路径（跨平台）
    std::string fullPath = cocos2d::FileUtils::getInstance()->getWritablePath() + savePath;
    // 写入文件
    return cocos2d::FileUtils::getInstance()->writeStringToFile(saveStr, fullPath);
}

// 从文件读档
bool VillageScene::loadGame(const std::string& savePath) {
    // 获取文件完整路径
    std::string fullPath = cocos2d::FileUtils::getInstance()->getWritablePath() + savePath;
    // 检查文件是否存在
    if (!cocos2d::FileUtils::getInstance()->isFileExist(fullPath)) {
        CCLOG("存档文件不存在：%s", fullPath.c_str());
        return false;
    }
    // 读取文件内容
    std::string saveStr = cocos2d::FileUtils::getInstance()->getStringFromFile(fullPath);
    // 反序列化数据
    SaveData::Village saveData = SaveData::Village::fromString(saveStr);
    // 恢复场景数据
    unpackSaveData(saveData);
    CCLOG("读档成功：恢复了 %d 栋建筑", (int)saveData.buildings.size());
    return true;
}

// 创建存档/读档按钮
void VillageScene::initSaveLoadButtons() {
    // 创建存档按钮
    _saveBtn = ui::Button::create(
        "ui/btn_normal.png",   // 正常状态图片（替换为你的资源路径）
        "ui/btn_pressed.png",  // 按下状态图片
        "ui/btn_disabled.png"  // 禁用状态图片（可选）
    );
    if (_saveBtn) {
        // 设置按钮大小（根据UI资源调整）
        _saveBtn->setContentSize(Size(120, 60));
        // 设置按钮位置（屏幕右上角，留出边距）
        Size winSize = Director::getInstance()->getWinSize();
        _saveBtn->setPosition(Vec2(winSize.width - 140, winSize.height - 80));
        // 设置按钮文字
        auto saveText = ui::Text::create("存档", "fonts/Marker Felt.ttf", 24);
        saveText->setColor(Color3B::WHITE);
        _saveBtn->addChild(saveText);
        // 绑定点击回调
        _saveBtn->addClickEventListener(CC_CALLBACK_1(VillageScene::onSaveBtnClicked, this));
        // 添加到场景（层级高于地图，避免被遮挡）
        this->addChild(_saveBtn, 200);
    }

    // ========== 2. 创建读档按钮 ==========
    _loadBtn = ui::Button::create(
        "ui/btn_normal.png",
        "ui/btn_pressed.png",
        "ui/btn_disabled.png"
    );
    if (_loadBtn) {
        _loadBtn->setContentSize(Size(120, 60));
        // 位置在存档按钮下方，间距20
        _loadBtn->setPosition(Vec2(_saveBtn->getPositionX(), _saveBtn->getPositionY() - 80));
        // 设置按钮文字
        auto loadText = ui::Text::create("读档", "fonts/Marker Felt.ttf", 24);
        loadText->setColor(Color3B::WHITE);
        _loadBtn->addChild(loadText);
        // 绑定点击回调
        _loadBtn->addClickEventListener(CC_CALLBACK_1(VillageScene::onLoadBtnClicked, this));
        // 添加到场景
        this->addChild(_loadBtn, 200);
    }
}

//存档按钮点击回调
void VillageScene::onSaveBtnClicked(Ref* sender) {
    // 调用已实现的存档方法
    bool success = saveGame();
    // 提示玩家存档结果
    std::string tip = success ? "save success" : "save failed";
    CCLOG("%s", tip.c_str());

    // 添加弹窗提示
    auto tipLabel = Label::createWithSystemFont(tip, "Arial", 30);
    tipLabel->setColor(success ? Color3B::GREEN : Color3B::RED);
    tipLabel->setPosition(Director::getInstance()->getWinSize() / 2);
    this->addChild(tipLabel, 300);
    // 2秒后隐藏提示
    tipLabel->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}

//读档按钮点击回调
void VillageScene::onLoadBtnClicked(Ref* sender) {
    // 调用已实现的读档方法
    bool success = loadGame();
    std::string tip = success ? "load success" : "load failed";
    CCLOG("%s", tip.c_str());

    // 弹窗提示
    auto tipLabel = Label::createWithSystemFont(tip, "Arial", 30);
    tipLabel->setColor(success ? Color3B::GREEN : Color3B::RED);
    tipLabel->setPosition(Director::getInstance()->getWinSize() / 2);
    this->addChild(tipLabel, 300);
    tipLabel->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}


// 初始化关卡选择按钮
void VillageScene::initLevelSelectBtn() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建关卡选择按钮（左下角）
    auto levelSelectBtn = MenuItemImage::create(
        "ui/level_select_btn_normal.png",  // 正常状态图片
        "ui/level_select_btn_selected.png",// 按下状态图片
        [this](Ref* sender) {
            this->toggleLevelSelectMenu(); // 点击切换关卡选择菜单
        }
    );

    // 设置按钮大小和位置（左下角）
    levelSelectBtn->setScale(0.8f);
    levelSelectBtn->setPosition(Vec2(100, 100)); // 左下角，距离左边缘和下边缘各50像素

    auto menu = Menu::create(levelSelectBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100); // 最高层级

    // 初始化关卡选择层状态
    _isLevelSelectShow = false;
    _levelSelectLayer = nullptr;
}

// 切换关卡选择菜单显示/隐藏
void VillageScene::toggleLevelSelectMenu() {
    if (!_isLevelSelectShow) {
        // 显示关卡选择菜单
        showLevelSelectMenu();
        _isLevelSelectShow = true;
    }
    else {
        // 隐藏关卡选择菜单
        hideLevelSelectMenu();
        _isLevelSelectShow = false;
    }
}

// 显示关卡选择菜单
void VillageScene::showLevelSelectMenu() {
    // 如果关卡选择层不存在，则创建
    if (!_levelSelectLayer) {
        createLevelSelectMenu();
    }

    if (_levelSelectLayer) {
        _levelSelectLayer->setVisible(true);

        // 添加淡入动画
        _levelSelectLayer->setOpacity(0);
        _levelSelectLayer->runAction(FadeIn::create(0.3f));
    }
}

// 隐藏关卡选择菜单
void VillageScene::hideLevelSelectMenu() {
    if (_levelSelectLayer) {
        // 添加淡出动画
        auto fadeOut = FadeOut::create(0.2f);
        auto hide = CallFunc::create([this]() {
            if (_levelSelectLayer) {
                _levelSelectLayer->setVisible(false);
            }
            });
        _levelSelectLayer->runAction(Sequence::create(fadeOut, hide, nullptr));
    }
}

// 创建关卡选择菜单
void VillageScene::createLevelSelectMenu() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建半透明背景层
    _levelSelectLayer = Layer::create();
    _levelSelectLayer->setVisible(false);
    this->addChild(_levelSelectLayer, 99); // 层级高于其他UI

    // 添加半透明黑色背景（覆盖整个屏幕）
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180));
    bg->setContentSize(visibleSize);
    _levelSelectLayer->addChild(bg);

    // 添加背景框
    auto frame = Sprite::create("ui/menu_bg.png");
    if (!frame) {
        // 如果没有背景图片，创建一个纯色背景
        frame = Sprite::create();
        auto colorBg = LayerColor::create(Color4B(50, 50, 100, 230), 400, 400);
        colorBg->setPosition(Vec2::ZERO);
        frame->addChild(colorBg);
    }
    frame->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    frame->setScale(0.8f);
    _levelSelectLayer->addChild(frame);

    // 标题
    auto title = Label::createWithTTF("选择关卡", "fonts/Marker Felt.ttf", 36);
    title->setColor(Color3B::YELLOW);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 150));
    _levelSelectLayer->addChild(title);

    // 关卡1按钮
    auto level1Btn = MenuItemImage::create(
        "ui/level1_btn_normal.png",
        "ui/level1_btn_selected.png",
        [this](Ref* sender) {
            this->gotoLevel1();
        }
    );
    level1Btn->setPosition(Vec2(-200, -110));
    level1Btn->setScale(0.9f);

    // 关卡2按钮
    auto level2Btn = MenuItemImage::create(
        "ui/level2_btn_normal.png",
        "ui/level2_btn_selected.png",
        [this](Ref* sender) {
            this->gotoLevel2();
        }
    );
    level2Btn->setPosition(Vec2(0, -110));
    level2Btn->setScale(0.9f);

    // 关卡3按钮
    auto level3Btn = MenuItemImage::create(
        "ui/level3_btn_normal.png",
        "ui/level3_btn_selected.png",
        [this](Ref* sender) {
            this->gotoLevel3();
        }
    );
    level3Btn->setPosition(Vec2(200, -110));
    level3Btn->setScale(0.9f);

    // 关闭按钮
    auto closeBtn = MenuItemImage::create(
        "ui/close_btn_normal.png",
        "ui/close_btn_selected.png",
        [this](Ref* sender) {
            this->hideLevelSelectMenu();
        }
    );
    closeBtn->setPosition(Vec2(200,0));
    closeBtn->setScale(0.8f);

    // 创建菜单
    auto menu = Menu::create(level1Btn, level2Btn, level3Btn, closeBtn, nullptr);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _levelSelectLayer->addChild(menu);

    // 如果图片不存在，创建文字标签作为备选
    if (!level1Btn->getNormalImage()) {
        auto label1 = Label::createWithTTF("关卡 1: 新手训练", "fonts/Marker Felt.ttf", 28);
        label1->setColor(Color3B::WHITE);
        label1->setPosition(Vec2(visibleSize.width / 2-200, visibleSize.height / 2 ));
        _levelSelectLayer->addChild(label1);

        auto label2 = Label::createWithTTF("关卡 2: 丛林之战", "fonts/Marker Felt.ttf", 28);
        label2->setColor(Color3B::WHITE);
        label2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 ));
        _levelSelectLayer->addChild(label2);

        auto label3 = Label::createWithTTF("关卡 3: 最终决战", "fonts/Marker Felt.ttf", 28);
        label3->setColor(Color3B::WHITE);
        label3->setPosition(Vec2(visibleSize.width / 2+200, visibleSize.height / 2 ));
        _levelSelectLayer->addChild(label3);
    }
}

auto myScene = Scene::create();



// 跳转到关卡1
void VillageScene::gotoLevel1() {
    hideLevelSelectMenu();

    // 创建场景切换过渡效果
    auto transition = TransitionFade::create(1.0f, myScene);
    Director::getInstance()->replaceScene(transition);
}

// 跳转到关卡2
void VillageScene::gotoLevel2() {
    hideLevelSelectMenu();

    // 创建场景切换过渡效果
    auto transition = TransitionFade::create(1.0f, myScene);
    Director::getInstance()->replaceScene(transition);
}

// 跳转到关卡3
void VillageScene::gotoLevel3() {
    hideLevelSelectMenu();

    // 创建场景切换过渡效果
    auto transition = TransitionFade::create(1.0f, myScene);
    Director::getInstance()->replaceScene(transition);
}

