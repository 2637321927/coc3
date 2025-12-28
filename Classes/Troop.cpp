#include "Troop.h"
#include "Barbarian.h"
#include "Archer.h"
#include "Giant.h"
#include "Bomber.h"
#include "Building.h"
#include "cocos2d.h"
#include "VillageScene.h"
#include "PathFinder.h" 
#include <cmath>
USING_NS_CC;
BaseTroop::~BaseTroop() {
    // 安全释放对目标的持有，防止内存泄漏
    CC_SAFE_RELEASE(_attackTarget);
}
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
    _attackRange = config.attackRange;
    _attackPower = config.attackPower;
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
// Troop.cpp

// 【新增】实现设置瓦片目标的方法
void BaseTroop::setTargetTilePosition(const Vec2& targetTilePos) {
    if (!_villageScene) return;

    // 1. 获取起点（瓦片坐标）
    Vec2 startTile = getCurrentTilePos();

    // 2. 记录终点（瓦片坐标）
    _targetPos = targetTilePos;

    // 3. 直接调用 A* 寻路（不做任何坐标转换！）
    _pathPoints = PathFinder::findPath(
        startTile,
        _targetPos, // 终点
        _villageScene->getPathLayer(),
        _villageScene->getMapSize(),
        _villageScene->getOccupiedTiles()
    );

    _currentPathIndex = 0;

    // Log调试：看看现在能不能算出来路径
    CCLOG("寻路请求: 起点(%.0f,%.0f) -> 终点(%.0f,%.0f) | 路径点数: %zu",
        startTile.x, startTile.y, _targetPos.x, _targetPos.y, _pathPoints.size());

    if (!_pathPoints.empty()) {
        setState(TroopState::MOVING);
    }
    else {
        setState(TroopState::IDLE);
    }
}

// 【修改】修正 updateMovement 逻辑
void BaseTroop::updateMovement(float dt) {
    if (_state != TroopState::MOVING) return;
    if (!_villageScene) return;

    // 1. 攻击范围检测
    // _targetPos 现在存的是瓦片坐标，必须转成容器像素坐标才能计算距离
    Vec2 targetPixelPos = _villageScene->isoTileToContainerPosPublic(_targetPos);

    // 计算兵种和目标的像素距离
    float distToTarget = this->getPosition().distance(targetPixelPos);

    // 判断是否在攻击范围内 (减去30是简单的半径修正，避免兵种完全重叠到建筑中心)
    if (distToTarget <= _attackRange) {
        setState(TroopState::ATTACKING);
        _pathPoints.clear();
        return;
    }

    // 2. 沿路径移动
    if (_currentPathIndex < _pathPoints.size()) {
        // 获取下一个路径点（瓦片坐标）
        Vec2 nextTile = _pathPoints[_currentPathIndex];

        // 将下一个瓦片转为像素坐标
        Vec2 nextWaypoint = _villageScene->isoTileToContainerPosPublic(nextTile);

        Vec2 direction = nextWaypoint - this->getPosition();
        float distToWaypoint = direction.length();
        float moveStep = _config.moveSpeed * dt * _mapScale;

        if (distToWaypoint < moveStep) {
            // 到达当前节点，吸附并前往下一个
            this->setPosition(nextWaypoint);
            _currentPathIndex++;
        }
        else {
            // 向当前节点移动
            direction.normalize();
            this->setPosition(this->getPosition() + direction * moveStep);
        }
    }
    else {
        // 路径走完了（通常意味着到了目标旁边）
        setState(TroopState::ATTACKING);
    }
}
// ========== 通用：帧更新（核心逻辑） ==========
void BaseTroop::update(float dt) {
    Sprite::update(dt);
    if (_state == TroopState::ATTACKING && _attackTarget == nullptr) {
        setState(TroopState::IDLE);
        return;
    }
    if (_state == TroopState::ATTACKING) {
        if (!_attackTarget) {
            setState(TroopState::IDLE);
            return;
        }
        // 如果能获取状态，且状态是 DESTROYED，也停止
        // 注意：如果上面 destroyBuilding 没写好，这里访问 getState() 就会崩
        if (_attackTarget->getState() == BuildingState::DESTROYED) {
            setState(TroopState::IDLE);
            _attackTarget = nullptr;
            return;
        }
    }
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
        // 1. 基础判空
        if (!_attackTarget) {
            setState(TroopState::IDLE);
            return;
        }

        // 2. [关键检查] 检查目标是否已经“死亡”
        // 如果建筑被 VillageScene::destroyBuilding 移除了，它的 parent 会变成 nullptr
        // 但因为我们 retain 了它，内存是安全的，这里可以放心访问
        if (_attackTarget->getParent() == nullptr || _attackTarget->getState() == BuildingState::DESTROYED) {
            setAttackTarget(nullptr); // 释放引用，此时建筑内存才真正被销毁
            setState(TroopState::IDLE);
            return;
        }

        updateAttackCD();

        // 3. 执行攻击
        if (_attackCDTimer >= _config.attackSpeed) {
            doSpecialAttack();

            // 造成伤害
            // 即使这一击导致建筑死亡，由于我们 retain 了，下面的代码依然安全
            _attackTarget->takeDamage(this->_attackPower);

            // 再次检查目标状态（防止刚才那一下把它打死了）
            if (_attackTarget && (_attackTarget->getParent() == nullptr || _attackTarget->getState() == BuildingState::DESTROYED)) {
                setAttackTarget(nullptr);
                setState(TroopState::IDLE);
                return;
            }

            _attackCDTimer = 0.1f;
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

void BaseTroop::setTargetWorldPosition(const Vec2& targetPos) {
    if (!_villageScene) return;

    // 转换：因为 targetPos 可能是屏幕触摸点（世界坐标）
    // 而兵种是在 _mapContainer 里的，计算距离最好统一到 mapContainer 坐标系
    // 这里假设 targetPos 已经是 mapContainer 坐标（因为它是从 Building->getPosition() 获取的）
    _targetPos = targetPos;

    Vec2 startTile = getCurrentTilePos();
    Vec2 targetTile = _villageScene->screenToIsoTilePublic(targetPos);

    _pathPoints = PathFinder::findPath(
        startTile,
        targetTile,
        _villageScene->getPathLayer(),
        _villageScene->getMapSize(),
        _villageScene->getOccupiedTiles()
    );

    _currentPathIndex = 0;

    // 只有找到路了才切状态，或者没找到路但是已经在射程内了
    if (!_pathPoints.empty()) {
        setState(TroopState::MOVING);
    }
    else {
        // 如果找不到路，但就在旁边，可能直接判定攻击？
        // 暂时先 IDLE
        setState(TroopState::IDLE);
    }
}

void BaseTroop::setAttackTarget(BaseBuilding* target) {
    // 1. 如果目标没变，直接返回
    if (_attackTarget == target) return;

    // 2. 释放旧目标（如果存在，引用计数-1）
    // 如果旧目标引用计数降为0，它会在此时被销毁，这是安全的
    CC_SAFE_RELEASE(_attackTarget);

    // 3. 设置新目标
    _attackTarget = target;

    // 4. 持有新目标（引用计数+1）
    // 关键：这保证了只要 _attackTarget 不为空，由于兵种“抓”着它，它绝不会在别处被彻底删除
    CC_SAFE_RETAIN(_attackTarget);

    // 5. 只有目标非空时才寻路
    if (_attackTarget) {
        setTargetTilePosition(_attackTarget->getTilePos());
    }
}
