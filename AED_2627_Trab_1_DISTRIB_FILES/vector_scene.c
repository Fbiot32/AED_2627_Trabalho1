/**
 * @file vector_scene.c
 * @brief Implementation skeleton of the VectorScene ADT (2D Vector Scene)
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

#include "vector_scene.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"

#define TOLERANCE 0.5 // Distância máxima, em píxeis, a que um ponto ainda conta como estando sobre um segmento (≈ 1 píxel de espessura)
// ============================================================================
// CONCRETE STRUCT LAYOUT (OPAQUE ADT)
// ============================================================================

/**
 * @brief Concrete structure definition for VectorScene.
 * Hidden from vector_scene.h to enforce encapsulation.
 */
struct _VectorScene
{
  int width;              /**< Canvas width in pixels */
  int height;             /**< Canvas height in pixels */
  Color bg_color;         /**< Default background color */
  Primitive **primitives; /**< Dynamic array of pointers to Primitive */
  size_t count;           /**< Current number of primitives in scene */
  size_t capacity;        /**< Allocated capacity for primitives array */
  int next_auto_id;       /**< Auto-increment ID generator */
};

#define INITIAL_SCENE_CAPACITY 8

// ============================================================================
// HELPER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

/**
 * @brief Helper to write an RGB pixel buffer to binary PPM file (P6 format).
 */
static bool save_ppm_image(const char *filename, int width, int height, const uint8_t *pixel_buffer)
{
  if (!filename || !pixel_buffer || width <= 0 || height <= 0)
  {
    return false;
  }

  FILE *file = fopen(filename, "wb");
  if (!file)
  {
    return false;
  }

  fprintf(file, "P6\n%d %d\n255\n", width, height);

  size_t total_bytes = (size_t)width * height * 3;
  size_t written = fwrite(pixel_buffer, 1, total_bytes, file);
  fclose(file);

  return written == total_bytes;
}

// ============================================================================
// ACCESSORS / GETTERS (PROVIDED TO STUDENTS)
// ============================================================================

int vector_scene_get_width(const VectorScene *scene)
{
  if (!scene)
  {
    return 0;
  }
  return scene->width;
}

int vector_scene_get_height(const VectorScene *scene)
{
  if (!scene)
  {
    return 0;
  }
  return scene->height;
}

Color vector_scene_get_background_color(const VectorScene *scene)
{
  if (!scene)
  {
    Color black = {0, 0, 0};
    return black;
  }
  return scene->bg_color;
}

size_t vector_scene_get_count(const VectorScene *scene)
{
  if (!scene)
  {
    return 0;
  }
  return scene->count;
}

Primitive *vector_scene_get_primitive_by_index(const VectorScene *scene, size_t index)
{
  if (!scene || index >= scene->count)
  {
    return NULL;
  }
  return scene->primitives[index];
}

Primitive *vector_scene_get_primitive_by_id(const VectorScene *scene, int id)
{
  if (!scene || id < 0)
  {
    return NULL;
  }

  for (size_t i = 0; i < scene->count; i++)
  {
    if (scene->primitives[i] && scene->primitives[i]->id == id)
    {
      return scene->primitives[i];
    }
  }
  return NULL;
}

// ============================================================================
// SCENE LIFECYCLE (STUDENTS MUST COMPLETE)
// ============================================================================

VectorScene *vector_scene_create(int width, int height, Color bg_color)
{
  // Validate width and height
  if (width <= 0 || height <= 0)
  {
    return NULL;
  }

  // Try to allocate and initialize an empty scene
  VectorScene *scene = malloc(sizeof *scene);

  // Check if scene got space in memory
  if (scene == NULL)
  {
    return NULL;
  }

  // Write drawing settings
  scene->width = width;
  scene->height = height;
  scene->bg_color = bg_color;

  // Prep for shapes
  scene->count = 0; // No shapes (0)
  scene->capacity = INITIAL_SCENE_CAPACITY;
  scene->next_auto_id = 0; // Start the shape id as 0

  // Try to allocate space for the shapes
  scene->primitives = malloc(scene->capacity * sizeof *scene->primitives);

  // If it fails to allocate for the capacity of the shapes
  if (scene->primitives == NULL)
  {
    free(scene);
    return NULL;
  }

  return scene;
}

