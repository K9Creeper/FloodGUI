#include "flood_gui_draw.h"
#include "flood_gui.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <functional>


#define FONT_HEIGHT_MODIFIER 1.4f

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

struct FloodHash {
    FloodWindow* parent = nullptr;
    std::string id{};
    FloodHash() {  }
    FloodHash(FloodWindow* win, const char* id) {
        this->parent = win;
        this->id = id;
    }
    uint64_t hash() const noexcept
    {
        std::size_t strHash = std::hash<std::string>{}(this->id);
        return reinterpret_cast<uint64_t>(parent) + strHash;
    }
};

//                  //
// HELPER FUNCTIONS //
//                  //
std::vector<std::pair<const char*, FloodWindow*>>SortWindows(int16_t vector_size) {
    std::vector<std::pair<const char*, FloodWindow*>>out;
    out.reserve(FloodGui::Context.Windows.size());
    for (const auto& [name, window] : FloodGui::Context.Windows) {  out.push_back({ name, window }); }
    
    bool sort = true;
    while (sort)
    {
        // Thanks S. Hendricks!
        // 
        sort = false;
        for (int16_t i = vector_size - 1; i > 0; i--)
        {
            const std::pair<const char*, FloodWindow*> pair1 = out[i];
            const std::pair<const char*, FloodWindow*> pair2 = out[i - 1];

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
std::vector<std::string> split(std::string s, std::string del = " ")
{
    int start, end = -1 * del.size();
    std::vector<std::string>out{};
    do {
        start = end + del.size();
        end = s.find(del, start);
        out.push_back(s.substr(start, end - start));
    } while (end != -1);
    if (!out.size())
        return {""};
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
            if ((!interest || window->GetZIndex() < interest->GetZIndex()) && window->shouldDraw) {
                interest = window;
            }
        }
    }
    // return a window of interest--if there is one
    return interest;
}
static const char* const GKeyNames[] =
{
    "Tab", "LeftArrow", "RightArrow", "UpArrow", "DownArrow", "PageUp", "PageDown",
    "Home", "End", "Insert", "Delete", "Backspace", "Space", "Enter", "Escape",
    "LeftCtrl", "LeftShift", "LeftAlt", "LeftSuper", "RightCtrl", "RightShift", "RightAlt", "RightSuper", "Menu",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
    "Apostrophe", "Comma", "Minus", "Period", "Slash", "Semicolon", "Equal", "LeftBracket",
    "Backslash", "RightBracket", "GraveAccent", "CapsLock", "ScrollLock", "NumLock", "PrintScreen",
    "Pause", "Keypad0", "Keypad1", "Keypad2", "Keypad3", "Keypad4", "Keypad5", "Keypad6",
    "Keypad7", "Keypad8", "Keypad9", "KeypadDecimal", "KeypadDivide", "KeypadMultiply",
    "KeypadSubtract", "KeypadAdd", "KeypadEnter", "KeypadEqual",
    "GamepadStart", "GamepadBack",
    "GamepadFaceLeft", "GamepadFaceRight", "GamepadFaceUp", "GamepadFaceDown",
    "GamepadDpadLeft", "GamepadDpadRight", "GamepadDpadUp", "GamepadDpadDown",
    "GamepadL1", "GamepadR1", "GamepadL2", "GamepadR2", "GamepadL3", "GamepadR3",
    "GamepadLStickLeft", "GamepadLStickRight", "GamepadLStickUp", "GamepadLStickDown",
    "GamepadRStickLeft", "GamepadRStickRight", "GamepadRStickUp", "GamepadRStickDown",
    "MouseLeft", "MouseRight", "MouseMiddle", "MouseX1", "MouseX2", "MouseWheelX", "MouseWheelY",
    "ModCtrl", "ModShift", "ModAlt", "ModSuper"
};
const char* GetKeyName(FloodKey key)
{
    if (key == FloodGuiKey_None)
        return "None";
    return GKeyNames[key - 512];
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
    return FloodVector2(static_cast<int>(strlen(text)) * (size + spacing), size* FONT_HEIGHT_MODIFIER);
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
    FloodGui::Context.FrameData.FrameStage = FloodRenderStage_FrameRenderEnd;
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
            
            // Why was the time??
             backend_data->pd3dDevice->SetTexture(0, material.texture);

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

    FloodGui::Context.FrameData.tFrameEnd = std::chrono::system_clock::now();
    FloodGui::Context.FrameData.tElaspedFrame = FloodGui::Context.FrameData.tFrameEnd - FloodGui::Context.FrameData.tFrameStart;
}


//                    //
// Namespace FloodGui //
//                    //
void FloodGui::NewFrame() {
    FloodGui::Context.FrameData.FrameStage = FloodRenderStage_FrameStart;
    FloodGui::Context.FrameData.FrameCount++;
    FloodGui::Context.FrameData.tFrameStart = std::chrono::system_clock::now();
    ChangeCursor(IDC_ARROW);
    // Make sure we clear the global draw list
    //
    if(FloodGui::Context.DrawData->Foreground)
    FloodGui::Context.DrawData->Foreground->Clear();
    if(FloodGui::Context.DrawData->Background)
    FloodGui::Context.DrawData->Background->Clear();
    FloodGui::Context.DrawData->DrawLists.clear();
}

void FloodGui::EndFrame() {
    FloodGui::Context.FrameData.FrameStage = FloodRenderStage_FrameEnd;
}

void FloodGui::Render()
{
    // Go over DrawList and validate things
    FloodGui::Context.FrameData.FrameStage = FloodRenderStage_FrameRenderStart;
    FloodDrawData* drawData = FloodGui::Context.DrawData;
    // Organize Global Draw Data
    drawData->DrawLists.push_back(FloodGui::Context.DrawData->Foreground);
    for (const auto& [name, window] : SortWindows(FloodGui::Context.Windows.size())) { if(window->shouldDraw)drawData->DrawLists.insert(drawData->DrawLists.end()-1, window->GetDrawList()); }
    drawData->DrawLists.insert(drawData->DrawLists.begin(), FloodGui::Context.DrawData->Background);
}

void FloodGui::BeginWindow(const char* windowName, bool shouldDraw)
{
    static FloodContext& context = FloodGui::Context;
    bool windowExists = context.Windows.find(windowName) != context.Windows.end();
    FloodWindow* window = nullptr;
    // Create a new window if it doesnt exist
    if (!windowExists)
    {
        window = new FloodWindow(windowName, { 600, 600 });
        window->SetZIndex(context.ActiveDrawingWindowZIndex++);
        context.Windows[windowName] = window;
    }
    else
        window = context.Windows[windowName];
    context.ActiveDrawingWindow = window;
    window->shouldDraw = shouldDraw;
    FloodDrawList* DrawList = window->GetDrawList(); window->Clear();
    // Window Interaction
    {
        const uint64_t hash = FloodHash(window, "").hash();
        FloodWindow* hovered = get_window_hovering();
        static std::unordered_map < uint64_t, bool> PrevMouseClick;
        if (PrevMouseClick.find(hash) == PrevMouseClick.end())
            PrevMouseClick[hash] = false;

        bool onmouseUp = PrevMouseClick[hash] && !FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];
        bool onmouseDown = !PrevMouseClick[hash] && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];

        if (window == hovered && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
            FocusNewWindow(window);

        // Window Resizing
        {
        bool& is_selected1 = window->WindowResizingActive();
        static std::unordered_map<uint64_t, FloodVector2> relative_dst1{};
        if (relative_dst1.find(hash) == relative_dst1.end())
            relative_dst1[hash] = FloodVector2();
        
            if (window->WindowIsActive() && (window->WindowIsActive() || is_selected1) && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])
            {
                if (FindPoint(window->GetBoundingContentMax() - 20.f, window->GetBoundingContentMax(), FloodGui::Context.IO.mouse_pos) || is_selected1) {
                    const FloodVector2& dst = FloodGui::Context.IO.mouse_pos - relative_dst1[hash];
                    if (relative_dst1[hash].x != 0 && relative_dst1[hash].y != 0)
                    {
                        is_selected1 = true;
                        // Here we calculate the new size of the window 
                        // based on movement and previous sizing
                        FloodVector2 new_size = (window->GetBoundingContentMax() - window->GetBoundingContentMin()) + dst;
                        // Lets have a minimum size for our window
                        // it will not behave nicely otherwise
                        if (new_size.x > 500 && new_size.y > 500) {
                            window->ResizeWindow(new_size);
                        }
                        else {
                            is_selected1 = false;
                        }
                    }
                    else {
                        is_selected1 = false;
                    }
                    relative_dst1[hash] = FloodGui::Context.IO.mouse_pos;
                }
            }
            if (onmouseUp)
            {
                relative_dst1[hash] = FloodVector2(0, 0);
                is_selected1 = false;
            }
        }

        // Window draggings
        {
            //if the user previously clicked in the title box and the mouse is still down
            bool& is_selected2 = window->WindowDraggingActive();
            static std::unordered_map<uint64_t, FloodVector2> relative_dst2;
            if (relative_dst2.find(hash) == relative_dst2.end())
                relative_dst2[hash] = FloodVector2();
            if (window->WindowIsActive() && onmouseDown)
            {
                // Get relative distance to top left on first click
                relative_dst2[hash] = window->GetFullBoundingMin() - FloodGui::Context.IO.mouse_pos;
            }
            if ((is_selected2 == false && (!window->WindowIsActive() && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse])) || onmouseUp)
            {
                relative_dst2[hash].x = 1; // invalidate
            }
            if (relative_dst2[hash].x <= 0 && (window->WindowIsActive() || is_selected2) && FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse]) {
                // Window Dragging
                if (FindPoint(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), FloodGui::Context.IO.mouse_pos) || is_selected2) {
                    const FloodVector2& dst = FloodGui::Context.IO.mouse_pos - FloodGui::Context.IO.pmouse_pos;
                    window->MoveWindow(relative_dst2[hash] + dst);
                    is_selected2 = true;
                }
                else {
                    //set select to false
                    is_selected2 = false;
                }
            }
            if (!FindPoint(window->GetBoundingTitleMin(), window->GetBoundingTitleMax(), FloodGui::Context.IO.mouse_pos))
                is_selected2 = false;
        }

        PrevMouseClick[hash] = FloodGui::Context.IO.MouseInput[FloodGuiButton_LeftMouse];
    }
    static const int font_size = 7;
    static const int spacing = 5;
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
    std::string sid = split(id, "##")[0];
    FloodWindow* win = Context.ActiveDrawingWindow;
    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };
    FloodVector2 textSize = CalcTextSize(sid.c_str(), 7, 5);
    FloodVector2 innerPadding = FloodVector2(textSize.x/5.f, textSize.y);
    FloodVector2 boxMin = win->GetBoundingContentMin() + offset;
    FloodVector2 boxMax = boxMin + textSize + innerPadding;
    const bool isHovering = win->WindowIsActive() && FindPoint(boxMin, boxMax, Context.IO.mouse_pos);
    static std::unordered_map<uint64_t, bool> pass{};
    const uint64_t hash = FloodHash(win, id).hash();
    if (pass.find(hash) == pass.end())
        pass[hash] = true;

    win->GetDrawList()->AddRectFilled(boxMin, boxMax, isHovering && pass[hash] ? Context.colors[FloodGuiCol_ButtonHovered] : Context.colors[FloodGuiCol_Button]);
    win->GetDrawList()->AddText(sid.c_str(), boxMin + FloodVector2(0, textSize.y) + innerPadding/3.f, Context.colors[FloodGuiCol_Text], 7, 5);
    win->content.push_back({ id, boxMax.y - boxMin.y });
    if (pass[hash] && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHovering)
    {
        pass[hash] = false;
        return true;
    }
    else if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass[hash] = true;
    else if(Context.IO.MouseInput[FloodGuiButton_LeftMouse])
        pass[hash] = false;
        
    return false;
}

