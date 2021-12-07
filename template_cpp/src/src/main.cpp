#include <chrono>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <thread>

// #include <arpa/inet.h>
// #include <netdb.h>
// #include <sys/socket.h>
// #include <sys/types.h>

#include "lcb_app.h"
#include "hello.h"
#include "parser.hpp"

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  // Stop network packet processing, write output and free memory
  app_destroy();

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
  struct ProcessInfo *processInfos = static_cast<struct ProcessInfo *>(
      calloc(n_process, sizeof(struct ProcessInfo)));
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

  // Init application
  app_init(parser.outputPath(), parser.configPath(), parser.id(), processInfos,
           n_process);

  std::cout << "Broadcasting and delivering messages...\n\n";
  app_run();

  std::cout << "Broadcasting finished...\n\n";
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
