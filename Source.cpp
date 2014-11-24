#include <iostream>
#include <iomanip>
#include <math.h>
#include <Windows.h>
#include <windows.system.h>
#include "Map.hpp"

using namespace std;

int main(){
	Map m = Map(9);
	system("cls");
	cout << m.getSeed();
	m.generate(0.5);	

	/*
	* Estas son algunas llamadas con las que ver el funcionamiento general, y como son
	* los argumentos de las llamadas:
	* 1 - Muestra el mapa con perspectiva Izquierda a Derecha desde la posicion 10,10 de la terminal, con grosor 2
	* 2 - Muestra a la derecha de esta, una vista superior del mismo mapa
	* 3 - Muestra el mapa con perspectiva Derecha a Izquierda desde la posicion 10,330 de la terminal, con grosor 2
	* 3 - Muestra debajo de la vista superior la escala de colores utilizada (normal y suave)
	*
	* A partir de aqui, todo es probar y modificar los metodos de la clase Map, y ver como funcionan
	*
	* NOTA: Conviene modificar el tamaño y la posicion de la terminal para que se vea bien, ponla a la izquierda, y que 
	* sea suficientemente grande
	*/
	//m.mostrarCorte3DRLQuick(500,0,2);
	//m.borrarCorte3DLRQuick(0,10,2);
	//m.mostrarCorte3DLR(0, 10, 2);
	//m.mostrarVistaPlanta(0, 10, 2, 2);
	m.mostrarCorte3DLRQuick(0, 10, 1);
	m.mostrarCorte3DLR(0, 10, 1);
	//m.mostrarVistaPlanta(400, 10, 3,3);
	//m.mostrarCorte3DRL(0, 330, 2);
	//m.mostrarEscala(400, 400);
	return 0;
}