bool FloodGui::Color4Slider(const char* id, uint8_t* col4) {
    std::string sid = split(id, "##")[0];
    FloodWindow* win = Context.ActiveDrawingWindow;
    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };
    FloodVector2 text_size1 = CalcTextSize(sid.c_str(), 7, 5);
    const FloodVector2 text_padding = { 7 , text_size1.y / 2.5f };
    const float length = (win->GetBoundingContentMax().x - (win->GetBoundingContentMin() + offset).x - offset.x - text_size1.x - text_padding.x - (5.f*4)) / 4;
    float endX = 0;
    static std::unordered_map<uint64_t, bool> pass{};
    bool ret = false;
    for (uint16_t i = 0; i < 4; i++)
    {
        // Here we calc the outer container for the slider
        const float xPadding = (i > 0 ? 5.f : 0.f);
        FloodVector2 minOuter = win->GetBoundingContentMin() + offset+ FloodVector2((length * i )+xPadding*i, 0);
        FloodVector2 maxOuter = minOuter + FloodVector2(length, 25);
        endX = maxOuter.x;


        uint8_t& value = col4[i];
        const int sections = abs(0 - 255) + 1; // Last mf
        const int section = (0 < 0 ? abs(255) : -1) + value;

        // Here we calc the inner dragger
        FloodVector2 innerSize = FloodVector2(length / sections, 25.f);

        const bool isHoveringOuter = win->WindowIsActive() && FindPoint(minOuter, maxOuter, Context.IO.mouse_pos);
        const uint64_t hash = FloodHash(win, (std::string(id) + std::to_string(i)).c_str()).hash();
        if (pass.find(hash) == pass.end())
            pass[hash] = false;
        std::string sValue = (i == 0 ? "R " : (i==1 ? "G " : (i==2 ? "B " : (i==3 ? "A " : "")))) + std::to_string(value);
        
        
        win->GetDrawList()->AddRectFilled(minOuter, maxOuter, Context.colors[FloodGuiCol_SliderBkg]);

        FloodVector2 text_size = CalcTextSize(sValue.c_str(), 7, 3);
        const FloodVector2& text_pos = (minOuter + FloodVector2((length / 2.f) - (text_size.x / 2.f), innerSize.y - innerSize.y / 2.f + text_size.y / 2.f));
        win->GetDrawList()->AddText(sValue.c_str(), text_pos, FloodGui::Context.colors[FloodGuiCol_Text], 7, 3);
    
        // This is the backend
    // We check if we have clicked on the dragger
        if (!pass[hash] && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHoveringOuter)
        {
            pass[hash] = true;
            ret = false;
        }
        // We make sure that we are still holding left mb
        else if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse] || !FindPoint(win->GetBoundingContentMin(), win->GetBoundingContentMax(), Context.IO.mouse_pos)) {
            pass[hash] = false;
            ret = false;
        }
        // Now we calc the dragger's new position
        // and val
        else if (pass[hash])
        {
            // This will contain the predicted value
            // hopefully no one will ever put this guy as the max
            int predValue = 256;

            // Here we loop through all sections
            // there must be a better way to calculate this without a loop
            for (int s = 0; s <= 255; s++)
            {
                const int psection = (0 < 0 ? abs(0) : -1) + s;
                const float minPred = minOuter.x + (psection * innerSize.x);
                const float maxPred = minPred + innerSize.x;
                // Find point but for X only
                if (!(Context.IO.mouse_pos.x > minPred && Context.IO.mouse_pos.x < maxPred))
                    continue;
                predValue = s;
                break;
            }

            // Here we make sure that the pred
            // is within range
            if (predValue > 255 || predValue < 0)
                ret = false;
            // Store prev state
            bool ret1 = predValue != value;
            // Now we change
            if (predValue != 256)
                value = predValue;
            else
            {
                if (Context.IO.mouse_pos.x < maxOuter.x)
                    value = 0;
                else
                    value = 255;
            }
            // return prev state
            ret = ret1;
        }
    }
    win->GetDrawList()->AddText(sid.c_str(), (win->GetBoundingContentMin() + offset + FloodVector2(endX- win->GetBoundingContentMin().x, 25)) + FloodVector2(text_padding.x, -text_padding.y * 1.6f), FloodGui::Context.colors[FloodGuiCol_Text], 7, 5);
    win->content.push_back({ id, 25.f});
    return ret;
}

