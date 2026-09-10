// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <vector>

// Device-independent renderer. After prepare(), rendering allocates no memory.
class ChordRenderer
{
public:
    struct Sample { std::vector<float> pcm; double rate=48000; int loopStart=0,loopEnd=0; };
    std::array<Sample,25> samples;
    void prepare(double outputRate);
    bool play(int root,bool minor,const std::array<double,12>& cents);
    void stop();
    void setReverb(double seconds);
    void setVolume(double level);
    void render(float* left,float* right,int frames);
    static double pitchRatio(double cents);
    static std::array<int,3> chordNotes(int root,bool minor);
    static float envelope(int age,int duration,int attack,int release);
private:
    struct Voice { int sample=0,age=0,releaseLeft=-1; double position=0,step=1; float releaseGain=1; bool active=false; };
    struct Delay { std::vector<float> data; size_t position=0; float gain=0,targetGain=0; };
    struct Filter
    {
        double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
        void configure(double rate,double hz,bool highPass);
        float process(float input);
        void clear() { z1=z2=0; }
    };
    std::array<Voice,64> voices;
    std::array<Delay,8> delays;
    std::array<Filter,3> highPass,lowPass;
    double rate=48000,reverbSeconds=0;
    int duration=96000,release=3360,attack=480,tailLeft=0,stopLeft=-1;
    float wet=0,volume=.5f,targetVolume=.5f;
    void clear();
    float readSample(const Voice&) const;
};
