#ifndef DATABASE_H
#define DATABASE_H
#include <mysql++.h>
#include <iostream>
#include <cstdio>
#include <exception>
#include <cmath>

class Database {
private:
  mysqlpp::Connection con;
public:
  Database(std::string server, std::string user, std::string password);
  void resetDatabase();
  void printMeasurement();
  void insert(int domain_index, double query_time);
private:
  int connect_mysql(std::string server, std::string user, std::string password);
  int initialize_tables();
};
#endif
