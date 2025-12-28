#ifndef __ENUM_TYPE_H__
#define __ENUM_TYPE_H__

#include <string>
#include <unordered_map>
#include <map>

/**
 * 游戏基础模式枚举.
 * 定义当前场景所处的交互模式.
 */
enum class BaseMode {
    NORMAL,   ///< 普通浏览模式
    CREATING, ///< 创造/编辑模式
    LEVEL,    ///< 关卡选择模式
    FIGHT     ///< 战斗模式
};

/**
 * 兵种类型枚举.
 * 定义游戏中所有可用的兵种单位.
 */
enum class TroopType {
    BARBARIAN, ///< 野蛮人 (近战肉盾)
    ARCHER,    ///< 弓箭手 (远程输出)
    GIANT,     ///< 巨人 (优先攻击建筑)
    BOMBER,    ///< 炸弹人 (破墙专家)
    UNKNOWN    ///< 未知/无效类型
};

/**
 * 建筑类型枚举.
 * 定义游戏中所有可建造的建筑.
 */
enum class BuildingType {
    TOWN_HALL,        ///< 大本营 (核心建筑)
    GOLD_MINE,        ///< 金矿 (生产金币)
    BARRACKS,         ///< 兵营 (增加人口)
    ELIXIR_COLLECTOR, ///< 圣水收集器 (生产圣水)
    TRAINING_CAMP,    ///< 训练营 (生产兵种)
    CANNON,           ///< 加农炮 (单体防御)
    ARROW_TOWER,      ///< 箭塔 (远程防御)
    WALL,             ///< 城墙 (阻挡地面单位)
    VAULT,            ///< 金库 (存储金币)
    ELIXIR_BOTTLE,    ///< 圣水瓶 (存储圣水)
    UNKNOWN           ///< 未知/无效类型
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
    const BuildingConfig& getBuildingConfigByType(BuildingType type, int level = 1);
    int getGoldCost(BuildingType type,int level=1);
    int getElixirCost(BuildingType type,int level=1);
#endif // __ENUM_TYPE_H__#pragma once
/**
 * 建筑状态枚举.
 * 定义建筑当前的生命周期状态.
 */
enum class BuildingState {
    IDLE,      ///< 空闲/正常工作状态
    BUILDING,  ///< 正在建造中
    ATTACKING, ///< 正在攻击中 (仅防御建筑)
    UPGRADING, ///< 正在升级中
    DESTROYED, ///< 已被摧毁
    TRAINING,  ///< 正在训练兵种 (仅训练营)
    UNKNOWN    ///< 未知状态
};

/**
 * 兵种状态枚举.
 * 定义兵种当前的行动状态.
 */
enum class TroopState {
    IDLE,      ///< 闲置/待机
    MOVING,    ///< 正在移动
    ATTACKING, ///< 正在攻击
    DEAD,      ///< 已死亡
    TRAINING,  ///< 正在训练中 (在训练营内)
    UNKNOWN    ///< 未知状态
};

/**
 * 建筑等级配置结构体.
 * 定义建筑在特定等级下的属性.
 */
struct BuildingLevelConfig {
    int level;              ///< 等级
    int goldCost;           ///< 升级所需金币
    int elixirCost;         ///< 升级所需圣水
    float buildTime;        ///< 升级耗时 (秒)
    int hp;                 ///< 该等级下的生命值
    std::string spritePath; ///< 该等级对应的纹理路径
};

/**
 * 建筑基础配置结构体.
 * 定义建筑的通用属性和初始状态.
 */
struct BuildingConfig {
    int id;                                         ///< 唯一ID
    BuildingType type;                              ///< 建筑类型
    std::string name;                               ///< 建筑名称
    std::string imgPath;                            ///< 基础纹理路径
    int hp;                                         ///< 基础生命值
    int tileWidth;                                  ///< 占用地图瓦片宽度
    int tileHeight;                                 ///< 占用地图瓦片高度
    std::unordered_map<std::string, int> cost;      ///< 建造消耗 (key:"gold"/"elixir")
    float buildTime;                                ///< 初始建造时长 (秒)
    int level = 1;                                  ///< 初始等级
    std::map<int, BuildingLevelConfig> levelConfigs;///< 各等级详细配置表
};

/**
 * 兵种基础配置结构体.
 * 定义兵种的战斗属性和训练需求.
 */
struct TroopConfig {
    int id;                 ///< 唯一ID
    TroopType type;         ///< 兵种类型
    std::string name;       ///< 兵种名称
    std::string imgPath;    ///< 纹理路径
    int hp;                 ///< 基础生命值
    int attackPower;        ///< 攻击力
    float attackRange;      ///< 攻击范围 (像素)
    float attackSpeed;      ///< 攻击间隔 (秒/次)
    float moveSpeed;        ///< 移动速度 (像素/秒)
    int elixirCost;         ///< 训练消耗 (圣水)
    float trainingTime;     ///< 训练时长 (秒)
    int level = 1;          ///< 初始等级
    int spaceCost;          ///< 占用人口空间
    int unlockCampLevel = 1;///< 解锁所需的训练营等级
};

/**
 * 获取指定类型的建筑配置.
 * * @param type 建筑类型.
 * @param level 建筑等级 (默认为1).
 * @return 对应的建筑配置结构体引用.
 */
const BuildingConfig& getBuildingConfigByType(BuildingType type, int level = 1);

/**
 * 获取建筑建造的金币消耗.
 * * @param type 建筑类型.
 * @return 金币数量.
 */
int getGoldCost(BuildingType type);

/**
 * 获取建筑建造的圣水消耗.
 * * @param type 建筑类型.
 * @return 圣水数量.
 */
int getElixirCost(BuildingType type);

#endif // __ENUM_TYPE_H__