// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "ChordPlayback.h"
#include "PrincipalData.h"
#include "PrincipalSamples/SampleMetadata.h"
#include <juce_audio_formats/juce_audio_formats.h>

juce::String ChordPlayback::loadSamples(ChordRenderer& target)
{
    juce::FlacAudioFormat format;
    for(int note=48;note<=72;++note)
    {
        int bytes=0;
        const auto name="note"+juce::String(note).paddedLeft('0',3)+"_flac";
        const auto* data=PrincipalData::getNamedResource(name.toRawUTF8(),bytes);
        if(data==nullptr||bytes==0) return "Missing embedded Principal sample: "+juce::String(note);
        std::unique_ptr<juce::AudioFormatReader> reader(format.createReaderFor(new juce::MemoryInputStream(data,static_cast<size_t>(bytes),false),true));
        if(!reader||reader->lengthInSamples<4||reader->numChannels!=1) return "Cannot decode Principal sample: "+juce::String(note);
        const auto& meta=principal::metadata[static_cast<size_t>(note-48)];
        if(reader->lengthInSamples!=meta.frames||reader->sampleRate!=meta.rate) return "Principal sample metadata mismatch.";
        juce::AudioBuffer<float> decoded(1,meta.frames);
        if(!reader->read(&decoded,0,meta.frames,0,true,false)) return "Incomplete Principal sample.";
        auto& sample=target.samples[static_cast<size_t>(note-48)];
        sample.pcm.assign(decoded.getReadPointer(0),decoded.getReadPointer(0)+meta.frames);
        sample.rate=reader->sampleRate;sample.loopStart=meta.loopStart;sample.loopEnd=meta.loopEnd;
    }
    return {};
}
ChordPlayback::~ChordPlayback() { disable();cancelPendingUpdate(); }
juce::String ChordPlayback::enable()
{
    if(enabled) return {};
    cancelPendingUpdate();framesRendered.store(0);peak.store(0);
    if(!loaded)
    {
        const auto error=loadSamples(renderer);if(error.isNotEmpty()) return error;loaded=true;
    }
    read.store(0);write.store(0);
    // Never request input channels or microphone permission.
    auto error=devices.initialiseWithDefaultDevices(0,2);
    if(error.isEmpty()&&devices.getCurrentAudioDevice()==nullptr) error="No default audio output is available.";
    if(error.isNotEmpty()) {devices.closeAudioDevice();return error;}
    devices.addAudioCallback(this);enabled=true;return {};
}
void ChordPlayback::disable()
{
    devices.removeAudioCallback(this);devices.closeAudioDevice();enabled=false;cancelPendingUpdate();
}
juce::String ChordPlayback::deviceDescription() const
{
    if(auto* device=devices.getCurrentAudioDevice()) return device->getName()+" / "+juce::String(device->getCurrentSampleRate())+" Hz";
    return "No audio output";
}
bool ChordPlayback::inputIsClosed() const
{
    if(const auto* device=devices.getCurrentAudioDevice()) return device->getActiveInputChannels().isZero();
    return true;
}
bool ChordPlayback::enqueue(const Request& request)
{
    if(!enabled) return false;
    const auto next=write.load(std::memory_order_relaxed),following=(next+1)%32u;
    if(following==read.load(std::memory_order_acquire)) return false;
    queue[next]=request;write.store(static_cast<unsigned>(following),std::memory_order_release);return true;
}
bool ChordPlayback::play(int root,bool minor,const std::array<double,12>& cents) {return enqueue({root,minor,cents});}
void ChordPlayback::stop() {enqueue({});}
void ChordPlayback::audioDeviceAboutToStart(juce::AudioIODevice* device) {renderer.prepare(device->getCurrentSampleRate());}
void ChordPlayback::audioDeviceIOCallbackWithContext(const float* const*,int,float* const* outputs,int channels,int frames,const juce::AudioIODeviceCallbackContext&)
{
    juce::ScopedNoDenormals guard;
    const auto available=write.load(std::memory_order_acquire);auto next=read.load(std::memory_order_relaxed);
    Request latest;bool changed=false;
    while(next!=available) {latest=queue[next];changed=true;next=(next+1)%32u;}
    read.store(next,std::memory_order_release);
    renderer.setReverb(reverb.load(std::memory_order_relaxed));
    renderer.setVolume(loudness.load(std::memory_order_relaxed));
    if(changed)
    {
        if(latest.root<0) renderer.stop();
        else if(!renderer.play(latest.root,latest.minor,latest.cents)) renderer.stop();
    }
    float* left=nullptr;float* right=nullptr;
    for(int channel=0;channel<channels;++channel) if(outputs[channel])
    {
        std::fill(outputs[channel],outputs[channel]+frames,0.0f);
        if(!left) left=outputs[channel];else if(!right) right=outputs[channel];
    }
    if(left)
    {
        renderer.render(left,right,frames);
        float maximum=peak.load(std::memory_order_relaxed);
        for(int i=0;i<frames;++i) maximum=std::max(maximum,std::max(std::abs(left[i]),right?std::abs(right[i]):0.0f));
        peak.store(maximum,std::memory_order_relaxed);
        framesRendered.fetch_add(static_cast<uint64_t>(frames),std::memory_order_relaxed);
    }
}
void ChordPlayback::audioDeviceError(const juce::String& error)
{
    {const juce::ScopedLock lock(errorLock);deviceError=error;}
    triggerAsyncUpdate();
}
void ChordPlayback::handleAsyncUpdate()
{
    juce::String error;{const juce::ScopedLock lock(errorLock);error=deviceError;}
    disable();if(onError) onError(error);
}
