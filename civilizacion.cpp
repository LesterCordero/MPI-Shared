#include <mpi.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

// Constantes de estados globales
#define healthy 0
#define infected 1
#define recovered 2
#define death 3

// Escribe la estructura de personas de forma rapida
void writeStruct(int* m, int i, int x, int y, int s, int t) {
	m[(4*i)+0] = x; // Coord X
	m[(4*i)+1] = y; // Coord Y
	m[(4*i)+2] = s; // Estado
	m[(4*i)+3] = t; // Tiempo enfermo
}

// Convierte rapidamente una estructura de personas, en una estructura vectorial
void structToVectorialSpace(int* m, int msize, int** d1, int** d2) {
	int x; // Coordenada x
	int y; // Coordenada y
	int s; // Estado actual
	for (int i = 0; i < msize; i++) {
		x = m[(4*i)+0];
		y = m[(4*i)+1];
		s = m[(4*i)+2];
		// Escribe en la matriz destino correspondiente al estado de salud de la persona
		if (s == healthy) {
			d1[x][y]++;
		}else {
			d2[x][y]++;
		}
	}
}

void printVectorialSpace(int** local_room_healthy, int param_room_size) {
	for (int y = 0; y < param_room_size; y++) {
	
	}
}

// Numero random entre min y max(inclusivo)
int getRandom(int min, int max) {
	return min + rand() % ((max + 1) - min);
}

// Genera una matriz dinamica de 1 dimensión
int* new1DArray(int size) {
	int* array1d = new int[size]();
	return array1d;
}

// Destruye una matriz dinamica de 1 dimensión
void delete1DArray(int* m) {
	delete[] m;
}

// Genera una matriz dinamica de 2 dimensiones
int** new2DArray(int size) {
	int **array2d = new int*[size];
	for (int i = 0; i < size; ++i) {
		array2d[i] = new int[size]();
	}
	return array2d;
}

// Destruye una matriz dinamica de 2 dimensiones
void delete2DArray(int** m, int size) {
	for (int i = 0; i < size; ++i) {
		delete[] m[i];
	}
	delete[] m;
}

// Método de inicio del programa
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
	int param_people = strtol(argv[1], NULL, 10);
	int param_infect_chance = strtol(argv[2], NULL, 10);
	int param_recover_chance = strtol(argv[3], NULL, 10);
	int param_disease_time = strtol(argv[4], NULL, 10);
	int param_infect_start = strtol(argv[5], NULL, 10);
	int param_room_size = strtol(argv[6], NULL, 10);

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
		cout << "[INF] Dimension de la ciudad (entero): " << param_room_size << "x" << param_room_size << endl;

		// Imprima que ya va realizar la simulacion
		cout << "[INF] Calculando, por favor espere..." << endl;
	}

	// Cambie la semilla para la lista random
	srand(time(NULL));

	// Solo el proceso maestro maneja los contadores globales
	int global_stats_healthy;
	int global_stats_infected;
	int global_stats_recovered;
	int global_stats_death;

	// Promedois del proceso maestro que cuenta los datos conforme van cambiando
	int global_prom_healthy = 0;
	int global_prom_infected = 0;
	int global_prom_recovered = 0;
	int global_prom_death = 0;

	// Los procesos esclavos, al realizar sus partes, contienen contadores locales que indican los cambios 
	int local_stats_healthy = 0;
	int local_stats_infected = 0;
	int local_stats_recovered = 0;
	int local_stats_death = 0;
	
	// Cada persona tiene 4 parámetros (x,y,estado,tiempoEnfermo) y todo los procesos lo tienen
	int* global_people = new1DArray(param_people*4);
	int* local_people = new1DArray((param_people*4)/process_num);

	// Todos los procesos en algún momento recrearan el espacio vectorial por lo que reservaremos la memoria
	int** local_room_healthy = new2DArray(param_room_size);
	int** local_room_infected = new2DArray(param_room_size);

	// Ahora el hilo maestro va gererar la matriz random de datos de personas y sus valores de contadores
	if (process_ID == 0) {
		global_stats_infected = (param_infect_start / 100) * param_people;
		global_stats_healthy = param_people - global_stats_infected;
		global_stats_recovered = 0;
		global_stats_death = 0;
		for (int i = 0; i < param_people; i++) {
			if (i < global_stats_infected) {
				writeStruct(global_people, i, getRandom(1, param_room_size - 1), getRandom(1, param_room_size - 1), infected, 0);
			}else {
				writeStruct(global_people, i, getRandom(1, param_room_size - 1), getRandom(1, param_room_size - 1), healthy, 0);
			}
		}
		
	}

	// Cronometro sincronizado inicial
	MPI_Barrier(MPI_COMM_WORLD);
	double local_start = MPI_Wtime();
	double local_elapsed = MPI_Wtime() - local_start;

	// Para debugeo, reconstruiremos la estructura de personas en un equivalente de espacio vectorial 
	structToVectorialSpace(global_people, param_people, local_room_healthy, local_room_infected);
	printVectorialSpace(local_room_healthy, param_room_size);

	// Distribuyalo a todos los hilos

	// N veces
	// Cada hilo reconstruye el espacio vectorial recorriendo una sola vez el vector
	// Cada hilo comprueba si estan enfermos o no, o cambia su tiempo o los mata, pero solo para una parte del vector, guardan sus contadores RELATIVOS (+6 healthy, +2 death, etc)
	// Una vez cambiados, olvidese del espacio vectorial y comuniquen sus subpartes con AllGather a todos el completo
	// Cada subhilo, le envia sus contadores relativos solo al 0, cuando el 0 lo recibe actualiza sus contadores e imprime el estado actual y actualiza sus promedios
	// SI el num de muertos es 0, termine, sino repita
	// Fin de la simulacion

	// Cronometro sincronizado final
	double total_elapsed;
	MPI_Reduce(&local_elapsed, &total_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	// Termina los procesos MPI
	if (process_ID == 0) {
		cout << "[INF] Tiempo transcurrido: " << total_elapsed << endl;
		cout << "[MPI] EL ambiente MPI ha terminado correctamente." << endl;
		cin.ignore();
	}

	// Destruyra toda la memoria al finalizar el programa
	delete1DArray(global_people);
	delete1DArray(local_people);
	delete2DArray(local_room_healthy, param_room_size);
	delete2DArray(local_room_infected, param_room_size);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();

	return 0;
}
