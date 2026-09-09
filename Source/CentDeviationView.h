// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// Chromatic note deviations from equal temperament, independent of comma units.
class CentDeviationView final : public juce::Component, public juce::SettableTooltipClient
{
public:
    CentDeviationView();
    void setChart(const std::array<double,12>&, bool valid);
    void paint(juce::Graphics&) override;
    void mouseMove(const juce::MouseEvent&) override;
    bool chartValid() const { return valid; }
    const std::array<double,12>& chart() const { return deviations; }
    double scaleCents() const { return extent; }
    juce::Rectangle<float> plotBounds() const;
private:
    std::array<double,12> deviations {};
    bool valid=false;
    double extent=1;
    int valueColumnWidth() const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CentDeviationView)
};
