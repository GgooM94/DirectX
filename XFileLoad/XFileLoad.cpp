#include<Windows.h>
#include<d3dx9.h>
#include"XFileUtil.h"

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGameDate();
HRESULT InitGeometry();
VOID CleanUP();
VOID SetupViewProjection();
VOID Render();

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("D3D Game, GgooM94");

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// 버텍스 버퍼

CXFileUtil g_XFile;							// X 파일 출력을 위한 클래스 객체

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
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd)) &&				// Direct3D 초기화에 성공하고,
		SUCCEEDED(InitGeometry()) &&			// 버텍스 버퍼 생성에 성공하고,
		SUCCEEDED(InitGameDate())) {			// 기타 게임 데이터 로드

												// 윈도우 출력
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		// 메시지 루프 시작하기
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT) {
			// 메시지가 있으면 가져 온다.
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				// 메시지가 없을 때는 항상 Render() 호출
				Render();
			}
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
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		return E_FAIL;
	}

	// 장치 생성용 데이터 준비
	D3DPRESENT_PARAMETERS d3dpp;				// 장치 생성용 정보 구조체 변수 선언
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// 변수 클리어

	d3dpp.BackBufferWidth = 800;				// 버퍼 해상도 넓이 설정
	d3dpp.BackBufferHeight = 600;				// 버퍼 해상도 높이 설정
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;	// 버퍼 포맷 설정
	d3dpp.BackBufferCount = 1;					// 백버퍼 수
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// 스왑 방법 설정
	d3dpp.hDeviceWindow = hWnd;					// 윈도우 핸들 설정
	d3dpp.Windowed = true;						// 윈도우 모드로 실행 설정

	d3dpp.EnableAutoDepthStencil = true;		// 스탠실 버퍼를 사용하도록 한다.
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;	// 스탠실 버퍼 포맷 설정


													// D3D객체의 장치 생성 함수 호출 (디폴트 비디오카드 사용 HAL 사용, 소프트웨어 버텍스 처리사용 지정)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	// 이제 장치가 정상적으로 생성되었음.

	// zBuffer 사용 설정
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	return S_OK;
}


//-------------------------------------------------------
//	이름 : InitGameDate()
//	기능 : 게임에 관련된 각종 데이터를 초기화 한다.
//-------------------------------------------------------
HRESULT InitGameDate() {
	USES_CONVERSION;		// CHAR TO TCHAR, 사용할 내부 버퍼
	g_XFile.XfileLoad(g_pd3dDevice, A2T("./image/tiger.x"));	//A2T : char to TCHAR
	return S_OK;
}

//-------------------------------------------------------
//	이름 : InitGeometry()
//	기능 : 버텍스 버퍼를 생성한 후 버텍스로 채운다.
//-------------------------------------------------------

HRESULT InitGeometry() {
	// 삼각형을 그리기 위하여 3 점에 대한 임시 배열을 만든다.
	CUSTOMVERTEX vertices[] = {
		// 버텍스의 위치와 색상
		{ -200.0f, 0.0f, 0.0f, 0xff00ff00, },		// x축 라인을 위한 버텍스
	{ 200.0f, 0.0f, 0.0f, 0xff00ff00, },
	{ 0.0f, 0.0f, -200.0f, 0xffffff00, },		// z축 라인을 위한 버텍스
	{ 0.0f, 0.0f, 200.0f, 0xffffff00, },
	{ 0.0f, -200.0f, 0.0f, 0xffff0000, },		// y축 라인을 위한 버텍스
	{ 0.0f, 200.0f, 0.0f, 0xffff0000, },

	{ 0.0f, 50.0f, 0.0f, 0xffff0000, },			// 삼각형의 첫 번째 버텍스
	{ -50.0f, 0.0f, 0.0f, 0xffff0000, },		// 삼각형의 두 번째 버텍스
	{ 50.0f, 0.0f, 0.0f, 0xffff0000, },			// 삼각형의 세 번째 버텍스


												// 0xffff0000 : 0x	ff (투명도)	00 (Red)	ff (Green)	ff (Blue)
	};

	// 버텍스 버퍼를 생성한다. 
	// 각 버텍스의 포맷은 D3DFVF_CUSTOMVERTEX 라는 것도 전달
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
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
//	이름 : SetupViewProjection()
//	기능 : 뷰 변환과 프로젝션 변환을 설정한다.
//-------------------------------------------------------
VOID SetupViewProjection() {
	//// 뷰 변환 설정
	D3DXVECTOR3 vEyePt(100.0f, 250.0f, -400.0f);		// 카메라의 위치
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);		// 바라보는 지점
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);			// 업벡터 설정
	D3DXMATRIXA16 matView;							// 뷰변환용 매트릭스

													// 뷰 매트릭스 설정
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D 장치에 뷰 매트릭스 전달
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	//// 프로젝션 변환 설정
	D3DXMATRIXA16 matProj;							// 프로젝션용 매트릭스
													// 프로젝션 매트릭스 설정
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
	//Direct3D 장치로 프로젝션 매트릭스 전달
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
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

	// 뷰 및 프로젝션 변환 설정
	SetupViewProjection();

	// 삼각형의 앞/뒤 변을 모두 랜더링하도록 컬링 기능을 끈다.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// 조명을 끈다. (조명에 의한 색상이 아니고, 버텍스 자체의 색상을 사용하도록), TRUE : 조명사용, FALSE : 조명 x
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// 백버퍼를 지정된 색상으로 지운다. (여기에는 검은색으로 지움)
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// 화면 그리기 시작
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		//// 버텍스 출력 부분

		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));		// 1. 버텍스 버퍼 지정
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);								// 2. 버텍스 포멧 지정

		D3DXMATRIXA16 matWorld;		//월드 변환용 매트릭스 선언
									// 3. 버텍스를 이용하여 그릴 도형 지정
		for (float x = -200; x <= 200; x += 20) {				// z 축에 평행한 라인을 여러 개 그리기
			D3DXMatrixTranslation(&matWorld, x, 0.0, 0.0);		// x 축에 따라 위치 이동 매트릭스
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// 변환 매트릭스 적용
			g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);	// z 축 라인 그리기
		}

		for (float z = -200; z <= 200; z += 20) {				// x 축에 평행한 라인을 여러개 그리기
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);		// z 축에 따라 위치 이동 매트릭스
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// 변환 매트릭스 적용
			g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);	// x 축 라인 그리기
		}

		D3DXMatrixIdentity(&matWorld);							// 매트릭스를 단위 행렬로 리셋
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		//변환 매트릭스 적용
		g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);		// y 축 라인 그리기


		// x파일 출력
		D3DXMatrixScaling(&matWorld, 100.0f, 100.0f, 100.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

		g_XFile.XFileDisplay(g_pd3dDevice);

		D3DXMatrixScaling(&matWorld, 1.0f, 1.0f, 1.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

		// 화면 그리기 끝
		g_pd3dDevice->EndScene();
	}

	// 백퍼퍼의 내용을 화면으로 보낸다.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}