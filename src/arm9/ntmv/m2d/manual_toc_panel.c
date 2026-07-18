#include "ntmv/m2d/manual_toc_panel.h"
#include "game/manual_viewer_factories.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/manual_button.h"
#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/m2d/ui_static.h"
#include "ntmv/ui_renderer.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct NtmvM2dManualTocButtonDefinition {
    NtmvM2dPoint position;
    NtmvM2dSize size;
    uint16_t animation_sequence;
    int16_t alternate_animation_sequence;
    uint16_t text_definition_index;
} NtmvM2dManualTocButtonDefinition;

typedef struct NtmvM2dManualTocTextDefinition {
    uint16_t animation_sequence;
    NtmvM2dPoint tile_origin;
    NtmvM2dSize canvas_size;
} NtmvM2dManualTocTextDefinition;

typedef struct NtmvM2dManualTocCharCanvas NtmvM2dManualTocCharCanvas;

typedef struct NtmvM2dManualTocCharCanvasOps {
    void (*unknown_00)(NtmvM2dManualTocCharCanvas *canvas);
    void (*clear)(NtmvM2dManualTocCharCanvas *canvas);
} NtmvM2dManualTocCharCanvasOps;

struct NtmvM2dManualTocCharCanvas {
    uint8_t unknown_00[0x14];
    const NtmvM2dManualTocCharCanvasOps *ops;
};

typedef struct NtmvM2dManualTocTextCanvasState {
    NtmvM2dManualTocCharCanvas *canvas;
    const NtmvM2dItemsFont *font;
    int32_t character_spacing;
    uint32_t reserved_0c;
} NtmvM2dManualTocTextCanvasState;

typedef struct NtmvM2dManualTocObjTextCanvas {
    NtmvM2dManualTocCharCanvas canvas;
    NtmvM2dManualTocTextCanvasState text_canvas;
} NtmvM2dManualTocObjTextCanvas;

typedef struct NtmvM2dManualTocTextDirection {
    int8_t x;
    int8_t y;
} NtmvM2dManualTocTextDirection;

typedef char NtmvM2dManualTocButtonDefinitionSizeCheck[
    sizeof(NtmvM2dManualTocButtonDefinition) == 0x0e ? 1 : -1];
typedef char NtmvM2dManualTocTextDefinitionSizeCheck[
    sizeof(NtmvM2dManualTocTextDefinition) == 0x0a ? 1 : -1];
#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dManualTocCharCanvasTargetSizeCheck[
    sizeof(NtmvM2dManualTocCharCanvas) == 0x18 ? 1 : -1];
typedef char NtmvM2dManualTocTextCanvasTargetSizeCheck[
    sizeof(NtmvM2dManualTocTextCanvasState) == 0x10 ? 1 : -1];
typedef char NtmvM2dManualTocObjTextCanvasTargetSizeCheck[
    sizeof(NtmvM2dManualTocObjTextCanvas) == 0x28 ? 1 : -1];
#endif

extern void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size);
extern void NNS_G2dCharCanvasInitForOBJ1D(
    NtmvM2dManualTocCharCanvas *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth);
extern void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    NtmvM2dManualTocTextCanvasState *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    NtmvM2dManualTocTextDirection direction);

/* ManualToc helpers outside the current 0x020ba948 recovery boundary. */
int32_t NtmvM2dManualTocPanel_MovePrevious_020bb450(
    NtmvM2dManualTocPanel *panel);
int32_t NtmvM2dManualTocPanel_MoveNext_020bb56c(
    NtmvM2dManualTocPanel *panel);
bool NtmvM2dManualTocPanel_ToggleGuideState_020bb660(
    NtmvM2dManualTocPanel *panel);
bool NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
    NtmvM2dManualTocPanel *panel);
void NtmvM2dManualTocPanel_ActivateControl1_020bb70c(
    NtmvM2dManualTocPanel *panel);
static const NtmvM2dManualTocTextDefinition sTextDefinitions[3] = {
    {20, {0, 39}, {12, 2}},
    {21, {24, 39}, {16, 2}},
    {22, {24, 40}, {16, 2}},
};

static const NtmvM2dItemsPanelDefinition sItemsDefinitions[2] = {
    {{0, -36}, 26, 25, 7, -30, 186, 5, 1},
    {{137, -59}, 34, -1, 13, 31, 186, 7, 2},
};

static const NtmvM2dManualTocButtonDefinition sButtonDefinitions[2][5] = {
    {
        {{-118, 1}, {18, 18}, 37, -1, UINT16_MAX},
        {{-60, 1}, {93, 18}, 27, -1, 0},
        {{6, 1}, {22, 18}, 23, 24, UINT16_MAX},
        {{-56, 0}, {64, 18}, 28, -1, 1},
        {{56, 0}, {64, 18}, 29, -1, 2},
    },
    {
        {{-118, 1}, {18, 18}, 37, -1, UINT16_MAX},
        {{-60, 1}, {93, 18}, 27, -1, 0},
        {{6, 1}, {22, 18}, 23, 24, UINT16_MAX},
        {{-65, 0}, {120, 18}, 30, -1, 1},
        {{65, 0}, {120, 18}, 31, -1, 2},
    },
};

const NtmvM2dManualTocPanelVTable gNtmvM2dManualTocPanelVTable = {
    NtmvM2dManualTocPanel_Destroy, /* slot 0: 0x020ba80c */
    NtmvM2dManualTocPanel_Render,  /* slot 1: 0x020ba84c */
    (void (*)(NtmvM2dManualTocPanel *, const NtmvM2dPoint *))
        NtmvM2dUIElement_SetPosition, /* slot 2: 0x020b7e48 */
    (void (*)(NtmvM2dManualTocPanel *, const NtmvM2dSize *))
        NtmvM2dUIElement_SetSize, /* slot 3: 0x020b7e5c */
    (void (*)(NtmvM2dManualTocPanel *, bool))
        NtmvM2dUIElement_SetVisible, /* slot 4: 0x020b7e70 */
};

