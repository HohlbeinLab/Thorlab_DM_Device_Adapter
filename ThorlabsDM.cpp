///////////////////////////////////////////////////////////////////////////////
// FILE:          MyDm.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Device adapter for Thorlab Deformable Mirror
//
// AUTHOR:        Abbas Jabermoradi October 2019



#include "TLDM.h"
#include "windows.h"
#include<cstdlib>
#include<cstring>
#include<string>
#include <sstream>
#include <sys/stat.h>
#include "../../MMDevice/ModuleInterface.h"
#include "../../MMDevice/MMDevice.h"
#include "../../MMDevice/DeviceBase.h"
#include "TLDFM.h"
#include "TLDFM_def.h"
#include "TLDFMX.h"
#include "TLDFMX_def.h"
#include "visa.h"
#include "visatype.h"
#include "vpptype.h"
#include <iostream>
#include <algorithm>
#include <stdint.h>
#include <fstream>


//#define  MAX_SEGMENTS            (40)
const char* g_DMname = "ThorLab Deformable Mirror";
const char* g_Hy;
const char* g_Hyt;
const char* t_txt_initpath  = "DMData/init/WavefrontCorrection.txt";
const char* t_savepath  = "DMData/WavefrontCorrection_save.txt";

inline bool fileexists (const std::string& name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
	{
        fclose(file);
        return true;
    } 
	else 
	{
        return false;
    }   
}

MODULE_API void InitializeModuleData()
{
	RegisterDevice(g_DMname, MM::GenericDevice, "DMP40");
}


