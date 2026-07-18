#include "ntmv/m2d/manual_toc_panel.h"
#include "game/sound_manager.h"
#include "ntmv/m2d/scroll_controls.h"
#include "ntmv/m2d/ui_static.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int32_t NtmvM2dManualTocPanel_MovePrevious_020bb450(
    NtmvM2dManualTocPanel *panel);
int32_t NtmvM2dManualTocPanel_MoveNext_020bb56c(
    NtmvM2dManualTocPanel *panel);
bool NtmvM2dManualTocPanel_ToggleGuideState_020bb660(
    NtmvM2dManualTocPanel *panel);
bool NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
    NtmvM2dManualTocPanel *panel, uint8_t *show_guide);
bool NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(
    NtmvM2dManualTocPanel *panel);
void NtmvM2dManualTocPanel_ActivateControl1_020bb70c(
    NtmvM2dManualTocPanel *panel);

typedef struct TestChild {
    NtmvM2dUIElement base;
    int id;
} TestChild;

static NtmvM2dUIElement *sContentElement;
static NtmvM2dUIElement *sFooterElement;
static unsigned int sRenderCount;
static int sRenderOrder[2];
static unsigned int sDestroyCount;
static int sDestroyOrder[2];
static unsigned int sAllocationCount;
static unsigned int sFreeCount;

typedef struct TestButtonConfiguration {
    NtmvM2dManualButton *button;
    NtmvM2dPoint position;
    NtmvM2dSize size;
    int32_t animation_handle;
    int32_t alternate_animation_handle;
    int32_t overlay_animation_handle;
} TestButtonConfiguration;

typedef struct TestItemsConfiguration {
    NtmvM2dItemsPanel *panel;
    int16_t definition[9];
    NtmvM2dUIElement *children[16];
} TestItemsConfiguration;

typedef struct TestCanvas TestCanvas;

typedef struct TestCanvasOps {
    void (*unknown_00)(TestCanvas *canvas);
    void (*clear)(TestCanvas *canvas);
} TestCanvasOps;

struct TestCanvas {
    uint8_t unknown_00[0x14];
    const TestCanvasOps *ops;
};

typedef struct TestTextDirection {
    int8_t x;
    int8_t y;
} TestTextDirection;

typedef struct TestCanvasConfiguration {
    void *vram;
    int32_t width_tiles;
    int32_t height_tiles;
    uint32_t color_depth;
} TestCanvasConfiguration;

typedef struct TestDrawConfiguration {
    int32_t width;
    int32_t height;
    uint32_t flags;
    uint32_t draw_value;
    const uint16_t *text;
    TestTextDirection direction;
} TestDrawConfiguration;

static TestButtonConfiguration sButtonConfigurations[5];
static TestItemsConfiguration sItemsConfigurations[2];
static TestCanvasConfiguration sCanvasConfigurations[3];
static TestDrawConfiguration sDrawConfigurations[3];
static NtmvM2dUIStatic *sConfiguredStatics[3];
static uint16_t sAnimationSequences[16];
static unsigned int sButtonConfigurationCount;
static unsigned int sItemsConfigurationCount;
static unsigned int sStaticConfigurationCount;
static unsigned int sCanvasConfigurationCount;
static unsigned int sDrawConfigurationCount;
static unsigned int sAnimationSequenceCount;
static unsigned int sCanvasClearCount;
static unsigned int sAuxiliaryCallCount;
static NtmvM2dItemsPanel *sAuxiliaryPanels[3];
static int32_t sAuxiliaryIndices[3];
static NtmvM2dUIElement *sAuxiliaryChildren[3];
static NtmvM2dScrollButton *sConfiguredScrollButton;
static NtmvM2dPoint sScrollPosition;
static int16_t sScrollPageSize;
static NtmvM2dManualButton *sAlternateButton;
static bool sAlternateValue;
static unsigned int sAlternateCallCount;

enum TestTocCall {
    TEST_TOC_SET_COUNT = 1,
    TEST_TOC_REFRESH_PRIMARY,
    TEST_TOC_EMPTY_SELECTION,
    TEST_TOC_SELECT_PRIMARY,
    TEST_TOC_SYNC_PRIMARY,
    TEST_TOC_NAVIGATION,
};

static int sTocCalls[8];
static unsigned int sTocCallCount;
static NtmvM2dManualTocPanel *sExpectedTocPanel;
static const NtmvM2dItemsTocData *sExpectedTocData;
static uint16_t sSetItemCountValue;
static int16_t sSelectedPrimaryItem;
static bool sTestingPrimarySelection;
static bool sTestingSecondarySelection;
static NtmvM2dManualTocPanel *sSelectionPanel;
static bool sSetSelectedResult;
static bool sSyncSelectionResult;
static unsigned int sSetSelectedCallCount;
static unsigned int sSyncSelectionCallCount;
static unsigned int sSelectionResetCallCount;
static unsigned int sSelectionRefreshCallCount;
static int16_t sDirectSelectedItem;
static unsigned int sSelectionStep;

enum TestInputCall {
    TEST_INPUT_PRIMARY_POINTER = 1,
    TEST_INPUT_SECONDARY_POINTER,
    TEST_INPUT_EMPTY_SELECTION,
    TEST_INPUT_NAVIGATION,
    TEST_INPUT_REFRESH_SECONDARY,
    TEST_INPUT_INITIAL_CHILD,
    TEST_INPUT_SOUND,
};

static bool sTestingPointerInput;
static int sInputCalls[32];
static unsigned int sInputCallCount;
static NtmvM2dManualTocPanel *sInputPanel;
static const NtmvM2dItemsPointerState *sInputPointer;
static bool sItemsHandled[2];
static NtmvM2dItemsPointerAction sItemsActions[2];
static int32_t sInitialChildIndex;
static const NtmvM2dItemsTocRecord *sExpectedInitialChildRecord;
static int32_t sSoundSequences[8];
static unsigned int sSoundCount;
static bool sTestingControlHandler;
static NtmvM2dManualTocPanel *sControlPanel;
static bool sTestingMoveHelpers;
static NtmvM2dManualTocPanel *sMovePanel;
static int32_t sMoveInitialChildIndex;
static unsigned int sMovePrimarySelectionCount;
static unsigned int sMoveSecondarySelectionCount;
static unsigned int sMoveSecondaryResetCount;
static unsigned int sMoveSecondaryRefreshCount;
static bool sTestingModeHelpers;
static NtmvM2dManualTocPanel *sModePanel;
static bool sTestingSecondaryMode;
static NtmvM2dManualTocPanel *sSecondaryModePanel;
static unsigned int sSecondaryModeFrameCount;
static uint32_t sSecondaryModeFrame;
static bool sTestingInteractionMode;
static NtmvM2dManualTocPanel *sInteractionPanel;
static NtmvM2dItemsPanel *sInteractionPanels[4];
static bool sInteractionValues[4];
static unsigned int sInteractionCallCount;
static bool sTestingNavigation;
static NtmvM2dManualTocPanel *sNavigationPanel;
static const NtmvM2dItemsTocRecord *sNavigationRecord;
static int32_t sNavigationInitialChildIndex;
static unsigned int sNavigationInitialChildCallCount;

static bool sTestingUpdate;
static NtmvM2dManualTocPanel *sUpdatePanel;
static bool sAnimateScrollResult;
static unsigned int sAnimateScrollCallCount;
static NtmvM2dItemsPanel *sAnimatedPanel;
static NtmvM2dItemsRowProvider sAnimatedProvider;
static void *sAnimatedContext;
static NtmvM2dUIElement *sUpdatePositionElements[8];
static NtmvM2dPoint sUpdatePositions[8];
static unsigned int sUpdatePositionCount;
static NtmvM2dUIElement *sUpdateVisibilityElements[4];
static bool sUpdateVisibilityValues[4];
static unsigned int sUpdateVisibilityCount;
static NtmvM2dObjButton *sTickedButtons[4];
static unsigned int sTickedButtonCount;

static bool sTestingWindowPublish;
static uint32_t sDisplayReadValues[2];
static unsigned int sDisplayReadCount;
static uint32_t sDisplayWrittenValue;
static unsigned int sDisplayWriteCount;
static uint32_t sWindowWriteIndices[3];
static uint32_t sWindowWrittenValues[3];
static unsigned int sWindowWriteCount;

static bool sTestingRefreshPrimary;
static NtmvM2dManualTocPanel *sRefreshPanel;
static NtmvM2dItemsRow sRefreshedRows[7];
static uint16_t sRefreshedRowCount;
static int16_t sRefreshedFirstItem;
static unsigned int sRefreshRowsCallCount;
static bool sTestingRefreshSecondary;
static NtmvM2dManualTocPanel *sSecondaryRefreshPanel;
static NtmvM2dItemsRow sRefreshedSecondaryRows[7];
static uint16_t sRefreshedSecondaryRowCount;
static int16_t sRefreshedSecondaryFirstItem;
static unsigned int sRefreshSecondaryRowsCallCount;
static bool sPointerEmptySelectionRefresh;

enum TestInitializeCall {
    TEST_INITIALIZE_SET_COUNT = 1,
    TEST_INITIALIZE_REFRESH,
    TEST_INITIALIZE_SELECT,
    TEST_INITIALIZE_SYNC,
};

static bool sTestingInitializeEmptySelection;
static NtmvM2dManualTocPanel *sInitializePanel;
static int sInitializeCalls[4];
static unsigned int sInitializeCallCount;
static uint16_t sInitializeSecondaryCount;
static int16_t sInitializeSelectedItem;

static void RecordInputCall(int call)
{
    assert(sTestingPointerInput);
    assert(sInputCallCount < sizeof(sInputCalls) / sizeof(sInputCalls[0]));
    sInputCalls[sInputCallCount++] = call;
}

static void TestCanvasClear(TestCanvas *canvas)
{
    assert(canvas != NULL);
    ++sCanvasClearCount;
}

static const TestCanvasOps sCanvasOps = {NULL, TestCanvasClear};

NtmvM2dManualButton *NtmvM2dManualButton_Construct(
    NtmvM2dManualButton *button)
{
    NtmvM2dUIElement_Init(&button->base.base);
    button->alternate_animation_handle = -1;
    button->overlay_animation_handle = -1;
    return button;
}

void NtmvM2dManualButton_Configure(
    NtmvM2dManualButton *button,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t animation_handle,
    int32_t alternate_animation_handle,
    int32_t overlay_animation_handle)
{
    TestButtonConfiguration *configuration;

    assert(sButtonConfigurationCount < 5);
    configuration = &sButtonConfigurations[sButtonConfigurationCount++];
    configuration->button = button;
    configuration->position = *position;
    configuration->size = *size;
    configuration->animation_handle = animation_handle;
    configuration->alternate_animation_handle = alternate_animation_handle;
    configuration->overlay_animation_handle = overlay_animation_handle;
    button->base.base.local_position = *position;
    button->base.base.size = *size;
    button->base.animation_handle = animation_handle;
    button->alternate_animation_handle = alternate_animation_handle;
    button->overlay_animation_handle = overlay_animation_handle;
}

void NtmvM2dManualButton_SetAlternateAnimation(
    NtmvM2dManualButton *button, bool use_alternate_animation)
{
    sAlternateButton = button;
    sAlternateValue = use_alternate_animation;
    ++sAlternateCallCount;
    button->use_alternate_animation = use_alternate_animation;
}

NtmvM2dItemsPanel *NtmvM2dItemsPanel_Construct(NtmvM2dItemsPanel *panel)
{
    NtmvM2dUIElement_Init(&panel->base.base);
    panel->base.children = NULL;
    panel->selected_item = -1;
    return panel;
}

void NtmvM2dItemsPanel_Configure(
    NtmvM2dItemsPanel *panel,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dItemsTextContext *text_context,
    const NtmvM2dItemsPanelDefinition *definition)
{
    const int16_t *values = (const int16_t *)definition;
    TestItemsConfiguration *configuration;

    assert(allocator->nns_allocator == (void *)(uintptr_t)0x5678);
    assert(animation_manager == (void *)(uintptr_t)0x3333);
    assert(text_context != NULL);
    assert(sItemsConfigurationCount < 2);
    configuration = &sItemsConfigurations[sItemsConfigurationCount++];
    configuration->panel = panel;
    memcpy(configuration->definition, definition, sizeof(configuration->definition));
    memset(configuration->children, 0, sizeof(configuration->children));
    panel->base.base.local_position.x = values[0];
    panel->base.base.local_position.y = values[1];
    panel->base.base.size.width = 0x100;
    panel->base.base.size.height = 0xc0;
    panel->base.children = configuration->children;
    panel->base.child_count = (uint16_t)(values[7] + values[8] + 5);
    panel->visible_row_count_minus_one = values[7];
    panel->text_context = text_context;
}

void NtmvM2dItemsPanel_SetAuxiliaryChild(
    NtmvM2dItemsPanel *panel,
    int32_t auxiliary_index,
    NtmvM2dUIElement *child)
{
    int32_t child_index = panel->visible_row_count_minus_one + 5 +
                          auxiliary_index;

    assert(sAuxiliaryCallCount < 3);
    sAuxiliaryPanels[sAuxiliaryCallCount] = panel;
    sAuxiliaryIndices[sAuxiliaryCallCount] = auxiliary_index;
    sAuxiliaryChildren[sAuxiliaryCallCount] = child;
    ++sAuxiliaryCallCount;
    panel->base.children[child_index] = child;
}

