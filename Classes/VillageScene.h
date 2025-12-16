#ifndef __VILLAGE_SCENE_H__
#define __VILLAGE_SCENE_H__

#include "cocos2d.h"

// 使用命名空间（和cpp文件保持一致）
using namespace cocos2d;

class VillageScene : public cocos2d::Scene
{
public:
    // Cocos2d-x 标准创建方法（必须）
    static cocos2d::Scene* createScene();

    // 初始化方法
    virtual bool init();

    // CREATE_FUNC 宏：自动生成 create() 方法
    CREATE_FUNC(VillageScene);

private:
    // -------------------------- 成员变量 --------------------------
    // 地图核心对象
    TMXTiledMap* _tileMap;       // 等轴测地图对象
    Size _tileSize;              // 单个瓦片尺寸
    Size _mapSize;               // 地图总瓦片数（宽×高）
    TMXLayer* _placeLayer;       // 可放置建筑的图层（对应Tiled的place_layer）
     TMXLayer* _pathLayer;      // 后续可启用：可通行图层（暂注释）
     TMXLayer* _bgLayer;
    // 建筑放置相关（暂时屏蔽但保留声明）
    Sprite* _buildPreview;       // 建筑放置预览图

    // -------------------------- 方法声明 --------------------------
    // 初始化地图
    void initMap();

    // 初始化建筑预览（暂时屏蔽）
    void initBuildPreview();

    // 初始化触摸事件（暂时屏蔽）
    void initTouchEvent();

    // 坐标转换：屏幕坐标 → 等轴测瓦片坐标
    Vec2 screenToIsoTile(Vec2 screenPos);

    // 坐标转换：等轴测瓦片坐标 → 屏幕坐标
    Vec2 isoTileToScreen(Vec2 tilePos);

    // 检测瓦片是否可放置建筑
    bool checkCanPlace(Vec2 tilePos);

    // 放置建筑
    void placeBuilding(Vec2 tilePos);
};

#endif // __VILLAGE_SCENE_H__#pragma once
