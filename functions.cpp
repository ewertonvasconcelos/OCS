/*#####################################################################################
## Universidade Federal do Rio de Janeiro
## ------------------------------------------------------------------
## Circuitos Eletricos II - EEL525
## Created by: Ewerton Vasconcelos, Antonio Galvão, Gabriel Lopes, Mateus Guina
## Date: 24/10/2018 - V1.0
## ------------------------------------------------------------------
## OCS - Open Circuit Simulator
## Module: netlist.h
## Description: Implementation of functions to read the netlist
##
#######################################################################################*/

#include "components.cpp"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include "utils.cpp"

using namespace std;

vector<Components*> ParseNetlits(string fileName) {
  
  vector<Components*> listFromNet;
  ifstream netlist;
  string line;
  char type;
  vector<string> param;
  int nParam;
  
  netlist.open("teste.net", ios::in);
  
  if(!netlist) {
	  cout << "Nao foi possível abrir o netlist" << endl;
	  abort();
  }
  
  while (getline(netlist,line)) {
	  nParam = countWords(line);
	  param = split(line);
	  type = param[0][0];
	  
	  switch (type) {
		case 'R':
			listFromNet.push_back(new Resistor(
					param[0],
                    stoi(param[1]),
                    stoi(param[2]),
                    stod(param[3])));
		break;	
		case 'V':
			listFromNet.push_back(new Vdc(
					param[0],
                    stoi(param[1]),
                    stoi(param[2]),
					param[3],
                    stod(param[4])));
		break;
		default:
			cout << "Linha inválida:" << line; 
		
	  }
	  
	  
	}
	 return listFromNet;
}
  
  
int GetNumberOfNodes( vector<Components*> vec ) {
	/*
	set<Components*> s( listFromNet.begin(), listFromNet.end() );
	cout << unique( listFromNet.begin(), listFromNet.end() );
	return s.size();*/
	
	cout << foos( vec.begin(), vec.end() ) << endl;

	return vec.size();
}