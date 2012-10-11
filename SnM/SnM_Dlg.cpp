/******************************************************************************
/ SnM_Dlg.cpp
/
/ Copyright (c) 2009-2012 Jeffos
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
#include "SnM.h"
#include "../reaper/localize.h"
#include "../Prompt.h"


///////////////////////////////////////////////////////////////////////////////
// Theming
///////////////////////////////////////////////////////////////////////////////

// not configurable on osx, optional on win (.ini file)
bool g_SNMClearType = 
#ifdef _WIN32
	false;
#else
	true;
#endif

ColorTheme* SNM_GetColorTheme(bool _checkForSize) {
	int sz; ColorTheme* ct = (ColorTheme*)GetColorThemeStruct(&sz);
	if (ct && (!_checkForSize || (_checkForSize && sz >= sizeof(ColorTheme)))) return ct;
	return NULL;
}

IconTheme* SNM_GetIconTheme(bool _checkForSize) {
	int sz; IconTheme* it = (IconTheme*)GetIconThemeStruct(&sz);
	if (it && (!_checkForSize || (_checkForSize && sz >= sizeof(IconTheme)))) return it;
	return NULL;
}

LICE_CachedFont* SNM_GetThemeFont()
{
	static LICE_CachedFont themeFont;
	if (!themeFont.GetHFont())
	{
		LOGFONT lf = {
			SNM_FONT_HEIGHT, 0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,SNM_FONT_NAME
		};
		themeFont.SetFromHFont(CreateFontIndirect(&lf),LICE_FONT_FLAG_OWNS_HFONT | (g_SNMClearType?LICE_FONT_FLAG_FORCE_NATIVE:0));
	}
	themeFont.SetBkMode(TRANSPARENT);
	ColorTheme* ct = SNM_GetColorTheme();
	themeFont.SetTextColor(ct ? LICE_RGBA_FROMNATIVE(ct->main_text,255) : LICE_RGBA(255,255,255,255));
	return &themeFont;
}

LICE_CachedFont* SNM_GetToolbarFont()
{
	static LICE_CachedFont themeFont;
	if (!themeFont.GetHFont())
	{
		LOGFONT lf = {
			SNM_FONT_HEIGHT,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,SNM_FONT_NAME
		};
		themeFont.SetFromHFont(CreateFontIndirect(&lf), LICE_FONT_FLAG_OWNS_HFONT | (g_SNMClearType?LICE_FONT_FLAG_FORCE_NATIVE:0));
	}
	themeFont.SetBkMode(TRANSPARENT);
	ColorTheme* ct = SNM_GetColorTheme();
	themeFont.SetTextColor(ct ? LICE_RGBA_FROMNATIVE(ct->toolbar_button_text,255) : LICE_RGBA(255,255,255,255));
	return &themeFont;
}

void SNM_GetThemeListColors(int* _bg, int* _txt, int* _grid)
{
	int bgcol=-1, txtcol=-1, gridcol=-1;
	ColorTheme* ct = SNM_GetColorTheme(true); // true: list view colors are recent (v4.11)
	if (ct) {
		bgcol = ct->genlist_bg;
		txtcol = ct->genlist_fg;
		gridcol = ct->genlist_gridlines;
		// note: selection colors are not managed here
	}
	if (bgcol == txtcol) { // safety
		bgcol = GSC_mainwnd(COLOR_WINDOW);
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (_bg) *_bg = bgcol;
	if (_txt) *_txt = txtcol;
	if (_grid) *_grid = gridcol;
}

void SNM_GetThemeEditColors(int* _bg, int* _txt)
{
	int bgcol=-1, txtcol=-1;
	ColorTheme* ct = SNM_GetColorTheme();
	if (ct) {
		bgcol =  ct->main_editbk;
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (bgcol == txtcol) { // safety
		bgcol = GSC_mainwnd(COLOR_WINDOW);
		txtcol = GSC_mainwnd(COLOR_BTNTEXT);
	}
	if (_bg) *_bg = bgcol;
	if (_txt) *_txt = txtcol;
}

void SNM_ThemeListView(SWS_ListView* _lv)
{
	if (_lv && _lv->GetHWND())
	{
		int bgcol, txtcol, gridcol;
		SNM_GetThemeListColors(&bgcol, &txtcol, &gridcol);
		ListView_SetBkColor(_lv->GetHWND(), bgcol);
		ListView_SetTextColor(_lv->GetHWND(), txtcol);
		ListView_SetTextBkColor(_lv->GetHWND(), bgcol);
#ifndef _WIN32
		ListView_SetGridColor(_lv->GetHWND(), gridcol);
#endif
	}
}

LICE_IBitmap* SNM_GetThemeLogo()
{
	static LICE_IBitmap* snmLogo;
	if (!snmLogo)
	{
/*JFB commented: load from resources KO for OSX (it looks for REAPER's resources..)
#ifdef _WIN32
		snmLogo = LICE_LoadPNGFromResource(g_hInst,IDB_SNM,NULL);
#else
		// SWS doesn't work, sorry. :( 
		//snmLogo =  LICE_LoadPNGFromNamedResource("SnM.png",NULL);
		snmLogo = NULL;
#endif
*/
		// logo is now loaded from memory (OSX support)
		if (WDL_HeapBuf* hb = TranscodeStr64ToHeapBuf(SNM_LOGO_PNG_FILE)) {
			snmLogo = LICE_LoadPNGFromMemory(hb->Get(), hb->GetSize());
			delete hb;
		}
	}
	return snmLogo;
}


