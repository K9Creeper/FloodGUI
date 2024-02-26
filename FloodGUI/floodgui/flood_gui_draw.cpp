#include "flood_gui_draw.h"

#include "flood_gui.h"

#include <algorithm>
#include <iostream>

#include "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/d3dx9.h"
#pragma comment(lib, "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64/d3dx9.lib")


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
        for (int16_t i = out.size() - 1; i > 0; i--)
        {
            const std::pair<const char*, FloodWindow*> pair1 = out[i];
            const std::pair<const char*, FloodWindow*> pair2 = out[i - 1];

            const uint16_t& z1 = pair1.second->GetZIndex();
            const uint16_t& z2 = pair2.second->GetZIndex();

            if (z1 > z2)
            {
                out[i] = pair2;
                out[i - 1] = pair1;
                sort = true;
            }
        }
    }
    return out;
}

constexpr bool FindPoint(const FloodVector2& min, const FloodVector2& max, const FloodVector2& point)
{
    return (point.x > min.x && point.x < max.x&& point.y > min.y && point.y < max.y);
}

FloodWindow* get_window_hovering()
{
    FloodWindow* interest = nullptr;
    const FloodVector2& mouse_pos = FloodGui::Context.IO.mouse_pos;
    for (const auto& [name, window] : FloodGui::Context.Windows)
    {
        if (FindPoint(window->GetFullBoundingMin(), window->GetFullBoundingMax(), mouse_pos))
        {
            if (!interest || window->GetZIndex() < interest->GetZIndex()) {
                interest = window;
            }
        }
    }
    return interest;
}

void FloodGui::NewFrame() {
    // Open DrawList
    //
    FloodGui::Context.FrameStage = FloodRenderStage_FrameStart;
    FloodGui::Context.DrawData->DrawLists.clear();
    
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
    for (const auto& [name, window] : SortWindows()) { drawData->DrawLists.push_back(window->GetDrawList()); }
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
    if (!FloodGui::Context.Initalized)
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
        vp.Width = (DWORD)((int)drawData->Display->DisplaySize.x);
        vp.Height = (DWORD)((int)drawData->Display->DisplaySize.y);
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
        int vtx_offset = 0;
        int idx_offset = 0;

        for (int i = 0; i < cmd_list->Elements.size(); i++) {
            const FloodDrawMaterial& material = cmd_list->Elements[i];
            /*
                Todo:
                Work on texture / text rendering
            */
            backend_data->pd3dDevice->SetTexture(0, material.texture);
            backend_data->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset + global_vtx_offset, 0, (UINT)cmd_list->VertexBuffer.size(), idx_offset + global_idx_offset, material.index_count / 3);
            idx_offset += material.index_count;
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

void FocusNewWindow(FloodWindow* target)
{
    if (target->WindowIsActive())
        return;
    uint16_t prev_index = target->GetZIndex();
    for (auto& [name, window] : FloodGui::Context.Windows)
    {
        const uint16_t& win_z = window->GetZIndex();
        if (win_z < prev_index && target != window /* Just in case??*/) {
            window->SetZIndex(window->GetZIndex() + 1);
            window->SetWindowActive(false);
        }
    }
    target->SetWindowActive(true);
}

void FloodGui::BeginWindow(const char* windowName)
{
    FloodContext& context = FloodGui::Context;
    bool windowExists = context.Windows.find(windowName) != context.Windows.end();
    FloodWindow* window = nullptr;
    if (!windowExists)
    {
        window = new FloodWindow({ 200, 200 });
        window->SetZIndex(context.ActiveDrawingWindowZIndex++);
        context.Windows[windowName] = window;
    }
    else
        window = context.Windows[windowName];
    context.ActiveDrawingWindow = window;
    FloodDrawList* DrawList = window->GetDrawList();
    DrawList->Clear();

    if (window == get_window_hovering() && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse]) {
        FocusNewWindow(window);
        if (FindPoint(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), FloodGui::Context.IO.mouse_pos)) {
            const FloodVector2& dst = FloodGui::Context.IO.mouse_pos - FloodGui::Context.IO.pmouse_pos ;
            window->MoveWindow(dst);
        }
    }

    DrawList->AddRectFilled(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), window->WindowIsActive() ? Context.colors[FloodGuiCol_WinTitleBarActive] :Context.colors[FloodGuiCol_WinTitleBar]);
    DrawList->AddRectFilled(window->GetBoundingContentMin(), window->GetBoundingContentMax() , Context.colors[FloodGuiCol_WinBkg]);
}

