
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

#include <wchar.h>
#include "LockState.h"
#include "ConversionWchar.h"
//#include "../include/AddInDefBase.h"
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "AddInNative.h"


std::wstring jstring2wstring(JNIEnv* jenv, jstring aStr)
{
	std::wstring result;

	if (aStr)
	{
		const jchar* pCh = jenv->GetStringChars(aStr, 0);
		jsize len = jenv->GetStringLength(aStr);
		const jchar* temp = pCh;
		while (len > 0)
		{
			result += *(temp++);
			--len;
		}
		jenv->ReleaseStringChars(aStr, pCh);
	}
	return result;
}

LockState::LockState() : cc(nullptr), obj(nullptr)
{
	
}

LockState::~LockState()
{
	if (obj)
	{

	}
}

void LockState::Initialize(IAddInDefBaseEx* cnn)
{
	if (!obj)
	{
		IAndroidComponentHelper* helper = (IAndroidComponentHelper*)cnn->GetInterface(eIAndroidComponentHelper);
		if (helper)
		{
			WCHAR_T* className = nullptr;
			convToShortWchar(&className, L"ru.infostart.education3.LockState");
			jclass ccloc = helper->FindClass(className);

			delete[] className;
			className = nullptr;
			if (ccloc)
			{
				JNIEnv* env = getJniEnv();
				cc = static_cast<jclass>(env->NewGlobalRef(ccloc));
				env->DeleteLocalRef(ccloc);
				activity = helper->GetActivity();
				// call of constructor for java class

				jmethodID methID = env->GetMethodID(cc, "<init>", "(Landroid/app/Activity;J)V");
				jobject objloc = env->NewObject(cc, methID, activity, (jlong)cnn);
				obj = static_cast<jobject>(env->NewGlobalRef(objloc));
				env->DeleteLocalRef(objloc);
				methID = env->GetMethodID(cc, "show", "()V");
				env->CallVoidMethod(obj, methID);
				env->DeleteLocalRef(activity);

				
				pAddIn = (IAddInDefBaseEx*)obj;
			}
		}
	}
}

void LockState::StartConnect(std::string adressdev) const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		jmethodID methID = env->GetMethodID(cc, "startConnect", "()V");
		env->CallVoidMethod(obj, methID);
	}
}

void LockState::StopConnect() const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		jmethodID methID = env->GetMethodID(cc, "stopConnect", "()V");
		env->CallVoidMethod(obj, methID);

	}
}

void LockState::ReadStreamStart(int cellID) const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		jmethodID methID = env->GetMethodID(cc, "ReadStreamStart", "(I)V");
		env->CallVoidMethod(obj, methID, cellID);
	}
}
void LockState::ReadStreamStop() const
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		jmethodID methID = env->GetMethodID(cc, "ReadStreamStop", "()V");
		env->CallVoidMethod(obj, methID);
	}
}



bool LockState::isInitialized() const
{
	if (!obj)
	{
		return false;
	}
	else
	{
		return true;
	}
}

wchar_t* LockState::GetInfo()
{
	
	return (wchar_t*)"Test C++";
}

void LockState::ReadDataRFID(std::string sAddresRFID, int cellID)
{
	if (obj)
	{
		JNIEnv* env = getJniEnv();

		jstring sAddresRFID_jstr = env->NewStringUTF(sAddresRFID.c_str());
		
		jmethodID methID = env->GetMethodID(cc, "readDataRFID", "(Ljava/lang/String;I)V");
		
		if (!methID) return;

		env->CallVoidMethod(obj, methID, sAddresRFID_jstr, cellID);
	}

}
void LockState::WriteData(std::string sAddresRFID, int cellID, std::string sData)
{
	if (obj)
	{

		JNIEnv* env = getJniEnv();

		jstring sAddresRFID_jstr = env->NewStringUTF(sAddresRFID.c_str());
		jstring sData_jstr = env->NewStringUTF(sData.c_str());

		jmethodID methID = env->GetMethodID(cc, "writeDataRFID", "(Ljava/lang/String;I)V");
		env->CallVoidMethod(obj, methID, sData_jstr, cellID);

	}

}

std::wstring LockState::GetJavaTest()
{
	std::wstring ddd;
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "GetTestJava", "()Ljava/lang/String;");
		jstring err = (jstring) env->CallObjectMethod(obj, methID);

		
		ddd = jstring2wstring(env, err);
	}

	return ddd;
}


char* js2c(JNIEnv* env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = env->FindClass("java/lang/String");
	jstring strencode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr = (jbyteArray)env->CallObjectMethod(jstr, mid, strencode);
	jsize alen = env->GetArrayLength(barr);
	jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	env->ReleaseByteArrayElements(barr, ba, 0);
	return rtn;
}

std::wstring LockState::GetBluetoothDevices()
{
	
	std::wstring mapOut;
	if (obj)
	{
		JNIEnv* env = getJniEnv();
		jmethodID methID = env->GetMethodID(cc, "GetBluetoothDevices", "()Ljava/lang/String;");
		jstring err = (jstring)env->CallObjectMethod(obj, methID);


		mapOut = jstring2wstring(env, err);
	}
	return mapOut;
}



static const wchar_t g_EventSource[] = L"RFIDReader";
static const wchar_t g_EventName[] = L"ReadData";
static const wchar_t g_DataSource[] = L"_";

static WcharWrapper s_EventSource(g_EventSource);
static WcharWrapper s_EventName(g_EventName);
static WcharWrapper s_DataSource(g_DataSource);

JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnLockChanged(JNIEnv* env, jclass jClass, jlong pObject)
{
	IAddInDefBaseEx* pAddIn = (IAddInDefBaseEx*)pObject;
	pAddIn->ExternalEvent(s_EventSource, s_EventName, nullptr);
}

JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnReadDataRFID(JNIEnv* env, jclass jClass, jlong pObject, jstring sValue, jstring sEventName)
{
	
	IAddInDefBaseEx* pAddIn = (IAddInDefBaseEx*)pObject;
	
	
	std::wstring ws_data = jstring2wstring(env, sValue);
	static WCHAR_T* retData = 0;
	convToShortWchar(&retData, ws_data.c_str());
	

	std::wstring ws_event = jstring2wstring(env, sEventName);
	static WCHAR_T* retEvent = 0;
	convToShortWchar(&retEvent, ws_event.c_str());
	

	pAddIn->ExternalEvent(s_EventSource, retEvent, retData);
}


JNIEXPORT void JNICALL Java_ru_infostart_education3_LockState_OnReadCellRFID(JNIEnv* env, jclass jClass, jlong pObject, jstring sValue)
{

	WcharWrapper l_EventName(L"ReadDataCell");

	IAddInDefBaseEx* pAddIn = (IAddInDefBaseEx*)pObject;

	std::wstring ws_data = jstring2wstring(env, sValue);
	static WCHAR_T* retData = 0;
	convToShortWchar(&retData, ws_data.c_str());


	pAddIn->ExternalEvent(s_EventSource, l_EventName, retData);


}

	