// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "MainComponent.h"
#include "AppIcon.h"
#include "UiFonts.h"
#include "AppLegal.h"
#include <cmath>

namespace
{
const juce::Colour background(0xff10171f), panel(0xff17222e), field(0xff202f3e);
const juce::Colour border(0xff506375), ink(0xfff5f8fa), muted(0xffc4d1dc);
const juce::Colour mint(0xff87e3c4), amber(0xffedbe7b), red(0xfff18f8d);

juce::Font font(float size, bool bold = false)
{
    return juce::Font(juce::FontOptions(ui::sansFont(), size, bold ? juce::Font::bold : juce::Font::plain));
}
void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds,
          float size = 24.0f, juce::Colour colour = ink,
          juce::Justification align = juce::Justification::centredLeft, bool bold = false)
{
    g.setColour(colour);
    g.setFont(font(size, bold));
    g.drawText(value, bounds, align, true);
}
juce::String cents(double value, bool sign = false) { return temperament::formatCents(value, sign); }
void card(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(panel);
    g.fillRoundedRectangle(bounds.toFloat(), 16);
    g.setColour(border.withAlpha(0.65f));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 16, 1);
}
juce::TextLayout tooltipLayout(const juce::String& value, float width)
{
    juce::AttributedString attributed;
    attributed.append(value, font(24), ink);
    juce::TextLayout layout;
    layout.createLayout(attributed, width);
    return layout;
}
}

StudioLookAndFeel::StudioLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, field);
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::outlineColourId, border);
    setColour(juce::ComboBox::focusedOutlineColourId, mint);
    setColour(juce::ComboBox::arrowColourId, muted);
    setColour(juce::TextButton::buttonColourId, field);
    setColour(juce::TextButton::buttonOnColourId, mint);
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::TextButton::textColourOnId, background);
    setColour(juce::TextEditor::backgroundColourId, field);
    setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TextEditor::outlineColourId, border);
    setColour(juce::TextEditor::focusedOutlineColourId, mint);
    setColour(juce::TextEditor::highlightColourId, mint.withAlpha(0.3f));
    setColour(juce::CaretComponent::caretColourId, mint);
    setColour(juce::PopupMenu::backgroundColourId, field);
    setColour(juce::PopupMenu::textColourId, ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, mint);
    setColour(juce::PopupMenu::highlightedTextColourId, background);
    setColour(juce::TooltipWindow::backgroundColourId, field);
    setColour(juce::TooltipWindow::textColourId, ink);
    setColour(juce::TooltipWindow::outlineColourId, border);
}
juce::Font StudioLookAndFeel::getComboBoxFont(juce::ComboBox&) { return font(26, true); }
juce::Font StudioLookAndFeel::getTextButtonFont(juce::TextButton&, int) { return font(26, true); }
juce::Font StudioLookAndFeel::getPopupMenuFont() { return font(26); }
void StudioLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(3, 1, box.getWidth() - 25, box.getHeight() - 2);
    label.setBorderSize(juce::BorderSize<int>(1));
    label.setFont(getComboBoxFont(box));
    label.setMinimumHorizontalScale(1.0f);
}
juce::Rectangle<int> StudioLookAndFeel::getTooltipBounds(const juce::String& tip, juce::Point<int> position,
                                                       juce::Rectangle<int> parent)
{
    const int width = juce::jmin(640, parent.getWidth() - 24);
    const auto layout = tooltipLayout(tip, static_cast<float>(width - 24));
    const int height = juce::roundToInt(std::ceil(layout.getHeight())) + 24;
    return juce::Rectangle<int>(position.x + 18, position.y + 24, width, height).constrainedWithin(parent);
}
void StudioLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& tip, int width, int height)
{
    g.fillAll(field);
    g.setColour(border);
    g.drawRect(0, 0, width, height, 2);
    tooltipLayout(tip, static_cast<float>(width - 24)).draw(g, juce::Rectangle<float>(12, 12,
        static_cast<float>(width - 24), static_cast<float>(height - 24)));
}
void StudioLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height)).reduced(0.5f);
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, 7);
    g.setColour(box.hasKeyboardFocus(true) ? mint : box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, 7, 1);
    const float x = static_cast<float>(width) - 15, y = static_cast<float>(height) * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath(x - 3, y - 2);
    arrow.lineTo(x, y + 1);
    arrow.lineTo(x + 3, y - 2);
    g.setColour(muted);
    g.strokePath(arrow, juce::PathStrokeType(1.4f));
}
void StudioLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& colour, bool hovered, bool down)
{
    auto fill = colour;
    if (hovered) fill = fill.brighter(0.10f);
    if (down) fill = fill.darker(0.12f);
    g.setColour(fill.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.4f));
    g.fillRoundedRectangle(button.getLocalBounds().toFloat().reduced(0.5f), 8);
}

