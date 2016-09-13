#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "server/common/username.h"
#include "server/utils/builtins.h"

#include <ntsecapi.h>
#include <Sddl.h>

void reportError(DWORD errorCode, const char *ownMsg);