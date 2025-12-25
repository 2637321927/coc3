
#ifndef __LEVEL1_SCENE_H__
#define __LEVEL1_SCENE_H__

#include "LevelScene.h"
#include "VillageScene.h"
class VillageScene;
class Level1Scene : public LevelScene {
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    CREATE_FUNC(Level1Scene);

protected:
    virtual void startLevel() override;
    virtual void onLevelComplete(bool success) override;

private:
    void spawnEnemyTroops();
    void updateWave(float dt);
    void showTutorial();

    int _currentWave;
    int _totalWaves;
    float _waveTimer;
};

#endif // __LEVEL1_SCENE_H__