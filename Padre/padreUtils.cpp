/******************************************************************************
/ padreUtils.cpp
/
/ Copyright (c) 2009-2010 Tim Payne (SWS), JF B�dague, P. Bourdon
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

const char* GetWaveShapeStr(WaveShape shape)
{
	switch(shape)
	{
		case eWAVSHAPE_SINE			: return "Sine";		break;
		case eWAVSHAPE_TRIANGLE		: return "Triangle";	break;
		case eWAVSHAPE_SQUARE		: return "Square";		break;
		case eWAVSHAPE_RANDOM		: return "Random";		break;
		case eWAVSHAPE_SAWUP		: return "Saw Up";		break;
		case eWAVSHAPE_SAWDOWN		: return "Saw Down";	break;

		case eWAVSHAPE_TRIANGLE_BEZIER		: return "Triangle (Bezier)";	break;
		case eWAVSHAPE_RANDOM_BEZIER		: return "Random (Bezier)";		break;
		case eWAVSHAPE_SAWUP_BEZIER			: return "Saw Up (Bezier)";		break;
		case eWAVSHAPE_SAWDOWN_BEZIER		: return "Saw Down (Bezier)";	break;

		default								: return "???";			break;
	}
}

const char* GetGridDivisionStr(GridDivision grid)
{
	switch(grid)
	{
		case eGRID_OFF		: return "<sync off>";	break;
		case eGRID_4_1		: return "4/1";			break;
		case eGRID_4T_1		: return "4T/1";		break;
		case eGRID_2_1		: return "2/1";			break;
		case eGRID_2T_1		: return "2T/1";		break;
		case eGRID_1_1		: return "1/1";			break;
		case eGRID_1T_1		: return "1T/1";		break;
		case eGRID_1_2		: return "1/2";			break;
		case eGRID_1_2T		: return "1/2T";		break;
		case eGRID_1_4		: return "1/4";			break;
		case eGRID_1_4T		: return "1/4T";		break;
		case eGRID_1_8		: return "1/8";			break;
		case eGRID_1_8T		: return "1/8T";		break;
		case eGRID_1_16		: return "1/16";		break;
		case eGRID_1_16T	: return "1/16T";		break;
		case eGRID_1_32		: return "1/32";		break;
		case eGRID_1_32T	: return "1/32T";		break;
		case eGRID_1_64		: return "1/64";		break;
		case eGRID_1_64T	: return "1/64T";		break;
		case eGRID_1_128	: return "1/128";		break;
		case eGRID_1_128T	: return "1/128T";		break;
		default				: return "???";			break;
	}
}

double GetGridDivisionFactor(GridDivision grid)
{
	switch(grid)
	{
		case eGRID_4_1		: return 0.25;			break;
		case eGRID_4T_1		: return 1.0/3.0;		break;
		case eGRID_2_1		: return 0.5;			break;
		case eGRID_2T_1		: return 2.0/3.0;		break;
		case eGRID_1_1		: return 1.0;			break;
		case eGRID_1T_1		: return 4.0/3.0;		break;
		case eGRID_1_2		: return 2.0;			break;
		case eGRID_1_2T		: return 3.0;			break;
		case eGRID_1_4		: return 4.0;			break;
		case eGRID_1_4T		: return 6.0;			break;
		case eGRID_1_8		: return 8.0;			break;
		case eGRID_1_8T		: return 12.0;			break;
		case eGRID_1_16		: return 16.0;			break;
		case eGRID_1_16T	: return 24.0;			break;
		case eGRID_1_32		: return 32.0;			break;
		case eGRID_1_32T	: return 48.0;			break;
		case eGRID_1_64		: return 64.0;			break;
		case eGRID_1_64T	: return 96.0;			break;
		case eGRID_1_128	: return 128.0;			break;
		case eGRID_1_128T	: return 192.0;			break;
		default				: return -1.0;			break;
	}
}

const char* GetTakeEnvelopeStr(TakeEnvType type)
{
	switch(type)
	{
		case eTAKEENV_VOLUME	: return "Volume";	break;
		case eTAKEENV_PAN		: return "Pan";		break;
		case eTAKEENV_MUTE		: return "Mute";	break;
		default					: return NULL;		break;
	}
}

double Sign(double value)
{
	if(value>0.0)
		return 1.0;
	else
		return -1.0;
}

double WaveformGeneratorSin(double t, double dFreq, double dDelay)
{
	return sin(2.0*PI*dFreq*(t+dDelay));
}

double WaveformGeneratorSquare(double t, double dFreq, double dDelay, bool &bFlipFlop)
{
	double dPhase = dFreq*(t+dDelay);
	dPhase = dPhase - (int)(dPhase);
	if(dPhase<0.0)
		dPhase += 1.0;
	if(dPhase<0.5)
	{
		bFlipFlop = false;
		return -1.0;
	}
	else
	{
		bFlipFlop = true;
		return 1.0;
	}
}

double WaveformGeneratorSquare(double t, double dFreq, double dDelay)
{
	 bool bFlipFlop;
	 return WaveformGeneratorSquare(t, dFreq, dDelay, bFlipFlop);
}

double WaveformGeneratorTriangle(double t, double dFreq, double dDelay, bool &bFlipFlop)
{
	double dPhase = dFreq*(t+dDelay);
	dPhase = dPhase - (int)(dPhase);
	if(dPhase<0.0)
		dPhase += 1.0;
	if(dPhase<0.5)
	{
		bFlipFlop = false;
		return -4.0*dPhase + 1.0;
	}
	else
	{
		bFlipFlop = true;
		return 4.0*dPhase - 3.0;
	}
}

double WaveformGeneratorTriangle(double t, double dFreq, double dDelay)
{
	 bool bFlipFlop;
	 return WaveformGeneratorTriangle(t, dFreq, dDelay, bFlipFlop);
}

double WaveformGeneratorSawUp(double t, double dFreq, double dDelay)
{
	double dPhase = dFreq*(t+dDelay);
	dPhase = dPhase - (int)(dPhase);
	if(dPhase<0.0)
		dPhase += 1.0;
	return 2.0*dPhase - 1.0;
}

double WaveformGeneratorSawDown(double t, double dFreq, double dDelay)
{
	double dPhase = dFreq*(t+dDelay);
	dPhase = dPhase - (int)(dPhase);
	if(dPhase<0.0)
		dPhase += 1.0;
	return -2.0*dPhase + 1.0;
}

double WaveformGeneratorRandom(double t, double dFreq, double dDelay)
{
	return 2.0*(double)rand()/(double)(RAND_MAX) - 1.0;
}

double EnvSignalProcessorFade(double dPos, double dLength, double dStrength, bool bFadeIn)
{
	if(bFadeIn)
		return pow((dPos/dLength), dStrength);
	else
		return pow(((dLength - dPos)/dLength), dStrength);
}

char* PadresGetEnvelopeState(TrackEnvelope* envelope)
{
	//! \note GetSetObjectState() does not work for take envelopes
	// While loop shamelessly stolen from SWS! :)
	if(!envelope)
		return NULL;

	char* envState = NULL;
	int iEnvStateMaxSize = 65536;
	int iEnvStateSize = 256;
	while(true)
	{
		envState = (char*)realloc(envState, iEnvStateSize);
		envState[0] = 0;
		bool bRes = GetSetEnvelopeState(envelope, envState, iEnvStateSize);
		if(bRes && (strlen(envState)!=iEnvStateSize-1))
			break;

		if (!bRes || (iEnvStateSize>=iEnvStateMaxSize))
		{
			free(envState);
			return NULL;
		}

		iEnvStateSize *= 2;
	}

	return envState;
}

void ShowConsoleMsgEx(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[2048];
	vsnprintf(buffer, 2048, format, args);
	ShowConsoleMsg(buffer);
	va_end(args);
}

void GetTimeSegmentPositions(TimeSegment timeSegment, double &dStartPos, double &dEndPos)
{
	switch(timeSegment)
	{
		case eTIMESEGMENT_TIMESEL:
			Main_OnCommandEx(ID_GOTO_TIMESEL_END, 0, 0);
			dEndPos = GetCursorPositionEx(0);
			Main_OnCommandEx(ID_GOTO_TIMESEL_START, 0, 0);
			dStartPos = GetCursorPositionEx(0);
		break;
		case eTIMESEGMENT_SELITEM:
			Main_OnCommandEx(ID_GOTO_SELITEM_END, 0, 0);
			dEndPos = GetCursorPositionEx(0);
			Main_OnCommandEx(ID_GOTO_SELITEM_START, 0, 0);
			dStartPos = GetCursorPositionEx(0);
		break;
		case eTIMESEGMENT_LOOP:
			Main_OnCommandEx(ID_GOTO_LOOP_END, 0, 0);
			dEndPos = GetCursorPositionEx(0);
			Main_OnCommandEx(ID_GOTO_LOOP_START, 0, 0);
			dStartPos = GetCursorPositionEx(0);
		break;
		case eTIMESEGMENT_PROJECT:
			Main_OnCommandEx(ID_GOTO_PROJECT_END, 0, 0);
			dEndPos = GetCursorPositionEx(0);
			Main_OnCommandEx(ID_GOTO_PROJECT_START, 0, 0);
			dStartPos = GetCursorPositionEx(0);
		break;
		//case eTIMESEGMENT_CURRENTMEASURE:
		//	Main_OnCommandEx(ID_GOTO_CURMEASURE_START, 0, 0);
		//	dStartPos = GetCursorPositionEx(0);
		//	Main_OnCommandEx(ID_GOTO_NEXTMEASURE_START, 0, 0);
		//	dEndPos = GetCursorPositionEx(0);
		//break;
		default:
		break;
	}
}

const char* GetTimeSegmentStr(TimeSegment timeSegment)
{
	switch(timeSegment)
	{
		case eTIMESEGMENT_TIMESEL			: return "Time selection";		break;
		case eTIMESEGMENT_PROJECT			: return "Project";				break;
		case eTIMESEGMENT_SELITEM			: return "Selected item";		break;
		case eTIMESEGMENT_LOOP				: return "Loop";				break;
		//case eTIMESEGMENT_CURRENTMEASURE	: return "Current measure";		break;
		default								: return "???";					break;
	}
}

void GetSelectedMediaItems(list<MediaItem*> &items)
{
	items.clear();
	int itemIdx = 0;
	while(MediaItem* item = GetSelectedMediaItem(0, itemIdx))
	{
		items.push_back(item);
		itemIdx++;
	}
}

void GetMediaItemTakes(MediaItem* item, list<MediaItem_Take*> &takes, bool bActiveOnly)
{
	takes.clear();

	if(bActiveOnly)
	{
		MediaItem_Take* take = GetActiveTake(item);
		if(take)
			takes.push_back(take);
	}

	else
	{
		int takeIdx = 0;
		while(MediaItem_Take* take = GetTake(item, takeIdx))
		{
			if(take)
				takes.push_back(take);
			takeIdx++;
		}
	}
}

void GetSelectedMediaTakes(list<MediaItem_Take*> &takes, bool bActiveOnly)
{
	takes.clear();
	list<MediaItem*> items;
	GetSelectedMediaItems(items);

	for(list<MediaItem*>::iterator item = items.begin(); item != items.end(); item++)
		GetMediaItemTakes(*item, takes, bActiveOnly);
}
