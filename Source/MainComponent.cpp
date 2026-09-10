// Copyright (C) 2026 zurek-jiri and contributors.
// SPDX-License-Identifier: AGPL-3.0-only
// See LICENSE and COPYRIGHT for terms and the no-warranty notice.

#include "MainComponent.h"
#include "ClosingPanel.h"
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
juce::Font StudioLookAndFeel::getComboBoxFont(juce::ComboBox& box) { return font(box.getHeight()<36?22.0f:24.0f, true); }
juce::Font StudioLookAndFeel::getTextButtonFont(juce::TextButton&, int height) { return font(height<40?22.0f:26.0f, true); }
juce::Font StudioLookAndFeel::getPopupMenuFont() { return font(26); }
void StudioLookAndFeel::drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,bool highlighted,bool down)
{
    const float tick=juce::jmin(30.0f,button.getHeight()-6.0f);
    drawTickBox(g,button,2,(button.getHeight()-tick)*0.5f,tick,tick,button.getToggleState(),button.isEnabled(),highlighted,down);
    g.setFont(font(24));g.setColour(button.isEnabled()?ink:muted.withAlpha(0.6f));
    g.drawFittedText(button.getButtonText(),40,0,button.getWidth()-42,button.getHeight(),juce::Justification::centredLeft,button.getHeight()>44?2:1,1.0f);
}
juce::Label* StudioLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label=juce::LookAndFeel_V4::createSliderTextBox(slider);label->setFont(font(22));return label;
}
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
    for (auto* button : { &fifthsButton, &syntonicButton, &harmonyButton, &aboutButton, &equalButton, &pureButton, &closeButton, &calculateButton, &rotateLeftButton, &rotateRightButton, &knownDetailsButton, &copyButton, &importButton })
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
    fifthsButton.setTooltip("Compact formula boxes around the circle. Both layouts edit the same temperament and support P, S and schisma.");
    syntonicButton.setTooltip("Wide formula fields to the right of the circle, for longer expressions. Both layouts edit the same temperament and support P, S and schisma.");
    harmonyButton.onClick = [this] { showHarmony(); };
    aboutButton.onClick = [this] {
        juce::DialogWindow::LaunchOptions options;
        auto* aboutPanel = new AboutPanel();
        const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
        const auto area = display != nullptr ? display->userArea : juce::Rectangle<int>(0, 0, 1280, 900);
        aboutPanel->setSize(1060, juce::jlimit(400, 780, area.getHeight() - 100));
        auto* viewport = new juce::Viewport();
        viewport->setViewedComponent(aboutPanel, true);
        viewport->setScrollBarThickness(24);
        viewport->setSize(juce::jmin(1084, area.getWidth() - 80), aboutPanel->getHeight());
        options.content.setOwned(viewport);
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
    closeButton.onClick = [this] { showClosingOptions(); };
    calculateButton.onClick = [this] { calculate(false); };
    rotateLeftButton.onClick = [this] { rotateCircle(-1); };
    rotateRightButton.onClick = [this] { rotateCircle(1); };
    rotateLeftButton.setTooltip("Rotate all fractions, including Auto, one fifth counter-clockwise: G-D moves to C-G. Recalculate with A = 0.");
    rotateRightButton.setTooltip("Rotate all fractions, including Auto, one fifth clockwise: C-G moves to G-D. Recalculate with A = 0.");
    knownDetailsButton.onClick = [this] { showCatalogueMatches(); };
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
    pureButton.setTooltip("Set every fifth to pure (0 comma). Use Close circle to choose how to distribute the required correction.");
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
    csvInput.setTooltip("Your original pasted values, kept for comparison with CSV out. Calculations, edits and rotation never replace or clear this text.");
    csvInput.setTextToShowWhenEmpty("Paste 12 semicolon-separated cent values here", muted);
    csvInput.onReturnKey = [this] { importCsv(); };
    csvInput.onTextChange = [this] { csvInput.setColour(juce::TextEditor::outlineColourId, border); };
    importButton.setTooltip("Reconstruct fifth corrections using the selected method. P = Pythagorean, S = syntonic, schisma = P - S. The chart retains the imported values until Calculate chart or Apply.");
    addAndMakeVisible(csvInput);
    reconstruction.addItem("Nearest fractions",1);
    reconstruction.addItem("Precise simple expressions",2);
    reconstruction.setSelectedId(1,juce::dontSendNotification);
    reconstruction.setTitle("CSV reconstruction method");
    reconstruction.setTooltip("Nearest uses menu fractions and small multiples, including 1/24. Precise prefers small expressions within CSV rounding precision: 0.0005 ct per note, up to 0.001 ct per fifth. schisma = P - S; H remains an accepted alias.");
    addAndMakeVisible(reconstruction);
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
    harmony.onColoursChanged=[this] {
        for(auto& group:circles) for(auto& ring:group) ring->setChart(result.cents,result.valid&&!dirty,harmony.colourLimits());
    };
    for(auto* label:{&knownLabel,&statusLabel})
    {
        label->setFont(font(22));label->setMinimumHorizontalScale(1.0f);addAndMakeVisible(label);
        label->setBorderSize(juce::BorderSize<int>());
    }
    addMouseListener(this,true);
    pureButton.setButtonText("Pure fifths");
    setSize(1480, 900);
    setMode(mode);
    calculate(false);
}
MainComponent::~MainComponent() { setLookAndFeel(nullptr); }
std::vector<std::unique_ptr<CircleEditor>>& MainComponent::activeCircles()
{
    return circles[mode == temperament::Mode::pythagoreanFifths ? 0 : 1];
}
void MainComponent::setMode(temperament::Mode selected)
{
    harmonyVisible = false;
    mode = selected;
    // The tabs are two input layouts for one working temperament. Switching views
    // must not replace an imported chart with a second, stale set of fractions.
    if(!dirty) calculatedSignature=inputSignature();
    closeButton.setTooltip("Preview closing one fifth, selected fifths, all equally, Auto fields, or an inferred fit with simple fractions.");
    calculateButton.setTooltip("Calculate the chart. Automatic calculation fields share the missing or excess correction equally in cents so the circle closes.");
    updateView();
    refreshHarmony();
}
void MainComponent::updateView()
{
    const bool fifths = mode == temperament::Mode::pythagoreanFifths;
    fifthsButton.setToggleState(!harmonyVisible && fifths, juce::dontSendNotification);
    syntonicButton.setToggleState(!harmonyVisible && !fifths, juce::dontSendNotification);
    harmonyButton.setToggleState(harmonyVisible, juce::dontSendNotification);
    for (auto* button : { &equalButton, &pureButton, &closeButton, &calculateButton, &rotateLeftButton, &rotateRightButton, &knownDetailsButton, &copyButton, &importButton })
        button->setVisible(!harmonyVisible);
    csvInput.setVisible(!harmonyVisible); csvOutput.setVisible(!harmonyVisible);
    reconstruction.setVisible(!harmonyVisible);
    for (size_t m = 0; m < circles.size(); ++m)
        for (auto& ring : circles[m]) ring->setVisible(!harmonyVisible && m == (fifths ? 0u : 1u));
    harmony.setVisible(harmonyVisible);
    knownLabel.setVisible(!harmonyVisible);statusLabel.setVisible(!harmonyVisible);
    resized(); repaint();
}
void MainComponent::refreshHarmony()
{
    const auto source = (importedChart ? juce::String("Imported CSV") : juce::String("Calculated chart"))
        + " / " + (mode == temperament::Mode::pythagoreanFifths ? fifthsButton.getButtonText() : syntonicButton.getButtonText());
    harmony.setChart(result.cents, result.valid && !dirty, result.valid && !dirty ? source : "No current chart / return to a fifths tab to calculate");
    for(auto& group:circles) for(auto& ring:group) ring->setChart(result.cents,result.valid&&!dirty,harmony.colourLimits());
    refreshRecognition();
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
    synchroniseOtherCircle();
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
    synchroniseOtherCircle();
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
    status = rings.front()->hasAutomatic() ? rings.front()->automaticSummary()
        : closeCircles ? "Closing intervals adjusted. Chart calculated with A = 0.000 cents."
                         : "Chart calculated. All circles close; A = 0.000 cents.";
    calculatedSignature = inputSignature();
    csvOutput.setText(temperament::csv(result.cents), false);
    copyButton.setEnabled(true);
    synchroniseOtherCircle();
    refreshHarmony();
    repaint();
}
void MainComponent::importCsv()
{
    const bool precise=reconstruction.getSelectedId()==2;
    const auto imported = temperament::reverseCsv(csvInput.getText().toStdString(), mode,
        precise?temperament::Reconstruction::precise:temperament::Reconstruction::nearestFraction);
    if (!imported.valid)
    {
        csvInput.setColour(juce::TextEditor::outlineColourId, red);
        status = "CSV import: " + juce::String(imported.error);
        statusError = true;
        repaint();
        return;
    }
    activeCircles().front()->setFromCsv(imported);
    synchroniseOtherCircle();
    result = {};
    result.valid = true;
    result.cents = imported.cents;
    result.closureErrors = { imported.closureError };
    importedChart = true;
    dirty = false;
    calculatedSignature = inputSignature();
    csvOutput.setText(temperament::csv(result.cents), false);
    csvInput.setColour(juce::TextEditor::outlineColourId, border);
    csvInput.setCaretPosition(0);
    copyButton.setEnabled(true);
    const bool closes = std::abs(imported.closureError) <= 0.000001;
    double maximum=0;for(double error:imported.intervalErrors) maximum=std::max(maximum,std::abs(error));
    statusError = maximum>0.0010001;
    status = juce::String(precise?(maximum<=0.0010001?"Within CSV precision: ":"Best simple approximation: "):"Nearest fractions: ")+"max fifth error "+juce::String(maximum,6)+" ct. "
        +(closes?"Formulas close.":"Use Close circle to balance formulas.")+" Chart retains CSV values.";
    refreshHarmony();
    repaint();
}
void MainComponent::showClosingOptions()
{
    temperament::ClosingRequest request;
    request.mode=mode;request.expressions=activeCircles().front()->expressions();
    request.useTargets=importedChart&&result.valid&&!dirty&&inputSignature()==calculatedSignature;
    if(request.useTargets)
    {
        const auto& notes=temperament::cycles(mode).front().notes;
        for(int i=0;i<12;++i) request.targets[i]=700+result.cents[notes[(i+1)%12]]-result.cents[notes[i]]-temperament::pureInterval(mode);
    }
    const auto signature=inputSignature();
    const juce::Component::SafePointer<MainComponent> safe(this);
    juce::DialogWindow::LaunchOptions options;
    auto* closingPanel=new ClosingPanel(request,[safe,signature](const auto& proposal) {
        if(safe==nullptr) return;
        if(safe->inputSignature()!=signature) {safe->status="Inputs changed. Open Close circle again for a new preview.";safe->repaint();return;}
        safe->activeCircles().front()->setExpressions(proposal.expressions);safe->calculate(false);
    });
    // Keep the large controls usable even on a short display.
    auto* viewport=new juce::Viewport();
    viewport->setViewedComponent(closingPanel,true);viewport->setScrollBarThickness(24);
    const auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
    const auto area=display!=nullptr?display->userArea:juce::Rectangle<int>(0,0,1280,900);
    closingPanel->setSize(960,juce::jlimit(560,720,area.getHeight()-100));
    viewport->setSize(juce::jmin(984,area.getWidth()-80),closingPanel->getHeight());
    options.content.setOwned(viewport);
    options.dialogTitle="Close circle";options.dialogBackgroundColour=background;
    options.escapeKeyTriggersCloseButton=true;options.useNativeTitleBar=true;options.resizable=false;
    options.componentToCentreAround=this;options.launchAsync();
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
void MainComponent::synchroniseOtherCircle()
{
    auto& other=circles[mode==temperament::Mode::pythagoreanFifths?1:0];
    other.front()->copyFrom(*activeCircles().front());
}
void MainComponent::rotateCircle(int clockwiseSteps)
{
    if(inputSignature()!=calculatedSignature) markDirty();
    const bool keepCsv=importedChart&&result.valid&&!dirty;
    activeCircles().front()->rotate(clockwiseSteps);
    if(keepCsv)
    {
        result.cents=temperament::rotateChartFifths(result.cents,clockwiseSteps);
        calculatedSignature=inputSignature();
        csvOutput.setText(temperament::csv(result.cents),false);
        synchroniseOtherCircle();
        refreshHarmony();
    }
    else calculate(false);
    status="Rotated one fifth "+juce::String(clockwiseSteps>0?"clockwise. ":"counter-clockwise. ")
        +(keepCsv?"Chart retains rotated CSV values; Calculate chart applies the formulas.":status);
    repaint();
}

void MainComponent::refreshRecognition()
{
    knownDetailsButton.setEnabled(false);
    knownDetailsButton.setButtonText("Suggestions");
    if(!result.valid||dirty) {knownLabel.setText({},juce::dontSendNotification);knownLabel.setTooltip({});return;}
    const auto matches=temperament::recogniseTemperament(result.cents);
    if(matches.empty())
    {
        knownLabel.setText("Catalogue comparison unavailable",juce::dontSendNotification);
        knownLabel.setColour(juce::Label::textColourId,muted);
        knownLabel.setTooltip("A valid finite tuning chart and catalogue are needed for comparison.");
        return;
    }
    const auto& first=matches.front();
    const auto& catalogue=temperament::knownTemperaments();
    juce::String label=first.maximumError<=0.0010001?"Match: ":first.maximumError<=1.000000001?"Near: ":"Compare: ";
    label+=juce::String(catalogue[first.index].name);
    label+=" / "+juce::String(temperament::rotationDescription(first.rotationFifths));
    label+=" / max "+juce::String(first.maximumError,3)+" ct";
    if(matches.size()>1) label+=" / +"+juce::String(static_cast<int>(matches.size()-1))+" more";
    knownLabel.setText(label,juce::dontSendNotification);knownLabel.setColour(juce::Label::textColourId,first.maximumError<=1.000000001?mint:ink);
    knownLabel.setTooltip(catalogueMatchDetails());
    knownDetailsButton.setEnabled(true);
    knownDetailsButton.setButtonText("Suggestions ("+juce::String(static_cast<int>(matches.size()))+")");
}
juce::String MainComponent::catalogueMatchDetails() const
{
    if(!result.valid||dirty) return "Calculate or import a temperament first.";
    const auto matches=temperament::recogniseTemperament(result.cents);
    const auto& catalogue=temperament::knownTemperaments();
    juce::String details="Catalogue suggestions\n\nMatches (up to 0.001 ct per note) and near matches (up to 1 ct) come first. "
        "Then the closest harmonic alternatives bring the list to at least five names. Each alternative uses its best fifth rotation.\n\n"
        "Harmonic distance compares all fifths and major/minor thirds: lower is closer. Fifths carry half the weight; "
        "each third family carries a quarter. Differences below are between your tuning and the catalogue, not errors from pure intervals. "
        "Alternatives may be distant: resemblance does not establish historical identity.\n\n";
    for(const auto& match:matches)
    {
        const auto& known=catalogue[match.index];
        details+=(match.maximumError<=0.0010001?"Match: ":match.maximumError<=1.000000001?"Near: ":"Compare: ")
            +juce::String(known.name)+"\n"+juce::String(temperament::rotationDescription(match.rotationFifths))
            +"; maximum difference "+juce::String(match.maximumError,3)+" ct; RMS "+juce::String(match.rmsError,3)+" ct.\n"
            +"Harmonic distance "+juce::String(match.similarityError,3)+" ct; interval RMS: fifths "
            +juce::String(match.fifthRmsError,3)+", major thirds "+juce::String(match.majorThirdRmsError,3)
            +", minor thirds "+juce::String(match.minorThirdRmsError,3)+" ct.\n";
        const auto comment=juce::String(known.comments).trim();
        if(comment.isNotEmpty()) details+="Note: "+comment+"\n";
        details+="\n";
    }
    return details;
}
void MainComponent::showCatalogueMatches()
{
    auto* body=new juce::TextEditor();
    body->setMultiLine(true,true);body->setReadOnly(true);body->setCaretVisible(false);
    body->setScrollbarsShown(true);body->setFont(font(24));body->setIndents(18,18);
    body->setColour(juce::TextEditor::backgroundColourId,background);
    body->setColour(juce::TextEditor::textColourId,ink);
    body->setText(catalogueMatchDetails(),false);body->setCaretPosition(0);
    const auto* display=juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds());
    const auto area=display!=nullptr?display->userArea:juce::Rectangle<int>(0,0,1280,900);
    body->setSize(juce::jmin(1080,area.getWidth()-80),juce::jmin(680,area.getHeight()-120));
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(body);options.dialogTitle="Catalogue suggestions";
    options.dialogBackgroundColour=background;options.escapeKeyTriggersCloseButton=true;
    options.useNativeTitleBar=true;options.resizable=true;options.componentToCentreAround=this;
    options.launchAsync();
}
void MainComponent::mouseDown(const juce::MouseEvent& event)
{
    if(harmonyVisible) return;
    const auto position=event.getEventRelativeTo(this).position;
    for(auto& circle:activeCircles())
    {
        const auto relative=position-circle->getPosition().toFloat();
        if(!circle->containsTone(relative)&&relative.getDistanceFrom(circle->ringBounds().getCentre())>circle->ringBounds().getWidth()*0.5f)
            circle->clearSelection();
    }
}
void MainComponent::resized()
{
    const int w=getWidth(),h=getHeight();
    aboutButton.setBounds(w-202,52,186,36);
    fifthsButton.setBounds(16,52,236,36);syntonicButton.setBounds(260,52,258,36);
    harmonyButton.setBounds(526,52,192,36);
    equalButton.setBounds(726,52,192,36);pureButton.setBounds(926,52,136,36);
    harmony.setBounds(16,100,w-32,h-112);
    for(auto& group:circles) for(auto& circle:group) circle->setBounds(16,96,w-32,h-322);
    const auto& circle=*activeCircles().front();
    const int buttonWidth=mode==temperament::Mode::pythagoreanFifths?190:180;
    const int left=circle.getX()+4,right=circle.getX()+circle.circleAreaWidth()-buttonWidth-4;
    const int top=circle.getY()+2,bottom=circle.getBottom()-32;
    rotateLeftButton.setBounds(left,top,buttonWidth,36);rotateRightButton.setBounds(right,top,buttonWidth,36);
    closeButton.setBounds(left,bottom,buttonWidth,32);calculateButton.setBounds(right,bottom,buttonWidth,32);
    for(auto* button:{&rotateLeftButton,&rotateRightButton,&closeButton,&calculateButton}) button->toFront(false);
    statusLabel.setBounds(20,h-216,w-40,26);knownLabel.setBounds(20,h-188,w-264,28);
    knownDetailsButton.setBounds(w-236,h-188,220,28);
    reconstruction.setBounds(w-404,h-80,246,34);
    csvInput.setBounds(98,h-80,w-510,34);importButton.setBounds(w-150,h-80,134,34);
    csvOutput.setBounds(98,h-40,w-256,34);copyButton.setBounds(w-150,h-40,134,34);
    importButton.setButtonText("Import CSV");
    for(auto* editor:{&csvInput,&csvOutput})
    {
        editor->setFont(juce::Font(juce::FontOptions(ui::monoFont(),22.0f,juce::Font::plain)));
        editor->setIndents(8,5);
    }
}
void MainComponent::paint(juce::Graphics& g)
{
    const int w=getWidth(),h=getHeight();
    g.fillAll(background);
    g.drawImageWithin(appIcon(),16,8,36,36,juce::RectanglePlacement::centred);
    text(g,"Temperament Generator",{62,8,462,36},32,ink,juce::Justification::centredLeft,true);
    text(g,"Cents vs equal / A = 0",{532,8,w-548,36},22,mint,juce::Justification::centredLeft);
    if(harmonyVisible) return;
    card(g,{12,92,w-24,h-314});
    statusLabel.setText(status,juce::dontSendNotification);statusLabel.setTooltip(status);
    statusLabel.setColour(juce::Label::textColourId,statusError?red:dirty?amber:mint);
    drawResults(g);
    text(g,"CSV in",{16,h-80,80,34},22,mint,juce::Justification::centredLeft,true);
    text(g,"CSV out",{16,h-40,80,34},22,mint,juce::Justification::centredLeft,true);
}
void MainComponent::drawResults(juce::Graphics& g)
{
    const int h=getHeight();
    card(g,{12,h-154,getWidth()-24,66});
    const float cell=static_cast<float>(getWidth()-40)/12;
    for(int i=0;i<12;++i)
    {
        const int x=20+juce::roundToInt(i*cell),width=juce::roundToInt(cell);
        text(g,temperament::noteNames()[static_cast<size_t>(i)],{x,h-153,width,28},22,i==9?mint:muted,juce::Justification::centred,true);
        text(g,result.valid?cents(result.cents[static_cast<size_t>(i)],true):"--",{x,h-125,width,34},24,ink,juce::Justification::centred);
    }
}

