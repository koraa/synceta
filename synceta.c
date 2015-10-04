#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

  /* Initialize *buf if necessary */
  if (*buf == NULL) {
    *buf = realloc(*buf, *len);
  }

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

int main() {
  printf("%lu\n", fscache_dirty());
  return 0;
}
