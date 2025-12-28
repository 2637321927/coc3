#include "PathFinder.h"
#include "cocos2d.h"
#include <algorithm>

using namespace cocos2d;

/**
 * A* 寻路算法核心实现.
 * 计算从起点到终点的最短路径.
 * * @param startPos 起点瓦片坐标.
 * @param targetPos 终点瓦片坐标.
 * @param pathLayer 地图路径层（用于检测地形阻挡，有瓦片的地方才可通行）.
 * @param mapSize 地图尺寸.
 * @param occupiedTiles 动态障碍物列表（如建筑占用的瓦片）.
 * @return 路径点列表（从起点到终点），若无路则返回空列表.
 */
std::vector<Vec2> PathFinder::findPath(
    const Vec2& startPos,
    const Vec2& targetPos,
    TMXLayer* pathLayer,
    const Size& mapSize,
    const std::vector<Vec2>& occupiedTiles) {

    std::vector<Vec2> path;

    // 1. 检查起点和终点是否有效
    if (!isTilePassable(startPos, pathLayer, mapSize, occupiedTiles) ||
        !isTilePassable(targetPos, pathLayer, mapSize, occupiedTiles)) {
        return path;
    }

    // 2. 如果起点就是终点，直接返回
    if (startPos.equals(targetPos)) {
        path.push_back(startPos);
        return path;
    }

    // 3. 初始化 A* 算法所需的数据结构
    // 开放列表 (Open List): 待检查的节点，按 fCost (g+h) 升序排列
    auto comparator = [](Node* a, Node* b) { return a->fCost() > b->fCost(); };
    std::priority_queue<Node*, std::vector<Node*>, decltype(comparator)> openList(comparator);

    // 所有节点记录 (All Nodes): 用于内存管理和查找是否已处理
    std::unordered_map<int, Node*> allNodes;

    // 起点入列
    // Key 生成规则: x * 10000 + y (假设地图尺寸不超过 10000x10000)
    int startKey = static_cast<int>(startPos.x * 10000 + startPos.y);
    Node* startNode = new Node(startPos);
    openList.push(startNode);
    allNodes[startKey] = startNode;

    // 4. 开始搜索循环
    while (!openList.empty()) {
        // 取出 fCost 最小的节点
        Node* currentNode = openList.top();
        openList.pop();

        // --- 找到终点 ---
        if (currentNode->tilePos.equals(targetPos)) {
            // 回溯构建路径：从终点反向追踪父节点到起点
            Node* temp = currentNode;
            while (temp) {
                path.push_back(temp->tilePos);
                temp = temp->parent;
            }
            // 反转路径，使其顺序变为 起点 -> 终点
            std::reverse(path.begin(), path.end());
            break;
        }

        // --- 处理邻居节点 ---
        std::vector<Vec2> neighbors = getNeighbors(currentNode->tilePos);
        for (const Vec2& neighborPos : neighbors) {
            // 过滤不可通过的邻居
            if (!isTilePassable(neighborPos, pathLayer, mapSize, occupiedTiles)) {
                continue;
            }

            // 计算新的 G 代价 (当前节点G + 移动代价)
            // 移动代价这里简化为曼哈顿距离或固定值
            float newGCost = currentNode->gCost + manhattanDistance(currentNode->tilePos, neighborPos);
            int neighborKey = static_cast<int>(neighborPos.x * 10000 + neighborPos.y);

            // 检查邻居状态
            if (allNodes.find(neighborKey) == allNodes.end()) {
                // 情况 A: 邻居从未被访问过 -> 创建新节点并加入 Open List
                Node* neighborNode = new Node(neighborPos);
                neighborNode->gCost = newGCost;
                neighborNode->hCost = manhattanDistance(neighborPos, targetPos); // 启发式代价
                neighborNode->parent = currentNode;

                openList.push(neighborNode);
                allNodes[neighborKey] = neighborNode;
            }
            else {
                // 情况 B: 邻居已存在 -> 检查是否发现了更优路径
                Node* existingNode = allNodes[neighborKey];
                if (newGCost < existingNode->gCost) {
                    // 更新代价和父节点
                    existingNode->gCost = newGCost;
                    existingNode->parent = currentNode;
                    // 重新加入 Open List 以便重新排序 (虽然可能有重复指针，但 A* 逻辑允许)
                    openList.push(existingNode);
                }
            }
        }
    }

    // 5. 清理内存 (释放所有动态分配的 Node)
    for (auto& pair : allNodes) {
        delete pair.second;
    }

    return path;
}

/**
 * 瓦片通行性检测.
 * 检查指定瓦片是否越界、是否为空地（无路径层数据）、是否被建筑占用.
 * * @param tilePos 目标瓦片坐标.
 * @param pathLayer 地图路径层（TMXLayer）.
 * @param mapSize 地图大小.
 * @param occupiedTiles 动态障碍物列表.
 * @return true 表示可通过，false 表示不可通过.
 */
bool PathFinder::isTilePassable(
    const Vec2& tilePos,
    TMXLayer* pathLayer,
    const Size& mapSize,
    const std::vector<Vec2>& occupiedTiles) {

    // 1. 检查是否在地图范围内
    if (tilePos.x < 0 || tilePos.x >= mapSize.width ||
        tilePos.y < 0 || tilePos.y >= mapSize.height) {
        return false;
    }

    // 2. 检查是否是路径层允许通过的瓦片
    // 逻辑：如果 pathLayer 存在，且该位置没有瓦片（GID=0），则视为障碍（悬崖/水域等）
    if (pathLayer && !pathLayer->getTileAt(tilePos)) {
        return false;
    }

    // 3. 检查是否被动态建筑占用
    for (const Vec2& occupied : occupiedTiles) {
        if (occupied.equals(tilePos)) {
            return false;
        }
    }

    return true;
}

/**
 * 获取相邻瓦片.
 * 生成当前瓦片周围的 8 个邻居坐标（支持米字形移动）.
 * * @param tilePos 中心瓦片坐标.
 * @return 邻居坐标列表.
 */
std::vector<Vec2> PathFinder::getNeighbors(const Vec2& tilePos) {
    std::vector<Vec2> neighbors;

    // 8个方向的邻居（上下左右及四个对角线）
    neighbors.emplace_back(tilePos.x + 1, tilePos.y);     // 右
    neighbors.emplace_back(tilePos.x - 1, tilePos.y);     // 左
    neighbors.emplace_back(tilePos.x, tilePos.y + 1);     // 上
    neighbors.emplace_back(tilePos.x, tilePos.y - 1);     // 下
    neighbors.emplace_back(tilePos.x + 1, tilePos.y + 1); // 右上
    neighbors.emplace_back(tilePos.x - 1, tilePos.y - 1); // 左下
    neighbors.emplace_back(tilePos.x + 1, tilePos.y - 1); // 右下
    neighbors.emplace_back(tilePos.x - 1, tilePos.y + 1); // 左上

    return neighbors;
}