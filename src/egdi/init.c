#include "egdi/egdi.h"
#include "error.h"

struct __GameObjectVTBL GameObject;
struct __TextureVTBL Texture;
struct __TextVTBL Text;

/*
 * GameObject Implementation
 */

void GameObject_AddChild(HGameObject this, HGameObject hgo)
{
	if (++this->Vtbl.m_nChild > this->Vtbl.m_cChild)
	{
		this->Vtbl.m_cChild <<= 1;
		HGameObject* arr = realloc(this->Vtbl.m_arrChild, this->Vtbl.m_cChild * sizeof(HGameObject));
		if (!arr)
		{
			DialogHRESULT(E_UNEXPECTED);
			return;
		}
		this->Vtbl.m_arrChild = arr;
	}

	this->Vtbl.m_arrChild[this->Vtbl.m_nChild - 1] = hgo;
}

void GameObject_DelChild(HGameObject this, HGameObject hgo)
{
	if (!this->Vtbl.m_nChild)
		return;

	DWORD i;
	for (i = 0; i < this->Vtbl.m_nChild; ++i)
		if (this->Vtbl.m_arrChild[i] == hgo)
			break;
	if (i == this->Vtbl.m_nChild)
		return;

	memmove(this->Vtbl.m_arrChild + i,
			this->Vtbl.m_arrChild + i + 1,
			sizeof(HGameObject) * (this->Vtbl.m_nChild - i - 1));

	this->Vtbl.m_nChild--;
}

Vec2 GameObject_GetPosition(HGameObject this)
{
	return this->Vtbl.m_position;
}

void GameObject_SetPosition(HGameObject this, Vec2 pos)
{
	this->Vtbl.m_position = pos;
}

Vec2 GameObject_GetScale(HGameObject this)
{
	return this->Vtbl.m_scale;
}

void GameObject_SetScale(HGameObject this, Vec2 scale)
{
	this->Vtbl.m_scale = scale;
}

BOOL GameObject_Render(HGameObject this, HRenderContext ctx)
{
	/* Prologue */
	Vec2 offset = ctx->m_offset;
	ctx->m_offset.X += this->Vtbl.m_position.X * ctx->m_scale.X;
	ctx->m_offset.Y += this->Vtbl.m_position.Y * ctx->m_scale.Y;

	Vec2 scale = ctx->m_scale;
	ctx->m_scale.X *= this->Vtbl.m_scale.X;
	ctx->m_scale.Y *= this->Vtbl.m_scale.Y;

	/* Render */
	BOOL ret = TRUE;
	for (DWORD i = 0; i < this->Vtbl.m_nChild; ++i)
	{
		if (this->Vtbl.m_arrChild[i]->Vtbl.Render(this->Vtbl.m_arrChild[i], ctx))
			continue;
		ret = FALSE;
		break;
	}

	/* Epilogue */
	ctx->m_offset = offset;
	ctx->m_scale = scale;
	return ret;
}

void GameObject_Release(HGameObject this)
{
	if (this->Vtbl.m_parent)
		this->Vtbl.m_parent->Vtbl.DelChild(this->Vtbl.m_parent, this);
	for (DWORD i = 0; i < this->Vtbl.m_nChild; ++i)
		this->Vtbl.m_arrChild[i]->Vtbl.Release(this->Vtbl.m_arrChild[i]);
	free(this->Vtbl.m_arrChild);
	free(this);
}

HGameObject GameObject_Load(HGameObject parent, void* arg);

void GameObject_Ctor(HGameObject this, struct __GameObjectVTBL* go, HGameObject parent)
{
	if (parent)
		parent->Vtbl.AddChild(parent, this);

	go->Signature = CLS(GameObject);

	go->m_parent = parent;
	go->m_arrChild = malloc(sizeof(HGameObject));
	go->m_nChild = 0;
	go->m_cChild = 1;

	go->m_position.X = 0;
	go->m_position.Y = 0;
	go->m_scale.X = 1;
	go->m_scale.Y = 1;

	go->Load = GameObject_Load;
	go->Release = GameObject_Release;
	go->AddChild = GameObject_AddChild;
	go->DelChild = GameObject_DelChild;
	go->GetPosition = GameObject_GetPosition;
	go->SetPosition = GameObject_SetPosition;
	go->GetScale = GameObject_GetScale;
	go->SetScale = GameObject_SetScale;
	go->Render = GameObject_Render;
}

HGameObject GameObject_Load(HGameObject parent, void* arg)
{
	HGameObject go = malloc(sizeof *go);
	memset(go, 0, sizeof *go);

	GameObject_Ctor(go, &go->Vtbl, parent);

	return go;
}

/*
 * Texture Implementation
 */

