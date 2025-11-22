#include "gui.h"
#include "scenes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// Global app pointer for callbacks
static GuiApp* g_app = NULL;

// Structure for passing render completion data to main thread
typedef struct {
    GuiApp* app;
    char status_text[256];
    double render_time;
} RenderCompleteData;

// Progress callback for pathtracer
void render_progress_callback(float progress) {
    if (g_app) {
        pthread_mutex_lock(&g_app->render_mutex);
        g_app->render_progress = progress;
        pthread_mutex_unlock(&g_app->render_mutex);
    }
}

// GUI update function called on main thread after render completes
gboolean render_complete_update_gui(gpointer user_data) {
    RenderCompleteData* data = (RenderCompleteData*)user_data;
    GuiApp* app = data->app;

    // Update all GUI elements on main thread
    gtk_label_set_text(GTK_LABEL(app->status_label), data->status_text);
    gtk_button_set_label(GTK_BUTTON(app->render_button), "Start Render");
    gtk_widget_set_sensitive(app->render_button, TRUE);
    gtk_widget_set_sensitive(app->save_button, TRUE);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 1.0);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress_bar), "100%");

    // Update image display
    update_image_display(app);

    // Free the data
    free(data);

    // Return FALSE to remove this idle handler
    return FALSE;
}

// Create GUI application
GuiApp* gui_app_create(void) {
    GuiApp* app = (GuiApp*)calloc(1, sizeof(GuiApp));
    g_app = app;

    // Initialize GTK
    gtk_init(NULL, NULL);

    // Initialize mutex
    pthread_mutex_init(&app->render_mutex, NULL);

    // Create main window
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "C Path Tracer - High Performance Rendering");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 1200, 800);
    gtk_container_set_border_width(GTK_CONTAINER(app->window), 10);

    // Main horizontal box
    app->main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_add(GTK_CONTAINER(app->window), app->main_box);

    // Create control panel (left side)
    GtkWidget* control_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(app->main_box), control_box, FALSE, FALSE, 0);

    // Control frame
    app->control_frame = gtk_frame_new("Render Settings");
    gtk_box_pack_start(GTK_BOX(control_box), app->control_frame, FALSE, FALSE, 0);

    GtkWidget* control_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(control_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(control_grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(control_grid), 10);
    gtk_container_add(GTK_CONTAINER(app->control_frame), control_grid);

    int row = 0;

    // Scene selection
    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Scene:"), 0, row, 1, 1);
    app->scene_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Cornell Box");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Random Spheres");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Glass Spheres");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Metal Spheres");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Studio Lighting");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->scene_combo), "Material Blending");
    gtk_combo_box_set_active(GTK_COMBO_BOX(app->scene_combo), 0);
    gtk_grid_attach(GTK_GRID(control_grid), app->scene_combo, 1, row++, 1, 1);

    // Resolution controls
    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Width:"), 0, row, 1, 1);
    app->width_spin = gtk_spin_button_new_with_range(100, 1920, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->width_spin), 800);
    gtk_grid_attach(GTK_GRID(control_grid), app->width_spin, 1, row++, 1, 1);

    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Height:"), 0, row, 1, 1);
    app->height_spin = gtk_spin_button_new_with_range(100, 1080, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->height_spin), 600);
    gtk_grid_attach(GTK_GRID(control_grid), app->height_spin, 1, row++, 1, 1);

    // Quality controls
    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Samples:"), 0, row, 1, 1);
    app->samples_spin = gtk_spin_button_new_with_range(1, 10000, 10);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->samples_spin), 100);
    gtk_grid_attach(GTK_GRID(control_grid), app->samples_spin, 1, row++, 1, 1);

    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Max Depth:"), 0, row, 1, 1);
    app->depth_spin = gtk_spin_button_new_with_range(1, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->depth_spin), 50);
    gtk_grid_attach(GTK_GRID(control_grid), app->depth_spin, 1, row++, 1, 1);

    gtk_grid_attach(GTK_GRID(control_grid), gtk_label_new("Threads:"), 0, row, 1, 1);
    app->threads_spin = gtk_spin_button_new_with_range(1, 32, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app->threads_spin), 8);
    gtk_grid_attach(GTK_GRID(control_grid), app->threads_spin, 1, row++, 1, 1);

    // Separator
    gtk_grid_attach(GTK_GRID(control_grid), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), 0, row++, 2, 1);

    // Render button
    app->render_button = gtk_button_new_with_label("Start Render");
    gtk_widget_set_size_request(app->render_button, 200, 40);
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button { background: #4CAF50; color: white; font-weight: bold; }",
        -1, NULL);
    GtkStyleContext* context = gtk_widget_get_style_context(app->render_button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_grid_attach(GTK_GRID(control_grid), app->render_button, 0, row++, 2, 1);

    // Progress bar
    app->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app->progress_bar), TRUE);
    gtk_grid_attach(GTK_GRID(control_grid), app->progress_bar, 0, row++, 2, 1);

    // Status label
    app->status_label = gtk_label_new("Ready");
    gtk_grid_attach(GTK_GRID(control_grid), app->status_label, 0, row++, 2, 1);

    // Save button
    app->save_button = gtk_button_new_with_label("Save Image");
    gtk_widget_set_sensitive(app->save_button, FALSE);
    gtk_grid_attach(GTK_GRID(control_grid), app->save_button, 0, row++, 2, 1);

    // Add some padding
    gtk_box_pack_start(GTK_BOX(control_box), gtk_label_new(""), TRUE, TRUE, 0);

    // Image display area (right side)
    GtkWidget* image_frame = gtk_frame_new("Rendered Image");
    gtk_box_pack_start(GTK_BOX(app->main_box), image_frame, TRUE, TRUE, 0);

    app->image_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(app->image_scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(image_frame), app->image_scroll);

    app->image_widget = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(app->image_scroll), app->image_widget);

    // Connect signals
    g_signal_connect(app->window, "destroy", G_CALLBACK(on_window_destroy), app);
    g_signal_connect(app->render_button, "clicked", G_CALLBACK(on_render_clicked), app);
    g_signal_connect(app->save_button, "clicked", G_CALLBACK(on_save_clicked), app);
    g_signal_connect(app->scene_combo, "changed", G_CALLBACK(on_scene_changed), app);

    // Initialize render settings
    app->settings.width = 800;
    app->settings.height = 600;
    app->settings.samples_per_pixel = 100;
    app->settings.max_depth = 50;
    app->settings.num_threads = 8;
    app->settings.use_bvh = true;
    app->settings.use_nee = false;

    // Set progress callback
    set_progress_callback(render_progress_callback);

    return app;
}

