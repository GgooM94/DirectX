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

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D ��ü
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// ������ ��ġ(���� ī��)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// ���ؽ� ����
LPDIRECT3DVERTEXBUFFER9 g_pVBLight = NULL;	// ����Ʈ�� ���ؽ� ����
PDIRECT3DVERTEXBUFFER9 g_pVBTexture = NULL;	// �ؽ��� ��¿� ���ؽ� ����
LPDIRECT3DTEXTURE9 g_pTexture = NULL;		// �ؽ��� �ε��� ����
LPDIRECT3DTEXTURE9 g_pFloorTexture = NULL;	// �ٴ� �ؽ���
LPDIRECT3DTEXTURE9 g_pBulletTexture = NULL;	// �Ѿ� �ؽ���

CXFileUtil g_XFile;							// X ���� ����� ���� Ŭ���� ��ü
CXFileUtil g_XTank;							// ��ũ �޽�
CXFileUtil g_XHouse;						// �� �޽�
CXFileUtil g_XTiger;						// ȣ���� �޽�

// ��������Ʈ ����ü
struct SPRITE
{
	int spriteNumber;		// ��ü ��������Ʈ �̹��� ��
	int curIndex;			// ���� ����ؾ� �ϴ� ��������Ʈ �ε���
	int frameCounter;		// ���� ��������Ʈ�� ����ϰ� ���ӵ� ������ ��
	int frameDelay;			// ��������Ʈ ���� �ӵ� ������ ���� ������ ������
	float x, y, z;			// ��������Ʈ�� �߻��� ��ġ
	BOOL state;
};

// ��������Ʈ ����ü ���� ���� �� �ʱ�ȭ
SPRITE g_Fire = { 15, 0, 0, 3 , 0, 0, 0, FALSE};		// ���� �� �ʱ�ȭ
HRESULT ChangeSpriteUV(SPRITE *sp);

// �� ��ǥ�� �ý��� ����ü
struct POLAR
{
	float x, y, z;
	float angle;
	float radius;
};

// ī�޶��� ����ǥ�� ����ü ���� �� �ʱ�ȭ
POLAR g_Camera = { 0, 15, -180, 90, 1.0 };

// ��ũ ��ġ
float g_TankX = 100.0f, g_TankZ = 0.0f;

// Ŀ���� ���ؽ� Ÿ�� ����ü
struct CUSTOMVERTEX
{
	FLOAT x, y, z;		// 3D ��ǥ��
	DWORD color;		// ���ؽ� ����
};

// Ŀ���� ���ؽ��� ������ ǥ���ϱ� ���� FVF (Flexible Vertex Format) ��
// D3DFVF_XYZ (3D ���� ��ǥ) �� D3DFVF_DIFFUSE (���� ����) Ư���� ��������.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// ���� ó���� ���� ���ؽ� ����ü
struct LIGHTVERTEX
{
	D3DXVECTOR3 position;		// 3D ��ǥ ����ü
	D3DXVECTOR3 normal;			// ���ؽ� �븻
};

// ���ؽ� ������ �����ϴ� FVF ����
#define D3DFVF_LIGHTVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL)


// �ؽ��� ��ǥ�� ������ ���ؽ� ����ü ����
struct TEXTUREVERTEX
{
	D3DXVECTOR3 position;		// ���ؽ� ��ġ
	D3DCOLOR color;				// ���ؽ� ����
	FLOAT tu, tv;				// �ؽ��� ��ǥ
};

// �ؽ��� ���ؽ� ������ �����ϴ� FVF ����
#define D3DFVF_TEXTUREVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// �Ѿ� ����ü
struct BULLET
{
	float x, y, z;
	float deltaX, deltaZ;
	BOOL state;
};

// �Ѿ� ����
BULLET g_Bullet = { 0, 0, 0, 0, 0, false };

