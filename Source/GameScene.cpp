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

#include "GameScene.h"
#include "SummerTTS.h"
#include "FastLLM.h"
#include "textconvertor.h"
#include "audio/AudioEngine.h"

USING_NS_AX;

dragonBones::CCFactory* ChatMonika::factory = dragonBones::CCFactory::getFactory();

#include "io.h"
void ChatMonika::getFilesName(std::string path, std::vector<std::string>& files, std::string suffix)
{
    long long hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
    {
        do
        {
            std::string str = fileinfo.name;
            if (str.find(suffix) != -1)
            {
                std::string fileName = fileinfo.name;
                files.push_back(fileName);
            }

        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

int ChatMonika::getIndex(std::string filePath)
{
    files.clear();
    getFilesName(filePath, files, ".wav");

    int max = 0;
    int index = 0;
    for (int i = 0; i < files.size(); i++)
    {
        auto str = files.at(i);
        int start_pos = str.find_last_of('e') + 1;
        int end_pos = str.find('.');
        int val = atoi(str.substr(start_pos, end_pos - start_pos).c_str());
        if (val > max)
        {
            max = val;
            index = i;
        }

    }

    return index;
}

void ChatMonika::stringSplit(std::string str, std::vector<std::string>& vector)
{
    std::string pat = "。";
    bool isRemovePat = true;
    int patSplitPos = pat.size();
    std::vector<std::string> bufStr;
    while (true)
    {
        int index = str.find(pat);
        int iSplitPos = 0;//分割点
        int iEraserPos = 0;//分割点
        if (isRemovePat || index == -1)//如果去掉分割字符串
        {
            iSplitPos = index;
            iEraserPos = index + pat.size();
        }
        else
        {
            iSplitPos = index + patSplitPos;
            iEraserPos = index + patSplitPos;
        }
        std::string subStr = str.substr(0, iSplitPos);
        if (!subStr.empty()) vector.push_back(subStr);
        str.erase(0, iEraserPos);
        if (index == -1) break;
    }
}

// on "init" you need to initialize your instance
bool ChatMonika::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init())
    {
        return false;
    }

    visibleSize = _director->getVisibleSize();

    //语音路径
    voicePath = FileUtils::getInstance()->getDefaultResourceRootPath() + "voice/out.wav";

    //加载背景
    bg = Sprite::create("spaceroom.png");
    bg->setPosition(visibleSize / 2);
    this->addChild(bg);

    UILayer[0] = Layer::create();
    UILayer[1] = Layer::create();

    //加载莫妮卡
    factory->loadDragonBonesData("Monika/Monika_ske.json");
    factory->loadTextureAtlasData("Monika/Monika_tex.json");

    factory->loadDragonBonesData("clothes/c_miku/c_miku_ske.json");
    factory->loadTextureAtlasData("clothes/c_miku/c_miku_tex.json");
    factory->loadDragonBonesData("clothes/c_monika/c_monika_ske.json");
    factory->loadTextureAtlasData("clothes/c_monika/c_monika_tex.json");

    chara = new dragonBones::CCArmatureDisplay();
    chara = factory->buildArmatureDisplay("Monika");
    chara->getAnimation()->play("steady");
    chara->setAnchorPoint(Vec2(0.5, 0));
    chara->setPosition(Vec2(visibleSize.width / 2, 0));
    this->addChild(chara);

    uiState = ChatMonika::UIState::UI_1;
    settings = ui::Button::create("settings.png");
    settings->setScale(visibleSize.height * 0.12f / settings->getContentSize().height);
    settings->setPosition(Vec2(visibleSize.width - visibleSize.height * 0.08f, visibleSize.height - visibleSize.height * 0.08f));
    this->addChild(settings, 1);
    settings->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                switch (uiState)
                {
                case ChatMonika::UIState::UI_1:
                    uiState = ChatMonika::UIState::UI_2;
                    UILayer[1]->setPositionY(0);
                    UILayer[0]->setPositionY(-visibleSize.height);
                    break;
                case ChatMonika::UIState::UI_2:
                    uiState = ChatMonika::UIState::UI_1;
                    UILayer[0]->setPositionY(0);
                    UILayer[1]->setPositionY(-visibleSize.height);
                    break;
                }
            }
        });

    initUILayer1();
    initUILayer2();

    this->addChild(UILayer[0], 1);
    this->addChild(UILayer[1], 1);
    return true;
}

