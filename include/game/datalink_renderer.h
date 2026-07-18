#ifndef GAME_DATALINK_RENDERER_H
#define GAME_DATALINK_RENDERER_H

#include "game/datalink_initial_phase.h"
#include "game/datalink_renderer_bindings.h"
#include "game/datalink_renderer_ready_row.h"

extern volatile Game_DatalinkRendererBindings
    gGameDatalinkRendererBindings;
extern Game_DatalinkRenderRowMetadata gGameDatalinkRenderRowMetadata[3];
extern volatile Game_DatalinkReadyRowRecord gGameDatalinkReadyRowRecords[3];

void Game_UpdateDatalinkRenderer(Game_DatalinkFlowState *state);

#endif
