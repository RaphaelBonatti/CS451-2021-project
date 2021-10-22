#include <chrono>
#include <iostream>
#include <signal.h>
#include <thread>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "hello.h"
#include "io_handler.h"
#include "network_handler.h"
#include "parser.hpp"
#include "stubborn_links.h"

#define FILENAME_SIZE 256
#define N_PROCESS 9

// TODO: is there another way to access it?
char filename[FILENAME_SIZE] = {0};
int sock_fd;

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  close(sock_fd);

  // write/flush output file if necessary
  std::cout << "Writing output.\n";
  write_output(filename);

  // Freeing memory
  destroy_events();

  // exit directly from signal handler
  exit(0);
}

int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv);
  parser.parse();

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid()
            << "` or `kill -SIGTERM " << getpid()
            << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  size_t n_process = parser.hosts().size();
  struct ProcessInfo processInfos[N_PROCESS];
  size_t i = 0;

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();
  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";

    // struct for C
    processInfos[i].id = host.id;
    processInfos[i].ip = host.ip;
    processInfos[i].port = host.port;
    ++i;
  }
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

  std::cout << "Doing some initialization...\n\n";
  struct ConfigInfo configInfo;
  init_config_info(&configInfo, parser.configPath());
  init_events();
  strncpy(filename, parser.outputPath(), FILENAME_SIZE - 1);

  std::cout << "Broadcasting and delivering messages...\n\n";
  run(&sock_fd, &configInfo, parser.id(), processInfos, n_process);

  std::cout << "Broadcasting finished...\n\n";
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
