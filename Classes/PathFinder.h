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
        float gCost; // 起点到当前节点的代价
        float hCost; // 当前节点到终点的预估代价
        Node* parent;

        Node(cocos2d::Vec2 pos)
            : tilePos(pos), gCost(0), hCost(0), parent(nullptr) {
        }

        float fCost() const { return gCost + hCost; }
    };

    // 计算两个点之间的曼哈顿距离
    static float manhattanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    // 计算两个点之间的欧几里得距离
    static float euclideanDistance(const cocos2d::Vec2& a, const cocos2d::Vec2& b) {
        return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
    }

    // 寻找路径
    static std::vector<cocos2d::Vec2> findPath(
        const cocos2d::Vec2& startPos,
        const cocos2d::Vec2& targetPos,
        cocos2d::TMXLayer* pathLayer,
        const cocos2d::Size& mapSize,
        const std::vector<cocos2d::Vec2>& occupiedTiles);

private:
    // 检查瓦片是否可通过
    static bool isTilePassable(
        const cocos2d::Vec2& tilePos,
        cocos2d::TMXLayer* pathLayer,
        const cocos2d::Size& mapSize,
        const std::vector<cocos2d::Vec2>& occupiedTiles);

    // 获取邻居节点
    static std::vector<cocos2d::Vec2> getNeighbors(const cocos2d::Vec2& tilePos);
};

#endif // __PATH_FINDER_H__
