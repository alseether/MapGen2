#include <iostream>
#include <iomanip>
#include <set>
#include <math.h>
#include <time.h>
#include <Windows.h>

class Map {
private:

	// ATRIBUTOS DE LA LOGICA DEL MAPA

	/*
	* roughness indica como de arisco es el terreno generado.
	* Los valores normales estan entre 0 y 1, aunque puede tomar valores mayores de 1, creando mapas muy irregulares
	* Cuanto mayor es el valor de roughness, mas variacion puede haber entre dos pixeles adyacentes
	*/
	float roughness;
	
	/*
	* size indica el largo del lado de la matriz (cuadrada) de valores de altura del terreno
	* size se calcula a partir de un nivel de detalle, de tal forma, size siempre es un valor del tipo:
	*	(2^detalle) + 1
	* O lo que es lo mismo, una potencia de 2 mas uno (5,9,17,33,...)
	*/
	int size;
	
	/*
	* max es size-1. Dentro de la implementación, es útil, ya que indica el ultimo valor valido para acceder a la matriz.
	* La matriz se declara de tamaño size x size, con posiciones validas desde 0 hasta size-1.
	* Esta variable facilita la comprension del codigo y la implementacion (Evita poner size-1 en todos lados)
	*/
	int max;
	
	/*
	* map es la matriz donde se guardan los valores de altura del terreno. Notese que no es un array bidimensional.
	* Para acceder a la posicion (x,y) de la matriz (se puede acceder con el metodo get()), seria:
	*	map[x + size*y];
	*/
	float *map;
	
	/*
	* higher guardara, una vez generados los valores de altura del mapa, el valor mas alto de todo el mapa
	*/
	float higher;
	
	/*
	* lower, al igual que higher, guardara el valor mas bajo del mapa (NOTA: puede seer un valor negativo)
	*/
	float lower;
	
	/*
	* guarda la seed con la que se ha generado el mapa
	*/
	int seed;
	

	// ATRIBUTOS DE LA REPRESENTACION DEL MAPA

	/*
	* altoMapa sera el valor maximo (en pixeles) que se utilizara para representar una altura (por defecto, 200 pixeles)
	* Asi pues, se considera que higher es altoMapa, y lower sera 0.
	* Una altura entre higher y lower, sera una altura entre 0 y altoMapa.
	*/
	int altoMapa;

	/*
	* alturaAgua sera el nivel a partir del cual se considera que hay agua en el mapa, siendo 0 el nivel mas alto de
	* agua y altoMapa el mas bajo (sin agua);
	*/
	int alturaAgua;
	

	/*
	* hdc es el handle de la ventana de salida (por defecto de la consola)
	*/
	HDC hdc;
	

	// METODOS PRIVADOS

	/**
	* Obtiene el valor de la posicion (x,y) del mapa. Devuelve -1 si la posicion es invalida y el valor en caso contrario
	*/
	float get(int x, int y){
		if (x < 0 || x > this->max || y < 0 || y > this->max) return -1;
		return this->map[x + this->size * y];
	}

	/**
	* Establece el valor de la posicion (x,y) del mapa a val.  No hace nada si la posicion es invalida
	*/
	void set(int x, int y, float val){
		if (x < 0 || x > this->max || y < 0 || y > this->max) return;
		this->map[x + this->size * y] = val;
	}

	/**
	* Calcula la media de 4 valores float (usado en diamantes y cuadrados).
	* Si algun elemento del conjunto es -1 (por intentar hacer media con una posicion fuera de rango), este valor se descarta
	* y se hace la media de el resto de valores validos.
	* Devuelve la media
	*/
	float average(float* values) {
		float suma = 0;
		int elementos = 0;
		for (int i = 0; i < 3; ++i){
			if (values[i] != -1){
				suma += values[i];
				++elementos;
			}
		}
		return suma / elementos;
	}

	/**
	* Realiza la media (cuadrado) de una posicion (x,y) mas un offset dado
	*/
	void square(int x, int y, int size, float offset) {
		float valores[4];
		valores[0] = this->get(x - size, y - size);	  // upper left
		valores[1] = this->get(x + size, y - size);   // upper right
		valores[2] = this->get(x + size, y + size);	  // lower right
		valores[3] = this->get(x - size, y + size);	  // lower left
		float ave = average(valores);
		this->set(x, y, ave + offset);
	}

