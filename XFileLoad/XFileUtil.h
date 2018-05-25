#ifndef XFILEUTIL_H
#define XFILEUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif	//_MSC_VER

#include<d3dx9.h>
#include<atlconv.h>			//USES_CONVERSION, A2T ���
class CXFileUtil
{
private:
	// x ���� �ε�� ����
	LPD3DXMESH g_pMesh;						//�޽� ��ü
	D3DMATERIAL9* g_pMeshMaterials;			// �޽��� ���� ����
	LPDIRECT3DTEXTURE9* g_pMeshTextures;	// �޽��� ���� �ؽ���
	DWORD g_dwNumMaterials;					// ������ ��

public:
	int XFileDisplay(LPDIRECT3DDEVICE9 pD3DDevice);
	int XfileLoad(LPDIRECT3DDEVICE9 pD3DDevice, TCHAR* xFileName);
	CXFileUtil();
	virtual ~CXFileUtil();
};

#endif // !XFILEUTIL_H