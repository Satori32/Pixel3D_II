#include <windows.h>

#include "WindowClass.h"
#include "Pixel3DProc.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Window W;
	Window::SharedEventHandler WE = std::make_shared<Pixel3DProc>();
	W.SetEventHandler(WE);

	const TCHAR Name[] = _T("Dot.HELL");
	const TCHAR Title[] = _T("Dot.HELL");

	W.Create(hInstance, Name, Title, 800, 600);

	W.SubStyle(WS_MAXIMIZEBOX);
	//W.SubStyle(WS_THICKFRAME);

	W.ShowWindow(nCmdShow);
	W.UpdateWindow();

	//W.SetClientArea(256, 32+16 + 256);
	
	return W.GetMessageLoop(0,0);
}