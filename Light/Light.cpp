#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
VOID CleanUP();
VOID SetupViewProjection();
VOID Render();

HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("D3D Game, GgooM94");

LPDIRECT3D9 g_pD3D = NULL;					// Direct3D ��ü
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// ������ ��ġ(���� ī��)
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;		// ���ؽ� ����

// Ŀ���� ���ؽ� Ÿ�� ����ü
struct CUSTOMVERTEX
{
	FLOAT x, y, z;		// 3D ��ǥ��
	DWORD color;		// ���ؽ� ����
};

// Ŀ���� ���ؽ��� ������ ǥ���ϱ� ���� FVF (Flexible Vertex Format) ��
// D3DFVF_XYZ (3D ���� ��ǥ) �� D3DFVF_DIFFUSE (���� ����) Ư���� ��������.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE)


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	// Direct3D �ʱ�ȭ�� �����ϰ�, ���ؽ� ���� ������ �����ϸ� ����, �����ϸ� �����Ѵ�.
	if (SUCCEEDED(InitD3D(hWnd)) && SUCCEEDED(InitGeometry())) {

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
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;


	// ��ġ ������ ������ �غ�
	D3DPRESENT_PARAMETERS d3dpp;				// ��ġ ������ ���� ����ü ���� ����
	ZeroMemory(&d3dpp, sizeof(d3dpp));			// ���� Ŭ����
	d3dpp.Windowed = true;						// ������ ���� ���� ����
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;	// ���� ��� ����

	// ����� ������ ���� ����ũž �������� ����
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// D3D��ü�� ��ġ ���� �Լ� ȣ�� (����Ʈ ����ī�� ��� HAL ���, ����Ʈ���� ���ؽ� ó����� ����)
	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice))) {
		return E_FAIL;
	}

	// ���� ��ġ�� ���������� �����Ǿ���.
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
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// ����۸� ������ �������� �����. (���⿡�� ���������� ����)
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

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
	//	�̸� : ����
	//	��� : �𷺼ų� ����Ʈ ����
	//-------------------------------------------------------
	D3DLIGHT9 light;								// Direct3D9 ���� ����ü ���� ����
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;				// ���� Ÿ���� �𷺼ųη� ����

	light.Diffuse.r = 1.0f;							// ������ Red �� ����
	light.Diffuse.g = 1.0f;							// ������ Green �� ����
	light.Diffuse.b = 1.0f;							// ������ Blue �� ����

	D3DXVECTOR3 vecDir;								// ���� ���� ���� ����
	vecDir = D3DXVECTOR3(10, -2, 30);				// ������ ���� ( �����ϴ� ���� )
	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);	// ���� ����ȭ

	g_pd3dDevice->SetLight(0, &light);				// ����Ʈ ��ȣ ����
	g_pd3dDevice->LightEnable(0, TRUE);				// 0�� ����Ʈ Ȱ��ȭ

	//-------------------------------------------------------
	//	�̸� : ����
	//	��� : ����Ʈ ����Ʈ ����
	//-------------------------------------------------------
	D3DLIGHT9 light1;								// Direct3D9 ���� ����ü ���� ����
	ZeroMemory(&light1, sizeof(D3DLIGHT9));
	light1.Type = D3DLIGHT_POINT;					// ���� Ÿ���� ����Ʈ�� ����

	light1.Diffuse.r = 1.0f;						// ������ Red �� ����
	light1.Diffuse.g = 1.0f;						// ������ Green �� ����
	light1.Diffuse.b = 1.0f;						// ������ Blue �� ����

	light1.Attenuation0 = 0.000000001f;				// �������κ��� �־��� ���� ���Ⱚ
	light1.Range = 5000000.0f;						// ������ ��ġ�� ����

	light1.Position.x = -2.0f;						// ������ ��ġ�ϴ� x ��ǥ
	light1.Position.y = 0.0f;						// ������ ��ġ�ϴ� y ��ǥ
	light1.Position.z = 0.0f;						// ������ ��ġ�ϴ� z ��ǥ

	g_pd3dDevice->SetLight(1, &light1);				// ����Ʈ ��ȣ ����
	g_pd3dDevice->LightEnable(1, TRUE);				// 1�� ����Ʈ �ѱ�


	//-------------------------------------------------------
	//	�̸� : ����
	//	��� : ����Ʈ ����Ʈ ����
	//-------------------------------------------------------

	D3DLIGHT9 light2;								// Direct3D9 ���� ����ü ���� ����
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light2.Type = D3DLIGHT_SPOT;					// ���� Ÿ���� ����Ʈ�� ����

	light2.Diffuse.r = 1.0f;						// ������ Red �� ����
	light2.Diffuse.g = 0.0f;						// ������ Green �� ����
	light2.Diffuse.b = 0.0f;						// ������ Blue �� ����

	light2.Attenuation0 = 0.0000001f;				// ���Ⱚ 0 ����
	light2.Attenuation1 = 0.0000001f;				// ���Ⱚ 1 ����
	light2.Attenuation2 = 0.0000001f;				// ���Ⱚ 2 ����
	light2.Range = 500000.0f;						// ������ ��ġ�� ���� ����

	light2.Position.x = 30;
	light2.Position.y = 50;
	light2.Position.z = -50;

	light2.Falloff = 1.0f;							// ������ �ܺ� ���ֿ��� ���� ���� ���� �� ������
	light2.Theta = D3DX_PI / 4.0f;					// ������ ���� ��
	light2.Phi = D3DX_PI / 2.0;						// ������ �ܺ� ��

	D3DXVECTOR3 vecDir2;							// ���� ���� ���� ����
	vecDir2 = D3DXVECTOR3(-10, -50, 50);			// ������ ���� ���� ����
	D3DXVec3Normalize((D3DXVECTOR3*)&light2.Direction, &vecDir2);	// ���� ����ȭ

	g_pd3dDevice->SetLight(2, &light2);				// ����Ʈ ��ȣ ����
	g_pd3dDevice->LightEnable(2, TRUE);				// 1�� ����Ʈ �ѱ�


	// ����Ʈ ��� ����� TRUE�� ��. ( �� ����� ���� ��� ����Ʈ ����� �����ȴ�. )
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	// ���������� �����Ʈ (ȯ�汤) ����Ʈ �ѱ� ( ȯ�汤�� ���� ����)
	g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x00202020);
	



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

		// ��ȯ�� ������� ���� ���¿��� �ﰢ�� �׸���
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// x �࿡ ���� ��ġ �̵� ��Ʈ����

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

		//matWorld�� Ȯ�� ��ɱ��� �߰����� ���� ���ϱ�
		D3DXMatrixMultiply(&matWorld, &matWorld2, &matWorld);
		
		//���� ��ȯ�� ����� ���� ��Ʈ���� matWorld�� ��ġ�� ����
		g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);		// ��ȯ ��Ʈ���� ����	
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 6, 1);	// �ﰢ�� �׸���
		

		// ȭ�� �׸��� ��
		g_pd3dDevice->EndScene();
	}

	// �������� ������ ȭ������ ������.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}