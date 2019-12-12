#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <mpi.h>


#define SIZE 256
#define DOCU_SIZE 256
#define FILENAME "arxiv-metadata.txt"
#define FILENAME2 "arxiv-citations.txt"


// LINK LIST NODE
typedef struct link_node{
	char document_name[DOCU_SIZE];
	struct link_node* next;
} link_node;



// A NODE IN THE BINARY TREE CONTAINS THE WORD AND THE LINKED LIST WHICH IS A LIST OF ALL IDS THAT THE WORD IS IN
typedef struct node {
	char word[SIZE];
	link_node *document_list;
	//index_word* ind_word;
	struct node* left;
	struct node* right;
} node;

//prototypes for functions
node* insert_node(node*, char[],FILE*);
void display_tree(node*, FILE*);
void delete_all(node*);

//DELETES A LINK LIST
void delete_list(link_node* head){
	if(head!=NULL){
		delete_list(head->next);
		free(head);
		head = NULL;
	}
}

//CREATES A LINK LIST NODE
link_node* create_link(char docu_name[DOCU_SIZE],node* root, FILE* index,node* curr_node){
	int i;

	link_node *new_node = (link_node*) malloc(sizeof(link_node));
	if(new_node!=NULL){ // if the malloc was successful, create new linked list
		for(i=0; i < strlen(docu_name); i++){
			new_node->document_name[i] = docu_name[i];
		}
		new_node->document_name[strlen(docu_name)] = '\0';
		new_node->next = NULL;
		return new_node;

	} else{ // if malloc broke, return NULL
		return NULL;

	}
}


//INSRETS A LINK INTO THE LINKED LIST
link_node* insert_link(link_node* head,char docu_name[DOCU_SIZE], node* root, FILE* back_ind,node* s){
	char w[SIZE];
	int i;
	if(head == NULL) // if head is NULL make the new node the head
		head = create_link(docu_name,root, back_ind,s);
	else{ // otherwise put a node at the end of the file
		link_node *cursor = head;
		link_node *prev = NULL;
		while(cursor!=NULL){
			if(strcmp(cursor->document_name, docu_name) == 0){
				return head;
			}
			prev = cursor;
			cursor = cursor->next;
		}
		link_node* tmp = create_link(docu_name, root,back_ind, s);
		if(tmp != NULL){ // if the malloc was successful 
			prev->next = tmp; //set the the node at the end of the list, otherwise return NULL
			
		} else{
		 return NULL; 
		}

	}
	return head;
}


//PRINTS A LINKED LIST
void print_list(link_node* head, FILE* index){
	link_node *cursor = head;

	if(head == NULL){
		printf("\n");
		return;
	}
	while(cursor->next!=NULL){
		fprintf(index, "%s ", cursor->document_name);
		cursor = cursor->next;
	}

	fprintf(index,"%s", cursor->document_name);
	fprintf(index,"\n");
}


//CREATES A NODE IN THE BINARY SEARCH TREE
node* create_node(char word[SIZE], FILE* index, node* root){ // CREATE A NODE
	int i;
	node *new_node = (node*)malloc(sizeof(node));
	if(new_node!=NULL){
		for(i=0; i < strlen(word); i++){
			new_node->word[i] = word[i];
		}
		new_node->word[strlen(word)] = '\0';
		new_node->left = NULL;
		new_node->right = NULL;
		new_node->document_list = NULL;
		return new_node;
	}else{
		return NULL;
	}
}

//PRINTS THE BINARY TREE AND EACH NODES LINKED LIST TO THE FILE RECURSIVELY
void display_tree(node* cursor, FILE* index){
	if(cursor == NULL)
		return;

	fprintf(index,"%s\n",cursor->word);
	/*
	   if(cursor->left != NULL)
	   printf(" (L:%s)",cursor->left->word);
	   if(cursor->right != NULL)
	   printf(" (R:%s)",cursor->right->word);
	   */

	print_list(cursor->document_list, index);


	display_tree(cursor->left, index);
	display_tree(cursor->right, index);
}

//DELETES ALL NODES
//CALLABLE AT END OF PROGRAM
void delete_all(node *cursor){
	if(cursor != NULL){
		delete_all(cursor->left);
		delete_all(cursor->right);
		delete_list(cursor->document_list);
		free(cursor);
		cursor = NULL;
	}

}

