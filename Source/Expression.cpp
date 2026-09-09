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
// Keep the symbolic coefficients alongside the numerical result. Nonlinear
// expressions still evaluate normally, but are never mistaken for comma sums.
struct Value
{
    double number = 0, constant = 0, other = 0;
    bool linear = true;
    Value(double n = 0) : number(n), constant(n) {}
    Value(double n, double c, double o, bool l = true) : number(n), constant(c), other(o), linear(l) {}
};
Value operator+(Value a, Value b) { return { a.number+b.number, a.constant+b.constant, a.other+b.other, a.linear && b.linear }; }
Value operator-(Value a, Value b) { return { a.number-b.number, a.constant-b.constant, a.other-b.other, a.linear && b.linear }; }
Value operator-(Value a) { return { -a.number, -a.constant, -a.other, a.linear }; }
Value operator*(Value a, Value b)
{
    return { a.number*b.number, a.constant*b.constant, a.constant*b.other+a.other*b.constant,
             a.linear && b.linear && (a.other == 0 || b.other == 0) };
}
Value operator/(Value a, Value b)
{
    const bool linear = a.linear && b.linear && b.other == 0 && b.constant != 0;
    return { a.number/b.number, linear ? a.constant/b.constant : 0,
             linear ? a.other/b.constant : 0, linear };
}
class Parser
{
public:
    Parser(std::string input, Mode selected) : source(std::move(input)), mode(selected)
    {
        std::replace(source.begin(), source.end(), ',', '.');
    }
    bool run(double& value, std::string& error, CommaComponents* components = nullptr)
    {
        if (source.size() > 512) failure = "Expression is too long (maximum 512 characters).";
        Value parsed;
        if (failure.empty()) parsed = sum(0);
        value = parsed.number;
        whitespace();
        if (failure.empty() && position != source.size()) failure = "Unexpected text. Use +, -, *, / and parentheses.";
        if (failure.empty() && !std::isfinite(value)) failure = "The result must be finite.";
        error = failure;
        if (components != nullptr)
        {
            if (!parsed.linear) return false;
            *components = mode == Mode::pythagoreanFifths
                ? CommaComponents { parsed.constant, parsed.other } : CommaComponents { parsed.other, parsed.constant };
        }
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
    Value checked(Value value)
    {
        if (!std::isfinite(value.number) && failure.empty()) failure = "Arithmetic overflow or division by zero.";
        return value;
    }
    Value sum(int depth)
    {
        Value value = product(depth);
        while (failure.empty())
        {
            if (consume('+')) value = checked(value + product(depth));
            else if (consume('-')) value = checked(value - product(depth));
            else break;
        }
        return value;
    }
    Value product(int depth)
    {
        Value value = atom(depth);
        while (failure.empty())
        {
            if (consume('*')) value = checked(value * atom(depth));
            else if (consume('/'))
            {
                const auto divisor = atom(depth);
                if (divisor.number == 0) { failure = "Division by zero."; return 0; }
                value = checked(value / divisor);
            }
            else break;
        }
        return value;
    }
    Value atom(int depth)
    {
        if (depth > 32 || !failure.empty()) { failure = "Expression nesting is too deep."; return 0; }
        if (consume('+')) return atom(depth + 1);
        if (consume('-')) return -atom(depth + 1);
        if (consume('('))
        {
            const auto value = sum(depth + 1);
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
            const bool pMode = mode == Mode::pythagoreanFifths;
            const Value p { pythagoreanComma()/commaSize(mode), pMode ? 1.0 : 0.0, pMode ? 0.0 : 1.0 };
            const Value s { syntonicComma()/commaSize(mode), pMode ? 0.0 : 1.0, pMode ? 1.0 : 0.0 };
            if (name == "h" || name == "schisma") return p - s;
            if (name == "s" || name == "syntonic" || name == "diatonic") return s;
            if (name == "p" || name == "pythagorean" || name == "ditonic") return p;
            if (name == "et") return -p / Value(12);
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

bool expressionComponents(const std::string& input, Mode mode, CommaComponents& components)
{
    double value = 0;
    std::string error;
    return Parser(input, mode).run(value, error, &components);
}

std::string expressionInMode(const std::string& input, Mode from, Mode to)
{
    if (isAutomaticExpression(input)) return "Auto";
    double original = 0, translated = 0;
    std::string error;
    if (!evaluateExpression(input, from, original, error)) return {};
    const double physical = original * commaSize(from);
    if (from == to || (evaluateExpression(input, to, translated, error)
        && std::abs(translated * commaSize(to) - physical) < 1e-10)) return input;
    CommaComponents components;
    if (expressionComponents(input, from, components))
    {
        const auto expression = commaExpression(components);
        if (evaluateExpression(expression, to, translated, error)
            && std::abs(translated * commaSize(to) - physical) < 1e-9) return expression;
    }
    // Unusual nonlinear expressions are preserved numerically, without rounding
    // to another menu fraction. The source tab keeps the user's original formula.
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(17) << physical / commaSize(to);
    return stream.str();
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
    if (mode == Mode::pythagoreanFifths)
        for (const auto& fraction : simpleFractions()) choices.push_back({ fraction.text, fraction.value });
    else choices.push_back({ "0", 0 });
    const auto add = [&](std::string expression) {
        double value = 0;
        std::string error;
        if (evaluateExpression(expression, mode, value, error)) choices.push_back({ std::move(expression), value });
    };
    for (const std::string unit : { "P", "S", "schisma" })
    for (int denominator : { 1, 2, 3, 4, 5, 6, 12, 24 })
        for (int sign : { 1, -1 })
        {
            add((sign < 0 ? "-" : "") + unit + (denominator == 1 ? "" : "/" + std::to_string(denominator)));
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

std::vector<ExpressionChoice> reconstructionChoices(Mode mode)
{
    auto result=expressionChoices(mode);
    for(const std::string unit:{"P","S","schisma"})
        for(int denominator:{1,2,3,4,5,6,12,24})
            for(int numerator:{-4,-3,-2,2,3,4})
            {
                const int divisor=std::gcd(std::abs(numerator),denominator);
                const int n=numerator/divisor,d=denominator/divisor;
                const std::string text=std::string(n<0?"-":"")+unit+(std::abs(n)==1?"":"*"+std::to_string(std::abs(n)))
                    +(d==1?"":"/"+std::to_string(d));
                double value=0;std::string error;
                if(evaluateExpression(text,mode,value,error)) result.push_back({text,value});
            }
    return result;
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
                const auto h = std::string("schisma") + (std::abs(hNumerator) == 1 ? "" : "*" + std::to_string(std::abs(hNumerator)))
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