// ������� ��Ʈ����, �Ѿ�
D3DXMATRIXA16 MatBillboardMatrix;
D3DXVECTOR3 g_vDir;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd)) &&				// Direct3D �ʱ�ȭ�� �����ϰ�,
		SUCCEEDED(InitGeometry()) &&			// ���ؽ� ���� ������ �����ϰ�,
		SUCCEEDED(InitGeometryLight()) &&		// ����Ʈ ���ؽ� ���� ������ ����
		SUCCEEDED(InitGeometryTexture()) &&		// �ؽ��� ���� ������ ����
		SUCCEEDED(InitGameDate())) {			// ��Ÿ ���� ������ �ε�

		// ������ ���
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		// �޽��� ���� �����ϱ�
		ZeroMemory(&msg, sizeof(msg));
		while (msg.message != WM_QUIT) {
			// �޽����� ������ ���� �´�.
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				// �޽����� ���� ���� �׻� Render() ȣ��
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
		Render();			// ȭ�� ����� ����ϴ� ������ �Լ� ȣ��
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_DESTROY:
		CleanUP();			// ���α׷� ���� �� ��ü ������ ���Ͽ� ȣ����;
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, msg, wParam, lParam));
}



//-------------------------------------------------------
//	�̸� : InitD3D()
//	��� : Direct3D �ʱ�ȭ
//-------------------------------------------------------

HRESULT InitD3D(HWND hWnd) {
	//Direct3D ��ü ����
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) {
		return E_FAIL;
	}

	// ��ġ ������ ������ �غ�
	D3DPRESENT_PARAMETERS d3dpp;				// ��ġ ������ ���� ����ü ���� ����
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// ���� Ŭ����

	d3dpp.BackBufferWidth = 800;				// ���� �ػ� ���� ����
	d3dpp.BackBufferHeight = 600;				// ���� �ػ� ���� ����
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;	// ���� ���� ����
	d3dpp.BackBufferCount = 1;					// ����� ��
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ���� ��� ����
	d3dpp.hDeviceWindow = hWnd;					// ������ �ڵ� ����
	d3dpp.Windowed = true;						// ������ ���� ���� ����

	d3dpp.EnableAutoDepthStencil = true;		// ���Ľ� ���۸� ����ϵ��� �Ѵ�.
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;	// ���Ľ� ���� ���� ����


	//d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		// ��ü ȭ���� ������ ����ȭ

	// D3D��ü�� ��ġ ���� �Լ� ȣ�� (����Ʈ ����ī�� ��� HAL ���, ����Ʈ���� ���ؽ� ó����� ����)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	// ���� ��ġ�� ���������� �����Ǿ���.

	// zBuffer ��� ����
	g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	return S_OK;
}


//-------------------------------------------------------
//	�̸� : InitGameDate()
//	��� : ���ӿ� ���õ� ���� �����͸� �ʱ�ȭ �Ѵ�.
//-------------------------------------------------------
HRESULT InitGameDate() {
	USES_CONVERSION;		// CHAR TO TCHAR, ����� ���� ����
	//A2T : char to TCHAR
	g_XFile.XfileLoad(g_pd3dDevice, A2T("./image/skybox2.x"));			// ��ī�� �ڽ� �ε�
	g_XTank.XfileLoad(g_pd3dDevice, A2T("./image/predator.x"));			// ��ũ �ε�
	g_XHouse.XfileLoad(g_pd3dDevice, A2T("./image/House3.x"));			// �� �ε�
	g_XTiger.XfileLoad(g_pd3dDevice, A2T("./image/tiger.x"));			// ȣ���� �ε�

	return S_OK;
}

//-------------------------------------------------------
//	�̸� : InitGeometry()
//	��� : ���ؽ� ���۸� ������ �� ���ؽ��� ä���.
//-------------------------------------------------------