	/**
	* Realiza la media (diamante) de una posicion (x,y) mas un offset dado
	*/
	void diamond(int x, int y, int size, float offset) {
		float valores[4];
		valores[0] = this->get(x, y - size);	// top
		valores[1] = this->get(x + size, y);	// right
		valores[2] = this->get(x, y + size);	// bottom
		valores[3] = this->get(x - size, y);	// left
		float ave = average(valores);
		this->set(x, y, ave + offset);
	}

	/**
	* Rellena el mapa con los valores de altura, mediante el algoritmo Diamond-Square, de forma recursiva.
	* Actua sobre TODOS los sectores cuadrados del mapa, de lado size. No confundir con this->size,
	* aqui size cada vez es dos veces mas pequeño, actuando primero sobre un cuadrado de tamaño
	* size x size, luego size/2 x size/2, y asi recursivamente, hasta que el lado es 2, donde no se puede realizar
	* ningun calculo mas
	*/
	void divide(int size) {
		int x, y, half = size / 2;
		float scale = this->roughness * size;
		/*
		* scale tiene la funcion de darle "menos peso" a roughness cuanto mas pequeña es la seccion a tratar.
		* Esto evita que haya grandes diferecias de altura en casillas adyacentes, aun poniendo un roughness
		* alto
		*/
		if (half < 1) return;	// CASO BASE, cuando se tratan secciones de 2x2

		for (y = half; y < this->max; y += size) {
			for (x = half; x < this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				square(x, y, half, r * scale * 2 - scale);
			}
		}
		/*
		* Primero se calculan TODAS las medias (tipo square) para todas las subdivisiones de tamaño size x size del mapa
		* Estas medias establecen valores que son necesarios para calcular la medias tipo diamond
		* No voy a explicar la forma de avance de los bucles for, porque no es trivial a simple vista.
		* Notese que el ultimo valor pasado a la funcion square (offset), tiene un factor aleatorio (entre 0 y 1),
		* que afecta a scale, permitiendo asi que la media calculada para una posicion pueda variar del valor exacto.
		*/
		for (y = 0; y <= this->max; y += half) {
			for (x = (y + half) % size; x <= this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				diamond(x, y, half, r * scale * 2 - scale);
			}
		}
		/*
		* Despues se calculan TODAS las medias (tipo diamond), para los puntos medios de los lados del la subdivision de 
		* tamaño size x size, esto es:
		*
		*	o-------=-------o		o---=---o---=---o			Los simbolos = y ! marcan las casillas sobre las que se 
		*	|       |       |		|   |   |   |   |			calculara la media diamond para la llamada actual
		*	|       |       |		!---X---!---X---!			Considerando los puntos o como casillas con valor de
		*	|       |       |		|   |   |   |   |			altura calculado
		*	!-------X-------!  -->	o---=---o---=---o			Los simbolos X son las casillas sobre las que se calculara
		*	|       |       |		|   |   |   |   |			la media square para la llamada actual
		*	|       |       |		!---X---!---X---!
		*	|       |       |		|   |   |   |   |
		*	o-------=-------o		o---=---o---=---o
		*
		* Siendo este el mapa, de tamaño (17 x 17) (se incluyen los puntos o), se estarian, en esta fase, calculando las medias
		* Para cuadrados  de (8 x 8), ya que la primera llamada a divide() se hace con el valor this->max, no con size (porque es
		* impar).
		* Si se sigue el algoritmo, se vera que para esta llamada inicial, solo se calcula el valor de la casilla marcada con X
		* (una media de tipo square), y luego se pasa a calcular las medias de los puntos marcados con = y !, tras hacerlo,
		* se llama a divide con size/2, y como se puede ver en el cuadrado de la derecha, los valores para las esquinas de cada
		* cuadrado de tamaño size/2 ya estan calculadas por la llamada anterior (representados con o)
		*/

		divide(size / 2);
	}

