#include<Windows.h>
#include<d3dx9.h>
#include"XFileUtil.h"
#include<XAudio2.h>


LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGameDate();
HRESULT InitGeometry();
HRESULT InitGeometryLight();
HRESULT InitGeometryTexture();
VOID CleanUP();
VOID SetupViewProjection();
VOID SetupLight();
VOID InputCheck();
void BulletControl();
VOID Render();


HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("D3D Game, GgooM94");

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D 객체
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// 랜더링 장치(비디오 카드)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// 버텍스 버퍼
LPDIRECT3DVERTEXBUFFER9 g_pVBLight = NULL;	// 라이트용 버텍스 버퍼
PDIRECT3DVERTEXBUFFER9 g_pVBTexture = NULL;	// 텍스쳐 출력용 버텍스 버퍼
LPDIRECT3DTEXTURE9 g_pTexture = NULL;		// 텍스쳐 로딩용 변수
LPDIRECT3DTEXTURE9 g_pFloorTexture = NULL;	// 바닥 텍스쳐
LPDIRECT3DTEXTURE9 g_pBulletTexture = NULL;	// 총알 텍스쳐

CXFileUtil g_XFile;							// X 파일 출력을 위한 클래스 객체
CXFileUtil g_XTank;							// 탱크 메쉬
CXFileUtil g_XHouse;						// 집 메쉬
CXFileUtil g_XTiger;						// 호랑이 메쉬

// 스프라이트 구조체
struct SPRITE
{
	int spriteNumber;		// 전체 스프라이트 이미지 수
	int curIndex;			// 현재 출력해야 하는 스프라이트 인덱스
	int frameCounter;		// 현재 스프라이트를 출력하고 지속된 프레임 수
	int frameDelay;			// 스프라이트 변경 속도 조절을 위한 프레임 딜레이
	float x, y, z;			// 스프라이트가 발생될 위치
	BOOL state;
};

// 스프라이트 구조체 변수 선언 및 초기화
SPRITE g_Fire = { 15, 0, 0, 3 , 0, 0, 0, FALSE};		// 선언 및 초기화
HRESULT ChangeSpriteUV(SPRITE *sp);

// 극 좌표계 시스템 구조체
struct POLAR
{
	float x, y, z;
	float angle;
	float radius;
};

// 카메라의 극좌표계 구조체 변수 및 초기화
POLAR g_Camera = { 0, 15, -180, 90, 1.0 };

// 탱크 위치
float g_TankX = 100.0f, g_TankZ = 0.0f;

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


// 텍스쳐 좌표를 가지는 버텍스 구조체 정의
struct TEXTUREVERTEX
{
	D3DXVECTOR3 position;		// 버텍스 위치
	D3DCOLOR color;				// 버텍스 색상
	FLOAT tu, tv;				// 텍스쳐 좌표
};

// 텍스쳐 버텍스 구조를 지정하는 FVF 정의
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// 총알 구조체
struct BULLET
{
	float x, y, z;
	float deltaX, deltaZ;
	BOOL state;
};

// 총알 변수
BULLET g_Bullet = { 0, 0, 0, 0, 0, false };

// 빌보드용 매트릭스, 총알
D3DXMATRIXA16 MatBillboardMatrix;
D3DXVECTOR3 g_vDir;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd)) &&				// Direct3D 초기화에 성공하고,
		SUCCEEDED(InitGeometry()) &&			// 버텍스 버퍼 생성에 성공하고,
		SUCCEEDED(InitGeometryLight()) &&		// 라이트 버텍스 버퍼 생성에 성공
		SUCCEEDED(InitGeometryTexture()) &&		// 텍스쳐 버퍼 생성에 성공
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
				InputCheck();
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


	//d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		// 전체 화면의 프레임 동기화

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
	//A2T : char to TCHAR
	g_XFile.XfileLoad(g_pd3dDevice, A2T("./image/skybox2.x"));			// 스카이 박스 로드
	g_XTank.XfileLoad(g_pd3dDevice, A2T("./image/predator.x"));			// 탱크 로드
	g_XHouse.XfileLoad(g_pd3dDevice, A2T("./image/House3.x"));			// 집 로드
	g_XTiger.XfileLoad(g_pd3dDevice, A2T("./image/tiger.x"));			// 호랑이 로드

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
//	이름 : InitGeometryTexture()
//	기능 : 텍스쳐 출력을 위한 버텍스 버퍼를 생성한 후 버텍스로 채운다.
//-------------------------------------------------------

