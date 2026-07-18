#ifndef GAME_DATALINK_SLOT_TRANSFER_H
#define GAME_DATALINK_SLOT_TRANSFER_H

void Game_BeginDatalinkSlotUpdate(void);
void Game_EndDatalinkSlotUpdate(void);

void Game_TransferDatalinkPlayerSlot(int source_slot, int destination_slot);
void Game_RemoveDatalinkPlayerSlot(int slot);

#endif
