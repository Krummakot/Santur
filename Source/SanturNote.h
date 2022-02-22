/*
  ==============================================================================

    SanturString.h
    Created: 16 Apr 2021 11:43:43am
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#pragma once

#include "DampedString.hpp"
#include "Global.h"

class SanturNote {
    
    public:
    SanturNote();
    SanturNote(double stringLength, double s0, double s1, double tension, double p, double A, double E, double I, double r, double sampleRate);
    
    ~SanturNote();
    
    void setPluckLoc(double pluckLoc);
    void excite(int exciteSelection, double velocity);
    void processScheme();
    void updateStates();
    double getOutput(double outPos);
    
    double k;
    
    private:
    OwnedArray<DampedString> strings;
};
