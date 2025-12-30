#include "VillageScene.h"
#include "cocos2d.h"
#include "Building.h"
#include "BuildingPopup.h"
#include "ui/CocosGUI.h" 
#include "Troop.h"
#include "EnumType.h" 
#include "LevelScene.h"
#include "TitleScene.h"
#include "SimpleAudioEngine.h"
using namespace CocosDenshion;
//VillageScene* VillageScene::_instance = nullptr;
bool VillageScene::init()
{
    if (!Scene::init()) return false;
    // 初始化流程
    this->scheduleUpdate();
    _mapContainer = Node::create();
    this->addChild(_mapContainer);
    _uiLayer = ui::Layout::create();
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _uiLayer->setContentSize(visibleSize); // 布局尺寸等于可视区域
    _uiLayer->setPosition(Vec2::ZERO);
    _uiLayer->setLayoutType(ui::Layout::Type::ABSOLUTE); // 绝对定位
    this->addChild(_uiLayer, 200); // 布局层级200
    initMap();
    SimpleAudioEngine::getInstance()->preloadEffect("audio/update.mid");
    initBtns(_baseMode);
    if (_baseMode == BaseMode::FIGHT) {
        initFightScene();
    }
    if (_baseMode != BaseMode::FIGHT) {
        initBuildPreview();
        initResourceBar();
        init_troop_upgrade_ModeBtn();
        _bgmList.push_back("audio/village1.mp3"); 
        _bgmList.push_back("audio/village2.mp3");
        _bgmList.push_back("audio/village3.mp3"); 
        _bgmList.push_back("audio/village4.mp3");
    }
    else {
        _bgmList.push_back("audio/fight1.mp3"); 
        _bgmList.push_back("audio/fight2.mp3");
        _bgmList.push_back("audio/fight3.mp3"); 
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
    if (_baseMode == BaseMode::FIGHT) {
        loadGame("fight.txt",false);
    }
    srand((unsigned int)time(nullptr));
    return true;
}
void VillageScene::playRandomBackgroundMusic() {
    if (_bgmList.empty()) {
        CCLOG("BGM NULL");
        return;
    }

    //生成随机索引，从BGM列表中随机选取一个音频
    int randomIndex = rand() % _bgmList.size(); // 取模运算，确保索引在列表范围内
 std::string randomBgmPath = _bgmList[randomIndex];; // 获取随机BGM路径
    CCLOG("BGM：%s", randomBgmPath.c_str()); // 可选：打印日志，查看当前播放的BGM

    // 播放随机选取的背景音乐（先停止当前播放的BGM，避免叠加）
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    // 预加载该BGM
    SimpleAudioEngine::getInstance()->preloadBackgroundMusic(randomBgmPath.c_str());
    // 循环播放
    SimpleAudioEngine::getInstance()->playBackgroundMusic(randomBgmPath.c_str(), true);
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

// 恢复上一个瓦片的原始颜色
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
// 鼠标按下：开始拖拽/记录位置//放置建筑//抬起建筑
void VillageScene::onMouseDown(Event* event)
{
    //_buildPreview->setTexture("building/town_hall_preview.png");
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
        if (_isBuildBarShow && _Mode == Mode::PLACE_BUILDING && !_isLastMouseLeftButtonDown) {
            Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
            Vec2 tilePos = screenToIsoTile(currentPos);
            tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));
            if (checkCanPlace(tilePos, _selectedBuildingType)) {
                placeBuilding(tilePos, _selectedBuildingType);
                // 可选：放置后不清空建造模式，继续放置同类型建筑
                // _buildMode = BuildMode::NONE;
                // _buildPreview->setVisible(false);
            }
            else {
                showCannotPlaceTip(currentPos);
            }
        }
        // 兵种放置逻辑
        else if (_isTroopBarShow && _Mode == Mode::SPAWN_TROOP && _isLastMouseLeftButtonDown) {
            Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
            spawnTroop(currentPos, _selectedTroopType);
            // 可选：放置后不退出模式，继续生成同类型兵种
            // _Mode = Mode::NONE;
            // _troopPreview->setVisible(false);
        }
        else if (_Mode == Mode::MOVE && !_isAnyBuildSelected && _isLastMouseLeftButtonDown) {
            Vec2 currentMousePos = Vec2(e->getCursorX(), e->getCursorY());
            Vec2 tilePos = screenToIsoTile(currentMousePos);
            for (auto building : _buildings) {
                std::vector<Vec2> tiles = building->getTilePositions();
                auto it = std::find(tiles.begin(), tiles.end(), tilePos);
                if (it != tiles.end()) {
                    // 选中该建筑，显示预览
                    _movingBuilding = building;
                    _selectedBuildingType = building->getType();
                    _buildPreview->setTexture(building->getConfig().imgPath);
                    Vec2 containerLocalPos = _mapContainer->convertToNodeSpaceAR(currentMousePos);
                    _buildPreview->setPosition(containerLocalPos);
                    _buildPreview->setVisible(true);
                    building->setVisible(false);
                    _isAnyBuildSelected = true;
                    // 释放该建筑占用的瓦片（预览时不占用）
                    releaseBuildingTiles(building);
                    break;
                }
            }
        }
        else if (_Mode == Mode::MOVE && _isAnyBuildSelected && _isLastMouseLeftButtonDown) {
            Vec2 currentPos = Vec2(e->getCursorX(), e->getCursorY());
            Vec2 tilePos = screenToIsoTile(currentPos);
            tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));
            if (checkCanPlace(tilePos, _selectedBuildingType)) {

                _isAnyBuildSelected = false;
                _buildPreview->setVisible(false);
                isoTileToContainerPos(tilePos);
                moveBuilding(_movingBuilding, tilePos);
                _movingBuilding->setVisible(true);
                _Mode = Mode::NONE;
            }
            else {
                showCannotPlaceTip(currentPos);
            }
        }
        _isLastMouseLeftButtonDown = true;
    }
    else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
        if (_isBuildBarShow && _Mode == Mode::PLACE_BUILDING) {
            // 右键取消建造模式
            if (_buildPreview) {
                _buildPreview->setVisible(false);
            }
            _Mode = Mode::NONE;
            _isContinuousPlace = false;//自动退出连续放置
        }
        if (_Mode == Mode::SPAWN_TROOP && _isTroopBarShow) {
            if (_troopPreview) {
                _troopPreview->setVisible(false);
            }
            _Mode = Mode::NONE;
        }
        _isLastMouseLeftButtonDown = false;
    }
}
//鼠标松开：结束拖拽
void VillageScene::onMouseUp(Event* event)
{
    _isLastMouseLeftButtonDown = false;
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
// 鼠标移动：处理拖拽偏移/建筑预览跟随
void VillageScene::onMouseMove(Event* event)
{
    _isLastMouseLeftButtonDown = false;
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
        if ((_Mode == Mode::PLACE_BUILDING || _Mode == Mode::MOVE) && _buildPreview->isVisible()) {
            Vec2 tilePos = screenToIsoTile(currentMousePos);
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

            _lastTile = tilePos;
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
    auto tip = Label::createWithTTF("No Place Here!", "fonts/Marker Felt.ttf", 20);
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
void VillageScene::showText(std::string string, Vec2 pos, float duringTime) {
    auto tip = Label::createWithTTF(string, "fonts/Marker Felt.ttf", 20);
    tip->setColor(Color3B::RED);
    tip->setPosition(pos + Vec2(0, 30));
    this->addChild(tip, 20);

    // duringTime秒后自动消失
    tip->runAction(Sequence::create(
        DelayTime::create(duringTime),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));
}
// 滚轮缩放核心函数
void VillageScene::onMouseScroll(Event* event)
{
    _isLastMouseLeftButtonDown = false;
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

Scene* VillageScene::createScene(BaseMode mode)
{
    auto scene = Scene::create();
    if (!scene) {
        return nullptr;
    }

    //手动新建 VillageScene 对象（不依赖 CREATE_FUNC，掌控初始化顺序）
    VillageScene* layer = new (std::nothrow) VillageScene();
    if (layer) {
        // 3. 关键：先设置 BaseMode（此时未调用 init()，参数有效）
        layer->setBaseMode(mode); // 设置模式

        // 4. 再手动调用 init()（此时 _baseMode 已赋值，init 中可识别模式）
        if (layer->init()) {
            layer->autorelease(); // 加入自动释放池，兼容 Cocos 内存管理
            scene->addChild(layer);
            if (mode != BaseMode::FIGHT) {
                layer->setTag(100);//便于getInstance获取
                // }
            }
            else {
                layer->setTag(25);
            }
        }
        else {
            // 初始化失败，释放资源避免内存泄漏
            delete layer;
            layer = nullptr;
            scene = nullptr;
        }
    }
    else {
        delete scene;
        scene = nullptr;
    }
    return scene;
}
bool VillageScene::loadMap(const std::string& mapPath) {
    // 销毁旧地图（如果存在）
    if (_tileMap) {
        _tileMap->removeFromParentAndCleanup(true); // 彻底销毁旧地图
        _tileMap = nullptr;
        _bgLayer = nullptr;
    }

    // 加载新地图
    _tileMap = TMXTiledMap::create(mapPath);
    if (!_tileMap) { // 加载失败容错
        CCLOG("地图加载失败：%s", mapPath.c_str());
        return false;
    }

    // 重置地图相关参数（和原initMap逻辑一致）
    _tileSize = _tileMap->getTileSize();
    _mapSize = _tileMap->getMapSize();
    _bgLayer = _tileMap->getLayer("bg_layer"); // 确保新地图也有bg_layer层

    // 地图在容器内的锚点和位置（保持和原逻辑一致）
    _tileMap->setAnchorPoint(Vec2(0.5f, 0.5f));
    _tileMap->setPosition(Vec2::ZERO);
    _mapContainer->addChild(_tileMap, 0);

    CCLOG("地图加载成功：%s", mapPath.c_str());
    return true;
}
void VillageScene::initMapContainer() {
    _mapContainer = Node::create();
    this->addChild(_mapContainer, 0);

    // 容器居中（原initMap中的居中逻辑）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    _mapContainer->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
}
// 加载等轴测地图
void VillageScene::initMap()
{
    initMapContainer(); // 初始化容器
    loadMap("map/map1.tmx"); // 加载默认地图
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
                onFightSettle();
            });
    }
    else {
        _backBtn->addClickEventListener([this](Ref* sender) {  
            // 点击回调：销毁当前场景，返回主菜单
            //SimpleAudioEngine::getInstance()->stopBackgroundMusic();
            this->destroyScene();
            });
    }

    if (_baseMode == BaseMode::CREATING || _baseMode == BaseMode::FIGHT) {
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
        if (_baseMode == BaseMode::FIGHT) {
            _troopModeBtn->setVisible(false);
        }
    }
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
        // 创建一键收集按钮
        _collectAllBtn = ui::Button::create(
            "ui/collect_all_btn_normal.png",
            "ui/collect_all_btn_normal.png"
        );
        _uiLayer->addChild(_collectAllBtn, 200);
        _collectAllBtn->setScale(0.8f);
        _collectAllBtn->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + visibleSize.height - 200));
        _collectAllBtn->addClickEventListener([this](Ref* sender) {
            this->collectAllResources(); // 点击收集所有资源
            });
        CCLOG("canrun：%s", _collectAllBtn ? "yes" : "no");
        CCLOG("width=%.2f, height=%.2f", _collectAllBtn->getContentSize().width, _collectAllBtn->getContentSize().height);
        CCLOG("cansee：%s", _collectAllBtn->isVisible() ? "yes" : "no");
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

        _moveBtn = ui::Button::create(
            "ui/btn_normal.png",   // 正常状态图片（替换为你的资源路径）
            "ui/btn_pressed.png"  // 按下状态图片
        );
        _uiLayer->addChild(_moveBtn, 200);
        _moveBtn->setContentSize(Size(120, 60));
        _moveBtn->setPosition(Vec2(_loadBtn->getPositionX(), _loadBtn->getPositionY() - 80));
        _moveBtn->addClickEventListener([this](Ref* sender) {
            _Mode = Mode::MOVE;
            });
        auto moveText = ui::Text::create("move", "fonts/Marker Felt.ttf", 24);
        moveText->setColor(Color3B::GREEN);
        _moveBtn->addChild(moveText);

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
            setTroopModeBtnInvisible();
            this->beginFight(); // 开始战斗
            _fightStartBtn->setVisible(false);
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
    Vec2 localCenter = basePos + Vec2(_tileSize.width / 2.0f
        , _tileSize.height / 2.0f
    );
    CCLOG("localCenter.x: %f, basePos.y: %f", localCenter.x, localCenter.y);
    // 3. 将 Layer 内部坐标转换为容器 (_mapContainer) 的坐标
    // 考虑到你可能有多个 Layer 或者 Layer 做了偏移，用转换函数最安全
    return _mapContainer->convertToNodeSpace(_bgLayer->convertToWorldSpace(localCenter));
}

