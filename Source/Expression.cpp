// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "Temperament.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <locale>
#include <numeric>
#include <sstream>

namespace temperament
{
namespace
{
class Parser
{
public:
    Parser(std::string input, Mode selected) : source(std::move(input)), mode(selected)
    {
        std::replace(source.begin(), source.end(), ',', '.');
    }
    bool run(double& value, std::string& error)
    {
        if (source.size() > 512) failure = "Expression is too long (maximum 512 characters).";
        if (failure.empty()) value = sum(0);
        whitespace();
        if (failure.empty() && position != source.size()) failure = "Unexpected text. Use +, -, *, / and parentheses.";
        if (failure.empty() && !std::isfinite(value)) failure = "The result must be finite.";
        error = failure;
        return failure.empty();
    }
private:
    std::string source, failure;
    Mode mode;
    size_t position = 0;
    void whitespace() { while (position < source.size() && std::isspace(static_cast<unsigned char>(source[position]))) ++position; }
    bool consume(char c)
    {
        whitespace();
        if (position < source.size() && source[position] == c) { ++position; return true; }
        return false;
    }
    double checked(double value)
    {
        if (!std::isfinite(value) && failure.empty()) failure = "Arithmetic overflow or division by zero.";
        return value;
    }
    double sum(int depth)
    {
        double value = product(depth);
        while (failure.empty())
        {
            if (consume('+')) value = checked(value + product(depth));
            else if (consume('-')) value = checked(value - product(depth));
            else break;
        }
        return value;
    }
    double product(int depth)
    {
        double value = atom(depth);
        while (failure.empty())
        {
            if (consume('*')) value = checked(value * atom(depth));
            else if (consume('/'))
            {
                const double divisor = atom(depth);
                if (divisor == 0) { failure = "Division by zero."; return 0; }
                value = checked(value / divisor);
            }
            else break;
        }
        return value;
    }
    double atom(int depth)
    {
        if (depth > 32 || !failure.empty()) { failure = "Expression nesting is too deep."; return 0; }
        if (consume('+')) return atom(depth + 1);
        if (consume('-')) return -atom(depth + 1);
        if (consume('('))
        {
            const double value = sum(depth + 1);
            if (!consume(')') && failure.empty()) failure = "Missing closing parenthesis.";
            return value;
        }
        whitespace();
        if (position == source.size()) { failure = "Expected a number or comma name."; return 0; }
        if (std::isalpha(static_cast<unsigned char>(source[position])))
        {
            std::string name;
            while (position < source.size() && std::isalpha(static_cast<unsigned char>(source[position])))
                name += static_cast<char>(std::tolower(static_cast<unsigned char>(source[position++])));
            if (name == "h" || name == "schisma") return schisma() / commaSize(mode);
            if (name == "s" || name == "syntonic" || name == "diatonic") return syntonicComma() / commaSize(mode);
            if (name == "p" || name == "pythagorean" || name == "ditonic") return pythagoreanComma() / commaSize(mode);
            if (name == "et") return equalFraction(mode);
            failure = "Unknown name: " + name + ". Use schisma (H), syntonic (S), Pythagorean (P), or ET.";
            return 0;
        }
        const size_t start = position;
        while (position < source.size() && (std::isdigit(static_cast<unsigned char>(source[position])) || source[position] == '.')) ++position;
        if (position < source.size() && (source[position] == 'e' || source[position] == 'E'))
        {
            ++position;
            if (position < source.size() && (source[position] == '+' || source[position] == '-')) ++position;
            while (position < source.size() && std::isdigit(static_cast<unsigned char>(source[position]))) ++position;
        }
        double value = 0;
        if (start == position || !parseNumber(source.substr(start, position - start), value)) failure = "Invalid number.";
        return value;
    }
};

std::string rational(int numerator, int denominator)
{
    const int divisor = std::gcd(std::abs(numerator), denominator);
    numerator /= divisor; denominator /= divisor;
    return std::to_string(numerator) + (denominator == 1 ? "" : "/" + std::to_string(denominator));
}
}

bool evaluateExpression(const std::string& input, Mode mode, double& value, std::string& error)
{
    value = 0;
    return Parser(input, mode).run(value, error);
}

bool isAutomaticExpression(const std::string& input)
{
    std::string name;
    for (const auto c : input)
        if (!std::isspace(static_cast<unsigned char>(c)))
            name += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return name == "auto" || name == "automaticcalculation";
}

std::vector<ExpressionChoice> expressionChoices(Mode mode)
{
    std::vector<ExpressionChoice> choices;
    for (const auto& fraction : simpleFractions()) choices.push_back({ fraction.text, fraction.value });
    const auto add = [&](std::string expression) {
        double value = 0;
        std::string error;
        if (evaluateExpression(expression, mode, value, error)) choices.push_back({ std::move(expression), value });
    };
    // Other-comma fractions are written as short, exact schisma expressions.
    // In S units P = 1 + H; in P units S = 1 - H.
    for (int denominator : { 1, 2, 3, 4, 5, 6, 12 })
        for (int sign : { 1, -1 })
        {
            const std::string h = denominator == 1 ? "H" : "H/" + std::to_string(denominator);
            const bool addH = (mode == Mode::syntonicFifths) == (sign > 0);
            add(rational(sign, denominator) + (addH ? "+" : "-") + h);
            add((sign < 0 ? "-" : "") + h);
        }
    return choices;
}

CommaExpressions pairedExpressions(double syntonicFraction)
{
    const double cents = syntonicFraction * syntonicComma();
    for (const auto& fraction : simpleFractions())
        if (std::abs(cents - fraction.value * syntonicComma()) < 1e-7)
            return { fraction.text, "0" };
    for (const auto& fraction : simpleFractions())
        if (std::abs(cents - fraction.value * pythagoreanComma()) < 1e-7)
            return { "0", fraction.text };
    return { fractionExpression(syntonicFraction, Mode::syntonicFifths), "0" };
}

std::string fractionExpression(double value, Mode mode)
{
    for (const auto& choice : expressionChoices(mode))
        if (std::abs(value - choice.value) * commaSize(mode) < 0.0000001) return choice.text;
    // Preserve simple, non-menu fractions when automatically closing a circle.
    for (int denominator = 1; denominator <= 144; ++denominator)
    {
        const int numerator = static_cast<int>(std::round(value * denominator));
        if (std::abs(value - static_cast<double>(numerator) / denominator) * commaSize(mode) < 0.0000001)
            return rational(numerator, denominator);
    }
    // Mixed comma input commonly closes as a rational amount plus a schisma fraction.
    std::string shortest;
    for (int hDenominator : { 1, 2, 3, 4, 5, 6, 11, 12 })
        for (int hNumerator = -12; hNumerator <= 12; ++hNumerator)
        {
            if (hNumerator == 0 || std::gcd(std::abs(hNumerator), hDenominator) != 1) continue;
            const double remainder = value - static_cast<double>(hNumerator) / hDenominator * schisma() / commaSize(mode);
            for (int denominator = 1; denominator <= 144; ++denominator)
            {
                const int numerator = static_cast<int>(std::round(remainder * denominator));
                if (std::abs(remainder - static_cast<double>(numerator) / denominator) * commaSize(mode) >= 0.0000001) continue;
                const auto h = (std::abs(hNumerator) == 1 ? std::string("H") : std::to_string(std::abs(hNumerator)) + "*H")
                    + (hDenominator == 1 ? "" : "/" + std::to_string(hDenominator));
                const auto expression = rational(numerator, denominator) + (hNumerator < 0 ? "-" : "+") + h;
                if (shortest.empty() || expression.size() < shortest.size()) shortest = expression;
                break;
            }
        }
    if (!shortest.empty()) return shortest;
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(12) << value;
    auto result = stream.str();
    result.erase(result.find_last_not_of('0') + 1);
    if (result.back() == '.') result.pop_back();
    return result;
}
}