/* 0x020ba004 */
NtmvM2dManualTocPanel *NtmvM2dManualTocPanel_Construct(
    NtmvM2dManualTocPanel *panel)
{
    uint32_t index;

    NtmvM2dUIElement_Init(&panel->base);
    panel->base.vtable =
        (const NtmvM2dUIElementVTable *)&gNtmvM2dManualTocPanelVTable;
    panel->toc_data = NULL;
    panel->render_resources = NULL;
    panel->feedback_context = NULL;
    NtmvM2dUIPanel_ConstructComplete(&panel->content_panel);
    NtmvM2dUIPanel_ConstructComplete(&panel->footer_panel);

    panel->body_panel = NULL;
    for (index = 0; index < 2; ++index) {
        panel->item_panels[index] = NULL;
    }
    for (index = 0; index < 9; ++index) {
        panel->controls[index] = NULL;
    }
    panel->selection_marker = NULL;
    panel->mode_markers[0] = NULL;
    panel->mode_markers[1] = NULL;
    panel->scroll_button = NULL;

    panel->active_control_index = -1;
    panel->item_panel_target_x = 0x89;
    panel->mode = NTMV_M2D_MANUAL_TOC_MODE_SPLIT;
    panel->target_window_x = -0x30;
    for (index = 0; index < 3; ++index) {
        panel->window_origins[index].x = 0;
        panel->window_origins[index].y = 0;
    }
    panel->state_ac = 0;
    panel->secondary_state_active = 0;
    panel->state_ae = 0;
    return panel;
}

static NtmvM2dManualTocTextDirection GetTextDirection(
    const NtmvM2dItemsFont *font)
{
    static const NtmvM2dManualTocTextDirection directions[8] = {
        {1, 0}, {0, 1}, {0, 1}, {-1, 0},
        {-1, 0}, {0, -1}, {0, -1}, {1, 0},
    };
    uint8_t orientation = font->resource->metadata->orientation;

    if (orientation >= 8) {
        return (NtmvM2dManualTocTextDirection){0, 0};
    }
    return directions[orientation];
}

static NtmvM2dManualTocObjTextCanvas *InitObjTextCanvas(
    NtmvM2dManualTocObjTextCanvas *output,
    const NtmvM2dPoint *tile_origin,
    const NtmvM2dSize *canvas_size,
    const NtmvM2dItemsFont *font)
{
    void *vram = (void *)(uintptr_t)(
        UINT32_C(0x06400000) +
        (uint32_t)(uint16_t)tile_origin->x * UINT32_C(0x20) +
        (uint32_t)(uint16_t)tile_origin->y * UINT32_C(0x400));

    NNS_G2dCharCanvasInitForOBJ1D(
        &output->canvas,
        vram,
        canvas_size->width,
        canvas_size->height,
        4);
    output->text_canvas.canvas = &output->canvas;
    output->text_canvas.font = font;
    output->text_canvas.character_spacing = 0;
    output->text_canvas.reserved_0c = 0;
    output->canvas.ops->clear(&output->canvas);
    return output;
}

static NtmvM2dManualButton *AllocateManualButton(
    NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dManualButton *button = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*button));

    if (button != NULL) {
        NtmvM2dManualButton_Construct(button);
    }
    return button;
}

static NtmvM2dUIStatic *AllocateUIStatic(
    NtmvM2dAllocatorContext *allocator)
{
    NtmvM2dUIStatic *element = NNS_FndAllocFromAllocator(
        allocator->nns_allocator, (uint32_t)sizeof(*element));

    if (element != NULL) {
        NtmvM2dUIStatic_ConstructComplete(element);
    }
    return element;
}

static void DrawControlText(
    const NtmvM2dManualTocTextDefinition *definition,
    const NtmvM2dItemsTextContext *text_context,
    const uint16_t *text)
{
    NtmvM2dManualTocObjTextCanvas canvas;
    NtmvM2dManualTocTextDirection direction;

    InitObjTextCanvas(
        &canvas,
        &definition->tile_origin,
        &definition->canvas_size,
        text_context->font);
    direction = GetTextDirection(canvas.text_canvas.font);
    NNSi_G2dTextCanvasDrawTextRect_020c6b50(
        &canvas.text_canvas,
        0,
        0,
        (int32_t)definition->canvas_size.width << 3,
        (int32_t)definition->canvas_size.height << 3,
        1,
        0x480,
        text,
        direction);
}

