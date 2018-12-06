#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <string.h>

typedef struct drand48_data Data;

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

AdjacencyNode* NewAdjacencyNode(int dest)
{
    AdjacencyNode *new = malloc(sizeof(AdjacencyNode));
    new->dest = dest;
    new->next = NULL;
    return new;
}

Graph* NewGraph(int nodeCount)
{
    Graph *new = malloc(sizeof(Graph));
    graph->nodeCount = nodeCount;
    graph->list = malloc(numNodes * sizeof(AdjList));
    //init adjacencylist
    for(int i = 0; i < nodeCount; i++) {
        graph->list[i].numLinks = 0;
        graph->list[i].head = NULL;
    }
    return graph;
}

void NewEdge(Graph *graph, int src, int dest)
{
    Node *new = NewAdjacencyNode(dest);
    new->next = graph->list[src].head;
    graph->list[src].head = new;//attach edge
    graph->list[src].linkCount++;//increase count
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

void BuildGraph(FILE *fp, Graph *graph) {
    char line[32];
    int x,y = 0;
    while(fgets(line, 32, fp) != NULL) {
        if (line[0] != '#') {//commented line
            line = strtok(line, " \t");
            x = atoi(temp);
            line = strtok(NULL, " \t");
            y = atoi(temp
            NewEdge(graph, x, y);
        }
    }
    fclose(fp);
}
