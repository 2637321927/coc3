// LevelScene.cpp
#include "LevelScene.h"
#include "VillageScene.h"
#include "cocos2d.h"

USING_NS_CC;

bool LevelScene::init() {
    if (!Scene::init()) return false;

    // 初始化默认值
    _minScale = 0.5f;
    _maxScale = 2.0f;
    _scaleStep = 0.1f;
    _isDragging = false;
    _starsEarned = 0;

    return true;
}

void LevelScene::onEnter() {
    Scene::onEnter();

    // 初始化触摸事件
    initTouchEvents();
}

bool LevelScene::initLevel(int levelNumber, const std::string& mapFile) {
    _currentLevel = levelNumber;

    // 初始化地图容器
    _mapContainer = Node::create();
    this->addChild(_mapContainer);

    // 加载地图
    initMap(mapFile);

    // 初始化UI
    initUI();

    // 开始关卡
    startLevel();

    return true;
}

void LevelScene::initMap(const std::string& mapFile) {
    _tileMap = TMXTiledMap::create(mapFile);
    if (!_tileMap) {
        CCLOG("Failed to load map: %s", mapFile.c_str());
        return;
    }

    _mapContainer->addChild(_tileMap, 0);

    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();
    _bgLayer = _tileMap->getLayer("bg_layer");
    _pathLayer = _tileMap->getLayer("path_layer");

    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 地图居中
    _mapContainer->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.0f));
    _tileMap->setPosition(Vec2::ZERO);
}

void LevelScene::initUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 返回按钮
    auto _returnBtn = MenuItemImage::create("ui/return_btn_normal.png",
        "ui/return_btn_selected.png");


    // 关卡标签
    _levelLabel = Label::createWithTTF(StringUtils::format("关卡 %d", _currentLevel),
        "fonts/Marker Felt.ttf", 32);
    _levelLabel->setColor(Color3B::YELLOW);
    _levelLabel->setPosition(Vec2(100, visibleSize.height - 50));
    this->addChild(_levelLabel, 100);

    // 星星显示
    auto star1 = Sprite::create("ui/star_empty.png");
    star1->setPosition(Vec2(visibleSize.width / 2 - 60, visibleSize.height - 50));
    this->addChild(star1, 100);

    auto star2 = Sprite::create("ui/star_empty.png");
    star2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 50));
    this->addChild(star2, 100);

    auto star3 = Sprite::create("ui/star_empty.png");
    star3->setPosition(Vec2(visibleSize.width / 2 + 60, visibleSize.height - 50));
    this->addChild(star3, 100);
}

void LevelScene::initTouchEvents() {
    auto mouseListener = EventListenerMouse::create();

    mouseListener->onMouseScroll = CC_CALLBACK_1(LevelScene::onMouseScroll, this);
    mouseListener->onMouseDown = CC_CALLBACK_1(LevelScene::onMouseDown, this);
    mouseListener->onMouseMove = CC_CALLBACK_1(LevelScene::onMouseMove, this);
    mouseListener->onMouseUp = CC_CALLBACK_1(LevelScene::onMouseUp, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

void LevelScene::onMouseScroll(Event* event) {
    EventMouse* e = (EventMouse*)event;
    float currentScale = _mapContainer->getScale();
    float newScale = (e->getScrollY() < 0) ?
        MIN(currentScale + _scaleStep, _maxScale) :
        MAX(currentScale - _scaleStep, _minScale);

    Vec2 mousePos = Vec2(e->getCursorX(), e->getCursorY());
    Vec2 containerPos = _mapContainer->getPosition();

    Vec2 offset = mousePos - containerPos;
    Vec2 newPos = mousePos - offset * (newScale / currentScale);

    _mapContainer->setScale(newScale);
    _mapContainer->setPosition(newPos);
}

void LevelScene::onMouseDown(Event* event) {
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        _isDragging = true;
        _lastMousePos = Vec2(e->getCursorX(), e->getCursorY());
        _mapOriginPos = _mapContainer->getPosition();
    }
}

void LevelScene::onMouseMove(Event* event) {
    EventMouse* e = (EventMouse*)event;
    Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());

    if (_isDragging) {
        Vec2 offset = currentMousePos - _lastMousePos;
        _mapContainer->setPosition(_mapOriginPos + offset);
        clampMapPosition();
    }
}

