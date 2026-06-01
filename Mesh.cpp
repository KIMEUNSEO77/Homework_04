// Mesh.cpp

#include "stdafx.h"
#include "Mesh.h"

CMeshLoadInfo::~CMeshLoadInfo()
{
	if (m_pxmf3Positions) delete[] m_pxmf3Positions;
	if (m_pxmf4Colors) delete[] m_pxmf4Colors;
	if (m_pxmf3Normals) delete[] m_pxmf3Normals;

	if (m_pnIndices) delete[] m_pnIndices;

	if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;

	if ((m_nSubMeshes > 0) && m_ppnSubSetIndices)
	{
		for (int i = 0; i < m_nSubMeshes; i++) if (m_ppnSubSetIndices[i]) delete[] m_ppnSubSetIndices[i];
		delete[] m_ppnSubSetIndices;
	}
}

// -----------------------------------------------------------------------------------------------------------------

CMeshFromFile::CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo)
{
	m_nVertices = pMeshInfo->m_nVertices;
	m_nType = pMeshInfo->m_nType;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);

	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_nSubMeshes = pMeshInfo->m_nSubMeshes;
	if (m_nSubMeshes > 0)
	{
		m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_nSubMeshes];
		m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_nSubMeshes];

		m_pnSubSetIndices = new int[m_nSubMeshes];

		for (int i = 0; i < m_nSubMeshes; i++)
		{
			m_pnSubSetIndices[i] = pMeshInfo->m_pnSubSetIndices[i];
			m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_ppnSubSetIndices[i], sizeof(UINT) * m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_ppd3dSubSetIndexUploadBuffers[i]);

			m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
			m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
			m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * pMeshInfo->m_pnSubSetIndices[i];
		}
	}
}

CMeshFromFile::~CMeshFromFile()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();

	if (m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexBuffers[i]) m_ppd3dSubSetIndexBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexBuffers) delete[] m_ppd3dSubSetIndexBuffers;
		if (m_pd3dSubSetIndexBufferViews) delete[] m_pd3dSubSetIndexBufferViews;

		if (m_pnSubSetIndices) delete[] m_pnSubSetIndices;
	}
}

void CMeshFromFile::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;

	if ((m_nSubMeshes > 0) && m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_nSubMeshes; i++)
		{
			if (m_ppd3dSubSetIndexUploadBuffers[i]) m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_ppd3dSubSetIndexUploadBuffers) delete[] m_ppd3dSubSetIndexUploadBuffers;
		m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
}

void CMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

// -----------------------------------------------------------------------------------------------------------------

CMeshIlluminatedFromFile::CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo) : CMeshFromFile::CMeshFromFile(pd3dDevice, pd3dCommandList, pMeshInfo)
{
	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CMeshIlluminatedFromFile::~CMeshIlluminatedFromFile()
{
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}

void CMeshIlluminatedFromFile::ReleaseUploadBuffers()
{
	CMeshFromFile::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
}

void CMeshIlluminatedFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

// -----------------------------------------------------------------------------------------------------------------

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	::ZeroMemory(pHeightMapPixels, m_nWidth * m_nLength);

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		hFile = ::CreateFile(_T("x64/Debug/Image/HeightMap.raw"), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	}
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesRead = 0;
		::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
		::CloseHandle(hFile);
	}

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}
	delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0) || (z < 0) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;

	float y1 = float(m_pHeightMapPixels[nHeightMapIndex]) * m_xmf3Scale.y;
	float y2 = float(m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd]) * m_xmf3Scale.y;
	float y3 = float(m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd]) * m_xmf3Scale.y;

	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	return(Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true));
}

float CHeightMapImage::GetHeight(float fx, float fz)
{
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= (m_nWidth - 1)) || (fz >= (m_nLength - 1))) return(0.0f);

	int x = int(fx);
	int z = int(fz);
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = float(m_pHeightMapPixels[x + (z * m_nWidth)]);
	float fBottomRight = float(m_pHeightMapPixels[(x + 1) + (z * m_nWidth)]);
	float fTopLeft = float(m_pHeightMapPixels[x + ((z + 1) * m_nWidth)]);
	float fTopRight = float(m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)]);

	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft)
	{
		if (fzPercent >= fxPercent) fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent)) fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}

	float fTopHeight = fTopLeft * (1.0f - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1.0f - fxPercent) + fBottomRight * fxPercent;
	return(fBottomHeight * (1.0f - fzPercent) + fTopHeight * fzPercent);
}

// -----------------------------------------------------------------------------------------------------------------

CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, void* pContext) : CMesh()
{
	m_nVertices = nWidth * nLength;
	m_nType = VERTEXT_POSITION | VERTEXT_NORMAL;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	XMFLOAT3* pxmf3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT3* pxmf3Normals = new XMFLOAT3[m_nVertices];

	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			pxmf3Positions[i] = XMFLOAT3((x * xmf3Scale.x), OnGetHeight(x, z, pContext), (z * xmf3Scale.z));
			pxmf3Normals[i] = OnGetNormal(x, z, pContext);
		}
	}

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Positions,
		sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Normals,
		sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	delete[] pxmf3Positions;
	delete[] pxmf3Normals;

	m_nIndices = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	UINT* pnIndices = new UINT[m_nIndices];
	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) pnIndices[j++] = UINT(x + (z * nWidth));
				pnIndices[j++] = UINT(x + (z * nWidth));
				pnIndices[j++] = UINT((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) pnIndices[j++] = UINT(x + (z * nWidth));
				pnIndices[j++] = UINT(x + (z * nWidth));
				pnIndices[j++] = UINT((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	delete[] pnIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
	pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
	pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	Render(pd3dCommandList);
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	return(pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y);
}

XMFLOAT3 CHeightMapGridMesh::OnGetNormal(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	return(pHeightMapImage->GetHeightMapNormal(x, z));
}

// -----------------------------------------------------------------------------------------------------------------

namespace
{
	void ReadUnityBinaryString(FILE* pFile, char* pstrToken, BYTE* pnStrLength)
	{
		fread(pnStrLength, sizeof(BYTE), 1, pFile);
		fread(pstrToken, sizeof(char), *pnStrLength, pFile);
		pstrToken[*pnStrLength] = '\0';
	}
}

CBinaryMeshFromFile::CBinaryMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* pstrFileName)
{
	FILE* pFile = NULL;
	fopen_s(&pFile, pstrFileName, "rb");
	if (!pFile) return;

	char pstrToken[256] = { '\0' };
	BYTE nStrLength = 0;
	UINT nReads = 0;
	BoundingBox xmBoundingBox;

	ReadUnityBinaryString(pFile, pstrToken, &nStrLength);
	nReads = (UINT)fread(&xmBoundingBox.Center, sizeof(float), 3, pFile);
	nReads = (UINT)fread(&xmBoundingBox.Extents, sizeof(float), 3, pFile);

	ReadUnityBinaryString(pFile, pstrToken, &nStrLength);
	nReads = (UINT)fread(&m_nVertices, sizeof(int), 1, pFile);
	XMFLOAT3* pxmf3Positions = new XMFLOAT3[m_nVertices];
	nReads = (UINT)fread(pxmf3Positions, sizeof(float), 3 * m_nVertices, pFile);

	ReadUnityBinaryString(pFile, pstrToken, &nStrLength);
	UINT nNormals = 0;
	nReads = (UINT)fread(&nNormals, sizeof(int), 1, pFile);
	XMFLOAT3* pxmf3Normals = new XMFLOAT3[nNormals];
	nReads = (UINT)fread(pxmf3Normals, sizeof(float), 3 * nNormals, pFile);

	ReadUnityBinaryString(pFile, pstrToken, &nStrLength);
	UINT nTextureCoords = 0;
	nReads = (UINT)fread(&nTextureCoords, sizeof(int), 1, pFile);
	XMFLOAT2* pxmf2TextureCoords = new XMFLOAT2[nTextureCoords];
	nReads = (UINT)fread(pxmf2TextureCoords, sizeof(float), 2 * nTextureCoords, pFile);
	delete[] pxmf2TextureCoords;

	ReadUnityBinaryString(pFile, pstrToken, &nStrLength);
	nReads = (UINT)fread(&m_nIndices, sizeof(int), 1, pFile);
	UINT* pnIndices = new UINT[m_nIndices];
	nReads = (UINT)fread(pnIndices, sizeof(UINT), m_nIndices, pFile);
	fclose(pFile);

	m_nType = VERTEXT_POSITION | VERTEXT_NORMAL;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Normals, sizeof(XMFLOAT3) * nNormals, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * nNormals;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	delete[] pxmf3Positions;
	delete[] pxmf3Normals;
	delete[] pnIndices;
}

CBinaryMeshFromFile::~CBinaryMeshFromFile()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CBinaryMeshFromFile::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}

void CBinaryMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CBinaryMeshFromFile::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	Render(pd3dCommandList);
}
// -----------------------------------------------------------------------------------------------------------------
// CCubeMesh

CCubeMesh::CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth)
{
	float x = fWidth * 0.5f;
	float y = fHeight * 0.5f;
	float z = fDepth * 0.5f;

	XMFLOAT3 pxmf3Positions[24] =
	{
		XMFLOAT3(-x, -y, -z), XMFLOAT3(-x, +y, -z), XMFLOAT3(+x, +y, -z), XMFLOAT3(+x, -y, -z),
		XMFLOAT3(+x, -y, +z), XMFLOAT3(+x, +y, +z), XMFLOAT3(-x, +y, +z), XMFLOAT3(-x, -y, +z),
		XMFLOAT3(-x, +y, -z), XMFLOAT3(-x, +y, +z), XMFLOAT3(+x, +y, +z), XMFLOAT3(+x, +y, -z),
		XMFLOAT3(-x, -y, +z), XMFLOAT3(-x, -y, -z), XMFLOAT3(+x, -y, -z), XMFLOAT3(+x, -y, +z),
		XMFLOAT3(-x, -y, +z), XMFLOAT3(-x, +y, +z), XMFLOAT3(-x, +y, -z), XMFLOAT3(-x, -y, -z),
		XMFLOAT3(+x, -y, -z), XMFLOAT3(+x, +y, -z), XMFLOAT3(+x, +y, +z), XMFLOAT3(+x, -y, +z)
	};

	XMFLOAT3 pxmf3Normals[24] =
	{
		XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f),
		XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT3(0.0f, 0.0f, +1.0f), XMFLOAT3(0.0f, 0.0f, +1.0f),
		XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT3(0.0f, +1.0f, 0.0f),
		XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f),
		XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f)
	};

	UINT pnIndices[36] =
	{
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	m_nVertices = 24;
	m_nIndices = 36;
	m_nType = VERTEXT_POSITION | VERTEXT_NORMAL;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Positions, sizeof(pxmf3Positions), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dPositionUploadBuffer);
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(pxmf3Positions);

	m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pxmf3Normals, sizeof(pxmf3Normals), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);
	m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
	m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dNormalBufferView.SizeInBytes = sizeof(pxmf3Normals);

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(pnIndices), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(pnIndices);
}

CCubeMesh::~CCubeMesh()
{
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
}

void CCubeMesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) m_pd3dPositionUploadBuffer->Release();
	m_pd3dPositionUploadBuffer = NULL;
	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}

void CCubeMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
	pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
	pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
}

void CCubeMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	Render(pd3dCommandList);
}