namespace
{
class AboutPanel final : public juce::Component
{
public:
    AboutPanel()
    {
        setLookAndFeel(&look);
        for (auto* button : { &overview, &license, &notices }) addAndMakeVisible(button);
        overview.onClick = [this] { showPage(0); };
        license.onClick = [this] { showPage(1); };
        notices.onClick = [this] { showPage(2); };
        source.setFont(font(24, true), false);
        source.setColour(juce::HyperlinkButton::textColourId, mint);
        addAndMakeVisible(source);
        body.setMultiLine(true, true);
        body.setReadOnly(true);
        body.setCaretVisible(false);
        body.setScrollbarsShown(true);
        body.setFont(font(24));
        body.setIndents(16, 16);
        addAndMakeVisible(body);
        showPage(0);
        setSize(1060, 780);
    }
    ~AboutPanel() override { setLookAndFeel(nullptr); }
    void showPage(int index)
    {
        overview.setToggleState(index == 0, juce::dontSendNotification);
        license.setToggleState(index == 1, juce::dontSendNotification);
        notices.setToggleState(index == 2, juce::dontSendNotification);
        body.setTitle(index == 0 ? "About this application" : index == 1 ? "Full AGPLv3 license" : "Third-party notices");
        body.setText(index == 0 ? juce::String::fromUTF8(AppLegal::About_txt, AppLegal::About_txtSize)
                     : index == 1 ? juce::String::fromUTF8(AppLegal::LICENSE, AppLegal::LICENSESize)
                                  : juce::String::fromUTF8(AppLegal::ThirdPartyNotices_txt, AppLegal::ThirdPartyNotices_txtSize), false);
        body.setCaretPosition(0);
        body.scrollEditorToPositionCaret(16, 16);
    }
    void paint(juce::Graphics& g) override
    {
        g.fillAll(background);
        text(g, "Temperament Generator " JUCE_APPLICATION_VERSION_STRING, { 24, 14, getWidth() - 48, 52 }, 34, ink,
             juce::Justification::centredLeft, true);
    }
    void resized() override
    {
        overview.setBounds(24, 82, 150, 48);
        license.setBounds(186, 82, 230, 48);
        notices.setBounds(428, 82, 300, 48);
        source.setBounds(746, 82, getWidth() - 770, 48);
        body.setBounds(24, 150, getWidth() - 48, getHeight() - 174);
    }
private:
    StudioLookAndFeel look;
    juce::TextButton overview { "About" }, license { "AGPLv3 license" }, notices { "Third-party notices" };
    juce::HyperlinkButton source { "Project source", juce::URL("https://github.com/zurek-jiri/temperament-generator") };
    juce::TextEditor body;
};
}

