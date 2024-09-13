// Includes
//=========

#include "../UserInput.h"

#include <Engine/Windows/Includes.h>
#include <Engine/Logging/Logging.h>

#include <windowsx.h>
//#include "../Mouse.h"


// Static Data
//============

namespace
{
	eae6320::UserInput::Mouse mouse;
}

// Interface
//==========

eae6320::UserInput::Mouse& eae6320::UserInput::GetMouse() {
	return mouse;
}

bool eae6320::UserInput::IsKeyPressed( const uint_fast8_t i_keyCode )
{
	const auto keyState = GetAsyncKeyState( i_keyCode );
	constexpr short isKeyDownMask = ~1;
	return ( keyState & isKeyDownMask ) != 0;
}

int eae6320::UserInput::MouseWindowContainer(UINT i_message, WPARAM i_wParam, LPARAM i_lParam)
{
	switch (i_message) {
	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnMouseMove(x, y);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnLeftPressed(x, y);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnRightPressed(x, y);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnMiddlePressed(x, y);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnLeftReleased(x, y);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnRightReleased(x, y);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		mouse.OnMiddleReleased(x, y);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int x = GET_X_LPARAM(i_lParam);
		int y = GET_Y_LPARAM(i_lParam);
		if (GET_WHEEL_DELTA_WPARAM(i_wParam) > 0)
		{
			mouse.OnWheelUp(x, y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(i_wParam) < 0)
		{
			mouse.OnWheelDown(x, y);
		}
		return 0;
	}
	default: return -1;
	
	}
}


