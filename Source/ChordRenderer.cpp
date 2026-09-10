// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "ChordRenderer.h"
#include <algorithm>
#include <cmath>

double ChordRenderer::pitchRatio(double cents) { return std::exp2(cents/1200.0); }
std::array<int,3> ChordRenderer::chordNotes(int root,bool minor)
{
    const int base=48+(root%12+12)%12;
    return {base,base+(minor?3:4),base+7};
}
float ChordRenderer::envelope(int age,int length,int fadeIn,int fadeOut)
{
    if(age<0||age>=length) return 0;
    return static_cast<float>(std::min({1.0,age/static_cast<double>(std::max(1,fadeIn)),
        (length-1-age)/static_cast<double>(std::max(1,fadeOut-1))}));
}
void ChordRenderer::Filter::configure(double sampleRate,double hz,bool high)
{
    const double w=2*3.14159265358979323846*hz/sampleRate,c=std::cos(w),s=std::sin(w),alpha=s/std::sqrt(2.0),a0=1+alpha;
    b0=(1+(high?c:-c))/(2*a0);b1=(high?-(1+c):1-c)/a0;b2=b0;
    a1=-2*c/a0;a2=(1-alpha)/a0;clear();
}
float ChordRenderer::Filter::process(float x)
{
    const double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;
    return static_cast<float>(y);
}
void ChordRenderer::clear()
{
    for(auto& v:voices) v.active=false;
    for(auto& d:delays) {std::fill(d.data.begin(),d.data.end(),0.0f);d.position=0;}
    for(auto& f:highPass) f.clear();
    for(auto& f:lowPass) f.clear();
    tailLeft=0;stopLeft=-1;
}
void ChordRenderer::prepare(double outputRate)
{
    rate=std::isfinite(outputRate)&&outputRate>=8000?outputRate:48000;
    duration=static_cast<int>(std::round(rate*2));release=static_cast<int>(std::round(rate*.070));attack=static_cast<int>(std::round(rate*.010));
    const std::array<double,8> times {.0311,.0377,.0419,.0437,.0533,.0599,.0673,.0719};
    for(size_t i=0;i<delays.size();++i) delays[i].data.assign(static_cast<size_t>(std::round(rate*times[i]))|1u,0);
    for(auto& f:highPass) f.configure(rate,200,true);
    for(auto& f:lowPass) f.configure(rate,2500,false);
    clear();wet=0;setReverb(reverbSeconds);
}
void ChordRenderer::setReverb(double seconds)
{
    reverbSeconds=std::isfinite(seconds)?std::clamp(seconds,0.0,4.0):0;
    for(auto& d:delays) d.targetGain=reverbSeconds>0?static_cast<float>(std::pow(10.0,-3.0*static_cast<double>(d.data.size())/(rate*reverbSeconds))):0;
}
void ChordRenderer::setVolume(double level)
{
    targetVolume=std::isfinite(level)?static_cast<float>(std::clamp(level,0.0,1.0)):0;
}
void ChordRenderer::stop()
{
    if(tailLeft>0) stopLeft=release;
}
bool ChordRenderer::play(int root,bool minor,const std::array<double,12>& cents)
{
    if(root<0||root>11) return false;
    const auto notes=chordNotes(root,minor);
    std::array<double,3> steps;
    for(size_t i=0;i<notes.size();++i)
    {
        const auto& sample=samples[static_cast<size_t>(notes[i]-48)];
        if(sample.pcm.size()<4||!std::isfinite(cents[notes[i]%12])||!std::isfinite(cents[9])) return false;
        steps[i]=sample.rate/rate*pitchRatio(cents[notes[i]%12]-cents[9]);
        if(!std::isfinite(steps[i])||steps[i]<=0||steps[i]>64) return false;
    }
    for(auto& v:voices) if(v.active&&v.releaseLeft<0)
    {
        v.releaseLeft=release;v.releaseGain=envelope(v.age,duration,attack,release);
    }
    for(size_t i=0;i<notes.size();++i)
    {
        auto* available=&voices.front();
        for(auto& v:voices) if(!v.active) {available=&v;break;} else if(v.age>available->age) available=&v;
        *available=Voice {};available->active=true;available->sample=notes[i]-48;available->step=steps[i];
    }
    tailLeft=static_cast<int>(rate*8);stopLeft=-1;
    return true;
}
float ChordRenderer::readSample(const Voice& v) const
{
    const auto& s=samples[static_cast<size_t>(v.sample)];
    const int index=static_cast<int>(v.position);const float t=static_cast<float>(v.position-index);
    const auto at=[&](int i) {
        if(s.loopEnd>s.loopStart&&i>=s.loopEnd) i=s.loopStart+(i-s.loopEnd)%(s.loopEnd-s.loopStart);
        return s.pcm[static_cast<size_t>(std::clamp(i,0,static_cast<int>(s.pcm.size())-1))];
    };
    const float a=at(index-1),b=at(index),c=at(index+1),d=at(index+2);
    return b+.5f*t*(c-a+t*(2*a-5*b+4*c-d+t*(3*(b-c)+d-a)));
}
void ChordRenderer::render(float* left,float* right,int frames)
{
    for(int frame=0;frame<frames;++frame)
    {
        volume+=static_cast<float>((targetVolume-volume)/(rate*.020));
        if(tailLeft<=0) {left[frame]=0;if(right) right[frame]=0;continue;}
        float dry=0;
        for(auto& v:voices) if(v.active)
        {
            float gain=envelope(v.age,duration,attack,release);
            if(v.releaseLeft>=0) gain=std::min(gain,v.releaseGain*v.releaseLeft/static_cast<float>(release));
            dry+=readSample(v)*gain*.22f;
            ++v.age;v.position+=v.step;
            const auto& s=samples[static_cast<size_t>(v.sample)];
            if(s.loopEnd>s.loopStart&&v.position>=s.loopEnd)
                v.position=s.loopStart+std::fmod(v.position-s.loopEnd,s.loopEnd-s.loopStart);
            if(v.age>=duration||(v.releaseLeft>=0&&--v.releaseLeft<=0)||v.position>=s.pcm.size()-2) v.active=false;
        }
        // Eight unequal delay lines with orthogonal Hadamard feedback provide
        // diffusion. Each line's gain sets approximately the requested RT60.
        std::array<float,8> taps,feedback;
        for(size_t i=0;i<8;++i) taps[i]=delays[i].data[delays[i].position];
        feedback=taps;
        for(size_t stride=1;stride<8;stride*=2)
            for(size_t base=0;base<8;base+=stride*2)
                for(size_t i=0;i<stride;++i)
                {const float a=feedback[base+i],b=feedback[base+i+stride];feedback[base+i]=a+b;feedback[base+i+stride]=a-b;}
        const float input=lowPass[0].process(highPass[0].process(dry));
        for(size_t i=0;i<8;++i)
        {
            auto& d=delays[i];d.gain+=static_cast<float>((d.targetGain-d.gain)/(rate*.020));
            d.data[d.position]=input*.35355339f+feedback[i]*.35355339f*d.gain;
            d.position=(d.position+1)%d.data.size();
        }
        const float a=(taps[0]-taps[1]+taps[2]-taps[3]+taps[4]-taps[5]+taps[6]-taps[7])*.35355339f;
        const float b=(taps[0]+taps[1]-taps[2]-taps[3]+taps[4]+taps[5]-taps[6]-taps[7])*.35355339f;
        wet+=static_cast<float>(((reverbSeconds>0?.25:0)-wet)/(rate*.020));
        const float master=volume*(stopLeft>=0?stopLeft/static_cast<float>(release):1);
        const float l=std::clamp((dry+wet*lowPass[1].process(highPass[1].process(a)))*master,-.98f,.98f);
        const float r=std::clamp((dry+wet*lowPass[2].process(highPass[2].process(b)))*master,-.98f,.98f);
        left[frame]=right?l:(l+r)*.5f;if(right) right[frame]=r;
        if(--tailLeft==0||(stopLeft>=0&&--stopLeft==0)) clear();
    }
}