bool FloodGui::Color3Slider(const char* id, uint8_t* col3) {
    std::string sid = split(id, "##")[0];
    FloodWindow* win = Context.ActiveDrawingWindow;
    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };
    FloodVector2 text_size1 = CalcTextSize(sid.c_str(), 7, 5);
    const FloodVector2 text_padding = { 7 , text_size1.y / 2.5f };
    const float length = (win->GetBoundingContentMax().x - (win->GetBoundingContentMin() + offset).x - offset.x - text_size1.x - text_padding.x - (5.f * 3)) / 3;
    float endX = 0;
    static std::unordered_map<uint64_t, bool> pass{};
    bool ret = false;
    for (uint16_t i = 0; i < 3; i++)
    {
        // Here we calc the outer container for the slider
        const float xPadding = (i > 0 ? 5.f : 0.f);
        FloodVector2 minOuter = win->GetBoundingContentMin() + offset + FloodVector2((length * i) + xPadding * i, 0);
        FloodVector2 maxOuter = minOuter + FloodVector2(length, 25);
        endX = maxOuter.x;


        uint8_t& value = col3[i];
        const int sections = abs(0 - 255) + 1; // Last mf
        const int section = (0 < 0 ? abs(255) : -1) + value;

        // Here we calc the inner dragger
        FloodVector2 innerSize = FloodVector2(length / sections, 25.f);

        const bool isHoveringOuter = win->WindowIsActive() && FindPoint(minOuter, maxOuter, Context.IO.mouse_pos);
        const uint64_t hash = FloodHash(win, (std::string(id) + std::to_string(i)).c_str()).hash();
        if (pass.find(hash) == pass.end())
            pass[hash] = false;
        std::string sValue = (i == 0 ? "R " : (i == 1 ? "G " : (i == 2 ? "B " : (i == 3 ? "A " : "")))) + std::to_string(value);


        win->GetDrawList()->AddRectFilled(minOuter, maxOuter, Context.colors[FloodGuiCol_SliderBkg]);

        FloodVector2 text_size = CalcTextSize(sValue.c_str(), 7, 3);
        const FloodVector2& text_pos = (minOuter + FloodVector2((length / 2.f) - (text_size.x / 2.f), innerSize.y - innerSize.y / 2.f + text_size.y / 2.f));
        win->GetDrawList()->AddText(sValue.c_str(), text_pos, FloodGui::Context.colors[FloodGuiCol_Text], 7, 3);

        // This is the backend
    // We check if we have clicked on the dragger
        if (!pass[hash] && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHoveringOuter)
        {
            pass[hash] = true;
            ret = false;
        }
        // We make sure that we are still holding left mb
        else if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse] || !FindPoint(win->GetBoundingContentMin(), win->GetBoundingContentMax(), Context.IO.mouse_pos)) {
            pass[hash] = false;
            ret = false;
        }
        // Now we calc the dragger's new position
        // and val
        else if (pass[hash])
        {
            // This will contain the predicted value
            // hopefully no one will ever put this guy as the max
            int predValue = 256;

            // Here we loop through all sections
            // there must be a better way to calculate this without a loop
            for (int s = 0; s <= 255; s++)
            {
                const int psection = (0 < 0 ? abs(0) : -1) + s;
                const float minPred = minOuter.x + (psection * innerSize.x);
                const float maxPred = minPred + innerSize.x;
                // Find point but for X only
                if (!(Context.IO.mouse_pos.x > minPred && Context.IO.mouse_pos.x < maxPred))
                    continue;
                predValue = s;
                break;
            }

            // Here we make sure that the pred
            // is within range
            if (predValue > 255 || predValue < 0)
                ret = false;
            // Store prev state
            bool ret1 = predValue != value;
            // Now we change
            if (predValue != 256)
                value = predValue;
            else
            {
                if (Context.IO.mouse_pos.x < maxOuter.x)
                    value = 0;
                else
                    value = 255;
            }
            // return prev state
            ret = ret1;
        }
    }
    win->GetDrawList()->AddText(sid.c_str(), (win->GetBoundingContentMin() + offset + FloodVector2(endX - win->GetBoundingContentMin().x, 25)) + FloodVector2(text_padding.x, -text_padding.y * 1.6f), FloodGui::Context.colors[FloodGuiCol_Text], 7, 5);

    win->content.push_back({ id, 25.f });
    return ret;
}