	/**
	* Se puede considerar una excepcion de divide(), se usa cuando se modifica un sector del mapa, se llama en generateSector()
	* donde se puede establecer la altura base del punto central, que influye en todas el terreno colindante.
	* Para esta primera llamada a divide() (la cual es divideSector), no se calcula la media para el punto medio del sector,
	* sino que se establece directamente al valor centralHeight, para el resto de puntos del sector, se sigue el algoritmo
	* normal de divide
	*
	* NO ES UNA FUNCION RECURSIVA, pero si que llama a divide, que si lo es
	*/
	void divideSector(int size, float centralHeight) {
		int x, y, half = size / 2;
		float scale = this->roughness * size;
		if (half < 1) return;	// por si se tratan secciones de 2x2
	
		this->set(half, half, centralHeight);	// La PRIMERA vez no se calcula una media square, se pone directamente este valor

		for (y = 0; y <= this->max; y += half) {
			for (x = (y + half) % size; x <= this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				diamond(x, y, half, r * scale * 2 - scale);
			}
		}
		divide(size / 2);	// Notese que la llamada es a divide, y no a divideSector()
	}

	/**
	* Devuelve el valor representativo de altura de una casilla entre 0 - altoMapa
	* Para lower el valor devuelto sera 0
	* Para higher el valor devuelto sera altoMapa;
	*/
	int calculaAlto(float n){
		int tamRangoAlturas = higher - lower;	// El rango de alturas siempre sera higher-lower, incluso si lower < 0
		return (tamRangoAlturas == 0) ? 200 : (n - lower) * 200 / tamRangoAlturas;
	}

	/**
	* Devuelve el color correspondiente a un valor de altura (representativo, entre 0-altoMapa)
	*/
	COLORREF calculaColor(int alto){
		COLORREF color;
		int offset = (alto * 128 / altoMapa);
		/*
		* offset aplica una variacion al color base, para que alturas diferentes en un rango,
		* tengan distinta intensidad de ese mismo color, haciendolo mas visual y realista
		*/
		int valorA = 128 + offset;
		int valorB = 128 - offset;
		if (alto < altoMapa / 7){
			/*
			* Blancos para las cimas
			* Valores de alto: de 0  a altoMapa/7
			* Valores de blancos: de 255 a 210
			* La relacion se calcula con una regla de 3
			*/
			valorA = 255 - (alto * 45 / (altoMapa/7));
			color = RGB(valorA,valorA,valorA);
		}
		else if (alto < 2 * altoMapa / 7){
			/*
			* Grises claros para laderas de montaña
			* Valores de alto: de altoMapa/7 a 2 * altoMapa/7
			* Valores de gris: de 140 a 64 (76 valores)
			*/
			int altoMin = altoMapa / 7;
			int altoMax = 2 * altoMapa / 7;
			valorA = 140 - ((alto - altoMin) * 76 / (altoMax - altoMin));
			color = RGB(valorA, valorA, valorA);
		}
		else if (alto < 3 * altoMapa / 7){
			/*
			* Verdes oscuros para pies de montaña, simulando bosques
			* Valores de alto: de 2 * altoMapa/7 a 3 * altoMapa/7
			* Valores de verde: de 64 a 128 (64 valores)
			*/
			int altoMin = 2 * altoMapa / 7;
			int altoMax = 3 *  altoMapa / 7;
			valorA = 64 + ((alto - altoMin) * 64 / (altoMax - altoMin));
			color = RGB(0, valorA, 0,);
		}
		else if (alto < 4 * altoMapa / 7){
			/*
			* Amarillos claros para simular arena de desiertos o playas
			* Valores de alto: de 3*altoMapa/7 a 4*altoMapa/7
			* Valores de verde: de 223 a 255 (32 valores)
			*/
			int altoMin = 3 * altoMapa / 7;
			int altoMax = 4 * altoMapa / 7;
			valorA = 223 + ((alto - altoMin) * 32 / (altoMax - altoMin));
			color = RGB(255,valorA,128);
		}
		else if (alto < 5 * altoMapa / 7){
			/*
			* Marrones oscuros para barro cerca del agua
			* Valores de alto: de 4*altoMapa/7 a 5*altoMapa/7
			* Valores de rojo: de 100 a 50
			* Valores de verde/azul: de 48 a 24 (24 valores)
			*/
			int altoMin = 4 * altoMapa / 7;
			int altoMax = 5 * altoMapa / 7;
			valorA = 100 - ((alto - altoMin) * 50 / (altoMax -altoMin));
			valorB = 48 - ((alto - altoMin) * 24 / (altoMax - altoMin));
			color = RGB(valorA, valorB, valorB);
		}
		else if (alto < 6 * altoMapa / 7){
			/*
			* Grises y negros para zonas pantanosas profundas
			* Valores de alto: de 5*altoMapa/7 a 6*altoMapa/7
			* Valores de gris: de 40 a 15 (25 valores)
			*/
			int altoMin = 5 * altoMapa / 7;
			int altoMax = 6 * altoMapa / 7;
			valorA = 40 - ((alto-altoMin) * 25 / (altoMax -altoMin));
			color = RGB(valorA, valorA, valorA);
		}
		else{	// (alto < altoMapa)
			/*
			* "Casi" negro para zonas muy profundas
			* Valores de alto: 6*altoMapa/7 a altoMapa
			* Valores de negro: 3
			*/
			color = RGB(10,10,10);
		}
		return color;
		/*
		* La eleccion de los colores no sigue ningun patron, he ido probando combinaciones hasta encontrar colores
		* bastante diferentes entre capas, pero que tambien varien de intensidad dentro de la misma capa
		* Los colores por capas se pueden ver usando el metodo mostrarEscala()
		* NOTA: Si, ya se que los colores son una mierda, sois libres de cambiarlos y si encontrais una combinacion mejor,
		* hacedmelo saber
		*/
	}