void vector_scene_destroy(VectorScene *scene)
{

  // Case where there's no scene to delete
  if (!scene)
  {
    return;
  }

  // Delete shapes one by one inside the list of shapes
  for (size_t i = 0; i < scene->count; i++)
  {
    primitive_destroy(scene->primitives[i]); // From primitive.c
  }

  free(scene->primitives);
  free(scene);
}

// ============================================================================
// COLLECTION MANAGEMENT (STUDENTS MUST COMPLETE)
// ============================================================================

int vector_scene_add_primitive(VectorScene *scene, Primitive *p)
{

  if (scene == NULL || p == NULL)
  {
    return -1;
  }

  // If the array is full, double the capacity
  if (scene->count == scene->capacity)
  {
    size_t new_capacity = scene->capacity * 2;
    Primitive **new_array = realloc(scene->primitives, new_capacity * sizeof *scene->primitives);

    if (new_array == NULL)
    {
      return -1;
    }

    scene->primitives = new_array;
    scene->capacity = new_capacity;
  }

  // A negative id asks the scene to assign an id
  if (p->id < 0)
  {
    p->id = scene->next_auto_id;
    scene->next_auto_id++;
  }
  else
  {
    if (p->id >= scene->next_auto_id)
    {
      scene->next_auto_id = p->id + 1;
    }
  }

  // Store the shape's address in the next available space
  scene->primitives[scene->count] = p;
  scene->count++;

  // Update bouding box
  primitive_update_aabb(p);

  return p->id;
}

bool vector_scene_remove_primitive(VectorScene *scene, int id)
{
  if (!scene || id < 0)
  {
    return false;
  }

  for (size_t i = 0; i < scene->count; i++)
  {
    if (scene->primitives[i]->id == id)
    {
      primitive_destroy(scene->primitives[i]);

      for (size_t j = i + 1; j < scene->count; j++)
      {
        scene->primitives[j - 1] = scene->primitives[j];
      }

      scene->count--;
      scene->primitives[scene->count] = NULL;
      return true;
    }
  }

  return false;
}

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_translate_primitive(VectorScene *scene, int id, double dx, double dy)
{
  if (!scene)
  {
    return false;
  }

  Primitive *p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL)
  {
    return false;
  }

  primitive_translate(p, dx, dy);
  primitive_update_aabb(p);

  return true;
}

bool vector_scene_set_z_index(VectorScene *scene, int id, int new_z_index)
{
  if (!scene)
  {
    return false;
  }

  Primitive *p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL)
  {
    return false;
  }

  p->z_index = new_z_index;
  return true;
}

// ============================================================================
// SPATIAL SELECTION AND SORTING (STUDENTS MUST COMPLETE & ANALYZE)
// ============================================================================

/**
 * @brief Sorts an array of primitives by z_index using O(N^2) Insertion Sort.
 *
 * Iterative in-place sorting algorithm. Iterates through the primitive array
 * and inserts each element into its correct sorted position based on z_index.
 * Increments comparison and swap/move metrics in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param count Number of elements in the array.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */

static void insertion_sort_by_z(Primitive **arr, size_t count, SortStats *stats)
{

  if (arr == NULL || count < 2)
  {
    return;
  }

  for (size_t i = 1; i < count; i++)
  {

    Primitive *key = arr[i];
    size_t j = i;

    while (j > 0)
    {
      if (stats != NULL)
      {
        stats->comparisons++;
      }

      if (arr[j - 1]->z_index <= key->z_index)
      {
        break;
      }

      arr[j] = arr[j - 1];
      if (stats != NULL)
      {
        stats->swaps++;
      }
      j--;
    }

    if (j != i)
    {
      arr[j] = key;

      if (stats != NULL)
      {
        stats->swaps++;
      }
    }
  }
}

/**
 * @brief Merges two sorted subarrays of primitive pointers by z_index.
 *
 * Helper function for MergeSort that combines [left..mid] and [mid+1..right]
 * using a temporary pointer array. Increments comparisons and swaps/moves
 * counters in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to merge.
 * @param temp_arr Auxiliary temporary array of Primitive pointers.
 * @param left Starting index of the first subarray.
 * @param mid Middle index separating the two subarrays.
 * @param right Ending index of the second subarray.
 * @param stats Pointer to SortStats structure to track metrics (can be NULL).
 */

