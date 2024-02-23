#include "flood_gui_draw.h"

#include "flood_gui.h"

#include <algorithm>

struct FloodGuiD3D9Data
{
    LPDIRECT3DDEVICE9           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9     pVB;
    LPDIRECT3DINDEXBUFFER9      pIB;
    LPDIRECT3DTEXTURE9          FontTexture;
    int                         VertexBufferSize;
    int                         IndexBufferSize;

    FloodGuiD3D9Data() { 
        VertexBufferSize = 5000; 
        IndexBufferSize = 10000; 
    }
};

std::vector<std::pair<const char*, FloodWindow*>>SortWindows() {
    std::vector<std::pair<const char*, FloodWindow*>>out;
    out.reserve(FloodGui::Context.Windows.size());
    for (const auto& [name, window] : FloodGui::Context.Windows) { out.push_back({ name, window }); }
    bool sort = true;
    while (sort)
    {
        // Thanks S. Hendricks!
        // 
        sort = false;
        for (uint16_t i = out.size()-1; i > 1; i--)
        {
            const std::pair<const char*, FloodWindow*> pair1 = out[i];
            const std::pair<const char*, FloodWindow*> pair2 = out[i - 1];

            if (pair1.second->GetZIndex() > pair2.second->GetZIndex())
            {
                out[i] = pair2;
                out[i - 1] = pair1;
                sort = true;
            }
        }
    }
    return out;
}

void FloodGui::NewFrame() {
    // Open DrawList
    //
    FloodGui::Context.FrameStage = FloodRenderStage_FrameStart;
    
}

void FloodGui::EndFrame() {
    // Close DrawList
    //
    FloodGui::Context.FrameStage = FloodRenderStage_FrameEnd;
}

void FloodGui::Render()
{
    
    // Go over DrawList and validate things
    FloodGui::Context.FrameStage = FloodRenderStage_FrameRenderStart;
  
    
}

bool FloodDrawData::isMinimized()
{
    return this && Display && (Display->DisplaySize.x <= 0.0f || Display->DisplaySize.y <= 0.0f);
}

static void SetUpRenderState()
{
    FloodGuiD3D9Data* backend_data = FloodGui::Context.IO.BackendRendererData;

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)FloodGui::Context.Display.DisplaySize.x;
    vp.Height = (DWORD)FloodGui::Context.Display.DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    backend_data->pd3dDevice->SetViewport(&vp);

    backend_data->pd3dDevice->SetPixelShader(nullptr);
    backend_data->pd3dDevice->SetVertexShader(nullptr);
    backend_data->pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    backend_data->pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    backend_data->pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_CLIPPING, TRUE);
    backend_data->pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    backend_data->pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    backend_data->pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    backend_data->pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    backend_data->pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    backend_data->pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    {
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection = FloodGui::Context.Display.matrix_project();
        
        backend_data->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        backend_data->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        backend_data->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }
}

bool FloodGuiD3D9Init(IDirect3DDevice9* device)
{
    // We should check if we already intialized this..
    if (FloodGui::Context.Initalized)
        return false;
    FloodGuiD3D9Data* backend_data = new FloodGuiD3D9Data();

    FloodGui::Context.IO.BackendRendererData = backend_data;

    backend_data->pd3dDevice = device;
    backend_data->pd3dDevice->AddRef();
    FloodGui::Context.Initalized = true;
}

void FloodGuiD3D9Shutdown() {
    if (!FloodGui::Context.Initalized)
        return;
    FloodGuiD3D9Data* backend_data = FloodGui::Context.IO.BackendRendererData;
    if (!backend_data || !backend_data->pd3dDevice)
        return;
    if (backend_data->pVB) { backend_data->pVB->Release(); backend_data->pVB = nullptr; }
    if (backend_data->pIB) { backend_data->pIB->Release(); backend_data->pIB = nullptr; }
    if (backend_data->FontTexture) { backend_data->FontTexture->Release(); backend_data->FontTexture = nullptr; }
    if (backend_data->pd3dDevice) { backend_data->pd3dDevice->Release(); }
    delete backend_data;
}

void FloodGuiD3D9NewFrame() {
    // Todo:
    // We should validate fonts if needed here...

}

void FloodGuiD3D9RenderDrawData(FloodDrawData* drawData) {
    if(!FloodGui::Context.Initalized)
        return;
    FloodGuiD3D9Data* backend_data = FloodGui::Context.IO.BackendRendererData;
    if (!backend_data)
        return;
    FloodGui::Context.FrameStage = FloodRenderStage_FrameRenderEnd;
    // not fun if minimized
    if (drawData->isMinimized())
        return;
    
    // Store state
    IDirect3DStateBlock9* d3d9_state = nullptr;
    if (backend_data->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state) < 0)
        return;
    if (d3d9_state->Capture() < 0)
    {
        d3d9_state->Release();
        return;
    }
    
    // Todo:
    // Transform
    // Setup the render state

    d3d9_state->Apply();
    d3d9_state->Release();
}


void FloodGui::BeginWindow(const char* windowName)
{
    FloodContext& context = FloodGui::Context;
    bool windowExists = context.Windows.find(windowName) != context.Windows.end();
    FloodWindow* window = nullptr;
    if (!windowExists)
    {
        window = new FloodWindow({ 200, 200 });
        context.Windows[windowName] = window;
        context.ActiveDrawingWindow = window;
    }
    else
        window = context.Windows[windowName];
    FloodDrawList* DrawList = window->GetDrawList();
    DrawList->AddRectFilled();
}

void FloodGui::EndWindow() {
    FloodContext& context = FloodGui::Context;
    if (context.Windows.size() == 0 || !context.ActiveDrawingWindow)
        return;



    context.ActiveDrawingWindow = nullptr;
}



void FloodDrawList::AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness)
{
    PathLineTo(p1);
    PathLineTo(p2);

    AddPoly(Paths, col, thickness);
    Paths.clear();
}

void FloodDrawList::AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col)
{
    PathRect(min, max);
    AddPolyFilled({ min , max }, col);
    Paths.clear();
}

void FloodDrawList::AddRect(const FloodVector2& min, const FloodVector2& max, FloodColor col, float thickness) {
    PathRect(min, max);
    AddPoly(Paths, col, thickness);
    Paths.clear();
}

void FloodDrawList::PathRect(const FloodVector2& a, const FloodVector2& b)
{
    PathLineTo(a);
    PathLineTo(FloodVector2(b.x, a.y));
    PathLineTo(b);
    PathLineTo(FloodVector2(a.x, b.y));
}

void FloodDrawList::AddPoly(const std::vector<FloodVector2> points, FloodColor col, float thickness) {
    if (points.size() < 2)
        return;

}

void FloodDrawList::AddPolyFilled(const std::vector<FloodVector2> points, FloodColor col)
{
    // You can't fill in a non-shape...lines are not shapes
    if (points.size() < 3)
        return;

}