/* 0x020ba0d0 */
void NtmvM2dManualTocPanel_Configure(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    int16_t layout_threshold,
    const uint16_t *const control_texts[3],
    void *feedback_context)
{
    const NtmvM2dManualTocButtonDefinition *button_definitions =
        sButtonDefinitions[layout_threshold > 0x40 ? 1 : 0];
    const NtmvM2dPoint root_position = {0x80, 0x60};
    const NtmvM2dSize root_size = {0x100, 0xc0};
    const NtmvM2dSize marker_size = {0x1a, 0xc0};
    const NtmvM2dSize mode_marker_size = {0xba, 0xc0};
    const NtmvM2dSize body_size = {0x189, 0xc0};
    const NtmvM2dSize controls_size = {0x100, 0x17};
    const NtmvM2dSize content_size = {0x100, 0xab};
    const NtmvM2dSize footer_size = {0x100, 0x15};
    NtmvM2dUIElement *children[5];
    NtmvM2dUIPanel *controls_panel;
    NtmvM2dManualButton *button;
    NtmvM2dUIStatic *static_control;
    NtmvM2dItemsPanel *items;
    uint32_t index;

    panel->base.local_position = root_position;
    panel->base.size = root_size;
    panel->render_resources = animation_manager;
    panel->feedback_context = feedback_context;

    for (index = 0; index < 5; ++index) {
        const NtmvM2dManualTocButtonDefinition *definition =
            &button_definitions[index];
        const NtmvM2dManualTocTextDefinition *text_definition = NULL;
        int32_t overlay_handle = -1;
        int32_t alternate_handle = -1;
        int32_t animation_handle;

        button = AllocateManualButton(allocator);
        if (definition->text_definition_index != UINT16_MAX) {
            text_definition =
                &sTextDefinitions[definition->text_definition_index];
            overlay_handle = NtmvUiRenderer_CreateAnimation(
                animation_manager, text_definition->animation_sequence);
        }
        if (definition->alternate_animation_sequence != -1) {
            alternate_handle = NtmvUiRenderer_CreateAnimation(
                animation_manager,
                (uint16_t)definition->alternate_animation_sequence);
        }
        animation_handle = NtmvUiRenderer_CreateAnimation(
            animation_manager, definition->animation_sequence);
        NtmvM2dManualButton_Configure(
            button,
            &definition->position,
            &definition->size,
            animation_handle,
            alternate_handle,
            overlay_handle);
        panel->controls[index] = &button->base.base;

        if (text_definition != NULL) {
            DrawControlText(
                text_definition,
                text_context,
                control_texts[definition->text_definition_index]);
        }
    }

    for (index = 0; index < 2; ++index) {
        items = GameManualItemsPanel_Allocate_020ba7e0(
            &allocator->nns_allocator);
        panel->item_panels[index] = &items->base.base;
        NtmvM2dItemsPanel_Configure(
            items,
            allocator,
            animation_manager,
            text_context,
            &sItemsDefinitions[index]);
    }

    {
        const NtmvM2dPoint position = {-0xa7, 0};

        static_control = AllocateUIStatic(allocator);
        NtmvM2dUIStatic_Configure(
            static_control,
            &position,
            &marker_size,
            NtmvUiRenderer_CreateAnimation(animation_manager, 0x26));
        NtmvM2dUIStatic_SetAnimationFrame(
            static_control, animation_manager, 1);
        NtmvM2dUIElement_SetVisible(&static_control->base, false);
        panel->selection_marker = &static_control->base;
        NtmvM2dItemsPanel_SetAuxiliaryChild(
            (NtmvM2dItemsPanel *)panel->item_panels[1],
            0,
            panel->selection_marker);
    }

    {
        const NtmvM2dPoint positions[2] = {{-30, 36}, {31, 59}};

        panel->mode_markers[0] =
            GameManualUIElement_AllocateWithGeometry_020ba774(
                &allocator->nns_allocator, &positions[0], &mode_marker_size);
        panel->mode_markers[1] =
            GameManualUIElement_AllocateWithGeometry_020ba774(
                &allocator->nns_allocator, &positions[1], &mode_marker_size);
        NtmvM2dItemsPanel_SetAuxiliaryChild(
            (NtmvM2dItemsPanel *)panel->item_panels[0],
            0,
            panel->mode_markers[0]);
        NtmvM2dItemsPanel_SetAuxiliaryChild(
            (NtmvM2dItemsPanel *)panel->item_panels[1],
            1,
            panel->mode_markers[1]);
        panel->controls[7] = panel->mode_markers[0];
        panel->controls[8] = panel->mode_markers[1];
    }

    panel->body_panel = GameManualUIPanel_Allocate_020ba748(
        &allocator->nns_allocator);
    children[0] = panel->item_panels[0];
    children[1] = panel->item_panels[1];
    {
        const NtmvM2dPoint position = {0, 0};

        NtmvM2dUIPanel_Configure(
            panel->body_panel,
            allocator,
            children,
            2,
            &position,
            &body_size);
    }

    controls_panel = GameManualUIPanel_Allocate_020ba748(
        &allocator->nns_allocator);
    {
        const NtmvM2dPoint position = {0x4b, 0};
        NtmvM2dScrollButton *scroll_button =
            GameManualScrollButton_Allocate_020ba71c(
                &allocator->nns_allocator);

        NtmvM2dScrollButton_Configure(
            scroll_button,
            allocator,
            animation_manager,
            &position,
            0xae);
        panel->scroll_button = &scroll_button->base.base;
    }
    {
        const NtmvM2dPoint position = {0, 0};

        static_control = AllocateUIStatic(allocator);
        NtmvM2dUIStatic_Configure(
            static_control,
            &position,
            &controls_size,
            NtmvUiRenderer_CreateAnimation(animation_manager, 0x23));
        static_control->base.flags |= NTMV_M2D_UI_HIT_STATE_LOCKED;
        panel->controls[5] = &static_control->base;
    }
    children[0] = panel->controls[0];
    children[1] = panel->controls[1];
    children[2] = panel->controls[2];
    children[3] = panel->scroll_button;
    children[4] = panel->controls[5];
    {
        const NtmvM2dPoint position = {0, -0x57};

        NtmvM2dUIPanel_Configure(
            controls_panel,
            allocator,
            children,
            5,
            &position,
            &controls_size);
    }

    children[0] = &controls_panel->base;
    children[1] = &panel->body_panel->base;
    {
        const NtmvM2dPoint position = {0, 0x96};

        NtmvM2dUIPanel_Configure(
            &panel->content_panel,
            allocator,
            children,
            2,
            &position,
            &content_size);
    }

    {
        const NtmvM2dPoint position = {0, 0};

        static_control = AllocateUIStatic(allocator);
        NtmvM2dUIStatic_Configure(
            static_control,
            &position,
            &footer_size,
            NtmvUiRenderer_CreateAnimation(animation_manager, 0x24));
        static_control->base.flags |= NTMV_M2D_UI_HIT_STATE_LOCKED;
        panel->controls[6] = &static_control->base;
    }
    children[0] = panel->controls[3];
    children[1] = panel->controls[4];
    children[2] = panel->controls[6];
    {
        const NtmvM2dPoint position = {0, 0x56};

        NtmvM2dUIPanel_Configure(
            &panel->footer_panel,
            allocator,
            children,
            3,
            &position,
            &footer_size);
    }

    NtmvM2dManualTocPanel_SetNavigationAvailability(panel, false, false);
    panel->mode = NTMV_M2D_MANUAL_TOC_MODE_SPLIT;
    panel->item_panel_target_x = 0x89;
    NtmvM2dUIElement_SetVisible(panel->mode_markers[0], false);
    NtmvM2dUIElement_SetVisible(panel->mode_markers[1], false);
    NtmvM2dManualTocPanel_UpdateTargetWindowX(panel);
    panel->window_origins[0].x = -0x30;
    panel->window_origins[0].y = 0;
    panel->window_origins[1].x = 0x89;
    panel->window_origins[1].y = 0x3b;
    NtmvM2dManualTocPanel_SetSecondaryState_020bac98(panel, true);
}

/*
 * 0x020ba80c.  Embedded panels own the configured child trees, so the ROM
 * destroys footer first, then content, before invoking the direct base dtor.
 */
void NtmvM2dManualTocPanel_Destroy(
    NtmvM2dManualTocPanel *panel, NtmvM2dAllocatorContext *allocator)
{
    panel->footer_panel.base.vtable->destroy(
        &panel->footer_panel.base, allocator);
    panel->content_panel.base.vtable->destroy(
        &panel->content_panel.base, allocator);
    NtmvM2dUIElement_Destroy(&panel->base, allocator);
}