HRESULT InitGeometry() {
	// �ﰢ���� �׸��� ���Ͽ� 3 ���� ���� �ӽ� �迭�� �����.
	CUSTOMVERTEX vertices[] = {
		// ���ؽ��� ��ġ�� ����
		{ -200.0f, 0.0f, 0.0f, 0xff00ff00, },		// x�� ������ ���� ���ؽ�
		{ 200.0f, 0.0f, 0.0f, 0xff00ff00, },
		{ 0.0f, 0.0f, -200.0f, 0xffffff00, },		// z�� ������ ���� ���ؽ�
		{ 0.0f, 0.0f, 200.0f, 0xffffff00, },
		{ 0.0f, -200.0f, 0.0f, 0xffff0000, },		// y�� ������ ���� ���ؽ�
		{ 0.0f, 200.0f, 0.0f, 0xffff0000, },

		{ 0.0f, 50.0f, 0.0f, 0xffff0000, },			// �ﰢ���� ù ��° ���ؽ�
		{ -50.0f, 0.0f, 0.0f, 0xffff0000, },		// �ﰢ���� �� ��° ���ؽ�
		{ 50.0f, 0.0f, 0.0f, 0xffff0000, },			// �ﰢ���� �� ��° ���ؽ�


		// 0xffff0000 : 0x	ff (����)	00 (Red)	ff (Green)	ff (Blue)
	};

	// ���ؽ� ���۸� �����Ѵ�. 
	// �� ���ؽ��� ������ D3DFVF_CUSTOMVERTEX ��� �͵� ����
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
		return E_FAIL;
	}

	// ���ؽ� ���ۿ� ���� �� �� ���ؽ��� �ִ´�.
	VOID* pVertices;
	if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
	
}


//-------------------------------------------------------
//	�̸� : InitGeometryLight()
//	��� : ���� ó���� ���� ���ؽ� ���۸� ������ �� ���ؽ��� ä���.
//-------------------------------------------------------

HRESULT InitGeometryLight() {
	// ���ؽ� ���۸� �����Ѵ�.
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(6 * sizeof(LIGHTVERTEX), 0, D3DFVF_LIGHTVERTEX, D3DPOOL_DEFAULT, &g_pVBLight, NULL))) {
		return E_FAIL;
	}

	// ���ؽ� ���ۿ� ���� �� �� ���ؽ��� �ִ´�.
	LIGHTVERTEX* pVertices;
	if (FAILED(g_pVBLight->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	// �ﰢ�� 1
	pVertices[0].position = D3DXVECTOR3(-30.0f, 0.0f, -30.0f);
	pVertices[1].position = D3DXVECTOR3(-60.0f, 30.0f, 0.0f);
	pVertices[2].position = D3DXVECTOR3(-90.0f, 0.0f, 30.0f);

	// �ﰢ��1�� ���� �븻 ���ϱ�
	D3DXVECTOR3 p1 = pVertices[1].position - pVertices[0].position;
	D3DXVECTOR3 p2 = pVertices[2].position - pVertices[0].position;
	D3DXVECTOR3 pNormal;
	D3DXVec3Cross(&pNormal, &p2, &p1);

	// �ﰢ��1�� �� ���ؽ��� �븻 �� �Ҵ�
	pVertices[0].normal = pNormal;
	pVertices[1].normal = pNormal;
	pVertices[2].normal = pNormal;

	// �ﰢ�� 2
	pVertices[3].position = D3DXVECTOR3(90.0f, 0.0f, 30.0f);
	pVertices[4].position = D3DXVECTOR3(60.0f, 30.0f, 0.0f);
	pVertices[5].position = D3DXVECTOR3(30.0f, 0.0f, -30.0f);

	// �ﰢ��2�� ���� �븻 ���ϱ�
	p1 = pVertices[4].position - pVertices[3].position;
	p2 = pVertices[5].position - pVertices[3].position;
	D3DXVec3Cross(&pNormal, &p2, &p1);

	// �ﰢ��2�� �� ���ؽ��� �븻 �� �Ҵ�
	pVertices[3].normal = pNormal;
	pVertices[4].normal = pNormal;
	pVertices[5].normal = pNormal;

	g_pVBLight->Unlock();

	return S_OK;
}


//-------------------------------------------------------
//	�̸� : InitGeometryTexture()
//	��� : �ؽ��� ����� ���� ���ؽ� ���۸� ������ �� ���ؽ��� ä���.
//-------------------------------------------------------

HRESULT InitGeometryTexture() {
	// �ؽ��� �ε�
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/Fire.bmp"), &g_pTexture))) {
		return E_FAIL;
	}
	
	// �ٴ� ���� �ؽ��� �ε�
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/seafloor.bmp"), &g_pFloorTexture))) {
		return E_FAIL;
	}

	// �Ѿ� �ؽ��� �ε�
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/bullet.dds"), &g_pBulletTexture))) {
		return E_FAIL;
	}

	// ���ؽ� ���� ����
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(12 * sizeof(TEXTUREVERTEX), 0, D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL))) {
		return E_FAIL;
	}

	// ���ؽ� ���� ����
	TEXTUREVERTEX* pVertices;
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	pVertices[0].position = D3DXVECTOR3(-50, 100, 100);		// ���ؽ� ��ġ
	pVertices[0].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[0].tu = 0.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[0].tv = 0.0f;									// ���ؽ� V �ؽ��� ��ǥ

	pVertices[1].position = D3DXVECTOR3(50, 100, 100);		// ���ؽ� ��ġ
	pVertices[1].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[1].tu = 64.0f/960.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[1].tv = 0.0f;									// ���ؽ� V �ؽ��� ��ǥ

	pVertices[2].position = D3DXVECTOR3(-50, 0, 100);		// ���ؽ� ��ġ
	pVertices[2].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[2].tu = 0.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[2].tv = 1.0f;									// ���ؽ� V �ؽ��� ��ǥ

	pVertices[3].position = D3DXVECTOR3(50, 0, 100);		// ���ؽ� ��ġ
	pVertices[3].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[3].tu = 64.0f / 960.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[3].tv = 1.0f;									// ���ؽ� V �ؽ��� ��ǥ

	// �ٴ� �ؽ��İ� �� ���ؽ�
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

	// �Ѿ� �ؽ��İ� �� ���ؽ�
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
//	�̸� : Cleanup()
//	��� : �ʱ�ȭ�Ǿ��� ��� ��ü���� �����Ѵ�.
//-------------------------------------------------------

