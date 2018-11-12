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
#include <algorithm>

using namespace std;

vector<Components*> ParseNetlits(string fileName) {
  
  vector<Components*> listFromNet;
  ifstream netlist;
  string line;
  char type;
  vector<string> param;
  
  netlist.open("teste.net", ios::in);
  
  if(!netlist) {
	  cout << "Nao foi possível abrir o netlist" << endl;
	  abort();
  }
  
  while (getline(netlist,line)) {
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
  
  
int GetNumberOfNodes( vector<Components*> vec, vector<int>& nodes  ) {
	vector<int> nodesList;
	int a,b,c,d;

  	for (unsigned int i = 0; i < vec.size(); i++) {
		a=vec.at(i)->noA;
		b=vec.at(i)->noB;
		c=vec.at(i)->noC;
		d=vec.at(i)->noD;
		
		if ( find(nodesList.begin(), nodesList.end(), a) == nodesList.end() )
			nodesList.push_back(vec.at(i)->noA);
		if ( find(nodesList.begin(), nodesList.end(), b) == nodesList.end() )
			nodesList.push_back(vec.at(i)->noB);          
		if ( find(nodesList.begin(), nodesList.end(), c) == nodesList.end() )
			nodesList.push_back(vec.at(i)->noC);          
		if ( find(nodesList.begin(), nodesList.end(), d) == nodesList.end() )
			nodesList.push_back(vec.at(i)->noD);	
	}
	nodes = nodesList;
	return nodesList.size();
}

void printMatrix ( vector<vector<double> > matrix( int, vector<double>(int)) ) {
	for (unsigned int i = 0; i < matrix[i].size(); i++) {
		for (unsigned int j = 0; i < matrix[j].size(), j++) {
			cout << matrix[i][j]
		}
	}