/* 0x020ba84c */
void NtmvM2dManualTocPanel_Render(
    NtmvM2dManualTocPanel *panel,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    int16_t content_relative_y;
    int16_t item_relative_y;

    if ((panel->base.flags & NTMV_M2D_UI_HIDDEN) != 0) {
        return;
    }

    panel->base.world_center.x =
        (int16_t)(parent_position->x + panel->base.local_position.x);
    panel->base.world_center.y =
        (int16_t)(parent_position->y + panel->base.local_position.y);
    panel->content_panel.base.vtable->render(
        &panel->content_panel.base, render_context, &panel->base.world_center);
    panel->footer_panel.base.vtable->render(
        &panel->footer_panel.base, render_context, &panel->base.world_center);

    content_relative_y = (int16_t)(
        panel->content_panel.base.world_center.y - panel->base.world_center.y);
    panel->window_origins[0].y = content_relative_y;
    panel->window_origins[2].y = content_relative_y;

    item_relative_y = (int16_t)(
        panel->item_panels[0]->world_center.y - panel->base.world_center.y + 0x24);
    if (panel->mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT) {
        panel->window_origins[1].x = 0x40;
    } else {
        panel->window_origins[1].x = (int16_t)(
            panel->item_panels[0]->world_center.x - panel->base.world_center.x);
    }
    panel->window_origins[1].y = item_relative_y;
}

/* 0x020ba948 */
void NtmvM2dManualTocPanel_SetTocData(
    NtmvM2dManualTocPanel *panel,
    const NtmvM2dItemsTocData *toc_data)
{
    panel->toc_data = toc_data;
    NtmvM2dItemsPanel_SetItemCount(
        (NtmvM2dItemsPanel *)panel->item_panels[0],
        toc_data->record_count);
    NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(panel);

    if (panel->toc_data->record_count == 0) {
        NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(panel);
    } else {
        NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(panel, 0);
        NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(panel, true);
    }
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
}

static void PlayInputFeedback(
    NtmvM2dManualTocPanel *panel, int32_t sequence_id)
{
    (void)GameSoundManager_PlaySequenceArc(
        (GameSoundManager *)panel->feedback_context,
        sequence_id,
        0x80);
}

/* 0x020ba9a4 */
bool NtmvM2dManualTocPanel_HandlePointer(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dManualTocAction *action,
    const NtmvM2dItemsPointerState *pointer)
{
    NtmvM2dItemsPointerAction item_actions[2] = {{0}, {0}};
    bool handled = false;
    int32_t panel_index;

    action->kind = NTMV_M2D_MANUAL_TOC_ACTION_NONE;
    action->value = 0;
    if ((panel->base.flags & NTMV_M2D_UI_HIDDEN) != 0 ||
        NtmvM2dManualTocPanel_IsAnimating_020bb730(panel)) {
        return false;
    }

    if (NtmvM2dManualTocPanel_HandleControls_020bb290(
            panel, action, pointer)) {
        return true;
    }

    for (panel_index = 0; panel_index < 2; ++panel_index) {
        NtmvM2dItemsPointerAction *item_action =
            &item_actions[panel_index];
        if (NtmvM2dItemsPanel_HandlePointer(
                (NtmvM2dItemsPanel *)panel->item_panels[panel_index],
                item_action,
                pointer)) {
            handled = true;
            if (item_action->control_hit != 0) {
                PlayInputFeedback(
                    panel, item_action->control_pressed != 0 ? 0 : 5);
            }
        }
    }

    if (!handled) {
        if (pointer->pressed == 0 && pointer->released == 0) {
            return false;
        }
        return NtmvM2dUIElement_ContainsPoint(
            &panel->content_panel.base, &pointer->position);
    }

    if (item_actions[0].page_changed != 0) {
        NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(panel, true);
        PlayInputFeedback(panel, 3);
    } else if (item_actions[1].page_changed != 0) {
        NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(panel);
        PlayInputFeedback(panel, 3);
    } else if (item_actions[0].selection_changed != 0) {
        NtmvM2dManualTocPanel_SetInteractionMode_020bb980(panel, false);
        NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(panel, false);
        NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(panel);
        NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
        action->kind = NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM;
        PlayInputFeedback(panel, 2);
    } else if (item_actions[1].selection_changed != 0) {
        NtmvM2dManualTocPanel_SetInteractionMode_020bb980(panel, false);
        NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
        action->kind = NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM;
        PlayInputFeedback(panel, 2);
    } else if (item_actions[0].row_activated != 0) {
        const NtmvM2dItemsPanel *primary =
            (const NtmvM2dItemsPanel *)panel->item_panels[0];
        const NtmvM2dItemsTocRecord *record =
            &panel->toc_data->records[primary->selected_item];

        NtmvM2dManualTocPanel_SetInteractionMode_020bb980(panel, false);
        if (record->child_count == 0) {
            PlayInputFeedback(panel, 1);
        } else {
            const NtmvM2dItemsPanel *secondary =
                (const NtmvM2dItemsPanel *)panel->item_panels[1];
            NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(panel, false);
            if (NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(record) ==
                secondary->selected_item) {
                return true;
            }
            NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(panel);
            NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
            action->kind = NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM;
            PlayInputFeedback(panel, 2);
        }
    } else if (item_actions[1].row_activated != 0) {
        PlayInputFeedback(panel, 1);
    }

    return true;
}

/* 0x020bac20 */
int32_t NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(
    const NtmvM2dManualTocPanel *panel)
{
    const NtmvM2dItemsPanel *primary =
        (const NtmvM2dItemsPanel *)panel->item_panels[0];
    return primary->selected_item;
}

/* 0x020bac2c */
void NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(
    NtmvM2dManualTocPanel *panel, int16_t item_index)
{
    NtmvM2dItemsPanel *primary =
        (NtmvM2dItemsPanel *)panel->item_panels[0];

    if (!NtmvM2dItemsPanel_SetSelectedItem(primary, item_index)) {
        return;
    }
    (void)NtmvM2dItemsPanel_SyncPageToSelection(primary);
    NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(panel);
}

/* 0x020bac58 */
int32_t NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(
    const NtmvM2dManualTocPanel *panel)
{
    const NtmvM2dItemsPanel *secondary =
        (const NtmvM2dItemsPanel *)panel->item_panels[1];
    return secondary->selected_item;
}

