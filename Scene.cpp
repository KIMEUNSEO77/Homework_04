// CScene.cpp

#include "stdafx.h"
#include "Scene.h"

namespace
{
	// 지형 객체가 있으면 x,z 위치의 높이에 보정값을 더해 y 좌표를 계산
	float TerrainY(CHeightMapTerrain* pTerrain, float x, float z, float fOffset)
	{
		return((pTerrain) ? (pTerrain->GetHeight(x, z) + fOffset) : fOffset);
	}
	// xz 거리 계산
	float DistanceXZ(XMFLOAT3 a, XMFLOAT3 b)
	{
		float x = a.x - b.x;
		float z = a.z - b.z;

		return(sqrtf((x * x) + (z * z)));
	}

	float RandomRange(float fMin, float fMax)
	{
		return(fMin + ((fMax - fMin) * (rand() / float(RAND_MAX))));
	}
	// 목표 위치를 바라보도록 y축 회전값 계산
	void TurnObjectToTarget(CGameObject* pObject, XMFLOAT3 xmf3Target)
	{
		if (!pObject) return;

		XMFLOAT3 xmf3Position = pObject->GetPosition();
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Target, xmf3Position);

		xmf3Direction.y = 0.0f;

		float fLength = Vector3::Length(xmf3Direction);
		if (fLength < 0.001f) return;

		xmf3Direction = Vector3::ScalarProduct(xmf3Direction, 1.0f / fLength, false);

		XMFLOAT3 xmf3Look = pObject->GetLook();
		xmf3Look.y = 0.0f;
		fLength = Vector3::Length(xmf3Look);
		if (fLength < 0.001f) return;
		xmf3Look = Vector3::ScalarProduct(xmf3Look, 1.0f / fLength, false);

		float fDot = max(-1.0f, min(1.0f, (xmf3Look.x * xmf3Direction.x) + (xmf3Look.z * xmf3Direction.z)));
		float fCross = (xmf3Look.z * xmf3Direction.x) - (xmf3Look.x * xmf3Direction.z);
		float fYaw = XMConvertToDegrees(atan2f(fCross, fDot));

		pObject->Rotate(0.0f, fYaw, 0.0f);
	}

	// 외부 Binary Mesh 파일을 읽어 지형 높이에 맞는 건물 오브젝트를 생성
	CGameObject* CreateHouseObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		const char* pstrMeshFile, float x, float z, float fScale, float fYaw, CHeightMapTerrain* pTerrain)
	{
		CGameObject* pHouseObject = new CGameObject();

		pHouseObject->SetMesh(new CBinaryMeshFromFile(pd3dDevice, pd3dCommandList, pstrMeshFile));
		pHouseObject->m_nMaterials = 1;
		pHouseObject->m_ppMaterials = new CMaterial * [1];
		pHouseObject->m_ppMaterials[0] = new CMaterial();
		pHouseObject->m_ppMaterials[0]->SetPseudoLightingShader();
		pHouseObject->SetScale(fScale, fScale, fScale);
		pHouseObject->Rotate(0.0f, fYaw, 0.0f);
		pHouseObject->SetPosition(x, TerrainY(pTerrain, x, z, 4.0f), z);

		return(pHouseObject);
	}
}


// 글자나 버튼 구성하는 큐브 오브젝트 생성해 벡터에 추가
static CGameObject* CreateTextCube(vector<CGameObject*>& vObjects, CMesh* pMesh, float x, float y, float z, XMFLOAT4 xmf4Color)
{
	CGameObject* pObject = new CGameObject();

	pObject->SetMesh(pMesh);
	pObject->m_nMaterials = 1;
	pObject->m_ppMaterials = new CMaterial * [1];
	pObject->m_ppMaterials[0] = new CMaterial();
	pObject->m_ppMaterials[0]->SetPseudoLightingShader();
	pObject->SetColor(xmf4Color);
	pObject->SetPosition(x, y, z);
	vObjects.push_back(pObject);

	return(pObject);
}

// 글자 패턴 가로 칸 수 계산
static int GetPatternWidth(const char** ppPattern)
{
	int nWidth = 0;
	while (ppPattern[0][nWidth] != '\0') nWidth++;
	return(nWidth);
}

// 글자 패턴에서 값이 1인 위치마다 큐브를 배치
static int AddPatternGlyph(vector<CGameObject*>& vObjects, CMesh* pMesh, const char** ppPattern, float fLeft, float fTop, float fZ, float fStep, XMFLOAT4 xmf4Color)
{
	int nMaxWidth = 0;
	for (int y = 0; y < 7; y++)
	{
		int nWidth = 0;
		while (ppPattern[y][nWidth] != '\0') nWidth++;
		if (nMaxWidth < nWidth) nMaxWidth = nWidth;

		for (int x = 0; x < nWidth; x++)
		{
			if (ppPattern[y][x] == '1') CreateTextCube(vObjects, pMesh, fLeft + (x * fStep), fTop - (y * fStep), fZ, xmf4Color);
		}
	}
	return(nMaxWidth);
}

