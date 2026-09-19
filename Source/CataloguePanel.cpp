// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "CataloguePanel.h"
#include "MainComponent.h"
#include "UiFonts.h"

namespace
{
const juce::Colour background(0xff10171f), field(0xff202f3e), ink(0xfff5f8fa), mint(0xff87e3c4);
juce::Font catalogueFont(float size=24) { return juce::Font(juce::FontOptions(ui::sansFont(),size,juce::Font::plain)); }
}
CataloguePanel::CataloguePanel(std::function<void(size_t)> selected)
    : style(std::make_unique<StudioLookAndFeel>()),load(std::move(selected))
{
    setLookAndFeel(style.get());
    search.setFont(catalogueFont());search.setIndents(12,8);
    search.setTextToShowWhenEmpty("Search names or notes",juce::Colour(0xffc4d1dc));
    search.setTitle("Search the temperament catalogue");
    search.onTextChange=[this] {filter();};
    search.onReturnKey=[this] {loadSelected();};
    search.onEscapeKey=[this] {close(0);};
    list.setModel(this);list.setRowHeight(40);list.setMultipleSelectionEnabled(false);
    list.setColour(juce::ListBox::backgroundColourId,field);
    list.getViewport()->setScrollBarThickness(22);list.setTitle("Temperament catalogue");
    details.setMultiLine(true,true);details.setReadOnly(true);details.setCaretVisible(false);
    details.setFont(catalogueFont());details.setIndents(14,14);
    details.setTitle("Selected temperament and catalogue note");
    accept.setColour(juce::TextButton::buttonColourId,mint);
    accept.setColour(juce::TextButton::textColourOffId,background);
    accept.onClick=[this] {loadSelected();};cancel.onClick=[this] {close(0);};
    for(auto* child:std::initializer_list<juce::Component*>{&search,&list,&details,&accept,&cancel}) addAndMakeVisible(child);
    setSize(1040,600);filter();
}
CataloguePanel::~CataloguePanel() {list.setModel(nullptr);setLookAndFeel(nullptr);}
void CataloguePanel::setSearch(const juce::String& text) {search.setText(text,false);filter();}
int CataloguePanel::selectedCatalogueIndex() const
{
    const int row=list.getSelectedRow();
    return row>=0&&row<visibleEntries()?static_cast<int>(entries[static_cast<size_t>(row)]):-1;
}
void CataloguePanel::filter()
{
    const int previous=selectedCatalogueIndex();
    const auto query=search.getText().trim();entries.clear();
    const auto& catalogue=temperament::knownTemperaments();
    for(size_t i=0;i<catalogue.size();++i)
        if(query.isEmpty()||juce::String(catalogue[i].name).containsIgnoreCase(query)
                         ||juce::String(catalogue[i].comments).containsIgnoreCase(query)) entries.push_back(i);
    list.deselectAllRows();list.updateContent();
    int row=0;
    for(size_t i=0;i<entries.size();++i) if(static_cast<int>(entries[i])==previous) row=static_cast<int>(i);
    if(!entries.empty()) list.selectRow(row);
    selectedRowsChanged(list.getSelectedRow());repaint();
}
void CataloguePanel::selectedRowsChanged(int)
{
    const int index=selectedCatalogueIndex();accept.setEnabled(index>=0);
    if(index<0) {details.setText("No matching temperaments. Try another search.",false);return;}
    const auto& entry=temperament::knownTemperaments()[static_cast<size_t>(index)];
    const auto comment=juce::String(entry.comments).trim();
    details.setText(juce::String(entry.name)+(comment.isEmpty()?"":"\n\nNote: "+comment)
        +"\n\nLoad fills both fifth circles and CSV out. Your pasted CSV in stays available for comparison.",false);
    details.setCaretPosition(0);
}
void CataloguePanel::loadSelected()
{
    const int index=selectedCatalogueIndex();
    if(index>=0) {load(static_cast<size_t>(index));close(1);}
}
void CataloguePanel::close(int result)
{
    if(auto* window=findParentComponentOfClass<juce::DialogWindow>()) window->exitModalState(result);
}
void CataloguePanel::paintListBoxItem(int row,juce::Graphics& g,int width,int height,bool selected)
{
    if(row<0||row>=visibleEntries()) return;
    g.fillAll(selected?mint:field);g.setColour(selected?background:ink);g.setFont(catalogueFont());
    g.drawText(juce::String(temperament::knownTemperaments()[entries[static_cast<size_t>(row)]].name),
               12,0,width-24,height,juce::Justification::centredLeft,true);
}
void CataloguePanel::paint(juce::Graphics& g)
{
    g.fillAll(background);g.setFont(catalogueFont());g.setColour(ink);
    g.drawText("Choose a temperament",20,12,getWidth()-40,34,juce::Justification::centredLeft);
    g.drawText(juce::String(visibleEntries())+" shown",20,getHeight()-60,200,40,juce::Justification::centredLeft);
}
void CataloguePanel::resized()
{
    search.setBounds(20,58,getWidth()-40,44);
    const int left=(getWidth()-60)*3/5;
    list.setBounds(20,118,left,getHeight()-202);
    details.setBounds(40+left,118,getWidth()-60-left,getHeight()-202);
    cancel.setBounds(getWidth()-344,getHeight()-64,120,44);
    accept.setBounds(getWidth()-212,getHeight()-64,192,44);
}
