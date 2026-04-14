#include "PluginEditor.h"

namespace {
void setupSlider(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
}
}

HulusiVSTAudioProcessorEditor::HulusiVSTAudioProcessorEditor(HulusiVSTAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setupSlider(gainSlider);
    setupSlider(breathSlider);
    setupSlider(vibDepthSlider);
    setupSlider(vibRateSlider);

    addAndMakeVisible(gainSlider);
    addAndMakeVisible(breathSlider);
    addAndMakeVisible(vibDepthSlider);
    addAndMakeVisible(vibRateSlider);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "gain", gainSlider);
    breathAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "breath", breathSlider);
    vibDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "vibDepth", vibDepthSlider);
    vibRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, "vibRate", vibRateSlider);

    setSize(420, 180);
}

void HulusiVSTAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawFittedText("Hulusi VST (JUCE)", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void HulusiVSTAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12).withTrimmedTop(36);
    auto w = area.getWidth() / 4;
    gainSlider.setBounds(area.removeFromLeft(w).reduced(4));
    breathSlider.setBounds(area.removeFromLeft(w).reduced(4));
    vibDepthSlider.setBounds(area.removeFromLeft(w).reduced(4));
    vibRateSlider.setBounds(area.removeFromLeft(w).reduced(4));
}
