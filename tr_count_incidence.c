#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define NIL UINT_MAX

typedef enum {false, true} bool;

//defining the adjacency list ie the list of nodes adjacent to the current node under exam
//variable D in the pseudocode of the paper
typedef struct node{
	unsigned int vertex;
	struct node *next_node;
	struct node *prev_node;
} node_t;
typedef struct adjacency_list{
	unsigned int size;
	node_t *head;
} adjacency_list_t;
void empty_adj_list(adjacency_list_t *list){
	if(list->size < 1) return;
	node_t *current, *pred;
	current = list->head;
	while(current->next_node != NULL)
		current = current->next_node;
	while(current != NULL){
		pred = current->prev_node;
		free(current);
		current = pred;	
	}
	list->size = 0;
	list->head = NULL;
}
void add_node_adj_list(adjacency_list_t *list, node_t *node){
	if(list->size++ < 1) list->head = node;
	else{
		node_t *tail = list->head;
		while(tail->next_node != NULL)
			tail = tail->next_node;
		tail->next_node = node;
		node->prev_node = tail;
	}
}
unsigned int get_vertex_adj(adjacency_list_t *list, unsigned int index){
	node_t *current = list->head;
	while(--index)
		current = current->next_node;
	return current->vertex;
}
void print_adj_list(adjacency_list_t *list){
	printf("\n--ADJACENCY LIST:\n");
	unsigned int index = 0;
	node_t *current = list->head;
	if(list->size < 1) printf("Empty list\n");
	else{
		while(current != NULL){
			printf("Node of index %u is at address: %p; contains vertex (%u) and points to nodes at addresses: %p and %p\n", 
				index++, (void*)current, current->vertex, (void*)current->prev_node, (void*)current->next_node);
			current = current->next_node;
		}
	}
}

//defining the hash table
typedef struct nugget{
	unsigned int vertex1, vertex2;
	unsigned int c, c1, c2;
	struct nugget *next;
} nugget_t;
typedef struct bucket{
	nugget_t *head;
} bucket_t;
typedef struct hash_table{
	unsigned int size, r1, r2;
	bucket_t *bucket_list;
} hash_table_t;
void print_hash_table(hash_table_t *hash_table){
	printf("\n--HASH TABLE:\n");
	for(unsigned int bucket_index = 0; bucket_index < hash_table->size; bucket_index++){
		if(hash_table->bucket_list[bucket_index].head == NULL)
			printf("No element in the bucket at index %u of the hash table\n", bucket_index);
		else{
			nugget_t* current = hash_table->bucket_list[bucket_index].head;
			do{
				printf("Bucket %u of the hash table contains [e: (%u,%u), c: %u, c1: %u, c2: %u] at address: %p\n", 
					bucket_index, current->vertex1, current->vertex2, 
					current->c, current->c1, current->c2, (void*)current);
				current = current->next;
			}while(current != NULL);
		}
	}
}
void clean_hash_table(hash_table_t *hash_table){
	for(unsigned int bucket_index = 0; bucket_index < hash_table->size; bucket_index++){
		if(hash_table->bucket_list[bucket_index].head != NULL){
			nugget_t* to_be_eliminated = hash_table->bucket_list[bucket_index].head;
			while(to_be_eliminated->next != NULL){
				nugget_t* temp = to_be_eliminated;
				to_be_eliminated = to_be_eliminated->next;
				free(temp);
			}
			free(to_be_eliminated);
		}
	}
}
unsigned int compute_hash(const hash_table_t hash_table, const unsigned int u, const unsigned int v){
	if(u < v) return (hash_table.r1 * u + hash_table.r2 * v) % hash_table.size;
	return (hash_table.r1 * v + hash_table.r2 * u) % hash_table.size;
}

//checkTri() in the paper
void check_triangles(hash_table_t *hash_table, const unsigned int vertex1, const unsigned int vertex2){
	unsigned int bucket_index = compute_hash(*hash_table, vertex1, vertex2);
	nugget_t *head = hash_table->bucket_list[bucket_index].head;

	while(head != NULL){
		if((head->vertex1 == vertex1 && head->vertex2 == vertex2) || (head->vertex1 == vertex2 && head->vertex2 == vertex1)){
			head->c2 = head->c1;
			head->c1 = head->c;
			head->c = 0;
		}
		head = head->next;
	}
}

