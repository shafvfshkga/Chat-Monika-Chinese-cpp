/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2021 Bytedance Inc.

 https://axmolengine.github.io/

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

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "axmol.h"
#include "ui/CocosGUI.h"
#include "DragonBones/CCArmatureDisplay.h"
#include "DragonBones/CCFactory.h"

USING_NS_AX;

class ChatMonika : public Scene
{
public:
    bool init() override;
    //void update(float delta) override;

    static dragonBones::CCFactory* factory;
    dragonBones::CCArmatureDisplay* chara;

    Vec2 visibleSize;
    void initUILayer1();
    void initUILayer2();

    int clothes = 0;
    enum UIState
    {
        UI_1,
        UI_2,
    }uiState;
    ui::Button* settings;
private:
    Sprite* bg;
    Sprite* textbox;
    ui::Button* button[2];
    ui::EditBox* editbox;
    Label* text;
    Layer* UILayer[2];

    int talkTimes = 0;
    std::vector<std::string> talkList;
    std::vector<std::string> files;
    int getIndex(std::string filePath);
    void getFilesName(std::string path, std::vector<std::string>& files, std::string);
    void stringSplit(std::string str, std::vector<std::string>& vector);

    std::string voicePath;
    void voiceCallBack();
    void settingsCallBack(Ref* sender);
};

#endif
