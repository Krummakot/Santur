/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DampedString.hpp"
#include "SanturNote.h"
#include "Global.h"

//==============================================================================
/**
*/
class SanturTestAudioProcessor  : public juce::AudioProcessor, juce::Timer
{
public:
    //==============================================================================
    SanturTestAudioProcessor();
    ~SanturTestAudioProcessor() override;

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
    
    float limit(float input, float min, float max);

    void checkActiveNotes();
    void processAndUpdateStrings();
    double outputSound();
    
    void removeExpired();
    void enqueue(int value);
    void dequeue();

    void timerCallback() override;
    
    int excitationSelection = 1;
    double Fs;

private:
    
    double damp, stiffness, stringDiameter, stringRadius;
    
    int detuneValue;
    int stringLength = 1;
    double s0 = 1.14;
    double s1 = 0.0006;
    
    double pBrass = 8400.0;
    double pSteel = 7700.0;
    double Ebrass = 113500000000.0;
    double Esteel = 180000000000.0;
    
    int midiBaseNote = 48;
 
    OwnedArray<SanturNote> santurNotes;
    
    std::vector<double> stringTensions{11800.f, 39003.f, 16079.f, 68503.f, 22079.f, 98926.f, 29772.f, 138692.f, 31010.f, 147464.f, 42466.f, 200700.f, 54010.f, 263315.f, 53351.f, 273722.f, 60384.f, 352863.f};
    
    std::vector<double> rValues, aValues, iValues;
    
    std::vector<int> midiValues;
    
    std::vector<bool> playNote, triggerProcess;
    
    std::vector<double> stringsOut;
    
    double mainOut;
    
    double vel[Global::nrSanturNotes] = {0.0};
    double velocityFullRange, velocityNormalized;

    int currentActiveNotes = 0;
    
    int currentMidiNotes[Global::maxActiveNotes] = {0};

    int A[Global::maxActiveNotes] = {0};

    bool isActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SanturTestAudioProcessor)
};