HRESULT InitGeometryTexture() {
	// 텍스쳐 로딩
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/Fire.bmp"), &g_pTexture))) {
		return E_FAIL;
	}
	
	// 바닥 지형 텍스쳐 로딩
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/seafloor.bmp"), &g_pFloorTexture))) {
		return E_FAIL;
	}

	// 총알 텍스쳐 로딩
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/bullet.dds"), &g_pBulletTexture))) {
		return E_FAIL;
	}

	// 버텍스 버퍼 생성
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(12 * sizeof(TEXTUREVERTEX), 0, D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL))) {
		return E_FAIL;
	}

	// 버텍스 버퍼 설정
	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	pVertices[0].position = D3DXVECTOR3(-50, 100, 100);		// 버텍스 위치
	pVertices[0].color = 0xffffffff;						// 버텍스 알파 및 색상
	pVertices[0].tu = 0.0f;									// 버텍스 U 텍스쳐 좌표
	pVertices[0].tv = 0.0f;									// 버텍스 V 텍스쳐 좌표

	pVertices[1].position = D3DXVECTOR3(50, 100, 100);		// 버텍스 위치
	pVertices[1].color = 0xffffffff;						// 버텍스 알파 및 색상
	pVertices[1].tu = 64.0f/960.0f;									// 버텍스 U 텍스쳐 좌표
	pVertices[1].tv = 0.0f;									// 버텍스 V 텍스쳐 좌표

	pVertices[2].position = D3DXVECTOR3(-50, 0, 100);		// 버텍스 위치
	pVertices[2].color = 0xffffffff;						// 버텍스 알파 및 색상
	pVertices[2].tu = 0.0f;									// 버텍스 U 텍스쳐 좌표
	pVertices[2].tv = 1.0f;									// 버텍스 V 텍스쳐 좌표

	pVertices[3].position = D3DXVECTOR3(50, 0, 100);		// 버텍스 위치
	pVertices[3].color = 0xffffffff;						// 버텍스 알파 및 색상
	pVertices[3].tu = 64.0f / 960.0f;									// 버텍스 U 텍스쳐 좌표
	pVertices[3].tv = 1.0f;									// 버텍스 V 텍스쳐 좌표

	// 바닥 텍스쳐가 들어갈 버텍스
	pVertices[4].position = D3DXVECTOR3(-200, 0, 200);		
	pVertices[4].color = 0xffffffff;						
	pVertices[4].tu = 0.0f;									
	pVertices[4].tv = 0.0f;				

	pVertices[5].position = D3DXVECTOR3(200, 0, 200);
	pVertices[5].color = 0xffffffff;
	pVertices[5].tu = 10.0f;
	pVertices[5].tv = 0.0f;

	pVertices[6].position = D3DXVECTOR3(-200, 0, -200);
	pVertices[6].color = 0xffffffff;
	pVertices[6].tu = 0.0f;
	pVertices[6].tv = 10.0f;

	pVertices[7].position = D3DXVECTOR3(200, 0, -200);
	pVertices[7].color = 0xffffffff;
	pVertices[7].tu = 10.0f;
	pVertices[7].tv = 10.0f;

	// 총알 텍스쳐가 들어갈 버텍스
	pVertices[8].position = D3DXVECTOR3(-0.5, 2, 0);
	pVertices[8].color = 0xffffffff;
	pVertices[8].tu = 0.0f;
	pVertices[8].tv = 0.0f;

	pVertices[9].position = D3DXVECTOR3(0.5, 2, 0);
	pVertices[9].color = 0xffffffff;
	pVertices[9].tu = 1.0f;
	pVertices[9].tv = 0.0f;

	pVertices[10].position = D3DXVECTOR3(-0.5, 1, 0);
	pVertices[10].color = 0xffffffff;
	pVertices[10].tu = 0.0f;
	pVertices[10].tv = 1.0f;

	pVertices[11].position = D3DXVECTOR3(0.5, 1, 0);
	pVertices[11].color = 0xffffffff;
	pVertices[11].tu = 1.0f;
	pVertices[11].tv = 1.0f;

	g_pVBTexture->Unlock();

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
	D3DXVECTOR3 vEyePt(g_Camera.x, g_Camera.y, g_Camera.z);		// 카메라의 위치
	
	// 카메라가 바라보는 위치 구하기, 평면 운동이므로 y는 그대로 사용한다.
	float destX = (float)(g_Camera.x + g_Camera.radius * cos(g_Camera.angle*(D3DX_PI / 180.0f)));
	float dextZ = (float)(g_Camera.z + g_Camera.radius * sin(g_Camera.angle*(D3DX_PI / 180.0f)));

	D3DXVECTOR3 vLookatPt(destX, g_Camera.y, dextZ);		// 바라보는 지점
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);			// 업벡터 설정
	D3DXMATRIXA16 matView;							// 뷰변환용 매트릭스

	// 뷰 매트릭스 설정
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D 장치에 뷰 매트릭스 전달
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// 빌보드용 매트릭스 설정, 총알
	D3DXVECTOR3 vDir = vLookatPt - vEyePt;
	if (vDir.x > 0.0f) {
		D3DXMatrixRotationY(&MatBillboardMatrix, -atanf(vDir.z / vDir.x) + D3DX_PI / 2);
	}
	else {
		D3DXMatrixRotationY(&MatBillboardMatrix, -atanf(vDir.z / vDir.x) - D3DX_PI / 2);
	}

	//// 프로젝션 변환 설정
	D3DXMATRIXA16 matProj;							// 프로젝션용 매트릭스
	// 프로젝션 매트릭스 설정
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 800.0f/600.0f, 1.0f, 1000.0f);
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
//	이름 : ChangeSpriteUV()
//	기능 : 스프라이트를 위한 uv 함수
//-------------------------------------------------------
HRESULT ChangeSpriteUV(SPRITE *sp) {
	float u = (sp->curIndex * 64.0f) / 960.0f;			// 현재 인덱스를 이용한 u 계산
	float u2 = ((sp->curIndex + 1)*64.0f) / 960.0f;		// 현재 인덱스+1을 위용한 u 계산

	TEXTUREVERTEX *pVertices;							// 버텍스 버퍼 접근용 포인터
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	pVertices[0].tu = u;			// u 좌표 변경
	pVertices[0].tv = 0.0f;			// v 좌표 변경

	pVertices[1].tu = u2;			// u 좌표 변경
	pVertices[1].tv = 0.0f;			// v 좌표 변경

	pVertices[2].tu = u;			// u 좌표 변경
	pVertices[2].tv = 1.0f;			// v 좌표 변경

	pVertices[3].tu = u2;			// u 좌표 변경
	pVertices[3].tv = 1.0f;			// v 좌표 변경

	g_pVBTexture->Unlock();

	// 지정된 딜레이 프레임이 지난 경우
	if (sp->frameCounter >= sp->frameCounter) {
		sp->curIndex++;				// 인덱스 변경
		if (sp->curIndex == sp->spriteNumber) {		// 마지막 장까지 출력이 종료되면
			sp->state = FALSE;		// 사용 중지
		}
	}
	else {
		sp->frameCounter++;			// 프레임 카운터 초기화
	}

\
	return S_OK;

}


