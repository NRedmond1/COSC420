#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include "multMatrix.h"

#define SIZE 256
#define DOCU_SIZE 256
#define FILENAME "arxiv-metadata.txt"
#define FILENAME2 "arxiv-citations.txt"

//https://www.zentut.com/c-tutorial/c-binary-search-tree/

//--------------------------------------------------------------------------
//ALL THIS IS FOR THE LINKED LISTS INSIDE EACH NODE IN THE BINDARY TREE.
//DO NOT CALL THESE FUNCTIONS
typedef struct link_node{
	char document_name[DOCU_SIZE];
	int number;
	struct link_node* next;
} link_node;


void delete_list(link_node* head){
	if(head!=NULL){
		delete_list(head->next);
		free(head);
	}
}

link_node* create_link(char docu_name[DOCU_SIZE]){
	int i;
	link_node *new_node = (link_node*) malloc(sizeof(link_node));
	for(i=0; i < strlen(docu_name); i++){
		new_node->document_name[i] = docu_name[i];
	}
	new_node->document_name[strlen(docu_name)] = '\0';
	new_node->number = 0;
	new_node->next = NULL;
	return new_node;
}

link_node* insert_link(link_node* head,char docu_name[DOCU_SIZE]){
	if(head == NULL){
		head = create_link(docu_name);
		head->number = 1;
	}
	else{
		link_node *cursor = head;
		link_node *prev = NULL;
		while(cursor!=NULL){
			if(strcmp(cursor->document_name, docu_name) == 0){
				return head;
			}
			prev = cursor;
			cursor = cursor->next;
		}
		prev->next = create_link(docu_name);
		prev->next->number = (prev->number + 1);
	}
	return head;
}


void print_list(link_node* head, FILE* index){
	link_node *cursor = head;

	if(head == NULL){
		printf("\n");
		return;
	}
	while(cursor->next!=NULL){
		//fprintf(index, "%s, ", cursor->document_name);
		printf("%s, ", cursor->document_name);
		cursor = cursor->next;
	}

	//fprintf(index,"%s. ", cursor->document_name);
	printf("%s. ", cursor->document_name);
	//fprintf(index,"\n");
	printf("\n");
}
// END OF LINKED LIST FUNCTIONS
//-----------------------------------------------------------------------

typedef struct index_word{
	//char word[SIZE];
	link_node *document_list;
} index_word;



//-----------------------------------------------------------------------
//FOLLOWING IS FUNCTIONS FOR THE BINARY TREE
//ONLY CALL FUNCTIONS LABELED  CALLABLE

typedef struct node {
	char word[SIZE];
	link_node *document_list;
	//index_word* ind_word;
	struct node* left;
	struct node* right;
} node;


node* create_node(char word[SIZE]){ // CREATE A NODE
	int i;
	node *new_node = (node*)malloc(sizeof(node));
	for(i=0; i < strlen(word); i++){
		new_node->word[i] = word[i];
	}
	new_node->word[strlen(word)] = '\0';
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->document_list = NULL;
	return new_node;
}

// CREATE A NODE AND INSERTS INTO TREE, CALLABLE
node* insert_node(node *root, char word[SIZE]){
	if(root==NULL)
	{
		root = create_node(word);
	}
	else{
		int left = 0;
		int compare = 0;

		node* cursor = root;
		node* prev = NULL;

		while(cursor != NULL){
			compare = strcmp(cursor->word,word);
			prev = cursor;
			if(compare > 0){
				left = 1;
				cursor = cursor->left ;
			}else if(compare < 0){
				left = 0   ;
				cursor = cursor->right;
			} else{
				return root;
			}
		}
		if(left == 1)
			prev->left = create_node(word);
		else
			prev->right = create_node(word);
	}
	return root;
}

//DISPLAYS THE TREE AND EACH NODES WORD AND DOCUMENT LIST
//CALLABLE
void display_tree(node* cursor, FILE* index){
	if(cursor == NULL)
		return;

	//fprintf(index,"%s\n",cursor->word);
	printf("%s\n",cursor->word);
	/*
	   if(cursor->left != NULL)
	   printf(" (L:%s)",cursor->left->word);
	   if(cursor->right != NULL)
	   printf(" (R:%s)",cursor->right->word);
	   */

	print_list(cursor->document_list, index);
	//fprintf(index,"\n");

	display_tree(cursor->left, index);
	display_tree(cursor->right, index);
}


