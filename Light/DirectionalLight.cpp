#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
HRESULT InitGeometryLight();
VOID CleanUP();
VOID SetupViewProjection();
VOID SetupLight();
VOID Render();

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("D3D Game, GgooM94");

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// 버텍스 버퍼
LPDIRECT3DVERTEXBUFFER9 g_pVBLight = NULL;	// 라이트용 버텍스 버퍼

// 커스텀 버텍스 타입 구조체
struct CUSTOMVERTEX
{
	FLOAT x, y, z;		// 3D 좌표값
	DWORD color;		// 버텍스 색상
};

// 커스텀 버텍스의 구조를 표시하기 위한 FVF (Flexible Vertex Format) 값
// D3DFVF_XYZ (3D 월드 좌표) 와 D3DFVF_DIFFUSE (점의 색상) 특성을 가지도록.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// 조명 처리를 위한 버텍스 구조체
struct LIGHTVERTEX
{
	D3DXVECTOR3 position;		// 3D 좌표 구조체
	D3DXVECTOR3 normal;			// 버텍스 노말
};

// 버텍스 구조를 지정하는 FVF 정의
#define D3DFVF_LIGHTVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL)


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd)) &&				// Direct3D 초기화에 성공하고,
		SUCCEEDED(InitGeometry()) &&			// 버텍스 버퍼 생성에 성공하고,
		SUCCEEDED(InitGeometryLight())) {		// 라이트 버텍스 버퍼 생성에 성공해야

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
//	이름 : InitGeometryLight()
//	기능 : 조명 처리를 위한 버텍스 버퍼를 생성한 후 버텍스로 채운다.
//-------------------------------------------------------

HRESULT InitGeometryLight() {
	// 버텍스 버퍼를 생성한다.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(6 * sizeof(LIGHTVERTEX), 0, D3DFVF_LIGHTVERTEX, D3DPOOL_DEFAULT, &g_pVBLight, NULL))) {
		return E_FAIL;
	}

	// 버텍스 버퍼에 락을 건 후 버텍스를 넣는다.
	LIGHTVERTEX* pVertices;
	if (FAILED(g_pVBLight->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	// 삼각형 1
	pVertices[0].position = D3DXVECTOR3(-30.0f, 0.0f, -30.0f);
	pVertices[1].position = D3DXVECTOR3(-60.0f, 30.0f, 0.0f);
	pVertices[2].position = D3DXVECTOR3(-90.0f, 0.0f, 30.0f);

	// 삼각형1에 대한 노말 구하기
	D3DXVECTOR3 p1 = pVertices[1].position - pVertices[0].position;
	D3DXVECTOR3 p2 = pVertices[2].position - pVertices[0].position;
	D3DXVECTOR3 pNormal;
	D3DXVec3Cross(&pNormal, &p2, &p1);

	// 삼각형1의 각 버텍스에 노말 값 할당
	pVertices[0].normal = pNormal;
	pVertices[1].normal = pNormal;
	pVertices[2].normal = pNormal;

	// 삼각형 2
	pVertices[3].position = D3DXVECTOR3(90.0f, 0.0f, 30.0f);
	pVertices[4].position = D3DXVECTOR3(60.0f, 30.0f, 0.0f);
	pVertices[5].position = D3DXVECTOR3(30.0f, 0.0f, -30.0f);

	// 삼각형2에 대한 노말 구하기
	p1 = pVertices[4].position - pVertices[3].position;
	p2 = pVertices[5].position - pVertices[3].position;
	D3DXVec3Cross(&pNormal, &p2, &p1);

	// 삼각형2의 각 버텍스에 노말 값 할당
	pVertices[3].normal = pNormal;
	pVertices[4].normal = pNormal;
	pVertices[5].normal = pNormal;

	g_pVBLight->Unlock();

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
// D3DLIGHT9 구조
//-------------------------------------------------------
//
//typedef struct _D3DLIGHT9 {
//	D3DLIGHTTYPE Type;			// 라이트 타입 (포인트 디렉션, 스포트) 3가지 타입 가능
//	D3DCOLORVALUE Diffuse;		// 확산광을 표현하는 D3DCOLORVALUE
//	D3DCOLORVALUE Specular;		// 반사광을 표현하는 D3DCOLORVALUE
//	D3DCOLORVALUE Ambient;		// 주면광을 표현하는 D3DCOLORVALUE
//	D3DVECTOR Position;			// 라이트 객체의 위치, 디렉션 라이트는 없어도 된다.
//	D3DVECTOR Direction;		// 광선의 방향, 포인트 라이트는 없어도 된다.
//	float Range;				// 조명을 받는 객체의 가장 먼 거리
//	float Falloff;				// SPOTLIGHT의 내/외부 콘 사이이의 강도 차이
//	float Attenuation0;			// 거리에 따른 선형 감쇄값
//	float Attenuation1;			// 거리에 따른 선형 감쇄값
//	float Attenuation2;			// 거리에 따른 선형 감쇄값
//	float Theta;				// SPOTLIGHT의 내부 콘의 각도
//	float Phi;					// SPOTLIGHT의 외부 콘의 각도
//};
//-------------------------------------------------------


//-------------------------------------------------------
//	이름 : SetupLight()
//	기능 : 조명을 설정한다.
//-------------------------------------------------------
VOID SetupLight() {
	D3DLIGHT9 light;			// Direct3D 9 조명 구조체 변수 선언
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;		// 조명 타입을 디렉셔널로 설정

	light.Diffuse.r = 1.0f;					// 조명의 Red 색 설정
	light.Diffuse.g = 1.0f;					// 조명의 Green 색 설정
	light.Diffuse.g = 1.0f;					// 조명의 Blue 색 설정

	D3DXVECTOR3 vecDir;						// 방향 벡터 변수 선언
	vecDir = D3DXVECTOR3(10, -10, 10);		// 조명의 방향 ( 진행하는 방향)
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);	//벡터 정규화

	g_pd3dDevice->SetLight(0, &light);		// 라이트 번호 지정
	g_pd3dDevice->LightEnable(0, TRUE);		// 0 번 라이트 켜기
	
	// 라이트 사용 기능을 TRUE로 설정. ( 이 기능을 끄면 모든 라이트 사용 중지)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	// 최종적으로 엠비언트 라이트 켜기 ( 환경과의 양을 결정)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00808080);
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
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

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

		// 변환이 적용되지 않은 상태에서 삼각형 그리기
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// x 축에 따라 위치 이동 매트릭스

		D3DXMatrixTranslation(&matWorld, 100.0f, 0.0f, 0.0f);	// x 축으로 100 만큼 이동, 매트릭스

		// <이동후 회전 매트릭스> = <회전 매트릭스> x <이동 매트릭스>
		// 함수사용 방법에 기반한 표현이므로 먼저 계산되는 매트릭스가 우측에 위치한다.

		// 매트릭스 변수 추가
		D3DXMATRIXA16 matWorld2;
		// matWorld2를 y 축을 중심으로 -45도 회전하는 매트릭스로 만들기
		D3DXMatrixRotationY(&matWorld2, -45.0f*(D3DX_PI / 180.0f));
		
		// matWorld2를 이동 후 회전을 하는 매트릭스로 만들어주는 곱하기
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		
		// matWorld2를 x 축 방향으로 2배 확대하는 매트릭스로 만들기
		D3DXMatrixScaling(&matWorld2, 2.0f, 1.0f, 1.0f);

		// matWorld에 확대 기능까지 추가히가 위한 곱하기
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		
		// 복합 변환이 적용된 최종 매트릭스 matWorld를 장치로 전달
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		// 변환 매트릭스 전달	
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// 삼각형 그리기
		

		// 라이트 설정
		SetupLight();

		// 재질 설정
		D3DMATERIAL9 mtrl;			// 재질 설정용 구조체
		ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
		// 확산광과 주변광 재질에 대하여 R, G, B, A 값을 설정, R,G,B : (1,1,0) 노란색
		mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
		mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
		mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
		mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
		
		// 설정한 재질을 적용
		g_pd3dDevice->SetMaterial(&mtrl);

		// 삼각형 출력을 위한 이동
		D3DXMatrixTranslation(&matWorld, 0, 0, -100);		// -z 방향으로 100 이동
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// 변환 매트릭스 전달

		// 삼각형 출력을 위한 버텍스 버퍼 지정
		g_pd3dDevice->SetStreamSource(0, g_pVBLight, 0, sizeof(LIGHTVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_LIGHTVERTEX);		// 버텍스 포맷 지정
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);	//삼각형 2개 그리기

		// 화면 그리기 끝
		g_pd3dDevice->EndScene();
	}

	// 백퍼퍼의 내용을 화면으로 보낸다.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}