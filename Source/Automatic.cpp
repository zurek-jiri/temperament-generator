// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "Temperament.h"
#include <cmath>
#include <numeric>

namespace temperament
{
namespace
{
CommaRatio estimateRatio(double cents, Mode unit)
{
    CommaRatio best;
    best.unit = unit;
    const double value = cents / commaSize(unit);
    int bestNumerator = 0;
    for (int denominator = 1; denominator <= 24; ++denominator)
    {
        const int numerator = static_cast<int>(std::round(value * denominator));
        const double error = static_cast<double>(numerator) / denominator * commaSize(unit) - cents;
        if (denominator == 1 || std::abs(error) < std::abs(best.errorCents) - 1e-10)
        {
            const int divisor = std::gcd(std::abs(numerator), denominator);
            bestNumerator = numerator / divisor;
            best.denominator = denominator / divisor;
            best.errorCents = error;
        }
    }
    best.fraction = std::to_string(bestNumerator) + (best.denominator == 1 ? "" : "/" + std::to_string(best.denominator));
    best.exact = std::abs(best.errorCents) <= 1e-7;
    return best;
}
}

AutomaticRatios automaticRatios(double cents, Mode preferred)
{
    AutomaticRatios result;
    result.syntonic = estimateRatio(cents, Mode::syntonicFifths);
    result.pythagorean = estimateRatio(cents, Mode::pythagoreanFifths);
    const double difference = std::abs(result.syntonic.errorCents) - std::abs(result.pythagorean.errorCents);
    if (std::abs(difference) > 1e-10)
        result.closest = difference < 0 ? Mode::syntonicFifths : Mode::pythagoreanFifths;
    else if (result.syntonic.denominator != result.pythagorean.denominator)
        result.closest = result.syntonic.denominator < result.pythagorean.denominator ? Mode::syntonicFifths : Mode::pythagoreanFifths;
    else result.closest = preferred;
    return result;
}

AutomaticCalculation resolveAutomatic(Mode mode, const std::array<FifthCorrection, 12>& input)
{
    AutomaticCalculation result;
    const bool paired = mode == Mode::syntonicFifths;
    double fixedCents = 0;
    for (const auto& fifth : input)
        for (int column = 0; column < (paired ? 2 : 1); ++column)
        {
            const bool automatic = column == 0 ? fifth.automaticPrimary : fifth.automaticPythagorean;
            const double value = column == 0 ? fifth.primary : fifth.pythagorean;
            const double unit = column == 0 ? commaSize(mode) : pythagoreanComma();
            if (automatic) ++result.automaticFields;
            else
            {
                if (!std::isfinite(value) || std::abs(value) > 1000)
                {
                    result.error = "Comma corrections must be finite and between -1000 and 1000.";
                    return result;
                }
                fixedCents += value * unit;
            }
        }
    if (result.automaticFields > 0)
        result.centsPerField = (12 * (equalInterval(mode) - pureInterval(mode)) - fixedCents)
                            / result.automaticFields;
    for (size_t i = 0; i < input.size(); ++i)
    {
        const auto& fifth = input[i];
        const double primary = fifth.automaticPrimary ? result.centsPerField / commaSize(mode) : fifth.primary;
        const double secondary = !paired ? 0 : fifth.automaticPythagorean
            ? result.centsPerField / pythagoreanComma() : fifth.pythagorean;
        const double total = primary + secondary * pythagoreanComma() / commaSize(mode);
        if (!std::isfinite(total) || std::abs(primary) > 1000 || std::abs(secondary) > 1000 || std::abs(total) > 1000)
        {
            result.error = "The required correction exceeds 1000 commas. Reduce fixed values or select more automatic fields.";
            return result;
        }
        result.primaryFractions[i] = primary;
        result.pythagoreanFractions[i] = secondary;
        result.fractions[i] = total;
    }
    result.valid = true;
    if (result.automaticFields > 0) result.ratios = automaticRatios(result.centsPerField, mode);
    return result;
}
}
