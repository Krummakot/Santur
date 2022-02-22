/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SanturTestAudioProcessor::SanturTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                  )
#endif
{

}

SanturTestAudioProcessor::~SanturTestAudioProcessor()
{
}

//==============================================================================
const juce::String SanturTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SanturTestAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SanturTestAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SanturTestAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SanturTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SanturTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SanturTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SanturTestAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SanturTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void SanturTestAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SanturTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    DBG("prepareToPlay Called");
    
    //resize vectors
    rValues.resize(Global::nrSanturNotes);
    aValues.resize(Global::nrSanturNotes);
    iValues.resize(Global::nrSanturNotes);
    stringsOut.resize(Global::nrSanturNotes);
    playNote.resize(Global::nrSanturNotes);
    triggerProcess.resize(Global::nrSanturNotes);
    midiValues.resize(Global::nrSanturNotes);
    
    
    // prepare circular buffers
    for (int i = 0; i < Global::maxActiveNotes; ++i) {
        enqueue(0);
    }
    
    // Fill r, a, and i vectors with appropriate value for each string
    // If number is even, use Ebrass, otherwise Esteel
    for(int i = 0; i < Global::nrSanturNotes; ++i) {
        playNote[i] = false;
        triggerProcess[i] = false;
        midiValues[i] = i + midiBaseNote;
        if(i % 2 == 0) {
            rValues[i] = (pow(4,0.25)*pow(Global::B,0.25)*pow(stringTensions[i],0.25))/(pow(Global::PI,0.25)*pow(Ebrass,0.25));
            aValues[i] = Global::PI * (rValues[i] * rValues[i]);
            iValues[i] = Global::PI * rValues[i] * rValues[i] * rValues[i] * rValues[i] * 0.25;
        } else {
            rValues[i] = (pow(4,0.25)*pow(Global::B,0.25)*pow(stringTensions[i],0.25))/(pow(Global::PI,0.25)*pow(Esteel,0.25));
            aValues[i] = Global::PI * (rValues[i] * rValues[i]);
            iValues[i] = Global::PI * rValues[i] * rValues[i] * rValues[i] * rValues[i] * 0.25;
        }
    }
    
    Fs = getSampleRate();
    
    santurNotes.clear();
    
    for (int i = 0; i < Global::nrSanturNotes; ++i) {
        if (i % 2 == 0) {
            santurNotes.add(std::make_unique<SanturNote>(stringLength, s0, s1, stringTensions[i], pBrass, aValues[i], Ebrass, iValues[i], rValues[i], Fs));
        } else {
            santurNotes.add(std::make_unique<SanturNote>(stringLength, s0, s1, stringTensions[i], pSteel, aValues[i], Esteel, iValues[i], rValues[i], Fs));
        }
    }
    isActive = true;
}

void SanturTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SanturTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void SanturTestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    MidiBuffer processMidi;

    // Get Midi input
    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        
        if(message.isNoteOn()) { // If note is triggered
            for (int n = 0; n < Global::nrSanturNotes; n++){
                if(message.getNoteNumber() == midiValues[n]) { // Get midi note and velocity
                    velocityFullRange = message.getVelocity();
                    velocityNormalized = (velocityFullRange/127); // normalise velocity to 0-1
                    vel[n] = velocityNormalized; // store the velocity in the velocity vector
                    
                    if (std::find(std::begin(A), std::end(A), message.getNoteNumber()) != std::end(A)) {
                        playNote[n] = true;
                        triggerProcess[n] = true;
                        startTimer(10000);
                        
                    } else { // if note is not in the circular buffer add it to the buffer
                        dequeue();
                        enqueue(message.getNoteNumber());
                        playNote[n] = true;
                        triggerProcess[n] = true;

                        startTimer(10000);
                    }
                }
            }
        }
    }
    //  Check which notes are active, and only update them
    checkActiveNotes();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    if (!isActive) {
            return;
    }
    
    // ..do something to the data...
    float* const channelDataL = buffer.getWritePointer(0);
    float* const channelDataR = buffer.getWritePointer(1);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        
        // Process the active strings
        processAndUpdateStrings();
        
        // Get output of the active strings
        mainOut = outputSound();
        
        // output sound
        channelDataL[i] = limit(mainOut, -1.f, 1.f);
        channelDataR[i] = limit(mainOut, -1.f, 1.f);
        
    }
}

//==============================================================================
bool SanturTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SanturTestAudioProcessor::createEditor()
{
    return new SanturTestAudioProcessorEditor (*this);
}

//==============================================================================
void SanturTestAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {

}

void SanturTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {

}

float SanturTestAudioProcessor::limit(float input, float min, float max) {
    if(input > max)
        return max;
    else if (input < min)
        return min;
    
    return input;
}

void SanturTestAudioProcessor::checkActiveNotes() {
    
    for(int i = 0; i < Global::nrSanturNotes; ++i) {
        if(playNote[i]) {
            santurNotes[i]->setPluckLoc(Global::pluckLoc);
            santurNotes[i]->excite(excitationSelection, vel[i]);
            playNote[i] = false;
        }
    }
}

void SanturTestAudioProcessor::processAndUpdateStrings() {
    for(int i = 0; i < Global::nrSanturNotes; ++i) {
        if(triggerProcess[i]) {
            santurNotes[i]->processScheme();
            santurNotes[i]->updateStates();
        }
    }
}

double SanturTestAudioProcessor::outputSound() {
    double out = 0.0;
    
    for (int i = 0; i < Global::nrSanturNotes; ++i) {
        stringsOut[i] = santurNotes[i]->getOutput(Global::outPos);
        out += stringsOut[i];
    }
    
    out *= 0.3;
    
    return out;
}

void SanturTestAudioProcessor::timerCallback() {
    for(int i = 0; i < Global::nrSanturNotes; ++i) {
        if (triggerProcess[i] == true) {
            triggerProcess[i] = false;
            currentActiveNotes = 0;
        }
    }
}


using namespace std;
int front = -1;
int rear = -1;

//function to enter elements in queue
void SanturTestAudioProcessor::enqueue (int value) {
    //first element inserted
    if( front == -1)
        front = 0;
    
    //insert element at rear
    rear = (rear+1)%Global::maxActiveNotes;
    A[rear] = value;
}

//function to delete/remove element from queue
void SanturTestAudioProcessor::dequeue () {
    //only one element
    if( front == rear ) {
        front = rear = -1;
    } else {
        for (int i = 0; i < Global::nrSanturNotes; ++i) {
            if(midiValues[i] == A[front]) {
                triggerProcess[i] = false;
            }
        }
        front = (front + 1)%Global::maxActiveNotes;
    }
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SanturTestAudioProcessor();
}