//SEARCHES TREE FOR A CERTAIN WORD AND RETURNS THAT NODE
node* search_tree(node* root, char word[SIZE]){

	if(root == NULL)
		return NULL;

	int r;
	node* cursor = root;
	while(cursor != NULL){

		r = strcmp(cursor->word,word);

		if( r > 0 )
			cursor = cursor->left;
		else if( r < 0 )
			cursor = cursor->right;
		else
			return cursor;
	}
	return cursor;
}

//SAME AS SEARCH TREE BUT ONLY PRINTS OUT IF THE WORD IS IN THE TREE OR NOT
//CALLABLE
int check_tree(node* root, char word[SIZE]){
	node *s = search_tree(root, word);
	if(s!=NULL)
		return 1;
	else
		return 0;

}

//DELETES ALL NODES
//CALLABLE AT END OF PROGRAM
void delete_all(node *cursor){
	if(cursor == NULL){
		return;
	}
	delete_all(cursor->left);
	delete_all(cursor->right);

	delete_list(cursor->document_list);
	free(cursor);
}

//INSERTS A DOCUMENTS INTO A NODES DOCUMENT LIST, FINDS NODE USING WORD PARAMETER
//CALLABLE
void insert_document(node* root, char word[SIZE], char document[DOCU_SIZE]){
	node *s = search_tree(root, word);
	if(s==NULL){
		//printf("NODE NOT FOUND\n");
		return;
	}
	s->document_list = insert_link(s->document_list, document);

}