	/**
	* Devuelve un color en funcion de la altura, a diferencia de calculaColor(), los colores devueltos no son tan radicalmente
	* distintos, sino que se encuentran en una gama de grises, mas claros cuanta mayor altura
	*/

	COLORREF calculaColorSuave(int alto){
		COLORREF color;
		int valorA = 255 - alto;
		color = RGB(valorA, valorA, valorA);
		return color;
	}

	/**
	* Devuelve el color del agua correspondiente a un valor de altura (representativo, entre alturaAgua-altoMapa)
	*/
	COLORREF calculaColorAgua(int alto){
		COLORREF color;
		int valorA, valorB;
		/*
		* Gama de azules entre RGB(3,35,239) a RGB(3,11,78)
		* Valores de alto: desde alturaAgua a altoMapa
		* Valores de verde: desde 35 a 11 (24 valores)
		* Valores de azul: desde 239 a 78 (161 valores)
		*/
		valorA = 35 - ((alto - alturaAgua) * 24 / (altoMapa - alturaAgua));
		valorB = 239 - ((alto - alturaAgua) * 161 / (altoMapa - alturaAgua));
		color = RGB(3, valorA, valorB);
		return color;
	}

public:

	// CONTRUCTORA SIN SEMILLA
	Map(int detail){
		this->size = pow(2,detail) +1;
		this->max = size - 1;
		this->map = new float[size * size];
		this->altoMapa = 200;
		this->alturaAgua = 3 * altoMapa / 5; // a partir de 3/5 de la altura hay agua
		this->seed = time(NULL);
		srand(this->seed);
		this->hdc = GetDC(GetConsoleWindow()); // Get the DC from console
	}

	// CONSTRUCTORA CON SEMILLA
	Map(int detail, int seed){
		this->size = pow(2, detail) + 1;
		this->max = size - 1;
		this->map = new float[size * size];
		this->altoMapa = 200;
		this->alturaAgua = 3 * altoMapa / 5; // a partir de 3/5 de la altura hay agua
		this->seed = seed;
		srand(this->seed);
		this->hdc = GetDC(GetConsoleWindow());
	}

	// METODOS PUBLICOS

	/**
	* Inicializa el mapa con los valores de altura (llamada a divide), y establece los valores higher y lower
	*/
	void generate(float roughness) {
		this->roughness = roughness;
		// Roughness, valor entre 0 y 1 (aunque puede ser > 1)
		this->set(0, 0, this->max * 3 / 4);
		this->set(this->max, 0, this->max * 3 / 4);
		this->set(this->max, this->max, this->max * 3 / 4);
		this->set(0, this->max, this->max * 3 / 4);
		/* 
		* Se pone un valor igual para todas las esquinas (size/3). Esto se puede variar, si se quiere, por ejemplo
		* un mapa que caiga o que tenga una elevacion hacia una o varia esquinas.
		*/

		divide(this->max);
		this->higher = findHigher();
		this->lower = findLower();
	};

