/* * * * * * * *\
	NTSTYLE.C -
		Copyright � 2023 Brady McDermott, Vortesys
	DESCRIPTION -
		Defines the entry point for the DLL application.
	LICENSE INFORMATION -
		MIT License, see LICENSE.txt in the root folder
 \* * * * * * * */

// Includes
#include "ntshook.h"
#include <strsafe.h>

// System Metrics
static INT g_iCaptionHeight;
static INT g_iBorderWidth;
static INT g_iBorderHeight;
static INT g_iDlgFrmWidth;
static INT g_iDlgFrmHeight;

// System Colors
static DWORD g_dwActiveCaption;
static DWORD g_dwActiveCaptionGradient;
static DWORD g_dwActiveCaptionText;
static DWORD g_dwActiveBorder;
static DWORD g_dwInactiveCaption;
static DWORD g_dwInactiveCaptionGradient;
static DWORD g_dwInactiveCaptionText;
static DWORD g_dwInactiveBorder;
static DWORD g_dwWindowFrame;

/* * * *\
	DllMain -
		NT Style Hook's entry point
\* * * */
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

/* * * *\
	NTStyleHookProc -
		NT Style Hook procedure
		Uses WH_CALLWNDPROC.
\* * * */
__declspec(dllexport) LRESULT NTStyleHookProc(
	_In_ UINT uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	PCWPSTRUCT pcwps;

	// Switch case for window painting
	switch (uMsg)
	{
	/* BEGIN HC_ACTION */
	case HC_ACTION:
		pcwps = (CWPSTRUCT*)lParam;

		switch (pcwps->message)
		{
		case WM_DISPLAYCHANGE:
		case WM_SYNCPAINT:
		case WM_ACTIVATE:
		case WM_CREATE:
		case WM_PAINT:
		case WM_MOVE:
			NTStyleDrawWindow(pcwps->hwnd, pcwps->wParam, pcwps->lParam);
			break;

		case WM_NCACTIVATE:
		case WM_NCCALCSIZE:
		case WM_NCPAINT:
			NTStyleDrawWindow(pcwps->hwnd, pcwps->wParam, pcwps->lParam);
			return 0;

		default:
			break;
		}
	/* END HC_ACTION */

	default:
		break;
	}

	return CallNextHookEx(NULL, uMsg, wParam, lParam);
}

/* * * *\
	NTStyleDrawWindow -
		Draws window using the
		DrawWindow* helper
		functions.
\* * * */
VOID NTStyleDrawWindow(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// Always refresh the colors and metrics before drawing
	NTStyleGetWindowMetrics();

	// Make sure we're only painting windows with titlebars
	if ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_SYSMENU) == WS_SYSMENU)
	{
		// Draw the window
		NTStyleDrawWindowCaption(hWnd, wParam, lParam);
		NTStyleDrawWindowBorders(hWnd, wParam, lParam);
		NTStyleDrawWindowButtons(hWnd, wParam, lParam);
	}

	return;
}