static const char* g_pA[7] = { "01110", "10001", "10001", "11111", "10001", "10001", "10001" };
static const char* g_pC[7] = { "01111", "10000", "10000", "10000", "10000", "10000", "01111" };
static const char* g_pD[7] = { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
static const char* g_pE[7] = { "11111", "10000", "10000", "11110", "10000", "10000", "11111" };
static const char* g_pG[7] = { "01111", "10000", "10000", "10011", "10001", "10001", "01111" };
static const char* g_pI[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "11111" };
static const char* g_pL[7] = { "10000", "10000", "10000", "10000", "10000", "10000", "11111" };
static const char* g_pM[7] = { "10001", "11011", "10101", "10101", "10001", "10001", "10001" };
static const char* g_pN[7] = { "10001", "11001", "10101", "10011", "10001", "10001", "10001" };
static const char* g_pO[7] = { "01110", "10001", "10001", "10001", "10001", "10001", "01110" };
static const char* g_pR[7] = { "11110", "10001", "10001", "11110", "10100", "10010", "10001" };
static const char* g_pS[7] = { "01111", "10000", "10000", "01110", "00001", "00001", "11110" };
static const char* g_pT[7] = { "11111", "00100", "00100", "00100", "00100", "00100", "00100" };
static const char* g_pU[7] = { "10001", "10001", "10001", "10001", "10001", "10001", "01110" };
static const char* g_pV[7] = { "10001", "10001", "10001", "10001", "01010", "01010", "00100" };
static const char* g_pHyphen[7] = { "00000", "00000", "00000", "11111", "00000", "00000", "00000" };
static const char* g_p1[7] = { "00100", "01100", "00100", "00100", "00100", "00100", "11111" };
static const char* g_p2[7] = { "11110", "00001", "00001", "11110", "10000", "10000", "11111" };
static const char* g_p3[7] = { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };

// 문자 하나에 대응하는 7줄짜리 큐브 글자 패턴을 반환
static const char** GetEnglishGlyph(char ch)
{
	switch (ch)
	{
	case 'A': return(g_pA);
	case 'C': return(g_pC);
	case 'D': return(g_pD);
	case 'E': return(g_pE);
	case 'G': return(g_pG);
	case 'I': return(g_pI);
	case 'L': return(g_pL);
	case 'M': return(g_pM);
	case 'N': return(g_pN);
	case 'O': return(g_pO);
	case 'R': return(g_pR);
	case 'S': return(g_pS);
	case 'T': return(g_pT);
	case 'U': return(g_pU);
	case 'V': return(g_pV);
	case '-': return(g_pHyphen);
	case '1': return(g_p1);
	case '2': return(g_p2);
	case '3': return(g_p3);
	default: return(NULL);
	}
}

// 문자열 전체의 폭을 계산한 후 여러 글자 패턴을 이어 붙여 큐브 글자를 만듦
static void AddCubeLabel(vector<CGameObject*>& vObjects, CMesh* pMesh, const char* pstrText, float fCenterX, float fCenterY, float fZ, float fStep, float fGap, XMFLOAT4 xmf4Color)
{
	float fTotalWidth = 0.0f;
	int nLetters = (int)strlen(pstrText);
	for (int i = 0; i < nLetters; i++)
	{
		const char** ppGlyph = GetEnglishGlyph(pstrText[i]);
		fTotalWidth += (pstrText[i] == ' ') ? (fStep * 4.0f) : ((ppGlyph ? GetPatternWidth(ppGlyph) : 5) * fStep);
		if (i < (nLetters - 1)) fTotalWidth += fGap;
	}

	float fLeft = fCenterX - (fTotalWidth * 0.5f);
	float fTop = fCenterY + (3.0f * fStep);
	for (int i = 0; i < nLetters; i++)
	{
		if (pstrText[i] == ' ')
		{
			fLeft += (fStep * 4.0f) + fGap;
			continue;
		}

		const char** ppGlyph = GetEnglishGlyph(pstrText[i]);
		if (ppGlyph)
		{
			int nWidth = AddPatternGlyph(vObjects, pMesh, ppGlyph, fLeft, fTop, fZ, fStep, xmf4Color);
			fLeft += (nWidth * fStep) + fGap;
		}
	}
}

// 버튼의 위, 아래, 왼쪽, 오른쪽 테두리를 큐브로 배치
static void AddButtonFrame(vector<CGameObject*>& vObjects, CMesh* pMesh, float fCenterX, float fCenterY, float fZ, float fWidth, float fHeight, float fStep, XMFLOAT4 xmf4Color)
{
	float fLeft = fCenterX - (fWidth * 0.5f);
	float fRight = fCenterX + (fWidth * 0.5f);
	float fTop = fCenterY + (fHeight * 0.5f);
	float fBottom = fCenterY - (fHeight * 0.5f);

	for (float x = fLeft; x <= fRight; x += fStep)
	{
		CreateTextCube(vObjects, pMesh, x, fTop, fZ, xmf4Color);
		CreateTextCube(vObjects, pMesh, x, fBottom, fZ, xmf4Color);
	}
	for (float y = fBottom; y <= fTop; y += fStep)
	{
		CreateTextCube(vObjects, pMesh, fLeft, y, fZ, xmf4Color);
		CreateTextCube(vObjects, pMesh, fRight, y, fZ, xmf4Color);
	}
}

static void AddCubeButton(vector<CGameObject*>& vObjects, CMesh* pButtonMesh, CMesh* pTextMesh, const char* pstrText, float fCenterX, float fCenterY, float fWidth, float fHeight, XMFLOAT4 xmf4Color)
{
	AddButtonFrame(vObjects, pButtonMesh, fCenterX, fCenterY, 0.0f, fWidth, fHeight, 8.0f, xmf4Color);
	AddCubeLabel(vObjects, pTextMesh, pstrText, fCenterX, fCenterY, 0.0f, 4.0f, 3.0f, xmf4Color);
}

static void AddResultButton(vector<CGameObject*>& vObjects, CMesh* pButtonMesh, const char* pstrText, float fCenterX, float fCenterY, float fWidth, float fHeight)
{
	XMFLOAT4 xmf4Color(0.9f, 0.9f, 0.95f, 1.0f);
	AddButtonFrame(vObjects, pButtonMesh, fCenterX, fCenterY, 0.0f, fWidth, fHeight, 8.0f, xmf4Color);
	AddCubeLabel(vObjects, pButtonMesh, pstrText, fCenterX, fCenterY, 0.0f, 4.2f, 5.0f, xmf4Color);
}

CScene::CScene()
{
}

CScene::~CScene()
{
}

CGameObject* CScene::CreateColorCube(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 xmf4Color, float fSize)
{
	CGameObject* pObject = new CGameObject();

	pObject->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList, fSize, fSize, fSize));
	pObject->m_nMaterials = 1;
	pObject->m_ppMaterials = new CMaterial * [1];
	pObject->m_ppMaterials[0] = new CMaterial();
	pObject->m_ppMaterials[0]->SetPseudoLightingShader();
	pObject->SetColor(xmf4Color);
	pObject->SetPosition(0.0f, -10000.0f, 0.0f);

	return(pObject);
}

void CScene::ReleaseSceneObjects(CGameObject** ppObjects, int nObjects)
{
	if (!ppObjects) return;
	for (int i = 0; i < nObjects; i++) if (ppObjects[i]) delete ppObjects[i];
	delete[] ppObjects;
}

void CScene::RenderSceneObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** ppObjects, int nObjects)
{
	if (!ppObjects) return;
	for (int i = 0; i < nObjects; i++) if (ppObjects[i]) ppObjects[i]->Render(pd3dCommandList, pCamera);
}

