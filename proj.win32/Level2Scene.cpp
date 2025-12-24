// Level2Scene.cpp
#include "Level2Scene.h"

USING_NS_CC;

Scene* Level2Scene::createScene() {
    auto scene = Scene::create();
    auto layer = Level2Scene::create();
    scene->addChild(layer);
    return scene;
}

bool Level2Scene::init() {
    if (!LevelScene::init()) return false;

    // 初始化关卡2
    if (!initLevel(2, "map/level2.tmx")) {
        return false;
    }

    return true;
}

void Level2Scene::startLevel() {
    _levelLabel->setString("关卡 2: 丛林之战");

    // 这里可以添加关卡2特有的逻辑
}

void Level2Scene::onLevelComplete(bool success) {
    // 实现关卡2的完成逻辑
    LevelScene::onLevelComplete(success);
}