//insertIntoHash() in the paper
void insert_into_hash(hash_table_t *hash_table, const unsigned int vertex1, const unsigned int vertex2){
	unsigned int bucket_index = compute_hash(*hash_table, vertex1, vertex2);

	nugget_t *head = hash_table->bucket_list[bucket_index].head;
	if(head == NULL){
		nugget_t *new_nugget = (nugget_t*)malloc(sizeof(nugget_t));
		new_nugget->vertex1 = vertex1, new_nugget->vertex2 = vertex2, new_nugget->c = 1, new_nugget->c1 = 0, new_nugget->c2 = 0, new_nugget->next = NULL;
		hash_table->bucket_list[bucket_index].head = new_nugget;
		return;
	}
	while(head->next != NULL){
		if((head->vertex1 == vertex1 && head->vertex2 == vertex2) || (head->vertex1 == vertex2 && head->vertex2 == vertex1)){
			head->c++;
			return;
		}
		head = head->next;
	}
	if((head->vertex1 == vertex1 && head->vertex2 == vertex2) || (head->vertex1 == vertex2 && head->vertex2 == vertex1)) head->c++;
	else{
		nugget_t *new_nugget = (nugget_t*)malloc(sizeof(nugget_t));
		new_nugget->vertex1 = vertex1, new_nugget->vertex2 = vertex2, new_nugget->c = 1, new_nugget->c1 = 0, new_nugget->c2 = 0, new_nugget->next = NULL;
		head->next = new_nugget;
	}	
}

//nextSample() in the paper
unsigned int next_sample(const unsigned int inv_prob, const unsigned int curr_index){
	return (inv_prob == 1)? curr_index + 1 : curr_index + ceil(log((double)rand() / RAND_MAX)/log(1.0 - (1.0 / inv_prob)));
}

//clearHalfSampleSet() in the paper
unsigned int clean_half_sample_set(hash_table_t *hash_table, unsigned int sample_size){
	unsigned int c_iterations, c1_iterations, c2_iterations;
	nugget_t *current, *pred;
	for(unsigned int bucket_index = 0; bucket_index < hash_table->size; bucket_index++){
		current = hash_table->bucket_list[bucket_index].head;
		pred = NULL;
		while(current != NULL){
			c_iterations = current->c;
			for(unsigned int c_counter = 0; c_counter < c_iterations; c_counter++)
				if((double)rand() / RAND_MAX > 0.5){
					current->c--;
					sample_size--;
				}
			c1_iterations = current->c1;
			for(unsigned int c1_counter = 0; c1_counter < c1_iterations; c1_counter++)
				if((double)rand() / RAND_MAX > 0.5){
					current->c1--;
					sample_size--;
				}
			c2_iterations = current->c2;
			for(unsigned int c2_counter = 0; c2_counter < c2_iterations; c2_counter++)
				if((double)rand() / RAND_MAX > 0.5){
					current->c2--;
					sample_size--;
				}
			
			if(current->c == 0 && current->c1 == 0 && current->c2 == 0){
				nugget_t *temp = current;
				if(pred != NULL){
					pred->next = current->next;
					current = pred->next;
				} 
				else{
					hash_table->bucket_list[bucket_index].head = current->next;
					current = hash_table->bucket_list[bucket_index].head;
				} 
				free(temp);
			}
			else{
				pred = current;
				current = current->next;
			}
		}
	}
	return sample_size;
}

//calculateT() in the paper
unsigned int calculate_triangles(hash_table_t *hash_table){
	unsigned int counter = 0;
	nugget_t *current;
	for(unsigned int bucket_index = 0; bucket_index < hash_table->size; bucket_index++){
		current = hash_table->bucket_list[bucket_index].head;
		while(current != NULL){
			counter += (2*current->c1 + current->c2);
			current = current->next;
		}
	}
	return counter;
}

