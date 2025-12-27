#include "VillageScene.h"
#include "cocos2d.h"
#include "Building.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 
#include "Troop.h"
#include "EnumType.h" 
#include "LevelScene.h"
#include "TitleScene.h"
//VillageScene* VillageScene::_instance = nullptr;
BaseMode initBaseMode;
bool VillageScene::init()
{
    if (!Scene::init()) return false;
    // 初始化流程

    _mapContainer = Node::create();
    this->addChild(_mapContainer);
    _uiLayer = ui::Layout::create();
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _uiLayer->setContentSize(visibleSize); // 布局尺寸等于可视区域
    _uiLayer->setPosition(Vec2::ZERO);
    _uiLayer->setLayoutType(ui::Layout::Type::ABSOLUTE); // 绝对定位
    this->addChild(_uiLayer, 200); // 布局层级200
    initMap();
    initBtns(initBaseMode);
    if (initBaseMode == BaseMode::FIGHT) {
        initFightScene();
    }
    if (initBaseMode != BaseMode::FIGHT) {
        initBuildPreview();
        initResourceBar();
    }
    initTroopPreview();
    
    //initSaveLoadButtons();
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
        for (int j = tilePos.y;j <= tilePos.y + config.tileWidth - 1;j++) {
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
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
        if (_isBuildBarShow && _Mode == Mode::PLACE_BUILDING) {
            // 右键取消建造模式
            if (_buildPreview) {
                _buildPreview->setVisible(false);
            }
            _Mode = Mode::NONE;
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
            //currentMousePos.y += 50; // 微调Y轴位置，以便显示真实放置格子
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
        // 和建筑预览用同一个位置计算方式
        Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(currentMousePos);
        _troopPreview->setPosition(containerLocalPos);
        _troopPreview->setScale(_mapContainer->getScale());
        // 和建筑用同一个检测函数，确保预览颜色正确
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
Scene* VillageScene::createScene(BaseMode baseMode)
{
	initBaseMode = baseMode;
    auto scene = Scene::create();
    auto layer = VillageScene::create();
    scene->addChild(layer);
    if (baseMode != BaseMode::FIGHT) {
        layer->setTag(100);//便于getInstance获取
    }
    auto villageScene = dynamic_cast<VillageScene*>(layer);
    villageScene->setBaseMode(baseMode); // 设置模式
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
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.5f));
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
// 初始化按钮相关代码
void VillageScene::initBtns(BaseMode baseMode) {
    //获取可视区域
    Size visibleSize = Director::getInstance()->getVisibleSize();
    //有黑边时origin不为0
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    //origin.x, origin.y为左下角
    // origin.x+visibleSize.width最右边
    // origin。y+visibleSize.height最上边
    //  创建布局容器（统一管理按钮）
    //  创建返回按钮（MenuItemImage）
    _backBtn = ui::Button::create(
        "ui/btn_back.png",       // 正常状态图片
        "ui/btn_back_selected.png"// 按下状态图片
    );
    _uiLayer->addChild(_backBtn, 200);
    _backBtn->setScale(0.8f);
    _backBtn->setPosition(Vec2(origin.x + 50, origin.y + visibleSize.height - 50)); // 右上角
	if (baseMode == BaseMode::FIGHT) {
        _backBtn->addClickEventListener([this](Ref* sender) {    // 点击回调：战斗
            this->backfromFight();
            });
	}
    else {
        _backBtn->addClickEventListener([this](Ref* sender) {    // 点击回调：销毁当前场景，返回主菜单
            this->destroyScene();
            });
    }


    // 兵种模式开关按钮（在建筑按钮左侧）
   _troopModeBtn = ui::Button::create(
        "ui/troop_mode_btn_normal.png",  // 正常状态图片
        "ui/troop_mode_btn_selected.png"// 按下状态图片
    );
    _uiLayer->addChild(_troopModeBtn, 200);
    _troopModeBtn->setScale(0.8f);
    _troopModeBtn->setPosition(Vec2(origin.x + visibleSize.width - 220, origin.y + visibleSize.height - 100)); // 建筑按钮左侧
    _troopModeBtn->addClickEventListener([this](Ref* sender) {
        this->toggleTroopBar(); // 点击切换兵种栏
        });
    // 创建建筑模式开关按钮（右上角悬浮）
    if (baseMode != BaseMode::FIGHT) {
         _buildModeBtn = ui::Button::create(
            "ui/build_mode_btn_normal.png",  // 正常状态图片
            "ui/build_mode_btn_selected.png"// 按下状态图片
        );
        _uiLayer->addChild(_buildModeBtn, 200);
        _buildModeBtn->setScale(0.8f);
        _buildModeBtn->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height - 100));
        _buildModeBtn->addClickEventListener([this](Ref* sender) {
            this->toggleBuildBar(); // 点击切换建筑栏
            });
    }


    // 创建关卡选择按钮（左下角）
    if (baseMode != BaseMode::FIGHT) {
        _levelSelectBtn = ui::Button::create(
            "ui/level_select_btn_normal.png",  // 正常状态图片
            "ui/level_select_btn_selected.png"// 按下状态图片
        );
        _uiLayer->addChild(_levelSelectBtn, 200);
        // 初始化关卡选择层状态（TODO：为什么要加在这里？）
        _isLevelSelectShow = false;
        _levelSelectLayer = nullptr;
        _levelSelectBtn->setScale(0.8f);
        _levelSelectBtn->setPosition(Vec2(origin.x + 100, origin.y + 100)); // 左下角，距离左边缘和下边缘各50像素
        _levelSelectBtn->addClickEventListener([this](Ref* sender) {
            this->toggleLevelSelectMenu(); // 点击切换关卡选择菜单
            });
    }

    if (baseMode != BaseMode::FIGHT) {
        // 创建存档按钮
         _loadBtn = ui::Button::create(
            "ui/btn_normal.png",
            "ui/btn_pressed.png",
            "ui/btn_disabled.png"
        );
        _uiLayer->addChild(_loadBtn, 200);
        // 创建存档按钮
         _saveBtn = ui::Button::create(
            "ui/btn_normal.png",   // 正常状态图片（替换为你的资源路径）
            "ui/btn_pressed.png",  // 按下状态图片
            "ui/btn_disabled.png"  // 禁用状态图片（可选）
        );
        _uiLayer->addChild(_saveBtn, 200);
        _saveBtn->setContentSize(Size(120, 60));
        _saveBtn->setPosition(Vec2(_backBtn->getPositionX(), _backBtn->getPositionY() - 80));

        _loadBtn->setContentSize(Size(120, 60));// 位置在存档按钮下方，间距20
        _loadBtn->setPosition(Vec2(_saveBtn->getPositionX(), _saveBtn->getPositionY() - 80));
        // 设置按钮文字
        auto saveText = ui::Text::create("save", "fonts/Marker Felt.ttf", 24);
        saveText->setColor(Color3B::WHITE);
        _saveBtn->addChild(saveText);
        auto loadText = ui::Text::create("load", "fonts/Marker Felt.ttf", 24);
        loadText->setColor(Color3B::WHITE);
        _loadBtn->addChild(loadText);
        _saveBtn->addClickEventListener(CC_CALLBACK_1(VillageScene::onSaveBtnClicked, this));
        _loadBtn->addClickEventListener(CC_CALLBACK_1(VillageScene::onLoadBtnClicked, this));
    }

    if (baseMode != BaseMode::FIGHT) {
         _fightBtn = ui::Button::create(
            "ui/fight_btn_normal.png",  // 正常状态图片
            "ui/fight_btn_selected.png"// 按下状态图片
        );
        _uiLayer->addChild(_fightBtn, 200);
        _fightBtn->setScale(0.8f);
        _fightBtn->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + 100)); // 右下角，距离右边缘和下边缘各50像素
        _fightBtn->addClickEventListener([this](Ref* sender) {
            this->gotoFight(); // 点击进入战斗场景
            });
    }
    if (baseMode == BaseMode::FIGHT) {
        _fightStartBtn = ui::Button::create("ui/fight_start_btn.png"); 
        _uiLayer->addChild(_fightStartBtn, 200);
        _fightStartBtn->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + 100));
        _fightStartBtn->addClickEventListener([this](Ref* sender) {
            this->beginFight(); // 开始战斗
            });
    }
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
//初始化战斗场景
void  VillageScene::initFightScene() {

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
    CCLOG("basePos.x: %f, basePos.y: %f", basePos.x, basePos.y);
    // 2. 计算中心点偏移：向上移动半个瓦片高度
    // Cocos2d-x 坐标系 Y 轴向上，所以是 + height/2
    Vec2 localCenter = basePos + Vec2(_tileSize.width /2.0f
		, _tileSize.height / 2.0f
    );
    CCLOG("localCenter.x: %f, basePos.y: %f", localCenter.x, localCenter.y);
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
        for (int j = tilePos.y;j <= tilePos.y + config.tileWidth - 1;j++) {
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
// 初始化建筑栏（新增建筑类型需要扩展此函数）
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
    // 建筑按钮 - 圣水收集器
    auto elixirCollectorBtn = MenuItemImage::create(
        "building/elixir_collector_icon.png",
        "building/elixir_collector_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ELIXIR_COLLECTOR;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/elixir_collector_preview.png");
        }
    );
    // 建筑按钮 - 军营
    auto barracksBtn = MenuItemImage::create(
        "building/barracks_icon.png",
        "building/barracks_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::BARRACKS;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/barracks_preview.png");
        }
    );
    // 建筑按钮 - 训练营
    auto trainingCampBtn = MenuItemImage::create(
        "building/training_camp_icon.png",
        "building/training_camp_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::TRAINING_CAMP;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/training_camp_preview.png");
        }
    );
    auto cannonBtn = MenuItemImage::create(
        "building/cannon_icon.png",
        "building/cannon_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::CANNON;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/cannon_preview.png");
        }
    );
    auto arrowTowerBtn = MenuItemImage::create(
        "building/arrow_tower_icon.png",
        "building/arrow_tower_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ARROW_TOWER;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/arrow_tower_preview.png");
        }
    );
    auto wallBtn = MenuItemImage::create(
        "building/wall_icon.png",
        "building/wall_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::WALL;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/wall_preview.png");
        }
    );
    auto elixirBottleBtn = MenuItemImage::create(
        "building/elixir_bottle_icon.png",
        "building/elixir_bottle_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ELIXIR_BOTTLE;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/elixir_bottle_preview.png");
        }
    );
    auto vaultBtn = MenuItemImage::create(
        "building/vault_icon.png",
        "building/vault_icon_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::VAULT;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/vault_preview.png");
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
    auto menu = Menu::create(townHallBtn, goldMineBtn, elixirCollectorBtn, barracksBtn, trainingCampBtn, cannonBtn, arrowTowerBtn, wallBtn, elixirBottleBtn, vaultBtn, cancelPlaceBtn, nullptr);
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
// 处理按钮点击的核心逻辑（新增建筑类型可能需要扩展此函数）
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
            CCLOG("建筑开始升级！");
        }
        else {
            CCLOG("建筑非闲置状态，无法升级！");
        }
        break;

    case BuildingPopup::ButtonType::COLLECT:
        // 收集金矿资源（仅金矿有效）
        if (building->getType() == BuildingType::GOLD_MINE) {
            auto goldMine = dynamic_cast<GoldMine*>(building);
            if (goldMine) {
                // 调用金矿收集资源的方法
                addGold(goldMine->collectGold());
            }
        }
        else if (building->getType() == BuildingType::ELIXIR_COLLECTOR) {
            auto elixirCollector = dynamic_cast<ElixirCollector*>(building);
            if (elixirCollector) {
                // 调用收集资源的方法
                addElixir(elixirCollector->collectElixir());
            }
        }
        else {
        }
        break;

    case BuildingPopup::ButtonType::DESTROY:
        // 摧毁建筑
        destroyBuilding(building);
        break;
    case BuildingPopup::ButtonType::TRAINING:
        if (building->getType() == BuildingType::TRAINING_CAMP) {
            auto trainingCamp = dynamic_cast<TrainingCamp*>(building);
            if (trainingCamp) {
                // 弹出训练窗口
                showTrainingCampPopup(trainingCamp);
            }

        }
        else {}
    default:
        break;
    }
}
// 放置建筑（新增建筑类型可能需要扩展此函数）
void VillageScene::placeBuilding(Vec2 tilePos, BuildingType type) {
    auto building = BaseBuilding::create(type, tilePos, 1.0f);
    if (building) {
        _buildPreview->setVisible(false);
        // 加入建筑总列表（核心：保存实例引用，避免内存泄漏/无法管理）
        _buildings.push_back(building);
        // 按类型加入细分列表
        //TODO: 哥布林攻击金矿，炸弹人攻击城墙等逻辑需要用到这些列表
        if (type == BuildingType::GOLD_MINE) {
            _goldMines.push_back(dynamic_cast<GoldMine*>(building));
        }
        else if (type == BuildingType::ELIXIR_COLLECTOR) {
            _elixirCollectors.push_back(dynamic_cast<ElixirCollector*>(building));
        }
        else if (type == BuildingType::BARRACKS) {
            auto barracks = dynamic_cast<Barracks*>(building);
            if (barracks) {
                // 绑定建造/升级完成回调
                barracks->bindBuildFinishCallback([this, barracks](BaseBuilding* b) {
                    // 升级时增加容量
                    if (barracks->getLevel() > 1) {
                        this->addTroopCapacity(barracks->getPulseSpace());
                    }
                    // 增加新的容量加成
                    else {
                        this->addTroopCapacity(barracks->getTroopSpace());
                    }
                    });
            }
        }
        else if (type == BuildingType::VAULT) {
            auto vault = dynamic_cast<Vault*>(building);
            if (vault) {
                // 绑定建造/升级完成回调
                vault->bindBuildFinishCallback([this, vault](BaseBuilding* b) {
                    // 升级时增加容量
                    if (vault->getLevel() > 1) {
                        this->addGoldStorageCapacity(vault->getStoragePulse());
                    }
                    // 增加新的容量加成
                    else {
                        this->addGoldStorageCapacity(vault->getStorageCapacity());
                    }
                    });
            }
        }
        else if (type == BuildingType::ELIXIR_BOTTLE) {
            auto elixirBottle = dynamic_cast<ElixirBottle*>(building);
            if (elixirBottle) {
                // 绑定建造/升级完成回调
                elixirBottle->bindBuildFinishCallback([this, elixirBottle](BaseBuilding* b) {
                    // 升级时增加容量
                    if (elixirBottle->getLevel() > 1) {
                        this->addGoldStorageCapacity(elixirBottle->getStoragePulse());
                    }
                    // 增加新的容量加成
                    else {
                        this->addGoldStorageCapacity(elixirBottle->getStorageCapacity());
                    }
                    });
            }
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
        int n = config.tileWidth;
        Vec2 topLeftTile = tilePos; // 左上角瓦片（基准瓦片）
        Vec2 bottomRightTile = Vec2(
            tilePos.x + config.tileWidth - 1,
            tilePos.y + config.tileHeight - 1
        ); // 右下角瓦片
        CCLOG("aaatile.x: %f, tile.y: %f", tilePos.x, tilePos.y);
        // 计算两个瓦片的中心点容器坐标
        Vec2 posTopLeft = isoTileToContainerPos(topLeftTile);
        Vec2 posBottomRight = isoTileToContainerPos(bottomRightTile);

        // 求中点（区域中心点）
        Vec2 containerLocalPos = Vec2(
            (posTopLeft.x + posBottomRight.x) / 2.0f,
            (posTopLeft.y + posBottomRight.y) / 2.0f
        );
        CCLOG("containerLocalPos.x: %f, containerLocalPos.y: %f", containerLocalPos.x, containerLocalPos.y);
        // 确保建筑锚点居中（关键：默认锚点可能不是中心，需显式设置）
        building->setAnchorPoint(Vec2(0.5f, 0.5f));
        building->setPosition(containerLocalPos);
        _mapContainer->addChild(building);

		// 越在右，越在下的建筑层级越高
        building->setLocalZOrder(1000 + (tilePos.x + tilePos.y));
        // 通用逻辑：添加到场景 + 绑定点击回调
        if (building) {

            // 统一绑定点击回调（弹窗逻辑）
            building->bindClickCallback([this](BaseBuilding* building) {
                if (_Mode != Mode::NONE) {
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
// 摧毁建筑（新增建筑类型可能需要扩展此函数）
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
    if (building->getType() == BuildingType::ELIXIR_COLLECTOR) {
        auto it1 = std::find(_elixirCollectors.begin(), _elixirCollectors.end(), dynamic_cast<ElixirCollector*>(building));
        if (it1 != _elixirCollectors.end()) {
            _elixirCollectors.erase(it1);
        }
    }
    else if (building->getType() == BuildingType::GOLD_MINE) {
        auto it1 = std::find(_goldMines.begin(), _goldMines.end(), dynamic_cast<GoldMine*>(building));
        if (it1 != _goldMines.end()) {
            _goldMines.erase(it1);
        }
    }
    else if (building->getType() == BuildingType::BARRACKS) {
        auto barrack = dynamic_cast<Barracks*>(building);
        this->removeTroopCapacity(barrack->getTroopSpace());
    }
    else if (building->getType() == BuildingType::TRAINING_CAMP) {
    }
    else if (building->getType() == BuildingType::VAULT) {
        auto vault = dynamic_cast<Vault*>(building);
        this->addGoldStorageCapacity(-(vault->getStorageCapacity()));
    }
    else if (building->getType() == BuildingType::ELIXIR_BOTTLE) {
        auto elixirBottle = dynamic_cast<ElixirBottle*>(building);
        this->addElixirStorageCapacity(-(elixirBottle->getStorageCapacity()));
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
// 一键收集资源
void VillageScene::collectOneNote() {
    for (auto goldMine : _goldMines) {
        addGold(goldMine->collectGold());
    }
    for (auto elixirCollector : _elixirCollectors) {
        addElixir(elixirCollector->collectElixir());
    }
}
// 显示训练营弹窗
void VillageScene::showTrainingCampPopup(TrainingCamp* camp) {
    if (!camp || _trainingPopup) return;

    _currentCamp = camp;
    _trainingPopup = ui::Layout::create();
    _trainingPopup->setContentSize(Size(600, 400));
    _trainingPopup->setBackGroundColor(Color3B::WHITE);
    _trainingPopup->setAnchorPoint(Vec2(0.5f, 0.5f));
    _trainingPopup->setPosition(Vec2(Director::getInstance()->getWinSize() / 2));
    _trainingPopup->setOpacity(250);
    addChild(_trainingPopup, 100); // 置顶显示

    // 弹窗遮罩（点击遮罩不关闭，避免误触）
    auto mask = LayerColor::create(Color4B(0, 0, 0, 180));
    mask->setContentSize(Director::getInstance()->getWinSize());
    mask->setPosition(Vec2::ZERO);
    addChild(mask, 99);
    _trainingPopup->setTag(1001); // 标记弹窗
    mask->setTag(1002); // 标记遮罩

    // 初始化弹窗UI
    initTrainingPopupUI();
    // 启动倒计时更新
    schedule([this](float dt) {
        this->updateTrainQueueTimer(dt);
        }, 0.1f, "updateTrainQueueTimerKey");
}
// 隐藏训练弹窗
void VillageScene::hideTrainingCampPopup() {
    if (_trainingPopup) {
        _trainingPopup->removeFromParent();
        _trainingPopup = nullptr;
    }
    auto mask = getChildByTag(1002);
    if (mask) mask->removeFromParent();
    _currentCamp = nullptr;
    unschedule("updateTrainQueueTimerKey");//隐藏不代表训练停止
}
// 初始化弹窗UI
void VillageScene::initTrainingPopupUI() {

    /*
    tipLabel->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ))*/
    // 弹窗标题
    auto title = Label::createWithTTF(StringUtils::format("Training Camp Lv.%d", _currentCamp->getLevel()),
        "fonts/arial.ttf", 30);
    title->setPosition(Vec2(_trainingPopup->getContentSize().width / 2, _trainingPopup->getContentSize().height - 30));
    title->setColor(Color3B::BLACK);
    _trainingPopup->addChild(title, 10000);

    // 关闭按钮
    auto closeBtn = ui::Button::create("ui/btn_close.png");
    closeBtn->setScale(0.8f);
    closeBtn->setPosition(Vec2(_trainingPopup->getContentSize().width - 20, _trainingPopup->getContentSize().height - 20));
    closeBtn->addClickEventListener([this](Ref*) {
        this->hideTrainingCampPopup();
        });
    _trainingPopup->addChild(closeBtn, 10000);
    // 初始化兵种按钮和队列面板
    initTroopButtonsInPopup();
    initTrainQueuePanelInPopup();
}
// 初始化训练队列面板
void VillageScene::initTrainQueuePanelInPopup() {
    if (!_trainingPopup || !_currentCamp) return;

    // 队列面板（弹窗上半部分）
    auto queuePanel = ui::Layout::create();
    queuePanel->setContentSize(Size(1000, 200));
    queuePanel->setPosition(Vec2(25, 120));
    queuePanel->setBackGroundColor(Color3B(240, 240, 240));
    queuePanel->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
    queuePanel->setTag(1003); // 标记队列面板
    _trainingPopup->addChild(queuePanel);

    // 队列标题
    auto queueTitle = Label::createWithTTF("train queue", "fonts/arial.ttf", 20);
    queueTitle->setPosition(Vec2(queuePanel->getContentSize().width / 2, queuePanel->getContentSize().height - 15));
    queueTitle->setColor(Color3B::BLACK);
    queuePanel->addChild(queueTitle);

    // 初始刷新队列UI
    refreshTrainQueueUI();
}
// 初始化可训练兵种按钮
void VillageScene::initTroopButtonsInPopup() {
    if (!_trainingPopup || !_currentCamp) return;

    // 兵种按钮容器（底部横向排列）
    auto btnPanel = ui::Layout::create();
    btnPanel->setContentSize(Size(550, 80));
    btnPanel->setPosition(Vec2(25, 20));
    _trainingPopup->addChild(btnPanel);

    // 遍历兵种配置，按训练营等级筛选可训练兵种
    int campLevel = _currentCamp->getLevel();
    float btnX = 30;
    const float btnSize = 70;
    for (const auto& pair : g_troopTrainConfig) {
        TroopType type = pair.first;
        const TroopConfig& config = pair.second;

        // 等级不足则跳过
        if (config.unlockCampLevel > campLevel) continue;

        // 创建兵种按钮
        auto troopBtn = ui::Button::create(config.imgPath);
        troopBtn->setContentSize(Size(btnSize, btnSize));
        troopBtn->setPosition(Vec2(btnX, btnPanel->getContentSize().height / 2));
        btnPanel->addChild(troopBtn);
        btnX += btnSize + 20;

        // 按钮点击事件
        troopBtn->addClickEventListener([this, type](Ref*) {
            // 检查队列是否已满
            if (_currentCamp->getTrainQueue().size() >= MAX_QUEUE_SIZE) {
                showResourceShortageTip("训练队列已满!");
                return;
            }
            // 检查资源
            if (!checkTroopResourceEnough(type)) return;
            // 扣除资源 + 添加到队列
            deductTroopResource(type);
            addTroopToQueue(type);
            // 刷新队列UI
            refreshTrainQueueUI();
            });

        // 添加资源提示（按钮下方）
        auto costTip = Label::createWithTTF(
            StringUtils::format("elixir:%d", pair.second.elixirCost),
            "fonts/Marker Felt.ttf", 14);
        costTip->setPosition(Vec2(troopBtn->getPositionX(), troopBtn->getPositionY() - btnSize / 2 - 10));
        costTip->setColor(Color3B::BLACK);
        btnPanel->addChild(costTip);
    }
}
// 更新队列倒计时
void VillageScene::updateTrainQueueTimer(float dt) {
    if (!_currentCamp) return;

    auto& trainQueue = _currentCamp->getTrainQueue();
    auto& queueTimers = _currentCamp->getQueueTimers();
    if (trainQueue.empty()) return;

    // 遍历更新倒计时
    for (int i = 0; i < trainQueue.size(); ++i) {
        queueTimers[i] -= dt;
        // 训练完成：移除队列项，创建兵种（此处简化，仅移除）
        if (queueTimers[i] <= 0) {
            removeTroopFromQueue(i);
            refreshTrainQueueUI();
            break; // 避免索引错乱，一次只处理一个完成项
        }
    }
}
// 检查兵种训练资源是否足够
bool VillageScene::checkTroopResourceEnough(TroopType type) {
    const auto& config = g_troopTrainConfig.at(type);
    bool elixirOk = (config.elixirCost <= 0) || (_elixir >= config.elixirCost);
    if (!elixirOk) showResourceShortageTip("elixir not enough!");
    return  elixirOk;
}
// 扣除兵种训练资源
void VillageScene::deductTroopResource(TroopType type) {
    const auto& config = g_troopTrainConfig.at(type);
    if (config.elixirCost > 0) spendElixir(config.elixirCost);
}
// 返还兵种训练资源
void VillageScene::refundTroopResource(TroopType type) {
    const auto& config = g_troopTrainConfig.at(type);
    if (config.elixirCost > 0) addElixir(config.elixirCost);
}
// 添加兵种到训练队列
void VillageScene::addTroopToQueue(TroopType type) {
    if (!_currentCamp) return;

    auto& trainQueue = _currentCamp->getTrainQueue();
    auto& queueTimers = _currentCamp->getQueueTimers();
    trainQueue.push_back(type);
    queueTimers.push_back(g_troopTrainConfig.at(type).trainingTime);
}
// 移除队列指定位置的兵种
void VillageScene::removeTroopFromQueue(int index) {
    if (!_currentCamp) return;

    auto& trainQueue = _currentCamp->getTrainQueue();
    auto& queueTimers = _currentCamp->getQueueTimers();
    if (index < 0 || index >= trainQueue.size()) return;

    trainQueue.erase(trainQueue.begin() + index);
    queueTimers.erase(queueTimers.begin() + index);
}
// 刷新训练队列UI
void VillageScene::refreshTrainQueueUI() {
    if (!_currentCamp) return;

    auto queuePanel = _trainingPopup->getChildByTag(1003);
    if (!queuePanel) return;

    // 清空原有队列项
    queuePanel->removeAllChildrenWithCleanup(true);
    // 重新添加队列标题
    auto queueTitle = Label::createWithTTF("训练队列", "fonts/Marker Felt.ttf", 20);
    queueTitle->setPosition(Vec2(queuePanel->getContentSize().width / 2, queuePanel->getContentSize().height - 15));
    queueTitle->setColor(Color3B::BLACK);
    queuePanel->addChild(queueTitle);

    // 获取训练营队列数据
    auto& trainQueue = _currentCamp->getTrainQueue();
    auto& queueTimers = _currentCamp->getQueueTimers();

    // 遍历队列创建UI项
    const float itemWidth = 500;
    const float itemHeight = 40;
    const float startY = queuePanel->getContentSize().height - 40;
    for (int i = 0; i < trainQueue.size(); ++i) {
        TroopType type = trainQueue[i];
        float remainTime = queueTimers[i];
        const auto& config = g_troopTrainConfig.at(type);

        // 队列项容器
        auto queueItem = ui::Layout::create();
        queueItem->setContentSize(Size(itemWidth, itemHeight));
        queueItem->setPosition(Vec2(queuePanel->getContentSize().width / 2 - itemWidth / 2, startY - i * (itemHeight + 5)));
        queueItem->setBackGroundColor(Color3B::WHITE);
        queueItem->setBackGroundColorType(ui::Layout::BackGroundColorType::SOLID);
        queueItem->setBackGroundColorOpacity(200);
        queuePanel->addChild(queueItem);

        // 兵种图标
        auto troopIcon = Sprite::create(config.imgPath);
        troopIcon->setScale(0.6f);
        troopIcon->setPosition(Vec2(30, itemHeight / 2));
        queueItem->addChild(troopIcon);

        // 倒计时文本
        auto timerLabel = Label::createWithTTF(StringUtils::format("剩余: %.1fs", remainTime),
            "fonts/Marker Felt.ttf", 18);
        timerLabel->setPosition(Vec2(itemWidth / 2, itemHeight / 2));
        timerLabel->setColor(Color3B::BLACK);
        queueItem->addChild(timerLabel);

        // 取消按钮（减号）
        auto cancelBtn = ui::Button::create("ui/btn_minus.png");
        cancelBtn->setScale(0.5f);
        cancelBtn->setPosition(Vec2(itemWidth - 30, itemHeight / 2));
        cancelBtn->addClickEventListener([this, i](Ref*) {
            // 返还资源 + 移除队列项
            this->refundTroopResource(_currentCamp->getTrainQueue()[i]);
            this->removeTroopFromQueue(i);
            this->refreshTrainQueueUI();
            });
        queueItem->addChild(cancelBtn);
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
    this->addChild(menu, 100);
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
void VillageScene::spawnTroop(Vec2 screenPos, TroopType type) {
    // ===== 第一步：计算瓦片坐标 =====
    Vec2 tilePos = screenToIsoTile(screenPos);
    tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));

    // 调试日志
    CCLOG("adasdasdasvs生成：屏幕坐标(%.1f,%.1f) → 瓦片坐标(%.1f,%.1f)",
        screenPos.x, screenPos.y, tilePos.x, tilePos.y);

    // ===== 第二步：可放置检测 =====
    // 注意：兵种检测应该使用checkCanSpawnTroop而不是checkCanPlace
    if (!checkCanSpawnTroop(tilePos)) {
        showCannotPlaceTip(screenPos);
        CCLOG("failure：1111111");
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
   // CCLOG("troop种坐标转换：瓦片(%.1f,%.1f) → 容器(%.1f,%.1f)",
       // centerTilePos.x, centerTilePos.y, containerLocalPos.x, containerLocalPos.y);

    // ===== 第四步：创建兵种并设置位置 =====
    BaseTroop* troop = BaseTroop::create(type, tilePos, 1.0f);
    if (!troop) {
        // 兜底创建纯色占位
       // CCLOG("BaseTroop创建失败，创建纯色占位");
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
        //CCLOG("成功：纯色占位已生成");
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
   // CCLOG("success：兵种已生成，容器位置(%.1f,%.1f)，总数=%zu",
        //containerLocalPos.x, containerLocalPos.y, _spawnedTroops.size());
    if (_Mode == Mode::FIGHT) {
        _enemyTroops.push_back(troop);
    }    // 新增的寻路部分
    addEnemyTroop(troop);
    // 1. 绑定场景指针（让兵种能访问寻路相关接口）
    troop->setVillageScene(this);  // 需要在 BaseTroop 中声明该方法

    // 2. 查找最近的敌方建筑作为目标
    BaseBuilding* targetBuilding = findNearestEnemyBuilding(containerLocalPos);
    if (targetBuilding) {
        // 3. 触发寻路（使用兵种已实现的 setTargetWorldPosition 方法）
        CCLOG("troop is %s", troop ? "valid" : "null");
        troop->setTargetWorldPosition(targetBuilding->getPosition());
        CCLOG("YEoS!!!!!!!");
        CCLOG("为兵种设置寻路目标，目标building位置(%.1f,%.1f)",
            targetBuilding->getPosition().x, targetBuilding->getPosition().y);
    }
    else {
        CCLOG("未找到敌方建筑，兵种进入lazy状态");
    }
}
void VillageScene::removeEnemyTroop(BaseTroop* troop) {
    auto it = std::find(_enemyTroops.begin(), _enemyTroops.end(), troop);
    if (it != _enemyTroops.end()) {
        _enemyTroops.erase(it); // 从列表中移除失效指针
    }
    auto it1 = std::find(_spawnedTroops.begin(), _spawnedTroops.end(), troop);
    if (it1 != _spawnedTroops.end()) {
        _spawnedTroops.erase(it1); // 从列表中移除失效指针
    }
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

    // 初始化资源数值（可以从存档或服务器获取）
    _gold = 1000;      // 示例：初始金币
    _elixir = 500;     // 示例：初始圣水

    // 金币显示
    // 金币图标
    _goldIcon = Sprite::create("ui/icon_gold.png");
    if (_goldIcon) {
        _goldIcon->setScale(0.8f);
        _goldIcon->setPosition(Vec2(70, visibleSize.height - 30));
        _uiLayer->addChild(_goldIcon, 1);
    }

    // 金币标签
    _goldLabel = Label::createWithTTF(StringUtils::format("%d", _gold),
        "fonts/Marker Felt.ttf", 24);
    if (_goldLabel) {
        _goldLabel->setAnchorPoint(Vec2(0, 0.5f));
        _goldLabel->setPosition(Vec2(90, visibleSize.height - 30));
        _goldLabel->setColor(Color3B::YELLOW);
        _goldLabel->enableOutline(Color4B::BLACK, 2); // 添加黑色描边，提高可读性
        _uiLayer->addChild(_goldLabel, 1);
    }

    // 圣水显示（在金币右侧）
    // 圣水图标
    _elixirIcon = Sprite::create("ui/icon_elixir.png");
    if (_elixirIcon) {
        _elixirIcon->setScale(0.8f);
        _elixirIcon->setPosition(Vec2(200, visibleSize.height - 30));
        _uiLayer->addChild(_elixirIcon, 1);
    }

    // 圣水标签
    _elixirLabel = Label::createWithTTF(StringUtils::format("%d", _elixir),
        "fonts/Marker Felt.ttf", 24);
    if (_elixirLabel) {
        _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
        _elixirLabel->setPosition(Vec2(220, visibleSize.height - 30));
        _elixirLabel->setColor(Color3B::MAGENTA); // 紫色表示圣水
        _elixirLabel->enableOutline(Color4B::BLACK, 2);
        _uiLayer->addChild(_elixirLabel, 1);
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
    _uiLayer->addChild(menu, 2);
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
// 仓库容量
void VillageScene::addGoldStorageCapacity(int bonus) {
    _maxGold += bonus;
}
void VillageScene::addElixirStorageCapacity(int bonus) {
    _maxElixir += bonus;
}
// 部队容量相关方法
void VillageScene::addTroopCapacity(int bonus) {
    _maxPopulation += bonus;
}

// 移除部队容量
void VillageScene::removeTroopCapacity(int bonus) {
    _maxPopulation = _maxPopulation - bonus;
}
// 显示资源不足提示
void VillageScene::showResourceShortageTip(const std::string& message) {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto tip = Label::createWithTTF(message, "fonts/Marker Felt.ttf", 20);
    tip->setColor(Color3B::RED);
    tip->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 100));
    this->addChild(tip, 10000);

    // 添加闪烁效果
    auto blink = Blink::create(1.0f, 3);
    auto remove = RemoveSelf::create();
    tip->runAction(Sequence::create(blink, remove, nullptr));
}
//存档相关

// 打包当前场景数据为存档结构(添加建筑时可能需要实现）
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
	CCLOG("Packing save data: gold=%d, elixir=%d, buildings=%zu",
        saveData.gold, saveData.elixir, _buildings.size());
    // 填充所有建筑数据
    for (const auto& building : _buildings) {
        SaveData::Building bData;
        bData.type = building->getType();
        bData.tilePos = building->getTilePos();
        bData.state = building->getState();
        bData.level = building->getLevel();
        saveData.buildings.push_back(bData);
        CCLOG("tile.x: %f, tile.y: %f", bData.tilePos.x, bData.tilePos.y);
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
// 从存档结构恢复场景数据（添加建筑时需要实现）
void VillageScene::unpackSaveData(const SaveData::Village& saveData) {
    // 清空当前场景的旧数据
    for (auto building : _buildings) {
        if (building) {
            building->destroy();
        }
    }
    _buildings.clear();
    _goldMines.clear();
    _elixirCollectors.clear();
    _occupiedTiles.clear();
    // 恢复地图尺寸
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
			//TODO:恢复建造中的建筑有问题，需要额外处理建造进度
            building->setState(bData.state);    // 需给BaseBuilding添加setState方法
            building->setLevel(bData.level);    // 需给BaseBuilding添加setLevel方法

            // 复用placeBuilding中的坐标/锚点/ZOrder逻辑
            auto config = building->getConfig();
            CCLOG("tile.x: %f, tile.y: %f", bData.tilePos.x, bData.tilePos.y);
            Vec2 topLeftTile = bData.tilePos; // 左上角瓦片（基准瓦片）
            Vec2 bottomRightTile = Vec2(
                bData.tilePos.x + config.tileWidth - 1,
                bData.tilePos.y + config.tileHeight - 1
            ); // 右下角瓦片
            Vec2 posTopLeft = isoTileToContainerPos(topLeftTile);
            Vec2 posBottomRight = isoTileToContainerPos(bottomRightTile);

            // 求中点（区域中心点）
            Vec2 containerLocalPos = Vec2(
                (posTopLeft.x + posBottomRight.x) / 2.0f,
                (posTopLeft.y + posBottomRight.y) / 2.0f
            );
            CCLOG("load  containerLocalPos.x: %f, containerLocalPos.y: %f", containerLocalPos.x, containerLocalPos.y);
            building->setAnchorPoint(Vec2(0.5f, 0.5f));
            building->setPosition(containerLocalPos);
            building->setLocalZOrder(1000 + (bData.tilePos.x + bData.tilePos.y));

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
            else if (bData.type == BuildingType::ELIXIR_COLLECTOR) {
                _elixirCollectors.push_back(dynamic_cast<ElixirCollector*>(building));
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
            this->gotoLevel1("level1_preset.txt");
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
    closeBtn->setPosition(Vec2(200, 0));
    closeBtn->setScale(0.8f);

    // 创建菜单
    auto menu = Menu::create(level1Btn, level2Btn, level3Btn, closeBtn, nullptr);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _levelSelectLayer->addChild(menu);

    // 如果图片不存在，创建文字标签作为备选
    if (!level1Btn->getNormalImage()) {
        auto label1 = Label::createWithTTF("关卡 1: 新手训练", "fonts/Marker Felt.ttf", 28);
        label1->setColor(Color3B::WHITE);
        label1->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2));
        _levelSelectLayer->addChild(label1);

        auto label2 = Label::createWithTTF("关卡 2: 丛林之战", "fonts/Marker Felt.ttf", 28);
        label2->setColor(Color3B::WHITE);
        label2->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
        _levelSelectLayer->addChild(label2);

        auto label3 = Label::createWithTTF("关卡 3: 最终决战", "fonts/Marker Felt.ttf", 28);
        label3->setColor(Color3B::WHITE);
        label3->setPosition(Vec2(visibleSize.width / 2 + 200, visibleSize.height / 2));
        _levelSelectLayer->addChild(label3);
    }
}

bool VillageScene::level_init()
{
    if (!Scene::init()) return false;
    // 初始化流程


    _mapContainer = Node::create();
    this->addChild(_mapContainer);
    initMap();
    initBuildPreview();

    initTroopPreview();
    go_back_Btn();
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

// 初始化回营按钮
void VillageScene::go_back_Btn() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建关卡选择按钮（左下角）
    auto levelSelectBtn = MenuItemImage::create(
        "ui/btn_blue.png",  // 正常状态图片
        "ui/go_back_bt.png",// 按下状态图片
        [this](Ref* sender) {
            this->go_back("village.txt"); // 点击切换关卡选择菜单
        }
    );

    // 设置按钮大小和位置（左下角）
    levelSelectBtn->setScale(0.8f);
    levelSelectBtn->setPosition(Vec2(100, 100)); // 左下角，距离左边缘和下边缘各50像素

    auto menu = Menu::create(levelSelectBtn, nullptr);
    menu->setPosition(Vec2::ZERO); // Menu 位置设为原点，按钮相对 Menu 定位
    menu->setName("LevelSelectMenu"); // 给 Menu 命名，方便隐藏/显示
    this->addChild(menu, 300); // 添加到场景，层级300（确保在最上层）
    // 初始化关卡选择层状态
    _isLevelSelectShow = false;
    _levelSelectLayer = nullptr;
}

void VillageScene::go_back(const std::string& fileName) {
    std::string fullPath = FileUtils::getInstance()->getWritablePath() + fileName;
    std::string saveStr = FileUtils::getInstance()->getStringFromFile(fullPath);
    auto nextScene = VillageScene::create(); // 假设你使用了 CREATE_FUNC

    // 3. 将数据交给新场景（可以写个成员变量存起来）
    // nextScene->setPendingData(saveStr); 

    // 4. 切换
    auto transition = TransitionFade::create(0.5f, nextScene);
    Director::getInstance()->replaceScene(transition);
}



// 创建关卡1场景

void VillageScene::gotoLevel1(const std::string& Path) {
    hideLevelSelectMenu();
    VillageScene::saveGame();
    // 1. 调用 LevelScene 的工厂函数（返回 Scene*，需强转为 LevelScene*）
    Scene* scene = LevelScene::createWithLevel(Path);
    LevelScene* levelScene = dynamic_cast<LevelScene*>(scene); // 安全强转

    if (levelScene) { // 强转成功才继续
        // 2. 获取文件完整路径（Cocos 自动从 Resources 目录查找）
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(Path);

        if (FileUtils::getInstance()->isFileExist(fullPath)) {
            // 3. 调用 loadGame（无需加 VillageScene::，子类已继承）
            bool success = levelScene->loadGame(Path);

            if (success) {
                CCLOG("成功将关卡数据注入 levelScene: %s", Path.c_str());
            }
            else {
                CCLOGERROR("加载关卡数据失败: %s", Path.c_str());
                return; // 加载失败则不切换场景
            }
        }
        else {
            CCLOGERROR("错误：找不到关卡文件 %s", fullPath.c_str());
            return;
        }

        // 4. 场景切换（确保传入 Scene* 类型）
        auto transition = TransitionSlideInR::create(0.5f, scene);
        Director::getInstance()->replaceScene(transition);
    }
    else {
        CCLOGERROR("LevelScene::createWithLevel 创建实例失败！");
    }
}

// 跳转到关卡2
void VillageScene::gotoLevel2() {
    hideLevelSelectMenu();
    bool success = this->loadGame("level2_preset.txt");
    if (!success) CCLOG("无法加载关卡 2");
}

// 跳转到关卡3
void VillageScene::gotoLevel3() {
    hideLevelSelectMenu();
    bool success = this->loadGame("level3_preset.txt");
    if (!success) CCLOG("无法加载关卡 3");
}
void VillageScene::gotoFight() {
    Scene* villageScene = VillageScene::createScene(BaseMode::FIGHT);
    //场景切换（加淡入淡出动画，提升体验）
    if (!villageScene) {
        CCLOG("FIght创建f失败！");
        return;
    }
    CCLOG("FIGHT创建s成功，准备切换");
    Director::getInstance()->pushScene(villageScene);
}
void VillageScene::backfromFight() {
	Director::getInstance()->popScene();
}
void VillageScene::beginFight() {
    // 创建倒计时显示标签（用于展示剩余时间）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    //有黑边时origin不为0
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    _countDownLabel = Label::createWithSystemFont("2:30", "Arial", 36);
    _countDownLabel->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height - 100)); // 调整显示位置
    _uiLayer->addChild(_countDownLabel,200);
    // 初始化计时变量
    _totalTime = 150.0f; // 2分30秒 = 150秒
    _remainingTime = _totalTime;
    this->schedule(CC_SCHEDULE_SELECTOR(VillageScene::updateCountDown), 0.1f);
    _fightStartBtn->setVisible(false);
}
void VillageScene::updateCountDown(float dt)
{
    CCLOG("%f", _remainingTime);
    // 扣除流逝的时间（dt是定时器两次回调的实际间隔）
    _remainingTime -= dt;
    _countDownLabel->setColor(_remainingTime>=50.0f ? Color3B::GREEN : Color3B::RED);
    // 处理时间小于0的情况（避免负数）
    if (_remainingTime <= 0)
    {
        _remainingTime = 0;
        // 停止定时器（避免重复回调）
        this->unschedule(CC_SCHEDULE_SELECTOR(VillageScene::updateCountDown));
        // 触发战斗结算
        this->onFightSettle();
    }

    // 可选：更新倒计时UI（转换为 分:秒 格式）
    int minutes = (int)_remainingTime / 60;
    int seconds = (int)_remainingTime % 60;
    // 格式化字符串（补0，如显示 02:30、00:05）
    _countDownLabel->setString(StringUtils::format("%02d:%02d", minutes, seconds));
}
void VillageScene::onFightSettle() {

}
// 销毁场景并返回标题界面
void VillageScene::destroyScene() {
    //_eventDispatcher->removeEventListenersForTarget(this);
    auto titleSceneLayer = TitleScene::getInstance();
    if (titleSceneLayer) {
        Scene* titleSceneContainer = dynamic_cast<Scene*>(titleSceneLayer->getParent());
        if (titleSceneContainer) {
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, titleSceneContainer, Color3B::BLACK)) ;
        }
    }
}

void VillageScene::onExit() {
    Scene::onExit();
    // 释放自定义资源：比如定时器、监听器、指针等
    //_eventDispatcher->removeEventListenersForTarget(this);// 移除本层下事件监听器，TODO：未知是否必要
    CCLOG("VillageScene 已退出，准备销毁");
}

void VillageScene::cleanup() {
    Node::cleanup();
    _buildings.clear();
    _goldMines.clear();
    _elixirCollectors.clear();
	_spawnedTroops.clear();
    _enemyTroops.clear();
    _eventDispatcher->removeEventListenersForTarget(this);
    this->removeAllChildrenWithCleanup(true);
    CCLOG("VillageScene 已完全whole清理所有资源");
}