bool MainComponent::renderPreviews(const juce::File& directory)
{
    if(directory.createDirectory().failed()) return false;
    auto snapshot=[&directory](juce::Component& component,const juce::String& name) {
        auto output=directory.getChildFile(name).createOutputStream();
        if(!output) return false;
        output->setPosition(0);output->truncate();
        return juce::PNGImageFormat().writeImageToStream(component.createComponentSnapshot(component.getLocalBounds()),*output);
    };
    auto save=[&](const juce::String& name) {
        // Both bar charts must show the same current notes as the result table,
        // including imported charts that have not yet been closed or calculated.
        for(const auto& group:circles) for(const auto& circle:group)
        {
            const auto& chart=circle->centChart();
            if(chart.chartValid()!=(result.valid&&!dirty)) return false;
            if(chart.chartValid()&&(chart.chart()!=result.cents||chart.chart()[9]!=0)) return false;
        }
        if(circles[0].front()->centChart().scaleCents()!=circles[1].front()->centChart().scaleCents()) return false;
        return snapshot(*this,name);
    };
    const auto failed=[&](int line) {
        directory.getChildFile("gui-check.txt").replaceWithText("GUI check failed at MainComponent.cpp:"+juce::String(line)
            +"\nMode: "+juce::String(static_cast<int>(mode))+"\nStatus: "+status+"\nCSV: "+csvOutput.getText());
        snapshot(*this,"gui-check-failure.png");
        return false;
    };
    directory.getChildFile("gui-check.txt").replaceWithText("GUI checks started.\n");
    if(!harmony.isPlaybackSelected()||harmony.isAudioOutputOpen()) return failed(__LINE__);
    AboutPanel about;
    for(int i=0;i<3;++i) {about.showPage(i);if(!snapshot(about,"about-page-"+juce::String(i)+".png")) return failed(__LINE__);}
    // Laptop and desktop layouts must expose every control without shrinking
    // the canvas or placing formula boxes over the musical circle.
    for(const auto size:{juce::Point<int>(1280,660),juce::Point<int>(1320,660),juce::Point<int>(1320,680),juce::Point<int>(1320,800),juce::Point<int>(1480,900)})
    {
        setSize(size.x,size.y);
        for(auto selectedMode:{temperament::Mode::pythagoreanFifths,temperament::Mode::syntonicFifths})
        {
            setMode(selectedMode);setPreset(true);
            for(auto* child:getChildren()) if(child->isVisible()&&!getLocalBounds().contains(child->getBounds())) return failed(__LINE__);
            auto& circle=*activeCircles().front();
            const auto chartBounds=circle.centChart().getBounds();
            if(!circle.getLocalBounds().contains(chartBounds)||circle.centChart().plotBounds().getHeight()/12<22) return failed(__LINE__);
            for(int i=0;i<12;++i)
            {
                const auto bounds=circle.editor(i).getBounds();
                if(!circle.getLocalBounds().contains(bounds)) return failed(__LINE__);
                if(chartBounds.intersects(bounds)) return failed(__LINE__);
                if(selectedMode==temperament::Mode::syntonicFifths&&chartBounds.getRight()>=bounds.getX()) return failed(__LINE__);
                for(auto* button:{&rotateLeftButton,&rotateRightButton,&closeButton,&calculateButton})
                    if(button->getBounds().intersects(bounds.translated(circle.getX(),circle.getY()))) return failed(__LINE__);
                if(selectedMode==temperament::Mode::pythagoreanFifths)
                {
                    const auto c=circle.ringBounds().getCentre();
                    const juce::Point<float> closest(juce::jlimit(static_cast<float>(bounds.getX()),static_cast<float>(bounds.getRight()),c.x),
                        juce::jlimit(static_cast<float>(bounds.getY()),static_cast<float>(bounds.getBottom()),c.y));
                    if(closest.getDistanceFrom(c)<circle.ringBounds().getWidth()*0.5f) return failed(__LINE__);
                }
            }
            const auto signature=inputSignature();
            circle.selectAt({circle.ringBounds().getCentreX(),circle.ringBounds().getY()});
            circle.selectAt({4,4});
            if(circle.selectedTone()!=-1||inputSignature()!=signature) return failed(__LINE__);
            if(!save(juce::String(selectedMode==temperament::Mode::pythagoreanFifths?"pyth-":"combined-")+juce::String(size.x)+"x"+juce::String(size.y)+".png")) return failed(__LINE__);
        }
        showHarmony();
        for(auto* child:harmony.getChildren()) if(child->isVisible()&&!harmony.getLocalBounds().contains(child->getBounds())) return failed(__LINE__);
        if(!save("harmony-"+juce::String(size.x)+"x"+juce::String(size.y)+".png")) return failed(__LINE__);
    }
    for(auto selectedMode:{temperament::Mode::pythagoreanFifths,temperament::Mode::syntonicFifths})
    {
        setMode(selectedMode);setPreset(true);
        csvInput.clear();csvInput.setColour(juce::TextEditor::outlineColourId,border);
        auto& circle=*activeCircles().front();
        if(!result.valid||!circle.chartValid()||result.cents[9]!=0) return failed(__LINE__);
        for(int i=0;i<12;++i) if(circle.editor(i).getWidth()<(selectedMode==temperament::Mode::pythagoreanFifths?140:300)) return failed(__LINE__);
        const auto before=inputSignature();
        if(!circle.selectAt({circle.ringBounds().getCentreX(),circle.ringBounds().getY()})||circle.selectedTone()!=0||inputSignature()!=before) return failed(__LINE__);
        if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"fifths-selected.png":"syntonic-selected.png")) return failed(__LINE__);
        circle.selectAt({circle.ringBounds().getCentreX(),circle.ringBounds().getY()});
        if(circle.selectedTone()!=-1) return failed(__LINE__);
        setPreset(false);
        if(result.valid||circle.chartValid()||circle.centChart().chartValid()||csvOutput.getText().isNotEmpty()) return failed(__LINE__);
        calculate(true);
        if(!result.valid||!save(selectedMode==temperament::Mode::pythagoreanFifths?"fifths-chart.png":"syntonic-chart.png")) return failed(__LINE__);
        circle.editor(0).setText("1/0",juce::sendNotificationSync);
        if(result.valid||copyButton.isEnabled()||circle.chartValid()||circle.centChart().chartValid()) return failed(__LINE__);
        calculate(false);if(result.valid||!statusError) return failed(__LINE__);
        setPreset(true);
        auto& typed=circle.editor(0);typed.showEditor();
        juce::Label* label=nullptr;
        for(auto* child:typed.getChildren()) if(auto* found=dynamic_cast<juce::Label*>(child)) label=found;
        if(label==nullptr||label->getCurrentTextEditor()==nullptr) return failed(__LINE__);
        label->getCurrentTextEditor()->setText("-P/6",false);
        calculate(false);if(result.valid) return failed(__LINE__);
        label->hideEditor(false);calculate(true);if(!result.valid) return failed(__LINE__);
        setPreset(false);
        circle.editor(0).setText("-S/4-schisma/4",juce::sendNotificationSync);
        circle.editor(1).setText("1-1/11",juce::sendNotificationSync);
        circle.editor(4).setSelectedId(CircleEditor::automaticItemId,juce::sendNotificationSync);
        circle.editor(8).setSelectedId(CircleEditor::automaticItemId,juce::sendNotificationSync);
        calculate(false);
        if(!result.valid||!circle.editor(4).getTooltip().contains("Calculated value:")) return failed(__LINE__);
        if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"fifths-auto.png":"syntonic-auto.png")) return failed(__LINE__);
        csvInput.setText("0;4;-1;2;-3;5;-2;1;3;0;-4;2",false);
        for(int method:{1,2})
        {
            reconstruction.setSelectedId(method,juce::dontSendNotification);importCsv();
            if(!result.valid||!importedChart||!circle.chartValid()||result.cents[9]!=0) return failed(__LINE__);
            const auto source=csvOutput.getText();
            showHarmony();if(!harmony.chart().valid) return failed(__LINE__);
            setMode(selectedMode);if(csvOutput.getText()!=source||!importedChart) return failed(__LINE__);
            const auto formulas=circle.expressions();
            setMode(selectedMode==temperament::Mode::pythagoreanFifths?temperament::Mode::syntonicFifths:temperament::Mode::pythagoreanFifths);
            if(csvOutput.getText()!=source||!importedChart||!result.valid) return failed(__LINE__);
            for(int i=0;i<12;++i)
            {
                double a=0,b=0;std::string error;
                if(!temperament::evaluateExpression(formulas[i],selectedMode,a,error)
                    ||!temperament::evaluateExpression(activeCircles().front()->expressions()[i],mode,b,error)
                    ||std::abs(a*temperament::commaSize(selectedMode)-b*temperament::commaSize(mode))>1e-8) return failed(__LINE__);
            }
            setMode(selectedMode);
            if(circle.expressions()!=formulas) return failed(__LINE__);
            if(!save(juce::String(selectedMode==temperament::Mode::pythagoreanFifths?"pyth-":"synt-")+(method==1?"csv-nearest.png":"csv-precise.png"))) return failed(__LINE__);
        }
        const auto validOutput=csvOutput.getText();
        csvInput.setText("broken CSV",false);importCsv();
        if(csvOutput.getText()!=validOutput||!result.valid) return failed(__LINE__);
    }
    // The pasted row is a reference for comparing input with output, even when
    // A is nonzero, formatting differs, or the reconstructed circle cannot close.
    const juce::String pastedReference=" 6.4000; 3.5000;8.3000;3.3000;3.4000;8.9000;5.7000;2.4000;-0.2000;5.0000;10.3000;0.6000 ";
    for(auto selectedMode:{temperament::Mode::pythagoreanFifths,temperament::Mode::syntonicFifths})
    {
        setMode(selectedMode);
        reconstruction.setSelectedId(1,juce::dontSendNotification);
        csvInput.setText(pastedReference,false);importCsv();
        if(!result.valid||csvInput.getText()!=pastedReference||csvOutput.getText()==pastedReference) return failed(__LINE__);
        const auto importedOutput=csvOutput.getText();
        rotateRightButton.onClick();
        if(csvInput.getText()!=pastedReference||csvOutput.getText()==importedOutput) return failed(__LINE__);
        rotateLeftButton.onClick();
        calculateButton.onClick();
        if(result.valid||!statusError||csvInput.getText()!=pastedReference||csvOutput.getText().isNotEmpty()) return failed(__LINE__);
        showHarmony();
        if(csvInput.getText()!=pastedReference) return failed(__LINE__);
        const auto otherMode=selectedMode==temperament::Mode::pythagoreanFifths?temperament::Mode::syntonicFifths:temperament::Mode::pythagoreanFifths;
        setMode(otherMode);
        if(csvInput.getText()!=pastedReference||result.valid) return failed(__LINE__);
        activeCircles().front()->editor(0).setSelectedId(CircleEditor::automaticItemId,juce::sendNotificationSync);
        if(csvInput.getText()!=pastedReference) return failed(__LINE__);
        calculateButton.onClick();
        if(!result.valid||csvInput.getText()!=pastedReference||csvOutput.getText().isEmpty()) return failed(__LINE__);
        const auto calculatedOutput=csvOutput.getText();
        setMode(selectedMode);
        if(csvInput.getText()!=pastedReference||csvOutput.getText()!=calculatedOutput
            ||activeCircles().front()->editor(0).getText()!="Auto") return failed(__LINE__);
        setSize(1320,680);
        if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"csv-comparison-around.png":"csv-comparison-list.png")) return failed(__LINE__);
        rotateRightButton.onClick();rotateLeftButton.onClick();
        if(csvInput.getText()!=pastedReference||csvOutput.getText()!=calculatedOutput) return failed(__LINE__);
        setPreset(true);
        if(csvInput.getText()!=pastedReference||!result.valid) return failed(__LINE__);
        setPreset(false);
        if(csvInput.getText()!=pastedReference||result.valid) return failed(__LINE__);
        calculate(true);
        if(csvInput.getText()!=pastedReference||!result.valid) return failed(__LINE__);
        activeCircles().front()->editor(0).setText("1/0",juce::sendNotificationSync);calculate(false);
        if(csvInput.getText()!=pastedReference||result.valid) return failed(__LINE__);
        // Even an invalid newly pasted row stays available for the user to edit.
        csvInput.setText("broken CSV",false);importCsv();calculate(false);
        if(csvInput.getText()!="broken CSV") return failed(__LINE__);
    }
    setMode(temperament::Mode::syntonicFifths);setPreset(false);
    csvInput.setText("10.265;-13.686;3.422;20.530;-3.421;13.686;-10.265;6.843;-17.108;0.000;17.108;-6.843",false);
    reconstruction.setSelectedId(2,juce::dontSendNotification);importCsv();
    if(!knownLabel.getText().contains("Meantone (-1/4)")) return failed(__LINE__);
    for(int i=0;i<12;++i) if(i!=8&&activeCircles().front()->editor(i).getText()!="-S/4") return failed(__LINE__);
    setSize(1320,680);
    for(auto& group:circles)
    {
        auto& chart=group.front()->centChart();
        const auto plot=chart.plotBounds();
        const auto rendered=chart.createComponentSnapshot(chart.getLocalBounds());
        const int zero=juce::roundToInt(plot.getCentreX());
        const int cRow=juce::roundToInt(plot.getY()+plot.getHeight()/24);
        const int cisRow=juce::roundToInt(plot.getY()+3*plot.getHeight()/24);
        // The known meantone fixture has a sharp C and a flat C#. Check the
        // pixels on both sides of the zero axis, independent of formula units.
        if(rendered.getPixelAt(zero+5,cRow)!=juce::Colour(0xfff4b4d8)
            ||rendered.getPixelAt(zero-5,cRow)==juce::Colour(0xfff4b4d8)
            ||rendered.getPixelAt(zero-5,cisRow)!=juce::Colour(0xff8dbfff)
            ||rendered.getPixelAt(zero+5,cisRow)==juce::Colour(0xff8dbfff)) return failed(__LINE__);
    }
    if(!save("meantone-laptop.png")) return failed(__LINE__);
    const auto unrotated=result.cents;
    const auto unrotatedFormulas=activeCircles().front()->expressions();
    rotateRightButton.onClick();
    if(!importedChart||result.cents!=temperament::rotateChartFifths(unrotated,1)
        ||!knownLabel.getText().contains("1 fifth clockwise")) return failed(__LINE__);
    const auto rotatedFormulas=activeCircles().front()->expressions();
    for(int i=0;i<12;++i) if(rotatedFormulas[(i+1)%12]!=unrotatedFormulas[i]) return failed(__LINE__);
    if(!save("meantone-rotated-laptop.png")) return failed(__LINE__);
    rotateLeftButton.onClick();
    if(activeCircles().front()->expressions()!=unrotatedFormulas||temperament::csv(result.cents)!=temperament::csv(unrotated)) return failed(__LINE__);
    for(const auto& entry:temperament::knownTemperaments()) if(entry.name.find("Trost")!=std::string::npos)
    {
        for(auto selectedMode:{temperament::Mode::pythagoreanFifths,temperament::Mode::syntonicFifths})
        {
            setMode(selectedMode);
            reconstruction.setSelectedId(1,juce::dontSendNotification);
            csvInput.setText(temperament::csv(entry.cents),false);importCsv();
            const auto pasted=csvInput.getText();
            activeCircles().front()->editor(0).setSelectedId(CircleEditor::automaticItemId,juce::sendNotificationSync);
            calculateButton.onClick();
            if(!result.valid||!knownLabel.getText().contains("Trost")||!knownDetailsButton.isEnabled()
                ||status.contains("Closest:")||csvInput.getText()!=pasted) return failed(__LINE__);
            const auto chart=result.cents;
            const auto beforeRotation=activeCircles().front()->expressions();
            rotateRightButton.onClick();
            if(activeCircles().front()->editor(1).getText()!="Auto"||!knownLabel.getText().contains("1 fifth clockwise")) return failed(__LINE__);
            for(int step=1;step<12;++step) rotateRightButton.onClick();
            if(activeCircles().front()->expressions()!=beforeRotation||temperament::csv(result.cents)!=temperament::csv(chart)) return failed(__LINE__);
            setMode(selectedMode==temperament::Mode::pythagoreanFifths?temperament::Mode::syntonicFifths:temperament::Mode::pythagoreanFifths);
            if(csvOutput.getText().toStdString()!=temperament::csv(chart)||activeCircles().front()->editor(0).getText()!="Auto") return failed(__LINE__);
            calculate(false);
            if(!knownLabel.getText().contains("Trost")||csvOutput.getText().toStdString()!=temperament::csv(chart)) return failed(__LINE__);
            if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"trost-combined-laptop.png":"trost-pyth-laptop.png")) return failed(__LINE__);
        }
    }
    setPreset(true);
    if(!catalogueMatchDetails().contains("Equal")||!catalogueMatchDetails().contains("Neidhardt Hof")
        ||knownDetailsButton.getButtonText()!="Suggestions (5)") return failed(__LINE__);
    csvInput.setText("10.26;-8.31;3.42;-2.2;-3.42;8.31;-10.26;6.84;-6.35;0;4.15;-6.84",false);
    importCsv();
    if(!knownDetailsButton.isEnabled()||!catalogueMatchDetails().contains("Compare: Ordinaire")
        ||!catalogueMatchDetails().contains("Rousseau")||!knownLabel.getText().startsWith("Compare:")) return failed(__LINE__);
    if(!save("catalogue-similarity.png")) return failed(__LINE__);
    // Nearly identical fifths must receive identical purity colours. Exercise
    // the real circle rendering and the lattice's selected sensitivity together.
    for(auto selectedMode:{temperament::Mode::pythagoreanFifths,temperament::Mode::syntonicFifths})
    {
        setMode(selectedMode);
        std::array<std::string,12> comparison;comparison.fill("-P/12");
        comparison[0]="-schisma";comparison[11]="Auto";
        activeCircles().front()->setExpressions(comparison);calculate(false);
        if(!result.valid) return failed(__LINE__);
        harmony.sensitivityControl().setSelectedId(1,juce::sendNotificationSync);
        harmony.sensitivityControl().setSelectedId(2,juce::sendNotificationSync);
        const auto& analysed=harmony.chart();
        for(int note:{0,7})
            if(temperament::consonance(analysed.intervals[0][note],harmony.colourLimits())!=temperament::Consonance::good) return failed(__LINE__);
        auto& circle=*activeCircles().front();
        circle.clearSelection();
        const auto rendered=circle.createComponentSnapshot(circle.getLocalBounds());
        for(int edge:{0,1})
        {
            const auto ring=circle.ringBounds();
            bool greenArc=false;
            // Note badges and the box connector cover portions of each arc.
            for(int sample=2;sample<=8;++sample)
            {
                const float angle=juce::MathConstants<float>::twoPi*(edge+sample/10.0f)/12;
                const auto point=ring.getCentre()+juce::Point<float>(std::sin(angle),-std::cos(angle))*ring.getWidth()*0.5f;
                greenArc=greenArc||rendered.getPixelAt(juce::roundToInt(point.x),juce::roundToInt(point.y))==juce::Colour(0xff7fdda0);
            }
            if(!greenArc) return failed(__LINE__);
        }
        if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"schisma-et-pyth-green.png":"schisma-et-combined-green.png")) return failed(__LINE__);
        showHarmony();
        if(!save(selectedMode==temperament::Mode::pythagoreanFifths?"schisma-et-lattice-green.png":"schisma-et-lattice-combined-green.png")) return failed(__LINE__);
        setMode(selectedMode);
    }
    const auto referenceBeforeInvalid=csvInput.getText();
    activeCircles().front()->editor(0).setText("1/0",juce::sendNotificationSync);
    setMode(mode==temperament::Mode::pythagoreanFifths?temperament::Mode::syntonicFifths:temperament::Mode::pythagoreanFifths);
    if(result.valid||csvOutput.getText().isNotEmpty()||csvInput.getText()!=referenceBeforeInvalid
        ||activeCircles().front()->editor(0).getText().isNotEmpty()) return failed(__LINE__);
    setSize(1480,900);setPreset(false);
    temperament::ClosingRequest request;request.mode=mode;
    request.expressions=activeCircles().front()->expressions();request.selected[2]=true;
    const auto proposal=temperament::proposeClosing(request);
    if(!proposal.valid||proposal.expressions[2]!="-P") return failed(__LINE__);
    const auto original=inputSignature();
    ClosingPanel closingPanel(request,[](const auto&) {});closingPanel.showProposal(proposal);
    if(!snapshot(closingPanel,"closing-preview.png")||inputSignature()!=original) return failed(__LINE__);
    activeCircles().front()->setExpressions(proposal.expressions);calculate(false);
    if(!result.valid) return failed(__LINE__);
    showHarmony();harmony.selectChord(9,false);
    if(harmony.displayedNoteName(1)!="C#"||!save("harmony-a-major.png")) return failed(__LINE__);
    harmony.selectChord(10,true);
    if(harmony.displayedNoteName(1)!="Db"||!save("harmony-bb-minor.png")) return failed(__LINE__);
    harmony.sensitivityControl().setSelectedId(1,juce::sendNotificationSync);
    harmony.thirdsSensitivityControl().setSelectedId(1,juce::sendNotificationSync);
    setMode(temperament::Mode::pythagoreanFifths);setPreset(true);
    reconstruction.setSelectedId(1,juce::dontSendNotification);
    harmony.sensitivityControl().setSelectedId(2,juce::sendNotificationSync);
    harmony.thirdsSensitivityControl().setSelectedId(2,juce::sendNotificationSync);
    csvInput.clear();csvInput.setColour(juce::TextEditor::outlineColourId,border);
    if(!save("fifths-equal.png")) return failed(__LINE__);
    if(!harmony.isPlaybackSelected()||harmony.isAudioOutputOpen()) return failed(__LINE__);
    if(!catalogueMatchDetails().contains("Note: ")) return failed(__LINE__);
    directory.getChildFile("gui-check.txt").replaceWithText("GUI checks passed: layouts, cent-deviation bars, preserved CSV input, import, shared tabs, rotation, Auto, catalogue matches and purity colours.\n");
    return true;
}
