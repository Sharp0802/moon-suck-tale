#ifndef SPROG_MAN_H
#define SPROG_MAN_H

#include "pch.h"

/*
 * Macros
 *
 * - Use only when field is reserved.
 *
 */

#define _STRCAT(a, b) a##b
#define STRCAT(a, b) _STRCAT(a, b)
#define RESERVED(n) char STRCAT(__reserved_, __COUNTER__) [ (n) ]

/*
 * TYPEDEFS
 *
 * - GameObject : Root element of all game-objects
 * - Texture    : Image management element
 * - Text       : Text-rendering management element
 *
 */

typedef struct
{
	LONG X;
	LONG Y;
} LONG_COORD, *PLONG_COORD;

typedef struct __GameObject* HGameObject;
typedef struct __Texture* HTexture;
typedef struct __Text* HText;

#define CLS_F_SEALED (1<<31)

#define CLS(cls) CLS_##cls
#define CLS_GameObject 0x00
#define CLS_Texture    (0x10 | CLS_F_SEALED)
#define CLS_Text       (0x11 | CLS_F_SEALED)

#define CLS_IsSubOf(a, b) ((a&b)==b)

typedef struct
{
	double X;
	double Y;
} Vec2;

typedef struct __RenderContext
{
	HWND m_hwnd;
	HDC m_hdc;
	HDC m_mem;

	HGameObject m_root;

	Vec2 m_offset;
	Vec2 m_scale;
} * HRenderContext;

/* See `src/egdi/rdrctx.c` */
extern HRenderContext RenderContext;

/*
 * Indirect Initializers
 *
 * - Initialization should be indirect because of vtable.
 *
 */

typedef struct
{
	LPCWSTR filepath;
} TEXTURE;

typedef struct
{
	RESERVED(sizeof(void*));
} TEXT;

/*
 * Virtual-Table/Namespace Declarations
 *
 * - First 32bit of vtable is used to indicates actual type of struct
 * - An object loaded by 'Load' must be released by 'Release'.
 *   Not released object causes memory-leak and undefined-behavior.
 *
 */

struct __GameObjectVTBL
{
#define GO_VTBL(tthis)                            \
    uint32_t Signature;                           \
                                                  \
    HGameObject m_parent;                         \
    HGameObject* m_arrChild;                      \
    DWORD m_nChild;                               \
    DWORD m_cChild;                               \
                                                  \
    Vec2 m_position;                              \
	Vec2 m_scale;                                 \
                                                  \
    tthis (*Load)(HGameObject parent, void* arg); \
    void (*Release)(tthis this);                  \
                                                  \
    void (*AddChild)(tthis this, HGameObject hgo);\
    void (*DelChild)(tthis this, HGameObject hgo);\
                                                  \
    Vec2 (*GetPosition)(tthis this);              \
    void (*SetPosition)(tthis this, Vec2 pos);    \
    Vec2 (*GetScale)(tthis this);                 \
    void (*SetScale)(tthis this, Vec2 scale);     \
                                                  \
    BOOL (*Render)(tthis this, HRenderContext ctx)

	GO_VTBL(HGameObject);
};
extern struct __GameObjectVTBL GameObject;

struct __TextureVTBL
{
	GO_VTBL(HTexture);

	LONG_COORD (* GetBitmapSize)(HTexture this);
};
extern struct __TextureVTBL Texture;

struct __TextVTBL
{
	GO_VTBL(HText);

	LPCWSTR (* GetValue)(HText this);
	void (* SetValue)(HText this, LPCWSTR txt);

	HFONT (* GetFont)(HText this);
	void (* SetFont)(HText this, HFONT hf);
};
extern struct __TextVTBL Text;


/*
 * Plain Old Data Structures
 *
 * - Functions of object should be written in vtable
 * - Data field of object must be written in here
 *
 */

struct __GameObject
{
	struct __GameObjectVTBL Vtbl;
};

struct __Texture
{
	struct __TextureVTBL Vtbl;

	LPCSTR m_lpszPath;
	HBITMAP m_hbm;
};

struct __Text
{
	struct __TextVTBL Vtbl;

	LPCWSTR m_lpszText;
	HFONT m_hf;
};

/*
 * Global Initializers
 *
 * - Keep it simple, stupid.
 *
 */

HRESULT EGDI_Initialize();
HRESULT EGDI_Render();

#endif //SPROG_MAN_H
