#include "XFileUtil.h"
CXFileUtil::CXFileUtil()
{
	g_pMesh = NULL;						//�޽� ��ü
	g_pMeshMaterials = NULL;			// �޽��� ���� ����
	g_pMeshTextures = NULL;				// �޽��� ���� �ؽ���
	g_dwNumMaterials = NULL;			// ������ ��
}


CXFileUtil::~CXFileUtil()
{
}

int CXFileUtil::XfileLoad(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* xFileName) {
	LPD3DXBUFFER pD3DXMtrlBuffer;
	// x������ �ε��Ѵ�.
	if (FAILED(D3DXLoadMeshFromX(xFileName, D3DXMESH_SYSTEMMEM, pD3DDevice, NULL, &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh))) {
		MessageBox(NULL, TEXT("X���� �ε� ����"), TEXT("�޽��ε� ����"), MB_OK);
		return E_FAIL;
	}

	//�ؽ��� ������ �ٸ� ������ ���� ��츦 ���Ͽ� �ؽ��� �н� ��ġ ���
	TCHAR texturePath[256];
	if (wcschr(xFileName, '/') == NULL) {		// ���� ������ ���
		wsprintf(texturePath, TEXT("./"));
	}
	else {										// ��Ÿ ������ ���
		TCHAR temp[256], *pChar;
		wcscpy_s(temp, xFileName);
		_wcsrev(temp);
		pChar = wcschr(temp, '/');
		wcscpy_s(texturePath, pChar);
		_wcsrev(texturePath);
	}
	
	// x ���� �ε� �ڵ�
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];

	for (DWORD i = 0; i < g_dwNumMaterials; i++) {
		// ���� ����
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		// ������ ���� �����Ʈ ���� ���� (D3DX �� ������ �����Ƿ� ����ڰ� ����)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;

		// �ؽ��� ������ �����ϴ� ���
		if (d3dxMaterials[i].pTextureFilename != NULL && strlen(d3dxMaterials[i].pTextureFilename) > 0) {

			// ��� + �ؽ��� ���� �̸� �����
			TCHAR tempFile[256];
			USES_CONVERSION;
			wsprintf(tempFile, TEXT("%s%s"), texturePath, A2T(d3dxMaterials[i].pTextureFilename));
			
			// �ؽ��� ����
			if (FAILED(D3DXCreateTextureFromFile(pD3DDevice, tempFile, &g_pMeshTextures[i]))) {
				g_pMeshTextures[i] = NULL;

			}

		}
	}

	// ���� ���� ��� ��, ����
	pD3DXMtrlBuffer->Release();
	return S_OK;
}


int CXFileUtil::XFileDisplay(LPDIRECT3DDEVICE9 pD3DDevice) {
	// �޽� ���
	for (DWORD i = 0; i < g_dwNumMaterials; i++) {
		// ���� Sub Set�� ���� ���� ����
		pD3DDevice->SetMaterial(&g_pMeshMaterials[i]);
		// ���� Sub Set�� ���� �ؽ��� ����
		pD3DDevice->SetTexture(0, g_pMeshTextures[i]);

		// Draw the mesh subset
		g_pMesh->DrawSubset(i);
	}
	pD3DDevice->SetTexture(0, NULL);
	return 0;
}