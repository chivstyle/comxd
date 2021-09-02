//
// (c) 2021 chiv
//
#include "Database.h"
#include "s_exception.h"

#define TAG "<Database>:"

using namespace Upp;

#define SCHEMADIALECT <plugin/sqlite3/Sqlite3Schema.h>
#define MODEL <actuatord/actuatord.sch>
#include <Sql/sch_schema.h>
#include <Sql/sch_header.h>
#include <Sql/sch_source.h>

static inline void _initdb()
{
	SqlSchema sch(SQLITE3);
	All_Tables(sch);
	SqlPerformScript(sch.Upgrade());
	SqlPerformScript(sch.Attributes());
	SqlPerformScript(sch.ConfigDrop());
	SqlPerformScript(sch.Config());
	sch.SaveNormal();
}

Database::Database(const Value& conf)
{
	mSqlite3.LogErrors(true);
	LOG(TAG << "Connecting to:" << conf["URL"]);
	if (!mSqlite3.Open(conf["URL"].ToString())) {
		throw s_exception("Failed to open:\"%s\"", conf["URL"].ToString().Begin());
	}
	SQL = mSqlite3;
	SQL * Select(SqlId("tbl_name")).From(SqlId("sqlite_master")).Where(SqlId("tbl_name") == "DEVICE_STATUS");
	if (!SQL.Fetch()) {
		LOG(TAG << "_initdb for first use");
		_initdb();
	}
}

Database::~Database()
{
}

static inline String _SysTime()
{
	Time t = GetSysTime();
	return Upp::Format("%04d-%02d-%02d %02d:%02d:%02d",
		t.year, t.month, t.day, t.hour, t.minute, t.second);
}

bool Database::Save(const Upp::Value& req, const Upp::Value& device_status)
{
	try {
	    String user = req["userName"].ToString();
	    SQL & Insert(DEVICE_STATUS)(USER, user)(DATE_TIME, _SysTime())
	        (REQUEST, AsJSON(req))(STATUS, AsJSON(device_status));
	    //
	    return true;
	} catch (const String& err) {
		LOG(TAG << "Failed to save:" << err);
	}
	return false;
}
