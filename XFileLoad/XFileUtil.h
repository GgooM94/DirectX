#ifndef XFILEUTIL_H
#define XFILEUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif	//_MSC_VER

#include<d3dx9.h>
#include<atlconv.h>			//USES_CONVERSION, A2T 사용
class CXFileUtil
{
private:
	// x 파일 로드용 변수
	LPD3DXMESH g_pMesh;						//메쉬 객체
	D3DMATERIAL9* g_pMeshMaterials;			// 메쉬에 대한 재질
	LPDIRECT3DTEXTURE9* g_pMeshTextures;	// 메쉬에 대한 텍스쳐
	DWORD g_dwNumMaterials;					// 재질의 수

public:
	int XFileDisplay(LPDIRECT3DDEVICE9 pD3DDevice);
	int XfileLoad(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* xFileName);
	CXFileUtil();
	virtual ~CXFileUtil();
};

#endif // !XFILEUTIL_H