static void merge_by_z(Primitive **arr, Primitive **temp_arr, size_t left, size_t mid, size_t right, SortStats *stats)
{
  size_t i = left;    // Next shape left half
  size_t j = mid + 1; // Next shape right half
  size_t k = left;    // Next free position temp_arr

  while (i <= mid && j <= right)
  {
    if (stats != NULL)
    {
      stats->comparisons++;
    }

    if (arr[i]->z_index <= arr[j]->z_index)
    {
      temp_arr[k++] = arr[i++];
    }
    else
    {
      temp_arr[k++] = arr[j++];
    }

    if (stats != NULL)
    {
      stats->swaps++;
    }
  }

  while (i <= mid)
  {
    temp_arr[k++] = arr[i++];
    if (stats != NULL)
    {
      stats->swaps++;
    }
  }

  while (j <= right)
  {
    temp_arr[k++] = arr[j++];
    if (stats != NULL)
    {
      stats->swaps++;
    }
  }

  for (size_t pos = left; pos <= right; pos++)
  {
    arr[pos] = temp_arr[pos];
    if (stats != NULL)
    {
      stats->swaps++;
    }
  }
}

/**
 * @brief Sorts a subarray of primitives by z_index using O(N log N) MergeSort.
 *
 * Stable recursive Divide-and-Conquer algorithm. Splits the array in half,
 * recursively sorts both halves, and invokes merge_by_z() to merge them.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param temp_arr Auxiliary temporary array allocated for merging.
 * @param left Starting index of the subarray to sort.
 * @param right Ending index of the subarray to sort.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */

static void merge_sort_by_z(Primitive **arr, Primitive **temp_arr, size_t left, size_t right, SortStats *stats)
{

  if (left >= right)
  {
    return;
  }

  size_t mid = left + (right - left) / 2;

  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid + 1, right, stats);
  merge_by_z(arr, temp_arr, left, mid, right, stats);
}

void vector_scene_sort_by_z(VectorScene *scene, bool use_fast_sort, SortStats *stats)
{

  if (stats != NULL)
  {
    stats->comparisons = 0;
    stats->swaps = 0;
  }

  if (scene == NULL || scene->count < 2)
  {
    return;
  }

  if (!use_fast_sort)
  {
    insertion_sort_by_z(scene->primitives, scene->count, stats);
    return;
  }

  Primitive **temp_arr = malloc(scene->count * sizeof *temp_arr);
  if (temp_arr == NULL)
  {
    return;
  }

  merge_sort_by_z(scene->primitives, temp_arr, 0, scene->count - 1, stats);
  free(temp_arr);
}