bool FloodGui::Checkbox(const char* id, bool* val) {
    std::string sid = split(id, "##")[0];
    FloodWindow* win = Context.ActiveDrawingWindow;
    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };
    FloodVector2 textSize = CalcTextSize(sid.c_str(), 7, 5);

    FloodVector2 boxMin = win->GetBoundingContentMin() + offset;
    FloodVector2 boxMax = boxMin + FloodVector2(20, 20);

    const bool isHovering = win->WindowIsActive() && FindPoint(boxMin, boxMax, Context.IO.mouse_pos);
    const bool isToggled = (val && *val);
    static bool pass = true;

    win->GetDrawList()->AddRectFilled(boxMin, boxMax, isHovering && pass ? Context.colors[FloodGuiCol_ButtonHovered] : Context.colors[FloodGuiCol_Button]);
    if (isToggled)
        win->GetDrawList()->AddRectFilled(boxMin+ (FloodVector2(20, 20)/5.f), boxMax-(FloodVector2(20, 20) / 5.f), Context.colors[FloodGuiCol_CheckboxActivated]);
    win->GetDrawList()->AddText(sid.c_str(), boxMax + FloodVector2{7, -textSize.y/2.f}- (FloodVector2(20, 20) / 7.5f), Context.colors[FloodGuiCol_Text],7, 5);
    
    win->content.push_back({ id, boxMax.y - boxMin.y });

    if (pass && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHovering)
    {
        if (val)
            *val = !*val;
        pass = false;
        return true;
    }
    
    if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse] ) {
        pass = true;
    }

    if (Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHovering)
    {
        pass = false;
    }

    return false;
}

