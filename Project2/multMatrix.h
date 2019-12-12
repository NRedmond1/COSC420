#ifndef MULTMATRIX_H
#define MULTMATRIX_H

#define INDEX(n,m,i,j) m*i + j
#define ACCESS(A,i,j) A->arr[INDEX(A->rows, A->cols, i ,j)]
#define ACCESS2(A,i,j) A.arr[INDEX(A.rows, A.cols, i ,j)]

struct floatmatrix{
	int rows, cols;
	float* arr;

};

void iniFloatMatrix(struct floatmatrix* A, int r, int c){
	A->rows = r;
	A->cols = c;
	A->arr = malloc(r*c*sizeof(float));
	int i = 0;
	int j = 0;
	for( i = 0; i < r; i++){
		for(j = 0; j < c; j++){
			ACCESS(A,i,j) = 0;
			//ACCESS(A,i,j) = (float)rand()/(float)(RAND_MAX);
		  //printf("Index: %d\n", c*i+j);
		}
	}
}

void printFloatMatrix(struct floatmatrix* A){
	int i = 0;
	int j = 0;
	for( i = 0; i < A->rows; i++){
		for( j = 0; j < A->cols; j++){
			printf("%.2f ", ACCESS(A,i,j));
		}
		puts("");
	}
}

void print_vect(float *x, int length){
	int i;
	printf("X VECTOR: \n");
	for(i=0;i<length;i++){
		printf("%.4f ", x[i]);
	}
	printf("\n");
}

void multiplication_return(struct floatmatrix A, float *x, int row1, int col1, int row2, int col2){
	int nprocs, me;
	srand(time(0));

	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &nprocs);
	MPI_Comm_rank(world,&me);

	struct floatmatrix B;
	struct floatmatrix C;

	int i,j;

	if(col1 != row2){ // Check for if the dimensions do not work for multipication
		if(me==0){
			printf("CANNOT MULTIPLY: Row length of Matrix 1 does not equal Column length of Matrix 2\n");

		}
		return;
	}

	int num_it; // number of operations each node must do.

	if(nprocs <= 1){ // Check to make sure there are enough processors to do multipication

		if(me==0)
			printf("CANNOT MULTIPLY: requires worldsize of at least 2\n");

		return ;
	}

	iniFloatMatrix(&C,row1,col2);
	if(me==0){
		//iniMatrix(&A,row1,col1);
		iniFloatMatrix(&B,row2,col2);
		for( i = 0; i < row2; i++){
			for(j = 0; j < col2; j++){
				ACCESS2(B,i,j) = x[i];

				//printf("Index: %d\n", c*i+j);
			}
		}

		/*
		   printf("Multiplying first matrix by the second matrix\n");


		   printFloatMatrix(&A);
		   printf("\n");
		   printFloatMatrix(&B);
		   printf("\n");
		   */
		float* col_arr;
		float prod;
		col_arr=malloc(row2*sizeof(float));
		int col_count = 0;
		int row_count = 0;
		int access_row = 0;//used to properly put in the product into the correct spot of the new array
		int proc_it;
		int *it_arr; // if not even iterations for each node, we use this array to determine how many iterations each node gets
		it_arr = malloc(sizeof(int)*(nprocs-1));

		//This determines the number of iterations each node must do. In other words, the amount of rows and columns each node must multiply
		if((col2 * row1)%(nprocs-1)==0){ //if the work divides evenly..
			num_it = (col2 * row1)/(nprocs-1); // each node gets the same amount

			for(i=1;i<nprocs;i++){ // send this to each node
				MPI_Send(&num_it,1,MPI_INT,i,2,world);
			}
		} else { // if not even work between each node...
			for(i=1;i<nprocs;i++){ // initiliaze the array
				it_arr[i-1] = 0;
			}

			for(i=0;i<col2*row1; i++){ //sequentially increment each element in the array based on which processor is up
				proc_it = (i%(nprocs-1)); // current processor
				it_arr[proc_it]++;
			}

			for(i=1;i<nprocs;i++){ // send each nodes # of iterations to it
				MPI_Send(&it_arr[i-1],1,MPI_INT,i,2,world);
			}
		}

		MPI_Request req;
		for(i=0;i<col2*row1;i++){ // divides up work and sends it to each node. Sends a row and a column to a node
			if((i)%col2==0){ // this is put to get the first column again once a new row is being used
				col_count=0;
			}

			proc_it = (i%(nprocs-1))+1;//which processor the root will send the row and clumn too

			if(((i)%col2==0)&&(i!=0)){// used to get the proper row, only incrementing once the previous node has been evaluated with every column
				row_count+=col1;
				access_row++;
			}

			//NEW FOR MULTIPLICATION, uses non-blocking send in order to calcualte the column before being blocking
			MPI_Isend(&A.arr[row_count],col1, MPI_FLOAT,proc_it,0,world, &req);

			for(j=0;j<row2;j++){//populates array with the correct column from matrix 2.
				col_arr[j] = ACCESS2(B,j, col_count);

			}

			//WAITS to make sure other node has received the send call
			MPI_Wait(&req, MPI_STATUS_IGNORE);

			// Send proper row from 1st matrix and column from second matrix
			//MPI_Send(&A.arr[row_count],col1, MPI_INT,proc_it,0,world); // TRY PUTTING AS FIRST OPERATION IN LOOP W AN I_SEND, WAIT AFTER THE LOOP
			MPI_Send(col_arr,row2, MPI_FLOAT,proc_it,1,world);
			//printf("HERE\n");
			// receive product from proper child node
			MPI_Recv(&prod, 1, MPI_FLOAT,proc_it,1,world,MPI_STATUS_IGNORE);

			ACCESS2(C, access_row, col_count) = prod; // puts product into proper place in array
			col_count++;
		}
		/*
		   printf("MATRIX MULTIPLICATION PRODUCT IS:\n");
		   printFloatMatrix(&C);
		   printf("\n\n");*/

		free(col_arr);
		free(B.arr);
		free(it_arr);
	}
	else{ // CHILD NODES
		float* sub_mat1;
		float* sub_mat2;
		float product;
		sub_mat1 = malloc(col1*sizeof(float)); // row from matrix 1
		sub_mat2 = malloc(row2*sizeof(float)); // column from matrix 2

		int it; // iterations

		MPI_Recv(&it,1,MPI_INT,0,2,world,MPI_STATUS_IGNORE);//receives number of iteration it must do in for loop

		for(j=0; j<it;j++){
			product = 0;
			MPI_Recv(sub_mat1, col1, MPI_FLOAT, 0, 0, world, MPI_STATUS_IGNORE); // receive row from root
			MPI_Recv(sub_mat2,row2, MPI_FLOAT,0,1,world,MPI_STATUS_IGNORE); //receive column from root

			for(i=0;i<col1;i++){ // calculate product
				product = product+(sub_mat1[i]*sub_mat2[i]);
			}

			MPI_Send(&product,1, MPI_FLOAT,0,1,world); // Send product back to root
		}

		free(sub_mat1);
		free(sub_mat2);
	}

	MPI_Bcast(C.arr,row1*col2, MPI_FLOAT, 0, world );

	for(i=0; i <C.rows*C.cols; i++){
		x[i] = C.arr[i];
	}
	free(C.arr);
}

