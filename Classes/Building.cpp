#include "Building.h"
#include "cocos2d.h"
#include "ui/CocosGUI.h" 
#include "VillageScene.h"
#include "EffectManager.h"
USING_NS_CC;

// 根据类型创建子类实例
//直接在Building.h中声明，在Building.cpp中实现
BaseBuilding* BaseBuilding::create(BuildingType type, const Vec2& tilePos, float mapScale) {
    BaseBuilding* building = nullptr;
    // 按类型创建子类（后续新增建筑只需加case，无需改基类）
    switch (type) {
    case BuildingType::GOLD_MINE:
        building = GoldMine::create(tilePos, mapScale);
        break;
    case BuildingType::TOWN_HALL:
        building = TownHall::create(tilePos, mapScale);
        break;
    case BuildingType::ELIXIR_COLLECTOR:
        building = ElixirCollector::create(tilePos, mapScale);
        break;
    case BuildingType::BARRACKS:
        building = Barracks::create(tilePos, mapScale);
        break;
    case BuildingType::TRAINING_CAMP:
        building = TrainingCamp::create(tilePos, mapScale);
        break;
    case BuildingType::CANNON:
        building = Cannon::create(tilePos, mapScale);
        break;
    case BuildingType::ARROW_TOWER:
        building = ArrowTower::create(tilePos, mapScale);
        break;
    case BuildingType::VAULT:
        building = Vault::create(tilePos, mapScale);
        break;
    case BuildingType::WALL:
        building = Wall::create(tilePos, mapScale);
        break;
    case BuildingType::ELIXIR_BOTTLE:
        building = ElixirBottle::create(tilePos, mapScale);
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
    //_tilePos = tilePos;
    for (int x = 0;x < config.tileWidth;x++) {
        for (int y = 0;y < config.tileHeight;y++) {
            _tilePos.push_back(Vec2(tilePos.x + x, tilePos.y + y));
        }
    }
    _mapScale = mapScale;
    _currentHp = config.hp; // 初始化当前血量为满值
    CCLOG("tile.x: %f, tile.y: %f", tilePos.x, tilePos.y);
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

    startBuild();
    this->scheduleUpdate();
    // setState(BuildingState::IDLE);
    doSpecialAction();
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
    _buildingSprite->setPosition(Vec2::ZERO); // Node默认锚点是(0,0)，精灵居中则设为(0,0
    // _buildingSprite->setContentSize(Size(64*_config.tileWidth,32*_config.tileHeight));//统一设置尺寸TODO：后续调试
    this->addChild(_buildingSprite, -1); // Z=-1：保证在UI下方

    // 同步精灵缩放（和建筑节点一致）
    _buildingSprite->setScale(_mapScale);

    return true;
}


// 更新建筑精灵图片
void BaseBuilding::updateBuildingSprite() {

}

// 刷新建筑属性
void BaseBuilding::refreshBuildingAttributes() {
    _config = getBuildingConfigByType(_config.type, _currentLevel);
    _currentHp = _config.hp;
    switch (_config.type) {
    case(BuildingType::ARROW_TOWER): {
        auto it1 = dynamic_cast<ArrowTower*>(this);
        it1->upGrade();
        break;
    }
    case(BuildingType::CANNON): {
        auto it2 = dynamic_cast<Cannon*>(this);
        it2->upGrade();
        break;
    }
    case(BuildingType::BARRACKS): {
        auto it3 = dynamic_cast<Barracks*>(this);
        it3->upGrade();
        break;
    }
    case(BuildingType::VAULT): {
        auto it4 = dynamic_cast<Vault*>(this);
        it4->upGrade();
        break;
    }
    case(BuildingType::ELIXIR_BOTTLE): {
        auto it5 = dynamic_cast<ElixirBottle*>(this);
        it5->upGrade();
        break;
    }
    case(BuildingType::GOLD_MINE): {
        auto it6 = dynamic_cast<GoldMine*>(this);
        it6->upGrade();
        break;
    }
    case(BuildingType::ELIXIR_COLLECTOR): {
        auto it7 = dynamic_cast<ElixirCollector*>(this);
        it7->upGrade();
        break;
    }
    }

}


// 初始化通用UI（进度条、等级标签，血条）
void BaseBuilding::initCommonUI() {
    //建造/升级进度条
    auto progressBg = Sprite::create("ui/progress_bar.png");

    if (!progressBg) {
        return;
    }

    auto hpBg = Sprite::create("ui/hp_bar.png");
    if (!hpBg) {
        return;
    }
    // 血量条位置：建筑上方20像素（基于精灵尺寸）
    _hpBar = ProgressTimer::create(hpBg);
    _hpBar->setType(ProgressTimer::Type::BAR);
    _hpBar->setMidpoint(Vec2(0, 0.5f));
    _hpBar->setBarChangeRate(Vec2(1, 0));
    _hpBar->setPosition(0, -_buildingSprite->getContentSize().height / 2 + 20);
    _hpBar->setVisible(false);
    this->addChild(_hpBar, 100);
    _progressBar = ProgressTimer::create(progressBg);
    _progressBar->setType(ProgressTimer::Type::BAR);
    _progressBar->setMidpoint(Vec2(0, 0.5f));
    _progressBar->setBarChangeRate(Vec2(1, 0));
    // 进度条位置：建筑下方20像素（基于精灵尺寸）
    _progressBar->setPosition(0, -_buildingSprite->getContentSize().height / 2 - 20);
    _progressBar->setScale(0.8f); // 基础缩放，syncScale会叠加地图缩放
    _progressBar->setVisible(false);
    this->addChild(_progressBar, 1);

    // 等级标签
    _levelLabel = Label::createWithTTF("Lv" + std::to_string(_currentLevel), "fonts/Marker Felt.ttf", 16);
    if (_levelLabel) {
        // 标签位置：建筑右上角（基于精灵尺寸）
        _levelLabel->setPosition(_buildingSprite->getContentSize().width / 2 - 15, _buildingSprite->getContentSize().height / 2 - 15);
        _levelLabel->setColor(Color3B::YELLOW);
        this->addChild(_levelLabel, 1);
    }
}
void BaseBuilding::updateLevelLabel() {
    if (_levelLabel) {
        _levelLabel->setString("Lv" + std::to_string(_currentLevel));
        return;
    }
    else {
        _levelLabel = Label::createWithTTF("Lv" + std::to_string(_currentLevel), "fonts/Marker Felt.ttf", 16);
        if (_levelLabel) {
            _levelLabel->setPosition(_buildingSprite->getContentSize().width / 2 - 15, _buildingSprite->getContentSize().height / 2 - 15);
            _levelLabel->setColor(Color3B::YELLOW);
            this->addChild(_levelLabel, 1);
        }
    }
}
void BaseBuilding::takeDamage(int damage) {
    _hpBar->setVisible(true);
    _currentHp -= damage;
    // 更新血条等UI...
    // 显示受击特效
    //EffectManager::getInstance()->playHitEffect(this->getPosition());
    // 检查是否摧毁
    if (_currentHp <= 0) {
        _currentHp = 0;
        _hpBar->setPercentage(0);
        // 调用场景的销毁方法
        VillageScene::getInstance()->destroyBuilding(this);

        // [致命警告] 销毁后绝对不要访问成员变量！
        // 错误示例： this->runAction(...);      <-- 这会导致崩溃

        return; // 必须立刻结束函数
    }
    float hpPercent = (static_cast<float>(_currentHp) / static_cast<float>(_config.hp)) * 100.0f;
    if (_hpBar) {
        _hpBar->setPercentage(hpPercent);
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
    /*if (VillageScene::getInstance()->getBaseMode() == BaseMode::CREATING) {
        setState(BuildingState::IDLE);
        this->scheduleUpdate();//便于哪些建筑依赖于帧更新执行本职工作
        return;
    }*/
    setState(BuildingState::BUILDING);
    _progressTimer = 0.0f;
    _progressBar->setVisible(true);
    _progressBar->setPercentage(0);
    this->scheduleUpdate();
}

// 通用：完成建造
void BaseBuilding::finishBuild() {
    setState(BuildingState::IDLE);
    doSpecialAction(); // 执行特有行为
    _progressBar->setVisible(false);
    //依赖于帧更新执行本职工作的建筑不停止帧更新
    if (_config.type != BuildingType::TRAINING_CAMP && _config.type != BuildingType::CANNON && _config.type != BuildingType::ARROW_TOWER)
        this->unscheduleUpdate();

    if (_buildFinishCallback) {
        _buildFinishCallback(this);
    }
}
// 开始升级
void BaseBuilding::startUpgrade() {
    setState(BuildingState::UPGRADING);
    _progressTimer = 0.0f;
    _progressBar->setVisible(true);
    _progressBar->setPercentage(0);

    // 启动升级倒计时（每帧更新剩余时间）
    this->scheduleUpdate();
}

// 升级完成
void BaseBuilding::finishUpgrade() {
    // 提升等级
    _currentLevel += 1;
    // 恢复闲置状态
    // 刷新属性（生命值/产量/容量等）
    refreshBuildingAttributes();
    // 刷新图片
    updateBuildingSprite();
    updateLevelLabel();
    // 提示升级完成
    setState(BuildingState::IDLE);
    doSpecialAction(); // 执行特有行为
    if (_config.type != BuildingType::TRAINING_CAMP && _config.type != BuildingType::CANNON && _config.type != BuildingType::ARROW_TOWER)
        this->unscheduleUpdate();
    _progressBar->setVisible(false);
    if (_buildFinishCallback) {
        _buildFinishCallback(this);
    }
}

// 通用：绑定回调
void BaseBuilding::bindBuildFinishCallback(const std::function<void(BaseBuilding*)>& callback) {
    _buildFinishCallback = callback;
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

// 通用：帧更新（进度处理,主建造和升级）子类如果有额外的帧更新逻辑（如加农炮的攻击逻辑），只需重写 update，并在开头调用 BaseBuilding::update(dt) 即可复用父类进度逻辑，本身重写自父类Node
void BaseBuilding::update(float dt) {
    if (_state != BuildingState::BUILDING && _state != BuildingState::UPGRADING) return;
    _progressTimer += dt;
    float progress = _progressTimer / _config.buildTime;
    //TODO:升级要有升级时间
    progress = clampf(progress, 0.0f, 1.0f);
    _progressBar->setPercentage(progress * 100);
    if (immediatelyBuild) {
        progress = 1.0f;
        _progressBar->setPercentage(progress * 100);
    }
    if (progress >= 1.0f) {
        _state == BuildingState::BUILDING ? finishBuild() : finishUpgrade();
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
    BuildingConfig config = getBuildingConfigByType(BuildingType::GOLD_MINE);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    return true;
}

void GoldMine::doSpecialAction() {
    // 逻辑：每隔一段时间增加玩家金币
        // 非闲置/未摧毁状态才生产
    if (getState() != BuildingState::IDLE) {
        return;
    }

    // 停止已有生产定时器，避免重复
    this->unschedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold));
    // 每2秒生产一次金币
    this->schedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold), _produceInterval);
}
// 金币生产具体逻辑
void GoldMine::produceGold(float dt) {
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
std::string GoldMine::getSpecialDesc() {
    return "生产金币的建筑，等级越高产量越高";
}
// 覆盖摧毁方法
void GoldMine::destroy() {
    this->unschedule(CC_SCHEDULE_SELECTOR(GoldMine::produceGold)); // 停止生产
    BaseBuilding::destroy(); // 父类设置状态为 DESTROYED
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
    this->unschedule(CC_SCHEDULE_SELECTOR(ElixirCollector::produceElixir)); // 停止生产
    BaseBuilding::destroy(); // 父类设置状态为 DESTROYED
}
//Barracks
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
    BuildingConfig config = getBuildingConfigByType(BuildingType::BARRACKS);
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
    BuildingConfig config = getBuildingConfigByType(BuildingType::TOWN_HALL);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;

    return true;
}

