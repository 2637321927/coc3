#ifndef __ENUM_TYPE_H__
#define __ENUM_TYPE_H__
#include <string>
#include <unordered_map>
enum class BaseMode {
    NORMAL,   //普通
    CREATING, //创造
    LEVEL,     //关卡
	FIGHT     //战斗
};
// 兵种类型枚举
enum class TroopType {
    BARBARIAN,    // 野蛮人
    ARCHER,       // 弓箭手
    GIANT,        // 巨人
    BOMBER,       // 炸弹人
    UNKNOWN       
};

// 建筑类型枚举
enum class BuildingType {
    TOWN_HALL,   // 大本营
    GOLD_MINE,   // 金矿
    BARRACKS,    // 兵营
    ELIXIR_COLLECTOR, // 圣水收集器
    TRAINING_CAMP, // 训练营
	CANNON, // 加农炮
	ARROW_TOWER, // 箭塔
	WALL,         // 城墙
	VAULT,        // 金库
	ELIXIR_BOTTLE, // 圣水瓶
    UNKNOWN
};

// 建筑状态枚举
enum class BuildingState {
    IDLE,        // 正常
    BUILDING,    // 建造中
    ATTACKING,      // 攻击中
    UPGRADING,   // 升级中
    DESTROYED,   // 摧毁
    TRAINING,    // 训练中（仅训练营）
    UNKNOWN
};
// 兵种状态枚举
enum class TroopState {
    IDLE,         // 闲置
    MOVING,       // 移动中
    ATTACKING,    // 攻击中
    DEAD,         // 死亡
    TRAINING,     // 训练中
    UNKNOWN
};
struct BuildingConfig {
    int id;                     // 唯一ID
    BuildingType type;          // 建筑类型
    std::string name;           // 名称
    std::string imgPath;        // 纹理路径
    int hp;                     // 生命值
    int tileWidth;              // 占用瓦片宽
    int tileHeight;             // 占用瓦片高
    std::unordered_map<std::string, int> cost; // 建造消耗（金币/圣水）
    float buildTime;            // 建造时长（秒）
    int level = 1;              // 初始等级
};
struct TroopConfig {
    int id;                     // 唯一ID
    TroopType type;             // 兵种类型
    std::string name;           // 名称
    std::string imgPath;        // 纹理路径
    int hp;                     // 生命值
    int attackPower;            // 攻击力
    float attackRange;          // 攻击范围（像素）
    float attackSpeed;          // 攻击速度（攻击间隔，秒/次）
    float moveSpeed;            // 移动速度（像素/秒）
    int elixirCost; // 训练消耗（圣水）
    float trainingTime;         // 训练时长（秒）
    int level = 1;              // 初始等级
    int spaceCost;              // 占用人口空间
    int unlockCampLevel = 1;        // 解锁所需兵营等级
};
const BuildingConfig& getBuildingConfigByType(BuildingType type,int level=1);
#endif // __ENUM_TYPE_H__#pragma once
