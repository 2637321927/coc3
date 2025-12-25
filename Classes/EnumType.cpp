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