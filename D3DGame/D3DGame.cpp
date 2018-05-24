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
	MSG Message;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("D3D Game Program"), WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	// Direct3D �ʱ�ȭ�� �����ϰ�, ���ؽ� ���� ������ �����ϸ� ����, �����ϸ� �����Ѵ�.
	if (SUCCEEDED(InitD3D(hWnd)) && SUCCEEDED(InitGeometry())) {

		// ������ ���
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);

		// �޽��� ���� �����ϱ�
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
		{ 0.0f, 100.0f, 0.0f, 0xffff0000, },		// ���ؽ��� ��ġ�� ����
		{100.0f, -100.0f, 0.0f, 0xff00ffff, },
		{-100.0f, -100.0f, 0.0f, 0xff00ff00, },

		// 0xffff0000 : 0x	ff (����)	00 (Red)	ff (Green)	ff (Blue)
	};

	// ���ؽ� ���۸� �����Ѵ�. 
	// �� ���ؽ��� ������ D3DFVF_CUSTOMVERTEX ��� �͵� ����
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL))) {
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
//	�̸� : Render()
//	��� : ȭ���� �׸���.
//-------------------------------------------------------

VOID Render() {

	// if������ NULL ���� ���ʿ� �δ� ������ ���α׷��� �Ǽ��� ���� �ϱ� ����!!
	// ex) g_pd3dDevice = NULL <<< ���� �Ǽ��� �߻� �� ���� �ֱ⶧���� �Ʒ��� ���� �ϴ� ����.

	if (NULL == g_pd3dDevice)		// ��ġ ��ü�� �������� �ʾ����� ����
		return;

	// ����۸� ������ �������� �����. (���⿡�� û������ ����)
	g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	// ȭ�� �׸��� ����
	if (SUCCEEDED(g_pd3dDevice->BeginScene())) {
		//// ���ؽ� ��� �κ�
		// 1. ���ؽ� ���� ����
		g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));

		// 2. ���ؽ� ���� ����
		g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

		// 3. ���ؽ��� �̿��Ͽ� �׸� ���� ����
		g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

		//ȭ�� �׸��� ��
		g_pd3dDevice->EndScene();
	}

	// �������� ������ ȭ������ ������.
	g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}