// 시작 화면
void CScene::BuildTitleObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	CMesh* pMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 4.0f, 4.0f, 4.0f);

	const char* p3[7] = { "11110", "00001", "00001", "01110", "00001", "00001", "11110" };
	const char* pD[7] = { "11110", "10001", "10001", "10001", "10001", "10001", "11110" };
	const char* p1[7] = { "00100", "01100", "00100", "00100", "00100", "00100", "11111" };
	const char* pGe[7] = { "1110101", "0010101", "0010101", "0011101", "0010101", "0010101", "0010101" };
	const char* pIm[7] = { "0111001", "1000101", "1000101", "0111001", "1111100", "1000100", "1111100" };
	const char* pPeu[7] = { "11111111", "00100100", "00100100", "11111111", "00000000", "11111111", "00000000" };
	const char* pRo[7] = { "111100", "000100", "111100", "100000", "111100", "001000", "111110" };
	const char* pGeu[7] = { "111110", "000010", "000010", "000010", "000000", "111110", "000000" };
	const char* pRae[7] = { "111001001", "001001001", "001001001", "111101101", "100001001", "111101001", "000001001" };
	const char* pMing[7] = { "111101", "100101", "111101", "000100", "011100", "100010", "011100" };
	const char* pKim[7] = { "11101", "00101", "00101", "00101", "11111", "10001", "11111" };
	const char* pEun[7] = { "01110", "10001", "10001", "01110", "11111", "10000", "11111" };
	const char* pSeo[7] = { "00001", "00101", "01001", "10011", "01001", "00001", "00001" };

	const char** ppTitle[10] = { p3, pD, pGe, pIm, pPeu, pRo, pGeu, pRae, pMing, p1 };
	const char** ppName[3] = { pKim, pEun, pSeo };

	float fX = -210.0f;
	for (int i = 0; i < 10; i++)
	{
		int nWidth = AddPatternGlyph(vObjects, pMesh, ppTitle[i], fX, 55.0f, 0.0f, 5.0f, XMFLOAT4(0.15f, 0.75f, 1.0f, 1.0f));
		fX += (nWidth * 5.0f) + 8.0f;
		if ((i == 1) || (i == 3) || (i == 8)) fX += 14.0f;
	}

	fX = -42.0f;
	m_nTitleNameStart = (int)vObjects.size();
	for (int i = 0; i < 3; i++)
	{
		AddPatternGlyph(vObjects, pMesh, ppName[i], fX, -30.0f, 0.0f, 5.0f, XMFLOAT4(1.0f, 0.85f, 0.12f, 1.0f));
		fX += 32.0f;
	}

	m_nTitleNameObjects = (int)vObjects.size() - m_nTitleNameStart;
	m_nTitleObjects = (int)vObjects.size();
	m_ppTitleObjects = new CGameObject * [m_nTitleObjects];

	for (int i = 0; i < m_nTitleObjects; i++) m_ppTitleObjects[i] = vObjects[i];

	delete[] m_pxmf3TitleObjectVelocity;
	m_pxmf3TitleObjectVelocity = new XMFLOAT3[m_nTitleObjects];

	for (int i = 0; i < m_nTitleObjects; i++) m_pxmf3TitleObjectVelocity[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

// 메뉴 화면
void CScene::BuildMenuObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	CMesh* pButtonMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);
	CMesh* pTextMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 3.0f, 3.0f, 3.0f);
	XMFLOAT4 xmf4White(0.85f, 0.85f, 0.9f, 1.0f);
	XMFLOAT4 xmf4Green(0.1f, 1.0f, 0.2f, 1.0f);

	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "TUTORIAL", -255.0f, 90.0f, 180.0f, 60.0f, xmf4White);
	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "LEVEL-1", -85.0f, 90.0f, 145.0f, 60.0f, xmf4White);
	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "LEVEL-2", 80.0f, 90.0f, 145.0f, 60.0f, xmf4White);
	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "LEVEL-3", 245.0f, 90.0f, 145.0f, 60.0f, xmf4White);
	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "START", 0.0f, -25.0f, 200.0f, 65.0f, xmf4Green);
	AddCubeButton(vObjects, pButtonMesh, pTextMesh, "END", 0.0f, -125.0f, 200.0f, 65.0f, xmf4White);

	m_nMenuObjects = (int)vObjects.size();
	m_ppMenuObjects = new CGameObject * [m_nMenuObjects];
	for (int i = 0; i < m_nMenuObjects; i++) m_ppMenuObjects[i] = vObjects[i];
}

// 게임 오버 화면
void CScene::BuildGameOverObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	CMesh* pTextMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);
	CMesh* pButtonMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);

	AddCubeLabel(vObjects, pTextMesh, "GAME OVER", 0.0f, 95.0f, 0.0f, 9.0f, 10.8f, XMFLOAT4(4.0f, 0.15f, 0.1f, 1.0f));
	AddResultButton(vObjects, pButtonMesh, "MENU", 0.0f, -75.0f, 185.0f, 65.0f);

	m_nGameOverObjects = (int)vObjects.size();
	m_ppGameOverObjects = new CGameObject * [m_nGameOverObjects];
	for (int i = 0; i < m_nGameOverObjects; i++) m_ppGameOverObjects[i] = vObjects[i];
}

