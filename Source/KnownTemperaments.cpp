// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "KnownTemperaments.h"
#include "KnownTemperamentsData.h"
#include "Temperament.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

namespace temperament
{
const std::vector<KnownTemperament>& knownTemperaments()
{
    static const auto catalogue=[] {
        std::vector<KnownTemperament> entries;
        std::istringstream input(knownTemperamentCsv);
        std::string line;
        while(std::getline(input,line))
        {
            std::istringstream row(line);KnownTemperament item;
            if(!std::getline(row,item.name,';') || item.name.empty() || item.name=="Name") continue;
            bool valid=true;std::string value;
            for(double& cents:item.cents)
                if(!std::getline(row,value,';')||!parseNumber(value,cents)) {valid=false;break;}
            if(!valid) continue;
            std::getline(row,item.comments);
            while(!item.comments.empty()&&(item.comments.back()=='\r'||item.comments.back()==' ')) item.comments.pop_back();
            const double reference=item.cents[9];
            for(auto& cents:item.cents) cents-=reference;
            entries.push_back(std::move(item));
        }
        return entries;
    }();
    return catalogue;
}
std::vector<KnownMatch> recogniseTemperament(const std::array<double,12>& values,double limit)
{
    std::vector<KnownMatch> matches;
    std::vector<KnownMatch> alternatives;
    if(!std::isfinite(limit)||limit<0) return matches;
    for(double value:values) if(!std::isfinite(value)) return matches;
    std::array<double,12> normalised;
    for(int i=0;i<12;++i)
    {
        normalised[i]=values[i]-values[9];
        if(!std::isfinite(normalised[i])) return matches;
    }
    const auto bySimilarity=[](const auto& a,const auto& b) {
        if(a.similarityError!=b.similarityError) return a.similarityError<b.similarityError;
        if(a.maximumError!=b.maximumError) return a.maximumError<b.maximumError;
        if(a.index!=b.index) return a.index<b.index;
        return a.rotationFifths<b.rotationFifths;
    };
    const auto& catalogue=knownTemperaments();
    for(size_t index=0;index<catalogue.size();++index)
    {
        std::vector<std::array<double,12>> distinct;
        KnownMatch best;best.similarityError=std::numeric_limits<double>::infinity();
        bool hasCloseMatch=false;
        for(int rotation=0;rotation<12;++rotation)
        {
            const auto rotated=rotateChartFifths(catalogue[index].cents,rotation);
            // Equal temperament and other rotational symmetries need one entry,
            // rather than twelve identical copies of the same chart.
            const bool duplicate=std::any_of(distinct.begin(),distinct.end(),[&](const auto& previous) {
                for(int i=0;i<12;++i) if(std::abs(previous[i]-rotated[i])>1e-9) return false;
                return true;
            });
            if(duplicate) continue;
            distinct.push_back(rotated);
            KnownMatch match;match.index=index;match.rotationFifths=rotation;
            std::array<double,12> errors;
            for(int i=0;i<12;++i)
            {
                const double error=normalised[i]-rotated[i];errors[i]=error;
                match.maximumError=std::max(match.maximumError,std::abs(error));
                match.rmsError=std::hypot(match.rmsError,error/std::sqrt(12.0));
            }
            // Interval differences measure the harmonic pattern independently of
            // the reference note. Retain absolute cent differences (no rescaling
            // or correlation-only score that could disguise a much stronger tuning).
            const auto intervalRms=[&](int semitones) {
                double rms=0;
                for(int i=0;i<12;++i)
                    rms=std::hypot(rms,(errors[(i+semitones)%12]-errors[i])/std::sqrt(12.0));
                return rms;
            };
            match.fifthRmsError=intervalRms(7);
            match.majorThirdRmsError=intervalRms(4);
            match.minorThirdRmsError=intervalRms(3);
            // Fifths carry half the weight; the two third families share half.
            match.similarityError=std::hypot(match.fifthRmsError/std::sqrt(2.0),
                match.majorThirdRmsError/2,match.minorThirdRmsError/2);
            if(!std::isfinite(match.similarityError)||!std::isfinite(match.rmsError)) continue;
            if(bySimilarity(match,best)) best=match;
            if(match.maximumError<=limit+1e-9) {matches.push_back(match);hasCloseMatch=true;}
        }
        if(!hasCloseMatch&&std::isfinite(best.similarityError)) alternatives.push_back(best);
    }
    std::stable_sort(matches.begin(),matches.end(),[](const auto& a,const auto& b) {
        if(a.maximumError!=b.maximumError) return a.maximumError<b.maximumError;
        if(a.rmsError!=b.rmsError) return a.rmsError<b.rmsError;
        if(a.index!=b.index) return a.index<b.index;
        return a.rotationFifths<b.rotationFifths;
    });
    std::stable_sort(alternatives.begin(),alternatives.end(),bySimilarity);
    std::vector<size_t> names;
    for(const auto& match:matches)
        if(std::find(names.begin(),names.end(),match.index)==names.end()) names.push_back(match.index);
    for(const auto& alternative:alternatives)
    {
        if(names.size()>=5) break;
        matches.push_back(alternative);names.push_back(alternative.index);
    }
    return matches;
}
std::string rotationDescription(int clockwiseFifths)
{
    const int steps=(clockwiseFifths%12+12)%12;
    if(steps==0) return "original position";
    const int count=steps<=6?steps:12-steps;
    return std::to_string(count)+(count==1?" fifth ":" fifths ")
        +(steps<=6?"clockwise":"counter-clockwise");
}
}
