#pragma once
#include <deque>
#include <unordered_map>
#include <tuple>
#include <cstdint>
#include <string>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include "Surface256.h"
#include "WindowClass.h"
#include "StringMultiByteAndWideChar.h"

//#pragma comment(lib,"Comdlg32.lib")

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


		switch (msg)
		{
		case WM_CREATE:
			WMCreate(hWnd, msg, wp, lp);
			break;

		case WM_GETMINMAXINFO:
			WMGetMinMaxInfo(hWnd, msg, wp, lp);
			break;

		case WM_COMMAND:
			WMCommand(hWnd, msg, wp, lp);
			break;
		case WM_HSCROLL:
			WMHScroll(hWnd, msg, wp, lp);
			break;
		case WM_PAINT:
			WMPaint(hWnd, msg, wp, lp);
			break;

		case WM_KEYUP:
			if (wp & VK_SPACE) {
				RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			break;
		case WM_RBUTTONUP:
			RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

			break;
		case WM_LBUTTONDOWN:
		case WM_MOUSEMOVE:


			if (wp & MK_LBUTTON) {
				if (Sel != nullptr) {
					if (GET_Y_LPARAM(lp) > (SH + BH * 2 + CH + CBH)) {
						int X = GET_X_LPARAM(lp);
						int Y = GET_Y_LPARAM(lp);
						int Pad = 2;
						Sel->IndexData(X, Y - (SH + BH * 2 + CH + CBH)) = PenIndex;
						RECT rtx = { X - Pad, Y - Pad,X + Pad,Y + Pad };
						InvalidateRect(hWnd, &rtx, TRUE);
						//RedrawWindow(hWnd, &rtx, nullptr, RDW_UPDATENOW|RDW_NOCHILDREN);
					}
					else {

						INT X = (GET_X_LPARAM(lp));
						INT Y = (GET_Y_LPARAM(lp)) - (SH + BH + CBH);
						if (0 > Y)break;
						if (CH < Y)break;

						if (W <= X)break;

						PenIndex = (GET_X_LPARAM(lp) / (W / static_cast<double>(Sel->PaletteSize())));
						RECT rt = { 0,SH + BH,W,SH + BH + CH + CBH };
						RedrawWindow(hWnd, &rt, nullptr, RDW_INVALIDATE |RDW_UPDATENOW | RDW_NOCHILDREN);
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

protected:
	std::tuple<bool, Surface> ReadBitmap256(const std::string& Name) {
		BITMAPFILEHEADER BMF = { 0, };
		BITMAPINFOHEADER BMH = { 0, };
		WORD WO = 0;
		DWORD DWO = 0;

		std::ifstream ifs(Name, std::ios::binary);
		if (!ifs.is_open()) { return { false ,{} }; }

		ifs.read((char*)& BMF.bfType, sizeof(WORD));//sig
		if (BMF.bfType != 'B' + ('M' << 8)) { return { false,{} }; }
		ifs.read((char*)& DWO, sizeof(DWORD));//fsize.
		ifs.read((char*)& WO, sizeof(WORD));	//resurved	
		ifs.read((char*)& WO, sizeof(WORD));// resurved
		ifs.read((char*)& DWO, sizeof(DWORD));//offbits.	

		ifs.read((char*)& BMH, sizeof(BITMAPINFOHEADER));//try read once.
		if (BMH.biSize != 40) { return{ false,{} }; }
		if (BMH.biBitCount != 8) { return{ false,{} }; }
		if (BMH.biClrUsed == 0) { BMH.biClrUsed = 255; }

		Surface S(BMH.biWidth, BMH.biHeight);
		S.GetPallete().clear();
		RGBQUAD RQ = { 0, };

		for (DWORD i = 0; i < BMH.biClrUsed; i++) {
			ifs.read((char*)& RQ, sizeof(RGBQUAD));//read Pallete.
			S.GetPallete().push_back(RGB(RQ.rgbRed, RQ.rgbGreen, RQ.rgbBlue));
		}
		BYTE* p = new BYTE[S.Width() * S.Height()];
		if (!p) { return { false,{} }; }
		ifs.read((char*)p, S.Width() * S.Height());

		for (std::intmax_t y = 0; y < S.Height(); y++) {
			for (std::size_t x = 0; x < S.Width(); x++) {
				S.IndexData(x, y) = p[(S.Height()-y-1) * S.Height() + x];
			}
		}
		delete[] p;
		Surface R(256, 256);
		R.GetPallete().clear();
		for (std::size_t i = 0; i < std::min<size_t>(S.PaletteSize(), 256); i++) { 
			R.GetPallete().push_back(S.IndexColor(i));
		}
		
		for (std::size_t y = 0; y < R.Height(); y++) {
			for (std::size_t x = 0; x < R.Width(); x++) {
				R.IndexData(x, y) = S.IndexData(x, y);
			}
		}
		return { true,R };


	}
protected:
	bool SaveBitmap256(Pixel3DProc::Surface& S, const std::string& Name) {
		BITMAPFILEHEADER BMH = { 0, };
		BITMAPINFOHEADER BMI = { 0, };

		BMH.bfType = 'B' | 'M' << 8;
		BMH.bfSize = 0;
		BMH.bfOffBits = 0;

		BMI.biSize = sizeof(BMI);
		BMI.biWidth = S.Width();
		BMI.biHeight = S.Height();
		BMI.biBitCount = 8;
		BMI.biPlanes = 1;
		BMI.biCompression = 0;
		BMI.biSizeImage = S.GetData().size();
		BMI.biXPelsPerMeter = 0;
		BMI.biYPelsPerMeter = 0;
		BMI.biClrUsed = S.PaletteSize();
		BMI.biClrImportant = 0;

		BMH.bfOffBits = sizeof(BMH) + sizeof(BMI) + BMI.biClrUsed * 4;
		BMH.bfSize = BMH.bfOffBits + S.GetData().size();

		std::ofstream ofs(Name, std::ios::binary);
		if (ofs.is_open() == false) return false;

		ofs.write((const char*)& BMH, sizeof(BMH));
		ofs.write((const char*)& BMI, sizeof(BMI));
		for (auto& o : S.GetPallete()) {

			BYTE r = o & 0xff;
			BYTE g = (o & 0xff00) >> 8;
			BYTE b = (o & 0xff0000) >> 16;

			RGBQUAD Q = { b,g,r,0 };

			ofs.write((const char*)& Q, sizeof(Q));
		}

		for (std::intmax_t y = S.Height() - 1; y >= 0; y--) {
			for (std::size_t x = 0; x < S.Width(); x++) {
				ofs.write((const char*)& S.IndexData(x, y), sizeof(std::uint8_t));
			}
		}

		return true;
	}

	bool SaveMQO(Pixel3DProc::Surfaces& Box, const std::string& Name) {
		int Line = 10;
		std::vector<std::tuple<Cood, std::size_t>> Vertex;
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
			for (std::intmax_t y = 0; y < Box[z].Height(); y++) {
				for (std::size_t x = 0; x < Box[z].Width(); x++) {
					if (Box[z].IndexData(x, y) == 0) { continue; }
					for (auto& o : DD) {
						//Vertex.push_back({ { (std::get<0>(o) + (Line * (x + 1))),(std::get<1>(o) + (Line * (Box[z].Height() - (y + 1)))),(std::get<2>(o) + (Line * (z + 1))) } ,Box[z].IndexData(x,y)});
						//Vertex.push_back({ { (std::get<0>(o) + (Line * x)),(std::get<1>(o) + (Line * (Box[z].Height() - y))),(std::get<2>(o) + (Line * z)) } ,Box[z].IndexColor(Box[z].IndexData(x,y)) });
						Vertex.push_back({ { (std::get<0>(o) + (Line * x)),(std::get<1>(o) + (Line * y)),(std::get<2>(o) + (Line * z)) } ,Box[z].IndexColor(Box[z].IndexData(x,y)) });
					}
				}
			}

		}
		std::unordered_map<COLORREF, std::size_t> M;
		for (auto& oo : Box) {
			for (auto& o : oo.GetPallete()) {
				M[o] = 0;
			}
		}
		std::size_t z = 0;
		for (auto& o : M) {
			o.second = z;
			z++;
		}
		std::string Face;
		std::size_t x = 0;
		for (std::size_t i = 0; i < Vertex.size() / 8; i++) {
			Face += "\t\t4 V(" + std::to_string(0 + (8 * i)) + ' ' + std::to_string(2 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ' ' + std::to_string(1 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			Face += "\t\t4 V(" + std::to_string(2 + (8 * i)) + ' ' + std::to_string(4 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			Face += "\t\t4 V(" + std::to_string(4 + (8 * i)) + ' ' + std::to_string(6 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			Face += "\t\t4 V(" + std::to_string(6 + (8 * i)) + ' ' + std::to_string(0 + (8 * i)) + ' ' + std::to_string(1 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			Face += "\t\t4 V(" + std::to_string(6 + (8 * i)) + ' ' + std::to_string(4 + (8 * i)) + ' ' + std::to_string(2 + (8 * i)) + ' ' + std::to_string(0 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			Face += "\t\t4 V(" + std::to_string(1 + (8 * i)) + ' ' + std::to_string(3 + (8 * i)) + ' ' + std::to_string(5 + (8 * i)) + ' ' + std::to_string(7 + (8 * i)) + ')' + " M(" + std::to_string(M[std::get<1>(Vertex[i * 8])]) + ") UV(0 0 1 0 1 1 0 1)\n";
			x += 6;//count face.																																															  
		}



		std::string Doc = "Metasequoia Document\t\t\nFormat Text Ver 1.0\n\n		Scene{\n			pos 0 0 1500\n			lookat 0 0 0\n			head - 0.5236\n			pich 0.5236\n			ortho 1\n			zoom2 5.0000\n			amb 0.250 0.250 0.250\n	}\n";


		Doc += "Material " + std::to_string(z) + " {\n";
		//do...
		z = 0;
		for (auto& o : M) {
			Doc += "\t\"mat" + std::to_string(++z) + "\" col(" + std::to_string((o.first & 0xff) / 255.0) + " " + std::to_string(((o.first & 0xff00) >> 8) / 255.0) + " " + std::to_string(((o.first & 0xff0000) >> 16) / 255.0) + " 1.000) dif(0.800) amb(0.600) emi(0.000) spc(0.000) power(5.00)\n";
			//Doc += "\t\"mat1\" col(1.000 1.000 1.000 1.000) dif(0.800) amb(0.600) emi(0.000) spc(0.000) power(5.00)\n";
		}
		Doc += "}\n";

		Doc += "Object obj" + std::to_string(1) + " {\n";
		Doc += "\tvisible 15\n\tlocking 0\n\tshading 1\n\tfacet 59.5\n\tcolor 0.898 0.498 0.698\n\tcolor_type 0\n";
		Doc += "	vertex " + std::to_string(Vertex.size()) + " {\n";
		//do...
		for (auto& o : Vertex) {
			Doc += "\t\t" + std::to_string(static_cast<double>(std::get<0>(std::get<0>(o)))) + ' ' + std::to_string(static_cast<double>(std::get<1>(std::get<0>(o)))) + ' ' + std::to_string(static_cast<double>(std::get<2>(std::get<0>(o)))) + '\n';
		}
		Doc += "}\n";
		Doc += "\tface " + std::to_string(x) + " {\n";
		//do...
		Doc += Face;
		Doc += "}\n";
		Doc += "}\n";
		Doc += "Eof\n";


		std::ofstream ofs(Name);
		if (!ofs.is_open()) return false;

		ofs << Doc;

		return true;

	}
protected:
	//message cracker??
	BOOL WMCreate(HWND& hWnd, UINT& msg, WPARAM& wp, LPARAM& lp) {
		UINT BaseH = H + SH + BH + CH + CBH + BH;

		RECT rt = MakeClientRectForWindow(hWnd, W, BaseH);
		MoveWindow(hWnd, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, TRUE);

		Scroll = CreateWindow(_T("SCROLLBAR"), _T(""), WS_CHILD | WS_VISIBLE | SBS_HORZ,
			0, 0, W, SH,
			hWnd, (HMENU)ControlID::Scroll01, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

		HWND BO1 = CreateWindow(_T("Button"), _T("面追加"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, SH, W / 3, BH,
			hWnd, (HMENU)ControlID::Button_AddS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

		HWND BO2 = CreateWindow(_T("Button"), _T("面削除"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			W / 3, SH, W / 3, BH,
			hWnd, (HMENU)ControlID::Button_DelS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

		HWND BO3 = CreateWindow(_T("Button"), _T("面読込"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			W / 3 * 2, SH, W / 3, BH / 2,
			hWnd, (HMENU)ControlID::Button_LoadS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);
		HWND BO4 = CreateWindow(_T("Button"), _T("面出力"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			W / 3 * 2, SH + BH / 2, W / 3, BH / 2,
			hWnd, (HMENU)ControlID::Button_SaveS, ((LPCREATESTRUCT)lp)->hInstance, nullptr);
		HWND BO5 = CreateWindow(_T("Button"), _T("色追加"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, SH + BH + CH + CBH, W / 2, BH,
			hWnd, (HMENU)ControlID::Button_AddC, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

		HWND BO6 = CreateWindow(_T("Button"), _T("色削除"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			W / 2, SH + BH + CH + CBH, W / 2, BH,
			hWnd, (HMENU)ControlID::Button_DelC, ((LPCREATESTRUCT)lp)->hInstance, nullptr);

		Box.push_back({ static_cast<Surface::Cood>(W), static_cast<Surface::Cood>(H) });
		Sel = &Box[0];
		SetScrollRange(Scroll, SB_CTL, 0, 1, TRUE);
		EnableWindow(Scroll, FALSE);

		PenIndex = 1;
		Sel->GetPallete().push_back(RGB(0xff, 0xff, 0xff));
		Sel->GetPallete().push_back(RGB(0xff, 0, 0));
		Sel->GetPallete().push_back(RGB(0, 0xff, 0));
		Sel->GetPallete().push_back(RGB(0, 0, 0xff));
		Sel->GetPallete().push_back(RGB(0xff, 0xff, 0));
		Sel->GetPallete().push_back(RGB(0, 0xff, 0xff));
		Sel->GetPallete().push_back(RGB(0xff, 0, 0xff));
		Sel->GetPallete().push_back(RGB(0xcc, 0xcc, 0xcc));

		SetLocaleJapanese();

		GetWindowRect(hWnd, &WR);
		f = true;
		return TRUE;
	}
	BOOL WMPaint(HWND& hWnd, UINT& msg, WPARAM& wp, LPARAM& lp) {
		PAINTSTRUCT ps = { 0, };
		HDC hDC = BeginPaint(hWnd, &ps);


		HBRUSH B1 = CreateSolidBrush(Sel->IndexColor(PenIndex));
		HBRUSH Old1 = SelectBrush(hDC, B1);
		Rectangle(hDC, 0, SH + BH, W, SH + BH + CBH);//Sel Color
		SelectBrush(hDC, Old1);
		DeleteBrush(B1);

		Rectangle(hDC, 0, SH + BH + CBH, W, SH + BH + CH + CBH);//pallete
		if (Sel != nullptr) {
			for (std::size_t i = 0; i < Sel->PaletteSize(); i++) {
				HBRUSH B = CreateSolidBrush(Sel->IndexColor(i));
				HPEN Pen = CreatePen(PS_SOLID, 1, Sel->IndexColor(i));
				HBRUSH Old = SelectBrush(hDC, B);
				HPEN Op = SelectPen(hDC, Pen);
				Rectangle(hDC, (W / static_cast<double>(Sel->PaletteSize())) * i, SH + BH + CBH, static_cast<int>(W / static_cast<double>(Sel->PaletteSize()) * (i + 1)), SH + BH + CH + CBH);
				SelectPen(hDC, Op);
				SelectBrush(hDC, Old);
				DeleteBrush(B);
				DeletePen(Pen);

			}
		}
		Rectangle(hDC, 0, SH + BH + CH + CBH + BH, W, SH + BH + CH + CBH + BH + H);//canvas
		if (Sel == nullptr) return FALSE;
		std::size_t L = sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256);
		BYTE* p = new BYTE[L+16];
		ZeroMemory(p, L);
		LPBITMAPINFO BMI =(LPBITMAPINFO)p;
		RGBQUAD* RQ = (RGBQUAD*)&p[sizeof(BITMAPINFOHEADER)];

		BMI->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		BMI->bmiHeader.biWidth = Sel->Width();
		BMI->bmiHeader.biHeight = Sel->Height();
		BMI->bmiHeader.biBitCount = 8;
		BMI->bmiHeader.biPlanes = 1;
		BMI->bmiHeader.biClrUsed = Sel->PaletteSize();
		for (std::size_t i = 0; i < Sel->PaletteSize(); i++) {
			RQ[i].rgbRed = Sel->IndexColor(i) & 0xff;
			RQ[i].rgbGreen = (Sel->IndexColor(i) & 0xff00)>>8;
			RQ[i].rgbBlue = (Sel->IndexColor(i) & 0xff0000)>>16;
		}
		/**/
		for (std::size_t i = 0; i < Sel->Height(); i++) {
			SetDIBitsToDevice(hDC, 0, SH + BH + CH + CBH + BH+i, Sel->Width(), 1, 0, 0, 0, 1, &Sel->IndexData(0,i), BMI, DIB_RGB_COLORS);	
		}
		/**/
		//SetDIBitsToDevice(hDC, 0, SH + BH + CH + CBH + BH, Sel->Width(), Sel->Height(), 0, 0, 0, Sel->Height(), &Sel->GetData()[0], BMI, DIB_RGB_COLORS);
		delete[] p;
		/** /
		HDC mDC = CreateCompatibleDC(hDC);
		HBITMAP hBM = CreateCompatibleBitmap(hDC, W, H);

		HBITMAP OBM = SelectBitmap(mDC, hBM);

		for (int j = 0; j < Sel->Height(); j++) {
			for (int i = 0; i < Sel->Width(); i++) {
				if (Sel->IndexData(i, j) != 0 && Sel->IndexData(i, j) < Sel->PaletteSize()) {

					SetPixel(mDC, i,  j, Sel->IndexColor(Sel->IndexData(i, j)));
				}
			}
		}

		BitBlt(hDC, 0, SH + BH + CH + CBH + BH, W, H, mDC, 0, 0, SRCCOPY);

		SelectBitmap(mDC, OBM);
		DeleteBitmap(hBM);
		DeleteDC(mDC);
		/**/

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
		return TRUE;
	}
	BOOL WMHScroll(HWND& hWnd, UINT& msg, WPARAM& wp, LPARAM& lp) {

		SetScrollRange(Scroll, SB_CTL, 0, std::max<int>(0, static_cast<std::intmax_t>(Box.size()) - 1), FALSE);

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
			if (N >= 0) { SetScrollPos(Scroll, SB_CTL, N, FALSE); }
		}
		break;


		case SB_LINERIGHT:
		{
			int N = GetScrollPos(Scroll, SB_CTL) + 1;
			if (N < Box.size()) { SetScrollPos(Scroll, SB_CTL, N, FALSE); }
			break;
		}
		case SB_PAGELEFT:
		{
			int N = GetScrollPos(Scroll, SB_CTL) - 1;
			if (N >= 0) { SetScrollPos(Scroll, SB_CTL, N, FALSE); }
		}
		break;
		case SB_PAGERIGHT:
		{
			int N = GetScrollPos(Scroll, SB_CTL) + 1;
			if (N < Box.size()) { SetScrollPos(Scroll, SB_CTL, N, FALSE); }
			break;
		}
		case SB_THUMBPOSITION:
			SetScrollPos(Scroll, SB_CTL, HIWORD(wp), FALSE);
			break;

		default:
			break;

		}
		INT P = std::clamp<INT>(GetScrollPos(Scroll, SB_CTL), 0, std::max<INT>(((INT)Box.size()) - 1, 0));
		Sel = &Box[P];
		PenIndex = std::clamp<INT>(PenIndex, 0, std::max<INT>(Sel->PaletteSize() - 1, 0));
		RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return TRUE;
	}

	/**/
	BOOL WMCommand(HWND& hWnd, UINT& msg, WPARAM& wp, LPARAM& lp) {
		switch (LOWORD(wp)) {
			 /**/
			case static_cast<INT>(ControlID::Button_AddS) :
			{
				int P = GetScrollPos(Scroll, SB_CTL);
				Surface* Te = &Box[P];
				Box.insert(Box.begin() + P, { static_cast<Surface::Cood>(W), static_cast<Surface::Cood>(H) });
				if (Box.size() > 1)EnableWindow(Scroll, TRUE);
				Sel = &Box[GetScrollPos(Scroll, SB_CTL)];
				if (PenIndex >= Sel->PaletteSize()) { PenIndex = 0; }
				Sel->GetPallete() = Te->GetPallete();
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

				const int L = (1 << 10) + 1;
				TCHAR Str[L] = { 0, };;
				OPENFILENAME OFN{ 0, };

				OFN.lStructSize = sizeof(OPENFILENAME);
				OFN.hwndOwner = hWnd;
				OFN.lpstrFilter = (TCHAR*)_T("8bit_Bitmap (*.bmp)\0*.bmp\0\0");
				OFN.lpstrFile = Str;
				OFN.nMaxFile = L;
				OFN.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

				if (GetOpenFileName(&OFN) != FALSE) {
					auto [b, Su] = ReadBitmap256(WideCharToMultiByte(Str));

					if (b) {

						INT P = std::clamp<INT>(GetScrollPos(Scroll, SB_CTL), 0, std::max<INT>(((INT)Box.size()) - 1, 0));

						Box.insert(Box.begin() + P, Su);
						EnableWindow(Scroll, TRUE);
						SetScrollRange(Scroll, SB_CTL, 0, Box.size() - 1, FALSE);
						PenIndex = 1;
						Sel = &Box[P];
						MessageBox(hWnd, _T("ロードしました。（BITMAP)"), _T("ロード！！"), 0);
						InvalidateRect(hWnd, nullptr, TRUE);
					}
				}
				//MessageBox(hWnd, _T("ごめんなさい。"), _T("未実装です。"), 0);
				break;
			}

			case static_cast<INT>(ControlID::Button_SaveS) :
			{

			   const int L = (1 << 10) + 1;
			   TCHAR Str[L] = { 0, };;
			   OPENFILENAME OFN{ 0, };

			   OFN.lStructSize = sizeof(OPENFILENAME);
			   OFN.hwndOwner = hWnd;
			   OFN.lpstrFilter = (TCHAR*)_T("8bit_Bitmap (*.bmp)\0*.bmp\0Metasequoia(2.x) (*.mqo)\0*.mqo\0\0");
			   OFN.lpstrFile = Str;
			   OFN.nMaxFile = L;
			   OFN.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

			   if (GetSaveFileName(&OFN) != FALSE) {
				   std::string S = WideCharToMultiByte(Str);
				   if (OFN.nFilterIndex == 1) {
					   for (std::size_t i = 0; i < Box.size(); i++) {

						   SaveBitmap256(Box[i], S + (Box.size() > 1 ? '_' + std::to_string(i) : "") + ".bmp");
					   }
					   MessageBox(hWnd, _T("セーブしました。(BMP)"), _T("セーブ！"), 0);
				   }
				   if (OFN.nFilterIndex == 2) {

					   SaveMQO(Box, S + ".mqo");
					   MessageBox(hWnd, _T("セーブしました。(MQO)"), _T("セーブ！"), 0);
				   }
			   }
			}
			 break;

			 case static_cast<INT>(ControlID::Button_DelC) : {
				  if (Sel->PaletteSize() <= 1) { break; }

				  Sel->GetPallete().erase(Sel->GetPallete().begin() + PenIndex);
				  PenIndex = std::clamp<std::int16_t>(PenIndex,0,Sel->PaletteSize()-1);
				  break;
			 }

			case static_cast<INT>(ControlID::Button_AddC) :
			{
			  CHOOSECOLOR CC = { 0, };
			  COLORREF R();

			  CC.lStructSize = sizeof(CHOOSECOLOR);
			  CC.hwndOwner = hWnd;
			  CC.Flags = CC_RGBINIT;
			  CC.lpCustColors = UserColor;
			  CC.rgbResult = 0;
			  //CC.hInstance = (HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE);

			  if (ChooseColor(&CC) != false) {
				  Sel->GetPallete().push_back(CC.rgbResult);
				  PenIndex = Sel->PaletteSize() - 1;
			  }
			  break;
			}
			default:												  
				break;
	}

		SetScrollRange(Scroll, SB_CTL, 0, std::max<int>(0, static_cast<std::intmax_t>(Box.size()) - 1), FALSE);
		RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		return TRUE;
	}
	/**/
	BOOL WMGetMinMaxInfo(HWND& hWnd, UINT& msg, WPARAM& wp, LPARAM& lp) {
		if (f == false) return DefWindowProc(hWnd, msg, wp, lp);
		LPMINMAXINFO LPMMI = (LPMINMAXINFO)lp;
		POINT P{ WR.right - WR.left,WR.bottom - WR.top };
		LPMMI->ptMaxSize = P;
		LPMMI->ptMaxTrackSize = P;
		LPMMI->ptMinTrackSize = P;
		return TRUE;
	}
protected:
	HWND Scroll = nullptr;

	Surfaces Box;
	Surface* Sel = nullptr;
	std::uint8_t PenIndex = 0;

	INT W = 256;
	INT H = 256;

	INT SH = 16;//scrool 
	INT BH = 48;//button
	INT CBH = 16;//selected color.
	INT CH = 32;//color select
	RECT WR = { 0, };
	bool f = false;
	COLORREF UserColor[16] = { 0, };

}; 