void NtmvM2dItemsPanel_SetItemCount(
    NtmvM2dItemsPanel *panel, uint16_t item_count)
{
    if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        assert((NtmvM2dItemsPanel *)sMovePanel->item_panels[1] == panel);
        ++sMoveSecondaryResetCount;
        panel->item_count = item_count;
        panel->first_visible_item = 0;
        panel->selected_item = -1;
        return;
    }
    if (sTestingInitializeEmptySelection) {
        assert(sInitializePanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sInitializePanel->item_panels[1] == panel);
        assert(sInitializeCallCount == 0);
        sInitializeCalls[sInitializeCallCount++] = TEST_INITIALIZE_SET_COUNT;
        sInitializeSecondaryCount = item_count;
        panel->item_count = item_count;
        panel->first_visible_item = 0;
        panel->selected_item = -1;
        return;
    }
    if (sTestingPointerInput) {
        assert(sInputPanel != NULL);
        assert((NtmvM2dItemsPanel *)sInputPanel->item_panels[1] == panel);
        RecordInputCall(TEST_INPUT_EMPTY_SELECTION);
        panel->item_count = item_count;
        panel->first_visible_item = 0;
        panel->selected_item = -1;
        sPointerEmptySelectionRefresh = true;
        return;
    }
    if (sTestingPrimarySelection) {
        assert(sSelectionPanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sSelectionPanel->item_panels[1] == panel);
        assert(sSelectionStep == 2);
        ++sSelectionStep;
        ++sSelectionResetCallCount;
        panel->item_count = item_count;
        panel->first_visible_item = 0;
        panel->selected_item = -1;
        return;
    }
    assert(sTocCallCount < sizeof(sTocCalls) / sizeof(sTocCalls[0]));
    assert(sExpectedTocPanel != NULL);
    assert(sExpectedTocPanel->toc_data == sExpectedTocData);
    if ((NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[0] == panel) {
        sTocCalls[sTocCallCount++] = TEST_TOC_SET_COUNT;
        sSetItemCountValue = item_count;
    } else {
        assert(
            (NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[1] == panel);
        sTocCalls[sTocCallCount++] = TEST_TOC_EMPTY_SELECTION;
    }
    panel->item_count = item_count;
    panel->first_visible_item = 0;
    panel->selected_item = -1;
}

void NtmvM2dItemsPanel_SetSelectedRowVisual(
    NtmvM2dItemsPanel *panel, bool selected)
{
    if (sTestingInteractionMode) {
        assert(sInteractionPanel != NULL);
        assert(sInteractionCallCount < 4);
        assert(
            (NtmvM2dItemsPanel *)sInteractionPanel->item_panels[0] == panel ||
            (NtmvM2dItemsPanel *)sInteractionPanel->item_panels[1] == panel);
        sInteractionPanels[sInteractionCallCount] = panel;
        sInteractionValues[sInteractionCallCount] = selected;
        ++sInteractionCallCount;
    }
}

bool NtmvM2dItemsPanel_HandlePointer(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsPointerAction *action,
    const NtmvM2dItemsPointerState *pointer)
{
    int index;

    assert(sTestingPointerInput);
    assert(sInputPanel != NULL);
    assert(pointer == sInputPointer);
    if ((NtmvM2dItemsPanel *)sInputPanel->item_panels[0] == panel) {
        index = 0;
        RecordInputCall(TEST_INPUT_PRIMARY_POINTER);
    } else {
        assert((NtmvM2dItemsPanel *)sInputPanel->item_panels[1] == panel);
        index = 1;
        RecordInputCall(TEST_INPUT_SECONDARY_POINTER);
    }
    *action = sItemsActions[index];
    return sItemsHandled[index];
}

bool NtmvM2dItemsPanel_AnimateScroll(
    NtmvM2dItemsPanel *panel,
    NtmvM2dItemsRowProvider row_provider,
    void *provider_context)
{
    assert(sTestingUpdate);
    assert(sAnimateScrollCallCount == 0);
    sAnimateScrollCallCount = 1;
    sAnimatedPanel = panel;
    sAnimatedProvider = row_provider;
    sAnimatedContext = provider_context;
    return sAnimateScrollResult;
}

void NtmvM2dItemsPanel_BuildTocRow(
    NtmvM2dItemsRow *output, void *context, int16_t item_index)
{
    (void)output;
    (void)context;
    (void)item_index;
    assert(0);
}

void NtmvM2dObjButton_TickActiveState(NtmvM2dObjButton *button)
{
    assert(sTestingUpdate);
    assert(sTickedButtonCount < 4);
    sTickedButtons[sTickedButtonCount++] = button;
}

void NtmvM2dItemsPanel_UpdateRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index)
{
    if (sTestingRefreshPrimary) {
        assert(sRefreshPanel != NULL);
        assert((NtmvM2dItemsPanel *)sRefreshPanel->item_panels[0] == panel);
        assert(populated_count <= 7);
        ++sRefreshRowsCallCount;
        sRefreshedRowCount = populated_count;
        sRefreshedFirstItem = first_item_index;
        if (populated_count != 0) {
            memcpy(
                sRefreshedRows,
                rows,
                populated_count * sizeof(sRefreshedRows[0]));
        }
        return;
    }
    assert(sExpectedTocPanel != NULL);
    assert(sExpectedTocPanel->toc_data == sExpectedTocData);
    assert((NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[0] == panel);
    assert(sTocCallCount < sizeof(sTocCalls) / sizeof(sTocCalls[0]));
    sTocCalls[sTocCallCount++] = TEST_TOC_REFRESH_PRIMARY;
}

uint32_t NtmvM2dManualTocMmio_ReadDisplayControlForHost(void)
{
    assert(sTestingWindowPublish);
    assert(sDisplayReadCount < 2);
    return sDisplayReadValues[sDisplayReadCount++];
}

void NtmvM2dManualTocMmio_WriteDisplayControlForHost(uint32_t value)
{
    assert(sTestingWindowPublish);
    assert(sDisplayWriteCount == 0);
    sDisplayWrittenValue = value;
    sDisplayWriteCount = 1;
}

void NtmvM2dManualTocMmio_WriteWindowOffsetForHost(
    uint32_t window_index, uint32_t value)
{
    assert(sTestingWindowPublish);
    assert(sWindowWriteCount < 3);
    sWindowWriteIndices[sWindowWriteCount] = window_index;
    sWindowWrittenValues[sWindowWriteCount] = value;
    ++sWindowWriteCount;
}

bool NtmvM2dItemsPanel_SetSelectedItem(
    NtmvM2dItemsPanel *panel, int16_t selected_item)
{
    if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        if ((NtmvM2dItemsPanel *)sMovePanel->item_panels[0] == panel) {
            ++sMovePrimarySelectionCount;
        } else {
            assert((NtmvM2dItemsPanel *)sMovePanel->item_panels[1] == panel);
            ++sMoveSecondarySelectionCount;
        }
        panel->selected_item = selected_item;
        return true;
    }
    if (sTestingInitializeEmptySelection) {
        assert(sInitializePanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sInitializePanel->item_panels[1] == panel);
        assert(sInitializeCallCount == 2);
        sInitializeCalls[sInitializeCallCount++] = TEST_INITIALIZE_SELECT;
        sInitializeSelectedItem = selected_item;
        panel->selected_item = selected_item;
        return true;
    }
    if (sTestingPrimarySelection || sTestingSecondarySelection) {
        int panel_index = sTestingSecondarySelection ? 1 : 0;
        assert(sSelectionPanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sSelectionPanel->item_panels[panel_index] ==
            panel);
        assert(sSelectionStep == 0);
        ++sSelectionStep;
        ++sSetSelectedCallCount;
        sDirectSelectedItem = selected_item;
        if (sSetSelectedResult) {
            panel->selected_item = selected_item;
        }
        return sSetSelectedResult;
    }
    if (sTestingPointerInput) {
        assert(sInputPanel != NULL);
        assert((NtmvM2dItemsPanel *)sInputPanel->item_panels[1] == panel);
        panel->selected_item = selected_item;
        return true;
    }
    assert(sExpectedTocPanel != NULL);
    assert(sExpectedTocPanel->toc_data == sExpectedTocData);
    assert((NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[0] == panel);
    assert(sTocCallCount < sizeof(sTocCalls) / sizeof(sTocCalls[0]));
    sTocCalls[sTocCallCount++] = TEST_TOC_SELECT_PRIMARY;
    sSelectedPrimaryItem = selected_item;
    panel->selected_item = selected_item;
    return true;
}

bool NtmvM2dItemsPanel_SyncPageToSelection(NtmvM2dItemsPanel *panel)
{
    if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sMovePanel->item_panels[0] == panel ||
            (NtmvM2dItemsPanel *)sMovePanel->item_panels[1] == panel);
        return false;
    }
    if (sTestingInitializeEmptySelection) {
        assert(sInitializePanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sInitializePanel->item_panels[1] == panel);
        assert(sInitializeCallCount == 3);
        sInitializeCalls[sInitializeCallCount++] = TEST_INITIALIZE_SYNC;
        return false;
    }
    if (sTestingPrimarySelection || sTestingSecondarySelection) {
        int panel_index = sTestingSecondarySelection ? 1 : 0;
        assert(sSelectionPanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sSelectionPanel->item_panels[panel_index] ==
            panel);
        assert(sSelectionStep == 1);
        ++sSelectionStep;
        ++sSyncSelectionCallCount;
        return sSyncSelectionResult;
    }
    if (sTestingPointerInput) {
        assert(sInputPanel != NULL);
        assert((NtmvM2dItemsPanel *)sInputPanel->item_panels[1] == panel);
        return false;
    }
    assert(sExpectedTocPanel != NULL);
    assert((NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[0] == panel);
    assert(sTocCallCount < sizeof(sTocCalls) / sizeof(sTocCalls[0]));
    sTocCalls[sTocCallCount++] = TEST_TOC_SYNC_PRIMARY;
    return true;
}

void NtmvM2dItemsPanel_UpdateSimpleRows(
    NtmvM2dItemsPanel *panel,
    const NtmvM2dItemsRow *rows,
    uint16_t populated_count,
    int16_t first_item_index)
{
    if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        assert((NtmvM2dItemsPanel *)sMovePanel->item_panels[1] == panel);
        ++sMoveSecondaryRefreshCount;
        return;
    }
    if (sTestingRefreshSecondary) {
        assert(sSecondaryRefreshPanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sSecondaryRefreshPanel->item_panels[1] ==
            panel);
        assert(populated_count <= 7);
        ++sRefreshSecondaryRowsCallCount;
        sRefreshedSecondaryRowCount = populated_count;
        sRefreshedSecondaryFirstItem = first_item_index;
        if (populated_count != 0) {
            memcpy(
                sRefreshedSecondaryRows,
                rows,
                populated_count * sizeof(sRefreshedSecondaryRows[0]));
        }
        return;
    }
    if (sTestingInitializeEmptySelection) {
        assert(sInitializePanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sInitializePanel->item_panels[1] == panel);
        assert(sInitializeCallCount == 1);
        sInitializeCalls[sInitializeCallCount++] = TEST_INITIALIZE_REFRESH;
        return;
    }
    if (sTestingPrimarySelection) {
        assert(sSelectionPanel != NULL);
        assert(
            (NtmvM2dItemsPanel *)sSelectionPanel->item_panels[1] == panel);
        assert(sSelectionStep == 3);
        return;
    }
    if (sTestingSecondarySelection) {
        assert((NtmvM2dItemsPanel *)sSelectionPanel->item_panels[1] == panel);
        assert(sSelectionStep == 2);
        ++sSelectionStep;
        ++sSelectionRefreshCallCount;
        return;
    }
    if (sTestingPointerInput) {
        assert((NtmvM2dItemsPanel *)sInputPanel->item_panels[1] == panel);
        if (sPointerEmptySelectionRefresh) {
            sPointerEmptySelectionRefresh = false;
        } else {
            RecordInputCall(TEST_INPUT_REFRESH_SECONDARY);
        }
        return;
    }
    assert(sExpectedTocPanel != NULL);
    assert(
        (NtmvM2dItemsPanel *)sExpectedTocPanel->item_panels[1] == panel);
}

int32_t NtmvM2dUIElementArray_FindContainingPoint_020b8e88(
    NtmvM2dUIElement *const *elements,
    int32_t element_count,
    const NtmvM2dPoint *point)
{
    int32_t index;

    for (index = 0; index < element_count; ++index) {
        if (NtmvM2dUIElement_ContainsPoint(elements[index], point)) {
            return index;
        }
    }
    return -1;
}

int32_t NtmvM2dItemsTocRecord_GetInitialChildIndex_020b8ef8(
    const NtmvM2dItemsTocRecord *record)
{
    if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        assert(record >= sMovePanel->toc_data->records);
        assert(record <
               sMovePanel->toc_data->records +
                   sMovePanel->toc_data->record_count);
        return sMoveInitialChildIndex;
    }
    if (sTestingNavigation) {
        assert(sNavigationPanel != NULL);
        assert(record == sNavigationRecord);
        ++sNavigationInitialChildCallCount;
        return sNavigationInitialChildIndex;
    }
    if (sTestingPointerInput) {
        assert(record == sExpectedInitialChildRecord);
        RecordInputCall(TEST_INPUT_INITIAL_CHILD);
        return sInitialChildIndex;
    }
    assert(sExpectedTocPanel != NULL);
    assert(sExpectedTocPanel->toc_data == sExpectedTocData);
    assert(record == &sExpectedTocData->records[0]);
    assert(sTocCallCount < sizeof(sTocCalls) / sizeof(sTocCalls[0]));
    sTocCalls[sTocCallCount++] = TEST_TOC_NAVIGATION;
    return 0;
}

GameSoundEntry *GameSoundManager_PlaySequenceArc(
    GameSoundManager *manager,
    int32_t sequence_id,
    int32_t channel_priority)
{
    if (sTestingControlHandler) {
        assert(sControlPanel != NULL);
        assert(manager == (GameSoundManager *)sControlPanel->feedback_context);
    } else if (sTestingMoveHelpers) {
        assert(sMovePanel != NULL);
        assert(manager == (GameSoundManager *)sMovePanel->feedback_context);
    } else if (sTestingModeHelpers) {
        assert(sModePanel != NULL);
        assert(manager == (GameSoundManager *)sModePanel->feedback_context);
    } else {
        assert(manager == (GameSoundManager *)sInputPanel->feedback_context);
    }
    assert(channel_priority == 0x80);
    assert(sSoundCount < sizeof(sSoundSequences) / sizeof(sSoundSequences[0]));
    sSoundSequences[sSoundCount++] = sequence_id;
    if (!sTestingControlHandler &&
        !sTestingMoveHelpers &&
        !sTestingModeHelpers) {
        RecordInputCall(TEST_INPUT_SOUND);
    }
    return NULL;
}

NtmvM2dUIStatic *NtmvM2dUIStatic_ConstructComplete(
    NtmvM2dUIStatic *element)
{
    NtmvM2dUIElement_Init(&element->base);
    element->resource_id = -1;
    return element;
}

void NtmvM2dUIStatic_Configure(
    NtmvM2dUIStatic *element,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size,
    int32_t resource_id)
{
    assert(sStaticConfigurationCount < 3);
    sConfiguredStatics[sStaticConfigurationCount++] = element;
    element->base.local_position = *position;
    element->base.size = *size;
    element->resource_id = resource_id;
}

void NtmvM2dUIStatic_SetAnimationFrame(
    NtmvM2dUIStatic *element, void *render_context, uint32_t frame)
{
    if (sTestingSecondaryMode) {
        assert(sSecondaryModePanel != NULL);
        assert(element ==
               (NtmvM2dUIStatic *)sSecondaryModePanel->selection_marker);
        assert(render_context == sSecondaryModePanel->render_resources);
        ++sSecondaryModeFrameCount;
        sSecondaryModeFrame = frame;
        return;
    }
    assert(element == sConfiguredStatics[0]);
    assert(render_context == (void *)(uintptr_t)0x3333);
    assert(frame == 1);
}

NtmvM2dScrollButton *NtmvM2dScrollButton_Construct(
    NtmvM2dScrollButton *button)
{
    NtmvM2dUIElement_Init(&button->base.base);
    button->base.children = NULL;
    return button;
}

void NtmvM2dScrollButton_Configure(
    NtmvM2dScrollButton *button,
    NtmvM2dAllocatorContext *allocator,
    void *animation_manager,
    const NtmvM2dPoint *position,
    int16_t page_size)
{
    assert(allocator->nns_allocator == (void *)(uintptr_t)0x5678);
    assert(animation_manager == (void *)(uintptr_t)0x3333);
    sConfiguredScrollButton = button;
    sScrollPosition = *position;
    sScrollPageSize = page_size;
    button->base.base.local_position = *position;
    button->page_size = page_size;
}

int32_t NtmvUiRenderer_CreateAnimation(
    void *renderer, uint16_t sequence_index)
{
    assert(renderer == (void *)(uintptr_t)0x3333);
    assert(sAnimationSequenceCount < 16);
    sAnimationSequences[sAnimationSequenceCount++] = sequence_index;
    return 1000 + sequence_index;
}

