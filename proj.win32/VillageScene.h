#ifndef VILLAGE_SCENE_H
#define VILLAGE_SCENE_H

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

class VillageScene : public cocos2d::Scene
{
public:
    CREATE_FUNC(VillageScene);
    virtual bool init() override;
    static cocos2d::Scene* createScene(); // 静态创建方法
private:
    // 地图相关成员
    TMXTiledMap* _tileMap;       // 等轴测地图
    Size _tileSize;              // 瓦片尺寸（64×32）
    Size _mapSize;               // 地图总瓦片数
    TMXLayer* _placeLayer;       // 可放置建筑层
    TMXLayer* _pathLayer;        // 兵种可通行层
    Sprite* _buildPreview;       // 建筑放置预览图

    // 初始化函数
    void initMap();              // 加载等轴测地图
    void initTouchEvent();       // 初始化触摸交互
    void initBuildPreview();     // 初始化建筑预览

    // 坐标转换（等轴测核心）
    Vec2 screenToIsoTile(Vec2 screenPos);  // 屏幕→瓦片
    Vec2 isoTileToScreen(Vec2 tilePos);    // 瓦片→屏幕

    // 交互逻辑
    bool checkCanPlace(Vec2 tilePos);      // 检测是否可放置建筑
    void placeBuilding(Vec2 tilePos);      // 放置建筑
};

#endif // VILLAGE_SCENE_H#pragma once