//the OptimizedOnePassSampling-Incidence(r) algorithm from the paper
unsigned int count_triangles(const unsigned int num_samples, const char* stream_name)
{
	//initialization just like in the paper 
	unsigned int sample_size = 0;
	unsigned long num_2paths_seen = 0;
	unsigned long next_sample_index = 1;
	unsigned int inv_sample_prob = 1;
	unsigned int max_sample_size = num_samples;
	unsigned int current_vertex = NIL;

	//initializing the hash table
	unsigned int r1 = (unsigned int)rand() % num_samples + 1;
	unsigned int r2 = (unsigned int)rand() % num_samples + 1;
	bucket_t* bucket_list = malloc(2*num_samples*sizeof(bucket_t));
	for(unsigned int bucket_index = 0; bucket_index < 2*num_samples; bucket_index++)
		bucket_list[bucket_index].head = NULL;
	hash_table_t hash_table = {.size = 2*num_samples, .r1 = r1, .r2 = r2, .bucket_list = bucket_list};

	//initializing the adjacency list
	adjacency_list_t adj_list = {.size = 0, .head = NULL};

	//file that contains the edge stream
	FILE* input_stream = fopen(stream_name, "r");

	//edge from the stream currently being read
	unsigned int end_vertex1_read, end_vertex2_read;
	//edge from the stream currently being processed
	unsigned int end_vertex1, end_vertex2;
	//while there are still edges in the stream
	bool stream_ended = false;
	while(!stream_ended){
		if(EOF == fscanf(input_stream, "%u 	%u", &end_vertex1_read, &end_vertex2_read)){
			stream_ended = true;
			continue;
		}

		while((num_2paths_seen < next_sample_index) && !stream_ended){
			end_vertex1 = end_vertex1_read;
			end_vertex2 = end_vertex2_read;
			if(end_vertex1 != current_vertex){
				empty_adj_list(&adj_list);
				current_vertex = end_vertex1;
			}
			node_t *last_end_vertex = (node_t*)malloc(sizeof(node_t));
			last_end_vertex->vertex = end_vertex2, last_end_vertex->next_node = NULL, last_end_vertex->prev_node = NULL;
			add_node_adj_list(&adj_list, last_end_vertex);

			check_triangles(&hash_table, end_vertex1, end_vertex2);

			num_2paths_seen += (adj_list.size - 1); 

			if(num_2paths_seen < next_sample_index && EOF == fscanf(input_stream, "%u 	%u", &end_vertex1_read, &end_vertex2_read)) stream_ended = true;
		}
		while(num_2paths_seen >= next_sample_index){
			unsigned int vertex_index = adj_list.size + next_sample_index - num_2paths_seen - 1;
			unsigned int end_vertex3 = get_vertex_adj(&adj_list, vertex_index);
			insert_into_hash(&hash_table, end_vertex2, end_vertex3);
			sample_size++;
			next_sample_index = next_sample(inv_sample_prob, next_sample_index);
		}
		while(num_2paths_seen >= max_sample_size){
			max_sample_size *= 2;
			inv_sample_prob *= 2;
			sample_size = clean_half_sample_set(&hash_table, sample_size);
		}
	}
	fclose(input_stream);

	unsigned int beta = calculate_triangles(&hash_table);
	empty_adj_list(&adj_list);
	clean_hash_table(&hash_table);
	free(bucket_list);

	return (sample_size > 0)? (beta*num_2paths_seen)/(3*sample_size) : 0;
}

int main(int argc, char *argv[]){
	if(argc < 2){
		printf("\nToo few command line arguments!\nCorrect usage: num_samples filename\n");
		return 1;
	}
	int num_samples = (unsigned int)atoi(argv[1]);
	char* filename = argv[2];
	printf("\n*********************************************************\n");
	printf("*********************************************************\n");
	printf("SAMPLE SIZE: %u\nFILENAME: %s", num_samples, filename);
	printf("\n*********************************************************\n");
	printf("*********************************************************\n");

	srand(time(0));
	unsigned int triangles_found = count_triangles(num_samples, filename);
	printf("\nTRIANGLES COUNTED: %u\n", triangles_found);

	return 0;
}