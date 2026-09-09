// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
#include "CentDeviationView.h"
#include "Temperament.h"
#include "UiFonts.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour ink(0xfff5f8fa), muted(0xffc4d1dc), border(0xff506375), mint(0xff87e3c4);
const juce::Colour flat(0xff8dbfff), sharp(0xfff4b4d8);
const juce::String help="Cents from equal temperament, with A = 0. Blue bars to the left are flat; pink bars to the right are sharp. "
    "All notes use the same symmetric scale. Hover over a row for the exact deviation.";
void label(juce::Graphics& g,const juce::String& text,juce::Rectangle<float> bounds,
           juce::Colour colour,juce::Justification alignment=juce::Justification::centredLeft,bool bold=false)
{
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions(ui::sansFont(),22.0f,bold?juce::Font::bold:juce::Font::plain)));
    g.drawText(text,bounds,alignment,true);
}
}
CentDeviationView::CentDeviationView()
{
    setTitle("Note deviations from equal temperament");
    setDescription(help);
    setTooltip(help);
}
void CentDeviationView::setChart(const std::array<double,12>& values,bool available)
{
    valid=available&&std::all_of(values.begin(),values.end(),[](double value){return std::isfinite(value);});
    deviations=valid?values:std::array<double,12>{};
    double largest=1;
    for(double value:deviations) largest=std::max(largest,std::abs(value));
    const double decade=std::pow(10.0,std::floor(std::log10(largest)));
    extent=10*decade;
    for(double step:{1.0,2.0,2.5,5.0,10.0})
        if(largest<=step*decade+1e-10) {extent=step*decade;break;}
    setTooltip(help);
    repaint();
}
int CentDeviationView::valueColumnWidth() const
{
    return getWidth()<340?0:extent>=1000?136:112;
}
juce::Rectangle<float> CentDeviationView::plotBounds() const
{
    return {44.0f,54.0f,static_cast<float>(getWidth()-56-valueColumnWidth()),static_cast<float>(getHeight()-56)};
}
void CentDeviationView::paint(juce::Graphics& g)
{
    const auto plot=plotBounds();
    const float zero=plot.getCentreX(),row=plot.getHeight()/12;
    const float numericWidth=static_cast<float>(valueColumnWidth());
    label(g,"Cents vs equal",{4,0,static_cast<float>(getWidth()-8),27},ink,juce::Justification::centredLeft,true);
    if(valid)
    {
        const auto range=juce::String(extent,extent<10&&std::floor(extent)!=extent?1:0);
        label(g,"-"+range,{plot.getX(),27,plot.getWidth()*0.5f-14,25},flat);
        label(g,"0",{zero-14,27,28,25},ink,juce::Justification::centred);
        label(g,"+"+range,{zero+14,27,plot.getWidth()*0.5f-14,25},sharp,juce::Justification::centredRight);
        if(numericWidth>0) label(g,"cents",{plot.getRight()+6,27,numericWidth-6,25},muted,juce::Justification::centredRight);
    }
    else label(g,"Calculate to view",{4,27,static_cast<float>(getWidth()-8),25},muted);
    g.setColour(flat.withAlpha(0.045f));g.fillRect(plot.withWidth(plot.getWidth()*0.5f));
    g.setColour(sharp.withAlpha(0.045f));g.fillRect(plot.withLeft(zero));
    for(int note=0;note<12;++note)
    {
        const float y=plot.getY()+note*row,centreY=y+row*0.5f;
        g.setColour(border.withAlpha(0.25f));
        g.drawHorizontalLine(juce::roundToInt(y),2,static_cast<float>(getWidth()-2));
        label(g,temperament::noteNames()[note],{4,y,38,row},note==9?mint:ink,juce::Justification::centredLeft,note==9);
        if(valid)
        {
            const double value=deviations[note];
            const float length=static_cast<float>(std::abs(value)/extent)*plot.getWidth()*0.5f;
            if(std::abs(value)>1e-9)
            {
                g.setColour(value<0?flat:sharp);
                const float thickness=std::min(18.0f,row*0.58f);
                g.fillRect(juce::Rectangle<float>(value<0?zero-length:zero,centreY-thickness*0.5f,length,thickness));
            }
            if(numericWidth>0) label(g,temperament::formatCents(value,true),{plot.getRight()+6,y,numericWidth-6,row},
                note==9?mint:ink,juce::Justification::centredRight);
        }
        else label(g,"--",{plot.getX(),y,plot.getWidth()+numericWidth,row},muted,juce::Justification::centred);
    }
    g.setColour(ink.withAlpha(valid?0.8f:0.2f));
    g.drawVerticalLine(juce::roundToInt(zero),plot.getY(),plot.getBottom());
    if(valid) for(int note=0;note<12;++note) if(std::abs(deviations[note])<=1e-9)
    {
        g.setColour(note==9?mint:muted);
        g.fillEllipse(zero-3,plot.getY()+(note+0.5f)*row-3,6,6);
    }
}
void CentDeviationView::mouseMove(const juce::MouseEvent& event)
{
    const auto plot=plotBounds();
    const int note=static_cast<int>(std::floor((event.position.y-plot.getY())/(plot.getHeight()/12)));
    if(valid&&note>=0&&note<12)
    {
        const double value=deviations[note];
        setTooltip(juce::String(temperament::noteNames()[note])+": "+juce::String(temperament::formatCents(value,true))
            +" cents from equal temperament (A = 0). "+(value<0?"Flat / left.":value>0?"Sharp / right.":"At equal temperament.")
            +" Scale: -"+juce::String(extent)+" to +"+juce::String(extent)+" cents.");
    }
    else setTooltip(help);
}
