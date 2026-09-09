// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "MainComponent.h"
#include "UiFonts.h"
#include <cmath>

namespace
{
const juce::Colour background(0xff10171f), field(0xff202f3e), border(0xff506375);
const juce::Colour ink(0xfff5f8fa), muted(0xffc4d1dc), mint(0xff87e3c4), amber(0xffedbe7b), red(0xfff18f8d);
void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds,
          float size, juce::Colour colour = ink, juce::Justification align = juce::Justification::centred, bool bold = false)
{
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions(ui::sansFont(), size, bold ? juce::Font::bold : juce::Font::plain)));
    g.drawText(value, bounds, align, true);
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
    const int numerator=juce::String(ratio.fraction).getIntValue();
    const auto expression=juce::String(numerator<0?"-":"")+(ratio.unit==temperament::Mode::syntonicFifths?"S":"P")
        +(std::abs(numerator)==1?"":"*"+juce::String(std::abs(numerator)))
        +(ratio.denominator==1?"":"/"+juce::String(ratio.denominator));
    return (ratio.exact ? juce::String("= ") : juce::String::charToString(0x2248) + " ") + expression;
}
juce::String orderedRatios(const temperament::AutomaticRatios& ratios, bool withErrors = false)
{
    const bool syntonicFirst=ratios.closest==temperament::Mode::syntonicFifths;
    const auto describe=[withErrors](const temperament::CommaRatio& ratio) {
        return ratioText(ratio)+(withErrors?" (error "+juce::String(ratio.errorCents,9)+" ct)":"");
    };
    return describe(syntonicFirst?ratios.syntonic:ratios.pythagorean)+"; "
        +describe(syntonicFirst?ratios.pythagorean:ratios.syntonic);
}
}

