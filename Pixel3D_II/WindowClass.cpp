#include <thread>
#include <chrono>
#include "WindowClass.h"

//コンストラクタ
Window::Window() {

}
//デストラクタ
Window::~Window() {

}

bool Window::Create(HINSTANCE hInstance,const TCHAR class_name[],const TCHAR app_name[], int width, int height) {
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize = sizeof(WNDCLASSEX);   //この構造体自体のサイズなので。
	wcex.cbClsExtra = 0;    //拡張用？とりあえず使わないので0
	wcex.cbWndExtra = 0;    //拡張用？とりあえず使わないので0
	wcex.lpfnWndProc = Window::BaseWndProc;//このウィンドウクラスのプロシージャを指定。このソースコードではこうする
	wcex.hInstance = hInstance;
	wcex.lpszClassName = class_name;
	wcex.lpszMenuName = nullptr;    //メニューの名前。使わないならとりあえずNULLでOK
	wcex.style = CS_VREDRAW | CS_HREDRAW;
	wcex.hCursor = ::LoadCursor(hInstance, IDC_ARROW);      //カーソルアイコン
	wcex.hIcon = ::LoadIcon(hInstance, IDI_APPLICATION);    //プログラムアイコン
	wcex.hIconSm = ::LoadIcon(hInstance, IDI_APPLICATION);  //プログラムアイコン（小）::タイトルバーに使われるやつ？
	wcex.hbrBackground = GetStockBrush(WHITE_BRUSH);      //クライアント領域の塗りつぶし色（ブラシ）
//↑とかは特に、メソッドの引数を受け取ってカスタマイズできるようにするのが望ましいかな                                                 
			//ATOMへWNDCLASSEX登録
	if (!::RegisterClassEx(&wcex)) {
		return false;
	}
	//ウィンドウ生成
	HWND ret = ::CreateWindow(
		class_name,
		app_name,
		WS_OVERLAPPEDWINDOW,    //ウィンドウスタイル。とりあえずデフォルトな感じで
		CW_USEDEFAULT, CW_USEDEFAULT,   //初期位置。適当にやってくれる。
		width, height,      //ウィンドウサイズ
		nullptr,    //親ウィンドウのハンドル。特にないんで今回はNULL
		nullptr,    //メニューハンドル。特にないので今回はNULL
		hInstance,
		this    //トリックの肝。CreateParameterに設定
	);
	//ウィンドウ生成に失敗？
	if (ret == nullptr) {
		return false;
	}
	//成功したのでtrueを返して抜ける
	return true;
}

bool Window::IsCreated()
{
	if (this->hWnd == nullptr) {
		return false;
	}
	else {
		return true;
	}
}

bool Window::UpdateWindow() {
	if (this->hWnd == nullptr) {
		return false;
	}
	::UpdateWindow(this->hWnd);
	return true;
}

bool Window::ShowWindow(int nCmdShow) {
	if (this->hWnd == nullptr) {
		return false;
	}
	::ShowWindow(this->hWnd, nCmdShow);
	return true;
}

void Window::SetTitle(const TCHAR title[MAX_LOADSTRING]) {
	::SetWindowText(hWnd, title);
}

WPARAM Window::GetMessageLoop(UINT wMsgFilterMin, UINT wMsgFilterMax) {
	MSG msg;
	while (::GetMessage(&msg, nullptr, wMsgFilterMin, wMsgFilterMax) > 0) {  //「-1」が返ってくるかもしれないのでこうする
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	this->hWnd = nullptr; //ココに記述しておいてみる
	return msg.wParam;
}
WPARAM Window::PeekMessageLoop(UINT wMsgFilterMin, UINT wMsgFilterMax/*, UINT wRemoveMSG*/,UINT NanoSleep) {
	MSG msg;
	do {
		//既定の処理はプロシージャなどに任せて……
		if (::PeekMessage(&msg, nullptr, wMsgFilterMin, wMsgFilterMax, PM_REMOVE)) {//if 2nd param not null to cant get WM_QUIT.
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		//そうでない処理は自前でする
		else {
			//自前の処理であるInnerPeekMessage()を呼び出し
			this->InnerPeekMessage();
		}
		std::this_thread::sleep_for(std::chrono::nanoseconds(NanoSleep));
	} while (msg.message != WM_QUIT);//イベントドリブン処理ループを抜けるまで実行
	this->hWnd = nullptr; //ココに記述しておいてみる
	return msg.wParam;
}

void Window::InnerPeekMessage() {
	//TODO:InnerPeekMessage()
	EH->InnerPeekMessage();
}
/** /
LRESULT Window::LocalWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case(WM_DESTROY):
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}
/**/
LRESULT CALLBACK Window::BaseWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	//Window::SetPointerでセットしたthisポインタを取得
	Window* window = (Window*)(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	//取得に失敗してる場合
	if (window == nullptr) {
		//おそらくWM_CREATEの最中なので
		if (msg == WM_CREATE) {
			//CreateWindowのパラメータから取得する
			window = (Window*)(((LPCREATESTRUCT)lp)->lpCreateParams);
		}
		//↑の処理で取得できてるはずなんだけど、一応チェックしてから
		if (window != nullptr) {
			//windowオブジェクトとウィンドウハンドルを関連付ける
			window->SetPointer(hWnd);
		}
	}
	//無事取得できててる
	if (window != nullptr) {
		//そのオブジェクトによるウィンドウプロシージャ実装を使うというか振り分ける
		return window->EH->WndProc(hWnd, msg, wp, lp);
	}
	//よくわからんけど、それでも取得できたりしてない場合（以下のようにするよりも例外作って投げるのがよさそうだがｗ
	else {
		return ::DefWindowProc(hWnd, msg, wp, lp);
	}
}
void Window::SetPointer(HWND hWnd) {
	::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	this->hWnd = hWnd;
}

bool Window::SetEventHandler(SharedEventHandler In) {
	this->EH = In;
	return true;
}
void Window::EventHandler::InnerPeekMessage() {
	//TODO:InnerPeekMessage()
	
}
LRESULT Window::EventHandler::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case(WM_DESTROY):
		::PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}