void NNS_G2dCharCanvasInitForOBJ1D(
    void *canvas,
    void *vram,
    int32_t width_tiles,
    int32_t height_tiles,
    uint32_t color_depth)
{
    TestCanvas *test_canvas = canvas;
    TestCanvasConfiguration *configuration;

    assert(sCanvasConfigurationCount < 3);
    configuration = &sCanvasConfigurations[sCanvasConfigurationCount++];
    configuration->vram = vram;
    configuration->width_tiles = width_tiles;
    configuration->height_tiles = height_tiles;
    configuration->color_depth = color_depth;
    test_canvas->ops = &sCanvasOps;
}

void NNSi_G2dTextCanvasDrawTextRect_020c6b50(
    void *canvas,
    int32_t x,
    int32_t y,
    int32_t width,
    int32_t height,
    uint32_t flags,
    uint32_t draw_value,
    const uint16_t *text,
    TestTextDirection direction)
{
    TestDrawConfiguration *configuration;

    assert(canvas != NULL);
    assert(x == 0 && y == 0);
    assert(sDrawConfigurationCount < 3);
    configuration = &sDrawConfigurations[sDrawConfigurationCount++];
    configuration->width = width;
    configuration->height = height;
    configuration->flags = flags;
    configuration->draw_value = draw_value;
    configuration->text = text;
    configuration->direction = direction;
}

void *NNS_FndAllocFromAllocator(void *allocator, uint32_t size)
{
    assert(allocator == (void *)(uintptr_t)0x5678);
    ++sAllocationCount;
    return malloc(size);
}

NtmvM2dScrollButton *GameManualScrollButton_Allocate_020ba71c(
    void *const *allocator_context)
{
    NtmvM2dScrollButton *button = NNS_FndAllocFromAllocator(
        *allocator_context, (uint32_t)sizeof(*button));

    return button != NULL ? NtmvM2dScrollButton_Construct(button) : NULL;
}

NtmvM2dUIPanel *GameManualUIPanel_Allocate_020ba748(
    void *const *allocator_context)
{
    NtmvM2dUIPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, (uint32_t)sizeof(*panel));

    return panel != NULL ? NtmvM2dUIPanel_ConstructComplete(panel) : NULL;
}

NtmvM2dUIElement *GameManualUIElement_AllocateWithGeometry_020ba774(
    void *const *allocator_context,
    const NtmvM2dPoint *position,
    const NtmvM2dSize *size)
{
    NtmvM2dUIElement *element = NNS_FndAllocFromAllocator(
        *allocator_context, (uint32_t)sizeof(*element));

    if (element != NULL) {
        NtmvM2dUIElement_InitWithGeometry(element, position, size);
    }
    return element;
}

NtmvM2dItemsPanel *GameManualItemsPanel_Allocate_020ba7e0(
    void *const *allocator_context)
{
    NtmvM2dItemsPanel *panel = NNS_FndAllocFromAllocator(
        *allocator_context, (uint32_t)sizeof(*panel));

    return panel != NULL ? NtmvM2dItemsPanel_Construct(panel) : NULL;
}

void NNS_FndFreeToAllocator(void *allocator, void *memory)
{
    assert(allocator == (void *)(uintptr_t)0x5678);
    ++sFreeCount;
    free(memory);
}

static void TestChildDestroy(
    NtmvM2dUIElement *element, NtmvM2dAllocatorContext *allocator)
{
    TestChild *child = (TestChild *)element;

    assert(allocator->nns_allocator == (void *)(uintptr_t)0x5678);
    assert(sDestroyCount < 2);
    sDestroyOrder[sDestroyCount++] = child->id;
}

static void TestPanelRender(
    NtmvM2dUIElement *element,
    void *render_context,
    const NtmvM2dPoint *parent_position)
{
    assert(render_context == (void *)(uintptr_t)0x2222);
    assert(parent_position->x == 110 && parent_position->y == 70);
    assert(sRenderCount < 2);
    if (element == sContentElement) {
        sRenderOrder[sRenderCount++] = 1;
        element->world_center.x = (int16_t)(parent_position->x + 5);
        element->world_center.y = (int16_t)(parent_position->y + 7);
    } else {
        assert(element == sFooterElement);
        sRenderOrder[sRenderCount++] = 2;
        element->world_center.x = (int16_t)(parent_position->x - 3);
        element->world_center.y = (int16_t)(parent_position->y + 9);
    }
}

static const NtmvM2dUIElementVTable sRenderVTable = {
    NtmvM2dUIElement_Destroy,
    TestPanelRender,
    NtmvM2dUIElement_SetPosition,
    NtmvM2dUIElement_SetSize,
    NtmvM2dUIElement_SetVisible,
};

static const NtmvM2dUIElementVTable sChildVTable = {
    TestChildDestroy,
    NtmvM2dUIElement_Render,
    NtmvM2dUIElement_SetPosition,
    NtmvM2dUIElement_SetSize,
    NtmvM2dUIElement_SetVisible,
};

static void TestUpdateSetPosition(
    NtmvM2dUIElement *element, const NtmvM2dPoint *position)
{
    assert(sTestingUpdate);
    assert(sUpdatePositionCount < 8);
    sUpdatePositionElements[sUpdatePositionCount] = element;
    sUpdatePositions[sUpdatePositionCount] = *position;
    ++sUpdatePositionCount;
    element->local_position = *position;
}

static void TestUpdateSetVisible(NtmvM2dUIElement *element, bool visible)
{
    assert(sTestingUpdate);
    assert(sUpdateVisibilityCount < 4);
    sUpdateVisibilityElements[sUpdateVisibilityCount] = element;
    sUpdateVisibilityValues[sUpdateVisibilityCount] = visible;
    ++sUpdateVisibilityCount;
    NtmvM2dUIElement_SetVisible(element, visible);
}

static const NtmvM2dUIElementVTable sUpdateVTable = {
    NtmvM2dUIElement_Destroy,
    NtmvM2dUIElement_Render,
    TestUpdateSetPosition,
    NtmvM2dUIElement_SetSize,
    TestUpdateSetVisible,
};

static TestChild *AllocateChild(int id)
{
    TestChild *child = malloc(sizeof(*child));

    assert(child != NULL);
    NtmvM2dUIElement_Init(&child->base);
    child->base.vtable = &sChildVTable;
    child->id = id;
    return child;
}

static void TestConstructorAndRender(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dUIElement item_panel;
    NtmvM2dUIElement previous;
    NtmvM2dUIElement next;
    NtmvM2dPoint parent = {100, 50};
    NtmvM2dPoint changed_position = {1, 2};
    NtmvM2dSize changed_size = {3, 4};
    unsigned int index;

    memset(&panel, 0xa5, sizeof(panel));
    assert(NtmvM2dManualTocPanel_Construct(&panel) == &panel);
    assert(panel.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dManualTocPanelVTable);
    assert(panel.toc_data == NULL);
    assert(panel.render_resources == NULL);
    assert(panel.feedback_context == NULL);
    assert(panel.content_panel.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dUIPanelVTable);
    assert(panel.footer_panel.base.vtable ==
           (const NtmvM2dUIElementVTable *)&gNtmvM2dUIPanelVTable);
    assert(panel.body_panel == NULL);
    for (index = 0; index < 2; ++index) {
        assert(panel.item_panels[index] == NULL);
        assert(panel.mode_markers[index] == NULL);
    }
    for (index = 0; index < 9; ++index) {
        assert(panel.controls[index] == NULL);
    }
    assert(panel.selection_marker == NULL);
    assert(panel.scroll_button == NULL);
    assert(panel.active_control_index == -1);
    assert(panel.item_panel_target_x == 0x89);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(panel.target_window_x == -0x30);
    for (index = 0; index < 3; ++index) {
        assert(panel.window_origins[index].x == 0);
        assert(panel.window_origins[index].y == 0);
    }
    assert(
        panel.state_ac == 0 && panel.secondary_state_active == 0 &&
        panel.state_ae == 0);
    assert(panel.reserved_af == 0xa5);

    panel.base.local_position.x = 10;
    panel.base.local_position.y = 20;
    NtmvM2dUIElement_Init(&item_panel);
    item_panel.world_center.x = 150;
    item_panel.world_center.y = 125;
    panel.item_panels[0] = &item_panel;
    sContentElement = &panel.content_panel.base;
    sFooterElement = &panel.footer_panel.base;
    panel.content_panel.base.vtable = &sRenderVTable;
    panel.footer_panel.base.vtable = &sRenderVTable;

    sRenderCount = 0;
    panel.base.vtable->render(
        &panel.base, (void *)(uintptr_t)0x2222, &parent);
    assert(sRenderCount == 2);
    assert(sRenderOrder[0] == 1 && sRenderOrder[1] == 2);
    assert(panel.base.world_center.x == 110 && panel.base.world_center.y == 70);
    assert(panel.window_origins[0].y == 7);
    assert(panel.window_origins[2].y == 7);
    assert(panel.window_origins[1].x == 0x40);
    assert(panel.window_origins[1].y == 91);

    panel.mode = NTMV_M2D_MANUAL_TOC_MODE_CENTER;
    sRenderCount = 0;
    panel.base.vtable->render(
        &panel.base, (void *)(uintptr_t)0x2222, &parent);
    assert(sRenderCount == 2);
    assert(panel.window_origins[1].x == 40);

    panel.base.flags = NTMV_M2D_UI_HIDDEN;
    sRenderCount = 0;
    panel.base.vtable->render(
        &panel.base, (void *)(uintptr_t)0x2222, &parent);
    assert(sRenderCount == 0);
    panel.base.flags = 0;

    NtmvM2dUIElement_Init(&previous);
    NtmvM2dUIElement_Init(&next);
    previous.flags = NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED;
    next.flags = NTMV_M2D_UI_HOVERED;
    panel.controls[3] = &previous;
    panel.controls[4] = &next;
    NtmvM2dManualTocPanel_SetNavigationAvailability(&panel, true, false);
    assert(previous.flags == NTMV_M2D_UI_HIDDEN);
    assert(next.flags ==
           (NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED));
    NtmvM2dManualTocPanel_SetNavigationAvailability(&panel, false, true);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED));
    assert(next.flags == NTMV_M2D_UI_HOVERED);

    panel.mode = NTMV_M2D_MANUAL_TOC_MODE_LEFT;
    NtmvM2dManualTocPanel_UpdateTargetWindowX(&panel);
    assert(panel.target_window_x == -0x60);
    panel.mode = NTMV_M2D_MANUAL_TOC_MODE_CENTER;
    NtmvM2dManualTocPanel_UpdateTargetWindowX(&panel);
    assert(panel.target_window_x == 0);
    panel.mode = NTMV_M2D_MANUAL_TOC_MODE_SPLIT;
    NtmvM2dManualTocPanel_UpdateTargetWindowX(&panel);
    assert(panel.target_window_x == -0x30);
    panel.mode = 9;
    panel.target_window_x = 123;
    NtmvM2dManualTocPanel_UpdateTargetWindowX(&panel);
    assert(panel.target_window_x == 123);

    panel.base.vtable->set_position(&panel.base, &changed_position);
    panel.base.vtable->set_size(&panel.base, &changed_size);
    panel.base.vtable->set_visible(&panel.base, false);
    assert(panel.base.local_position.x == 1 && panel.base.local_position.y == 2);
    assert(panel.base.size.width == 3 && panel.base.size.height == 4);
    assert((panel.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
}

static void TestOwnedPanelDestruction(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x5678};
    TestChild *content_child = AllocateChild(1);
    TestChild *footer_child = AllocateChild(2);
    NtmvM2dUIElement *content_children[1] = {&content_child->base};
    NtmvM2dUIElement *footer_children[1] = {&footer_child->base};
    NtmvM2dPoint position = {0, 0};
    NtmvM2dSize size = {10, 10};

    NtmvM2dManualTocPanel_Construct(&panel);
    NtmvM2dUIPanel_Configure(
        &panel.content_panel,
        &allocator,
        content_children,
        1,
        &position,
        &size);
    NtmvM2dUIPanel_Configure(
        &panel.footer_panel,
        &allocator,
        footer_children,
        1,
        &position,
        &size);
    assert(sAllocationCount == 2);

    panel.base.vtable->destroy(&panel.base, &allocator);
    assert(sDestroyCount == 2);
    assert(sDestroyOrder[0] == 2 && sDestroyOrder[1] == 1);
    assert(sFreeCount == 4); /* two child objects and two child arrays */
}

static void ResetTocCalls(
    NtmvM2dManualTocPanel *panel,
    const NtmvM2dItemsTocData *toc_data)
{
    memset(sTocCalls, 0, sizeof(sTocCalls));
    sTocCallCount = 0;
    sExpectedTocPanel = panel;
    sExpectedTocData = toc_data;
    sSetItemCountValue = UINT16_MAX;
    sSelectedPrimaryItem = -123;
}

static void TestSetTocData(void)
{
    static const uint16_t title[] = {'T', 0};
    NtmvM2dItemsTocRecord records[3];
    NtmvM2dItemsTocData nonempty_data;
    NtmvM2dItemsTocData empty_data;
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dUIElement previous;
    NtmvM2dUIElement next;
    NtmvM2dUIElement selection_marker;
    NtmvM2dUIElement mode_markers[2];
    static const int expected_nonempty[6] = {
        TEST_TOC_SET_COUNT,
        TEST_TOC_REFRESH_PRIMARY,
        TEST_TOC_SELECT_PRIMARY,
        TEST_TOC_SYNC_PRIMARY,
        TEST_TOC_EMPTY_SELECTION,
        TEST_TOC_NAVIGATION,
    };
    static const int expected_empty[3] = {
        TEST_TOC_SET_COUNT,
        TEST_TOC_REFRESH_PRIMARY,
        TEST_TOC_EMPTY_SELECTION,
    };

    memset(records, 0, sizeof(records));
    records[0].text = title;
    records[1].text = title;
    records[2].text = title;
    nonempty_data.records = records;
    nonempty_data.record_count = 3;
    nonempty_data.reserved_06 = 0;
    empty_data.records = NULL;
    empty_data.record_count = 0;
    empty_data.reserved_06 = 0;

    NtmvM2dManualTocPanel_Construct(&panel);
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dUIElement_Init(&previous);
    NtmvM2dUIElement_Init(&next);
    NtmvM2dUIElement_Init(&selection_marker);
    NtmvM2dUIElement_Init(&mode_markers[0]);
    NtmvM2dUIElement_Init(&mode_markers[1]);
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.controls[3] = &previous;
    panel.controls[4] = &next;
    panel.selection_marker = &selection_marker;
    panel.mode_markers[0] = &mode_markers[0];
    panel.mode_markers[1] = &mode_markers[1];
    secondary.base.base.local_position.x = 0x89;

    /* Non-empty data selects primary item zero, applies secondary mode one,
     * then computes navigation after all preceding initialization. */
    panel.toc_data = &empty_data;
    primary.item_count = 99;
    primary.selected_item = 7;
    secondary.item_count = 88;
    secondary.selected_item = 6;
    previous.flags = NTMV_M2D_UI_HIDDEN;
    next.flags = NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED;
    ResetTocCalls(&panel, &nonempty_data);
    NtmvM2dManualTocPanel_SetTocData(&panel, &nonempty_data);
    assert(panel.toc_data == &nonempty_data);
    assert(sSetItemCountValue == 3);
    assert(primary.item_count == 3);
    assert(primary.selected_item == 0);
    assert(sSelectedPrimaryItem == 0);
    assert(secondary.item_count == 0);
    assert(secondary.selected_item == -1);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(sTocCallCount == 6);
    assert(memcmp(
               sTocCalls,
               expected_nonempty,
               sizeof(expected_nonempty)) == 0);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED));
    assert(next.flags == NTMV_M2D_UI_HOVERED);

    /* Empty data skips selection/mode and invokes the dedicated empty-state
     * initializer between row refresh and final navigation. */
    primary.item_count = 77;
    primary.selected_item = 5;
    secondary.item_count = 66;
    secondary.selected_item = 4;
    previous.flags = NTMV_M2D_UI_HIDDEN;
    next.flags = NTMV_M2D_UI_HOVERED;
    ResetTocCalls(&panel, &empty_data);
    NtmvM2dManualTocPanel_SetTocData(&panel, &empty_data);
    assert(panel.toc_data == &empty_data);
    assert(sSetItemCountValue == 0);
    assert(primary.item_count == 0);
    assert(primary.selected_item == -1);
    assert(secondary.item_count == 0);
    assert(secondary.selected_item == -1);
    assert(sSelectedPrimaryItem == -123);
    assert(sTocCallCount == 3);
    assert(memcmp(
               sTocCalls,
               expected_empty,
               sizeof(expected_empty)) == 0);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED));
    assert(next.flags ==
           (NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED));
}