/* 0x020bac64 */
void NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(
    NtmvM2dManualTocPanel *panel, int16_t item_index)
{
    NtmvM2dItemsPanel *secondary =
        (NtmvM2dItemsPanel *)panel->item_panels[1];

    if (!NtmvM2dItemsPanel_SetSelectedItem(secondary, item_index)) {
        return;
    }
    if (!NtmvM2dItemsPanel_SyncPageToSelection(secondary)) {
        return;
    }
    NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(panel);
}

/* 0x020bac98 */
void NtmvM2dManualTocPanel_SetSecondaryState_020bac98(
    NtmvM2dManualTocPanel *panel, bool active)
{
    panel->secondary_state_active = active;
    NtmvM2dManualButton_SetAlternateAnimation(
        (NtmvM2dManualButton *)panel->controls[2], !active);
}

static int16_t StepToward(int16_t current, int16_t target, int16_t maximum_step)
{
    int32_t delta = (int32_t)target - current;

    if (delta > maximum_step) {
        delta = maximum_step;
    } else if (delta < -maximum_step) {
        delta = -maximum_step;
    }
    return (int16_t)(current + delta);
}

static void SetElementPosition(
    NtmvM2dUIElement *element, int16_t x, int16_t y)
{
    const NtmvM2dPoint position = {x, y};
    element->vtable->set_position(element, &position);
}

/* 0x020bacb8 */
void NtmvM2dManualTocPanel_Update(NtmvM2dManualTocPanel *panel)
{
    NtmvM2dItemsPanel *primary =
        (NtmvM2dItemsPanel *)panel->item_panels[0];
    NtmvM2dItemsPanel *secondary =
        (NtmvM2dItemsPanel *)panel->item_panels[1];
    int16_t content_y = panel->content_panel.base.local_position.y;
    int16_t secondary_x;

    /* Phase 1: vertical content transition and footer anchoring. */
    if (panel->secondary_state_active != 0) {
        if (content_y > 0) {
            content_y = StepToward(content_y, 0, 0x20);
            SetElementPosition(
                &panel->content_panel.base,
                panel->content_panel.base.local_position.x,
                content_y);
        }
    } else if (content_y < 0x96) {
        content_y = StepToward(content_y, 0x96, 0x20);
        SetElementPosition(
            &panel->content_panel.base,
            panel->content_panel.base.local_position.x,
            content_y);
    }
    SetElementPosition(
        &panel->footer_panel.base,
        panel->footer_panel.base.local_position.x,
        (int16_t)(0xec - content_y));

    /* Phase 2: linked secondary/primary horizontal transition. */
    secondary_x = secondary->base.base.local_position.x;
    if (secondary_x != panel->item_panel_target_x) {
        int16_t primary_x;

        secondary_x = StepToward(
            secondary_x, panel->item_panel_target_x, 0x20);
        SetElementPosition(
            &secondary->base.base,
            secondary_x,
            secondary->base.base.local_position.y);
        primary_x = (int16_t)(secondary_x - 0x89);
        if (primary_x > 0) {
            primary_x = 0;
        }
        SetElementPosition(
            &primary->base.base,
            primary_x,
            primary->base.base.local_position.y);
        if (secondary_x == panel->item_panel_target_x) {
            NtmvM2dManualTocPanel_UpdateTargetWindowX(panel);
        }
    }

    /* Phase 3: first clipping-window X transition. */
    panel->window_origins[0].x = StepToward(
        panel->window_origins[0].x, panel->target_window_x, 0x20);

    /* Phase 4: primary circular rows and dependent visibility. */
    if (NtmvM2dItemsPanel_AnimateScroll(
            primary,
            NtmvM2dItemsPanel_BuildTocRow,
            (void *)panel->toc_data) &&
        panel->state_ae != 0) {
        bool scroll_complete = primary->scroll_y == primary->target_scroll_y;

        panel->selection_marker->vtable->set_visible(
            panel->selection_marker, scroll_complete);
        secondary->base.base.vtable->set_visible(
            &secondary->base.base, scroll_complete);
    }

    /* Phase 5: the four interactive buttons consume one active frame. */
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)panel->controls[1]);
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)panel->controls[2]);
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)panel->controls[3]);
    NtmvM2dObjButton_TickActiveState(
        (NtmvM2dObjButton *)panel->controls[4]);
}

#if UINTPTR_MAX > UINT32_MAX
extern uint32_t NtmvM2dManualTocMmio_ReadDisplayControlForHost(void);
extern void NtmvM2dManualTocMmio_WriteDisplayControlForHost(uint32_t value);
extern void NtmvM2dManualTocMmio_WriteWindowOffsetForHost(
    uint32_t window_index, uint32_t value);
#endif

static uint32_t ReadDisplayControl(void)
{
#if UINTPTR_MAX > UINT32_MAX
    return NtmvM2dManualTocMmio_ReadDisplayControlForHost();
#else
    return *(volatile const uint32_t *)(uintptr_t)0x04000000;
#endif
}

static void WriteDisplayControl(uint32_t value)
{
#if UINTPTR_MAX > UINT32_MAX
    NtmvM2dManualTocMmio_WriteDisplayControlForHost(value);
#else
    *(volatile uint32_t *)(uintptr_t)0x04000000 = value;
#endif
}

static void WriteWindowOffset(uint32_t window_index, uint32_t value)
{
#if UINTPTR_MAX > UINT32_MAX
    NtmvM2dManualTocMmio_WriteWindowOffsetForHost(window_index, value);
#else
    volatile uint32_t *offset_registers =
        (volatile uint32_t *)(uintptr_t)0x04000010;
    offset_registers[window_index] = value;
#endif
}

static uint32_t PackWindowOffset(
    const NtmvM2dPoint *origin, bool eight_bit_horizontal)
{
    uint32_t horizontal_mask = eight_bit_horizontal ? 0xffu : 0x1ffu;
    uint32_t horizontal_base = eight_bit_horizontal ? 0x100u : 0x200u;
    uint32_t horizontal =
        (horizontal_base - (int32_t)origin->x) & horizontal_mask;
    uint32_t vertical =
        ((0x200u - (int32_t)origin->y) & 0x1ffu) << 16;

    return horizontal | vertical;
}

/* 0x020baef4 */
void NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(
    const NtmvM2dManualTocPanel *panel)
{
    uint32_t first_display = ReadDisplayControl();
    uint32_t window_bits = (first_display & 0x1f00u) >> 8;
    uint32_t second_display;

    if ((panel->base.flags & NTMV_M2D_UI_HIDDEN) != 0) {
        window_bits &= ~7u;
    } else {
        window_bits |= 7u;
    }
    second_display = ReadDisplayControl();
    WriteDisplayControl(
        (second_display & ~0x1f00u) | (window_bits << 8));
    WriteWindowOffset(0, PackWindowOffset(&panel->window_origins[0], false));
    WriteWindowOffset(1, PackWindowOffset(&panel->window_origins[1], false));
    WriteWindowOffset(2, PackWindowOffset(&panel->window_origins[2], true));
}

