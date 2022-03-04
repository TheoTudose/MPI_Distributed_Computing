#include<mpi.h>
#include<stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#define CONVERGENCE_COEF 100

/**
 * Run: mpirun -np 12 ./a.out
**/

int minim(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

int read_cluster(int id, int **workers) {
    int num_worker;
	FILE *fp;
    char file_name[15];
    sprintf(file_name, "cluster%d.txt", id);
    fp = fopen(file_name, "r");
	fscanf(fp, "%d", &num_worker);
	*workers = (int*) malloc(sizeof(int) * num_worker);

	for (size_t i = 0; i < num_worker; i++) {
		fscanf(fp, "%d", *workers+i);
	}
	fclose(fp);
	return num_worker;
}



void leader_send(int rank, int *workers, int num_worker) {
	for (int i = 0; i < num_worker; i++) {
		MPI_Send(&rank, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << workers[i] << ")\n";
	}
}

void print_topo(std::vector<int>topology, int nProcesses, int rank) {
	int workers0 = 0, workers1 = 0, workers2 = 0, k = 0;
	for (int i = 0; i < nProcesses; i++)
	{
		if (topology[i] == 0) {
			workers0++;
		} else if (topology[i] == 1) {
			workers1++;
		} else if (topology[i] == 2) {
			workers2++;
		}
			
	}
	std::cout << rank << " -> 0:";
		for (int i = 0; i < nProcesses; i++)
		{
			if (topology[i] == 0) {
				k++;
				std::cout << i;
				if (k != workers0) {
				std::cout << ",";
				} else {
				std::cout << " ";
				}
			}	
		}

	std::cout << "1:";
	k = 0;
	for (int i = 0; i < nProcesses; i++)
	{
			if (topology[i] == 1) {
			k++;
			std::cout << i;
			if (k != workers1) {
			std::cout << ",";
			} else {
			std::cout << " ";
			}
		}	
	}

	std::cout << "2:";
	k = 0;
	for (int i = 0; i < nProcesses; i++)
	{
			if (topology[i] == 2) {
			k++;
			std::cout << i;
			if (k != workers2) {
			std::cout << ",";
			} else {
			std::cout << "\n";
			}
		}	
	}
}


int main(int argc, char * argv[]) {
	int rank, nProcesses, leader = -1, N;
	int num_workers1 = 0, num_workers2 = 0;
	int *workers = NULL;
	int num_workers;
	int workload0,workload1, workload2;
	int gap;
	int error;
	bool topo_comp = true;
	
	std::vector<int>topology;
	std::vector<int>V;
	std::vector<int>V_recv1;
	std::vector<int>V_recv2; 
	std::vector<int>workerVect;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	topology.resize(nProcesses, -2);
	error = atoi(argv[2]);
	N = atoi(argv[1]);
	V.resize(N);
	MPI_Status status;
	// MPI_Request request;
	//printf("ALTCEVA\n");

	if (rank == 0 || rank == 1 || rank == 2) { 
	num_workers = read_cluster(rank, &workers);
	leader_send(rank, workers, num_workers);
	} else {
	 	MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	}

	if(rank == 0) {
		topology[0] = -1;
		for (int i = 0; i < num_workers; i++)
		{
			topology[workers[i]] = 0;
		}
		for (int i = 0; i < nProcesses; i++)
		{
			if (topology[i] == -2) 
			{
				topo_comp = false;
			}
		}

		if (!topo_comp)
		{
			MPI_Send(&topology[0], nProcesses, MPI_INT, 2, 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << 2 << ")\n";
		}
		
		MPI_Recv(&topology[0], nProcesses, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

		print_topo(topology, nProcesses, rank);
		
		for (int i = 0; i < num_workers; i++)
		{
			MPI_Send(&topology[0], nProcesses, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";
		}
		
		
				
	}

	else if(rank == 1) {
		MPI_Recv(&topology[0], nProcesses, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

		topology[1] = -1;
		for (int i = 0; i < num_workers; i++)
		{
			topology[workers[i]] = 1;
		}

		print_topo(topology, nProcesses, rank);

		MPI_Send(&topology[0], nProcesses, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";
		for (int i = 0; i < num_workers; i++)
			{
				MPI_Send(&topology[0], nProcesses, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
				std::cout << "M(" << rank << "," << workers[i] << ")\n";
			}
		
		
	}

	else if(rank == 2) {
		MPI_Recv(&topology[0], nProcesses, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		topology[2] = -1;
		for (int i = 0; i < num_workers; i++)
		{
			topology[workers[i]] = 2;
		}
		for (int i = 0; i < nProcesses; i++)
		{
			if (topology[i] == -2) 
			{
				topo_comp = false;
			}
		}

		if (!topo_comp)
		{
			MPI_Send(&topology[0], nProcesses, MPI_INT, 1, 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << 1 << ")\n";
		} 
		
		MPI_Recv(&topology[0], nProcesses, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);

		print_topo(topology, nProcesses, rank);
		
		MPI_Send(&topology[0], nProcesses, MPI_INT, 0, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 0 << ")\n";
		
		for (int i = 0; i < num_workers; i++)
		{
			MPI_Send(&topology[0], nProcesses, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";
		}
			
		
		
	} else {
		MPI_Recv(&topology[0], nProcesses, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		print_topo(topology, nProcesses, rank);
		
	}

	if (rank == 0)
	{
		for (int i = 0; i < N; i++)
		{
			V[i] = i;
		}

		for (int i = 0; i < nProcesses; i++)
		{
			if (topology[i] == 1) {
				num_workers1++;
			} else if (topology[i] == 2) {
				num_workers2++;
			}
		}

		workload0 = (N / nProcesses) * num_workers;
		workload2 = (N / nProcesses) * num_workers2;
		workload1 = N - workload0 - workload2;

		for (int i = 0; i < num_workers; i++)
		{
			int start = i * ((double)workload0 / num_workers);
			int end = minim((i + 1) * (double)workload0 / num_workers, workload0);
			gap = end - start;
			MPI_Send(&gap, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";
			MPI_Send(&V[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";

			MPI_Recv(&V[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD, &status);
		}

		MPI_Send(&workload2, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";
		MPI_Send(&V[workload0], workload2, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";

		MPI_Send(&workload1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";
		MPI_Send(&V[workload0 + workload2], workload1, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";

		MPI_Recv(&V[workload0 + workload2], workload1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

		MPI_Recv(&V[workload0], workload2, MPI_INT, 2, 1, MPI_COMM_WORLD, &status);

		std::cout << "Rezultat:";
		for (int i = 0; i < N; i++)
		{
			std::cout<< " ";
			std::cout<< V[i];

		}
		std::cout << "\n";
		


	} else if (rank == 2) {

		MPI_Recv(&workload2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		V_recv2.resize(workload2);
		MPI_Recv(&V_recv2[0], workload2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		for (int i = 0; i < num_workers; i++)
		{
			int start = i * ((double)workload2 / num_workers);
			int end = minim((i + 1) * (double)workload2 / num_workers, workload2);
			gap = end - start;
			MPI_Send(&gap, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";
			MPI_Send(&V_recv2[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";

			MPI_Recv(&V_recv2[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD, &status);

		}

		MPI_Recv(&workload1, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		V_recv1.resize(workload1);
		MPI_Recv(&V_recv1[0], workload1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		MPI_Send(&workload1, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 1 << ")\n";
		MPI_Send(&V_recv1[0], workload1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 1 << ")\n";

		MPI_Recv(&V_recv1[0], workload1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
		
		MPI_Send(&V_recv1[0], workload1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 0 << ")\n";
		MPI_Send(&V_recv2[0], workload2, MPI_INT, 0, 1, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 0 << ")\n";
		

	} else if (rank == 1) {

		MPI_Recv(&workload1, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
		V_recv1.resize(workload1);
		MPI_Recv(&V_recv1[0], workload1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);

		for (int i = 0; i < num_workers; i++)
		{
			int start = i * ((double)workload1 / num_workers);
			int end = minim((i + 1) * (double)workload1 / num_workers, workload1);
			gap = end - start;
			MPI_Send(&gap, 1, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";
			MPI_Send(&V_recv1[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD);
			std::cout << "M(" << rank << "," << workers[i] << ")\n";

			MPI_Recv(&V_recv1[start], gap, MPI_INT, workers[i], 0, MPI_COMM_WORLD, &status);
		}
		MPI_Send(&V_recv1[0], workload1, MPI_INT, 2, 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << 2 << ")\n";
		
	} else {
		MPI_Recv(&gap, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		workerVect.resize(gap);
		MPI_Recv(&workerVect[0], gap, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

		for (int i = 0; i < gap; i++)
		{
			workerVect[i] = 2 * workerVect[i];
		}

		MPI_Send(&workerVect[0], gap, MPI_INT, topology[rank], 0, MPI_COMM_WORLD);
		std::cout << "M(" << rank << "," << topology[rank] << ")\n";
		
	}
	


	// MPI_Barrier(MPI_COMM_WORLD);

	//printf("Leader: %d\n", leader);
	
	MPI_Finalize();
	return 0;
}