// 게임 클리어 화면
void CScene::BuildGameClearObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	CMesh* pTextMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);
	CMesh* pButtonMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 5.0f, 5.0f, 5.0f);

	AddCubeLabel(vObjects, pTextMesh, "GAME CLEAR", 0.0f, 95.0f, 0.0f, 9.0f, 10.8f, XMFLOAT4(0.25f, 0.55f, 4.0f, 1.0f));
	AddResultButton(vObjects, pButtonMesh, "MENU", 0.0f, -75.0f, 185.0f, 65.0f);

	m_nGameClearObjects = (int)vObjects.size();
	m_ppGameClearObjects = new CGameObject * [m_nGameClearObjects];
	for (int i = 0; i < m_nGameClearObjects; i++) m_ppGameClearObjects[i] = vObjects[i];
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	BuildTitleObjects(pd3dDevice, pd3dCommandList);
	BuildMenuObjects(pd3dDevice, pd3dCommandList);
	BuildGameOverObjects(pd3dDevice, pd3dCommandList);
	BuildGameClearObjects(pd3dDevice, pd3dCommandList);

	XMFLOAT3 xmf3TerrainScale(8.0f, 2.0f, 8.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Image/HeightMap.raw"), 257, 257, 257, 257, xmf3TerrainScale);

	m_nGameObjects = 19;
	m_ppGameObjects = new CGameObject * [m_nGameObjects];

	CApacheObject *pApacheObject = new CApacheObject();
	CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Apache.txt");
	pApacheObject->SetChild(pGameObject);
	pApacheObject->OnInitialize();
	pApacheObject->SetScale(1.5f, 1.5f, 1.5f);
	pApacheObject->Rotate(0.0f, 90.0f, 0.0f);
	pApacheObject->SetPosition(620.0f, TerrainY(m_pTerrain, 620.0f, 760.0f, 45.0f), 760.0f);
	m_ppGameObjects[0] = pApacheObject;

	CSuperCobraObject* pSuperCobraObject = new CSuperCobraObject();
	pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/SuperCobra.txt");
	pSuperCobraObject->SetChild(pGameObject);
	pSuperCobraObject->OnInitialize();
	pSuperCobraObject->SetScale(8.0f, 8.0f, 8.0f);
	pSuperCobraObject->Rotate(0.0f, -90.0f, 0.0f);
	pSuperCobraObject->SetPosition(760.0f, TerrainY(m_pTerrain, 760.0f, 900.0f, 45.0f), 900.0f);
	m_ppGameObjects[1] = pSuperCobraObject;

	CHummerObject* pHummerObject = new CHummerObject();
	pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/M26.txt");
	pHummerObject->SetChild(pGameObject);
	pHummerObject->OnInitialize();
	pHummerObject->SetScale(8.0f, 8.0f, 8.0f);
	pHummerObject->Rotate(0.0f, -90.0f, 0.0f);
	pHummerObject->SetPosition(900.0f, TerrainY(m_pTerrain, 900.0f, 720.0f, 14.0f), 720.0f);
	m_ppGameObjects[2] = pHummerObject;

	m_ppGameObjects[3] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house3.bin", 920.0f, 1040.0f, 12.0f, 20.0f, m_pTerrain);
	m_ppGameObjects[4] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house4.bin", 1040.0f, 920.0f, 12.0f, -35.0f, m_pTerrain);
	m_ppGameObjects[5] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house5.bin", 1120.0f, 1120.0f, 12.0f, 55.0f, m_pTerrain);
	m_ppGameObjects[6] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house6.bin", 780.0f, 1120.0f, 12.0f, -10.0f, m_pTerrain);
	const char* ppstrHouseFiles[4] =
	{
		"Models/Meshes/ams_house3.bin",
		"Models/Meshes/ams_house4.bin",
		"Models/Meshes/ams_house5.bin",
		"Models/Meshes/ams_house6.bin"
	};
	for (int i = 7; i < m_nGameObjects; i++)
	{
		m_ppGameObjects[i] = CreateHouseObject(pd3dDevice, pd3dCommandList, ppstrHouseFiles[(i - 3) % 4], 0.0f, 0.0f, 12.0f, RandomRange(-60.0f, 60.0f), m_pTerrain);
		m_ppGameObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		m_bHouseActive[i - 3] = false;
	}
	m_pBomb = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.15f, 0.1f, 1.0f), 8.0f);
	for (int i = 0; i < 10; i++) m_ppCoinObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 3.0f, 0.1f, 1.0f), 1.4f);
	for (int i = 0; i < 10; i++) m_ppUltimateGaugeObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.1f, 0.1f, 1.0f), 2.0f);
	for (int i = 0; i < 10; i++) m_ppUltimateBulletObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.1f, 0.1f, 1.0f), 6.0f);
	for (int i = 0; i < 16; i++)
	{
		m_ppExplosionObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.8f, 0.1f, 1.0f), 4.0f);
		m_pxmf3ExplosionVelocity[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

// 폭탄 발사
void CScene::FireBomb()
{
	if (!m_pPlayer || m_bBombActive || m_bGameOver || m_bGameClear) return;

	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	xmf3Position.y -= 20.0f;
	m_pBomb->SetPosition(xmf3Position);
	m_bBombActive = true;
}

// 건물 배치
void CScene::RespawnHouse(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= 16) || !m_pTerrain) return;

	float fMargin = 180.0f;
	float x = RandomRange(fMargin, m_pTerrain->GetWidth() - fMargin);
	float z = RandomRange(fMargin, m_pTerrain->GetLength() - fMargin);
	float y = TerrainY(m_pTerrain, x, z, 4.0f);

	CGameObject* pHouse = m_ppGameObjects[3 + nIndex];
	pHouse->SetPosition(x, y, z);
	m_bHouseActive[nIndex] = true;
}

// 폭발
void CScene::MakeExplosion(XMFLOAT3 xmf3Position)
{
	for (int i = 0; i < 16; i++)
	{
		float fAngle = XM_2PI * (i / 16.0f);
		float fSpeed = RandomRange(45.0f, 95.0f);
		m_pxmf3ExplosionVelocity[i] = XMFLOAT3(cosf(fAngle) * fSpeed, RandomRange(35.0f, 90.0f), sinf(fAngle) * fSpeed);
		m_ppExplosionObjects[i]->SetPosition(xmf3Position);
		m_pfExplosionTime[i] = 0.7f;
	}
}

// 코인 
void CScene::UpdateCoinObjects(CCamera* pCamera)
{
	if (!pCamera) return;

	XMFLOAT3 xmf3Camera = pCamera->GetPosition();
	XMFLOAT3 xmf3Look = pCamera->GetLookVector();
	XMFLOAT3 xmf3Right = pCamera->GetRightVector();
	XMFLOAT3 xmf3Up = pCamera->GetUpVector();

	for (int i = 0; i < 10; i++)
	{
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3Camera, xmf3Look, 35.0f);
		xmf3Position = Vector3::Add(xmf3Position, xmf3Right, -8.0f + (i * 1.8f));
		xmf3Position = Vector3::Add(xmf3Position, xmf3Up, 6.0f);
		if ((i >= m_nCoins) && !m_bGameOver && !m_bGameClear) xmf3Position.y -= 10000.0f;
		m_ppCoinObjects[i]->SetPosition(xmf3Position);
	}
}

