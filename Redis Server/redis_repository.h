#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <cpprest/json.h>

class redis_data_repository {
    public : 
        void connect_To_Database(redisContext **c);
        // web::json::value getData(std::string ueid);
        web::json::value getData();
        void storeData();
};