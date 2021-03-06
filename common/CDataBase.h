//
//	CDataBase
//		mySQL wrapper for easier data operations witheen in-game server
//
#ifndef CDATABASE_H
#define	CDATABASE_H

#include "../graysvr/graysvr.h"
#include <mysql.h>
#include <errmsg.h>	// mysql standart include
#ifdef _WIN32
	#pragma comment(lib, "libmySQL")
#else
	#pragma comment(lib, "libmysqlclient")
#endif

class CDataBase
{
public:
	class RowSet
	{
	public:
		RowSet(MYSQL *mySql, const char *query, int *res);
		~RowSet();

		//	set / get methods
		void	getString(char *buf, UINT column = 0, UINT row = 0);
		UINT	getUINT(UINT column = 0, UINT row = 0);
		double	getDouble(UINT column = 0, UINT row = 0);

		bool	isValid();						//	is query result valid?
		UINT	getRows();						//	get rows in query

	protected:
		bool		m_bValid;
		MYSQL_RES	*m_res;						//	SELECT result
		MYSQL		*m_sql;
	};

	//	construction
	CDataBase();
	~CDataBase();
	bool Connect(const char *user, const char *password, const char *base = "", const char *host = "localhost");
	void Close();							//	close link with db

	//	select
	RowSet	query(const char *query);			//	proceeds the query for SELECT
	RowSet	__cdecl queryf(char *fmt, ...);
	void	exec(const char *query);			//	executes query (pretty faster) for ALTER, UPDATE, INSERT, DELETE, ...
	void	__cdecl execf(char *fmt, ...);

	//	set / get / info methods
	bool	isConnected();
	UINT	getLastId();					//	get last id generated by auto-increment field

	void	OnTick();

protected:
	bool	_bConnected;					//	are we online?
	MYSQL	*_myData;						//	mySQL link
};

#endif