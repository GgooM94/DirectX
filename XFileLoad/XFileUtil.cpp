#include "XFileUtil.h"
CXFileUtil::CXFileUtil()
{
	g_pMesh = NULL;						//메쉬 객체
	g_pMeshMaterials = NULL;			// 메쉬에 대한 재질
	g_pMeshTextures = NULL;				// 메쉬에 대한 텍스쳐
	g_dwNumMaterials = NULL;			// 재질의 수
}


CXFileUtil::~CXFileUtil()
{
}

int CXFileUtil::XfileLoad(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* xFileName) {
	LPD3DXBUFFER pD3DXMtrlBuffer;
	// x파일을 로딩한다.
	if (FAILED(D3DXLoadMeshFromX(xFileName, D3DXMESH_SYSTEMMEM, pD3DDevice, NULL, &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh))) {
		MessageBox(NULL, TEXT("X파일 로드 실패"), TEXT("메쉬로드 실패"), MB_OK);
		return E_FAIL;
	}

	//텍스쳐 파일이 다른 폴더에 있을 경우를 위하여 텍스쳐 패스 위치 얻기
	TCHAR texturePath[256];
	if (wcschr(xFileName, '/') == NULL) {		// 현재 폴더의 경우
		wsprintf(texturePath, TEXT("./"));
	}
	else {										// 기타 폴더의 경우
		TCHAR temp[256], *pChar;
		wcscpy_s(temp, xFileName);
		_wcsrev(temp);
		pChar = wcschr(temp, '/');
		wcscpy_s(texturePath, pChar);
		_wcsrev(texturePath);
	}
	
	// x 파일 로딩 코드
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];

	for (DWORD i = 0; i < g_dwNumMaterials; i++) {
		// 재질 복사
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		// 재질에 대한 엠비언트 색상 설정 (D3DX 가 해주지 않으므로 사용자가 설정)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;

		// 텍스쳐 파일이 존재하는 경우
		if (d3dxMaterials[i].pTextureFilename != NULL && strlen(d3dxMaterials[i].pTextureFilename) > 0) {

			// 경로 + 텍스쳐 파일 이름 만들기
			TCHAR tempFile[256];
			USES_CONVERSION;
			wsprintf(tempFile, TEXT("%s%s"), texturePath, A2T(d3dxMaterials[i].pTextureFilename));
			
			// 텍스쳐 생성
			if (FAILED(D3DXCreateTextureFromFile(pD3DDevice, tempFile, &g_pMeshTextures[i]))) {
				g_pMeshTextures[i] = NULL;

			}

		}
	}

	// 재질 버퍼 사용 끝, 해제
	pD3DXMtrlBuffer->Release();
	return S_OK;
}


int CXFileUtil::XFileDisplay(LPDIRECT3DDEVICE9 pD3DDevice) {
	// 메쉬 출력
	for (DWORD i = 0; i < g_dwNumMaterials; i++) {
		// 현재 Sub Set에 대한 재질 설정
		pD3DDevice->SetMaterial(&g_pMeshMaterials[i]);
		// 현재 Sub Set에 대한 텍스쳐 설정
		pD3DDevice->SetTexture(0, g_pMeshTextures[i]);

		// Draw the mesh subset
		g_pMesh->DrawSubset(i);
	}
	pD3DDevice->SetTexture(0, NULL);
	return 0;
}