// Destroy GUI application
void gui_app_destroy(GuiApp* app) {
    if (app) {
        if (app->scene) scene_destroy(app->scene);
        if (app->camera) free(app->camera);
        if (app->render_image) image_destroy(app->render_image);
        if (app->display_pixbuf) g_object_unref(app->display_pixbuf);
        pthread_mutex_destroy(&app->render_mutex);
        free(app);
        g_app = NULL;
    }
}

// Run GUI application
void gui_app_run(GuiApp* app) {
    gtk_widget_show_all(app->window);
    gtk_main();
}

// Convert Image to GdkPixbuf for display
GdkPixbuf* image_to_pixbuf(const Image* img) {
    if (!img || !img->pixels) return NULL;

    GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
                                        img->width, img->height);
    if (!pixbuf) return NULL;

    guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);

    for (uint32_t y = 0; y < img->height; y++) {
        for (uint32_t x = 0; x < img->width; x++) {
            Vec3 color = img->pixels[y * img->width + x];

            // Apply ACES tone mapping
            color = aces_tonemap(color);

            // Clamp values
            color.x = fminf(fmaxf(color.x, 0.0f), 1.0f);
            color.y = fminf(fmaxf(color.y, 0.0f), 1.0f);
            color.z = fminf(fmaxf(color.z, 0.0f), 1.0f);

            // Convert to 8-bit RGB
            guchar* p = pixels + y * rowstride + x * 3;
            p[0] = (guchar)(color.x * 255.99f);
            p[1] = (guchar)(color.y * 255.99f);
            p[2] = (guchar)(color.z * 255.99f);
        }
    }

    return pixbuf;
}

// Update image display
void update_image_display(GuiApp* app) {
    if (app->render_image) {
        if (app->display_pixbuf) {
            g_object_unref(app->display_pixbuf);
            app->display_pixbuf = NULL;
        }
        app->display_pixbuf = image_to_pixbuf(app->render_image);
        if (app->display_pixbuf) {
            gtk_image_set_from_pixbuf(GTK_IMAGE(app->image_widget), app->display_pixbuf);
        }
    }
}

