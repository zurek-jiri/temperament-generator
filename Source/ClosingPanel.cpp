// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "ClosingPanel.h"
#include "MainComponent.h"
#include "UiFonts.h"

namespace
{
const juce::Colour ink(0xfff5f8fa), mint(0xff87e3c4), muted(0xffc4d1dc);
juce::String edgeName(int i)
{
    static const std::array<const char*,12> names {"C > G","G > D","D > A","A > E","E > B","B > F#","F# > C#","Db > Ab","Ab > Eb","Eb > Bb","Bb > F","F > C"};
    return juce::String(i+1)+"  "+names[static_cast<size_t>(i)];
}
}
ClosingPanel::ClosingPanel(temperament::ClosingRequest input,std::function<void(const temperament::ClosingProposal&)> applied)
    : style(std::make_unique<StudioLookAndFeel>()),request(std::move(input)),apply(std::move(applied))
{
    setLookAndFeel(style.get());
    method.addItem("Adjust one fifth",1);
    method.addItem("Share equally across selected fifths",2);
    method.addItem("Share equally across all twelve fifths",3);
    method.addItem("Prefer simple fractions (inference)",4);
    method.addItem("Resolve automatic fields",5);
    bool automatic=false;
    for(const auto& value:request.expressions) automatic|=temperament::isAutomaticExpression(value);
    int selectedCount=0,selectedEdge=6;
    for(int i=0;i<12;++i) if(request.selected[i]) {++selectedCount;selectedEdge=i;}
    method.setSelectedId(request.method==temperament::ClosingMethod::simpleFit?4:request.method==temperament::ClosingMethod::allEqually?3:
                         automatic?5:selectedCount>1?2:1,juce::dontSendNotification);
    method.setTitle("Circle closing method");
    method.onChange=[this] {changed();};addAndMakeVisible(method);
    for(int i=0;i<12;++i)
    {
        singleFifth.addItem(edgeName(i),i+1);
        selected[i].setButtonText(edgeName(i));selected[i].setToggleState(selectedCount==0||request.selected[i],juce::dontSendNotification);
        selected[i].onClick=[this] {changed();};addAndMakeVisible(selected[i]);
    }
    singleFifth.setSelectedId(selectedEdge+1,juce::dontSendNotification);singleFifth.onChange=[this] {changed();};addAndMakeVisible(singleFifth);
    singleFifth.setTitle("Fifth to adjust");
    limit.setEditableText(true);int id=1;
    for(const auto* value:{"0.001","0.01","0.05","0.1","0.5","1","2"}) limit.addItem(value,id++);
    limit.setText("0.1",juce::dontSendNotification);limit.onChange=[this] {changed();};addAndMakeVisible(limit);
    limit.setTitle("Maximum change per selected fifth in cents");
    for(auto* button:{&preview,&accept,&cancel})
    {
        addAndMakeVisible(button);button->setWantsKeyboardFocus(true);
        button->setMouseClickGrabsKeyboardFocus(true);
    }
    preview.onClick=[this] {generate();};
    accept.onClick=[this] {if(proposal.valid) {apply(proposal);if(auto* w=findParentComponentOfClass<juce::DialogWindow>()) w->exitModalState(1);}};
    cancel.onClick=[this] {if(auto* w=findParentComponentOfClass<juce::DialogWindow>()) w->exitModalState(0);};
    description.setMultiLine(true);description.setReadOnly(true);
    description.setFont(juce::Font(juce::FontOptions(ui::monoFont(),24.0f,juce::Font::plain)));
    description.setTitle("Proposed fifth formulas and changes in cents");description.setIndents(12,12);addAndMakeVisible(description);
    setSize(960,640);changed();
}
ClosingPanel::~ClosingPanel() {setLookAndFeel(nullptr);}
void ClosingPanel::changed()
{
    ++generation;proposal={};accept.setEnabled(false);preview.setEnabled(true);
    const int choice=method.getSelectedId();
    singleFifth.setEnabled(choice==1);limit.setEnabled(choice==4);
    for(auto& box:selected) box.setEnabled(choice==2||choice==4);
    description.setText("Choose a method, then Preview changes.\n\nSimple fitting favours small fractions and readable expressions. It keeps simple fifths and puts the remaining correction where needed. Your change limit applies to each fifth.\n\nExplicit adjustment replaces Auto with calculated formulas.\n\n"+juce::String(request.useTargets?"Changes are measured against the imported CSV fifths.":"Changes are measured against the current formula values."),false);
    repaint();
}
void ClosingPanel::generate()
{
    const int choice=method.getSelectedId();
    request.method=choice==3?temperament::ClosingMethod::allEqually:choice==4?temperament::ClosingMethod::simpleFit:
        choice==5?temperament::ClosingMethod::automaticFields:temperament::ClosingMethod::selectedEqually;
    for(int i=0;i<12;++i) request.selected[i]=choice==1?singleFifth.getSelectedId()==i+1:selected[i].getToggleState();
    if(choice==4&&!temperament::parseNumber(limit.getText().toStdString(),request.maximumChange))
    {description.setText("Enter a valid maximum change in cents.",false);return;}
    // Only inference uses CSV as the optimisation target; exact equal adjustment
    // acts on the formulas. The report still compares with CSV when available.
    const int token=++generation;
    preview.setEnabled(false);accept.setEnabled(false);description.setText("Calculating a proposal...",false);
    const auto input=request;
    const juce::Component::SafePointer<ClosingPanel> safe(this);
    juce::Thread::launch([input,safe,token] {
        const auto result=temperament::proposeClosing(input);
        juce::MessageManager::callAsync([safe,token,result] {if(safe!=nullptr&&safe->generation==token) safe->showProposal(result);});
    });
}
void ClosingPanel::showProposal(const temperament::ClosingProposal& result)
{
    proposal=result;preview.setEnabled(true);accept.setEnabled(result.valid);
    juce::String output(result.message);
    if(result.valid)
    {
        output+="\n\nFormula closure: "+juce::String(result.closureBefore,9)+" -> "+juce::String(result.closureAfter,9)+" ct\nLargest fifth change: "+juce::String(result.maximumChange,9)+" ct\n\n";
        for(int i=0;i<12;++i) output+=edgeName(i)+"\n  "+juce::String(request.expressions[i])+"  ->  "+juce::String(result.expressions[i])
            +"\n  Change: "+juce::String(result.changes[i],9)+" ct\n\n";
    }
    description.setText(output,false);description.moveCaretToTop(false);
}
void ClosingPanel::resized()
{
    method.setBounds(24,44,912,36);singleFifth.setBounds(24,92,280,36);
    for(int i=0;i<12;++i) selected[i].setBounds(24+(i%4)*228,140+(i/4)*36,220,36);
    limit.setBounds(586,262,140,36);preview.setBounds(24,262,270,36);
    description.setBounds(24,310,912,getHeight()-372);
    accept.setBounds(510,getHeight()-50,280,36);cancel.setBounds(802,getHeight()-50,134,36);
}
void ClosingPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10171f));
    g.setFont(juce::Font(juce::FontOptions(ui::sansFont(),28.0f,juce::Font::bold)));g.setColour(ink);
    g.drawText("Close the circle / review before applying",24,6,912,32,juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(ui::sansFont(),24.0f,juce::Font::plain)));g.setColour(muted);
    g.drawText("One fifth",320,92,250,36,juce::Justification::centredLeft);
    g.drawText("Fit change limit",318,262,260,36,juce::Justification::centredRight);
    g.drawText("cents / fifth",738,262,198,36,juce::Justification::centredLeft);
    g.setColour(mint);g.drawText("Only Apply changes the tuning.",24,getHeight()-50,480,36,juce::Justification::centredLeft);
}
