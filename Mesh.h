// Mesh.h

#pragma once

class CMesh
{
public:
	CMesh() { }
	virtual ~CMesh() { }

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual void ReleaseUploadBuffers() { }

protected:
	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nVertices = 0;
	UINT							m_nOffset = 0;

	UINT							m_nType = 0;

public:
	UINT GetType() { return(m_nType); }

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) { }
};

// -----------------------------------------------------------------------------------------------------------------

#define VERTEXT_POSITION			0x01
#define VERTEXT_COLOR				0x02
#define VERTEXT_NORMAL				0x04

class CMeshLoadInfo
{
public:
	CMeshLoadInfo() { }
	~CMeshLoadInfo();

public:
	TCHAR							m_pstrMeshName[256] = { 0 };

	UINT							m_nType = 0x00;

	XMFLOAT3						m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3						m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	int								m_nVertices = 0;
	XMFLOAT3*						m_pxmf3Positions = NULL;
	XMFLOAT4*						m_pxmf4Colors = NULL;
	XMFLOAT3*						m_pxmf3Normals = NULL;

	int								m_nIndices = 0;
	UINT*							m_pnIndices = NULL;

	int								m_nSubMeshes = 0;
	int*							m_pnSubSetIndices = NULL;
	UINT**							m_ppnSubSetIndices = NULL;
};


// -----------------------------------------------------------------------------------------------------------------

class CMeshFromFile : public CMesh
{
public:
	CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CMeshFromFile();

public:
	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource*					m_pd3dPositionBuffer = NULL;
	ID3D12Resource*					m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dPositionBufferView;

	int								m_nSubMeshes = 0;
	int*							m_pnSubSetIndices = NULL;

	ID3D12Resource**				m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource**				m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW*		m_pd3dSubSetIndexBufferViews = NULL;

public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

// -----------------------------------------------------------------------------------------------------------------

class CMeshIlluminatedFromFile : public CMeshFromFile
{
public:
	CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CMeshLoadInfo* pMeshInfo);
	virtual ~CMeshIlluminatedFromFile();

	virtual void ReleaseUploadBuffers();

protected:
	ID3D12Resource*					m_pd3dNormalBuffer = NULL;
	ID3D12Resource*					m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

public:
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

// -----------------------------------------------------------------------------------------------------------------
// 바이너리 메시 파일을 현재 프로젝트에서 렌더링할 수 있게 읽어오는 로더 클래스
// 정점/노멀/인덱스 읽기 -> GPU 버퍼 생성 -> 렌더링할 때 Draw 호출

class CBinaryMeshFromFile : public CMesh
{
public:
	CBinaryMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* pstrFileName);
	virtual ~CBinaryMeshFromFile();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

protected:
	UINT m_nIndices = 0;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dPositionBufferView;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView;
	ID3D12Resource* m_pd3dIndexBuffer = NULL;
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
};

// -----------------------------------------------------------------------------------------------------------------
class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 1.0f, float fHeight = 1.0f, float fDepth = 1.0f);
	virtual ~CCubeMesh();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

protected:
	UINT m_nIndices = 0;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dPositionBufferView;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView;
	ID3D12Resource* m_pd3dIndexBuffer = NULL;
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
};

// -----------------------------------------------------------------------------------------------------------------
class CHeightMapImage
{
private:
	BYTE* m_pHeightMapPixels = NULL;
	int m_nWidth = 0;
	int m_nLength = 0;
	XMFLOAT3 m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage();

	float GetHeight(float x, float z);
	XMFLOAT3 GetHeightMapNormal(int x, int z);

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	BYTE* GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

// -----------------------------------------------------------------------------------------------------------------

class CHeightMapGridMesh : public CMesh
{
public:
	CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		int xStart, int zStart, int nWidth, int nLength,
		XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void* pContext = NULL);
	virtual ~CHeightMapGridMesh();

	virtual void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	virtual float OnGetHeight(int x, int z, void* pContext);
	virtual XMFLOAT3 OnGetNormal(int x, int z, void* pContext);

protected:
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dPositionBufferView;

	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView;

	ID3D12Resource* m_pd3dIndexBuffer = NULL;
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
	UINT m_nIndices = 0;
};