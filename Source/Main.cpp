// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "MainComponent.h"
bool checkEmbeddedAudio(const juce::File&);
#include "AppIcon.h"

class ReadableContent final : public juce::Component
{
public:
    ReadableContent()
    {
        viewport.setViewedComponent(&content, false);
        viewport.setScrollBarThickness(24);
        viewport.setScrollBarsShown(true, true);
        addAndMakeVisible(viewport);
        setSize(1480, 900);
    }
    void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xff10171f)); }
    void resized() override
    {
        viewport.setBounds(getLocalBounds());
        content.setSize(juce::jmax(1280,getWidth()),juce::jmax(660,getHeight()));
    }
private:
    MainComponent content;
    juce::Viewport viewport;
};

class TemperamentApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "Temperament Generator"; }
    const juce::String getApplicationVersion() override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void initialise(const juce::String& commandLine) override
    {
        const auto args = juce::StringArray::fromTokens(commandLine, true);
        if(args.size()==2&&args[0]=="--check-default-audio")
        {
            const auto directory=juce::File(args[1].unquoted());directory.createDirectory();
            audioCheck=std::make_unique<ChordPlayback>();
            const auto error=audioCheck->enable();
            if(error.isNotEmpty())
            {directory.getChildFile("default-audio-check.txt").replaceWithText(error);setApplicationReturnValue(1);quit();return;}
            audioCheck->play(0,false,{});
            juce::Timer::callAfterDelay(2400,[this,directory] {
                const bool ok=audioCheck->renderedFrames()>16000&&audioCheck->outputPeak()>.0001f&&audioCheck->inputIsClosed();
                directory.getChildFile("default-audio-check.txt").replaceWithText((ok?"PASS: ":"FAIL: ")+audioCheck->deviceDescription()
                    +"; rendered frames "+juce::String(static_cast<juce::int64>(audioCheck->renderedFrames()))
                    +"; peak "+juce::String(audioCheck->outputPeak(),6)+"; input channels closed: "+(audioCheck->inputIsClosed()?"yes":"no"));
                audioCheck.reset();setApplicationReturnValue(ok?0:1);quit();
            });
            return;
        }
        if (args.size() == 2 && args[0] == "--render-preview")
        {
            const auto directory = juce::File(args[1].unquoted());
            directory.createDirectory();
            if(!checkEmbeddedAudio(directory)) {setApplicationReturnValue(1);quit();return;}
            MainComponent preview;
            bool success = preview.renderPreviews(directory);
            if (success)
            {
                // A small window scrolls the interface without reducing its font sizes.
                Window smallWindow(getApplicationName(), false);
                smallWindow.setSize(1000, 720);
                if (auto stream = directory.getChildFile("window-small.png").createOutputStream())
                {
                    auto* content = smallWindow.getContentComponent();
                    stream->setPosition(0);
                    stream->truncate();
                    success = juce::PNGImageFormat().writeImageToStream(
                        content->createComponentSnapshot(content->getLocalBounds()), *stream);
                }
                else success = false;
            }
            setApplicationReturnValue(success ? 0 : 1);
            quit();
            return;
        }
        window = std::make_unique<Window>(getApplicationName());
    }
    void shutdown() override { audioCheck.reset();window.reset(); }
    void systemRequestedQuit() override { quit(); }
private:
    class Window final : public juce::DocumentWindow
    {
    public:
        explicit Window(const juce::String& name, bool visible = true)
            : DocumentWindow(name, juce::Colour(0xff10171f), allButtons)
        {
            setUsingNativeTitleBar(true);
            setIcon(appIcon());
            setContentOwned(new ReadableContent(), true);
            setResizable(true, true);
            setResizeLimits(1000, 650, 3840, 2160);
            const auto* display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
            const auto available = display != nullptr ? display->userArea : juce::Rectangle<int>(0, 0, 1400, 1000);
            centreWithSize(juce::jmin(1480, available.getWidth() - 16), juce::jmin(930, available.getHeight() - 16));
            setVisible(visible);
        }
        void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
    };
    std::unique_ptr<Window> window;
    std::unique_ptr<ChordPlayback> audioCheck;
};

START_JUCE_APPLICATION(TemperamentApplication)