// Create scene based on selection
static Scene* create_scene(const char* name) {
    if (strcmp(name, "Cornell Box") == 0) {
        return create_cornell_box();
    } else if (strcmp(name, "Random Spheres") == 0) {
        return create_random_spheres();
    } else if (strcmp(name, "Glass Spheres") == 0) {
        return create_glass_spheres();
    } else if (strcmp(name, "Metal Spheres") == 0) {
        return create_metal_spheres();
    } else if (strcmp(name, "Studio Lighting") == 0) {
        return create_studio_lighting();
    } else if (strcmp(name, "Material Blending") == 0) {
        return create_material_blend();
    }

    return create_cornell_box();
}

// Create camera for scene
static Camera* create_camera_for_scene(const char* name, float aspect) {
    Camera* cam = (Camera*)malloc(sizeof(Camera));

    if (strcmp(name, "Cornell Box") == 0) {
        *cam = camera_create(
            vec3_create(278, 278, -800),
            vec3_create(278, 278, 0),
            vec3_create(0, 1, 0),
            40.0f, aspect, 0.0f, 10.0f
        );
    } else if (strcmp(name, "Random Spheres") == 0) {
        // Wide angle view to capture the random field with hero spheres
        *cam = camera_create(
            vec3_create(13, 2, 3),
            vec3_create(0, 0.5f, 0),
            vec3_create(0, 1, 0),
            20.0f, aspect, 0.1f, 10.0f
        );
    } else if (strcmp(name, "Glass Spheres") == 0) {
        // Elevated view to see the 7x7 grid pattern
        *cam = camera_create(
            vec3_create(-8, 6, 8),
            vec3_create(0, 1, 0),
            vec3_create(0, 1, 0),
            45.0f, aspect, 0.0f, 15.0f
        );
    } else if (strcmp(name, "Metal Spheres") == 0) {
        // Side view to showcase the metallic lineup and reflections
        *cam = camera_create(
            vec3_create(0, 2.5f, -10),
            vec3_create(0, 1, 0),
            vec3_create(0, 1, 0),
            50.0f, aspect, 0.0f, 10.0f
        );
    } else if (strcmp(name, "Studio Lighting") == 0) {
        *cam = camera_create(
            vec3_create(0, 2, 8),
            vec3_create(0, 1, -2),
            vec3_create(0, 1, 0),
            40.0f, aspect, 0.05f, 10.0f
        );
    } else if (strcmp(name, "Material Blending") == 0) {
        *cam = camera_create(
            vec3_create(0, 2, 10),
            vec3_create(0, 1, 0),
            vec3_create(0, 1, 0),
            45.0f, aspect, 0.1f, 12.0f
        );
    } else {
        // Default camera for any future scenes
        *cam = camera_create(
            vec3_create(13, 2, 3),
            vec3_create(0, 0, 0),
            vec3_create(0, 1, 0),
            20.0f, aspect, 0.1f, 10.0f
        );
    }

    return cam;
}

// Rendering thread function
void* render_thread_func(void* user_data) {
    GuiApp* app = (GuiApp*)user_data;

    // Get settings from GUI
    pthread_mutex_lock(&app->render_mutex);
    RenderSettings settings = app->settings;
    pthread_mutex_unlock(&app->render_mutex);

    // Set cancel flag pointer for render cancellation
    settings.cancel_flag = &app->cancel_render;

    // Create or resize image
    if (app->render_image) {
        if (app->render_image->width != settings.width ||
            app->render_image->height != settings.height) {
            image_destroy(app->render_image);
            app->render_image = image_create(settings.width, settings.height);
        }
    } else {
        app->render_image = image_create(settings.width, settings.height);
    }

    // Start timer (wall clock time, not CPU time)
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // Render - this is where progress callbacks happen
    render_parallel(app->scene, app->camera, &settings, app->render_image);

    // Calculate render time (wall clock)
    gettimeofday(&end_time, NULL);
    double render_time = (end_time.tv_sec - start_time.tv_sec) +
                        (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    // Update status
    pthread_mutex_lock(&app->render_mutex);
    app->is_rendering = false;
    pthread_mutex_unlock(&app->render_mutex);

    // Prepare data for GUI update
    RenderCompleteData* data = (RenderCompleteData*)malloc(sizeof(RenderCompleteData));
    data->app = app;
    data->render_time = render_time;

    // Check if render was cancelled
    if (app->cancel_render) {
        snprintf(data->status_text, sizeof(data->status_text),
                 "Render cancelled after %.2f seconds", render_time);
    } else {
        snprintf(data->status_text, sizeof(data->status_text),
                 "Render complete: %.2f seconds (%.2f Mrays/s)",
                 render_time,
                 (settings.width * settings.height * settings.samples_per_pixel) / (render_time * 1e6));
    }

    // Schedule GUI update on main thread
    g_idle_add(render_complete_update_gui, data);

    return NULL;
}

// Update progress callback for GTK
gboolean update_progress(gpointer user_data) {
    GuiApp* app = (GuiApp*)user_data;

    pthread_mutex_lock(&app->render_mutex);
    float progress = app->render_progress;
    bool is_rendering = app->is_rendering;
    pthread_mutex_unlock(&app->render_mutex);

    if (is_rendering) {
        // Clamp progress to avoid any overflow issues
        progress = fminf(progress, 1.0f);
        progress = fmaxf(progress, 0.0f);

        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), progress);

        char progress_text[64];
        snprintf(progress_text, sizeof(progress_text), "%d%%", (int)(progress * 100));
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app->progress_bar), progress_text);

        // Continue updating
        return TRUE;
    }

    // Stop updating
    return FALSE;
}