void VillageScene::init_level_Btns(BaseMode baseMode) {
    //获取可视区域
    Size visibleSize = Director::getInstance()->getVisibleSize();
    //有黑边时origin不为0
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    //origin.x, origin.y为左下角
    // origin.x+visibleSize.width最右边
    // origin。y+visibleSize.heigh
    if (baseMode == BaseMode::LEVEL1) {
        if (_troopModeBtn) {
            _troopModeBtn->setVisible(true);
        }
        else {
            initTroopModeBtn(); // 如果没初始化，先初始化
        }
    }


    if (baseMode == BaseMode::LEVEL1) {
        _fightStartBtn = ui::Button::create("ui/fight_start_btn.png");
        _uiLayer->addChild(_fightStartBtn, 200);
        _fightStartBtn->setPosition(Vec2(origin.x + visibleSize.width - 100, origin.y + 100));
        _fightStartBtn->addClickEventListener([this](Ref* sender) {
            // 点击开始战斗后，隐藏掉选兵按钮
            if (_troopModeBtn) _troopModeBtn->setVisible(true);
            this->beginFight();
            });
    }
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
     _barBg = Sprite::create("ui/build_bar_bg.png");
    _barBg->setPosition(Vec2(visibleSize.width / 2, 50));
    _barBg->setScaleX(visibleSize.width / _barBg->getContentSize().width * 0.8f);
    _buildBarLayer->addChild(_barBg);

    // 建筑按钮 - 大本营
     _townHallBtn = MenuItemImage::create(
        "building/town_hall_icon.png",
        "building/town_hall_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall != nullptr) {
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::TOWN_HALL;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/town_hall_preview.png");
        }
    );
     if (_townHallBtn == nullptr) {
         CCLOG("ERROR: Failed to create _townHallBtn! Check image path: building/town_hall_icon.png");
         // 可以在这里做一些容错处理，比如创建一个纯色块代替，或者直接 return 防止后续崩溃
     }
     else {
         CCLOG("Successfully created _townHallBtn. Size: %f, %f",
             _townHallBtn->getContentSize().width,
             _townHallBtn->getContentSize().height);
     }


    // 建筑按钮 - 金矿
     _goldMineBtn = MenuItemImage::create(
        "building/gold_mine_icon.png",
        "building/gold_mine_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall==nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::GOLD_MINE;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/gold_mine_preview.png");
        }
    );
    // 建筑按钮 - 圣水收集器
     _elixirCollectorBtn = MenuItemImage::create(
        "building/elixir_collector_icon.png",
        "building/elixir_collector_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ELIXIR_COLLECTOR;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/elixir_collector_preview.png");
        }
    );
    // 建筑按钮 - 军营
     _barracksBtn = MenuItemImage::create(
        "building/barracks_icon.png",
        "building/barracks_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::BARRACKS;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/barracks_preview.png");
        }
    );
    // 建筑按钮 - 训练营
    _trainingCampBtn = MenuItemImage::create(
        "building/training_camp_icon.png",
        "building/training_camp_icon_selected.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                showText("Not Enough TownHallLevel");
                return;
            }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::TRAINING_CAMP;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/training_camp_preview.png");
        }
    );
     _cannonBtn = MenuItemImage::create(
        "building/cannon_icon.png",
        "building/cannon_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::CANNON;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/cannon_preview.png");
        }
    );
     _arrowTowerBtn = MenuItemImage::create(
        "building/arrow_tower_icon.png",
        "building/arrow_tower_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL &&( _townHall==nullptr|| _townHall->getLevel()<2)) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ARROW_TOWER;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/arrow_tower_preview.png");
        }
    );
     _wallBtn = MenuItemImage::create(
        "building/wall_icon.png",
        "building/wall_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::WALL;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/wall_preview.png");
        }
    );
     _elixirBottleBtn = MenuItemImage::create(
        "building/elixir_bottle_icon.png",
        "building/elixir_bottle_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::ELIXIR_BOTTLE;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/elixir_bottle_preview.png");
        }
    );
     _vaultBtn = MenuItemImage::create(
        "building/vault_icon.png",
        "building/vault_icon_selected.png",
        [this](Ref* sender) {
             if (_baseMode == BaseMode::NORMAL && _townHall == nullptr) {
                 showText("Not Enough TownHallLevel");
                 return;
             }
            _Mode = Mode::PLACE_BUILDING;
            _selectedBuildingType = BuildingType::VAULT;
            _buildPreview->setVisible(true);
            _buildPreview->setTexture("building/vault_preview.png");
        }
    );
    // 连续放置按钮（仅退出当前建造模式，不隐藏建筑栏）
     _cancelPlaceBtn = MenuItemImage::create(
        "ui/cancel_place_btn.png",
        "ui/cancel_place_btn_selected.png",
        [this](Ref* sender) {
            _isContinuousPlace = !_isContinuousPlace; 
        }
    );
     auto createSplitCostLabels = [](MenuItemImage* btn, int goldCost, int elixirCost) {
         if (!btn) return; 
         Size btnSize = btn->getContentSize();
         float offsetY = 20.0f; 
         std::string goldText = StringUtils::format("gold: %d", goldCost);
         Label* goldLabel = Label::createWithSystemFont(goldText, "Marker Felt.ttf", 16);
         goldLabel->setColor(Color3B::YELLOW); 
         goldLabel->enableOutline(Color4B::BLACK, 1); 
         goldLabel->setPosition(Vec2(btnSize.width / 2, btnSize.height + offsetY*2));
         btn->addChild(goldLabel); 

         std::string elixirText = StringUtils::format("elixir: %d", elixirCost);
         Label* elixirLabel = Label::createWithSystemFont(elixirText, "Marker Felt.ttf", 16);
         elixirLabel->setColor(Color3B::MAGENTA); 
         elixirLabel->enableOutline(Color4B::BLACK, 1); 
         elixirLabel->setPosition(Vec2(btnSize.width / 2, btnSize.height + offsetY));
         btn->addChild(elixirLabel); 
         };
     createSplitCostLabels(_goldMineBtn, getBuildingConfigByType(BuildingType::GOLD_MINE).cost.find("gold")->second, getBuildingConfigByType(BuildingType::GOLD_MINE).cost.find("elixir")->second);
     createSplitCostLabels(_elixirCollectorBtn, getBuildingConfigByType(BuildingType::ELIXIR_COLLECTOR).cost.find("gold")->second, getBuildingConfigByType(BuildingType::ELIXIR_COLLECTOR).cost.find("elixir")->second);
     createSplitCostLabels(_barracksBtn, getBuildingConfigByType(BuildingType::BARRACKS).cost.find("gold")->second, getBuildingConfigByType(BuildingType::BARRACKS).cost.find("elixir")->second);
     createSplitCostLabels(_trainingCampBtn, getBuildingConfigByType(BuildingType::TRAINING_CAMP).cost.find("gold")->second, getBuildingConfigByType(BuildingType::TRAINING_CAMP).cost.find("elixir")->second);
     createSplitCostLabels(_arrowTowerBtn, getBuildingConfigByType(BuildingType::ARROW_TOWER).cost.find("gold")->second, getBuildingConfigByType(BuildingType::ARROW_TOWER).cost.find("elixir")->second);
     createSplitCostLabels(_cannonBtn, getBuildingConfigByType(BuildingType::CANNON).cost.find("gold")->second, getBuildingConfigByType(BuildingType::CANNON).cost.find("elixir")->second);
     createSplitCostLabels(_wallBtn, getBuildingConfigByType(BuildingType::WALL).cost.find("gold")->second, getBuildingConfigByType(BuildingType::WALL).cost.find("elixir")->second);
     createSplitCostLabels(_elixirBottleBtn, getBuildingConfigByType(BuildingType::ELIXIR_BOTTLE).cost.find("gold")->second, getBuildingConfigByType(BuildingType::ELIXIR_BOTTLE).cost.find("elixir")->second);
     createSplitCostLabels(_vaultBtn, getBuildingConfigByType(BuildingType::VAULT).cost.find("gold")->second, getBuildingConfigByType(BuildingType::VAULT).cost.find("elixir")->second);

    // 排列按钮
    auto menu = Menu::create(_townHallBtn, _goldMineBtn, _elixirCollectorBtn, _barracksBtn, _trainingCampBtn, _cannonBtn, _arrowTowerBtn, _wallBtn, _elixirBottleBtn, _vaultBtn, _cancelPlaceBtn, nullptr);
    menu->alignItemsHorizontallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width / 2, 50));
    _buildBarLayer->addChild(menu);
    if (_baseMode == BaseMode::NORMAL) {
        checkCanGetBuilding();
    }
}

