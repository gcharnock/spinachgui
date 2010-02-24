#ifndef NUCLEAR_DATA_H
#define NUCLEAR_DATA_H

#include <string>

struct Isotope;

//Elemental functions
long getElementCount();
const char* getElementSymbol(long p);
const char* getElementName(long p);
double getElementR(long p);
double getElementG(long p);
double getElementB(long p);

///Given a particular nucleous, get a sensible default isotope
///(eg. p=6 should return 6, -> carbon is usually carbon 12)
long getCommonIsotope(long p);

long getElementBySymbol(const char* symb);


//Isotope functions

///Call this somewhere in the initalisation to load the istope data
///from data/isotopes.dat
void LoadIsotopes();

///Get the number of known isotopes of a given element
long getIsotopeCount(long protonN);
///Get the index'th isotope of a given element where 0 <= index <
///getIsotopeCount()

///Get the number of neutrons given an isotobe index
long getNeutrons(long protonN,long index);

///Get the nuclear spin in units of hbar/2
long getNuclearSpin(long protonN,long index);

///
double getGyromagneticRatio(long protonN,long index);
#endif // __spinachcalcFrameBase__


