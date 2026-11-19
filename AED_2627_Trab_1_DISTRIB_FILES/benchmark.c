/**
 * @file benchmark.c
 * @brief Measurement program for sorting functions in vector_scene
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

 /**
 * STUDENT NAME: Fábio Alexandre Coragem Espírito Santo
 * STUDENT NUMBER: 132325
 * STUDENT NAME: Francisco Miguel Ferreira Almeida
 * STUDENT NUMBER: 103751
 */

#define _POSIX_C_SOURCE 199309L   // Ativa o clock_gettime do POSIX

#include <stdio.h>                
#include <stdlib.h>
#include <stdbool.h>

#include "primitive.h"
#include "vector_scene.h"
#include "elapsed_time.h"

#define SEED 12

typedef enum { 
    RANDOM, 
    ORDERED,
    INVERTED 
} SortCase;


static VectorScene* create_scene(size_t N, unsigned int seed, SortCase caso) {

    srand(seed);

    int k = N * 10;

    Color bg = {255, 255, 255};
    VectorScene* scene = vector_scene_create(200, 200, bg);

    Color color = {0, 255, 0};
    Point center = {100.0, 100.0};

    for (size_t i = 0; i < N; i++)
    {
        int z = i;
        switch (caso)
        {
        case RANDOM:
            z = rand() % k;
            break;

        case ORDERED:
            z = i;
            break;
        
        case INVERTED:
            z = N - i;
            break;
        }
        Primitive* circle = primitive_create_circle(-1, center, 40.0, color, 1.0, z);
        vector_scene_add_primitive(scene, circle);
    }
    return scene;
}

static void run_sort_benchmark(size_t sizes[], size_t num_sizes, bool use_fast_sort, SortCase caso) {


    SortStats sort_stats = {0, 0};

    if (use_fast_sort) {
        printf("\n-------------------Merge_sort-------------------\n");
    } else {
        printf("\n-------------------Insertion_sort-------------------\n");
    }
    
    for (size_t i = 0; i < num_sizes; i++)
    {
        VectorScene* scene = create_scene(sizes[i], SEED, caso);

        double t1 = cpu_time();
        vector_scene_sort_by_z(scene, use_fast_sort, &sort_stats);
        double t2 = cpu_time();

        
        printf("N: %zu | Comparacoes: %lu | Atribuicoes: %lu | Tempo: %f\n", sizes[i], sort_stats.comparisons, sort_stats.swaps, t2-t1);

        vector_scene_destroy(scene);
        
    }

}

int main(void)
{

    size_t sizes[] = {100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000};
    size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);


    printf("\n  ------------------------------------------------\n");
    printf("|                      Random                      |\n");
    printf("  ------------------------------------------------\n");

    run_sort_benchmark(sizes, num_sizes, false, RANDOM);
    run_sort_benchmark(sizes, num_sizes, true, RANDOM);
    
    printf("\n  ------------------------------------------------\n");
    printf("|                     Ordered                     |\n");
    printf("  ------------------------------------------------\n");

    run_sort_benchmark(sizes, num_sizes, false, ORDERED);
    run_sort_benchmark(sizes, num_sizes, true, ORDERED);

    printf("\n  ------------------------------------------------\n");
    printf("|                     Inverted                    |\n");
    printf("  ------------------------------------------------\n");

    run_sort_benchmark(sizes,num_sizes, false, INVERTED);
    run_sort_benchmark(sizes,num_sizes, true, INVERTED);

    return 0;
}
