// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Temperament.h"
#include "HarmonyView.h"
#include "KnownTemperaments.h"
#include "CentDeviationView.h"

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
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class CircleEditor final : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr int automaticItemId = 1000;
    CircleEditor(temperament::Mode, size_t index, std::function<void()> changed);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override { selectAt(e.position); }
    void mouseMove(const juce::MouseEvent&) override;
    bool selectAt(juce::Point<float>);
    void setChart(const std::array<double,12>&, bool, temperament::HarmonyColourLimits);
    int selectedTone() const { return selected; }
    void clearSelection() { selected=-1; repaint(); }
    juce::Rectangle<float> ringBounds() const;
    bool containsTone(juce::Point<float>) const;
    bool chartValid() const { return analysis.valid; }
    std::array<std::string,12> expressions() const;
    void setExpressions(const std::array<std::string,12>&);
    void setEqual();
    void setPure();
    void setFromCsv(const temperament::ReverseCalculation&);
    void copyFrom(const CircleEditor&);
    void rotate(int clockwiseSteps);
    int circleAreaWidth() const { return diagramWidth(); }
    CentDeviationView& centChart() { return deviations; }
    const CentDeviationView& centChart() const { return deviations; }
    bool read(std::vector<double>&, juce::String& error, bool skipClosing = false);
    bool hasAutomatic() const;
    juce::String automaticSummary() const;
    void close(const std::vector<double>&);
    void showClosure(double error);
    void clearClosure();
    juce::String signature() const;
    juce::ComboBox& editor(size_t index) { return *editors[index]; }
private:
    temperament::Mode mode;
    size_t ringIndex;
    std::function<void()> onChange;
    std::vector<std::unique_ptr<juce::ComboBox>> editors;
    std::vector<juce::String> defaultTooltips;
    std::vector<juce::String> importedTexts;
    std::vector<double> roundingErrors;
    temperament::AutomaticCalculation automaticResult;
    temperament::HarmonyAnalysis analysis;
    temperament::HarmonyColourLimits limits;
    CentDeviationView deviations;
    int selected = -1;
    juce::Rectangle<float> nodeBounds(int index) const;
    void refreshRounding(size_t);
    double residual = 0;
    bool closureKnown = false;
    juce::Point<float> centre() const;
    float radius() const;
    int diagramWidth() const;
    int formulaListX() const;
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
    void mouseDown(const juce::MouseEvent&) override;
    bool renderPreviews(const juce::File& directory);
private:
    StudioLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips { this, 500 };
    juce::TextButton fifthsButton { "Inputs around circle" }, syntonicButton { "Formula list" };
    juce::TextButton harmonyButton { "Harmony lattice" };
    juce::TextButton aboutButton { "About / licenses" };
    juce::TextButton equalButton { "Equal temperament" }, pureButton { "Pure intervals" };
    juce::TextButton closeButton { "Close circle..." }, calculateButton { "Calculate chart" };
    juce::TextButton rotateLeftButton { "Rotate CCW" }, rotateRightButton { "Rotate CW" };
    juce::TextButton knownDetailsButton { "Suggestions" };
    juce::TextButton copyButton { "Copy CSV" };
    juce::TextButton importButton { "CSV to fifths" };
    juce::TextEditor csvOutput;
    // User-owned reference text: calculations, edits, presets and rotation must
    // never write to or clear this field. Both layouts share this one editor.
    juce::TextEditor csvInput;
    juce::ComboBox reconstruction;
    juce::Label knownLabel,statusLabel;
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
    void refreshRecognition();
    juce::String catalogueMatchDetails() const;
    void showCatalogueMatches();
    void synchroniseOtherCircle();
    void rotateCircle(int clockwiseSteps);
    void markDirty();
    void calculate(bool closeCircles);
    void importCsv();
    void showClosingOptions();
    void setPreset(bool equal);
    void invalidate(const juce::String& message);
    void drawResults(juce::Graphics&);
    juce::String inputSignature();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
