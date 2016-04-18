/*
 * mx is a small program that prints out the mx records
 * for a particular domain
 * (c) NLnet Labs, 2005 - 2008
 * See the file LICENSE for the license
 */

#include "config.h"
#include "dnsservice.h"

#include <ldns/ldns.h>

static int
usage(FILE *fp, char *prog) {
	fprintf(fp, "%s domain\n", prog);
	fprintf(fp, "  print out the ipv4 address for domain\n");
	return 0;
}

std::string gen_random(const int len) {
  char * s = new char[len + 1];
  static const char alphanum[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
  free(s);
  s = NULL;
  return std::string(s);
}

int
test_main(int argc, char *argv[])
{
	ldns_resolver *res;
	ldns_rdf *domain;
	ldns_pkt *p;
	ldns_rr_list *mx;
	ldns_status s;
	uint32_t querytime; 
	
	p = NULL;
	mx = NULL;
	domain = NULL;
	res = NULL;
	
	if (argc != 2) {
		usage(stdout, argv[0]);
		exit(EXIT_FAILURE);
	} else {
		/* create a rdf from the command line arg */
		domain = ldns_dname_new_frm_str(argv[1]);
		if (!domain) {
			usage(stdout, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/* create a new resolver from /etc/resolv.conf */
	s = ldns_resolver_new_frm_file(&res, NULL);

	if (s != LDNS_STATUS_OK) {
		exit(EXIT_FAILURE);
	}

	/* use the resolver to send a query for the mx 
	 * records of the domain given on the command line
	 */
	p = ldns_resolver_query(res,
	                        domain,
	                        LDNS_RR_TYPE_A,
	                        LDNS_RR_CLASS_IN,
	                        LDNS_RD);

	ldns_rdf_deep_free(domain);
	
        if (!p)  {
		exit(EXIT_FAILURE);
        } else {
		/* retrieve the MX records from the answer section of that
		 * packet
		 */
		ldns_pkt_print(stdout, p);
		querytime = ldns_pkt_querytime(p);
		fprintf(stdout, "query time is %u\n", querytime);
		mx = ldns_pkt_rr_list_by_type(p,
		                              LDNS_RR_TYPE_A,
		                              LDNS_SECTION_ANSWER);
		if (!mx) {
			fprintf(stderr, 
					" *** invalid answer name %s after MX query for %s\n",
					argv[1], argv[1]);
                        ldns_pkt_free(p);
                        ldns_resolver_deep_free(res);
			exit(EXIT_FAILURE);
		} else {
			ldns_rr_list_sort(mx); 
			ldns_rr_list_print(stdout, mx);
			ldns_rr_list_deep_free(mx);
		}
        }
        ldns_pkt_free(p);
        ldns_resolver_deep_free(res);
        return 0;
}
