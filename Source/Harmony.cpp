// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "Harmony.h"
#include <algorithm>
#include <cmath>

namespace temperament
{
const std::array<CircleConnection,36>& fifthCircleConnections()
{
    static const auto connections = [] {
        std::array<CircleConnection,36> result {};
        int n = 0;
        for (auto kind : { HarmonyInterval::majorThird, HarmonyInterval::minorThird, HarmonyInterval::fifth })
            for (int i = 0; i < 12; ++i)
                result[static_cast<size_t>(n++)] = { i, (i + (kind == HarmonyInterval::fifth ? 1 : kind == HarmonyInterval::majorThird ? 4 : 9)) % 12, kind };
        return result;
    }();
    return connections;
}
int harmonySteps(HarmonyInterval kind)
{
    return kind == HarmonyInterval::fifth ? 7 : kind == HarmonyInterval::majorThird ? 4 : 3;
}
double harmonyPureCents(HarmonyInterval kind)
{
    return 1200 * std::log2(kind == HarmonyInterval::fifth ? 3.0 / 2 : kind == HarmonyInterval::majorThird ? 5.0 / 4 : 6.0 / 5);
}
const char* harmonyIntervalName(HarmonyInterval kind)
{
    return kind == HarmonyInterval::fifth ? "Fifth" : kind == HarmonyInterval::majorThird ? "Major third" : "Minor third";
}
const char* harmonyRatio(HarmonyInterval kind)
{
    return kind == HarmonyInterval::fifth ? "3:2" : kind == HarmonyInterval::majorThird ? "5:4" : "6:5";
}
Consonance consonance(double errorCents, ColourLimits limits)
{
    const double magnitude = std::abs(errorCents);
    if (!std::isfinite(magnitude)) return Consonance::rough;
    return magnitude <= limits.good + 1e-9 ? Consonance::good
        : magnitude <= limits.tempered + 1e-9 ? Consonance::tempered : Consonance::rough;
}
Consonance consonance(const IntervalAnalysis& interval, HarmonyColourLimits limits)
{
    return consonance(interval.errorCents, interval.kind == HarmonyInterval::fifth ? limits.fifths : limits.thirds);
}
Consonance consonance(const TriadAnalysis& chord, HarmonyColourLimits limits)
{
    auto worst = Consonance::good;
    for (const auto& interval : chord.intervals) worst = std::max(worst, consonance(interval, limits));
    return worst;
}
std::string harmonyRootName(int root, bool minor)
{
    static const char* majorNames[] { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    static const char* minorNames[] { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "G#", "A", "Bb", "B" };
    return (minor ? minorNames : majorNames)[(root % 12 + 12) % 12];
}
std::array<std::string, 12> harmonyNoteNames(int root, bool minor)
{
    root = (root % 12 + 12) % 12;
    const bool flats = minor ? (root == 0 || root == 2 || root == 3 || root == 5 || root == 7 || root == 10)
                             : (root == 1 || root == 3 || root == 5 || root == 8 || root == 10);
    std::array<std::string, 12> names = flats
        ? std::array<std::string, 12> { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
        : std::array<std::string, 12> { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    // Scale tones follow successive letter names, so e.g. F# major gets E#, not F.
    const std::string letters = "CDEFGAB";
    const int naturals[] { 0, 2, 4, 5, 7, 9, 11 };
    const int majorSteps[] { 0, 2, 4, 5, 7, 9, 11 };
    const int minorSteps[] { 0, 2, 3, 5, 7, 8, 10 };
    const auto firstLetter = letters.find(harmonyRootName(root, minor).front());
    for (size_t degree = 0; degree < 7; ++degree)
    {
        const auto letter = (firstLetter + degree) % 7;
        const int pitch = (root + (minor ? minorSteps : majorSteps)[degree]) % 12;
        const int accidental = (pitch - naturals[letter] + 18) % 12 - 6;
        names[static_cast<size_t>(pitch)] = std::string(1, letters[letter])
            + std::string(static_cast<size_t>(std::abs(accidental)), accidental < 0 ? 'b' : '#');
    }
    return names;
}
HarmonyAnalysis analyseHarmony(const std::array<double, 12>& cents)
{
    HarmonyAnalysis result;
    for (const auto value : cents)
        if (!std::isfinite(value) || std::abs(value) > 24000)
        {
            result.error = "A valid twelve-note chart is required.";
            return result;
        }
    for (const auto kind : { HarmonyInterval::fifth, HarmonyInterval::majorThird, HarmonyInterval::minorThird })
        for (int root = 0; root < 12; ++root)
        {
            const int steps = harmonySteps(kind);
            const int other = (root + steps) % 12;
            const double interval = steps * 100.0 + cents[static_cast<size_t>(other)] - cents[static_cast<size_t>(root)];
            result.intervals[static_cast<size_t>(kind)][static_cast<size_t>(root)] =
                { root, other, kind, interval, interval - harmonyPureCents(kind) };
        }
    for (int root = 0; root < 12; ++root)
        for (int minor = 0; minor < 2; ++minor)
        {
            auto& triad = result.triads[static_cast<size_t>(root * 2 + minor)];
            const int third = (root + (minor ? 3 : 4)) % 12;
            triad.root = root;
            triad.minor = minor != 0;
            triad.notes = { root, third, (root + 7) % 12 };
            triad.intervals = {
                result.intervals[0][static_cast<size_t>(root)],
                result.intervals[minor ? 2u : 1u][static_cast<size_t>(root)],
                result.intervals[minor ? 1u : 2u][static_cast<size_t>(third)]
            };
            for (const auto& interval : triad.intervals)
                triad.worstError = std::max(triad.worstError, std::abs(interval.errorCents));
        }
    result.valid = true;
    return result;
}
int latticeNote(int fifthSteps, int majorThirdSteps, int origin)
{
    const int note = (origin + 7 * fifthSteps + 4 * majorThirdSteps) % 12;
    return note < 0 ? note + 12 : note;
}
}
