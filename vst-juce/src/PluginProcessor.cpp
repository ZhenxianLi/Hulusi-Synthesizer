#include "PluginProcessor.h"
#include "PluginEditor.h"

HulusiVSTAudioProcessor::HulusiVSTAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout())
{}

void HulusiVSTAudioProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
}

void HulusiVSTAudioProcessor::releaseResources() {}

bool HulusiVSTAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void HulusiVSTAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn()) engine.noteOn(msg.getNoteNumber(), msg.getVelocity());
        if (msg.isNoteOff()) engine.noteOff(msg.getNoteNumber());
    }

    engine.setParams(
        *apvts.getRawParameterValue("gain"),
        *apvts.getRawParameterValue("breath"),
        *apvts.getRawParameterValue("vibDepth"),
        *apvts.getRawParameterValue("vibRate"));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        const float y = engine.renderSample();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, s, y);
    }
}

juce::AudioProcessorEditor* HulusiVSTAudioProcessor::createEditor()
{
    return new HulusiVSTAudioProcessorEditor(*this);
}

void HulusiVSTAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HulusiVSTAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout HulusiVSTAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 1.0f, 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("breath", "Breath", 0.0f, 0.5f, 0.12f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("vibDepth", "Vibrato Depth", 0.0f, 0.02f, 0.003f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("vibRate", "Vibrato Rate", 0.1f, 12.0f, 5.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HulusiVSTAudioProcessor();
}
