/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
/*
  Class definitions for MySamplerSound and MySamplerVoice
*/

// Custom SamplerSound class to hold sample data and parameters
class MySamplerSound : public juce::SynthesiserSound
{
public:
    MySamplerSound(const juce::String& soundName,
                   juce::AudioFormatReader& source,
                   const juce::BigInteger& notes,
                   int midiNoteForNormalPitch,
                   double attackTimeSecs,
                   double releaseTimeSecs,
                   double maxSampleLengthSeconds)
        : name(soundName),
          midiNotes(notes),
          midiRootNote(midiNoteForNormalPitch)
    {
        if (maxSampleLengthSeconds > 0)
            length = juce::roundToIntAccurate(source.sampleRate * maxSampleLengthSeconds);
        else
            length = (int)source.lengthInSamples;

        data.reset(new juce::AudioBuffer<float>(juce::jmin(2, (int)source.numChannels), length + 4));

        source.read(data.get(), 0, length + 4, 0, true, true);

        params.attack = attackTimeSecs;
        params.release = releaseTimeSecs;

        sourceSampleRate = source.sampleRate;
    }

    bool appliesToNote (int midiNoteNumber) override
    {
        return midiNotes[midiNoteNumber];
    }

    bool appliesToChannel (int /*midiChannel*/) override
    {
        return true;
    }

    juce::AudioBuffer<float>* getAudioData() const noexcept
    {
        return data.get();
    }

    const juce::ADSR::Parameters& getADSRParameters() const noexcept
    {
        return params;
    }

    int getMidiRootNote() const noexcept
    {
        return midiRootNote;
    }

    double getSourceSampleRate() const noexcept
    {
        return sourceSampleRate;
    }

private:
    juce::String name;
    std::unique_ptr<juce::AudioBuffer<float>> data;
    juce::BigInteger midiNotes;
    int midiRootNote;
    double sourceSampleRate;
    juce::ADSR::Parameters params;
    int length;
};

// Custom SamplerVoice class to handle ADSR and sample playback
class MySamplerVoice : public juce::SynthesiserVoice
{
public:
    MySamplerVoice() {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<MySamplerSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound, int /*currentPitchWheelPosition*/) override
    {
        if (auto* samplerSound = dynamic_cast<MySamplerSound*> (sound))
        {
            pitchRatio = std::pow (2.0, (midiNoteNumber - samplerSound->getMidiRootNote()) / 12.0)
                            * (samplerSound->getSourceSampleRate() / getSampleRate());

            sourceSamplePosition = 0.0;
            lgain = velocity;
            rgain = velocity;

            adsr.setSampleRate (getSampleRate());
            adsr.setParameters (adsrParameters);
            adsr.noteOn();

            // Keep a reference to the audio data
            soundData = samplerSound->getAudioData();
        }
        else
        {
            jassertfalse; // This object can only play MySamplerSounds!
        }
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            clearCurrentNote();
            adsr.reset();
            soundData = nullptr; // Invalidate the soundData pointer
        }
    }

    void pitchWheelMoved (int /*newValue*/) override {}
    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (soundData == nullptr)
            return;

        const float* const inL = soundData->getReadPointer (0);
        const float* const inR = soundData->getNumChannels() > 1 ? soundData->getReadPointer (1) : nullptr;

        auto numSourceSamples = soundData->getNumSamples();

        while (--numSamples >= 0)
        {
            // Check if soundData is still valid
            if (soundData == nullptr)
                break;

            auto pos = (int) sourceSamplePosition;

            if (pos + 1 >= numSourceSamples)
            {
                // Stop the note and exit the loop to prevent out-of-bounds access
                stopNote (0.0f, false);
                break;
            }

            auto alpha = (float) (sourceSamplePosition - pos);
            auto invAlpha = 1.0f - alpha;

            // Simple linear interpolation
            float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
            float r = inR != nullptr ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

            auto envelopeValue = adsr.getNextSample();

            if (envelopeValue <= 0.0f)
            {
                clearCurrentNote();
                soundData = nullptr; // Invalidate the soundData pointer
                break;
            }

            outputBuffer.addSample (0, startSample, l * lgain * envelopeValue);
            outputBuffer.addSample (1, startSample, r * rgain * envelopeValue);

            sourceSamplePosition += pitchRatio;

            ++startSample;
        }
    }

    void setADSRParameters(const juce::ADSR::Parameters& params)
    {
        adsrParameters = params;
    }

private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;

    double pitchRatio = 0.0;
    double sourceSamplePosition = 0.0;
    float lgain = 0.0f, rgain = 0.0f;

    juce::AudioBuffer<float>* soundData = nullptr;
};

//==============================================================================
// Now proceed with the rest of your PluginProcessor.cpp code

NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       #if ! JucePlugin_IsMidiEffect
                        #if ! JucePlugin_IsSynth
                         .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        #endif
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       #endif
                         ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    // Register basic audio formats (WAV, AIFF, etc.)
    formatManager.registerBasicFormats();

    // Set the path to the samples directory
    samplesDirectory = juce::File::getSpecialLocation(juce::File::userMusicDirectory).getChildFile("Towel Tuned 808s");

    // Add voices to the sampler for polyphony
    for (int i = 0; i < numVoices; ++i)
        sampler.addVoice(new MySamplerVoice());

    // Get the list of sample files
    samplesDirectory.findChildFiles(sampleFiles, juce::File::findFiles, false, "*.wav");
}

NewProjectAudioProcessor::~NewProjectAudioProcessor()
{
}

//==============================================================================
const juce::String NewProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NewProjectAudioProcessor::acceptsMidi() const
{
    return true;
}

bool NewProjectAudioProcessor::producesMidi() const
{
    return false;
}

bool NewProjectAudioProcessor::isMidiEffect() const
{
    return false;
}

double NewProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NewProjectAudioProcessor::getNumPrograms()
{
    return 1; // We have only one program
}

int NewProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NewProjectAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String NewProjectAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void NewProjectAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    // Set the playback sample rate for the sampler
    sampler.setCurrentPlaybackSampleRate(sampleRate);

    // Optionally load a default sample
    if (sampleFiles.size() > 0)
        loadSample(sampleFiles[0].getFileNameWithoutExtension());
}

void NewProjectAudioProcessor::releaseResources()
{
    // Free up any spare memory when playback stops
}

// This checks if the layout is supported.
#ifndef JucePlugin_PreferredChannelConfigurations
bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Only mono or stereo is supported
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    // Input and output layout must match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Update keyboard state
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    // Update ADSR parameters
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = apvts.getRawParameterValue("envAttack")->load();
    adsrParams.decay = apvts.getRawParameterValue("envDecay")->load();
    adsrParams.sustain = apvts.getRawParameterValue("envSustain")->load();
    adsrParams.release = apvts.getRawParameterValue("envRelease")->load();

    // Implement the Cut functionality
    bool cutEnabled = apvts.getRawParameterValue("cutEnabled")->load() > 0.5f;

    if (cutEnabled)
    {
        // Set a very short release time when Cut is enabled
        adsrParams.release = 0.01f; // 10 milliseconds release time

        juce::MidiBuffer::Iterator it(midiMessages);
        juce::MidiMessage message;
        int samplePosition;

        while (it.getNextEvent(message, samplePosition))
        {
            if (message.isNoteOn())
            {
                // Stop all voices before processing this note
                sampler.allNotesOff(0, true); // Force immediate stop
                break; // Only need to stop once per block
            }
        }
    }

    // Set ADSR parameters for each voice
    for (int i = 0; i < sampler.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<MySamplerVoice*>(sampler.getVoice(i)))
        {
            voice->setADSRParameters(adsrParams);
        }
    }

    buffer.clear(); // Clear the buffer before rendering

    // Render audio from the sampler
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool NewProjectAudioProcessor::hasEditor() const
{
    return true; // Return true if you have a GUI editor
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor()
{
    return new NewProjectAudioProcessorEditor (*this);
}

//==============================================================================
void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save your plugin's parameters here
    juce::MemoryOutputStream stream(destData, true);
    apvts.state.writeToStream(stream);
}

void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore your plugin's parameters here
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.state = tree;
    }
}

//==============================================================================
juce::StringArray NewProjectAudioProcessor::getSampleNames() const
{
    juce::StringArray names;

    for (auto& file : sampleFiles)
    {
        names.add(file.getFileNameWithoutExtension());
    }

    return names;
}

void NewProjectAudioProcessor::loadSample (const juce::String& sampleName)
{
    // Stop all voices immediately
    sampler.allNotesOff(0, true); // Force immediate stop

    // Clear existing sounds
    sampler.clearSounds();

    // Find the sample file by name
    for (auto& file : sampleFiles)
    {
        if (file.getFileNameWithoutExtension() == sampleName)
        {
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

            if (reader.get() != nullptr)
            {
                juce::BigInteger midiNotes;
                midiNotes.setRange(0, 128, true); // Respond to all MIDI notes

                auto duration = static_cast<float>(reader->lengthInSamples) / reader->sampleRate;

                // Create a MySamplerSound instance
                MySamplerSound* sound = new MySamplerSound(
                    file.getFileNameWithoutExtension(),
                    *reader,
                    midiNotes,
                    60,    // MIDI root note (middle C)
                    0.0,   // Attack time
                    0.1,   // Release time
                    duration
                );

                // Add the sound to the sampler
                sampler.addSound(sound);

                currentSampleName = sampleName;
            }

            break;
        }
    }
}

// Create parameter layout
juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("envAttack", "Attack", 0.01f, 5.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("envDecay", "Decay", 0.01f, 5.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("envSustain", "Sustain", 0.0f, 1.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("envRelease", "Release", 0.01f, 5.0f, 0.5f));

    // Add the Cut parameter
    params.push_back(std::make_unique<juce::AudioParameterBool>("cutEnabled", "Cut", false));

    return { params.begin(), params.end() };
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NewProjectAudioProcessor();
}
