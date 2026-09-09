// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "Temperament.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <cctype>
#include <numeric>

namespace temperament
{
double pythagoreanComma() { return 1200.0 * std::log2(531441.0 / 524288.0); }
double syntonicComma() { return 1200.0 * std::log2(81.0 / 80.0); }
double schisma() { return pythagoreanComma() - syntonicComma(); }
double commaSize(Mode mode) { return mode == Mode::pythagoreanFifths ? pythagoreanComma() : syntonicComma(); }
double pureInterval(Mode) { return 1200.0 * std::log2(1.5); }
double equalInterval(Mode) { return 700.0; }
double equalFraction(Mode mode) { return (equalInterval(mode) - pureInterval(mode)) / commaSize(mode); }

const std::vector<Cycle>& cycles(Mode)
{
    // Clockwise fifths. Edge 6 joins the two branches at the bottom of the circle.
    static const std::vector<Cycle> fifths { { { 0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5 }, 6 } };
    return fifths;
}

const std::array<std::string, 12>& noteNames()
{
    static const std::array<std::string, 12> names {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    return names;
}

const std::array<SimpleFraction, 17>& simpleFractions()
{
    static const std::array<SimpleFraction, 17> choices {{
        { "0", 0 }, { "1", 1 }, { "1/2", 0.5 }, { "1/3", 1.0 / 3 },
        { "1/4", 0.25 }, { "1/5", 0.2 }, { "1/6", 1.0 / 6 }, { "1/12", 1.0 / 12 },
        { "-1", -1 }, { "-1/2", -0.5 }, { "-1/3", -1.0 / 3 },
        { "-1/4", -0.25 }, { "-1/5", -0.2 }, { "-1/6", -1.0 / 6 }, { "-1/12", -1.0 / 12 },
        { "1/24", 1.0 / 24 }, { "-1/24", -1.0 / 24 }
    }};
    return choices;
}

size_t nearestSimpleFraction(double value)
{
    const auto& choices = simpleFractions();
    size_t best = 0;
    for (size_t i = 1; i < choices.size(); ++i)
    {
        const double difference = std::abs(value - choices[i].value);
        const double bestDifference = std::abs(value - choices[best].value);
        // Stable tie break: choose the smaller magnitude, then menu order.
        if (difference < bestDifference - 1e-12
            || (std::abs(difference - bestDifference) <= 1e-12
                && std::abs(choices[i].value) < std::abs(choices[best].value)))
            best = i;
    }
    return best;
}

std::array<double, 12> rotateChartFifths(const std::array<double, 12>& chart, int steps)
{
    const int shift = ((steps % 12 + 12) % 12) * 7 % 12;
    std::array<double, 12> rotated {};
    const double reference = chart[(9 - shift + 12) % 12];
    for (int i = 0; i < 12; ++i) rotated[i] = chart[(i - shift + 12) % 12] - reference;
    rotated[9] = 0;
    return rotated;
}

ReverseCalculation reverseCsv(const std::string& line, Mode mode, Reconstruction reconstruction)
{
    ReverseCalculation result;
    size_t start = 0;
    for (size_t i = 0; i < 12; ++i)
    {
        const auto end = line.find(';', start);
        if ((i < 11 && end == std::string::npos) || (i == 11 && end != std::string::npos))
        {
            result.error = "Enter exactly 12 values separated by semicolons, from C to B.";
            return result;
        }
        if (!parseNumber(line.substr(start, end == std::string::npos ? end : end - start), result.cents[i])
            || std::abs(result.cents[i]) > 12000)
        {
            result.error = "Invalid value for " + noteNames()[i] + ". Use cents between -12000 and 12000.";
            return result;
        }
        start = end == std::string::npos ? line.size() : end + 1;
    }
    const double reference = result.cents[9];
    for (double& value : result.cents) value -= reference;
    result.cents[9] = 0;
    const auto& notes = cycles(Mode::pythagoreanFifths).front().notes;
    const auto choices = reconstructionChoices(mode);
    for (size_t i = 0; i < notes.size(); ++i)
    {
        const double deviation = result.cents[static_cast<size_t>(notes[(i + 1) % notes.size()])]
                               - result.cents[static_cast<size_t>(notes[i])];
        const double fraction = (700.0 + deviation - pureInterval(mode)) / commaSize(mode);
        const ExpressionChoice* best = &choices.front();
        for (const auto& choice : choices)
            if (std::abs(choice.value - fraction) < std::abs(best->value - fraction) - 1e-12
                || (std::abs(std::abs(choice.value - fraction) - std::abs(best->value - fraction)) <= 1e-12
                    && std::abs(choice.value) < std::abs(best->value) - 1e-12))
                best = &choice;
        const auto precise = reconstruction == Reconstruction::precise
            ? preciseExpression(fraction * commaSize(mode), mode) : *best;
        result.expressions[i] = precise.text;
        result.pythagoreanExpressions[i] = "0";
        result.intervalErrors[i] = (precise.value - fraction) * commaSize(mode);
        result.closureError += pureInterval(mode) - 700.0 + precise.value * commaSize(mode);
    }
    result.valid = true;
    return result;
}

static bool parseDecimal(std::string text, double& result)
{
    std::replace(text.begin(), text.end(), ',', '.');
    std::istringstream stream(text);
    stream.imbue(std::locale::classic());
    stream >> std::ws >> result;
    if (stream.fail() || !std::isfinite(result))
        return false;
    stream >> std::ws;
    return stream.eof();
}

bool parseNumber(const std::string& text, double& result)
{
    const auto slash = text.find('/');
    if (slash == std::string::npos)
        return parseDecimal(text, result);
    if (text.find('/', slash + 1) != std::string::npos)
        return false;
    double numerator = 0.0, denominator = 0.0;
    if (!parseDecimal(text.substr(0, slash), numerator)
        || !parseDecimal(text.substr(slash + 1), denominator) || denominator == 0.0)
        return false;
    result = numerator / denominator;
    return std::isfinite(result);
}

double closingFraction(Mode mode, const std::vector<double>& fractions, int closingEdge)
{
    double required = static_cast<double>(fractions.size()) * equalFraction(mode);
    for (size_t i = 0; i < fractions.size(); ++i)
        if (static_cast<int>(i) != closingEdge)
            required -= fractions[i];
    return required;
}

Calculation calculate(Mode mode, const std::vector<std::vector<double>>& fractions)
{
    Calculation result;
    const auto& rings = cycles(mode);
    if (fractions.size() != rings.size())
    {
        result.error = "Incorrect number of circles.";
        return result;
    }
    for (size_t ring = 0; ring < rings.size(); ++ring)
    {
        if (fractions[ring].size() != rings[ring].notes.size())
        {
            result.error = "Incorrect number of intervals.";
            return result;
        }
        double residual = 0.0;
        for (const double fraction : fractions[ring])
        {
            if (!std::isfinite(fraction) || std::abs(fraction) > 1000.0)
            {
                result.error = "Comma fractions must be finite and between -1000 and 1000.";
                return result;
            }
            residual += pureInterval(mode) - equalInterval(mode) + fraction * commaSize(mode);
        }
        result.closureErrors.push_back(residual);
    }
    if (std::any_of(result.closureErrors.begin(), result.closureErrors.end(),
                    [](double error) { return std::abs(error) > 0.000001; }))
    {
        result.error = "The intervals do not close. Edit the fractions or use Close circles.";
        return result;
    }
    for (size_t ring = 0; ring < rings.size(); ++ring)
    {
        const auto& notes = rings[ring].notes;
        double offset = 0.0;
        result.cents[static_cast<size_t>(notes.front())] = offset;
        for (size_t i = 0; i + 1 < notes.size(); ++i)
        {
            offset += pureInterval(mode) - equalInterval(mode) + fractions[ring][i] * commaSize(mode);
            result.cents[static_cast<size_t>(notes[i + 1])] = offset;
        }
    }
    const double reference = result.cents[9];
    for (double& value : result.cents)
        value -= reference;
    result.cents[9] = 0.0;
    result.valid = true;
    return result;
}

std::string formatCents(double value, bool showPositiveSign)
{
    const double rounded = std::round(value * 1000.0) / 1000.0;
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    if (showPositiveSign && rounded > 0.0)
        stream << '+';
    stream << std::fixed << std::setprecision(3) << (rounded == 0.0 ? 0.0 : rounded);
    return stream.str();
}

std::string csv(const std::array<double, 12>& values)
{
    std::string line;
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i != 0) line += ';';
        line += formatCents(values[i]);
    }
    return line;
}
}