void LevelScene::onMouseUp(Event* event) {
    EventMouse* e = (EventMouse*)event;
    if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
        _isDragging = false;
    }
}

void LevelScene::clampMapPosition() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 containerPos = _mapContainer->getPosition();
    float containerScale = _mapContainer->getScale();

    float mapWidth = _tileMap->getContentSize().width * containerScale;
    float mapHeight = _tileMap->getContentSize().height * containerScale;

    float minX = visibleSize.width - mapWidth / 2;
    float maxX = mapWidth / 2;
    float minY = visibleSize.height - mapHeight / 2;
    float maxY = mapHeight / 2;

    float clampedX = clampf(containerPos.x, minX, maxX);
    float clampedY = clampf(containerPos.y, minY, maxY);
    _mapContainer->setPosition(Vec2(clampedX, clampedY));
}

Vec2 LevelScene::screenToIsoTile(Vec2 screenPos) {
    Vec2 localPos = _tileMap->convertToNodeSpace(screenPos);

    float tw = _tileSize.width;
    float th = _tileSize.height;
    float mw = _mapSize.width;
    float mh = _mapSize.height;

    float isox = localPos.x / tw;
    float isoy = localPos.y / th;
    float originShift = mw / 2.0f;

    int tileX = (int)(mh - isoy + (isox - originShift));
    int tileY = (int)(mh - isoy - (isox - originShift));

    return Vec2(tileX, tileY);
}

Vec2 LevelScene::isoTileToScreen(Vec2 tilePos) {
    const float tileW = _tileSize.width;
    const float tileH = _tileSize.height;
    const float mapHalfW = _mapSize.width * tileW / 2.0f;

    float tileMapLocalX = (tilePos.x - tilePos.y) * (tileW / 2.0f) + mapHalfW;
    float tileMapLocalY = (tilePos.x + tilePos.y) * (tileH / 2.0f);
    Vec2 tileCenterInTileMap(tileMapLocalX, tileMapLocalY);

    Vec2 tileMapWorldPos = _tileMap->convertToWorldSpaceAR(tileCenterInTileMap);

    return tileMapWorldPos;
}

void LevelScene::returnToVillage() {
    auto transition = TransitionFade::create(1.0f, VillageScene::createScene());
    Director::getInstance()->replaceScene(transition);
}

void LevelScene::showLevelInfo() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto infoBg = LayerColor::create(Color4B(0, 0, 0, 200), 300, 200);
    infoBg->setPosition(Vec2(visibleSize.width / 2 - 150, visibleSize.height / 2 - 100));
    this->addChild(infoBg, 101);

    auto title = Label::createWithTTF(StringUtils::format("关卡 %d 信息", _currentLevel),
        "fonts/Marker Felt.ttf", 28);
    title->setColor(Color3B::YELLOW);
    title->setPosition(Vec2(150, 150));
    infoBg->addChild(title);

    auto closeBtn = MenuItemImage::create("ui/close_btn_normal.png",
        "ui/close_btn_selected.png");

}

void LevelScene::startLevel() {
    // 子类重写此函数
    CCLOG("Level %d started", _currentLevel);
}

void LevelScene::checkLevelComplete() {
    // 子类重写此函数
}

void LevelScene::onLevelComplete(bool success) {
    // 子类重写此函数
}

Scene* LevelScene::createScene() {
    auto scene = Scene::create();
    auto layer = LevelScene::create();
    scene->addChild(layer);
    return scene;
}