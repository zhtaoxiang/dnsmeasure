#include "config.h"
#include "database.h"
#include "dnsservice.h"

#include <thread>
#include <chrono>
#include <getopt.h>

static void
usage(char* program) {
  std::cout << "Usage:" << std::endl << "    " <<
    program << " [-h] [-r] [-s <server>] [-u <username>] [-p <password>] [-f <frequency of queries>]" << std::endl << std::endl <<
    "    -h: show this help message" << std::endl <<
    "    -r: reset the database" << std::endl << 
    "    -s: specify the database server, the default setting is \"localhost\"" << std::endl <<
    "    -u: specify the database username, the default setting is empty string" << std::endl <<
    "    -p: specify the database password, the default setting is empty string" << std::endl <<
    "    -f: specify the frequency of queries, the default setting is 10 seconds. Its unit is second." << std::endl;
  return;
}

int main(int argc, char* argv[])
{
  int opt = -1, option_index = 0;
  int frequency = 10;
  bool resetdatabase = false;
  std::string server = "localhost", user = "", password = "";
  char optstring[] = "s:u:p:f:rh";
  static struct option long_options[] = 
  {
    {"server", required_argument, NULL, 's'},
    {"user", required_argument, NULL, 'u'},
    {"password", required_argument, NULL, 'p'},
    {"frequency", no_argument, NULL, 'f'},
    {"reset_database", no_argument, NULL, 'r'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
  };
  while((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1) {
    switch(opt) {
      case 's':
        server = optarg;
        break;
      case 'u':
        user = optarg;
        break;
      case 'p':
        password = optarg;
        break;
      case 'f':
        frequency = std::stoi(optarg);
        break;
      case 'r':
        resetdatabase = true;
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
      default:
        usage(argv[0]);
        exit(0);
        break;
    }
  }
  Database db(server, user, password);
  DnsService ds;
  if(resetdatabase) {
    db.resetDatabase();
    exit(0);
  }
  while(true) {
    for(int i = 0; i < num_of_domains; i++) {
      double query_time = ds.resolve(domains[i]);
      db.insert(i, query_time);
    }
    std::this_thread::sleep_for(std::chrono::seconds(frequency));
  }
}

