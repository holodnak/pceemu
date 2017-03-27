#include <windows.h>
#include <windowsx.h>

#include <SDL2/SDL.h>

#include "source/resource.h"
#include "source/pce.h"
#include "source/system/system.h"
#include "source/ui/ui.h"


const char g_szClassName[] = "myWindowClass";
HWND hwnd;
HINSTANCE hinst;

SDL_Window *sdlWindow = 0;

CSDLSystem *sys;
CUI *ui;
CPce *pce;

bool quit;

int pitch;
uint32_t *screen;

//512x256 framebuffer for pce output
uint32_t *pce_framebuffer;

uint32_t *getline(int line)
{
	return(pce_framebuffer + (line * 512));
}

static void draw_screen(uint32_t *screen, int pitch)
{
	int x, y;
	uint32_t *srcptr, *destptr, *uiptr;

	int pce_width = pce->Huc6270()->GetLineWidth();

	destptr = screen + ((0 * 4) * (pitch / 4));
	srcptr = pce_framebuffer + ((0) * 512);

	//copy 240 lines to (240 * 4) lines, 512 width -> 1024 width
	for (y = 0; y < 240; y++) {

		uint32_t line_buf[1024 * 2];
		uint32_t *line_ptr;

		srcptr = pce_framebuffer + ((y) * 512);
		line_ptr = line_buf;

		//double resolution
		if (pce_width == 256) {
			for (x = 0; x < 256; x++) {
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
			}
		}
		else if (pce_width == 384) {
			for (x = 0; x < 384; x++) {
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
			}
		}
		else if (pce_width == 512) {
			for (x = 0; x < 512; x++) {
				*line_ptr++ = srcptr[x];
				*line_ptr++ = srcptr[x];
			}
		}

		for (x = 0; x < 1024; x++) {
			int n = pitch / 4;
			uint32_t pix = line_buf[x];

			destptr[x + n * 0] = pix;
			destptr[x + n * 1] = pix;
			destptr[x + n * 2] = pix;
			destptr[x + n * 3] = pix;
		}
		destptr += pitch;
	}

/*	for (y = 0; y < 960; y++) {

		destptr = screen + (y * (pitch / 4));
		uiptr = ui->Framebuffer()->Pixels() + (y * 1024);

		for (x = 0; x < 1024; x++) {
		//	if (uiptr[x])
		//		destptr[x] = uiptr[x];
			destptr[x] = ((uiptr[x] + destptr[x]) >> 1) & 0x7F7F7F7F;
		}
	}*/
	/*
	//pce FB = 512x240 (stretch to 512x480), UI FB = 1024x960
	for (y = 0; y < 960; y++) {

	destptr = screen + (y * (pitch / 4));
	srcptr = pce_framebuffer + ((y / 1) * 512);
	uiptr = ui_framebuffer + (y * 1024);

	uint32_t linebuf[1024], pix;

	#define BLEND_UI(al, c1, c2) \
	(((uint8_t)(c1)+(uint8_t)(c2)) / 2)

	for (x = 0; x < 1024; x++) {
	uint32_t pix;

	if (uiptr[x]) {
	uint8_t r, g, b;

	r = BLEND_UI(0xC0, uiptr[x] >> 16, srcptr[x / 2] >> 16);
	g = BLEND_UI(0xC0, uiptr[x] >> 8, srcptr[x / 2] >> 8);
	b = BLEND_UI(0xC0, uiptr[x] >> 0, srcptr[x / 2] >> 0);
	pix = (r << 16) | (g << 8) | b;
	pix = uiptr[x];

	}
	else
	pix = srcptr[x / 2];

	destptr[x] = pix;
	}
	}
	*/
}

int InitSDL()
{
	sdlWindow = SDL_CreateWindowFrom((void*)hwnd);

	if (sdlWindow == NULL) {
		printf("SDL_Error: %s\n",SDL_GetError());
		return(1);
	}

	return(0);
}

void resizeclient(HWND hwnd, int w, int h)
{
	RECT rc, rw;

	GetWindowRect(hwnd, &rw);
	GetClientRect(hwnd, &rc);
	SetWindowPos(hwnd, 0, 0, 0, ((rw.right - rw.left) - rc.right) + w, ((rw.bottom - rw.top) - rc.bottom) + h, SWP_NOZORDER | SWP_NOMOVE);
}

void RefreshDebugger(HWND hwnd)
{
	HWND listbox = GetDlgItem(hwnd, IDC_LIST_DISASM);
	char disasm[1024];
	uint32_t pc;
	int i;

	ListBox_ResetContent(listbox);
	pc = pce->Huc6280()->GetPC();
	for (i = 0; i < 20; i++) {
		pc = pce->Huc6280()->Disasm(pc, disasm);
		ListBox_AddString(listbox, disasm);
	}

	sprintf(disasm,
		"PC:\t%04X\n"
		"A:\t%02X\n"
		"X:\t%02X\n"
		"Y:\t%02X\n"
		"S:\t%02X\n"
		"P:\t%02X\n"
		"MPR0:\t%02X\n"
		"MPR1:\t%02X\n"
		"MPR2:\t%02X\n"
		"MPR3:\t%02X\n"
		"MPR4:\t%02X\n"
		"MPR5:\t%02X\n"
		"MPR6:\t%02X\n"
		"MPR7:\t%02X\n"
		"\n"
		"Cycle:\t%ld\n"
		,
		pce->Huc6280()->GetReg(REG_PC),
		pce->Huc6280()->GetReg(REG_A), pce->Huc6280()->GetReg(REG_X), pce->Huc6280()->GetReg(REG_Y), pce->Huc6280()->GetReg(REG_S),
		pce->Huc6280()->GetReg(REG_P),
		pce->Huc6280()->GetReg(REG_MPR + 0),
		pce->Huc6280()->GetReg(REG_MPR + 1),
		pce->Huc6280()->GetReg(REG_MPR + 2),
		pce->Huc6280()->GetReg(REG_MPR + 3),
		pce->Huc6280()->GetReg(REG_MPR + 4),
		pce->Huc6280()->GetReg(REG_MPR + 5),
		pce->Huc6280()->GetReg(REG_MPR + 6),
		pce->Huc6280()->GetReg(REG_MPR + 7),
		pce->Huc6280()->Cycle()
	);

	SetWindowText(GetDlgItem(hwnd, IDC_STATIC_CPUREGS), disasm);

	sprintf(disasm,
		"Frame:\t%d\n"
		"Line:\t%d\n"
		"Cycle:\t%d\n"
		"\n"
		,
		pce->Huc6270()->Frame(),
		pce->Huc6270()->Scanline(),
		pce->Huc6270()->Cycle()

	);

	SetWindowText(GetDlgItem(hwnd, IDC_STATIC_VDPREGS), disasm);
}

