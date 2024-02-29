#include "flood_gui_draw.h"
#include "flood_gui.h"

#include <algorithm>
#include <cstddef>
#include <iostream>


struct FloodGuiD3D9Data
{
    LPDIRECT3DDEVICE9           pd3dDevice;
    LPDIRECT3DVERTEXBUFFER9     pVB;
    LPDIRECT3DINDEXBUFFER9      pIB;

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


//                  //
// HELPER FUNCTIONS //
//                  //
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
            const std::pair<const char*, FloodWindow*>& pair1 = out[i];
            const std::pair<const char*, FloodWindow*>& pair2 = out[i - 1];

            const uint16_t& z1 = pair1.second->GetZIndex();
            const uint16_t& z2 = pair2.second->GetZIndex();

            if (z1 > z2)
            {
                out[i] = pair2;
                out[i-1] = pair1;
                sort = true;
            }
        }
    }
    return out;
}
FloodWindow* get_window_hovering()
{
    FloodWindow* interest = nullptr;
    const FloodVector2& mouse_pos = FloodGui::Context.IO.mouse_pos;
    // Here we are looping through all windows
    for (const auto& [name, window] : FloodGui::Context.Windows)
    {
        // We check if the mouse is on the window
        if (FindPoint(window->GetFullBoundingMin(), window->GetFullBoundingMax(), mouse_pos))
        {
            // Now if a other window of interest is there but it has a higher z-index...
            // meaning it is behind our current window of interest then the current window is not our only window of interest
            if (!interest || window->GetZIndex() < interest->GetZIndex()) {
                interest = window;
            }
        }
    }
    // return a window of interest--if there is one
    return interest;
}
void FocusNewWindow(FloodWindow* target)
{
    // We check if the target is already active
    // if so no need to compute
    if (target->WindowIsActive())
        return;
    // Store the pre-index
    uint16_t prev_index = target->GetZIndex();
    // Looping through all windows
    for (auto& [name, window] : FloodGui::Context.Windows)
    {
        const uint16_t& win_z = window->GetZIndex();
        // So now if the current window of the loop
        // has a z-index lower, meaning more in focus
        // then we will need to shift it down
        if (win_z < prev_index && target != window /* Just in case??*/) {
            window->SetZIndex(window->GetZIndex() + 1);
            window->SetWindowActive(false);
        }
    }
    // Ok, now this is where we can now set this window as active
    target->SetWindowActive(true);
    FloodGui::Context.ActiveWindow = target;
}
FloodVector2 CalcTextSize(const char* text, const float& size, const float& spacing)
{
    return FloodVector2(static_cast<int>(strlen(text)) * (size + spacing), size*1.3f);
}
bool ChangeCursor(LPCWSTR cursor)
{
    // We like, load the cursor img / data--possibly can be optimized?
    if (HCURSOR hCursor = LoadCursor(NULL, cursor)) {
        // Then we set it...
        return SetCursor(hCursor);
    }
    return false;
}
//                                  //
//  Global Flood Gui D3D Functions  //
//                                  //

bool FloodGuiD3D9Init(IDirect3DDevice9* device)
{
    if (FloodGui::Context.Initalized)
        return false;

    // Set up the backend for rendering...
    FloodGuiD3D9Data* backend_data = new FloodGuiD3D9Data();
    FloodGui::Context.IO.BackendRendererData = backend_data;

    // We now set up our ver. of the device
    backend_data->pd3dDevice = device;
    backend_data->pd3dDevice->AddRef();

    // Notfiy that backend has been initalized
    FloodGui::Context.Initalized = true;
}

void FloodGuiD3D9Shutdown() {
    // This is clearly cleaning up and freeing memory
    if (!FloodGui::Context.Initalized)
        return;

    FloodGuiD3D9Data* backend_data = FloodGui::Context.IO.BackendRendererData;
    if (!backend_data || !backend_data->pd3dDevice)
        return;
    if (backend_data->pVB) { backend_data->pVB->Release(); backend_data->pVB = nullptr; }
    if (backend_data->pIB) { backend_data->pIB->Release(); backend_data->pIB = nullptr; }
    if (backend_data->pd3dDevice) { backend_data->pd3dDevice->Release(); }
    delete backend_data;
}

