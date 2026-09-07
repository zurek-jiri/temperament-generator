// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "HarmonyView.h"
#include "Temperament.h"
#include "UiFonts.h"
#include <algorithm>
#include <cmath>

namespace
{
const juce::Colour panel(0xff17222e), field(0xff202f3e), border(0xff506375);
const juce::Colour ink(0xfff5f8fa), muted(0xffc4d1dc), green(0xff7fdda0), orange(0xfff0ad55), red(0xfff18f8d);
juce::Font font(float size, bool bold = false)
{
    return juce::Font(juce::FontOptions(ui::sansFont(), size, bold ? juce::Font::bold : juce::Font::plain));
}
void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds, float size = 24,
          juce::Colour colour = ink, juce::Justification align = juce::Justification::centredLeft, bool bold = false)
{
    g.setFont(font(size, bold)); g.setColour(colour); g.drawText(value, bounds, align, true);
}
void card(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(panel); g.fillRoundedRectangle(bounds, 16);
    g.setColour(border); g.drawRoundedRectangle(bounds.reduced(0.5f), 16, 1);
}
juce::String chordName(int root, bool minor)
{
    return juce::String(temperament::harmonyRootName(root, minor)) + (minor ? " minor" : " major");
}
juce::String qualityName(temperament::Consonance quality)
{
    switch (quality)
    {
        case temperament::Consonance::good: return "Good";
        case temperament::Consonance::tempered: return "Tempered";
        case temperament::Consonance::rough: return "Rough";
    }
    return {};
}
}

HarmonyView::HarmonyView()
{
    for (int i = 0; i < 12; ++i) rootChoice.addItem(temperament::harmonyRootName(i, false), i + 1);
    chordChoice.addItem("Major", 1); chordChoice.addItem("Minor", 2);
    sensitivity.addItem("Strict: 1 / 4 ct", 1);
    sensitivity.addItem("Standard: 2 / 6 ct", 2);
    sensitivity.addItem("Gentle: 4 / 10 ct", 3);
    thirdsSensitivity.addItem("Strict: 5 / 15 ct", 1);
    thirdsSensitivity.addItem("Standard: 8 / 22 ct", 2);
    thirdsSensitivity.addItem("Gentle: 12 / 30 ct", 3);
    rootChoice.setTitle("Chord root"); chordChoice.setTitle("Chord type"); sensitivity.setTitle("Fifth colour sensitivity");
    thirdsSensitivity.setTitle("Major and minor third colour sensitivity");
    for (auto* control : { &sensitivity, &thirdsSensitivity })
        control->setTooltip("Absolute deviation from pure: green up to the first limit, orange up to the second, red above it. Fifths and thirds have independent limits.");
    for (auto* control : { &rootChoice, &chordChoice, &sensitivity, &thirdsSensitivity }) addAndMakeVisible(control);
    rootChoice.setSelectedId(1, juce::dontSendNotification);
    chordChoice.setSelectedId(1, juce::dontSendNotification);
    sensitivity.setSelectedId(2, juce::dontSendNotification);
    thirdsSensitivity.setSelectedId(2, juce::dontSendNotification);
    rootChoice.onChange = [this] { selectChord(rootChoice.getSelectedId() - 1, minor); };
    chordChoice.onChange = [this] { selectChord(root, chordChoice.getSelectedId() == 2); };
    sensitivity.onChange = [this] { updateSensitivity(); };
    thirdsSensitivity.onChange = [this] { updateSensitivity(); };
    for (size_t i = 0; i < chordButtons.size(); ++i)
    {
        auto& button = chordButtons[i];
        const int note = static_cast<int>(i / 2);
        const bool isMinor = i % 2 != 0;
        button.setButtonText(chordName(note, isMinor));
        button.onClick = [this, note, isMinor] { selectChord(note, isMinor); };
        addAndMakeVisible(button);
    }
    rebuildLattice();
    refreshButtons();
}

