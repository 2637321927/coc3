#include "PathFinder.h"
#include "cocos2d.h"
#include <algorithm>

using namespace cocos2d;

std::vector<Vec2> PathFinder::findPath(
    const Vec2& startPos,
    const Vec2& targetPos,
    TMXLayer* pathLayer,
    const Size& mapSize,
    const std::vector<Vec2>& occupiedTiles) {

    std::vector<Vec2> path;

    // 检查起点和终点是否有效
    if (!isTilePassable(startPos, pathLayer, mapSize, occupiedTiles) ||
        !isTilePassable(targetPos, pathLayer, mapSize, occupiedTiles)) {
        return path;
    }

    // 如果起点就是终点，直接返回
    if (startPos.equals(targetPos)) {
        path.push_back(startPos);
        return path;
    }

    // 初始化开放列表和关闭列表
    auto comparator = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
    std::priority_queue<Node*, std::vector<Node*>, decltype(comparator)> openList(comparator);
    std::unordered_map<int, Node*> allNodes;

    // 起点哈希值计算 (x * 10000 + y 确保唯一性)
    int startKey = static_cast<int>(startPos.x * 10000 + startPos.y);
    Node* startNode = new Node(startPos);
    openList.push(startNode);
    allNodes[startKey] = startNode;

    while (!openList.empty()) {
        // 获取fCost最小的节点
        Node* currentNode = openList.top();
        openList.pop();

        // 如果到达目标位置，回溯构建路径
        if (currentNode->tilePos.equals(targetPos)) {
            Node* temp = currentNode;
            while (temp) {
                path.push_back(temp->tilePos);
                temp = temp->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        // 处理邻居节点
        std::vector<Vec2> neighbors = getNeighbors(currentNode->tilePos);
        for (const Vec2& neighborPos : neighbors) {
            // 检查邻居是否可通过
            if (!isTilePassable(neighborPos, pathLayer, mapSize, occupiedTiles)) {
                continue;
            }

            // 计算代价
            float newGCost = currentNode->gCost + manhattanDistance(currentNode->tilePos, neighborPos);
            int neighborKey = static_cast<int>(neighborPos.x * 10000 + neighborPos.y);

            // 检查邻居是否已在列表中
            if (allNodes.find(neighborKey) == allNodes.end()) {
                // 新节点
                Node* neighborNode = new Node(neighborPos);
                neighborNode->gCost = newGCost;
                neighborNode->hCost = manhattanDistance(neighborPos, targetPos);
                neighborNode->parent = currentNode;

                openList.push(neighborNode);
                allNodes[neighborKey] = neighborNode;
            }
            else {
                // 已存在的节点，检查是否有更优路径
                Node* existingNode = allNodes[neighborKey];
                if (newGCost < existingNode->gCost) {
                    existingNode->gCost = newGCost;
                    existingNode->parent = currentNode;
                    // 重新加入优先队列（因为优先级已改变）
                    openList.push(existingNode);
                }
            }
        }
    }

    // 清理内存
    for (auto& pair : allNodes) {
        delete pair.second;
    }

    return path;
}

bool PathFinder::isTilePassable(
    const Vec2& tilePos,
    TMXLayer* pathLayer,
    const Size& mapSize,
    const std::vector<Vec2>& occupiedTiles) {

    // 检查是否在地图范围内
    if (tilePos.x < 0 || tilePos.x >= mapSize.width ||
        tilePos.y < 0 || tilePos.y >= mapSize.height) {
        return false;
    }

    // 检查是否是路径层允许通过的瓦片
    if (pathLayer && !pathLayer->getTileAt(tilePos)) {
        return false;
    }

    // 检查是否被建筑占用
    /*for (const Vec2& occupied : occupiedTiles) {
        if (occupied.equals(tilePos)) {
            return false;
        }
    }*/

    return true;
}

std::vector<Vec2> PathFinder::getNeighbors(const Vec2& tilePos) {
    std::vector<Vec2> neighbors;

    // 8个方向的邻居（上下左右及四个对角线）
    neighbors.emplace_back(tilePos.x + 1, tilePos.y);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y);
    neighbors.emplace_back(tilePos.x, tilePos.y + 1);
    neighbors.emplace_back(tilePos.x, tilePos.y - 1);
    neighbors.emplace_back(tilePos.x + 1, tilePos.y + 1);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y - 1);
    neighbors.emplace_back(tilePos.x + 1, tilePos.y - 1);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y + 1);

    return neighbors;
}