void VillageScene::checkCanGetBuilding() {
    if (!_townHall) {
         _goldMineBtn->setColor(Color3B::GRAY);
         _elixirCollectorBtn->setColor(Color3B::GRAY);
         _barracksBtn->setColor(Color3B::GRAY);
         _trainingCampBtn->setColor(Color3B::GRAY);
         _cannonBtn->setColor(Color3B::GRAY);
         _wallBtn->setColor(Color3B::GRAY);
         _elixirBottleBtn->setColor(Color3B::GRAY);
         _vaultBtn->setColor(Color3B::GRAY);
         _arrowTowerBtn->setColor(Color3B::GRAY);
    }
    else if(_townHall->getLevel() == 1) {
        _townHallBtn->setColor(Color3B::GRAY);
        _goldMineBtn->setColor(Color3B::WHITE);
        _elixirCollectorBtn->setColor(Color3B::WHITE);
        _barracksBtn->setColor(Color3B::WHITE);
        _trainingCampBtn->setColor(Color3B::WHITE);
        _cannonBtn->setColor(Color3B::WHITE);
        _wallBtn->setColor(Color3B::WHITE);
        _elixirBottleBtn->setColor(Color3B::WHITE);
        _vaultBtn->setColor(Color3B::WHITE);
    }
    else if (_townHall->getLevel() == 2) {
        _arrowTowerBtn->setColor(Color3B::WHITE);
    }
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
        createInfoPanel(building);
        if (building->getConfig().type == BuildingType::ARROW_TOWER || building->getConfig().type == BuildingType::CANNON) {
            auto attackBuilding = dynamic_cast<BaseAttackBuilding*>(building);
            attackBuilding->showAttackRange(attackBuilding->getIsShow()?false:true);
        }
        break;

    case BuildingPopup::ButtonType::UPGRADE:
        // 建筑升级逻辑（调用BaseBuilding的升级方法）
        if (building->getState() == BuildingState::IDLE) { // 仅闲置状态可升级
            if (_baseMode == BaseMode::NORMAL) {
                if ((building->getLevel()+1) >_maxLevel) {
                    showText("Not Enough TownHallLevel");
                    return;
                }
                if (_gold <= getGoldCost(building->getType(), building->getLevel() + 1) && _elixir <= getElixirCost(building->getType(), building->getLevel() + 1)) {
                    showText("Not Enough Money And Elixir");
                    return;
                }
                else if (_gold <= getGoldCost(building->getType(), building->getLevel() + 1)) {
                    showText("Not Enough Money");
                    return;
                }
                else if (_elixir <= getElixirCost(building->getType(), building->getLevel() + 1)) {
                    showText("Not Enough Elixir");
                    return;
                }
                else {
                    spendGold(getGoldCost(building->getType(), building->getLevel() + 1));
                    spendElixir(getElixirCost(building->getType(), building->getLevel() + 1));
                }
            }
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
        if (_baseMode == BaseMode::NORMAL&& building->getConfig().type==BuildingType::TOWN_HALL) {
            showText("you can not destroy your town hall!!!");
        }
        else {
            destroyBuilding(building);
        }
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
void VillageScene::moveBuilding(BaseBuilding* building, Vec2 tilePos) {
    auto config = building->getConfig();
    building->clearTilePos();
    // 记录该建筑占用的所有瓦片
    for (int x = 0; x < config.tileWidth; ++x) {
        for (int y = 0; y < config.tileHeight; ++y) {
            addOccupiedTile(Vec2(tilePos.x + x, tilePos.y + y));
            building->setTilePos(Vec2(tilePos.x + x, tilePos.y + y));
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
    building->setAnchorPoint(Vec2(0.5f, 0.5f));
    building->setPosition(containerLocalPos);
}
// 放置建筑（新增建筑类型可能需要扩展此函数）
void VillageScene::placeBuilding(Vec2 tilePos, BuildingType type) {
    if (_baseMode == BaseMode::NORMAL && type != BuildingType::TOWN_HALL) {
        if (_gold <= getGoldCost(type) && _elixir <= getElixirCost(type)) {
            showText("Not Enough Money And Elixir");
            return;
        }
        else if (_gold <= getGoldCost(type)) {
            showText("Not Enough Money");
            return;
        }
        else if (_elixir <= getElixirCost(type)) {
            showText("Not Enough Elixir");
            return;
        }
        else {
            spendGold(getGoldCost(type));
            spendElixir(getElixirCost(type));
        }
    }
    auto building = BaseBuilding::create(type, tilePos, 1.0f);
    if (_baseMode == BaseMode::NORMAL && type == BuildingType::TOWN_HALL) {
        if (!_townHall) {
            _townHall = building;
            building->setBuildTime(0.1f);
        }
        else {
            showText("To Many TownHall!!!");
            return;
        }
    }
    //创造模式立即建造完成
    if (_baseMode == BaseMode::CREATING) {
        building->buildImmediately();
    }
    if (building) {
        _buildPreview->setVisible(false);
        // 加入建筑总列表（核心：保存实例引用，避免内存泄漏/无法管理）
        _buildings.push_back(building);
        // 按类型加入细分列表
        //TODO: 哥布林攻击金矿，炸弹人攻击城墙等逻辑需要用到这些列表
        if (type == BuildingType::TOWN_HALL) {
            if (_baseMode == BaseMode::NORMAL) {
                _townHall = building;
            }
            auto townHall = dynamic_cast<TownHall*>(building);
            if (townHall) {
                // 绑定建造/升级完成回调
                if (_baseMode == BaseMode::NORMAL) {
                    townHall->bindBuildFinishCallback([this, townHall](BaseBuilding* b) {
                        checkCanGetBuilding();
                        _maxLevel = townHall->getLevel() * 2;
                        });
                }
            }
        }
        else if (type == BuildingType::GOLD_MINE) {
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
                        this->addElixirStorageCapacity(elixirBottle->getStoragePulse());
                    }
                    // 增加新的容量加成
                    else {
                        this->addElixirStorageCapacity(elixirBottle->getStorageCapacity());
                    }
                    });
            }
        }
        else if (type == BuildingType::TRAINING_CAMP) {
            auto trainingCamp = dynamic_cast<TrainingCamp*>(building);
            trainingCamp->setTrainFinishCallback(
                [this](TroopType type) { // 仅1个参数
                    this->onTroopTrainFinished(type);
                }
            );
        }
        auto config = building->getConfig();

        // 记录该建筑占用的所有瓦片
        for (int x = 0; x < config.tileWidth; ++x) {
            for (int y = 0; y < config.tileHeight; ++y) {
                addOccupiedTile(Vec2(tilePos.x + x, tilePos.y + y));
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
        if (building && _baseMode != BaseMode::FIGHT) {
            // 统一绑定点击回调（弹窗逻辑）
            building->bindClickCallback([this](BaseBuilding* building) {
                if (_Mode != Mode::NONE) {
                    // 非 NONE 模式，直接返回（不触发任何交互）
                    return;

                }
                SimpleAudioEngine::getInstance()->playEffect("audio/update.mid", false);
                
                // 弹出功能窗口
                auto popup = BuildingPopup::create(building, [this, building](BuildingPopup::ButtonType type) {
                    handleBuildingBtnClick(building, type);
                    });
                this->addChild(popup, 100); // 高层级显示弹窗
                });
        }

        // 1. 播放建造音效
        // 2. 延迟0.5秒切换回NONE模式(放置点击触碰到其他建筑会触发弹窗)
        if (!_isContinuousPlace) {
            _Mode = Mode::PROTECT;
            this->scheduleOnce([this](float delay) {
                _Mode = Mode::NONE;
                }, 0.1f, "delay_switch_to_none_mode"); // 0.1秒延迟，定时器标签用于防重复
        }
        else {
            _buildPreview->setVisible(true);
        }

    }

}
void VillageScene::addOccupiedTiles(const std::vector<Vec2>& tiles) {
    for (auto tile : tiles) {
        _occupiedTiles.push_back(Vec2(tile.x, tile.y));
        int tileX = static_cast<int>(tile.x);
        int tileY = static_cast<int>(tile.y);
        _tileOccupiedGrid[tileX][tileY] = true;
    }
}
void VillageScene::addOccupiedTile(const Vec2& tile) {
    _occupiedTiles.push_back(Vec2(tile.x, tile.y));
    int tileX = static_cast<int>(tile.x);
    int tileY = static_cast<int>(tile.y);
    _tileOccupiedGrid[tileX][tileY] = true;

}
// 摧毁建筑（新增建筑类型可能需要扩展此函数）
// [VillageScene.cpp]
void VillageScene::destroyBuilding(BaseBuilding* building) {
    if (!building) return;

    // 1. 【关键步骤】先从 _buildings 总列表中移除
    // 这样后续兵种调用 findNewTarget -> findNearestEnemyBuilding 时，
    // 就不会再搜索到这个即将死亡的建筑，而是找到第二近的。
    auto it = std::find(_buildings.begin(), _buildings.end(), building);
    if (it != _buildings.end()) {
        _buildings.erase(it);
    }

    // 2. 处理分类列表移除 (保持你原有的逻辑不变)
    if (building->getType() == BuildingType::ELIXIR_COLLECTOR) {
        auto it1 = std::find(_elixirCollectors.begin(), _elixirCollectors.end(), dynamic_cast<ElixirCollector*>(building));
        if (it1 != _elixirCollectors.end()) _elixirCollectors.erase(it1);
    }
    else if (building->getType() == BuildingType::GOLD_MINE) {
        auto it1 = std::find(_goldMines.begin(), _goldMines.end(), dynamic_cast<GoldMine*>(building));
        if (it1 != _goldMines.end()) _goldMines.erase(it1);
    }
    else if (building->getType() == BuildingType::BARRACKS) {
        if (_baseMode != BaseMode::CREATING) {
            auto barrack = dynamic_cast<Barracks*>(building);
            this->removeTroopCapacity(barrack->getTroopSpace());
        }
    }
    else if (building->getType() == BuildingType::VAULT) {
        auto vault = dynamic_cast<Vault*>(building);
        this->addGoldStorageCapacity(-(vault->getStorageCapacity()));
    }
    else if (building->getType() == BuildingType::ELIXIR_BOTTLE) {
        auto elixirBottle = dynamic_cast<ElixirBottle*>(building);
        this->addElixirStorageCapacity(-(elixirBottle->getStorageCapacity()));
    }

    // 3. 【核心修复】通知兵种寻找新目标
    // 之前你在这里还有一个循环先把 target 设为 nullptr，必须删掉那个循环！
    // 直接检查谁的目标是当前建筑，然后命令它找新的。
    if (!_spawnedTroops.empty()) {
        for (auto troop : _spawnedTroops) {
            // 只有兵种存在，且当前锁定的目标就是这个正在销毁的建筑时
            if (troop && troop->getAttackTarget() == building) {
                // 命令它寻找新目标
                // findNewTarget 内部会自动调用 setAttackTarget(new)
                // 如果找不到新目标，内部也会自动设为 nullptr 并 IDLE
                troop->findNewTarget();
            }
        }
    }

    // 4. 释放资源和物理销毁
    releaseBuildingTiles(building);
    building->destroy();

    // 5. 战斗统计更新 (保持不变)
    if (_baseMode == BaseMode::FIGHT) {
        _destroyedBuildingCount++;
        updateDestroyPercent();
        checkStarUnlock();
    }
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
                int tileX = static_cast<int>(tile.x);
                int tileY = static_cast<int>(tile.y);
                _tileOccupiedGrid[tileX][tileY] = false;
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
void VillageScene::collectAllResources() {
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
        this->refreshTrainQueueUI();
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
    //unschedule("updateTrainQueueTimerKey");//隐藏不代表训练停止
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
    //TODO:兵种基本确定之后添加NORMAL限定
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
        int spaceCost = config.spaceCost;
        // 等级不足则跳过
        if (config.unlockCampLevel > campLevel) continue;

        // 创建兵种按钮
        auto troopBtn = ui::Button::create(config.imgPath);
        troopBtn->setContentSize(Size(btnSize, btnSize));
        troopBtn->setPosition(Vec2(btnX, btnPanel->getContentSize().height / 2));
        btnPanel->addChild(troopBtn);
        btnX += btnSize + 20;

        // 按钮点击事件
        troopBtn->addClickEventListener([this, type, spaceCost](Ref*) {
            // 检查队列是否已满
            if (_currentCamp->getTrainQueue().size() >= MAX_QUEUE_SIZE) {
                showResourceShortageTip("train queue max!");
                return;
            }
            if (_baseMode == BaseMode::NORMAL) {
                if (_population + spaceCost > _maxPopulation) {
                    showResourceShortageTip("Population Full!");
                    return;
                }
                if (!checkTroopResourceEnough(type)) return;
                deductTroopResource(type);
            }
            // 检查资源

            // 扣除资源 + 添加到队列
            _currentCamp->addTrainTask(type);
            _population += spaceCost;
            //deductTroopResource(type);
           //addTroopToQueue(type);
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
    refreshTrainQueueUI();
    // 遍历更新倒计时
    for (int i = 0; i < trainQueue.size(); ++i) {
        refreshTrainQueueUI();
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
    auto queueTitle = Label::createWithTTF("Train Queue", "fonts/Marker Felt.ttf", 20);
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
        auto timerLabel = Label::createWithTTF(StringUtils::format("remain: %.1fs", remainTime),
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
void VillageScene::onTroopTrainFinished(TroopType type) {
    _trainedTroops[type]+=1;
    setPopulation(_population);
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
        this->hideTroopBar();
        _isTroopBarShow = false;
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
    // 1. 将瓦片坐标强转为整数下标（瓦片坐标本身是取整后的，无精度问题）
    int tileX = static_cast<int>(tilePos.x);
    int tileY = static_cast<int>(tilePos.y);

    // 2. 边界校验（避免数组越界，保障程序稳定性）
    if (tileX < 0 || tileX >= MAX_TILE_X || tileY < 0 || tileY >= MAX_TILE_Y) {
        return false; // 超出场景范围，视为未占用（可按需调整逻辑）
    }

    // 3. 直接返回数组值：纯内存寻址，最快查询（无哈希、无查找、无计算）
    return _tileOccupiedGrid[tileX][tileY];
    /*
    for (const auto& pos : _occupiedTiles) {
        if (pos.equals(tilePos)) {
            return true;
        }
    }
    return false;*/
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
    _uiLayer->addChild(menu); // 最高层级
}
void VillageScene::createTroopBar() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // Troop bar container
    auto troopBarLayer = Layer::create();
    this->addChild(troopBarLayer, 99);
    troopBarLayer->setTag(1001);

    // Background
    auto barBg = Sprite::create("ui/build_bar_bg.png");
    barBg->setPosition(Vec2(visibleSize.width / 2, 50));
    barBg->setScaleX(visibleSize.width / barBg->getContentSize().width * 0.8f);
    troopBarLayer->addChild(barBg);

    // Clear previous UI references to be safe
    _troopCountLabels.clear();
    _troopButtons.clear();

    // Helper lambda to create buttons with counts
    auto createTroopBtn = [this](std::string imgPath, TroopType type) -> MenuItemImage* {
        auto btn = MenuItemImage::create(
            imgPath,
            imgPath,
            [this, type](Ref* sender) {
                // 1. Check if we have troops available (Logic Check)
                if ((_baseMode == BaseMode::FIGHT || _baseMode == BaseMode::LEVEL1) && _trainedTroops[type] <= 0) {
                    showText("No troops left!", Vec2(Director::getInstance()->getVisibleSize() / 2));
                    return;
                }

                _Mode = Mode::SPAWN_TROOP;
                _selectedTroopType = type;

                // Update preview
                if (g_troopTrainConfig.find(type) != g_troopTrainConfig.end()) {
                    _troopPreview->setTexture(g_troopTrainConfig.at(type).imgPath);
                }
                _troopPreview->setVisible(true);
            }
        );
        btn->setScale(0.4f);

        // 2. Add Count Label (UI Display)
        // Only show count in fight/level modes, not in creative/normal if not needed
        if (_baseMode != BaseMode::CREATING) {
            int count = _trainedTroops[type];
            auto label = Label::createWithTTF(std::to_string(count), "fonts/Marker Felt.ttf", 36);
            label->setPosition(Vec2(btn->getContentSize().width / 2, btn->getContentSize().height + 30));
            label->setColor(Color3B::WHITE);
            label->enableOutline(Color4B::BLACK, 2);
            btn->addChild(label);

            // Store references
            _troopCountLabels[type] = label;
            _troopButtons[type] = btn;

            // Initial state check (Grey out if 0)
            if (count <= 0) {
                btn->setColor(Color3B::GRAY);
            }
        }
        return btn;
        };

    // Create Buttons using the helper
    auto barbarianBtn = createTroopBtn("troops/barbarian.png", TroopType::BARBARIAN);
    auto archerBtn = createTroopBtn("troops/archer.png", TroopType::ARCHER);
    auto bomberBtn = createTroopBtn("troops/bomber.png", TroopType::BOMBER);
    auto giantBtn = createTroopBtn("troops/giant.png", TroopType::GIANT);

    // Cancel Button
    auto cancelTroopBtn = MenuItemImage::create(
        "ui/cancel_place_btn.png",
        "ui/cancel_place_btn_selected.png",
        [this](Ref* sender) {
            _Mode = Mode::NONE;
            _troopPreview->setVisible(false);
        }
    );

    // Align Buttons
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
        this->hideBuildBar();
        _isBuildBarShow = false;
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
void VillageScene::updateTroopButtonState(TroopType type) {
    // Check if the label exists in our map
    if (_troopCountLabels.find(type) != _troopCountLabels.end()) {
        int count = _trainedTroops[type];

        // Update text
        _troopCountLabels[type]->setString(std::to_string(count));

        // Update button color (Gray if 0, White if > 0)
        if (_troopButtons.find(type) != _troopButtons.end()) {
            if (count <= 0) {
                _troopButtons[type]->setColor(Color3B::GRAY);
            }
            else {
                _troopButtons[type]->setColor(Color3B::WHITE);
            }
        }
    }
}
// 生成兵种（放置到地图）
void VillageScene::spawnTroop(Vec2 screenPos, TroopType type) {
    // 1. [Logic Check] Validate troop count before anything else
    // Only apply this restriction in Fight/Level modes, not Creative mode
    if (_baseMode == BaseMode::FIGHT || _baseMode == BaseMode::LEVEL1 || _baseMode == BaseMode::LEVEL2 || _baseMode == BaseMode::LEVEL3) {
        if (_trainedTroops[type] <= 0) {
            showCannotPlaceTip(screenPos); // Reuse your existing tip or showText
            return; // Stop execution
        }
    }

    // ===== Step 1: Tile Calculation =====
    Vec2 tilePos = screenToIsoTile(screenPos);
    tilePos = Vec2(floor(tilePos.x), floor(tilePos.y));

    // ===== Step 2: Placement Check =====
    if (!checkCanSpawnTroop(tilePos)) {
        showCannotPlaceTip(screenPos);
        return;
    }

    // ===== Step 3: Create Troop =====
    // ... existing calculation code ...
    float centerTileX = tilePos.x - 0.5f;
    float centerTileY = tilePos.y - 2.0f;
    Vec2 centerTilePos(centerTileX, centerTileY);
    Vec2 containerLocalPos = isoTileToContainerPos(centerTilePos);

    BaseTroop* troop = BaseTroop::create(type, tilePos, 1.0f);
    if (!troop) {
        CCLOG("BaseTroop create failed");
        return;
    }

    // ===== Step 4: Add to Scene =====
    troop->setAnchorPoint(Vec2(0.5f, 0.5f));
    troop->setPosition(containerLocalPos);
    troop->setScale(1.0f);
    troop->setLocalZOrder(1500 - (tilePos.x + tilePos.y));
    _mapContainer->addChild(troop);

    // ... existing logic for adding to _spawnedTroops, _enemyTroops ...
    _spawnedTroops.push_back(troop);
    _enemyTroops.push_back(troop);
    troop->setVillageScene(this);

    // ... existing target finding logic ...
    // (Your existing target finding logic here)
    BaseBuilding* targetBuilding = nullptr;
    if (type == TroopType::BOMBER) {
        std::vector<BuildingType> wallType = { BuildingType::WALL };
        targetBuilding = findNearestBuildingByTypes(containerLocalPos, wallType);
    }
    if (!targetBuilding) {
        BuildingType ignoreType = BuildingType::UNKNOWN;
        if (type == TroopType::ARCHER) ignoreType = BuildingType::WALL;
        targetBuilding = findNearestEnemyBuilding(containerLocalPos, ignoreType);
        if (!targetBuilding && ignoreType == BuildingType::WALL) {
            targetBuilding = findNearestEnemyBuilding(containerLocalPos, BuildingType::UNKNOWN);
        }
    }
    if (targetBuilding) troop->setAttackTarget(targetBuilding);

    // ===== NEW Step 5: Update Count and UI =====
    if (_baseMode != BaseMode::CREATING) { // Don't decrease count in creative mode
        // 1. Decrease Data
        _trainedTroops[type]--;

        // 2. Update UI
        updateTroopButtonState(type);

        // 3. Auto-deselect if empty
        if (_trainedTroops[type] <= 0) {
            _Mode = Mode::NONE;
            if (_troopPreview) _troopPreview->setVisible(false);
        }
    }
}
BaseBuilding* VillageScene::findNearestEnemyBuilding(const Vec2& troopPos, BuildingType ignoreType) {
    BaseBuilding* nearestBuilding = nullptr;
    float minDistance = FLT_MAX;

    // 遍历所有建筑
    for (auto& building : _buildings) {
        // 1. 基础判空和死亡检查
        if (!building || building->getState() == BuildingState::DESTROYED) continue;

        // 2. 【新增】如果是需要忽略的类型（比如弓箭手忽略围墙），直接跳过
        if (building->getType() == ignoreType) {
            continue;
        }

        // 3. 计算距离
        float distance = troopPos.distance(building->getPosition());
        if (distance < minDistance) {
            minDistance = distance;
            nearestBuilding = building;
        }
    }
    return nearestBuilding;
}
// 实现按类型列表搜索最近建筑
BaseBuilding* VillageScene::findNearestBuildingByTypes(const Vec2& troopPos, const std::vector<BuildingType>& targetTypes) {
    BaseBuilding* nearestBuilding = nullptr;
    float minDistance = FLT_MAX;

    for (auto& building : _buildings) {
        if (!building || building->getState() == BuildingState::DESTROYED) continue;

        // 检查建筑类型是否在目标列表中
        bool isTargetType = false;
        for (auto type : targetTypes) {
            if (building->getType() == type) {
                isTargetType = true;
                break;
            }
        }

        // 如果不是目标类型，跳过
        if (!isTargetType) continue;

        // 计算距离
        float distance = troopPos.distance(building->getPosition());
        if (distance < minDistance) {
            minDistance = distance;
            nearestBuilding = building;
        }
    }
    return nearestBuilding;
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
    _gold = 10000;
    _elixir = 10000;
    // 创建资源显示层

    // 初始化资源数值（可以从存档或服务器获取）
    // 金币显示
    // 金币图标
    _goldIcon = Sprite::create("ui/icon_gold.png");
    if (_goldIcon) {
        _goldIcon->setScale(0.8f);
        _goldIcon->setPosition(Vec2(70, visibleSize.height - 30));
        _uiLayer->addChild(_goldIcon, 1);
    }

    // 金币标签
    if (_baseMode == BaseMode::NORMAL) {
        _goldLabel = Label::createWithTTF(StringUtils::format("%d/%d", _gold, _maxGold),
            "fonts/Marker Felt.ttf", 24);
    }
    else {
        _goldLabel = Label::createWithTTF(StringUtils::format("%d", _gold),
            "fonts/Marker Felt.ttf", 24);
    }
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
        _elixirIcon->setPosition(Vec2(280, visibleSize.height - 30));
        _uiLayer->addChild(_elixirIcon, 1);
    }

    // 圣水标签
    if (_baseMode == BaseMode::NORMAL) {
        _elixirLabel = Label::createWithTTF(StringUtils::format("%d/%d", _elixir,_maxElixir),
            "fonts/Marker Felt.ttf", 24);
    }
    else {
        _elixirLabel = Label::createWithTTF(StringUtils::format("%d", _elixir),
            "fonts/Marker Felt.ttf", 24);
    }

    if (_elixirLabel) {
        _elixirLabel->setAnchorPoint(Vec2(0, 0.5f));
        _elixirLabel->setPosition(Vec2(300, visibleSize.height - 30));
        _elixirLabel->setColor(Color3B::MAGENTA); // 紫色表示圣水
        _elixirLabel->enableOutline(Color4B::BLACK, 2);
        _uiLayer->addChild(_elixirLabel, 1);
    }

    _populationIcon = Sprite::create("ui/icon_population.png");
    if (_populationIcon) {
        _populationIcon->setScale(0.8f);
        _populationIcon->setPosition(Vec2(590, visibleSize.height - 30));
        _uiLayer->addChild(_populationIcon, 1);
    }
    if (_baseMode == BaseMode::NORMAL) {
        _populationLabel = Label::createWithTTF(StringUtils::format("%d/%d", _population, _maxPopulation),
            "fonts/Marker Felt.ttf", 24);
    }
    else {
        _populationLabel = Label::createWithTTF(StringUtils::format("%d", _population),
            "fonts/Marker Felt.ttf", 24);
    }

    if (_populationLabel) {
        _populationLabel->setAnchorPoint(Vec2(0, 0.5f));
        _populationLabel->setPosition(Vec2(620, visibleSize.height - 30));
        _populationLabel->setColor(Color3B::RED);
        _populationLabel->enableOutline(Color4B::BLACK, 2);
        _uiLayer->addChild(_populationLabel, 1);
    }

    // 可选：添加资源获取按钮（点击可打开商店等）
    auto addGoldBtn = MenuItemImage::create(
        "ui/btn_add_gold.png",
        "ui/btn_add_selected.png",
        [this](Ref* sender) {
            // 点击增加金币按钮，可以打开商店或直接增加（测试用）
            this->addGold(1000);
        }
    );
    addGoldBtn->setScale(2.0f);
    addGoldBtn->setPosition(Vec2(450, visibleSize.height - 30));

    auto addElixirBtn = MenuItemImage::create(
        "ui/btn_add_elixir.png",
        "ui/btn_add_selected.png",
        [this](Ref* sender) {
            // 点击增加圣水按钮
            this->addElixir(1000);
        }
    );
    addElixirBtn->setScale(2.0f);
    addElixirBtn->setPosition(Vec2(500, visibleSize.height - 30));

    auto menu = Menu::create(addGoldBtn, addElixirBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    _uiLayer->addChild(menu, 2);
}

// 更新金币数量/容量
void VillageScene::setGold(int gold) {
    _gold = gold;
    if (_goldLabel) {
        // 播放数字变化动画（可选）
        auto fadeOut = FadeOut::create(0.2f);
        auto fadeIn = FadeIn::create(0.2f);
        auto updateText = CallFunc::create([this]() {
            if (_baseMode == BaseMode::NORMAL) {
                _goldLabel->setString(StringUtils::format("%d/%d", _gold,_maxGold));
            }
            else {
                _goldLabel->setString(StringUtils::format("%d", _gold));
            }
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
        auto fadeOut = FadeOut::create(0.2f);
        auto fadeIn = FadeIn::create(0.2f);
        auto updateText = CallFunc::create([this]() {
            if (_baseMode == BaseMode::NORMAL) {
                _elixirLabel->setString(StringUtils::format("%d/%d", _elixir, _maxElixir));
            }
            else {
                _elixirLabel->setString(StringUtils::format("%d", _elixir));
            }});
            _elixirLabel->runAction(Sequence::create(
                fadeOut,
                updateText,
                fadeIn,
                nullptr
            ));
        // 如果需要动画效果，可以像setGold那样实现
    }
}
void VillageScene::setPopulation (int amount) {
    _population = amount;
    if (_populationLabel) {
        auto fadeOut = FadeOut::create(0.2f);
        auto fadeIn = FadeIn::create(0.2f);
        auto updateText = CallFunc::create([this]() {
            if (_baseMode == BaseMode::NORMAL) {
                _populationLabel->setString(StringUtils::format("%d/%d", _population, _maxPopulation));
            }
            else {
                _populationLabel->setString(StringUtils::format("%d", _population));
            }});
            _populationLabel->runAction(Sequence::create(
                fadeOut,
                updateText,
                fadeIn,
                nullptr
            ));
            // 如果需要动画效果，可以像setGold那样实现
    }
}
// 增加金币（带数量检查）
bool VillageScene::addGold(int amount) {
    if (_baseMode == BaseMode::NORMAL&& _gold + amount>_maxGold) {
        showText("gold max!");
        _gold = _maxGold;
        return true;
    }
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
        showResourceShortageTip("金币不足");
        return false;
    }

    setGold(_gold - amount);
    return true;
}

// 增加圣水
bool VillageScene::addElixir(int amount) {
    if (_baseMode == BaseMode::NORMAL && _elixir + amount > _maxElixir) {
        showText("elixir max!");
        _elixir = _maxElixir;
        return true;
    }

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
    setGold(_gold);
}
void VillageScene::addElixirStorageCapacity(int bonus) {
    _maxElixir += bonus;
    setElixir(_elixir);
}
// 部队容量相关方法
void VillageScene::addTroopCapacity(int bonus) {
    _maxPopulation += bonus;
    setPopulation(_population);
}

// 移除部队容量
void VillageScene::removeTroopCapacity(int bonus) {
    _maxPopulation = _maxPopulation - bonus;
    if (_population > _maxPopulation) {
        removeTroopsFromStart(_population - _maxPopulation);
    }
    _population -= _population - _maxPopulation;
    setPopulation(_population);
}
int VillageScene::removeTroopsFromStart(int targetRemoveTotal)
{
    if (targetRemoveTotal <= 0)
    {
        return 0;
    }

    int actualRemoved = 0; 
    int remainingToRemove = targetRemoveTotal; 
    for (TroopType troopType = (TroopType)0; ; troopType = (TroopType)((int)troopType + 1))
    {
        auto it = _trainedTroops.find(troopType);
        if (remainingToRemove <= 0 || ((int)troopType >= 5))
        {
            break;
        }
        if (it == _trainedTroops.end())
        {
            if (_trainedTroops.empty())
            {
                CCLOG("部队容器已空，停止移除");
                break;
            }
            continue;
        }

        int currentTroopCount = it->second; 
        int removeCountInThisType;
        if ((int)troopType == 2) {
            removeCountInThisType = std::min(remainingToRemove%5, currentTroopCount);

        }
        else {
            removeCountInThisType = std::min(remainingToRemove, currentTroopCount);
        }

        actualRemoved += removeCountInThisType;
        if ((int)troopType == 2) {
            remainingToRemove -= removeCountInThisType*5;
        }
        else {
            remainingToRemove -= removeCountInThisType;
        }

        if (currentTroopCount == removeCountInThisType)
        {
            _trainedTroops.erase(it);
        }
        else
        {
            it->second = currentTroopCount - removeCountInThisType;
        }
    }
    return actualRemoved;
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
    saveData.poplation = _population;
    saveData.maxGold = _maxGold;
    saveData.maxElixir = _maxElixir;
    saveData.maxPopulation = _maxPopulation;
    CCLOG("Packing save data: gold=%d, elixir=%d, buildings=%zu",
        saveData.gold, saveData.elixir, _buildings.size());
    for (auto const& pair : _trainedTroops) {
        TroopType type = pair.first;
        int count = pair.second;

        saveData.troops[(int)type] = count;
    }
    // 填充所有建筑数据

    for (const auto& building : _buildings) {
        SaveData::Building bData;
        bData.type = building->getType();
        bData.tilePos = building->getTilePos();
        bData.state = building->getState();
        bData.level = building->getLevel();
        bData.immediatelyBuild = building->immediatelyBuild;
        bData.progressTimer = building->getProgressTimer();
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
    std::vector<BaseBuilding*> buildingsToDestroy;
    for (auto building : _buildings) {
        if (building) { // 空指针防护
            buildingsToDestroy.push_back(building);
        }
    }

    // 2. 遍历收集到的临时列表，调用 destroyBuilding（此时修改原容器 _buildings 不影响临时列表）
    for (auto building : buildingsToDestroy) {
        destroyBuilding(building);
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
    //_occupiedTiles = saveData.occupiedTiles;
    addOccupiedTiles(saveData.occupiedTiles);
    // 恢复资源数值
    _maxGold = saveData.maxGold;
    _maxElixir = saveData.maxElixir;
    _maxPopulation = saveData.maxPopulation;
    setGold(saveData.gold);
    setElixir(saveData.elixir);
    setPopulation(saveData.poplation);
    // 【新增】恢复兵种数据

    _trainedTroops.clear();
    for (auto const& pair : saveData.troops) {
        int typeInt = pair.first;
        int count = pair.second;

        _trainedTroops[(TroopType)typeInt] = count;
    }
    CCLOG("Troops restored. Barbarian count: %d", _trainedTroops[TroopType::BARBARIAN]);
    // 重新创建所有建筑
    for (const auto& bData : saveData.buildings) {
        // 调用placeBuilding逻辑创建建筑（复用现有代码）
        auto building = BaseBuilding::create(bData.type, bData.tilePos, 1.0f);
        if (building) {
            // 恢复建筑状态/等级
            //TODO:恢复建造中的建筑有问题，需要额外处理建造进度
            if (_baseMode != BaseMode::FIGHT) {
                building->setState(bData.state);   
            }
            else {
                building->setState(BuildingState::IDLE);
            }
            building->setLevel(bData.level);    
            if (bData.immediatelyBuild) {
                building->buildImmediately();
            }
            building->setProgressTimer(bData.progressTimer);
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

            // 重新绑定点击回调::TODO:可以测试摧毁假惺惺功能
            if (_baseMode != BaseMode::FIGHT) {
                building->bindClickCallback([this](BaseBuilding* building) {
                    if (_Mode != Mode::NONE) return;
                    auto popup = BuildingPopup::create(building, [this, building](BuildingPopup::ButtonType type) {
                        handleBuildingBtnClick(building, type);
                        });
                    this->addChild(popup, 100);
                    });
            }
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
    if (_baseMode == BaseMode::FIGHT || _baseMode == BaseMode::LEVEL1 || _baseMode == BaseMode::LEVEL2 || _baseMode == BaseMode::LEVEL3) {
        // 1. 更新总建筑数
        _totalBuildingCount = _buildings.size();

        // 2. 重置已摧毁数量 (新关卡开始应当是0)
        _destroyedBuildingCount = 0;

        // 3. 立即刷新一次UI显示 (0/Total = 0%)
        updateDestroyPercent();

        CCLOG("关卡数据重置完成：总建筑数=%d", (int)_totalBuildingCount);
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
// VillageScene.cpp

bool VillageScene::loadGame(const std::string& savePath, bool fallbackToResources) {
    auto fileUtils = cocos2d::FileUtils::getInstance();
    std::string fullPath;

    // 1. 总是优先检查“可写路径”（Writable Path）
    std::string writablePath = fileUtils->getWritablePath() + savePath;

    if (fileUtils->isFileExist(writablePath)) {
        fullPath = writablePath;
        CCLOG("从可写目录加载存档: %s", fullPath.c_str());
    }
    else if (fallbackToResources) {
        // 2. 如果允许回退，且可写目录没找到，尝试在“资源目录”（Resources）查找
        // fullPathForFilename 会自动在项目的 Resources 目录及搜索路径中查找文件
        fullPath = fileUtils->fullPathForFilename(savePath);

        if (fullPath.empty() || !fileUtils->isFileExist(fullPath)) {
            CCLOG("错误：在可写目录和资源目录均未找到文件: %s", savePath.c_str());
            return false;
        }
        CCLOG("从资源目录加载预设: %s", fullPath.c_str());
    }
    else {
        // 3. 不允许回退（fallbackToResources == false），且可写目录没找到
        // 这里对应原来 loadGame2 的失败逻辑
        CCLOG("读档失败：文件不存在(仅检查可写目录) -> %s", writablePath.c_str());
        return false;
    }

    // --- 以下为公共读取逻辑 ---

    // 4. 读取文件内容
    std::string saveStr = fileUtils->getStringFromFile(fullPath);

    // 5. 校验内容是否为空
    if (saveStr.empty()) {
        CCLOG("错误：文件内容为空！");
        return false;
    }

    // 6. 反序列化数据
    SaveData::Village saveData = SaveData::Village::fromString(saveStr);

    // 7. 恢复场景数据
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

// 创建存档选择菜单
void VillageScene::createSaveSelectMenu() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建半透明背景层
    _saveSelectLayer = Layer::create();
    _saveSelectLayer->setVisible(false);
    this->addChild(_saveSelectLayer, 99); // 层级高于其他UI

    // 添加半透明黑色背景（覆盖整个屏幕）
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180));
    bg->setContentSize(visibleSize);
    _saveSelectLayer->addChild(bg);

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
    _saveSelectLayer->addChild(frame);

    // 标题
    auto title = Label::createWithTTF("choose save path", "fonts/Marker Felt.ttf", 36);
    title->setColor(Color3B::YELLOW);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 150));
    _saveSelectLayer->addChild(title);
    auto save1Btn = MenuItemImage::create(
        "ui/btn_1.png",
        "ui/btn_1.png",

            [this](Ref* sender) {
                if (_baseMode == BaseMode::CREATING) {
                    this->saveTip("normal1.txt");
                }
                else {
                    this->saveTip("create1.txt");
                }
                
        }
    );
    save1Btn->setPosition(Vec2(-200, -110));
    save1Btn->setScale(0.9f);
    auto save2Btn = MenuItemImage::create(
        "ui/btn_2.png",
        "ui/btn_2.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::CREATING) {
                this->saveTip("normal2.txt");
            }
            else {
                this->saveTip("create2.txt");
            }
        }
    );
    save2Btn->setPosition(Vec2(0, -110));
    save2Btn->setScale(0.9f);

    auto save3Btn = MenuItemImage::create(
        "ui/btn_3.png",
        "ui/btn_3.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::CREATING) {
                this->saveTip("normal3.txt");
            }
            else {
                this->saveTip("create3.txt");
            }
        }
    );
    save3Btn->setPosition(Vec2(200, -110));
    save3Btn->setScale(0.9f);

    // 关闭按钮
    auto closeBtn = MenuItemImage::create(
        "ui/close_btn_normal.png",
        "ui/close_btn_selected.png",
        [this](Ref* sender) {
            this->hideSaveSelectMenu();
        }
    );
    closeBtn->setPosition(Vec2(200, 0));
    closeBtn->setScale(0.8f);

    // 创建菜单
    auto menu = Menu::create(save1Btn, save2Btn, save3Btn, closeBtn, nullptr);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _saveSelectLayer->addChild(menu);
}
//存档按钮点击回调
void VillageScene::onSaveBtnClicked(Ref* sender) {
    // 调用已实现的存档方法
    toggsaveSelectMenu();
}
void VillageScene::saveTip(std::string path) {
    bool success = saveGame(path);
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
void VillageScene::hideSaveSelectMenu() {
    if (_saveSelectLayer) {
        // 添加淡出动画
        auto fadeOut = FadeOut::create(0.2f);
        auto hide = CallFunc::create([this]() {
            if (_saveSelectLayer) {
                _saveSelectLayer->setVisible(false);
            }
            });
        _saveSelectLayer->runAction(Sequence::create(fadeOut, hide, nullptr));
    }
}
void VillageScene::showSaveSelectMenu() {
    // 如果关卡选择层不存在，则创建
    if (!_saveSelectLayer) {
        createSaveSelectMenu();
    }

    if (_saveSelectLayer) {
        _saveSelectLayer->setVisible(true);

        // 添加淡入动画
        _saveSelectLayer->setOpacity(0);
        _saveSelectLayer->runAction(FadeIn::create(0.3f));
    }
}
void VillageScene::toggsaveSelectMenu() {
    if (!_isSaveSelectShow) {
        // 显示关卡选择菜单
        showSaveSelectMenu();
        _isSaveSelectShow = true;
    }
    else {
        // 隐藏关卡选择菜单
        hideSaveSelectMenu();
        _isSaveSelectShow = false;
    }
}
void VillageScene::createLoadSelectMenu() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建半透明背景层
    _loadSelectLayer = Layer::create();
    _loadSelectLayer->setVisible(false);
    this->addChild(_loadSelectLayer, 99); // 层级高于其他UI

    // 添加半透明黑色背景（覆盖整个屏幕）
    auto bg = LayerColor::create(Color4B(0, 0, 0, 180));
    bg->setContentSize(visibleSize);
    _loadSelectLayer->addChild(bg);

    // 添加背景框
    auto frame = Sprite::create("ui/menu_bg.png");
    if (!frame) {
        // 如果没有背景图片，创建一个纯色背景
        frame = Sprite::create();
        auto colorBg = LayerColor::create(Color4B(100, 50, 50, 230), 400, 400);
        colorBg->setPosition(Vec2::ZERO);
        frame->addChild(colorBg);
    }
    frame->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    frame->setScale(0.8f);
    _loadSelectLayer->addChild(frame);

    // 标题（与存档区分，改为“选择读档”）
    auto title = Label::createWithTTF("choose load path", "fonts/Marker Felt.ttf", 36);
    title->setColor(Color3B::RED);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 150));
    _loadSelectLayer->addChild(title);

    // 读档1按钮
    auto load1Btn = MenuItemImage::create(
        "ui/btn_1.png",
        "ui/btn_1.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::CREATING) {
                this->loadTip("normal1.txt");
            }
            else {
                this->loadTip("create1.txt");
            }
        }
    );
    load1Btn->setPosition(Vec2(-200, -110));
    load1Btn->setScale(0.9f);

    // 读档2按钮
    auto load2Btn = MenuItemImage::create(
        "ui/btn_2.png",
        "ui/btn_2.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::CREATING) {
                this->loadTip("normal2.txt");
            }
            else {
                this->loadTip("create2.txt");
            }
        }
    );
    load2Btn->setPosition(Vec2(0, -110));
    load2Btn->setScale(0.9f);

    // 读档3按钮
    auto load3Btn = MenuItemImage::create(
        "ui/btn_3.png",
        "ui/btn_3.png",
        [this](Ref* sender) {
            if (_baseMode == BaseMode::CREATING) {
                this->loadTip("normal3.txt");
            }
            else {
                this->loadTip("create3.txt");
            }
        }
    );
    load3Btn->setPosition(Vec2(200, -110));
    load3Btn->setScale(0.9f);

    // 关闭按钮
    auto closeBtn = MenuItemImage::create(
        "ui/close_btn_normal.png",
        "ui/close_btn_selected.png",
        [this](Ref* sender) {
            this->hideLoadSelectMenu();
        }
    );
    closeBtn->setPosition(Vec2(200, 0));
    closeBtn->setScale(0.8f);

    // 创建菜单
    auto menu = Menu::create(load1Btn, load2Btn, load3Btn, closeBtn, nullptr);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
    _loadSelectLayer->addChild(menu);
}

// 读档按钮点击回调（与存档按钮对应）
void VillageScene::onLoadBtnClicked(Ref* sender) {
    // 调用读档菜单切换方法
    toggleLoadSelectMenu();
}

// 读档结果提示
void VillageScene::loadTip(std::string path) {
    bool success = loadGame(path,false); // 调用读档核心方法（需你实现loadGame逻辑）
    // 提示玩家读档结果
    std::string tip = success ? "load success" : "load failed";
    CCLOG("%s", tip.c_str());

    // 添加弹窗提示（与存档提示样式一致）
    auto tipLabel = Label::createWithSystemFont(tip, "Arial", 30);
    tipLabel->setColor(success ? Color3B::GREEN : Color3B::RED);
    tipLabel->setPosition(Director::getInstance()->getWinSize() / 2);
    this->addChild(tipLabel, 300);
    // 2秒后隐藏提示（带动画效果）
    tipLabel->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(0.5f),
        RemoveSelf::create(),
        nullptr
    ));

    // 读档成功后，可额外添加逻辑（如隐藏读档菜单、刷新游戏场景等）
    if (success) {
        this->hideLoadSelectMenu();
        _isLoadSelectShow = false;
    }
}

