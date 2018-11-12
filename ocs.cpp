/*#####################################################################################
## Universidade Federal do Rio de Janeiro
## ------------------------------------------------------------------
## Circuitos Eletricos II - EEL525
## Created by: Ewerton Vasconcelos, Antonio Galvão, Gabriel Lopes, Mateus Guina
## Date: 24/10/2018 - V1.0
## ------------------------------------------------------------------
## OCS - Open Circuit Simulator
## Module: ocs.cpp
## Description: Main file
##
#######################################################################################*/
//------------ Begin of Macros -------------------
#define OK                 0  // Return OK for SO
//------------ End of Macros -------------------
#include "components.cpp"
#include "AllComponents.cpp"
#include "functions.cpp"
#include <iostream> // std::cout
#include <string> // std::stoi
using namespace std;



int main ()
{
  cout << " -------------------- OCS ----------------------" << endl;
  string fileName = "simples.net";
  
  vector<Components*> listFromNet;
  int nNodes, nVirtualCurrents=0;
  vector<int> nodes;
  
  listFromNet = ParseNetlits(fileName);
  nNodes=GetNumberOfNodes(listFromNet, nodes);
  
  vector<double> result(nNodes + nVirtualCurrents);
  vector<vector<double> > conductances( nNodes + nVirtualCurrents, vector<double>(nNodes + nVirtualCurrents));
  vector<double> correntes(nNodes + nVirtualCurrents);

  
  
  	for (unsigned int i = 0; i < listFromNet.size(); i++) {
		//listFromNet.at(i)->stamp(mt1)
		cout << listFromNet.at(i)->getName() << endl;
		cout << listFromNet.at(i)->noA << endl;
		cout << listFromNet.at(i)->noB << endl;
		if ((listFromNet.at(i)->getName()[0]) == 'R')
			cout << listFromNet.at(i)->getResistance() << endl;
	}
	
	
	//stamp all elements
	for (unsigned int i = 0; i < listFromNet.size(); i++) {
		cout << "#" << i  << endl;
		listFromNet.at(i)->stamp(conductances);
	}
	
	
	
	printMatrix(conductances);
	

  return OK;
}

