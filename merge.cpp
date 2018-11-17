#include <mpi.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

bool esPadre(int nivel_arbol, int process_ID, int process_num) {
	if (process_ID == 0) return true;
	if (nivel_arbol == 8) {
		if (process_ID % 2 == 0) return true;
	}
	else if (nivel_arbol == 4) {
		if (process_ID == process_num / 2) return true;
	}
	return false;
}

bool esHijo(int nivel_arbol, int process_ID, int process_num) {
	if (process_ID == 0) return false;
	if (nivel_arbol == 8) {
		if (process_ID % 2 == 1) return true;
	}
	else if (nivel_arbol == 4) {
		if (process_num == 8) {
			if (process_ID == 2 || process_ID == 6) return true;
		}
		else if (process_num == 4) {
			if (process_ID % 2 == 1) return true;
		}
	}
	else if (nivel_arbol == 2) {
		if (process_ID == process_num / 2) return true;
	}
	return false;
}

int main(int argc, char* argv[]) {


	// Inicializacion básica de MPI
	int process_ID;
	int process_num;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_ID);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);

	// Modo de uso obligatorio
	if (argc < (2)) {
		cout << "Error: debe pasar: cantidad de numeros." << endl;
		cin.ignore();
		exit(0);
	}

	// Datos inciales del enunciado
	MPI_Status mpi_status;
	int n = strtol(argv[1], NULL, 10);

	// Notificacion de parámetros iniciales
	if (process_ID == 0) {
		cout << "[MPI] EL ambiente MPI ha iniciado correctamente." << endl;
		cout << "[MPI] Numero de procesos paralelos: " << process_num << endl;
		cout << "[INF] Numero de iteraciones (n): " << n << endl;
		cout << "[INF] Calculando, por favor espere..." << endl;
	}

	// Cambie la semilla para la lista random
	srand(time(NULL));

	// Cronometro sincronizado
	MPI_Barrier(MPI_COMM_WORLD);
	double local_start = MPI_Wtime();
	int version = 2;
	int* numeros = new int[n];
	int* numeros_ordenados = new int[n];
	int* numeros_ordenados_parte = new int[n / process_num];
	double local_elapsed;

	// Si soy el master, genere numeros al azar y luego repartalos a todos los subprocesos
	if (process_ID == 0) {
		//cout << "[INF] Lista Random: ";
		for (int i = 0; i < n; ++i) {
			numeros[i] = rand() % n;
			//cout << numeros[i] << " ";
		}
		//cout << endl;
	}

	// ORDENAMIENTO ----------------------------------------------------------------------------------
	// Separe en cada subproceso
	MPI_Scatter(numeros, n / process_num, MPI_INT, numeros_ordenados_parte, n / process_num, MPI_INT, 0, MPI_COMM_WORLD);

	// Cada subproceso hace solamente el sort de su parte
	std::sort(&numeros_ordenados_parte[0], &numeros_ordenados_parte[n / process_num]);

	// MERGE SORT PARALELO O SERIAL-------------------------------------------------------------------
	if (version == 1) {

		// Junte todo en el proceso 0, ya que el hará todo el trabajo
		MPI_Gather(numeros_ordenados_parte, n / process_num, MPI_INT, numeros_ordenados, n / process_num, MPI_INT, 0, MPI_COMM_WORLD);

		// Haga el merge el v1 con un sistema de arbol sencillo donde el proceso 0 hace todos los merges
		/*
			0 0  0 0  0 0  0  0
			\ /  \ /  \ /  \ /
			 0    0    0    0
			   \ /       \ /
				0         0
				  \     /
					 0
		*/

		if (process_ID == 0) {

			// El proceso hace todo el trabajo
			int contador = process_num;
			int offset = 1;

			// Pero el merge debe esta en el proceso 0 solamente
			// Repita hasta que solo quede 1 nivel
			while (contador != 1) {
				contador /= 2;

				// Cuantos merges debe hacer todo el hilo 0
				for (int merges = 0; merges < contador; merges++) {

					// Tamaño de las "tajadas" a analizar
					int corte = (n*offset) / (process_num / 2);

					// Con respecto al corte, que "tajada" que debe hacerle el merge
					int disp = merges * corte;
					std::inplace_merge(&numeros_ordenados[disp], &numeros_ordenados[(corte / 2) + disp], &numeros_ordenados[corte + disp]);
				}

				// Multiplique el offset para que se repita hasta que estemos el en nivel 1
				offset *= 2;
			}

			local_elapsed = MPI_Wtime() - local_start;

			// Imprima la lista ya ordenada
			cout << "[INF] Lista Ordenada (Version 1): ";
			for (int i = 0; i < n; i++) {
				cout << numeros_ordenados[i] << " ";
			}
			cout << endl;

		}
	}
	else if (version == 2) {

		// Haga el merge el v2 con un sistema de arbol donde se va resolviendo de esta manera
		/*
			0 1  2 3  4 5  6  7
			\ /  \ /  \ /  \ /
			 0    2    4    6
			   \ /       \ /
				0         4
				  \     /
					 0
		*/

		int* sublista_izquierda = new int[n];  // Lista izquierda del arbol y a su vez, del nodo actual
		int* sublista_derecha = new int[n];    // La lista del hijo derecho, ya que el izquierdo no se va enviar los datos a el mismo
		int sublista_size = (n / process_num);   // Tamaño de la lista
		int offset_arbol = 1;                  // Multiplo para encontrar a los hijos y padres (2^0, 2^1, 2^2, etc..)
		int nivel_arbol = process_num;         // Que nivel estamos, en el grafico el primer nivel seria 8, el segundo 4, y asi

		for (int i = 0; i < sublista_size; i++) {
			sublista_izquierda[i] = numeros_ordenados_parte[i];
		}

		// Cada proceso multiplo de 2 obtiene datos del proceso hijo derecha (los multiplos de 2 son los padres izquierdos)
		while (nivel_arbol > 1) {

			if (esPadre(nivel_arbol, process_ID, process_num)) {

				// Los padres seran los procesos con ID de multiplos de 2
				int* sublista_central = new int[n];
				int miHijo = process_ID + offset_arbol;

				// Reciba la segunda lista que le corresponde
				MPI_Recv(sublista_derecha, sublista_size, MPI_INT, miHijo, 0, MPI_COMM_WORLD, &mpi_status);

				// Hagales merge entre el mensaje del hilo hijo del arbol
				std::merge(&sublista_izquierda[0], &sublista_izquierda[sublista_size], &sublista_derecha[0], &sublista_derecha[sublista_size], &sublista_central[0]);

				// Ahora convierta la nueva lista en la lista izquierda, para formar la "recursivdad" colocando la barrera
				for (int i = 0; i < sublista_size * 2; i++) {
					sublista_izquierda[i] = sublista_central[i];
				}

				// Borre el buffer temporal
				delete[] sublista_central;

			}
			else if (esHijo(nivel_arbol, process_ID, process_num)) {
				int miPadre = process_ID - offset_arbol;
				MPI_Send(sublista_izquierda, sublista_size, MPI_INT, miPadre, 0, MPI_COMM_WORLD);

			}

			// Baje el nivel de 8 a 4, de 4 a 2, etc...
			nivel_arbol /= 2;

			// Cambie el offset de los nodos, por potencias de 2 siguiente.
			offset_arbol *= 2;
			sublista_size *= 2;

			MPI_Barrier(MPI_COMM_WORLD);

		}

		local_elapsed = MPI_Wtime() - local_start;

		// Imprime la lista ordenada si es el proceso maestro
		if (process_ID == 0) {
			cout << "[INF] Lista Ordenada (Version 2): ";
			for (int i = 0; i < n; i++) {
				cout << sublista_izquierda[i] << " ";
			}
			cout << endl;
		}
		delete[] sublista_izquierda;
		delete[] sublista_derecha;

	}
	// ------------------------------------------------------------------------------------------------------------------------------
	// Elimine la memoria
	delete[] numeros;
	delete[] numeros_ordenados;
	delete[] numeros_ordenados_parte;

	// Cronometro final
	double total_elapsed;

	MPI_Reduce(&local_elapsed, &total_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	// Termina MPI
	if (process_ID == 0) {
		cout << "[INF] Tiempo transcurrido: " << total_elapsed << endl;
		cout << "[MPI] EL ambiente MPI ha terminado correctamente." << endl;

		cin.ignore();
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