// 隐藏读档选择菜单（带动画）
void VillageScene::hideLoadSelectMenu() {
    if (_loadSelectLayer) {
        // 添加淡出动画
        auto fadeOut = FadeOut::create(0.2f);
        auto hide = CallFunc::create([this]() {
            if (_loadSelectLayer) {
                _loadSelectLayer->setVisible(false);
            }
            });
        _loadSelectLayer->runAction(Sequence::create(fadeOut, hide, nullptr));
    }
}

// 显示读档选择菜单（带动画）
void VillageScene::showLoadSelectMenu() {
    // 如果读档选择层不存在，则创建
    if (!_loadSelectLayer) {
        createLoadSelectMenu();
    }

    if (_loadSelectLayer) {
        _loadSelectLayer->setVisible(true);

        // 添加淡入动画
        _loadSelectLayer->setOpacity(0);
        _loadSelectLayer->runAction(FadeIn::create(0.3f));
    }
}

// 切换读档选择菜单的显示/隐藏状态
void VillageScene::toggleLoadSelectMenu() {
    if (!_isLoadSelectShow) {
        // 显示读档选择菜单
        showLoadSelectMenu();
        _isLoadSelectShow = true;
    }
    else {
        // 隐藏读档选择菜单
        hideLoadSelectMenu();
        _isLoadSelectShow = false;
    }
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
            this->gotoLevel1("level2_preset.txt");
        }
    );
    level2Btn->setPosition(Vec2(0, -110));
    level2Btn->setScale(0.9f);

    // 关卡3按钮
    auto level3Btn = MenuItemImage::create(
        "ui/level3_btn_normal.png",
        "ui/level3_btn_selected.png",
        [this](Ref* sender) {
            this->gotoLevel1("level3_preset.txt");
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
}

bool VillageScene::level_init()
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
    init_level_Btns(BaseMode::LEVEL1);
    go_back_Btn();

    initStarRatingUI();
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
    if (_baseMode == BaseMode::LEVEL1) {
        loadGame("level1.txt",true);
    }
    if (_baseMode == BaseMode::LEVEL2) {
        loadGame("level2.txt", true);
    }
    if (_baseMode == BaseMode::LEVEL3) {
        loadGame("level3.txt", true);
    }
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



void VillageScene::gotoLevel1(const std::string& levelFilename) {
    // 1. 先保存当前村庄状态
    saveGame("village_save.txt");

    // 2. 创建一个新的战斗场景
    Scene* levelScene = VillageScene::createScene(BaseMode::FIGHT);
    VillageScene* villageLayer = dynamic_cast<VillageScene*>(levelScene->getChildByTag(25));

    if (villageLayer) {
        // 尝试加载指定的关卡文件
        bool loadSuccess = villageLayer->loadGame(levelFilename, true);

        // 【修复】如果加载失败（文件不存在），尝试加载玩家的1号存档作为备用
        if (!loadSuccess) {
            CCLOG("关卡文件 %s 不存在，尝试加载 normal1.txt", levelFilename.c_str());
            loadSuccess = villageLayer->loadGame("normal1.txt", true);
        }

        // 【修复】如果还是失败，强制允许进入（作为空地图测试），防止卡死
        if (!loadSuccess) {
            CCLOG("无可用存档，生成临时测试关卡");
            // 手动添加一个大本营作为测试目标
            // 注意：这里需要确保坐标在地图范围内
            villageLayer->placeBuilding(Vec2(20, 20), BuildingType::TOWN_HALL);

            // 手动触发布局更新
            villageLayer->_totalBuildingCount = villageLayer->_buildings.size();
            villageLayer->updateDestroyPercent();

            loadSuccess = true; // 强制设为成功，以便进入场景
        }
        std::string playerSavePath = FileUtils::getInstance()->getWritablePath() + "village_save.txt";

        if (FileUtils::getInstance()->isFileExist(playerSavePath)) {
            // B. 读取文件内容字符串
            std::string saveStr = FileUtils::getInstance()->getStringFromFile(playerSavePath);

            // C. 使用 SaveData 解析字符串
            SaveData::Village playerData = SaveData::Village::fromString(saveStr);

            // D. 将解析出的兵种数据赋值给战斗场景的 _trainedTroops
            villageLayer->_trainedTroops.clear();
            for (auto const& pair : playerData.troops) {
                int typeInt = pair.first;
                int count = pair.second;
                // 转换 int -> TroopType 并存入 map
                villageLayer->_trainedTroops[(TroopType)typeInt] = count;
            }

            CCLOG("兵种数据已传输至战斗场景! 野蛮人数量: %d", villageLayer->_trainedTroops[TroopType::BARBARIAN]);
        }
        else {
            CCLOG("警告：未找到 village_save.txt，无法带入兵种");
        }
        if (loadSuccess) {
            CCLOG("进入战斗场景");
            Director::getInstance()->pushScene(levelScene);
        }
    }
    else {
        CCLOG("获取 VillageScene 层失败");
    }
}

void VillageScene::gotoFight() {
    saveGame("fight.txt");
    Scene* fightScene = VillageScene::createScene(BaseMode::FIGHT);
    if (!fightScene) {
        CCLOG("FIght创建f失败！");
        return;
    }
    CCLOG("FIGHT创建s成功，准备切换");
    Director::getInstance()->pushScene(fightScene);
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
    _uiLayer->addChild(_countDownLabel, 200);
    // 初始化计时变量
    _totalTime = 150.0f; // 2分30秒 = 150秒
    _remainingTime = _totalTime;
    this->schedule(CC_SCHEDULE_SELECTOR(VillageScene::updateCountDown), 0.1f);
    //_fightStartBtn->setVisible(false);
    initStarRatingUI();
}
void VillageScene::updateCountDown(float dt)
{
    CCLOG("%f", _remainingTime);
    // 扣除流逝的时间（dt是定时器两次回调的实际间隔）
    _remainingTime -= dt;
    _countDownLabel->setColor(_remainingTime >= 50.0f ? Color3B::GREEN : Color3B::RED);
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


void VillageScene::showFightSettlePopup()
{
    // 1. 创建弹窗根节点（用于承载所有弹窗元素，方便统一管理）
    LayerColor* popupRoot = LayerColor::create(Color4B(0, 0, 0, 180)); // 半透明黑色背景遮罩
    popupRoot->setName("FightSettlePopup");
    popupRoot->setContentSize(Director::getInstance()->getVisibleSize());
    // 开启触摸吞噬，防止点击弹窗穿透到下层界面
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->setSwallowTouches(true);
    touchListener->onTouchBegan = [](Touch* touch, Event* event) { return true; };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, popupRoot);
    _uiLayer->addChild(popupRoot, 1000); // 设为最高层级，确保弹窗在最上层

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    Vec2 popupCenter = Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height / 2);

    // 2. 创建弹窗背景（可替换为自定义弹窗背景图片）
    Sprite* popupBg = Sprite::create("ui/menu_bg.png"); // 自定义弹窗背景资源
    if (!popupBg) {
        // 备用：若没有图片资源，创建纯色背景
        popupBg = Sprite::create();
        popupBg->setTextureRect(Rect(0, 0, 500, 600));
        popupBg->setColor(Color3B(50, 50, 80));
        popupBg->setOpacity(255);
    }
    popupBg->setPosition(popupCenter);
    popupBg->setScale(1.0f);
    popupRoot->addChild(popupBg);
    int starShowCount = currentStars; // 从现有逻辑获取已解锁星级
    float starStartX = popupBg->getContentSize().width/2  - 180; // 星级居中排列
    float starY = popupBg->getContentSize().height-starShowCount * 180; // 星级在胜负图片下方
    if (destroyPercent >= 50.0f&& destroyPercent < 80.0f) {
        starShowCount = 1;
        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->preloadBackgroundMusic("audio/vectory1.mp3");
        SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/vectory1.mp3");
    }
    else if (destroyPercent >= 80.0f&& destroyPercent < 100.0f) {
        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->preloadBackgroundMusic("audio/vectory2.mp3");
        SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/vectory2.mp3");
        starShowCount = 2;
    }
    else if (destroyPercent >= 100.0f) {
        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->preloadBackgroundMusic("audio/vectory3.mp3");
        SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/vectory3.mp3");
        starShowCount = 3;
    }
    else {
        SimpleAudioEngine::getInstance()->stopBackgroundMusic();
        SimpleAudioEngine::getInstance()->preloadBackgroundMusic("audio/fail.mp3");
        SimpleAudioEngine::getInstance()->playBackgroundMusic("audio/fail.mp3");
    }

    for (int i = 0; i < 3; i++) { // 固定显示3颗星（已解锁显示黄色，未解锁显示灰色）
        Sprite* starSprite = nullptr;
        if (i <starShowCount) {
            starSprite = Sprite::create("ui/star_yellow.png"); // 已解锁黄色星星
        }
        else {
            starSprite = Sprite::create("ui/star_grey.png");   // 未解锁灰色星星
        }
        if (starSprite) {
            starSprite->setScale(0.6f);
            Vec2 starPos = Vec2(starStartX + i * 180, starY );
            starSprite->setPosition(starPos);
            popupBg->addChild(starSprite);
        }
    }

    // 显示摧毁百分比文字标签
    Label* percentDisplayLabel = Label::createWithTTF(StringUtils::format("%.1f%%", destroyPercent),
        "fonts/Marker Felt.ttf", 32);
    if (percentDisplayLabel) {
        percentDisplayLabel->setColor(Color3B::YELLOW);
        percentDisplayLabel->setPosition(Vec2(popupBg->getContentSize().width / 2, popupBg->getContentSize().height - 260));
        popupBg->addChild(percentDisplayLabel);
    }


    Sprite* btnNormal = Sprite::create("ui/btn_destroy.png");
    Sprite* btnPressed = Sprite::create("ui/btn_destroy.png");
    if (!btnPressed) {
        btnPressed = Sprite::create();
        btnPressed->setTextureRect(btnNormal->getTextureRect());
        btnPressed->setColor(Color3B(180, 180, 180)); // 按下时变暗
    }

    MenuItemSprite* backBtn = MenuItemSprite::create(btnNormal, btnPressed,
        CC_CALLBACK_0(VillageScene::backfromFight, this)); // 绑定返回函数

    backBtn->setScale(0.9f);
    // 按钮放置在弹窗下方居中位置
    Vec2 btnPos = Vec2(popupBg->getContentSize().width / 2, 80);
    backBtn->setPosition(btnPos);

    // 创建菜单并添加按钮
    Menu* menu = Menu::create(backBtn, nullptr);
    menu->setPosition(Vec2::ZERO); // 菜单位置相对弹窗背景，设为原点
    popupBg->addChild(menu);
    popupBg->setScale(0.8f);
    popupBg->setOpacity(0);
    auto scaleTo = ScaleTo::create(0.3f, 1.0f);
    auto fadeIn = FadeIn::create(0.3f);
    auto ease = EaseBackOut::create(scaleTo);
    popupBg->runAction(Spawn::create(ease, fadeIn, nullptr));
}
void VillageScene::onFightSettle() {
    showFightSettlePopup();
}
void VillageScene::initStarRatingUI() {
    Size visibleSize = Director::getInstance()->getVisibleSize();
    //有黑边时origin不为0
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // 初始化星级数据
    _totalBuildingCount = _buildings.size(); // 替换为实际总建筑数量（建议从场景数据中读取）
    _destroyedBuildingCount = 0;
    destroyPercent = 0.0f;
    currentStars = 0;
    _starTargetPos = Vec2(origin.x + 150, origin.y + visibleSize.height - 50); // 星级最终位置

    // 1. 创建百分比显示标签
    percentLabel = Label::createWithTTF("0%", "fonts/Marker Felt.ttf", 32);
    percentLabel->setPosition(_starTargetPos.x +150, _starTargetPos.y -100);
    percentLabel->setColor(Color3B::YELLOW);
    _uiLayer->addChild(percentLabel, 201);

    // 2. 创建3颗星星（初始为灰色未激活状态）
    for (int i = 0; i < 3; i++) {
        Sprite* star = Sprite::create("ui/star_grey.png"); // 灰色星星资源
        star->setScale(0.5f);
        star->setPosition(_starTargetPos.x+100*i, _starTargetPos.y);
        star->setVisible(true);
        _uiLayer->addChild(star, 201);
        starSprites.pushBack(star);
    }
}
// 建筑被摧毁时调用（外部触发，如建筑血量为0时）
//void VillageScene::onBuildingDestroyed() {
    //_destroyedBuildingCount++;
   // updateDestroyPercent();
    //checkStarUnlock();
