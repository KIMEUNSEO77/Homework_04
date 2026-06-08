//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

enum GAME_SCENE_ID
{
	GAME_SCENE_TITLE = 0,
	GAME_SCENE_MENU = 1,
	GAME_SCENE_LEVEL1 = 2,
	GAME_SCENE_LEVEL2 = 3,
	GAME_SCENE_LEVEL3 = 4,
	GAME_SCENE_GAMEOVER = 5,
	GAME_SCENE_GAMECLEAR = 6
};

struct GAME_STATE
{
	int m_nScene = GAME_SCENE_TITLE;
	bool m_bMouseDown = false;
};
class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	CHeightMapTerrain* GetTerrain() { return(m_pTerrain); }
	bool IsLevelScene() { return((m_GameState.m_nScene == GAME_SCENE_LEVEL1) || (m_GameState.m_nScene == GAME_SCENE_LEVEL2) || (m_GameState.m_nScene == GAME_SCENE_LEVEL3)); }
	bool IsLevel2Scene() { return(m_GameState.m_nScene == GAME_SCENE_LEVEL2); }
	bool IsLevel2ObstacleCollision(XMFLOAT3 xmf3Position);

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void ReleaseUploadBuffers();

	void FireBomb();
	void FirePlayerShell();
	void FirePlayerShellAtSelectedEnemy();
	void UpdatePlayerShell(float fTimeElapsed);
	void UpdateLevel2Explosions(float fTimeElapsed);
	float RotatePlayerTowardSelectedEnemy(float fTimeElapsed);
	void UpdateAutoAttack(float fTimeElapsed);
	float RotateEnemyTankTowardPlayer(int nTank, float fTimeElapsed);
	void FireEnemyTankShell(int nTank);
	void UpdateEnemyTankAttacks(float fTimeElapsed);
	void UpdateEnemyTankShells(float fTimeElapsed);
	void SelectEnemyTankFromMouse(HWND hWnd, LPARAM lParam);
	void RespawnHouse(int nIndex);
	void MakeExplosion(XMFLOAT3 xmf3Position);
	void UpdateCoinObjects(CCamera* pCamera);
	void UpdateUltimateGaugeObjects(CCamera* pCamera);
	void UpdateHealthObjects(CCamera* pCamera);
	void UpdatePlayerShield(float fTimeElapsed);
	void StartUltimateRain();
	void HitHouseByBullet(XMFLOAT3 xmf3BulletPosition, bool* pbBulletActive, CGameObject* pBulletObject);
	CGameObject* CreateColorCube(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 xmf4Color, float fSize);
	void BuildTitleObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildMenuObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildGameOverObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildGameClearObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildLevel2Objects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildLevel2EnemyTanks(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseSceneObjects(CGameObject** ppObjects, int nObjects);
	void RenderSceneObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** ppObjects, int nObjects);
	bool IsTitleNameClicked(HWND hWnd, LPARAM lParam);
	bool IsMenuStartClicked(HWND hWnd, LPARAM lParam);
	int IsMenuLevelClicked(HWND hWnd, LPARAM lParam);
	bool IsGameOverMenuClicked(HWND hWnd, LPARAM lParam);
	void ResetMenuCamera();
	void ResetLevelState();
	void UpdateMenuLevelSelection();
	void BeginLevel1();
	void BeginLevel2();
	void BeginLevel3();
	void StartTitleNameExplosion();

	CPlayer*					m_pPlayer = NULL;
	CHeightMapTerrain*			m_pTerrain = NULL;

public:
	ID3D12RootSignature*		m_pd3dGraphicsRootSignature = NULL;
	GAME_STATE					 m_GameState;

	CGameObject**				m_ppGameObjects = NULL;
	int							m_nGameObjects = 0;
	CGameObject**				m_ppLevel2Objects = NULL;
	int							m_nLevel2Objects = 0;
	CGameObject*				m_ppEnemyTankObjects[10] = { NULL };
	CGameObject*				m_ppEnemyTankLodObjects[10] = { NULL };
		CGameObject*				 m_ppEnemyTankShellObjects[10] = { NULL };
	bool							 m_bEnemyTankShellActive[10] = { false };
	XMFLOAT3					 m_xmf3EnemyTankShellVelocity[10];
	float						 m_fEnemyTankShellLifeTime[10] = { 0.0f };
	float						 m_fEnemyTankFireCooldown[10] = { 0.0f };
bool						m_bEnemyTankActive[10] = { false };
		int									 m_nSelectedEnemyTank = -1;
	bool								 m_bAutoAttack = false;
	float							 m_fAutoAttackTimer = 0.0f;
	CGameObject*					 m_pSelectedEnemyMarker = NULL;
CGameObject**				 m_ppTitleObjects = NULL;
	int								 m_nTitleObjects = 0;
	int							 m_nTitleNameStart = 0;
	int							 m_nTitleNameObjects = 0;
	bool							 m_bTitleNameExploding = false;
	float							 m_fTitleExplosionTimer = 0.0f;
	XMFLOAT3*				 m_pxmf3TitleObjectVelocity = NULL;
	CGameObject**				 m_ppMenuObjects = NULL;
	int								 m_nMenuObjects = 0;
	int							 m_nSelectedMenuLevel = 1;
	int							 m_nMenuLevelButtonStart[3] = { 0, 0, 0 };
	int							 m_nMenuLevelButtonCount[3] = { 0, 0, 0 };
	CGameObject**				 m_ppGameOverObjects = NULL;
	int								 m_nGameOverObjects = 0;
	CGameObject**				 m_ppGameClearObjects = NULL;
	int							 m_nGameClearObjects = 0;
	bool								m_bHouseActive[16] = { true, true, true, true };
	bool								m_bBombActive = false;
	bool								m_bFireKeyDown = false;
	bool								m_bGameClear = false;
	bool								m_bGameOver = false;
	int									m_nCoins = 0;
	float								m_fHouseRespawnTimer = 0.0f;
	float								m_fGameEndBlink = 0.0f;

	CGameObject*					m_pBomb = NULL;
	CGameObject*					m_ppCoinObjects[10] = { NULL };
	CGameObject*					m_ppUltimateGaugeObjects[10] = { NULL };
	CGameObject*					m_ppHealthObjects[10] = { NULL };
	CGameObject*					m_ppShieldObjects[12] = { NULL };
	CGameObject*					m_ppUltimateBulletObjects[10] = { NULL };
	bool							 m_bUltimateBulletActive[10] = { false };
	float						 m_fUltimateGaugeTimer = 0.0f;
	float						 m_fUltimateFireTimer = 0.0f;
	int							 m_nUltimateGauge = 0;
	int							 m_nPlayerHealth = 10;
	bool							 m_bPlayerShieldActive = false;
	float						 m_fPlayerShieldTime = 0.0f;
	int							 m_nUltimateNextBullet = 0;
	bool						 m_bUltimateFiring = false;
	CGameObject*					 m_pPlayerShell = NULL;
	bool								 m_bPlayerShellActive = false;
	bool								 m_bPlayerShellKeyDown = false;
	XMFLOAT3						 m_xmf3PlayerShellVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float							 m_fPlayerShellLifeTime = 0.0f;
	CGameObject*					m_ppExplosionObjects[16] = { NULL };
	XMFLOAT3						m_pxmf3ExplosionVelocity[16];
	float								m_pfExplosionTime[16] = { 0.0f };
};
