#include "config.h"
#include "dnsservice.h"

#include <sstream>

static int random_prefix_len = 10;

DnsService::DnsService() {
  ldns_status s = ldns_resolver_new_frm_file(&res, NULL);

  if (s != LDNS_STATUS_OK) {
    exit(EXIT_FAILURE);
  }
}

DnsService::~DnsService() {
  ldns_resolver_deep_free(res);
}

double DnsService::resolve(std::string domain_str) {
  ldns_rdf *domain;
  ldns_pkt *p;
  uint32_t querytime;
  /* create a rdf from the command line arg */
  std::stringstream ss; 
  ss << gen_random(random_prefix_len) << "." << domain_str;
  std::string random_str = ss.str();
  //std::cout <<  random_str << std:: endl;
  domain = ldns_dname_new_frm_str(domain_str.c_str());
  if (!domain) {
    std::cerr << "Please fix the domain names!" << std::endl;
    exit(EXIT_FAILURE);
  }
  /* use the resolver to send a query for the ip 
   * records of the domain given on the command line
   */
  p = ldns_resolver_query(res, domain, LDNS_RR_TYPE_A, LDNS_RR_CLASS_IN, LDNS_RD);

  ldns_rdf_deep_free(domain);
	
  if (!p)  {
    std::cerr << "Encountered error when trying to resolve the given domain!" << std::endl;
    exit(EXIT_FAILURE);
  } else {
    /* retrieve the ip records from the answer section of that
     * packet
     */
    //ldns_pkt_print(stdout, p);
    querytime = ldns_pkt_querytime(p);
  }
  ldns_pkt_free(p);
  return (double)querytime;
}

std::string DnsService::gen_random(const int len) {
  char * s = new char[len + 1];
  static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
  srand(time(NULL));
  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
  std::string result(s);
  free(s);
  s = NULL;
  return result;
}