/* 0x020bafb4 */
void NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(
    NtmvM2dManualTocPanel *panel)
{
    static const uint16_t empty_text[] = {'(', '-', '-', '-', ')', 0};
    NtmvM2dItemsPanel *primary =
        (NtmvM2dItemsPanel *)panel->item_panels[0];
    NtmvM2dItemsRow rows[7];
    uint16_t populated_count = 0;
    int16_t first_item = 0;
    uint16_t row;

    if (panel->toc_data->record_count != 0) {
        uint16_t remaining;
        uint16_t visible_capacity;

        first_item = primary->first_visible_item;
        remaining = (uint16_t)(panel->toc_data->record_count - first_item);
        visible_capacity =
            (uint16_t)(primary->visible_row_count_minus_one + 1);
        populated_count = remaining <= visible_capacity
            ? remaining
            : visible_capacity;
    }

    for (row = 0; row < populated_count; ++row) {
        const NtmvM2dItemsTocRecord *record =
            &panel->toc_data->records[first_item + row];

        rows[row].text = record->text;
        if (rows[row].text[0] == 0) {
            rows[row].text = empty_text;
        }
        rows[row].use_alternate_animation = record->child_count != 0;
        rows[row].reserved_05[0] = 0;
        rows[row].reserved_05[1] = 0;
        rows[row].reserved_05[2] = 0;
    }
    NtmvM2dItemsPanel_UpdateRows(
        primary, rows, populated_count, first_item);
}

/* 0x020bb080 */
void NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(
    NtmvM2dManualTocPanel *panel)
{
    static const uint16_t empty_text[] = {'(', '-', '-', '-', ')', 0};
    const NtmvM2dItemsPanel *primary =
        (const NtmvM2dItemsPanel *)panel->item_panels[0];
    NtmvM2dItemsPanel *secondary =
        (NtmvM2dItemsPanel *)panel->item_panels[1];
    const NtmvM2dItemsTocRecord *record = NULL;
    NtmvM2dItemsRow rows[7];
    uint16_t populated_count = 0;
    int16_t first_item = 0;
    uint16_t row;

    if (primary->selected_item != -1) {
        uint16_t remaining;
        uint16_t visible_capacity;

        record = &panel->toc_data->records[primary->selected_item];
        first_item = secondary->first_visible_item;
        remaining = (uint16_t)(record->child_count - first_item);
        visible_capacity =
            (uint16_t)(secondary->visible_row_count_minus_one + 1);
        populated_count = remaining <= visible_capacity
            ? remaining
            : visible_capacity;
    }

    for (row = 0; row < populated_count; ++row) {
        rows[row].text = record->child_texts[first_item + row];
        if (rows[row].text[0] == 0) {
            rows[row].text = empty_text;
        }
        rows[row].use_alternate_animation = 0;
        rows[row].reserved_05[0] = 0;
        rows[row].reserved_05[1] = 0;
        rows[row].reserved_05[2] = 0;
    }
    NtmvM2dItemsPanel_UpdateSimpleRows(
        secondary, rows, populated_count, first_item);
}

/* 0x020bb148 */
void NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(
    NtmvM2dManualTocPanel *panel)
{
    const NtmvM2dItemsPanel *primary =
        (const NtmvM2dItemsPanel *)panel->item_panels[0];
    NtmvM2dItemsPanel *secondary =
        (NtmvM2dItemsPanel *)panel->item_panels[1];
    uint16_t secondary_count = 0;
    bool select_first = false;

    if (primary->selected_item != -1) {
        const NtmvM2dItemsTocRecord *record =
            &panel->toc_data->records[primary->selected_item];

        secondary_count = record->child_count;
        select_first = secondary_count != 0 && record->reserved_00 == 0;
    }

    NtmvM2dItemsPanel_SetItemCount(secondary, secondary_count);
    NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(panel);
    if (select_first) {
        NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(panel, 0);
    }
}

/* 0x020bb1bc..0x020bb207 */
static void SetAvailable(NtmvM2dUIElement *element, bool available)
{
    element->flags = (uint8_t)(
        (element->flags & (uint8_t)~NTMV_M2D_UI_HIT_STATE_LOCKED) |
        (available ? 0 : NTMV_M2D_UI_HIT_STATE_LOCKED));
}

void NtmvM2dManualTocPanel_SetNavigationAvailability(
    NtmvM2dManualTocPanel *panel,
    bool previous_available,
    bool next_available)
{
    SetAvailable(panel->controls[3], previous_available);
    SetAvailable(panel->controls[4], next_available);
}

/* 0x020bb208..0x020bb28f */
void NtmvM2dManualTocPanel_UpdateNavigation_020bb208(
    NtmvM2dManualTocPanel *panel)
{
    const NtmvM2dItemsPanel *primary =
        (const NtmvM2dItemsPanel *)panel->item_panels[0];
    bool previous_available = false;
    bool next_available = false;

    if (primary->selected_item >= 0) {
        const NtmvM2dItemsTocRecord *record =
            &panel->toc_data->records[primary->selected_item];
        const NtmvM2dItemsPanel *secondary =
            (const NtmvM2dItemsPanel *)panel->item_panels[1];

        next_available =
            (int32_t)primary->selected_item + 1 <
                panel->toc_data->record_count ||
            (int32_t)secondary->selected_item + 1 < record->child_count;
        previous_available =
            primary->selected_item > 0 ||
            secondary->selected_item >
                NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(record);
    }

    NtmvM2dManualTocPanel_SetNavigationAvailability(
        panel, previous_available, next_available);
}

