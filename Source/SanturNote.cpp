/*
  ==============================================================================

    SanturString.cpp
    Created: 16 Apr 2021 11:43:43am
    Author:  Oddur Kristjansson

  ==============================================================================
*/

#include "SanturNote.h"

SanturNote::SanturNote(){}

SanturNote::SanturNote(double stringLength, double s0, double s1, double tension, double p, double A, double E, double I, double r, double sampleRate){
    
    NamedValueSet parameters, parameters2, parameters3, parameters4;
    
    parameters.set("stringLength", stringLength);
    parameters.set("tension", tension);
    parameters.set("p", p);
    parameters.set("A", Global::PI * (r*r));
    parameters.set("E", E);
    parameters.set("I", Global::PI * r * r * r * r * 0.25);
    parameters.set("s0", s0);                               // Frequency-independent damping
    parameters.set("s1",  s1);
    
    parameters2.set("stringLength", stringLength);
    parameters2.set("tension", tension - tension/50);
    parameters2.set("p", p);
    parameters2.set("A", Global::PI * (r*r));
    parameters2.set("E", E);
    parameters2.set("I", Global::PI * r * r * r * r * 0.25);
    parameters2.set("s0", s0);                               // Frequency-independent damping
    parameters2.set("s1",  s1);
    
    parameters3.set("stringLength", stringLength);
    parameters3.set("tension", tension + tension/99);
    parameters3.set("p", p);
    parameters3.set("A", Global::PI * (r*r));
    parameters3.set("E", E);
    parameters3.set("I", Global::PI * r * r * r * r * 0.25);
    parameters3.set("s0", s0);                               // Frequency-independent damping
    parameters3.set("s1",  s1);
   
    parameters4.set("stringLength", stringLength);
    parameters4.set("tension", tension - tension/98);
    parameters4.set("p", p);
    parameters4.set("A", Global::PI * (r*r));
    parameters4.set("E", E);
    parameters4.set("I", Global::PI * r * r * r * r * 0.25);
    parameters4.set("s0", s0);                               // Frequency-independent damping
    parameters4.set("s1",  s1);
    
    k = stringLength / sampleRate;              // Time-step
    
    strings.clear();
    strings.add(std::make_unique<DampedString>(parameters, k));
    strings.add(std::make_unique<DampedString>(parameters2, k));
    strings.add(std::make_unique<DampedString>(parameters3, k));
    strings.add(std::make_unique<DampedString>(parameters4, k));
    
}

SanturNote::~SanturNote() {
}


void SanturNote::setPluckLoc(double pluckLoc) {
    for (int i = 0; i < Global::stringsPerNote; ++i) {
        strings[i]->setPluckLoc(pluckLoc);
    }
}


void SanturNote::excite(int exciteSelection, double velocity) {
    for (int i = 0; i < Global::stringsPerNote; ++i) {
        strings[i]->excite(exciteSelection, velocity);
    }
}
void SanturNote::processScheme() {
    for (int i = 0; i < Global::stringsPerNote; ++i) {
        strings[i]->processScheme();
    }
}
void SanturNote::updateStates() {
    for (int i = 0; i < Global::stringsPerNote; ++i) {
        strings[i]->updateStates();
    }
}

double SanturNote::getOutput(double outPos) {
    double out = 0.0;
    
    for (int i = 0; i < Global::stringsPerNote; ++i) {
        out += strings[i]->getOutput(outPos);
    }
    
    return out *= 0.25;
}