Primitive *vector_scene_pick_top_primitive(const VectorScene *scene, Point pt, bool is_opt, PickStats *stats)
{
  if (stats != NULL)
  {
    stats->aabb_tests = 0;
    stats->exact_tests = 0;
  }

  if (scene == NULL)
  {
    return NULL;
  }

  Primitive *top = NULL;

  for (size_t i = scene->count; i > 0; i--)
  {
    Primitive *p = scene->primitives[i - 1];

    if (p == NULL || p->alpha <= 0.0)
    {
      continue;
    }

    if (is_opt)
    {
      BoundingBox box = primitive_get_aabb(p);

      
      double margin = (p->type == PRIM_SEGMENT) ? TOLERANCE : 0.0;

      if (stats != NULL)
      {
        stats->aabb_tests++;
      }

      if (pt.x < box.min.x - margin || pt.x > box.max.x + margin ||
          pt.y < box.min.y - margin || pt.y > box.max.y + margin)
      {
        continue;
      }
    }

    if (stats != NULL)
    {
      stats->exact_tests++;
    }

    if (primitive_contains_point(p, pt) &&
        (top == NULL || p->z_index > top->z_index))
    {
      top = p;
    }
  }

  return top;
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_render(const VectorScene *scene, const char *output_filename)
{

  if (!scene || !output_filename)
  {
    return false;
  }

  if (scene == NULL || output_filename == NULL || scene->width <= 0 || scene->height <= 0)
  {
    return false;
  }

  size_t width = (size_t)scene->width;
  size_t height = (size_t)scene->height;

  if (width > SIZE_MAX / height / 3)
  {
    return false;
  }

  size_t pixel_count = width * height;
  uint8_t *pixels = malloc(pixel_count * 3);

  if (pixels == NULL)
  {
    return false;
  }

  // The canvas filled with the background color
  for (size_t i = 0; i < pixel_count; i++)
  {
    pixels[i * 3] = scene->bg_color.r;
    pixels[i * 3 + 1] = scene->bg_color.g;
    pixels[i * 3 + 2] = scene->bg_color.b;
  }

  Primitive **ordered = NULL;

  if (scene->count > 0)
  {

    if (scene->count > SIZE_MAX / sizeof *ordered)
    {
      free(pixels);
      return false;
    }

    ordered = malloc(scene->count * sizeof *ordered);
    if (ordered == NULL)
    {
      free(pixels);
      return false;
    }

    memcpy(ordered, scene->primitives, scene->count * sizeof *ordered);

    if (scene->count > 1)
    {
      Primitive **temp = malloc(scene->count * sizeof *temp);
      if (temp == NULL)
      {
        free(ordered);
        free(pixels);
        return false;
      }

      merge_sort_by_z(ordered, temp, 0, scene->count - 1, NULL);
      free(temp);
    }
  }

  // Draw the shapes over the pixels already in buffer
  for (size_t i = 0; i < scene->count; i++)
  {
    Primitive *p = ordered[i];

    if (p == NULL || !(p->alpha > 0.0) || p->alpha > 1.0)
    {
      continue;
    }

    BoundingBox box = primitive_get_aabb(p);
    double margin = (p->type == PRIM_SEGMENT) ? TOLERANCE : 0.0;
    double x_min = floor(box.min.x - margin);
    double x_max = ceil(box.max.x + margin);
    double y_min = floor(box.min.y - margin);
    double y_max = ceil(box.max.y + margin);

    // Skip a shape if its bounding box is outside the canvas
    if (!(x_min < (double)width && x_max >= 0.0 && y_min < (double)height && y_max >= 0.0))
    {
      continue;
    }

    size_t first_x = (size_t)fmax(0.0, x_min);
    size_t last_x = (size_t)fmin((double)(width - 1), x_max);
    size_t first_y = (size_t)fmax(0.0, y_min);
    size_t last_y = (size_t)fmin((double)(height - 1), y_max);

    for (size_t y = first_y; y <= last_y; y++)
    {
      for (size_t x = first_x; x <= last_x; x++)
      {
        Point center = {(double)x + 0.5, (double)y + 0.5};

        if (!primitive_contains_point(p, center))
        {
          continue;
        }

        size_t at = (y * width + x) * 3;
        double alpha = p->alpha;

        pixels[at] = (uint8_t)(alpha * p->color.r + (1.0 - alpha) * pixels[at] + 0.5);
        pixels[at + 1] = (uint8_t)(alpha * p->color.g + (1.0 - alpha) * pixels[at + 1] + 0.5);
        pixels[at + 2] = (uint8_t)(alpha * p->color.b + (1.0 - alpha) * pixels[at + 2] + 0.5);
      }
    }
  }

  bool saved = save_ppm_image(output_filename, scene->width, scene->height, pixels);

  free(ordered);
  free(pixels);
  return saved;
}

// ============================================================================
// FILE INPUT / OUTPUT (PROVIDED TO STUDENTS)
// ============================================================================

bool vector_scene_save_text(const VectorScene *scene, const char *filename)
{
  if (!scene || !filename)
  {
    return false;
  }

  FILE *file = fopen(filename, "w");
  if (!file)
  {
    return false;
  }

  Color bg = scene->bg_color;
  fprintf(file, "SCENE %d %d %d %d %d\n", scene->width, scene->height, bg.r,
          bg.g, bg.b);

  for (size_t i = 0; i < scene->count; i++)
  {
    Primitive *p = scene->primitives[i];
    if (!p)
    {
      continue;
    }

    Color c = p->color;

    switch (p->type)
    {
    case PRIM_CIRCLE:
      fprintf(file, "CIRC %d %d %d %d %d %.3f %.2f %.2f %.2f\n", p->id,
              p->z_index, c.r, c.g, c.b, p->alpha, p->geom.circle.center.x,
              p->geom.circle.center.y, p->geom.circle.radius);
      break;

    case PRIM_RECTANGLE:
      fprintf(file, "RECT %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
              p->z_index, c.r, c.g, c.b, p->alpha,
              p->geom.rectangle.top_left.x, p->geom.rectangle.top_left.y,
              p->geom.rectangle.width, p->geom.rectangle.height);
      break;

    case PRIM_SEGMENT:
      fprintf(file, "SEG %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
              p->z_index, c.r, c.g, c.b, p->alpha, p->geom.segment.p1.x,
              p->geom.segment.p1.y, p->geom.segment.p2.x,
              p->geom.segment.p2.y);
      break;

    case PRIM_POLYGON:
      fprintf(file, "POLY %d %d %d %d %d %.3f %zu", p->id, p->z_index, c.r,
              c.g, c.b, p->alpha, p->geom.polygon.num_vertices);

      for (size_t v = 0; v < p->geom.polygon.num_vertices; v++)
      {
        fprintf(file, " %.2f %.2f", p->geom.polygon.vertices[v].x,
                p->geom.polygon.vertices[v].y);
      }
      fprintf(file, "\n");
      break;
    }
  }

  fclose(file);
  return true;
}

VectorScene *vector_scene_load_text(const char *filename)
{
  if (!filename)
  {
    return NULL;
  }

  FILE *file = fopen(filename, "r");
  if (!file)
  {
    return NULL;
  }

  char line[1024];
  VectorScene *scene = NULL;

  while (fgets(line, sizeof(line), file))
  {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
    {
      continue;
    }

    char tag[16];
    if (sscanf(line, "%15s", tag) != 1)
    {
      continue;
    }

    if (strcmp(tag, "SCENE") == 0)
    {
      int w, h, r, g, b;
      if (sscanf(line, "SCENE %d %d %d %d %d", &w, &h, &r, &g, &b) == 5)
      {
        Color bg = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        scene = vector_scene_create(w, h, bg);
      }
    }
    else if (scene != NULL)
    {
      int id, z_index, r, g, b;
      double alpha;

      if (strcmp(tag, "CIRC") == 0)
      {
        double cx, cy, radius;
        if (sscanf(line, "CIRC %d %d %d %d %d %lf %lf %lf %lf", &id, &z_index,
                   &r, &g, &b, &alpha, &cx, &cy, &radius) == 9)
        {
          Point center = {cx, cy};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive *p = primitive_create_circle(id, center, radius, color,
                                                 alpha, z_index);
          if (p)
          {
            vector_scene_add_primitive(scene, p);
          }
        }
      }
      else if (strcmp(tag, "RECT") == 0)
      {
        double x, y, w, h;
        if (sscanf(line, "RECT %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x, &y, &w, &h) == 10)
        {
          Point top_left = {x, y};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive *p = primitive_create_rectangle(id, top_left, w, h, color,
                                                    alpha, z_index);
          if (p)
          {
            vector_scene_add_primitive(scene, p);
          }
        }
      }
      else if (strcmp(tag, "SEG") == 0)
      {
        double x1, y1, x2, y2;
        if (sscanf(line, "SEG %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x1, &y1, &x2, &y2) == 10)
        {
          Point p1 = {x1, y1};
          Point p2 = {x2, y2};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive *p =
              primitive_create_segment(id, p1, p2, color, alpha, z_index);
          if (p)
          {
            vector_scene_add_primitive(scene, p);
          }
        }
      }
      else if (strcmp(tag, "POLY") == 0)
      {
        size_t num_verts;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts);
        if (read_header == 7)
        {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive *p = primitive_create_polygon(id, color, alpha, z_index);
          if (p)
          {
            char *ptr = line;
            for (int k = 0; k < 7; k++)
            {
              while (*ptr && *ptr != ' ' && *ptr != '\t')
              {
                ptr++;
              }
              while (*ptr && (*ptr == ' ' || *ptr == '\t'))
              {
                ptr++;
              }
            }

            for (size_t v = 0; v < num_verts; v++)
            {
              double vx, vy;
              if (sscanf(ptr, "%lf %lf", &vx, &vy) == 2)
              {
                Point pt = {vx, vy};
                polygon_add_vertex(p, pt);

                for (int k = 0; k < 2; k++)
                {
                  while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' &&
                         *ptr != '\r')
                  {
                    ptr++;
                  }
                  while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' ||
                                  *ptr == '\r'))
                  {
                    ptr++;
                  }
                }
              }
            }
            vector_scene_add_primitive(scene, p);
          }
        }
      }
    }
  }

  fclose(file);
  return scene;
}