void FloodGuiD3D9NewFrame() {
    // This is practically unused
    //

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

    // We extend the buffer size if needed
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

    // We should store the d3d state
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
    // We are filling up the global drawlist
    //
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

    // Here we are setting up the veiwport and rendering
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

        // We set up our viewmatrix..this is very important
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

    // We loop throught the global draw list
    for (int n = 0; n < drawData->DrawLists.size(); n++)
    {
        const FloodDrawList* cmd_list = drawData->DrawLists[n];
        
        int vtx_offset = 0;
        int idx_offset = 0;

        // Here we set the clip dimensions of the current window
        const FloodVector2& minRect = cmd_list->parent ? cmd_list->parent->GetFullBoundingMin() : FloodVector2();
        const FloodVector2& maxRect = cmd_list->parent ? cmd_list->parent->GetFullBoundingMax() : drawData->Display->DisplaySize;
        const RECT& r = { (LONG)minRect.x, (LONG)minRect.y, (LONG)maxRect.x, (LONG)maxRect.y };
        
        // We are looping through practically every cmd to the gpu
        for (int i = 0; i < cmd_list->Elements.size(); i++) {
            const FloodDrawMaterial& material = cmd_list->Elements[i];
            // This is to clip the drawings to inside the window
            
            backend_data->pd3dDevice->SetScissorRect(&r);
            // This is where we draw our vertexs and points
            backend_data->pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset + global_vtx_offset, 0, (UINT)cmd_list->VertexBuffer.size(), idx_offset + global_idx_offset, material.index_count / 3);
            // Make sure to increase the index offsets, or we are drawing points in the wrong order
            idx_offset += material.index_count;
        }

        // Make sure to increase these offsets or we are not going to be
        // drawing anything correctly
        global_idx_offset += cmd_list->IndexBuffer.size();
        global_vtx_offset += cmd_list->VertexBuffer.size();
    }

    // Now can begin resoring
    backend_data->pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
    backend_data->pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
    backend_data->pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}


//                    //
// Namespace FloodGui //
//                    //
void FloodGui::NewFrame() {
    FloodGui::Context.FrameStage = FloodRenderStage_FrameStart;

    // Make sure we clear the global draw list
    //
    if(FloodGui::Context.DrawData->Foreground)
    FloodGui::Context.DrawData->Foreground->Clear();
    if(FloodGui::Context.DrawData->Background)
    FloodGui::Context.DrawData->Background->Clear();
    FloodGui::Context.DrawData->DrawLists.clear();
}

void FloodGui::EndFrame() {
    FloodGui::Context.FrameStage = FloodRenderStage_FrameEnd;
}

void FloodGui::Render()
{
    // Go over DrawList and validate things
    FloodGui::Context.FrameStage = FloodRenderStage_FrameRenderStart;
    FloodDrawData* drawData = FloodGui::Context.DrawData;
    // Organize Global Draw Data
    if (FloodGui::Context.DrawData->Foreground)
    drawData->DrawLists.push_back(FloodGui::Context.DrawData->Foreground);
    for (const auto& [name, window] : SortWindows()) { drawData->DrawLists.insert(drawData->DrawLists.begin(), (window->GetDrawList())); }
    if (FloodGui::Context.DrawData->Background)
    drawData->DrawLists.push_back(FloodGui::Context.DrawData->Background);
}

