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
    // 缩放相关
    float _minScale = 0.5f;      // 最小缩放比例（避免缩太小）
    float _maxScale = 2.0f;      // 最大缩放比例（避免缩太大）
    float _scaleStep = 0.1f;     // 滚轮每滚一次的缩放步长
    // 拖拽相关
    bool _isDragging = false;    // 是否正在拖拽
    Vec2 _lastMousePos;          // 上一帧鼠标位置
    Vec2 _mapOriginPos;          // 地图初始位置（用于计算偏移）
    // -------------------------- 方法声明 --------------------------
    // 滚轮回调函数
    // 事件回调（新增鼠标按下/移动/松开）
    void onMouseScroll(Event* event);
    void onMouseDown(Event* event);
    void onMouseMove(Event* event);
    void onMouseUp(Event* event);

    // 限制地图拖动范围（新增）
    void clampMapPosition();
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
