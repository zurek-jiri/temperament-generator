// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "MainComponent.h"
#include "UiFonts.h"
#include <cmath>

namespace
{
const juce::Colour background(0xff10171f), field(0xff202f3e), border(0xff506375);
const juce::Colour ink(0xfff5f8fa), muted(0xffc4d1dc), mint(0xff87e3c4), amber(0xffedbe7b), red(0xfff18f8d);
juce::Font font(float size, bool bold = false)
{
    return juce::Font(juce::FontOptions(ui::sansFont(), size, bold ? juce::Font::bold : juce::Font::plain));
}
void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds,
          float size, juce::Colour colour = ink, juce::Justification align = juce::Justification::centred, bool bold = false)
{
    g.setColour(colour); g.setFont(font(size, bold)); g.drawText(value, bounds, align, true);
}
juce::String inputText(const juce::ComboBox& box)
{
    for (auto* child : box.getChildren())
        if (auto* label = dynamic_cast<juce::Label*>(child)) return label->getText(true);
    return box.getText();
}
juce::String cents(double value, bool sign = false) { return temperament::formatCents(value, sign); }
juce::String ratioText(const temperament::CommaRatio& ratio)
{
    return (ratio.exact ? juce::String("= ") : juce::String::charToString(0x2248) + " ") + juce::String(ratio.fraction);
}
juce::String ratioName(temperament::Mode unit)
{
    return unit == temperament::Mode::syntonicFifths ? "Syntonic / diatonic" : "Pythagorean / ditonic";
}
juce::String ratioDetails(const temperament::CommaRatio& ratio)
{
    return ratioName(ratio.unit) + ": " + ratioText(ratio)
        + (ratio.exact ? " (exact). " : " (fraction minus actual: " + juce::String(ratio.errorCents, 6) + " cents). ");
}
}

CircleEditor::CircleEditor(temperament::Mode selectedMode, size_t index, std::function<void()> changed)
    : mode(selectedMode), ringIndex(index), onChange(std::move(changed))
{
    const auto& ring = temperament::cycles(mode)[ringIndex];
    importedTexts.resize(ring.notes.size());
    roundingErrors.resize(ring.notes.size(), 0);
    auto makeEditor = [this](temperament::Mode unit, const juce::String& title, size_t i) {
        auto box = std::make_unique<juce::ComboBox>();
        box->setEditableText(true);
        box->setJustificationType(juce::Justification::centred);
        box->addItem("Automatic calculation", automaticItemId);
        box->addSeparator();
        int item = 1;
        for (const auto& choice : temperament::expressionChoices(unit)) box->addItem(choice.text, item++);
        box->addItem("ET", item);
        box->setTitle(title);
        box->setTooltip(title + ". Examples: -1/4, 1-1/11, -1-schisma, -H/2. H = schisma, S = syntonic, P = Pythagorean. Use + - * / and parentheses.");
        box->onChange = [this, i, input = box.get()] {
            if (temperament::isAutomaticExpression(inputText(*input).toStdString()))
                input->setText("Auto", juce::dontSendNotification);
            refreshRounding(i);
            onChange();
        };
        for (auto* child : box->getChildren())
            if (auto* label = dynamic_cast<juce::Label*>(child))
                label->onEditorShow = [this, label, i] {
                    if (auto* input = label->getCurrentTextEditor())
                        input->onTextChange = [this, i] { refreshRounding(i); onChange(); };
                };
        addAndMakeVisible(*box);
        return box;
    };
    for (size_t i = 0; i < ring.notes.size(); ++i)
    {
        const auto interval = noteLabel(ring.notes[i]) + " -> " + noteLabel(ring.notes[(i + 1) % ring.notes.size()]);
        auto box = makeEditor(mode, interval + (mode == temperament::Mode::syntonicFifths ? " / syntonic contribution" : " / Pythagorean correction"), i);
        defaultTooltips.push_back(box->getTooltip());
        editors.push_back(std::move(box));
        if (mode == temperament::Mode::syntonicFifths)
        {
            secondaryEditors.push_back(makeEditor(temperament::Mode::pythagoreanFifths, interval + " / Pythagorean contribution (added to syntonic)", i));
            secondaryDefaultTooltips.push_back(secondaryEditors.back()->getTooltip());
        }
    }
    setEqual();
}