VOID CleanUP() {
	if (g_pd3dDevice != NULL)		// ��ġ ��ü ����
		g_pd3dDevice->Release();

	if (g_pd3dDevice != NULL)
		g_pD3D->Release();			// D3D ��ü ����
}


//-------------------------------------------------------
//	�̸� : SetupViewProjection()
//	��� : �� ��ȯ�� �������� ��ȯ�� �����Ѵ�.
//-------------------------------------------------------
VOID SetupViewProjection() {
	//// �� ��ȯ ����
	D3DXVECTOR3 vEyePt(g_Camera.x, g_Camera.y, g_Camera.z);		// ī�޶��� ��ġ
	
	// ī�޶� �ٶ󺸴� ��ġ ���ϱ�, ��� ��̹Ƿ� y�� �״�� ����Ѵ�.
	float destX = (float)(g_Camera.x + g_Camera.radius * cos(g_Camera.angle*(D3DX_PI / 180.0f)));
	float dextZ = (float)(g_Camera.z + g_Camera.radius * sin(g_Camera.angle*(D3DX_PI / 180.0f)));

	D3DXVECTOR3 vLookatPt(destX, g_Camera.y, dextZ);		// �ٶ󺸴� ����
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);			// ������ ����
	D3DXMATRIXA16 matView;							// �亯ȯ�� ��Ʈ����

	// �� ��Ʈ���� ����
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D ��ġ�� �� ��Ʈ���� ����
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	// ������� ��Ʈ���� ����, �Ѿ�
	D3DXVECTOR3 vDir = vLookatPt - vEyePt;
	if (vDir.x > 0.0f) {
		D3DXMatrixRotationY(&MatBillboardMatrix, -atanf(vDir.z / vDir.x) + D3DX_PI / 2);
	}
	else {
		D3DXMatrixRotationY(&MatBillboardMatrix, -atanf(vDir.z / vDir.x) - D3DX_PI / 2);
	}

	//// �������� ��ȯ ����
	D3DXMATRIXA16 matProj;							// �������ǿ� ��Ʈ����
	// �������� ��Ʈ���� ����
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 800.0f/600.0f, 1.0f, 1000.0f);
	//Direct3D ��ġ�� �������� ��Ʈ���� ����
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

