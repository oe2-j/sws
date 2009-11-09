/******************************************************************************
/ sws_extension.cpp
/
/ Copyright (c) 2009 Tim Payne (SWS)
/ http://www.standingwaterstudios.com/reaper
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/ 
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/ 
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/


#include "stdafx.h"
#include "Console/Console.h"
#include "Freeze/Freeze.h"
#include "MarkerActions/MarkerActions.h"
#include "ObjectState/TrackFX.h"
#include "Snapshots/SnapshotClass.h"
#include "Snapshots/Snapshots.h"
#include "Zoom.h"
#include "Color/Color.h"
#include "MarkerList/MarkerListClass.h"
#include "MarkerList/MarkerList.h"
#include "TrackList/TrackListFilter.h"
#include "TrackList/Tracklist.h"
#include "ProjectMgr.h"
#include "SnM/SnM_Actions.h"

// Globals
REAPER_PLUGIN_HINSTANCE g_hInst = NULL;
HWND g_hwndParent = NULL;
static WDL_PtrList<COMMAND_T> g_commands;
int g_iFirstCommand = 0;

bool hookCommandProc(int command, int flag)
{
	static bool bReentrancyCheck = false;
	if (bReentrancyCheck)
		return false;

	// for Xen extensions
	g_KeyUpUndoHandler=0;

	// "Hack" to make actions will #s less than 1000 work with SendMessage (AHK)
	if (command < 1000 && KBD_OnMainActionEx)
	{
		bReentrancyCheck = true;	
		KBD_OnMainActionEx(command, 0, 0, 0, g_hwndParent, NULL);
		bReentrancyCheck = false;
		return true;
	}

	// Ignore commands that don't have anything to do with us from this point forward
	if (command < g_iFirstCommand)
		return false;

	for (int i = 0; i < g_commands.GetSize(); i++)
	{
		if (g_commands.Get(i)->accel.accel.cmd && command == g_commands.Get(i)->accel.accel.cmd)
		{
			bReentrancyCheck = true;
			g_commands.Get(i)->doCommand(g_commands.Get(i));
			bReentrancyCheck = false;
			return true;
		}
	}
	return false;
}

// 1) Get command ID from Reaper
// 2) Add keyboard accelerator and add to the "action" list
int SWSRegisterCommand(COMMAND_T* pCommand)
{
	if (pCommand->doCommand)
	{
		if (!(pCommand->accel.accel.cmd = plugin_register("command_id", pCommand->id)))
			return 0;
		if (!plugin_register("gaccel",&pCommand->accel))
			return 0;
	}
	if (!g_iFirstCommand)
		g_iFirstCommand = pCommand->accel.accel.cmd;
	g_commands.Add(pCommand);
	return 1;
}

// For each item in table call SWSRegisterCommand
int SWSRegisterCommands(COMMAND_T* pCommands)
{
	// Register our commands from table
	int i = 0;
	while(pCommands[i].id != LAST_COMMAND)
	{
		SWSRegisterCommand(&pCommands[i]);
		i++;
	}
	return 1;
}

int SWSGetCommandID(void (*cmdFunc)(COMMAND_T*), int user, char** pMenuText)
{
	for (int i = 0; i < g_commands.GetSize(); i++)
	{
		if (g_commands.Get(i)->doCommand == cmdFunc && g_commands.Get(i)->user == user)
		{
			if (pMenuText)
				*pMenuText = g_commands.Get(i)->menuText;
			return g_commands.Get(i)->accel.accel.cmd;
		}
	}
	return 0;
}

HMENU SWSCreateMenu(COMMAND_T pCommands[], HMENU hMenu, int* iIndex)
{
	// Add menu items
	if (!hMenu)
		hMenu = CreatePopupMenu();
	int i = 0;
	if (iIndex)
		i = *iIndex;

	while (pCommands[i].id != LAST_COMMAND && pCommands[i].id != SWS_ENDSUBMENU)
	{
		if (pCommands[i].id == SWS_STARTSUBMENU)
		{
			char* subMenuName = pCommands[i].menuText;
			i++;
			HMENU hSubMenu = SWSCreateMenu(pCommands, NULL, &i);
			AddSubMenu(hMenu, hSubMenu, subMenuName);
		}
		else
			AddToMenu(hMenu, pCommands[i].menuText, pCommands[i].accel.accel.cmd);

		i++;
	}
	
	if (iIndex)
		*iIndex = i;
	return hMenu;
}

// Fake control surface just to get a low priority periodic time slice from Reaper
class SWSTimeSlice : public IReaperControlSurface
{
public:
	const char *GetTypeString() { return ""; }
	const char *GetDescString() { return ""; }
	const char *GetConfigString() { return ""; }

	bool m_bChanged;
	SWSTimeSlice() : m_bChanged() {}

	void Run()
	{
		ZoomSlice();
		MarkerActionSlice();

		if (m_bChanged)
		{
			m_bChanged = false;
			ScheduleTracklistUpdate();
			pMarkerList->Update();
			UpdateSnapshotsDialog();
		}
	}

	// This is our only notification of active project tab change, so update everything
	void SetTrackListChange()							{ m_bChanged = true; }
	// The rest only are applicable only to the TrackList
	void SetSurfaceSelected(MediaTrack *tr, bool bSel)	{ ScheduleTracklistUpdate(); }
	void SetTrackTitle(MediaTrack *tr, const char *c)	{ ScheduleTracklistUpdate(); }
	void SetSurfaceMute(MediaTrack *tr, bool mute)		{ ScheduleTracklistUpdate(); }
	void SetSurfaceSolo(MediaTrack *tr, bool solo)		{ ScheduleTracklistUpdate(); }
	void SetSurfaceRecArm(MediaTrack *tr, bool arm)		{ ScheduleTracklistUpdate(); }
};

// WDL Stuff
bool WDL_STYLE_GetBackgroundGradient(double *gradstart, double *gradslope) { return false; }
int WDL_STYLE_GetSysColor(int i) { if (GSC_mainwnd) return GSC_mainwnd(i); else return GetSysColor(i); }

// Hook Reaper's WNDPROC?
/*WNDPROC g_ReaperWndproc = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_MOUSEWHEEL)
	{
		// Do a mousewheel thingy.
		int i = 0;
	}
	if (g_ReaperWndproc)
		return g_ReaperWndproc(hwnd, uMsg, wParam, lParam);
	return 0;
}*/

