/**
 * @file demo_app.c
 * @brief Application Program Demonstrating VectorScene and Primitive ADTs
 *
 * Algoritmos e Estruturas de Dados (2026/2027)
 * Departamento de Electrónica, Telecomunicações e Informática
 * Universidade de Aveiro
 *
 * This program illustrates the core functionalities of the 2D Vector Engine:
 * 1. Programmatic creation of a 2D vector scene with diverse shapes.
 * 2. Saving the vector scene to a text file (.vsc).
 * 3. Raster rendering of the scene to a PPM image file with alpha blending.
 * 4. Loading a scene from a text file (.vsc).
 * 5. Spatial selection (picking), transformations, z-index updates, and
 * sorting.
 * 6. Saving and rendering the modified scene to new output files.
 */

#include <stdio.h>
#include <stdlib.h>

#include "primitive.h"
#include "primitive_internal.h"
#include "vector_scene.h"

// ============================================================================
// HELPER PRINT FUNCTIONS
// ============================================================================

static void print_banner(const char *title)
{
	printf(
			"\n===================================================================="
			"\n");
	printf(" %s\n", title);
	printf(
			"===================================================================="
			"\n");
}

static void print_primitive_info(const Primitive *p)
{
	if (p == NULL)
	{
		printf("  [Primitive] NULL\n");
		return;
	}

	const char *type_names[] = {"Circle", "Rectangle", "Segment", "Polygon"};
	Color rgb = primitive_get_color(p);
	printf(
			"  [Primitive ID %d] Type: %s | Z-Index: %d | Color: (%d, %d, %d) | "
			"Alpha: %.2f\n",
			primitive_get_id(p), type_names[primitive_get_type(p)],
			primitive_get_z_index(p), rgb.r, rgb.g, rgb.b, primitive_get_alpha(p));
}

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(void)
{
	print_banner("1. PROGRAMMATIC SCENE CREATION");

	// Canvas 800x600 with dark sky blue background
	Color bg_sky = {10, 10, 40};
	VectorScene *scene = vector_scene_create(800, 600, bg_sky);

	if (scene == NULL)
	{
		fprintf(stderr, "Error: Failed to create vector scene.\n");
		return EXIT_FAILURE;
	}

	printf("Created canvas: %dx%d pixels with background RGB(%d, %d, %d)\n",
				 vector_scene_get_width(scene), vector_scene_get_height(scene),
				 bg_sky.r, bg_sky.g, bg_sky.b);

	// ====================================
	// 1. Green Grass Ground (Rectangle)
	// ====================================

	Color green_grass = {10, 100, 34};
	Point ground_top_left = {0.0, 400.0};
	Primitive *ground = primitive_create_rectangle(
			-1,
			ground_top_left,
			800.0,
			200.0,
			green_grass,
			1.0,
			1);
	vector_scene_add_primitive(scene, ground);

	// ====================================
	// 2. Grey moon (Circle)
	// ====================================

	Color grey_moon = {220, 220, 220};
	Point moon_center = {680.0, 120.0};
	Primitive *moon = primitive_create_circle(
			-1,
			moon_center,
			65.0,
			grey_moon,
			0.9,
			2);
	vector_scene_add_primitive(scene, moon);

	// Moon Cover
	Point moon_cover_center = {700, 100};
	Primitive *moon_over = primitive_create_circle(
			-1,
			moon_cover_center,
			65,
			bg_sky,
			0.95,
			4);
	vector_scene_add_primitive(scene, moon_over);

	// ==================================
	// 3. Moon craters
	// ==================================

	Color dark_crater = {150, 150, 150};

	// 1
	Point crater_1_center = {624.0, 116.0};
	Primitive *crater_1 = primitive_create_circle(
			-1,
			crater_1_center,
			5.0,
			dark_crater,
			1,
			3);
	vector_scene_add_primitive(scene, crater_1);

	// 2
	Point crater_2_center = {640.0, 128.0};
	Primitive *crater_2 = primitive_create_circle(
			-1,
			crater_2_center,
			10.0,
			dark_crater,
			1,
			3);
	vector_scene_add_primitive(scene, crater_2);

	// 3
	Point crater_3_center = {670.0, 170.0};
	Primitive *crater_3 = primitive_create_circle(
			-1,
			crater_3_center,
			8.0,
			dark_crater,
			1,
			3);
	vector_scene_add_primitive(scene, crater_3);

	// ==================================
	// 4. House Body (Beige Rectangle)
	// ==================================

	// Main
	Color house_color = {165, 163, 145};
	Primitive *house = primitive_create_rectangle(
			-1,
			(Point){300.0, 320.0},
			200.0,
			150.0,
			house_color,
			1.0,
			2);
	int house_id = vector_scene_add_primitive(scene, house);

	// Garage
	Primitive *garage = primitive_create_rectangle(
			-1,
			(Point){500.0, 380.0},
			100.0, 90.0,
			house_color,
			1.0,
			2);
	vector_scene_add_primitive(scene, garage);

	// ==================================
	// 6. Roofs
	// ==================================

	Color roof_color = {178, 34, 34};

	// House Roof (Red Triangular Polygon)
	Primitive *house_roof = primitive_create_polygon(
			-1,
			roof_color,
			1.0,
			2);
	polygon_add_vertex(house_roof, (Point){280.0, 320.0});
	polygon_add_vertex(house_roof, (Point){520.0, 320.0});
	polygon_add_vertex(house_roof, (Point){400.0, 220.0});
	int roof_id = vector_scene_add_primitive(scene, house_roof);

	// =====================================
	// 7. House Doors and Window
	// =====================================

	// Main Door
	Color main_door_color = {139, 69, 19};
	Primitive *main_door = primitive_create_rectangle(
			-1,
			(Point){380.0, 410.0},
			40.0,
			60.0,
			main_door_color,
			1.0,
			3);
	vector_scene_add_primitive(scene, main_door);

	// Main Door Handle
	Color main_door_handle_color = {100, 20, 10};
	Primitive *main_door_handle = primitive_create_circle(
			-1,
			(Point){410, 445},
			3,
			main_door_handle_color,
			1.0,
			4);
	vector_scene_add_primitive(scene, main_door_handle);

	// Garage Door
	Color garage_door_color = {40, 40, 40};
	Primitive *garage_door = primitive_create_rectangle(
			-1,
			(Point){510.0, 410.0},
			80,
			60,
			garage_door_color, 1.0,
			3);
	vector_scene_add_primitive(scene, garage_door);

	// Garage Door Handle
	Color garage_door_handle_color = {20, 20, 20};
	Primitive *garage_door_handle = primitive_create_circle(
			-1,
			(Point){550, 465},
			3,
			garage_door_handle_color,
			1.0,
			4);
	vector_scene_add_primitive(scene, garage_door_handle);

	// Windows
	Color window_color = {135, 206, 250};
	Color window_div_color = {220, 220, 220};

	// Window left
	Primitive *window_left = primitive_create_rectangle(
			-1,
			(Point){330.0, 350.0},
			40.0,
			40.0,
			window_color,
			0.75,
			3);
	vector_scene_add_primitive(scene, window_left);

	// Window left division
	Primitive *window_left_div = primitive_create_segment(
			-1,
			(Point){350, 350},
			(Point){350, 390},
			window_div_color,
			1.0,
			4);
	vector_scene_add_primitive(scene, window_left_div);

	// Window right
	Primitive *window_right = primitive_create_rectangle(
			-1,
			(Point){430.0, 350.0},
			40.0,
			40.0,
			window_color,
			0.75,
			3);
	vector_scene_add_primitive(scene, window_right);

	// Window right division
	Primitive *window_right_div = primitive_create_segment(
			-1,
			(Point){450, 350},
			(Point){450, 390},
			window_div_color,
			1.0,
			4);
	vector_scene_add_primitive(scene, window_right_div);
	printf("Added %zu primitives to scene.\n", vector_scene_get_count(scene));

	print_banner("2. SAVING SCENE TO TEXT FILE AND RASTERING TO PPM IMAGE");

	// info: if we want to change name
	const char *created_text_file = "created_scene.vsc";
	const char *created_image_file = "created_scene.ppm";

	if (vector_scene_save_text(scene, created_text_file))
	{
		printf("Successfully saved scene configuration to text file: '%s'\n",
					 created_text_file);
	}
	else
	{
		fprintf(stderr, "Error: Failed to save scene text file.\n");
	}

	if (vector_scene_render(scene, created_image_file))
	{
		printf("Successfully rendered raster image file: '%s' (PPM format)\n",
					 created_image_file);
	}
	else
	{
		fprintf(stderr, "Error: Failed to render raster image file.\n");
	}

	// Destroy created scene to free memory
	vector_scene_destroy(scene);
	scene = NULL;
	printf("Freed initial scene memory.\n");

	print_banner("3. LOADING SCENE FROM FILE AND MANIPULATING OBJECTS");

	// Load scene from file
	VectorScene *loaded_scene = vector_scene_load_text(created_text_file);
	if (loaded_scene == NULL)
	{
		fprintf(stderr, "Error: Failed to load scene from file '%s'.\n",
						created_text_file);
		return EXIT_FAILURE;
	}

	printf("Loaded scene from '%s': Canvas %dx%d, Total Primitives: %zu\n",
				 created_text_file, vector_scene_get_width(loaded_scene),
				 vector_scene_get_height(loaded_scene),
				 vector_scene_get_count(loaded_scene));

	// Demonstration of Spatial Selection (Picking)
	print_banner("4. SPATIAL SELECTION (PICKING)");

	Point pick_pt1 = {400.0, 250.0}; // Point inside the roof
	PickStats pick_stats = {0, 0};
	Primitive *picked1 = vector_scene_pick_top_primitive(loaded_scene, pick_pt1,
																											 true, &pick_stats);

	printf("Testing pick at Point (%.1f, %.1f):\n", pick_pt1.x, pick_pt1.y);
	print_primitive_info(picked1);
	printf(
			"  -> Performance Stats: AABB Broad-Phase Tests: %lu | Exact "
			"Narrow-Phase Tests: %lu\n",
			pick_stats.aabb_tests, pick_stats.exact_tests);

	Point pick_pt2 = {680.0, 120.0}; // Point inside the sun
	Primitive *picked2 = vector_scene_pick_top_primitive(loaded_scene, pick_pt2,
																											 true, &pick_stats);

	printf("\nTesting pick at Point (%.1f, %.1f):\n", pick_pt2.x, pick_pt2.y);
	print_primitive_info(picked2);
	printf(
			"  -> Performance Stats: AABB Broad-Phase Tests: %lu | Exact "
			"Narrow-Phase Tests: %lu\n",
			pick_stats.aabb_tests, pick_stats.exact_tests);

	// Demonstration of Transformations and Updates
	print_banner("5. TRANSFORMATIONS, Z-INDEX UPDATES AND SORTING");

	// Translate roof ID
	if (vector_scene_translate_primitive(loaded_scene, roof_id, 0.0, -20.0))
	{
		printf("Translated roof (ID %d) by dy = -20.0 pixels.\n", roof_id);
	}

	// Change house Z-Index
	if (vector_scene_set_z_index(loaded_scene, house_id - 1, 10))
	{
		printf("Updated house body (ID 9) z-index to 8.\n");
	}

	// Sort by Z-Index using fast sort (QuickSort)
	SortStats sort_stats = {0, 0};
	vector_scene_sort_by_z(loaded_scene, true, &sort_stats);
	printf("Sorted primitives by Z-Index (QuickSort):\n");
	printf("  -> Comparisons: %lu | Swaps/Moves: %lu\n", sort_stats.comparisons,
				 sort_stats.swaps);

	print_banner("6. SAVING AND RENDERING MODIFIED SCENE");

	const char *mod_text_file = "modified_scene.vsc";
	const char *mod_image_file = "modified_scene.ppm";

	if (vector_scene_save_text(loaded_scene, mod_text_file))
	{
		printf(
				"Successfully saved modified scene configuration to text file: '%s'\n",
				mod_text_file);
	}

	if (vector_scene_render(loaded_scene, mod_image_file))
	{
		printf("Successfully rendered modified raster image file: '%s'\n",
					 mod_image_file);
	}

	// Clean up loaded scene
	vector_scene_destroy(loaded_scene);
	printf("Freed loaded scene memory.\n");

	print_banner("DEMO COMPLETED SUCCESSFULLY!");
	return EXIT_SUCCESS;
}
