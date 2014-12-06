#include<stdio.h>
#include<sqlite3.h>
#include<iostream>
#include<map>
#include<vector>


/*
Firestream: A Database Designed For Streaming Data

Notes:
DB has many partitions
Partitions in a DB many host N tables where N >= 1

*/

int numberSeconds;


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    std::string str = "";
    for(i=0; i<argc; i++){
        string arg = argv[i] ? argv[i] : "NULL";
        str +=  arg+"\t";
    }
    str += "\n";
    result.push_back(str);
    return 0;
}


class Partition {
    public:
       Partition(int pID, sqlite3 * dBase, int tSize);
       void lock();
       void unlock(); 
       int maxTableSize;
       bool locked;
       sqlite3 * db;
       int partitionID;
};

Partition::Partition(int pID, sqlite3 * dBase, int tSize){
    this->maxTableSize = tSize;
    this->db = dBase;
    this->partitionID = pID;
    this->locked = false;
}

void Partition::lock(){
    this->locked = true;
}

void Partition::unlock(){
    this->locked = false;
}

/* All partitions  stored here*/
std::vector<Partition> pList;

/* Results of last query stored here, row by row */
std::vector<string>results;

void createPartition(int partitionID, const char * partitionName, sqlite3 * db, int maxTableSize){
    int message;
    message = sqlite3_open(partitionName, &db);
    if( message ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	exit(0);
    }else{
	fprintf(stderr, "Opened database successfully\n");
    }
    Partition p(partitionID, db, maxTableSize);
    pList.push_back(p);
}

void createDB(int numberOfPartitions, int numSeconds, int tSize){
    for(int i = 0; i< numberOfPartitions; i++){
	const char * pName = "partition" + std::to_string(i);
        sqlite3 * db;
	std::string dName = "db" + std::to_string(i);
        createPartition(i, pName, db, tSize);
    }
    numberSeconds = numSeconds;
}

/* Function called to execute query */
void execQuery(std::string SQLquery){
    char *zErrMsg = 0;
    std::vector<Partition>::iterator itr;
    for ( itr = pList.begin(); itr < pList.end(); ++itr )
    {
        if(*itr->locked == false){
            
            int status;
            status = sqlite3_exec(*itr->db, SQLquery, callback, 0, &zErrMsg);
            if( status != SQLITE_OK ){
                fprintf(stderr, "SQL error on partition %d: %s\n", *itr->partitionID, zErrMsg);
                sqlite3_free(zErrMsg);
            }else{
                fprintf(stdout, "Query executed successfully on partition %d \n", *itr->partitionID);
            }

        }
    }
}

/* Close connection to SQLite3 database */
void closeTable(sqlite3 * db){
    sqlite3_close(db);
}


void lockPartition(int partitionID){
    pList[partitionID].lock();
}

void unlockPartition(int partitionID){
    pList[partitionID].unlock();
}

int howManyPartitions(){
    return pList.size();
}

 

std:string printResults(){
    vector<std::string>::iterator row;
    for (row = results.begin(); row != results.end(); row++) {
            printf(row);
    }
}


int main(){

    cout << "Welcome to the Firestream console";
    cout << "Firestream is a DB designed for streaming data \n \n";

    while(true){
        std:string = query;
        cout << "Please enter an SQL Query:";
        cin >> query;
        execQuery(query)
        cout << printResults;
    }

    return 0;
}
