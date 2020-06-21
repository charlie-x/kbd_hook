#include "stdafx.h"
#include <Windows.h>

#define _WIN32_WINNT 0x0603

LRESULT CALLBACK LowLevelKeyboardProc ( int nCode, WPARAM wParam, LPARAM lParam )
{
	BOOL fEatKeystroke = FALSE;

	if ( nCode == HC_ACTION ) {
		switch ( wParam ) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:

				PKBDLLHOOKSTRUCT p = ( PKBDLLHOOKSTRUCT ) lParam;

				if ( fEatKeystroke = ( p->vkCode == 'A' ) ) {

					if ( ( wParam == WM_KEYDOWN ) || ( wParam == WM_SYSKEYDOWN ) ) {
						keybd_event ( 'B', 0, 0, 0 );

					} else if ( ( wParam == WM_KEYUP ) || ( wParam == WM_SYSKEYUP ) ) {
						keybd_event ( 'B', 0, KEYEVENTF_UP, 0 );
					}

					break;
				}

				break;
		}
	}

	return ( fEatKeystroke ? 1 : CallNextHookEx ( NULL, nCode, wParam, lParam ) );
}


int main()
{
	// Get Number Of Devices
	UINT nDevices = 0;
	INT nResult;

	GetRawInputDeviceList ( NULL, &nDevices, sizeof ( RAWINPUTDEVICELIST ) );
	printf ( "number of raw devices = %d\n", nDevices );

	// uhhh.
	if ( nDevices == 0 ) { exit ( 1 ); }

	// Allocate Memory For Device List

	PRAWINPUTDEVICELIST pRawInputDeviceList = NULL;
	pRawInputDeviceList = new RAWINPUTDEVICELIST[sizeof ( RAWINPUTDEVICELIST ) * nDevices];

	if ( pRawInputDeviceList == NULL ) {

		printf ( "failed to allocate memory  %d\n", GetLastError() );
		exit ( 2 );
	}

	memset ( pRawInputDeviceList, 0, sizeof ( RAWINPUTDEVICELIST ) * nDevices );


	GetRawInputDeviceList ( pRawInputDeviceList, &nDevices, sizeof ( RAWINPUTDEVICELIST ) );

	// Got Device List?
	if ( nDevices < 0 ) {
		// Clean Up
		delete[] pRawInputDeviceList;
		printf ( "failed to get list of raw devices %d\n", GetLastError() );
		exit ( 3 );
	}

	// Loop Through Device List
	for ( UINT i = 0; i < nDevices; i++ ) {

		// Get Character Count For Device Name
		UINT nBufferSize = 0;

		nResult = GetRawInputDeviceInfo ( pRawInputDeviceList[i].hDevice, // Device
		                                  RIDI_DEVICENAME,                // Get Device Name
		                                  NULL,
		                                  &nBufferSize );

		if ( nResult < 0 ) {

			printf ( "ERR: Unable to get Device Name character count.. Moving to next device  %d\n", GetLastError() );

			continue;
		}


		// Allocate Memory For Device Name
		WCHAR* wcDeviceName = new WCHAR[nBufferSize + 1];

		if ( wcDeviceName == NULL ) {
			// Clean Up
			delete[] pRawInputDeviceList;
			printf ( "failed to allocate memory for raw device names %d\n", GetLastError() );
			exit ( 3 );

		}

		nResult = GetRawInputDeviceInfo ( pRawInputDeviceList[i].hDevice, // Device
		                                  RIDI_DEVICENAME,                // Get Device Name
		                                  wcDeviceName,                   // Get Name!
		                                  &nBufferSize );                // Char Count

		// Got Device Name?
		if ( nResult < 0 ) {
			// Error
			printf ( "ERR: Unable to get Device Name.. Moving to next device %d\n", GetLastError() );

			// Clean Up
			delete[] wcDeviceName;

			// Next
			continue;
		}

		// Set Device Info & Buffer Size
		RID_DEVICE_INFO rdiDeviceInfo;
		rdiDeviceInfo.cbSize = sizeof ( RID_DEVICE_INFO );
		nBufferSize = rdiDeviceInfo.cbSize;

		// Get Device Info
		nResult = GetRawInputDeviceInfo ( pRawInputDeviceList[i].hDevice,
		                                  RIDI_DEVICEINFO,
		                                  &rdiDeviceInfo,
		                                  &nBufferSize );

		// Got All Buffer?
		if ( nResult < 0 ) {
			// Error
			printf ( "ERR: Unable to read Device Info.. Moving to next device  %d\n", GetLastError() );

			// Next
			continue;
		}

		// Keyboard
		else if ( rdiDeviceInfo.dwType == RIM_TYPEKEYBOARD ) {
			// Current Device
			printf ( "Displaying device %d,information. (KEYBOARD)\n", i + 1 );
			wprintf ( L"Device Name: %s\n" , wcDeviceName );
			printf (  "Keyboard mode %d\n" , rdiDeviceInfo.keyboard.dwKeyboardMode );
			printf (  "Number of function keys: %d\n" , rdiDeviceInfo.keyboard.dwNumberOfFunctionKeys );
			printf (  "Number of indicators: %d\n" , rdiDeviceInfo.keyboard.dwNumberOfIndicators );
			printf (  "Number of keys total: %d\n" , rdiDeviceInfo.keyboard.dwNumberOfKeysTotal );
			printf ( "Type of the keyboard: %d\n" , rdiDeviceInfo.keyboard.dwType );
			printf ( "Subtype of the keyboard: %d\n" , rdiDeviceInfo.keyboard.dwSubType );
		}

		delete[] wcDeviceName;

	}

	RAWINPUTDEVICE rid;

	rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
	rid.usUsagePage = 1;
	rid.usUsage = 6;
	rid.hwndTarget = 0;
	nResult = RegisterRawInputDevices ( &rid, 1, sizeof ( rid ) );

	delete[] pRawInputDeviceList;

	printf ( "setting global hook\n" );

	HHOOK hhkLowLevelKybd = SetWindowsHookEx ( WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0 );

	MSG msg;

	while ( !GetMessage ( &msg, NULL, NULL, NULL ) ) {
		TranslateMessage ( &msg );
		DispatchMessage ( &msg );
	}

	UnhookWindowsHookEx ( hhkLowLevelKybd );

	return ( 0 );
}