//-------------------------------------------------------
// D3DLIGHT9 ����
//-------------------------------------------------------
//
//typedef struct _D3DLIGHT9 {
//	D3DLIGHTTYPE Type;			// ����Ʈ Ÿ�� (����Ʈ �𷺼�, ����Ʈ) 3���� Ÿ�� ����
//	D3DCOLORVALUE Diffuse;		// Ȯ�걤�� ǥ���ϴ� D3DCOLORVALUE
//	D3DCOLORVALUE Specular;		// �ݻ籤�� ǥ���ϴ� D3DCOLORVALUE
//	D3DCOLORVALUE Ambient;		// �ָ鱤�� ǥ���ϴ� D3DCOLORVALUE
//	D3DVECTOR Position;			// ����Ʈ ��ü�� ��ġ, �𷺼� ����Ʈ�� ��� �ȴ�.
//	D3DVECTOR Direction;		// ������ ����, ����Ʈ ����Ʈ�� ��� �ȴ�.
//	float Range;				// ������ �޴� ��ü�� ���� �� �Ÿ�
//	float Falloff;				// SPOTLIGHT�� ��/�ܺ� �� �������� ���� ����
//	float Attenuation0;			// �Ÿ��� ���� ���� ���Ⱚ
//	float Attenuation1;			// �Ÿ��� ���� ���� ���Ⱚ
//	float Attenuation2;			// �Ÿ��� ���� ���� ���Ⱚ
//	float Theta;				// SPOTLIGHT�� ���� ���� ����
//	float Phi;					// SPOTLIGHT�� �ܺ� ���� ����
//};
//-------------------------------------------------------


//-------------------------------------------------------
//	�̸� : SetupLight()
//	��� : ������ �����Ѵ�.
//-------------------------------------------------------
VOID SetupLight() {
	D3DLIGHT9 light;			// Direct3D 9 ���� ����ü ���� ����
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;		// ���� Ÿ���� �𷺼ųη� ����

	light.Diffuse.r = 1.0f;					// ������ Red �� ����
	light.Diffuse.g = 1.0f;					// ������ Green �� ����
	light.Diffuse.g = 1.0f;					// ������ Blue �� ����

	D3DXVECTOR3 vecDir;						// ���� ���� ���� ����
	vecDir = D3DXVECTOR3(10, -10, 10);		// ������ ���� ( �����ϴ� ����)
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);	//���� ����ȭ

	g_pd3dDevice->SetLight(0, &light);		// ����Ʈ ��ȣ ����
	g_pd3dDevice->LightEnable(0, TRUE);		// 0 �� ����Ʈ �ѱ�
	
	// ����Ʈ ��� ����� TRUE�� ����. ( �� ����� ���� ��� ����Ʈ ��� ����)
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	// ���������� �����Ʈ ����Ʈ �ѱ� ( ȯ����� ���� ����)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00808080);
}


//-------------------------------------------------------
//	�̸� : ChangeSpriteUV()
//	��� : ��������Ʈ�� ���� uv �Լ�
//-------------------------------------------------------
HRESULT ChangeSpriteUV(SPRITE *sp) {
	float u = (sp->curIndex * 64.0f) / 960.0f;			// ���� �ε����� �̿��� u ���
	float u2 = ((sp->curIndex + 1)*64.0f) / 960.0f;		// ���� �ε���+1�� ������ u ���

	TEXTUREVERTEX *pVertices;							// ���ؽ� ���� ���ٿ� ������
	if (FAILED(g_pVBTexture->Lock(0, 0, (void**)&pVertices, 0))) {
		return E_FAIL;
	}

	pVertices[0].tu = u;			// u ��ǥ ����
	pVertices[0].tv = 0.0f;			// v ��ǥ ����

	pVertices[1].tu = u2;			// u ��ǥ ����
	pVertices[1].tv = 0.0f;			// v ��ǥ ����

	pVertices[2].tu = u;			// u ��ǥ ����
	pVertices[2].tv = 1.0f;			// v ��ǥ ����

	pVertices[3].tu = u2;			// u ��ǥ ����
	pVertices[3].tv = 1.0f;			// v ��ǥ ����

	g_pVBTexture->Unlock();

	// ������ ������ �������� ���� ���
	if (sp->frameCounter >= sp->frameCounter) {
		sp->curIndex++;				// �ε��� ����
		if (sp->curIndex == sp->spriteNumber) {		// ������ ����� ����� ����Ǹ�
			sp->state = FALSE;		// ��� ����
		}
	}
	else {
		sp->frameCounter++;			// ������ ī���� �ʱ�ȭ
	}

\
	return S_OK;

}


