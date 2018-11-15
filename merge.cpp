#include <mpi.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

void merge(){

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

	// Cambie la semilla
	srand(time(NULL));

	// Cronometro sincronizado
	MPI_Barrier(MPI_COMM_WORLD);
	double local_start = MPI_Wtime();
	int version = 1;
	int repartir = n / process_num;
	int * numeros = new int[n];
	int * numeros_ordenados = new int[n];
	int *sub_numeros = new int[repartir];
	// Si soy el master, genere numeros al azar y luego repartalos a todos los subprocesos
	if (process_ID == 0) {
		cout << "[INF] Lista Random: ";
		for (int i = 0; i < n; ++i) {
			numeros[i] = rand() % n;
			//numeros[i] = n - i;
			cout << numeros[i] << " ";
		}
		cout << endl;
	}

	// ORDENAMIENTO ----------------------------------------------------------------------------------
	// Separe en cada subproceso
	MPI_Scatter(numeros, repartir, MPI_INT, sub_numeros, repartir, MPI_INT, 0, MPI_COMM_WORLD);

	// Version 1
	// Cada subproceso hace solamente el sort
	if (version == 1) {
		std::sort(&sub_numeros[0], &sub_numeros[repartir]);
	}
	
	// Junte todo en cada proceso
	MPI_Gather(sub_numeros, repartir, MPI_INT, numeros_ordenados, repartir, MPI_INT, 0, MPI_COMM_WORLD);
	// -----------------------------------------------------------------------------------------------

	// MERGE -----------------------------------------------------------------------------------------
	// 
	if (version == 1) {
		if (process_ID == 0) {

			int contador = process_num;
			int offset = 1;

			// Pero el merge debe esta en el proceso 0 solamente
			while(contador!=1){
				contador /= 2;
				for (int merges = 0; merges < contador; merges++) {
					int corte = (n*offset) / (process_num / 2);
					int disp = merges*corte;
					cout << "[INF] De " << disp << " hasta " << corte + disp << endl;
					std::inplace_merge(&numeros_ordenados[disp], &numeros_ordenados[(corte/2)+disp], &numeros_ordenados[corte+disp]);

				}
				offset *= 2;
				cout << "[INF] Etapa: " << contador*2 << " a "  << contador << ": ";
				for (int i = 0; i < n; i++) {
					cout << numeros_ordenados[i] << " ";
				}
				cout << endl;
			}

			cout << "[INF] Lista Ordenada: ";
			for (int i = 0; i < n; i++) {
				cout << numeros_ordenados[i] << " ";
			}
			cout << endl;
		
		}
	}else if(version == 2){

		// Proceso 0 le manda los numeros ordenados en todos los procesos

		//int * mergeV2 = new int[n];

		int contador = process_num;
		int offset = 1;
		while (contador != 1) {
			contador /= 2;

			// Si soy el proceso 0, solo haga merge de la parte de el

			if (process_ID == 0) {

				int corte = (n*offset) / (process_num / 2);
				std::inplace_merge(&numeros_ordenados[0], &numeros_ordenados[(corte / 2)], &numeros_ordenados[corte]);
				offset *= 2;

				for (int i = 1; i < contador; ++i) {
					int* mensaje = new int[corte];

					MPI_Recv(mensaje, corte, MPI_INT, 0, 0, MPI_COMM_WORLD);
					for (int j = 0; j < corte; j++) {
						numeros_ordenados[j]
					}
					delete[] mensaje;
				}

				for (int q = 1; q < comm sz; q++) {
					MPI_Recv(greeting, MAX_STRING, MPI_CHAR, q, 0, MPI_COMM_WORLD,
						MPI_STATUS_IGNORE);
				}

			}else if (process_ID < contador && contador!=1) {

				int corte = (n*offset) / (process_num / 2);
				int disp = process_ID * corte;

				std::inplace_merge(&numeros_ordenados[disp], &numeros_ordenados[(corte / 2) + disp], &numeros_ordenados[corte + disp]);

				// Cuando ya todos calcularon su parte, solo asi se puede continuar
				MPI_Barrier(MPI_COMM_WORLD);

				// Todos los procesos ahora tienen sincronizados 
				MPI_Send(numeros_ordenados, strlen(greeting) + 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

				offset *= 2;

			}*/


			/*
			// Cada proceso agrupa la memoria
			if (process_ID < contador) {

				int corte = (n*offset) / (process_num / 2);
				int disp = process_ID * corte;

				std::inplace_merge(&numeros_ordenados[disp], &numeros_ordenados[(corte / 2) + disp], &numeros_ordenados[corte + disp]);
				
				// Cuando ya todos calcularon su parte, solo asi se puede continuar
				MPI_Barrier(MPI_COMM_WORLD);

				// Todos los procesos ahora tienen sincronizados 
				MPI_Send(greeting, strlen(greeting) + 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

				offset *= 2;

			}*/
		}

		// Imprime la lista ordenada si es el proceso maestro
		if (process_ID == 0) {
			cout << "[INF] Lista Ordenada: ";
			for (int i = 0; i < n; i++) {
				cout << numeros_ordenados[i] << " ";
			}
		}

		//delete[] mergeV2;

	}
	// ------------------------------------------------------------------------------------------------------------------------------
	// Elimine la memoria
	delete[] numeros;
	delete[] numeros_ordenados;
	delete[] sub_numeros;

	// Cronometro final
	double total_elapsed;
	double local_elapsed = MPI_Wtime() - local_start;

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