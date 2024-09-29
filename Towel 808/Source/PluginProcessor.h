/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// No forward declarations needed since we'll define classes in the .cpp file

//==============================================================================
/**
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Method to get the list of sample names
    juce::StringArray getSampleNames() const;

    // Method to load a sample by name
    void loadSample (const juce::String& sampleName);

    // AudioProcessorValueTreeState for parameter management
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Midi Keyboard State
    juce::MidiKeyboardState keyboardState;

private:
    //==============================================================================
    // Synthesiser for playing samples
    juce::Synthesiser sampler;

    // Format manager to handle audio formats
    juce::AudioFormatManager formatManager;

    // Path to the samples directory
    juce::File samplesDirectory;

    // Available sample files
    juce::Array<juce::File> sampleFiles;

    // Currently loaded sample
    juce::String currentSampleName;

    // Number of voices in the sampler
    static constexpr int numVoices = 64;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessor)
};
