// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "Temperament.h"
#include "Harmony.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

using namespace temperament;
static int checks = 0;
static void check(bool condition, const char* message)
{
    ++checks;
    if (!condition) { std::cerr << "FAILED: " << message << '\n'; std::exit(1); }
}
static bool near(double a, double b, double tolerance = 1.0e-8) { return std::abs(a - b) < tolerance; }
static double evaluated(const std::string& expression, Mode mode = Mode::pythagoreanFifths)
{
    double value = 0;
    std::string error;
    check(evaluateExpression(expression, mode, value, error), "Returned expression is valid");
    return value;
}
static double recoveredFraction(const ReverseCalculation& reverse, size_t i, Mode mode)
{
    return evaluated(reverse.expressions[i], mode)
        + evaluated(reverse.pythagoreanExpressions[i]) * pythagoreanComma() / commaSize(mode);
}
static std::vector<std::vector<double>> equalInputs(Mode mode)
{
    std::vector<std::vector<double>> values;
    for (const auto& ring : cycles(mode)) values.emplace_back(ring.notes.size(), equalFraction(mode));
    return values;
}

int main()
{
    check(near(pythagoreanComma(), 23.460010384649, 1e-9), "Pythagorean comma ratio");
    check(near(syntonicComma(), 21.506289596715, 1e-9), "Syntonic comma ratio");
    double number = 0;
    check(parseNumber(" -1 / 12 ", number) && near(number, -1.0 / 12.0), "Signed fraction with spaces");
    check(parseNumber("0.2", number) && near(number, 0.2), "Decimal");
    check(parseNumber("0,2", number) && near(number, 0.2), "Decimal comma");
    check(parseNumber("+1/6", number) && near(number, 1.0 / 6), "Explicit positive sign");
    check(parseNumber("1e-3", number) && near(number, 0.001), "Scientific notation");
    for (const auto* bad : { "", " ", "1/0", "1/", "/2", "1/2/3", "0.2x", "nan", "inf", "1e999", "1 2", "1,2.3" })
        check(!parseNumber(bad, number), "Reject invalid numeric input");
    for (auto mode : { Mode::pythagoreanFifths, Mode::syntonicFifths })
    {
        const auto et = calculate(mode, equalInputs(mode));
        check(et.valid, "Equal temperament closes");
        for (double value : et.cents) check(near(value, 0), "Equal temperament gives zero deviations");
        check(et.cents[9] == 0.0, "A is exactly zero");
    }
    auto fifths = equalInputs(Mode::pythagoreanFifths);
    fifths[0].assign(12, 0.0);
    auto unclosed = calculate(Mode::pythagoreanFifths, fifths);
    check(!unclosed.valid && near(unclosed.closureErrors[0], pythagoreanComma()), "Pure fifths do not close");
    fifths[0][6] = closingFraction(Mode::pythagoreanFifths, fifths[0], 6);
    auto pythagorean = calculate(Mode::pythagoreanFifths, fifths);
    check(pythagorean.valid && near(fifths[0][6], -1), "Wolf fifth consumes one Pythagorean comma");
    check(near(pythagorean.cents[0], -5.865002596162, 1e-9), "Pythagorean C relative to A");
    check(near(pythagorean.cents[4], 1.955000865387, 1e-9), "Pythagorean E relative to A");
    auto syntonic = equalInputs(Mode::syntonicFifths);
    syntonic[0].assign(12, 0.0);
    unclosed = calculate(Mode::syntonicFifths, syntonic);
    check(!unclosed.valid && near(unclosed.closureErrors[0], pythagoreanComma()), "Pure syntonic fifth circle has the Pythagorean closure gap");
    syntonic[0][6] = closingFraction(Mode::syntonicFifths, syntonic[0], 6);
    check(near(syntonic[0][6], -pythagoreanComma() / syntonicComma()), "Syntonic closure includes the schisma");
    const auto syntonicChart = calculate(Mode::syntonicFifths, syntonic);
    check(syntonicChart.valid && csv(syntonicChart.cents) == csv(pythagorean.cents), "Same physical fifths give the same chart in both units");
    // Reconstruct every requested interval from the returned pitch-class deviations.
    for (const auto mode : { Mode::pythagoreanFifths, Mode::syntonicFifths })
    {
        auto input = equalInputs(mode);
        for (size_t r = 0; r < input.size(); ++r)
        {
            for (size_t i = 0; i < input[r].size(); ++i) input[r][i] = 0.1 * static_cast<double>(i) - 0.25;
            const auto closing = cycles(mode)[r].closingEdge;
            input[r][static_cast<size_t>(closing)] = closingFraction(mode, input[r], closing);
        }
        const auto result = calculate(mode, input);
        check(result.valid, "Nonuniform temperament closes");
        for (size_t r = 0; r < input.size(); ++r)
        {
            const auto& notes = cycles(mode)[r].notes;
            for (size_t i = 0; i < notes.size(); ++i)
                check(near(result.cents[notes[(i + 1) % notes.size()]] - result.cents[notes[i]],
                           pureInterval(mode) - equalInterval(mode) + input[r][i] * commaSize(mode)), "Every edge agrees, including closing edge");
        }
    }
    check(formatCents(-0.0001) == "0.000", "No negative zero");
    check(formatCents(2.1236, true) == "+2.124", "Three decimals with sign");
    check(csv(pythagorean.cents) == "-5.865;-15.640;-1.955;-11.730;1.955;-7.820;5.865;-3.910;-13.685;0.000;-9.775;3.910",
          "Pythagorean CSV in chromatic order, C through B");
    const auto line = csv(calculate(Mode::pythagoreanFifths, equalInputs(Mode::pythagoreanFifths)).cents);
    check(line == "0.000;0.000;0.000;0.000;0.000;0.000;0.000;0.000;0.000;0.000;0.000;0.000", "CSV order and format");
    check(!calculate(Mode::syntonicFifths, {}).valid, "Reject missing circles");
    const auto reversedEt = reverseCsv(line);
    check(reversedEt.valid, "Import ET CSV");
    for (size_t i = 0; i < 12; ++i)
    {
        check(reversedEt.expressions[i] == "-1/12", "ET imports as a clear fraction");
        check(near(reversedEt.intervalErrors[i], 0), "ET is exact");
    }
    const auto reversedPythagorean = reverseCsv(csv(pythagorean.cents));
    check(reversedPythagorean.valid && near(reversedPythagorean.closureError, 0), "Rounded Pythagorean CSV recovers a closed circle");
    for (size_t i = 0; i < 12; ++i)
    {
        check(near(evaluated(reversedPythagorean.expressions[i]), i == 6 ? -1.0 : 0.0), "Wolf and pure intervals recovered");
        check(std::abs(reversedPythagorean.intervalErrors[i]) <= 0.0010001, "CSV rounding is at most 0.001 cents per interval");
    }
    // A closed temperament containing positive and negative menu fractions.
    const std::vector<std::vector<double>> mixed { { 0.5, -0.5, 1.0/3, -1.0/3, 0.25, -0.25, -1, 0.2, -0.2, 1.0/6, -1.0/6, 0 } };
    const auto mixedChart = calculate(Mode::pythagoreanFifths, mixed);
    const auto mixedReverse = reverseCsv(csv(mixedChart.cents));
    check(mixedChart.valid && mixedReverse.valid, "Mixed fractional temperament round trip");
    for (size_t i = 0; i < 12; ++i)
        check(near(evaluated(mixedReverse.expressions[i]), mixed[0][i]), "Every mixed fraction is recovered in circle order");
    auto transposed = mixedChart.cents;
    for (auto& value : transposed) value += 12.345;
    const auto shiftedReverse = reverseCsv(csv(transposed));
    check(shiftedReverse.valid && shiftedReverse.cents[9] == 0, "CSV reference is normalised to A");
    check(shiftedReverse.expressions == mixedReverse.expressions, "Uniform transposition does not change fractions");
    const auto arbitrary = reverseCsv("0;4;-1;2;-3;5;-2;1;3;0;-4;2");
    check(arbitrary.valid && std::abs(arbitrary.closureError) > 0.000001, "Independent nearest fractions can fail closure");
    const auto& fifthNotes = cycles(Mode::pythagoreanFifths)[0].notes;
    for (size_t i = 0; i < 12; ++i)
    {
        const double raw = (700 + arbitrary.cents[fifthNotes[(i + 1) % 12]] - arbitrary.cents[fifthNotes[i]]
                           - pureInterval(Mode::pythagoreanFifths)) / pythagoreanComma();
        const auto recovered = evaluated(arbitrary.expressions[i]);
        for (const auto& choice : expressionChoices(Mode::pythagoreanFifths))
            check(std::abs(raw - recovered) <= std::abs(raw - choice.value) + 1e-12,
                  "Each reversed interval uses the nearest menu fraction");
    }
    check(simpleFractions()[nearestSimpleFraction(1.0/48)].value == 0, "Stable midpoint tie chooses smaller magnitude");
    check(near(simpleFractions()[nearestSimpleFraction(1.0/24)].value,1.0/24), "One twenty-fourth is in the menu");
    check(reverseCsv("0,0;0;0;0;0;0;0;0;0;0;0;0\r\n").valid, "Accept decimal comma and trailing line ending");
    for (const auto* invalid : { "", "0;0", "0;0;0;0;0;0;0;0;0;0;0;0;", "0;0;0;0;0;0;0;0;0;0;0;0;0",
                                 "0;0;0;0;0;0;0;0;0;0;0;nan", "0;0;0;0;0;0;0;0;0;0;0;", "0;0;0;0;0;0;0;0;0;0;0;12001" })
        check(!reverseCsv(invalid).valid, "Reject malformed CSV");
    check(near(schisma(), 1200 * std::log2(32805.0 / 32768.0)), "Schisma is Pythagorean minus syntonic comma");
    for (auto mode : { Mode::pythagoreanFifths, Mode::syntonicFifths })
    {
        std::string error;
        check(evaluateExpression("1-1/11", mode, number, error) && near(number, 10.0 / 11), "Subtraction and division precedence");
        check(evaluateExpression("-(1-1/11)/2", mode, number, error) && near(number, -5.0 / 11), "Parentheses and unary minus");
        check(evaluateExpression("2*(1/3+1/6)", mode, number, error) && near(number, 1), "Multiplication of sums");
        check(evaluateExpression(" 0,2 + 1e-3 ", mode, number, error) && near(number, 0.201), "Decimal comma and scientific notation in expressions");
        check(evaluateExpression("schisma", mode, number, error) && near(number * commaSize(mode), schisma()), "Named schisma has a physical cent value");
        check(evaluateExpression("-H/2", mode, number, error) && near(number * commaSize(mode), -schisma() / 2), "Schisma fraction");
        check(evaluateExpression("P-S", mode, number, error) && near(number * commaSize(mode), schisma()), "Difference of named commas");
        check(evaluateExpression("-syntonic/4+pythagorean/12", mode, number, error)
              && near(number * commaSize(mode), -syntonicComma() / 4 + pythagoreanComma() / 12), "Mixed named comma expression");
        check(evaluateExpression("ET", mode, number, error) && near(number, equalFraction(mode)), "Equal temperament alias in both units");
        for (const auto* bad : { "", "1-", "1/(2-2)", "(1/2", "1/2)", "1**2", "1 2", "unknown", "nan", "1e999", "1e308*1e308", "2H", "()" })
            check(!evaluateExpression(bad, mode, number, error) && !error.empty(), "Reject malformed or nonfinite expressions");
        check(!evaluateExpression(std::string(70, '(') + "1" + std::string(70, ')'), mode, number, error), "Bound expression nesting");
        check(!evaluateExpression(std::string(600, '1'), mode, number, error), "Bound expression length");
        const auto reverse = reverseCsv(csv(pythagorean.cents), mode);
        check(reverse.valid && near(reverse.closureError, 0), "CSV imports into the selected comma unit");
        std::vector<std::vector<double>> recovered { std::vector<double>(12) };
        for (size_t i = 0; i < 12; ++i)
        {
            recovered[0][i] = recoveredFraction(reverse, i, mode);
            check(std::abs(reverse.intervalErrors[i]) <= 0.0010001, "Imported expressions preserve CSV precision");
        }
        check(csv(calculate(mode, recovered).cents) == csv(pythagorean.cents), "Forward/reverse round trip in each unit");
        const auto equalReverse = reverseCsv(line, mode);
        for (size_t i = 0; i < 12; ++i)
        {
            check(near(recoveredFraction(equalReverse, i, mode), equalFraction(mode)), "ET CSV recovers exact fractional expressions");
            if (mode == Mode::syntonicFifths)
                check(equalReverse.expressions[i] == "-P/12" && equalReverse.pythagoreanExpressions[i] == "0",
                      "Unified ET import explicitly uses Pythagorean units");
        }
        for (const auto& choice : expressionChoices(mode))
            check(evaluateExpression(choice.text, mode, number, error) && near(number, choice.value), "All menu expressions parse consistently");
    }
    std::string parseError;
    check(evaluateExpression("-1-schisma", Mode::syntonicFifths, number, parseError)
          && near(number * syntonicComma(), -pythagoreanComma()), "One syntonic comma plus schisma equals one Pythagorean comma");
    check(fractionExpression(number, Mode::syntonicFifths) == "-P", "Closing wolf uses an explicit Pythagorean expression");
    const auto quarter = pairedExpressions(evaluated("-1/4-H/4", Mode::syntonicFifths));
    check(quarter.syntonic == "0" && quarter.pythagorean == "-1/4", "Quarter S plus quarter schisma belongs in P");
    // Recover every signed menu fraction from a three-decimal chart in its native column.
    for (auto unit : { Mode::syntonicFifths, Mode::pythagoreanFifths })
        for (const auto& fraction : simpleFractions())
        {
            std::vector<std::vector<double>> input { std::vector<double>(12, 0) };
            input[0][0] = fraction.value * commaSize(unit) / pythagoreanComma();
            input[0][6] = closingFraction(Mode::pythagoreanFifths, input[0], 6);
            const auto chart = calculate(Mode::pythagoreanFifths, input);
            const auto reverse = reverseCsv(csv(chart.cents), Mode::syntonicFifths);
            check(chart.valid && reverse.valid, "Native comma fraction imports from rounded CSV");
            check(fraction.value==0 || reverse.expressions[0].find_first_of("PS")!=std::string::npos || reverse.expressions[0].find("schisma")!=std::string::npos, "Unified fraction explicitly names a comma");
            check(reverse.pythagoreanExpressions[0] == "0", "No hidden second contribution");
            check(std::abs(reverse.intervalErrors[0]) <= 0.0010001, "Column choice retains CSV precision");
            // P/24 and H/2 differ by only 0.00064 cents, so rounded CSV may
            // legitimately select the other. Test the physical accuracy.
            check(near(recoveredFraction(reverse, 0, Mode::syntonicFifths) * syntonicComma(), fraction.value * commaSize(unit),0.0010001),
                  "Unified fractions retain the physical correction within CSV precision");
        }
    for (const auto* expression : { "H", "-H", "H/4", "-H/12" })
    {
        std::vector<std::vector<double>> input { std::vector<double>(12, 0) };
        input[0][0] = evaluated(expression);
        input[0][6] = closingFraction(Mode::pythagoreanFifths, input[0], 6);
        const auto reverse = reverseCsv(csv(calculate(Mode::pythagoreanFifths, input).cents), Mode::syntonicFifths);
        check(reverse.valid && std::abs(evaluated(reverse.expressions[0],Mode::syntonicFifths)*syntonicComma()-evaluated(expression)*pythagoreanComma())<1e-7 && reverse.pythagoreanExpressions[0] == "0",
              "Schisma remains explicit when it is the correction itself");
    }
    const auto pairedArbitrary = reverseCsv("0;4;-1;2;-3;5;-2;1;3;0;-4;2", Mode::syntonicFifths);
    for (size_t i = 0; i < 12; ++i)
    {
        const double raw = 700 + pairedArbitrary.cents[fifthNotes[(i + 1) % 12]] - pairedArbitrary.cents[fifthNotes[i]] - pureInterval(Mode::syntonicFifths);
        const double correction = recoveredFraction(pairedArbitrary, i, Mode::syntonicFifths) * syntonicComma();
        check(near(correction - raw, pairedArbitrary.intervalErrors[i]), "Rounding error includes both columns");
        for (const auto& choice : expressionChoices(Mode::syntonicFifths))
            check(std::abs(correction - raw) <= std::abs(choice.value * syntonicComma() - raw) + 1e-9,
                  "Choosing native columns preserves nearest-menu accuracy");
    }
    check(isAutomaticExpression(" Automatic calculation ") && isAutomaticExpression("AUTO"), "Recognise automatic selection and short text");
    check(!isAutomaticExpression("Auto/2") && !isAutomaticExpression("1+Auto"), "Auto is a field mode, not arithmetic");
    for (const auto mode : { Mode::pythagoreanFifths, Mode::syntonicFifths })
    {
        std::array<FifthCorrection, 12> automatic {};
        for (auto& fifth : automatic) fifth.automaticPrimary = true;
        const auto balanced = resolveAutomatic(mode, automatic);
        check(balanced.valid && balanced.automaticFields == 12, "All automatic fields resolve");
        check(near(balanced.centsPerField, -pythagoreanComma() / 12), "Each automatic field gets an equal cent share");
        const auto chart = calculate(mode, { std::vector<double>(balanced.fractions.begin(), balanced.fractions.end()) });
        check(chart.valid && chart.cents[9] == 0, "All-auto closes at A = 0");
        for (size_t i = 0; i < 12; ++i)
        {
            check(near(chart.cents[i], 0), "All-auto in one column produces equal temperament");
            check(near(balanced.primaryFractions[i], equalFraction(mode)), "Automatic values use the column's comma unit");
        }
    }
    std::array<FifthCorrection, 12> automatic {};
    for (const auto i : { 0, 3, 7, 10 }) automatic[i].automaticPrimary = true;
    auto balanced = resolveAutomatic(Mode::pythagoreanFifths, automatic);
    check(balanced.valid && balanced.automaticFields == 4, "Four automatic fifths resolve");
    for (size_t i = 0; i < 12; ++i)
        check(near(balanced.fractions[i], automatic[i].automaticPrimary ? -0.25 : 0), "Only automatic fields receive the missing comma");
    automatic[6].primary = -2;
    balanced = resolveAutomatic(Mode::pythagoreanFifths, automatic);
    check(balanced.valid && near(balanced.centsPerField, pythagoreanComma() / 4), "Excess narrowing gives positive automatic corrections");
    check(balanced.primaryFractions[6] == -2, "Fixed closing fifth is preserved");
    automatic[6].primary = -1;
    balanced = resolveAutomatic(Mode::pythagoreanFifths, automatic);
    check(balanced.valid && near(balanced.centsPerField, 0), "Already closed fixed contributions give zero to Auto");
    automatic = {};
    automatic[0].automaticPrimary = true;
    automatic[0].automaticPythagorean = true;
    automatic[1].automaticPythagorean = true;
    automatic[2].primary = 10.0 / 11;
    automatic[6].pythagorean = -0.25;
    balanced = resolveAutomatic(Mode::syntonicFifths, automatic);
    const double expectedShare = (-0.75 * pythagoreanComma() - (10.0 / 11) * syntonicComma()) / 3;
    check(balanced.valid && balanced.automaticFields == 3 && near(balanced.centsPerField, expectedShare), "Auto counts fields across both columns, including the same fifth");
    check(near(balanced.primaryFractions[0] * syntonicComma(), expectedShare)
        && near(balanced.pythagoreanFractions[0] * pythagoreanComma(), expectedShare)
        && near(balanced.pythagoreanFractions[1] * pythagoreanComma(), expectedShare), "Mixed S/P automatic fields have equal cent contributions");
    check(balanced.primaryFractions[2] == 10.0 / 11 && balanced.pythagoreanFractions[6] == -0.25, "Mixed fixed contributions stay untouched");
    check(calculate(Mode::syntonicFifths, { std::vector<double>(balanced.fractions.begin(), balanced.fractions.end()) }).valid, "Mixed automatic fields close without quantisation");
    automatic = {};
    for (auto& fifth : automatic) fifth.automaticPrimary = fifth.automaticPythagorean = true;
    balanced = resolveAutomatic(Mode::syntonicFifths, automatic);
    check(balanced.valid && balanced.automaticFields == 24 && near(balanced.centsPerField, -pythagoreanComma() / 24), "All 24 paired fields share the correction");
    for (const auto value : balanced.fractions) check(near(value, equalFraction(Mode::syntonicFifths)), "Two Auto contributions per fifth give equal temperament");
    automatic = {};
    balanced = resolveAutomatic(Mode::pythagoreanFifths, automatic);
    check(balanced.valid && balanced.automaticFields == 0, "No automatic fields leaves explicit corrections alone");
    check(!calculate(Mode::pythagoreanFifths, { std::vector<double>(balanced.fractions.begin(), balanced.fractions.end()) }).valid, "No Auto cannot silently fix an open circle");
    automatic[0].primary = std::numeric_limits<double>::quiet_NaN();
    check(!resolveAutomatic(Mode::pythagoreanFifths, automatic).valid, "Reject nonfinite fixed corrections");
    automatic[0].primary = 1001;
    check(!resolveAutomatic(Mode::pythagoreanFifths, automatic).valid, "Reject out-of-range fixed corrections");
    for (auto& fifth : automatic) fifth.primary = 1000;
    automatic[0].automaticPrimary = true;
    check(!resolveAutomatic(Mode::pythagoreanFifths, automatic).valid, "Reject automatic correction beyond supported range");
    const auto quarterP = automaticRatios(-pythagoreanComma() / 4, Mode::syntonicFifths);
    check(quarterP.closest == Mode::pythagoreanFifths && quarterP.pythagorean.fraction == "-1/4" && quarterP.pythagorean.exact,
          "Auto recognises an exact Pythagorean quarter even in the syntonic column");
    check(quarterP.syntonic.fraction == "-3/11" && !quarterP.syntonic.exact, "Auto also shows the nearby syntonic ratio as approximate");
    const auto quarterS = automaticRatios(-syntonicComma() / 4, Mode::pythagoreanFifths);
    check(quarterS.closest == Mode::syntonicFifths && quarterS.syntonic.fraction == "-1/4" && quarterS.syntonic.exact,
          "Auto recognises an exact syntonic quarter in either mode");
    const auto eighthP = automaticRatios(pythagoreanComma() / 8, Mode::syntonicFifths);
    check(eighthP.pythagorean.fraction == "1/8" && eighthP.pythagorean.exact && eighthP.closest == Mode::pythagoreanFifths,
          "Display supports useful Auto fractions beyond the preset menu");
    const auto twentyFourth = automaticRatios(-pythagoreanComma() / 24, Mode::syntonicFifths);
    check(twentyFourth.pythagorean.fraction == "-1/24" && twentyFourth.pythagorean.exact, "Twenty-four Auto fields have an exact fraction display");
    const auto nearQuarter = automaticRatios(-pythagoreanComma() / 4 + 0.0002, Mode::syntonicFifths);
    check(nearQuarter.closest == Mode::pythagoreanFifths && !nearQuarter.pythagorean.exact
        && near(nearQuarter.pythagorean.errorCents, -0.0002), "Approximate ratios retain a signed cent error and are not marked exact");
    for (const auto mode : { Mode::pythagoreanFifths, Mode::syntonicFifths })
    {
        const auto zero = automaticRatios(0, mode);
        check(zero.closest == mode && zero.syntonic.fraction == "0" && zero.pythagorean.fraction == "0"
            && zero.syntonic.exact && zero.pythagorean.exact, "Zero has exact reduced fractions and a stable tie break");
    }
    for (const double correction : { -12.382123, 37.6, -21.2, 0.163, -pythagoreanComma() / 11 })
    {
        const auto ratios = automaticRatios(correction, Mode::pythagoreanFifths);
        const auto& best = ratios.closest == Mode::syntonicFifths ? ratios.syntonic : ratios.pythagorean;
        check(std::abs(best.errorCents) <= std::abs(ratios.syntonic.errorCents) + 1e-10
            && std::abs(best.errorCents) <= std::abs(ratios.pythagorean.errorCents) + 1e-10, "Best comma is chosen by physical cent error");
        for (const auto& ratio : { ratios.syntonic, ratios.pythagorean })
        {
            const double displayedCents = evaluated(ratio.fraction, ratio.unit) * commaSize(ratio.unit);
            check(near(displayedCents - correction, ratio.errorCents), "Displayed fraction and approximation error agree");
            check(ratio.denominator >= 1 && ratio.denominator <= 24, "Readable fraction denominators are bounded");
            for (int denominator = 1; denominator <= 24; ++denominator)
            {
                const double rawNumerator = correction / commaSize(ratio.unit) * denominator;
                for (const double numerator : { std::floor(rawNumerator), std::ceil(rawNumerator) })
                    check(std::abs(ratio.errorCents) <= std::abs(numerator / denominator * commaSize(ratio.unit) - correction) + 1e-10,
                          "Displayed ratio is closest among all supported small denominators");
            }
        }
    }
    const auto harmonyEt = analyseHarmony({});
    check(harmonyEt.valid, "Equal temperament has a harmony analysis");
    for (int root = 0; root < 12; ++root)
    {
        const auto& fifth = harmonyEt.intervals[0][root];
        const auto& major = harmonyEt.intervals[1][root];
        const auto& minor = harmonyEt.intervals[2][root];
        check(near(fifth.cents, 700) && near(fifth.errorCents, -1.955000865387, 1e-9), "ET fifth error from pure 3:2");
        check(near(major.cents, 400) && near(major.errorCents, 13.686286135165, 1e-9), "ET major third error from pure 5:4");
        check(near(minor.cents, 300) && near(minor.errorCents, -15.641287000553, 1e-9), "ET minor third error from pure 6:5");
        check(consonance(fifth) == Consonance::good && consonance(major) == Consonance::tempered
            && consonance(minor) == Consonance::tempered, "Separate default limits classify ET intervals");
        for (int minorChord = 0; minorChord < 2; ++minorChord)
        {
            const auto& chord = harmonyEt.triads[root * 2 + minorChord];
            check(chord.root == root && chord.minor == (minorChord != 0), "All 24 triads have the right root and type");
            check(chord.notes[1] == (root + (minorChord ? 3 : 4)) % 12 && chord.notes[2] == (root + 7) % 12,
                  "Major and minor triads connect their correct notes, including octave wrap");
            check(near(chord.worstError, std::abs(minor.errorCents)), "Chord colour retains its worst interval");
        }
    }
    std::array<double, 12> pureTriad {};
    pureTriad[4] = harmonyPureCents(HarmonyInterval::majorThird) - 400;
    pureTriad[7] = harmonyPureCents(HarmonyInterval::fifth) - 700;
    const auto pureMajor = analyseHarmony(pureTriad);
    check(pureMajor.valid && near(pureMajor.triads[0].worstError, 0), "A pure C major triad has three pure intervals");
    pureTriad[3] = harmonyPureCents(HarmonyInterval::minorThird) - 300;
    const auto pureMinor = analyseHarmony(pureTriad);
    check(near(pureMinor.triads[1].worstError, 0), "A pure C minor triad has three pure intervals");
    auto transposedHarmony = pureTriad;
    for (auto& value : transposedHarmony) value += 27.325;
    const auto shiftedHarmony = analyseHarmony(transposedHarmony);
    for (size_t kind = 0; kind < 3; ++kind)
        for (size_t note = 0; note < 12; ++note)
            check(near(shiftedHarmony.intervals[kind][note].errorCents, pureMinor.intervals[kind][note].errorCents),
                  "Harmony colours do not depend on the reference pitch");
    const auto wolfHarmony = analyseHarmony(pythagorean.cents);
    check(near(wolfHarmony.intervals[0][6].errorCents, -pythagoreanComma()), "Harmony identifies the actual F# to Db wolf fifth");
    check(consonance(wolfHarmony.intervals[0][6]) == Consonance::rough
        && consonance(wolfHarmony.triads[12]) == Consonance::rough, "Wolf interval and its triad are red");
    check(near(wolfHarmony.intervals[1][0].errorCents, syntonicComma()), "Pythagorean C major third is one syntonic comma wide");
    for(double deviation:{-schisma(),-pythagoreanComma()/12,schisma(),pythagoreanComma()/12,-2.0,2.0})
    {
        IntervalAnalysis fifth;fifth.kind=HarmonyInterval::fifth;fifth.errorCents=deviation;
        check(consonance(fifth)==Consonance::good,"Standard fifth sensitivity gives schisma, ET, and the inclusive 2-cent boundary the same green rating");
    }
    check(consonance(6) == Consonance::good && consonance(-6) == Consonance::good
        && consonance(18) == Consonance::tempered && consonance(-18.001) == Consonance::rough, "Colour limits are symmetric and include their endpoints");
    check(consonance(14, { 3, 12 }) == Consonance::rough && consonance(14, { 10, 25 }) == Consonance::tempered,
          "Sensitivity changes the visual judgement");
    auto badHarmony = pureTriad;
    badHarmony[9] = std::numeric_limits<double>::quiet_NaN();
    check(!analyseHarmony(badHarmony).valid, "Harmony rejects invalid chart values");
    for (int q = -4; q <= 4; ++q)
        for (int r = -3; r <= 3; ++r)
        {
            const int originNote = latticeNote(q, r, -3);
            check(latticeNote(q + 1, r, -3) == (originNote + 7) % 12, "Horizontal lattice edges are fifths");
            check(latticeNote(q, r + 1, -3) == (originNote + 4) % 12, "Up-right lattice edges are major thirds");
            check(latticeNote(q + 1, r - 1, -3) == (originNote + 3) % 12, "Down-right lattice edges are minor thirds");
        }
    auto sensitiveChord = harmonyEt.triads[0];
    sensitiveChord.intervals[0].errorCents = 7;
    sensitiveChord.intervals[1].errorCents = 12;
    sensitiveChord.intervals[2].errorCents = 0;
    sensitiveChord.worstError = 12;
    check(consonance(sensitiveChord.intervals[0]) == Consonance::rough
        && consonance(sensitiveChord.intervals[1]) == Consonance::tempered, "Fifths are judged more strictly than thirds");
    check(consonance(sensitiveChord) == Consonance::rough, "A red fifth outweighs a larger orange third when rating a chord");
    HarmonyColourLimits independentLimits;
    independentLimits.thirds = { 12, 30 };
    check(consonance(sensitiveChord) == consonance(sensitiveChord, independentLimits), "Relaxing thirds cannot hide a rough fifth");
    independentLimits.fifths = { 4, 10 };
    check(consonance(sensitiveChord, independentLimits) == Consonance::tempered, "Fifth sensitivity controls the limiting fifth independently");
    sensitiveChord.intervals[0].errorCents = 2;
    sensitiveChord.intervals[1].errorCents = 8;
    check(consonance(sensitiveChord) == Consonance::good, "Each interval type includes its own green boundary");
    sensitiveChord.intervals[0].errorCents = -2.001;
    check(consonance(sensitiveChord) == Consonance::tempered, "Small excess over the fifth boundary makes the chord orange");
    const auto aMajorNames = harmonyNoteNames(9, false);
    check(aMajorNames[9] == "A" && aMajorNames[1] == "C#" && aMajorNames[4] == "E", "A major is spelled A C# E");
    const auto bbMinorNames = harmonyNoteNames(10, true);
    check(bbMinorNames[10] == "Bb" && bbMinorNames[1] == "Db" && bbMinorNames[5] == "F", "Bb minor is spelled Bb Db F");
    check(harmonyNoteNames(6, false)[5] == "E#", "F# major keeps its diatonic E# spelling");
    check(harmonyNoteNames(3, true)[11] == "Cb", "Eb minor keeps its diatonic Cb spelling");
    check(harmonyRootName(1, true) == "C#" && harmonyRootName(8, true) == "G#", "Minor chord names use conventional sharp roots");
    const std::string letters = "CDEFGAB";
    const int naturalPitch[] { 0, 2, 4, 5, 7, 9, 11 };
    for (int root = 0; root < 12; ++root)
        for (bool minor : { false, true })
        {
            const auto names = harmonyNoteNames(root, minor);
            for (size_t pitch = 0; pitch < names.size(); ++pitch)
            {
                const auto& name = names[pitch];
                const auto letter = letters.find(name.front());
                check(letter != std::string::npos, "Contextual note starts with a musical letter");
                int value = naturalPitch[letter];
                for (size_t i = 1; i < name.size(); ++i)
                {
                    check(name[i] == '#' || name[i] == 'b', "Only accidentals follow a note letter");
                    value += name[i] == '#' ? 1 : -1;
                }
                check((value + 12) % 12 == static_cast<int>(pitch), "Enharmonic spelling preserves the tuned pitch class");
            }
            const auto& chord = harmonyEt.triads[root * 2 + (minor ? 1 : 0)];
            const auto rootLetter = letters.find(names[static_cast<size_t>(root)].front());
            check(letters.find(names[static_cast<size_t>(chord.notes[1])].front()) == (rootLetter + 2) % 7,
                  "Triad thirds use the third letter in their own key");
            check(letters.find(names[static_cast<size_t>(chord.notes[2])].front()) == (rootLetter + 4) % 7,
                  "Triad fifths use the fifth letter in their own key");
        }
    std::cout << "Passed " << checks << " temperament checks.\n";
}
