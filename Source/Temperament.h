// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once

#include <array>
#include <string>
#include <vector>

namespace temperament
{
enum class Mode { pythagoreanFifths, syntonicFifths };

struct Cycle
{
    std::vector<int> notes;
    int closingEdge;
};

struct Calculation
{
    bool valid = false;
    std::array<double, 12> cents {};
    std::vector<double> closureErrors;
    std::string error;
};

struct SimpleFraction
{
    const char* text;
    double value;
};

struct ReverseCalculation
{
    bool valid = false;
    std::array<double, 12> cents {};
    std::array<double, 12> intervalErrors {};
    std::array<std::string, 12> expressions;
    // Legacy core compatibility: always zero; the GUI uses expressions only.
    std::array<std::string, 12> pythagoreanExpressions;
    double closureError = 0;
    std::string error;
};

enum class Reconstruction { nearestFraction, precise };
const std::array<SimpleFraction, 17>& simpleFractions();
size_t nearestSimpleFraction(double value);
ReverseCalculation reverseCsv(const std::string& line, Mode mode = Mode::pythagoreanFifths,
                              Reconstruction = Reconstruction::nearestFraction);

struct CommaComponents { double p = 0, s = 0; };
bool expressionComponents(const std::string&, Mode, CommaComponents&);
std::string commaExpression(CommaComponents);

enum class ClosingMethod { automaticFields, selectedEqually, allEqually, simpleFit };
struct ClosingRequest
{
    Mode mode = Mode::pythagoreanFifths;
    ClosingMethod method = ClosingMethod::selectedEqually;
    std::array<std::string, 12> expressions;
    std::array<bool, 12> selected {};
    bool useTargets = false; // Unrounded fifth corrections from imported CSV, in cents.
    std::array<double, 12> targets {};
    double maximumChange = 0.1; // Simple-fit limit for each selected fifth, in cents.
};
struct ClosingProposal
{
    bool valid = false;
    std::array<std::string, 12> expressions;
    std::array<double, 12> corrections {}, changes {};
    double closureBefore = 0, closureAfter = 0, maximumChange = 0;
    std::string message;
};
ClosingProposal proposeClosing(const ClosingRequest&);

struct ExpressionChoice { std::string text; double value; };
ExpressionChoice preciseExpression(double cents, Mode);
std::vector<ExpressionChoice> expressionChoices(Mode);
std::vector<ExpressionChoice> reconstructionChoices(Mode);
bool evaluateExpression(const std::string&, Mode, double& value, std::string& error);
// Preserve the physical correction when showing a formula in the other tab.
// An empty result denotes an invalid expression; Auto remains Auto.
std::string expressionInMode(const std::string&, Mode from, Mode to);
std::string fractionExpression(double value, Mode);
struct CommaExpressions { std::string syntonic, pythagorean; };
// Prefer a native menu fraction in its own column; never round the correction.
CommaExpressions pairedExpressions(double syntonicFraction);
bool isAutomaticExpression(const std::string&);

struct FifthCorrection
{
    double primary = 0, pythagorean = 0;
    bool automaticPrimary = false, automaticPythagorean = false;
};
struct CommaRatio
{
    std::string fraction;
    Mode unit = Mode::pythagoreanFifths;
    double errorCents = 0; // displayed fraction minus actual correction
    int denominator = 1;
    bool exact = false;
};
struct AutomaticRatios
{
    CommaRatio syntonic, pythagorean;
    Mode closest = Mode::pythagoreanFifths;
};
// Display estimates: reduced fractions with denominators up to 24, compared in cents.
AutomaticRatios automaticRatios(double cents, Mode preferred);
struct AutomaticCalculation
{
    bool valid = false;
    std::array<double, 12> fractions {}, primaryFractions {}, pythagoreanFractions {};
    int automaticFields = 0;
    double centsPerField = 0;
    AutomaticRatios ratios;
    std::string error;
};
// Every automatic field receives the same physical cent correction, in its own comma unit.
AutomaticCalculation resolveAutomatic(Mode, const std::array<FifthCorrection, 12>&);

double pythagoreanComma();
double syntonicComma();
double schisma();
double commaSize(Mode);
double pureInterval(Mode);
double equalInterval(Mode);
double equalFraction(Mode);
const std::vector<Cycle>& cycles(Mode);
const std::array<std::string, 12>& noteNames();
bool parseNumber(const std::string&, double& result);
double closingFraction(Mode, const std::vector<double>&, int closingEdge);
Calculation calculate(Mode, const std::vector<std::vector<double>>& fractions);
std::string formatCents(double, bool showPositiveSign = false);
std::string csv(const std::array<double, 12>&);
// Positive steps move C-G's correction to G-D; A is normalised back to zero.
std::array<double, 12> rotateChartFifths(const std::array<double, 12>&, int steps);
}
