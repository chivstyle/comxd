#ifndef _actuatord_Database_h_
#define _actuatord_Database_h_

#include <plugin/sqlite3/Sqlite3.h>

class Database {
public:
	Database(const Upp::Value& conf);
	virtual ~Database();
	// save the request and the current status of device to database
	// this routine will add USER and DATE_TIME to table
	bool Save(const Upp::Value& req, const Upp::Value& device_status);
	//
private:
	Upp::Sqlite3Session mSqlite3;
};

#endif