//-------------------------------------------------------
//	�̸� : InputCheck()
//	��� : ����� �Է��� �˻��Ѵ�.
//-------------------------------------------------------
VOID InputCheck() {
	// ����
	if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W')) {
		g_Camera.x += (float)(g_Camera.radius * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
		g_Camera.z += (float)(g_Camera.radius * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
	}

	// ����
	if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S')) {
		g_Camera.x -= (float)(g_Camera.radius * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
		g_Camera.z -= (float)(g_Camera.radius * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
	}
	
	// �·� ȸ��
	if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A')) {
		g_Camera.angle += 1;
	}

	// ��� ȸ��
	if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D')) {
		g_Camera.angle -= 1;
	}

	if (GetAsyncKeyState(VK_LBUTTON)) {
		if (g_Bullet.state == FALSE) {		// ������ ��� ������ ���
			g_Bullet.state = TRUE;			// ����� ǥ��
		
			g_Bullet.x = g_Camera.x;		// �Ѿ��� ���� ��ġ�� ī�޶��� ��ġ�� ����
			g_Bullet.y = g_Camera.y;
			g_Bullet.z = g_Camera.z;

			// �� �����Ӹ��� �Ѿ��� �̵��� ������ ����
			float speed = 10.0f;
			g_Bullet.deltaX = (float)(speed * cos(g_Camera.angle * (D3DX_PI / 180.0f)));
			g_Bullet.deltaZ = (float)(speed * sin(g_Camera.angle * (D3DX_PI / 180.0f)));
		
		}

	}

}


//-------------------------------------------------------
//	�̸� : BulletControl()
//	��� : �Ѿ��� �̵�, �浹 �� ���� ��Ż �� ó��
//-------------------------------------------------------
void BulletControl() {
	if (g_Bullet.state == FALSE)		// ��������� ���� �Ѿ��� ó������ ����
		return;

	g_Bullet.x += g_Bullet.deltaX;
	g_Bullet.z += g_Bullet.deltaZ;

	// ���� ������ ��踦 ����� �Ѿ� �Ұ� �� ������ �����ϵ��� ���� ����
	if (g_Bullet.x <= -200 || g_Bullet.x >= 200)
		g_Bullet.state = FALSE;

	if (g_Bullet.z <= -200 || g_Bullet.z >= 200)
		g_Bullet.state = FALSE;

	// �Ѿ˰� ��ũ�� �Ÿ� ���
	float distance = (float)sqrt((g_Bullet.x - g_TankX) * (g_Bullet.x - g_TankX) + (g_Bullet.z - g_TankZ) * (g_Bullet.z - g_TankZ));

	// ��ȣ �Ÿ��� 20���� ������ �浹�� ����
	if (distance < 20) {
		g_Bullet.state = FALSE;			// �Ѿ� �Ұ�
		if (g_Fire.state == FALSE) {	// ���� ��������Ʈ�� ��� ������ ���
			// ��������Ʈ �߻� ó��
			g_Fire.x = g_Bullet.x;
			g_Fire.y = g_Bullet.y;
			g_Fire.z = g_Bullet.z;
			g_Fire.frameDelay = 1;		// ������ �����̴� 1;
			g_Fire.curIndex = 0;		// �ε��� ����
			g_Fire.frameCounter = 0;	// ������ ī���� ����
			g_Fire.state = TRUE;		// ����� ǥ��

		}
	}
}





//-------------------------------------------------------
//	�̸� : Render()
//	��� : ȭ���� �׸���.
//-------------------------------------------------------

