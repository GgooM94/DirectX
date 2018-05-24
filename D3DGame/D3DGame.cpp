#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
VOID CleanUP();
VOID Render();

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("D3D Game");

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// 버텍스 버퍼

// 커스텀 버텍스 타입 구조체
struct CUSTOMVERTEX
{
	FLOAT x, y, z;		// 3D 좌표값
	DWORD color;		// 버텍스 색상
};

// 커스텀 버텍스의 구조를 표시하기 위한 FVF (Flexible Vertex Format) 값
// D3DFVF_XYZ (3D 월드 좌표) 와 D3DFVF_DIFFUSE (점의 색상) 특성을 가지도록.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG Message;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	// Direct3D 초기화에 성공하고, 버텍스 버퍼 생성에 성공하면 진행, 실패하면 종료한다.
	if (SUCCEEDED(InitD3D(hWnd)) && SUCCEEDED(InitGeometry())) {

		// 윈도우 출력
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		// 메시지 루프 시작하기
		while (GetMessage(&Message, NULL, 0, 0))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	UnregisterClass(lpszClass, WndClass.hInstance);
	return 0;
}
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_CREATE:
		hWndMain = hWnd;
		return 0;
	case WM_PAINT:
		Render();			// 화면 출력을 담당하는 랜더링 함수 호출
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_DESTROY:
		CleanUP();			// 프로그램 종룔 시 객체 해제를 위하여 호출함;
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, msg, wParam, lParam));
}



//-------------------------------------------------------
//	이름 : InitD3D()
//	기능 : Direct3D 초기화
//-------------------------------------------------------

HRESULT InitD3D(HWND hWnd) {
	//Direct3D 객체 생성
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;


	// 장치 생성용 데이터 준비
	D3DPRESENT_PARAMETERS d3dpp;				// 장치 생성용 정보 구조체 변수 선언
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// 변수 클리어
	d3dpp.Windowed = true;						// 윈도우 모드로 실행 설정
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 스왑 방법 설정

	// 백버퍼 포맷은 현재 데스크탑 형식으로 설정
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// D3D객체의 장치 생성 함수 호출 (디폴트 비디오카드 사용 HAL 사용, 소프트웨어 버텍스 처리사용 지정)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	// 이제 장치가 정상적으로 생성되었음.
	return S_OK;
}


//-------------------------------------------------------
//	이름 : InitGeometry()
//	기능 : 버텍스 버퍼를 생성한 후 버텍스로 채운다.
//-------------------------------------------------------

HRESULT InitGeometry() {
	// 삼각형을 그리기 위하여 3 점에 대한 임시 배열을 만든다.
	CUSTOMVERTEX vertices[] = {
		{ 0.0f, 100.0f, 0.0f, 0xffff0000, },		// 버텍스의 위치와 색상
		{100.0f, -100.0f, 0.0f, 0xff00ffff, },
		{-100.0f, -100.0f, 0.0f, 0xff00ff00, },

		// 0xffff0000 : 0x	ff (투명도)	00 (Red)	ff (Green)	ff (Blue)
	};

	// 버텍스 버퍼를 생성한다. 
	// 각 버텍스의 포맷은 D3DFVF_CUSTOMVERTEX 라는 것도 전달
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// 버텍스 버퍼에 락을 건 후 버텍스를 넣는다.
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
	
}


//-------------------------------------------------------
//	이름 : Cleanup()
//	기능 : 초기화되었던 모든 객체들을 해제한다.
//-------------------------------------------------------

VOID CleanUP() {
	if (g_pd3dDevice != NULL)		// 장치 객체 해제
		g_pd3dDevice->Release();

	if (g_pd3dDevice != NULL)
		g_pD3D->Release();			// D3D 객체 해제
}

//-------------------------------------------------------
//	이름 : Render()
//	기능 : 화면을 그린다.
//-------------------------------------------------------

VOID Render() {

	// if문에서 NULL 값을 왼쪽에 두는 이유는 프로그래머 실수를 방지 하기 위해!!
	// ex) g_pd3dDevice = NULL <<< 같은 실수가 발생 할 수도 있기때문에 아래와 같이 하는 습관.

	if (NULL == g_pd3dDevice)		// 장치 객체가 생성되지 않았으면 리턴
		return;

	// 백버퍼를 지정된 색상으로 지운다. (여기에는 청색으로 지움)
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// 화면 그리기 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		//// 버텍스 출력 부분
		// 1. 버텍스 버퍼 지정
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));

		// 2. 버텍스 포멧 지정
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

		// 3. 버텍스를 이용하여 그릴 도형 지정
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

		//화면 그리기 끝
		g_pd3dDevice->EndScene();
	}

	// 백퍼퍼의 내용을 화면으로 보낸다.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}