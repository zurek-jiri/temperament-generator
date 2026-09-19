// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

class CataloguePanel final : public juce::Component, private juce::ListBoxModel
{
public:
    explicit CataloguePanel(std::function<void(size_t)>);
    ~CataloguePanel() override;
    void resized() override;
    void paint(juce::Graphics&) override;
    void setSearch(const juce::String&);
    int selectedCatalogueIndex() const;
    int visibleEntries() const { return static_cast<int>(entries.size()); }
    void loadSelected();
private:
    std::unique_ptr<juce::LookAndFeel> style;
    std::function<void(size_t)> load;
    juce::TextEditor search, details;
    juce::ListBox list;
    juce::TextButton accept { "Load selected" }, cancel { "Cancel" };
    std::vector<size_t> entries;
    void filter();
    int getNumRows() override { return visibleEntries(); }
    void paintListBoxItem(int,juce::Graphics&,int,int,bool) override;
    void selectedRowsChanged(int) override;
    void listBoxItemDoubleClicked(int,const juce::MouseEvent&) override { loadSelected(); }
    void returnKeyPressed(int) override { loadSelected(); }
    void close(int);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CataloguePanel)
};