static void ResetPrimarySelectionTest(
    NtmvM2dManualTocPanel *panel,
    bool set_selected_result,
    bool sync_selection_result)
{
    sTestingPrimarySelection = true;
    sTestingSecondarySelection = false;
    sSelectionPanel = panel;
    sSetSelectedResult = set_selected_result;
    sSyncSelectionResult = sync_selection_result;
    sSetSelectedCallCount = 0;
    sSyncSelectionCallCount = 0;
    sSelectionResetCallCount = 0;
    sSelectionRefreshCallCount = 0;
    sDirectSelectedItem = INT16_MIN;
    sSelectionStep = 0;
}

static void TestPrimarySelectionAccessAndSet(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord records[6];
    NtmvM2dItemsTocData toc_data;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    toc_data.records = records;
    toc_data.record_count = 6;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;

    primary.selected_item = -1;
    assert(NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(&panel) == -1);
    primary.selected_item = INT16_MIN;
    assert(
        NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(&panel) ==
        INT16_MIN);
    primary.selected_item = INT16_MAX;
    assert(
        NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(&panel) ==
        INT16_MAX);

    /* An unchanged selection exits immediately after SetSelectedItem. */
    primary.selected_item = 3;
    ResetPrimarySelectionTest(&panel, false, true);
    NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(&panel, 3);
    assert(primary.selected_item == 3);
    assert(sDirectSelectedItem == 3);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 0);
    assert(sSelectionResetCallCount == 0);
    assert(sSelectionStep == 1);

    /* A changed selection always resets the secondary list, regardless of
     * whether synchronizing the primary page reports another change. */
    ResetPrimarySelectionTest(&panel, true, false);
    NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(&panel, 5);
    assert(primary.selected_item == 5);
    assert(sDirectSelectedItem == 5);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 1);
    assert(sSelectionResetCallCount == 1);
    assert(sSelectionStep == 3);

    ResetPrimarySelectionTest(&panel, true, true);
    NtmvM2dManualTocPanel_SelectPrimaryItem_020bac2c(&panel, -1);
    assert(primary.selected_item == -1);
    assert(sDirectSelectedItem == -1);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 1);
    assert(sSelectionResetCallCount == 1);
    assert(sSelectionStep == 3);

    sTestingPrimarySelection = false;
}

static void ResetSecondarySelectionTest(
    NtmvM2dManualTocPanel *panel,
    bool set_selected_result,
    bool sync_selection_result)
{
    sTestingPrimarySelection = false;
    sTestingSecondarySelection = true;
    sSelectionPanel = panel;
    sSetSelectedResult = set_selected_result;
    sSyncSelectionResult = sync_selection_result;
    sSetSelectedCallCount = 0;
    sSyncSelectionCallCount = 0;
    sSelectionResetCallCount = 0;
    sSelectionRefreshCallCount = 0;
    sDirectSelectedItem = INT16_MIN;
    sSelectionStep = 0;
}

static void TestSecondarySelectionAccessAndSet(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord record;
    NtmvM2dItemsTocData toc_data;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(&record, 0, sizeof(record));
    toc_data.records = &record;
    toc_data.record_count = 1;
    toc_data.reserved_06 = 0;
    primary.selected_item = -1;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;

    secondary.selected_item = -1;
    assert(
        NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(&panel) == -1);
    secondary.selected_item = INT16_MIN;
    assert(
        NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(&panel) ==
        INT16_MIN);
    secondary.selected_item = INT16_MAX;
    assert(
        NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(&panel) ==
        INT16_MAX);

    /* An unchanged selection bypasses both page synchronization and rows. */
    secondary.selected_item = 2;
    ResetSecondarySelectionTest(&panel, false, true);
    NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(&panel, 2);
    assert(secondary.selected_item == 2);
    assert(sDirectSelectedItem == 2);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 0);
    assert(sSelectionRefreshCallCount == 0);
    assert(sSelectionStep == 1);

    /* A selection on the current page synchronizes but does not redraw. */
    ResetSecondarySelectionTest(&panel, true, false);
    NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(&panel, 4);
    assert(secondary.selected_item == 4);
    assert(sDirectSelectedItem == 4);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 1);
    assert(sSelectionRefreshCallCount == 0);
    assert(sSelectionStep == 2);

    /* Moving to another page is the only path that refreshes its rows. */
    ResetSecondarySelectionTest(&panel, true, true);
    NtmvM2dManualTocPanel_SelectSecondaryItem_020bac64(&panel, -1);
    assert(secondary.selected_item == -1);
    assert(sDirectSelectedItem == -1);
    assert(sSetSelectedCallCount == 1);
    assert(sSyncSelectionCallCount == 1);
    assert(sSelectionRefreshCallCount == 1);
    assert(sSelectionResetCallCount == 0);
    assert(sSelectionStep == 3);

    sTestingSecondarySelection = false;
}

static void TestSecondaryStateSetter(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dManualButton button;

    memset(&panel, 0, sizeof(panel));
    memset(&button, 0, sizeof(button));
    NtmvM2dManualButton_Construct(&button);
    panel.controls[2] = &button.base.base;
    panel.secondary_state_active = 0xff;
    sAlternateButton = NULL;
    sAlternateValue = false;
    sAlternateCallCount = 0;

    NtmvM2dManualTocPanel_SetSecondaryState_020bac98(&panel, false);
    assert(panel.secondary_state_active == 0);
    assert(sAlternateCallCount == 1);
    assert(sAlternateButton == &button);
    assert(sAlternateValue);
    assert(button.use_alternate_animation);

    NtmvM2dManualTocPanel_SetSecondaryState_020bac98(&panel, true);
    assert(panel.secondary_state_active == 1);
    assert(sAlternateCallCount == 2);
    assert(sAlternateButton == &button);
    assert(!sAlternateValue);
    assert(!button.use_alternate_animation);
}

typedef struct TestUpdateFixture {
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dUIElement selection_marker;
    NtmvM2dObjButton controls[4];
    NtmvM2dItemsTocData toc_data;
} TestUpdateFixture;

static void InitializeUpdateFixture(TestUpdateFixture *fixture)
{
    unsigned int index;

    memset(fixture, 0, sizeof(*fixture));
    fixture->panel.content_panel.base.vtable = &sUpdateVTable;
    fixture->panel.footer_panel.base.vtable = &sUpdateVTable;
    fixture->primary.base.base.vtable = &sUpdateVTable;
    fixture->secondary.base.base.vtable = &sUpdateVTable;
    fixture->selection_marker.vtable = &sUpdateVTable;
    fixture->panel.item_panels[0] = &fixture->primary.base.base;
    fixture->panel.item_panels[1] = &fixture->secondary.base.base;
    fixture->panel.selection_marker = &fixture->selection_marker;
    fixture->panel.toc_data = &fixture->toc_data;
    fixture->panel.content_panel.base.local_position.x = 11;
    fixture->panel.footer_panel.base.local_position.x = 22;
    fixture->primary.base.base.local_position.y = -36;
    fixture->secondary.base.base.local_position.y = -59;
    fixture->panel.secondary_state_active = 1;
    for (index = 0; index < 4; ++index) {
        fixture->panel.controls[index + 1] = &fixture->controls[index].base;
    }
}

static void ResetUpdateTest(TestUpdateFixture *fixture, bool animate_scroll)
{
    memset(sUpdatePositionElements, 0, sizeof(sUpdatePositionElements));
    memset(sUpdatePositions, 0, sizeof(sUpdatePositions));
    sUpdatePositionCount = 0;
    memset(sUpdateVisibilityElements, 0, sizeof(sUpdateVisibilityElements));
    memset(sUpdateVisibilityValues, 0, sizeof(sUpdateVisibilityValues));
    sUpdateVisibilityCount = 0;
    memset(sTickedButtons, 0, sizeof(sTickedButtons));
    sTickedButtonCount = 0;
    sUpdatePanel = &fixture->panel;
    sAnimateScrollResult = animate_scroll;
    sAnimateScrollCallCount = 0;
    sAnimatedPanel = NULL;
    sAnimatedProvider = NULL;
    sAnimatedContext = NULL;
    sTestingUpdate = true;
}

static void AssertUpdateCommon(const TestUpdateFixture *fixture)
{
    unsigned int index;

    assert(sAnimateScrollCallCount == 1);
    assert(sAnimatedPanel == &fixture->primary);
    assert(sAnimatedProvider == NtmvM2dItemsPanel_BuildTocRow);
    assert(sAnimatedContext == &fixture->toc_data);
    assert(sTickedButtonCount == 4);
    for (index = 0; index < 4; ++index) {
        assert(sTickedButtons[index] == &fixture->controls[index]);
    }
}

static void RunVerticalUpdateCase(
    TestUpdateFixture *fixture,
    bool active,
    int16_t initial_y,
    int16_t expected_y,
    bool content_moved)
{
    fixture->panel.secondary_state_active = active;
    fixture->panel.content_panel.base.local_position.y = initial_y;
    fixture->secondary.base.base.local_position.x = 7;
    fixture->panel.item_panel_target_x = 7;
    fixture->panel.window_origins[0].x = 9;
    fixture->panel.target_window_x = 9;
    fixture->panel.state_ae = 0;
    ResetUpdateTest(fixture, false);

    NtmvM2dManualTocPanel_Update(&fixture->panel);

    assert(fixture->panel.content_panel.base.local_position.y == expected_y);
    assert(fixture->panel.footer_panel.base.local_position.y == 0xec - expected_y);
    assert(sUpdatePositionCount == (content_moved ? 2u : 1u));
    if (content_moved) {
        assert(sUpdatePositionElements[0] == &fixture->panel.content_panel.base);
        assert(sUpdatePositions[0].x == 11);
        assert(sUpdatePositions[0].y == expected_y);
    }
    assert(
        sUpdatePositionElements[content_moved ? 1 : 0] ==
        &fixture->panel.footer_panel.base);
    assert(sUpdatePositions[content_moved ? 1 : 0].x == 22);
    assert(sUpdatePositions[content_moved ? 1 : 0].y == 0xec - expected_y);
    assert(sUpdateVisibilityCount == 0);
    AssertUpdateCommon(fixture);
}

static void TestUpdateVerticalPhase(void)
{
    TestUpdateFixture fixture;

    InitializeUpdateFixture(&fixture);
    RunVerticalUpdateCase(&fixture, true, 50, 18, true);
    RunVerticalUpdateCase(&fixture, true, 10, 0, true);
    RunVerticalUpdateCase(&fixture, true, 0, 0, false);
    RunVerticalUpdateCase(&fixture, false, 100, 132, true);
    RunVerticalUpdateCase(&fixture, false, 140, 150, true);
    RunVerticalUpdateCase(&fixture, false, 150, 150, false);
    sTestingUpdate = false;
}

static void TestUpdateHorizontalPhases(void)
{
    TestUpdateFixture fixture;

    InitializeUpdateFixture(&fixture);
    fixture.panel.content_panel.base.local_position.y = 0;
    fixture.secondary.base.base.local_position.x = 0;
    fixture.panel.item_panel_target_x = 100;
    fixture.panel.window_origins[0].x = -100;
    fixture.panel.target_window_x = 50;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(fixture.secondary.base.base.local_position.x == 32);
    assert(fixture.primary.base.base.local_position.x == -105);
    assert(fixture.panel.window_origins[0].x == -68);
    assert(sUpdatePositionCount == 3);
    assert(sUpdatePositionElements[0] == &fixture.panel.footer_panel.base);
    assert(sUpdatePositionElements[1] == &fixture.secondary.base.base);
    assert(sUpdatePositionElements[2] == &fixture.primary.base.base);
    AssertUpdateCommon(&fixture);

    /* Reaching the secondary target recalculates the window target. */
    fixture.secondary.base.base.local_position.x = 120;
    fixture.panel.item_panel_target_x = 100;
    fixture.panel.mode = NTMV_M2D_MANUAL_TOC_MODE_CENTER;
    fixture.panel.target_window_x = 77;
    fixture.panel.window_origins[0].x = 0;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(fixture.secondary.base.base.local_position.x == 100);
    assert(fixture.primary.base.base.local_position.x == -37);
    assert(fixture.panel.target_window_x == 0);
    assert(fixture.panel.window_origins[0].x == 0);
    AssertUpdateCommon(&fixture);

    /* Primary X is capped at zero while secondary is still moving right. */
    fixture.secondary.base.base.local_position.x = 150;
    fixture.panel.item_panel_target_x = 200;
    fixture.panel.target_window_x = 12;
    fixture.panel.window_origins[0].x = 12;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(fixture.secondary.base.base.local_position.x == 182);
    assert(fixture.primary.base.base.local_position.x == 0);
    assert(fixture.panel.target_window_x == 12);
    AssertUpdateCommon(&fixture);

    /* Equal list positions skip phase 2; window X still advances left. */
    fixture.secondary.base.base.local_position.x = 50;
    fixture.panel.item_panel_target_x = 50;
    fixture.panel.target_window_x = -50;
    fixture.panel.window_origins[0].x = 100;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(fixture.secondary.base.base.local_position.x == 50);
    assert(fixture.panel.window_origins[0].x == 68);
    assert(sUpdatePositionCount == 1);
    assert(sUpdatePositionElements[0] == &fixture.panel.footer_panel.base);
    AssertUpdateCommon(&fixture);

    fixture.panel.target_window_x = 20;
    fixture.panel.window_origins[0].x = 10;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(fixture.panel.window_origins[0].x == 20);
    AssertUpdateCommon(&fixture);
    sTestingUpdate = false;
}

