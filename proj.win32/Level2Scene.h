#pragma once
// Level2Scene.h
#ifndef __LEVEL2_SCENE_H__
#define __LEVEL2_SCENE_H__

#include "LevelScene.h"
#include "VillageScene.h"
class VillageScene;
class Level2Scene : public LevelScene {
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;

    CREATE_FUNC(Level2Scene);

protected:
    virtual void startLevel() override;
    virtual void onLevelComplete(bool success) override;
};

#endif // __LEVEL2_SCENE_H__