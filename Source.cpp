#include <iostream>
#include <iomanip>
#include <math.h>
#include <Windows.h>
#include <windows.system.h>
#include "Map.hpp"

using namespace std;

int main(){
	Map m = Map(7);
	system("cls");
	cout << m.getSeed();
	m.generate(0.5);	

	m.mostrarVistaPlanta(0, 0, 2, 2);	m.borrarVistaPlanta(0, 0, 2, 2);
	m.mostrarCorte3DLR(0, 0, 2);		m.borrarCorte3DLR(0, 0, 2);
	m.mostrarCorte3DRL(0, 0, 2);		m.borrarCorte3DRL(0, 0, 2);
	m.mostrarCorte3DFront(0, 0, 2);		m.borrarCorte3DFront(0, 0, 2);
	m.mostrarCorte3DLRQuick(0, 0, 2);	m.borrarCorte3DLRQuick(0, 0, 2);
	m.mostrarCorte3DRLQuick(0, 0, 2);	m.borrarCorte3DRLQuick(0, 0, 2);
	m.mostrarCorte3DFrontQuick(0, 0, 2); m.borrarCorte3DFrontQuick(0, 0, 2);

	return 0;
}
