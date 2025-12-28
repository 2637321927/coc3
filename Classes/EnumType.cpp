#include "EnumType.h" 
#include "VillageScene.h"
#include <unordered_map>

/**
 * 全局兵种训练配置表.
 * Key: 兵种类型枚举.
 * Value: 兵种的具体属性配置 (ID, 名称, 血量, 攻击力, 训练消耗等).
 */
std::unordered_map<TroopType, TroopConfig> g_troopTrainConfig = {
    {TroopType::BARBARIAN, {
        1001,                   // id
        TroopType::BARBARIAN,   // type
        "Barbarian",            // name
        "troops/barbarian.png", // imgPath
        400,                    // hp
        80,                     // attackPower
        50.0f,                  // attackRange
        1.0f,                   // attackSpeed
        120.0f,                 // moveSpeed
        25,                     // elixirCost
        2.0f,                   // trainingTime
        1,                      // level
        1,                      // spaceCost
        1                       // unlockCampLevel
    }},
    {TroopType::ARCHER, {
        1002,                   // id
        TroopType::ARCHER,      // type
        "Archer",               // name
        "troops/archer.png",    // imgPath
        200,                    // hp (比野蛮人低)
        60,                     // attackPower
        200.0f,                 // attackRange (远程)
        1.5f,                   // attackSpeed
        100.0f,                 // moveSpeed
        50,                     // elixirCost
        3.0f,                   // trainingTime
        1,                      // level
        1,                      // spaceCost
        1                       // unlockCampLevel
    }},
    {TroopType::GIANT, {
        1003,                   // id
        TroopType::GIANT,       // type
        "Giant",                // name
        "troops/giant.png",     // imgPath
        1000,                   // hp (肉盾)
        50,                     // attackPower
        30.0f,                  // attackRange
        1.5f,                   // attackSpeed
        60.0f,                  // moveSpeed (移动慢)
        150,                    // elixirCost
        8.0f,                   // trainingTime
        1,                      // level
        5,                      // spaceCost (占5人口)
        1                       // unlockCampLevel
    }}
};

/**
 * 获取建筑建造所需的金币成本.
 * 辅助函数，从配置中提取 gold 字段.
 * * @param type 建筑类型.
 * @return 金币消耗数量 (如果未配置则返回 0).
 */
int getGoldCost(BuildingType type, int level) {
    auto config = getBuildingConfigByType(type, level);
    std::unordered_map<std::string, int>::const_iterator it = config.cost.find("gold");
    if (it != config.cost.end()) {
        return it->second;
    }
    return 0;
}

/**
 * 获取建筑建造所需的圣水成本.
 * 辅助函数，从配置中提取 elixir 字段.
 * * @param type 建筑类型.
 * @return 圣水消耗数量 (如果未配置则返回 0).
 */
int getElixirCost(BuildingType type, int level) {
    auto config = getBuildingConfigByType(type, level);
    std::unordered_map<std::string, int>::const_iterator it = config.cost.find("elixir");
    if (it != config.cost.end()) {
        return it->second;
    }
    return 0;
}

/**
 * 获取建筑的基础配置数据.
 * 根据建筑类型返回对应的静态配置（包含贴图路径、占地大小、血量、造价等）.
 * 注意：新增建筑类型时需要在此处添加 case.
 * * @param type 建筑类型枚举.
 * @param level 建筑等级 (预留参数，目前返回默认等级配置).
 * @return BuildingConfig 配置结构体的常量引用.
 */
const BuildingConfig& getBuildingConfigByType(BuildingType type, int level)
{
    static BuildingConfig Config;
    switch (type) {
    case(BuildingType::TOWN_HALL):
        Config = {
            1,                      // id
            BuildingType::TOWN_HALL,// type
            "大本营",               // name
            "building/town_hall.png", // imgPath
            2000+level*1000,                   // hp
            3,                      // tileWidth
            3,                      // tileHeight
            { {"gold", 1000*level}, {"elixir", 0} }, // cost
            30.0f+level*5                   // buildTime
        };
        break;

    case(BuildingType::GOLD_MINE):
        Config = {
            2,                      // id
            BuildingType::GOLD_MINE,// type
            "金矿",                 // name
            "building/gold_mine.png", // imgPath
            500 * level,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 0}, {"elixir", 200*level} }, // cost
            10.0f + level * 5                     // buildTime
        };
        break;

    case(BuildingType::ELIXIR_COLLECTOR):
        Config = {
            4,                          // id
            BuildingType::ELIXIR_COLLECTOR, // type
            "圣水收集器",               // name
            "building/elixir_collector.png", // imgPath
            500 * level,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 200*level}, {"elixir",0} }, // cost
            12.0f + level * 5                   // buildTime
        };
        break;

    case(BuildingType::BARRACKS):
        Config = {
            3,                      // id
            BuildingType::BARRACKS, // type
            "兵营",                 // name
            "building/barracks.png", // imgPath
            800+600*level,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 800+600*level}, {"elixir", 300+200*level} }, // cost
            20.0f +level*5                  // buildTime
        };
        break;

    case(BuildingType::TRAINING_CAMP):
        Config = {
            5,                          // id
            BuildingType::TRAINING_CAMP,// type
            "训练营",                   // name
            "building/training_camp.png", // imgPath
            800 + 600 * level,                    // hp
            1,                      // tileWidth
            1,                      // tileHeight
            { {"gold", 900*level}, {"elixir",0} }, // cost
            15.0f + level * 5                 // buildTime
        };
        break;

    case(BuildingType::CANNON):
        Config = {
            6,                      // id
            BuildingType::CANNON, // type
            "加农炮",                 // name
            "building/cannon.png", // imgPath
            1000 + level * 400,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 1000 + level * 800}, {"elixir", 0} }, // cost
            25.0f +level * 5                  // buildTime
        };
        break;

    case(BuildingType::ARROW_TOWER):
        Config = {
            7,                          // id
            BuildingType::ARROW_TOWER,  // type
            "箭塔",                     // name
            "building/arrow_tower.png", // imgPath
            900 + level * 300,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 1200 + level * 1000}, {"elixir", 0} }, // cost
            22.0f + level * 5                      // buildTime
        };
        break;
	case(BuildingType::WALL):
		Config = {
			8,
			BuildingType::WALL,
			"城墙",
			"building/wall.png",
			800*level,
			1,
			1,
			{ {"gold", 300*level}, {"elixir", 0} },
			0.0f
		};
		break;
	case(BuildingType::VAULT):
        Config = {
            9,
            BuildingType::VAULT,
            "金库",
            "building/vault.png",
            2500 + level * 1000,
            2,
            2,
            { {"gold", 2000 +level*500}, {"elixir", 0} },
            40.0f + level * 5
        };
        break;
	case(BuildingType::ELIXIR_BOTTLE):
		Config = {
			10,
			BuildingType::ELIXIR_BOTTLE,
			"圣水瓶",
			"building/elixir_bottle.png",
			2500 + level * 1000,
			2,
			2,
			{ {"gold", 0}, {"elixir", 2000 + level * 500} },
			40.0f + level * 5
		};
		break;
    default:
        Config = {
            -1,
            BuildingType::UNKNOWN,
            "未知建筑",
            "",
            0,
            0,
            0,
            {},
            0.0f
        };
        break;
    }
    return Config;
}