// 궁극기 게이지
void CScene::UpdateUltimateGaugeObjects(CCamera* pCamera)
{
	if (!pCamera) return;

	XMFLOAT3 xmf3Camera = pCamera->GetPosition();
	XMFLOAT3 xmf3Look = pCamera->GetLookVector();
	XMFLOAT3 xmf3Right = pCamera->GetRightVector();
	XMFLOAT3 xmf3Up = pCamera->GetUpVector();

	for (int i = 0; i < 10; i++)
	{
		if (!m_ppUltimateGaugeObjects[i]) continue;
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3Camera, xmf3Look, 36.0f);
		xmf3Position = Vector3::Add(xmf3Position, xmf3Right, -11.25f + (i * 2.5f));
		xmf3Position = Vector3::Add(xmf3Position, xmf3Up, 10.0f);
		if (i >= m_nUltimateGauge) xmf3Position.y -= 10000.0f;
		m_ppUltimateGaugeObjects[i]->SetPosition(xmf3Position);
	}
}

// 궁극기 발사
void CScene::StartUltimateRain()
{
	if (!m_pPlayer || m_bUltimateFiring || m_bGameOver || m_bGameClear) return;

	m_bUltimateFiring = true;
	m_fUltimateFireTimer = 0.0f;
	m_nUltimateNextBullet = 0;

	for (int i = 0; i < 10; i++)
	{
		m_bUltimateBulletActive[i] = false;
		if (m_ppUltimateBulletObjects[i]) m_ppUltimateBulletObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
	}
}

// 건물, 폭탄 충돌 검사
void CScene::HitHouseByBullet(XMFLOAT3 xmf3BulletPosition, bool* pbBulletActive, CGameObject* pBulletObject)
{
	if (!pbBulletActive || !(*pbBulletActive) || !pBulletObject) return;

	for (int i = 0; i < 16; i++)
	{
		if (!m_bHouseActive[i]) continue;
		CGameObject* pHouse = m_ppGameObjects[3 + i];
		XMFLOAT3 xmf3House = pHouse->GetPosition();

		if ((DistanceXZ(xmf3BulletPosition, xmf3House) < 55.0f) && (xmf3BulletPosition.y <= (xmf3House.y + 95.0f)))
		{
			MakeExplosion(xmf3House);
			pHouse->SetPosition(0.0f, -10000.0f, 0.0f);
			m_bHouseActive[i] = false;
			pBulletObject->SetPosition(0.0f, -10000.0f, 0.0f);
			*pbBulletActive = false;

			if (m_nCoins < 10) m_nCoins++;
			if (m_nCoins >= 10)
			{
				m_bGameClear = true;
				m_GameState.m_nScene = GAME_SCENE_GAMECLEAR;
				ResetMenuCamera();
			}
			break;
		}
	}
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_pTerrain) delete m_pTerrain;
	m_pTerrain = NULL;

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) delete m_ppGameObjects[i];
		delete[] m_ppGameObjects;
	}
	ReleaseSceneObjects(m_ppTitleObjects, m_nTitleObjects);
	m_ppTitleObjects = NULL;
	m_nTitleObjects = 0;
	delete[] m_pxmf3TitleObjectVelocity;
	m_pxmf3TitleObjectVelocity = NULL;
	m_nTitleNameStart = 0;
	m_nTitleNameObjects = 0;
	ReleaseSceneObjects(m_ppMenuObjects, m_nMenuObjects);
	m_ppMenuObjects = NULL;
	m_nMenuObjects = 0;
	ReleaseSceneObjects(m_ppGameOverObjects, m_nGameOverObjects);
	m_ppGameOverObjects = NULL;
	m_nGameOverObjects = 0;
	ReleaseSceneObjects(m_ppGameClearObjects, m_nGameClearObjects);
	m_ppGameClearObjects = NULL;
	m_nGameClearObjects = 0;

	ReleaseShaderVariables();
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)& pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::ReleaseShaderVariables()
{
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nTitleObjects; i++) if (m_ppTitleObjects[i]) m_ppTitleObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nMenuObjects; i++) if (m_ppMenuObjects[i]) m_ppMenuObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameOverObjects; i++) if (m_ppGameOverObjects[i]) m_ppGameOverObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameClearObjects; i++) if (m_ppGameClearObjects[i]) m_ppGameClearObjects[i]->ReleaseUploadBuffers();
	if (m_pBomb) m_pBomb->ReleaseUploadBuffers();
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 10; i++) if (m_ppUltimateGaugeObjects[i]) m_ppUltimateGaugeObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 10; i++) if (m_ppUltimateBulletObjects[i]) m_ppUltimateBulletObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 16; i++) if (m_ppExplosionObjects[i]) m_ppExplosionObjects[i]->ReleaseUploadBuffers();
}

void CScene::ResetMenuCamera()
{
	if (!m_pPlayer) return;

	m_pPlayer->Rotate(0.0f, -m_pPlayer->GetYaw(), 0.0f);
	m_pPlayer->SetVelocity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	CCamera* pCamera = m_pPlayer->GetCamera();
	if (pCamera)
	{
		XMFLOAT3 xmf3CameraPosition(0.0f, 0.0f, -430.0f);
		XMFLOAT3 xmf3LookAt(0.0f, 0.0f, 0.0f);
		XMFLOAT3 xmf3Up(0.0f, 1.0f, 0.0f);
		pCamera->SetTimeLag(0.0f);
		pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, -430.0f));
		pCamera->SetPosition(xmf3CameraPosition);
		pCamera->GenerateViewMatrix(xmf3CameraPosition, xmf3LookAt, xmf3Up);
	}
}

