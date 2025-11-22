#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include <pthread.h>
#include "pathtracer.h"

// GUI Application structure
typedef struct {
    // Main window
    GtkWidget* window;
    GtkWidget* main_box;

    // Control panel
    GtkWidget* control_frame;
    GtkWidget* width_spin;
    GtkWidget* height_spin;
    GtkWidget* samples_spin;
    GtkWidget* depth_spin;
    GtkWidget* threads_spin;
    GtkWidget* scene_combo;
    GtkWidget* render_button;
    GtkWidget* save_button;
    GtkWidget* progress_bar;
    GtkWidget* status_label;

    // Image display
    GtkWidget* image_scroll;
    GtkWidget* image_widget;

    // Rendering state
    Scene* scene;
    Camera* camera;
    Image* render_image;
    GdkPixbuf* display_pixbuf;

    // Threading
    pthread_t render_thread;
    pthread_mutex_t render_mutex;
    volatile bool is_rendering;
    volatile bool cancel_render;
    volatile float render_progress;

    // Settings
    RenderSettings settings;
    char* current_scene_name;
} GuiApp;

// GUI functions
GuiApp* gui_app_create(void);
void gui_app_destroy(GuiApp* app);
void gui_app_run(GuiApp* app);

// Callbacks
void on_render_clicked(GtkButton* button, gpointer user_data);
void on_save_clicked(GtkButton* button, gpointer user_data);
void on_scene_changed(GtkComboBox* combo, gpointer user_data);
void on_window_destroy(GtkWidget* widget, gpointer user_data);

// Rendering thread
void* render_thread_func(void* user_data);
gboolean update_progress(gpointer user_data);
void render_progress_callback(float progress);

// Image display
void update_image_display(GuiApp* app);
GdkPixbuf* image_to_pixbuf(const Image* img);

#endif // GUI_H