#include "Troop.h"
#include "Barbarian.h"
#include "Archer.h"
#include "Giant.h"
#include "Bomber.h"
#include "Building.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "PathFinder.h" 
USING_NS_CC;

// ========== 工厂方法：根据类型创建子类实例 ==========
BaseTroop* BaseTroop::create(TroopType type, const Vec2& spawnPos, float mapScale) {
    BaseTroop* troop = nullptr;
    // 按类型创建子类（后续新增兵种只需加case，无需改基类）
    switch (type) {
    case TroopType::BARBARIAN:
        troop = BarbarianTroop::create(spawnPos, mapScale);
        break;
    case TroopType::ARCHER:
        troop = ArcherTroop::create(spawnPos, mapScale);
        break;
    case TroopType::GIANT:
        troop = GiantTroop::create(spawnPos, mapScale);
        break;
    case TroopType::BOMBER:
        troop = BomberTroop::create(spawnPos, mapScale);
        break;
    default:
        break;
    }
    return troop;
}

// ========== 通用初始化 ==========
bool BaseTroop::init(const TroopConfig& config, const Vec2& spawnPos, float mapScale) {
    if (!Sprite::initWithFile(config.imgPath)) {
        return false;
    }

    // 通用数据初始化
    _config = config;
    _spawnPos = spawnPos;
    _mapScale = mapScale;
    _currentHp = config.hp; // 初始化当前血量为满值
    this->setScale(mapScale);
    this->setPosition(spawnPos);

    // 通用UI初始化（血条、等级标签）
    initCommonUI();

    // 默认状态
    setState(TroopState::IDLE);
    this->scheduleUpdate();
    return true;
}

// ========== 初始化通用UI（血条、等级标签） ==========
void BaseTroop::initCommonUI() {
    // 血量条（通用）
    _hpBar = ProgressTimer::create(Sprite::create("ui/hp_bar.png"));
    _hpBar->setType(ProgressTimer::Type::BAR);
    _hpBar->setMidpoint(Vec2(0, 0.5f));
    _hpBar->setBarChangeRate(Vec2(1, 0));
    _hpBar->setPosition(this->getContentSize().width / 2, this->getContentSize().height + 10);
    _hpBar->setScale(0.8f * _mapScale);
    _hpBar->setPercentage(100); // 初始满血
    this->addChild(_hpBar, 1);

    // 等级标签（通用）
    auto levelLabel = Label::createWithTTF("Lv" + std::to_string(_config.level), "fonts/Marker Felt.ttf", 14);
    levelLabel->setPosition(this->getContentSize().width - 12, this->getContentSize().height - 12);
    levelLabel->setColor(Color3B::YELLOW);
    this->addChild(levelLabel, 1);
}

// ========== 通用状态管理：开始训练 ==========
void BaseTroop::startTrain() {
    setState(TroopState::TRAINING);
    _trainingTimer = 0.0f;
    this->scheduleUpdate();
}

// ========== 通用状态管理：完成训练 ==========
void BaseTroop::finishTrain() {
    setState(TroopState::IDLE);
    this->unscheduleUpdate();
}

// ========== 通用状态管理：开始攻击 ==========
void BaseTroop::startAttack(BaseBuilding* target) {
    if (_state == TroopState::DEAD) return; // 死亡状态无法攻击

    setState(TroopState::ATTACKING);
    _attackTarget = target;
    _attackCDTimer = 0.0f; // 重置攻击冷却
    this->scheduleUpdate();
}

// ========== 通用状态管理：受到伤害 ==========
void BaseTroop::takeDamage(int damage) {
    if (_state == TroopState::DEAD) return; // 死亡状态不受伤害

    _currentHp -= damage;
    _currentHp = std::max(0, _currentHp); // 血量不低于0

    // 更新血条显示
    float hpPercent = static_cast<float>(_currentHp) / _config.hp * 100;
    _hpBar->setPercentage(hpPercent);

    // 血量为0则死亡
    if (_currentHp <= 0) {
        die();
    }
}

// ========== 通用状态管理：死亡 ==========
void BaseTroop::die() {
    setState(TroopState::DEAD);
    this->setOpacity(100); // 半透明表示死亡
    _hpBar->setVisible(false); // 隐藏血条
    this->unscheduleUpdate();
    auto villageScene = VillageScene::getInstance();
    if (villageScene) {
        villageScene->removeEnemyTroop(this);
    }
    // 死亡动画（可在子类中重写增强）
    auto fadeOut = FadeOut::create(1.0f);
    auto remove = RemoveSelf::create();
    this->runAction(Sequence::create(fadeOut, remove, nullptr));
}

// ========== 通用状态管理：设置状态 ==========
void BaseTroop::setState(TroopState state) {
    _state = state;
    // 不同状态的视觉表现
    switch (state) {
    case TroopState::IDLE:
        this->setColor(Color3B::WHITE);
        break;
    case TroopState::MOVING:
        this->setColor(Color3B::GREEN);
        break;
    case TroopState::ATTACKING:
        this->setColor(Color3B::RED);
        break;
    case TroopState::DEAD:
        this->setColor(Color3B::GRAY);
        break;
    case TroopState::TRAINING:
        this->setColor(Color3B::BLUE);
        break;
    default:
        break;
    }
}

// ========== 通用：同步地图缩放 ==========
void BaseTroop::syncScale(float mapScale) {
    _mapScale = mapScale;
    this->setScale(mapScale);
    if (_hpBar) _hpBar->setScale(0.8f * mapScale);
}