bool FloodGui::IntSlider(const char* id, int* val, int min, int max) {
    std::string sid = split(id, "##")[0];
    FloodWindow* win = Context.ActiveDrawingWindow;
    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };
    
    // Here we calc the outer container for the slider
    FloodVector2 minOuter = win->GetBoundingContentMin() + offset;
     FloodVector2 text_size1 = CalcTextSize(sid.c_str(), 7, 5);
     const FloodVector2 text_padding = { 7 , text_size1 .y/2.5f};
    const float length = win->GetBoundingContentMax().x - minOuter.x - offset.x - text_size1.x - text_padding.x;
    FloodVector2 maxOuter = minOuter + FloodVector2(length, 25);

    // These will help with calculating the inner dragger
    // and the backend
    int& value = *val;
    const int sections = abs(min - max) +  1 ; // Last mf
    const int section = (min < 0 ? abs(min) : -1) + value;

    // Here we calc the inner dragger
    FloodVector2 innerSize = FloodVector2(length /sections, 25.f);
    FloodVector2 minInner = minOuter +FloodVector2(section * innerSize.x, 0);
    FloodVector2 maxInner= minInner + FloodVector2(innerSize.x, 25);

    // These help us style the inner and also 
    // help with the backend
    const bool isHoveringOuter = win->WindowIsActive() && FindPoint(minOuter, maxOuter, Context.IO.mouse_pos);
    const bool isHoveringInner = win->WindowIsActive() && FindPoint(minInner, maxInner, Context.IO.mouse_pos);
    static std::unordered_map<uint64_t, bool> pass{};
    const uint64_t hash = FloodHash(win, id).hash();
    if (pass.find(hash) == pass.end())
        pass[hash] = false;
    
    // Str the value so we can display it
    std::string sValue = std::to_string(value);

    // Here we draw everything
    win->GetDrawList()->AddRectFilled(minOuter, maxOuter, Context.colors[FloodGuiCol_SliderBkg]);
    win->GetDrawList()->AddRectFilled(minInner, maxInner, (isHoveringInner || pass[hash])? Context.colors[FloodGuiCol_SliderSliderHover] : Context.colors[FloodGuiCol_SliderSlider]);
    win->GetDrawList()->AddText(sid.c_str(), maxOuter + FloodVector2(text_padding.x, -text_padding.y*1.6f), FloodGui::Context.colors[FloodGuiCol_Text], 7, 5);
        // This is where we draw the text
        FloodVector2 text_size = CalcTextSize(sValue.c_str(), 7, 3);
        const FloodVector2& text_pos = (minOuter + FloodVector2((length / 2.f) - (text_size.x / 2.f), innerSize.y- innerSize.y/2.f + text_size.y/2.f));
        win->GetDrawList()->AddText(sValue.c_str(), text_pos, FloodGui::Context.colors[FloodGuiCol_Text], 7, 3);

    win->content.push_back({ id, maxOuter.y - minOuter.y });

    // This is the backend
    // We check if we have clicked on the dragger
    if (!pass[hash] && Context.IO.MouseInput[FloodGuiButton_LeftMouse] && isHoveringOuter)
    {
        pass[hash] = true;
        return false;
    }
    // We make sure that we are still holding left mb
    if (!Context.IO.MouseInput[FloodGuiButton_LeftMouse] || !FindPoint(win->GetBoundingContentMin(), win->GetBoundingContentMax(), Context.IO.mouse_pos)) {
        pass[hash] = false;
        return false;
    }
    // Now we calc the dragger's new position
    // and val
    if (pass[hash])
    {
        // This will contain the predicted value
        // hopefully no one will ever put this guy as the max
        int predValue = MAXINT32;

        // Here we loop through all sections
        // there must be a better way to calculate this without a loop
        for (int s = min; s <= max; s++)
        {
            const int psection = (min < 0 ? abs(min) : -1) + s;
            const float minPred = minOuter.x + (psection * innerSize.x);
            const float maxPred = minPred + innerSize.x;
            // Find point but for X only
            if (!(Context.IO.mouse_pos.x > minPred && Context.IO.mouse_pos.x < maxPred))
                continue;
            predValue = s;
            break;
        }

        // Here we make sure that the pred
        // is within range
        if (predValue > max || predValue < min)
            return false;
        // Store prev state
        bool ret = predValue != value;
        // Now we change
        value = predValue;
        // return prev state
        return ret;
    }
    return false;
}

