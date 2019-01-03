#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/random.h>

#include <cpuid.h>
#include <immintrin.h>

#define OK_ENTROPY 2048

static void ensure_rdrand(void) {
  unsigned int eax, ebx, ecx, edx;
  __cpuid(1, eax, ebx, ecx, edx);
  if ((ecx & (1 << 30)) == 0) {
    fprintf(stderr, "fatal: CPU does not support rdrand!\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  const char *device_path = "/dev/random";
  int fd, ret;
  int entropy_count;

  ensure_rdrand();

  fd = open(device_path, O_RDWR);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  int count = 128; // 128 * 4 bits
  struct rand_pool_info *rpi;
  rpi = malloc(sizeof(struct rand_pool_info) + count * 4);
  if (rpi == NULL) {
    perror("malloc");
    return 1;
  }

  for (;;) {
    ret = ioctl(fd, RNDGETENTCNT, &entropy_count);
    if (ret < 0) {
      perror("RNDGETENTCNT");
      return 1;
    }

    if (entropy_count >= OK_ENTROPY) {
      sleep(1);
      continue;
    }

    rpi->entropy_count = count * 8;
    rpi->buf_size = count;

    fprintf(stderr,
            "entropy count (%d) < limit (%d), adding %d bits of entropy...\n",
            entropy_count, OK_ENTROPY, rpi->entropy_count);

    int32_t r;
    for (int i = 0; i < count; i++) {
      // rdrand can fail, returns 0 if so
      while (_rdrand32_step((unsigned int *)&r) == 0)
        ;
      rpi->buf[i] = r;
    }

    ret = ioctl(fd, RNDADDENTROPY, rpi);
    if (ret < 0) {
      perror("RNDADDENTROPY");
      return 1;
    }
  }

  free(rpi);

  ret = close(fd);
  if (ret < 0) {
    perror("close");
    return 1;
  }

  return 0;
}
