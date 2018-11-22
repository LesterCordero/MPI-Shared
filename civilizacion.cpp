#include <mpi.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

int getRandom(int min, int max){
	return min + rand() % (( max + 1 ) - min);
}

int** creeMatrixDinamica(int x) {

	int **array2d = new int*[size];
	for (int i = 0; i < size; ++i) {
		array2d[i] = new int[size];
	}
	return array2d;
}

void destruirMatrixDinamica(int** m, int size) {
	for (int i = 0; i < size; ++i) {
		delete[] m[i];
	}
	delete[] m;
}

int main(int argc, char* argv[]) {

	// Inicialización básica de MPI
	int process_ID;
	int process_num;
	MPI_Status mpi_status;

	// Arranque el ambiente
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &process_ID);
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);

	// Modo de uso obligatorio para los parámetros
	if (argc < (2)) {
		cout << "Error de parametros: debe pasar en este orden, procesos, n-people, infect-chance, recover-chance, disease-time, infect-start-chance, city-size." << endl;
		cin.ignore();
		exit(0);
	}

	// Datos inciales del enunciado
	int param_people         = strtol(argv[1], NULL, 10);
	int param_infect_chance  = strtol(argv[2], NULL, 10);
	int param_recover_chance = strtol(argv[3], NULL, 10);
	int param_disease_time   = strtol(argv[4], NULL, 10);
	int param_infect_start   = strtol(argv[5], NULL, 10);
	int param_city_size      = strtol(argv[6], NULL, 10);

	// Notificacion de parámetros iniciales
	if (process_ID == 0) {
		cout << "[MPI] EL ambiente MPI ha iniciado correctamente." << endl;
		cout << "[MPI] Numero de procesos paralelos: " << process_num << endl;
		
		//Imprima los parametros inciales para el usuario
		cout << "[INF] Numero de personas en la civilizacion (entero): " << param_people << endl;
		cout << "[INF] Probabilidad de infeccion (entero de 0 a 100): " << param_infect_chance << "%" << endl;
		cout << "[INF] Probabilidad de recuperacion (entero de 0 a 100): " << param_recover_chance << "%" << endl;
		cout << "[INF] Duracion de la enfermedad en semanas (entero de 0 a 50): " << param_disease_time << endl;
		cout << "[INF] Porcentaje de infectados iniciales (entero de 0 a 100): " << param_infect_start << "%" << endl;
		cout << "[INF] Dimension de la ciudad (entero): " << param_city_size << "x" << param_city_size << endl;

		// Imprima que ya va realizar la simulacion
		cout << "[INF] Calculando, por favor espere..." << endl;
	}

	// Cambie la semilla para la lista random
	srand(time(NULL));
	
	// Defina la memoria que todos los hilos tienen
	int** people_healthy = creeMatrixDinamica(param_city_size);
	int** people_infected = creeMatrixDinamica(param_city_size);
	int** people_recovered = creeMatrixDinamica(param_city_size);
	int global_stats_healthy;
	int global_stats_infected;
	int global_stats_recovered;
	int global_stats_death;
	
	if(process_ID == 0){
		global_stats_infected = (param_infect_start/100)
		global_stats_healthy = HGJJHK
	}

	// Cronometro sincronizado inicial
	MPI_Barrier(MPI_COMM_WORLD);
	double local_start = MPI_Wtime();
	double local_elapsed = MPI_Wtime() - local_start;

	// Ejecute la simulacion a partir de este punto


	

	// Cronometro sincronizado final
	double total_elapsed;
	MPI_Reduce(&local_elapsed, &total_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	// Termina los procesos MPI
	if (process_ID == 0) {
		cout << "[INF] Tiempo transcurrido: " << total_elapsed << endl;
		cout << "[MPI] EL ambiente MPI ha terminado correctamente." << endl;
		cin.ignore();
	}
	
	//Destruya la memoria consumida
	destruirMatrixDinamica(people_healthy,param_city_size);
	destruirMatrixDinamica(people_healthy,param_city_size);
	destruirMatrixDinamica(people_healthy,param_city_size);
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