MODULE_API MM::Device* CreateDevice(const char* deviceName)                  
{
	if (deviceName == 0) return 0;
	if (strcmp(deviceName, g_DMname)  == 0) return new TLDM();
	return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

TLDM::TLDM() :
port_("Undefined"),
	initialized_(false),
	instrumentHandle(VI_NULL),
	wfcpath_(t_txt_initpath),
    savepath_(t_savepath)
{
	
InitializeDefaultErrorMessages();
	SetErrorText(ERR_PORT_CHANGE_FORBIDDEN, "You can't change the port after device has been initialized.");
	SetErrorText(ERR_INVALID_DEVICE, "The selected plugin does not fit for the device.");

	// Name
	CreateProperty(MM::g_Keyword_Name, "DMP40", MM::String, true);

	// Description
	CreateProperty(MM::g_Keyword_Description, "Thorlabs Deformable Mirror", MM::String, true);
	//
   CPropertyAction* pAct = new CPropertyAction (this, &TLDM::OnPort);
   CreateProperty(MM::g_Keyword_Port, "Undefined", MM::String, false, pAct, true);  


}

TLDM::~TLDM()
{
	if (initialized_)
		Shutdown();
}

bool TLDM::Busy()
{
	return false;
}

void TLDM::GetName(char* name) const
{
	CDeviceUtils::CopyLimitedString(name, g_DMname);
}

// initialize function
int TLDM::Initialize()
{	
	//Validate the Device
	int nRet = ValidateDevice();
	if (DEVICE_OK != nRet)	return nRet;

	R=0;
	for(int jj=0; jj<MAX_Seg;jj++)
		{
	
			mirrorPattern1[jj] = 100.00;
		}
		for(int jj23=0; jj23<MAX_Seg;jj23++)
		{
	
			AllPattern[jj23] = 100.00;
		}
	for(int jk=0; jk<TLDFMX_MAX_ZERNIKE_TERMS;jk++)
		{
	
			zz[jk] = 0.00;
		}
	
	//for(int jk2=0; jk2<MAX_Seg;jk2++)
	//	{
	//
	//		segmentVoltage[jk2] = 100.00;
	//	}
	//Initial Value
	TLDFM_get_segment_maximum (instrumentHandle, &SeVMaximum);
	TLDFM_get_segment_minimum (instrumentHandle, &SeVMinimum);
	TLDFM_get_tilt_maximum (instrumentHandle, &TiVMaximum);
	TLDFM_get_tilt_minimum (instrumentHandle, &TiVMinimum);
	TLDFM_get_device_information (instrumentHandle, 0, VI_NULL, VI_NULL, VI_NULL , VI_NULL, VI_NULL);
	TLDFMX_get_parameters (instrumentHandle, &minimumZernikeAmplitude, &maximumZernikeAmplitude, VI_NULL, VI_NULL, VI_NULL);

	ViBoolean enabled;
	TLDFMX_get_pattern_range_check(instrumentHandle, &enabled);
	if (enabled == VI_FALSE)
		{
			enabled = VI_TRUE;
			TLDFMX_set_pattern_range_check (instrumentHandle, enabled);
		}
  
	
	//if (fileexists(t_txt_initpath))
	//	{
	//		TLDM::LoadWavefront(t_txt_initpath);
	//	}
	
   //Serial Number
   nRet = CreateStringProperty("Serial Number", Serial , true);
   assert(nRet == DEVICE_OK);

   	//Hysteresis Compensation Notification :)
   TLDFM_enabled_hysteresis_compensation (instrumentHandle, T_MIRROR,  &state0);
   TLDFM_enabled_hysteresis_compensation (instrumentHandle, T_TILT,  &state1);
   if(state0 == 1 && state1 == 1)
    {
	   Con = "ON";
    }
   nRet = CreateStringProperty("Hysteresis", Con.c_str() , true);
   assert(nRet == DEVICE_OK);

   
	//tilt element voltage control
	//TLDFM_get_tilt_voltage (instrumentHandle, 0, &tiltVoltage1);
	// pAct = new CPropertyAction (this, &TLDM::OnSetTiltVoltage1);
	//nRet = CreateFloatProperty("Tilt Segment 1 Voltage", tiltVoltage1 ,false, pAct);
	//assert(nRet == DEVICE_OK);
	//SetPropertyLimits("Tilt Segment 1 Voltage", TiVMinimum, TiVMaximum);
	//
	//
	//TLDFM_get_tilt_voltage (instrumentHandle, 1, &tiltVoltage2);
	//pAct = new CPropertyAction (this, &TLDM::OnSetTiltVoltage2);
	//nRet = CreateFloatProperty("Tilt Segment 2 Voltage",tiltVoltage2,false, pAct);
	//SetPropertyLimits("Tilt Segment 2 Voltage", TiVMinimum, TiVMaximum);
	//if (DEVICE_OK != nRet)	return nRet;
	//
	//TLDFM_get_tilt_voltage (instrumentHandle, 2, &tiltVoltage3);
	//pAct = new CPropertyAction (this, &TLDM::OnSetTiltVoltage3);
	//nRet = CreateFloatProperty("Tilt Segment 3 Voltage",tiltVoltage3,false, pAct);
	//SetPropertyLimits("Tilt Segment 3 Voltage", TiVMinimum, TiVMaximum);
	//if (DEVICE_OK != nRet)	return nRet;

	CPropertyAction*pAct = new CPropertyAction (this, &TLDM::OnSetTiltAmplitude);
	nRet = CreateFloatProperty("Amplitude",Amplitude,false, pAct);
	SetPropertyLimits("Amplitude", 0.0, 1.0);
	if (DEVICE_OK != nRet)	return nRet;

	pAct = new CPropertyAction (this, &TLDM::OnSetTiltAngle);
	nRet = CreateFloatProperty("Angle",Angle,false, pAct);
	SetPropertyLimits("Angle", -180.0, 180.0);
	if (DEVICE_OK != nRet)	return nRet;

	pAct = new CPropertyAction(this, &TLDM::OnLoadWavefront);
	nRet = CreateProperty("Load wavefront correction", t_txt_initpath, MM::String, false, pAct);
	if (nRet!=DEVICE_OK)
	return nRet;

	pAct = new CPropertyAction(this, &TLDM::OnSaveCurrentPosition);
	nRet = CreateProperty("Save current position [input filename]", t_savepath, MM::String, false, pAct);
	if (nRet!=DEVICE_OK)
	   return nRet;

	//Create an array for 40 segments of mirror


		CPropertyActionEx *pActX = 0;
					
	for(ViInt32 Index = 0; Index < MAX_Seg ;++Index)
	{
      std::ostringstream ob;
      ob<<Index + 1;
      std::string propName = "Segment " + ob.str();
	  TLDFM_get_segment_voltage (instrumentHandle, Index, &segmentVoltage[Index]);
	  pActX = new CPropertyActionEx(this, &TLDM::OnSetSegmentVoltage, Index);
      nRet = CreateFloatProperty(propName.c_str(), segmentVoltage[Index], false, pActX);
      SetPropertyLimits(propName.c_str(), SeVMinimum ,SeVMaximum);
	  if (DEVICE_OK != nRet)	return nRet;
	}


	pAct = new CPropertyAction (this, &TLDM::OnResetSegments);
   std::string propName3 = "Reset Segments";
   CreateStringProperty(propName3.c_str(), "		", false, pAct);
   AddAllowedValue(propName3.c_str(), "Click");
   AddAllowedValue(propName3.c_str(), "		");

   //Mirror Tempreture
   pAct = new CPropertyAction (this, &TLDM::onGetTempruture);
   nRet = CreateFloatProperty("Mirror Tempreture", 0, false, pAct);
   assert(nRet == DEVICE_OK);
   
   //Zernik polynomials
		
	for(int in = 0; in < TLDFMX_MAX_ZERNIKE_TERMS ; ++in)
	{
      //std::ostringstream ol;
      //ol<<in + 4;
      //std::string propName = "Zernike " + ol.str();
		std::string propName;
		if(in == 0) propName = "Z2-2";
		if(in == 1) propName = "Z20";
		if(in == 2) propName = "Z22";
		if(in == 3) propName = "Z3-3";
		if(in == 4) propName = "Z3-1";
		if(in == 5) propName = "Z31";
		if(in == 6) propName = "Z33";
		if(in == 7) propName = "Z4-4";
		if(in == 8) propName = "Z4-2";
		if(in == 9) propName = "Z40";
		if(in == 10) propName = "Z42";
		if(in == 11) propName = "Z44";
	  pActX = new CPropertyActionEx(this, &TLDM::OnZ4_Ast45, in);
      nRet = CreateFloatProperty(propName.c_str(), zz[in], false, pActX);
      SetPropertyLimits(propName.c_str(), minimumZernikeAmplitude ,maximumZernikeAmplitude);
	
	  if (DEVICE_OK != nRet)	return nRet;
	  	  
	}

	pAct = new CPropertyAction (this, &TLDM::OnResetZernikes);
   std::string propName4 = "Reset Zernikes";
   CreateStringProperty(propName4.c_str(), "		", false, pAct);
   AddAllowedValue(propName4.c_str(), "Click");
   AddAllowedValue(propName4.c_str(), "		");			
			    
			    
	pAct = new CPropertyAction(this, &TLDM::OnApplyZernmodes);
	nRet = CreateProperty("ApplyZernikes", "0", MM::Integer, false, pAct);
	if (nRet!=DEVICE_OK)
	   return nRet;
		    
			    

   pAct = new CPropertyAction (this, &TLDM::OnRelax);
   std::string propName2 = "Relax the Mirror";
   CreateStringProperty(propName2.c_str(), "OFF", false, pAct);
   AddAllowedValue(propName2.c_str(), "ON");
   AddAllowedValue(propName2.c_str(), "OFF");
	////////////////////////////////////////////////////////////////////
	
	initialized_ = true;

	return DEVICE_OK;
}

// Shut down function
int TLDM::Shutdown()
{
	if (instrumentHandle)
   {
		TLDFMX_close (instrumentHandle);
		instrumentHandle = VI_NULL;
   }
   return DEVICE_OK;
}

int TLDM::ValidateDevice(void)
{
	int nRet = DEVICE_OK;
	ViUInt32	deviceCount,segmentCount,tiltCount;	
	ViReal64	SeVMaximum,SeVMinimum,TiVMaximum,TiVMinimum;
	ViBoolean  state1,state2;
	



	// get the list of available devices
	nRet = TLDFM_get_device_count (VI_NULL, &deviceCount);
	if (nRet ==  0xBFFF0011 || deviceCount == 0)
		nRet = ERR_NO_DEVICE_CONNECTED;

	if (VI_SUCCESS != nRet)
		return nRet;

	// initialize the first device
	nRet = TLDFM_get_device_information (VI_NULL, 0, MName, IName, Serial, VI_NULL,  resName);
	if (VI_SUCCESS != nRet)
		return nRet;
	
	nRet = TLDFMX_init (resName , VI_TRUE, VI_TRUE, &instrumentHandle);
	if (VI_SUCCESS != nRet)
		return nRet;
	// get some device parameter
	
	nRet = TLDFM_get_segment_count (instrumentHandle, &segmentCount); //Counting the number of diformable mirror segments
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_get_segment_maximum (instrumentHandle, &SeVMaximum); //Get the maximum voltage of segments
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_get_segment_minimum (instrumentHandle, &SeVMinimum); //Get the maximum voltage of segments
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_get_tilt_count (instrumentHandle, &tiltCount); //Counting the number of tilt elements of diformable mirror
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_get_tilt_maximum (instrumentHandle, &TiVMaximum); // Get the maximum voltage of tilt elements diformable mirror
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_get_tilt_minimum (instrumentHandle, &TiVMinimum); // Get the minimum voltage of tilt elements of diformable mirror
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_enabled_hysteresis_compensation (instrumentHandle, T_MIRROR, &state1); // Check the hysteresis compensation on/off for mirror segments 
	if (VI_SUCCESS != nRet)
		return nRet;

	nRet = TLDFM_enabled_hysteresis_compensation (instrumentHandle, T_TILT, &state2); // Check the hysteresis compensation on/off for tilt elements of deformable mirror 
	if (VI_SUCCESS != nRet)
		return nRet;

	return nRet;
}

///////////////////////////////////////////////////////////////////////////////
// Action handlers
// Handle changes and updates to property values.
///////////////////////////////////////////////////////////////////////////////
int TLDM::OnPort(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	m_serialNumber = Serial;
	if (eAct == MM::BeforeGet)
	{
		pProp->Set(Serial);
	}
	else if (eAct == MM::AfterSet)
	{
		if (instrumentHandle)
		{
			// revert
			pProp->Set(Serial);
			return ERR_PORT_CHANGE_FORBIDDEN;
		}

		pProp->Get(m_serialNumber);
	}

	return DEVICE_OK;
}

int TLDM::OnSetSegmentVoltage(MM::PropertyBase* pProp, MM::ActionType eAct , ViInt32 Index)
{
		int nRet = DEVICE_OK;

		if (eAct == MM::BeforeGet)
		{
				
			//TLDFMX_set_voltages_setpoint (instrumentHandle, segmentVoltage);
			
			TLDFMX_set_single_voltage_setpoint (instrumentHandle,Index, segmentVoltage[Index]);
			nRet = TLDFM_set_segment_voltage(instrumentHandle, Index, segmentVoltage[Index]);
			TLDFM_get_segment_voltage (instrumentHandle, Index, &segmentVoltage[Index]);
			pProp->Set(segmentVoltage[Index]);
			
			
			
			if(VI_SUCCESS != nRet)
				return nRet;
			
			

		}
		else if (eAct == MM::AfterSet)
		{
			// get property
			//nRet = TLDFM_set_segment_voltage(instrumentHandle, Index, segmentVoltage[Index]);
			//TLDFMX_set_single_voltage_setpoint (instrumentHandle,Index, segmentVoltage[Index]);
			TLDFM_get_segment_voltage (instrumentHandle, Index, &segmentVoltage[Index]);
			
			pProp->Get(segmentVoltage[Index]);

			
			if(VI_SUCCESS != nRet)
				return nRet;
					
		}
	
	return nRet;
}
int TLDM::OnResetSegments (MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int nRet = DEVICE_OK;
	std::string val = "		";
		if (eAct == MM::BeforeGet)
		{

			pProp->Set(val.c_str());
				if(VI_SUCCESS != nRet)
					return nRet;
		
			
			

		}
		else if (eAct == MM::AfterSet)
		{
			// get property
			val ="		";
				pProp->Get(val);

			for(int jk2=0; jk2<MAX_Seg;jk2++)
					{
	
						segmentVoltage[jk2] = 100.00;
						TLDFM_set_segment_voltage(instrumentHandle, jk2, segmentVoltage[jk2]);
					}
			
			if(VI_SUCCESS != nRet)
				return nRet;
					
		}
	
	return nRet;

}
int TLDM::OnResetZernikes (MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int nRet = DEVICE_OK;
	std::string val = "		";
		if (eAct == MM::BeforeGet)
		{

			pProp->Set(val.c_str());
				if(VI_SUCCESS != nRet)
					return nRet;
		
			
			

		}
		else if (eAct == MM::AfterSet)
		{
			// get property
			val ="		";
				pProp->Get(val);

			for (int rr= 0 ; rr< TLDFMX_MAX_ZERNIKE_TERMS ; rr++)
			{
			
				zz[rr] = 0;
			
			}

			TLDFMX_calculate_zernike_pattern (instrumentHandle,  Z_Ast45_Flag | Z_Def_Flag | Z_Ast0_Flag | Z_TreY_Flag | Z_ComX_Flag | Z_ComY_Flag | Z_TreX_Flag | Z_TetY_Flag | Z_SAstY_Flag | Z_SAb3_Flag  | Z_SAstX_Flag | Z_TetX_Flag , zz, mirrorPattern1);	
			nRet = TLDFM_set_segment_voltages (instrumentHandle, mirrorPattern1);
			for(int jk2=0; jk2<MAX_Seg;jk2++)
					{
	
						segmentVoltage[jk2] = 100.00;
						TLDFM_set_segment_voltage(instrumentHandle, jk2, segmentVoltage[jk2]);
					}
			
			
			if(VI_SUCCESS != nRet)
				return nRet;
					
		}
	
	return nRet;

}
//int TLDM::OnSetTiltVoltage1(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
//	int nRet = DEVICE_OK;
//	ViUInt32 segmentCount;
//	ViUInt32 tiltIndex = 0;
//
//	if (eAct == MM::BeforeGet)
//	{
//
//			nRet = TLDFM_set_tilt_voltage (instrumentHandle, tiltIndex, tiltVoltage1);
//			if(VI_SUCCESS != nRet)
//				return nRet;
//			pProp->Set(tiltVoltage1);
//
//	}
//	else if (eAct == MM::AfterSet)
//	{
//		 //get property
//				pProp->Get(tiltVoltage1);
//				//nRet = TLDFM_set_tilt_voltage (instrumentHandle, tiltIndex, tiltVoltage1);
//				if(VI_SUCCESS != nRet)
//					return nRet;
//	}
//
//	return nRet;
//}
//int TLDM::OnSetTiltVoltage2(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
//	int nRet = DEVICE_OK;
//	ViUInt32 segmentCount;
//	TLDFM_get_segment_count (instrumentHandle, &segmentCount);
//	ViUInt32 tiltIndex = 1;
//
//	if (eAct == MM::BeforeGet)
//	{
//
//			nRet = TLDFM_set_tilt_voltage (instrumentHandle, tiltIndex, tiltVoltage2);
//			if(VI_SUCCESS != nRet)
//				return nRet;
//			pProp->Set(tiltVoltage2);
//	}
//	else if (eAct == MM::AfterSet)
//	{
//		// get property
//				pProp->Get(tiltVoltage2);
//				if(VI_SUCCESS != nRet)
//					return nRet;
//	}
//
//	return nRet;
//}
//int TLDM::OnSetTiltVoltage3(MM::PropertyBase* pProp, MM::ActionType eAct)
//{
//	int nRet = DEVICE_OK;
//	ViUInt32 segmentCount;
//	TLDFM_get_segment_count (instrumentHandle, &segmentCount);
//	ViUInt32 tiltIndex = 2;
//	//ViReal64 tiltVoltage = 0;
//
//	if (eAct == MM::BeforeGet)
//	{
//			nRet = TLDFM_set_tilt_voltage (instrumentHandle, tiltIndex, tiltVoltage3);
//			if(VI_SUCCESS != nRet)
//				return nRet;
//			pProp->Set(tiltVoltage3);
//
//	}
//	else if (eAct == MM::AfterSet)
//	{
//		// get property
//
//				pProp->Get(tiltVoltage3);
//				if(VI_SUCCESS != nRet)
//					return nRet;
//	}
//
//	return nRet;
//}
int TLDM::onGetTempruture(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::BeforeGet)
   {
	  TLDFM_get_temperatures (instrumentHandle, &p1, &p2, &mirrorTemperatur, &p3);
      pProp->Set(mirrorTemperatur);
   }
   else if (eAct == MM::AfterSet)
   {
	  TLDFM_get_temperatures (instrumentHandle, &p1, &p2, &mirrorTemperatur, &p3);
      pProp->Get(mirrorTemperatur);
   }
   return DEVICE_OK;

}
int TLDM::OnRelax(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int nRet = DEVICE_OK, Param;
	ViUInt32  relaxPart = T_BOTH;
	ViBoolean isFirstStep = VI_TRUE, reload = VI_TRUE;
	//double q = 0;
	ViInt32   remainingRelaxSteps;
	//T_MIRROR         (0)
	//T_TILT           (1) these three definitions are used for device part
	//T_BOTH           (2)
	std::string val = "OFF";

		if (eAct == MM::BeforeGet)
		{		
			//

				

					pProp->Set(val.c_str());
					if(VI_SUCCESS != nRet)
						return nRet;
			
			

			 return DEVICE_OK;


		}


		else if (eAct == MM::AfterSet)
		{		
				val ="OFF";
				pProp->Get(val);
				TLDFMX_relax(instrumentHandle, relaxPart, isFirstStep, reload, relaxPatternMirror, relaxPatternArm, &remainingRelaxSteps);
				nRet = TLDFM_set_voltages (instrumentHandle, relaxPatternMirror, relaxPatternArm);
				if(VI_SUCCESS != nRet)
				return nRet;
				isFirstStep = VI_FALSE;
				while(0 < remainingRelaxSteps)
				{
					TLDFMX_relax(instrumentHandle, relaxPart, isFirstStep, reload, relaxPatternMirror, relaxPatternArm, &remainingRelaxSteps);

					nRet = TLDFM_set_voltages (instrumentHandle, relaxPatternMirror, relaxPatternArm);
					if(VI_SUCCESS != nRet)
						return nRet;
				}
				if(VI_SUCCESS != nRet)
					return nRet;
		}

	return nRet;
}
int TLDM::OnSetTiltAmplitude(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int nRet = DEVICE_OK;


	if (eAct == MM::BeforeGet)
	{		
		
		    
			nRet = TLDFM_set_tilt_amplitude_angle (instrumentHandle, Amplitude, Angle);
			if(VI_SUCCESS != nRet)
				return nRet;
			pProp->Set(Amplitude);

	}
	else if (eAct == MM::AfterSet)
	{
		 //get property
				pProp->Get(Amplitude);
				if(VI_SUCCESS != nRet)
					return nRet;
	}

	return nRet;

}
int TLDM::OnSetTiltAngle(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	int nRet = DEVICE_OK;
	
	if (eAct == MM::BeforeGet)
	{
			nRet = TLDFM_set_tilt_amplitude_angle (instrumentHandle, Amplitude, Angle);
			if(VI_SUCCESS != nRet)
				return nRet;
			pProp->Set(Angle);

	}
	else if (eAct == MM::AfterSet)
	{
		 //get property
				pProp->Get(Angle);
				if(VI_SUCCESS != nRet)
					return nRet;
	}

	return nRet;

}

