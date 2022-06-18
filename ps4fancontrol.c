#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

uint8_t DEFAULT_THRESHOLD = 0x4f;

#define ICC_MAJOR 'I'

struct icc_cmd {
  uint8_t major;
  uint16_t minor;
  uint8_t *data;
  uint16_t data_length;
  uint8_t *reply;
  uint16_t reply_length;
};

#define ICC_IOCTL_CMD _IOWR(ICC_MAJOR, 1, struct icc_cmd)

uint8_t get_threshold(int fd, int debug_mode) {

  struct icc_cmd arg;
  arg.major = 0xA;
  arg.minor = 0x7;
  arg.data = 0;
  arg.data_length = 0;
  arg.reply = malloc(0x52);
  arg.reply_length = 0x52;

  int ret = 0;
  ret = ioctl(fd, ICC_IOCTL_CMD, &arg);
  usleep(1000);

  switch (ret) {
  case -EFAULT:
    printf("Error: Bad address\n");
    return DEFAULT_THRESHOLD;
  case -ENOENT:
    printf("Error: Invalid command\n");
    return DEFAULT_THRESHOLD;
  case -1:
    perror("Error: Unknown");
    return DEFAULT_THRESHOLD;
  default:
    break;
  }

  if (debug_mode) {
    printf("Debug: Reply:\n");
    for (int i = 0; i < ret; i++) {
      printf("0x%02x\n", *(arg.reply + i));
    }
  }

  switch (*arg.reply) {
  case 0x00:
    return *(arg.reply + 5);
    break;
  case 0x02:
    printf("Error: Invalid data\n");
    return DEFAULT_THRESHOLD;
  case 0x04:
    printf("Error: Invalid data length\n");
    return DEFAULT_THRESHOLD;
  default:
    printf("Unknown status code: 0x%02x\n", *arg.reply);
    return DEFAULT_THRESHOLD;
  }

  return DEFAULT_THRESHOLD;
}

uint8_t set_threshold(int fd, int debug_mode, uint8_t threshold) {

  uint8_t data[] = {0x00, 0x00, 0x00, 0x00, 0x00, DEFAULT_THRESHOLD};
  struct icc_cmd arg;
  arg.major = 0xA;
  arg.minor = 0x6;
  arg.data = malloc(sizeof(data));
  arg.data_length = 0x34;
  arg.reply = malloc(0x20);
  arg.reply_length = 0x20;

  if (threshold >= 45 && threshold <= 85)
    data[5] = threshold;
  else {
    printf("New threshold temperature (%u°C) is out of range, setting default "
           "(%u°C)\n",
           threshold, DEFAULT_THRESHOLD);
  }

  memcpy(arg.data, data, sizeof(data));
  int ret = 0;
  ret = ioctl(fd, ICC_IOCTL_CMD, &arg);
  usleep(1000);

  switch (ret) {
  case -EFAULT:
    printf("Error: Bad address\n");
    return DEFAULT_THRESHOLD;
  case -ENOENT:
    printf("Error: Invalid command\n");
    return DEFAULT_THRESHOLD;
  case -1:
    perror("Error: Unknown");
    return DEFAULT_THRESHOLD;
  default:
    break;
  }

  if (debug_mode) {
    printf("Debug: Reply:\n");
    for (int i = 0; i < ret; i++) {
      printf("0x%02x\n", *(arg.reply + i));
    }
  }

  switch (*arg.reply) {
  case 0x00:
    return get_threshold(fd, debug_mode);
    break;
  case 0x02:
    printf("Error: Invalid data\n");
    return DEFAULT_THRESHOLD;
  case 0x04:
    printf("Error: Invalid data length\n");
    return DEFAULT_THRESHOLD;
  default:
    printf("Unknown status code: 0x%02x\n", *arg.reply);
    return DEFAULT_THRESHOLD;
  }

  return DEFAULT_THRESHOLD;
}

int main(int argc, char *argv[]) {

  gid_t gid = getegid();
  uid_t uid = geteuid();
  dev_t dev;

  if (uid != 0) {
    printf("Run the program as root\n");
    return -1;
  }

  setgid(gid);
  setuid(uid);

  // Get program arg
  int debug_mode = 0;
  uint8_t threshold = DEFAULT_THRESHOLD;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--debug") == 0)
      debug_mode = 1;
    if (strcmp(argv[i], "--threshold") == 0)
      threshold = (uint8_t)atoi(argv[i + 1]);
  }

  if (debug_mode) {
    printf("debug: (%s) threshold temp: (%u°C)\n",
           debug_mode ? "true" : "false", threshold);
  }

  // Create icc dev
  dev = makedev(0x49, 1);
  mknod("/dev/icc", S_IFCHR | 0666, dev);

  // Open icc dev
  int fd = open("/dev/icc", 0);
  if (fd == -1) {
    printf("Unable to open /dev/icc\n");
    return -1;
  }

  uint8_t curr_threshold = get_threshold(fd, debug_mode);
  printf("Current threshold temperature (%u°C)\n", curr_threshold);

  if (threshold == curr_threshold)
    return 0;

  curr_threshold = set_threshold(fd, debug_mode, threshold);
  printf("New threshold temperature (%u°C)\n", curr_threshold);

  // Close icc dev
  close(fd);

  return 0;
}
