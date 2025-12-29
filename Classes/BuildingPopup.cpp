#include "BuildingPopup.h"
#include "VillageScene.h"

/**
 * 创建弹窗实例 (工厂方法).
 * 初始化弹窗并绑定目标建筑和回调函数.
 * * @param building 目标建筑指针.
 * @param btnCallback 按钮点击回调函数.
 * @return 创建成功的 BuildingPopup 对象指针，失败返回 nullptr.
 */
BuildingPopup* BuildingPopup::create(BaseBuilding* building,
    const std::function<void(ButtonType)>& btnCallback) {
    auto popup = new (std::nothrow) BuildingPopup();
    if (popup && popup->init(building, btnCallback)) {
        popup->autorelease();
        return popup;
    }
    CC_SAFE_DELETE(popup);
    return nullptr;
}

/**
 * 初始化弹窗.
 * 设置背景颜色、触摸监听和按钮布局.
 * * @param building 目标建筑.
 * @param btnCallback 回调函数.
 * @return 初始化成功返回 true.
 */
bool BuildingPopup::init(BaseBuilding* building,
    const std::function<void(ButtonType)>& btnCallback) {
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180))) { // 半透明黑色背景
        return false;
    }

    _targetBuilding = building;
    _btnCallback = btnCallback;
    _targetBuilding->retain(); // 防止建筑被提前释放 (注意：建议在onExit或析构中release)

    // 弹窗大小（适配屏幕）
    Size winSize = Director::getInstance()->getWinSize();
    this->setContentSize(winSize);
    this->setPosition(Vec2::ZERO);

    // 点击背景关闭
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true); // 吞噬触摸，防止穿透到下层地图
    listener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
        };
    listener->onTouchEnded = [this](Touch* touch, Event* event) {
        this->removeFromParentAndCleanup(true);
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // 生成按钮
    createButtons();

    return true;
}

/**
 * 创建功能按钮.
 * 根据建筑类型和状态，计算按钮位置并添加到弹窗中.
 * TODO: 要新加一个模式，以便在进行建筑操作的时候鼠标不会影响其他地方.
 */