/* * * *\
	NTStyleDrawWindowBorders -
		Draws window borders
\* * * */
VOID NTStyleDrawWindowBorders(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	/* The system maintains an update region for the nonclient area.
	When an application receives a WM_NCPAINT message, the wParam
	parameter contains a handle to a region defining the dimensions of
	the update region. The application can use the handle to combine the
	update region with the clipping region for the window device context.
	The system does not automatically combine the update region when
	retrieving the window device context unless the application uses
	GetDCEx and specifies both the region handle and the DCX_INTERSECTRGN
	flag. If the application does not combine the update region, only
	drawing operations that would otherwise extend outside the window are
	clipped. The application is not responsible for clearing the update
	region, regardless of whether it uses the region. */

	HDC hdc = NULL;

	// Verify our window handle
	if (hWnd != NULL)
		hdc = GetWindowDC(hWnd);

	// Verify the device context
	if (hdc)
	{
		HBRUSH hbr = NULL;
		HPEN hpn = NULL;

		WINDOWINFO wi;
		BOOL bIsActiveWindow = FALSE;
		INT iBorderColor = 0;

		UINT uiw = 0;
		UINT uih = 0;

		INT i;

		// Get external window size
		wi.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hWnd, &wi);

		// Check whether or not we're the active window
		// and change colors based on that
		bIsActiveWindow = wi.dwWindowStatus == WS_ACTIVECAPTION;
		iBorderColor = bIsActiveWindow ? COLOR_ACTIVEBORDER : COLOR_INACTIVEBORDER;

		// Get window width, height and border size
		uiw = wi.rcWindow.right - wi.rcWindow.left;
		uih = wi.rcWindow.bottom - wi.rcWindow.top;

		g_iBorderHeight = wi.cyWindowBorders - 1;
		g_iBorderWidth = wi.cxWindowBorders - 1;

		// Draw the colored rectangles around the edges
		i = 0;

		for (i = 0; i < 4; i++)
		{
			INT iModX = (i & 1) == 1;
			INT iModY = (i & 2) == 2;

			RECT rc = { 0, 0, 0, 0 };

			rc.left = (i == 2) * (uiw - g_iBorderWidth - 1);
			rc.top = (i == 3) * (uih - g_iBorderHeight - 1);
			rc.right = (i == 0) ? g_iBorderWidth + 1 : uiw;
			rc.bottom = (i == 1) ? g_iBorderHeight + 1 : uih;

			// Draw rectangle
			hbr = GetSysColorBrush(iBorderColor);
			FillRect(hdc, &rc, hbr);

			// Draw the frame
			hbr = GetSysColorBrush(COLOR_WINDOWFRAME);
			FrameRect(hdc, &rc, hbr);
		}

		// Set up our brushes
		hbr = GetSysColorBrush(iBorderColor);
		hpn = CreatePen(PS_SOLID, 0, (COLORREF)GetSysColor(COLOR_WINDOWFRAME));

		SelectObject(hdc, hbr);
		SelectObject(hdc, hpn);

		// Paint the "caps"
		i = 0;

		for (i = 0; i < 4; i++)
		{
			INT iFlipX = (i & 1) == 1;
			INT iFlipY = (i & 2) == 2;
			UINT uiFlipX = (iFlipX ? -1 : 1);
			UINT uiFlipY = (iFlipY ? -1 : 1);

			POINT apt[7] = { 0, 0 };

			// Calculate the polygon points
			apt[0].x = 0 + uiw * iFlipX - iFlipX;
			apt[0].y = 0 + uih * iFlipY - iFlipY;
			apt[1].x = apt[0].x + uiFlipX * (g_iCaptionHeight + g_iBorderWidth);
			apt[1].y = apt[0].y;
			apt[2].x = apt[1].x;
			apt[2].y = apt[0].y + uiFlipY * g_iBorderHeight;
			apt[3].x = apt[1].x - uiFlipX * g_iCaptionHeight;
			apt[3].y = apt[2].y;
			apt[4].x = apt[3].x;
			apt[4].y = apt[2].y + uiFlipY * (g_iCaptionHeight - 1);
			apt[5].x = apt[0].x;
			apt[5].y = apt[4].y;

			// These are necessary to close the gap
			apt[6].x = apt[0].x;
			apt[6].y = apt[0].y;

			// Draw
			SetPolyFillMode(hdc, 0);
			Polygon(hdc, apt, sizeof(apt) / sizeof(apt[0]));
		}

		// Cleanup
		if (hbr)
			DeleteObject(hbr);
		if (hpn)
			DeleteObject(hpn);

		ReleaseDC(hWnd, hdc);
	}

	return;
}