juce::String CircleEditor::noteLabel(int note) const
{
    static const std::array<const char*, 12> names { "C", "Db/C#", "D", "Eb/D#", "E", "F", "F#/Gb", "G", "Ab/G#", "A", "Bb", "B" };
    return names[static_cast<size_t>(note)];
}
juce::Point<float> CircleEditor::centre() const { return { 426, 274 }; }
juce::Point<float> CircleEditor::point(float index, bool forEditor) const
{
    const float angle = juce::MathConstants<float>::twoPi * index / 12;
    return centre() + juce::Point<float>(std::sin(angle) * (forEditor ? 190.0f : 250.0f),
                                       -std::cos(angle) * (forEditor ? 180.0f : 250.0f));
}
void CircleEditor::resized()
{
    for (size_t i = 0; i < editors.size(); ++i)
    {
        if (mode == temperament::Mode::pythagoreanFifths)
            editors[i]->setBounds(juce::Rectangle<int>(96, 44).withCentre(point(static_cast<float>(i) + 0.5f, true).roundToInt()));
        else
        {
            editors[i]->setBounds(1080, 60 + static_cast<int>(i) * 40, 148, 38);
            secondaryEditors[i]->setBounds(1240, 60 + static_cast<int>(i) * 40, 148, 38);
        }
    }
}
void CircleEditor::paint(juce::Graphics& g)
{
    const bool paired = mode == temperament::Mode::syntonicFifths;
    const auto& ring = temperament::cycles(mode)[ringIndex];
    const auto c = centre();
    g.setColour(border);
    g.drawEllipse(c.x - 250, c.y - 250, 500, 500, 1.5f);
    g.setColour(border.withAlpha(0.35f));
    g.drawEllipse(c.x - 139, c.y - 139, 278, 278, 1);
    if (hasAutomatic())
    {
        text(g, automaticResult.valid ? "Auto: " + juce::String(automaticResult.automaticFields) + " fields" : "Auto: pending",
             { 288, 160, 276, 34 }, 26, mint, juce::Justification::centred, true);
        if (automaticResult.valid)
        {
            text(g, cents(automaticResult.centsPerField, true) + " ct each", { 288, 196, 276, 36 }, 30, ink);
            const auto& ratios = automaticResult.ratios;
            const auto syntonicColour = ratios.closest == temperament::Mode::syntonicFifths ? mint : muted;
            const auto pythagoreanColour = ratios.closest == temperament::Mode::pythagoreanFifths ? mint : muted;
            text(g, ratioText(ratios.syntonic) + " S", { 288, 238, 276, 36 }, 32, syntonicColour, juce::Justification::centred, true);
            text(g, "Syntonic / diatonic", { 288, 274, 276, 30 }, 24, syntonicColour);
            text(g, ratioText(ratios.pythagorean) + " P", { 288, 310, 276, 36 }, 32, pythagoreanColour, juce::Justification::centred, true);
            text(g, "Pythagorean", { 288, 346, 276, 30 }, 24, pythagoreanColour);
            text(g, "Green: closest", { 288, 380, 276, 30 }, 24, mint);
        }
        else text(g, "Calculate chart", { 288, 248, 276, 36 }, 28, muted);
    }
    else
    {
        text(g, paired ? "Syntonic" : "Pythagorean", { 288, 196, 276, 36 }, 28, mint, juce::Justification::centred, true);
        text(g, cents(temperament::commaSize(mode)), { 296, 239, 260, 55 }, 48, ink, juce::Justification::centred, true);
        text(g, "cents / comma", { 296, 301, 260, 34 }, 26, muted);
        text(g, "Clockwise fifths", { 288, 346, 276, 34 }, 24, muted);
    }
    for (size_t i = 0; i < ring.notes.size(); ++i)
    {
        const auto p = point(static_cast<float>(i), false);
        const auto middle = point(static_cast<float>(i) + 0.5f, true);
        const auto next = point(static_cast<float>(i + 1), false);
        g.setColour(border);
        g.drawLine({ p, middle }, 1); g.drawLine({ middle, next }, 1);
        const auto delta = next - middle;
        const auto direction = delta / delta.getDistanceFromOrigin();
        const auto tip = next - direction * 26.0f;
        const juce::Point<float> perpendicular(-direction.y, direction.x);
        juce::Path arrow;
        arrow.startNewSubPath(tip - direction * 6.0f + perpendicular * 3.0f);
        arrow.lineTo(tip); arrow.lineTo(tip - direction * 6.0f - perpendicular * 3.0f);
        g.setColour(muted); g.strokePath(arrow, juce::PathStrokeType(1.2f));
        if (paired)
        {
            const auto badge = juce::Rectangle<float>(42, 38).withCentre(middle);
            g.setColour(field); g.fillRoundedRectangle(badge, 8);
            text(g, juce::String(static_cast<int>(i + 1)), badge.toNearestInt(), 26, mint, juce::Justification::centred, true);
            static const std::array<const char*, 12> shortNames { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
            const auto row = juce::String(static_cast<int>(i + 1)) + "  " + shortNames[ring.notes[i]] + " > " + shortNames[ring.notes[(i + 1) % 12]];
            text(g, row, { 912, 60 + static_cast<int>(i) * 40, 164, 38 }, 24, ink, juce::Justification::centredLeft);
        }
    }
    for (size_t i = 0; i < ring.notes.size(); ++i)
    {
        const int note = ring.notes[i];
        auto p = point(static_cast<float>(i), false);
        const bool pair = note == 1 || note == 3 || note == 8;
        if (pair) p.x -= 32;
        const auto node = juce::Rectangle<float>(pair ? 120.0f : note == 6 ? 104.0f : 56.0f, 50).withCentre(p);
        g.setColour(note == 9 ? mint : field); g.fillRoundedRectangle(node, 15);
        g.setColour(note == 0 ? mint : border); g.drawRoundedRectangle(node, 15, 1);
        text(g, noteLabel(note), node.toNearestInt(), note == 6 ? 27.0f : 32.0f,
             note == 9 ? background : ink, juce::Justification::centred, true);
    }
    if (closureKnown)
        text(g, std::abs(residual) <= 0.000001 ? "Circle closed" : "Closure error: " + cents(residual, true) + " ct",
             { 8, 555, 836, 30 }, 24, std::abs(residual) <= 0.000001 ? mint : red);
}
void CircleEditor::setEqual()
{
    automaticResult = {};
    for (auto& value : importedTexts) value.clear();
    for (auto& box : editors) box->setText(mode == temperament::Mode::pythagoreanFifths ? "-1/12" : "0", juce::dontSendNotification);
    for (auto& box : secondaryEditors) box->setText("-1/12", juce::dontSendNotification);
    for (size_t i = 0; i < editors.size(); ++i) refreshRounding(i);
}
void CircleEditor::setPure()
{
    automaticResult = {};
    for (auto& value : importedTexts) value.clear();
    for (auto& box : editors) box->setText("0", juce::dontSendNotification);
    for (auto& box : secondaryEditors) box->setText("0", juce::dontSendNotification);
    for (size_t i = 0; i < editors.size(); ++i) refreshRounding(i);
}
bool CircleEditor::read(std::vector<double>& values, juce::String& error, bool skipClosing)
{
    values.clear();
    automaticResult = {};
    const auto& ring = temperament::cycles(mode)[ringIndex];
    const bool skip = skipClosing && !hasAutomatic();
    std::array<temperament::FifthCorrection, 12> corrections {};
    for (size_t i = 0; i < editors.size(); ++i)
    {
        if (skip && static_cast<int>(i) == ring.closingEdge) continue;
        for (int column = 0; column < (secondaryEditors.empty() ? 1 : 2); ++column)
        {
            auto& box = column == 0 ? *editors[i] : *secondaryEditors[i];
            const auto unit = column == 0 ? mode : temperament::Mode::pythagoreanFifths;
            const auto expression = inputText(box).toStdString();
            const bool automatic = temperament::isAutomaticExpression(expression);
            double value = 0;
            std::string parseError;
            if (!automatic && (!temperament::evaluateExpression(expression, unit, value, parseError) || std::abs(value) > 1000))
            {
                box.setColour(juce::ComboBox::outlineColourId, red);
                error = "Fifth " + juce::String(static_cast<int>(i + 1)) + ": "
                    + (parseError.empty() ? juce::String("Correction exceeds 1000 commas.") : juce::String(parseError));
                return false;
            }
            if (column == 0) { corrections[i].primary = value; corrections[i].automaticPrimary = automatic; }
            else { corrections[i].pythagorean = value; corrections[i].automaticPythagorean = automatic; }
        }
    }
    automaticResult = temperament::resolveAutomatic(mode, corrections);
    if (!automaticResult.valid) { error = automaticResult.error; return false; }
    values.assign(automaticResult.fractions.begin(), automaticResult.fractions.end());
    for (size_t i = 0; i < editors.size(); ++i) refreshRounding(i);
    return true;
}
bool CircleEditor::hasAutomatic() const
{
    for (const auto& box : editors)
        if (temperament::isAutomaticExpression(inputText(*box).toStdString())) return true;
    for (const auto& box : secondaryEditors)
        if (temperament::isAutomaticExpression(inputText(*box).toStdString())) return true;
    return false;
}
void CircleEditor::close(const std::vector<double>& values)
{
    const auto index = static_cast<size_t>(temperament::cycles(mode)[ringIndex].closingEdge);
    const double value = temperament::closingFraction(mode, values, static_cast<int>(index));
    if (secondaryEditors.empty())
        editors[index]->setText(temperament::fractionExpression(value, mode), juce::dontSendNotification);
    else
    {
        const auto columns = temperament::pairedExpressions(value);
        editors[index]->setText(columns.syntonic, juce::dontSendNotification);
        secondaryEditors[index]->setText(columns.pythagorean, juce::dontSendNotification);
    }
    refreshRounding(index);
    auto& closingBox = !secondaryEditors.empty() && inputText(*secondaryEditors[index]) != "0"
        ? *secondaryEditors[index] : *editors[index];
    closingBox.setColour(juce::ComboBox::outlineColourId, mint);
}
void CircleEditor::refreshRounding(size_t index)
{
    if (index >= editors.size()) return;
    const auto current = inputText(*editors[index]) + (secondaryEditors.empty() ? "" : "|" + inputText(*secondaryEditors[index]));
    if (current != importedTexts[index]) importedTexts[index].clear();
    const bool rounded = importedTexts[index].isNotEmpty() && std::abs(roundingErrors[index]) > 1e-7;
    const auto colour = std::abs(roundingErrors[index]) > 0.0010001 ? red : amber;
    const auto* roundedBox = !secondaryEditors.empty() && inputText(*secondaryEditors[index]) != "0"
        ? secondaryEditors[index].get() : editors[index].get();
    for (auto* box : { editors[index].get(), secondaryEditors.empty() ? nullptr : secondaryEditors[index].get() })
        if (box != nullptr)
        {
            const bool marked = rounded && box == roundedBox;
            const bool automatic = temperament::isAutomaticExpression(inputText(*box).toStdString());
            box->setColour(juce::ComboBox::textColourId, automatic ? mint : marked ? colour : ink);
            box->setColour(juce::ComboBox::outlineColourId, automatic ? mint : marked ? colour : border);
            box->setColour(juce::ComboBox::backgroundColourId, field);
            const auto& tooltip = box == editors[index].get() ? defaultTooltips[index] : secondaryDefaultTooltips[index];
            box->setTooltip(tooltip + " Current expression: " + inputText(*box)
                + (marked ? ". Rounded from CSV; interval difference " + juce::String(roundingErrors[index], 6) + " cents." : ""));
            if (automatic)
            {
                juce::String details = "Press Calculate chart or Close circles.";
                if (automaticResult.valid)
                {
                    const auto& ratios = automaticResult.ratios;
                    const auto& closest = ratios.closest == temperament::Mode::syntonicFifths ? ratios.syntonic : ratios.pythagorean;
                    details = "Calculated value: " + juce::String(automaticResult.centsPerField, 6) + " cents. Closest fraction: "
                        + ratioText(closest) + " " + ratioName(closest.unit) + ". "
                        + ratioDetails(ratios.syntonic) + ratioDetails(ratios.pythagorean)
                        + "Fractions use denominators up to 24. The chart uses the full-precision Auto value.";
                }
                box->setTooltip(box->getTitle() + ". Automatic calculation. " + details);
            }
        }
    repaint();
}
void CircleEditor::setFromCsv(const temperament::ReverseCalculation& imported)
{
    automaticResult = {};
    for (size_t i = 0; i < editors.size(); ++i)
    {
        editors[i]->setText(imported.expressions[i], juce::dontSendNotification);
        if (!secondaryEditors.empty()) secondaryEditors[i]->setText(imported.pythagoreanExpressions[i], juce::dontSendNotification);
        importedTexts[i] = inputText(*editors[i]) + (secondaryEditors.empty() ? "" : "|" + inputText(*secondaryEditors[i]));
        roundingErrors[i] = imported.intervalErrors[i];
        refreshRounding(i);
    }
    showClosure(imported.closureError);
}
void CircleEditor::showClosure(double error) { residual = error; closureKnown = true; repaint(); }
void CircleEditor::clearClosure()
{
    closureKnown = false;
    automaticResult = {};
    for (size_t i = 0; i < editors.size(); ++i) refreshRounding(i);
    repaint();
}
juce::String CircleEditor::signature() const
{
    juce::String signatureText;
    const auto append = [&](const auto& box) {
        const auto value = inputText(*box);
        signatureText += (temperament::isAutomaticExpression(value.toStdString()) ? "Auto" : value) + "|";
    };
    for (const auto& box : editors) append(box);
    for (const auto& box : secondaryEditors) append(box);
    return signatureText;
}