// Callbacks
void on_render_clicked(GtkButton* button, gpointer user_data) {
    GuiApp* app = (GuiApp*)user_data;

    pthread_mutex_lock(&app->render_mutex);
    bool is_rendering = app->is_rendering;
    pthread_mutex_unlock(&app->render_mutex);

    if (is_rendering) {
        // Cancel render
        pthread_mutex_lock(&app->render_mutex);
        app->cancel_render = true;
        pthread_mutex_unlock(&app->render_mutex);
        return;
    }

    // Update settings from GUI
    app->settings.width = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->width_spin));
    app->settings.height = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->height_spin));
    app->settings.samples_per_pixel = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->samples_spin));
    app->settings.max_depth = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->depth_spin));
    app->settings.num_threads = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app->threads_spin));

    // Get scene name
    const char* scene_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(app->scene_combo));
    if (!scene_name) scene_name = "Cornell Box";

    // Create scene and camera
    if (app->scene) scene_destroy(app->scene);
    app->scene = create_scene(scene_name);

    // Build BVH
    gtk_label_set_text(GTK_LABEL(app->status_label), "Building BVH...");
    scene_build_bvh(app->scene);

    // Create camera
    if (app->camera) free(app->camera);
    float aspect = (float)app->settings.width / app->settings.height;
    app->camera = create_camera_for_scene(scene_name, aspect);

    // Start rendering
    pthread_mutex_lock(&app->render_mutex);
    app->is_rendering = true;
    app->cancel_render = false;
    app->render_progress = 0.0f;
    pthread_mutex_unlock(&app->render_mutex);

    gtk_button_set_label(GTK_BUTTON(button), "Cancel Render");
    gtk_widget_set_sensitive(app->save_button, FALSE);
    gtk_label_set_text(GTK_LABEL(app->status_label), "Rendering...");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app->progress_bar), 0.0);

    // Start render thread
    pthread_create(&app->render_thread, NULL, render_thread_func, app);
    pthread_detach(app->render_thread);

    // Start progress update timer
    g_timeout_add(100, update_progress, app);
}

void on_save_clicked(GtkButton* button, gpointer user_data) {
    (void)button; // Suppress unused parameter warning
    GuiApp* app = (GuiApp*)user_data;

    if (!app->render_image) return;

    // Create save dialog
    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Save Image",
        GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL
    );

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    // Set default filename
    char default_name[256];
    time_t t = time(NULL);
    const struct tm* tm = localtime(&t);
    strftime(default_name, sizeof(default_name), "render_%Y%m%d_%H%M%S.bmp", tm);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), default_name);

    // Add filters
    GtkFileFilter* filter_bmp = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_bmp, "BMP Image (*.bmp)");
    gtk_file_filter_add_pattern(filter_bmp, "*.bmp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_bmp);

    GtkFileFilter* filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, "All Files");
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_all);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        // Save image
        image_save_bmp(app->render_image, filename);

        // Update status
        char status_text[512];
        snprintf(status_text, sizeof(status_text), "Image saved to: %s", filename);
        gtk_label_set_text(GTK_LABEL(app->status_label), status_text);

        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void on_scene_changed(GtkComboBox* combo, gpointer user_data) {
    (void)combo; // Suppress unused parameter warning
    (void)user_data; // Suppress unused parameter warning
    // Scene will be recreated when render is clicked
}

void on_window_destroy(GtkWidget* widget, gpointer user_data) {
    (void)widget; // Suppress unused parameter warning
    GuiApp* app = (GuiApp*)user_data;

    // Stop rendering if in progress
    pthread_mutex_lock(&app->render_mutex);
    if (app->is_rendering) {
        app->cancel_render = true;
    }
    pthread_mutex_unlock(&app->render_mutex);

    gtk_main_quit();
}