void FloodGui::BeginWindow(const char* windowName)
{
    FloodContext& context = FloodGui::Context;
    bool windowExists = context.Windows.find(windowName) != context.Windows.end();
    FloodWindow* window = nullptr;
    // Create a new window if it doesnt exist
    if (!windowExists)
    {
        window = new FloodWindow(windowName, { 400, 400 });
        window->SetZIndex(context.ActiveDrawingWindowZIndex++);
        context.Windows[windowName] = window;
    }
    else
        window = context.Windows[windowName];
    context.ActiveDrawingWindow = window;

    FloodDrawList* DrawList = window->GetDrawList();  window->Clear();
    // Window Interaction
    {
        FloodWindow* hovered = get_window_hovering();
        static bool PrevMouseClick;
        static FloodVector2 relative_dst1;
        bool onmouseUp = PrevMouseClick && !FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];
        bool onmouseDown = !PrevMouseClick && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];
        // Window Resizing
        {
            if (window->WindowIsActive() && window == hovered && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
            {
                if (FindPoint(window->GetBoundingContentMax() - 20.f, window->GetBoundingContentMax(), FloodGui::Context.IO.mouse_pos)) {
                    const FloodVector2& dst = FloodGui::Context.IO.mouse_pos - relative_dst1;
                    if (relative_dst1.x != 0 && relative_dst1.y != 0)
                    {
                        window->ResizeWindow((window->GetBoundingContentMax() - window->GetBoundingContentMin()) + dst);
                    }
                    relative_dst1 = FloodGui::Context.IO.mouse_pos;
                }
            }
            else if (window->WindowIsActive() && window == hovered && onmouseUp)
            {
                relative_dst1 = FloodVector2(0, 0);
            }
            if (window == hovered) {
                if (FindPoint(window->GetBoundingContentMax() - 20.f, window->GetBoundingContentMax(), FloodGui::Context.IO.mouse_pos))
                {
                    ChangeCursor(IDC_SIZENWSE);
                }
                else {
                    ChangeCursor(IDC_ARROW);
                }
            }
            else {
                ChangeCursor(IDC_ARROW);
            }
        }

        static FloodVector2 relative_dst2;
        if (window == hovered && onmouseDown)
        {
            // Get realative distance to top left on first click
            relative_dst2 = window->GetFullBoundingMin() - FloodGui::Context.IO.mouse_pos;
        }
        if ((window != hovered && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse]) || onmouseUp)
        {
            relative_dst2.x = 1; // invalidate
        }
        if (relative_dst2.x <= 0 && window == hovered && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse]) {
            // Window Focusing
            FocusNewWindow(window);
            // Window Dragging
            if (FindPoint(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), FloodGui::Context.IO.mouse_pos)) {
                const FloodVector2& dst = FloodGui::Context.IO.mouse_pos - FloodGui::Context.IO.pmouse_pos;
                window->MoveWindow(relative_dst2 +dst);
            }
        }

        PrevMouseClick = FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];
    }
    static const int font_size = 7;
    static const int spacing = 7;
    DrawList->AddRectFilled(window->GetBoundingContentMin(), window->GetBoundingContentMax() , Context.colors[FloodGuiCol_WinBkg]);
    DrawList->AddTriangleFilled(window->GetBoundingContentMax() - FloodVector2(0, 20.f), window->GetBoundingContentMax() - FloodVector2(20.f, 0), window->GetBoundingContentMax(), (FindPoint(window->GetBoundingContentMax() - 20.f, window->GetBoundingContentMax(), FloodGui::Context.IO.mouse_pos) ? Context.colors[FloodGuiCol_ResizeActivated] : Context.colors[FloodGuiCol_Resize]));
    DrawList->AddRectFilled(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), window->WindowIsActive() ? Context.colors[FloodGuiCol_WinTitleBarActive] : Context.colors[FloodGuiCol_WinTitleBar]);
    DrawList->AddText(windowName, window->GetBoundingTitleMin() + FloodVector2(font_size / .4f, font_size * (font_size / 3.1f)), Context.colors[FloodGuiCol_Text], font_size, spacing);
}
void FloodGui::EndWindow() {
    FloodContext& context = FloodGui::Context;
    if (context.Windows.size() == 0 || !context.ActiveDrawingWindow)
        return;

    context.ActiveDrawingWindow = nullptr;
}

