// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "Harmony.h"
#include "ChordPlayback.h"

class HarmonyView final : public juce::Component, public juce::SettableTooltipClient
{
public:
    HarmonyView();
    ~HarmonyView() override;
    void setChart(const std::array<double, 12>&, bool valid, const juce::String& source);
    void selectChord(int root, bool minor);
    bool selectAt(juce::Point<float>);
    const temperament::HarmonyAnalysis& chart() const { return analysis; }
    int selectedRoot() const { return root; }
    bool selectedMinor() const { return minor; }
    juce::ComboBox& sensitivityControl() { return sensitivity; }
    juce::ComboBox& thirdsSensitivityControl() { return thirdsSensitivity; }
    juce::String displayedNoteName(int note) const { return spellings[static_cast<size_t>(note)]; }
    temperament::HarmonyColourLimits colourLimits() const { return limits; }
    std::function<void()> onColoursChanged;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override { setTooltip({}); }
    void mouseDown(const juce::MouseEvent& e) override { selectAt(e.position); }
    void visibilityChanged() override { if(!isVisible()) playback.stop(); }
private:
    struct Node { int q, r, note; juce::Point<float> point; };
    struct Edge { size_t from, to; temperament::HarmonyInterval kind; };
    struct Face { size_t a, b, c; int root; bool minor; };
    temperament::HarmonyAnalysis analysis;
    temperament::HarmonyColourLimits limits;
    std::array<std::string, 12> spellings = temperament::harmonyNoteNames(0, false);
    juce::String sourceLabel;
    juce::ComboBox rootChoice, chordChoice, sensitivity, thirdsSensitivity;
    std::array<juce::TextButton, 24> chordButtons;
    juce::Component chordPanel;
    juce::Viewport chordViewport;
    juce::ToggleButton playChords { "Play Chords into Default Audio Output" };
    juce::Slider reverb { juce::Slider::LinearHorizontal,juce::Slider::TextBoxRight };
    juce::Slider loudness { juce::Slider::LinearHorizontal,juce::Slider::TextBoxRight };
    juce::Label reverbLabel,loudnessLabel,audioStatus;
    ChordPlayback playback;
    std::array<double,12> tuning {};
    void audition();
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    std::vector<Face> faces;
    int root = 0;
    bool minor = false;
    void rebuildLattice();
    void refreshButtons();
    void updateSensitivity();
    juce::Path triangle(const Face&) const;
    juce::Colour colour(temperament::Consonance quality) const;
    juce::String noteName(int note) const { return displayedNoteName(note); }
    juce::String intervalDetails(const temperament::IntervalAnalysis&, const std::array<std::string, 12>& names) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmonyView)
};