VOID Render() {

	// if������ NULL ���� ���ʿ� �δ� ������ ���α׷��� �Ǽ��� ���� �ϱ� ����!!
	// ex) g_pd3dDevice = NULL <<< ���� �Ǽ��� �߻� �� ���� �ֱ⶧���� �Ʒ��� ���� �ϴ� ����.

	if (NULL == g_pd3dDevice)		// ��ġ ��ü�� �������� �ʾ����� ����
		return;

	// �� �� �������� ��ȯ ����
	SetupViewProjection();

	// �ﰢ���� ��/�� ���� ��� �������ϵ��� �ø� ����� ����.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// ������ ����. (���� ���� ������ �ƴϰ�, ���ؽ� ��ü�� ������ ����ϵ���), TRUE : ������, FALSE : ���� x
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// ����۸� ������ �������� �����. (���⿡�� ���������� ����)
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// ȭ�� �׸��� ����
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		//// ���ؽ� ��� �κ�

		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));		// 1. ���ؽ� ���� ����
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);								// 2. ���ؽ� ���� ����

		D3DXMATRIXA16 matWorld;		//���� ��ȯ�� ��Ʈ���� ����
																// 3. ���ؽ��� �̿��Ͽ� �׸� ���� ����
		for (float x = -200; x <= 200; x += 20) {				// z �࿡ ������ ������ ���� �� �׸���
			D3DXMatrixTranslation(&matWorld, x, 0.0, 0.0);		// x �࿡ ���� ��ġ �̵� ��Ʈ����
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// ��ȯ ��Ʈ���� ����
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);	// z �� ���� �׸���
		}
		
		for (float z = -200; z <= 200; z += 20) {				// x �࿡ ������ ������ ������ �׸���
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);		// z �࿡ ���� ��ġ �̵� ��Ʈ����
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// ��ȯ ��Ʈ���� ����
			//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);	// x �� ���� �׸���
		}

		D3DXMatrixIdentity(&matWorld);							// ��Ʈ������ ���� ��ķ� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		//��ȯ ��Ʈ���� ����
		//g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);		// y �� ���� �׸���

		// ��ȯ�� ������� ���� ���¿��� �ﰢ�� �׸���
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// x �࿡ ���� ��ġ �̵� ��Ʈ����

		D3DXMatrixTranslation(&matWorld, 100.0f, 0.0f, 0.0f);	// x ������ 100 ��ŭ �̵�, ��Ʈ����

		// <�̵��� ȸ�� ��Ʈ����> = <ȸ�� ��Ʈ����> x <�̵� ��Ʈ����>
		// �Լ���� ����� ����� ǥ���̹Ƿ� ���� ���Ǵ� ��Ʈ������ ������ ��ġ�Ѵ�.

		// ��Ʈ���� ���� �߰�
		D3DXMATRIXA16 matWorld2;
		// matWorld2�� y ���� �߽����� -45�� ȸ���ϴ� ��Ʈ������ �����
		D3DXMatrixRotationY(&matWorld2, -45.0f*(D3DX_PI / 180.0f));
		
		// matWorld2�� �̵� �� ȸ���� �ϴ� ��Ʈ������ ������ִ� ���ϱ�
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		
		// matWorld2�� x �� �������� 2�� Ȯ���ϴ� ��Ʈ������ �����
		D3DXMatrixScaling(&matWorld2, 2.0f, 1.0f, 1.0f);

		// matWorld�� Ȯ�� ��ɱ��� �߰����� ���� ���ϱ�
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		
		// ���� ��ȯ�� ����� ���� ��Ʈ���� matWorld�� ��ġ�� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		// ��ȯ ��Ʈ���� ����	
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// �ﰢ�� �׸���
		

		// ����Ʈ ����
		SetupLight();

		// ���� ����
		D3DMATERIAL9 mtrl;			// ���� ������ ����ü
		ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
		// Ȯ�걤�� �ֺ��� ������ ���Ͽ� R, G, B, A ���� ����, R,G,B : (1,1,0) �����
		mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
		mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
		mtrl.Diffuse.b = mtrl.Ambient.b = 0.0f;
		mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
		
		// ������ ������ ����
		g_pd3dDevice->SetMaterial(&mtrl);

		// �ﰢ�� ����� ���� �̵�
		D3DXMatrixTranslation(&matWorld, 0, 0, -100);		// -z �������� 100 �̵�
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// ��ȯ ��Ʈ���� ����

		// �ﰢ�� ����� ���� ���ؽ� ���� ����
		g_pd3dDevice->SetStreamSource(0, g_pVBLight, 0, sizeof(LIGHTVERTEX));
		g_pd3dDevice->SetFVF(D3DFVF_LIGHTVERTEX);		// ���ؽ� ���� ����
		//g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);	//�ﰢ�� 2�� �׸���

		
		//////////////////// x���� ���
		// ��ī�� �ڽ� ���
		D3DXMatrixScaling(&matWorld, 60.0f, 60.0f, 60.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XFile.XFileDisplay(g_pd3dDevice);


		// ȣ���� ���
		D3DXMatrixTranslation(&matWorld, 60.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);

		// ȣ���� ���
		D3DXMatrixTranslation(&matWorld, 30.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);

		// ȣ���� ���
		D3DXMatrixTranslation(&matWorld, 0.0f, 10.0f, 60.0f);
		D3DXMatrixScaling(&matWorld2, 30.0f, 30.0f, 30.0f);
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
		g_XTiger.XFileDisplay(g_pd3dDevice);


		// �� ����� ���� ��Ʈ���� ��ȯ
		{
		   D3DXMATRIXA16 matWorld;
		   D3DXMatrixTranslation( &matWorld, 0, -2, 0 );
		   D3DXMATRIXA16 matWorld2;
		   D3DXMatrixScaling( &matWorld2, 0.01f, 0.01f, 0.01f);
		   D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		   g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
		}
		// �� ���
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_XHouse.XFileDisplay(g_pd3dDevice);

		// ��ũ ����� ���� ��Ʈ���� ��ȯ
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
		// ��ũ ���
		g_XTank.XFileDisplay(g_pd3dDevice);
		
		D3DXMatrixScaling(&matWorld, 1.0f, 1.0f, 1.0f);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

															
		// �ؽ��� ���
		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);		// ���� ����


		// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// �ؽ��İ� ī�޶� ����� �������� Ȯ��Ǿ� ��µ� �� �簢�� ������� Ȯ�� �Ǵ� ����� ����
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		g_pd3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		// �ٴ� �ؽ��� ���
		g_pd3dDevice->SetTexture(0, g_pFloorTexture);

		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));		// ����� ���ؽ� ���� ����
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);							// FVF �� ����
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2);				// �簢�� ���� ���


		

		// ���� �ؽ��� ���
		if (g_Fire.state == TRUE) {
			g_pd3dDevice->SetTexture(0, g_pTexture);		// �ؽ��� ����, �ؽ��� ��Ī�� ���� g_pTexture ���
			
			// ���ؽ����� ���ļ¿� ���� ���� ����
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

			{
				// �ش� ��ġ�� �̵� �� ������
				MatBillboardMatrix._41 = g_Fire.x;
				MatBillboardMatrix._42 = 0;
				MatBillboardMatrix._43 = g_Fire.z;
				g_pd3dDevice->SetTransform(D3DTS_WORLD, &MatBillboardMatrix);
			}

			g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));		// ����� ���ؽ� ���� ����
			g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);							// FVF �� ����
			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);				// �簢�� ���
			g_pd3dDevice->SetTexture(0, NULL);									// �ؽ��� ���� ����

			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		// ���ؽ����� ���ĺ��� ���Ͽ� ���� ���� ����

			ChangeSpriteUV(&g_Fire);		// ���� ��������Ʈ �ִϸ��̼��� ���� uv �ִϸ��̼� �Լ� ȣ��
		}



		if (g_Bullet.state == TRUE) {
			g_pd3dDevice->SetTexture(0, g_pBulletTexture);

			// ���� �ؽ��� ó�� �κ�
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			{
				// �ش� ��ġ�� �̵� �� ������
				MatBillboardMatrix._41 = g_Bullet.x;
				MatBillboardMatrix._42 = 0;
				MatBillboardMatrix._43 = g_Bullet.z;

				g_pd3dDevice->SetTransform(D3DTS_WORLD, &MatBillboardMatrix);
			}

			g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 8, 2);

			// �Ѿ� ��� �� ó��
			BulletControl();
		}


		// ȭ�� �׸��� ��
		g_pd3dDevice->EndScene();
	}

	// �������� ������ ȭ������ ������.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}