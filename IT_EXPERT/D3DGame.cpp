#include<Windows.h>
#include<d3dx9.h>
LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("IT_EXPERT");

/*----------------------------------------------------------------------
* 전역 변수
-----------------------------------------------------------------------*/
LPDIRECT3D9 g_pD3D = NULL;					// Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)


/*----------------------------------------------------------------------
* Direct3D 초기화
-----------------------------------------------------------------------*/
HRESULT InitD3D(HWND hWnd) {

	// 디바이스를 생성하기 위한 D3D 객체 생성
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;				// 디바이스 생성을 위한 구조체
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// 구조체 초기화!!
	
	d3dpp.Windowed = TRUE;						// 윈도우 모드로 생성
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 가장 효출적인 SWAP 효과
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;	// 현재 바탕화면 모드에 맞춰서 푸면 버퍼를 생성

	// 디바이스를 다음과 같은 설정으로 생성한다.
	// 1. 디폴트 비디오카트를 사용한다.
	// 2. HAL 디바이스 생성. (HW 가속 장치를 사용하겠다는 의미)
	// 3. 정점 처리는 모든 카드에서 지원하는 SW 처리로 생성한다. (HW로 생성할 경우 더욱 높은 성능을 낸다).

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}
	return S_OK;
}


/*----------------------------------------------------------------------
* 초기화된 객체들을 소거한다.
-----------------------------------------------------------------------*/
VOID CleanUp() {	
	// 장치 객체 해제
	if (g_pd3dDevice != NULL) {
		g_pd3dDevice->Release();
	}

	// D3D 객체 해제
	if (g_pD3D != NULL) {
		g_pD3D->Release();
	}
}


/*----------------------------------------------------------------------
 * 화면 그리기
 -----------------------------------------------------------------------*/
VOID Render() {
	if (NULL == g_pd3dDevice) {		// 장치 객체가 생성되지 않았으면 리턴
		return;
	}

	// 후면 버퍼 클리어
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	// 랜더링 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		// 랜더링 명령 입력 부분


		// 랜더링 종료
		g_pd3dDevice->EndScene();
	}

	// 후면 버퍼를 보이는 화면으로
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}



/*----------------------------------------------------------------------
* 윈도우 프로시저
-----------------------------------------------------------------------*/
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



/*----------------------------------------------------------------------
* 프로그램 시작점
-----------------------------------------------------------------------*/
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;

	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);		// 윈도우 생성

	if (SUCCEEDED(InitD3D(hWnd))) {
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));		//구조체 초기화!!

		while (GetMessage(&msg, NULL, 0, 0))
		{

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	// 등록된 클래스 소거
	UnregisterClass(lpszClass, WndClass.hInstance);
	return 0;
}