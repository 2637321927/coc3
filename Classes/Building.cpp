#include "Building.h"
#include "cocos2d.h"
#include "Troop.h"
#include "ui/CocosGUI.h" 
USING_NS_CC;

// 根据类型创建子类实例
//直接在Building.h中声明，在Building.cpp中实现
BaseBuilding* BaseBuilding::create(BuildingType type, const Vec2& tilePos, float mapScale) {
    BaseBuilding* building = nullptr;
    // 按类型创建子类（后续新增建筑只需加case，无需改基类）
    switch (type) {
    case BuildingType::GOLD_MINE:
        // 后续实现GOLD_MINE后替换
        building = GoldMine::create(tilePos, mapScale);
        break;
    case BuildingType::TOWN_HALL:
        building = TownHall::create(tilePos, mapScale);
        break;
	case BuildingType::ELIXIR_COLLECTOR:
		building = ElixirCollector::create(tilePos, mapScale);
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
    // 只初始化Node，不再初始化Sprite
    if (!Node::init()) return false;

    // 保存核心配置
    _config = config;
    _tilePos = tilePos;
    _mapScale = mapScale;

    // 加载建筑图片（仅创建一次Sprite子节点，避免重复）
    if (!loadBuildingSprite()) {
        return false;
    }
    // 设置节点缩放（基于地图缩放比例）
    this->setScale(mapScale);

    // 初始化通用UI（进度条、等级标签）
    initCommonUI();

    // 初始化触摸监听器（统一调用封装的方法，避免重复绑定）
    initTouchListener();

    //if普通模式
    //startBuild()
    // 默认状态初始化
    setState(BuildingState::IDLE);
	doSpecialAction(); // 金矿开始生产金币
    return true;
}
bool BaseBuilding::loadBuildingSprite() {
    // 检查路径是否为空
    if (_config.imgPath.empty()) {
        return false;
    }

    // 加载纹理（优先用缓存，避免重复加载）
    Texture2D* texture = Director::getInstance()->getTextureCache()->addImage(_config.imgPath);
    if (!texture) {
        return false;
    }

    // 创建图片精灵（作为子节点）
    _buildingSprite = Sprite::createWithTexture(texture);
    if (!_buildingSprite) {
        return false;
    }

    // 设置精灵锚点和位置（居中在BaseBuilding节点）
    _buildingSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    _buildingSprite->setPosition(Vec2::ZERO); // Node默认锚点是(0,0)，精灵居中则设为(0,0)
    this->addChild(_buildingSprite, -1); // Z=-1：保证在UI下方

    // 同步精灵缩放（和建筑节点一致）
    _buildingSprite->setScale(_mapScale);

    return true;
}

// 初始化通用UI（进度条、等级标签）
void BaseBuilding::initCommonUI() {
    //建造/升级进度条
    auto progressBg = Sprite::create("ui/progress_bar.png");
    if (!progressBg) {
        return;
    }
    _progressBar = ProgressTimer::create(progressBg);
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5f));
    _progressBar->setBarChangeRate(Vec2(1, 0));
    // 进度条位置：建筑下方20像素（基于精灵尺寸）
    _progressBar->setPosition(0, -_buildingSprite->getContentSize().height / 2 - 20);
    _progressBar->setScale(0.8f); // 基础缩放，syncScale会叠加地图缩放
    _progressBar->setVisible(false);
    this->addChild(_progressBar, 1);

    // 2. 等级标签
    auto levelLabel = Label::createWithTTF("Lv" + std::to_string(_config.level), "fonts/Marker Felt.ttf", 16);
    if (levelLabel) {
        // 标签位置：建筑右上角（基于精灵尺寸）
        levelLabel->setPosition(_buildingSprite->getContentSize().width / 2 - 15, _buildingSprite->getContentSize().height / 2 - 15);
        levelLabel->setColor(Color3B::YELLOW);
        this->addChild(levelLabel, 1);
    }
}
// 初始化触摸监听器
void BaseBuilding::initTouchListener() {
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);

    // 触摸开始：判断是否点击到建筑范围内
    _touchListener->onTouchBegan = [this](Touch* touch, Event* event) -> bool {
        if (!_buildingSprite) return false; // 无图片则不响应
        // 转换触摸坐标到建筑节点本地
        Vec2 touchPos = this->convertTouchToNodeSpace(touch);
        // 基于精灵的包围盒判断（更精准）
        Rect spriteRect = _buildingSprite->getBoundingBox();
        if (spriteRect.containsPoint(touchPos)) {
            return true; // 接收后续触摸事件
        }
        return false;
        };

    // 触摸结束：触发点击回调
    _touchListener->onTouchEnded = [this](Touch* touch, Event* event) {
        if (_clickCallback) {
            _clickCallback(this);
        }
        };

    // 添加监听器到事件分发器
    Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);
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
    if (_buildFinishCallback) {
        _buildFinishCallback(this);
    }
}
// 通用：绑定回调
void BaseBuilding::bindBuildFinishCallback(const std::function<void(BaseBuilding*)>& callback) {
    _buildFinishCallback = callback;
}
// 通用：开始升级
void BaseBuilding::startUpgrade() {
    setState(BuildingState::UPGRADING);
    _progressTimer = 0.0f;
    _progressBar->setVisible(true);
    _progressBar->setPercentage(0);
    this->scheduleUpdate();
    if (_buildFinishCallback) {
        _buildFinishCallback(this);
    }
}

