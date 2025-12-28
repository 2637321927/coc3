#include "EffectManager.h"

// 初始化静态单例实例
EffectManager* EffectManager::_instance = nullptr;

/**
 * 获取单例实例.
 * 使用懒加载模式，确保在首次调用时创建实例.
 * * @return EffectManager 的唯一实例指针.
 */
EffectManager* EffectManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new (std::nothrow) EffectManager(); // 安全创建，避免内存分配失败崩溃
    }
    return _instance;
}

/**
 * 释放单例实例.
 * 销毁 EffectManager 实例并置空指针，防止野指针.
 */
void EffectManager::destroyInstance() {
    if (_instance != nullptr) {
        delete _instance;
        _instance = nullptr;
    }
}

/**
 * 播放抛射物特效 (如箭矢、炮弹).
 * 创建一个从起点飞向终点的精灵，到达后自动销毁.
 * * @param imgPath 抛射物图片的资源路径.
 * @param start 起始位置坐标 (世界坐标).
 * @param end 目标位置坐标 (世界坐标).
 * @param duration 飞行持续时间 (秒).
 */
void EffectManager::playProjectileEffect(const std::string& imgPath,
    const Vec2& start,
    const Vec2& end,
    float duration) {
    // 校验参数：图片路径为空/时长≤0时直接返回，避免崩溃
    if (imgPath.empty() || duration <= 0) {
        return;
    }

    // 创建特效精灵（炮弹/箭矢图片）
    auto projectile = Sprite::create(imgPath);
    if (!projectile) { // 图片不存在时打印日志，避免崩溃
        return;
    }

    // 获取当前运行的场景，将特效添加到场景中
    auto runningScene = Director::getInstance()->getRunningScene();
    if (!runningScene) {
        return;
    }
    // 保证特效在建筑/兵种上层显示，不会被遮挡 (层级10000)
    runningScene->addChild(projectile, 10000);

    // 设置特效起始位置
    projectile->setPosition(start);

    // 创建动作序列：先移动到终点，再自动移除特效（避免内存泄漏）
    auto moveAction = MoveTo::create(duration, end); // 移动动作
    auto removeAction = RemoveSelf::create();        // 移除动作

    // 执行动作：先移动，移动完成后移除
    projectile->runAction(Sequence::create(moveAction, removeAction, nullptr));
}

/**
 * 播放爆炸特效 (如炮弹命中).
 * 创建一个在指定位置播放放大+渐隐动画的精灵，播放完后自动销毁.
 * * @param imgPath 爆炸图片的资源路径.
 * @param pos 爆炸发生的中心位置 (世界坐标).
 * @param duration 动画持续时间 (秒).
 */
void EffectManager::playExplosionEffect(const std::string& imgPath,
    const Vec2& pos,
    float duration) {
    // 参数校验
    if (imgPath.empty() || duration <= 0) {
        return;
    }

    // 创建爆炸精灵
    auto explosion = Sprite::create(imgPath);
    if (!explosion) {
        return;
    }

    // 添加到场景
    auto runningScene = Director::getInstance()->getRunningScene();
    if (!runningScene) return;
    runningScene->addChild(explosion, 10001); // 比抛射物更高一层

    // 设置爆炸位置
    explosion->setPosition(pos);

    // 爆炸动画：缩放+渐隐，最后移除
    auto scaleBig = ScaleTo::create(duration / 2, 1.5f); // 先放大
    auto scaleSmall = ScaleTo::create(duration / 2, 0.1f); // 再缩小
    auto fadeOut = FadeOut::create(duration); // 渐隐
    auto removeAction = RemoveSelf::create();

    // 组合动作：同时缩放+渐隐，完成后移除
    explosion->runAction(Sequence::create(
        Spawn::create(scaleBig, fadeOut->clone(), nullptr),
        scaleSmall,
        removeAction,
        nullptr
    ));
}