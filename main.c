#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <string.h>

//resource pages.tacc.utexas.edu/omp-loop.html iterations are independent but a regular directive

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
void PageRankEstimator(Graph *graph, int K, double D, int vertices[], int p);
void TopFive(int vertices[], int N);

int main(int argc, char *argv[])
{
    double D, Taverage, T0, T1 = 0;//entered at program start as a small float
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
    printf("P %d K %d D %f File %s\n", p, K, D, file);

    omp_set_num_threads(p);
    #pragma omp parallel//init parrallel
    {
        assert(p == omp_get_num_threads());
        int rank = omp_get_thread_num();
        printf("Rank %d & Thread %d\n", rank, omp_get_num_threads());
    }

    //this could be done two ways. read file then create graph, or create graph then read file.
    //if we create graph first, we can go line by line and not atempt to store things in a buffer.
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Unable to open file.\n");
        exit(1);
    }
    printf("Finding Max...\n");
    while(fgets(line, 64, fp) != NULL) {
        if (line[0] != '#') {//comment
            char *temp = strtok(line, " \t");
            x = atoi(temp);
            temp = strtok(NULL, " \t");
            y = atoi(temp);
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
    printf("Last Node %d\n", maxNode);

    printf("Init AdjacencyList...\n");
    //new graph
    vertices = malloc(sizeof(int) * maxNode);
    memset(vertices, 0, sizeof(vertices));
    printf("Init graph root...\n");
    Graph *graph = malloc(sizeof(Graph));//init root
    graph->nodeCount = maxNode;
    graph->list = malloc(maxNode * sizeof(AdjacencyList));
    //init walk
    printf("Init graph...\n");
    int l;
    for(l = 0; l < maxNode; l++) {
        graph->list[l].linkCount = 0;
        graph->list[l].head = NULL;
    }
    printf("Build graph...\n");
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
    for(i = 0; i < 1; i++) {
        T0 = omp_get_wtime();
        PageRankEstimator(graph, K, D, vertices, p);
        T1 = omp_get_wtime() - T0;
        Taverage += T1;
    }
    TopFive(vertices, graph->nodeCount);
    printf("Average time = %f seconds\n", Taverage/1);
    return 0;
}

void PageRankEstimator(Graph *graph, int K, double D, int vertices[], int p) {
    omp_set_num_threads(p);
    int N = graph->nodeCount;
    int index, localNode;
    AdjacencyNode *node;

    #pragma omp parallel for schedule(static) shared(vertices, graph) private(node, localNode)
    for(index = 0; index < N; index++)
    {
        node = graph->list[index].head;
        localNode = index;
        int jindex;
        printf("Index %d\n", index);
        for(jindex = 0; jindex < K; jindex++)
        {
            #pragma omp atomic
            vertices[localNode] += 1;
            double result;
            Data buf;
            //printf("Getting random coin flip...\n");
            int rank = omp_get_thread_num();
            int seed = rank + 1;
            seed = seed * index;
            srand48_r(time(NULL) + seed, &buf);
            drand48_r(&buf, &result);
            printf("Result %f\n", result);
            if(result <= D) {
                printf("Getting random node...\n");
                int rank = omp_get_thread_num();
                int seed = rank +1;
                seed = seed * index;
                int gotoNode = rand_r(&seed) % N;
                node = graph->list[gotoNode].head;
                localNode = gotoNode;
            }
            else {
                if(graph->list[localNode].linkCount != 0) {
                    printf("Getting random neighboring node...\n");
                    int rank = omp_get_thread_num();
                    int seed = rank + 1;
                    seed = seed * index;
                    int rNode = rand_r(&seed) % graph->list[localNode].linkCount + 1;
                    //AdjacencyNode *nodeNeighbor = graph->list[rNode].head;
                    int kindex;
                    //printf("Walking to the random neighbor...\n");
                    for(kindex = 1; kindex < rNode; kindex++)
                        node = node->next;
                    int gotoNode = node->dest;//now have rNode
                    printf("Hello %d.\n", jindex);
                    node = graph->list[gotoNode].head;
                    localNode = gotoNode;
                }
            }
        }
    }
}

void TopFive(int vertices[], int N)
{
    int i;
    int j;
    printf("Sorting vertices...\n");
    for(i=0; i<N; i++)
    {
        for(j=i+1; j<N; j++)
        {
            if(vertices[i] < vertices[j])
            {
                int tmp = vertices[i];
                vertices[i] = vertices[j];
                vertices[j] = tmp;
            }
        }
    }

    i=0;
    printf("Printing top 5\n");
    for(i=0; i<5; i++)
    {
        printf("%d: %d\n", i, vertices[i]);
    }
}

AdjacencyNode* NewAdjacencyNode(int dest)
{
    AdjacencyNode *new = malloc(sizeof(AdjacencyNode));
    new->dest = dest;
    new->next = NULL;
    return new;
}

void printGraph(Graph *graph)
{
    int i;
    for(i = 0; i < graph->nodeCount; i++) {
        AdjacencyNode *node = graph->list[i].head;
        printf("Node %d walks to", i);//print first node in list
        while(node != NULL) {
            printf(" %d.", node->dest);//print remainder in list
            node = node->next;
        }
        printf("Nowhere\n");
    }
}
