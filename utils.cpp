/*#####################################################################################
## Universidade Federal do Rio de Janeiro
## ------------------------------------------------------------------
## Circuitos Eletricos II - EEL525
## Created by: Ewerton Vasconcelos, Antonio Galvão, Gabriel Lopes, Mateus Guina
## Date: 24/10/2018 - V1.0
## ------------------------------------------------------------------
## OCS - Open Circuit Simulator
## Module: utils.cpp
## Description: Ultils functions
##
#######################################################################################*/
#include <stdio.h> 
#include <string>


unsigned countWords(string str) 
{ 
	int out = 0;
	int in = 1;
	int i = 0;
    int state = out; 
    unsigned wc = 0;  // word count 
  
    // Scan all characters one by one 
    while (str[i]) 
    { 
        // If next character is a separator, set the  
        // state as out 
        if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') 
            state = out; 
        else if (state == out) 
        { 
            state = in; 
            ++wc; 
        } 
		i++;
    } 
  
    return wc; 
} 

vector<string> split(string str, char delimiter = ' ')
{
    vector<string> ret;
    if(str.empty()) 
    {
        ret.push_back(string(""));
        return ret;
    }

    unsigned i = 0;
    string strstack;
    while(!(str.empty()) && (str[0] == delimiter)) {str.erase(0,1);}
    reverse(str.begin(), str.end());
    while(!(str.empty()) && (str[0] == delimiter)) {str.erase(0,1);}
    reverse(str.begin(), str.end());
    while(!str.empty())
    {
        ret.push_back(str.substr(i, str.find(delimiter)));
        str.erase(0,str.find(delimiter));
        while(!(str.empty()) && (str[0] == delimiter)) {str.erase(0,1);}
    }

    return ret;
}