#pragma once
#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "cocos2d.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>

using namespace cocos2d;

/**
 * 寻路算法工具类.
 * 提供基于 A* 算法的网格路径搜索功能，支持静态地形和动态障碍物检测.
 */
class PathFinder {
public:
    /**
     * 寻路节点结构体.
     * 用于 A* 算法中存储每个网格的状态和代价.
     */
    struct Node {
        cocos2d::Vec2 tilePos; ///< 节点对应的瓦片坐标
        float gCost;           ///< G代价：从起点到当前节点的实际移动代价
        float hCost;           ///< H代价：从当前节点到终点的预估代价（启发式）
        Node* parent;          ///< 父节点指针（用于回溯构建路径）

        Node(cocos2d::Vec2 pos)
            : tilePos(pos), gCost(0), hCost(0), parent(nullptr) {
        }

        /** 获取 F代价 (总代价 = G + H). */
        float fCost() const { return gCost + hCost; }
    };

    /**
     * 计算曼哈顿距离.
     * 适用于只允许上下左右移动的网格地图，通常用作 H 代价.
     * * @param a 点 A.
     * @param b 点 B.
     * @return 两点间的曼哈顿距离 (|dx| + |dy|).
     */
    static float manhattanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    /**
     * 计算欧几里得距离.
     * 适用于允许任意方向移动或对角线移动的场景.
     * * @param a 点 A.
     * @param b 点 B.
     * @return 两点间的直线距离.
     */
    static float euclideanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
    }

    /**
     * 寻找路径 (A* 算法入口).
     * * @param startPos 起点瓦片坐标.
     * @param targetPos 终点瓦片坐标.
     * @param pathLayer 地图路径层（用于判断地形是否可通行）.
     * @param mapSize 地图总尺寸.
     * @param occupiedTiles 动态障碍物列表（如建筑占用的格子）.
     * @return 路径点列表（从起点到终点），若无路径则返回空列表.
     */
    static std::vector<cocos2d::Vec2> findPath(
        const cocos2d::Vec2& startPos,
        const cocos2d::Vec2& targetPos,
        cocos2d::TMXLayer* pathLayer,
        const cocos2d::Size& mapSize,
        const std::vector<cocos2d::Vec2>& occupiedTiles);

private:
    /**
     * 检查瓦片是否可通过.
     * 综合判断越界、地形阻挡（路径层）和动态障碍物.
     * * @param tilePos 待检查的瓦片坐标.
     * @param pathLayer 地图路径层.
     * @param mapSize 地图尺寸.
     * @param occupiedTiles 动态障碍物列表.
     * @return true 表示可通过，false 表示不可通过.
     */
    static bool isTilePassable(
        const cocos2d::Vec2& tilePos,
        cocos2d::TMXLayer* pathLayer,
        const cocos2d::Size& mapSize,
        const std::vector<cocos2d::Vec2>& occupiedTiles);

    /**
     * 获取邻居节点.
     * 获取当前瓦片周围可移动的相邻瓦片坐标（支持8方向或4方向）.
     * * @param tilePos 当前瓦片坐标.
     * @return 相邻瓦片坐标列表.
     */
    static std::vector<cocos2d::Vec2> getNeighbors(const cocos2d::Vec2& tilePos);
};

#endif // __PATH_FINDER_H__