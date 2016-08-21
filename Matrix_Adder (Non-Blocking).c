#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>

FILE *fpA,*fpB,*fpO;

float **alloc_2d_int(int rows,int cols){
    int i ;
    float *data = (float *)malloc(rows*cols*sizeof(float));
    float **array = (float **)malloc(rows*sizeof(float*));
    for (i=0; i<rows; i++) {
        array[i] = &(data[cols*i]);
    }
    return array;
}
int main(int argc, char *argv[])
{
	int rank, nproc;

	MPI_Request request[10];
    MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);

	if(rank==0){

		int nrows,ncolumns;
		double starttime,endtime;
        
        starttime = MPI_Wtime();

		fpA = fopen("D:\\3rdYrs\\first term\\CPE374 Parallel\\MatrixAddDataSet\\matAmedium.txt","r");
		fscanf(fpA,"%d %d",&nrows,&ncolumns);
		fpB = fopen("D:\\3rdYrs\\first term\\CPE374 Parallel\\MatrixAddDataSet\\matBmedium.txt","r");
		fscanf(fpB,"%d %d",&nrows,&ncolumns);

		int n = (nrows/nproc);
		int i,j;

		float **matA = alloc_2d_int(nrows,ncolumns);
		float **matB = alloc_2d_int(nrows,ncolumns);
		float **matR = alloc_2d_int(nrows,ncolumns);

		for(i=0;i<nrows;i++){
			for(j=0;j<ncolumns;j++){
				fscanf(fpA,"%f ",&matA[i][j]);
			}
		}
		
		fclose(fpA);
		for(i=0;i<nrows;i++){
			for(j=0;j<ncolumns;j++){
				fscanf(fpB,"%f ",&matB[i][j]);
			}
		}
		
		fclose(fpB);
		
		for (i=1; i<nproc; i++) {
			MPI_Isend(&n, 1, MPI_INT, i, i, MPI_COMM_WORLD, &request[0]);
            MPI_Isend(&ncolumns, 1, MPI_INT, i, i, MPI_COMM_WORLD,&request[1]);
            MPI_Isend(&matA[(n*i)+(nrows%nproc)][0], n*ncolumns, MPI_FLOAT, i, 10, MPI_COMM_WORLD,&request[2]);
            MPI_Isend(&matB[(n*i)+(nrows%nproc)][0], n*ncolumns, MPI_FLOAT, i, 11, MPI_COMM_WORLD,&request[3]);
            
            MPI_Wait(&request[0], &status);
            MPI_Wait(&request[1], &status);
            MPI_Wait(&request[2], &status);
            MPI_Wait(&request[3], &status);
        }

		for (i=0; i<n+(nrows%nproc); i++) {
            for (j=0; j<ncolumns; j++) {
                matR[i][j] = matA[i][j]+matB[i][j];
            }
        }

		for (i=1; i<nproc; i++) {
			MPI_Irecv(&matR[(n*i)+(nrows%nproc)][0], n*ncolumns, MPI_FLOAT,i, 12, MPI_COMM_WORLD,&request[4]);
            MPI_Wait(&request[4], &status);
        }

		fpO = fopen("D:\\3rdYrs\\first term\\CPE374 Parallel\\MatrixAddDataSet\\matR.txt", "w");

        for (i=0; i<nrows; i++) {
            for (j=0; j<ncolumns; j++) {
                fprintf(fpO,"%.1f ",matR[i][j]);
            }
            fprintf(fpO,"\n");
        }

        fclose(fpO);

		endtime = MPI_Wtime();
		printf("Total : %lf sec\n",endtime-starttime);

	}else{

		int i,j,n,ncolumns;

		MPI_Irecv(&n, 1, MPI_INT, 0, rank, MPI_COMM_WORLD,&request[5]);
		MPI_Irecv(&ncolumns, 1, MPI_INT, 0, rank, MPI_COMM_WORLD,&request[6]);
        
        MPI_Wait(&request[5], &status);
        MPI_Wait(&request[6], &status);

		float **matAn = alloc_2d_int(n,ncolumns);
		float **matBn = alloc_2d_int(n,ncolumns);
		float **matRn = alloc_2d_int(n,ncolumns);

		MPI_Recv(&matAn[0][0],n*ncolumns, MPI_FLOAT, 0, 10, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&matBn[0][0],n*ncolumns, MPI_FLOAT, 0, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (i=0; i<n; i++) {
            for (j=0; j<ncolumns; j++) {
                matRn[i][j] = matAn[i][j] + matBn[i][j];
            }
        }
        
		MPI_Isend(&matRn[0][0], n*ncolumns, MPI_FLOAT, 0, 12, MPI_COMM_WORLD,&request[9]);
        MPI_Wait(&request[9], &status);
	}

	MPI_Finalize();
	return 0;
}