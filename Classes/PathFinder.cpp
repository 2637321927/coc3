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

    // 1. 简单的起点/终点校验 (注意：如果终点是建筑，它是被占用的，所以这里不能简单判断终点是否Passable)
    if (!isTilePassable(startPos, pathLayer, mapSize, occupiedTiles)) {
        // 如果出生点就被卡住，暂时返回空，或者允许走出
        // return path; 
    }

    if (startPos.equals(targetPos)) {
        path.push_back(startPos);
        return path;
    }

    // A* 初始化
    auto comparator = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
    std::priority_queue<Node*, std::vector<Node*>, decltype(comparator)> openList(comparator);
    std::unordered_map<int, Node*> allNodes;

    int startKey = static_cast<int>(startPos.x * 10000 + startPos.y);
    Node* startNode = new Node(startPos);
    openList.push(startNode);
    allNodes[startKey] = startNode;

    while (!openList.empty()) {
        Node* currentNode = openList.top();
        openList.pop();

        // 找到终点
        if (currentNode->tilePos.equals(targetPos)) {
            Node* temp = currentNode;
            while (temp) {
                path.push_back(temp->tilePos);
                temp = temp->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        // 遍历邻居
        std::vector<Vec2> neighbors = getNeighbors(currentNode->tilePos);
        for (const Vec2& neighborPos : neighbors) {

            // 【关键修复】障碍物判断逻辑
            // 如果邻居是不可通行的...
            if (!isTilePassable(neighborPos, pathLayer, mapSize, occupiedTiles)) {
                // ...但如果这个邻居恰好是我们的攻击目标(targetPos)，允许进入！
                // 否则跳过
                if (!neighborPos.equals(targetPos)) {
                    continue;
                }
            }

            float newGCost = currentNode->gCost + manhattanDistance(currentNode->tilePos, neighborPos);
            int neighborKey = static_cast<int>(neighborPos.x * 10000 + neighborPos.y);

            if (allNodes.find(neighborKey) == allNodes.end()) {
                Node* neighborNode = new Node(neighborPos);
                neighborNode->gCost = newGCost;
                neighborNode->hCost = manhattanDistance(neighborPos, targetPos);
                neighborNode->parent = currentNode;
                openList.push(neighborNode);
                allNodes[neighborKey] = neighborNode;
            }
            else {
                Node* existingNode = allNodes[neighborKey];
                if (newGCost < existingNode->gCost) {
                    existingNode->gCost = newGCost;
                    existingNode->parent = currentNode;
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

    // 1. 越界检查
    if (tilePos.x < 0 || tilePos.x >= mapSize.width ||
        tilePos.y < 0 || tilePos.y >= mapSize.height) {
        return false;
    }

    // 2. 路径层检查 (如果需要)
    /*if (pathLayer && !pathLayer->getTileAt(tilePos)) {
        return false;
    }*/

    // 3. 占用检查 (取消注释，启用障碍物)
    for (const Vec2& occupied : occupiedTiles) {
        if (occupied.equals(tilePos)) {
            return false;
        }
    }

    return true;
}

// getNeighbors 保持原样...
std::vector<Vec2> PathFinder::getNeighbors(const Vec2& tilePos) {
    std::vector<Vec2> neighbors;
    neighbors.emplace_back(tilePos.x + 1, tilePos.y);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y);
    neighbors.emplace_back(tilePos.x, tilePos.y + 1);
    neighbors.emplace_back(tilePos.x, tilePos.y - 1);
    // 对角线移动（可选，如果不想让兵种穿墙角，可以注释掉下面4行）
    neighbors.emplace_back(tilePos.x + 1, tilePos.y + 1);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y - 1);
    neighbors.emplace_back(tilePos.x + 1, tilePos.y - 1);
    neighbors.emplace_back(tilePos.x - 1, tilePos.y + 1);
    return neighbors;
}