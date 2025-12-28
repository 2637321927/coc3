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

#include "HelloWorldScene.h"

USING_NS_CC;

/**
 * 创建场景实例.
 * 静态工厂方法，用于创建并初始化 HelloWorld 场景.
 * * @return 创建成功的 Scene 指针.
 */
Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

/**
 * 打印资源加载错误信息.
 * 当资源文件不存在时打印详细的错误提示，防止程序直接因空指针崩溃.
 * * @param filename 加载失败的文件名字符串.
 */
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

/**
 * 初始化场景.
 * 负责初始化背景、菜单按钮、标签和精灵等场景元素.
 * * @return 初始化成功返回 true，失败返回 false.
 */
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. 调用父类初始化
    if (!Scene::init())
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. 添加菜单项（关闭按钮）

    // 创建关闭按钮项
    auto closeItem = MenuItemImage::create(
        "CloseNormal.png",
        "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width / 2;
        float y = origin.y + closeItem->getContentSize().height / 2;
        closeItem->setPosition(Vec2(x, y));
    }

    // 创建菜单并添加关闭按钮
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. 添加场景内容

    // 创建并初始化 "Hello World" 标签
    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // 将标签放置在屏幕中心顶部
        label->setPosition(Vec2(origin.x + visibleSize.width / 2,
            origin.y + visibleSize.height - label->getContentSize().height));

        // 添加标签到层
        this->addChild(label, 1);
    }

    // 添加 Cocos 图标精灵
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // 将精灵放置在屏幕正中心
        sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        // 添加精灵到层
        this->addChild(sprite, 0);
    }
    return true;
}

/**
 * 关闭按钮回调.
 * 点击关闭按钮时调用，结束当前游戏场景并退出应用程序.
 * * @param pSender 触发回调的对象指针.
 */
void HelloWorld::menuCloseCallback(Ref* pSender)
{
    // 关闭 Cocos2d-x 游戏场景并退出应用
    Director::getInstance()->end();

    /* * 注意：如果是在 iOS 原生界面中嵌入 Cocos，
     * 若要返回原生界面而不是退出整个应用，不应使用 Director::end()，
     * 而是应该分发一个自定义事件，由原生代码捕获处理.
     */
     //EventCustom customEndEvent("game_scene_close_event");
     //_eventDispatcher->dispatchEvent(&customEndEvent);
}