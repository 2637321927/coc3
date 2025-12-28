/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

 /**
  * HelloWorld 场景类.
  * 游戏启动后的默认场景，展示 Cocos Logo 和基础 UI.
  */
class HelloWorld : public cocos2d::Scene
{
public:
    /**
     * 创建场景实例.
     * 静态工厂方法，用于获取场景对象.
     * * @return 创建成功的 Scene 对象指针.
     */
    static cocos2d::Scene* createScene();

    /**
     * 初始化场景.
     * 负责创建并添加场景内的所有节点（背景、按钮、标签等）.
     * * @return 初始化成功返回 true，失败返回 false.
     */
    virtual bool init();

    /**
     * 关闭按钮回调函数.
     * 点击右下角关闭按钮时触发.
     * * @param pSender 触发事件的对象指针.
     */
    void menuCloseCallback(cocos2d::Ref* pSender);

    /** * 手动实现静态 create() 方法的宏.
     * 自动生成 create() 函数，内部调用 init().
     */
    CREATE_FUNC(HelloWorld);
};

#endif // __HELLOWORLD_SCENE_H__