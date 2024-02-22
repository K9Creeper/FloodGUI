#include "flood_gui_draw.h"

#include "flood_gui.h"

struct FloodGuiD3D9ata
{
    LPDIRECT3DDEVICE9           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9     pVB;
    LPDIRECT3DINDEXBUFFER9      pIB;
    LPDIRECT3DTEXTURE9          FontTexture;
    int                         VertexBufferSize;
    int                         IndexBufferSize;

    FloodGuiD3D9ata() { 
        memset((void*)this, 0, sizeof(*this)); // Allocate space for ourselves
        
        // Size up
        VertexBufferSize = 5000; 
        IndexBufferSize = 10000; 
    }
};


bool FloodDrawData::isMinimized()
{
    return (Display->DisplaySize.x <= 0.0f || Display->DisplaySize.y <= 0.0f);
}

static void SetUpRenderState()
{
    FloodGuiD3D9ata* bd = FloodGui::Context.IO.BackendRendererUserData;

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)FloodGui::Context.Display.DisplaySize.x;
    vp.Height = (DWORD)FloodGui::Context.Display.DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    bd->pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
    bd->pd3dDevice->SetPixelShader(nullptr);
    bd->pd3dDevice->SetVertexShader(nullptr);
    bd->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    bd->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    bd->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    bd->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
    bd->pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
    bd->pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    bd->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
    bd->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    bd->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    bd->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // orthographic projection matrix 
    {
        float L = FloodGui::Context.Display.DisplayPosition.x + 0.5f;
        float R = FloodGui::Context.Display.DisplayPosition.x + FloodGui::Context.Display.DisplaySize.x + 0.5f;
        float T = FloodGui::Context.Display.DisplayPosition.y + 0.5f;
        float B = FloodGui::Context.Display.DisplayPosition.y + FloodGui::Context.Display.DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection =
        { { {
            2.0f / (R - L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f / (T - B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f
        } } };
        bd->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        bd->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

bool FloodGui_D3D9_Init(IDirect3DDevice9* device)
{
    // We should check if we already intialized this..
    if (!FloodGui::Context.Initalized)
        return false;
    FloodGuiD3D9ata* bd = new FloodGuiD3D9ata();

    FloodGui::Context.IO.BackendRendererUserData = bd;

    bd->pd3dDevice = device;
    bd->pd3dDevice->AddRef();
}

void FloodGui_D3D9_Shutdown() {

}

void FloodGui_D3D9_NewFrame() {

}

void FloodGui_D3D9_RenderDrawData(FloodDrawData* drawData) {
    // not fun if minimized
    if (drawData->isMinimized())
        return;


}