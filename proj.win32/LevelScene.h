#ifndef __LEVEL_SCENE_H__
#define __LEVEL_SCENE_H__

#include "cocos2d.h"
#include "Building.h"
#include "Troop.h"
#include "VillageScene.h"
class VillageScene;
class LevelScene : public cocos2d::Scene {
public:
    virtual bool init() override;
    virtual void onEnter() override;

    // 创建场景
    static cocos2d::Scene* createScene();

    // 创建关卡场景的通用函数
    virtual bool initLevel(int levelNumber, const std::string& mapFile);

    // 返回主村庄
    void returnToVillage();

    // 显示关卡信息
    void showLevelInfo();

protected:
    cocos2d::Node* _mapContainer;
    cocos2d::TMXTiledMap* _tileMap;
    cocos2d::TMXLayer* _bgLayer;
    cocos2d::TMXLayer* _pathLayer;

    cocos2d::Size _tileSize;
    cocos2d::Size _mapSize;


    cocos2d::Label* _levelLabel;

    // 关卡数据
    int _currentLevel;
    int _starsEarned; // 获得的星星数
    int _maxTroops;   // 最大可用兵数

    // 触摸事件
    bool _isDragging;
    cocos2d::Vec2 _lastMousePos;
    cocos2d::Vec2 _mapOriginPos;

    // 缩放相关
    float _minScale;
    float _maxScale;
    float _scaleStep;

    // 初始化函数
    virtual void initMap(const std::string& mapFile);
    void initUI();
    void initTouchEvents();

    // 事件回调
    void onMouseScroll(cocos2d::Event* event);
    void onMouseDown(cocos2d::Event* event);
    void onMouseMove(cocos2d::Event* event);
    void onMouseUp(cocos2d::Event* event);

    // 工具函数
    cocos2d::Vec2 screenToIsoTile(cocos2d::Vec2 screenPos);
    cocos2d::Vec2 isoTileToScreen(cocos2d::Vec2 tilePos);
    void clampMapPosition();

    // 关卡逻辑
    virtual void startLevel();
    virtual void checkLevelComplete();
    virtual void onLevelComplete(bool success);

    CREATE_FUNC(LevelScene);
};

#endif // __LEVEL_SCENE_H__