//}

// 更新摧毁百分比显示
void VillageScene::updateDestroyPercent() {
    // 【修复】防止除以0导致的崩溃 (NaN)
    if (_totalBuildingCount <= 0) {
        destroyPercent = 0.0f;
        if (percentLabel) {
            percentLabel->setString("0.0%");
        }
        return;
    }

    // 计算百分比（防止超过100%）
    destroyPercent = std::min(100.0f, (_destroyedBuildingCount / _totalBuildingCount) * 100);

    // 更新标签显示
    if (percentLabel) {
        percentLabel->setString(StringUtils::format("%.1f%%", destroyPercent));
    }

    // 100% 结算逻辑
    if (destroyPercent >= 100.0f) {
        // 防止重复调用
        if (this->isScheduled(CC_SCHEDULE_SELECTOR(VillageScene::updateCountDown))) {
            this->unschedule(CC_SCHEDULE_SELECTOR(VillageScene::updateCountDown));
            this->onFightSettle();
        }
    }
}

// 检查星级解锁逻辑
void VillageScene::checkStarUnlock() {
    int newStarCount = 0;
    // 判断当前应解锁的星级
    if (destroyPercent >= 50.0f) newStarCount = 1;
    if (destroyPercent >= 80.0f) newStarCount = 2;
    if (destroyPercent >= 100.0f) newStarCount = 3;

    // 解锁新星级时执行动画
    while (currentStars < newStarCount) {
        currentStars++;
        flyStarToTarget(currentStars - 1); // 索引从0开始
        updateStarDisplay();
    }
}

