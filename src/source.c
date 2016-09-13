#include "header.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/* Add a prototype marked PGDLLEXPORT */
PGDLLEXPORT Datum current_user_sid();

PG_FUNCTION_INFO_V1(current_user_sid);

Datum current_user_sid() {
	DWORD dwSidSize = 0;
	TCHAR szDomain[128] = { 0 };
	DWORD dwDomainSize = 128;
	SID_NAME_USE SidType;
	PSID pSid;
	char *sidstr = NULL;

	char *user_name;

	user_name = current_user(NULL);
	
	LookupAccountNameA(NULL, user_name, NULL, &dwSidSize, szDomain,
		&dwDomainSize, &SidType);
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		pSid = (PSID)palloc(dwSidSize);
		if (LookupAccountNameA(NULL, user_name, pSid, &dwSidSize,
			szDomain, &dwDomainSize, &SidType)) {
			if (!ConvertSidToStringSidA(pSid, &sidstr)) {
				const char *format = "Could not perform account lookup for role %s";
				const char *msg = palloc(strlen(user_name) + strlen(format));
				sprintf(msg, format, user_name);
				reportError(GetLastError(), msg);
				pfree(msg);
			}

			VarChar *t = palloc(VARHDRSZ + strlen(sidstr));
			SET_VARSIZE(t, VARHDRSZ + strlen(sidstr));
			memcpy(t->vl_dat, sidstr, strlen(sidstr));

			PG_RETURN_VARCHAR_P(t);

			pfree(t);
		} else {
			const char *format = "Could not perform account lookup for role %s";
			const char *msg = palloc(strlen(user_name) + strlen(format));
			sprintf(msg, format, user_name);
			reportError(GetLastError(), msg);
			pfree(msg);
		}
		pfree(pSid);
	} else {
		const char *format = "Could not perform account lookup for role %s";
		const char *msg = palloc(strlen(user_name) + strlen(format));
		sprintf(msg, format, user_name);
		reportError(GetLastError(), msg);
		pfree(msg);
	}
}

void reportError(DWORD errorCode, const char *ownMsg) {
	LPTSTR msg = NULL;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 
		NULL, 
		errorCode, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		&msg, 
		0, 
		NULL);

	ereport(ERROR,
		(
		errcode(ERRCODE_SYSTEM_ERROR),
		errmsg(ownMsg),
		errdetail("%s", msg))
		);
}