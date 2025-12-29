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
// [Troop.cpp]
void BaseTroop::setTargetTilePosition(const Vec2& targetTilePos) {
    if (!_villageScene) return;

    Vec2 startTile = getCurrentTilePos();
    _targetPos = targetTilePos;

    // 调用 A* 寻路
    _pathPoints = PathFinder::findPath(
        startTile,
        _targetPos,
        _villageScene->getPathLayer(),
        _villageScene->getMapSize(),
        _villageScene->getOccupiedTiles()
    );

    _currentPathIndex = 0;

    if (!_pathPoints.empty()) {
        // 找到了路径，开始移动
        setState(TroopState::MOVING);
    }
    else {
        // 【修改】如果没有路径（可能是就在旁边，也可能是不可达）
        // 计算物理距离检查是否已经在攻击范围内
        Vec2 myPos = this->getPosition();
        Vec2 targetPixelPos = _villageScene->isoTileToContainerPosPublic(_targetPos);
        float dist = myPos.distance(targetPixelPos);

        // 注意：这里加一个缓冲距离（比如40像素），因为attackRange可能很小
        // 如果真的很近，直接攻击；否则待机（防止隔空攻击）
        if (dist <= _attackRange + 30.0f) { // 30.0f是预估的半个瓦片像素宽，容错用
            setState(TroopState::ATTACKING);
        }
        else {
            // 找不到路，又离得远 -> 呆着别动
            findNewTarget();
            CCLOG("无法到达目标，且不在攻击范围内，待机。");
        }
    }
}