void BuildingPopup::createButtons() {
    if (!_targetBuilding) return; // 空指针防护

    // ========== 步骤1：获取建筑的屏幕坐标（核心修改） ==========

    // 1.1 获取建筑在世界坐标系中的位置（相对于屏幕）
    Vec2 buildingWorldPos = _targetBuilding->convertToWorldSpaceAR(Vec2::ZERO);
    // 1.2 转换为弹窗节点的本地坐标（如果弹窗是全屏节点，可直接用worldPos）
    Vec2 buildingLocalPos = this->convertToNodeSpace(buildingWorldPos);

    // ========== 步骤2：定义按钮尺寸和间距 ==========
    float btnWidth = 80;
    float btnHeight = 40;
    float spacing = 20; // 按钮间距
    float offsetY = 60; // 按钮组相对于建筑向上偏移的距离（避免遮挡建筑）

    // ========== 步骤3：根据建筑类型计算按钮数量和总宽度 ==========
    int btnCount = 0;
    if (_targetBuilding->getState() == BuildingState::IDLE) {
        if (_targetBuilding->getType() == BuildingType::GOLD_MINE) {
            btnCount = 4; // 金矿4个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::ELIXIR_COLLECTOR) {
            btnCount = 4; // 圣水收集器4个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::TOWN_HALL) {
            btnCount = 3; // 大本营3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::BARRACKS) {
            btnCount = 3; // 兵营3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::TRAINING_CAMP) {
            btnCount = 4; // 训练营4个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::CANNON) {
            btnCount = 3; // 加农炮3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::ARROW_TOWER) {
            btnCount = 3; // 箭塔3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::VAULT) {
            btnCount = 3; // 金库3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::ELIXIR_BOTTLE) {
            btnCount = 3; // 圣水瓶3个按钮
        }
        else if (_targetBuilding->getType() == BuildingType::WALL) {
            btnCount = 3; // 城墙3个按钮
        }
        else {
            return; // 其他建筑类型暂不支持弹窗
        }
    }
    else {
        btnCount = 2; // 非闲置状态仅信息和摧毁按钮
    }

    // 按钮总宽度 = 按钮数*宽度 + (按钮数-1)*间距
    float totalWidth = btnCount * btnWidth + (btnCount - 1) * spacing;
    // 按钮组起始X（相对于建筑居中）
    float startX = buildingLocalPos.x - totalWidth / 2;
    // 按钮组Y坐标（建筑上方offsetY像素）
    float startY = buildingLocalPos.y + offsetY;

    // ========== 步骤4：创建按钮（基于建筑坐标） ==========
    if (_targetBuilding->getType() == BuildingType::GOLD_MINE) {
        if (_targetBuilding->getState() == BuildingState::IDLE) {
            // 1. 信息按钮
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            // 2. 升级按钮
            createButton("ui/btn_upgrade.png", ButtonType::UPGRADE, Vec2(startX + btnWidth + spacing, startY));
            // 3. 收集资源按钮
            createButton("ui/btn_collect_gold.png", ButtonType::COLLECT, Vec2(startX + 2 * (btnWidth + spacing), startY));
            // 4. 摧毁按钮
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + 3 * (btnWidth + spacing), startY));
        }
        else {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + btnWidth + spacing, startY));
        }
    }

    else if (_targetBuilding->getType() == BuildingType::ELIXIR_COLLECTOR) {
        if (_targetBuilding->getState() == BuildingState::IDLE) {
            // 1. 信息按钮
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            // 2. 升级按钮
            createButton("ui/btn_upgrade.png", ButtonType::UPGRADE, Vec2(startX + btnWidth + spacing, startY));
            // 3. 收集资源按钮
            createButton("ui/btn_collect_elixir.png", ButtonType::COLLECT, Vec2(startX + 2 * (btnWidth + spacing), startY));
            // 4. 摧毁按钮
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + 3 * (btnWidth + spacing), startY));
        }
        else {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + btnWidth + spacing, startY));
        }
    }
    else if (_targetBuilding->getType() == BuildingType::TRAINING_CAMP) {
        // 训练营按钮（4个，居中）
        // TODO: 加入预览当前部队按钮
        if (_targetBuilding->getState() == BuildingState::IDLE|| _targetBuilding->getState() == BuildingState::TRAINING) {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_upgrade.png", ButtonType::UPGRADE, Vec2(startX + btnWidth + spacing, startY));
            createButton("ui/btn_train.png", ButtonType::TRAINING, Vec2(startX + 2 * (btnWidth + spacing), startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + 3 * (btnWidth + spacing), startY));
        }
        else {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + btnWidth + spacing, startY));
        }
    }
    else {
        // 标准3按钮布局（信息/升级/摧毁）
        if (_targetBuilding->getState() == BuildingState::IDLE) {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_upgrade.png", ButtonType::UPGRADE, Vec2(startX + btnWidth + spacing, startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + 2 * (btnWidth + spacing), startY));
        }
        else {
            createButton("ui/btn_info.png", ButtonType::INFO, Vec2(startX, startY));
            createButton("ui/btn_destroy.png", ButtonType::DESTROY, Vec2(startX + btnWidth + spacing, startY));
        }
    }
}

/**
 * 创建单个按钮.
 * * @param imgPath 按钮图片路径.
 * @param type 按钮功能类型.
 * @param pos 按钮位置坐标.
 */
void BuildingPopup::createButton(const std::string& imgPath, ButtonType type, const Vec2& pos) {
    // 创建按钮（图片形式）
    auto btn = MenuItemImage::create(
        imgPath,          // 正常状态图片
        imgPath,          // 按下状态图片（可替换为btn_xxx_pressed.png）
        [this, type](Ref* sender) { // 点击回调
            if (_btnCallback) {
                _btnCallback(type); // 触发外部回调
            }
            this->removeFromParentAndCleanup(true); // 关闭弹窗
        }
    );
    btn->setPosition(pos);

    // 创建菜单并添加按钮
    auto menu = Menu::create(btn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
}