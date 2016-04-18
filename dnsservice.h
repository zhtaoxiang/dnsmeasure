#ifndef DNSSERVICE_H
#define DNSSERVICE_H

#include <iostream>
#include <ldns/ldns.h>

class DnsService {
  private:
    ldns_resolver *res;
  public: 
    DnsService();
    ~DnsService();
    double resolve(std::string domain_str);
  private:
    std::string gen_random(const int len);
};

#endif
