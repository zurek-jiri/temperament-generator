// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "ChordRenderer.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

static int checks=0;
static void check(bool condition,const char* message)
{++checks;if(!condition) {std::cerr<<message<<'\n';std::exit(1);}}
static double energy(const std::vector<float>& values,size_t start,size_t end)
{double sum=0;for(size_t i=start;i<end;++i) sum+=values[i]*values[i];return sum/(end-start);}
int main()
{
    check(std::abs(ChordRenderer::pitchRatio(1200)-2)<1e-12,"Octave resampling ratio");
    check(std::abs(ChordRenderer::pitchRatio(-1200)-.5)<1e-12,"Downward octave resampling ratio");
    for(int root=0;root<12;++root) for(bool minor:{false,true})
    {
        const auto notes=ChordRenderer::chordNotes(root,minor);
        check(notes[0]%12==root&&notes[1]-notes[0]==(minor?3:4)&&notes[2]-notes[0]==7,"Root position triad voicing");
        check(notes[0]>=48&&notes[2]<=72,"Every chord fits the supplied sample bank");
    }
    ChordRenderer renderer;
    for(size_t i=0;i<renderer.samples.size();++i)
    {
        auto& sample=renderer.samples[i];sample.pcm.resize(144000);
        for(size_t j=0;j<sample.pcm.size();++j) sample.pcm[j]=static_cast<float>(.6*std::sin(2*3.141592653589793*220*j/48000));
    }
    // Isolate the root to measure resampling independently of chord beating.
    std::fill(renderer.samples[4].pcm.begin(),renderer.samples[4].pcm.end(),0.0f);
    std::fill(renderer.samples[7].pcm.begin(),renderer.samples[7].pcm.end(),0.0f);
    for(double rate:{44100,48000,96000})
    {
        renderer.prepare(rate);renderer.setReverb(0);
        std::array<double,12> cents {};cents[0]=100;
        check(renderer.play(0,false,cents),"Playable chart accepted");
        std::vector<float> left(static_cast<size_t>(rate*2.3)),right(left.size());
        renderer.render(left.data(),right.data(),static_cast<int>(left.size()));
        int crossings=0;
        for(int i=static_cast<int>(rate*.2);i<static_cast<int>(rate*1.7);++i)
            if(left[i-1]<=0&&left[i]>0) ++crossings;
        check(std::abs(crossings/1.5-220*std::exp2(100.0/1200))<1,"Pitch shift stays accurate across device rates");
        check(left.front()==0&&left[static_cast<size_t>(rate*2)-1]==0,"Attack and final release end at zero");
        check(energy(left,static_cast<size_t>(rate*2),left.size())==0,"Dry chord lasts exactly two seconds");
        check(energy(left,static_cast<size_t>(rate*1.99),static_cast<size_t>(rate*2))<energy(left,static_cast<size_t>(rate*1.9),static_cast<size_t>(rate*1.92))*.05,"Last 70ms fade audibly reduces amplitude");
        check(left==right,"Dry mono recording is centred in stereo");
        const auto original=left;renderer.prepare(rate);
        for(auto& v:cents) v+=17.25;
        renderer.play(0,false,cents);renderer.render(left.data(),right.data(),static_cast<int>(left.size()));
        check(original==left,"Reference offsets preserve A-relative playback");
    }
    check(ChordRenderer::envelope(96000-3360,96000,480,3360)==1,"70ms fade starts at full envelope");
    check(ChordRenderer::envelope(95999,96000,480,3360)==0,"70ms fade ends at zero");
    std::array<double,12> cents {};
    const auto tailEnergy=[&](double seconds) {
        renderer.prepare(48000);renderer.setReverb(seconds);renderer.play(0,false,cents);
        std::vector<float> left(48000*9),right(left.size());
        renderer.render(left.data(),right.data(),static_cast<int>(left.size()));
        check(std::all_of(left.begin(),left.end(),[](float x){return std::isfinite(x)&&std::abs(x)<=.98f;}),"Reverb is finite and bounded");
        check(energy(left,48000*8,left.size())==0,"Reverb finishes in silence");
        check(left!=right,"Reverb has stereo diffusion");
        check(energy(left,48000*6,48000*7)<energy(left,48000*2,48000*3)*.001,"Tail decays without persistent feedback");
        return energy(left,48000*3,48000*4);
    };
    const double shortTail=tailEnergy(1),longTail=tailEnergy(4);
    check(longTail>shortTail*100,"Longer reverb produces a longer tail");
    const auto filteredTail=[&](double frequency) {
        auto& pcm=renderer.samples[0].pcm;
        for(size_t i=0;i<pcm.size();++i) pcm[i]=static_cast<float>(.6*std::sin(2*3.141592653589793*frequency*i/48000));
        renderer.prepare(48000);renderer.setReverb(2);renderer.play(0,false,cents);
        std::vector<float> l(48000*3),r(l.size());renderer.render(l.data(),r.data(),static_cast<int>(l.size()));
        return energy(l,100800,144000)+energy(r,100800,144000);
    };
    const double inBand=filteredTail(1000);
    check(filteredTail(50)<inBand*.03,"Reverb suppresses frequencies below 200 Hz");
    check(filteredTail(10000)<inBand*.03,"Reverb suppresses frequencies above 2500 Hz");
    renderer.prepare(48000);renderer.setReverb(4);renderer.play(0,false,cents);
    std::vector<float> left(4800),right(4800);renderer.render(left.data(),right.data(),4800);
    renderer.stop();renderer.render(left.data(),right.data(),4800);
    check(energy(left,3360,left.size())==0,"Stopping clears voices and reverb after a 70ms fade");
    for(int i=0;i<100;++i)
    {
        renderer.setReverb((i%41)*.1);
        renderer.play(i%12,(i%2)!=0,cents);renderer.render(left.data(),right.data(),128);
        check(std::all_of(left.begin(),left.begin()+128,[](float x){return std::isfinite(x)&&std::abs(x)<=.98f;}),"Rapid selection stays bounded");
    }
    cents[0]=std::numeric_limits<double>::quiet_NaN();check(!renderer.play(0,false,cents),"Invalid tuning is rejected");
    cents[0]=0;
    const auto volumeEnergy=[&](double level) {
        renderer.prepare(48000);renderer.setReverb(0);renderer.setVolume(level);
        renderer.render(left.data(),right.data(),4800); // Smooth before the test note.
        renderer.play(0,false,cents);
        std::vector<float> l(48000),r(48000);renderer.render(l.data(),r.data(),48000);
        return energy(l,24000,48000);
    };
    const double full=volumeEnergy(1),half=volumeEnergy(.5),muted=volumeEnergy(0);
    check(std::abs(half/full-.25)<.001,"50 percent loudness halves amplitude");
    check(muted<full*1e-12,"Zero loudness mutes output after smoothing");
    std::cout<<checks<<" sampled audio checks passed\n";
}
