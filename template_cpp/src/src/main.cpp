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

// TODO: DEL
#include <unistd.h>
#define GetCurrentDir getcwd
using namespace std;

#define EVENTS_SIZE 256
#define FILENAME_SIZE 256
#define N_PROCESS 3

// TODO: make it dynamic. Is it is ok to use global variable?
char events[EVENTS_SIZE] = {0};
// TODO: is there another way to access it?
char filename[FILENAME_SIZE] = {0};
int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  close(sock_fd);

  // write/flush output file if necessary
  std::cout << "Writing output.\n";
  // TODO: ask if this name is ok
  // char filename[FILENAME_SIZE] = {0};
  // snprintf(filename, FILENAME_SIZE, "%d", host_id);
  write_output(events, filename);

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

  // TODO: DEL
  char buff[FILENAME_MAX];
  char *cwd = getcwd(buff, FILENAME_MAX);
  cout << buff << endl;

  size_t n_process = parser.hosts().size();
  struct ProcessInfo processInfos[N_PROCESS];
  uint i = 0;

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

    processInfos[i].id = host.id;
    processInfos[i].ip = host.ip;
    processInfos[i].port = host.port;
    // (*processInfos)[i].id = host.id;
    // (*processInfos)[i].ip = host.ip;
    // (*processInfos)[i].port = host.port;
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
  strncpy(filename, parser.outputPath(), FILENAME_SIZE - 1);

  std::cout << "Broadcasting and delivering messages...\n\n";

  struct sockaddr_in receiver_addr, sender_addr;
  receiver_addr.sin_family = AF_INET;
  receiver_addr.sin_port = hosts[configInfo.receiver_id - 1].port;
  receiver_addr.sin_addr.s_addr = hosts[configInfo.receiver_id - 1].ip;
  printf("port: %d, ip: %u", receiver_addr.sin_port,
         receiver_addr.sin_addr.s_addr);

  printf("Start running.");
  run(receiver_addr, sock_fd, configInfo, events, parser.id(), processInfos,
      n_process);

  printf("Run ended.");
  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
