/*
 * mp3_player.c — Background MP3 playback via libmpg123 + libout123
 *
 * Dependencies:
 *   - libmpg123-dev   (MP3 decoder)
 *   - libout123       (audio output, bundled with mpg123)
 *   - pthreads
 *
 * Compile:
 *   gcc -o mp3_player_demo mp3_player.c main.c -lmpg123 -lout123 -lpthread
 *
 * Buildroot:
 *   BR2_PACKAGE_MPG123=y
 */

// Generated from Claude AI: https://claude.ai/chat/e05980ff-3152-4e54-8500-b6134312b416

#include "mp3_player.h"

#include <mpg123.h>
#include <out123.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------- */

#define DECODE_BUFFER_SIZE  (4096 * 4)  /* bytes per decode chunk */

struct mp3_player {
    pthread_t       thread;
    pthread_mutex_t lock;

    atomic_int      stop_requested;   /* 1 → thread should exit         */
    atomic_int      playing;          /* 1 → thread is alive            */

    atomic_llong    samples_played;   /* total PCM samples decoded so far */
    atomic_long     sample_rate;      /* samples per second (set at open) */

    char            path[4096];
};

/* --------------------------------------------------------------------------
 * Playback thread
 * -------------------------------------------------------------------------- */

static void *playback_thread(void *arg)
{
    mp3_player_t *p = (mp3_player_t *)arg;
    int err;

    /* --- mpg123: open and probe the file --- */
    mpg123_handle *mh = mpg123_new(NULL, &err);
    if (!mh) {
        fprintf(stderr, "[mp3_player] mpg123_new: %s\n", mpg123_plain_strerror(err));
        goto done;
    }

    if (mpg123_open(mh, p->path) != MPG123_OK) {
        fprintf(stderr, "[mp3_player] mpg123_open '%s': %s\n",
                p->path, mpg123_strerror(mh));
        mpg123_delete(mh);
        goto done;
    }

    /* Read format info (sample rate, channels, encoding) */
    long rate;
    int  channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        fprintf(stderr, "[mp3_player] mpg123_getformat: %s\n", mpg123_strerror(mh));
        mpg123_close(mh);
        mpg123_delete(mh);
        goto done;
    }

    /* Lock format so it doesn't change mid-stream */
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    printf("[mp3_player] Playing '%s' (%ld Hz, %d ch)\n", p->path, rate, channels);

    /* Reset position counters */
    atomic_store(&p->sample_rate, rate);
    atomic_store(&p->samples_played, 0);

    /* --- out123: open audio output --- */
    out123_handle *ao = out123_new();
    if (!ao) {
        fprintf(stderr, "[mp3_player] out123_new failed\n");
        mpg123_close(mh);
        mpg123_delete(mh);
        goto done;
    }

    if (out123_open(ao, NULL, NULL) != OUT123_OK) {
        fprintf(stderr, "[mp3_player] out123_open: %s\n", out123_strerror(ao));
        out123_del(ao);
        mpg123_close(mh);
        mpg123_delete(mh);
        goto done;
    }

    if (out123_start(ao, rate, channels, encoding) != OUT123_OK) {
        fprintf(stderr, "[mp3_player] out123_start: %s\n", out123_strerror(ao));
        out123_close(ao);
        out123_del(ao);
        mpg123_close(mh);
        mpg123_delete(mh);
        goto done;
    }

    /* --- Decode + play loop --- */
    unsigned char buf[DECODE_BUFFER_SIZE];
    size_t bytes_decoded;

    while (!atomic_load(&p->stop_requested)) {
        int ret = mpg123_read(mh, buf, sizeof(buf), &bytes_decoded);

        if (ret == MPG123_DONE || bytes_decoded == 0)
            break;  /* end of file */

        if (ret != MPG123_OK && ret != MPG123_NEW_FORMAT) {
            fprintf(stderr, "[mp3_player] mpg123_read: %s\n", mpg123_strerror(mh));
            break;
        }

        out123_play(ao, buf, bytes_decoded);

        /* bytes → samples: divide by (channels * bytes-per-sample).
         * mpg123 default encoding is MPG123_ENC_SIGNED_16 = 2 bytes/sample. */
        long long samples = (long long)(bytes_decoded / (size_t)(channels * 2));
        atomic_fetch_add(&p->samples_played, samples);
    }

    out123_close(ao);
    out123_del(ao);
    mpg123_close(mh);
    mpg123_delete(mh);

    printf("[mp3_player] Playback finished: %s\n", p->path);

done:
    atomic_store(&p->playing, 0);
    return NULL;
}

/* --------------------------------------------------------------------------
 * Public API
 * -------------------------------------------------------------------------- */

mp3_player_t *mp3_player_create(void)
{
    if (mpg123_init() != MPG123_OK) {
        fprintf(stderr, "[mp3_player] mpg123_init failed\n");
        return NULL;
    }

    mp3_player_t *p = calloc(1, sizeof(*p));
    if (!p)
        return NULL;

    pthread_mutex_init(&p->lock, NULL);
    atomic_store(&p->stop_requested, 0);
    atomic_store(&p->playing, 0);
    atomic_store(&p->samples_played, 0);
    atomic_store(&p->sample_rate, 0);
    return p;
}

int mp3_player_play(mp3_player_t *player, const char *path)
{
    if (!player || !path)
        return -1;

    mp3_player_stop(player);

    pthread_mutex_lock(&player->lock);

    strncpy(player->path, path, sizeof(player->path) - 1);
    player->path[sizeof(player->path) - 1] = '\0';

    atomic_store(&player->stop_requested, 0);
    atomic_store(&player->playing, 1);
    atomic_store(&player->samples_played, 0);
    atomic_store(&player->sample_rate, 0);

    int rc = pthread_create(&player->thread, NULL, playback_thread, player);
    if (rc != 0) {
        fprintf(stderr, "[mp3_player] pthread_create: %s\n", strerror(rc));
        atomic_store(&player->playing, 0);
        pthread_mutex_unlock(&player->lock);
        return -1;
    }

    pthread_mutex_unlock(&player->lock);
    return 0;
}

double mp3_player_get_position(mp3_player_t *player)
{
    if (!player)
        return 0.0;

    long rate = atomic_load(&player->sample_rate);
    if (rate <= 0)
        return 0.0;

    long long samples = atomic_load(&player->samples_played);
    return (double)samples / (double)rate;
}

void mp3_player_stop(mp3_player_t *player)
{
    if (!player || !atomic_load(&player->playing))
        return;

    atomic_store(&player->stop_requested, 1);
    pthread_join(player->thread, NULL);
    atomic_store(&player->playing, 0);
}

int mp3_player_is_playing(mp3_player_t *player)
{
    if (!player)
        return 0;
    return atomic_load(&player->playing);
}

void mp3_player_destroy(mp3_player_t *player)
{
    if (!player)
        return;
    mp3_player_stop(player);
    pthread_mutex_destroy(&player->lock);
    free(player);
    mpg123_exit();
}