MainComponent::MainComponent()
{
    setLookAndFeel(&lookAndFeel);
    setOpaque(true);
    for (auto* button : { &fifthsButton, &syntonicButton, &harmonyButton, &aboutButton, &equalButton, &pureButton, &closeButton, &calculateButton, &copyButton, &importButton })
    {
        addAndMakeVisible(button);
        // Taking focus commits a fraction currently being typed into an editable ComboBox.
        button->setWantsKeyboardFocus(true);
        button->setMouseClickGrabsKeyboardFocus(true);
    }
    fifthsButton.setClickingTogglesState(false);
    syntonicButton.setClickingTogglesState(false);
    fifthsButton.onClick = [this] { setMode(temperament::Mode::pythagoreanFifths); };
    syntonicButton.onClick = [this] { setMode(temperament::Mode::syntonicFifths); };
    harmonyButton.onClick = [this] { showHarmony(); };
    aboutButton.onClick = [this] {
        juce::DialogWindow::LaunchOptions options;
        options.content.setOwned(new AboutPanel());
        options.dialogTitle = "About and licenses";
        options.dialogBackgroundColour = background;
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = false;
        options.componentToCentreAround = this;
        options.launchAsync();
    };
    addChildComponent(harmony);
    equalButton.onClick = [this] { setPreset(true); };
    pureButton.onClick = [this] { setPreset(false); };
    closeButton.onClick = [this] { calculate(true); };
    calculateButton.onClick = [this] { calculate(false); };
    importButton.onClick = [this] { importCsv(); };
    calculateButton.setColour(juce::TextButton::buttonColourId, mint);
    calculateButton.setColour(juce::TextButton::textColourOffId, background);
    copyButton.onClick = [this] {
        if (inputSignature() != calculatedSignature) markDirty();
        if (result.valid && !dirty)
        {
            juce::SystemClipboard::copyTextToClipboard(csvOutput.getText());
            status = "CSV copied to clipboard.";
            repaint();
        }
    };
    pureButton.setTooltip("Set every interval to pure (0 comma). Use Close circles to assign the remaining difference to one interval per circle.");
    equalButton.setTooltip("Reset all notes to twelve-tone equal temperament.");
    csvOutput.setReadOnly(true);
    csvOutput.setMultiLine(false);
    csvOutput.setFont(juce::Font(juce::FontOptions(ui::monoFont(), 24.0f, juce::Font::plain)));
    csvOutput.setIndents(12, 12);
    csvOutput.setTitle("CSV deviations, C through B, separated by semicolons");
    csvOutput.setTextToShowWhenEmpty("Calculate a closed temperament to generate the CSV row.", muted);
    addAndMakeVisible(csvOutput);
    csvInput.setFont(juce::Font(juce::FontOptions(ui::monoFont(), 24.0f, juce::Font::plain)));
    csvInput.setMultiLine(false);
    csvInput.setIndents(12, 12);
    csvInput.setTitle("CSV input: twelve cent deviations, C through B");
    csvInput.setTextToShowWhenEmpty("Paste 12 semicolon-separated cent values here", muted);
    csvInput.onReturnKey = [this] { importCsv(); };
    csvInput.onTextChange = [this] { csvInput.setColour(juce::TextEditor::outlineColourId, border); };
    importButton.setTooltip("Reconstruct clockwise fifths and choose the nearest menu fraction. In paired mode, syntonic fractions go in Syntonic and Pythagorean fractions in Pythagorean.");
    addAndMakeVisible(csvInput);
    for (size_t m = 0; m < circles.size(); ++m)
    {
        const auto selected = m == 0 ? temperament::Mode::pythagoreanFifths : temperament::Mode::syntonicFifths;
        for (size_t i = 0; i < temperament::cycles(selected).size(); ++i)
        {
            auto ring = std::make_unique<CircleEditor>(selected, i, [this] { markDirty(); });
            addChildComponent(*ring);
            circles[m].push_back(std::move(ring));
        }
    }
    setSize(1480, 1080);
    setMode(mode);
}
MainComponent::~MainComponent() { setLookAndFeel(nullptr); }
std::vector<std::unique_ptr<CircleEditor>>& MainComponent::activeCircles()
{
    return circles[mode == temperament::Mode::pythagoreanFifths ? 0 : 1];
}
void MainComponent::setMode(temperament::Mode selected)
{
    const bool returnToSource = harmonyVisible && mode == selected;
    harmonyVisible = false;
    mode = selected;
    closeButton.setTooltip("With Auto fields: share the remaining correction equally in cents among those fields, keeping fixed entries. Otherwise keep eleven fifths and calculate F#/Gb -> Db/C#.");
    calculateButton.setTooltip("Calculate the chart. Automatic calculation fields share the missing or excess correction equally in cents so the circle closes.");
    updateView();
    if (!returnToSource) calculate(false);
}
void MainComponent::updateView()
{
    const bool fifths = mode == temperament::Mode::pythagoreanFifths;
    fifthsButton.setToggleState(!harmonyVisible && fifths, juce::dontSendNotification);
    syntonicButton.setToggleState(!harmonyVisible && !fifths, juce::dontSendNotification);
    harmonyButton.setToggleState(harmonyVisible, juce::dontSendNotification);
    for (auto* button : { &equalButton, &pureButton, &closeButton, &calculateButton, &copyButton, &importButton })
        button->setVisible(!harmonyVisible);
    csvInput.setVisible(!harmonyVisible); csvOutput.setVisible(!harmonyVisible);
    for (size_t m = 0; m < circles.size(); ++m)
        for (auto& ring : circles[m]) ring->setVisible(!harmonyVisible && m == (fifths ? 0u : 1u));
    harmony.setVisible(harmonyVisible);
    setSize(1480, !harmonyVisible && fifths ? 1080 : 1234);
    resized(); repaint();
}
void MainComponent::refreshHarmony()
{
    const auto source = (importedChart ? juce::String("Imported CSV") : juce::String("Calculated chart"))
        + (mode == temperament::Mode::pythagoreanFifths ? " / Pythagorean fifths" : " / Syntonic + Pythagorean fifths");
    harmony.setChart(result.cents, result.valid && !dirty, result.valid && !dirty ? source : "No current chart / return to a fifths tab to calculate");
}
void MainComponent::showHarmony()
{
    if (dirty || inputSignature() != calculatedSignature) calculate(false);
    harmonyVisible = true;
    refreshHarmony(); updateView();
}
void MainComponent::markDirty()
{
    if (inputSignature() == calculatedSignature) return;
    dirty = true;
    importedChart = false;
    result.valid = false;
    statusError = false;
    status = "Inputs changed. Calculate chart to update the deviations.";
    csvOutput.clear();
    copyButton.setEnabled(false);
    for (auto& ring : activeCircles()) ring->clearClosure();
    refreshHarmony();
    repaint();
}
void MainComponent::invalidate(const juce::String& message)
{
    dirty = false;
    result.valid = false;
    status = message;
    statusError = true;
    csvOutput.clear();
    copyButton.setEnabled(false);
    refreshHarmony();
    repaint();
}
void MainComponent::calculate(bool closeCircles)
{
    importedChart = false;
    auto& rings = activeCircles();
    std::vector<std::vector<double>> values(rings.size());
    juce::String error;
    for (auto& ring : rings) ring->clearClosure();
    for (size_t i = 0; i < rings.size(); ++i)
        if (!rings[i]->read(values[i], error, closeCircles))
        {
            invalidate(error);
            return;
        }
    if (closeCircles)
    {
        for (size_t i = 0; i < rings.size(); ++i)
        {
            if (rings[i]->hasAutomatic()) continue;
            const int edge = temperament::cycles(mode)[i].closingEdge;
            const double fraction = temperament::closingFraction(mode, values[i], edge);
            if (!std::isfinite(fraction) || std::abs(fraction) > 1000)
            {
                invalidate("The required closing fraction is outside -1000 to 1000. Reduce the other fractions.");
                return;
            }
        }
        for (size_t i = 0; i < rings.size(); ++i)
        {
            if (rings[i]->hasAutomatic()) continue;
            rings[i]->close(values[i]);
            values[i].clear();
            if (!rings[i]->read(values[i], error)) { invalidate(error); return; }
        }
    }
    result = temperament::calculate(mode, values);
    for (size_t i = 0; i < result.closureErrors.size(); ++i) rings[i]->showClosure(result.closureErrors[i]);
    if (!result.valid) { invalidate(result.error); return; }
    dirty = false;
    statusError = false;
    status = rings.front()->hasAutomatic() ? "Automatic fields balanced equally in cents. Circle closed; A = 0.000 cents."
        : closeCircles ? "Closing intervals adjusted. Chart calculated with A = 0.000 cents."
                         : "Chart calculated. All circles close; A = 0.000 cents.";
    calculatedSignature = inputSignature();
    csvOutput.setText(temperament::csv(result.cents), false);
    copyButton.setEnabled(true);
    refreshHarmony();
    repaint();
}
void MainComponent::importCsv()
{
    const auto imported = temperament::reverseCsv(csvInput.getText().toStdString(), mode);
    if (!imported.valid)
    {
        csvInput.setColour(juce::TextEditor::outlineColourId, red);
        status = "CSV import: " + juce::String(imported.error);
        statusError = true;
        repaint();
        return;
    }
    activeCircles().front()->setFromCsv(imported);
    result = {};
    result.valid = true;
    result.cents = imported.cents;
    result.closureErrors = { imported.closureError };
    importedChart = true;
    dirty = false;
    calculatedSignature = inputSignature();
    csvOutput.setText(temperament::csv(result.cents), false);
    csvInput.setColour(juce::TextEditor::outlineColourId, border);
    copyButton.setEnabled(true);
    const bool closes = std::abs(imported.closureError) <= 0.000001;
    statusError = !closes;
    status = closes ? "CSV loaded. Nearest fractions close. Calculate chart to use the rounded circle."
                    : "CSV loaded. Rounded circle does not close (" + cents(imported.closureError, true)
                        + " ct). Edit fractions or use Close circles.";
    refreshHarmony();
    repaint();
}
juce::String MainComponent::inputSignature()
{
    juce::String signatureText(mode == temperament::Mode::pythagoreanFifths ? "pythagorean|" : "syntonic|");
    for (const auto& ring : activeCircles()) signatureText += ring->signature() + "|";
    return signatureText;
}
void MainComponent::setPreset(bool equal)
{
    for (auto& ring : activeCircles())
        if (equal) ring->setEqual(); else ring->setPure();
    calculate(false);
}
void MainComponent::resized()
{
    aboutButton.setBounds(760, 20, 270, 48);
    fifthsButton.setBounds(24, 78, 282, 48);
    syntonicButton.setBounds(318, 78, 264, 48);
    harmonyButton.setBounds(594, 78, 316, 48);
    harmony.setBounds(24, 140, 1432, 1070);
    equalButton.setBounds(934, 78, 258, 48);
    pureButton.setBounds(1204, 78, 252, 48);
    circles[0][0]->setBounds(40, 204, 852, 598);
    circles[1][0]->setBounds(40, 204, 1392, 598);
    closeButton.setBounds(470, 153, 200, 46);
    calculateButton.setBounds(682, 153, 210, 46);
    const int extra = mode == temperament::Mode::syntonicFifths ? 154 : 0;
    csvInput.setBounds(44, 926 + extra, 1172, 52);
    importButton.setBounds(1230, 926 + extra, 206, 52);
    csvOutput.setBounds(44, 1012 + extra, 1172, 52);
    copyButton.setBounds(1230, 1012 + extra, 206, 52);
}
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.drawImageWithin(appIcon(), 24, 13, 54, 54, juce::RectanglePlacement::centred);
    text(g, "Temperament Generator", { 92, 16, 650, 48 }, 40, ink, juce::Justification::centredLeft, true);
    text(g, "12 notes / A = 0 cents", { 1030, 22, 426, 38 }, 28, mint, juce::Justification::centredRight, true);
    if (harmonyVisible) return;
    card(g, { 24, 140, 884, 740 });
    card(g, { 928, 140, 528, 740 });
    const int extra = mode == temperament::Mode::syntonicFifths ? 154 : 0;
    card(g, { 24, 892 + extra, 1432, 188 });
    text(g, "Circle of fifths", { 46, 155, 418, 42 }, 32, ink, juce::Justification::centredLeft, true);
    text(g, "H = schisma = " + cents(temperament::schisma()) + " ct    S = syntonic    P = Pythagorean", { 46, 792, 840, 34 }, 24, muted, juce::Justification::centred);
    text(g, "Amber: rounded", { 90, 832, 290, 34 }, 24, amber, juce::Justification::centred, true);
    text(g, "Red: exceeds CSV precision", { 382, 832, 450, 34 }, 24, red, juce::Justification::centred, true);
    drawResults(g);
    text(g, "CSV input", { 44, 895 + extra, 160, 30 }, 24, mint, juce::Justification::centredLeft, true);
    text(g, "C ; C# ; D ; D# ; E ; F ; F# ; G ; G# ; A ; A# ; B", { 210, 895 + extra, 1226, 30 }, 24, muted);
    text(g, importedChart ? "CSV output / imported values, A = 0" : "CSV output / calculated chart, A = 0",
         { 44, 980 + extra, 1392, 32 }, 24, mint, juce::Justification::centredLeft, true);
}
void MainComponent::drawResults(juce::Graphics& g)
{
    if (mode == temperament::Mode::syntonicFifths)
    {
        text(g, "Fifth corrections", { 950, 154, 482, 44 }, 32, ink, juce::Justification::centredLeft, true);
        text(g, "Syntonic + Pythagorean contributions", { 950, 201, 482, 34 }, 24, muted);
        text(g, "Fifth", { 950, 235, 160, 32 }, 24, muted, juce::Justification::centredLeft, true);
        text(g, "Syntonic", { 1120, 235, 148, 32 }, 24, mint, juce::Justification::centred, true);
        text(g, "Pythagorean", { 1280, 235, 148, 32 }, 24, mint, juce::Justification::centred, true);
        g.setColour(statusError ? red : dirty ? amber : mint);
        g.setFont(font(24));
        g.drawFittedText(status, { 952, 760, 478, 105 }, juce::Justification::topLeft, 4, 1.0f);
        card(g, { 24, 892, 1432, 142 });
        text(g, importedChart ? "Imported CSV chart / A = 0" : "Temperament chart / A = 0", { 44, 899, 1360, 34 }, 26, ink, juce::Justification::centredLeft, true);
        for (int i = 0; i < 12; ++i)
        {
            const int x = 44 + i * 116;
            text(g, temperament::noteNames()[static_cast<size_t>(i)], { x, 941, 112, 34 }, 30, i == 9 ? mint : muted, juce::Justification::centred, true);
            text(g, result.valid ? cents(result.cents[static_cast<size_t>(i)], true) : "--", { x, 979, 112, 40 }, 27, ink, juce::Justification::centred);
        }
        return;
    }
    text(g, importedChart ? "Imported CSV chart" : "Temperament chart", { 950, 154, 482, 44 }, 32, ink, juce::Justification::centredLeft, true);
    text(g, "Deviation from equal temperament", { 950, 201, 482, 34 }, 24, muted);
    text(g, "Note", { 953, 240, 80, 32 }, 24, muted, juce::Justification::centredLeft, true);
    text(g, "Cents", { 1040, 240, 135, 32 }, 24, muted, juce::Justification::centredRight, true);
    double range = 10;
    if (result.valid) for (auto value : result.cents) range = juce::jmax(range, std::abs(value));
    range = std::ceil(range / 5.0) * 5.0;
    text(g, "+/- " + juce::String(range, 0), { 1220, 240, 210, 32 }, 24, muted, juce::Justification::centred);
    for (int i = 0; i < 12; ++i)
    {
        const int y = 280 + i * 38;
        if (i == 9)
        {
            g.setColour(mint.withAlpha(0.09f));
            g.fillRoundedRectangle(946.0f, static_cast<float>(y), 492, 37, 5);
        }
        else if (i % 2 == 0)
        {
            g.setColour(field.withAlpha(0.35f));
            g.fillRoundedRectangle(946.0f, static_cast<float>(y), 492, 37, 5);
        }
        const auto label = temperament::noteNames()[static_cast<size_t>(i)];
        text(g, label, { 959, y, 76, 37 }, 32, i == 9 ? mint : ink, juce::Justification::centredLeft, true);
        text(g, result.valid ? cents(result.cents[static_cast<size_t>(i)], true) : "--",
             { 1035, y, 140, 37 }, 30, result.valid ? ink : muted, juce::Justification::centredRight);
        g.setColour(border);
        g.drawVerticalLine(1317, static_cast<float>(y + 4), static_cast<float>(y + 32));
        if (result.valid)
        {
            const auto rawValue = result.cents[static_cast<size_t>(i)];
            const auto value = std::abs(rawValue) < 0.0005 ? 0.0 : rawValue;
            const float width = static_cast<float>(std::abs(value) / range) * 102;
            g.setColour(value < 0 ? amber : mint);
            if (width < 1) g.fillEllipse(1314, static_cast<float>(y + 15), 6, 6);
            else g.fillRoundedRectangle(value < 0 ? 1317 - width : 1317, static_cast<float>(y + 13), width, 12, 4);
        }
    }
    g.setColour(border);
    g.drawHorizontalLine(746, 950, 1432);
    g.setColour(statusError ? red : dirty ? amber : mint);
    g.setFont(font(24));
    g.drawFittedText(status, { 952, 760, 478, 105 }, juce::Justification::topLeft, 4, 1.0f);
}

