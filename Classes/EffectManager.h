#ifndef __EFFECT_MANAGER_H__
#define __EFFECT_MANAGER_H__

#include "cocos2d.h"
USING_NS_CC;

/**
 * 特效管理单例类
 * 核心功能：统一管理游戏中的抛射物特效（炮弹、箭矢、火球等）
 * 特性：全局唯一实例，封装特效创建/播放/销毁逻辑，避免内存泄漏
 */
class EffectManager {
private:
    // 静态成员：存储全局唯一的实例（单例核心）
    static EffectManager* _instance;

    // 私有构造函数：禁止外部直接创建对象（保证单例）
    EffectManager() = default;
    // 私有析构函数：禁止外部直接销毁对象
    ~EffectManager() = default;
    // 禁用拷贝构造和赋值运算符（防止单例被拷贝）
    EffectManager(const EffectManager&) = delete;
    EffectManager& operator=(const EffectManager&) = delete;

public:
    /**
     * 获取全局唯一的特效管理器实例
     * @return EffectManager* 单例实例
     */
    static EffectManager* getInstance();

    /**
     * 释放单例实例（游戏退出/场景销毁时调用，避免内存泄漏）
     */
    static void destroyInstance();

    /**
     * 播放抛射物特效（炮弹、箭矢等从起点飞向终点）
     * @param imgPath 特效图片路径（如 "effect/cannon_ball.png"）
     * @param start 特效起始位置（如建筑坐标）
     * @param end 特效结束位置（如敌方兵种坐标）
     * @param duration 特效飞行时长（秒，如0.5f）
     */
    void playProjectileEffect(const std::string& imgPath,
        const Vec2& start,
        const Vec2& end,
        float duration);

    /**
     * 播放爆炸特效（可选，攻击命中后触发）
     * @param imgPath 爆炸图片路径
     * @param pos 爆炸位置（目标坐标）
     * @param duration 爆炸特效持续时长
     */
    void playExplosionEffect(const std::string& imgPath,
        const Vec2& pos,
        float duration = 0.3f);
};

#endif // __EFFECT_MANAGER_H__#pragma once
