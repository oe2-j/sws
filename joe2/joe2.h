//
// joe2.h
//

#pragma once

int joe2_Init();
//void joe2_Exit();

void ToggleSnapMediaItemsToNearby(COMMAND_T* _ct);
void ToggleMediaItemsSnapAt(COMMAND_T* _ct);

void MediaItemsSnapNearbyMaxTracksDecrease(COMMAND_T* _ct);
void MediaItemsSnapNearbyMaxTracksIncrease(COMMAND_T* _ct);
void MediaItemsSnapNearbyMaxTracksZero(COMMAND_T* _ct);

