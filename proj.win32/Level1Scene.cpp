// Level1Scene.cpp
#include "Level1Scene.h"
#include "BaseTroop.h"

USING_NS_CC;

Scene* Level1Scene::createScene() {
    auto scene = Scene::create();
    auto layer = Level1Scene::create();
    scene->addChild(layer);
    return scene;
}

bool Level1Scene::init() {
    if (!LevelScene::init()) return false;

    // 初始化关卡1
    if (!initLevel(1, "map/level1.tmx")) {
        return false;
    }

    _currentWave = 0;
    _totalWaves = 3;
    _waveTimer = 0.0f;

    return true;
}

void Level1Scene::startLevel() {
    // 显示教程
    showTutorial();

    // 开始第一波敌人
    scheduleOnce([this](float dt) {
        spawnEnemyTroops();
        }, 3.0f, "first_wave");

    // 更新波次
    schedule(CC_SCHEDULE_SELECTOR(Level1Scene::updateWave), 1.0f);
}

void Level1Scene::showTutorial() {
    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto tutorialBg = LayerColor::create(Color4B(0, 0, 0, 220), 500, 300);
    tutorialBg->setPosition(Vec2(visibleSize.width / 2 - 250, visibleSize.height / 2 - 150));
    this->addChild(tutorialBg, 101);

    auto title = Label::createWithTTF("新手训练", "fonts/Marker Felt.ttf", 36);
    title->setColor(Color3B::YELLOW);
    title->setPosition(Vec2(250, 250));
    tutorialBg->addChild(title);

    auto tip1 = Label::createWithTTF("目标: 摧毁所有敌方建筑", "fonts/Marker Felt.ttf", 24);
    tip1->setColor(Color3B::WHITE);
    tip1->setPosition(Vec2(250, 180));
    tutorialBg->addChild(tip1);

    auto tip2 = Label::createWithTTF("操作: 鼠标拖拽移动，滚轮缩放", "fonts/Marker Felt.ttf", 24);
    tip2->setColor(Color3B::WHITE);
    tip2->setPosition(Vec2(250, 140));
    tutorialBg->addChild(tip2);

    auto tip3 = Label::createWithTTF("提示: 点击返回按钮可回到村庄", "fonts/Marker Felt.ttf", 24);
    tip3->setColor(Color3B::WHITE);
    tip3->setPosition(Vec2(250, 100));
    tutorialBg->addChild(tip3);

    auto closeBtn = ui::Button::create("ui/ok_btn_normal.png",
        "ui/ok_btn_selected.png");
    closeBtn->setPosition(Vec2(250, 50));
    closeBtn->addClickEventListener([tutorialBg](Ref* sender) {
        tutorialBg->removeFromParent();
        });
    tutorialBg->addChild(closeBtn);
}

void Level1Scene::spawnEnemyTroops() {
    if (_currentWave >= _totalWaves) return;

    _currentWave++;

    // 在关卡左边生成敌人
    for (int i = 0; i < 3 + _currentWave; i++) {
        auto troop = BaseTroop::create(TroopType::BARBARIAN, Vec2(5 + i, 5 + i), 1.0f);
        if (troop) {
            troop->setScale(0.8f);

            // 转换为容器坐标并放置
            float centerTileX = 5 + i + 0.5f;
            float centerTileY = 5 + i + 0.5f;
            Vec2 centerTilePos(centerTileX, centerTileY);

            // 使用与村庄场景相同的坐标转换
            Vec2 containerLocalPos = _tileMap->convertToNodeSpace(isoTileToScreen(centerTilePos));
            troop->setPosition(containerLocalPos);

            _mapContainer->addChild(troop);
        }
    }

    // 更新波次标签
    if (_levelLabel) {
        _levelLabel->setString(StringUtils::format("关卡 %d - 波次 %d/%d",
            _currentLevel, _currentWave, _totalWaves));
    }
}

void Level1Scene::updateWave(float dt) {
    _waveTimer += dt;

    // 每30秒生成一波新敌人
    if (_waveTimer >= 30.0f && _currentWave < _totalWaves) {
        _waveTimer = 0.0f;
        spawnEnemyTroops();
    }
}

void Level1Scene::onLevelComplete(bool success) {
    unscheduleAllCallbacks();

    Size visibleSize = Director::getInstance()->getVisibleSize();

    auto resultBg = LayerColor::create(Color4B(0, 0, 0, 200), 400, 300);
    resultBg->setPosition(Vec2(visibleSize.width / 2 - 200, visibleSize.height / 2 - 150));
    this->addChild(resultBg, 101);

    auto title = Label::createWithTTF(success ? "胜利！" : "失败",
        "fonts/Marker Felt.ttf", 48);
    title->setColor(success ? Color3B::GREEN : Color3B::RED);
    title->setPosition(Vec2(200, 220));
    resultBg->addChild(title);

    auto score = Label::createWithTTF(StringUtils::format("获得星星: %d", _starsEarned),
        "fonts/Marker Felt.ttf", 32);
    score->setColor(Color3B::YELLOW);
    score->setPosition(Vec2(200, 150));
    resultBg->addChild(score);

    auto returnBtn = ui::Button::create("ui/return_btn_normal.png",
        "ui/return_btn_selected.png");
    returnBtn->setPosition(Vec2(200, 80));
    returnBtn->addClickEventListener([this](Ref* sender) {
        this->returnToVillage();
        });
    resultBg->addChild(returnBtn);

    // 保存关卡进度
    UserDefault::getInstance()->setIntegerForKey("level1_stars", _starsEarned);
    UserDefault::getInstance()->setBoolForKey("level1_completed", true);
}