
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Examples for the report "Making external components for 1C mobile platform for Android""
// at the conference INFOSTART 2018 EVENT EDUCATION https://event.infostart.ru/2018/
//
// Sample 1: Delay in code
// Sample 2: Getting device information
// Sample 3: Device blocking: receiving external event about changing of sceen
//
// Copyright: Igor Kisil 2018
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../include/AddInDefBase.h"
#include "../include/IAndroidComponentHelper.h"
#include "../jni/jnienv.h"
#include "../include/IMemoryManager.h"
#include <string>
#include <iostream>
#include <map>

/* Wrapper calling class LockState from java build ru.infostart.education */

class LockState
{
private:

	jclass cc;
	jobject obj;
	jmethodID   methodID_getErrorDescr;
	jobject gobj;
	IAddInDefBaseEx* pAddIn;
	
	

public:

	jobject activity;

	LockState();
	~LockState();

	void Initialize(IAddInDefBaseEx*);

	void StartConnect(std::string adressdev) const;
	void StopConnect() const; // End of monitoring

	void ReadStreamStart(int cellID) const;
	void ReadStreamStop() const; // End of monitoring


	bool isInitialized() const;

	wchar_t* GetInfo();
	std::wstring GetJavaTest();
	std::wstring GetBluetoothDevices();

	void ReadDataRFID(std::string sAddresRFID, int cellID);
	void WriteData(std::string sAddresRFID, int cellID, std::string sData);
	
};

extern "C"
{
	JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnLockChanged(JNIEnv* env, jclass jClass, jlong pObject);
	JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnReadDataRFID(JNIEnv* env, jclass jClass, jlong pObject, jstring sValue, jstring sEventName);
	JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnReadCellRFID(JNIEnv* env, jclass jClass, jlong pObject, jstring sValue);
}