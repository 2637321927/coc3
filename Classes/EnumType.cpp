#include "EnumType.h" 
#include "VillageScene.h"
#include <unordered_map>

std::unordered_map<TroopType, TroopConfig> g_troopTrainConfig = {
  {TroopType::BARBARIAN, {
      1001,
      TroopType::BARBARIAN,
      "Barbarian",
      "troops/barbarian.png",
      400,
      80,
      50.0f,
      1.0f,
      120.0f,
      25,
      2.0f,
      1,
      1,
      1
  }},
  {TroopType::ARCHER, {
      1002,                  // 弓箭手唯一ID（区别于野蛮人1001）
      TroopType::ARCHER,   // 兵种类型为弓箭手
      "Archer",       // 兵种名称
      "troops/archer.png", // 弓箭手纹理路径（需替换为你的实际资源）
       200,                // 生命值（比野蛮人低，远程脆皮）
      60,         // 攻击力（远程攻击，略低于野蛮人）
      200.0f,      // 攻击范围（远程核心，远大于野蛮人）
      1.5f,         // 攻击速度（比野蛮人慢，远程平衡）
      100.0f,         // 移动速度（比野蛮人稍慢）
      50, // 训练消耗（圣水50，比野蛮人高）
      3.0f,       // 训练时长（3秒）
       1,                 // 初始等级
      1,             // 占用人口（和野蛮人一致）
      1
  }},
  {TroopType::GIANT, {
          1003,
      TroopType::GIANT,
      "Giant",
      "troops/giant.png",
      1000,
      50,
      30.0f,
      1.5f,
      60.0f,
      150,
      8.0f,
      1,
      5,
      1
  }}
};
//获取建筑的配置(新增建筑类型时需要在此添加对应配置)
const BuildingConfig& getBuildingConfigByType(BuildingType type,int level)
{
    static BuildingConfig Config;
    switch (type) {
    case(BuildingType::TOWN_HALL):
        Config = {
            1,                      // id
            BuildingType::TOWN_HALL,// type
            "大本营",               // name
            "building/town_hall.png", // imgPath
            2000,                   // hp
            3,                      // tileWidth
            3,                      // tileHeight
            { {"gold", 1000}, {"elixir", 500} }, // cost
            30.0f                   // buildTime
        };
        break;
    case(BuildingType::GOLD_MINE):
        Config = {
            2,                      // id
            BuildingType::GOLD_MINE,// type
            "金矿",                 // name
            "building/55.png", // imgPath
            500,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 500}, {"elixir", 200} }, // cost
            10.0f                   // buildTime
        };
        break;

    case(BuildingType::ELIXIR_COLLECTOR):
        Config = {
            4,                      // id
            BuildingType::ELIXIR_COLLECTOR, // type
            "圣水收集器",           // name
            "building/elixir_collector.png", // imgPath
            500,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 600}, {"elixir", 250} }, // cost
            12.0f                   // buildTime
        };
        break;
    case(BuildingType::BARRACKS):
        Config = {
            3,                      // id
            BuildingType::BARRACKS, // type
            "兵营",                 // name
            "building/barracks.png", // imgPath
            800,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 800}, {"elixir", 300} }, // cost
            20.0f                   // buildTime
        };
        break;
    case(BuildingType::TRAINING_CAMP):
        Config = {
            5,                      // id
            BuildingType::TRAINING_CAMP, // type
            "训练营",                 // name
            "building/training_camp.png", // imgPath
            800,                    // hp
            1,                      // tileWidth
            1,                      // tileHeight
            { {"gold", 900}, {"elixir", 400} }, // cost
            15.0f                   // buildTime
        };
        break;
    case(BuildingType::CANNON):
        Config = {
            6,                      // id
            BuildingType::CANNON, // type
            "加农炮",                 // name
            "building/cannon.png", // imgPath
            1000,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 1200}, {"elixir", 0} }, // cost
            25.0f                   // buildTime
        };
        break;
    case(BuildingType::ARROW_TOWER):
        Config = {
            7,                      // id
            BuildingType::ARROW_TOWER, // type
            "箭塔",                 // name
            "building/arrow_tower.png", // imgPath
            900,                    // hp
            2,                      // tileWidth
            2,                      // tileHeight
            { {"gold", 1100}, {"elixir", 0} }, // cost
            22.0f                   // buildTime
        };
        break;
	case(BuildingType::WALL):
		Config = {
			8,
			BuildingType::WALL,
			"城墙",
			"building/wall.png",
			1500,
			1,
			1,
			{ {"gold", 300}, {"elixir", 0} },
			0.0f
		};
		break;
	case(BuildingType::VAULT):
        Config = {
            9,
            BuildingType::VAULT,
            "金库",
            "building/vault.png",
            2500,
            2,
            2,
            { {"gold", 2000}, {"elixir", 0} },
            40.0f
        };
        break;
	case(BuildingType::ELIXIR_BOTTLE):
		Config = {
			10,
			BuildingType::ELIXIR_BOTTLE,
			"圣水瓶",
			"building/elixir_bottle.png",
			2500,
			2,
			2,
			{ {"gold", 0}, {"elixir", 2000} },
			40.0f
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
    return  Config;
}