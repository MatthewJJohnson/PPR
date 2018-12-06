#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <string.h>

typedef struct adjacencynode
{
    int dest;
    struct adjacencynode *next;
}AdjacencyNode;//base node

typedef struct adjacencylist
{
    int linkCount;
    AdjacencyNode *head;
}AdjacencyList;//base list

typedef struct graph
{
    int nodeCount;
    AdjacencyList *list;
}Graph;//graph of lists

typedef struct drand48_data Data;

AdjacencyNode* NewAdjacencyNode(int dest);
void PageRank(Graph *graph, int vertices[], int p, int K, double D);
void TopFive(int vertices[], int N);

int main(int argc, char *argv[])
{
    double D, T0, T1 = 0;//entered at program start as a small float
    int *vertices;
    int maxNode;
    char *file;
    int p,i,max,x,y, K = 0;
    char line[64];
    FILE *fp;

    p = atoi(argv[1]);
    K = atoi(argv[2]);
    sscanf(argv[3], "%lf", &D);
    file = argv[4];

    omp_set_num_threads(p);
    #pragma omp parallel//init parrallel
    {
        assert(p == omp_get_num_threads());
        int rank = omp_get_thread_num();
        //printf("Rank %d has come alive!\n", rank);
    }

    //begin setup
    //this could be done two ways. read file then create graph, or create graph then read file.
    //if we create graph first, we can go line by line and not atempt to store things in a buffer.
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Unable to open file.\n");
        exit(1);
    }
    //printf("Finding Max...\n");
    while(fgets(line, 64, fp) != NULL) {
        if (line[0] != '#') {//comment
            char *temp = strtok(line, " \t");
            x = atoi(temp);
            temp = strtok(NULL, " \t");
            y = atoi(temp);
            //not optimized, not timed
            if (x > y) {//to greater than from
                if(x >= max)//to greater than max
                    max = x;//max
            } else {//from greater than to
                if(y >= max)//from greater than max
                    max = y;//max
            }
        }
    }
    fclose(fp);
    maxNode = max +1;
    //printf("Last Node %d\n", maxNode);

    //printf("Init AdjacencyList...\n");
    //new graph
    vertices = malloc(sizeof(int) * maxNode);
    memset(vertices, 0, sizeof(vertices));
    printf("Init graph root...\n");
    Graph *graph = malloc(sizeof(Graph));//init root
    graph->nodeCount = maxNode;
    graph->list = malloc(maxNode * sizeof(AdjacencyList));
    //init walk
    //printf("Init graph...\n");
    int l;
    for(l = 0; l < maxNode; l++) {
        graph->list[l].linkCount = 0;
        graph->list[l].head = NULL;
    }
    //printf("Build graph...\n");
    //init Graph
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Unable to open file.\n");
        exit(1);
    }
    while(fgets(line, 64, fp) != NULL) {
        if (line[0] != '#') {//commented line of file
            char *temp = strtok(line, " \t");
            x = atoi(temp);
            temp= strtok(NULL, " \t");
            y = atoi(temp);
            AdjacencyNode *new = NewAdjacencyNode(y);
            new->next = graph->list[x].head;
            graph->list[x].head = new;//attach edge
            graph->list[x].linkCount++;//increase count
        }
    }
    fclose(fp);

    printf("Running Page rank in parallel...\n");
    T0 = omp_get_wtime();
    PageRank(graph, vertices, p, K, D);
    T1 = omp_get_wtime() - T0;
    TopFive(vertices, graph->nodeCount);
    printf("Time = %f seconds\n", T1);
    return 0;
}

void PageRank(Graph *graph, int vertices[], int p, int K, double D) {
    int N = graph->nodeCount;
    int index, localNode;
    AdjacencyNode *node;

    #pragma omp parallel for schedule(static) private(node, localNode) shared(vertices, graph)
    for(index = 0; index < N; index++)//I,J,K
    {
        localNode = index;
        node = graph->list[index].head;
        int jindex;
        for(jindex = 0; jindex < K; jindex++)
        {
            #pragma omp atomic
            vertices[localNode] += 1;
            double result;
            Data buf;
            int rank = omp_get_thread_num();
            int seed = rank + 1;
            seed = seed * index;
            srand48_r(time(NULL) + seed, &buf);
            drand48_r(&buf, &result);
            if(result <= D) {
                int rank = omp_get_thread_num();
                int seed = rank +1;
                seed = seed * index;
                int gotoNode = rand_r(&seed) % N;
                localNode = gotoNode;
                node = graph->list[gotoNode].head;
            }
            else {
                if(graph->list[localNode].linkCount != 0) {
                    int rank = omp_get_thread_num();
                    int seed = rank + 1;
                    seed = seed * index;
                    int rNode = rand_r(&seed) % graph->list[localNode].linkCount + 1;
                    int kindex;
                    for(kindex = 1; kindex < rNode; kindex++){
                        node = node->next;
                    }
                    int gotoNode = node->dest;//now have rNode
                    localNode = gotoNode;                    
                    node = graph->list[gotoNode].head;
                }
            }
        }
    }
}

void TopFive(int vertices[], int N) 
{ 
    //SUUUUUUUUUUUUPER not optimized, not timed
    int *indexes;
    indexes = (int*) malloc(N * sizeof(int));  //memory allocated using malloc
    int i;
    for(i = 0; i < N; i++)
    {
        indexes[i] = i;
    }

    int k = 5;
    int j;
    int max, temp;
    // Partial selection sort, move k elements to front
    for (i = 0; i < k; i++)
    {
        max = i;
        for (j = i+1; j < N; j++)
        {
            if (vertices[j] > vertices[max])  {
                max = j;
            }
        }
        temp = vertices[i];
        vertices[i] = vertices[max];
        vertices[max] = temp;
        temp = indexes[i];
        indexes[i] = indexes[max];
        indexes[max] = temp;//capture original indice
    }

    for (i = 0; i < k; i++) {
        printf("Node %d has page rank %d\n", indexes[i], vertices[i]);
    }
} 

AdjacencyNode* NewAdjacencyNode(int dest)
{
    AdjacencyNode *new = malloc(sizeof(AdjacencyNode));
    new->dest = dest;
    new->next = NULL;
    return new;
}