CircleEditor::CircleEditor(temperament::Mode selectedMode, size_t index, std::function<void()> changed)
    : mode(selectedMode), ringIndex(index), onChange(std::move(changed))
{
    const auto& ring = temperament::cycles(mode)[ringIndex];
    addAndMakeVisible(deviations);
    importedTexts.resize(12); roundingErrors.resize(12,0);
    for (size_t i=0;i<12;++i)
    {
        auto box=std::make_unique<juce::ComboBox>();
        box->setEditableText(true);
        box->setJustificationType(juce::Justification::centredLeft);
        box->addItem("Automatic calculation",automaticItemId);box->addSeparator();
        int id=1;
        for (const auto& choice:temperament::expressionChoices(mode)) box->addItem(choice.text,id++);
        box->addItem("ET",id);
        box->setTitle("Fifth "+juce::String(static_cast<int>(i+1))+": "+noteLabel(ring.notes[i])+" to "+noteLabel(ring.notes[(i+1)%12]));
        box->setTooltip(box->getTitle()+". Use P/12, -S/4-schisma/4, 1-1/11, or Auto. schisma = P - S (H is an alias). A bare fraction uses "
                        +(mode==temperament::Mode::pythagoreanFifths?"P":"S")+" units. S is the syntonic (diatonic) comma; P is the Pythagorean (ditonic) comma.");
        defaultTooltips.push_back(box->getTooltip());
        box->onChange=[this,i,input=box.get()] {
            if (temperament::isAutomaticExpression(inputText(*input).toStdString())) input->setText("Auto",juce::dontSendNotification);
            refreshRounding(i);onChange();
        };
        for (auto* child:box->getChildren())
            if(auto* label=dynamic_cast<juce::Label*>(child))
                label->onEditorShow=[this,label,i] {
                    if(auto* input=label->getCurrentTextEditor()) input->onTextChange=[this,i] {refreshRounding(i);onChange();};
                };
        addAndMakeVisible(*box);editors.push_back(std::move(box));
    }
    setEqual();
}
juce::String CircleEditor::noteLabel(int note) const
{
    static const std::array<const char*,12> names {"C","Db/C#","D","Eb/D#","E","F","F#/Gb","G","Ab/G#","A","Bb","B"};
    return names[static_cast<size_t>(note)];
}
int CircleEditor::diagramWidth() const
{
    return mode==temperament::Mode::pythagoreanFifths?juce::roundToInt(getWidth()*0.66f)
        :juce::jlimit(520,600,juce::roundToInt(getWidth()*0.40f));
}
int CircleEditor::formulaListX() const { return deviations.getRight()+12; }
juce::Point<float> CircleEditor::centre() const { return {diagramWidth()*0.5f,getHeight()*0.5f}; }
float CircleEditor::radius() const
{
    return juce::jmin(200.0f,getHeight()*0.5f-50.0f,
        mode==temperament::Mode::syntonicFifths?diagramWidth()*0.5f-80.0f:200.0f);
}
juce::Rectangle<float> CircleEditor::ringBounds() const
{
    return juce::Rectangle<float>(2*radius(),2*radius()).withCentre(centre());
}
juce::Point<float> CircleEditor::point(float index,bool marker) const
{
    const float angle=juce::MathConstants<float>::twoPi*index/12;
    juce::Point<float> result;
    if(marker&&mode==temperament::Mode::pythagoreanFifths)
    {
        float x=std::sin(angle)*juce::jmin(radius()+120,diagramWidth()*0.5f-82);
        if(std::abs(x)<130) x=x<0?-130.0f:130.0f;
        result=centre()+juce::Point<float>(x,-std::cos(angle)*juce::jmin(radius()+60,getHeight()*0.5f-18));
    }
    else
    {
        const float r=marker?juce::jmin(radius()+44,getHeight()*0.5f-18):radius();
        result=centre()+juce::Point<float>(std::sin(angle)*r,-std::cos(angle)*r);
    }
    if(marker)
    {
        const bool pyth=mode==temperament::Mode::pythagoreanFifths;
        const float width=pyth?148.0f:38.0f;
        const float height=pyth?static_cast<float>(juce::jmin(40,(getHeight()-22)/12)):28.0f;
        for(int attempt=0;attempt<40;++attempt)
        {
            const auto bounds=juce::Rectangle<float>(width,height).withCentre(result).expanded(3);
            bool overlaps=false;
            for(int i=0;i<12;++i) if(bounds.intersects(nodeBounds(i))) {overlaps=true;break;}
            if(!overlaps) break;
            result.x+=std::sin(angle)*4;
            result.y=juce::jlimit(height*0.5f+1,getHeight()-height*0.5f-1,result.y-std::cos(angle)*4);
        }
    }
    return result;
}
juce::Rectangle<float> CircleEditor::nodeBounds(int i) const
{
    const int note=temperament::cycles(mode)[ringIndex].notes[static_cast<size_t>(i)];
    const bool compact=getHeight()<460;
    return juce::Rectangle<float>(note==1||note==3||note==8||note==6?(compact?88.0f:106.0f):(compact?46.0f:58.0f),compact?34.0f:44.0f)
        .withCentre(point(static_cast<float>(i),false));
}
void CircleEditor::resized()
{
    const int row=juce::jmin(42,(getHeight()-22)/12);
    const int chartX=diagramWidth()+12;
    const int chartWidth=mode==temperament::Mode::pythagoreanFifths?getWidth()-chartX-8
        :juce::jlimit(250,320,juce::roundToInt(getWidth()*0.20f));
    deviations.setBounds(chartX,0,chartWidth,juce::jmin(getHeight(),486));
    const int fieldX=formulaListX()+148;
    for(size_t i=0;i<12;++i)
        if(mode==temperament::Mode::pythagoreanFifths)
            editors[i]->setBounds(juce::Rectangle<int>(148,juce::jmin(40,row)).withCentre(point(static_cast<float>(i)+0.5f,true).roundToInt()));
        else editors[i]->setBounds(fieldX,22+static_cast<int>(i)*row,getWidth()-fieldX-4,row-1);
}
void CircleEditor::setChart(const std::array<double,12>& values,bool valid,temperament::HarmonyColourLimits colours)
{
    analysis=valid?temperament::analyseHarmony(values):temperament::HarmonyAnalysis {};
    deviations.setChart(values,analysis.valid);
    limits=colours;repaint();
}
bool CircleEditor::selectAt(juce::Point<float> p)
{
    const auto& notes=temperament::cycles(mode)[ringIndex].notes;
    for (int i=0;i<12;++i)
    {
        if (nodeBounds(i).contains(p)) {selected=selected==notes[i]?-1:notes[i];repaint();return true;}
        if (juce::Rectangle<float>(52,44).withCentre(point(static_cast<float>(i)+0.5f,true)).contains(p))
        {editors[i]->showEditor();return true;}
    }
    if(p.getDistanceFrom(centre())>radius()) {clearSelection();return true;}
    return false;
}
void CircleEditor::mouseMove(const juce::MouseEvent& event)
{
    const auto& notes=temperament::cycles(mode)[ringIndex].notes;
    for (int i=0;i<12;++i)
    {
        if (nodeBounds(i).contains(event.position))
        {
            juce::String details=noteLabel(notes[i])+": click to highlight all connected fifths and thirds.";
            if(notes[i]==1) details+=" C# is also called cis.";
            if(analysis.valid) for(int kind=0;kind<3;++kind)
            {
                const auto& interval=analysis.intervals[kind][notes[i]];
                details+=" "+juce::String(temperament::harmonyIntervalName(interval.kind))+" to "+noteLabel(interval.to)+": "
                    +cents(interval.errorCents,true)+" cents from pure.";
            }
            setTooltip(details);return;
        }
    }
    setTooltip("Click outside the circle to clear the selection. Arcs are fifths (3:2), triangles are major thirds (5:4), and squares are minor thirds (6:5). "
        "Their purity colours share the lattice sensitivities. The note chart shows flat notes to the left and sharp notes to the right of equal temperament.");
}
void CircleEditor::paint(juce::Graphics& g)
{
    const auto& notes=temperament::cycles(mode)[ringIndex].notes;
    const auto c=centre();
    // Four major-third triangles and three minor-third squares, then fifth arcs.
    // Draw highlighted connections last so their crossings stay readable.
    for(int pass=0;pass<2;++pass)
    for(const auto& connection:temperament::fifthCircleConnections())
    {
        const int i=connection.fromIndex, j=connection.toIndex, kind=static_cast<int>(connection.kind);
        const bool highlighted=selected>=0&&(notes[i]==selected||notes[j]==selected);
        if(highlighted!=(pass==1)) continue;
        juce::Colour colour=border;
        if(analysis.valid)
        {
            const auto quality=temperament::consonance(analysis.intervals[kind][notes[i]],limits);
            colour=quality==temperament::Consonance::good?juce::Colour(0xff7fdda0):quality==temperament::Consonance::tempered?juce::Colour(0xfff0ad55):red;
        }
        if(selected>=0&&!highlighted) colour=colour.withAlpha(0.28f);
        g.setColour(colour);
        const float width=highlighted?10.0f:kind==0?7.0f:4.5f;
        if(kind==0)
        {
            juce::Path arc;
            arc.addCentredArc(c.x,c.y,radius(),radius(),0,juce::MathConstants<float>::twoPi*i/12,
                             juce::MathConstants<float>::twoPi*(i+1)/12,true);
            g.strokePath(arc,juce::PathStrokeType(width,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        }
        else g.drawLine({point(static_cast<float>(i),false),point(static_cast<float>(j),false)},width);
    }
    static const std::array<const char*,12> intervals {"C > G","G > D","D > A","A > E","E > B","B > F#","F# > C#","Db > Ab","Ab > Eb","Eb > Bb","Bb > F","F > C"};
    for(int i=0;i<12;++i)
    {
        const auto middle=point(static_cast<float>(i)+0.5f,true);
        const auto near=point(static_cast<float>(i)+0.5f,false);
        g.setColour(border);g.drawLine({near,middle},2);
        if(mode==temperament::Mode::syntonicFifths)
        {
            const auto badge=juce::Rectangle<float>(38,28).withCentre(middle);
            g.setColour(field);g.fillRoundedRectangle(badge,6);
            text(g,juce::String(i+1),badge.toNearestInt(),22,mint,juce::Justification::centred,true);
            const auto rowText=juce::String(i+1)+"  "+intervals[i];
            const int row=juce::jmin(42,(getHeight()-22)/12);
            text(g,rowText,{formulaListX(),22+i*row,136,row},getHeight()<460?22.0f:24.0f,ink,juce::Justification::centredLeft);
        }
    }
    for(int i=0;i<12;++i)
    {
        const auto node=nodeBounds(i);
        const bool active=notes[i]==selected;
        g.setColour(active?mint:field);g.fillRoundedRectangle(node,15);
        g.setColour(notes[i]==9||active?mint:border);g.drawRoundedRectangle(node,15,active?3.0f:1.5f);
        text(g,noteLabel(notes[i]),node.toNearestInt(),getHeight()<460?22.0f:28.0f,active?background:ink,juce::Justification::centred,true);
    }
    if(mode==temperament::Mode::syntonicFifths)
    {
        text(g,"Fifth",{formulaListX(),0,130,22},22,muted,juce::Justification::centredLeft);
        text(g,"P / S / schisma  (bare = S)",{formulaListX()+148,0,getWidth()-formulaListX()-152,22},22,mint,juce::Justification::centredLeft);
        g.setColour(border.withAlpha(0.5f));
        g.drawVerticalLine(deviations.getX()-6,0,static_cast<float>(getHeight()));
        g.drawVerticalLine(deviations.getRight()+6,0,static_cast<float>(getHeight()));
    }
    else if(getHeight()-deviations.getBottom()>=84)
    {
        const int x=deviations.getX(),w=deviations.getWidth(),y=deviations.getBottom()+6;
        text(g,"Arcs: fifths 3:2",{x,y,w,24},22,muted,juce::Justification::centredLeft);
        text(g,"Triangles: major thirds 5:4",{x,y+26,w,24},22,muted,juce::Justification::centredLeft);
        text(g,"Squares: minor thirds 6:5",{x,y+52,w,24},22,muted,juce::Justification::centredLeft);
    }
}

void CircleEditor::setEqual()
{
    std::array<std::string,12> values;values.fill(mode==temperament::Mode::pythagoreanFifths?"-1/12":"-P/12");setExpressions(values);
}
void CircleEditor::setPure() {std::array<std::string,12> values;values.fill("0");setExpressions(values);}
std::array<std::string,12> CircleEditor::expressions() const
{
    std::array<std::string,12> values;
    for(size_t i=0;i<12;++i) values[i]=inputText(*editors[i]).toStdString();
    return values;
}
void CircleEditor::setExpressions(const std::array<std::string,12>& values)
{
    automaticResult={};
    for(size_t i=0;i<12;++i) {importedTexts[i].clear();editors[i]->setText(values[i],juce::dontSendNotification);refreshRounding(i);}
}
bool CircleEditor::read(std::vector<double>& values,juce::String& error,bool skipClosing)
{
    values.clear();automaticResult={};
    std::array<temperament::FifthCorrection,12> corrections {};
    const auto inputs=expressions();
    for(size_t i=0;i<12;++i)
    {
        if(skipClosing&&!hasAutomatic()&&static_cast<int>(i)==temperament::cycles(mode)[ringIndex].closingEdge) continue;
        corrections[i].automaticPrimary=temperament::isAutomaticExpression(inputs[i]);
        std::string parseError;
        if(!corrections[i].automaticPrimary&&(!temperament::evaluateExpression(inputs[i],mode,corrections[i].primary,parseError)||std::abs(corrections[i].primary)>1000))
        {
            editors[i]->setColour(juce::ComboBox::outlineColourId,red);
            error="Fifth "+juce::String(static_cast<int>(i+1))+": "+juce::String(parseError.empty()?"Correction exceeds 1000 commas.":parseError);return false;
        }
    }
    automaticResult=temperament::resolveAutomatic(mode,corrections);
    if(!automaticResult.valid) {error=automaticResult.error;return false;}
    values.assign(automaticResult.fractions.begin(),automaticResult.fractions.end());
    for(size_t i=0;i<12;++i) refreshRounding(i);
    return true;
}
bool CircleEditor::hasAutomatic() const
{
    for(const auto& box:editors) if(temperament::isAutomaticExpression(inputText(*box).toStdString())) return true;
    return false;
}
void CircleEditor::copyFrom(const CircleEditor& source)
{
    const auto inputs=source.expressions();
    std::array<std::string,12> translated;
    for(size_t i=0;i<12;++i) translated[i]=temperament::expressionInMode(inputs[i],source.mode,mode);
    setExpressions(translated);
    for(size_t i=0;i<12;++i)
    {
        roundingErrors[i]=source.roundingErrors[i];
        importedTexts[i]=source.importedTexts[i].isNotEmpty()&&source.importedTexts[i]==juce::String(inputs[i])
            ?juce::String(translated[i]):juce::String();
    }
    std::vector<double> values;juce::String error;
    read(values,error);
    residual=source.residual;closureKnown=source.closureKnown;selected=source.selected;
    repaint();
}
void CircleEditor::rotate(int clockwiseSteps)
{
    const int shift=(clockwiseSteps%12+12)%12;
    const auto previous=expressions();
    const auto previousImported=importedTexts;
    const auto previousErrors=roundingErrors;
    automaticResult={};selected=-1;
    for(int i=0;i<12;++i)
    {
        const int source=(i-shift+12)%12;
        editors[i]->setText(previous[source],juce::dontSendNotification);
        importedTexts[i]=previousImported[source];roundingErrors[i]=previousErrors[source];
        refreshRounding(i);
    }
    repaint();
}
bool CircleEditor::containsTone(juce::Point<float> pointToTest) const
{
    for(int i=0;i<12;++i) if(nodeBounds(i).contains(pointToTest)) return true;
    return false;
}
juce::String CircleEditor::automaticSummary() const
{
    if(!automaticResult.valid) return "Automatic fields need calculation.";
    return "Auto: "+juce::String(automaticResult.automaticFields)+" fields, "+cents(automaticResult.centsPerField,true)+" ct each / "
        +orderedRatios(automaticResult.ratios)+". Circle closed; A = 0.";
}
void CircleEditor::close(const std::vector<double>& values)
{
    const auto index=static_cast<size_t>(temperament::cycles(mode)[ringIndex].closingEdge);
    editors[index]->setText(temperament::fractionExpression(temperament::closingFraction(mode,values,static_cast<int>(index)),mode),juce::dontSendNotification);
    refreshRounding(index);
}
void CircleEditor::refreshRounding(size_t i)
{
    if(i>=editors.size()) return;
    auto& box=*editors[i];
    if(inputText(box)!=importedTexts[i]) importedTexts[i].clear();
    const bool rounded=importedTexts[i].isNotEmpty()&&std::abs(roundingErrors[i])>1e-7;
    const bool automatic=temperament::isAutomaticExpression(inputText(box).toStdString());
    const auto colour=std::abs(roundingErrors[i])>0.0010001?red:amber;
    box.setColour(juce::ComboBox::textColourId,automatic?mint:rounded?colour:ink);
    box.setColour(juce::ComboBox::outlineColourId,automatic?mint:rounded?colour:border);
    box.setColour(juce::ComboBox::backgroundColourId,field);
    box.setTooltip(defaultTooltips[i]+" Current expression: "+inputText(box)
                   +(rounded?". Rounded from CSV; interval difference "+juce::String(roundingErrors[i],9)+" cents.":""));
    if(automatic&&automaticResult.valid)
        box.setTooltip(box.getTooltip()+" Calculated value: "+juce::String(automaticResult.centsPerField,9)+" cents. "
            +orderedRatios(automaticResult.ratios,true)
            +". Display fractions have denominators up to 24; calculation uses full precision.");
    repaint();
}
void CircleEditor::setFromCsv(const temperament::ReverseCalculation& imported)
{
    automaticResult={};
    for(size_t i=0;i<12;++i)
    {
        editors[i]->setText(imported.expressions[i],juce::dontSendNotification);
        importedTexts[i]=inputText(*editors[i]);roundingErrors[i]=imported.intervalErrors[i];refreshRounding(i);
    }
    showClosure(imported.closureError);
}
void CircleEditor::showClosure(double error) {residual=error;closureKnown=true;repaint();}
void CircleEditor::clearClosure()
{
    closureKnown=false;automaticResult={};for(size_t i=0;i<12;++i) refreshRounding(i);repaint();
}
juce::String CircleEditor::signature() const
{
    juce::String value;
    for(const auto& expression:expressions()) value+=(temperament::isAutomaticExpression(expression)?juce::String("Auto"):juce::String(expression))+"|";
    return value;
}
