#include <database.h>
#include "config.h"

static std::string database_name = "dnsmeasure";
static std::string table_name = "stat";
static std::string original_table_names[] = {
    "google", "facebook", "youtube", "yahoo", "live", "wikipedia", "baidu", "blogger", "msn", "qq"
};
static int num_of_domains = 10;

Database::Database() 
{
  connect_mysql();
}

int Database::connect_mysql()
{
  try {
    con.connect(0, "localhost", "root", "123456");
  } catch (std::exception& er) {
    std::cerr << "Connection failed: " << er.what() << std::endl;
    return 1;
  }

  bool new_db = false;
  {
    mysqlpp::NoExceptions ne(con);
    mysqlpp::Query query = con.query();
    if (con.select_db(database_name)) {
      // Toss old tables, ignoring errors because it would just
      // mean the table doesn't exist, which doesn't matter.
      return 0;
    }
    else {
      // Database doesn't exist yet, so create and select it.
      if (con.create_db(database_name) &&
            con.select_db(database_name)) {
        new_db = true;
      }
      else {				
        std::cerr << "Error creating DB: " << con.error() << std::endl;
        return 1;
      }
    }
  }
  // need to create the database
  if(!new_db)
    return 0;
  try {
    // Send the query to create the stat table and execute it.
    std::cout << "Creating stat table..." << std::endl;
    mysqlpp::Query query = con.query();
    query << 
      "CREATE TABLE " << table_name << " (" <<
      "  rank INTEGER NOT NULL, " <<
      "  name VARCHAR(30) NOT NULL, " <<
      "  aveque DOUBLE NOT NULL, " <<
      "  std DOUBLE NOT NULL, " << 
      "  quenum BIGINT NOT NULL, " <<
      "  stime TIMESTAMP, " <<
      "  etime TIMESTAMP)" <<
      "ENGINE = InnoDB " <<
      "CHARACTER SET utf8 COLLATE utf8_general_ci";
    query.execute();

    for(int i = 0; i < num_of_domains; i++) {
      query << 
        "CREATE TABLE " << original_table_names[i] << " (" <<
        "  id INTEGER NOT NULL AUTO_INCREMENT, " <<
        "  latency DOUBLE NOT NULL, " <<
        "  PRIMARY KEY (ID))";
      query.execute();
    }

    // Set up the template query to insert the data.  The parse()
    // call tells the query object that this is a template and
    // not a literal query string.
    query << "insert into " << table_name << " values " << "(%0q, %1q, %2q, %3q, %4q, %5q, %6q)";
    query.parse();
    for(int i = 0; i < num_of_domains; i++) {
      query.execute(i + 1, domains[i], 0, 0, 0, "1970-01-01 00:00:00", "1970-01-01 00:00:00");
    }

    // Test that above did what we wanted.
    std::cout << "inserted " << con.count_rows(table_name) << " rows." << std::endl;
    
    return 0;
  }
  catch (const mysqlpp::BadQuery& er) {
    // Handle any query errors
    std::cerr << std::endl << "Query error: " << er.what() << std::endl;
    return 1;
  }
  catch (const mysqlpp::BadConversion& er) {
    // Handle bad conversions
    std::cerr << std::endl << "Conversion error: " << er.what() << std::endl <<
      "\tretrieved data size: " << er.retrieved <<
      ", actual size: " << er.actual_size << std::endl;
    return 1;
  }
  catch (const mysqlpp::Exception& er) {
    // Catch-all for any other MySQL++ exceptions
    std::cerr << std::endl << "Error: " << er.what() << std::endl;
    return 1;
  }
}

void Database::printMeasurement() 
{
  mysqlpp::Query query = con.query();
  query << "select * from stat";
  mysqlpp::StoreQueryResult res = query.store();
  
  //print out the header  
  std::cout << std::setw(6) << "Rank" <<
    std::setw(16) << "Domain" <<
    std::setw(16) << "Ave Query T(ms)" <<
    std::setw(16) << "Std Dev(ms)" <<
    std::setw(16) << "Total Query Num" <<
    std::setw(21) << "Time of First Query" <<
    std::setw(21) << "Time of Last Query" << std::endl;
  // Use the StoreQueryResult class's read-only random access iterator to walk
  // through the query results.
  mysqlpp::StoreQueryResult::iterator i;
  for (i = res.begin(); i != res.end(); ++i) {
    // Notice that a dereferenced result iterator can be converted
    // to a Row object, which makes for easier element access.
    std::cout << std::setw(6) << (*i)[0] << ' ' <<
      std::setw(15) << std::string((*i)[1]) << ' ' <<
      std::setw(15) << (*i)[2] << ' ' << 
      std::setw(15) << (*i)[3] << ' ' <<
      std::setw(15) << (*i)[4] << ' ' <<
      std::setw(20) << (*i)[5] << ' ' << 
      std::setw(20) << (*i)[6] << std::endl;
  }
}

void Database::insert(int domain_index, double query_time) 
{
  //insert data into original data table;
  mysqlpp::Query query = con.query();
  query << "insert into " << original_table_names[domain_index] << " (latency) values (" << query_time << ")";
  query.execute();

  //modify the statistics table;
  query << "select * from stat where rank = " << (domain_index + 1);
  mysqlpp::StoreQueryResult res = query.store();
  mysqlpp::StoreQueryResult::iterator i;
  i = res.begin();
  if(i == res.end()) {
    std::cerr << "Something is wrong with the database, please fix it first" << std::endl;
    return;
  }

  query.reset();
  query << "UPDATE stat set ";
  if((int) (*i)[4] == 0) {
    query << " aveque=" << query_time << 
      ", quenum=" << 1 << 
      ", stime=NOW(), etime=NOW()";
  }
  else {
    int total_query_num = ((int)(*i)[4]) + 1;
    double ave_query_time = (((double)(*i)[2]) * (total_query_num - 1) + query_time) / total_query_num;

    // calculate the standard deviation
    double sum = 0;
    mysqlpp::Query query_from_original_table = con.query();
    query_from_original_table << "select * from " << original_table_names[domain_index];
    mysqlpp::StoreQueryResult res_from_original_table = query_from_original_table.store();
    mysqlpp::StoreQueryResult::iterator i_from_original_table;
    for (i_from_original_table = res_from_original_table.begin(); i_from_original_table != res_from_original_table.end(); ++i_from_original_table) {
      sum += pow((double)(*i_from_original_table)[1] - ave_query_time, 2.0);
    }
    double std = sqrt(sum / total_query_num);

    query << " aveque=" << ave_query_time << 
      ", std=" << std << 
      ", quenum=" << total_query_num << 
      ", stime=\"" << (*i)[5] << "\", etime=NOW()";
  }
  query << " WHERE rank=" << (domain_index + 1);
  query.execute();
}
