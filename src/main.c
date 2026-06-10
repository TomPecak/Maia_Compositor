#include <stdio.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

struct tinywl_server {
  struct wl_display* wl_display;
};

int main(int argc, char* argv[]) {
  wlr_log_init(WLR_DEBUG, NULL);
  char* startup_cmd = NULL;

  int c;
  while ((c = getopt(argc, argv, "s:h")) != -1) {
    switch (c) {
      case 's':
        startup_cmd = optarg;
        break;
      default:
        printf("Usage: %s [-s startup command]\n", argv[0]);
        return 0;
    }
  }
  if (optind < argc) {
    printf("Usage: %s [-s startup command]\n", argv[0]);
    return 0;
  }

  struct tinywl_server server = {0};
  server.wl_display = wl_display_create();

  printf("Hello Maia Compositor :)\n");
  return 0;
}
