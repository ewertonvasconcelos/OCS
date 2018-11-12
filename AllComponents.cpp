/*#####################################################################################
## Universidade Federal do Rio de Janeiro
## ------------------------------------------------------------------
## Circuitos Eletricos II - EEL525
## Created by: Ewerton Vasconcelos, Antonio Galvão, Gabriel Lopes, Mateus Guina
## Date: 24/10/2018 - V1.0
## ------------------------------------------------------------------
## OCS - Open Circuit Simulator
## Module: components.h
## Description: Definition of component classes
##
#######################################################################################*/
#include <string>
#include <vector>
#include <algorithm>    // std::find
#include "components.cpp"

/*Resistor*/
#ifndef RESISTOR
#define RESISTOR
class Resistor : public Components {
	public:
		Resistor(string n, int a, int b, double v) : Components() {
				name = n;
				noA = a;
				noB = b;
				noC=0;
				noD=0;
				resistance = v;
		}
		
		double getResistance (){
			return this->resistance;
		}
		
				/*void stamp(vector<vector<double> >& conductances,
				vector<double>& currents,
				vector<string> nodes,
				vector<double> results)*/
				
		void stamp(vector<vector<double> >& conductances) {
				conductances[noA][noA] += 1/resistance;
				conductances[noB][noB] += 1/resistance;
				conductances[noA][noB] += -1/resistance;
				conductances[noB][noA] += -1/resistance;
		}
	private:
		double resistance;

};
#endif

/*Vdc*/
#ifndef VDC
#define VDC
class Vdc : public Components {
	public:
		Vdc(string n, int a, int b, string type, double v) : Components() {
			name = n;
			noA = a;
			noB = b;
			noC=0;
			noD=0;
			vdc = v;
		}
		
		double getVdcValue (){
			return this->vdc;
		}
		
		string getVdcType() {
			return this->type;
		}

		void stamp(vector<vector<double> >& conductances,
				vector<double>& currents,
				vector<string> nodes,
				vector<double> results)
		{
				vector<string>::iterator it;
				it = find(nodes.begin(), nodes.end(), "j"+name);
				auto pos = it - nodes.begin();

				conductances[pos][noA] += -1;
				conductances[pos][noB] += 1;
				currents[pos] += -1*vdc;
		}
	private:
		double vdc;
		string type;
};
#endif


/*
class Inductor {
	public:
		string name;
		double inductance;
		int noA;
		int noB
		double ic;
		Inductor();
};

class Capacitor {
	public:
		string name;
		double capacitance
		int noA;
		int noB;
		double ic;
		Capacitor();
};

class Vscv {
	public:
		string name;
		int noA;
		int noB;
		int noAc;
		int noBc;
		double Av;
		Vscv();
};

class Isci {
	public:
		string name;
		int noA;
		int noB;
		int noAc;
		int noBc;
		double Ai;
		Isci();
};

class Iscv {
	public:
		string name;
		int noA;
		int noB;
		int noAc;
		int noBc;
		double Gm;
		Iscv();
};

class Vsci {
	public:
		string name;
		int noA;
		int noB;
		int noAc;
		int noBc;
		double Rm;
		Vsci();
};
class Vsci {
	public:
		string name;
		int noA;
		int noB;
		int noAc;
		int noBc;
		double Rm;
		Vsci();
};

class Vdc {
	public:
		string name;
		int noA;
		int noB;
		double V;
		Vdc();
};

class Vsin {
	public:
		string name;
		int noA;
		int noB;
		double V;
		double Amp;
		double Freq;
		double delay;
		double amort;
		double lag;
		double cycles;
		Vsin();
};
class Vpulse {
	public:
		string name;
		int noA;
		int noB;
		double Amp1;
		double Amp2;
		double delay;
		double Tr;
		double Td;
		double Ton;
		double period;
		double cycles;
		Vpulse();
};

class Idc {
	public:
		string name;
		int noA;
		int noB;
		double I;
		Idc();
};
class Isin {
	public:
		string name;
		int noA;
		int noB;
		double I;
		double Amp;
		double Freq;
		double delay;
		double amort;
		double lag;
		double cycles;
		Isin();
};
class Ipulse {
	public:
		string name;
		int noA;
		int noB;
		double Amp1;
		double Amp2;
		double delay;
		double Tr;
		double Td;
		double Ton;
		double period;
		double cycles;
		Ipulse();
};
class AmpOp {
	public:
		string name;
		int noAi;
		int noBi;
		int noAo;
		int noBo;
		AmpOp();
};


class And {
	public:
		string name;
		int noAi;
		int noBi;
		int no;
		double Vout;
		double Rout;
		double Cin;
		double A;
		And();

}
class Nand {
	public:
		string name;
		int noAi;
		int noBi;
		int no;
		int noBo;
		double Vout;
		double Rout;
		double Cin;
		double A;
		Nand();

};

class Or {
	public:
		string name;
		int noAi;
		int noBi;
		int no;
		double Vout;
		double Rout;
		double Cin;
		double A;
		Or();

}
class Nor {
	public:
		string name;
		int noAi;
		int noBi;
		int no;
		double Vout;
		double Rout;
		double Cin;
		double A;
		Nor();

}

class FlipFlop {
	public:
		string name;
		int noAQ;
		int noBQ;
		int nD;
		int nCk;
		int nReset;
		double Vout;
		double Rout;
		double Cin;
		FlipFlop();

};

class Monoestable {
	public:
		string name;
		int noAQ;
		int noBQ;
		int nTrig;
		int nReset;
		double Vout;
		double Rout;
		double Cin;
		double Time;
		Monoestable();
};

class Reset {
	public:
		string name;
		int nSet;
		int nReset;
		double Vout;
		double Cin;
		Reset();
};

class Comment {
	public:
		string content;
		Comment();
};

class TypeOfAna {
	public:
		double Tend;
		double step;
		string methode;
		double StepsPP;
		bool UCI;
		TypeOfAna();
};
*/
