#ifndef __TITLE_SCENE_H__
#define __TITLE_SCENE_H__

#include "cocos2d.h"
USING_NS_CC;
class TitleScene : public cocos2d::Scene
{
public:
    // Cocos2d-x创建场景的标准宏
    CREATE_FUNC(TitleScene);
    virtual bool init() override;
    static Scene* createScene()
    {
        auto scene = Scene::create();
        auto layer = TitleScene::create();
        scene->addChild(layer);
        _instance = layer;
        layer->setTag(6);//便于getInstance获取
        return scene;
    }
    static TitleScene* getInstance() { return _instance; }
    /*static TitleScene* getInstance() { // 提供外部访问接口
        auto currentScene = Director::getInstance()->getRunningScene();
        if (!currentScene) return nullptr;
        // 找到场景中 Tag=100 的子节点
        return dynamic_cast<TitleScene*>(currentScene->getChildByTag(6));
    }*/
private:
    // 初始化UI元素（背景、按钮）
    void initBackground();
    void initButtons();
    // 全局静态变量保存实例
    static TitleScene* _instance;
    // 按钮点击回调（预留不同模式入口）
    void onStartGameClick(cocos2d::Ref* pSender);
    void onLevelSelectClick(cocos2d::Ref* pSender);
    void onSettingClick(cocos2d::Ref* pSender);
    void onEnter();
    // 适配按钮点击区域（可选，优化触摸体验）
    cocos2d::Rect getButtonRect(cocos2d::Sprite* btnSprite);
};

#endif // __TITLE_SCENE_H__
