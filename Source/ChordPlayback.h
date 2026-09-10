// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "ChordRenderer.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>

class ChordPlayback final : private juce::AudioIODeviceCallback, private juce::AsyncUpdater
{
public:
    ~ChordPlayback() override;
    juce::String enable();
    void disable();
    bool play(int root,bool minor,const std::array<double,12>& cents);
    void stop();
    void setReverb(double seconds) { reverb.store(static_cast<float>(seconds)); }
    void setLoudness(double percent) { loudness.store(static_cast<float>(percent*.01)); }
    bool isEnabled() const { return enabled; }
    uint64_t renderedFrames() const { return framesRendered.load(); }
    float outputPeak() const { return peak.load(); }
    juce::String deviceDescription() const;
    bool inputIsClosed() const;
    std::function<void(const juce::String&)> onError;
    static juce::String loadSamples(ChordRenderer&);
private:
    struct Request { int root=-1; bool minor=false; std::array<double,12> cents {}; };
    juce::AudioDeviceManager devices;
    ChordRenderer renderer;
    std::array<Request,32> queue;
    std::atomic<unsigned> read {0},write {0};
    std::atomic<float> reverb {3},loudness {.5f};
    std::atomic<uint64_t> framesRendered {0};
    std::atomic<float> peak {0};
    bool loaded=false,enabled=false;
    juce::CriticalSection errorLock;
    juce::String deviceError;
    bool enqueue(const Request&);
    void audioDeviceAboutToStart(juce::AudioIODevice*) override;
    void audioDeviceStopped() override {}
    void audioDeviceError(const juce::String&) override;
    void audioDeviceIOCallbackWithContext(const float* const*,int,float* const*,int,int,const juce::AudioIODeviceCallbackContext&) override;
    void handleAsyncUpdate() override;
};