#ifdef _WIN32

#define MAX_THEMED_CTRLS 256

// calling RemoveXPStyle() straight in there would crash!
static BOOL CALLBACK EnumRemoveXPStyles(HWND _hwnd, LPARAM _ids)
{
	int i=0;

	// do not deal with list views & list boxes
	char className[64] = "";
	if (GetClassName(_hwnd, className, sizeof(className)) && strcmp(className, WC_LISTVIEW) && strcmp(className, WC_LISTBOX))
	{
		LONG style = GetWindowLong(_hwnd, GWL_STYLE);
		if ((style & BS_AUTOCHECKBOX) == BS_AUTOCHECKBOX ||
			(style & BS_AUTORADIOBUTTON) == BS_AUTORADIOBUTTON ||
			(style & BS_GROUPBOX) == BS_GROUPBOX)
		{
			int* ids = (int*)_ids;
			int i=0; while (ids[i]!=-1 && i<MAX_THEMED_CTRLS) i++;
			if (i<MAX_THEMED_CTRLS)
				ids[i] = (int)GetWindowLong(_hwnd, GWL_ID);
			else
				return FALSE;
		}
	}
	return TRUE;
}

#endif

WDL_DLGRET SNM_HookThemeColorsMessage(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam, bool _wantColorEdit)
{
	if (SWS_THEMING)
	{
		switch(_uMsg)
		{
#ifdef _WIN32
			case WM_INITDIALOG :
			{
				// remove XP style on some child ctrls (cannot be themed otherwise)
				int ids[MAX_THEMED_CTRLS];
				memset(ids, -1, sizeof(int)*MAX_THEMED_CTRLS);
				EnumChildWindows(_hwnd, EnumRemoveXPStyles, (LPARAM)ids);
				int i=0;
				while (i<MAX_THEMED_CTRLS && ids[i]!=-1)
					RemoveXPStyle(GetDlgItem(_hwnd, ids[i++]), 1);
				return 0;
			}
#endif
			case WM_CTLCOLOREDIT:
				if (!_wantColorEdit) return 0;
			case WM_CTLCOLORSCROLLBAR: // not managed yet, just in case..
			case WM_CTLCOLORLISTBOX:
			case WM_CTLCOLORBTN:
			case WM_CTLCOLORDLG:
			case WM_CTLCOLORSTATIC:
/* commented (allow custom implementations)
			case WM_DRAWITEM:
*/
				return SendMessage(GetMainHwnd(), _uMsg,_wParam,_lParam);
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////

void SNM_UIInit() {}

void SNM_UIExit() {
	if (LICE_IBitmap* logo = SNM_GetThemeLogo())
		DELETE_NULL(logo);
}


///////////////////////////////////////////////////////////////////////////////
// Messages, prompt, etc..
///////////////////////////////////////////////////////////////////////////////

void SNM_ShowMsg(const char* _msg, const char* _title, HWND _hParent)
{
	char msg[1024*8] = "";
	GetStringWithRN(_msg, msg, sizeof(msg)); // truncates if needed
	DisplayInfoBox(_hParent?_hParent:GetMainHwnd(), _title, msg, false, false); // modeless
}

// _min and _max: 1-based (i.e. as displayed)
// returns -1 on cancel, 0-based number otherwise
int PromptForInteger(const char* _title, const char* _what, int _min, int _max)
{
	WDL_String str;
	int nb = -1;
	while (nb == -1)
	{
		str.SetFormatted(128, "%s (%d-%d):", _what, _min, _max);
		char reply[8]= ""; // no default
		if (GetUserInputs(_title, 1, str.Get(), reply, 8))
		{
			nb = atoi(reply); // 0 on error
			if (nb >= _min && nb <= _max)
				return (nb-1);
			else {
				nb = -1;
				str.SetFormatted(128, "Invalid %s!\nPlease enter a value in [%d; %d].", _what, _min, _max);
				MessageBox(GetMainHwnd(), str.Get(), __LOCALIZE("S&M - Error","sws_mbox"), MB_OK);
			}
		}
		else return -1; // user has cancelled
	}
	return -1;
}


///////////////////////////////////////////////////////////////////////////////
// Cue buss dialog box
///////////////////////////////////////////////////////////////////////////////

int GetComboSendIdxType(int _reaType) 
{
	switch(_reaType) {
		case 0: return 1;
		case 3: return 2; 
		case 1: return 3; 
		default: return 1;
	}
	return 1; // in case _reaType comes from mars
}

const char* GetSendTypeStr(int _type) 
{
	switch(_type) {
		case 1: return __localizeFunc("Post-Fader (Post-Pan)","common",0);
		case 2: return __localizeFunc("Pre-Fader (Post-FX)","common",0);
		case 3: return __localizeFunc("Pre-FX","common",0);
		default: return "";
	}
}

void FillHWoutDropDown(HWND _hwnd, int _idc)
{
	char buffer[BUFFER_SIZE];
	lstrcpyn(buffer, __LOCALIZE("None","sws_DLG_149"), BUFFER_SIZE);
	SendDlgItemMessage(_hwnd,_idc,CB_ADDSTRING,0,(LPARAM)buffer);
	SendDlgItemMessage(_hwnd,_idc,CB_SETITEMDATA,0,0);
	
	// get mono outputs
	WDL_PtrList<WDL_FastString> monos;
	int monoIdx=0;
	while (GetOutputChannelName(monoIdx))
	{
		monos.Add(new WDL_FastString(GetOutputChannelName(monoIdx)));
		monoIdx++;
	}

	// add stereo outputs
	WDL_PtrList<WDL_FastString> stereos;
	if (monoIdx)
	{
		for(int i=0; i < (monoIdx-1); i++)
		{
			WDL_FastString* hw = new WDL_FastString();
			hw->SetFormatted(256, "%s / %s", monos.Get(i)->Get(), monos.Get(i+1)->Get());
			stereos.Add(hw);
		}
	}

	// fill dropdown
	for(int i=0; i < stereos.GetSize(); i++) {
		SendDlgItemMessage(_hwnd,_idc,CB_ADDSTRING,0,(LPARAM)stereos.Get(i)->Get());
		SendDlgItemMessage(_hwnd,_idc,CB_SETITEMDATA,i,i+1); // +1 for <none>
	}
	for(int i=0; i < monos.GetSize(); i++) {
		SendDlgItemMessage(_hwnd,_idc,CB_ADDSTRING,0,(LPARAM)monos.Get(i)->Get());
		SendDlgItemMessage(_hwnd,_idc,CB_SETITEMDATA,i,i+1); // +1 for <none>
	}

//	SendDlgItemMessage(_hwnd,_idc,CB_SETCURSEL,x0,0);
	monos.Empty(true);
	stereos.Empty(true);
}

HWND g_cueBussHwnd = NULL;
int g_cueBussConfId = 0; // not saved in prefs yet
bool g_cueBussDisableSave = false;

void FillCueBussDlg(HWND _hwnd = NULL)
{
	HWND hwnd = _hwnd?_hwnd:g_cueBussHwnd;
	if (!hwnd)
		return;

	g_cueBussDisableSave=true;
	char busName[BUFFER_SIZE]="", trTemplatePath[BUFFER_SIZE]="";
	int reaType, userType, soloDefeat, hwOuts[8];
	bool trTemplate, showRouting, sendToMaster;
	ReadCueBusIniFile(g_cueBussConfId, busName, &reaType, &trTemplate, trTemplatePath, &showRouting, &soloDefeat, &sendToMaster, hwOuts);
	userType = GetComboSendIdxType(reaType);
	SetDlgItemText(hwnd,IDC_SNM_CUEBUS_NAME,busName);

	for(int i=0; i<3; i++) {
		if (_hwnd) SendDlgItemMessage(hwnd,IDC_SNM_CUEBUS_TYPE,CB_ADDSTRING,0,(LPARAM)GetSendTypeStr(i+1)); // do it once (WM_INITDIALOG)
		if (userType==(i+1)) SendDlgItemMessage(hwnd,IDC_SNM_CUEBUS_TYPE,CB_SETCURSEL,i,0);
	}

	SetDlgItemText(hwnd,IDC_SNM_CUEBUS_TEMPLATE,trTemplatePath);
	CheckDlgButton(hwnd, IDC_CHECK1, sendToMaster);
	CheckDlgButton(hwnd, IDC_CHECK2, showRouting);
	CheckDlgButton(hwnd, IDC_CHECK3, trTemplate);
	CheckDlgButton(hwnd, IDC_CHECK4, (soloDefeat == 1));

	for(int i=0; i < SNM_MAX_HW_OUTS; i++) {
		if (_hwnd) FillHWoutDropDown(hwnd,IDC_SNM_CUEBUS_HWOUT1+i); // do it once (WM_INITDIALOG)
		SendDlgItemMessage(hwnd,IDC_SNM_CUEBUS_HWOUT1+i,CB_SETCURSEL,hwOuts[i],0);
	}

//	SetFocus(GetDlgItem(hwnd, IDC_SNM_CUEBUS_NAME));
	PostMessage(hwnd, WM_COMMAND, IDC_CHECK3, 0); // enable//disable state

	g_cueBussDisableSave = false;
}

void SaveCueBussSettings()
{
	if (!g_cueBussHwnd || g_cueBussDisableSave)
		return;

	char cueBusName[BUFFER_SIZE]="";
	GetDlgItemText(g_cueBussHwnd,IDC_SNM_CUEBUS_NAME,cueBusName,BUFFER_SIZE);

	int userType=2, reaType;
	int combo = (int)SendDlgItemMessage(g_cueBussHwnd,IDC_SNM_CUEBUS_TYPE,CB_GETCURSEL,0,0);
	if(combo != CB_ERR)
		userType = combo+1;
	switch(userType) {
		case 1: reaType=0; break;
		case 2: reaType=3; break;
		case 3: reaType=1; break;
		default: reaType=3; break;
	}

	int sendToMaster = IsDlgButtonChecked(g_cueBussHwnd, IDC_CHECK1);
	int showRouting = IsDlgButtonChecked(g_cueBussHwnd, IDC_CHECK2);
	int trTemplate = IsDlgButtonChecked(g_cueBussHwnd, IDC_CHECK3);
	int soloDefeat = IsDlgButtonChecked(g_cueBussHwnd, IDC_CHECK4);

	char trTemplatePath[BUFFER_SIZE]="";
	GetDlgItemText(g_cueBussHwnd,IDC_SNM_CUEBUS_TEMPLATE,trTemplatePath,BUFFER_SIZE);

	int hwOuts[SNM_MAX_HW_OUTS];
	for (int i=0; i<SNM_MAX_HW_OUTS; i++) {
		hwOuts[i] = (int)SendDlgItemMessage(g_cueBussHwnd,IDC_SNM_CUEBUS_HWOUT1+i,CB_GETCURSEL,0,0);
		if(hwOuts[i] == CB_ERR)	hwOuts[i] = '\0';
	}
	SaveCueBusIniFile(g_cueBussConfId, cueBusName, reaType, (trTemplate == 1), trTemplatePath, (showRouting == 1), soloDefeat, (sendToMaster == 1), hwOuts);
}

WDL_DLGRET CueBussDlgProc(HWND _hwnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	if (INT_PTR r = SNM_HookThemeColorsMessage(_hwnd, _uMsg, _wParam, _lParam))
		return r;

	const char cWndPosKey[] = "CueBus Window Pos";
	switch(_uMsg)
	{
		case WM_INITDIALOG:
		{
			WDL_UTF8_HookComboBox(GetDlgItem(_hwnd, IDC_SNM_CUEBUS_TYPE));
			WDL_UTF8_HookComboBox(GetDlgItem(_hwnd, IDC_COMBO));
			for(int i=0; i < SNM_MAX_HW_OUTS; i++)
				WDL_UTF8_HookComboBox(GetDlgItem(_hwnd, IDC_SNM_CUEBUS_HWOUT1+i));

			RestoreWindowPos(_hwnd, cWndPosKey, false);
			char buf[16] = "";
			for(int i=0; i < SNM_MAX_CUE_BUSS_CONFS; i++)
				if (_snprintfStrict(buf,sizeof(buf),"%d",i+1) > 0)
					SendDlgItemMessage(_hwnd,IDC_COMBO,CB_ADDSTRING,0,(LPARAM)buf);
			SendDlgItemMessage(_hwnd,IDC_COMBO,CB_SETCURSEL,0,0);
			FillCueBussDlg(_hwnd);
			return 0;
		}
		break;

		case WM_CLOSE :
			g_cueBussHwnd = NULL; // for proper toggle state report, see openCueBussWnd()
			break;

		case WM_COMMAND :
			switch(LOWORD(_wParam))
			{
				case IDC_COMBO:
					if(HIWORD(_wParam) == CBN_SELCHANGE) // config id update?
					{ 
						int id = (int)SendDlgItemMessage(_hwnd, IDC_COMBO, CB_GETCURSEL, 0, 0);
						if (id != CB_ERR) {
							g_cueBussConfId = id;
							FillCueBussDlg();
						}
					}
					break;
				case IDOK:
					CueBuss(__LOCALIZE("Create cue buss from track selection","sws_undo"), g_cueBussConfId);
					return 0;
				case IDCANCEL:
					g_cueBussHwnd = NULL; // for proper toggle state report, see openCueBussWnd()
					ShowWindow(_hwnd, SW_HIDE);
					return 0;
				case IDC_FILES: {
					char curPath[BUFFER_SIZE]="";
					GetDlgItemText(_hwnd, IDC_SNM_CUEBUS_TEMPLATE, curPath, BUFFER_SIZE);
					if (!*curPath || !FileExists(curPath))
						if (_snprintfStrict(curPath, sizeof(curPath), "%s%cTrackTemplates", GetResourcePath(), PATH_SLASH_CHAR) <= 0)
							*curPath = '\0';
					if (char* fn = BrowseForFiles(__LOCALIZE("S&M - Load track template","sws_DLG_149"), curPath, NULL, false, "REAPER Track Template (*.RTrackTemplate)\0*.RTrackTemplate\0")) {
						SetDlgItemText(_hwnd,IDC_SNM_CUEBUS_TEMPLATE,fn);
						free(fn);
						SaveCueBussSettings();
					}
					break;
				}
				case IDC_CHECK3: {
					bool templateEnable = (IsDlgButtonChecked(_hwnd, IDC_CHECK3) == 1);
					EnableWindow(GetDlgItem(_hwnd, IDC_SNM_CUEBUS_TEMPLATE), templateEnable);
					EnableWindow(GetDlgItem(_hwnd, IDC_FILES), templateEnable);
					EnableWindow(GetDlgItem(_hwnd, IDC_SNM_CUEBUS_NAME), !templateEnable);
					for(int k=0; k < SNM_MAX_HW_OUTS ; k++)
						EnableWindow(GetDlgItem(_hwnd, IDC_SNM_CUEBUS_HWOUT1+k), !templateEnable);
					EnableWindow(GetDlgItem(_hwnd, IDC_CHECK1), !templateEnable);
					EnableWindow(GetDlgItem(_hwnd, IDC_CHECK4), !templateEnable);
//					SetFocus(GetDlgItem(_hwnd, templateEnable ? IDC_SNM_CUEBUS_TEMPLATE : IDC_SNM_CUEBUS_NAME));
					SaveCueBussSettings();
					break;
				}
				case IDC_SNM_CUEBUS_SOLOGRP:
				case IDC_CHECK1:
				case IDC_CHECK2:
				case IDC_CHECK4:
					SaveCueBussSettings();
					break;
				case IDC_SNM_CUEBUS_TYPE:
				case IDC_SNM_CUEBUS_HWOUT1:
				case IDC_SNM_CUEBUS_HWOUT2:
				case IDC_SNM_CUEBUS_HWOUT3:
				case IDC_SNM_CUEBUS_HWOUT4:
				case IDC_SNM_CUEBUS_HWOUT5:
				case IDC_SNM_CUEBUS_HWOUT6:
				case IDC_SNM_CUEBUS_HWOUT7:
				case IDC_SNM_CUEBUS_HWOUT8:
					if(HIWORD(_wParam) == CBN_SELCHANGE)
						SaveCueBussSettings();
					break;
				case IDC_SNM_CUEBUS_TEMPLATE:
				case IDC_SNM_CUEBUS_NAME:
					if (HIWORD(_wParam)==EN_CHANGE)
						SaveCueBussSettings();
					break;
			}
			break;

		case WM_DESTROY:
			SaveWindowPos(_hwnd, cWndPosKey);
			break;
	}
	return 0;
}

void OpenCueBussDlg(COMMAND_T* _ct)
{
	static HWND hwnd = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_SNM_CUEBUSS), GetMainHwnd(), CueBussDlgProc);
	if (g_cueBussHwnd) {
		g_cueBussHwnd = NULL;
		ShowWindow(hwnd, SW_HIDE);
	}
	else {
		g_cueBussHwnd = hwnd;
		ShowWindow(hwnd, SW_SHOW);
		SetFocus(hwnd);
	}
}

bool IsCueBussDlgDisplayed(COMMAND_T* _ct) {
	return (g_cueBussHwnd && SWS_IsWindow(g_cueBussHwnd) && IsWindowVisible(g_cueBussHwnd) ? true : false);
}