bool FloodGui::Hotkey(const char* id, uint16_t key, bool global)
{
    FloodWindow* win = Context.ActiveDrawingWindow;
    struct custom {
        FloodKey k;
        long long tClick{};
    };
    static std::unordered_map<uint64_t, custom> keys{};
    const uint64_t hash = FloodHash(win, id).hash();
    if (keys.find(hash) == keys.end())
        keys[hash].k = (FloodKey)key;

    static const FloodVector2 coffset{ 15, 5 };
    FloodVector2 offset{ 15, 5 };
    for (int i = 0; i < win->CurrentContentCount(); i++)
        offset = offset + FloodVector2{ 0, win->content[i].second + coffset.y };

    FloodVector2 buttonMax;
    std::string sid1 = (keys[hash].k == FloodGuiKey_None ? ("<...>##" + std::string(id)) : (" " + std::string(GetKeyName((FloodKey)keys[hash].k)) + "##" + std::string(id)));
    {
        std::string sid11 = split(sid1, "##")[0];
        FloodVector2 textSize = CalcTextSize(sid11.c_str(), 7, 5);
        FloodVector2 innerPadding = FloodVector2(textSize.x / 5.f, textSize.y);
        FloodVector2 boxMin = win->GetBoundingContentMin() + offset;
        buttonMax = boxMin + textSize + innerPadding;
    }

    std::string sid2 = split(id, "##")[0];
    
    FloodVector2 textSize = CalcTextSize(sid2.c_str(), 7, 5);
    if (FloodGui::Button(sid1.c_str()))
    {
        keys[hash].k = FloodGuiKey_None;
    }

    win->GetDrawList()->AddText(sid2.c_str(), buttonMax + FloodVector2{ 15, -textSize.y / 2.f } - ((buttonMax- (win->GetBoundingContentMin() + offset))/ 7.5f), Context.colors[FloodGuiCol_Text], 7, 5);
    if (keys[hash].k == FloodGuiKey_None)
    {
        for(const auto& k : FloodGui::Context.IO.KeyboardInputs)
            if (k.second.raw_down) { keys[hash].k = (FloodKey)k.first;         keys[hash].tClick = FloodGui::Context.IO.KeyboardInputs[keys[hash].k].tClick + 75; /* Add another 75ms delay after the press to intalize*/break; }
    }
    else if (FloodGui::Context.IO.KeyboardInputs[keys[hash].k].raw_down && FloodGui::Context.IO.KeyboardInputs[keys[hash].k].tClick > keys[hash].tClick && (global || (!global && win->shouldDraw && win->WindowIsActive()))) {
        keys[hash].tClick = FloodGui::Context.IO.KeyboardInputs[keys[hash].k].tClick;
        return true;
    }
    return false;
}

