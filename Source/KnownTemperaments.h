// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <array>
#include <string>
#include <vector>

namespace temperament
{
struct KnownTemperament {std::string name,comments;std::array<double,12> cents {};};
struct KnownMatch
{
    size_t index=0;
    int rotationFifths=0; // 0..11 clockwise steps of the fifth circle.
    double maximumError=0,rmsError=0;
    double fifthRmsError=0,majorThirdRmsError=0,minorThirdRmsError=0,similarityError=0;
};
const std::vector<KnownTemperament>& knownTemperaments();
// Every close name/distinct rotation first, then the best harmonic alternatives
// until at least five distinct catalogue names are represented. Alternatives
// use each entry's best rotation; they are comparisons, not claimed identities.
// The limit classifies close matches; it does not exclude distant suggestions.
std::vector<KnownMatch> recogniseTemperament(const std::array<double,12>&,double maximumError=1.0);
std::string rotationDescription(int clockwiseFifths);
}
