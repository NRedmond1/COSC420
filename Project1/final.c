#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <crypt.h>
#include <mpi.h>

void freedata(char **t1, char **t2, char **t3, char *t4, char *t5){
	int i;
	for (i=0; i<11; i++){
		free(t1[i]);
		free(t2[i]);
		free(t3[i]);
	}
	free(t1);
	free(t2);
	free(t3);
	free(t4);
	free(t5);
}

void splitupshadow(FILE *fd, char **username, char **salt_string, char **encrypted_password, char *algorithm_id){
	char *buffer = (char*)malloc(69*sizeof(char));
	size_t len = 0;
	ssize_t read;
	int tmp = 0, number_user = 0, placehold = 0, i;

	while ((read = getline(&buffer, &len, fd)) != -1) {
		tmp = 0;
		while( buffer[tmp] != ':' ){
			username[number_user][tmp] = buffer[tmp];
			tmp++;
		}
		username[number_user][tmp] = '\0';
		tmp += 2;
		algorithm_id[number_user] = buffer[tmp];
		tmp += 1;
		placehold = tmp;
		salt_string[number_user][tmp-placehold] = buffer[tmp];
		tmp++;
		while( buffer[tmp] != '$' ){
			salt_string[number_user][tmp-placehold] = buffer[tmp];
			tmp++;
		}
		salt_string[number_user][tmp-placehold] = '\0';
		placehold = tmp;

		for( i = tmp; i < read; i++ ){
			encrypted_password[number_user][i-placehold] = buffer[i];
		}
		encrypted_password[number_user][read-placehold] = '\0';

		number_user++;
	}
	free(buffer);
}

int main(){
	MPI_Init(NULL,NULL);
	MPI_Comm world = MPI_COMM_WORLD;
	int me, nprocs, i;
	MPI_Comm_size(world, &nprocs);
	MPI_Comm_rank(world,&me);

	int k=0;
	int j;
	int blocksize = 0, charcount;
	int count_length = 124;

	char **username;
	username = (char**)malloc(sizeof(char*)*11);
	for(i=0; i < 11; i++)
		username[i] = (char*)malloc(sizeof(char)*20);

	char *algorithm_id = (char*)malloc(sizeof(char)*11);

	char **salt_string;
	salt_string = (char**)malloc(sizeof(char*)*11);
	for(i=0; i<11; i++)
		salt_string[i] = (char*)malloc(sizeof(char)*5);

	char **encrypted_password;
	encrypted_password = (char**)malloc(sizeof(char*)*11);
	for(i=0; i<11; i++)
		encrypted_password[i] = (char*)malloc(sizeof(char)*59);

	if(me==0){
		FILE *fd;
		if( (fd = fopen("shadow2", "r")) == NULL ){
			printf("ERROR: cant open shadow.");
			exit(-1);
		}
		splitupshadow(fd, username, salt_string, encrypted_password, algorithm_id);
		fclose(fd);

	}

	//Broadcast all things to nodes
	for(i=0;i<11;i++){
		MPI_Bcast(username[i],20,MPI_CHAR,0,world);
		MPI_Bcast(salt_string[i],5,MPI_CHAR,0,world);
		MPI_Bcast(encrypted_password[i],49,MPI_CHAR,0,world);
		MPI_Bcast(&algorithm_id[i],1,MPI_CHAR,0,world);

	}

	FILE *filed;
	if( (filed = fopen("words", "r")) == NULL ){
		printf("ERROR: cant open shadow.");
		exit(-1);
	}

	fseek(filed,0,SEEK_END);
	charcount =  ftell(filed);
	blocksize = (charcount/nprocs);
	fclose(filed);

	int counter, charcounter;
	int start, end;
	int num_dig = 4;
	int pass = 1;

	char *tmpString = "$1$ab";
	char *passw = (char*)malloc(100*sizeof(char));
	char *result = (char*)malloc(100*sizeof(char));
	char *num = (char*)malloc(sizeof(char)*num_dig*100);
	char *suf = (char*)malloc(sizeof(char) *num_dig*100);
	char *temp = (char*)malloc(sizeof(char)*num_dig);
	char *buffer2 = (char*)malloc(100*sizeof(char));

	size_t len2 = 100;
	size_t read2 = 0;

	counter = 0;

	FILE* fdarray[nprocs];
	for(k=0; k < nprocs; k++)
	{
		fdarray[k] = fopen("words", "r");
		fseek(fdarray[k], 0, SEEK_SET);
	}

	k=me;
	fseek(fdarray[k], (k*blocksize), SEEK_SET);
	read2 = getline(&buffer2, &len2, fdarray[k]);
	charcounter = 0;

	pid_t pid, p1, p2;
  int status;

	while (charcounter < blocksize && read2 > 0){
		charcounter = charcounter + read2;
		buffer2[read2-1] = '\0';


		for(i=0; i<11; i++){
			memset(passw,0,strlen(passw));
			strcat(passw, tmpString);
			strcat(passw, encrypted_password[i]);
			passw[strlen(passw)-1] = '\0';
			pass = -1;

			result = crypt(buffer2, tmpString);

			if( (pass = strcmp( result, passw )) == 0 ){
				printf("rank: %d\n", me);
				printf("%s: %s   password: %s\n\n", username[i],"SUCCEEDED", buffer2);
				break;
			}
			else
				pass = -1;

		}

		// fork???-----------------------------------------------------------------------------

		if((pid = fork()) < 0){
			printf("ERROR: fork");
			exit(-1);
		}
		else if( pid == 0 ){
			p1 = getpid();
			for(counter = 1; counter < count_length; counter++){
				memset(num,0,strlen(num));

				sprintf(num, "%d", counter);
				strcat(num, buffer2);
				num[strlen(num)] = '\0';

				result = crypt(num, tmpString);
				//printf("C: %s\n", num);

				for(i=0; i<11; i++){
					memset(passw,0,strlen(passw));
					strcat(passw, tmpString);
					strcat(passw, encrypted_password[i]);
					passw[strlen(passw)-1] = '\0';
					pass = -1;

					if( (pass = strcmp( result, passw )) == 0 ){
						printf("rank: %d\n", me);
						printf("%s: %s   password: %s\n\n", username[i], "SUCCEEDED", &num[start]);
						break;
					}
					else{
						pass = -1;
					}

				}
			}
			_exit(0);
		}
		else{
			for(counter = 1; counter < count_length; counter++){
				memset(suf, 0, strlen(suf));

				strcat(suf, buffer2);
				sprintf(temp, "%d", counter);
				strcat(suf,temp);
				suf[strlen(suf)] = '\0';

				result = crypt(suf, tmpString);
				//printf("P: %s\n", suf);

				for(i=0; i<11; i++){
					memset(passw,0,strlen(passw));
					strcat(passw, tmpString);
					strcat(passw, encrypted_password[i]);
					passw[strlen(passw)-1] = '\0';
					pass = -1;

					if( (pass = strcmp( result, passw )) == 0 ){
						printf("rank: %d\n", me);
						printf("%s: %s   password: %s\n\n", username[i], "SUCCEEDED", suf);
						break;
					}
					else
						pass = -1;

				}
			}
			p2 = wait(&p1);
		}

		//printf("finished %d: %s\n", me, buffer2);
		read2 = getline(&buffer2, &len2, fdarray[k]);
	}

	freedata(username, salt_string, encrypted_password, algorithm_id, buffer2);
	free(temp);
	free(passw);
	free(result);
	free(suf);
	free(num);

	MPI_Finalize();

	exit(0);
}
