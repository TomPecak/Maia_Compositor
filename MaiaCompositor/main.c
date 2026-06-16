
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

struct tinywl_server {
  struct wl_display* wl_display;
  struct wlr_backend* backend;
  struct wlr_renderer* renderer;
  struct wlr_allocator* allocator;
  struct wlr_scene *scene;
  struct wlr_scene_output_layout *scene_layout;

  struct wlr_xdg_shell* xdg_shell;
  struct wl_listener new_xdg_toplevel;
  struct wl_listener new_xdg_popup;
  struct wl_list toplevels;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
};

struct tinywl_output {
  struct wl_list link;
  struct tinywl_server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
  struct wl_listener request_state;
  struct wl_listener destroy;
};

#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"

#define maia_info(fmt, ...)                                        \
  fprintf(stderr, COLOR_GREEN "[MAIA INFO] " COLOR_RESET fmt "\n", \
          ##__VA_ARGS__)

static void output_frame(struct wl_listener *listener, void *data) {
  /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
  struct tinywl_output *output = wl_container_of(listener, output, frame);
  struct wlr_scene *scene = output->server->scene;

  struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
      scene, output->wlr_output);

  /* Render the scene if needed and commit the output */
  wlr_scene_output_commit(scene_output, NULL);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
  /* This function is called when the backend requests a new state for
   * the output. For example, Wayland and X11 backends request a new mode
   * when the output window is resized. */
  struct tinywl_output *output = wl_container_of(listener, output, request_state);
  const struct wlr_output_event_request_state *event = data;
  wlr_output_commit_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
  struct tinywl_output *output = wl_container_of(listener, output, destroy);

  wl_list_remove(&output->frame.link);
  wl_list_remove(&output->request_state.link);
  wl_list_remove(&output->destroy.link);
  wl_list_remove(&output->link);
  free(output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
  /* This event is raised by the backend when a new output (aka a display or
   * monitor) becomes available. */
  struct tinywl_server *server =
      wl_container_of(listener, server, new_output);
  struct wlr_output *wlr_output = data;

  /* Configures the output created by the backend to use our allocator
   * and our renderer. Must be done once, before commiting the output */
  wlr_output_init_render(wlr_output, server->allocator, server->renderer);

  /* The output may be disabled, switch it on. */
  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);

  /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
   * before we can use the output. The mode is a tuple of (width, height,
   * refresh rate), and each monitor supports only a specific set of modes. We
   * just pick the monitor's preferred mode, a more sophisticated compositor
   * would let the user configure it. */
  struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
  if (mode != NULL) {
    wlr_output_state_set_mode(&state, mode);
  }

  /* Atomically applies the new output state. */
  wlr_output_commit_state(wlr_output, &state);
  wlr_output_state_finish(&state);

  /* Allocates and configures our state for this output */
  struct tinywl_output *output = calloc(1, sizeof(*output));
  output->wlr_output = wlr_output;
  output->server = server;

  /* Sets up a listener for the frame event. */
  output->frame.notify = output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);

  /* Sets up a listener for the state request event. */
  output->request_state.notify = output_request_state;
  wl_signal_add(&wlr_output->events.request_state, &output->request_state);

  /* Sets up a listener for the destroy event. */
  output->destroy.notify = output_destroy;
  wl_signal_add(&wlr_output->events.destroy, &output->destroy);

  wl_list_insert(&server->outputs, &output->link);

  /* Adds this to the output layout. The add_auto function arranges outputs
   * from left-to-right in the order they appear. A more sophisticated
   * compositor would let the user configure the arrangement of outputs in the
   * layout.
   *    * The output layout utility automatically adds a wl_output global to the
   * display, which Wayland clients can see to find out information about the
   * output (such as DPI, scale factor, manufacturer, etc).
   */
  struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->output_layout,
                                                                         wlr_output);
  struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene, wlr_output);
  wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}

static void server_new_xdg_toplevel(struct wl_listener* listener, void* data) {}

static void server_new_xdg_popup(struct wl_listener* listener, void* data) {
  }

  int main(int argc, char* argv[]) {
    wlr_log_init(WLR_ERROR, NULL);
    char* startup_cmd = NULL;

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

    server.backend = wlr_backend_autocreate(
        wl_display_get_event_loop(server.wl_display), NULL);
    if (server.backend == NULL) {
      wlr_log(WLR_ERROR, "failed to create wlr_backend");
      return 1;
    }

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (server.renderer == NULL) {
      wlr_log(WLR_ERROR, "failed to create wlr_renderer");
      return 1;
    }

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    // CREATE GBM BUFFERS
    server.allocator =
        wlr_allocator_autocreate(server.backend, server.renderer);
    if (server.allocator == NULL) {
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
    // server.output_layout = wlr_output_layout_create(server.wl_display);
    server.output_layout = wlr_output_layout_create(server.wl_display);

    wl_list_init(&server.outputs);

    // SET CALLBACK FOR NEW MONITOR
    server.new_output.notify = server_new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    server.scene = wlr_scene_create();
    server.scene_layout =
        wlr_scene_attach_output_layout(server.scene, server.output_layout);

    wl_list_init(&server.toplevels);
    server.xdg_shell = wlr_xdg_shell_create(server.wl_display, 3);
    server.new_xdg_toplevel.notify = server_new_xdg_toplevel;
    wl_signal_add(&server.xdg_shell->events.new_toplevel,
                  &server.new_xdg_toplevel);
    server.new_xdg_popup.notify = server_new_xdg_popup;
    wl_signal_add(&server.xdg_shell->events.new_popup, &server.new_xdg_popup);

    //------------------ RUN -----------------------

    // Add a Unix socket to the Wayland display
    const char* socket = wl_display_add_socket_auto(server.wl_display);
    if (!socket) {
      wlr_backend_destroy(server.backend);
      return 1;
    }

    // START THE BACKEND. THIS WILL ENUMERATE OUTPUTS AND INPUTS, BECOME THE DRM
    // MASTER, etc
    if (!wlr_backend_start(server.backend)) {
      wlr_backend_destroy(server.backend);
      wl_display_destroy(server.wl_display);
    }

    // SET ENVIROMENT VARIABLE
    setenv("WAYLAND_DISPLAY", socket, true);
    if (startup_cmd) {
      if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void*)NULL);
      }
    }

    wlr_log(WLR_INFO, "Running Maia Compositor on WAYLAND_DISPLAY=%s", socket);
    maia_info("export  WAYLAND_DISPLAY=%s\n", socket);
    printf("export  WAYLAND_DISPLAY=%s\n", socket);

    // RUN EVENT LOOP
    wl_display_run(server.wl_display);

    //* QUIT *//
    wl_display_destroy(server.wl_display);

    return 0;
  }