void FloodGui::EndWindow() {
    FloodContext& context = FloodGui::Context;
    if (context.Windows.size() == 0 || !context.ActiveDrawingWindow)
        return;

    context.ActiveDrawingWindow = nullptr;
}

void FloodDrawList::AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col)
{
    static const int index_count = 6, vertex_count = 4;
    Elements.push_back(FloodDrawMaterial{ { min, {max.x,min.y}, max, { min.x, max.y } },  col, 0.f, index_count, vertex_count });
    ReserveGeo(index_count, vertex_count);
    AllocRectFilled(min, max, col);
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

void FloodDrawList::AllocRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col)
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

void FloodDrawList::AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness)
{
    AddPolyLine({ p1, p2 }, col, thickness);
}

void FloodDrawList::AllocChar(char text, const FloodVector2& position, FloodColor col, float font_size) {
    const float width = font_size;
    const float height = font_size * 1.3f;
    switch (text)
    {
    case 'A':
    case 'a':
    {
        const FloodVector2& top = FloodVector2(position) + FloodVector2(width / 2, -height);
        // Diagnals
        AddLine(position, top, col, 1);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        // Line in middle
        AddLine(FloodVector2(position)+FloodVector2(width*.2,-height/ 2.6f), FloodVector2(position) + FloodVector2(width- width * .2, -height / 2.6f),  col, 1);
        break;
    }
    case 'B':
    case 'b':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 1.7f);
        // Left ver line
        AddLine(position, top, col, 1);
        
        // Top horz line
        AddLine(top, top + FloodVector2{width- width*.19f, 0}, col, 1);

        // Bottom horz line
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        // Middle horz line
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);

        // Right lines
        AddLine(FloodVector2(position) + FloodVector2(width, 0), middle + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2{ width - width * .19f, 0 }, middle + FloodVector2{ width - width * .19f, 0 }, col, 1);
        break;
    }
    case 'C':
    case 'c':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position)+FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'D':
    case 'd':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(width, -height / 2.f);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width* .8, 0), col, 1);
        AddLine(top, top + FloodVector2(width * .8, 0), col, 1);

        AddLine(FloodVector2(position) + FloodVector2(width * .8, 0), middle, col, 1);
        AddLine(top + FloodVector2(width * .8, 0), middle, col, 1);
        break;
    }
    case 'E':
    case 'e':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);

        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'F':
    case 'f':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(position, top, col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width*.7f, 0), col, 1);

        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'G':
    case 'g':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        // Left ver line
        AddLine(position, top, col, 1);

        // Top horz line
        AddLine(top, top + FloodVector2{ width, 0 }, col, 1);

        // Bottom horz line
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        // Middle horz line
        AddLine(middle+ FloodVector2(width*.25f, 0), middle + FloodVector2(width, 0), col, 1);

        // Right lines
        AddLine(FloodVector2(position) + FloodVector2(width, 0), middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'H':
    case 'h':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0) , top + FloodVector2(width, 0), col, 1);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'I':
    case 'i':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'J':
    case 'j':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top + FloodVector2(width*.3f, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position)+ FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width * .2f, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width * .2f, 0), FloodVector2(position) + FloodVector2(width * .2f, -height*.35f), col, 1);
        break;
    }
    case 'K':
    case 'k':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(middle, top + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'L':
    case 'l':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'M':
    case 'm':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height/2.5f);
        AddLine(position, top, col, 1);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top+ FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position)+ FloodVector2(width, 0), col, 1);
        break;
    }
    case 'N':
    case 'n':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(position, top, col, 1);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'O':
    case 'o':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'P':
    case 'p':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), middle+ FloodVector2(width, 0), col, 1);
        AddLine(middle + FloodVector2(width, 0), middle, col, 1);

        break;
    }
    case 'Q':
    case 'q':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height / 2.5f);
        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), vmiddle, col, 1);
        break;
    }
    case 'R':
    case 'r':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), middle + FloodVector2(width, 0), col, 1);
        AddLine(middle + FloodVector2(width, 0), middle, col, 1);
        AddLine(middle + FloodVector2(width*.3f,0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'S':
    case 's':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(middle, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(middle+ FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'T':
    case 't':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'U':
    case 'u':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'V':
    case 'v':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'W':
    case 'w':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height / 2.5f);
        AddLine(position, top, col, 1);
        AddLine(position, vmiddle, col, 1);
        AddLine(vmiddle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'X':
    case 'x':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) , col, 1);
        break;
    }
    case 'Y':
    case 'y':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height/2.f);
        FloodVector2 vmiddle2 = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        AddLine(vmiddle, vmiddle2, col, 1);
        break;
    }
    case 'Z':
    case 'z':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top + FloodVector2(width, 0), top, col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), position, col, 1);
        break;
    }
    }
}

