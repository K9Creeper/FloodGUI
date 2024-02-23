#include "flood_gui_draw.h"

#include "flood_gui.h"

#include <algorithm>
#include <iostream>



struct FloodGuiD3D9Data
{
    LPDIRECT3DDEVICE9           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9     pVB;
    LPDIRECT3DINDEXBUFFER9      pIB;
    LPDIRECT3DTEXTURE9          FontTexture;
   
    int                         VertexBufferSize;
    int                         IndexBufferSize;

    FloodGuiD3D9Data() { 
        memset((void*)this, 0, sizeof(*this)); // Make sure to zero padding! If not pVB and pIB are going to be angwy!!
        VertexBufferSize = 5000; 
        IndexBufferSize = 10000; 
    }
};

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
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
        for (int16_t i = out.size()-1; i > 1; i--)
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
    FloodGui::Context.DrawData->DrawLists.clear();
    FloodGuiD3D9Data* backend_data = new FloodGuiD3D9Data();

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
    FloodDrawData* drawData = FloodGui::Context.DrawData;
    FloodGuiD3D9Data* backend_data = new FloodGuiD3D9Data();
    for (const auto&[name, window] : SortWindows()){ drawData->DrawLists.push_back(window->GetDrawList());    }
}

bool FloodDrawData::isMinimized()
{
    return this && Display && (Display->DisplaySize.x <= 0.0f || Display->DisplaySize.y <= 0.0f);
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
    if (drawData->isMinimized()) {
        return;
    }
    
    if (!backend_data->pVB || backend_data->VertexBufferSize < drawData->GetVertexCount())
    {
        if (backend_data->pVB) { backend_data->pVB->Release(); backend_data->pVB = nullptr; }
        backend_data->VertexBufferSize = drawData->GetVertexCount() + 5000;
        if (backend_data->pd3dDevice->CreateVertexBuffer(backend_data->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1), D3DPOOL_DEFAULT, &backend_data->pVB, nullptr) < 0)
            return;
    }
    if (!backend_data->pIB || backend_data->IndexBufferSize < drawData->GetIndexCount())
    {
        if (backend_data->pIB) { backend_data->pIB->Release(); backend_data->pIB = nullptr; }
        backend_data->IndexBufferSize = drawData->GetIndexCount() + 10000;
        if (backend_data->pd3dDevice->CreateIndexBuffer(backend_data->IndexBufferSize * sizeof(FloodDrawIndex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(FloodDrawIndex) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &backend_data->pIB, nullptr) < 0)
            return;
    }

    IDirect3DStateBlock9* d3d9_state_block = nullptr;
    if (backend_data->pd3dDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
        return;
    if (d3d9_state_block->Capture() < 0)
    {
        d3d9_state_block->Release();
        return;
    }

    D3DMATRIX last_world, last_view, last_projection;
    backend_data->pd3dDevice->GetTransform(D3DTS_WORLD, &last_world);
    backend_data->pd3dDevice->GetTransform(D3DTS_VIEW, &last_view);
    backend_data->pd3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

    CUSTOMVERTEX* vtx_dst;
    FloodDrawIndex* idx_dst;
    if (backend_data->pVB->Lock(0, (UINT)(drawData->GetVertexCount() * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
    {
        d3d9_state_block->Release();
        return;
    }
    if (backend_data->pIB->Lock(0, (UINT)(drawData->GetIndexCount() * sizeof(FloodDrawIndex)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
    {
        backend_data->pVB->Unlock();
        d3d9_state_block->Release();
        return;
    }

    for (int n = 0; n < drawData->DrawLists.size(); n++)
    {
        const FloodDrawList* cmd_list = drawData->DrawLists[n];
        const FloodDrawVertex* vtx_src = cmd_list->VertexBuffer.data();
        for (int i = 0; i < cmd_list->VertexBuffer.size(); i++)
        {
            vtx_dst->pos[0] = vtx_src->position.x;
            vtx_dst->pos[1] = vtx_src->position.y;
            std::cout << vtx_dst->pos[0] << ", " << vtx_dst->pos[1] << "\n";
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = vtx_src->col;
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IndexBuffer.data(), cmd_list->IndexBuffer.size() * sizeof(FloodDrawIndex));
        idx_dst += cmd_list->IndexBuffer.size();
    }

    backend_data->pVB->Unlock();
    backend_data->pIB->Unlock();
    backend_data->pd3dDevice->SetStreamSource(0, backend_data->pVB, 0, sizeof(CUSTOMVERTEX));
    backend_data->pd3dDevice->SetIndices(backend_data->pIB);
    backend_data->pd3dDevice->SetFVF((D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));

    {
        D3DVIEWPORT9 vp;
        vp.X = vp.Y = 0;
        vp.Width = (DWORD)drawData->Display->DisplaySize.x;
        vp.Height = (DWORD)drawData->Display->DisplaySize.y;
        vp.MinZ = 0.0f;
        vp.MaxZ = 1.0f;
        backend_data->pd3dDevice->SetViewport(&vp);

        // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
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

        // Setup orthographic projection matrix
        // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
        // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
        {
            D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
            D3DMATRIX mat_projection = drawData->Display->matrix_project();
            backend_data->pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
            backend_data->pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
            backend_data->pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
        }
    }

    int global_vtx_offset = 0;
    int global_idx_offset = 0;

    for (int n = 0; n < drawData->DrawLists.size(); n++)
    {
        const FloodDrawList* cmd_list = drawData->DrawLists[n];  
        
        for (int i = 0; i < cmd_list->Elements.size(); i++) {
            const FloodDrawMaterial& material = cmd_list->Elements[i];
            //backend_data->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, material.vertex_count + global_vtx_offset, 0, (UINT)cmd_list->VertexBuffer.size(), material.index_count + global_idx_offset, material.Points.size() / 3);
        }
        global_idx_offset += cmd_list->IndexBuffer.size();
        global_vtx_offset += cmd_list->VertexBuffer.size();
    }

    
    backend_data->pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
    backend_data->pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
    backend_data->pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
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
    DrawList->Clear();

    DrawList->AddRectFilled(window->position, { window->position.x + window->size.x, window->position.y + window->size.y }, FloodColor());
}

void FloodGui::EndWindow() {
    FloodContext& context = FloodGui::Context;
    if (context.Windows.size() == 0 || !context.ActiveDrawingWindow)
        return;

    context.ActiveDrawingWindow = nullptr;
}

void FloodDrawList::AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness)
{

}

void FloodDrawList::AddRect(const FloodVector2& min, const FloodVector2& max, FloodColor col, float thickness)
{


}

void FloodDrawList::AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col)
{
    static const int index_count = 6, vertex_count = 4;
    Elements.push_back(FloodDrawMaterial{ { min, {max.x,min.y}, max, { min.x, max.y } }, true,  col, 0.f, index_count, vertex_count });
    ReserveGeo(index_count, vertex_count);
    AllocRect(min, max, col);
}

void FloodDrawList::ReserveGeo(const int& index_count, const int& vertex_count)
{
    int vtx_buffer_old_size = VertexBuffer.size();
    VertexBuffer.resize(vtx_buffer_old_size + vertex_count);
    VertexWrite = (VertexBuffer.data() + vtx_buffer_old_size);

    int idx_buffer_old_size = IndexBuffer.size();
    IndexBuffer.resize(idx_buffer_old_size + index_count);
    IndexWrite = (IndexBuffer.data() + idx_buffer_old_size);
}

void FloodDrawList::AllocRect(const FloodVector2& min, const FloodVector2& max, FloodColor col)
{
    FloodVector2 b(max.x, min.y);
    FloodVector2 d(min.x, max.y);
    FloodVector2 uv(0.f, 0.f);
    FloodDrawIndex idx = (FloodDrawIndex)VertexCurrentIdx;
    unsigned int color = ColorToUint32(col);
    IndexWrite[0] = idx; IndexWrite[1] = (FloodDrawIndex)(idx + 1); IndexWrite[2] = (FloodDrawIndex)(idx + 2);
    IndexWrite[3] = idx; IndexWrite[4] = (FloodDrawIndex)(idx + 2); IndexWrite[5] = (FloodDrawIndex)(idx + 3);
    VertexWrite[0].position = min; VertexWrite[0].uv = uv; VertexWrite[0].col = color;
    VertexWrite[1].position = b; VertexWrite[1].uv = uv; VertexWrite[1].col = color;
    VertexWrite[2].position = max; VertexWrite[2].uv = uv; VertexWrite[2].col = color;
    VertexWrite[3].position = d; VertexWrite[3].uv = uv; VertexWrite[3].col = color;
    VertexWrite += 4;
    VertexCurrentIdx += 4;
    IndexWrite += 6;
}

unsigned int FloodDrawData::GetVertexCount() const
{
    unsigned int ttl = 0;
    for (const FloodDrawList* list : DrawLists)
    {
        ttl += list->GetVertexCount();
    }
    return ttl;
}

unsigned int FloodDrawData::GetIndexCount() const
{
    unsigned int ttl = 0;
    for (const FloodDrawList* list : DrawLists)
    {
        ttl += list->GetIndexCount();
    }
    return ttl;
}