void ChatMonika::initUILayer1()
{
    //加载文本框
    textbox = Sprite::create("textbox.png");
    textbox->setAnchorPoint(Vec2(0.5, 0));
    textbox->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.1f));
    UILayer[0]->addChild(textbox);
    textbox->setVisible(false);

    //加载输入框
    auto size = textbox->getContentSize();
    editbox = ui::EditBox::create(Size(size.width, size.height / 2), ui::Scale9Sprite::create("choice.png"));
    editbox->setAnchorPoint(Vec2(0.5, 0));
    editbox->setPosition(textbox->getPosition());
    editbox->setFontSize(visibleSize.height * 0.04f);
    const char* tip = "本项目github开源，禁止商用。\n输入文字后，点击[对话]。";
    editbox->setText(GBKToUTF8(tip).c_str());
    UILayer[0]->addChild(editbox);

    //加载文字
    text = Label::createWithTTF("", "fonts/black.ttf", visibleSize.height * 0.04f);
    text->setAnchorPoint(Vec2(0, 1));
    text->setPosition(Vec2(visibleSize.width / 2 - size.width * 0.48f, visibleSize.height * 0.1f + size.height * 0.94f));
    text->setDimensions(size.width * 0.96f, size.height * 0.88f);
    UILayer[0]->addChild(text, 1);

    //确认按钮
    button[0] = ui::Button::create("choice.png", "choice_dark.png");
    button[0]->setTitleFontName("fonts/black.ttf");
    button[0]->setTitleFontSize(0.5f * button[0]->getContentSize().height);
    const char* button_text1 = "对话";
    button[0]->setTitleText(GBKToUTF8(button_text1));

    button[0]->setPosition(Vec2(visibleSize.width * 0.36f, visibleSize.height * 0.05f));
    UILayer[0]->addChild(button[0]);
    button[0]->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED && editbox->isEnabled())
            {
                auto str = editbox->getText();
                editbox->setVisible(false);
                editbox->setEnabled(false);
                auto answer = FastLLM::getInstance()->getResponse(str);

                stringSplit(UTF8ToGBK(answer), talkList);
                answer = GBKToUTF8(talkList.at(0));
                
                bool isDone = SummerTTS::getInstance()->generateVoice(answer);
                if (isDone)
                {
                    textbox->setVisible(true);

                    AudioEngine::uncacheAll();
                    int audioID = AudioEngine::play2d(voicePath);
                    AudioEngine::setFinishCallback(audioID, AX_CALLBACK_0(ChatMonika::voiceCallBack, this));
                    chara->getAnimation()->play("talk");
                    text->setString(answer.c_str());
                }
            }
        });

    //继续按钮
    button[1] = ui::Button::create("choice.png", "choice_dark.png");
    button[1]->setTitleFontName("fonts/black.ttf");
    button[1]->setTitleFontSize(0.5f * button[1]->getContentSize().height);
    const char* button_text2 = "继续";
    button[1]->setTitleText(GBKToUTF8(button_text2));

    button[1]->setPosition(Vec2(visibleSize.width * 0.64f, visibleSize.height * 0.05f));
    UILayer[0]->addChild(button[1]);
    button[1]->addTouchEventListener([&](Ref* sender, ui::Widget::TouchEventType type)
        {
            if (type == ui::Widget::TouchEventType::ENDED)
            {
                if (textbox->isVisible() && (talkTimes < talkList.size() - 1))
                {
                    talkTimes += 1;
                    auto answer = GBKToUTF8(talkList.at(talkTimes));

                    bool isDone = SummerTTS::getInstance()->generateVoice(answer);
                    if (isDone)
                    {
                        AudioEngine::uncacheAll();
                        int audioID = AudioEngine::play2d(voicePath);
                        AudioEngine::setFinishCallback(audioID, AX_CALLBACK_0(ChatMonika::voiceCallBack, this));
                        chara->getAnimation()->play("talk");
                        text->setString(answer.c_str());
                    }
                }
                else if (talkTimes == talkList.size() - 1)
                {
                    talkTimes = 0;
                    talkList.clear();
                    text->setString("");
                    textbox->setVisible(false);
                    editbox->setVisible(true);
                    editbox->setEnabled(true);
                }
            }
        });
}

void ChatMonika::initUILayer2()
{
    UILayer[1]->setPositionY(-visibleSize.height);
    auto vw = visibleSize.width;
    auto vh = visibleSize.height;

    auto dataMask = DrawNode::create();
    dataMask->drawSolidRect(Vec2(vw / 2 - vh * 0.4f, vh * 0.04f), Vec2(vw / 2 + vh * 0.4f, vh * 0.96f), Color4F::BLACK);
    dataMask->setOpacity(100);
    UILayer[1]->addChild(dataMask);

    auto dataList = Menu::create();
    auto fileUtils = FileUtils::getInstance();

    auto item1 = MenuItemToggle::createWithCallback(CC_CALLBACK_1(ChatMonika::settingsCallBack, this),
        MenuItemLabel::create(Label::createWithTTF(GBKToUTF8("[模样]经典套装"), "fonts/black.ttf", vh * 0.08f)),
        MenuItemLabel::create(Label::createWithTTF(GBKToUTF8("[模样]初音未来"), "fonts/black.ttf", vh * 0.08f)), NULL);
    auto item2 = MenuItemLabel::create(Label::createWithTTF(GBKToUTF8("退出"), "fonts/black.ttf", vh * 0.08f), CC_CALLBACK_1(ChatMonika::settingsCallBack, this));
    item1->setTag(0);
    item2->setTag(1);
    dataList->addChild(item1);
    dataList->addChild(item2);
    dataList->setPosition(Vec2(vw / 2, vh / 2));
    dataList->alignItemsVertically();
    UILayer[1]->addChild(dataList);
}

void ChatMonika::voiceCallBack()
{
    chara->getAnimation()->play("steady");
}

void ChatMonika::settingsCallBack(Ref* sender)
{
    auto item = (MenuItemLabel*)sender;
    auto itemTag = item->getTag();
    switch (itemTag)
    {
    case 0:
    {
        std::string suitName = "";
        if (clothes == 0)
        {
            clothes = 1;
            suitName = "c_miku";
        }
        else
        {
            clothes = 0;
            suitName = "c_monika";
        }
        const auto armatureData = factory->getArmatureData(suitName);
        if (armatureData != nullptr)
            factory->replaceSkin(chara->getArmature(), armatureData->defaultSkin);
        break;
    }
    case 1:
        Director::getInstance()->end();
        break;
    }
}