// ========== 通用：绑定攻击回调 ==========
void BaseTroop::bindAttackCallback(const std::function<void(BaseTroop*, BaseBuilding*)>& callback) {
    _attackCallback = callback;
}

// ========== 通用：更新攻击冷却 ==========
void BaseTroop::updateAttackCD() {
    if (_attackCDTimer < _config.attackSpeed) {
        _attackCDTimer += Director::getInstance()->getDeltaTime();
    }
}

// ========== 通用：更新移动逻辑 ==========
void BaseTroop::updateMovement(float dt) {
    if (_state != TroopState::MOVING) {
        CCLOG("NOMOVE1");
        return;
    }

    // 如果路径为空，尝试重新计算路径
    if (_pathPoints.empty()) {
        // 复用setTargetWorldPosition中的路径计算逻辑
        if (!_villageScene) {
            CCLOG("nopath");
            return;
        }

        // 1. 坐标转换（通过公有接口）
        Vec2 startTile = getCurrentTilePos();
        Vec2 targetTile = _villageScene->screenToIsoTilePublic(_targetPos);

        // 2. 调用 PathFinder 寻路
        _pathPoints = PathFinder::findPath(
            startTile,
            targetTile,
            _villageScene->getPathLayer(),
            _villageScene->getMapSize(),
            _villageScene->getOccupiedTiles()
        );

        // 3. 初始化路径索引
        _currentPathIndex = 0;
        CCLOG("重新计算路径：起点(%.1f,%.1f) 终点(%.1f,%.1f) 路径点数量:%zu",
            startTile.x, startTile.y, targetTile.x, targetTile.y, _pathPoints.size());

        // 若路径仍为空，停止移动
        if (_pathPoints.empty()) {
            setState(TroopState::IDLE);
            CCLOG("路径为空，停止移动");
            return;
        }
    }

    // 处理路径点移动
    Vec2 currentTarget = _villageScene->isoTileToContainerPosPublic(_pathPoints[_currentPathIndex]);
    Vec2 direction = currentTarget - this->getPosition();
    float distance = direction.length();

    // 到达当前路径点
    if (distance < _config.moveSpeed * dt * _mapScale) {
        this->setPosition(currentTarget);
        _currentPathIndex++;

        // 检查是否到达最终目标
        if (_currentPathIndex >= _pathPoints.size()) {
            // 到达终点，切换状态
            setState(TroopState::IDLE);
            _pathPoints.clear();
            CCLOG("到达最终目标");
            return;
        }
    }
    else {
        // 向当前路径点移动
        direction.normalize();
        Vec2 moveStep = direction * _config.moveSpeed * dt * _mapScale;
        this->setPosition(this->getPosition() + moveStep);
    }
}

// ========== 通用：帧更新（核心逻辑） ==========
void BaseTroop::update(float dt) {
    Sprite::update(dt);
    setState(TroopState::MOVING);
    // 根据不同状态处理逻辑
    switch (_state) {
    case TroopState::TRAINING: {
        // 训练进度处理
        _trainingTimer += dt;
        float trainProgress = _trainingTimer / _config.trainingTime;
        if (trainProgress >= 1.0f) {
            finishTrain();
        }
        break;
    }
    case TroopState::MOVING: {
        // 移动逻辑

        updateMovement(dt);
        break;
    }
    case TroopState::ATTACKING: {
        // 攻击冷却与攻击逻辑
        updateAttackCD();

        // 冷却完成且有目标则发起攻击
        if (_attackCDTimer >= _config.attackSpeed && _attackTarget != nullptr) {
            // 调用特有攻击行为
            doSpecialAttack();

            // 触发攻击回调（外部处理伤害结算）
            if (_attackCallback) {
                _attackCallback(this, _attackTarget);
            }

            // 重置冷却
            _attackCDTimer = 0.0f;
        }
        break;
    }
    default:
        break;
    }
}

// ========== 寻路逻辑 ========== 
// 获取当前瓦片坐标（使用公有接口）
Vec2 BaseTroop::getCurrentTilePos() const {
    if (!_villageScene) return Vec2::ZERO;
    // 调用 VillageScene 提供的公有转换接口
    return _villageScene->screenToIsoTilePublic(this->convertToWorldSpace(Vec2::ZERO));
}

// 设置目标并寻路（使用公有接口）
void BaseTroop::setTargetWorldPosition(const Vec2& targetPos) {
    CCLOG("setTargetWorldPosition entered"); // 入口日志
    if (!_villageScene) {
        return;  // 确保场景指针有效 
    }
        // 1. 坐标转换（通过公有接口）
        Vec2 startTile = getCurrentTilePos();
        Vec2 targetTile = _villageScene->screenToIsoTilePublic(targetPos);
        // 2. 调用 PathFinder 寻路（通过公有接口获取地图数据）
        _pathPoints = PathFinder::findPath(
            startTile,
            targetTile,
            _villageScene->getPathLayer(),        // 公有接口获取路径层
            _villageScene->getMapSize(),          // 公有接口获取地图尺寸
            _villageScene->getOccupiedTiles()     // 公有接口获取占用瓦片
        );

        // 3. 初始化路径索引
        _currentPathIndex = 0;
        _targetPos = targetPos;
        CCLOG("Troop：(%.1f,%.1f)(%.1f,%.1f),%z",
            startTile.x, startTile.y, targetTile.x, targetTile.y, _pathPoints.size());
        if (!_pathPoints.empty()) {
            setState(TroopState::MOVING);
        }
    }