// [Troop.cpp]
void BaseTroop::updateMovement(float dt) {
    if (_state != TroopState::MOVING) return;
    if (!_villageScene) return;

    // 1. 攻击范围检测 (保持原样)
    Vec2 targetPixelPos = _villageScene->isoTileToContainerPosPublic(_targetPos);
    float distToTarget = this->getPosition().distance(targetPixelPos);

    if (distToTarget <= _attackRange) {
        setState(TroopState::ATTACKING);
        _pathPoints.clear();
        return;
    }

    // 2. 沿路径移动
    if (_currentPathIndex < _pathPoints.size()) {
        // ... (保持你原有的移动代码不变) ...
        Vec2 nextTile = _pathPoints[_currentPathIndex];
        Vec2 nextWaypoint = _villageScene->isoTileToContainerPosPublic(nextTile);
        Vec2 direction = nextWaypoint - this->getPosition();
        float distToWaypoint = direction.length();
        float moveStep = _config.moveSpeed * dt * _mapScale;

        if (distToWaypoint < moveStep) {
            this->setPosition(nextWaypoint);
            _currentPathIndex++;
        }
        else {
            direction.normalize();
            this->setPosition(this->getPosition() + direction * moveStep);
        }
    }
    else {
        // 【修改】路径走完了
        // 再次检查距离！防止路径被阻断导致只走了一半就停下
        if (distToTarget <= _attackRange + 10.0f) { // 允许微小误差
            setState(TroopState::ATTACKING);
        }
        else {
            // 路径走完了但还没够着目标（比如目标在墙里，寻路只寻到了墙外）
            // 暂时设为 IDLE 或者重新寻路
            findNewTarget();
            // 可选：这里可以触发 verifyAttackTarget() 再次尝试贴脸
        }
    }
}
// ========== 通用：帧更新（核心逻辑） ==========
void BaseTroop::update(float dt) {
    Sprite::update(dt);


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


        // 2. [关键检查] 检查目标是否已经“死亡”
        // 如果建筑被 VillageScene::destroyBuilding 移除了，它的 parent 会变成 nullptr
        // 但因为我们 retain 了它，内存是安全的，这里可以放心访问
        if (_attackTarget->getParent() == nullptr || _attackTarget->getState() == BuildingState::DESTROYED) {
            
            findNewTarget();
            return;
        }

        updateAttackCD();

        // [Troop.cpp]

// ... 在文件末尾或合适位置添加 ...

        // 3. 执行攻击
        if (_attackCDTimer >= _config.attackSpeed) {
            doSpecialAttack();

            // 造成伤害
            // 即使这一击导致建筑死亡，由于我们 retain 了，下面的代码依然安全
            _attackTarget->takeDamage(this->_attackPower);

            // 再次检查目标状态（防止刚才那一下把它打死了）
            if (_attackTarget && (_attackTarget->getParent() == nullptr || _attackTarget->getState() == BuildingState::DESTROYED)) {
                
                findNewTarget();
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

// [Troop.cpp]

// [Troop.cpp]

void BaseTroop::findNewTarget() {
    // 0. 安全检查
    if (!_villageScene) return;

    // 获取当前位置
    Vec2 myPos = this->getPosition();
    BaseBuilding* newTarget = nullptr;

    // ==================== 第一阶段：优先目标搜索 (偏好特定类型的兵种) ====================

    // 1. 【巨人逻辑】优先寻找防御建筑
    if (_config.type == TroopType::GIANT) {
        std::vector<BuildingType> defenseTypes = { BuildingType::CANNON, BuildingType::ARROW_TOWER };
        newTarget = _villageScene->findNearestBuildingByTypes(myPos, defenseTypes);
    }
    // 2. 【炸弹人逻辑】优先寻找城墙
    else if (_config.type == TroopType::BOMBER) {
        std::vector<BuildingType> wallTypes = { BuildingType::WALL };
        newTarget = _villageScene->findNearestBuildingByTypes(myPos, wallTypes);
    }

    // 注意：弓箭手没有“优先目标”（她打最近的任意建筑），所以不需要在第一阶段处理

    // ==================== 第二阶段：通用搜索 (设定忽略规则) ====================

    // 如果第一阶段没有找到目标（或者兵种没有优先偏好），执行通用搜索
    if (!newTarget) {
        BuildingType ignoreType = BuildingType::UNKNOWN;

        // --- 设定忽略规则 ---

        // A. 【弓箭手逻辑】
        // 弓箭手是远程，始终忽略围墙，直接锁定墙后的建筑
        if (_config.type == TroopType::ARCHER) {
            ignoreType = BuildingType::WALL;
        }
        // B. 【巨人逻辑 (无防御塔时)】
        // 如果防御塔都被拆光了，巨人转为普通攻击模式，但也应该忽略围墙（除非只剩围墙）
        else if (_config.type == TroopType::GIANT) {
            ignoreType = BuildingType::WALL;
        }
        // C. 【炸弹人逻辑 (无墙时)】
        // 如果全图没墙了，炸弹人就打普通建筑，不忽略任何东西
        else if (_config.type == TroopType::BOMBER) {
            ignoreType = BuildingType::UNKNOWN;
        }
        // D. 【野蛮人逻辑】
        // 默认不忽略，谁近打谁（包括墙）

        // --- 执行搜索 ---
        // 尝试寻找最近的、非忽略类型的建筑
        newTarget = _villageScene->findNearestEnemyBuilding(myPos, ignoreType);

        // ==================== 第三阶段：绝境兜底 ====================

        // 如果我们设定了忽略墙（比如弓箭手/巨人），但找不到任何目标
        // 说明全图只剩下围墙了（所有普通建筑都被摧毁）
        // 这时必须强制允许攻击围墙，否则弓箭手会原地发呆
        if (!newTarget && ignoreType == BuildingType::WALL) {
            newTarget = _villageScene->findNearestEnemyBuilding(myPos, BuildingType::UNKNOWN);
        }
    }

    // ==================== 第四阶段：应用目标 ====================

    if (newTarget) {
        // 找到新目标，锁定并自动触发寻路
        setAttackTarget(newTarget);
    }
    else {
        // 全图没有任何敌方单位了，待机
        setAttackTarget(nullptr);
        setState(TroopState::IDLE);
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

}

// [Troop.cpp]

// 恢复为标准的一个参数，去掉多余的 ignoretype
void BaseTroop::setAttackTarget(BaseBuilding* target) {
    // 1. 安全检查：如果目标没有变化，直接返回
    if (_attackTarget == target) return;

    // 2. 释放旧目标（防止内存泄漏）
    CC_SAFE_RELEASE(_attackTarget);

    // 3. 设置新目标
    _attackTarget = target;

    // 4. 持有新目标（防止野指针崩溃）
    CC_SAFE_RETAIN(_attackTarget);

    // 5. 执行寻路逻辑
    if (_attackTarget) {
        // 只有目标存在时才去获取位置
        setTargetTilePosition(_attackTarget->getTilePos());
    }
    else {
        // 如果目标被置空（比如没找到目标），兵种应当待机
        setState(TroopState::IDLE);
    }
}
