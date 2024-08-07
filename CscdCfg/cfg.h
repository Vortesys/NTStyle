/* * * * * * * *\
	CFG.H -
		Copyright � 2024 Brady McDermott
	DESCRIPTION -
		Cascades' Service Configuration component header
	LICENSE INFORMATION -
		MIT License, see LICENSE.txt in the root folder
\* * * * * * * */

/* Pragmas */
#pragma once
#pragma comment(lib, "advapi32.lib")

/* Function Prototypes */
VOID WINAPI DisplayUsage(void);

VOID WINAPI DoQuerySvc(void);
BOOL WINAPI DoDisableSvc(void);
BOOL WINAPI DoEnableSvc(void);
VOID WINAPI DoDeleteSvc(void);

BOOL WINAPI DoStartSvc(void);
VOID WINAPI DoUpdateSvcDacl(void);
BOOL WINAPI DoStopSvc(void);

BOOL WINAPI StopDependentServices(void);
