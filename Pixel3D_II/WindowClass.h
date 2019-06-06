#pragma once
#include <memory>
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

//https://qiita.com/seekerkrt/items/2478c05c590c308146fd
//base code auther can use this.
//who can drop this talk.

class Window {
public:
	class EventHandler;
	typedef std::shared_ptr<EventHandler> SharedEventHandler;
public:
	Window();
	virtual ~Window();
public:	
	static const int MAX_LOADSTRING = 100;
	bool Create(HINSTANCE hInstance, const TCHAR (class_name)[],const TCHAR (app_name)[], int width, int height);
	bool IsCreated();   //ウィンドウが生成（生存）されているか確認
	HWND GetHWND() { return hWnd; }
	bool UpdateWindow();
	bool ShowWindow(int nCmdShow);
	void SetTitle(const TCHAR title[]);
	WPARAM GetMessageLoop(UINT wMsgFilterMin, UINT wMsgFilterMax);  //汎用かつ定番のメッセージループ
	WPARAM PeekMessageLoop(UINT wMsgFilterMin, UINT wMsgFilterMax,/* UINT wRemoveMSG,*/ UINT NanoSleep);    //特定用途メッセージループ


	bool AddStyle(LONG In) {
		LONG lStyle;
		lStyle = GetWindowLong(hWnd, GWL_STYLE);
		lStyle |= In;
		lStyle = SetWindowLong(hWnd, GWL_STYLE, lStyle);
		return true;
	}
	bool SubStyle(LONG In) {
		LONG lStyle;
		lStyle = GetWindowLong(hWnd, GWL_STYLE);
		lStyle &= ~In;
		lStyle = SetWindowLong(hWnd, GWL_STYLE, lStyle);
		return true;
	}

	bool SetClientArea(int W, int H) {
		RECT wrt = { 0, };
		RECT crt = { 0, };
		GetWindowRect(hWnd, &wrt);
		GetClientRect(hWnd, &crt);

		int BW = (wrt.right - wrt.left) - (crt.right-crt.left);
		int BH = (wrt.bottom - wrt.top) - (crt.bottom-crt.top);

		return TRUE == MoveWindow(hWnd, wrt.left, wrt.top, BW + W, BH + H, TRUE);
		
		
	}

	class EventHandler {
	public:
		virtual void InnerPeekMessage();//PeekMessageLoopで呼ばれる
		virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp); //デフォルトではDefWndProcを呼ぶかWM_DESTORYを処理するぐらい
		virtual ~EventHandler() {}
	};
	
	bool SetEventHandler(SharedEventHandler In);
	

public: //カスタマイズするメソッド
	virtual void InnerPeekMessage();//PeekMessageLoopで呼ばれる
	//virtual LRESULT LocalWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp); //デフォルトではDefWndProcを呼ぶかWM_DESTORYを処理するぐらい

private:
	static LRESULT CALLBACK BaseWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp); //ラップするためのトリックな関数
	void SetPointer(HWND hWnd); //windowオブジェクト(thisポインタ)とウィンドウハンドルを関連付けるためのトリックな関数

protected:
	volatile HWND hWnd = nullptr;
	SharedEventHandler EH = std::make_shared<EventHandler>();
};