//-------------------------------------------------------
//	이름 : InputCheck()
//	기능 : 사용자 입력을 검사한다.
//-------------------------------------------------------
VOID InputCheck() {
	// 전진
	if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W')) {
		g_Camera.x += (float)(g_Camera.radius * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
		g_Camera.z += (float)(g_Camera.radius * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
	}

	// 후진
	if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S')) {
		g_Camera.x -= (float)(g_Camera.radius * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
		g_Camera.z -= (float)(g_Camera.radius * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
	}
	
	// 좌로 회전
	if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A')) {
		g_Camera.angle += 1;
	}

	// 우로 회전
	if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D')) {
		g_Camera.angle -= 1;
	}

	if (GetAsyncKeyState(VK_LBUTTON)) {
		if (g_Bullet.state == FALSE) {		// 변수를 사용 가능한 경우
			g_Bullet.state = TRUE;			// 사용중 표시
		
			g_Bullet.x = g_Camera.x;		// 총알의 최초 위치는 카메라의 위치로 설정
			g_Bullet.y = g_Camera.y;
			g_Bullet.z = g_Camera.z;

			// 매 프레임마다 총알이 이동할 변위를 설정
			float speed = 10.0f;
			g_Bullet.deltaX = (float)(speed * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
			g_Bullet.deltaZ = (float)(speed * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
		
		}

	}

}


//-------------------------------------------------------
//	이름 : BulletControl()
//	기능 : 총알의 이동, 충돌 및 경계면 이탈 등 처리
//-------------------------------------------------------
void BulletControl() {
	if (g_Bullet.state == FALSE)		// 사용중이지 않은 총알은 처리하지 않음
		return;

	g_Bullet.x += g_Bullet.deltaX;
	g_Bullet.z += g_Bullet.deltaZ;

	// 게임 세계의 경계를 벗어나면 총알 소거 및 재사용이 가능하도록 상태 설정
	if (g_Bullet.x <= -200 || g_Bullet.x >= 200)
		g_Bullet.state = FALSE;

	if (g_Bullet.z <= -200 || g_Bullet.z >= 200)
		g_Bullet.state = FALSE;

	// 총알과 탱크의 거리 계산
	float distance = (float)sqrt((g_Bullet.x - g_TankX) * (g_Bullet.x - g_TankX) + (g_Bullet.z - g_TankZ) * (g_Bullet.z - g_TankZ));

	// 상호 거리가 20보다 작으면 충돌로 간주
	if (distance < 20) {
		g_Bullet.state = FALSE;			// 총알 소거
		if (g_Fire.state == FALSE) {	// 폭발 스프라이트가 사용 가능한 경우
			// 스프라이트 발생 처리
			g_Fire.x = g_Bullet.x;
			g_Fire.y = g_Bullet.y;
			g_Fire.z = g_Bullet.z;
			g_Fire.frameDelay = 1;		// 프레임 딜레이는 1;
			g_Fire.curIndex = 0;		// 인덱스 리셋
			g_Fire.frameCounter = 0;	// 프레임 카운터 리셋
			g_Fire.state = TRUE;		// 사용중 표시

		}
	}
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
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);	// z 축 라인 그리기
		}
		
		for (float z = -200; z <= 200; z += 20) {				// x 축에 평행한 라인을 여러개 그리기
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);		// z 축에 따라 위치 이동 매트릭스
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// 변환 매트릭스 적용
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);	// x 축 라인 그리기
		}

		D3DXMatrixIdentity(&matWorld);							// 매트릭스를 단위 행렬로 리셋
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		//변환 매트릭스 적용
		//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);		// y 축 라인 그리기

		// 변환이 적용되지 않은 상태에서 삼각형 그리기
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// x 축에 따라 위치 이동 매트릭스

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
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// 삼각형 그리기
		

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
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);	//삼각형 2개 그리기

		
		//////////////////// x파일 출력
		// 스카이 박스 출력
		D3DXMatrixScaling(&matWorld, 60.0f, 60.0f, 60.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);


		// 호랑이 출력
		D3DXMatrixTranslation(&matWorld, 60.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);

		// 호랑이 출력
		D3DXMatrixTranslation(&matWorld, 30.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);

		// 호랑이 출력
		D3DXMatrixTranslation(&matWorld, 0.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);


		// 집 출력을 위한 매트릭스 변환
		{
		   D3DXMATRIXA16 matWorld;
		   D3DXMatrixTranslation( &matWorld, 0, -2, 0 );
		   D3DXMATRIXA16 matWorld2;
		   D3DXMatrixScaling( &matWorld2, 0.01f, 0.01f, 0.01f);
		   D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		   g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
		}
		// 집 출력
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_XHouse.XFileDisplay(g_pd3dDevice);

		// 탱크 출력을 위한 매트릭스 변환
		{
			D3DXMATRIXA16 matWorld;
			D3DXMatrixTranslation(&matWorld, g_TankX, 0, g_TankZ);
			D3DXMATRIXA16 matWorld2;
			D3DXMatrixScaling(&matWorld2, 0.1f, 0.1f, 0.1f);
			D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
			D3DXMatrixRotationY(&matWorld2, (-90.0f*(D3DX_PI / 180.0f)));
			D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		}
		// 탱크 출력
		g_XTank.XFileDisplay(g_pd3dDevice);
		
		D3DXMatrixScaling(&matWorld, 1.0f, 1.0f, 1.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

															
		// 텍스쳐 출력
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);		// 조명 중지


		// 텍스쳐 출력 환경 설정
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// 텍스쳐가 카메라에 가까운 지역에서 확대되어 출력될 때 사각형 모양으로 확대 되는 모습을 보완
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		// 바닥 텍스쳐 출력
		g_pd3dDevice->SetTexture(0, g_pFloorTexture);

		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));		// 출력할 버텍스 버퍼 설정
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);							// FVF 값 설정
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2);				// 사각형 역역 출력


		

		// 폭발 텍스쳐 출력
		if (g_Fire.state == TRUE) {
			g_pd3dDevice->SetTexture(0, g_pTexture);		// 텍스쳐 설정, 텍스쳐 매칭을 위해 g_pTexture 사용
			
			// 버텍스들의 알파셋에 대해 블랜딩 설정
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			{
				// 해당 위치로 이동 및 빌보딩
				MatBillboardMatrix._41 = g_Fire.x;
				MatBillboardMatrix._42 = 0;
				MatBillboardMatrix._43 = g_Fire.z;
				g_pd3dDevice->SetTransform(D3DTS_WORLD, &MatBillboardMatrix);
			}

			g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));		// 출력할 버텍스 버퍼 설정
			g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);							// FVF 값 설정
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);				// 사각형 출력
			g_pd3dDevice->SetTexture(0, NULL);									// 텍스쳐 설정 해제

			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		// 버텍스들의 알파벳에 대하여 블랜딩 설정 해제

			ChangeSpriteUV(&g_Fire);		// 폭발 스프라이트 애니메이션을 위한 uv 애니메이션 함수 호출
		}



		if (g_Bullet.state == TRUE) {
			g_pd3dDevice->SetTexture(0, g_pBulletTexture);

			// 투명 텍스쳐 처리 부분
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			{
				// 해당 위치로 이동 및 빌보드
				MatBillboardMatrix._41 = g_Bullet.x;
				MatBillboardMatrix._42 = 0;
				MatBillboardMatrix._43 = g_Bullet.z;

				g_pd3dDevice->SetTransform(D3DTS_WORLD, &MatBillboardMatrix);
			}

			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 8, 2);

			// 총알 출력 후 처리
			BulletControl();
		}


		// 화면 그리기 끝
		g_pd3dDevice->EndScene();
	}

	// 백퍼퍼의 내용을 화면으로 보낸다.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}