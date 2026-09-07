// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Temperament.h"
#include "HarmonyView.h"

class StudioLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    StudioLookAndFeel();
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;
    juce::Font getPopupMenuFont() override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
    juce::Rectangle<int> getTooltipBounds(const juce::String&, juce::Point<int>, juce::Rectangle<int>) override;
    void drawTooltip(juce::Graphics&, const juce::String&, int, int) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
};

class CircleEditor final : public juce::Component
{
public:
    static constexpr int automaticItemId = 1000;
    CircleEditor(temperament::Mode, size_t index, std::function<void()> changed);
    void paint(juce::Graphics&) override;
    void resized() override;
    void setEqual();
    void setPure();
    void setFromCsv(const temperament::ReverseCalculation&);
    bool read(std::vector<double>&, juce::String& error, bool skipClosing = false);
    bool hasAutomatic() const;
    void close(const std::vector<double>&);
    void showClosure(double error);
    void clearClosure();
    juce::String signature() const;
    juce::ComboBox& editor(size_t index) { return *editors[index]; }
    juce::ComboBox& pythagoreanEditor(size_t index) { return *secondaryEditors[index]; }
private:
    temperament::Mode mode;
    size_t ringIndex;
    std::function<void()> onChange;
    std::vector<std::unique_ptr<juce::ComboBox>> editors;
    std::vector<std::unique_ptr<juce::ComboBox>> secondaryEditors;
    std::vector<juce::String> defaultTooltips;
    std::vector<juce::String> secondaryDefaultTooltips;
    std::vector<juce::String> importedTexts;
    std::vector<double> roundingErrors;
    temperament::AutomaticCalculation automaticResult;
    void refreshRounding(size_t);
    double residual = 0;
    bool closureKnown = false;
    juce::Point<float> centre() const;
    juce::Point<float> point(float index, bool forEditor) const;
    juce::String noteLabel(int note) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircleEditor)
};

class MainComponent final : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool renderPreviews(const juce::File& directory);
private:
    StudioLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips { this, 500 };
    juce::TextButton fifthsButton { "Pythagorean / fifths" }, syntonicButton { "Syntonic / fifths" };
    juce::TextButton harmonyButton { "Harmony lattice" };
    juce::TextButton aboutButton { "About / licenses" };
    juce::TextButton equalButton { "Equal temperament" }, pureButton { "Pure intervals" };
    juce::TextButton closeButton { "Close circles" }, calculateButton { "Calculate chart" };
    juce::TextButton copyButton { "Copy CSV" };
    juce::TextButton importButton { "CSV to fifths" };
    juce::TextEditor csvOutput, csvInput;
    std::array<std::vector<std::unique_ptr<CircleEditor>>, 2> circles;
    HarmonyView harmony;
    bool harmonyVisible = false;
    temperament::Mode mode = temperament::Mode::pythagoreanFifths;
    temperament::Calculation result;
    juce::String status;
    juce::String calculatedSignature;
    bool dirty = false;
    bool statusError = false;
    bool importedChart = false;
    std::vector<std::unique_ptr<CircleEditor>>& activeCircles();
    void setMode(temperament::Mode);
    void showHarmony();
    void updateView();
    void refreshHarmony();
    void markDirty();
    void calculate(bool closeCircles);
    void importCsv();
    void setPreset(bool equal);
    void invalidate(const juce::String& message);
    void drawResults(juce::Graphics&);
    juce::String inputSignature();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
