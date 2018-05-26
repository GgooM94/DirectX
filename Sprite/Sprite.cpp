#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
HRESULT InitGeometryLight();
HRESULT InitGeometryTexture();
VOID CleanUP();
VOID SetupViewProjection();
VOID SetupLight();
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

struct SPRITE
{
	int spriteNumber;		// ��ü ��������Ʈ �̹��� ��
	int curIndex;			// ���� ����ؾ� �ϴ� ��������Ʈ �ε���
	int frameCounter;		// ���� ��������Ʈ�� ����ϰ� ���ӵ� ������ ��
	int frameDelay;			// ��������Ʈ ���� �ӵ� ������ ���� ������ ������
};

// ��������Ʈ ����ü ���� ���� �� �ʱ�ȭ
SPRITE g_Fire = { 15, 0, 0, 3 };		// ���� �� �ʱ�ȭ
HRESULT ChangeSpriteUV(SPRITE *sp);

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
		SUCCEEDED(InitGeometryTexture()))  	// �ؽ��� ���� ������ ����
		{			// ��Ÿ ���� ������ �ε�

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
	//�ؽ��� �ε�
	if (FAILED(D3DXCreateTextureFromFile(g_pd3dDevice, TEXT("./Image/Fire.bmp"), &g_pTexture))) {
		return E_FAIL;
	}

	// ���ؽ� ���� ����
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(4 * sizeof(TEXTUREVERTEX), 0, D3DFVF_TEXTUREVERTEX, D3DPOOL_DEFAULT, &g_pVBTexture, NULL))) {
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
	pVertices[1].tu = 64.0f / 960.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[1].tv = 0.0f;									// ���ؽ� V �ؽ��� ��ǥ

	pVertices[2].position = D3DXVECTOR3(-50, 0, 100);		// ���ؽ� ��ġ
	pVertices[2].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[2].tu = 0.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[2].tv = 1.0f;									// ���ؽ� V �ؽ��� ��ǥ

	pVertices[3].position = D3DXVECTOR3(50, 0, 100);		// ���ؽ� ��ġ
	pVertices[3].color = 0xffffffff;						// ���ؽ� ���� �� ����
	pVertices[3].tu = 64.0f / 960.0f;									// ���ؽ� U �ؽ��� ��ǥ
	pVertices[3].tv = 1.0f;									// ���ؽ� V �ؽ��� ��ǥ

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
	D3DXVECTOR3 vEyePt(100.0f, 250.0f, -400.0f);		// ī�޶��� ��ġ
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);		// �ٶ󺸴� ����
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);			// ������ ����
	D3DXMATRIXA16 matView;							// �亯ȯ�� ��Ʈ����

													// �� ��Ʈ���� ����
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	// Direct3D ��ġ�� �� ��Ʈ���� ����
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

	//// �������� ��ȯ ����
	D3DXMATRIXA16 matProj;							// �������ǿ� ��Ʈ����
													// �������� ��Ʈ���� ����
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 1000.0f);
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
	if (sp->frameCounter >= sp->spriteNumber) {
		sp->curIndex = (sp->curIndex + 1) % sp->spriteNumber;		//�ε��� ����
		sp->frameCounter = 0;		// ������ ī���� �ʱ�ȭ
	}
	else {
		sp->frameCounter++;			// ������ ī���� �ʱ�ȭ
	}

	return S_OK;

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
			g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 2, 1);	// z �� ���� �׸���
		}

		for (float z = -200; z <= 200; z += 20) {				// x �࿡ ������ ������ ������ �׸���
			D3DXMatrixTranslation(&matWorld, 0.0, 0.0, z);		// z �࿡ ���� ��ġ �̵� ��Ʈ����
			g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);	// ��ȯ ��Ʈ���� ����
			g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 0, 1);	// x �� ���� �׸���
		}

		D3DXMatrixIdentity(&matWorld);							// ��Ʈ������ ���� ��ķ� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		//��ȯ ��Ʈ���� ����
		g_pd3dDevice->DrawPrimitive(D3DPT_LINELIST, 4, 1);		// y �� ���� �׸���

		// ���� ��ȯ�� ����� ���� ��Ʈ���� matWorld�� ��ġ�� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		// ��ȯ ��Ʈ���� ����	
														// ����Ʈ ����
		SetupLight();

		
		// �ؽ��� ���

		g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);		// ���� ����
		g_pd3dDevice->SetTexture(0, g_pTexture);					// �ؽ��� ����

																	// �ؽ��� ��� ȯ�� ����
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		// ���ؽ����� ���ļ¿� ���� ���� ����
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		g_pd3dDevice->SetStreamSource(0, g_pVBTexture, 0, sizeof(TEXTUREVERTEX));		// ����� ���ؽ� ���� ����
		g_pd3dDevice->SetFVF(D3DFVF_TEXTUREVERTEX);							// FVF �� ����
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);				// �簢�� ���
		g_pd3dDevice->SetTexture(0, NULL);									// �ؽ��� ���� ����

		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);		// ���ؽ����� ���ĺ��� ���Ͽ� ���� ���� ����

		ChangeSpriteUV(&g_Fire);		// ���� ��������Ʈ �ִϸ��̼��� ���� uv �ִϸ��̼� �Լ� ȣ��

										// ȭ�� �׸��� ��
		g_pd3dDevice->EndScene();
	}

	// �������� ������ ȭ������ ������.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}