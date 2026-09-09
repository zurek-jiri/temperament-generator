// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "Temperament.h"
#include <juce_gui_extra/juce_gui_extra.h>

class ClosingPanel final : public juce::Component
{
public:
    ClosingPanel(temperament::ClosingRequest, std::function<void(const temperament::ClosingProposal&)>);
    ~ClosingPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    // Also used by the headless GUI smoke check.
    void showProposal(const temperament::ClosingProposal&);
private:
    std::unique_ptr<juce::LookAndFeel> style;
    temperament::ClosingRequest request;
    temperament::ClosingProposal proposal;
    std::function<void(const temperament::ClosingProposal&)> apply;
    juce::ComboBox method, singleFifth, limit;
    std::array<juce::ToggleButton,12> selected;
    juce::TextButton preview { "Preview changes" }, accept { "Apply and calculate" }, cancel { "Cancel" };
    juce::TextEditor description;
    int generation = 0;
    void changed();
    void generate();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClosingPanel)
};
