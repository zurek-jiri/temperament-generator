// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <array>
#include <string>

namespace temperament
{
enum class HarmonyInterval { fifth, majorThird, minorThird };
enum class Consonance { good, tempered, rough };
struct CircleConnection { int fromIndex, toIndex; HarmonyInterval kind; };
// Circle indices, clockwise from C. All 36 directed intervals appear once.
const std::array<CircleConnection,36>& fifthCircleConnections();
struct ColourLimits { double good = 6, tempered = 18; };
struct HarmonyColourLimits
{
    ColourLimits fifths { 2, 6 };
    ColourLimits thirds { 8, 22 };
};
struct IntervalAnalysis
{
    int from = 0, to = 0;
    HarmonyInterval kind = HarmonyInterval::fifth;
    double cents = 0, errorCents = 0;
};
struct TriadAnalysis
{
    int root = 0;
    bool minor = false;
    std::array<int, 3> notes {};
    std::array<IntervalAnalysis, 3> intervals {};
    double worstError = 0;
};
struct HarmonyAnalysis
{
    bool valid = false;
    std::array<std::array<IntervalAnalysis, 12>, 3> intervals {};
    std::array<TriadAnalysis, 24> triads {};
    std::string error;
};
int harmonySteps(HarmonyInterval);
double harmonyPureCents(HarmonyInterval);
const char* harmonyIntervalName(HarmonyInterval);
const char* harmonyRatio(HarmonyInterval);
Consonance consonance(double errorCents, ColourLimits = {});
Consonance consonance(const IntervalAnalysis&, HarmonyColourLimits = {});
Consonance consonance(const TriadAnalysis&, HarmonyColourLimits = {});
std::string harmonyRootName(int root, bool minor);
std::array<std::string, 12> harmonyNoteNames(int root, bool minor);
HarmonyAnalysis analyseHarmony(const std::array<double, 12>& cents);
int latticeNote(int fifthSteps, int majorThirdSteps, int origin);
}
