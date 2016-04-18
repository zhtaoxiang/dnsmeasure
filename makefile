CXX := g++ -std=c++11
CXXFLAGS := -I/usr/include/mysql -I/usr/include/mysql++ -I. -I/usr/local/include/mysql++ -I/usr/include/openssl

LDFLAGS := -L/usr/lib -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lldns -lmysqlpp -lmysqlclient -lnsl -lz -lm

all: dnsmeasure

dnsmeasure: dnsmeasure.cpp dnsservice.cpp database.cpp
	$(CXX) $(CXXFLAGS) -o dnsmeasure dnsmeasure.cpp dnsservice.cpp database.cpp $(LDFLAGS)

clean:
	rm -f *.o dnsmeasure *.cpp~ *.h~ makefile~ README~

dist:
	tar -cvzf dnsmeasure.tar.gz *.h *.cpp

clobber: clean
	rm -f *.tar.gz
