#ifndef __EFFECT_MANAGER_H__
#define __EFFECT_MANAGER_H__

#include "cocos2d.h"
USING_NS_CC;

/**
 * 特效管理单例类.
 * 核心功能：统一管理游戏中的抛射物特效（炮弹、箭矢、火球等）.
 * 特性：全局唯一实例，封装特效创建/播放/销毁逻辑，避免内存泄漏.
 */
class EffectManager {
private:
    /** 静态成员：存储全局唯一的实例（单例核心）. */
    static EffectManager* _instance;

    /** 私有构造函数：禁止外部直接创建对象. */
    EffectManager() = default;

    /** 私有析构函数：禁止外部直接销毁对象. */
    ~EffectManager() = default;

    /** 禁用拷贝构造：防止单例被拷贝. */
    EffectManager(const EffectManager&) = delete;

    /** 禁用赋值运算符：防止单例被赋值. */
    EffectManager& operator=(const EffectManager&) = delete;

public:
    /**
     * 获取特效管理器单例实例.
     * 如果实例不存在则创建，否则返回已有实例.
     * * @return EffectManager* 全局唯一的实例指针.
     */
    static EffectManager* getInstance();

    /**
     * 销毁单例实例.
     * 释放内存并置空指针，通常在游戏退出或场景销毁时调用.
     */
    static void destroyInstance();

    /**
     * 播放抛射物特效.
     * 创建一个从起点飞向终点的精灵（如炮弹、箭矢），到达终点后自动销毁.
     * * @param imgPath 特效图片的资源路径（如 "effect/cannon_ball.png"）.
     * @param start 特效起始位置（世界坐标）.
     * @param end 特效结束位置（世界坐标）.
     * @param duration 特效飞行时长（秒）.
     */
    void playProjectileEffect(const std::string& imgPath,
        const Vec2& start,
        const Vec2& end,
        float duration);

    /**
     * 播放爆炸/命中特效.
     * 在指定位置播放一次性的特效动画.
     * * @param imgPath 爆炸图片的资源路径.
     * @param pos 爆炸位置（世界坐标）.
     * @param duration 爆炸特效持续时长（秒，默认0.3秒）.
     */
    void playExplosionEffect(const std::string& imgPath,
        const Vec2& pos,
        float duration = 0.3f);
};

#endif // __EFFECT_MANAGER_H__