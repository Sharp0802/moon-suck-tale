#include "egdi/egdi.h"
#include "error.h"

HRenderContext RenderContext;

HRESULT EGDI_Render()
{
	if (!RenderContext->m_root->Vtbl.Render(RenderContext->m_root, RenderContext))
	{
		DWORD code = GetLastError();
		DialogWin32Code(code);
		return HRESULT_FROM_WIN32(code);
	}

	return S_OK;
}
