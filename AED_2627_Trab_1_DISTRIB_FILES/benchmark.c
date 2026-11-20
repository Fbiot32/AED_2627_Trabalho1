/**
 * @file benchmark.c
 * @brief Measurement program for sorting and picking functions in vector_scene
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
#include <math.h>

#include "primitive.h"
#include "vector_scene.h"
#include "elapsed_time.h"

#define PI 3.14159265358979323846   // M_PI não está garantido em -std=c17

#define SEED 12 // seed para os casos aleatorios serem iguais entre testes

#define NUM_PICKS 1000   // M: número de operações de picking por cena

typedef enum { 
    RANDOM, 
    ORDERED,
    INVERTED 
} SortCase;


static VectorScene* create_scene_sort(size_t N, unsigned int seed, SortCase caso) {

    srand(seed);

    int k = N * 10;

    Color bg = {255, 255, 255};
    VectorScene* scene = vector_scene_create(200, 200, bg);

    Color color = {0, 255, 0};
    double radius = 40.0;
    Point center = {100.0, 100.0};

    // Nota: assume-se que não há falhas de memória; os valores de retorno não são verificados
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
        Primitive* circle = primitive_create_circle(-1, center, radius, color, 1.0, z);
        vector_scene_add_primitive(scene, circle);
    }
    return scene;
}

static VectorScene* create_scene_pick(size_t N, unsigned int seed, int num_vertices) {

    srand(seed);

    int k = N * 10;

    Color bg = {255, 255, 255};
    VectorScene* scene = vector_scene_create(600, 600, bg);

    Color color = {0, 255, 0};
    double radius = 20.0;

    // Nota: assume-se que não há falhas de memória; os valores de retorno não são verificados
    for (size_t i = 0; i < N; i++)
    {
        Point center = {rand() % 600, rand() % 600};
        int z = rand() % k;
        Primitive* p;
        if (num_vertices == 0) {
            p = primitive_create_circle(-1, center, radius, color, 1.0, z);
        } else {
            p = primitive_create_polygon(-1, color, 1.0, z);
            // Vértices igualmente espaçados numa circunferência de raio 20
            for (int j = 0; j < num_vertices; j++) {
                double ang = 2.0 * PI * j / num_vertices;
                Point vert = {center.x + radius * cos(ang), center.y + radius * sin(ang)};
                polygon_add_vertex(p, vert);
            }
        }
        vector_scene_add_primitive(scene, p);
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
    printf("%8s | %11s | %11s | %9s\n",
        "N", "Comparacoes", "Atribuicoes", "Tempo");

    for (size_t i = 0; i < num_sizes; i++)
    {
        VectorScene* scene = create_scene_sort(sizes[i], SEED, caso);

        double t1 = cpu_time();
        vector_scene_sort_by_z(scene, use_fast_sort, &sort_stats);
        double t2 = cpu_time();

        
        printf("%8zu | %11lu | %11lu | %9f\n", sizes[i], sort_stats.comparisons, sort_stats.swaps, t2-t1);

        vector_scene_destroy(scene);
        
    }

}


static void run_pick_benchmark(size_t sizes[], size_t num_sizes, int num_vertices) {

    if (num_vertices == 0) {
        printf("\n----------------- Circulos -----------------\n");
    } else {
        printf("\n----------------- Poligonos com %d vertices -----------------\n", num_vertices);
    }

    printf("M = %d selecoes por cena\n\n", NUM_PICKS);
    printf("%8s | %14s | %12s | %14s | %12s | %10s | %10s | %10s | %8s\n",
           "N", "Direta exatos", "Tempo dir.", "Otim. AABB", "Otim. exatos",
           "Tempo opt.", "Racio exat", "Racio ops", "Racio t");

    for (size_t i = 0; i < num_sizes; i++)
    {
        VectorScene* scene = create_scene_pick(sizes[i], SEED, num_vertices);

        
        Point points[NUM_PICKS];
        srand(SEED + 1);
        for (int k = 0; k < NUM_PICKS; k++) {
            points[k].x = (rand() % 640) - 20;
            points[k].y = (rand() % 640) - 20;
        }

        PickStats stats;
        unsigned long direct_exact = 0;
        unsigned long opt_aabb = 0, opt_exact = 0;
        int mismatches = 0;   // verificação: as duas estratégias devem escolher a mesma primitiva

        // Estratégia direta
        Primitive* direct_result[NUM_PICKS];
        double t1 = cpu_time();
        for (int k = 0; k < NUM_PICKS; k++) {
            direct_result[k] = vector_scene_pick_top_primitive(scene, points[k], false, &stats);
            direct_exact += stats.exact_tests;
        }
        double t2 = cpu_time();

        // Estratégia otimizada
        double t3 = cpu_time();
        for (int k = 0; k < NUM_PICKS; k++) {
            Primitive* p = vector_scene_pick_top_primitive(scene, points[k], true, &stats);
            opt_aabb += stats.aabb_tests;
            opt_exact += stats.exact_tests;
            if (p != direct_result[k]) {
                mismatches++;
            }
        }
        double t4 = cpu_time();

        double time_direct = t2 - t1;
        double time_opt = t4 - t3;


        unsigned long opt_total = opt_aabb + opt_exact;

        double ratio_exact = 0.0;
        if (opt_exact > 0) {
            ratio_exact = (double)direct_exact / opt_exact;     // ratio_exact: só testes exatos
        }
        double ratio_ops = 0.0;
        if (opt_total > 0) {
            ratio_ops = (double)direct_exact / opt_total;       // ratio_ops: total de operações
        }
        double ratio_time = 0.0;
        if (time_opt > 0) {
            ratio_time = time_direct / time_opt;            // ratio_time: tempo de CPU
        }
    

        printf("%8zu | %14lu | %12f | %14lu | %12lu | %10f | %10.2f | %10.2f | %8.2f\n",
               sizes[i], direct_exact, time_direct, opt_aabb, opt_exact,
               time_opt, ratio_exact, ratio_ops, ratio_time);

        if (mismatches > 0) {
            printf("   AVISO: %d selecoes deram resultados diferentes entre as estrategias\n",
                   mismatches);
        }

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

    run_sort_benchmark(sizes, num_sizes, false, INVERTED);
    run_sort_benchmark(sizes, num_sizes, true, INVERTED);
    printf("\n  ------------------------------------------------------------------------------------------------\n");

    printf("\n  ------------------------------------------------\n");
    printf("|                      Picking                     |\n");
    printf("  ------------------------------------------------\n");

    run_pick_benchmark(sizes, num_sizes, 0);    // círculos
    run_pick_benchmark(sizes, num_sizes, 4);    // poligonos
    run_pick_benchmark(sizes, num_sizes, 8);
    run_pick_benchmark(sizes, num_sizes, 18);

    return 0;
}