static void TestUpdateScrollVisibilityAndTicks(void)
{
    TestUpdateFixture fixture;

    InitializeUpdateFixture(&fixture);
    fixture.panel.content_panel.base.local_position.y = 0;
    fixture.secondary.base.base.local_position.x = 0;
    fixture.panel.item_panel_target_x = 0;
    fixture.panel.window_origins[0].x = 0;
    fixture.panel.target_window_x = 0;
    fixture.panel.state_ae = 1;
    ResetUpdateTest(&fixture, false);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(sUpdateVisibilityCount == 0);
    AssertUpdateCommon(&fixture);

    fixture.panel.state_ae = 0;
    ResetUpdateTest(&fixture, true);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(sUpdateVisibilityCount == 0);
    AssertUpdateCommon(&fixture);

    fixture.panel.state_ae = 1;
    fixture.primary.scroll_y = 24;
    fixture.primary.target_scroll_y = 24;
    fixture.selection_marker.flags = NTMV_M2D_UI_HIDDEN;
    fixture.secondary.base.base.flags = NTMV_M2D_UI_HIDDEN;
    ResetUpdateTest(&fixture, true);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(sUpdateVisibilityCount == 2);
    assert(sUpdateVisibilityElements[0] == &fixture.selection_marker);
    assert(sUpdateVisibilityElements[1] == &fixture.secondary.base.base);
    assert(sUpdateVisibilityValues[0] && sUpdateVisibilityValues[1]);
    assert((fixture.selection_marker.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((fixture.secondary.base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    AssertUpdateCommon(&fixture);

    fixture.primary.scroll_y = 12;
    fixture.primary.target_scroll_y = 24;
    ResetUpdateTest(&fixture, true);
    NtmvM2dManualTocPanel_Update(&fixture.panel);
    assert(sUpdateVisibilityCount == 2);
    assert(!sUpdateVisibilityValues[0] && !sUpdateVisibilityValues[1]);
    assert((fixture.selection_marker.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((fixture.secondary.base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    AssertUpdateCommon(&fixture);
    sTestingUpdate = false;
}

static void ResetWindowPublishTest(uint32_t first_read, uint32_t second_read)
{
    sDisplayReadValues[0] = first_read;
    sDisplayReadValues[1] = second_read;
    sDisplayReadCount = 0;
    sDisplayWrittenValue = 0;
    sDisplayWriteCount = 0;
    memset(sWindowWriteIndices, 0, sizeof(sWindowWriteIndices));
    memset(sWindowWrittenValues, 0, sizeof(sWindowWrittenValues));
    sWindowWriteCount = 0;
    sTestingWindowPublish = true;
}

static void TestPublishWindowRegisters(void)
{
    NtmvM2dManualTocPanel panel;
    uint32_t second_read;

    memset(&panel, 0, sizeof(panel));
    panel.window_origins[0].x = -48;
    panel.window_origins[0].y = 0;
    panel.window_origins[1].x = 137;
    panel.window_origins[1].y = 59;
    panel.window_origins[2].x = -2;
    panel.window_origins[2].y = 192;

    /* Visible panels enable window bits 8..10.  Bits 11..12 come from the
     * first volatile read while every unrelated bit comes from the second. */
    second_read = 0xa5a5e5aa;
    ResetWindowPublishTest(0x12341800, second_read);
    NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(&panel);
    assert(sDisplayReadCount == 2);
    assert(sDisplayWriteCount == 1);
    assert(
        sDisplayWrittenValue ==
        ((second_read & ~0x1f00u) | 0x1f00u));
    assert(sWindowWriteCount == 3);
    assert(
        sWindowWriteIndices[0] == 0 && sWindowWriteIndices[1] == 1 &&
        sWindowWriteIndices[2] == 2);
    assert(sWindowWrittenValues[0] == 0x00000030);
    assert(sWindowWrittenValues[1] == 0x01c50177);
    assert(sWindowWrittenValues[2] == 0x01400002);

    /* Hidden panels clear only window bits 8..10 and preserve bits 11..12. */
    panel.base.flags = NTMV_M2D_UI_HIDDEN;
    second_read = 0x5a5a1fff;
    ResetWindowPublishTest(0x00001f00, second_read);
    NtmvM2dManualTocPanel_PublishWindowRegisters_020baef4(&panel);
    assert(sDisplayReadCount == 2);
    assert(sDisplayWriteCount == 1);
    assert(
        sDisplayWrittenValue ==
        ((second_read & ~0x1f00u) | 0x1800u));
    assert(sWindowWriteCount == 3);
    assert(sWindowWrittenValues[0] == 0x00000030);
    assert(sWindowWrittenValues[1] == 0x01c50177);
    assert(sWindowWrittenValues[2] == 0x01400002);
    sTestingWindowPublish = false;
}

static void ResetRefreshPrimaryTest(NtmvM2dManualTocPanel *panel)
{
    sTestingRefreshPrimary = true;
    sRefreshPanel = panel;
    memset(sRefreshedRows, 0, sizeof(sRefreshedRows));
    sRefreshedRowCount = UINT16_MAX;
    sRefreshedFirstItem = INT16_MIN;
    sRefreshRowsCallCount = 0;
}

static void TestRefreshPrimaryRows(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsTocData toc_data;
    NtmvM2dItemsTocRecord records[8];
    uint16_t texts[8][2];
    static const uint16_t empty_source[] = {0};
    static const uint16_t placeholder[] = {'(', '-', '-', '-', ')', 0};
    unsigned int index;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(records, 0, sizeof(records));
    for (index = 0; index < 8; ++index) {
        texts[index][0] = (uint16_t)('A' + index);
        texts[index][1] = 0;
        records[index].text = texts[index];
        records[index].child_count = (uint16_t)(index & 1u);
    }
    records[3].text = empty_source;
    toc_data.records = records;
    toc_data.record_count = 0;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;

    /* Empty data always forwards zero rows and first item zero. */
    primary.first_visible_item = 4;
    primary.visible_row_count_minus_one = 5;
    ResetRefreshPrimaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(&panel);
    assert(sRefreshRowsCallCount == 1);
    assert(sRefreshedRowCount == 0);
    assert(sRefreshedFirstItem == 0);

    /* A full viewport is bounded by visible_count_minus_one + 1. */
    toc_data.record_count = 8;
    primary.first_visible_item = 2;
    primary.visible_row_count_minus_one = 3;
    ResetRefreshPrimaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(&panel);
    assert(sRefreshRowsCallCount == 1);
    assert(sRefreshedRowCount == 4);
    assert(sRefreshedFirstItem == 2);
    assert(sRefreshedRows[0].text == texts[2]);
    assert(memcmp(sRefreshedRows[1].text, placeholder, sizeof(placeholder)) == 0);
    assert(sRefreshedRows[2].text == texts[4]);
    assert(sRefreshedRows[3].text == texts[5]);
    assert(sRefreshedRows[0].use_alternate_animation == 0);
    assert(sRefreshedRows[1].use_alternate_animation == 1);
    assert(sRefreshedRows[2].use_alternate_animation == 0);
    assert(sRefreshedRows[3].use_alternate_animation == 1);

    /* Near the end, remaining records bound the populated count. */
    toc_data.record_count = 5;
    primary.first_visible_item = 3;
    primary.visible_row_count_minus_one = 5;
    ResetRefreshPrimaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshPrimaryRows_020bafb4(&panel);
    assert(sRefreshRowsCallCount == 1);
    assert(sRefreshedRowCount == 2);
    assert(sRefreshedFirstItem == 3);
    assert(memcmp(sRefreshedRows[0].text, placeholder, sizeof(placeholder)) == 0);
    assert(sRefreshedRows[1].text == texts[4]);
    sTestingRefreshPrimary = false;
}

static void ResetRefreshSecondaryTest(NtmvM2dManualTocPanel *panel)
{
    sTestingRefreshSecondary = true;
    sSecondaryRefreshPanel = panel;
    memset(sRefreshedSecondaryRows, 0, sizeof(sRefreshedSecondaryRows));
    sRefreshedSecondaryRowCount = UINT16_MAX;
    sRefreshedSecondaryFirstItem = INT16_MIN;
    sRefreshSecondaryRowsCallCount = 0;
}

static void TestRefreshSecondaryRows(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocData toc_data;
    NtmvM2dItemsTocRecord records[2];
    const uint16_t *child_texts[8];
    uint16_t texts[8][2];
    static const uint16_t empty_source[] = {0};
    static const uint16_t placeholder[] = {'(', '-', '-', '-', ')', 0};
    unsigned int index;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    for (index = 0; index < 8; ++index) {
        texts[index][0] = (uint16_t)('a' + index);
        texts[index][1] = 0;
        child_texts[index] = texts[index];
    }
    child_texts[3] = empty_source;
    records[1].child_texts = child_texts;
    records[1].child_count = 8;
    toc_data.records = records;
    toc_data.record_count = 2;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;

    /* No primary selection forwards an empty secondary list at item zero. */
    primary.selected_item = -1;
    secondary.first_visible_item = 4;
    secondary.visible_row_count_minus_one = 5;
    ResetRefreshSecondaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(&panel);
    assert(sRefreshSecondaryRowsCallCount == 1);
    assert(sRefreshedSecondaryRowCount == 0);
    assert(sRefreshedSecondaryFirstItem == 0);

    /* The selected record supplies a bounded child-text viewport. */
    primary.selected_item = 1;
    secondary.first_visible_item = 2;
    secondary.visible_row_count_minus_one = 3;
    ResetRefreshSecondaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(&panel);
    assert(sRefreshSecondaryRowsCallCount == 1);
    assert(sRefreshedSecondaryRowCount == 4);
    assert(sRefreshedSecondaryFirstItem == 2);
    assert(sRefreshedSecondaryRows[0].text == texts[2]);
    assert(
        memcmp(
            sRefreshedSecondaryRows[1].text,
            placeholder,
            sizeof(placeholder)) == 0);
    assert(sRefreshedSecondaryRows[2].text == texts[4]);
    assert(sRefreshedSecondaryRows[3].text == texts[5]);
    for (index = 0; index < 4; ++index) {
        assert(sRefreshedSecondaryRows[index].use_alternate_animation == 0);
    }

    /* Remaining child texts, rather than viewport capacity, bound the tail. */
    records[1].child_count = 5;
    secondary.first_visible_item = 3;
    secondary.visible_row_count_minus_one = 5;
    ResetRefreshSecondaryTest(&panel);
    NtmvM2dManualTocPanel_RefreshSecondaryRows_020bb080(&panel);
    assert(sRefreshSecondaryRowsCallCount == 1);
    assert(sRefreshedSecondaryRowCount == 2);
    assert(sRefreshedSecondaryFirstItem == 3);
    assert(
        memcmp(
            sRefreshedSecondaryRows[0].text,
            placeholder,
            sizeof(placeholder)) == 0);
    assert(sRefreshedSecondaryRows[1].text == texts[4]);
    sTestingRefreshSecondary = false;
}

static void ResetInitializeEmptySelectionTest(
    NtmvM2dManualTocPanel *panel)
{
    sTestingInitializeEmptySelection = true;
    sInitializePanel = panel;
    memset(sInitializeCalls, 0, sizeof(sInitializeCalls));
    sInitializeCallCount = 0;
    sInitializeSecondaryCount = UINT16_MAX;
    sInitializeSelectedItem = INT16_MIN;
}

static void TestInitializeEmptySelection(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord records[2];
    NtmvM2dItemsTocData toc_data;
    static const uint16_t child_text[] = {'x', 0};
    static const uint16_t *const child_texts[3] = {
        child_text, child_text, child_text,
    };
    static const int reset_calls[2] = {
        TEST_INITIALIZE_SET_COUNT,
        TEST_INITIALIZE_REFRESH,
    };
    static const int select_calls[4] = {
        TEST_INITIALIZE_SET_COUNT,
        TEST_INITIALIZE_REFRESH,
        TEST_INITIALIZE_SELECT,
        TEST_INITIALIZE_SYNC,
    };

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    records[1].child_texts = child_texts;
    toc_data.records = records;
    toc_data.record_count = 2;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    secondary.visible_row_count_minus_one = 2;

    /* No primary selection always rebuilds an empty secondary list. */
    primary.selected_item = -1;
    ResetInitializeEmptySelectionTest(&panel);
    NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(&panel);
    assert(sInitializeCallCount == 2);
    assert(memcmp(sInitializeCalls, reset_calls, sizeof(reset_calls)) == 0);
    assert(sInitializeSecondaryCount == 0);
    assert(secondary.item_count == 0);
    assert(secondary.selected_item == -1);
    assert(sInitializeSelectedItem == INT16_MIN);

    /* A nonzero record marker preserves the empty child selection. */
    primary.selected_item = 1;
    records[1].reserved_00 = 1;
    records[1].child_count = 3;
    ResetInitializeEmptySelectionTest(&panel);
    NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(&panel);
    assert(sInitializeCallCount == 2);
    assert(memcmp(sInitializeCalls, reset_calls, sizeof(reset_calls)) == 0);
    assert(sInitializeSecondaryCount == 3);
    assert(secondary.item_count == 3);
    assert(secondary.selected_item == -1);

    /* Marker zero and at least one child select child zero after refresh. */
    records[1].reserved_00 = 0;
    ResetInitializeEmptySelectionTest(&panel);
    NtmvM2dManualTocPanel_InitializeEmptySelection_020bb148(&panel);
    assert(sInitializeCallCount == 4);
    assert(memcmp(sInitializeCalls, select_calls, sizeof(select_calls)) == 0);
    assert(sInitializeSecondaryCount == 3);
    assert(sInitializeSelectedItem == 0);
    assert(secondary.selected_item == 0);
    sTestingInitializeEmptySelection = false;
}

static void ResetNavigationTest(
    NtmvM2dManualTocPanel *panel,
    const NtmvM2dItemsTocRecord *record,
    int32_t initial_child_index)
{
    sTestingNavigation = true;
    sNavigationPanel = panel;
    sNavigationRecord = record;
    sNavigationInitialChildIndex = initial_child_index;
    sNavigationInitialChildCallCount = 0;
}

static void TestUpdateNavigation(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord records[2];
    NtmvM2dItemsTocData toc_data;
    NtmvM2dUIElement previous;
    NtmvM2dUIElement next;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    NtmvM2dUIElement_Init(&previous);
    NtmvM2dUIElement_Init(&next);
    toc_data.records = records;
    toc_data.record_count = 2;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.controls[3] = &previous;
    panel.controls[4] = &next;

    /* No primary selection locks both controls without inspecting a record. */
    primary.selected_item = -1;
    previous.flags = NTMV_M2D_UI_HIDDEN;
    next.flags = NTMV_M2D_UI_HOVERED;
    ResetNavigationTest(&panel, &records[0], 0);
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(&panel);
    assert(sNavigationInitialChildCallCount == 0);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED));
    assert(next.flags ==
           (NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED));

    /* A later primary record enables next; child zero cannot go previous. */
    primary.selected_item = 0;
    secondary.selected_item = 0;
    records[0].child_count = 1;
    previous.flags = NTMV_M2D_UI_HIDDEN;
    next.flags = NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED;
    ResetNavigationTest(&panel, &records[0], 0);
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(&panel);
    assert(sNavigationInitialChildCallCount == 1);
    assert(previous.flags ==
           (NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED));
    assert(next.flags == NTMV_M2D_UI_HOVERED);

    /* On the last record, the child range controls both directions. */
    toc_data.record_count = 1;
    records[0].child_count = 4;
    secondary.selected_item = 1;
    previous.flags = NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED;
    next.flags = NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED;
    ResetNavigationTest(&panel, &records[0], 0);
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(&panel);
    assert(sNavigationInitialChildCallCount == 1);
    assert(previous.flags == NTMV_M2D_UI_HIDDEN);
    assert(next.flags == NTMV_M2D_UI_HOVERED);

    secondary.selected_item = 3;
    previous.flags = NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED;
    next.flags = NTMV_M2D_UI_HOVERED;
    ResetNavigationTest(&panel, &records[0], 0);
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(&panel);
    assert(sNavigationInitialChildCallCount == 1);
    assert(previous.flags == NTMV_M2D_UI_HIDDEN);
    assert(next.flags ==
           (NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED));

    /* Any primary record after zero enables previous without helper lookup. */
    toc_data.record_count = 2;
    primary.selected_item = 1;
    secondary.selected_item = -1;
    records[1].child_count = 0;
    previous.flags = NTMV_M2D_UI_HIDDEN | NTMV_M2D_UI_HIT_STATE_LOCKED;
    next.flags = NTMV_M2D_UI_HOVERED;
    ResetNavigationTest(&panel, &records[1], 0);
    NtmvM2dManualTocPanel_UpdateNavigation_020bb208(&panel);
    assert(sNavigationInitialChildCallCount == 0);
    assert(previous.flags == NTMV_M2D_UI_HIDDEN);
    assert(next.flags ==
           (NTMV_M2D_UI_HOVERED | NTMV_M2D_UI_HIT_STATE_LOCKED));
    sTestingNavigation = false;
}

static void ResetControlHandlerTest(NtmvM2dManualTocPanel *panel)
{
    sTestingControlHandler = true;
    sControlPanel = panel;
    memset(sSoundSequences, 0, sizeof(sSoundSequences));
    sSoundCount = 0;
}

static void TestHandleControls(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dManualButton controls[9];
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dUIElement mode_markers[2];
    NtmvM2dManualTocAction action;
    NtmvM2dItemsPointerState pointer;
    static const uint16_t expected_kinds[9] = {
        NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_3,
        NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_1,
        NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
        NTMV_M2D_MANUAL_TOC_ACTION_NONE,
    };
    unsigned int index;

    memset(&panel, 0, sizeof(panel));
    memset(&pointer, 0, sizeof(pointer));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dUIElement_Init(&mode_markers[0]);
    NtmvM2dUIElement_Init(&mode_markers[1]);
    panel.feedback_context = (void *)(uintptr_t)0x4444;
    panel.active_control_index = -1;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.mode_markers[0] = &mode_markers[0];
    panel.mode_markers[1] = &mode_markers[1];
    for (index = 0; index < 9; ++index) {
        NtmvM2dManualButton_Construct(&controls[index]);
        controls[index].base.base.world_center.x =
            (int16_t)(10 + index * 20);
        controls[index].base.base.world_center.y = 10;
        controls[index].base.base.size.width = 10;
        controls[index].base.base.size.height = 10;
        controls[index].base.base.flags = NTMV_M2D_UI_HIDDEN;
        panel.controls[index] = &controls[index].base.base;
    }

    /* Inactive input and an active press outside every control are ignored. */
    ResetControlHandlerTest(&panel);
    action.kind = UINT16_MAX;
    action.value = UINT16_MAX;
    assert(!NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert(action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(action.value == 0);

    pointer.active = 1;
    pointer.pressed = 1;
    pointer.position.x = -20;
    pointer.position.y = -20;
    assert(!NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert(panel.active_control_index == -1);

    /* An unlocked press captures the control, plays feedback zero and tracks
     * hover until the pointer leaves its rectangle. */
    controls[2].base.base.flags = 0;
    pointer.position = controls[2].base.base.world_center;
    assert(NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert(panel.active_control_index == 2);
    assert((controls[2].base.base.flags & NTMV_M2D_UI_HOVERED) != 0);
    assert(sSoundCount == 1 && sSoundSequences[0] == 0);

    pointer.pressed = 0;
    pointer.position.x = -20;
    pointer.position.y = -20;
    assert(NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert((controls[2].base.base.flags & NTMV_M2D_UI_HOVERED) == 0);

    /* Locked controls 0..5 play rejection feedback; 6..8 stay silent. */
    panel.active_control_index = -1;
    controls[2].base.base.flags = NTMV_M2D_UI_HIDDEN;
    controls[4].base.base.flags = NTMV_M2D_UI_HIT_STATE_LOCKED;
    pointer.pressed = 1;
    pointer.position = controls[4].base.base.world_center;
    ResetControlHandlerTest(&panel);
    assert(NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert(panel.active_control_index == -1);
    assert(sSoundCount == 1 && sSoundSequences[0] == 5);

    controls[4].base.base.flags = NTMV_M2D_UI_HIDDEN;
    controls[7].base.base.flags = NTMV_M2D_UI_HIT_STATE_LOCKED;
    pointer.position = controls[7].base.base.world_center;
    ResetControlHandlerTest(&panel);
    assert(NtmvM2dManualTocPanel_HandleControls_020bb290(
        &panel, &action, &pointer));
    assert(sSoundCount == 0);

    /* Release dispatch covers all nine jump-table entries. */
    pointer.active = 0;
    pointer.pressed = 0;
    pointer.released = 1;
    for (index = 0; index < 9; ++index) {
        controls[index].base.base.flags = NTMV_M2D_UI_HOVERED;
        panel.active_control_index = (int16_t)index;
        panel.state_ac = 0;
        panel.secondary_state_active =
            (uint8_t)(index == 3 || index == 4);
        secondary.base.base.local_position.x =
            (int16_t)(index == 7 ? 0x89 : 0);
        ResetControlHandlerTest(&panel);
        action.kind = UINT16_MAX;
        action.value = UINT16_MAX;
        assert(NtmvM2dManualTocPanel_HandleControls_020bb290(
            &panel, &action, &pointer));
        assert(panel.active_control_index == -1);
        assert(
            (controls[index].base.base.flags & NTMV_M2D_UI_HOVERED) == 0);
        assert(action.kind == expected_kinds[index]);
        assert(action.value == (uint16_t)(index == 0 || index == 2));
        if (index == 7) {
            assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_LEFT);
        } else if (index == 8) {
            assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_CENTER);
        }
    }
    sTestingControlHandler = false;
}

static void ResetMoveHelperTest(
    NtmvM2dManualTocPanel *panel, int32_t initial_child_index)
{
    sTestingMoveHelpers = true;
    sMovePanel = panel;
    sMoveInitialChildIndex = initial_child_index;
    sMovePrimarySelectionCount = 0;
    sMoveSecondarySelectionCount = 0;
    sMoveSecondaryResetCount = 0;
    sMoveSecondaryRefreshCount = 0;
    memset(sSoundSequences, 0, sizeof(sSoundSequences));
    sSoundCount = 0;
    ((NtmvM2dObjButton *)panel->controls[3])->active_timer = 0;
    ((NtmvM2dObjButton *)panel->controls[4])->active_timer = 0;
}

static void TestMoveHelpers(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord records[3];
    NtmvM2dItemsTocData toc_data;
    NtmvM2dManualButton interaction;
    NtmvM2dObjButton previous;
    NtmvM2dObjButton next;
    NtmvM2dUIElement selection_marker;
    NtmvM2dUIElement mode_markers[2];
    static const uint16_t child_text[] = {'x', 0};
    static const uint16_t *const child_texts[3] = {
        child_text, child_text, child_text,
    };
    unsigned int index;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    memset(&previous, 0, sizeof(previous));
    memset(&next, 0, sizeof(next));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dManualButton_Construct(&interaction);
    NtmvM2dUIElement_Init(&selection_marker);
    NtmvM2dUIElement_Init(&mode_markers[0]);
    NtmvM2dUIElement_Init(&mode_markers[1]);
    for (index = 0; index < 3; ++index) {
        records[index].reserved_00 = 1;
        records[index].child_texts = child_texts;
    }
    toc_data.records = records;
    toc_data.record_count = 3;
    toc_data.reserved_06 = 0;
    panel.toc_data = &toc_data;
    panel.feedback_context = (void *)(uintptr_t)0x4444;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.controls[0] = &interaction.base.base;
    panel.controls[3] = &previous.base;
    panel.controls[4] = &next.base;
    panel.selection_marker = &selection_marker;
    panel.mode_markers[0] = &mode_markers[0];
    panel.mode_markers[1] = &mode_markers[1];
    secondary.base.base.local_position.x = 0x89;

    /* Secondary mode blocks both directions without feedback. */
    panel.secondary_state_active = 1;
    ResetMoveHelperTest(&panel, 0);
    assert(NtmvM2dManualTocPanel_MovePrevious_020bb450(&panel) == 0);
    assert(NtmvM2dManualTocPanel_MoveNext_020bb56c(&panel) == 0);
    assert(sSoundCount == 0);

    /* No primary selection cannot move previous and plays rejection 5. */
    panel.secondary_state_active = 0;
    primary.selected_item = -1;
    ResetMoveHelperTest(&panel, 0);
    assert(NtmvM2dManualTocPanel_MovePrevious_020bb450(&panel) == 0);
    assert(sSoundCount == 1 && sSoundSequences[0] == 5);

    /* Within one record, previous selects the preceding child. */
    records[0].child_count = 3;
    primary.selected_item = 0;
    secondary.selected_item = 2;
    ResetMoveHelperTest(&panel, 1);
    assert(
        NtmvM2dManualTocPanel_MovePrevious_020bb450(&panel) ==
        NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM);
    assert(primary.selected_item == 0 && secondary.selected_item == 1);
    assert(sMovePrimarySelectionCount == 0);
    assert(sMoveSecondarySelectionCount == 1);
    assert(panel.state_ac == 0);
    assert(previous.active_timer == 10);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);

    /* At a record boundary, previous selects the prior record's last child. */
    records[0].child_count = 2;
    records[1].child_count = 0;
    primary.selected_item = 1;
    secondary.selected_item = -1;
    ResetMoveHelperTest(&panel, 0);
    assert(
        NtmvM2dManualTocPanel_MovePrevious_020bb450(&panel) ==
        NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM);
    assert(primary.selected_item == 0 && secondary.selected_item == 1);
    assert(sMovePrimarySelectionCount == 1);
    assert(sMoveSecondarySelectionCount == 1);
    assert(sMoveSecondaryResetCount == 1);
    assert(sMoveSecondaryRefreshCount == 1);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(previous.active_timer == 10);

    /* From no selection, next selects primary zero. */
    records[0].child_count = 0;
    primary.selected_item = -1;
    secondary.selected_item = -1;
    ResetMoveHelperTest(&panel, 0);
    assert(
        NtmvM2dManualTocPanel_MoveNext_020bb56c(&panel) ==
        NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM);
    assert(primary.selected_item == 0 && secondary.selected_item == -1);
    assert(sMovePrimarySelectionCount == 1);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(next.active_timer == 10);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);

    /* Within a child range, next advances only the secondary selection. */
    records[0].child_count = 3;
    primary.selected_item = 0;
    secondary.selected_item = 0;
    ResetMoveHelperTest(&panel, 0);
    assert(
        NtmvM2dManualTocPanel_MoveNext_020bb56c(&panel) ==
        NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM);
    assert(primary.selected_item == 0 && secondary.selected_item == 1);
    assert(sMovePrimarySelectionCount == 0);
    assert(sMoveSecondarySelectionCount == 1);
    assert(next.active_timer == 10);

    /* The last child advances to the next primary record. */
    records[1].child_count = 0;
    primary.selected_item = 0;
    secondary.selected_item = 2;
    ResetMoveHelperTest(&panel, 0);
    assert(
        NtmvM2dManualTocPanel_MoveNext_020bb56c(&panel) ==
        NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM);
    assert(primary.selected_item == 1 && secondary.selected_item == -1);
    assert(sMovePrimarySelectionCount == 1);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);

    /* End of the final record is rejected with feedback 5. */
    primary.selected_item = 2;
    secondary.selected_item = -1;
    records[2].child_count = 0;
    ResetMoveHelperTest(&panel, 0);
    assert(NtmvM2dManualTocPanel_MoveNext_020bb56c(&panel) == 0);
    assert(sSoundCount == 1 && sSoundSequences[0] == 5);
    sTestingMoveHelpers = false;
}

static void ResetModeHelperTest(NtmvM2dManualTocPanel *panel)
{
    sTestingModeHelpers = true;
    sModePanel = panel;
    memset(sSoundSequences, 0, sizeof(sSoundSequences));
    sSoundCount = 0;
}

static void TestModeHelpers(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dManualButton guide_button;
    NtmvM2dManualButton secondary_button;
    NtmvM2dObjButton center_button;
    uint8_t show_guide;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(&center_button, 0, sizeof(center_button));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dManualButton_Construct(&guide_button);
    NtmvM2dManualButton_Construct(&secondary_button);
    panel.feedback_context = (void *)(uintptr_t)0x4444;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.controls[0] = &guide_button.base.base;
    panel.controls[1] = &center_button.base;
    panel.controls[2] = &secondary_button.base.base;
    panel.item_panel_target_x = secondary.base.base.local_position.x;
    primary.scroll_y = primary.target_scroll_y;

    ResetModeHelperTest(&panel);
    assert(NtmvM2dManualTocPanel_ToggleGuideState_020bb660(&panel));
    assert(panel.state_ac == 1);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);
    ResetModeHelperTest(&panel);
    assert(!NtmvM2dManualTocPanel_ToggleGuideState_020bb660(&panel));
    assert(panel.state_ac == 0);

    show_guide = 0x7f;
    ResetModeHelperTest(&panel);
    panel.content_panel.base.local_position.y = 1;
    assert(!NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
        &panel, &show_guide));
    assert(show_guide == 0x7f);
    assert(sSoundCount == 0);

    ResetModeHelperTest(&panel);
    panel.content_panel.base.local_position.y = 0;
    assert(NtmvM2dManualTocPanel_ToggleGuideRequest_020bb698(
        &panel, &show_guide));
    assert(show_guide == 1 && panel.state_ac == 1);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);

    panel.secondary_state_active = 0;
    ResetModeHelperTest(&panel);
    assert(NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(&panel));
    assert(panel.secondary_state_active == 1);
    assert(secondary_button.base.active_timer == 10);
    assert(sSoundCount == 1 && sSoundSequences[0] == 8);
    ResetModeHelperTest(&panel);
    assert(!NtmvM2dManualTocPanel_ToggleSecondaryState_020bb6c8(&panel));
    assert(panel.secondary_state_active == 0);

    center_button.active_timer = 0;
    ResetModeHelperTest(&panel);
    NtmvM2dManualTocPanel_ActivateControl1_020bb70c(&panel);
    assert(center_button.active_timer == 10);
    assert(sSoundCount == 1 && sSoundSequences[0] == 4);
    sTestingModeHelpers = false;
}

static void TestIsAnimating(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.item_panel_target_x = 0x89;
    secondary.base.base.local_position.x = 0x89;

    panel.content_panel.base.local_position.y = 1;
    assert(NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));
    panel.content_panel.base.local_position.y = 0x95;
    assert(NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));

    panel.content_panel.base.local_position.y = 0;
    assert(!NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));
    panel.content_panel.base.local_position.y = 0x96;
    assert(!NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));

    panel.item_panel_target_x = 0;
    assert(NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));
    panel.item_panel_target_x = 0x89;
    primary.target_scroll_y = 1;
    assert(NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));
    primary.scroll_y = 1;
    assert(!NtmvM2dManualTocPanel_IsAnimating_020bb730(&panel));
}