int TLDM::OnZ4_Ast45(MM::PropertyBase* pProp, MM::ActionType eAct, long in)
{
	ViStatus nRet = DEVICE_OK;
	if (eAct == MM::BeforeGet)
	{		

		pProp->Set(zz[in]);
			
			

			
			if(VI_SUCCESS != nRet)
				return nRet;
	}
	else if (eAct == MM::AfterSet)
	{		

			//TLDFM_get_segment_voltages (instrumentHandle, segmentVoltage);
			//TLDFM_set_segment_voltage(instrumentHandle, cou, segmentVoltage[cou]);
		    TLDFMX_calculate_zernike_pattern (instrumentHandle,  Z_Ast45_Flag | Z_Def_Flag | Z_Ast0_Flag | Z_TreY_Flag | Z_ComX_Flag | Z_ComY_Flag | Z_TreX_Flag | Z_TetY_Flag | Z_SAstY_Flag | Z_SAb3_Flag  | Z_SAstX_Flag | Z_TetX_Flag , zz, mirrorPattern1);
			nRet = TLDFM_set_segment_voltages (instrumentHandle, mirrorPattern1);
			TLDFM_get_segment_voltages (instrumentHandle, segmentVoltage);
			pProp->Get(zz[in]);
			if(VI_SUCCESS != nRet)
				return nRet;
	}


	return nRet;
}

