/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), keyboardComponent (audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Set the initial size of the plugin window
    setSize (600, 500);

    // Make the editor resizable and set resize limits
    setResizable (true, true);
    setResizeLimits (400, 300, 1000, 800);

    // Add the keyboard component
    addAndMakeVisible (keyboardComponent);

    // Define the color for sliders
    juce::Colour sliderColour = juce::Colours::grey;

    // Initialize and configure the Attack slider and label
    attackSlider.setSliderStyle(juce::Slider::LinearVertical);
    attackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    attackSlider.setColour(juce::Slider::thumbColourId, sliderColour);
    attackSlider.setColour(juce::Slider::trackColourId, sliderColour);
    addAndMakeVisible(attackSlider);

    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envAttack", attackSlider);

    // Initialize and configure the Decay slider and label
    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    decaySlider.setColour(juce::Slider::thumbColourId, sliderColour);
    decaySlider.setColour(juce::Slider::trackColourId, sliderColour);
    addAndMakeVisible(decaySlider);

    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(decayLabel);

    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envDecay", decaySlider);

    // Initialize and configure the Sustain slider and label
    sustainSlider.setSliderStyle(juce::Slider::LinearVertical);
    sustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    sustainSlider.setColour(juce::Slider::thumbColourId, sliderColour);
    sustainSlider.setColour(juce::Slider::trackColourId, sliderColour);
    addAndMakeVisible(sustainSlider);

    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainLabel);

    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envSustain", sustainSlider);

    // Initialize and configure the Release slider and label
    releaseSlider.setSliderStyle(juce::Slider::LinearVertical);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    releaseSlider.setColour(juce::Slider::thumbColourId, sliderColour);
    releaseSlider.setColour(juce::Slider::trackColourId, sliderColour);
    addAndMakeVisible(releaseSlider);

    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);

    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envRelease", releaseSlider);

    // Initialize and configure the Cut button
    cutButton.setButtonText("Cut");
    addAndMakeVisible(cutButton);

    cutButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "cutEnabled", cutButton);

    // Get the list of sample names from the processor
    auto sampleNames = audioProcessor.getSampleNames();

    // Add items to the ComboBox and configure it
    sampleSelector.addItemList(sampleNames, 1); // IDs start from 1
    sampleSelector.setSelectedId(1); // Select the first sample by default
    sampleSelector.addListener(this);
    addAndMakeVisible(sampleSelector);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    sampleSelector.removeListener(this);
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll (juce::Colours::darkgrey);

    // Set text properties
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    // Draw text
    g.drawFittedText ("Select an 808 Sample:", 10, 10, getWidth() - 20, 20, juce::Justification::centredLeft, 1);
}

//==============================================================================
void NewProjectAudioProcessorEditor::resized()
{
    // Define padding and margins
    const int padding = 10;
    const int componentSpacing = 10;

    int width = getWidth();
    int height = getHeight();

    // Position the ComboBox at the top
    int comboBoxHeight = 30;
    sampleSelector.setBounds(padding, padding, width - 2 * padding, comboBoxHeight);

    // Position the Cut button below the sampleSelector
    int buttonHeight = 30;
    cutButton.setBounds(padding, sampleSelector.getBottom() + componentSpacing, width - 2 * padding, buttonHeight);

    // Calculate area for sliders
    int slidersAreaY = cutButton.getBottom() + componentSpacing;
    int slidersAreaHeight = height * 0.35f; // 35% of window height for sliders

    // Calculate the width for each slider based on the total available width
    int numSliders = 4;
    int totalSliderPadding = (numSliders + 1) * padding;
    int sliderWidth = (width - totalSliderPadding) / numSliders;
    int sliderHeight = slidersAreaHeight - 2 * padding;

    // Position sliders horizontally with equal spacing
    int sliderY = slidersAreaY + padding;

    attackSlider.setBounds(padding, sliderY, sliderWidth, sliderHeight);
    attackLabel.setBounds(padding, sliderY + sliderHeight, sliderWidth, 20);

    decaySlider.setBounds(2 * padding + sliderWidth, sliderY, sliderWidth, sliderHeight);
    decayLabel.setBounds(2 * padding + sliderWidth, sliderY + sliderHeight, sliderWidth, 20);

    sustainSlider.setBounds(3 * padding + 2 * sliderWidth, sliderY, sliderWidth, sliderHeight);
    sustainLabel.setBounds(3 * padding + 2 * sliderWidth, sliderY + sliderHeight, sliderWidth, 20);

    releaseSlider.setBounds(4 * padding + 3 * sliderWidth, sliderY, sliderWidth, sliderHeight);
    releaseLabel.setBounds(4 * padding + 3 * sliderWidth, sliderY + sliderHeight, sliderWidth, 20);

    // Position the keyboard component at the bottom
    int keyboardY = slidersAreaY + slidersAreaHeight + componentSpacing;
    int keyboardHeight = height - keyboardY - padding;

    keyboardComponent.setBounds(padding, keyboardY, width - 2 * padding, keyboardHeight);
}

void NewProjectAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &sampleSelector)
    {
        // Get the selected sample name
        auto selectedSample = sampleSelector.getText();

        // Instruct the processor to load the selected sample
        audioProcessor.loadSample(selectedSample);
    }
}
