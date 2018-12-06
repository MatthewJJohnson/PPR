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
    double D = 0, double Taverage = 0, T0 = 0, T1 = 1;//entered at program start as a small float
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
            line = strtok(line, " \t");
            x = atoi(line);
            line = strtok(NULL, " \t");
            y = atoi(line);
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
    graph->list = malloc(numNodes * sizeof(AdjList));
    //init walk
    for(int i = 0; i < nodeCount; i++) {
        graph->list[i].numLinks = 0;
        graph->list[i].head = NULL;
    }

    //init Graph
    while(fgets(line, 32, fp) != NULL) {
        if (line[0] != '#') {//commented line of file
            line = strtok(line, " \t");
            x = atoi(temp);
            line = strtok(NULL, " \t");
            y = atoi(temp);
            Node *new = NewAdjacencyNode(y);
            new->next = graph->list[x].head;
            graph->list[x].head = new;//attach edge
            graph->list[x].linkCount++;//increase count
        }
    }
    fclose(fp);

    for(i = 0; i < 10; i++) {
        T0 = omp_get_wtime();
        PageRankEstimator(graph, K, D, vertices);
        T1 = omp_get_wtime() - T0;
        Taverage += T1;
    }

    printf("Average time = %f seconds\n", Taverage/10.0);
    return 0;
}

void PageRankEstimator(Graph *graph, int K, double D, int vertices[]) {
    omp_set_num_threads(p);
    int N = graph->numNodes;
    int index, localNode;
    Node *node;

    #pragma omp parallel for schedule(static) shared(vertices, graph) private(node, localNode)
    for(index = 0; index < N; index++)
    {
        node = graph->list[i].head;
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
            seed = seed * i;
            srand48_r(time(NULL) + seed, &buf);
            drand48_r(&buf, &result);
            if(result == 1) {
                int rank = omp_get_thread_num();
                int seed = rank + 1;
                seed = seed * iterator;
                int gotoNode = rand_r(&seed) % nodeCount;
                node = graph->list[gotoNode].head;
                localNode = gotoNode;
            }
            else {
                if(graph->list[localNode].numLinks != 0) {
                    int rank = omp_get_thread_num();
                    int seed = rank + 1;
                    seed = seed * iterator;
                    int rNode = rand_r(&seed) % neighborCount + 1;
                    Node *node = head;
                    int kindex;
                    for(kindex = 1; kindex < rNode; kindex++)
                        node = node->next;
                    int gotoNode = node->dest;//now have rNode
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
        printf("%d: %d\n", i, arr1[i]);
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
    for(int i = 0; i < graph->nodeCount; i++) {
        Node *node = graph->list[i].head;
        printf("Node %d walks to", i);//print first node in list
        Node *node = head;
        while(node != NULL) {
            printf(" %d.", node->dest);//print remainder in list
            node = node->next;
        }
        printf("Nowhere\n");
    }
}