void FloodDrawList::AddText(const char* text, const FloodVector2& position, FloodColor col, float font_size, float spacing)
{
    for (uint16_t i = 0; i < strlen(text); i++)
    {
        const char& c = text[i];
        AllocChar(c, FloodVector2(position) + FloodVector2{ i * (spacing + font_size), 0 }, col, font_size);
    }
}

void FloodDrawList::AddPolyLine(const std::vector<FloodVector2> points, FloodColor col, float thickness)
{
    if (points.size() < 2)
        return;
    const int nPoints = points.size();
    const int idx_count = nPoints * 6;
    const int vtx_count = nPoints * 4;
    
    ReserveGeo(idx_count, vtx_count);
    FloodVector2 uv(0.f, 0.f);
    unsigned int color = ColorToUint32(col);
    for (int i1 = 0; i1 < nPoints; i1++)
    {
        const int i2 = (i1 + 1) == nPoints ? 0 : i1 + 1;
        const FloodVector2& p1 = points[i1];
        const FloodVector2& p2 = points[i2];

        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;

        float d2 = dx * dx + dy * dy;
        if (d2 > 0.0f) { 
            float inv_len =(1.0f / sqrtf(d2));
            dx *= inv_len;
            dy *= inv_len;
        }

        dx *= (thickness * 0.5f);
        dy *= (thickness * 0.5f);

        VertexWrite[0].position.x = p1.x + dy; VertexWrite[0].position.y = p1.y - dx; VertexWrite[0].uv = uv; VertexWrite[0].col = color;
        VertexWrite[1].position.x = p2.x + dy; VertexWrite[1].position.y = p2.y - dx; VertexWrite[1].uv = uv; VertexWrite[1].col = color;
        VertexWrite[2].position.x = p2.x - dy; VertexWrite[2].position.y = p2.y + dx; VertexWrite[2].uv = uv; VertexWrite[2].col = color;
        VertexWrite[3].position.x = p1.x - dy; VertexWrite[3].position.y = p1.y + dx; VertexWrite[3].uv = uv; VertexWrite[3].col = color;
        Elements.push_back(FloodDrawMaterial{ {VertexWrite[0].position, VertexWrite[1].position, VertexWrite[2].position, VertexWrite[3].position }, col, thickness, 6, 4 });
        VertexWrite += 4;

        IndexWrite[0] = (FloodDrawIndex)(VertexCurrentIdx); IndexWrite[1] = (FloodDrawIndex)(VertexCurrentIdx + 1); IndexWrite[2] = (FloodDrawIndex)(VertexCurrentIdx + 2);
        IndexWrite[3] = (FloodDrawIndex)(VertexCurrentIdx); IndexWrite[4] = (FloodDrawIndex)(VertexCurrentIdx + 2); IndexWrite[5] = (FloodDrawIndex)(VertexCurrentIdx + 3);
        IndexWrite += 6;
        VertexCurrentIdx += 4;
    }
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