/* 0x020bb290..0x020bb44f */
bool NtmvM2dManualTocPanel_HandleControls_020bb290(
    NtmvM2dManualTocPanel *panel,
    NtmvM2dManualTocAction *action,
    const NtmvM2dItemsPointerState *pointer)
{
    bool handled = false;

    action->kind = NTMV_M2D_MANUAL_TOC_ACTION_NONE;
    action->value = 0;
    if (pointer->active != 0) {
        if (pointer->pressed != 0) {
            int32_t control_index =
                NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
                    panel->controls, 9, &pointer->position);

            if (control_index != -1) {
                NtmvM2dUIElement *control = panel->controls[control_index];

                handled = true;
                if ((control->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) == 0) {
                    panel->active_control_index = (int16_t)control_index;
                    PlayInputFeedback(panel, 0);
                } else if (control_index <= 5) {
                    PlayInputFeedback(panel, 5);
                }
            }
        }

        if (panel->active_control_index == -1) {
            return handled;
        }
        NtmvM2dUIElement_UpdateHitState(
            panel->controls[panel->active_control_index], &pointer->position);
        return true;
    }

    if (pointer->released == 0 || panel->active_control_index == -1) {
        return false;
    }

    {
        int16_t control_index = panel->active_control_index;
        NtmvM2dUIElement *control = panel->controls[control_index];

        panel->active_control_index = -1;
        if ((control->flags & NTMV_M2D_UI_HOVERED) == 0) {
            return true;
        }
        control->flags &= (uint8_t)~NTMV_M2D_UI_HOVERED;
        switch (control_index) {
        case 0:
            action->kind = NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3;
            action->value =
                NtmvM2dManualTocPanel_ToggleGuideState_020bb660(panel) != 0;
            break;
        case 1:
            action->kind = NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_1;
            NtmvM2dManualTocPanel_ActivateControl1_020bb70c(panel);
            break;
        case 2:
            action->kind = NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2;
            action->value =
                NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(panel) != 0;
            break;
        case 3:
            action->kind = (uint16_t)
                NtmvM2dManualTocPanel_MovePrevious_020bb450(panel);
            break;
        case 4:
            action->kind = (uint16_t)
                NtmvM2dManualTocPanel_MoveNext_020bb56c(panel);
            break;
        case 7:
            NtmvM2dManualTocPanel_SetMode_020bb788(
                panel, NTMV_M2D_MANUAL_TOC_MODE_LEFT);
            break;
        case 8:
            NtmvM2dManualTocPanel_SetMode_020bb788(
                panel, NTMV_M2D_MANUAL_TOC_MODE_CENTER);
            break;
        default:
            break;
        }
    }
    return true;
}

/* 0x020bb450..0x020bb56b */
int32_t NtmvM2dManualTocPanel_MovePrevious_020bb450(
    NtmvM2dManualTocPanel *panel)
{
    int32_t action_kind = NTMV_M2D_MANUAL_TOC_ACTION_NONE;

    if (panel->secondary_state_active != 0) {
        return action_kind;
    }

    {
        NtmvM2dItemsPanel *primary =
            (NtmvM2dItemsPanel *)panel->item_panels[0];
        int16_t primary_index = primary->selected_item;

        if (primary_index >= 0) {
            NtmvM2dItemsPanel *secondary =
                (NtmvM2dItemsPanel *)panel->item_panels[1];
            const NtmvM2dItemsTocRecord *record =
                &panel->toc_data->records[primary_index];
            int16_t secondary_index = secondary->selected_item;

            if (record->child_count != 0 &&
                secondary_index !=
                    NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
                        record)) {
                NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(
                    panel, (int16_t)(secondary_index - 1));
                action_kind = NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM;
            } else if (primary_index > 0) {
                const NtmvM2dItemsTocRecord *previous_record;

                --primary_index;
                NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(
                    panel, primary_index);
                previous_record = &panel->toc_data->records[primary_index];
                if (previous_record->child_count != 0) {
                    NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(
                        panel,
                        (int16_t)(previous_record->child_count - 1));
                }
                NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(
                    panel, false);
                action_kind = NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM;
            }
        }
    }

    if (action_kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE) {
        PlayInputFeedback(panel, 5);
    } else {
        NtmvM2dManualTocPanel_SetInteractionMode_020bb980(panel, false);
        NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
        ((NtmvM2dObjButton *)panel->controls[3])->active_timer = 10;
        PlayInputFeedback(panel, 2);
    }
    return action_kind;
}

/* 0x020bb56c..0x020bb65f */
int32_t NtmvM2dManualTocPanel_MoveNext_020bb56c(
    NtmvM2dManualTocPanel *panel)
{
    int32_t action_kind = NTMV_M2D_MANUAL_TOC_ACTION_NONE;

    if (panel->secondary_state_active != 0) {
        return action_kind;
    }

    {
        NtmvM2dItemsPanel *primary =
            (NtmvM2dItemsPanel *)panel->item_panels[0];
        int16_t primary_index = primary->selected_item;
        bool select_next_primary = primary_index == -1;

        if (!select_next_primary) {
            const NtmvM2dItemsTocRecord *record =
                &panel->toc_data->records[primary_index];
            NtmvM2dItemsPanel *secondary =
                (NtmvM2dItemsPanel *)panel->item_panels[1];
            int32_t next_secondary = (int32_t)secondary->selected_item + 1;

            select_next_primary =
                record->child_count == 0 ||
                next_secondary >= record->child_count;
            if (!select_next_primary) {
                NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(
                    panel, (int16_t)next_secondary);
                action_kind = NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM;
            }
        }

        if (select_next_primary) {
            int16_t next_primary = (int16_t)(primary_index + 1);

            if ((int32_t)next_primary < panel->toc_data->record_count) {
                NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(
                    panel, next_primary);
                NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(
                    panel, false);
                action_kind = NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM;
            }
        }
    }

    if (action_kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE) {
        PlayInputFeedback(panel, 5);
    } else {
        NtmvM2dManualTocPanel_SetInteractionMode_020bb980(panel, false);
        NtmvM2dManualTocPanel_UpdateNavigation_020bb208(panel);
        ((NtmvM2dObjButton *)panel->controls[4])->active_timer = 10;
        PlayInputFeedback(panel, 2);
    }
    return action_kind;
}

/* 0x020bb660..0x020bb697 */
bool NtmvM2dManualTocPanel_ToggleGuideState_020bb660(
    NtmvM2dManualTocPanel *panel)
{
    NtmvM2dManualTocPanel_SetInteractionMode_020bb980(
        panel, panel->state_ac == 0);
    PlayInputFeedback(panel, 2);
    return panel->state_ac != 0;
}

/* 0x020bb698..0x020bb6c7 */
bool NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
    NtmvM2dManualTocPanel *panel, uint8_t *show_guide)
{
    if (NtmvM2dManualTocPanel_IsAnimating_020bb730(panel)) {
        return false;
    }
    *show_guide = NtmvM2dManualTocPanel_ToggleGuideState_020bb660(panel);
    return true;
}