void HarmonyView::setChart(const std::array<double, 12>& cents, bool valid, const juce::String& source)
{
    analysis = valid ? temperament::analyseHarmony(cents) : temperament::HarmonyAnalysis {};
    sourceLabel = source;
    rootChoice.setEnabled(analysis.valid); chordChoice.setEnabled(analysis.valid);
    setTooltip({}); refreshButtons(); repaint();
}
void HarmonyView::selectChord(int selectedRoot, bool selectedMinor)
{
    if (selectedRoot < 0 || selectedRoot > 11) return;
    root = selectedRoot; minor = selectedMinor;
    spellings = temperament::harmonyNoteNames(root, minor);
    for (int i = 0; i < 12; ++i) rootChoice.changeItemText(i + 1, temperament::harmonyRootName(i, minor));
    rootChoice.setSelectedId(root + 1, juce::dontSendNotification);
    chordChoice.setSelectedId(minor ? 2 : 1, juce::dontSendNotification);
    rebuildLattice(); refreshButtons(); setTooltip({}); repaint();
}
void HarmonyView::updateSensitivity()
{
    limits.fifths = sensitivity.getSelectedId() == 1 ? temperament::ColourLimits { 1, 4 }
                  : sensitivity.getSelectedId() == 3 ? temperament::ColourLimits { 4, 10 } : temperament::ColourLimits { 2, 6 };
    limits.thirds = thirdsSensitivity.getSelectedId() == 1 ? temperament::ColourLimits { 5, 15 }
                  : thirdsSensitivity.getSelectedId() == 3 ? temperament::ColourLimits { 12, 30 } : temperament::ColourLimits { 8, 22 };
    refreshButtons(); setTooltip({}); repaint();
}
juce::Colour HarmonyView::colour(temperament::Consonance quality) const
{
    switch (quality)
    {
        case temperament::Consonance::good: return green;
        case temperament::Consonance::tempered: return orange;
        case temperament::Consonance::rough: return red;
    }
    return muted;
}
juce::String HarmonyView::intervalDetails(const temperament::IntervalAnalysis& interval, const std::array<std::string, 12>& names) const
{
    return juce::String(temperament::harmonyIntervalName(interval.kind)) + ": " + juce::String(names[static_cast<size_t>(interval.from)]) + " to " + juce::String(names[static_cast<size_t>(interval.to)])
        + ". " + temperament::formatCents(interval.cents) + " cents; " + temperament::formatCents(interval.errorCents, true)
        + " cents from pure " + temperament::harmonyRatio(interval.kind) + ". " + qualityName(temperament::consonance(interval, limits)) + ".";
}
void HarmonyView::refreshButtons()
{
    for (size_t i = 0; i < chordButtons.size(); ++i)
    {
        auto& button = chordButtons[i];
        button.setEnabled(analysis.valid);
        const bool selected = static_cast<int>(i) == root * 2 + (minor ? 1 : 0);
        const auto qualityColour = analysis.valid ? colour(temperament::consonance(analysis.triads[i], limits)) : muted;
        button.setColour(juce::TextButton::buttonColourId, selected ? qualityColour.withAlpha(0.28f) : field);
        button.setColour(juce::TextButton::textColourOffId, qualityColour);
        juce::String tip = "Calculate a chart to compare triads.";
        if (analysis.valid)
        {
            const auto& chord = analysis.triads[i];
            tip = chordName(chord.root, chord.minor) + ": " + qualityName(temperament::consonance(chord, limits))
                + ". Largest interval deviation: " + temperament::formatCents(chord.worstError) + " cents. ";
            const auto names = temperament::harmonyNoteNames(chord.root, chord.minor);
            for (const auto& interval : chord.intervals) tip += intervalDetails(interval, names) + " ";
        }
        button.setTooltip(tip);
    }
}
void HarmonyView::rebuildLattice()
{
    nodes.clear(); edges.clear(); faces.clear();
    // Rows stagger by half a fifth. Up/right = major third; down/right = minor third.
    for (int row = 0; row < 5; ++row)
        for (int column = 0; column < 6; ++column)
        {
            const int q = column - row / 2;
            nodes.push_back({ q, row, temperament::latticeNote(q, row, root - 3),
                { 90.0f + 140.0f * (column + 0.5f * (row % 2)), 202.0f + 112.0f * (4 - row) } });
        }
    const auto find = [this](int q, int r) {
        for (size_t i = 0; i < nodes.size(); ++i) if (nodes[i].q == q && nodes[i].r == r) return static_cast<int>(i);
        return -1;
    };
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        const auto& n = nodes[i];
        const int fifth = find(n.q + 1, n.r), major = find(n.q, n.r + 1), minorThird = find(n.q + 1, n.r - 1);
        if (fifth >= 0) edges.push_back({ i, static_cast<size_t>(fifth), temperament::HarmonyInterval::fifth });
        if (major >= 0) edges.push_back({ i, static_cast<size_t>(major), temperament::HarmonyInterval::majorThird });
        if (minorThird >= 0) edges.push_back({ i, static_cast<size_t>(minorThird), temperament::HarmonyInterval::minorThird });
        if (fifth >= 0 && major >= 0) faces.push_back({ i, static_cast<size_t>(major), static_cast<size_t>(fifth), n.note, false });
        if (fifth >= 0 && minorThird >= 0) faces.push_back({ i, static_cast<size_t>(minorThird), static_cast<size_t>(fifth), n.note, true });
    }
}
juce::Path HarmonyView::triangle(const Face& face) const
{
    juce::Path path;
    path.startNewSubPath(nodes[face.a].point); path.lineTo(nodes[face.b].point); path.lineTo(nodes[face.c].point); path.closeSubPath();
    return path;
}
void HarmonyView::resized()
{
    rootChoice.setBounds(956, 52, 246, 48); chordChoice.setBounds(1214, 52, 218, 48);
    sensitivity.setBounds(102, 860, 340, 48);
    thirdsSensitivity.setBounds(592, 860, 340, 48);
    for (size_t i = 0; i < chordButtons.size(); ++i)
        chordButtons[i].setBounds(974 + static_cast<int>(i % 2) * 226, 518 + static_cast<int>(i / 2) * 42, 214, 38);
}
void HarmonyView::paint(juce::Graphics& g)
{
    text(g, "Harmony lattice", { 0, 0, 932, 40 }, 34, ink, juce::Justification::centredLeft, true);
    text(g, sourceLabel, { 0, 44, 932, 34 }, 24, muted);
    text(g, "Choose a chord", { 956, 0, 476, 40 }, 28, ink, juce::Justification::centredLeft, true);
    const auto arrow = [](int code) { return juce::String::charToString(code); };
    text(g, arrow(0x2192) + " Fifth 3:2     " + arrow(0x2197) + " Major third 5:4     " + arrow(0x2198) + " Minor third 6:5",
         { 0, 82, 932, 34 }, 26, muted);
    card(g, { 0, 124, 932, 712 });
    card(g, { 956, 124, 476, 306 });
    card(g, { 956, 442, 476, 602 });
    text(g, "All triads", { 978, 449, 430, 34 }, 28, ink, juce::Justification::centredLeft, true);
    text(g, "Colour follows the weakest interval", { 978, 481, 430, 30 }, 24, muted);
    text(g, "Fifths", { 0, 860, 98, 48 }, 26, ink, juce::Justification::centredLeft, true);
    text(g, "Thirds", { 486, 860, 102, 48 }, 26, ink, juce::Justification::centredLeft, true);
    text(g, "Green: good", { 0, 924, 296, 32 }, 26, green, juce::Justification::centredLeft, true);
    text(g, "Orange: tempered", { 318, 924, 306, 32 }, 26, orange, juce::Justification::centredLeft, true);
    text(g, "Red: rough", { 652, 924, 280, 32 }, 26, red, juce::Justification::centredLeft, true);
    const auto lessEqual = arrow(0x2264);
    text(g, "Fifths " + lessEqual + " " + juce::String(limits.fifths.good, 0) + " ct", { 0, 956, 296, 30 }, 24, muted);
    text(g, "Fifths " + lessEqual + " " + juce::String(limits.fifths.tempered, 0) + " ct", { 318, 956, 306, 30 }, 24, muted);
    text(g, "Fifths > " + juce::String(limits.fifths.tempered, 0) + " ct", { 652, 956, 280, 30 }, 24, muted);
    text(g, "Thirds " + lessEqual + " " + juce::String(limits.thirds.good, 0) + " ct", { 0, 984, 296, 30 }, 24, muted);
    text(g, "Thirds " + lessEqual + " " + juce::String(limits.thirds.tempered, 0) + " ct", { 318, 984, 306, 30 }, 24, muted);
    text(g, "Thirds > " + juce::String(limits.thirds.tempered, 0) + " ct", { 652, 984, 280, 30 }, 24, muted);
    text(g, "Colours compare tuning purity. Timbre and register also affect the sound.", { 0, 1030, 932, 32 }, 24, muted);
    if (!analysis.valid)
    {
        text(g, "No current temperament chart", { 40, 360, 852, 48 }, 34, ink, juce::Justification::centred, true);
        text(g, "Return to a fifths tab and calculate a closed circle", { 40, 420, 852, 36 }, 26, muted, juce::Justification::centred);
        text(g, "or import twelve CSV cent values.", { 40, 456, 852, 36 }, 26, muted, juce::Justification::centred);
        return;
    }
    for (const auto& face : faces)
    {
        const auto& chord = analysis.triads[static_cast<size_t>(face.root * 2 + (face.minor ? 1 : 0))];
        const bool selected = nodes[face.a].q == 1 && nodes[face.a].r == 2 && face.minor == minor;
        const auto path = triangle(face);
        g.setColour(colour(temperament::consonance(chord, limits)).withAlpha(selected ? 0.25f : 0.06f)); g.fillPath(path);
        if (selected) { g.setColour(ink); g.strokePath(path, juce::PathStrokeType(14)); }
    }
    for (const auto& edge : edges)
    {
        const auto start = nodes[edge.from].point, end = nodes[edge.to].point;
        const auto& interval = analysis.intervals[static_cast<size_t>(edge.kind)][static_cast<size_t>(nodes[edge.from].note)];
        g.setColour(colour(temperament::consonance(interval, limits)));
        g.drawLine({ start, end }, 8.5f);
        const auto direction = (end - start) / start.getDistanceFrom(end);
        const auto tip = start + (end - start) * 0.57f;
        const juce::Point<float> perpendicular(-direction.y, direction.x);
        juce::Path arrowHead; arrowHead.startNewSubPath(tip - direction * 12.0f + perpendicular * 9.0f);
        arrowHead.lineTo(tip); arrowHead.lineTo(tip - direction * 12.0f - perpendicular * 9.0f);
        g.strokePath(arrowHead, juce::PathStrokeType(4));
    }
    const auto& chord = analysis.triads[static_cast<size_t>(root * 2 + (minor ? 1 : 0))];
    for (const auto& node : nodes)
    {
        const auto name = noteName(node.note);
        const bool chordTone = std::find(chord.notes.begin(), chord.notes.end(), node.note) != chord.notes.end();
        const auto bounds = juce::Rectangle<float>(name.length() > 2 ? 112.0f : 66.0f, 52).withCentre(node.point);
        g.setColour(field); g.fillRoundedRectangle(bounds, 15);
        g.setColour(chordTone ? ink : border); g.drawRoundedRectangle(bounds, 15, chordTone ? 2.5f : 1);
        text(g, name, bounds.toNearestInt(), 28, ink, juce::Justification::centred, true);
    }
    text(g, "Click a triangle to select its chord; a note selects the root.", { 24, 748, 884, 34 }, 26, ink, juce::Justification::centred);
    text(g, "Names follow the selected key. Hover an edge for interval details.", { 24, 785, 884, 32 }, 24, muted, juce::Justification::centred);
    text(g, chordName(root, minor), { 978, 135, 268, 40 }, 34, ink, juce::Justification::centredLeft, true);
    text(g, qualityName(temperament::consonance(chord, limits)), { 1248, 140, 162, 34 }, 26, colour(temperament::consonance(chord, limits)), juce::Justification::centredRight, true);
    text(g, noteName(chord.notes[0]) + " - " + noteName(chord.notes[1]) + " - " + noteName(chord.notes[2]), { 978, 177, 430, 34 }, 26, muted);
    for (size_t i = 0; i < chord.intervals.size(); ++i)
    {
        const auto& interval = chord.intervals[i];
        const int y = 222 + static_cast<int>(i) * 66;
        text(g, juce::String(temperament::harmonyIntervalName(interval.kind)) + "  " + noteName(interval.from) + " > " + noteName(interval.to),
             { 978, y, 430, 30 }, 26, colour(temperament::consonance(interval, limits)), juce::Justification::centredLeft, true);
        text(g, temperament::formatCents(interval.errorCents, true) + " ct from pure (" + temperament::harmonyRatio(interval.kind) + ")",
             { 978, y + 30, 430, 30 }, 24, muted);
    }
}
bool HarmonyView::selectAt(juce::Point<float> point)
{
    if (!analysis.valid) return false;
    for (const auto& node : nodes)
    {
        const auto bounds = juce::Rectangle<float>(noteName(node.note).length() > 2 ? 112.0f : 66.0f, 52).withCentre(node.point);
        if (bounds.contains(point)) { selectChord(node.note, minor); return true; }
    }
    for (const auto& face : faces)
        if (triangle(face).contains(point)) { selectChord(face.root, face.minor); return true; }
    return false;
}
void HarmonyView::mouseMove(const juce::MouseEvent& event)
{
    if (!analysis.valid) { setTooltip({}); return; }
    for (const auto& edge : edges)
    {
        const juce::Line<float> line(nodes[edge.from].point, nodes[edge.to].point);
        juce::Point<float> nearest;
        if (line.getDistanceFromPoint(event.position, nearest) <= 10)
        {
            setTooltip(intervalDetails(analysis.intervals[static_cast<size_t>(edge.kind)][static_cast<size_t>(nodes[edge.from].note)], spellings));
            return;
        }
    }
    for (const auto& face : faces)
        if (triangle(face).contains(event.position))
        {
            const auto& chord = analysis.triads[static_cast<size_t>(face.root * 2 + (face.minor ? 1 : 0))];
            setTooltip(chordName(face.root, face.minor) + ": " + qualityName(temperament::consonance(chord, limits))
                + ". Largest interval deviation: " + temperament::formatCents(chord.worstError) + " cents. Click to inspect.");
            return;
        }
    setTooltip({});
}