//                  //
//  Flood Draw List //
//                  //
void FloodDrawList::AddRectFilled(const FloodVector2& min, const FloodVector2& max, FloodColor col, LPDIRECT3DTEXTURE9 texture)
{
    static const int index_count = 6, vertex_count = 4;
    Elements.push_back(FloodDrawMaterial{ { min, {max.x,min.y}, max, { min.x, max.y } },  col, 0.f, index_count, vertex_count, texture });
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
void FloodDrawList::AddTriangleFilled(const FloodVector2& a, const FloodVector2& b, const FloodVector2& c, FloodColor col, LPDIRECT3DTEXTURE9 texture) {
    static const int index_count = 4, vertex_count = 3;
    Elements.push_back(FloodDrawMaterial{ {a,b,c},  col, 0.f, index_count, vertex_count, texture });
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
    const float height = font_size * FONT_HEIGHT_MODIFIER;
    const float half_height = height / 1.6f;

    switch (text)
    {
    case 'a': {
        const FloodVector2 Ltop = FloodVector2(position) + FloodVector2(width, -half_height);
        AddLine(Ltop, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(Ltop, FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(position, FloodVector2(position) + FloodVector2(0, -(half_height/1.8f)), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(0, -(half_height / 1.8f)), FloodVector2(position) + FloodVector2(width, -(half_height / 1.8f)), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top, position, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, -half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, -half_height), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'C':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position)+FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'd':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(width, -height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, -half_height), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        break;
    }
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
    {
        FloodVector2 Ltop = FloodVector2(position) + FloodVector2(0, -half_height);
        AddLine(Ltop, position, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(Ltop, Ltop+ FloodVector2(width, 0), col, 1);

        AddLine(FloodVector2(position) + FloodVector2(0, -(half_height / 1.8f)), FloodVector2(position) + FloodVector2(width, -(half_height / 1.8f)), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, -(half_height / 1.8f)), Ltop + FloodVector2(width, 0), col, 1);

        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(width/2.f, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(FloodVector2(position) + FloodVector2(width/ 2.f, 0), top, col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);

        AddLine(top, top + FloodVector2(width*.7f, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, half_height), col, 1);

        AddLine(FloodVector2(position) + FloodVector2(width, half_height), FloodVector2(position) + FloodVector2(0, half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(0, half_height), FloodVector2(position) + FloodVector2(0, half_height*.5), col, 1);

        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height/2.f);
        AddLine(position, top, col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), middle + FloodVector2(width, 0), col, 1);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -half_height), col, 1);

        AddLine(vmiddle + FloodVector2(0, -half_height * 1.3f), vmiddle + FloodVector2(0, -half_height*1.5f), col, 1.5f);

        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(width / 2.f, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle + FloodVector2(0, half_height), col, 1);

        AddLine(vmiddle + FloodVector2(0, -half_height * 1.3f), vmiddle + FloodVector2(0, -half_height * 1.5f), col, 1.5f);

        AddLine(FloodVector2(position) + FloodVector2(width / 2.f, half_height), FloodVector2(position) + FloodVector2(0, half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(0, half_height), FloodVector2(position) + FloodVector2(0, half_height * .5), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height*1.4f);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -half_height / 2.f);
        AddLine(position, top, col, 1);
        AddLine(middle, top + FloodVector2(width, half_height * .4f), col, 1);
        AddLine(middle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        break;
    }
    case 'L':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'm':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -half_height / 2.5f);
        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width / 2.f, 0), vmiddle, col, 1);
        AddLine(top + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);

        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'N':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(position, top, col, 1);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'o':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        AddLine(FloodVector2(position) + FloodVector2(0, -half_height), FloodVector2(position) + FloodVector2(0, height * .6f), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, -half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, -half_height), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(width, -half_height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, height * .6f), col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, -half_height), FloodVector2(position) + FloodVector2(0, -half_height), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(top, top + FloodVector2(width*.75f, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -half_height / 2.f);

        AddLine(middle, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);

        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        AddLine(middle + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -(half_height*1.36f) / 1.6f);

        AddLine(vmiddle, vmiddle + FloodVector2(0, -(half_height * 1.36f)), col, 1);
        AddLine(middle, FloodVector2(middle) + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'T':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -height), col, 1);
        AddLine(top, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'u':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'U':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);

        AddLine(position, top, col, 1);
        AddLine(position, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'v':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'V':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        break;
    }
    case 'w':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, -half_height);
        AddLine(position, top, col, 1);
        AddLine(position, vmiddle, col, 1);
        AddLine(vmiddle, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) + FloodVector2(width, 0), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position), col, 1);
        break;
    }
    case 'X':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        AddLine(top, FloodVector2(position) + FloodVector2(width, 0), col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position) , col, 1);
        break;
    }
    case 'y':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(top, vmiddle, col, 1);
        AddLine(vmiddle, top + FloodVector2(width, 0), col, 1);
        AddLine(vmiddle, vmiddle - FloodVector2(width / 2.2f, -height*.6f), col, 1);
        break;
    }
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
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -half_height);
        AddLine(top + FloodVector2(width, 0), top, col, 1);
        AddLine(top + FloodVector2(width, 0), FloodVector2(position), col, 1);
        AddLine(FloodVector2(position) + FloodVector2(width, 0), position, col, 1);
        break;
    }
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
    case '-':
    {
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        AddLine(middle, middle + FloodVector2(width, 0), col, 1);
        break;
    }
    case '.':
    {
        FloodVector2 vmiddle = FloodVector2(position) + FloodVector2(width / 2.f, 0);
        AddLine(vmiddle, vmiddle + FloodVector2(0, -1.5f), col, 1.5f);
        break;
    }
    case '<':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(width, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(0, -height / 2.f);
        FloodVector2 bottom = FloodVector2(position) + FloodVector2(width, 0);
        AddLine(top, middle, col);
        AddLine(middle, bottom, col);
        break;
    }
    case '>':
    {
        FloodVector2 top = FloodVector2(position) + FloodVector2(0, -height);
        FloodVector2 middle = FloodVector2(position) + FloodVector2(width, -height / 2.f);
        FloodVector2 bottom = FloodVector2(position) + FloodVector2(0, 0);
        AddLine(top, middle, col);
        AddLine(middle, bottom, col);
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