/* * * *\
	NTStyleDrawWindowCaption -
		Draws window caption & title
\* * * */
VOID NTStyleDrawWindowCaption(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	/* If an application processes the WM_NCACTIVATE message, after
	processing it must return TRUE to direct the system to complete
	the change of active window. If the window is minimized when the
	application receives the WM_NCACTIVATE message, it should pass
	the message to DefWindowProc. In such cases, the default function
	redraws the label for the icon. */

	HDC hdc = NULL;

	// Verify our window handle
	if (hWnd != NULL)
		hdc = GetWindowDC(hWnd);

	// Verify the device context
	if (hdc)
	{
		HBRUSH hbr = NULL;
		HFONT hft = NULL;

		WINDOWINFO wi;
		BOOL bIsActiveWindow = FALSE;
		INT iGradientCaptionColor = 0;
		INT iCaptionColor = 0;
		INT iCaptionTextColor = 0;

		RECT rc = { 0, 0, 0, 0 };
		UINT uiw = 0;

		INT cTxtLen = 0;
		LPWSTR pszTxt;

		// Get external window size
		wi.cbSize = sizeof(WINDOWINFO);
		GetWindowInfo(hWnd, &wi);

		// Check whether or not we're the active window
		// and change colors based on that
		bIsActiveWindow = wi.dwWindowStatus == WS_ACTIVECAPTION; // GetForegroundWindow() == hWnd;
		iGradientCaptionColor = bIsActiveWindow ? COLOR_GRADIENTACTIVECAPTION : COLOR_GRADIENTINACTIVECAPTION;
		iCaptionColor = bIsActiveWindow ? COLOR_ACTIVECAPTION : COLOR_INACTIVECAPTION;
		iCaptionTextColor = bIsActiveWindow ? COLOR_CAPTIONTEXT : COLOR_INACTIVECAPTIONTEXT;

		// Get window width
		uiw = wi.rcWindow.right - wi.rcWindow.left;

		// Always refresh the colors and metrics before drawing
		NTStyleGetWindowMetrics();

		g_iBorderHeight = wi.cyWindowBorders - 1;
		g_iBorderWidth = wi.cxWindowBorders - 1;

		// Calculate the rect points
		rc.left = g_iBorderWidth - 1;
		rc.top = g_iBorderHeight;
		rc.right = uiw - rc.left;
		rc.bottom = rc.top + g_iCaptionHeight;

		// Draw the caption rectangle
		// TODO: do it with a gradient
		hbr = GetSysColorBrush(iCaptionColor);
		FillRect(hdc, &rc, hbr);

		// Draw the frame around it
		hbr = GetSysColorBrush(COLOR_WINDOWFRAME);
		FrameRect(hdc, &rc, hbr);

		// Get the font object
		/* It is not recommended that you employ this method
		to obtain the current font used by dialogs and windows.
		Instead, use the SystemParametersInfo function with the
		SPI_GETNONCLIENTMETRICS parameter to retrieve the current
		font. SystemParametersInfo will take into account the
		current theme and provides font information for captions,
		menus, and message dialogs. */
		hft = (HFONT)GetStockObject(SYSTEM_FONT);

		// Set up our HDC for the text
		SetTextColor(hdc, (COLORREF)GetSysColor(iCaptionTextColor));
		SetBkColor(hdc, (COLORREF)GetSysColor(iCaptionColor));

		SelectObject(hdc, hbr);
		SelectObject(hdc, hft);

		// Calculate the rect points for the text
		rc.left = rc.left + g_iCaptionHeight;
		rc.right = rc.right - g_iCaptionHeight;

		// Allocate memory for the caption text
		cTxtLen = GetWindowTextLength(hWnd);

		pszTxt = (LPWSTR)VirtualAlloc((LPVOID)NULL,
			(DWORD)(cTxtLen + 1), MEM_COMMIT,
			PAGE_READWRITE);

		// Get the caption text
		GetWindowText(hWnd, pszTxt, cTxtLen + 1);

		// Draw the caption text
		ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED, &rc, pszTxt, cTxtLen + 1, NULL);

		// Cleanup
		if (pszTxt)
			VirtualFree(pszTxt, 0, MEM_RELEASE);

		if (hbr)
			DeleteObject(hbr);

		if (hft)
			DeleteObject(hft);

		ReleaseDC(hWnd, hdc);
	}

	return;
}

/* * * *\
	NTStyleDrawWindowButtons -
		Draws window buttons
\* * * */
VOID NTStyleDrawWindowButtons(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	return;
}

/* * * *\
	NTStyleGetWindowMetrics -
		Gathers system colors
		and metrics for use with
		the drawing functions.
\* * * */
VOID NTStyleGetWindowMetrics(VOID)
{
	// First get the system metrics.
	g_iCaptionHeight = GetSystemMetrics(SM_CYCAPTION); // Caption bar height
	g_iDlgFrmWidth = GetSystemMetrics(SM_CXDLGFRAME); // Dialog frame width
	g_iDlgFrmHeight = GetSystemMetrics(SM_CYDLGFRAME); // Dialog frame height

	// And the window frame.
	g_dwWindowFrame = GetSysColor(COLOR_WINDOWFRAME); // Window frame color

	return;
	//GetSystemMetrics(SM_SWAPBUTTON); i will prolly have to think about this later
}