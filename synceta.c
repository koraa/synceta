#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define __need_timespec 1
#include <time.h>

#include <pthread.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

/**
 * Read a file, automatically resizing the buffer to read
 * into.
 *
 * If *len == 0, the length will be automatically
 * initialized to an appropriate value.
 *
 * If the *buf == NULL, the buffer will allocated
 * automatically.
 *
 * If the file is larger than the available buffer, the
 * *buf and *len will be updated to accommodate the file.
 *
 * After the function exits, *buf shall point to memory
 * containing the data of the file plus a \0 terminator
 * after the file.
 *
 * @param fd The file (descriptor) to read from
 * @param buf The buffer to read into
 * @param len The length of the buffer
 * @return The actual length of the file, excluding the \0
 *         terminator
 */
size_t auto_read_fd(int fd, char **buf, size_t *len) {
  /* Initialize *len if necessary */
  if (*len == 0) {
    *len = 1024;
    *buf = realloc(*buf, *len);
  }
/* Initialize *buf if necessary */ if (*buf == NULL) { *buf = realloc(*buf, *len); } 
  size_t red = 0;

  /* Read the file piece by piece, gradually resizing the buffer */
  for (;;) {
    /* Available space in the free buffer, minus one for the
       string terminator \0 */
    size_t toread = *len - red - 1;
    ssize_t rednow = read(fd, *buf, toread);
    red += rednow;

    if (rednow == -1) { /* TODO: Handle Errors */
    } else if (rednow == 0) { /* EOF */
      break;
    } else if (rednow == toread) { /* Filled buffer */
      /* TODO: this is not optimal when this is hit at the file size */
      *len *= 2;
      *buf = realloc(*buf, *len);
    }
  }

  /* Append the string terminator */
  (*buf)[red] = '\0';

  return red;
};

/**
 * Automatically read a file, reallocating the buffer to
 * fit the size of the file.
 *
 * Exactly like auto_read_fd, but takes a path and
 * automatically opens that path read only.
 */
size_t auto_read_file(const char *path, char **buf, size_t *len) {
  int fd = open(path, O_RDONLY);
  size_t red = auto_read_fd(fd, buf, len);
  close(fd);
  return red;
}

/**
 * Get the number of bytes in the fs write cache that still
 * need to be written.
 *
 * Not thread safe!
 */
size_t fscache_dirty() {
  static char *buf = NULL;
  static size_t len = 16 * 1024; /* 16 KB */

  auto_read_file("/proc/meminfo", &buf, &len);

  char *line = strstr(buf, "Dirty:");
  line += sizeof("Dirty:");

  return atoll(line) * 1000;
}

/**
 * Sleep for N milliseconds
 * (Wrapper around nanosleep)
 * @param t The number of milliseconds to sleep
 */
void millisleep(long long t) {
  struct timespec tt;
  struct timespec out;
  tt.tv_sec = t / 1000;
  tt.tv_nsec = (t % 1000) * 1000000;
  nanosleep(&tt, &out); /* TODO: Error handling */
}

/** Minimum of two integers */
long long min(long long a, long long b) {
  return a > b ? b : a;
}

/** Maximum of two integers */
long long max(long long a, long long b) {
  return a > b ? a : b;
}

const char *si_byte[] = {"B", "KB", "MB", "GB", "TB", "EB"};

void pt_size(double size) {
  size_t unit = 0;
  for (unit = 0; unit < sizeof(si_byte) && fabs(size) > 1024; unit++)
    size /= 1024;
  printf("%.f%s", size, si_byte[unit]);
}

/**
 * Thread that continuously prints the progress syncing the
 * fs cache.
 */
void* prog_thr(void *_) {
  const static int samples_per_sec = 1
                 , avrage_window_sec = 2;
  const static size_t sample_no = samples_per_sec * avrage_window_sec;
  const static long long sleep_dur = 1000 / samples_per_sec;

  size_t idx;

  /* Ring buffer for fscache size samples */
  size_t dirty_hist[sample_no];
  size_t head = 0;

  size_t biggest_size = dirty_hist[head] = fscache_dirty();

  for (idx = 0;; idx++) {
    /* Wait until we can gather some more snapshots */
    millisleep(sleep_dur);

    /* Catch a race where the syncing finished, but this
       thread hasn't been stopped yet */
    if (dirty_hist[head] == 0) continue;

    head = (head + 1) % sample_no;
    dirty_hist[head] = fscache_dirty();

    biggest_size = max( biggest_size, dirty_hist[head]);

    size_t done = biggest_size - dirty_hist[head];
    int done_percent = ((double)done / biggest_size) * 100;

    size_t sample_count = min(idx + 1, sample_no);
    size_t oldest_sample = dirty_hist[(head + sample_no + 1 - sample_count) % sample_no];

    long long window_duration = sleep_dur * sample_count;

    /* bytes per second */
    double speed = ((double)oldest_sample - dirty_hist[head])
                 / (window_duration / 1000);

    /* Delete last line */
    putchar('\r');

    /* Print: $done/$biggest = ($done_percent) |  */
    pt_size(done);
    printf(" / ");
    pt_size(biggest_size);
    putchar(' ');
    printf("%i%% | ", done_percent);

    /* $speed/s | ETA: */
    pt_size(speed);
    printf("/s | ETA: ");

    /* hh:mm:ss */
    if (dirty_hist[head] >= oldest_sample)
      printf("--:--:--");
    else {
      int eta = dirty_hist[head] / speed;
      int hours = eta / 60 / 60,
          minutes = (eta / 60) % 60,
          seconds = eta % 60;
      printf("%02i:%02i:%02i", hours, minutes, seconds);
    }

    /* clear the rest of the line */
    printf("          ");

    fflush(stdout);
  }

  return NULL;
}

int main() {
  pthread_t pt;
  pthread_create(&pt, NULL, prog_thr, NULL);

  sync();

  pthread_cancel(pt);
  pthread_join(pt, NULL);
  putchar('\n');

  return 0;
}