// 星级飞行动画（由大到小飞到指定位置）
void VillageScene::flyStarToTarget(int starIndex) {
    if (starIndex < 0 || starIndex >= starSprites.size()) return;

    Sprite* star = starSprites.at(starIndex);
    // 随机生成初始位置（屏幕随机位置，模拟从战场飞出）
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 startPos = Vec2(rand() % (int)visibleSize.width, rand() % (int)visibleSize.height);

    // 初始状态：放大+透明
    star->setPosition(startPos);
    star->setScale(2.0f);
    star->setOpacity(0);
    star->setTexture("ui/star_yellow.png"); // 切换为黄色激活状态

    // 组合动画：淡入+缩放+移动
    auto fadeIn = FadeIn::create(0.2f);
    auto scaleTo = ScaleTo::create(0.5f, 0.5f); // 缩放到目标大小
    auto moveTo = MoveTo::create(0.5f,
        Vec2(_starTargetPos.x + (starIndex * 100), _starTargetPos.y));
    auto easeMove = EaseBackOut::create(moveTo); // 缓动效果更自然

    // 并行执行动画
    star->runAction(Spawn::create(fadeIn, scaleTo, easeMove, nullptr));
}

// 更新星级显示状态（处理边界情况）
void VillageScene::updateStarDisplay() {
    for (int i = 0; i < starSprites.size(); i++) {
        Sprite* star = starSprites.at(i);
        // 已解锁的星星显示黄色，未解锁的显示灰色
        if (i < currentStars) {
            star->setTexture("ui/star_yellow.png");
        }
        else {
            star->setTexture("ui/star_grey.png");
        }
    }
}
// 销毁场景并返回标题界面
void VillageScene::destroyScene() {
    //_eventDispatcher->removeEventListenersForTarget(this);
    auto titleSceneLayer = TitleScene::getInstance();
    if (titleSceneLayer) {
        Scene* titleSceneContainer = dynamic_cast<Scene*>(titleSceneLayer->getParent());
        if (titleSceneContainer) {
            Director::getInstance()->replaceScene(TransitionFade::create(0.5f, titleSceneContainer, Color3B::BLACK));
        }
    }
}


