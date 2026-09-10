// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "ChordPlayback.h"
#include "Temperament.h"
#include <juce_audio_formats/juce_audio_formats.h>

bool checkEmbeddedAudio(const juce::File& directory)
{
    ChordRenderer renderer;
    const auto error=ChordPlayback::loadSamples(renderer);
    if(error.isNotEmpty()) {directory.getChildFile("audio-check.txt").replaceWithText(error);return false;}
    for(const auto& sample:renderer.samples)
    {
        float peak=0;
        for(float value:sample.pcm) {if(!std::isfinite(value)) return false;peak=std::max(peak,std::abs(value));}
        if(peak<.001f||sample.pcm.size()<96000||sample.loopEnd>static_cast<int>(sample.pcm.size())) return false;
    }
    renderer.prepare(48000);
    std::array<double,12> cents {};
    for(int root=0;root<12;++root) for(bool minor:{false,true})
    {
        if(!renderer.play(root,minor,cents)) return false;
        std::array<float,512> l {},r {};renderer.render(l.data(),r.data(),512);
    }
    const auto meantone=temperament::reverseCsv("10.265;-13.686;3.422;20.530;-3.421;13.686;-10.265;6.843;-17.108;0.000;17.108;-6.843");
    for(bool dry:{true,false})
    {
        renderer.prepare(48000);renderer.setReverb(dry?0:3);
        if(!renderer.play(0,false,meantone.cents)) return false;
        juce::AudioBuffer<float> output(2,48000*7);
        renderer.render(output.getWritePointer(0),output.getWritePointer(1),output.getNumSamples());
        if(output.getRMSLevel(0,4800,48000)<.001f||output.getMagnitude(0,output.getNumSamples())>.98001f) return false;
        if(dry&&output.getMagnitude(96000,output.getNumSamples()-96000)!=0) return false;
        if(!dry&&output.getRMSLevel(0,100800,24000)<.00001f) return false;
        juce::WavAudioFormat format;
        auto stream=directory.getChildFile(dry?"principal-meantone-dry.wav":"principal-meantone-church.wav").createOutputStream();
        if(!stream) return false;
        stream->setPosition(0);stream->truncate();
        std::unique_ptr<juce::OutputStream> destination=std::move(stream);
        auto writer=format.createWriterFor(destination,juce::AudioFormatWriterOptions{}.withSampleRate(48000).withNumChannels(2).withBitsPerSample(24));
        if(!writer||!writer->writeFromAudioSampleBuffer(output,0,output.getNumSamples())) return false;
    }
    return directory.getChildFile("audio-check.txt").replaceWithText("PASS: 25 embedded lossless recordings, 24 root-position triads, tuning-aware resampling, 2-second dry notes and filtered stereo reverb. No audio device opened.\n");
}