bool MainComponent::renderPreviews(const juce::File& directory)
{
    if (directory.createDirectory().failed()) return false;
    AboutPanel about;
    for (int page = 0; page < 3; ++page)
    {
        about.showPage(page);
        auto output = directory.getChildFile("about-page-" + juce::String(page) + ".png").createOutputStream();
        if (output == nullptr) return false;
        output->setPosition(0);
        output->truncate();
        if (!juce::PNGImageFormat().writeImageToStream(about.createComponentSnapshot(about.getLocalBounds()), *output)) return false;
    }
    auto save = [this, &directory](const juce::String& name) {
        auto output = directory.getChildFile(name).createOutputStream();
        if (!output) return false;
        output->setPosition(0);
        output->truncate();
        return juce::PNGImageFormat().writeImageToStream(createComponentSnapshot(getLocalBounds()), *output);
    };
    setMode(temperament::Mode::pythagoreanFifths);
    setPreset(true);
    if (!result.valid || !save("fifths-equal.png")) return false;
    setPreset(false);
    if (result.valid || csvOutput.getText().isNotEmpty() || !save("fifths-unclosed.png")) return false;
    calculate(true);
    if (!result.valid || !save("fifths-chart.png")) return false;
    const auto pythagoreanCsv = csvOutput.getText();
    // Test stale output protection and parsing through the actual GUI controls.
    activeCircles()[0]->editor(0).setText("1/0", juce::sendNotificationSync);
    if (result.valid || copyButton.isEnabled() || csvOutput.getText().isNotEmpty()) return false;
    calculate(false);
    if (result.valid || !statusError) return false;
    setPreset(true);
    auto& typedBox = activeCircles()[0]->editor(0);
    typedBox.showEditor();
    juce::Label* editingLabel = nullptr;
    for (auto* child : typedBox.getChildren())
        if (auto* label = dynamic_cast<juce::Label*>(child)) editingLabel = label;
    if (editingLabel == nullptr || editingLabel->getCurrentTextEditor() == nullptr) return false;
    editingLabel->getCurrentTextEditor()->setText("-1/6", false);
    calculate(false);
    if (result.valid) return false; // Calculation must read the still-active text editor.
    editingLabel->hideEditor(false);
    calculate(true);
    if (!result.valid) return false;
    markDirty(); // A delayed ComboBox notification must not invalidate this fresh result.
    if (!result.valid) return false;
    setMode(temperament::Mode::syntonicFifths);
    if (!result.valid || !save("syntonic-fifths-equal.png")) return false;
    for (size_t i = 0; i < 12; ++i)
        if (activeCircles()[0]->editor(i).getText() != "0" || activeCircles()[0]->pythagoreanEditor(i).getText() != "-1/12") return false;
    setPreset(false);
    if (result.valid) return false;
    calculate(true);
    if (!result.valid || result.cents[9] != 0 || activeCircles()[0]->editor(6).getText() != "0"
        || activeCircles()[0]->pythagoreanEditor(6).getText() != "-1"
        || !save("syntonic-fifths-chart.png")) return false;
    csvInput.setText(pythagoreanCsv, false);
    importCsv();
    if (mode != temperament::Mode::syntonicFifths || !result.valid || activeCircles()[0]->editor(6).getText() != "0"
        || activeCircles()[0]->pythagoreanEditor(6).getText() != "-1"
        || !save("syntonic-fifths-imported.png")) return false;
    // Four quarter-P fifths close; two opposite S corrections exercise both columns.
    std::vector<std::vector<double>> quarterInputs { std::vector<double>(12, 0) };
    for (size_t i = 0; i < 4; ++i) quarterInputs[0][i] = -0.25;
    quarterInputs[0][4] = temperament::syntonicComma() / (6 * temperament::pythagoreanComma());
    quarterInputs[0][5] = -quarterInputs[0][4];
    const auto quarterChart = temperament::calculate(temperament::Mode::pythagoreanFifths, quarterInputs);
    if (!quarterChart.valid) return false;
    const juce::String quarterCsv = temperament::csv(quarterChart.cents);
    csvInput.setText(quarterCsv, false);
    importCsv();
    if (!result.valid || !importedChart || statusError || csvOutput.getText() != quarterCsv) return false;
    for (size_t i = 0; i < 12; ++i)
    {
        if (activeCircles()[0]->editor(i).getText() != (i == 4 ? "1/6" : i == 5 ? "-1/6" : "0")
            || activeCircles()[0]->pythagoreanEditor(i).getText() != (i < 4 ? "-1/4" : "0")) return false;
    }
    if (activeCircles()[0]->editor(0).findColour(juce::ComboBox::textColourId) != ink
        || activeCircles()[0]->pythagoreanEditor(0).findColour(juce::ComboBox::textColourId) != amber
        || !activeCircles()[0]->pythagoreanEditor(0).getTooltip().contains("Rounded from CSV")
        || !save("paired-native-fractions-imported.png")) return false;
    calculate(false);
    if (!result.valid || importedChart || csvOutput.getText() != quarterCsv) return false;
    activeCircles()[0]->pythagoreanEditor(0).setText("-1/3", juce::sendNotificationSync);
    if (result.valid || activeCircles()[0]->pythagoreanEditor(0).findColour(juce::ComboBox::textColourId) != ink
        || activeCircles()[0]->pythagoreanEditor(0).getTooltip().contains("Rounded from CSV")) return false;
    // Contributions in the two columns add in physical cents.
    setPreset(false);
    activeCircles()[0]->editor(0).setText("1-1/11", juce::sendNotificationSync);
    activeCircles()[0]->pythagoreanEditor(0).setText("-1/12", juce::sendNotificationSync);
    calculate(true);
    if (!result.valid || activeCircles()[0]->editor(0).getText() != "1-1/11") return false;
    const double intervalDifference = result.cents[7] - result.cents[0];
    const double expected = temperament::pureInterval(mode) - 700 + (10.0 / 11) * temperament::syntonicComma() - temperament::pythagoreanComma() / 12;
    if (std::abs(intervalDifference - expected) > 1e-8 || !save("syntonic-expressions.png")) return false;
    activeCircles()[0]->pythagoreanEditor(0).setText("1/(1-1)", juce::sendNotificationSync);
    calculate(false);
    if (result.valid) return false;
    setMode(temperament::Mode::pythagoreanFifths);
    csvInput.setText(pythagoreanCsv, false);
    importCsv();
    if (mode != temperament::Mode::pythagoreanFifths || !importedChart || !result.valid
        || csvOutput.getText() != pythagoreanCsv || !save("fifths-imported.png")) return false;
    for (size_t i = 0; i < 12; ++i)
        if (activeCircles()[0]->editor(i).getText() != (i == 6 ? "-1" : "0")) return false;
    if (activeCircles()[0]->editor(0).findColour(juce::ComboBox::textColourId) != amber) return false;
    const auto validSignature = inputSignature();
    csvInput.setText("0;broken", false);
    importCsv();
    if (!result.valid || inputSignature() != validSignature || csvOutput.getText() != pythagoreanCsv) return false;
    csvInput.setText("0;4;-1;2;-3;5;-2;1;3;0;-4;2", false);
    importCsv();
    if (!importedChart || !result.valid || !statusError || !save("fifths-imported-rounded.png")) return false;
    calculate(false);
    if (result.valid || importedChart || csvOutput.getText().isNotEmpty()) return false;
    setPreset(true);
    if (activeCircles()[0]->editor(0).findColour(juce::ComboBox::textColourId) != ink) return false;
    // Automatic selection remains a live mode, and balances only selected fields.
    setPreset(false);
    for (const auto i : { 0u, 3u, 7u, 10u })
    {
        auto& box = activeCircles()[0]->editor(i);
        if (box.getItemText(0) != "Automatic calculation") return false;
        box.setSelectedId(CircleEditor::automaticItemId, juce::sendNotificationSync);
        if (box.getText() != "Auto") return false;
    }
    calculate(false);
    if (!result.valid || result.cents[9] != 0 || !activeCircles()[0]->editor(0).getTooltip().contains("Closest fraction: = -1/4 Pythagorean")
        || !save("automatic-pythagorean.png")) return false;
    const auto autoSignature = inputSignature();
    calculate(true);
    if (!result.valid || inputSignature() != autoSignature || activeCircles()[0]->editor(6).getText() != "0") return false;
    activeCircles()[0]->editor(6).setText("-2", juce::sendNotificationSync);
    if (result.valid || csvOutput.getText().isNotEmpty()
        || activeCircles()[0]->editor(0).getTooltip().contains("Calculated value:")) return false;
    calculate(true);
    if (!result.valid || activeCircles()[0]->editor(6).getText() != "-2"
        || !activeCircles()[0]->editor(0).getTooltip().contains("Closest fraction: = 1/4 Pythagorean")) return false;
    activeCircles()[0]->editor(6).setText("1/0", juce::sendNotificationSync);
    calculate(true);
    if (result.valid || activeCircles()[0]->editor(6).getText() != "1/0") return false;
    for (size_t i = 0; i < 12; ++i) activeCircles()[0]->editor(i).setText("Auto", juce::sendNotificationSync);
    calculate(false);
    if (!result.valid) return false;
    for (const auto value : result.cents) if (std::abs(value) > 1e-8) return false;
    setMode(temperament::Mode::syntonicFifths);
    setPreset(false);
    activeCircles()[0]->editor(0).setSelectedId(CircleEditor::automaticItemId, juce::sendNotificationSync);
    activeCircles()[0]->pythagoreanEditor(0).setSelectedId(CircleEditor::automaticItemId, juce::sendNotificationSync);
    activeCircles()[0]->pythagoreanEditor(2).setSelectedId(CircleEditor::automaticItemId, juce::sendNotificationSync);
    activeCircles()[0]->editor(4).setText("1-1/11", juce::sendNotificationSync);
    activeCircles()[0]->pythagoreanEditor(6).setText("-1/4", juce::sendNotificationSync);
    const auto pairedAutoSignature = inputSignature();
    calculate(true);
    if (!result.valid || inputSignature() != pairedAutoSignature || !save("automatic-paired.png")) return false;
    const double share = (-0.75 * temperament::pythagoreanComma() - (10.0 / 11) * temperament::syntonicComma()) / 3;
    const double pureDeviation = temperament::pureInterval(mode) - 700;
    if (std::abs(result.cents[7] - result.cents[0] - pureDeviation - 2 * share) > 1e-8
        || std::abs(result.cents[9] - result.cents[2] - pureDeviation - share) > 1e-8
        || !activeCircles()[0]->editor(0).getTooltip().contains("Calculated value:")
        || !activeCircles()[0]->pythagoreanEditor(0).getTooltip().contains("Calculated value:")) return false;
    const auto approxSign = juce::String::charToString(0x2248);
    for (auto* box : { &activeCircles()[0]->editor(0), &activeCircles()[0]->pythagoreanEditor(0) })
        if (!box->getTooltip().contains("Syntonic / diatonic: " + approxSign)
            || !box->getTooltip().contains("Pythagorean / ditonic: " + approxSign)
            || !box->getTooltip().contains("fraction minus actual:")) return false;
    // Ratios are display estimates only: repeated calculation preserves exact Auto cents/CSV.
    const auto beforeRatioRecalculation = csvOutput.getText();
    calculate(false);
    if (!result.valid || csvOutput.getText() != beforeRatioRecalculation || inputSignature() != pairedAutoSignature) return false;
    // Mode switching preserves the persistent Auto selections and their results.
    const auto pairedAutoCsv = csvOutput.getText();
    setMode(temperament::Mode::pythagoreanFifths);
    if (!result.valid || activeCircles()[0]->editor(0).getText() != "Auto") return false;
    setMode(temperament::Mode::syntonicFifths);
    if (!result.valid || csvOutput.getText() != pairedAutoCsv) return false;
    csvInput.setText(quarterCsv, false);
    importCsv();
    if (!result.valid || activeCircles()[0]->hasAutomatic()) return false;
    const auto importedHarmonyCsv = csvOutput.getText();
    const auto importedHarmonySignature = inputSignature();
    showHarmony();
    if (!harmonyVisible || !harmony.isVisible() || csvInput.isVisible() || closeButton.isVisible()
        || !harmony.chart().valid || !importedChart || !save("harmony-mixed.png")) return false;
    if (!harmony.selectAt({ 440, 463 }) || harmony.selectedRoot() != 0 || !harmony.selectedMinor()) return false;
    if (!harmony.selectAt({ 440, 389 }) || harmony.selectedRoot() != 0 || harmony.selectedMinor()) return false;
    if (!harmony.selectAt({ 510, 426 }) || harmony.selectedRoot() != 7) return false;
    const auto analysedInterval = harmony.chart().intervals[0][0].errorCents;
    harmony.sensitivityControl().setSelectedId(1, juce::sendNotificationSync);
    if (harmony.chart().intervals[0][0].errorCents != analysedInterval || csvOutput.getText() != importedHarmonyCsv) return false;
    if (harmony.colourLimits().fifths.good != 1 || harmony.colourLimits().thirds.good != 8) return false;
    harmony.thirdsSensitivityControl().setSelectedId(3, juce::sendNotificationSync);
    if (harmony.colourLimits().fifths.good != 1 || harmony.colourLimits().thirds.good != 12
        || csvOutput.getText() != importedHarmonyCsv) return false;
    harmony.thirdsSensitivityControl().setSelectedId(2, juce::sendNotificationSync);
    harmony.sensitivityControl().setSelectedId(2, juce::sendNotificationSync);
    setMode(temperament::Mode::syntonicFifths);
    if (harmonyVisible || !importedChart || csvOutput.getText() != importedHarmonyCsv || inputSignature() != importedHarmonySignature) return false;
    // A CSV chart remains usable even when its independently rounded comma circle is open.
    csvInput.setText("0;4;-1;2;-3;5;-2;1;3;0;-4;2", false);
    importCsv();
    const auto nonclosingCsv = csvOutput.getText();
    showHarmony();
    if (!harmony.chart().valid || !importedChart || csvOutput.getText() != nonclosingCsv) return false;
    setMode(temperament::Mode::syntonicFifths);
    activeCircles()[0]->editor(0).setText("1/0", juce::sendNotificationSync);
    showHarmony();
    if (harmony.chart().valid || !harmonyVisible || !save("harmony-invalid.png")) return false;
    setMode(temperament::Mode::pythagoreanFifths);
    setPreset(true);
    showHarmony(); harmony.selectChord(0, false);
    if (!harmony.chart().valid || !save("harmony-equal.png")) return false;
    setMode(temperament::Mode::pythagoreanFifths);
    setPreset(false); calculate(true);
    showHarmony(); harmony.selectChord(6, false);
    if (temperament::consonance(harmony.chart().triads[12], harmony.colourLimits()) != temperament::Consonance::rough
        || !save("harmony-wolf.png")) return false;
    setMode(temperament::Mode::syntonicFifths);
    setPreset(false);
    for (size_t i = 0; i < 12; ++i) activeCircles()[0]->editor(i).setText("-1/4", juce::sendNotificationSync);
    calculate(true); showHarmony(); harmony.selectChord(0, false);
    if (!harmony.chart().valid || std::abs(harmony.chart().intervals[1][0].errorCents) > 1e-7
        || !save("harmony-meantone.png")) return false;
    const auto contextualCsv = csvOutput.getText();
    harmony.selectChord(9, false);
    if (harmony.displayedNoteName(1) != "C#" || harmony.displayedNoteName(9) != "A"
        || !save("harmony-a-major.png")) return false;
    harmony.selectChord(10, true);
    if (harmony.displayedNoteName(1) != "Db" || harmony.displayedNoteName(10) != "Bb"
        || !save("harmony-bb-minor.png")) return false;
    harmony.selectChord(6, false);
    if (harmony.displayedNoteName(5) != "E#" || csvOutput.getText() != contextualCsv) return false;
    if (auto output = directory.getChildFile("app-icon.png").createOutputStream())
    {
        output->setPosition(0); output->truncate();
        if (!juce::PNGImageFormat().writeImageToStream(appIcon(), *output)) return false;
    }
    else return false;
    return directory.getChildFile("ui-checks.txt").replaceWithText("PASS: existing math, Auto, CSV and harmony regressions; independent fifth/third sensitivities preserve tuning; A major uses C#, Bb minor uses Db, F# major uses E#; contextual labels and thicker lattice rendering; imported chart preservation, stale-output protection, triangle/note selection, and icon rendering.\n");
}