void TownHall::doSpecialAction() {
}
//TrainingCamp 子类实现
TrainingCamp* TrainingCamp::create(const Vec2& tilePos, float mapScale) {
    TrainingCamp* sprite = new (std::nothrow) TrainingCamp();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

// 初始化兵种训练时间配置
void TrainingCamp::initTroopTrainTimeConfig() {
    // 配置不同兵种的基础训练时间
    _troopTrainTimeMap = {
        {TroopType::BARBARIAN, 2.0f},    
        {TroopType::ARCHER, 3.0f},     
        {TroopType::GIANT, 8.0f},     
        {TroopType::BOMBER,2.0f}
        // 可扩展更多兵种
    };
}


// 初始化训练营
bool TrainingCamp::init(const Vec2& tilePos, float mapScale) {
    BuildingConfig config = getBuildingConfigByType(BuildingType::TRAINING_CAMP);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;

    // 初始化兵种训练时间配置
    initTroopTrainTimeConfig();


    return true;
}

// 添加训练任务到队列
void TrainingCamp::addTrainTask(TroopType type) {
    // 仅在空闲/训练中状态下可添加任务
    if (_state == BuildingState::DESTROYED || _state == BuildingState::BUILDING || _state == BuildingState::UPGRADING) {
        return;
    }
    // 获取该兵种的基础训练时间（根据训练营等级缩放）
    float baseTime = _troopTrainTimeMap[type];
    //float scaledTime = baseTime * (1.0f - 0.1f * (_config.level - 1)); // 等级越高，训练越快（每级减少10%）
   // scaledTime = std::max(scaledTime, baseTime * 0.5f); // 最低不低于基础时间的50%
    // 添加到队列
    _trainQueue.push_back(type);
    _queueTimers.push_back(baseTime);

    // 更新状态为训练中
    if (_state != BuildingState::TRAINING) {
        setState(BuildingState::TRAINING);
    }
}

// 移除指定位置的训练任务
void TrainingCamp::removeTrainTask(int index) {
    if (index < 0 || index >= _trainQueue.size()) {
        return;
    }

    // 从队列中移除
    _trainQueue.erase(_trainQueue.begin() + index);
    _queueTimers.erase(_queueTimers.begin() + index);

    // 如果队列为空，恢复空闲状态
    if (_trainQueue.empty()) {
        setState(BuildingState::IDLE);
        _trainTimer = 0.0f;
    }

    CCLOG("移除训练队列第%d个任务，剩余任务数：%zu", index, _trainQueue.size());
}

// 训练完成逻辑
void TrainingCamp::finishTrainTroop(TroopType type) {
    // 训练完成回调（可扩展：通知兵营添加士兵） { // 检查回调是否已注册
      if (_trainFinishCallback) { // 检查回调是否注册
           _trainFinishCallback(type); // 只传1个参数：兵种类型
      }; // 传递「兵种+数量」
    CCLOG("兵种%d训练完成！", (int)type);
    _troopsInTraining--;

    // 移除队列第一个任务
    if (!_trainQueue.empty()) {
        _trainQueue.erase(_trainQueue.begin());
        _queueTimers.erase(_queueTimers.begin());
    }

    // 如果队列还有任务，继续训练下一个；否则恢复空闲
    if (_trainQueue.empty()) {
        setState(BuildingState::IDLE);
        _trainTimer = 0.0f;
    }
    else {
        _trainTimer = 0.0f; // 重置计时器，开始下一个训练
    }
}

// 重写更新逻辑（每帧处理训练计时）
void TrainingCamp::update(float dt) {
    BaseBuilding::update(dt); // 调用父类更新逻辑（进度条等）

    // 仅在训练中状态处理计时
    if (_state != BuildingState::TRAINING || _trainQueue.empty()) {
        return;
    }

    // 处理第一个队列任务的计时
    _trainTimer += dt;
    float& currentTaskTime = _queueTimers[0];
    _queueTimers[0] -= dt;
    // 检查是否训练完成
    if (_trainTimer >= currentTaskTime) {
        finishTrainTroop(_trainQueue[0]);
    }

    // 可选：更新训练进度UI（可扩展）
    float progress = _trainTimer / currentTaskTime;
}

// 训练营特殊行为（核心训练逻辑）
void TrainingCamp::doSpecialAction() {
    // 仅在空闲/训练中状态生效
    if (_state == BuildingState::DESTROYED || _state == BuildingState::BUILDING || _state == BuildingState::UPGRADING) {
        return;
    }

    // 如果有训练队列，自动进入训练状态
    if (!_trainQueue.empty() && _state != BuildingState::TRAINING) {
        setState(BuildingState::TRAINING);
    }
}

// 训练营特殊描述
std::string TrainingCamp::getSpecialDesc() {
    return StringUtils::format("训练士兵的建筑，等级%d，训练速度提升%d%%",
        _currentLevel, (int)((_currentLevel - 1) * 10));
}

// 重写销毁逻辑
void TrainingCamp::destroy() {
    BaseBuilding::destroy(); // 调用父类销毁逻辑
    // 清空训练队列
    _trainQueue.clear();
    _queueTimers.clear();
    _trainTimer = 0.0f;
    _troopsInTraining = 0;
}

//Wall 子类实现
Wall* Wall::create(const cocos2d::Vec2& tilePos, float mapScale) {
    Wall* sprite = new (std::nothrow) Wall();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}
bool Wall::init(const cocos2d::Vec2& tilePos, float mapScale) {
    BuildingConfig config = getBuildingConfigByType(BuildingType::WALL);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    _state = BuildingState::IDLE;
    return true;
}
void Wall::doSpecialAction() {}

// Vault 子类实现
Vault* Vault::create(const cocos2d::Vec2& tilePos, float mapScale) {
    Vault* sprite = new (std::nothrow) Vault();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}
bool Vault::init(const cocos2d::Vec2& tilePos, float mapScale) {
    BuildingConfig config = getBuildingConfigByType(BuildingType::VAULT);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    return true;
}
void Vault::doSpecialAction() {}

// ElixirBottle 子类实现
ElixirBottle* ElixirBottle::create(const cocos2d::Vec2& tilePos, float mapScale) {
    ElixirBottle* sprite = new (std::nothrow) ElixirBottle();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}
bool ElixirBottle::init(const cocos2d::Vec2& tilePos, float mapScale) {
    BuildingConfig config = getBuildingConfigByType(BuildingType::ELIXIR_BOTTLE);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    return true;
}
void ElixirBottle::doSpecialAction() {}


//攻击型建筑基类

// 初始化攻击属性
void BaseAttackBuilding::initAttackProps(float range, float damage, float cooldown, const std::string& effectPath) {
    _attackRange = range;
    _attackDamage = damage;
    _attackCooldown = cooldown;
    _attackEffectPath = effectPath;
    _attackEffectDuration = 0.5f; // 特效时长默认0.5秒
    _currentCooldown = 0.0f;      // 冷却初始化为0
    _targetTroop = nullptr;       // 目标初始化为空
}

// 每帧更新（核心攻击逻辑）
void BaseAttackBuilding::update(float dt) {
    BaseBuilding::update(dt); // 调用父类更新（血量、建造进度等）

    // 销毁/建造/升级中，停止攻击
    if (_state == BuildingState::DESTROYED || _state == BuildingState::BUILDING || _state == BuildingState::UPGRADING) {
        _targetTroop = nullptr;
        return;
    }

    // 冷却计时更新
    if (_currentCooldown > 0) {
        _currentCooldown -= dt;
        _currentCooldown = std::max(_currentCooldown, 0.0f);
    }

    // 无目标时检测范围内敌方兵种
    if (!_targetTroop || _targetTroop && _targetTroop->getState() == TroopState::DEAD) {
        _targetTroop = findTargetInRange();
        // 有目标=ATTACKING，无目标=IDLE
        _state = _targetTroop ? BuildingState::ATTACKING : BuildingState::IDLE;
        if (!_targetTroop) return;
    }

    // 冷却完成且有目标，执行攻击
    if (_currentCooldown <= 0 && _targetTroop) {
        attackTarget();
        _currentCooldown = _attackCooldown; // 重置冷却
    }
}

// 目标检测逻辑
// 在 Building.cpp 中找到这个函数
BaseTroop* BaseAttackBuilding::findTargetInRange() {
    auto villageScene = VillageScene::getInstance();
    if (!villageScene) {
        return nullptr;
    }
    std::vector<BaseTroop*> allEnemies = villageScene->getAllEnemyTroops();
    if (allEnemies.empty()) {
        return nullptr;
    }

    BaseTroop* closestTroop = nullptr;
    float minDistance = FLT_MAX;
    Vec2 buildingPos = this->getPosition();

    for (auto troop : allEnemies) {
        if (!troop || troop->getState() == TroopState::DEAD) continue;

        float distance = buildingPos.distance(troop->getPosition());
        if (distance <= _attackRange && distance < minDistance) {
            minDistance = distance;
            closestTroop = troop;
        }
    }
    return closestTroop;
}

// 攻击范围可视化（点击攻击类建筑显示）
void BaseAttackBuilding::showAttackRange(bool isShow) {
    // 防护：如果攻击范围非法，直接返回（避免drawCircle崩溃）
    if (_attackRange <= 0) {
        CCLOG("警告：攻击范围非法（%.2f），无法显示攻击范围", _attackRange);
        return;
    }

    if (isShow) {
        if (!_rangeDraw) {
            _rangeDraw = DrawNode::create();
            if (!_rangeDraw) { // 防护：创建失败直接返回
                CCLOG("创建 DrawNode 失败！");
                return;
            }
            // 挂载到当前建筑节点，层级设为-1（在建筑图片下方）
            this->addChild(_rangeDraw, -1);
            // 关键：设置DrawNode的锚点和位置，确保圆圈中心和建筑重合
            _rangeDraw->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
            _rangeDraw->setPosition(this->getContentSize() / 2); // 对齐建筑中心
        }

        // 清空原有绘制内容，重新绘制攻击范围圆圈
        _rangeDraw->clear();
        // 参数说明：圆心、半径、角度、分段数、是否闭合、颜色（红半透）
        _rangeDraw->drawCircle(
            Vec2::ZERO,          // 相对于_rangeDraw自身的圆心（已对齐建筑中心）
            _attackRange,        // 攻击范围半径
            CC_DEGREES_TO_RADIANS(360), // 完整圆圈（弧度）
            60,                  // 分段数（越多越平滑）
            false,               // 不绘制扇形
            Color4F(1, 0, 0, 0.3f) // 红色半透明
        );
    }
    else {
        // 隐藏/销毁攻击范围
        if (_rangeDraw) {
            _rangeDraw->clear();          // 清空绘制内容
            _rangeDraw->removeFromParentAndCleanup(true); // 移除并释放内存
            _rangeDraw = nullptr;        // 置空，避免野指针
        }
    }
}

// 重写销毁逻辑（清空目标）
void BaseAttackBuilding::destroy() {
    _targetTroop = nullptr;
    _currentCooldown = 0.0f;
    if (_rangeDraw) {
        _rangeDraw->clear();
        _rangeDraw->removeFromParentAndCleanup(true);
        _rangeDraw = nullptr;
    }
    BaseBuilding::destroy();
}

//加农炮类
Cannon* Cannon::create(const Vec2& tilePos, float mapScale) {
    Cannon* sprite = new (std::nothrow) Cannon();
    if (sprite && sprite->init(tilePos, mapScale)) {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

// 初始化
bool Cannon::init(const Vec2& tilePos, float mapScale) {
    // 初始化建筑基础属性（和训练营一致）
    BuildingConfig config = getBuildingConfigByType(BuildingType::CANNON);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    // 初始化攻击属性（复用 BaseAttackBuilding 的接口）
    float range = 100;
    float damage = 100;
    float cooldown = 1.0f;
    //cooldown = std::max(cooldown, 1.0f); // 最低冷却1秒
    initAttackProps(range, damage, cooldown, "effect/cannon_ball.png");


    // 调试：显示攻击范围
    showAttackRange(false);

    return true;
}

// 加农炮攻击逻辑（炮弹）
void Cannon::attackTarget() {
    if (!_targetTroop) return;

    // 扣减目标血量
    _targetTroop->takeDamage(_attackDamage);
    CCLOG("attatsdack!");

    // 播放炮弹特效
    Vec2 startPos = this->getPosition();
    Vec2 endPos = _targetTroop->getPosition();
    EffectManager::getInstance()->playProjectileEffect(
        _attackEffectPath,
        startPos,
        endPos,
        _attackEffectDuration
    );
}

// 特殊描述（和训练营格式一致）
std::string Cannon::getSpecialDesc() {
    return StringUtils::format("近战攻击建筑，等级%d，攻击范围%.0f像素，伤害%.0f，攻速%.1f秒/次",
        _currentLevel, _attackRange, _attackDamage, _attackCooldown);
}
void Cannon::doSpecialAction() {};

ArrowTower* ArrowTower::create(const Vec2& tilePos, float mapScale) {
    ArrowTower* tower = new (std::nothrow) ArrowTower();
    if (tower && tower->init(tilePos, mapScale)) {
        tower->autorelease();
        return tower;
    }
    CC_SAFE_DELETE(tower);
    return nullptr;
}

bool ArrowTower::init(const Vec2& tilePos, float mapScale) {
    // 基础建筑属性初始化
    BuildingConfig config = getBuildingConfigByType(BuildingType::ARROW_TOWER);
    if (!BaseBuilding::init(config, tilePos, mapScale)) return false;
    //攻击属性初始化
    float range = 150;
    float damage = 30;
    float cooldown = 0.8f;
    cooldown = std::max(cooldown, 0.4f);
    initAttackProps(range, damage, cooldown, "effect/arrow.png");



    showAttackRange(false);

    return true;
}

void ArrowTower::attackTarget() {
    if (!_targetTroop) return;

    // 扣血
    _targetTroop->takeDamage(_attackDamage);

    // 箭矢特效（发射口稍高）
    Vec2 startPos = this->getPosition() + Vec2(0, 50);
    Vec2 endPos = _targetTroop->getPosition();
    EffectManager::getInstance()->playProjectileEffect(
        _attackEffectPath,
        startPos,
        endPos,
        _attackEffectDuration
    );
}

std::string ArrowTower::getSpecialDesc() {
    return StringUtils::format("远程攻击建筑，等级%d，攻击范围%.0f像素，伤害%.0f，攻速%.1f秒/次",
        _currentLevel, _attackRange, _attackDamage, _attackCooldown);
}
void ArrowTower::doSpecialAction() {};