BOOL Texture_Render(HTexture this, HRenderContext ctx)
{
	BITMAP bm;
	BOOL ret = TRUE;
	HBITMAP old = SelectObject(ctx->m_mem, this->m_hbm);

	GetObjectW(this->m_hbm, sizeof bm, &bm);
	if (!StretchBlt(
			ctx->m_hdc,
			(int)(ctx->m_offset.X + 0.5),
			(int)(ctx->m_offset.Y + 0.5),
			(int)(ctx->m_scale.X * bm.bmWidth + 0.5),
			(int)(ctx->m_scale.Y * bm.bmHeight + 0.5),
			ctx->m_mem,
			0, 0,
			bm.bmWidth,
			bm.bmHeight,
			SRCCOPY))
	{
		DialogWin32Code(GetLastError());
		ret = FALSE;
	}

	SelectObject(ctx->m_mem, old);
	return ret;
}

LONG_COORD Texture_GetBitmapSize(HTexture this)
{
	BITMAP bm;
	LONG_COORD lc;
	GetObjectW(this->m_hbm, sizeof bm, &bm);
	lc.X = bm.bmWidth;
	lc.Y = bm.bmHeight;
	return lc;
}

void Texture_Release(HTexture this)
{
	DeleteObject(this->m_hbm);
	GameObject_Release((HGameObject)this);
}

HTexture Texture_Load(HGameObject parent, void* arg)
{
	HTexture ht = malloc(sizeof *ht);
	memset(ht, 0, sizeof *ht);

	GameObject_Ctor((HGameObject)ht, (struct __GameObjectVTBL*)&ht->Vtbl, parent);

	ht->Vtbl.Release = Texture_Release;
	ht->Vtbl.Render = Texture_Render;
	ht->Vtbl.GetBitmapSize = Texture_GetBitmapSize;

	ht->m_hbm = LoadImageW(
			GetModuleHandleW(NULL),
			((TEXTURE*)arg)->filepath,
			IMAGE_BITMAP,
			0, 0,
			LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!ht->m_hbm)
		DialogWin32Code(GetLastError());

	return ht;
}

/*
 * Text Implementation
 */

BOOL Text_Render(HText this, HRenderContext ctx)
{

	DialogWin32Code(E_UNEXPECTED);

	BOOL ret = TextOutW(
			ctx->m_hdc,
			(int)(ctx->m_offset.X + 0.5),
			(int)(ctx->m_offset.Y + 0.5),
			this->m_lpszText,
			(int)wcsnlen(this->m_lpszText, INT32_MAX));

	return ret;
}

LPCWSTR Text_GetValue(HText this)
{
	return this->m_lpszText;
}

void Text_SetValue(HText this, LPCWSTR str)
{
	this->m_lpszText = str;
}

HFONT Text_GetFont(HText this)
{
	return this->m_hf;
}

void Text_SetFont(HText this, HFONT hf)
{
	this->m_hf = hf;
}

HText Text_Load(HGameObject parent, void* arg)
{
	HText ht = malloc(sizeof *ht);
	memset(ht, 0, sizeof *ht);

	GameObject_Ctor((HGameObject)ht, (struct __GameObjectVTBL*)&ht->Vtbl, parent);

	ht->Vtbl.Render = Text_Render;
	ht->Vtbl.GetValue = Text_GetValue;
	ht->Vtbl.SetValue = Text_SetValue;
	ht->Vtbl.GetFont = Text_GetFont;
	ht->Vtbl.SetFont = Text_SetFont;

	return ht;
}

/*
 * System Constructor
 */

HRESULT EGDI_Initialize()
{
	/* Class Initialization */
	GameObject.Load = GameObject_Load;
	Texture.Load = Texture_Load;
	Text.Load = Text_Load;

	/* Global Initialization */
	HWND hwnd = GetConsoleWindow();
	if (!hwnd) goto E_WIN32;

	HDC hdc, mem;
	if (!(hdc = GetDC(hwnd)))
		goto E_WIN32;
	if (!(mem = CreateCompatibleDC(hdc)))
	{
		ReleaseDC(hwnd, hdc);
		goto E_WIN32;
	}

	HGameObject go;
	if (!(go = GameObject.Load(NULL, NULL)))
	{
		DeleteDC(mem);
		ReleaseDC(hwnd, hdc);
		goto E_WIN32;
	}

	RenderContext = malloc(sizeof *RenderContext);

	RenderContext->m_scale.X = 1;
	RenderContext->m_scale.Y = 1;
	RenderContext->m_offset.X = 0;
	RenderContext->m_offset.Y = 0;

	RenderContext->m_root = go;
	RenderContext->m_hwnd = hwnd;
	RenderContext->m_hdc = hdc;
	RenderContext->m_mem = mem;

	return S_OK;

E_WIN32:
	return HRESULT_FROM_WIN32(GetLastError());
}
