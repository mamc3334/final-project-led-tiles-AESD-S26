Generated from Claude AI: https://claude.ai/chat/e05980ff-3152-4e54-8500-b6134312b416

# MP3 Background Player using ALSA + minimp3

## Files
- `mp3_player.c` — main player implementation
- `Makefile`     — build system

## Dependencies
You need two things:

### 1. minimp3 (header-only, no install needed)
Download from: https://github.com/lieff/minimp3

Place `minimp3.h` and `minimp3_ex.h` in the same directory as `mp3_player.c`.

```sh
wget https://raw.githubusercontent.com/lieff/minimp3/master/minimp3.h
wget https://raw.githubusercontent.com/lieff/minimp3/master/minimp3_ex.h
```

### 2. ALSA development library
```sh
# Debian/Ubuntu
sudo apt-get install libasound2-dev

# Fedora/RHEL
sudo dnf install alsa-lib-devel

# Buildroot (for embedded target)
BR2_PACKAGE_ALSA_LIB=y
```

## Build
```sh
make
```

## Usage
```c
#include "mp3_player.h"

int main() {
    mp3_player_t *player = mp3_player_create();
    mp3_player_play(player, "song.mp3");  // non-blocking, plays in background

    // ... your main program logic here ...
    sleep(10);

    mp3_player_stop(player);
    mp3_player_destroy(player);
    return 0;
}
```
