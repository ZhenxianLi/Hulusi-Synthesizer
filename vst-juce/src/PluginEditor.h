#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class HulusiVSTAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit HulusiVSTAudioProcessorEditor(HulusiVSTAudioProcessor&);
    ~HulusiVSTAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HulusiVSTAudioProcessor& processor;

    juce::Slider gainSlider, breathSlider, vibDepthSlider, vibRateSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> breathAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibRateAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HulusiVSTAudioProcessorEditor)
};
