///////////////////////////////////////////////////////////////////////////////
// FILE:          ThorlabsDMP40.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Device adapter for Thorlab Deformable Mirror
//
// AUTHOR:        Abbas Jabermoradi October 2019


#pragma once

#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "../../MMDevice/ImgBuffer.h"
#include "../../MMDevice/DeviceThreads.h"
#include <string>
#include <sstream>
#include "TLDFM.h"
#include "TLDFM_def.h"
#include "TLDFMX.h"
#include "TLDFMX_def.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <visa.h>
#include <iostream>
#include "visatype.h"
#include "vpptype.h"
#include <algorithm>
#include <stdint.h>

#define MAX_Seg 40
#define MAX_Tilt 3
//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_NO_DEVICE_CONNECTED		103
#define ERR_PORT_CHANGE_FORBIDDEN	101
#define ERR_INVALID_DEVICE			102
#define ERR_FILE_NONEXIST			105

//Character



class TLDM : public	CGenericBase<TLDM>
{
public:
	TLDM(void);
	~TLDM(void);

   // Device API
   // ----------
   int Initialize();
   int Shutdown();
   void GetName(char* name) const; 
   bool Busy();
   
    //Deformable mirror API
   ViSession instrumentHandle;

   // action interface
   // ----------------
   	ViReal64	SeVMaximum,SeVMinimum,TiVMaximum,TiVMinimum, mirrorTemperatur,p1,p2,p3;
	ViReal64  minimumZernikeAmplitude, maximumZernikeAmplitude;
	ViBoolean NM,NT;
   int OnGetSegmentVoltage    (MM::PropertyBase* pProp, MM::ActionType eAct); 
   int OnSetSegmentVoltage    (MM::PropertyBase* pProp, MM::ActionType eAct , ViInt32 Index);
   ViChar sss;
   ViChar MName[TLDFM_BUFFER_SIZE],
		  IName[TLDFM_MAX_INSTR_NAME_LENGTH],
		  Serial[TLDFM_MAX_SN_LENGTH],
		  resName[VI_FIND_BUFLEN];
   ViReal64 segmentVoltage[MAX_Seg],
			segmentVoltages[MAX_Seg],
			segmentVoltage2[MAX_Seg],
			ms,
			tiltVoltage2,
			tiltVoltage3,
			zz[TLDFMX_MAX_ZERNIKE_TERMS],
			zzss[TLDFMX_MAX_ZERNIKE_TERMS];
   ViReal64 mirrorPattern1[MAX_Seg];
   ViReal64 AllPattern[MAX_Seg];
   ViReal64  relaxPatternMirror[MAX_Seg];
   ViReal64  relaxPatternArm[MAX_Tilt];
   ViReal64 tiltVoltages[MAX_Tilt];
   ViReal64 tiltVoltage1,Amplitude,Angle;
   std::string 	m_serialNumber;
   ViUInt32 R;
   //double ab;
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGetSegmentVoltages    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetSegmentVoltages (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGetTiltVoltage (MM::PropertyBase* pProp, MM::ActionType eAct);

   int OnApplyZernmodes(MM::PropertyBase* pProp, MM::ActionType eAct);
   int SaveCurrentPosition(std::basic_string<char> path);
   int OnSaveCurrentPosition(MM::PropertyBase* pProp, MM::ActionType eAct);
   int LoadWavefront(std::basic_string<char>  path);
   int OnLoadWavefront(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltVoltage1 (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResetSegments (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResetZernikes (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltVoltage2 (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltVoltage3 (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGetTiltVoltages (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltVoltages (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltAmplitude (MM::PropertyBase* pProp, MM::ActionType eAct);
   int onGetTempruture(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSetTiltAngle (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnEnabledHysteresisCompensationArms (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnEnabledHysteresisCompensationMirror (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnEnableHysteresisCompensation (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnGetTemperatures (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRelax (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnZ4_Ast45 (MM::PropertyBase* pProp, MM::ActionType eAct, long in);
   int ValidateDevice(void);

   const char* SER;
   const char* Temp;
   ViUInt32  notif;
   ViBoolean state0 , state1;
   std::string wfcpath_;
   std::string savepath_;

protected:
   bool initialized_;
   std::string port_;
   MM::Device *device_;
   MM::Core *core_;
};