static void TestSetMode(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel secondary;
    NtmvM2dUIElement mode_markers[2];

    memset(&panel, 0, sizeof(panel));
    memset(&secondary, 0, sizeof(secondary));
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dUIElement_Init(&mode_markers[0]);
    NtmvM2dUIElement_Init(&mode_markers[1]);
    panel.feedback_context = (void *)(uintptr_t)0x4444;
    panel.item_panels[1] = &secondary.base.base;
    panel.mode_markers[0] = &mode_markers[0];
    panel.mode_markers[1] = &mode_markers[1];

    secondary.base.base.local_position.x = 0x89;
    NtmvM2dManualTocPanel_SetMode_020bb788(
        &panel, NTMV_M2D_MANUAL_TOC_MODE_LEFT);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_LEFT);
    assert(panel.item_panel_target_x == 0x89);
    assert(panel.target_window_x == -0x60);
    assert((mode_markers[0].flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((mode_markers[1].flags & NTMV_M2D_UI_HIDDEN) == 0);

    secondary.base.base.local_position.x = 0;
    NtmvM2dManualTocPanel_SetMode_020bb788(
        &panel, NTMV_M2D_MANUAL_TOC_MODE_CENTER);
    assert(panel.item_panel_target_x == 0);
    assert(panel.target_window_x == 0);
    assert((mode_markers[0].flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((mode_markers[1].flags & NTMV_M2D_UI_HIDDEN) != 0);

    secondary.base.base.local_position.x = 0x89;
    NtmvM2dManualTocPanel_SetMode_020bb788(
        &panel, NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(panel.item_panel_target_x == 0x89);
    assert(panel.target_window_x == -0x30);
    assert((mode_markers[0].flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((mode_markers[1].flags & NTMV_M2D_UI_HIDDEN) != 0);

    panel.item_panel_target_x = 0x89;
    panel.target_window_x = 123;
    NtmvM2dManualTocPanel_SetMode_020bb788(&panel, 9);
    assert(panel.mode == 9);
    assert(panel.item_panel_target_x == 0x89);
    assert(panel.target_window_x == 123);

    ResetModeHelperTest(&panel);
    secondary.base.base.local_position.x = 0;
    panel.target_window_x = 77;
    NtmvM2dManualTocPanel_SetMode_020bb788(
        &panel, NTMV_M2D_MANUAL_TOC_MODE_LEFT);
    assert(panel.target_window_x == 77);
    assert(sSoundCount == 1 && sSoundSequences[0] == 3);
    sTestingModeHelpers = false;
}

static void ResetSecondaryModeTest(NtmvM2dManualTocPanel *panel)
{
    sTestingSecondaryMode = true;
    sSecondaryModePanel = panel;
    sSecondaryModeFrameCount = 0;
    sSecondaryModeFrame = UINT32_MAX;
}

static void TestUpdateSecondaryMode(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dItemsTocRecord records[2];
    NtmvM2dItemsTocData toc_data;
    NtmvM2dUIStatic selection_marker;
    NtmvM2dUIElement mode_markers[2];

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    memset(records, 0, sizeof(records));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dUIStatic_ConstructComplete(&selection_marker);
    NtmvM2dUIElement_Init(&mode_markers[0]);
    NtmvM2dUIElement_Init(&mode_markers[1]);
    toc_data.records = records;
    toc_data.record_count = 2;
    toc_data.reserved_06 = 0;
    records[1].child_count = 2;
    panel.toc_data = &toc_data;
    panel.render_resources = (void *)(uintptr_t)0x3333;
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.selection_marker = &selection_marker.base;
    panel.mode_markers[0] = &mode_markers[0];
    panel.mode_markers[1] = &mode_markers[1];
    primary.selected_item = 1;
    primary.first_visible_item = 0;
    primary.visible_row_count_minus_one = 3;
    selection_marker.base.local_position.x = 11;

    secondary.base.base.local_position.x = 0;
    ResetSecondaryModeTest(&panel);
    NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(&panel, false);
    assert(panel.state_ae == 1);
    assert(selection_marker.base.local_position.x == 11);
    assert(selection_marker.base.local_position.y == 0x2e);
    assert(sSecondaryModeFrameCount == 1 && sSecondaryModeFrame == 0);
    assert((selection_marker.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert((secondary.base.base.flags & NTMV_M2D_UI_HIDDEN) == 0);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_CENTER);

    secondary.base.base.local_position.x = 0x89;
    ResetSecondaryModeTest(&panel);
    NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(&panel, true);
    assert(panel.state_ae == 1);
    assert(sSecondaryModeFrameCount == 1 && sSecondaryModeFrame == 0);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_LEFT);

    primary.selected_item = -1;
    secondary.base.base.local_position.x = 0x89;
    ResetSecondaryModeTest(&panel);
    NtmvM2dManualTocPanel_UpdateSecondaryMode_020bb898(&panel, false);
    assert(panel.state_ae == 0);
    assert(sSecondaryModeFrameCount == 0);
    assert((selection_marker.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((secondary.base.base.flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    sTestingSecondaryMode = false;
}

static void ResetInteractionModeTest(NtmvM2dManualTocPanel *panel)
{
    sTestingInteractionMode = true;
    sInteractionPanel = panel;
    memset(sInteractionPanels, 0, sizeof(sInteractionPanels));
    memset(sInteractionValues, 0, sizeof(sInteractionValues));
    sInteractionCallCount = 0;
}

static void TestInteractionMode(void)
{
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dManualButton control;

    memset(&panel, 0, sizeof(panel));
    memset(&primary, 0, sizeof(primary));
    memset(&secondary, 0, sizeof(secondary));
    NtmvM2dItemsPanel_Construct(&primary);
    NtmvM2dItemsPanel_Construct(&secondary);
    NtmvM2dManualButton_Construct(&control);
    panel.item_panels[0] = &primary.base.base;
    panel.item_panels[1] = &secondary.base.base;
    panel.controls[0] = &control.base.base;

    primary.interaction_enabled = 1;
    secondary.interaction_enabled = 1;
    ResetInteractionModeTest(&panel);
    NtmvM2dManualTocPanel_SetInteractionMode_020bb980(&panel, true);
    assert(panel.state_ac == 1 && control.selected);
    assert(primary.interaction_enabled == 0);
    assert(secondary.interaction_enabled == 0);
    assert(sInteractionCallCount == 2);
    assert(sInteractionPanels[0] == &primary && !sInteractionValues[0]);
    assert(sInteractionPanels[1] == &secondary && !sInteractionValues[1]);

    ResetInteractionModeTest(&panel);
    NtmvM2dManualTocPanel_SetInteractionMode_020bb980(&panel, false);
    assert(panel.state_ac == 0 && !control.selected);
    assert(primary.interaction_enabled == 1);
    assert(secondary.interaction_enabled == 1);
    assert(sInteractionCallCount == 2);
    assert(sInteractionPanels[0] == &primary && sInteractionValues[0]);
    assert(sInteractionPanels[1] == &secondary && sInteractionValues[1]);
    sTestingInteractionMode = false;
}

typedef struct TestPointerFixture {
    NtmvM2dManualTocPanel panel;
    NtmvM2dItemsPanel primary;
    NtmvM2dItemsPanel secondary;
    NtmvM2dManualButton controls[9];
    NtmvM2dUIElement selection_marker;
    NtmvM2dUIElement mode_markers[2];
    NtmvM2dItemsTocRecord records[2];
    NtmvM2dItemsTocData toc_data;
    NtmvM2dItemsPointerState pointer;
    NtmvM2dManualTocAction action;
} TestPointerFixture;

static void InitializePointerFixture(TestPointerFixture *fixture)
{
    static const uint16_t child_text[] = {'x', 0};
    static const uint16_t *const child_texts[2] = {
        child_text, child_text,
    };
    unsigned int index;

    memset(fixture, 0, sizeof(*fixture));
    NtmvM2dItemsPanel_Construct(&fixture->primary);
    NtmvM2dItemsPanel_Construct(&fixture->secondary);
    NtmvM2dUIElement_Init(&fixture->selection_marker);
    NtmvM2dUIElement_Init(&fixture->mode_markers[0]);
    NtmvM2dUIElement_Init(&fixture->mode_markers[1]);
    fixture->toc_data.records = fixture->records;
    fixture->toc_data.record_count = 2;
    fixture->panel.toc_data = &fixture->toc_data;
    fixture->panel.feedback_context = (void *)(uintptr_t)0x4444;
    fixture->panel.item_panels[0] = &fixture->primary.base.base;
    fixture->panel.item_panels[1] = &fixture->secondary.base.base;
    fixture->panel.selection_marker = &fixture->selection_marker;
    fixture->panel.mode_markers[0] = &fixture->mode_markers[0];
    fixture->panel.mode_markers[1] = &fixture->mode_markers[1];
    for (index = 0; index < 9; ++index) {
        NtmvM2dManualButton_Construct(&fixture->controls[index]);
        fixture->panel.controls[index] = &fixture->controls[index].base.base;
    }
    fixture->primary.selected_item = 0;
    fixture->secondary.selected_item = 0;
    fixture->secondary.base.base.local_position.x = 0x89;
    fixture->panel.item_panel_target_x = 0x89;
    fixture->records[0].child_texts = child_texts;
    fixture->panel.content_panel.base.world_center.x = 50;
    fixture->panel.content_panel.base.world_center.y = 50;
    fixture->panel.content_panel.base.size.width = 100;
    fixture->panel.content_panel.base.size.height = 100;
    fixture->pointer.position.x = 50;
    fixture->pointer.position.y = 50;
}

static void ResetPointerInput(TestPointerFixture *fixture)
{
    memset(sInputCalls, 0, sizeof(sInputCalls));
    sInputCallCount = 0;
    sInputPanel = &fixture->panel;
    sInputPointer = &fixture->pointer;
    memset(sItemsHandled, 0, sizeof(sItemsHandled));
    memset(sItemsActions, 0, sizeof(sItemsActions));
    sInitialChildIndex = 0;
    sExpectedInitialChildRecord = &fixture->records[0];
    memset(sSoundSequences, 0, sizeof(sSoundSequences));
    sSoundCount = 0;
    sPointerEmptySelectionRefresh = false;
    fixture->panel.base.flags = 0;
    fixture->panel.active_control_index = -1;
    fixture->panel.content_panel.base.local_position.y = 0;
    fixture->panel.item_panel_target_x = 0x89;
    fixture->panel.state_ac = 1;
    fixture->primary.scroll_y = 0;
    fixture->primary.target_scroll_y = 0;
    fixture->secondary.base.base.local_position.x = 0x89;
    fixture->records[0].child_count = 0;
    fixture->primary.selected_item = 0;
    fixture->secondary.selected_item = 0;
    fixture->pointer.active = 0;
    fixture->pointer.pressed = 0;
    fixture->pointer.released = 0;
    fixture->action.kind = UINT16_MAX;
    fixture->action.value = UINT16_MAX;
    sTestingPointerInput = true;
}

static void AssertInputCalls(const int *expected, unsigned int count)
{
    assert(sInputCallCount == count);
    assert(memcmp(sInputCalls, expected, count * sizeof(expected[0])) == 0);
}

static bool HandleFixturePointer(TestPointerFixture *fixture)
{
    return NtmvM2dManualTocPanel_HandlePointer(
        &fixture->panel, &fixture->action, &fixture->pointer);
}

static void TestHandlePointerGuardsAndCapture(void)
{
    TestPointerFixture fixture;
    static const int controls_calls[] = {
        TEST_INPUT_SOUND,
    };
    static const int lists_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
    };

    InitializePointerFixture(&fixture);

    ResetPointerInput(&fixture);
    fixture.panel.base.flags = NTMV_M2D_UI_HIDDEN;
    assert(!HandleFixturePointer(&fixture));
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(fixture.action.value == 0);
    assert(sInputCallCount == 0);

    ResetPointerInput(&fixture);
    fixture.panel.content_panel.base.local_position.y = 1;
    assert(!HandleFixturePointer(&fixture));
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(fixture.action.value == 0);
    assert(sInputCallCount == 0);

    ResetPointerInput(&fixture);
    fixture.pointer.released = 1;
    fixture.panel.active_control_index = 2;
    fixture.controls[2].base.base.flags = NTMV_M2D_UI_HOVERED;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_CONTROL_2);
    assert(fixture.action.value == 1);
    assert(sSoundCount == 1 && sSoundSequences[0] == 8);
    AssertInputCalls(controls_calls, 1);

    ResetPointerInput(&fixture);
    assert(!HandleFixturePointer(&fixture));
    AssertInputCalls(lists_calls, 2);

    ResetPointerInput(&fixture);
    fixture.pointer.pressed = 1;
    assert(HandleFixturePointer(&fixture));
    AssertInputCalls(lists_calls, 2);

    ResetPointerInput(&fixture);
    fixture.pointer.released = 1;
    fixture.pointer.position.x = 101;
    assert(!HandleFixturePointer(&fixture));
    AssertInputCalls(lists_calls, 2);

    ResetPointerInput(&fixture);
    fixture.pointer.released = 1;
    fixture.pointer.position.x = 100;
    assert(HandleFixturePointer(&fixture));
    AssertInputCalls(lists_calls, 2);
}

static void TestHandlePointerPageAndControlActions(void)
{
    TestPointerFixture fixture;
    static const int primary_page_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SOUND,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_SOUND,
    };
    static const int secondary_page_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_REFRESH_SECONDARY,
        TEST_INPUT_SOUND,
    };
    static const int rejected_control_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_SOUND,
    };

    InitializePointerFixture(&fixture);

    ResetPointerInput(&fixture);
    sItemsHandled[0] = true;
    sItemsActions[0].page_changed = 1;
    sItemsActions[0].control_hit = 1;
    sItemsActions[0].control_pressed = 1;
    sItemsHandled[1] = true;
    sItemsActions[1].page_changed = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(sSoundCount == 2);
    assert(sSoundSequences[0] == 0);
    assert(sSoundSequences[1] == 3);
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    AssertInputCalls(primary_page_calls, 4);

    ResetPointerInput(&fixture);
    sItemsHandled[1] = true;
    sItemsActions[1].page_changed = 1;
    assert(HandleFixturePointer(&fixture));
    assert(sSoundCount == 1 && sSoundSequences[0] == 3);
    AssertInputCalls(secondary_page_calls, 4);

    ResetPointerInput(&fixture);
    sItemsHandled[1] = true;
    sItemsActions[1].control_hit = 1;
    sItemsActions[1].control_pressed = 0;
    assert(HandleFixturePointer(&fixture));
    assert(sSoundCount == 1 && sSoundSequences[0] == 5);
    AssertInputCalls(rejected_control_calls, 3);
}

static void TestHandlePointerSelectionActions(void)
{
    TestPointerFixture fixture;
    static const int primary_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_EMPTY_SELECTION,
        TEST_INPUT_INITIAL_CHILD,
        TEST_INPUT_SOUND,
    };
    static const int secondary_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_INITIAL_CHILD,
        TEST_INPUT_SOUND,
    };

    InitializePointerFixture(&fixture);

    ResetPointerInput(&fixture);
    sItemsHandled[0] = true;
    sItemsActions[0].selection_changed = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.panel.state_ac == 0);
    assert(fixture.panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM);
    assert(fixture.action.value == 0);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);
    AssertInputCalls(primary_calls, 5);

    ResetPointerInput(&fixture);
    sItemsHandled[1] = true;
    sItemsActions[1].selection_changed = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.panel.state_ac == 0);
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_SECONDARY_ITEM);
    assert(fixture.action.value == 0);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);
    AssertInputCalls(secondary_calls, 4);
}