BOOL CALLBACK DebuggerDlg(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		RefreshDebugger(hwnd);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDC_BUTTON_JUMP:
			MessageBox(hwnd, "Hi!", "This is a message",
				MB_OK | MB_ICONEXCLAMATION);
			break;

		case IDC_BUTTON_STEP:
			pce->Huc6280()->Step();
			RefreshDebugger(hwnd);
			break;

		case IDC_BUTTON_RESET:
			pce->Reset();
			RefreshDebugger(hwnd);
			break;

		case IDOK:
			EndDialog(hwnd, IDOK);
			break;

		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		}
		break;

	default:
		return FALSE;

	}
	return TRUE;
}

static int filedialog(HWND parent, int type, char *buffer, char *title, char *filter, char *curdir)
{
	OPENFILENAME dlgdata;

	memset((void*)&dlgdata, 0, sizeof(OPENFILENAME));
	dlgdata.lStructSize = sizeof(OPENFILENAME);
	dlgdata.hwndOwner = parent;
	dlgdata.hInstance = GetModuleHandle(0);
	dlgdata.lpstrFilter = filter;
	dlgdata.lpstrFile = buffer;
	dlgdata.nMaxFile = 1024;
	dlgdata.lpstrInitialDir = curdir;
	dlgdata.lpstrTitle = title;
	dlgdata.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	switch (type) {
	default:
	case 0: //open
			//			dlgdata.Flags |= OFN_ENABLETEMPLATE | OFN_EXPLORER;
			//			dlgdata.lpTemplateName = (LPSTR)IDD_OPENEXT;
		if (GetOpenFileName(&dlgdata) != 0)
			return(0);
		break;
	case 1: //save
		if (GetSaveFileName(&dlgdata) != 0)
			return(0);
		break;
	}

	printf("CommDlgExtendedError() = %d\n", CommDlgExtendedError());
	return(1);
}

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	char filename[1024];

	switch (msg)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_OPEN:
			memset(filename, 0, 1024);
			filedialog(hWnd, 0, filename, "Open PCE ROM...", "PCE ROM Files\0*.pce;*.bin\0\0", "d:\\pce");
			pce->Unload();
			pce->Load(filename);
			pce->Reset();
//			file_open(hWnd);
			break;
		case ID_VIEW_DEBUGGER:
			DialogBox(hinst, (LPCTSTR)IDD_DEBUGGER, hWnd, (DLGPROC)DebuggerDlg);

		case ID_HELP_ABOUT:
//			DialogBox(hInst, (LPCTSTR)IDD_ABOUT, hWnd, (DLGPROC)AboutDlg);
			break;
//		case ID_HELP_SUPPORTEDMAPPERS:
//			DialogBox(hInst, (LPCTSTR)IDD_MAPPERS, hWnd, (DLGPROC)MappersDlg);
//			break;
		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		quit = true;
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int InitWindow()
{
	WNDCLASSEX wc;

	HINSTANCE hInstance = GetModuleHandle(0);
	int nCmdShow = SW_SHOW;

	hinst = hInstance;

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = (LPCTSTR)IDR_MENU1;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"pceemu",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	InitSDL();

	resizeclient(hwnd, 1024, 960);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	return 0;
}

int InitSystem()
{

	pce_framebuffer = new uint32_t[512 * (256 + 16) * 4];

	try {
		sys = new CSDLSystem();
		ui = new CUI();
		pce = new CPce(sys);

		InitWindow();

		sys->Init();
		ui->Init();
	}
	catch (bool fatal) {
		printf("error (%s)", fatal ? "fatal" : "not fatal");
		return(1);
	}

	pce->Init();


	return 0;
}

void system_checkevents()
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
/*		if (IsDialogMessage(hConsole, &msg) || IsDialogMessage(hDebugger, &msg))
			continue;
		if (TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			continue;*/
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main(int argc, char* argv[])
{
	char *fn = "rom.pce";

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		// Unrecoverable error, exit here.
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return(-1);
	}


	if (argc >= 2)
		fn = argv[1];

	printf("pceemu beta v0.1\n\n");

	InitSystem();

	//    pce_init();
	printf("loading %s\n", fn);
	if (pce->Load(fn) == false) {
		quit = true;
	}

	else {
		pce->Reset();
	}

	while (quit == false) {

		pce->Frame();
		ui->Tick();
		ui->Draw();

		screen = (uint32_t*)sys->GetVideo()->Lock(&pitch);

		draw_screen(screen, pitch);

		sys->GetVideo()->Unlock();
		sys->GetInput()->Poll();

		//		pce->Setjoystate();

		if (sys->CheckEvents() == true)
			quit = true;

		system_checkevents();
	}

	delete sys;
	delete pce_framebuffer;

	return 0;
}