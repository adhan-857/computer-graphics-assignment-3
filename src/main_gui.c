#include <stdio.h>
#include "gui.h"

int main() {
    printf("============================================================\n");
    printf("C PathTracer GUI - High Performance Rendering\n");
    printf("============================================================\n");
    printf("Starting GUI interface...\n\n");

    // Create and run GUI application
    GuiApp* app = gui_app_create();
    gui_app_run(app);
    gui_app_destroy(app);

    printf("\nGUI closed.\n");
    return 0;
}