/*#####################################################################################
## Universidade Federal do Rio de Janeiro
## ------------------------------------------------------------------
## Circuitos Eletricos II - EEL525
## Created by: Ewerton Vasconcelos, Antonio Galvão, Gabriel Lopes, Mateus Guina
## Date: 24/10/2018 - V1.0
## ------------------------------------------------------------------
## OCS - Open Circuit Simulator
## Module: components.cpp
## Description: Implementation of component classes
##
#######################################################################################*/
#include <string>
#include <vector>
using namespace std;

#ifndef COMPONENTS
#define COMPONENTS
class Components
{
    public:	  
		string getName() 
		{
			return name;
		}
	
      // Common 
	  string name;
      int noA;
      int noB;
      int noC;
      int noD;
	  /*void virtual stamp(vector<vector<double> >& conductances,
				vector<double>& currents,
				vector<string> nodes,
				vector<double> results);
	 
	  // Resistor Virtual Functions
	  */
	  double virtual getResistance() {return 0;};
	  
	  // Vdc Virtual Functions
	  double virtual getVdcValue() {return 0;};
	  string virtual getVdcType() {return "";};
	  void virtual stamp(vector<vector<double> >& conductances) {};
};

#endif