bool FloodGui::Button(const char* id) {
    FloodWindow* win = Context.ActiveDrawingWindow;
    FloodVector2 offset{15, 15};
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{0, win->content[i].second+ offset.y};
    FloodVector2 textSize = CalcTextSize(id, 7, 7);
    FloodVector2 innerPadding = FloodVector2(textSize.x/5.f, textSize.y);
    FloodVector2 boxMin = win->GetBoundingContentMin() + offset;
    FloodVector2 boxMax = boxMin + textSize + innerPadding;
    const bool isHovering = win->WindowIsActive() && FindPoint(boxMin, boxMax, Context.IO.mouse_pos);
    static bool pass = true;

    win->GetDrawList()->AddRectFilled(boxMin, boxMax, isHovering && pass ? Context.colors[FloodGuiCol_ButtonHovered] : Context.colors[FloodGuiCol_Button]);
    win->GetDrawList()->AddText(id, boxMin + FloodVector2(0, textSize.y) + innerPadding/3.f, Context.colors[FloodGuiCol_Text], 7, 7);
    win->content.push_back({ id, boxMax.y - boxMin.y });
    if (pass && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHovering)
    {
        pass = false;
        return true;
    }
    else if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass = true;
    else if(Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass = false;
        
    return false;
}


bool FloodGui::Checkbox(const char* id, bool* val) {
    FloodWindow* win = Context.ActiveDrawingWindow;
    FloodVector2 offset{ 15, 15 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + offset.y };
    FloodVector2 textSize = CalcTextSize(id, 6, 6);

    FloodVector2 boxMin = win->GetBoundingContentMin() + offset;
    FloodVector2 boxMax = boxMin + FloodVector2(20, 20);

    const bool isHovering = win->WindowIsActive() && FindPoint(boxMin, boxMax, Context.IO.mouse_pos);
    const bool isToggled = (val && *val);
    static bool pass = true;

    win->GetDrawList()->AddRectFilled(boxMin, boxMax, isHovering && pass ? Context.colors[FloodGuiCol_ButtonHovered] : Context.colors[FloodGuiCol_Button]);
    if (isToggled)
        win->GetDrawList()->AddRectFilled(boxMin+ (FloodVector2(20, 20)/5.f), boxMax-(FloodVector2(20, 20) / 5.f), Context.colors[FloodGuiCol_CheckboxActivated]);
    win->GetDrawList()->AddText(id, boxMax + FloodVector2{textSize.x * .2f, -textSize.y/2.f}- (FloodVector2(20, 20) / 7.5f), Context.colors[FloodGuiCol_Text], 6, 6);
    
    win->content.push_back({ id, boxMax.y - boxMin.y });
    if (pass && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHovering)
    {
        if (val)
            *val = !*val;
        pass = false;
        return true;
    }
    else if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass = true;
    else if(Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass = false;

    return false;
}

bool FloodGui::Hotkey(const char* id, uint16_t key, bool global)
{
    // Todo:
    // Drawing

    // Functionality
    //
    
    return false;
}

//                  //
//  Flood Draw List //
//                  //
void FloodDrawList::AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col)
{
    static const int index_count = 6, vertex_count = 4;
    Elements.push_back(FloodDrawMaterial{ { min, {max.x,min.y}, max, { min.x, max.y } },  col, 0.f, index_count, vertex_count });
    ReserveGeo(index_count, vertex_count);
    AllocRectFilled(min, max, col);
}
void FloodDrawList::ReserveGeo(const int& index_count, const int& vertex_count)
{
    // We need to reserve space for the vertexs and indecies
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
    unsigned int color = col.ToU32();
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
void FloodDrawList::AddTriangleFilled(const FloodVector2& a, const FloodVector2& b, const FloodVector2& c, FloodColor col) {
    static const int index_count = 4, vertex_count = 3;
    Elements.push_back(FloodDrawMaterial{ {a,b,c},  col, 0.f, index_count, vertex_count });
    ReserveGeo(index_count, vertex_count);
    AllocTriFilled(a, b, c, col);
}
void FloodDrawList::AllocTriFilled(const FloodVector2& a, const FloodVector2& b, const FloodVector2& c, FloodColor col)
{
    FloodVector2 uv(0.f, 0.f);
    unsigned int color = col.ToU32();
    FloodDrawIndex idx = (FloodDrawIndex)VertexCurrentIdx;
    IndexWrite[0] = idx; 
    IndexWrite[1] = (FloodDrawIndex)(idx + 1); 
    IndexWrite[2] = (FloodDrawIndex)(idx + 2);
    IndexWrite[3] = (FloodDrawIndex)(idx);

    VertexWrite[0].position = a; VertexWrite[0].uv = uv; VertexWrite[0].col = color;
    VertexWrite[1].position = b; VertexWrite[1].uv = uv; VertexWrite[1].col = color;
    VertexWrite[2].position = c; VertexWrite[2].uv = uv; VertexWrite[2].col = color;
    VertexWrite += 3;
    VertexCurrentIdx += 3;
    IndexWrite += 4;
}
void FloodDrawList::AddLine(const FloodVector2& p1, const FloodVector2& p2, FloodColor col, float thickness)
{
    AddPolyLine({ p1, p2 }, col, thickness);
}
void FloodDrawList::AddRect(const FloodVector2& min, const FloodVector2& max, FloodColor col, float thickness)
{
    // Just multiple calls to AddLine
    // (could just call once to AddPolyLine)
    AddLine(min, { max.x, min.y}, col, thickness);
    AddLine({max.x, min.y}, max, col, thickness);
    AddLine(max, {min.x, max.y}, col, thickness);
    AddLine({min.x, max.y}, min, col, thickness);
}
void FloodDrawList::AllocChar(char text, const FloodVector2& position, FloodColor col, float font_size) {
    const float width = font_size;
    const float height = font_size * 1.3f;
    switch (text)
    {
    case 'a':
    case 'A':
    {
        const FloodVector2& top = FloodVector2(position) + FloodVector2(width / 2, -height);
        // Diagnals
        AddLine(position, top, col, 1);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        // Line in middle
        AddLine(FloodVector2(position)+FloodVector2(width*.2,-height/ 2.6f), FloodVector2(position) + FloodVector2(width- width * .2, -height / 2.6f),  col, 1);
        break;
    }
    case 'b':
    case 'B':
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
    case 'c':
    case 'C':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position)+FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'd':
    case 'D':
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
    case 'e':
    case 'E':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'f':
    case 'F':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(position, top, col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width*.7f, 0), col, 1);

        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'g':
    case 'G':
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
    case 'h':
    case 'H':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0) , top + FloodVector2(width, 0), col, 1);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'i':
    case 'I':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'j':
    case 'J':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top + FloodVector2(width*.3f, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position)+ FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width * .2f, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width * .2f, 0), FloodVector2(position) + FloodVector2(width * .2f, -height*.35f), col, 1);
        break;
    }
    case 'k':
    case 'K':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(middle, top + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'l':
    case 'L':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'm':
    case 'M':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height/2.5f);
        AddLine(position, top, col, 1);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top+ FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position)+ FloodVector2(width, 0), col, 1);
        break;
    }
    case 'n':
    case 'N':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(position, top, col, 1);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'o':
    case 'O':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'p':
    case 'P':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), middle+ FloodVector2(width, 0), col, 1);
        AddLine(middle + FloodVector2(width, 0), middle, col, 1);

        break;
    }
    case 'q':
    case 'Q':
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
    case 'r':
    case 'R':
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
    case 's':
    case 'S':
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
    case 't':
    case 'T':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'u':
    case 'U':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'v':
    case 'V':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'w':
    case 'W':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height / 2.5f);
        AddLine(position, top, col, 1);
        AddLine(position, vmiddle, col, 1);
        AddLine(vmiddle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'x':
    case 'X':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) , col, 1);
        break;
    }
    case 'y':
    case 'Y':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -height/2.f);
        FloodVector2 vmiddle2 = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        AddLine(vmiddle, vmiddle2, col, 1);
        break;
    }
    case 'z':
    case 'Z':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top + FloodVector2(width, 0), top, col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), position, col, 1);
        break;
    }
    case '0':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(position, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '1':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width/2.f, 0), col, 1);
        break;
    }
    case '2':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(middle + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(position), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '3':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);

        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);

        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '4':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(middle, top, col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case '5':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(middle, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(middle + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '6':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2{ width, 0 }, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case '7':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '8':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case '9':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top, middle, col, 1);
        AddLine(middle + FloodVector2(width, 0), middle, col, 1);
        break;
    }
    }
}
void FloodDrawList::AddText(const char* text, const FloodVector2& position, FloodColor col, float font_size, float spacing)
{
    // We loop through each char of the text
    for (uint16_t i = 0; i < strlen(text); i++)
    {
        const char& c = text[i];
        // Now we actual fill up the vertex buffer and index buffer
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
    unsigned int color = col.ToU32();
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

//                 //
// Flood Draw Data //
//                 //
bool FloodDrawData::isMinimized()
{
    // A minimized window does not have a valid display size, it is 0
    return this && Display && (Display->DisplaySize.x <= 0.0f || Display->DisplaySize.y <= 0.0f);
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