// Main DLL entry point
#define ERR_RETURN(a) { return 0; }
#define OK_RETURN(a)  { return 1; }
//#define ERR_RETURN(a) { FILE* f = fopen("c:\\swserror.txt", "a"); if (f) { fprintf(f, a); fclose(f); } return 0; }
//#define OK_RETURN(a)  { FILE* f = fopen("c:\\swserror.txt", "a"); if (f) { fprintf(f, a); fclose(f); } return 1; }
extern "C"
{
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
	{
		if (!rec)
		{
			SnapshotsExit();
			TrackListExit();
			MarkerListExit();
			ERR_RETURN("Exiting Reaper.\n")
		}
		if (rec->caller_version != REAPER_PLUGIN_VERSION)
		{
			ERR_RETURN("Wrong REAPER_PLUGIN_VERSION!\n");
		}
		if (!rec->GetFunc)
		{
			ERR_RETURN("Null rec->GetFunc ptr\n")
		}

		int errcnt=0;
		IMPAPI(AddExtensionsMainMenu);
		IMPAPI(AddMediaItemToTrack);
		IMPAPI(AddProjectMarker);
		IMPAPI(AddTakeToMediaItem);
		IMPAPI(adjustZoom);
		IMPAPI(Audio_RegHardwareHook);
		IMPAPI(CoolSB_GetScrollInfo);
		IMPAPI(CoolSB_SetScrollInfo);
		IMPAPI(CountSelectedMediaItems);
		IMPAPI(CSurf_FlushUndo);
		IMPAPI(CSurf_GoEnd);
		IMPAPI(CSurf_OnMuteChange);
		IMPAPI(CSurf_OnPanChange);
		IMPAPI(CSurf_OnSelectedChange);
		IMPAPI(CSurf_OnTrackSelection);
		IMPAPI(CSurf_OnVolumeChange);
		IMPAPI(CSurf_TrackFromID);
		IMPAPI(CSurf_TrackToID);
		IMPAPI(DeleteProjectMarker);
		IMPAPI(DeleteTrack);
		IMPAPI(DeleteTrackMediaItem);
		IMPAPI(DockWindowActivate);
		IMPAPI(DockWindowAdd);
		IMPAPI(DockWindowRemove);
		IMPAPI(EnsureNotCompletelyOffscreen);
		IMPAPI(EnumProjectMarkers);
		IMPAPI(EnumProjects);
		IMPAPI(format_timestr);
		IMPAPI(format_timestr_pos);
		IMPAPI(FreeHeapPtr);
		IMPAPI(GetColorTheme);
		IMPAPI(GetContextMenu);
		IMPAPI(GetCursorPosition);
		IMPAPI(GetExePath);
		IMPAPI(GetHZoomLevel);
		IMPAPI(GetInputChannelName);
		IMPAPI(GetMainHwnd);
		IMPAPI(GetMediaItemNumTakes);
		IMPAPI(GetMediaItemTake);
		IMPAPI(GetNumTracks);
		IMPAPI(GetPeaksBitmap);
		IMPAPI(GetPlayPosition);
		IMPAPI(GetPlayPosition2);
		IMPAPI(GetPlayState);
		IMPAPI(GetProjectPath);
		IMPAPI(GetSelectedTrackEnvelope);
		IMPAPI(GetSetMediaItemInfo);
		IMPAPI(GetSetMediaItemTakeInfo);
		IMPAPI(GetSetMediaTrackInfo);
		IMPAPI(GetSetObjectState);
		IMPAPI(GetSetRepeat);
		IMPAPI(GetSetTrackSendInfo);
		IMPAPI(GetSet_LoopTimeRange);
		IMPAPI(GetTrackGUID);
		IMPAPI(GetTrackInfo);
		IMPAPI(GetTrackMediaItem);
		IMPAPI(GetTrackNumMediaItems);
		IMPAPI(GetTrackUIVolPan);
		IMPAPI(get_config_var);
		IMPAPI(get_ini_file);
		IMPAPI(GSC_mainwnd);
		IMPAPI(guidToString);
		IMPAPI(InsertMedia);
		IMPAPI(InsertTrackAtIndex);
		IMPAPI(IsMediaExtension);
		IMPAPI(kbd_enumerateActions);
		IMPAPI(kbd_getCommandName);
		IMPAPI(kbd_getTextFromCmd);
		IMPAPI(KBD_OnMainActionEx);
		IMPAPI(kbd_reprocessMenu);
		IMPAPI(kbd_RunCommandThroughHooks);
		IMPAPI(kbd_translateAccelerator);
		IMPAPI(Main_OnCommand);
		IMPAPI(Main_OnCommandEx);
		IMPAPI(Main_openProject);
		IMPAPI(MoveMediaItemToTrack);
		IMPAPI(parse_timestr_pos);
		IMPAPI(PCM_Sink_Create);
		IMPAPI(PCM_Source_CreateFromFile);
		IMPAPI(PCM_Source_CreateFromSimple);
		IMPAPI(PCM_Source_CreateFromType);
		IMPAPI(PlayPreview);
		IMPAPI(PlayTrackPreview);
		IMPAPI(plugin_register);
		IMPAPI(projectconfig_var_addr);
		IMPAPI(projectconfig_var_getoffs);
		IMPAPI(Resampler_Create);
		IMPAPI(screenset_register);
		IMPAPI(SelectProjectInstance);
		IMPAPI(SetEditCurPos);
		IMPAPI(SetProjectMarker);
		IMPAPI(SetTrackSelected);
		IMPAPI(StopPreview);
		IMPAPI(StopTrackPreview);
		IMPAPI(stringToGuid);
		IMPAPI(TimeMap_GetDividedBpmAtTime);
		IMPAPI(TimeMap_QNToTime);
		IMPAPI(TimeMap_timeToQN);
		IMPAPI(TrackFX_GetCount);
		IMPAPI(TrackFX_GetFXName);
		IMPAPI(TrackFX_GetNumParams);
		IMPAPI(TrackFX_GetParam);
		IMPAPI(TrackFX_GetParamName);
		IMPAPI(TrackFX_SetParam);
		IMPAPI(TrackList_AdjustWindows);
		IMPAPI(Undo_BeginBlock);
		IMPAPI(Undo_EndBlock);
		IMPAPI(Undo_OnStateChange);
		IMPAPI(Undo_OnStateChangeEx);
		IMPAPI(UpdateTimeline);

		g_hInst = hInstance;
		g_hwndParent = GetMainHwnd();

		if (errcnt)
		{
			MessageBox(g_hwndParent, "The version of SWS extension you have installed is incompatible with your version of Reaper.  You probably have a Reaper version less than 3.13 installed. "
				"Please install the latest version of Reaper from www.reaper.fm.", "Version Incompatibility", MB_OK);
			return 0;
		}

		//g_ReaperWndproc = (WNDPROC)SetWindowLongPtr(g_hwndParent, GWLP_WNDPROC, (LONG)WindowProc);

		if (!rec->Register("hookcommand",(void*)hookCommandProc))
			ERR_RETURN("hook command error\n")

		// Call plugin specific init
		if (!ConsoleInit())
			ERR_RETURN("ReaConsole init error\n")
		if (!FreezeInit())
			ERR_RETURN("Freeze init error\n")
		if (!MarkerActionsInit())
			ERR_RETURN("Marker action init error\n")
		if (!SnapshotsInit())
			ERR_RETURN("Snapshots init error\n")
		if (!MarkerListInit())
			ERR_RETURN("Marker list init error\n")
		if (!ColorInit())
			ERR_RETURN("Color init error\n")
		if (!TrackListInit())
			ERR_RETURN("Tracklist init error\n")
		if (!ZoomInit())
			ERR_RETURN("Zoom init error\n")
		if (!ProjectMgrInit())
			ERR_RETURN("Project Mgr init error\n")
		if (!XenakiosInit())
			ERR_RETURN("Xenakios init error\n")
		if (!SnMActionsInit())
			ERR_RETURN("SnM init error\n")
		if (!AboutBoxInit())
			ERR_RETURN("About box init error\n")

		SWSTimeSlice* ts = new SWSTimeSlice();
		if (!rec->Register("csurf_inst", ts))
		{
			delete ts;
			ERR_RETURN("TimeSlice init error\n")
		}

		OK_RETURN("SWS Extension successfully loaded.\n");
	}
};   // end extern C

#ifndef _WIN32 // MAC resources
#include "../WDL/swell/swell-dlggen.h"
#include "sws_extension.rc_mac_dlg"
#include "Xenakios/res.rc_mac_dlg"
#undef BEGIN
#undef END
#include "../WDL/swell/swell-menugen.h"
#include "sws_extension.rc_mac_menu"
#include "Xenakios/res.rc_mac_menu"
#endif
