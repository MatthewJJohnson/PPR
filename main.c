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

int main(int argc, char *argv[])
{
    double D, Taverage, T0, T1 = 0;//entered at program start as a small float
    int *vertices;
    int maxNode;
    char *file;
    int p,i,max,x,y, K = 0;
    char line[32];
    FILE *fp;

    p = atoi(argv[1]);
    omp_set_num_threads(p);
    #pragma omp parallel//init parrallel
    {
        assert(p == omp_get_num_threads());
        int rank = omp_get_thread_num();
        printf("Rank %d & Thread %d\n", rank, omp_get_num_threads());
    }
    K = atoi(argv[2]);
    sscanf(argv[3], "%lf", &D);
    file = argv[4];
    printf("P %d K %d D %f File %s\n", p, K, D, file);
    //this could be done two ways. read file then create graph, or create graph then read file.
    //if we create graph first, we can go line by line and not atempt to store things in a buffer.
    fp = fopen(file, "r");
    if (fp == NULL) {
        printf("Unable to open file.\n");
        exit(1);
    }

    while(fgets(line, 32, fp) != NULL) {
        if (line[0] != '#') {
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
    maxNode = max;
    printf("Last Node %d\n", maxNode);

    //new graph
    vertices = malloc(sizeof(int) * maxNode +1);
    memset(vertices, 0, sizeof(vertices));
    Graph *graph = malloc(sizeof(Graph));//init root
    graph->nodeCount = maxNode+1;
    graph->list = malloc(maxNode+1 * sizeof(AdjacencyList));
    //init walk
    i = 0;
    for(i = 0; i < graph->nodeCount; i++) {
        graph->list[i].linkCount = 0;
        graph->list[i].head = NULL;
    }

    //init Graph
    while(fgets(line, 32, fp) != NULL) {
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

    for(i = 0; i < 10; i++) {
        T0 = omp_get_wtime();
        PageRankEstimator(graph, K, D, vertices, p);
        T1 = omp_get_wtime() - T0;
        Taverage += T1;
    }

    printf("Average time = %f seconds\n", Taverage/10.0);
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
            if(result == 1) {
                int rank = omp_get_thread_num();
                int seed = rank + 1;
                seed = seed * index;
                int gotoNode = rand_r(&seed) % graph->nodeCount;
                node = graph->list[gotoNode].head;
                localNode = gotoNode;
            }
            else {
                if(graph->list[localNode].linkCount != 0) {
                    int rank = omp_get_thread_num();
                    int seed = rank + 1;
                    seed = seed * index;
                    int rNode = rand_r(&seed) % graph->list[localNode].linkCount + 1;
                    AdjacencyNode *nodeNeighbor = graph->list[rNode].head;
                    int kindex;
                    for(kindex = 1; kindex < rNode; kindex++)
                        nodeNeighbor = nodeNeighbor->next;
                    int gotoNode = nodeNeighbor->dest;//now have rNode
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
