/*
  ==============================================================================

    Global.h
    Created: 22 Feb 2022 8:27:58pm
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Global {
    
public:
    constexpr static const double PI = MathConstants<double>::pi;
    static const int maxActiveNotes = 6;
    static const int nrSanturNotes = 18;
    
    
    static const int stringsPerNote = 4;
    
    constexpr static const double B = 0.00031;

    
    constexpr static const double outPos = 0.12;
    constexpr static const double pluckLoc = 0.7;
    
private:
    
};