static void TestHandlePointerActivationActions(void)
{
    TestPointerFixture fixture;
    static const int no_children_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_SOUND,
    };
    static const int same_child_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_INITIAL_CHILD,
    };
    static const int changed_child_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_INITIAL_CHILD,
        TEST_INPUT_EMPTY_SELECTION,
        TEST_INPUT_INITIAL_CHILD,
        TEST_INPUT_SOUND,
    };
    static const int secondary_calls[] = {
        TEST_INPUT_PRIMARY_POINTER,
        TEST_INPUT_SECONDARY_POINTER,
        TEST_INPUT_SOUND,
    };

    InitializePointerFixture(&fixture);

    ResetPointerInput(&fixture);
    sItemsHandled[0] = true;
    sItemsActions[0].row_activated = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.panel.state_ac == 0);
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(sSoundCount == 1 && sSoundSequences[0] == 1);
    AssertInputCalls(no_children_calls, 3);

    ResetPointerInput(&fixture);
    fixture.records[0].child_count = 2;
    fixture.secondary.selected_item = 0;
    sInitialChildIndex = 0;
    sItemsHandled[0] = true;
    sItemsActions[0].row_activated = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.panel.state_ac == 0);
    assert(fixture.panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(sSoundCount == 0);
    AssertInputCalls(same_child_calls, 3);

    ResetPointerInput(&fixture);
    fixture.records[0].child_count = 2;
    fixture.secondary.selected_item = 1;
    sInitialChildIndex = 0;
    sItemsHandled[0] = true;
    sItemsActions[0].row_activated = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_PRIMARY_ITEM);
    assert(sSoundCount == 1 && sSoundSequences[0] == 2);
    AssertInputCalls(changed_child_calls, 6);

    ResetPointerInput(&fixture);
    sItemsHandled[1] = true;
    sItemsActions[1].row_activated = 1;
    assert(HandleFixturePointer(&fixture));
    assert(fixture.action.kind == NTMV_M2D_MANUAL_TOC_ACTION_NONE);
    assert(sSoundCount == 1 && sSoundSequences[0] == 1);
    AssertInputCalls(secondary_calls, 3);

    sTestingPointerInput = false;
}