// 게임 상태 초기화
void CScene::ResetLevelState()
{
	m_nCoins = 0;
	m_bGameClear = false;
	m_bGameOver = false;
	m_bBombActive = false;
	m_bFireKeyDown = false;
	m_fHouseRespawnTimer = 0.0f;
	m_fGameEndBlink = 0.0f;

	if (m_pBomb) m_pBomb->SetPosition(0.0f, -10000.0f, 0.0f);
	for (int i = 0; i < 10; i++)
	{
		if (m_ppCoinObjects[i])
		{
			m_ppCoinObjects[i]->SetColor(XMFLOAT4(4.0f, 3.0f, 0.1f, 1.0f));
			m_ppCoinObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		}
	}
	for (int i = 0; i < 10; i++)
	{
		if (m_ppUltimateGaugeObjects[i]) m_ppUltimateGaugeObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		if (m_ppUltimateBulletObjects[i]) m_ppUltimateBulletObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		m_bUltimateBulletActive[i] = false;
	}

	m_fUltimateGaugeTimer = 0.0f;
	m_fUltimateFireTimer = 0.0f;
	m_nUltimateGauge = 0;
	m_nUltimateNextBullet = 0;
	m_bUltimateFiring = false;

	for (int i = 0; i < 16; i++)
	{
		m_pfExplosionTime[i] = 0.0f;
		m_pxmf3ExplosionVelocity[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
		if (m_ppExplosionObjects[i]) m_ppExplosionObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
	}
}

// 마우스 클릭 좌표가 시작 화면의 이름 영역 안에 있는지 검사
bool CScene::IsTitleNameClicked(HWND hWnd, LPARAM lParam)
{
	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);

	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;
	if ((nWidth <= 0) || (nHeight <= 0)) return(false);

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	int nLeft = int(nWidth * 0.40f);
	int nRight = int(nWidth * 0.60f);
	int nTop = int(nHeight * 0.56f);
	int nBottom = int(nHeight * 0.66f);

	return((x >= nLeft) && (x <= nRight) && (y >= nTop) && (y <= nBottom));
}

// 마우스 클릭 좌표가 Start 버튼 영역 안에 있는지 검사
bool CScene::IsMenuStartClicked(HWND hWnd, LPARAM lParam)
{
	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;
	if ((nWidth <= 0) || (nHeight <= 0)) return(false);

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	int nLeft = int(nWidth * 0.34f);
	int nRight = int(nWidth * 0.66f);
	int nTop = int(nHeight * 0.49f);
	int nBottom = int(nHeight * 0.64f);

	return((x >= nLeft) && (x <= nRight) && (y >= nTop) && (y <= nBottom));
}

// 마우스 클릭 좌표가 결과 화면의 Menu 버튼 영역 안에 있는지 검사
bool CScene::IsGameOverMenuClicked(HWND hWnd, LPARAM lParam)
{
	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);
	int nWidth = rcClient.right - rcClient.left;
	int nHeight = rcClient.bottom - rcClient.top;
	if ((nWidth <= 0) || (nHeight <= 0)) return(false);

	int x = LOWORD(lParam);
	int y = HIWORD(lParam);
	int nLeft = int(nWidth * 0.36f);
	int nRight = int(nWidth * 0.64f);
	int nTop = int(nHeight * 0.55f);
	int nBottom = int(nHeight * 0.72f);

	return((x >= nLeft) && (x <= nRight) && (y >= nTop) && (y <= nBottom));
}

// 시작 화면 폭발
void CScene::StartTitleNameExplosion()
{
	if (m_bTitleNameExploding || !m_pxmf3TitleObjectVelocity) return;

	m_bTitleNameExploding = true;
	m_fTitleExplosionTimer = 0.0f;

	XMFLOAT3 xmf3Center(0.0f, -45.0f, 0.0f);

	for (int i = 0; i < m_nTitleNameObjects; i++)
	{
		int nObject = m_nTitleNameStart + i;
		if ((nObject < 0) || (nObject >= m_nTitleObjects) || !m_ppTitleObjects[nObject]) continue;

		XMFLOAT3 xmf3Position = m_ppTitleObjects[nObject]->GetPosition();
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Position, xmf3Center);
		float fLength = Vector3::Length(xmf3Direction);

		if (fLength < 0.01f) xmf3Direction = XMFLOAT3(RandomRange(-1.0f, 1.0f), RandomRange(-0.2f, 1.0f), 0.0f);
		else xmf3Direction = Vector3::ScalarProduct(xmf3Direction, 1.0f / fLength, false);

		m_pxmf3TitleObjectVelocity[nObject] = XMFLOAT3(
			xmf3Direction.x * RandomRange(90.0f, 180.0f),
			RandomRange(45.0f, 140.0f),
			RandomRange(-90.0f, 90.0f));
	}
}

// 현재 게임 상태에 따라 마우스 클릭으로 이름, START, MENU 버튼 입력을 처리
bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (nMessageID == WM_LBUTTONDOWN)
	{
		m_GameState.m_bMouseDown = true;

		if (m_GameState.m_nScene == GAME_SCENE_TITLE)
		{
			if (IsTitleNameClicked(hWnd, lParam)) StartTitleNameExplosion();
			return(true);
		}

		if ((m_GameState.m_nScene == GAME_SCENE_GAMEOVER) || (m_GameState.m_nScene == GAME_SCENE_GAMECLEAR))
		{
			if (IsGameOverMenuClicked(hWnd, lParam))
			{
				ResetMenuCamera();
				m_GameState.m_nScene = GAME_SCENE_MENU;
			}
			return(true);
		}

		if (m_GameState.m_nScene == GAME_SCENE_MENU)
		{
			if (!IsMenuStartClicked(hWnd, lParam)) return(true);
			ResetLevelState();

			m_GameState.m_nScene = GAME_SCENE_LEVEL1;

			if (m_pPlayer && m_pTerrain)
			{
				float fPlayerX = m_pTerrain->GetWidth() * 0.5f;
				float fPlayerZ = m_pTerrain->GetLength() * 0.5f;
				XMFLOAT3 xmf3PlayerPosition(fPlayerX, m_pTerrain->GetHeight(fPlayerX, fPlayerZ) + 70.0f, fPlayerZ);

				m_pPlayer->SetPosition(xmf3PlayerPosition);

				CCamera* pCamera = m_pPlayer->GetCamera();
				if (pCamera)
				{
					pCamera->SetOffset(XMFLOAT3(0.0f, 150.0f, -170.0f));
					pCamera->SetTimeLag(0.25f);
					pCamera->SetPosition(Vector3::Add(xmf3PlayerPosition, pCamera->GetOffset()));
				}
				m_pPlayer->Update(0.0f);
			}
			return(true);
		}
	}
	if (nMessageID == WM_LBUTTONUP) m_GameState.m_bMouseDown = false;
	return(false);
}