void normalize(float* x, int size){
	int i;
	float sum;
	for(i=0; i < size; i++){
		sum += x[i] * x[i];
	}
	sum = 1/(sqrt(sum));
	for(i=0; i < size; i++){
		x[i] = sum * x[i];
	}
}



int eigenvector(struct floatmatrix A, float *x){
	int nprocs, me;
	srand(time(0));

	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &nprocs);
	MPI_Comm_rank(world,&me);
	
	int row1,col1,i;
	float eigenvalue = 0;
	row1 = A.rows;
	col1 = A.cols;
	float *x2= malloc(sizeof(float)*row1);


	for(i=0;i<row1;i++){ // Initialize x vector
		x[i] = 1;
		x2[i] = 1;
	}

MPI_Barrier(world);
	for(i=0;i<row1*col1;i+=row1){
		MPI_Bcast(&A.arr[i],row1, MPI_FLOAT, 0, world );
	}

	//for(i=0; i<1; i++){
	int bool = 0;
	int counter = 0 ;
	while(bool == 0){
		for(i = 0; i < row1; i++){
			x2[i] = x[i];
		}

		multiplication_return(A,x,row1,col1,row1,1);
		normalize(x, row1);

		for(i=0;i<row1;i++){ // Initialize x vector
			if( abs(x[i] - x2[i]) > .0001){
				bool = 0;
			}
			else {
				counter++;
			}

		}
		if(counter == row1)
			bool = 1;
		else
			counter = 0 ;
	}
	float sum1=0, sum2=0;
	eigenvalue=0;
	for(i = 0; i < row1; i++){
		x2[i] = x[i];
		sum2 += (float)x2[i]*x2[i];
	}
	sum2 = sqrt(sum2);
	multiplication_return(A,x,row1,col1,row1,1);

	for(i = 0; i < row1; i++){
		sum1 += (float)x[i]*x[i];
	}
	sum1 = sqrt(sum1);
	eigenvalue = (float)sum1/sum2;


	if (me==0){
		//printFloatMatrix(&A);
		//printf("\n");

		//print_vect(x,row1);
		//printf("done.\n");
		//printf("Eigenvalue: %.4f\n", eigenvalue);
	}

	free(A.arr);
	free(x2);
	
	return 0;
}

#endif
