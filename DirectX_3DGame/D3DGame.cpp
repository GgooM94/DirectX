#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("DirectX_3DGame");

HRESULT InitD3D(HWND hWnd);

LPDIRECT3D9 g_pD3D = NULL;					//Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("DirectX_3DGame"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd))) {

		// 윈도우 출력
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		ZeroMemory(&msg, sizeof(msg));
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return (int)msg.wParam;
}


HRESULT InitD3D(HWND hWnd) {

	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;					// 장치 생성용 정보 구조체 변수 선언
	ZeroMemory(&d3dpp, sizeof(d3dpp));				// 변수 클리어
	d3dpp.BackBufferWidth = 800;					// 버퍼 해상도 넓이 설정
	d3dpp.BackBufferHeight = 600;					// 버퍼 해상도 높이 설정
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;		// 픽셀 포맷W
	d3dpp.BackBufferCount = 1;						// 백버퍼 수
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;	// 후면 버퍼에 이용할 샘플링 타입
	d3dpp.MultiSampleQuality = 0;					// 멀티 샘플링의 레벨
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;		// 스왑 방법 설정
	d3dpp.hDeviceWindow = hWnd;						// 윈도우 핸들 설정
	d3dpp.Windowed = true;							// 윈도우 모드 실행, false : 전체 화면
	d3dpp.EnableAutoDepthStencil = true;			// 스탠실 버퍼 사용
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;	// 깊이버퍼 24비트,  스탠실 버퍼 8비트 사용 예약
	d3dpp.Flags = 0;

	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		// 재생율 지정 , D3DPRESENT_RATE_DEFAULT : 디폴트값
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;		// 집합의 멤버, D3DPRESENT_INTERVAL_IMMEDIATE : 즉시 시연, D3DPRESENT_INTERVAL_DEFAULT : Direct3가 시연간격 조정, 보통 재생율과 동일

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}
}



LRESULT CALLBACK MsgProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMessage)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}