	/**
	* Inicializa un sector con los valores de altura (llamada a divideSector), y establece los valores higher y lower
	* Este metodo parte del hecho de que los valores ya estan establecidos.
	* Su uso se limita al ambito de modificaSector(), donde se crea un nuevo mapa, mas pequeño que el original, y sobre el
	* se recalcula un nuevo terreno.
	*/
	void generateSector(float roughness, int centralHeight) {
		this->roughness = roughness;
		// Roughness, valor entre 0 y 1 (aunque puede ser > 1)
		divideSector(this->max, centralHeight);
		this->higher = findHigher();
		this->lower = findLower();
	};

	/**
	* Muestra el mapa en 2D, como una vista de planta (desde arriba), con un ancho y alto de pixel dados
	* La llamada a la funcion sin argumentos establece un alto y un  ancho de 5 pixeles por cada valor del
	* mapa.
	*/
	void borrarVistaPlanta(){
		mostrarVistaPlanta(0, 0, 5, 5, RGB(0,0,0));
	}
	void borrarVistaPlanta(int desdeX, int desdeY){
		mostrarVistaPlanta(desdeX, desdeY, 5, 5, RGB(0, 0, 0));
	}
	void borrarVistaPlanta(int pixelWidth, float pixelHeight){
		mostrarVistaPlanta(0, 0, pixelWidth, pixelHeight, RGB(0, 0, 0));
	}
	void borrarVistaPlanta(int desdeX, int desdeY, int pixelWidth, float pixelHeight){
		mostrarVistaPlanta(desdeX, desdeY, pixelWidth, pixelHeight, RGB(0, 0, 0));
	}
	void mostrarVistaPlanta(){
		mostrarVistaPlanta(0,0,5, 5, RGB(255,255,255));
	}
	void mostrarVistaPlanta(int desdeX, int desdeY){
		mostrarVistaPlanta(desdeX, desdeY, 5, 5, RGB(255, 255, 255));
	}
	void mostrarVistaPlanta(int pixelWidth, float pixelHeight){
		// Es float, solo para poder tener dos llamadas con dos argumentos, aunque ambos son int
		mostrarVistaPlanta(0, 0, pixelWidth, pixelHeight, RGB(255, 255, 255));
	}
	void mostrarVistaPlanta(int desdeX, int desdeY, int pixelWidth, float pixelHeight){
		mostrarVistaPlanta(desdeX, desdeY, pixelWidth, pixelHeight, RGB(255, 255, 255));
	}
	void mostrarVistaPlanta(int desdeX, int desdeY, int pixelWidth, int pixelHeight, COLORREF c){
		bool borrar = (c == RGB(0, 0, 0));
		int anchoPixel = pixelWidth;
		int altoPixel = pixelHeight;
		int x, y; 
		int alto;
		COLORREF color;
		for (int i = 0; i < size*size; i++)
		{
			alto = calculaAlto(map[i]);
			if (borrar){
				color = c;
			}
			else{
				color = calculaColor(alto);
			}
			x = (i % size);
			y = (i / size);
			for (int j = desdeX; j < desdeX + anchoPixel; ++j){
				for (int k = desdeY; k < desdeY + altoPixel; ++k)
				SetPixel(hdc, (anchoPixel * x) + j, (altoPixel * y) + k, color);
				// Prototipo: SetPixel(HDC hdc, int x, int y, COLORREF color)
			}
		}
	}

	/**
	* Muestra la secuencia completa de cortes (capas) del mapa desde la capa y=0 hasta la capa
	* y=size
	* La llamada sin argumentos establece que el numero de lineas pintadas por casilla sea 1, desde el comienzo de
	* la ventana
	*/

	void mostrarCorte(){
		mostrarCorte(0,0,1);
	}
	void mostrarCorte(int grosor){
		mostrarCorte(0, 0, grosor);
	}
	void mostrarCorte(int desdeX, int desdeY, int grosor){
		int x, y;
		COLORREF color, colorS;
		COLORREF negro = RGB(0, 0, 0);
		int alto;
		for (int i = 0; i < size*size; i++)
		{			
			alto = calculaAlto(map[i]);
			color = calculaColor(alto);
			colorS = calculaColorSuave(alto);
			x = (i % size);
			y = (i / size);
			for (int j = desdeY; j < desdeY + altoMapa; ++j){
				for (int k = 0; k < grosor; ++k){
					if (j < alto){
						SetPixel(hdc, (desdeX + (grosor*x + k)), j, negro);
					}
					else{
						SetPixel(hdc, (desdeX + (grosor*x + k)), j, color);
					}
				}
			}
			SetPixel(hdc, (desdeX + (grosor*x)), alto, colorS);
		}
	}