// 通用：摧毁建筑
void BaseBuilding::destroy() {
    setState(BuildingState::DESTROYED);
    //设置为半透明
    this->setOpacity(100);
	//停止所有进度（如建造，升级）
    this->unscheduleUpdate();
	// 移除触摸监听器，禁止交互
    if (_touchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(_touchListener);
    }
    this->removeFromParentAndCleanup(true);
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

// GoldMine 子类实现
GoldMine* GoldMine::create(const Vec2& tilePos, float mapScale) {
    GoldMine* sprite = new (std::nothrow) GoldMine();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool GoldMine::init(const Vec2& tilePos, float mapScale) {
    BuildingConfig config;
    config.type = BuildingType::GOLD_MINE;
    config.name = "金矿";
    config.imgPath = "building/55.png"; // 确保路径正确
    config.hp = 500;
    config.tileWidth = 2;  // 占地 2x2
    config.tileHeight = 2;
    config.buildTime = 10.0f; // 10秒建完
    config.cost = { {"gold", 100}, {"elixir", 50} }; // 建造消耗
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;

    _state = BuildingState::IDLE;
    return true;
}

void GoldMine::doSpecialAction() {
    // 逻辑：每隔一段时间增加玩家金币
        // 非闲置/未摧毁状态才生产
    if (getState() != BuildingState::IDLE ) {
        return;
    }

    // 停止已有生产定时器，避免重复
    this->unschedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold));
    // 每2秒生产一次金币
    this->schedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold), _produceInterval);
}
// 金币生产具体逻辑
void GoldMine::produceGold(float dt) {
    // 升级后提升产量（可根据config.level动态调整）
    _goldPerInterval = 10 * getConfig().level;
	_goldStored += _goldPerInterval;
}
// 收集金币逻辑
int GoldMine::collectGold() {
    int collect = _goldStored;
    //金币超容量（大本营容量）未考虑，后续添加
    _goldStored = 0;
	return collect;
}
// 金矿专属描述
std::string GoldMine::getSpecialDesc()  {
    return "生产金币的建筑，等级越高产量越高";
}
// 覆盖摧毁方法
void GoldMine::destroy() {
    BaseBuilding::destroy(); // 父类设置状态为 DESTROYED
    this->unschedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold)); // 停止生产
}
//ElixirCollector 子类实现
ElixirCollector* ElixirCollector::create(const Vec2& tilePos, float mapScale) {
	ElixirCollector* sprite = new (std::nothrow) ElixirCollector();
	if (sprite && sprite->init(tilePos, mapScale)) {
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}
bool ElixirCollector::init(const Vec2& tilePos, float mapScale) {
	BuildingConfig config;
	config.type = BuildingType::ELIXIR_COLLECTOR;
	config.name = "圣水收集器";
	config.imgPath = "building/elixir_collector.png"; // 确保路径正确
	config.hp = 500;
	config.tileWidth = 2;  // 占地 2x2
	config.tileHeight = 2;
	config.buildTime = 10.0f; // 10秒建完
	config.cost = { {"gold", 100}, {"elixir", 50} }; // 建造消耗
	if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
	_state = BuildingState::IDLE;
	return true;
}
void ElixirCollector::doSpecialAction() {
	// 逻辑：每隔一段时间增加玩家圣水
		// 非闲置/未摧毁状态才生产
	if (getState() != BuildingState::IDLE) {
		return;
	}
	// 停止已有生产定时器，避免重复
	this->unschedule(CC_SCHEDULE_SELECTOR(ElixirCollector::produceElixir));
	// 每2秒生产一次圣水
	this->schedule(CC_SCHEDULE_SELECTOR(ElixirCollector::produceElixir), _produceInterval);
}
// 圣水生产具体逻辑
void ElixirCollector::produceElixir(float dt) {
	// 升级后提升产量（可根据config.level动态调整）
	_elixirPerInterval = 10 * getConfig().level;
	_elixirStored += _elixirPerInterval;
}
// 收集圣水逻辑
int ElixirCollector::collectElixir() {
	int collect = _elixirStored;
	//圣水超容量（大本营容量）未考虑，后续添加
	_elixirStored = 0;
	return collect;
}
// 圣水收集器专属描述
std::string ElixirCollector::getSpecialDesc() {
	return "生产圣水的建筑，等级越高产量越高";
}
// 覆盖摧毁方法
void ElixirCollector::destroy() {
	BaseBuilding::destroy(); // 父类设置状态为 DESTROYED
	this->unschedule(CC_SCHEDULE_SELECTOR(ElixirCollector::produceElixir)); // 停止生产
}
//Training
Barracks* Barracks::create(const cocos2d::Vec2& tilePos, float mapScale) {
    auto camp = new (std::nothrow) Barracks();
    if (camp && camp->init(tilePos, mapScale)) {
        camp->autorelease();
        return camp;
    }
    CC_SAFE_DELETE(camp);
    return nullptr;
}

bool Barracks::init(const cocos2d::Vec2& tilePos, float mapScale) {
    // 初始化兵营配置
    BuildingConfig config;
    config.type = BuildingType::TRAINING_CAMP;
    config.name = "兵营";
    config.imgPath = "building/camp.png";
    config.hp = 800;
    config.tileWidth = 2;
    config.tileHeight = 2;
    config.cost = { {"gold", 800}, {"elixir", 300} };
    config.buildTime = 20.0f;
    config.level = 1;
    _maxTroopSpace = 20; // 基础容量20
    if (!BaseBuilding::init(config, tilePos, mapScale)) {
        return false;
    }
    return true;
}

void Barracks::doSpecialAction() {
    // 兵营特殊行为：无持续行为，空实现
}

std::string Barracks::getSpecialDesc() {
    return StringUtils::format("驻军容量：%d，等级越高容量越大", _maxTroopSpace);
}
// TownHall 子类实现
TownHall* TownHall::create(const Vec2& tilePos, float mapScale) {
    TownHall* sprite = new (std::nothrow) TownHall();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool TownHall::init(const Vec2& tilePos, float mapScale) {
    BuildingConfig config;
    config.type = BuildingType::TOWN_HALL;
    config.name = "大本营";
    config.imgPath = "building/town_hall.png";
    config.hp = 2000;
    config.tileWidth = 3;  // 占地 3x3
    config.tileHeight = 3;
    config.buildTime = 30.0f;

    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;

    _state = BuildingState::IDLE;
    return true;
}

void TownHall::doSpecialAction() {
    CCLOG("大本营管理中心已就绪");
}

