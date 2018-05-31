#include<Windows.h>
#include<d3dx9.h>

LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
HWND hWndMain;
LPCTSTR lpszClass = TEXT("DirectX_3DGame");

HRESULT InitD3D(HWND hWnd);

LPDIRECT3D9 g_pD3D = NULL;					//Direct3D ��ü
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;		// ������ ��ġ(���� ī��)
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	HWND hWnd;
	MSG msg;
	g_hInst = hInstance;

	WNDCLASSEX WndClass = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(NULL), NULL,NULL,NULL,NULL, lpszClass, NULL };
	RegisterClassEx(&WndClass);
	hWnd = CreateWindow(lpszClass, TEXT("DirectX_3DGame"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, GetDesktopWindow(), NULL, WndClass.hInstance, NULL);


	if (SUCCEEDED(InitD3D(hWnd))) {

		// ������ ���
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

	D3DPRESENT_PARAMETERS d3dpp;					// ��ġ ������ ���� ����ü ���� ����
	ZeroMemory(&d3dpp, sizeof(d3dpp));				// ���� Ŭ����
	d3dpp.BackBufferWidth = 800;					// ���� �ػ� ���� ����
	d3dpp.BackBufferHeight = 600;					// ���� �ػ� ���� ����
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;		// �ȼ� ����W
	d3dpp.BackBufferCount = 1;						// ����� ��
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;	// �ĸ� ���ۿ� �̿��� ���ø� Ÿ��
	d3dpp.MultiSampleQuality = 0;					// ��Ƽ ���ø��� ����
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;		// ���� ��� ����
	d3dpp.hDeviceWindow = hWnd;						// ������ �ڵ� ����
	d3dpp.Windowed = true;							// ������ ��� ����, false : ��ü ȭ��
	d3dpp.EnableAutoDepthStencil = true;			// ���Ľ� ���� ���
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;	// ���̹��� 24��Ʈ,  ���Ľ� ���� 8��Ʈ ��� ����
	d3dpp.Flags = 0;

	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;		// ����� ���� , D3DPRESENT_RATE_DEFAULT : ����Ʈ��
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;		// ������ ���, D3DPRESENT_INTERVAL_IMMEDIATE : ��� �ÿ�, D3DPRESENT_INTERVAL_DEFAULT : Direct3�� �ÿ����� ����, ���� ������� ����

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