/* 0x020bb6c8..0x020bb70b */
bool NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
    NtmvM2dManualTocPanel *panel)
{
    NtmvM2dManualTocPanel_SetSecondaryState_020bac98(
        panel, panel->secondary_state_active == 0);
    ((NtmvM2dObjButton *)panel->controls[2])->active_timer = 10;
    PlayInputFeedback(panel, 8);
    return panel->secondary_state_active != 0;
}

/* 0x020bb70c..0x020bb72f */
void NtmvM2dManualTocPanel_ActivateControl1_020bb70c(
    NtmvM2dManualTocPanel *panel)
{
    ((NtmvM2dObjButton *)panel->controls[1])->active_timer = 10;
    PlayInputFeedback(panel, 4);
}

/* 0x020bb730..0x020bb787 */
bool NtmvM2dManualTocPanel_IsAnimating_020bb730(
    const NtmvM2dManualTocPanel *panel)
{
    const NtmvM2dItemsPanel *secondary;
    const NtmvM2dItemsPanel *primary;
    int16_t content_y = panel->content_panel.base.local_position.y;

    if (content_y > 0 && content_y < 0x96) {
        return true;
    }
    secondary = (const NtmvM2dItemsPanel *)panel->item_panels[1];
    if (panel->item_panel_target_x != secondary->base.base.local_position.x) {
        return true;
    }
    primary = (const NtmvM2dItemsPanel *)panel->item_panels[0];
    return primary->scroll_y != primary->target_scroll_y;
}

/* 0x020bb788..0x020bb85b */
void NtmvM2dManualTocPanel_SetMode_020bb788(
    NtmvM2dManualTocPanel *panel, int32_t mode)
{
    panel->mode = (int16_t)mode;
    switch (mode) {
    case NTMV_M2D_MANUAL_TOC_MODE_LEFT:
        panel->item_panel_target_x = 0x89;
        panel->mode_markers[0]->vtable->set_visible(
            panel->mode_markers[0], false);
        panel->mode_markers[1]->vtable->set_visible(
            panel->mode_markers[1], true);
        break;
    case NTMV_M2D_MANUAL_TOC_MODE_CENTER:
        panel->item_panel_target_x = 0;
        panel->mode_markers[0]->vtable->set_visible(
            panel->mode_markers[0], true);
        panel->mode_markers[1]->vtable->set_visible(
            panel->mode_markers[1], false);
        break;
    case NTMV_M2D_MANUAL_TOC_MODE_SPLIT:
        panel->item_panel_target_x = 0x89;
        panel->mode_markers[0]->vtable->set_visible(
            panel->mode_markers[0], false);
        panel->mode_markers[1]->vtable->set_visible(
            panel->mode_markers[1], false);
        break;
    default:
        break;
    }

    if (panel->item_panel_target_x == panel->item_panels[1]->local_position.x) {
        NtmvM2dManualTocPanel_UpdateTargetWindowX(panel);
    } else {
        PlayInputFeedback(panel, 3);
    }
}

/* 0x020bb85c..0x020bb897 */
void NtmvM2dManualTocPanel_UpdateTargetWindowX(
    NtmvM2dManualTocPanel *panel)
{
    if (panel->mode == NTMV_M2D_MANUAL_TOC_MODE_LEFT) {
        panel->target_window_x = -0x60;
    } else if (panel->mode == NTMV_M2D_MANUAL_TOC_MODE_CENTER) {
        panel->target_window_x = 0;
    } else if (panel->mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT) {
        panel->target_window_x = -0x30;
    }
}

/* 0x020bb898..0x020bb97f */
void NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(
    NtmvM2dManualTocPanel *panel, bool force_update)
{
    const NtmvM2dItemsPanel *primary =
        (const NtmvM2dItemsPanel *)panel->item_panels[0];
    int32_t selected_item = primary->selected_item;
    int32_t mode = NTMV_M2D_MANUAL_TOC_MODE_SPLIT;

    panel->state_ae = 0;
    if (selected_item >= 0) {
        int32_t visible_offset =
            selected_item - primary->first_visible_item;

        if (visible_offset >= 0 &&
            visible_offset < primary->visible_row_count_minus_one &&
            panel->toc_data->records[selected_item].child_count != 0) {
            NtmvM2dPoint position = {
                panel->selection_marker->local_position.x,
                (int16_t)((visible_offset + 1) * 0x17),
            };

            panel->selection_marker->vtable->set_position(
                panel->selection_marker, &position);
            NtmvM2dUIStatic_SetAnimationFrame(
                (NtmvM2dUIStatic *)panel->selection_marker,
                panel->render_resources,
                0);
            mode = force_update
                ? NTMV_M2D_MANUAL_TOC_MODE_LEFT
                : NTMV_M2D_MANUAL_TOC_MODE_CENTER;
            panel->state_ae = 1;
        }
    }

    panel->selection_marker->vtable->set_visible(
        panel->selection_marker, panel->state_ae != 0);
    panel->item_panels[1]->vtable->set_visible(
        panel->item_panels[1], panel->state_ae != 0);
    NtmvM2dManualTocPanel_SetMode_020bb788(panel, mode);
}

/* 0x020bb980..0x020bb9ff */
void NtmvM2dManualTocPanel_SetInteractionMode_020bb980(
    NtmvM2dManualTocPanel *panel, bool inactive)
{
    NtmvM2dItemsPanel *primary =
        (NtmvM2dItemsPanel *)panel->item_panels[0];
    NtmvM2dItemsPanel *secondary =
        (NtmvM2dItemsPanel *)panel->item_panels[1];
    bool enabled = !inactive;

    panel->state_ac = inactive;
    ((NtmvM2dManualButton *)panel->controls[0])->selected = inactive;
    if (!enabled) {
        NtmvM2dItemsPanel_SetSelectedRowVisual(primary, false);
        primary->interaction_enabled = false;
        NtmvM2dItemsPanel_SetSelectedRowVisual(secondary, false);
        secondary->interaction_enabled = false;
        return;
    }
    primary->interaction_enabled = true;
    NtmvM2dItemsPanel_SetSelectedRowVisual(primary, true);
    secondary->interaction_enabled = true;
    NtmvM2dItemsPanel_SetSelectedRowVisual(secondary, true);
}
