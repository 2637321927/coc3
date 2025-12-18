#include "Building.h"
#include "cocos2d.h"
USING_NS_CC;

// 根据类型创建子类实例
//直接在Building.h中声明，在Building.cpp中实现

BaseBuilding* BaseBuilding::create(BuildingType type, const Vec2& tilePos, float mapScale) {
    BaseBuilding* building = nullptr;
    // 按类型创建子类（后续新增建筑只需加case，无需改基类）
    switch (type) {
    case BuildingType::GOLD_MINE:
        // 后续实现GOLD_MINE后替换
        // building = GoldMineBuilding::create(tilePos, mapScale);
        break;
    case BuildingType::TOWN_HALL:
        // building = TownHallBuilding::create(tilePos, mapScale);
        break;
    case BuildingType::BARRACKS:
        // building = BarracksBuilding::create(tilePos, mapScale);
        break;
    default:
        break;
    }
    return building;
}

// 通用初始化
bool BaseBuilding::init(const BuildingConfig& config, const Vec2& tilePos, float mapScale) {
    if (!Sprite::initWithFile(config.imgPath)) {
        return false;
    }

    // 通用数据初始化
    _config = config;
    _tilePos = tilePos;
    _mapScale = mapScale;
    this->setScale(mapScale);

    // 通用UI初始化
    initCommonUI();

    // 通用点击事件
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = [this](Touch* touch, Event* event) {
        if (this->getBoundingBox().containsPoint(this->convertTouchToNodeSpace(touch))) {
            if (_clickCallback) _clickCallback(this);
            return true;
        }
        return false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 默认状态
    setState(BuildingState::IDLE);
    return true;
}

// 初始化通用UI（进度条、等级标签）
void BaseBuilding::initCommonUI() {
    // 建造/升级进度条（通用）
    _progressBar = ProgressTimer::create(Sprite::create("ui/progress_bar.png"));
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5f));
    _progressBar->setBarChangeRate(Vec2(1, 0));
    _progressBar->setPosition(this->getContentSize().width / 2, -20);
    _progressBar->setScale(0.8f * _mapScale);
    _progressBar->setVisible(false);
    this->addChild(_progressBar, 1);

    // 等级标签（通用）
    auto levelLabel = Label::createWithTTF("Lv" + std::to_string(_config.level), "fonts/Marker Felt.ttf", 16);
    levelLabel->setPosition(this->getContentSize().width - 15, this->getContentSize().height - 15);
    levelLabel->setColor(Color3B::YELLOW);
    this->addChild(levelLabel, 1);
}

// 通用：开始建造
void BaseBuilding::startBuild() {
    setState(BuildingState::BUILDING);
    _progressTimer = 0.0f;
    _progressBar->setVisible(true);
    _progressBar->setPercentage(0);
    this->scheduleUpdate();
}

// 通用：完成建造
void BaseBuilding::finishBuild() {
    setState(BuildingState::IDLE);
    _progressBar->setVisible(false);
    this->unscheduleUpdate();
}

// 通用：开始升级
void BaseBuilding::startUpgrade() {
    setState(BuildingState::UPGRADING);
    _progressTimer = 0.0f;
    _progressBar->setVisible(true);
    _progressBar->setPercentage(0);
    this->scheduleUpdate();
}

// 通用：摧毁建筑
void BaseBuilding::destroy() {
    setState(BuildingState::DESTROYED);
    this->setOpacity(100);
    this->unscheduleUpdate();
}

// 通用：设置状态（视觉表现）
void BaseBuilding::setState(BuildingState state) {
    _state = state;
    switch (state) {
    case BuildingState::BUILDING: this->setColor(Color3B::GRAY); break;
    case BuildingState::IDLE: this->setColor(Color3B::WHITE); break;
    case BuildingState::UPGRADING: this->setColor(Color3B::BLUE); break;
    case BuildingState::DESTROYED: this->setColor(Color3B::BLACK); break;
    default: break;
    }
}

// 通用：同步地图缩放
void BaseBuilding::syncScale(float mapScale) {
    _mapScale = mapScale;
    this->setScale(mapScale);
    if (_progressBar) _progressBar->setScale(0.8f * mapScale);
}

// 通用：绑定点击回调
void BaseBuilding::bindClickCallback(const std::function<void(BaseBuilding*)>& callback) {
    _clickCallback = callback;
}

// 通用：帧更新（进度处理）
void BaseBuilding::update(float dt) {
    if (_state != BuildingState::BUILDING && _state != BuildingState::UPGRADING) return;

    _progressTimer += dt;
    float progress = _progressTimer / _config.buildTime;
    progress = clampf(progress, 0.0f, 1.0f);
    _progressBar->setPercentage(progress * 100);

    if (progress >= 1.0f) {
        _state == BuildingState::BUILDING ? finishBuild() : finishBuild();
    }
}

// 通用：更新进度（子类可调用）
void BaseBuilding::updateProgress() {
    float progress = _progressTimer / _config.buildTime;
    _progressBar->setPercentage(clampf(progress, 0.0f, 1.0f) * 100);
}