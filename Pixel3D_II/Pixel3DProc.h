#pragma once
#include <deque>
#include <tuple>
#include <cstdint>
#include <string>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include "Surface256.h"
#include "WindowClass.h"

class Pixel3DProc :public Window::EventHandler {
public:
	//typedef std::tuple<std::uint8_t, std::uint8_t, std::uint8_t> ColorType;
	typedef std::tuple<std::uint64_t, std::uint64_t, std::uint64_t> Cood;
	typedef COLORREF ColorType;
	typedef Surface256<ColorType> Surface;
	typedef std::deque<Surface> Surfaces;

protected:
	enum class ControlID :int {
		None,
		Scroll01,
		Button_AddS,
		Button_DelS,
		Button_LoadS,
		Button_SaveS,
		Button_AddC,
		Button_DelC,

	};
	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {

		//static UINT BaseH = 0;
		static UINT W = 256;
		static UINT H = 256;

		static UINT SH = 16;//scrool 
		static UINT BH = 48;//button
		static UINT CBH = 16;//selected color.
		static UINT CH = 32;//color select

		switch (msg)
		{
		case WM_CREATE:
		{
			UINT BaseH = H + SH + BH + CH + CBH+BH;

			RECT rt = MakeClientRectForWindow(hWnd, W, BaseH);
			MoveWindow(hWnd, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, TRUE);

			Scroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_HORZ,
				0, 0, W, SH,
				hWnd, (HMENU)ControlID::Scroll01, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

			HWND BO1 = CreateWindow(_T("Button"), _T("�ʒǉ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, SH, W / 3, BH,
				hWnd, (HMENU)ControlID::Button_AddS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

			HWND BO2 = CreateWindow(_T("Button"), _T("�ʍ폜"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				W / 3, SH, W / 3, BH,
				hWnd, (HMENU)ControlID::Button_DelS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

			HWND BO3 = CreateWindow(_T("Button"), _T("�ʓǍ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				W / 3 * 2, SH, W / 3, BH/2,
				hWnd, (HMENU)ControlID::Button_LoadS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);
			HWND BO4 = CreateWindow(_T("Button"), _T("�ʏo��"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				W / 3 * 2, SH+BH/2, W / 3, BH/2,
				hWnd, (HMENU)ControlID::Button_SaveS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);
			HWND BO5 = CreateWindow(_T("Button"), _T("�F�ǉ�"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, SH + BH + CH + CBH, W / 2, BH,
				hWnd, (HMENU)ControlID::Button_AddC, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

			HWND BO6 = CreateWindow(_T("Button"), _T("�F�폜"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				W / 2, SH + BH + CH + CBH, W / 2, BH,
				hWnd, (HMENU)ControlID::Button_DelC, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

			Box.push_back({ static_cast<Surface::Cood>(W), static_cast<Surface::Cood>(H) });
			Sel = &Box[0];
			SetScrollRange(Scroll, SB_CTL, 0, 1, TRUE);
			EnableWindow(Scroll, FALSE);

			PenIndex = 1;
			Sel->GetPallete().push_back(RGB(0xff,0xff,0xff));
			Sel->GetPallete().push_back(RGB(0xff,0,0));
			Sel->GetPallete().push_back(RGB(0,0xff,0));
			Sel->GetPallete().push_back(RGB(0,0,0xff));	
			Sel->GetPallete().push_back(RGB(0xff,0xff,0));
			Sel->GetPallete().push_back(RGB(0xff,0,0xff));
			Sel->GetPallete().push_back(RGB(0xcc,0xcc,0xcc));

		}
		break;

		case WM_COMMAND: {
			switch (LOWORD(wp)) {
				case static_cast<INT>(ControlID::Button_AddS) :
				{
					int P = GetScrollPos(Scroll, SB_CTL);
					Surface* Te = &Box[P];
					Box.insert(Box.begin()+P,{ static_cast<Surface::Cood>(W), static_cast<Surface::Cood>(H) });	
					if(Box.size()>1)EnableWindow(Scroll,TRUE);	
					Sel = &Box[GetScrollPos(Scroll, SB_CTL)];
					if (PenIndex >= Sel->PaletteSize()) { PenIndex = 0; }
					Sel->GetPallete()=Te->GetPallete();
					break;
				}
				case static_cast<INT>(ControlID::Button_DelS) :
				{
					if (Box.size() <= 1) { break; }
					int P = GetScrollPos(Scroll, SB_CTL);

					Box.erase(Box.begin() + P);
					if (Box.size() <= 1) {
						EnableWindow(Scroll, FALSE); 
					}
					SetScrollPos(Scroll, SB_CTL, std::max<int>(0, P - 1), FALSE);
					Sel = &Box[GetScrollPos(Scroll, SB_CTL)];
					break;
				}
				case static_cast<INT>(ControlID::Button_LoadS) :
				{
					MessageBox(hWnd, _T("���߂�Ȃ����B"), _T("�������ł��B"), 0);
					//if (Box.size() == 1) { break; }
					//int P = GetScrollPos(Scroll, SB_CTL);
					//Box.erase(Box.begin() + P);
					////SetScrollRange(Scroll, SB_CTL, 0, Box.size(), TRUE);	
					//if (Box.size() <= 1) { EnableWindow(Scroll, FALSE); }	
					//Sel = &Box[GetScrollPos(Scroll, SB_CTL)];
					break;
				}
				case static_cast<INT>(ControlID::Button_SaveS):
				{
					SaveMQO(Box, "Output001.mqo");
					MessageBox(hWnd, _T("�Z�[�u���܂����B"), _T("�Z�[�u�I"), 0);
					//MessageBox(hWnd, _T("���߂�Ȃ����B"), _T("�������ł��B"), 0);
					break;
				}
				case static_cast<INT>(ControlID::Button_DelC) : {
					if (Sel->PaletteSize() <= 1) { break; }
					
					Sel->GetPallete().erase(Sel->GetPallete().begin()+PenIndex);
					PenIndex = 0;
					break;
				}
				break;
				case static_cast<INT>(ControlID::Button_AddC):
				{
					MessageBox(hWnd, _T("���߂�Ȃ����B"), _T("�������ł��B"), 0);		
					break;
				}

				break;
				default:
					break;
			}
		}
			SetScrollRange(Scroll, SB_CTL, 0, std::max<int>(0,static_cast<std::intmax_t>(Box.size())-1), FALSE);	
			RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
			break;
		case WM_HSCROLL: 

			SetScrollRange(Scroll, SB_CTL, 0, std::max<int>(0,static_cast<std::intmax_t>(Box.size())-1), FALSE);

			switch (LOWORD(wp)) {
				case SB_LEFT:
					SetScrollPos(Scroll, SB_CTL, 0, FALSE);
					break;

				case SB_RIGHT:
					SetScrollPos(Scroll, SB_CTL, Box.size(), FALSE);
					break;
				case SB_LINELEFT:
					{
						int N = GetScrollPos(Scroll, SB_CTL) - 1;
						if (N >= 0) {SetScrollPos(Scroll, SB_CTL, N, FALSE);}
					}
					break;
				
		
				case SB_LINERIGHT:
				{
					int N = GetScrollPos(Scroll, SB_CTL) + 1;
					if (N < Box.size()) {SetScrollPos(Scroll, SB_CTL, N, FALSE);	}
					break;
				}		
				case SB_PAGELEFT:
					{
						int N = GetScrollPos(Scroll, SB_CTL) - 1;
						if (N >= 0) {SetScrollPos(Scroll, SB_CTL, N, FALSE);}
					}		
					break;
				case SB_PAGERIGHT:
				{
					int N = GetScrollPos(Scroll, SB_CTL) + 1;
					if (N < Box.size()) {SetScrollPos(Scroll, SB_CTL, N, FALSE);	}
					break;
				}
				case SB_THUMBPOSITION:
					SetScrollPos(Scroll, SB_CTL, HIWORD(wp), FALSE);
					break;

				default:
					break;

			}
			Sel = &Box[GetScrollPos(Scroll, SB_CTL)];
			if (PenIndex >= Sel->PaletteSize()) { PenIndex = Sel->PaletteSize() - 1; }
			RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
			break;
		


		case WM_PAINT:
		{
			PAINTSTRUCT ps = { 0, };
			HDC hDC = BeginPaint(hWnd, &ps);


			HBRUSH B1 = CreateSolidBrush(Sel->IndexColor(PenIndex));
			HBRUSH Old1= SelectBrush(hDC, B1);
			Rectangle(hDC, 0, SH + BH, W, SH + BH + CBH);//Sel Color
			SelectBrush(hDC, Old1);
			DeleteBrush(B1);

			Rectangle(hDC, 0, SH + BH + CBH, W, SH + BH + CH + CBH);//pallete
			if (Sel != nullptr) {
				for (std::size_t i = 0; i < Sel->PaletteSize(); i++) {
					HBRUSH B = CreateSolidBrush(Sel->IndexColor(i));
					HBRUSH Old = SelectBrush(hDC, B);
					Rectangle(hDC, (W / Sel->PaletteSize()) * i, SH + BH + CBH, static_cast<int>(W / static_cast<double>(Sel->PaletteSize()) * (i + 1)), SH + BH + CH + CBH);
					SelectBrush(hDC, Old);
					DeleteBrush(B);
				}
			}
			Rectangle(hDC, 0, SH + BH + CH + CBH+BH, W, SH + BH + CH + CBH+BH + H);//canvas

			HDC mDC = CreateCompatibleDC(hDC);
			HBITMAP hBM = CreateCompatibleBitmap(hDC, W, H);

			HBITMAP OBM = SelectBitmap(mDC, hBM);

			for(int j=0;j<Sel->Height();j++){
				for (int i = 0; i < Sel->Width(); i++) {
					if (Sel->IndexData(i, j) != 0&& Sel->IndexData(i, j)<Sel->PaletteSize()) {
						SetPixel(mDC, i, /*SH + BH + CH + CBH + BH +*/ j, Sel->IndexColor(Sel->IndexData(i, j)));
					}
				}
			}
			BitBlt(hDC, 0, SH + BH + CH + CBH + BH, W, H, mDC, 0, 0, SRCCOPY);

			SelectBitmap(mDC, OBM);
			DeleteBitmap(hBM);
			DeleteDC(mDC);


/** /
			std::string s = std::to_string(GetScrollPos(Scroll, SB_CTL));
			RECT rt = { 0, };
			rt.left = 0;
			rt.top = SH + BH + CH + CBH+H-16;
			rt.right= W;
			rt.bottom = SH + BH + CH + CBH + H;
			DrawTextA(hDC, s.c_str(), -1, & rt, DT_CENTER);
/**/
			EndPaint(hWnd, &ps);

		}
		break;

		case WM_KEYUP:
			if (wp & VK_SPACE) {
				RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE|RDW_ALLCHILDREN);
			}
			break;
		case WM_LBUTTONUP:
				RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE);
			
			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEHOVER:
		case WM_MOUSEMOVE:
	
			
			if (wp & MK_LBUTTON) {
				if (Sel != nullptr) {
					if (GET_Y_LPARAM(lp) > (SH + BH + CH + CBH + BH)) {
						Sel->IndexData(GET_X_LPARAM(lp), GET_Y_LPARAM(lp) - (SH + BH + CH + CBH + BH)) = PenIndex;
					}
					else {

						INT X = (GET_X_LPARAM(lp));
						INT Y = (GET_Y_LPARAM(lp))-(SH + BH + CBH);
						if (0 > Y)break;
						if (CH < Y)break;

						if (W <= X)break;

						PenIndex = (GET_X_LPARAM(lp) / (W / static_cast<double>(Sel->PaletteSize())));
					}
				}
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, msg, wp, lp);
		}
		return 0;
	}
protected:
	RECT MakeClientRectForWindow(HWND hWnd, UINT W, UINT H) {

		RECT CRT = { 0, };
		GetClientRect(hWnd, &CRT);
		RECT WRT = { 0, };
		GetWindowRect(hWnd, &WRT);

		UINT CW = CRT.right - CRT.left;
		UINT CH = CRT.bottom - CRT.top;

		UINT WW = WRT.right - WRT.left;
		UINT WH = WRT.bottom - WRT.top;

		RECT R = { 0, };

		R.top = WRT.top;
		R.left = WRT.left;
		R.right = R.left + (WW - CW) + W;
		R.bottom = R.top + (WH - CH) + H;

		return R;

	}

bool SaveMQO(Pixel3DProc::Surfaces& Box, const std::string& Name) {
	int Line = 10;
	std::vector<Cood> Vertex;
	/** /
	std::vector<Cood> D{
			{Line,	Line*2,	Line*2},
			{Line,	Line,	Line*2},
			{Line*2,Line*2,	Line*2},
			{Line*2,Line,	Line*2},
			{Line*2,Line*2,	Line},
			{Line*2,Line,	Line},
			{Line,	Line*2,	Line},
			{Line,	Line,	Line},
	};
	std::vector<Cood> DD{
			{Line,	Line*2,	Line*2},
			{Line,	Line,	Line*2},
			{Line*2,Line*2,	Line*2},
			{Line*2,Line,	Line*2},
			{Line*2,Line*2,	Line},
			{Line*2,Line,	Line},
			{Line,	Line*2,	Line},
			{Line,	Line,	Line},
	};
	/**/
	std::vector<Cood> DD{
		{0	,	Line,	Line},
		{0	,	0	,	Line},
		{Line,	Line,	Line},
		{Line,	0	,	Line},
		{Line,	Line,	0  },
		{Line,	0	,	0  },
		{0	,	Line,	0  },
		{0	,	0	,	0  }
		};
	

	for (std::uintmax_t z = 0; z < Box.size(); z++) {
		for (std::intmax_t y = 0; y<Box[z].Height(); y++){
			for (std::size_t x = 0; x < Box[z].Width(); x++) {
				if (Box[z].IndexData(x, y) == 0) { continue; }
				for (auto& o : DD) {
					Vertex.push_back({ (std::get<0>(o) +(Line * (x + 1))),(std::get<1>(o) +(Line *(Box[z].Height()-(y + 1)))),(std::get<2>(o) +(Line * (z + 1))) });
				}
			}
		}

	}
	std::string Face;
	std::size_t x = 0;
	for (std::size_t i = 0; i < Vertex.size()/8; i++) {
		Face += "\t\t4 V(" + std::to_string(0 + (8 * i)) + ' ' + std::to_string(2 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ' ' + std::to_string(1 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		Face += "\t\t4 V(" + std::to_string(2 + (8 * i)) + ' ' + std::to_string(4 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		Face += "\t\t4 V(" + std::to_string(4 + (8 * i)) + ' ' + std::to_string(6 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		Face += "\t\t4 V(" + std::to_string(6 + (8 * i)) + ' ' + std::to_string(0 + (8 * i)) + ' ' + std::to_string(1 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		Face += "\t\t4 V(" + std::to_string(6 + (8 * i)) + ' ' + std::to_string(4 + (8 * i)) + ' ' + std::to_string(2 + (8 * i)) + ' ' + std::to_string(0 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		Face += "\t\t4 V(" + std::to_string(1 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ')' + " M(0) UV(0 0 1 0 1 1 0 1)\n";
		x+=6;
	}

	std::string Doc = "Metasequoia Document\t\t\nFormat Text Ver 1.0\n\n		Scene{\n			pos 0 0 1500\n			lookat 0 0 0\n			head - 0.5236\n			pich 0.5236\n			ortho 1\n			zoom2 5.0000\n			amb 0.250 0.250 0.250\n	}\n";


	Doc += "Material " + std::to_string(1) + " {\n";
	//do...
	Doc += "\t\"mat1\" col(1.000 1.000 1.000 1.000) dif(0.800) amb(0.600) emi(0.000) spc(0.000) power(5.00)\n";
	Doc += "}\n";

	Doc += "Object obj" + std::to_string(1) + " {\n";
	Doc += "\tvisible 15\n\tlocking 0\n\tshading 1\n\tfacet 59.5\n\tcolor 0.898 0.498 0.698\n\tcolor_type 0\n";
	Doc += "	vertex " + std::to_string(Vertex.size()) + " {\n";
	//do...
	for (auto& o : Vertex) {
		Doc += "\t\t" + std::to_string(static_cast<double>(std::get<0>(o))) + ' ' + std::to_string(static_cast<double>(std::get<1>(o))) + ' ' + std::to_string(static_cast<double>(std::get<2>(o))) + '\n';
	}
	Doc += "}\n";
	Doc += "\tface " + std::to_string(x) + " {\n";
	//do...
	Doc += Face;
	Doc += "}\n";
	Doc += "}\n";
	Doc += "Eof\n";


	std::ofstream ofs(Name);

	ofs << Doc;

	return true;

}
protected:
	HWND Scroll = nullptr;

	Surfaces Box;
	Surface* Sel = nullptr;
	std::size_t PenIndex = 0;

};