	/**
	* Muestra una perspectiva rellena de izquierda a derecha (LR -- LeftToRight) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	void borrarCorte3DLR(){
		mostrarCorte3DLR(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DLR(int grosor){
		mostrarCorte3DLR(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DLR(int desdeX, int desdeY, int grosor){
		mostrarCorte3DLR(desdeX, desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DLR(){
		mostrarCorte3DLR(0, 0, 1,RGB(255,255,255));
	}
	void mostrarCorte3DLR(int grosor){
		mostrarCorte3DLR(0, 0, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DLR(int desdeX, int desdeY, int grosor){
		mostrarCorte3DLR(desdeX, desdeY, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DLR(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));	// Si el color a pasar es negro, es que quiero borrar
		int x, y; 
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				gris = c;
				agua = c;
			}
			else{
				color = calculaColor(alto);
				gris = calculaColorSuave(alto);
				agua = calculaColorAgua(alto);
			}
			
			x = (i % size) * grosor;
			y = (i / size);
			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x + k) + y)), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + altoMapa; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x+k) +y)), (j + offset), color);
				}
			}
			if (alto % 10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x + k) + y)), (desdeY + alto + offset), gris);
				}
			}
			//SetPixel(hdc, (desdeX + (grosor*(x + y))), (desdeY + alto + offset), colorS);
		}
	}

	/**
	* Muestra una perspectiva de puntos de izquierda a derecha (LR -- LeftToRight) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre puntos)
	*/
	void borrarCorte3DLRQuick(){
		mostrarCorte3DLRQuick(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DLRQuick(int grosor){
		mostrarCorte3DLRQuick(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DLRQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DLRQuick(desdeX,desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DLRQuick(){
		mostrarCorte3DLRQuick(0,0,1,RGB(255,255,255));
	}
	void mostrarCorte3DLRQuick(int grosor){
		mostrarCorte3DLRQuick(0, 0, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DLRQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DLRQuick(desdeX,desdeY, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DLRQuick(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));
		int x, y;
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		int end;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				agua = c;
				gris = c;
			}
			else{
				color = calculaColor(alto);
				gris = calculaColorSuave(alto);
				agua = calculaColorAgua(alto);
			}
			x = (i % size) * grosor;
			y = (i / size);
			(x == 0) ? end = altoMapa : end = alto + 10;
			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x + k) + y)), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + end; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x + k) + y)), (j + offset), color);
				}
			}
			if (alto % 10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + ((x + k) + y)), (desdeY + alto + offset), gris);
				}
			}
			/*
			* Notese que el for de la representacion rellena aqui desaparece, porque solo se pinta el punto de la posicion alto
			*/
		}
	}

	/**
	* Muestra una perspectiva rellena de derecha a izquierda (RL -- RightToLeft) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	void borrarCorte3DRL(){
		mostrarCorte3DRL(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DRL(int grosor){
		mostrarCorte3DRL(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DRL(int desdeX, int desdeY, int grosor){
		mostrarCorte3DRL(desdeX, desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DRL(){
		mostrarCorte3DRL(0, 0, 1, RGB(255,255,255));
	}
	void mostrarCorte3DRL(int grosor){
		mostrarCorte3DRL(0, 0, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DRL(int desdeX, int desdeY, int grosor){
		mostrarCorte3DRL(desdeX, desdeY, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DRL(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));
		int x, y;
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				agua = c;
				gris = c;
			}
			else{
				color = calculaColor(alto);
				agua = calculaColorAgua(alto);
				gris = calculaColorSuave(alto);
			}
			x = (i % size) * grosor;
			y = (i / size);

			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k - y)), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + altoMapa; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k  - y)), (j + offset), color);
				}
			}
			if(alto%10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k - y)), (desdeY + alto + offset), gris);
				}
			}
		}
	}

	/**
	* Muestra una perspectiva de puntos de derecha a izquierda (RL -- RightToLeft) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre puntos)
	*/
	void borrarCorte3DRLQuick(){
		mostrarCorte3DRLQuick(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DRLQuick(int grosor){
		mostrarCorte3DRLQuick(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DRLQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DRLQuick(desdeX, desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DRLQuick(){
		mostrarCorte3DRLQuick(0,0,1, RGB(255,255,255));
	}
	void mostrarCorte3DRLQuick(int grosor){
		mostrarCorte3DRLQuick(0, 0, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DRLQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DRLQuick(desdeX, desdeY, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DRLQuick(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));
		int x, y;
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		int end;
		for (int i = 0; i < size*size; ++i){
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				agua = c;
				gris = c;
			}
			else{
				color = calculaColor(alto);
				agua = calculaColorAgua(alto);
				gris = calculaColorSuave(alto);
			}
			x = (i % size) * grosor;
			y = (i / size);
			(x == (size-1) * grosor) ? end = altoMapa : end = alto + 10;
			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k - y)), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + end; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k - y)), (j + offset), color);
				}
			}
			if (alto % 10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + (size + x + k - y)), (desdeY + alto + offset), gris);
				}
			}
		}
	}

	/**
	* Muestra una perspectiva rellena frontal del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	void borrarCorte3DFront(){
		mostrarCorte3DFront(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DFront(int grosor){
		mostrarCorte3DFront(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DFront(int desdeX, int desdeY, int grosor){
		mostrarCorte3DFront(desdeX, desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DFront(){
		mostrarCorte3DFront(0, 0, 1, RGB(255,255,255));
	}
	void mostrarCorte3DFront(int grosor){
		mostrarCorte3DFront(0, 0, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DFront(int desdeX, int desdeY, int grosor){
		mostrarCorte3DFront(desdeX, desdeY, grosor, RGB(255,255,255));
	}
	void mostrarCorte3DFront(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));
		int x, y;
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset ++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				agua = c;
				gris = c;
			}
			else{
				color = calculaColor(alto);
				agua = calculaColorAgua(alto);
				gris = calculaColorSuave(alto);
			}
			x = (i % size) * grosor;
			y = (i / size);

			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + altoMapa; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (j + offset), color);
				}
			}
			if (alto % 10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (desdeY + alto + offset), gris);
				}
			}
		}
	}

	/**
	* Muestra una perspectiva de puntos frontal del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre puntos)
	*/
	void borrarCorte3DFrontQuick(){
		mostrarCorte3DFrontQuick(0, 0, 1,RGB(0,0,0));
	}
	void borrarCorte3DFrontQuick(int grosor){
		mostrarCorte3DFrontQuick(0, 0, grosor, RGB(0,0,0));
	}
	void borrarCorte3DFrontQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DFrontQuick(desdeX, desdeY, grosor, RGB(0,0,0));
	}
	void mostrarCorte3DFrontQuick(){
		mostrarCorte3DFrontQuick(0,0,1,RGB(255,255,255));
	}
	void mostrarCorte3DFrontQuick(int grosor){
		mostrarCorte3DFrontQuick(0, 0, grosor,RGB(255,255,255));
	}
	void mostrarCorte3DFrontQuick(int desdeX, int desdeY, int grosor){
		mostrarCorte3DFrontQuick(desdeX, desdeY, grosor,RGB(255,255,255));
	}
	void mostrarCorte3DFrontQuick(int desdeX, int desdeY, int grosor, COLORREF c){
		bool borrar = (c == RGB(0,0,0));
		int x, y;
		COLORREF color, agua, gris;
		int offset = -1;
		int alto;
		int end;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset ++;
			}
			alto = calculaAlto(map[i]);
			if(borrar){
				color = c;
				agua = c;
				gris = c;
			}
			else{
				color = calculaColor(alto);
				agua = calculaColorAgua(alto);
				gris = calculaColorSuave(alto);
			}
			x = (i % size) * grosor;
			y = (i / size);
			if (alto > alturaAgua){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (desdeY + alturaAgua + offset), agua);
				}
			}
			for (int j = desdeY + alto; j < desdeY + alto + 10; ++j){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (j + offset), color);
				}
			}
			if (alto % 10 == 0){
				for (int k = 0; k < grosor; ++k){
					SetPixel(hdc, (desdeX + x + k), (desdeY + alto + offset), gris);
				}
			}
		}
	}

	/**
	* Muestra los distintos colores que se utilizan para cada altura del mapa
	* La llamada sin argumentos lo muestra al comienzo de la pantalla
	*/
	void mostrarEscala(){
		mostrarEscala(0, 0);
	}
	void mostrarEscala(int desdeX, int desdeY){
		COLORREF color;
		for (int i = desdeX; i < desdeX + 50; ++i)
		{
			for (int j = 0; j < altoMapa; ++j){
				color = calculaColor(j);
				SetPixel(hdc, i, (desdeY + j), color);
				color = calculaColorSuave(j);
				SetPixel(hdc, (i + 100), (desdeY + j), color);
			}
		}
	}

	/**
	* Borra tanto los pixeles dibujados en la terminal, como los caracteres impresos con cout o similares
	* Notese que para borrar pixeles, simplemente mueve la pantalla de sitio y la vuelve a colocar donde estaba
	* borrar() no funciona si la terminal se ha movido de su posicion original (aun no se porque)
	*/
	void borrar(){
		system("cls");
		WINDOWPLACEMENT oldPos;
		GetWindowPlacement(GetConsoleWindow(), &oldPos);
		SetWindowPlacement(GetConsoleWindow(), &oldPos);
	}

	/**
	* Devuelve el valor mas alto del mapa
	*/
	float findHigher(){
		float mayor = 0;
		for (int i = 0; i < this->size*this->size; ++i){
			if (map[i] > mayor){
				mayor = map[i];
			}
		}
		return mayor;
	}

	/**
	* Devuelve el valor mas bajo del mapa (Puede ser menor que 0)
	*/
	float findLower(){
		float menor = size;
		for (int i = 0; i < this->size*this->size; ++i){
			if (map[i] < menor){
				menor = map[i];
			}
		}
		return menor;
	}

	/**
	* Devuelve la semilla del mapa actual
	*/
	int getSeed(){
		return this->seed;
	}

	/**
	* Referido a las distintas persepectivas desde las que se puede ver el mapa
	* De izquierda a derecha, frontalmente, o de derecha a izquierda
	*/
	enum Pers{
		LR,
		FRONT,
		RL
	};

	// TO-DO

	/**
	* Muestra el mapa desde la posicion indicada, con un grosor dado, desde la perspectiva indicada, y hasta los limites dados
	*/
	void muestraMapa(int desdeX, int desdeY, int grosor, Pers perspectiva, int limitX, int limitY){
	}

	/**
	* Modifica el sector comprendido entre los puntos (origX,origY) y (origX+1+2^lado,origY+1+2^lado), con un nuevo valor de roughness
	* y permitiendo establecer el valor del punto central del sector, para, por ejemplo, crear montañas o valles
	* Solo se pueden modificar sectores cuyo lado sea una potencia de dos, ya que el algoritmo Diamond-Square solo actua en 
	* matrices de lado (2^n + 1)
	* No se haran sobre matrices de menos de lado 3.
	*
	* LA FUNCION ESTA IMPLEMENTADA, PERO PROVOCA CAMBIOS MUY BRUSCOS EN EL TERRENO, CONVIENE REVISARLO
	*/
	void modificaSector(int origX, int origY, int lado, float roughness, float centralHeight){
		if (origX >= 0 && origX < size && origY >= 0 && origY < size){
			int tam = pow(2, lado) + 1;
			int destX = origX + tam;
			int destY = origY + tam;
			if (destX >= 0 && destX < size && destY >= 0 && destY < size){
				Map* modified = new Map(lado);
				for (int i = origX, iM = 0; i < destX; ++i, ++iM){
					for (int j = origY, jM = 0; j < destY; ++j, ++jM){
						modified->set(iM, jM, this->get(i, j));
					}
				}
				modified->generateSector(roughness,centralHeight);
				for (int i = origX, iM = 0; i < destX; ++i, ++iM){
					for (int j = origY, jM = 0; j < destY; ++j, ++jM){
						float l = modified->get(iM, jM);
						this->set(i, j, l);
					}
				}
				delete modified;
			}
		}
	}

};