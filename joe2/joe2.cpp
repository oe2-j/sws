//
// joe2.cpp
//

#include "stdafx.h"
// Should this be in here? At some point, maybe.
//#include "../reaper/localize.h"
#include "joe2.h"
#include "Breeder\BR_Util.h" // May as well use BR's Get/SetConfig code.



// Look at BR.cpp (for example) if you want to play around with stateful (toggle-able) actions.
// Keep it simple for now (testing).


static COMMAND_T g_commandTable[] =
{
	{ { DEFACCEL, "SWS/joe2: toggle \"Snap media items to nearby media items ...\"" }, "JOE2_TOGGLE_SNAP_MEDIA_ITEMS_TO_NEARBY", ToggleSnapMediaItemsToNearby }, // This line can take more args. For what?
	{ { DEFACCEL, "SWS/joe2: \"toggle\" \"Media items snap at: ...\"" }, "JOE2_TOGGLE_MEDIA_ITEMS_SNAP_AT", ToggleMediaItemsSnapAt },
	{ { DEFACCEL, "SWS/joe2: decrease \"media items snap to nearby ...\" tracks by 1" }, "JOE2_MEDIA_ITEMS_SNAP_NEARBY_MAX_TRACKS_DECREASE", MediaItemsSnapNearbyMaxTracksDecrease },
	{ { DEFACCEL, "SWS/joe2: increase \"media items snap to nearby ...\" tracks by 1" }, "JOE2_MEDIA_ITEMS_SNAP_NEARBY_MAX_TRACKS_INCREASE", MediaItemsSnapNearbyMaxTracksIncrease },
	{ { DEFACCEL, "SWS/joe2: set \"media items snap to nearby ...\" tracks to 0" }, "JOE2_MEDIA_ITEMS_SNAP_NEARBY_MAX_TRACKS_ZERO", MediaItemsSnapNearbyMaxTracksZero },
	
	{ {}, LAST_COMMAND, },
};



void ToggleSnapMediaItemsToNearby(COMMAND_T* _ct)
{

	int option;
	GetConfig("projshowgrid", option);

	// BR's "ToggleBit" seems to "flip" the bit specified by the second argument.
	// Bit #8 (position 7 when you count from zero :) is what we're interested in here:

	SetConfig("projshowgrid", ToggleBit(option, 7));

	// Dunno if anything should be called to clean up.

}


void ToggleMediaItemsSnapAt(COMMAND_T* _ct)
{

	// ("option" is just the decimal integer value, I think.)

	int option0;
	
	GetConfig("projshowgrid", option0);
	

	// Not entirely surprisingly, projshowgrid is 4 bytes (32 bits).
	

	// Old testing bits:
	/*
	int sz = 0;

	int *projshowgrid = (int *)get_config_var("projshowgrid", &sz);
	
	int foo = GetBit(option0, 11);
	int bar = GetBit(option0, 12);

	char buffer0[128];
	char buffer1[128];
	sprintf_s(buffer0, "%d", foo);
	sprintf_s(buffer1, "%d", bar);
	OutputDebugStringA("joe2: bit 11 = ");
	OutputDebugStringA(buffer0);
	OutputDebugStringA("joe2: bit 12 = ");
	OutputDebugStringA(buffer1);
	*/

	// For safety, check if bits 11 and 12 are both 0 (i.e. "mouse-dependent" is selected).
	// If they are, just return to "start/offset" (my default).

	if (GetBit(option0, 11) == 0)
	{
		if (GetBit(option0, 12) == 0)
		{
			SetConfig("projshowgrid", ToggleBit(option0, 11));

			return;
		}
	}
	//else
	//{

		// OTT, surely can be done with one var. ;)
		int newOption0 = ToggleBit(option0, 11);
		int newOption1 = ToggleBit(newOption0, 12);

		SetConfig("projshowgrid", newOption1);

	//}

}



void MediaItemsSnapNearbyMaxTracksDecrease(COMMAND_T* _ct)
{

	// Could change this to use (possibly) "safer" methods ("Get/SetConfig").

	int sz = 0;

	int *maxsnaptrack = (int *)get_config_var("maxsnaptrack", &sz);

	if (*maxsnaptrack > 0)
	{

		*maxsnaptrack = *maxsnaptrack - 1;

	}

}



void MediaItemsSnapNearbyMaxTracksIncrease(COMMAND_T* _ct)
{

	// Likewise.

	int sz = 0;

	int *maxsnaptrack = (int *)get_config_var("maxsnaptrack", &sz);

	*maxsnaptrack = *maxsnaptrack + 1;

}



void MediaItemsSnapNearbyMaxTracksZero(COMMAND_T* _ct)
{

	// "

	int sz = 0;

	int *maxsnaptrack = (int *)get_config_var("maxsnaptrack", &sz);

	*maxsnaptrack = 0;

}


int joe2_Init()
{

	return SWSRegisterCommands(g_commandTable);

}


//void joe2_Exit()
//{
	// Is this needed? Should it return something?
//}