int TLDM::LoadWavefront(std::basic_string<char>  path)
{
	if (fileexists(path))
	{
		wfcpath_ = path;			
		 
		    std::ifstream input(wfcpath_);
			  if(!input) 
			     {
			 	   return ERR_FILE_NONEXIST;
				 }
			ViReal64 num;
			int cou = 0;
			while(!input.eof()) 
			{	
				input >> num;
				segmentVoltage[cou] = num;
				cou++;
			}

			for(int cou = 0; cou< MAX_Seg ; ++cou)
			  {
				TLDFMX_set_single_voltage_setpoint (instrumentHandle,cou, segmentVoltage[cou]);
			  	TLDFM_set_segment_voltage(instrumentHandle, cou, segmentVoltage[cou]);
			  }
		//for(int jk=0; jk<TLDFMX_MAX_ZERNIKE_TERMS;jk++)
		//	{
		//		zz[jk] = 0.00;
		//	}
		//for(int jk=0; jk<TLDFMX_MAX_ZERNIKE_TERMS;jk++)
		//	{
		//		zzss[jk] = 0.00;
		//	}


		Sleep(10);
	}
	else
	{
		return ERR_FILE_NONEXIST;
	}
	return DEVICE_OK;



}

int TLDM::OnLoadWavefront(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::BeforeGet)
		{
			
		 pProp->Set(wfcpath_.c_str());
		 //return LoadWavefront(wfcpath_);
		}
	else if (eAct == MM::AfterSet)
		{
		   //std::basic_string<char> path;
		   pProp->Get(wfcpath_);
		   return LoadWavefront(wfcpath_);
		  //return LoadWavefront(wfcpath_);
		}
   return DEVICE_OK;



}
int TLDM::SaveCurrentPosition(std::basic_string<char> path)
{
	savepath_ = path;
	std::ofstream saveFile (savepath_);
	for(int cou = 0; cou< MAX_Seg ; ++cou)
	  {
	  	TLDFM_get_segment_voltage (instrumentHandle, cou, &segmentVoltage[cou]);
		saveFile << segmentVoltage[cou] <<std::endl;;
	  	//TLDFM_set_segment_voltage(instrumentHandle, cou, segmentVoltage[cou]);
	  }

	 saveFile.close(); 
	return DEVICE_OK;
}
int TLDM::OnSaveCurrentPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
   pProp->Set(savepath_.c_str()); 
   }
   else if (eAct == MM::AfterSet)
   {
      std::basic_string<char> path;;
      pProp->Get(path);
      return SaveCurrentPosition(path);
   }
   return DEVICE_OK;
}
int TLDM::OnApplyZernmodes(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   
   return DEVICE_OK;
}