void VillageScene::onEnter() {
    Scene::onEnter();
    playRandomBackgroundMusic();
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
// VillageScene.cpp

void VillageScene::update(float dt)
{
    // 【新增】每帧清理无效兵种
    // 如果兵种已经被移除（引用计数为0或已被销毁），需要从列表中剔除
    // 使用 C++ 标准库的 remove_if 算法
    if (!_spawnedTroops.empty()) {
        auto it = std::remove_if(_spawnedTroops.begin(), _spawnedTroops.end(),
            [](BaseTroop* troop) {
                // 判断条件：指针为空，或者兵种已经被移出父节点
                return troop == nullptr || troop->getParent() == nullptr;
            });

        // 真正的物理删除
        if (it != _spawnedTroops.end()) {
            _spawnedTroops.erase(it, _spawnedTroops.end());
        }
    }

    // 如果有倒计时或其他逻辑，也可以放在这里
    // updateCountDown(dt); 
}





// 初始化升级栏
void VillageScene::createUpgradeBar() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 【修复3】不要覆盖 _buildBarLayer，建议使用局部变量或新建一个成员变量 _upgradeBarLayer
    // 这里暂时用局部变量演示，如果你需要后续隐藏它，请在头文件定义 Layer* _upgradeBarLayer;
    // 并将下面这行改为: _upgradeBarLayer = Layer::create();
    auto upgradeLayer = Layer::create();
    this->addChild(upgradeLayer, 99);

    // 1. 创建背景（带判空保护）
    _barBg = Sprite::create("ui/build_bar_bg.png");
    if (_barBg) {
        _barBg->setPosition(Vec2(visibleSize.width / 2, 50));
        _barBg->setScaleX(visibleSize.width / _barBg->getContentSize().width * 0.8f);
        upgradeLayer->addChild(_barBg);
    }
    else {
        CCLOG("ERROR: 'ui/build_bar_bg.png' not found! Upgrade bar background will be missing.");
    }

    // 2. 创建野蛮人升级按钮（带判空保护）
    _BarbarianBtn = MenuItemImage::create(
        "building/barbarian_upgrade_icon.png",
        "building/barbarian_upgrade_selected.png",
        [this](Ref* sender) {
            CCLOG("Barbarian upgrade clicked");
            int currentLevel = _troopLevel[TroopType::BARBARIAN];
            if (currentLevel == 0) currentLevel = 1; // 默认防错

            // 2. 检查资源 (假设升级消耗：等级 * 1000 圣水)
            int cost = currentLevel * 500;
            if (_elixir < cost) {
                showResourceShortageTip("圣水不足！");
                return;
            }

            // 3. 扣除资源 & 提升等级
            spendElixir(cost);
            _troopLevel[TroopType::BARBARIAN]++;
            int newLevel = _troopLevel[TroopType::BARBARIAN];

            // 4. 【核心】修改全局配置表 g_troopTrainConfig
            // 只有改了这里，Barbarian.cpp 里的 create 才能读到新数据
            TroopConfig& config = g_troopTrainConfig[TroopType::BARBARIAN];

            config.level = newLevel;
            config.hp += 50;           // 每次升级 +50 血量
            config.attackPower += 10;  // 每次升级 +10 攻击力
        }
    );
    if (!_BarbarianBtn) CCLOG("ERROR: Barbarian upgrade icon not found!");

    // 3. 创建弓箭手升级按钮（带判空保护）
    _ArcherBtn = MenuItemImage::create(
        "building/archer_upgrade_icon.png",
        "building/archer_upgrade_selected.png",
        [this](Ref* sender) {
            CCLOG("Archer upgrade clicked");
            // 1. 获取等级
            int currentLevel = _troopLevel[TroopType::ARCHER];
            if (currentLevel == 0) currentLevel = 1;

            // 2. 检查资源 (弓箭手升级稍贵一点)
            int cost = currentLevel * 1200;
            if (_elixir < cost) {
                showResourceShortageTip("圣水不足！");
                return;
            }

            // 3. 扣资源 & 升级
            spendElixir(cost);
            _troopLevel[TroopType::ARCHER]++;
            int newLevel = _troopLevel[TroopType::ARCHER];

            // 4. 修改全局配置 (HP+30, 攻击+8)
            TroopConfig& config = g_troopTrainConfig[TroopType::ARCHER];
            config.level = newLevel;
            config.hp += 30;
            config.attackPower += 8;
        }
    );
    if (!_ArcherBtn) CCLOG("ERROR: Archer upgrade icon not found!");
    // 2. 创建giant升级按钮（带判空保护）
    _GiantBtn = MenuItemImage::create(
        "building/giant_upgrade_icon.png",
        "building/giant_upgrade_selected.png",
        [this](Ref* sender) {
            CCLOG("Giant upgrade clicked");
            int currentLevel = _troopLevel[TroopType::GIANT];
            if (currentLevel == 0) currentLevel = 1;

            // 巨人升级比较贵
            int cost = currentLevel * 2000;
            if (_elixir < cost) {
                showResourceShortageTip("圣水不足！");
                return;
            }

            spendElixir(cost);
            _troopLevel[TroopType::GIANT]++;
            int newLevel = _troopLevel[TroopType::GIANT];

            // 巨人成长性高 (HP+200, 攻击+15)
            TroopConfig& config = g_troopTrainConfig[TroopType::GIANT];
            config.level = newLevel;
            config.hp += 200;
            config.attackPower += 15;
        }
    );
    if (!_GiantBtn) CCLOG("ERROR: Giant upgrade icon not found!");
    // 2. 创建野蛮人升级按钮（带判空保护）
    _BomberBtn = MenuItemImage::create(
        "building/bomber_upgrade_icon.png",
        "building/bomber_upgrade_selected.png",
        [this](Ref* sender) {
            CCLOG("Bomber upgrade clicked");
            int currentLevel = _troopLevel[TroopType::BOMBER];
            if (currentLevel == 0) currentLevel = 1;

            int cost = currentLevel * 1500;
            if (_elixir < cost) {
                showResourceShortageTip("圣水不足！");
                return;
            }

            spendElixir(cost);
            _troopLevel[TroopType::BOMBER]++;
            int newLevel = _troopLevel[TroopType::BOMBER];

            // 炸弹人主要提升伤害 (HP+10, 攻击+100)
            TroopConfig& config = g_troopTrainConfig[TroopType::BOMBER];
            config.level = newLevel;
            config.hp += 10;
            config.attackPower += 100; // 炸弹人攻击力成长很高
        }
    );
    if (!_BomberBtn) CCLOG("ERROR: Bomber upgrade icon not found!");
    // 4. 创建取消按钮
    _cancelPlaceBtn = MenuItemImage::create(
        "ui/btn_close.png",
        "ui/btn_close.png",
        [this, upgradeLayer](Ref* sender) {
            // 点击取消，移除升级栏
            upgradeLayer->removeFromParent();
        }
    );

    // 5. 安全创建菜单
    // Vector 容器可以容纳有效的 MenuItem，避免传入 nullptr 导致 Menu 截断
    Vector<MenuItem*> menuItems;
    if (_BarbarianBtn) menuItems.pushBack(_BarbarianBtn);
    if (_ArcherBtn) menuItems.pushBack(_ArcherBtn);
    if (_GiantBtn) menuItems.pushBack(_GiantBtn);
    if (_BomberBtn) menuItems.pushBack(_BomberBtn);

    if (_cancelPlaceBtn) menuItems.pushBack(_cancelPlaceBtn);

    if (menuItems.empty()) {
        CCLOG("ERROR: No buttons created for Upgrade Bar!");
        return;
    }

    auto menu = Menu::createWithArray(menuItems);
    menu->alignItemsHorizontallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width / 2, 50));
    upgradeLayer->addChild(menu);

    // 如果你在头文件定义了 _upgradeBarLayer，记得赋值
    // _upgradeBarLayer = upgradeLayer;
}