// Level1에서 ESC키 입력 시 메뉴 화면으로 전환
bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if ((nMessageID == WM_KEYUP) && (wParam == VK_ESCAPE) && (m_GameState.m_nScene == GAME_SCENE_LEVEL1))
	{
		m_GameState.m_nScene = GAME_SCENE_MENU;
		m_bFireKeyDown = false;

		ResetMenuCamera();

		return(true);
	}
	return(false);
}

// Level1에서 Space 키 입력 시 폭탄 발사 처리
bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	if (m_GameState.m_nScene != GAME_SCENE_LEVEL1) return(true);

	bool bFireKeyDown = ((pKeysBuffer[VK_SPACE] & 0xF0) != 0);
	if (bFireKeyDown && !m_bFireKeyDown) FireBomb();
	m_bFireKeyDown = bFireKeyDown;
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	// 시작 화면
	if (m_GameState.m_nScene == GAME_SCENE_TITLE)
	{
		if (m_bTitleNameExploding)
		{
			// 이름 폭발 효과
			m_fTitleExplosionTimer += fTimeElapsed;
			for (int i = 0; i < m_nTitleNameObjects; i++)
			{
				int nObject = m_nTitleNameStart + i;
				if ((nObject < 0) || (nObject >= m_nTitleObjects) || !m_ppTitleObjects[nObject]) continue;

				XMFLOAT3 xmf3Position = m_ppTitleObjects[nObject]->GetPosition();
				xmf3Position = Vector3::Add(xmf3Position, m_pxmf3TitleObjectVelocity[nObject], fTimeElapsed);
				m_pxmf3TitleObjectVelocity[nObject].y -= 220.0f * fTimeElapsed;
				m_ppTitleObjects[nObject]->SetPosition(xmf3Position);
			}

			if (m_fTitleExplosionTimer > 0.85f)
			{
				m_GameState.m_nScene = GAME_SCENE_MENU;
				m_bTitleNameExploding = false;
			}
		}

		// 시작 화면 큐브들은 계속 회전
		for (int i = 0; i < m_nTitleObjects; i++)
		{
			if (!m_ppTitleObjects[i]) continue;
			m_ppTitleObjects[i]->Rotate(0.0f, 20.0f * fTimeElapsed, 0.0f);
			m_ppTitleObjects[i]->UpdateTransform(NULL);
		}
		return;
	}

	if (m_GameState.m_nScene == GAME_SCENE_MENU)
	{
		for (int i = 0; i < m_nMenuObjects; i++) m_ppMenuObjects[i]->UpdateTransform(NULL);
		return;
	}

	if (m_GameState.m_nScene == GAME_SCENE_GAMEOVER)
	{
		for (int i = 0; i < m_nGameOverObjects; i++) m_ppGameOverObjects[i]->UpdateTransform(NULL);
		return;
	}

	if (m_GameState.m_nScene == GAME_SCENE_GAMECLEAR)
	{
		for (int i = 0; i < m_nGameClearObjects; i++) m_ppGameClearObjects[i]->UpdateTransform(NULL);
		return;
	}

	// Level1의 게임 오브젝트 갱신
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);

	if (!m_bGameOver && !m_bGameClear && m_pPlayer)
	{
		XMFLOAT3 xmf3Player = m_pPlayer->GetPosition();

		// 적 헬리콥터는 매 프레임 플레이어 위치를 향해 이동
		for (int i = 0; i < 2; i++)
		{
			XMFLOAT3 xmf3Enemy = m_ppGameObjects[i]->GetPosition();
			XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Player, xmf3Enemy);
			float fDistance = Vector3::Length(xmf3Direction);
			if (fDistance > 1.0f)
			{
				TurnObjectToTarget(m_ppGameObjects[i], xmf3Player);
				float fMove = min(42.0f * fTimeElapsed, fDistance);
				xmf3Enemy = Vector3::Add(xmf3Enemy, xmf3Direction, fMove / fDistance);
				m_ppGameObjects[i]->SetPosition(xmf3Enemy);
			}
			if (fDistance < 65.0f)
			{
				m_bGameOver = true;
				m_GameState.m_nScene = GAME_SCENE_GAMEOVER;
				ResetMenuCamera();
			}
		}

		m_fHouseRespawnTimer += fTimeElapsed;

		// 5초마다 파괴돼 비활성화된 건물 하나를 다시 지형 위에 배치
		if (m_fHouseRespawnTimer >= 5.0f)
		{
			m_fHouseRespawnTimer = 0.0f;
			for (int i = 4; i < 16; i++)
			{
				if (!m_bHouseActive[i])
				{
					RespawnHouse(i);
					break;
				}
			}
		}
	}

	// 궁극기 게이지는 궁극기 발사중에는 충전을 멈춤
	if (!m_bGameOver && !m_bGameClear)
	{
		if (!m_bUltimateFiring)
		{
			m_fUltimateGaugeTimer += fTimeElapsed;
			if (m_fUltimateGaugeTimer >= 1.0f)
			{
				m_fUltimateGaugeTimer -= 1.0f;
				if (m_nUltimateGauge < 10) m_nUltimateGauge++;
				if (m_nUltimateGauge >= 10) StartUltimateRain();
			}
		}

		// 궁극기 발사 
		if (m_bUltimateFiring && m_pPlayer)
		{
			m_fUltimateFireTimer += fTimeElapsed;

			while ((m_nUltimateNextBullet < 10) && (m_fUltimateFireTimer >= (m_nUltimateNextBullet * 0.08f)))
			{
				int nBullet = m_nUltimateNextBullet++;
				XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();

				xmf3Position.x += ((nBullet % 5) - 2) * 22.0f;
				xmf3Position.z += ((nBullet / 5) == 0) ? -16.0f : 16.0f;
				xmf3Position.y -= 20.0f;

				m_ppUltimateBulletObjects[nBullet]->SetPosition(xmf3Position);
				m_bUltimateBulletActive[nBullet] = true;
			}

			bool bAnyBulletAlive = false;

			// 활성화된 궁극기 탄환은 아래로 낙하시킨 뒤 지형/건물 충돌 검사
			for (int i = 0; i < 10; i++)
			{
				if (!m_bUltimateBulletActive[i] || !m_ppUltimateBulletObjects[i]) continue;

				bAnyBulletAlive = true;
				XMFLOAT3 xmf3Bullet = m_ppUltimateBulletObjects[i]->GetPosition();
				xmf3Bullet.y -= 430.0f * fTimeElapsed;

				m_ppUltimateBulletObjects[i]->SetPosition(xmf3Bullet);

				float fGround = (m_pTerrain) ? m_pTerrain->GetHeight(xmf3Bullet.x, xmf3Bullet.z) : 0.0f;

				if (xmf3Bullet.y <= (fGround + 2.0f))
				{
					m_ppUltimateBulletObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
					m_bUltimateBulletActive[i] = false;
					continue;
				}
				HitHouseByBullet(xmf3Bullet, &m_bUltimateBulletActive[i], m_ppUltimateBulletObjects[i]);
			}

			if ((m_nUltimateNextBullet >= 10) && !bAnyBulletAlive)
			{
				m_bUltimateFiring = false;
				m_fUltimateGaugeTimer = 0.0f;
				m_fUltimateFireTimer = 0.0f;
				m_nUltimateGauge = 0;
				m_nUltimateNextBullet = 0;
			}
		}
	}
	// 폭탄은 한 번에 하나만 활성화
	if (m_bBombActive && m_pBomb)
	{
		XMFLOAT3 xmf3Bomb = m_pBomb->GetPosition();
		xmf3Bomb.y -= 330.0f * fTimeElapsed;
		m_pBomb->SetPosition(xmf3Bomb);

		float fGround = (m_pTerrain) ? m_pTerrain->GetHeight(xmf3Bomb.x, xmf3Bomb.z) : 0.0f;
		if (xmf3Bomb.y <= (fGround + 2.0f))
		{
			m_pBomb->SetPosition(0.0f, -10000.0f, 0.0f);
			m_bBombActive = false;
		}

		// 건물들과 폭탄 충돌 검사
		for (int i = 0; i < 16; i++)
		{
			if (!m_bHouseActive[i]) continue;
			CGameObject* pHouse = m_ppGameObjects[3 + i];
			XMFLOAT3 xmf3House = pHouse->GetPosition();
			if ((DistanceXZ(xmf3Bomb, xmf3House) < 55.0f) && (xmf3Bomb.y <= (xmf3House.y + 95.0f)))
			{
				MakeExplosion(xmf3House);
				pHouse->SetPosition(0.0f, -10000.0f, 0.0f);
				m_bHouseActive[i] = false;
				m_pBomb->SetPosition(0.0f, -10000.0f, 0.0f);
				m_bBombActive = false;
				if (m_nCoins < 10) m_nCoins++;
				if (m_nCoins >= 10)
				{
					 m_bGameClear = true;
					 m_GameState.m_nScene = GAME_SCENE_GAMECLEAR;
					 ResetMenuCamera();
				}
				break;
			}
		}
	}

	// 폭발 파편은 시간이 끝나면 화면 밖으로 보냄
	for (int i = 0; i < 16; i++)
	{
		if (m_pfExplosionTime[i] > 0.0f)
		{
			m_pfExplosionTime[i] -= fTimeElapsed;
			XMFLOAT3 xmf3Position = m_ppExplosionObjects[i]->GetPosition();
			xmf3Position = Vector3::Add(xmf3Position, m_pxmf3ExplosionVelocity[i], fTimeElapsed);
			m_pxmf3ExplosionVelocity[i].y -= 160.0f * fTimeElapsed;
			m_ppExplosionObjects[i]->SetPosition(xmf3Position);
		}
		else
		{
			m_ppExplosionObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		}
	}

	// 게임이 끝난 순간 남은 코인 큐브 색을 결과 상태에 맞게 바꿈
	if (m_bGameClear || m_bGameOver)
	{
		m_fGameEndBlink += fTimeElapsed;
		XMFLOAT4 xmf4Color = (m_bGameClear) ? XMFLOAT4(4.0f, 3.0f, 0.1f, 1.0f) : XMFLOAT4(4.0f, 0.0f, 0.0f, 1.0f);
		for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->SetColor(xmf4Color);
	}

	// 월드 변환 행렬 갱신
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->UpdateTransform(NULL);

	if (m_pBomb) m_pBomb->UpdateTransform(NULL);

	for (int i = 0; i < 10; i++) if (m_ppUltimateBulletObjects[i]) m_ppUltimateBulletObjects[i]->UpdateTransform(NULL);
	for (int i = 0; i < 10; i++) if (m_ppUltimateGaugeObjects[i]) m_ppUltimateGaugeObjects[i]->UpdateTransform(NULL);
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->UpdateTransform(NULL);
	for (int i = 0; i < 16; i++) if (m_ppExplosionObjects[i]) m_ppExplosionObjects[i]->UpdateTransform(NULL);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	if (m_GameState.m_nScene == GAME_SCENE_TITLE)
	{
		RenderSceneObjects(pd3dCommandList, pCamera, m_ppTitleObjects, m_nTitleObjects);
		return;
	}

	if (m_GameState.m_nScene == GAME_SCENE_MENU)
	{
		RenderSceneObjects(pd3dCommandList, pCamera, m_ppMenuObjects, m_nMenuObjects);
		return;
	}
	if (m_GameState.m_nScene == GAME_SCENE_GAMEOVER)
	{
		RenderSceneObjects(pd3dCommandList, pCamera, m_ppGameOverObjects, m_nGameOverObjects);
		return;
	}
	if (m_GameState.m_nScene == GAME_SCENE_GAMECLEAR)
	{
		RenderSceneObjects(pd3dCommandList, pCamera, m_ppGameClearObjects, m_nGameClearObjects);
		return;
	}


	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++)
	{
		if ((i >= 3) && !m_bHouseActive[i - 3]) continue;
		m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_bBombActive && m_pBomb) m_pBomb->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < 10; i++) if (m_bUltimateBulletActive[i] && m_ppUltimateBulletObjects[i]) m_ppUltimateBulletObjects[i]->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < 16; i++) if (m_pfExplosionTime[i] > 0.0f) m_ppExplosionObjects[i]->Render(pd3dCommandList, pCamera);
	UpdateUltimateGaugeObjects(pCamera);
	for (int i = 0; i < 10; i++) if (m_ppUltimateGaugeObjects[i]) m_ppUltimateGaugeObjects[i]->Render(pd3dCommandList, pCamera);
	UpdateCoinObjects(pCamera);
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->Render(pd3dCommandList, pCamera);
}
