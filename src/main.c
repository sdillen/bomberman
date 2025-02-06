#include <dlfcn.h>
#include <raylib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

typedef void (*InitFunc)();
typedef void (*ReloadFunc)();
typedef int (*UpdateFunc)();
typedef void (*CleanFunc)();

time_t last_modified_time = 0;
const char *hotreload = "./src/hotreload.so";

_Bool check_for_update(const char *);

int main() {
  void *handle = dlopen(hotreload, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "Fehler beim Laden der Bibliothek: %s\n", dlerror());
    return 1;
  }

  InitFunc Init = (InitFunc)dlsym(handle, "Init");
  ReloadFunc Reload = (ReloadFunc)dlsym(handle, "Reload");
  UpdateFunc Update = (UpdateFunc)dlsym(handle, "Update");
  CleanFunc Clean = (CleanFunc)dlsym(handle, "Update");

  check_for_update(hotreload);
  Init();

  do {
    if (check_for_update(hotreload)) {
      dlclose(handle);
      handle = dlopen(hotreload, RTLD_LAZY);
      if (!handle) {
        fprintf(stderr, "Fehler beim Laden der Bibliothek: %s\n", dlerror());
        return 1;
      }
      printf("Aktualisierung in der Bibliothek: hotreload.so\n");
      // Funktionen aus der dynamische Library laden
      Init = (InitFunc)dlsym(handle, "Init");
      Reload = (ReloadFunc)dlsym(handle, "Reload");
      Update = (UpdateFunc)dlsym(handle, "Update");
      Clean = (CleanFunc)dlsym(handle, "Update");
      Reload();
    }
  } while (Update() == 0);

  Clean();
  dlclose(handle);
  return 0;
}

_Bool check_for_update(const char *filename) {
  struct stat file_stat;
  if (stat(filename, &file_stat) == 0) {
    if (file_stat.st_mtime > last_modified_time) {
      last_modified_time = file_stat.st_mtime;
      return 1; // Datei wurde geändert
    }
  }
  return 0; // Keine Änderung
}