//PRINTS A NODES DOCUMENT LIST
void print_document_list(node* root, char word[SIZE]){
	node *s = search_tree(root, word);
	if(s==NULL){
		//printf("NODE NOT FOUND\n");
		return;
	}
	//print_list(s->document_list);

}
int main(){

	//Initialize MPI
	int nprocs, me;
	MPI_Init(NULL,NULL);
	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &nprocs);
	MPI_Comm_rank(world,&me);

	//Initialize matrix for the adjacency matrix
	struct floatmatrix A;
	int sizeDocList;
	int bool = 0;
	char** listArray;
	int bool_inFile;
	node *refs = NULL;
	node *searching = NULL;

	if(me==0){
		printf("loading data...\n");

		//opens backward index to read words and their referenced page ids
		FILE* back_ind;
		//back_ind = fopen("backward_index.txt","r");
		back_ind = fopen("backward_index2.txt","r");


		char *line_buf = NULL;
		char *doc_buf = NULL;
		size_t line_buf_size = 0;
		int i, j, x, numABS;
		ssize_t line_size;
		char *PageNumber = NULL;

		//****************************************************************************************

		//OPENS up citations file, creates binary tree of each page id and what it references
		FILE *citations = fopen(FILENAME2, "r");
		if (!citations)
		{
			fprintf(stderr, "Error opening file '%s'\n", FILENAME2);
			return EXIT_FAILURE;
		}
		if(line_buf){
			free(line_buf); line_buf=NULL;
		}
		//first getline to get into while loop.
		line_size = getline(&line_buf, &line_buf_size, citations);
		line_buf[strlen(line_buf)-1] = '\0';
		// Loop through until we are done with the file.
		while (line_size >= 0)
			//for(i=0; i<40; i++)
		{
			if(strcmp(line_buf, "+++++") == 0){
				if(PageNumber){
					free(PageNumber); PageNumber=NULL;
				}
				// After the +++++ is the Page ID so we get that next
				line_size = getline(&PageNumber, &line_buf_size, citations);
				PageNumber[strlen(PageNumber)-1] = '\0';

				//INSERT into binary tree
				refs = insert_node(refs, PageNumber);

				if(line_buf){
					free(line_buf); line_buf=NULL;
				}
				line_size = getline(&line_buf, &line_buf_size, citations);
				line_buf[strlen(line_buf)-1] = '\0';

				//parse through all referenced page ids until end
				while(line_size >= 0){
					if(line_buf){
						free(line_buf); line_buf=NULL;
					}
					line_size = getline(&line_buf, &line_buf_size, citations);
					line_buf[strlen(line_buf)-1] = '\0';
					if(strcmp(line_buf, "+++++") == 0){
						break;
					}
					else{
						//insert referenced page into linked list
						insert_document(refs, PageNumber, line_buf);
					}
				}
			}
		}

		printf("finished!\n\n");


		/*
		 *************************************************************************************************
		 */

		//GETTING SEARCHED WORD OR SENTECE AND FINDING TOP RANKED PAGES ASSOCIATED WITH IT.

		char *input = (char*)malloc(sizeof(char)*100);
		char *Page = (char*)malloc(sizeof(char)*SIZE);
		char **text;
		text = (char**)malloc(sizeof(char*)*50);
		for(i=0; i<50; i++){
			text[i] = (char*)malloc(sizeof(char)*50);
		}
		int t;

		//Getting sentence from user and converting to lowercase
		printf("Enter text to search: \n");
		fgets(input, 100, stdin);
		for(i=0; i<strlen(input); i++){
			input[i] = tolower(input[i]);
		}

		int z=0;

		//Takes buffer of input sentence and coverts to array of each word.
		for(i=0; i<strlen(input); i++){
			memset(text[z], 0, sizeof(char)*50);
			t=0;
			while( (input[i] != ' ') && (input[i] != '\n') && (input[i] != '\r') ){
				text[z][t] = input[i];
				t++;
				i++;
			}
			text[z][strlen(text[z])] = '\0';

			//If the word is smaller than 3 characters dont include.
			//it will be very common with to much references.
			if(strlen(text[z]) > 3)
				searching = insert_node(searching, text[z]);
			z++;
		}

		if(input)
			free(input);

		node *select;
		int s;

		if(line_buf){
			free(line_buf); line_buf=NULL;
		}
		line_size = getline(&line_buf, &line_buf_size, back_ind);
		line_buf[strlen(line_buf)-1] = '\0';
		// Loop through until we are done with the file.
		while (line_size != -1){
			//Checking the getline from backward index to each input word for a match.
			for(i=0; i<z; i++){
				if(strcmp(line_buf, text[i]) == 0 && check_tree(searching,text[i]) == 1){
					bool_inFile = 1;
					if(doc_buf){
						free(doc_buf); doc_buf=NULL;
					}
					//Getting all page ids the matched word referenced to.
					line_size = getline(&doc_buf, &line_buf_size, back_ind);
					if(line_size>0)
						doc_buf[strlen(doc_buf)-1] = '\0';

					for(x=0; x<strlen(doc_buf)-1; x++){
						s=0;
						memset(Page, 0, sizeof(char)*SIZE);
						//Parse through page ids and get each individually
						while( (doc_buf[x] != ' ') && (doc_buf[x] != '\n') && (doc_buf[x] != '\r') ){
							Page[s] = doc_buf[x];
							s++;
							x++;
						}
						if(strlen(Page) > 0){
							Page[strlen(Page)-1] = '\0';
							insert_document(searching, text[i], Page);

						}
					}

				}
			}
			//Continue in the file for possible other matches.
			if(line_buf){
				free(line_buf); line_buf=NULL;
			}
			line_size = getline(&line_buf, &line_buf_size, back_ind);
			if(line_size != -1){
				line_buf[strlen(line_buf)-1] = '\0';
			}
		}

		free(Page);

		//This is a temporary file pointer just to check everything is in the tree correctly
		//FILE* some;
		//display_tree(searching, some);

		//Checks if any word was in backward index.
		if(bool_inFile == 0){
			printf("Input not in backward index or word is too small\n");
		}
		else{
			sizeDocList=0;
			link_node *cursor;
			link_node *prev;

			//adds each words referenced page ids into one vector.
			//And keeps track of how many there should be.
			for(i=0; i<z; i++){
				if(check_tree(searching,text[i]) == 1){
					select = search_tree(searching, text[i]);
					if(select->document_list == NULL){
						continue;
					}
					else{
						cursor = select->document_list;
						prev = NULL;
						while(cursor!=NULL){
							prev = cursor;
							cursor = cursor->next;
						}
						sizeDocList += prev->number;
					}
				}
			}

			listArray = (char **)malloc(sizeof(char*)*sizeDocList);
			for(i=0; i<sizeDocList; i++){
				listArray[i] = (char *)malloc(sizeof(char)*SIZE);
			}

			x=0;
			for(i=0; i<z; i++){

				if(check_tree(searching,text[i]) == 1){
					select = search_tree(searching, text[i]);
					if(select->document_list == NULL){
						continue;
					}
					else{
						cursor = select->document_list;
						while(cursor != NULL){
							strcpy(listArray[x], cursor->document_name);
							listArray[x][strlen(listArray[x])] = '\0';
							x++;
							cursor = cursor->next;
						}
					}
				}
			}

			for(i=0; i<50; i++){
				free(text[i]);
			}
			free(text);

			printf("\n");

			iniFloatMatrix(&A, sizeDocList, sizeDocList);

			//FILLS IN ADJACENCY MATRIX HERE
			bool = 0;
			for(i=0; i < sizeDocList; i++){
				//There is a possible error of the page id is not in the citations file.
				//and we found this does occur multiple times depending on how big adjacency list is.
				if( (select = search_tree(refs, listArray[i])) == NULL ){
					//printf("ERROR: finding page id.\n");
					continue;
				}
				for(j=0; j < sizeDocList; j++){
					if(i == j){
						ACCESS2(A, i, j) = 0;
					}
					else{
						cursor = select->document_list;
						prev = NULL;
						// Checks to see if any of the page ids reference any of the other page ids.
						while(cursor!=NULL){
							if(strcmp(cursor->document_name, listArray[j]) == 0){
								ACCESS2(A, i, j) = 1;
								//mark bool as 1 if at least one page id references another.
								bool = 1;
								break;
							}
							prev = cursor;
							cursor = cursor->next;
							if(cursor == NULL){
								ACCESS2(A, i, j) = 0;
							}
						}
					}
				}
			}
		}
	}

	MPI_Bcast(&bool_inFile, 1, MPI_INT, 0, world);

	//If none of the searched words are in backward index just quit all nodes.
	if(bool_inFile == 0){
		MPI_Barrier(world);
		MPI_Finalize();
		return 0;
	}

	MPI_Bcast(&bool, 1, MPI_INT, 0, world);
	MPI_Bcast(&sizeDocList,1, MPI_INT, 0, world );

	//for all other nodes. Make space for adjacency matrix
	if(me!=0){
		int i;
		iniFloatMatrix(&A, sizeDocList, sizeDocList);
		listArray = (char **)malloc(sizeof(char*)*sizeDocList);
		for(i=0; i<sizeDocList; i++){
			listArray[i] = (char *)malloc(sizeof(char)*SIZE);
		}
	}

	int i;
	for(i = 0; i < sizeDocList*sizeDocList; i+=sizeDocList){
		MPI_Bcast(&A.arr[i],sizeDocList, MPI_FLOAT, 0, world );
	}
	for(i=0; i<sizeDocList; i++){
		MPI_Bcast(listArray[i], SIZE, MPI_CHAR, 0, world);
	}
	float *x = (float *)malloc(sizeof(float)*sizeDocList);

	//IF the adjacency matrix is not full of zeros call eigenvector.
	if(bool == 1){
		eigenvector(A, x);
		if(me==0){
			int pos=0;
			int k;
			float temp=0;
			char *tmpString = (char *)malloc(sizeof(char)*100);

			//if the size of the returned eigenvector is > 10 print out top 10
			//else just print all out.
			if( sizeDocList > 10 ){
				printf("TOP 10 PAGE IDs\n");
				for(i=0; i<10; i++){
					for(k=i+1; k<sizeDocList; k++){
						if(x[i] < x[k]){
							temp = x[k];
							x[k] = x[i];
							x[i] = temp;

							memset(tmpString, 0, strlen(tmpString));
							strcpy(tmpString, listArray[k]);
							memset(listArray[k], 0, strlen(listArray[k]));
							strcpy(listArray[k], listArray[i]);
							memset(listArray[i], 0, strlen(listArray[i]));
							strcpy(listArray[i], tmpString);
						}
					}
					printf("%s\n", listArray[i]);
				}
			}
			else{
				printf("TOP PAGE IDs\n");
				for(i=0; i<sizeDocList; i++){
					for(k=i+1; k<sizeDocList; k++){
						if(x[i] < x[k]){
							temp = x[k];
							x[k] = x[i];
							x[i] = temp;

							memset(tmpString, 0, strlen(tmpString));
							strcpy(tmpString, listArray[k]);
							memset(listArray[k], 0, strlen(listArray[k]));
							strcpy(listArray[k], listArray[i]);
							memset(listArray[i], 0, strlen(listArray[i]));
							strcpy(listArray[i], tmpString);
						}
					}
					printf("%s\n", listArray[i]);
				}
			}
			free(tmpString);
		}
	}
	else{
		//this means adjacency matrix is empty so print out top 10
		if(me==0){
			if( sizeDocList < 10 ){
				printf("TOP PAGE IDs\n");
				for(i=0; i<sizeDocList; i++){
					printf("%s\n", listArray[i]);
				}
			}
			else{
				printf("TOP 10 PAGE IDs\n");
				for(i=0; i<10; i++){
					printf("%s\n", listArray[i]);
				}
			}
		}
	}
	/**************************************************************************************************
	*/

	if(me==0){
		delete_all(refs);
		delete_all(searching);
	}

	for(i=0; i<sizeDocList; i++){
		free(listArray[i]);
	}
	free(listArray);
	free(x);

	MPI_Barrier(world);
	MPI_Finalize();

	return 0;
}
