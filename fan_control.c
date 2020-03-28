#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define PIN_NAME "24"
#define START_FAN_TEMPERATURE (52)
#define TEMPERATURE_BUFFER (5)

void initializePin(void) {
  // Export the desired pin by writing to /sys/class/gpio/export
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if (fd == -1) {
    perror("Unable to open /sys/class/gpio/export");
    exit(1);
  }

  if (write(fd, PIN_NAME, 2) != 2) {
    perror("Error writing to /sys/class/gpio/export");
    exit(1);
  }
  close(fd);

  // Set the pin to be an output by writing "out" to /sys/class/gpio/gpio24/direction
  fd = open("/sys/class/gpio/gpio24/direction", O_WRONLY);
  if (fd == -1) {
    perror("Unable to open /sys/class/gpio/gpio24/direction");
    exit(1);
  }

  if (write(fd, "out", 3) != 3) {
    perror("Error writing to /sys/class/gpio/gpio24/direction");
    exit(1);
  }
  close(fd);
}

void deinitializePin(void) {
  int fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (fd == -1) {
    perror("Unable to open /sys/class/gpio/unexport");
    exit(1);
  }

  if (write(fd, PIN_NAME, 2) != 2) {
    perror("Error writing to /sys/class/gpio/unexport");
    exit(1);
  }
  close(fd);
}

void signalHandler(int signal) {
  deinitializePin();
  exit(0);
}

int readTemperature() {
  static char buffer[32];
  int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if (fd == -1) {
    perror("Unable to open temperature file");
    exit(1);
  }
  if (read(fd, buffer, sizeof(buffer)) == -1) {
    perror("Error reading temperature file");
    exit(1);
  }
  close(fd);
  return atoi(buffer) / 1000;
}

enum state{OFF, ON};

void setOutput(int output, enum state value) {
  static char buffer[2];
  buffer[0] = '0' + value;
  if (write(output, buffer, 1) != 1) {
    perror("Error writing to output");
    exit(1);
  }
}

void loop() {
  int output = open("/sys/class/gpio/gpio" PIN_NAME "/value", O_WRONLY);

  enum state currentState = OFF;
  setOutput(output, currentState);
  for (;;) {
    int temperature = readTemperature();
    enum state newState = currentState;
    switch (currentState) {
    case OFF:
      if (temperature > START_FAN_TEMPERATURE) {
        newState = ON;
      }
      break;
    case ON:
      if (temperature < START_FAN_TEMPERATURE - TEMPERATURE_BUFFER) {
        newState = OFF;
      }
      break;
    }	
    if (newState != currentState) {
      currentState = newState;
      setOutput(output, currentState);
    }
    sleep(1);
  }
}

int main() {
  int deamonResult = daemon(0, 0);
  if (deamonResult == -1) {
    perror("Daemon failed");
    exit(1);
  }

  initializePin();

  signal(SIGINT, signalHandler);
  signal(SIGSEGV, signalHandler);
  signal(SIGTERM, signalHandler);

  loop();

  return 0;
}
