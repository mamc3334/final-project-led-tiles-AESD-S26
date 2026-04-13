# MP3 Background Player using libmpg123 + libout123

## Files
- `mp3_player.c` — player implementation
- `mp3_player.h` — public API header
- `main.c`        — example program
- `Makefile`      — build system

## Dependencies

### Host (Ubuntu/Debian)
```sh
sudo apt-get install libmpg123-dev
```
`libout123` is bundled with the mpg123 package — no separate install needed.

### Host (Fedora/RHEL)
```sh
sudo dnf install mpg123-devel
```

### Buildroot (AArch64 embedded target)
```
BR2_PACKAGE_MPG123=y
```
Via `make menuconfig` → Target packages → Libraries → Audio/Sound → mpg123,
or add directly to your defconfig.

## Build

### Host
```sh
make
```

### Buildroot cross-compile
```sh
make BR=1 CC=aarch64-linux-gnu-gcc SYSROOT=/path/to/staging
```

## API

```c
#include "mp3_player.h"

// Create a player instance
mp3_player_t *mp3_player_create(void);

// Start playback in background (non-blocking). Stops any current track first.
int mp3_player_play(mp3_player_t *player, const char *path);

// Stop playback. Blocks until the background thread exits.
void mp3_player_stop(mp3_player_t *player);

// Returns 1 if playing, 0 otherwise.
int mp3_player_is_playing(mp3_player_t *player);

// Returns current playback position in seconds. Thread-safe.
double mp3_player_get_position(mp3_player_t *player);

// Stop playback and free all resources.
void mp3_player_destroy(mp3_player_t *player);
```

## Usage Example

```c
#include "mp3_player.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    mp3_player_t *player = mp3_player_create();
    mp3_player_play(player, "song.mp3");  // non-blocking, returns immediately

    while (mp3_player_is_playing(player)) {
        double pos = mp3_player_get_position(player);
        printf("Position: %02d:%02d\n", (int)pos / 60, (int)pos % 60);
        sleep(1);
    }

    mp3_player_destroy(player);
    return 0;
}
```