static void ResetConfigureRecords(void)
{
    sButtonConfigurationCount = 0;
    sItemsConfigurationCount = 0;
    sStaticConfigurationCount = 0;
    sCanvasConfigurationCount = 0;
    sDrawConfigurationCount = 0;
    sAnimationSequenceCount = 0;
    sCanvasClearCount = 0;
    sAuxiliaryCallCount = 0;
    sConfiguredScrollButton = NULL;
    sAlternateButton = NULL;
    sAlternateValue = true;
    sAlternateCallCount = 0;
}

static void TestConfigure(void)
{
    static const uint16_t text0[] = {'T', 'O', 'C', 0};
    static const uint16_t text1[] = {'B', 'A', 'C', 'K', 0};
    static const uint16_t text2[] = {'N', 'E', 'X', 'T', 0};
    static const uint16_t *const texts[3] = {text0, text1, text2};
    static const int16_t expected_items[2][9] = {
        {0, -36, 26, 25, 7, -30, 186, 5, 1},
        {137, -59, 34, -1, 13, 31, 186, 7, 2},
    };
    static const uint16_t expected_sequences[12] = {
        37, 20, 27, 24, 23, 21, 28, 22, 29, 38, 35, 36,
    };
    NtmvM2dItemsFontMetadata metadata;
    NtmvM2dItemsFontResource resource;
    NtmvM2dItemsFont font;
    NtmvM2dItemsTextContext text_context;
    NtmvM2dAllocatorContext allocator = {(void *)(uintptr_t)0x5678};
    NtmvM2dManualTocPanel panel;
    NtmvM2dUIPanel *controls_panel;
    unsigned int index;

    memset(&metadata, 0, sizeof(metadata));
    metadata.orientation = 3;
    memset(&resource, 0, sizeof(resource));
    resource.metadata = &metadata;
    font.resource = &resource;
    text_context.unknown_00 = NULL;
    text_context.font = &font;

    ResetConfigureRecords();
    NtmvM2dManualTocPanel_Construct(&panel);
    NtmvM2dManualTocPanel_Configure(
        &panel,
        &allocator,
        (void *)(uintptr_t)0x3333,
        &text_context,
        0x40,
        texts,
        (void *)(uintptr_t)0x4444);

    assert(panel.base.local_position.x == 0x80);
    assert(panel.base.local_position.y == 0x60);
    assert(panel.base.size.width == 0x100);
    assert(panel.base.size.height == 0xc0);
    assert(panel.render_resources == (void *)(uintptr_t)0x3333);
    assert(panel.feedback_context == (void *)(uintptr_t)0x4444);

    assert(sButtonConfigurationCount == 5);
    assert(sButtonConfigurations[0].position.x == -118);
    assert(sButtonConfigurations[0].position.y == 1);
    assert(sButtonConfigurations[0].size.width == 18);
    assert(sButtonConfigurations[0].size.height == 18);
    assert(sButtonConfigurations[0].animation_handle == 1037);
    assert(sButtonConfigurations[0].alternate_animation_handle == -1);
    assert(sButtonConfigurations[0].overlay_animation_handle == -1);
    assert(sButtonConfigurations[1].position.x == -60);
    assert(sButtonConfigurations[1].size.width == 93);
    assert(sButtonConfigurations[1].animation_handle == 1027);
    assert(sButtonConfigurations[1].overlay_animation_handle == 1020);
    assert(sButtonConfigurations[2].position.x == 6);
    assert(sButtonConfigurations[2].size.width == 22);
    assert(sButtonConfigurations[2].animation_handle == 1023);
    assert(sButtonConfigurations[2].alternate_animation_handle == 1024);
    assert(sButtonConfigurations[3].position.x == -56);
    assert(sButtonConfigurations[3].size.width == 64);
    assert(sButtonConfigurations[3].animation_handle == 1028);
    assert(sButtonConfigurations[3].overlay_animation_handle == 1021);
    assert(sButtonConfigurations[4].position.x == 56);
    assert(sButtonConfigurations[4].size.width == 64);
    assert(sButtonConfigurations[4].animation_handle == 1029);
    assert(sButtonConfigurations[4].overlay_animation_handle == 1022);
    for (index = 0; index < 5; ++index) {
        assert(panel.controls[index] ==
               &sButtonConfigurations[index].button->base.base);
    }

    assert(sAnimationSequenceCount == 12);
    assert(memcmp(
               sAnimationSequences,
               expected_sequences,
               sizeof(expected_sequences)) == 0);
    assert(sCanvasConfigurationCount == 3);
    assert(sCanvasClearCount == 3);
    assert(sCanvasConfigurations[0].vram ==
           (void *)(uintptr_t)0x06409c00);
    assert(sCanvasConfigurations[0].width_tiles == 12);
    assert(sCanvasConfigurations[0].height_tiles == 2);
    assert(sCanvasConfigurations[1].vram ==
           (void *)(uintptr_t)0x06409f00);
    assert(sCanvasConfigurations[1].width_tiles == 16);
    assert(sCanvasConfigurations[2].vram ==
           (void *)(uintptr_t)0x0640a300);
    assert(sCanvasConfigurations[2].width_tiles == 16);
    for (index = 0; index < 3; ++index) {
        assert(sCanvasConfigurations[index].color_depth == 4);
        assert(sDrawConfigurations[index].height == 16);
        assert(sDrawConfigurations[index].flags == 1);
        assert(sDrawConfigurations[index].draw_value == 0x480);
        assert(sDrawConfigurations[index].text == texts[index]);
        assert(sDrawConfigurations[index].direction.x == -1);
        assert(sDrawConfigurations[index].direction.y == 0);
    }
    assert(sDrawConfigurationCount == 3);
    assert(sDrawConfigurations[0].width == 96);
    assert(sDrawConfigurations[1].width == 128);
    assert(sDrawConfigurations[2].width == 128);

    assert(sItemsConfigurationCount == 2);
    for (index = 0; index < 2; ++index) {
        assert(memcmp(
                   sItemsConfigurations[index].definition,
                   expected_items[index],
                   sizeof(expected_items[index])) == 0);
        assert(panel.item_panels[index] ==
               &sItemsConfigurations[index].panel->base.base);
    }
    assert(sAuxiliaryCallCount == 3);
    assert(sAuxiliaryPanels[0] == sItemsConfigurations[1].panel);
    assert(sAuxiliaryIndices[0] == 0);
    assert(sAuxiliaryChildren[0] == panel.selection_marker);
    assert(sAuxiliaryPanels[1] == sItemsConfigurations[0].panel);
    assert(sAuxiliaryIndices[1] == 0);
    assert(sAuxiliaryChildren[1] == panel.mode_markers[0]);
    assert(sAuxiliaryPanels[2] == sItemsConfigurations[1].panel);
    assert(sAuxiliaryIndices[2] == 1);
    assert(sAuxiliaryChildren[2] == panel.mode_markers[1]);

    assert(panel.selection_marker == &sConfiguredStatics[0]->base);
    assert(panel.selection_marker->local_position.x == -0xa7);
    assert(panel.selection_marker->local_position.y == 0);
    assert(panel.selection_marker->size.width == 0x1a);
    assert(panel.selection_marker->size.height == 0xc0);
    assert((panel.selection_marker->flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert(sConfiguredStatics[0]->resource_id == 1038);
    assert(panel.mode_markers[0]->local_position.x == -30);
    assert(panel.mode_markers[0]->local_position.y == 36);
    assert(panel.mode_markers[1]->local_position.x == 31);
    assert(panel.mode_markers[1]->local_position.y == 59);
    assert(panel.mode_markers[0]->size.width == 0xba);
    assert(panel.mode_markers[0]->size.height == 0xc0);
    assert(panel.controls[7] == panel.mode_markers[0]);
    assert(panel.controls[8] == panel.mode_markers[1]);

    assert(panel.body_panel != NULL);
    assert(panel.body_panel->base.local_position.x == 0);
    assert(panel.body_panel->base.local_position.y == 0);
    assert(panel.body_panel->base.size.width == 0x189);
    assert(panel.body_panel->base.size.height == 0xc0);
    assert(panel.body_panel->child_count == 2);
    assert(panel.body_panel->children[0] == panel.item_panels[0]);
    assert(panel.body_panel->children[1] == panel.item_panels[1]);

    assert(sConfiguredScrollButton != NULL);
    assert(panel.scroll_button == &sConfiguredScrollButton->base.base);
    assert(sScrollPosition.x == 0x4b && sScrollPosition.y == 0);
    assert(sScrollPageSize == 0xae);
    assert(panel.controls[5] == &sConfiguredStatics[1]->base);
    assert(panel.controls[5]->size.width == 0x100);
    assert(panel.controls[5]->size.height == 0x17);
    assert((panel.controls[5]->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert(sConfiguredStatics[1]->resource_id == 1035);

    assert(panel.content_panel.base.local_position.x == 0);
    assert(panel.content_panel.base.local_position.y == 0x96);
    assert(panel.content_panel.base.size.width == 0x100);
    assert(panel.content_panel.base.size.height == 0xab);
    assert(panel.content_panel.child_count == 2);
    controls_panel = (NtmvM2dUIPanel *)panel.content_panel.children[0];
    assert(panel.content_panel.children[1] == &panel.body_panel->base);
    assert(controls_panel->base.local_position.x == 0);
    assert(controls_panel->base.local_position.y == -0x57);
    assert(controls_panel->base.size.width == 0x100);
    assert(controls_panel->base.size.height == 0x17);
    assert(controls_panel->child_count == 5);
    assert(controls_panel->children[0] == panel.controls[0]);
    assert(controls_panel->children[1] == panel.controls[1]);
    assert(controls_panel->children[2] == panel.controls[2]);
    assert(controls_panel->children[3] == panel.scroll_button);
    assert(controls_panel->children[4] == panel.controls[5]);

    assert(panel.controls[6] == &sConfiguredStatics[2]->base);
    assert(panel.controls[6]->size.width == 0x100);
    assert(panel.controls[6]->size.height == 0x15);
    assert((panel.controls[6]->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert(sConfiguredStatics[2]->resource_id == 1036);
    assert(panel.footer_panel.base.local_position.x == 0);
    assert(panel.footer_panel.base.local_position.y == 0x56);
    assert(panel.footer_panel.base.size.width == 0x100);
    assert(panel.footer_panel.base.size.height == 0x15);
    assert(panel.footer_panel.child_count == 3);
    assert(panel.footer_panel.children[0] == panel.controls[3]);
    assert(panel.footer_panel.children[1] == panel.controls[4]);
    assert(panel.footer_panel.children[2] == panel.controls[6]);

    assert((panel.controls[3]->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert((panel.controls[4]->flags & NTMV_M2D_UI_HIT_STATE_LOCKED) != 0);
    assert(panel.mode == NTMV_M2D_MANUAL_TOC_MODE_SPLIT);
    assert(panel.item_panel_target_x == 0x89);
    assert(panel.target_window_x == -0x30);
    assert((panel.mode_markers[0]->flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert((panel.mode_markers[1]->flags & NTMV_M2D_UI_HIDDEN) != 0);
    assert(panel.window_origins[0].x == -0x30);
    assert(panel.window_origins[0].y == 0);
    assert(panel.window_origins[1].x == 0x89);
    assert(panel.window_origins[1].y == 0x3b);
    assert(panel.secondary_state_active == 1);
    assert(sAlternateCallCount == 1);
    assert(sAlternateButton == sButtonConfigurations[2].button);
    assert(!sAlternateValue);

    ResetConfigureRecords();
    NtmvM2dManualTocPanel_Construct(&panel);
    NtmvM2dManualTocPanel_Configure(
        &panel,
        &allocator,
        (void *)(uintptr_t)0x3333,
        &text_context,
        0x41,
        texts,
        (void *)(uintptr_t)0x4444);
    assert(sButtonConfigurations[3].position.x == -65);
    assert(sButtonConfigurations[3].size.width == 120);
    assert(sButtonConfigurations[3].animation_handle == 1030);
    assert(sButtonConfigurations[4].position.x == 65);
    assert(sButtonConfigurations[4].size.width == 120);
    assert(sButtonConfigurations[4].animation_handle == 1031);
}

int main(void)
{
    TestConstructorAndRender();
    TestOwnedPanelDestruction();
    TestSetTocData();
    TestPrimarySelectionAccessAndSet();
    TestSecondarySelectionAccessAndSet();
    TestSecondaryStateSetter();
    TestUpdateVerticalPhase();
    TestUpdateHorizontalPhases();
    TestUpdateScrollVisibilityAndTicks();
    TestPublishWindowRegisters();
    TestRefreshPrimaryRows();
    TestRefreshSecondaryRows();
    TestInitializeEmptySelection();
    TestUpdateNavigation();
    TestHandleControls();
    TestMoveHelpers();
    TestModeHelpers();
    TestIsAnimating();
    TestSetMode();
    TestUpdateSecondaryMode();
    TestInteractionMode();
    TestHandlePointerGuardsAndCapture();
    TestHandlePointerPageAndControlActions();
    TestHandlePointerSelectionActions();
    TestHandlePointerActivationActions();
    TestConfigure();
    return 0;
}
