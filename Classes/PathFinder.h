#pragma once
#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "cocos2d.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>

using namespace cocos2d;

class PathFinder {
public:
    struct Node {
        cocos2d::Vec2 tilePos;
        float gCost; ///< 起点到当前节点的代价
        float hCost; ///< 当前节点到终点的预估代价
        Node* parent;

        Node(cocos2d::Vec2 pos)
            : tilePos(pos), gCost(0), hCost(0), parent(nullptr) {
        }

        float fCost() const { return gCost + hCost; }
    };

    /**
     * 计算两个点之间的曼哈顿距离.
     * @param a 点A.
     * @param b 点B.
     * @return 距离值.
     */
    static float manhattanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    /**
     * 计算两个点之间的欧几里得距离.
     * @param a 点A.
     * @param b 点B.
     * @return 距离值.
     */
    static float euclideanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
    }

    /**
     * 寻找路径.
     * A* 寻路算法实现.
     *
     * @param startPos 起点坐标.
     * @param targetPos 终点坐标.
     * @param pathLayer 路径层数据.
     * @param mapSize 地图尺寸.
     * @param occupiedTiles 已被占用的瓦片列表.
     * @return 路径点集合 (std::vector<cocos2d::Vec2>).
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
     *
     * @param tilePos 目标瓦片坐标.
     * @param pathLayer 路径层.
     * @param mapSize 地图尺寸.
     * @param occupiedTiles 占用列表.
     * @return 如果可通过返回 true，否则返回 false.
     */
    static bool isTilePassable(
        const cocos2d::Vec2& tilePos,
        cocos2d::TMXLayer* pathLayer,
        const cocos2d::Size& mapSize,
        const std::vector<cocos2d::Vec2>& occupiedTiles);

    /**
     * 获取邻居节点.
     * 获取当前节点上下左右的相邻节点坐标.
     *
     * @param tilePos 当前瓦片坐标.
     * @return 相邻瓦片坐标列表.
     */
    static std::vector<cocos2d::Vec2> getNeighbors(const cocos2d::Vec2& tilePos);
};

#endif // __PATH_FINDER_H__