// 初始化建筑模式切换按钮
void VillageScene::init_troop_upgrade_ModeBtn() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    // 创建建筑模式开关按钮（右上角悬浮）
    auto troop_upgrade_ModeBtn = MenuItemImage::create(
        "ui/upgrade_mode_btn_normal.png",  // 正常状态图片
        "ui/upgrade_mode_btn_selected.png",// 按下状态图片
        [this](Ref* sender) {
            this->createUpgradeBar(); // 点击切换建筑栏
        }
    );
    // 设置按钮大小和位置（可根据需求调整）
    troop_upgrade_ModeBtn->setScale(0.8f);
    troop_upgrade_ModeBtn->setPosition(Vec2(visibleSize.width - 300, visibleSize.height - 100));

    auto menu = Menu::create(troop_upgrade_ModeBtn, nullptr);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 100);
}


void VillageScene::createInfoPanel(BaseBuilding* targetBuilding) {
    auto maskLayer = LayerColor::create(Color4B(0, 0, 0, 180));
    Size visibleSize = Director::getInstance()->getVisibleSize();
    maskLayer->setContentSize(visibleSize);
    Size maskSize = maskLayer->getContentSize();
    maskLayer->setPosition(Vec2::ZERO);
    this->addChild(maskLayer, 1000); // 最高层级确保不被遮挡

    // 创建弹窗背景（图片）
    auto popupBg = Sprite::create("ui/build_bar_bg.png");
    popupBg->setContentSize(Size(500, 300));
    if (!popupBg) { // 图片不存在时，创建纯色备用背景
        popupBg = Sprite::create();
        auto colorBg = LayerColor::create(Color4B(255, 255, 255, 255), 500, 300);
        colorBg->setPosition(Vec2::ZERO);
        popupBg->addChild(colorBg);
        popupBg->setContentSize(Size(500, 300));
    }
    // 弹窗居中显示
    popupBg->setPosition(maskSize.width / 2, maskSize.height / 2);
    maskLayer->addChild(popupBg, 1);
    auto closeBtn = MenuItemImage::create(
        "ui/btn_destroy.png",
        "ui/btn_destroy.png",
        [maskLayer](Ref* sender) { maskLayer->removeFromParent(); }
    );
    closeBtn->setPosition(popupBg->getContentSize().width - 30, popupBg->getContentSize().height - 30); // 右上角
    auto menu = Menu::create(closeBtn, nullptr); // Menu是按钮容器
    menu->setPosition(Vec2::ZERO); // Menu位置归零（按钮位置基于popupBg）
    popupBg->addChild(menu, 2); // 按钮加到弹窗背景


    Vec2 panelPos = Vec2(visibleSize.width / 2, visibleSize.height / 2 - 50);
    auto hpLabel = Label::createWithSystemFont("", "Arial", 24);
    hpLabel->setPosition(250,250);
    hpLabel->setColor(Color3B::RED);
    popupBg->addChild(hpLabel, 1000);
    // 升级消耗Label
    auto upgradeCostLabel = Label::createWithSystemFont("", "Arial", 24);
    upgradeCostLabel->setPosition(250, 200);
    upgradeCostLabel->setColor(Color3B::GREEN);
    popupBg->addChild(upgradeCostLabel, 1000);

    // 专属属性Label1
    auto specialAttrLabel1 = Label::createWithSystemFont("", "Arial", 24);
    specialAttrLabel1->setPosition(250,150);
    specialAttrLabel1->setColor(Color3B::YELLOW);
    popupBg->addChild(specialAttrLabel1, 1000);

    //专属属性Label2
    auto specialAttrLabel2 = Label::createWithSystemFont("", "Arial", 24);
    specialAttrLabel2->setPosition(250,100);
    specialAttrLabel2->setColor(Color3B::BLACK);
    popupBg->addChild(specialAttrLabel2, 1000);

    auto specialAttrLabel3 = Label::createWithSystemFont("", "Arial", 24);
    specialAttrLabel3->setPosition(250,50);
    specialAttrLabel3->setColor(Color3B::WHITE);
    popupBg->addChild(specialAttrLabel3, 1000);
    if (!targetBuilding) return;


    int currentHp = targetBuilding->getCurrentHp();
    int maxHp = targetBuilding->getConfig().hp;
    hpLabel->setString(StringUtils::format("hp: %.0d/%.0d", currentHp, maxHp));


    int upgradeLevel = targetBuilding->getLevel() + 1;
    int upgradeGoldCost = getBuildingConfigByType(targetBuilding->getConfig().type, upgradeLevel).cost.find("gold")->second;
    int upgradeElixirCost = getBuildingConfigByType(targetBuilding->getConfig().type, upgradeLevel).cost.find("elixir")->second;
    upgradeCostLabel->setString(StringUtils::format("upgrade cost: gold%d elixir%d",
        upgradeGoldCost, upgradeElixirCost));


    BuildingType type = targetBuilding->getConfig().type;
    switch (type) {
    case BuildingType::BARRACKS: { // 兵营显示容量
        int capacity = dynamic_cast<Barracks*>(targetBuilding)->getTroopSpace();
        specialAttrLabel1->setString(StringUtils::format("population capacity: %d", capacity));
        specialAttrLabel2->setString("");
        specialAttrLabel3->setString("");
        break;
    }
    case BuildingType::ARROW_TOWER: // 攻击类显示攻击力/范围/间隔
    case BuildingType::CANNON: {
        float attack = dynamic_cast<BaseAttackBuilding*>(targetBuilding)->getAttackDamage();
        float attackRange = dynamic_cast<BaseAttackBuilding*>(targetBuilding)->getAttackRange();
        float attackInterval = dynamic_cast<BaseAttackBuilding*>(targetBuilding)->getAttackCooldown();
        specialAttrLabel1->setString(StringUtils::format("AttackDamage: %.0f", attack));
        specialAttrLabel2->setString(StringUtils::format("AttackRange: %.0f", attackRange));
        specialAttrLabel3->setString(StringUtils::format("AttackCooldown: %.1fs", attackInterval));
        break;
    }
    case BuildingType::VAULT: // 存储类显示当前存储/最大存储
    case BuildingType::ELIXIR_BOTTLE: {
        int stored = (type == BuildingType::VAULT)
            ? dynamic_cast<Vault*>(targetBuilding)->getStorageCapacity()
            : dynamic_cast<ElixirBottle*>(targetBuilding)->getStorageCapacity();
        std::string resType = (type == BuildingType::VAULT) ? "gold capacity" : "elixir capacity";
        specialAttrLabel1->setString(StringUtils::format("%s:%d", resType.c_str(), stored));
        specialAttrLabel2->setString("");
        specialAttrLabel3->setString("");
        break;
    }

    case BuildingType::GOLD_MINE: // 生产类显示每分钟产量
    case BuildingType::ELIXIR_COLLECTOR:
    {
        int production = (type == BuildingType::GOLD_MINE)
            ? dynamic_cast<GoldMine*>(targetBuilding)->getProduce()
            : dynamic_cast<ElixirCollector*>(targetBuilding)->getProduce();
        std::string prodType = (type == BuildingType::GOLD_MINE) ? "gold" : "elixir";
        float produceInterval = (type == BuildingType::GOLD_MINE)
            ? dynamic_cast<GoldMine*>(targetBuilding)->getProduceInterval()
            : dynamic_cast<ElixirCollector*>(targetBuilding)->getProduceInterval();
        int store = (type == BuildingType::GOLD_MINE)
            ? dynamic_cast<GoldMine*>(targetBuilding)->getStored()
            : dynamic_cast<ElixirCollector*>(targetBuilding)->getStored();
        specialAttrLabel1->setString(StringUtils::format("produce %s: %d/%.1fs", prodType.c_str(), production, produceInterval));
        specialAttrLabel2->setString(StringUtils::format("Stored : %d", store));
        specialAttrLabel3->setString("");
        break;
    }
    default:
        specialAttrLabel1->setString("");
        specialAttrLabel2->setString("");
        specialAttrLabel3->setString("");
        break;
    }
}