// CREATE A NODE AND INSERTS INTO TREE, CALLABLE
node* insert_node(node *root, char word[SIZE],FILE* back_ind){
	if(root==NULL)
	{
		root = create_node(word,back_ind,root);
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
		if(left == 1){

			node* tmp = create_node(word, back_ind,root);

			if(tmp == NULL)
				return NULL;

			prev->left = tmp;

		}	else{

			node* tmp = create_node(word, back_ind,root);

			if(tmp == NULL)
				return NULL;


			prev->right = tmp;


		}
	}

	return root;
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
void check_tree(node* root, char word[SIZE]){
	node *s = search_tree(root, word);
	if(s!=NULL)
		printf("NODE FOUND: %s\n", s->word);
	else
		printf("NODE NOT FOUND\n");

}



//INSERTS A DOCUMENTS INTO A NODES DOCUMENT LIST, FINDS NODE USING WORD PARAMETER
int insert_document(node* root, char word[SIZE], char document[DOCU_SIZE],FILE* back_ind){
	node *s = search_tree(root, word); // SEARCHES THE TREE FOR A WORD
	if(s==NULL){ // IF A WORD IS NOT IN THE TREE, DONT DO ANYTHING
		//printf("NODE NOT FOUND\n");
		return 0;
	}
	link_node *tmp = insert_link(s->document_list, document,root,back_ind,s); // IF THE NODE WAS FOUND, INSERT THE DOCUMENT INTO ITS LINKED LIST
	if(tmp != NULL){// NO MALLOC PROBLEM RETURN 0
		s->document_list = tmp; // IF NO MALLOC ERROR
		return 0;
	} else{ // RETURN 1 IF MALLOC ERROR
		return 1;
	}

}

int main(){

	int nprocs, me;
	srand(time(0)+me);

	MPI_Init(NULL,NULL);
	MPI_Comm world = MPI_COMM_WORLD;
	MPI_Comm_size(world, &nprocs);
	MPI_Comm_rank(world,&me);

	/*FILE* tst;
	tst= fopen("test.txt","w");
	fputs("loading data...\n\n",tst);*/
	if(me == 0){
  	printf("loading data\n");
	}

	node *meta = NULL;
	FILE* back_ind ;
	back_ind = fopen("backward_index2.txt","w");
	char *line_buf = NULL;
	char *PageId = NULL;
	char *words = (char*)malloc(sizeof(char)*256);
	memset(words, '\0', sizeof(char)*256);
	size_t line_buf_size = 0;
	int line_count = 0, i, x;
	ssize_t line_size;
	char c;

	FILE *metadata2 = fopen(FILENAME, "r");
	if (!metadata2)
	{
		fprintf(stderr, "Error opening file '%s'\n", FILENAME);
		return EXIT_FAILURE;
	}

	int blocksize = 0, charcount, charcounter, k;
	fseek(metadata2,0,SEEK_END);
	charcount =  ftell(metadata2);
	blocksize = (charcount/nprocs);
	fclose(metadata2);

	FILE* metadata[nprocs]; // WE DO THIS SO EACH PROCESSOR CAN READ THE FILE WITH ITS OWN FILE POINTER
	for(k=0; k < nprocs; k++)
	{
		metadata[k] = fopen(FILENAME, "r");
		fseek(metadata[k], 0, SEEK_SET);
	}
	k=me;
	fseek(metadata[k], (k*blocksize), SEEK_SET);//SET EACH NODE AT A DIFFERENT PART IN THE FILE

	charcounter = 0;

	line_size = getline(&line_buf, &line_buf_size, metadata[k]);
	line_buf[strlen(line_buf)-1] = '\0';
	charcounter = charcounter + line_size;
	/* Loop through until we are done with the file. */
	//while (charcounter < (blocksize+100) && line_size >= 0)
	while (charcounter < (blocksize+100) && line_size >= 0){
		/* Show the line details */
		if(strcmp(line_buf, "++++++") == 0){
			if(PageId) {
				free(PageId); PageId=NULL;
			}
			line_size = getline(&PageId, &line_buf_size, metadata[k]);
			PageId[strlen(PageId)-1] = '\0';
			charcounter = charcounter + line_size;
			if(line_buf) {
				free(line_buf); line_buf=NULL;
			}
			line_size = getline(&line_buf, &line_buf_size, metadata[k]);
			charcounter = charcounter + line_size;
			if(line_buf){
				free(line_buf); line_buf=NULL;
			}
			line_size = getline(&line_buf, &line_buf_size, metadata[k]);
			charcounter = charcounter + line_size;
			if(line_buf){
				free(line_buf); line_buf=NULL;
			}
			line_size = getline(&line_buf, &line_buf_size, metadata[k]);
			charcounter = charcounter + line_size;
			if(line_size > 0){
				for(i=0; i < line_size; i++){
					memset(words, 0, sizeof(char)*256);
					x=0;
					while( (line_buf[i] != ' ') && (line_buf[i] != '\n') ){
						if( (line_buf[i] >= 'a' && line_buf[i] <= 'z') || (line_buf[i] >= 'A' && line_buf[i] <= 'Z') || (line_buf[i] == '-') ){
							words[x] = tolower(line_buf[i]);
							x++;
						}
						i++;
					}
					//printf("%c\n", c);
					words[x] = '\0';

					if(strlen(words) != 0){
						//printf("%s\n", words);
						node* tmp = insert_node(meta, words,back_ind);

						if(tmp!=NULL){
							meta = tmp;

						} else {
						//	printf("IN ROOT: HERE\n");

							display_tree(meta, back_ind);
							delete_all(meta);
							meta = NULL;
							meta = 	insert_node(meta,words,back_ind);
						}
							//printf("HERE0\n");
						if(insert_document(meta, words, PageId,back_ind) != 0){
						//	printf("IN LIST: HERE\n");

							display_tree(meta,back_ind);
							delete_all(meta);
							meta=NULL;
							meta = insert_node(meta,words,back_ind);
							insert_document(meta,words,PageId,back_ind);

						}

					}
				}
			}
			else{
				break;
			}
		}
		else{
			line_size = -1;
		}

		if(line_buf){
			free(line_buf); line_buf=NULL;
		}
		line_size = getline(&line_buf, &line_buf_size, metadata[k]);
		line_buf[strlen(line_buf)-1] = '\0';
		charcounter = charcounter + line_size;
	}
	printf("End Metadata\n");
	free(words);
	display_tree(meta,back_ind);

	fclose(metadata[k]);



	delete_all(meta);
	//delete_all(refs);
	if(me == 0){
		printf("finished!\n");
	}
	MPI_Finalize();
	return 0;
}
