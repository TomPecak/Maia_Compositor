
#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>

#include <wlr/types/wlr_subcompositor.h>

#include <wlr/backend.h>
#include <wlr/util/log.h>

struct tinywl_server {
  struct wl_display* wl_display;
  struct wlr_backend* backend;
  struct wlr_renderer* renderer;
  struct wlr_allocator* allocator;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
};

int main(int argc, char *argv[]) {
  wlr_log_init(WLR_DEBUG, NULL);
  char *startup_cmd = NULL;

  int c;
  while ((c = getopt(argc, argv, "s:h")) != -1) {
    switch (c) {
      case 's':
        startup_cmd = optarg;
        break;
      default:
        printf("Usage: %s [-s startup command]\n", argv[0]);
        break;
    }
  }
  if (optind < argc) {
    printf("Usage: %s [-s startup command]\n", argv[0]);
    return 0;
  }

  struct tinywl_server server = {0};

  server.wl_display = wl_display_create();
  if (server.wl_display == NULL) {
    wlr_log(WLR_ERROR, "Failed to create wayland display");
    return 1;
  }

  server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
  if (server.backend == NULL) {
    wlr_log(WLR_ERROR, "failed to create wlr_backend");
    return 1;
  }

  server.renderer = wlr_renderer_autocreate(server.backend);
  if(server.renderer == NULL){
    wlr_log(WLR_ERROR, "failed to create wlr_renderer");
    return 1;
  }

  wlr_renderer_init_wl_display(server.renderer, server.wl_display);

  //CREATE GBM BUFFERS
  server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
  if(server.allocator == NULL){
    wlr_log(WLR_ERROR, "failed to create wlr_allocator");
    return 1;
  }

  // INITIALIZE WAYLAND CORE GLOBALS

  // CORE COMPOSITOR: HANDLES SURFACES AND BINDS GRAPHICS RENDERER
  wlr_compositor_create(server.wl_display, 5, server.renderer);

  // SUBCOMPOSITOR: ALLOWS NESTED SURFACES (E.G., BROWSER TABS/VIDEO PLAYERS)
  wlr_subcompositor_create(server.wl_display);

  // DATA DEVICE MANAGER: ENABLES CLIPBOARD AND DRAG-AND-DROP
  wlr_data_device_manager_create(server.wl_display);

  //?????????????????????????????????????????????????????????????????
  //server.output_layout = wlr_output_layout_create(server.wl_display);
  server.output_layout = wlr_output_layout_create(server.wl_display);

  wl_list_init(&server.outputs);

  // SET CALLBACK FOR NEW MONITOR
  //server.new_output.notify = server_new_output;
  //wl_signal_add(&server.backend->events.new_output, &server.new_output);

  //------------------ RUN -----------------------


  //Add a Unix socket to the Wayland display
  const char *socket = wl_display_add_socket_auto(server.wl_display);
  if(!socket){
    wlr_backend_destroy(server.backend);
    return 1;
  }

  // START THE BACKEND. THIS WILL ENUMERATE OUTPUTS AND INPUTS, BECOME THE DRM MASTER, etc
  if(!wlr_backend_start(server.backend)){
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
  }

  //SET ENVIROMENT VARIABLE
  setenv("WAYLAND_DISPLAY", socket, true);
  if (startup_cmd) {
    if (fork() == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
    }
  }

  wlr_log(WLR_INFO, "Running Maia Compositor on WAYLAND_DISPLAY=%s", socket);

  // RUN EVENT LOOP
  wl_display_run(server.wl_display);

  return 0;
}
