

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

#include "AddInNative.h"
#include "ConversionWchar.h"
#include "wchar.h"
#include <chrono>
#include <thread>
#include "../jni/jnienv.h"
#include "../include/IAndroidComponentHelper.h"

static const wchar_t *g_PropNames[] = 
{
	L"DeviceInfo",
	L"Screen"
};

static const wchar_t *g_MethodNames[] =
{
	L"Delay",
	L"GetTestInfo",
	L"GetTestInfoJava",
	L"GetListDevice",
	L"StartConnect",
	L"StopConnect",
	L"ReadStreamStart",
	L"ReadStreamStop",
	L"ReadData",
	L"WriteData"
};

static const wchar_t *g_PropNamesRu[] = 
{
	L"ОписаниеУстройства",
	L"Экран"

};

static const wchar_t *g_MethodNamesRu[] =
{
	L"Пауза",
	L"ПолучитьТестовуюСтроку",
	L"ПолучитьТестовуюСтрокуJava",
	L"ПолучитьСписокУстройств",
	L"СтартСоединения",
	L"СтопСоединения",
	L"ЗапуститьЧтениеПотока",
	L"ОстановитьЧтениеПотока",
	L"Прочитать",
	L"Записать"

};

static const wchar_t g_ComponentNameAddIn[] = L"Infostart2018";
static WcharWrapper s_ComponentClass(g_ComponentNameAddIn);
// This component supports 2.1 version
const long g_VersionAddIn = 2102;
static AppCapabilities g_capabilities = eAppCapabilitiesInvalid;

//---------------------------------------------------------------------------//
long GetClassObject(const WCHAR_T* wsName, IComponentBase** pInterface)
{
	if (!*pInterface)
	{
		*pInterface = new Infostart2018AddIn();
		return (long)*pInterface;
	}
	return 0;
}

//---------------------------------------------------------------------------//
AppCapabilities SetPlatformCapabilities(const AppCapabilities capabilities)
{
	g_capabilities = capabilities;
	return eAppCapabilitiesLast;
}

//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pInterface)
{
	if (!*pInterface)
		return -1;

	delete *pInterface;
	*pInterface = 0;
	return 0;
}

//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	return s_ComponentClass;
}

Infostart2018AddIn::Infostart2018AddIn() : m_iConnect(nullptr), m_iMemory(nullptr), isScreenOn(false)
{
}

Infostart2018AddIn::~Infostart2018AddIn()
{
}

/////////////////////////////////////////////////////////////////////////////
// IInitDoneBase
//---------------------------------------------------------------------------//
bool Infostart2018AddIn::Init(void* pConnection)
{
	m_iConnect = (IAddInDefBaseEx*)pConnection;
	m_devState.Initialize(m_iConnect);
	return m_iConnect != nullptr;
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::setMemManager(void* mem)
{
	m_iMemory = (IMemoryManager*)mem;
	return m_iMemory != nullptr;
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::GetInfo()
{
	return g_VersionAddIn;
}

//---------------------------------------------------------------------------//
void Infostart2018AddIn::Done()
{
	m_iConnect = nullptr;
	m_iMemory = nullptr;
	m_devState.StopConnect();
}

/////////////////////////////////////////////////////////////////////////////
// ILanguageExtenderBase
//---------------------------------------------------------------------------//
bool Infostart2018AddIn::RegisterExtensionAs(WCHAR_T** wsExtensionName)
{
	const wchar_t *wsExtension = g_ComponentNameAddIn;
	uint32_t iActualSize = static_cast<uint32_t>(::wcslen(wsExtension) + 1);

	if (m_iMemory)
	{
		if (m_iMemory->AllocMemory((void**)wsExtensionName, iActualSize * sizeof(WCHAR_T)))
		{
			convToShortWchar(wsExtensionName, wsExtension, iActualSize);
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::GetNProps()
{
	// You may delete next lines and add your own implementation code here
	return ePropLast;
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::FindProp(const WCHAR_T* wsPropName)
{
	long plPropNum = -1;
	wchar_t* propName = 0;
	convFromShortWchar(&propName, wsPropName);

	plPropNum = findName(g_PropNames, propName, ePropLast);

	if (plPropNum == -1)
		plPropNum = findName(g_PropNamesRu, propName, ePropLast);

	delete[] propName;
	return plPropNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T* Infostart2018AddIn::GetPropName(long lPropNum, long lPropAlias)
{
	if (lPropNum >= ePropLast)
		return NULL;

	wchar_t *wsCurrentName = NULL;
	WCHAR_T *wsPropName = NULL;

	switch (lPropAlias)
	{
	case 0: // First language (english)
		wsCurrentName = (wchar_t*)g_PropNames[lPropNum];
		break;
	case 1: // Second language (local)
		wsCurrentName = (wchar_t*)g_PropNamesRu[lPropNum];
		break;
	default:
		return 0;
	}

	uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsPropName, iActualSize * sizeof(WCHAR_T)))
			convToShortWchar(&wsPropName, wsCurrentName, iActualSize);
	}

	return wsPropName;
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	switch (lPropNum)
	{
	// SAMPLE 2 START - Read device manufacturer and model
	case ePropDevice:
	{
		IAndroidComponentHelper* helper = (IAndroidComponentHelper*)m_iConnect->GetInterface(eIAndroidComponentHelper);
		pvarPropVal->vt = VTYPE_EMPTY;
		if (helper)
		{
			WCHAR_T* className = nullptr;
			convToShortWchar(&className, L"android.os.Build");
			jclass ccloc = helper->FindClass(className);
			delete[] className;
			className = nullptr;
			std::wstring wData{};
			if (ccloc)
			{
				JNIEnv* env = getJniEnv();
				jfieldID fldID = env->GetStaticFieldID(ccloc, "MANUFACTURER", "Ljava/lang/String;");
				jstring	jManufacturer = (jstring)env->GetStaticObjectField(ccloc, fldID);
				std::wstring wManufacturer = ToWStringJni(jManufacturer);
				env->DeleteLocalRef(jManufacturer);
				fldID = env->GetStaticFieldID(ccloc, "MODEL", "Ljava/lang/String;");
				jstring	jModel = static_cast<jstring>(env->GetStaticObjectField(ccloc, fldID));
				std::wstring wModel = ToWStringJni(jModel);
				env->DeleteLocalRef(jModel);
				if (wManufacturer.length())
					wData = wManufacturer + L": " + wModel;
				else
					wData = wModel;
				env->DeleteLocalRef(ccloc);
			}
			if (wData.length())
				ToV8String(wData.c_str(), pvarPropVal);
		}
		return true;
		break;
	}
	
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::SetPropVal(const long lPropNum, tVariant *varPropVal)
{
	switch (lPropNum)
	{
	
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::IsPropReadable(const long lPropNum)
{
	return true;
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::IsPropWritable(const long lPropNum)
{
	switch (lPropNum)
	{
	case ePropScreen:
		return true;
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::GetNMethods()
{
	return eMethLast;
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::FindMethod(const WCHAR_T* wsMethodName)
{
	long plMethodNum = -1;
	wchar_t* name = 0;
	convFromShortWchar(&name, wsMethodName);

	plMethodNum = findName(g_MethodNames, name, eMethLast);

	if (plMethodNum == -1)
		plMethodNum = findName(g_MethodNamesRu, name, eMethLast);

	delete[] name;

	return plMethodNum;
}

//---------------------------------------------------------------------------//
const WCHAR_T* Infostart2018AddIn::GetMethodName(const long lMethodNum, const long lMethodAlias)
{
	if (lMethodNum >= eMethLast)
		return NULL;

	wchar_t *wsCurrentName = NULL;
	WCHAR_T *wsMethodName = NULL;

	switch (lMethodAlias)
	{
	case 0: // First language (english)
		wsCurrentName = (wchar_t*)g_MethodNames[lMethodNum];
		break;
	case 1: // Second language (local)
		wsCurrentName = (wchar_t*)g_MethodNamesRu[lMethodNum];
		break;
	default:
		return 0;
	}

	uint32_t iActualSize = static_cast<uint32_t>(wcslen(wsCurrentName) + 1);

	if (m_iMemory && wsCurrentName)
	{
		if (m_iMemory->AllocMemory((void**)&wsMethodName, iActualSize * sizeof(WCHAR_T)))
			convToShortWchar(&wsMethodName, wsCurrentName, iActualSize);
	}

	return wsMethodName;
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::GetNParams(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethDelay:
	{
		return 1;
		break;
	}
	case eMethStartConnect:
	{
		return 1;
		break;
	}
	case eMethReadData:
	{
		return 2;
		break;
	}
	case eMethWriteData:
	{
		return 3;
		break;
	}
	case eMethReadStreamStart:
	{
		return 1;
		break;
	}

	default:
	{
		return 0;
		break;
	}
	}
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::GetParamDefValue(const long lMethodNum, const long lParamNum,	tVariant *pvarParamDefValue)
{
	switch (lMethodNum)
	{
	default:
		return false;
	}
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::HasRetVal(const long lMethodNum)
{
	switch (lMethodNum)
	{
	case eMethGetInfo:
	{
		return true;
		break;
	}
	case eMethGetInfoJava:
	{
		return true;
		break;
	}
	case eMethGetListDevice:
	{
		return true;
		break;
	}
	default:
		break;
	}
	return false;
}

//---------------------------------------------------------------------------//
bool Infostart2018AddIn::CallAsProc(const long lMethodNum, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{

	case eMethDelay:
	{
		long lDelay = numericValue(paParams);
		if (lDelay > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(lDelay));
		return true;
		break;
	}
	case eMethStartConnect:
	{
		std::string sAddresDev = "";

		m_devState.StartConnect(sAddresDev);
		return true;
		break;
	}
	case eMethStopConnect:
	{
		m_devState.StopConnect();
		return true;
		break;
	}
	case eMethReadStreamStart:
	{
		
		
		int cellID = numericValue(paParams);

		m_devState.ReadStreamStart(cellID);
		return true;
		break;

	}
	case eMethReadStreamStop:
	{

		m_devState.ReadStreamStop();
		return true;
		break;

	}



	case eMethReadData:
	{

		char* sAddresRFID = 0;
		int cellID = 0;
		int size = 0;
		char* mbstr = 0;
		wchar_t* wsTmp;

		switch (TV_VT(&(paParams[0])))
		{
		case VTYPE_PSTR:
		{
			sAddresRFID = TV_STR(&(paParams[0]));
			break;
		}
		case VTYPE_PWSTR:
		{
			::convFromShortWchar(&wsTmp,TV_WSTR(&(paParams[0])));
			size = wcstombs(0, wsTmp, 0) + 1;
			mbstr = new char[size];
			memset(mbstr,0,size);
			size = wcstombs(mbstr, wsTmp, getLenShortWcharStr(TV_WSTR(&(paParams[0]))));
			sAddresRFID = mbstr;

			break;
		}
		default:
			return false;

		}
		
		int cellID = numericValue(&(paParams[1]));
		
		m_devState.ReadDataRFID(sAddresRFID, cellID);
		
		return true;
		break;
	}
	case eMethWriteData:
	{

		return true;
		break;
	}
	default:
		return false;
	}
	return false;
}


//---------------------------------------------------------------------------//
bool Infostart2018AddIn::CallAsFunc(const long lMethodNum, tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	switch (lMethodNum)
	{
	case eMethGetInfo:
	{

		TV_VT(pvarRetValue) = VTYPE_I4;
		TV_I4(pvarRetValue) = static_cast<int32_t>(11);

		return true;
		
		break;
	}
	case eMethGetInfoJava:
	{
		
		std::wstring errDescr = m_devState.GetJavaTest();
		uint32_t iActualSize = static_cast<uint32_t>(errDescr.size() + 1);
		if (m_iMemory->AllocMemory((void**)& pvarRetValue->pwstrVal, iActualSize * sizeof(WCHAR_T)))
		{
			convToShortWchar(&pvarRetValue->pwstrVal, errDescr.c_str(), iActualSize);
			pvarRetValue->wstrLen = iActualSize - 1;
			TV_VT(pvarRetValue) = VTYPE_PWSTR;
			return true;
		}

		return true;
		break;
	}
	case eMethGetListDevice:
	{
		
		std::wstring errDescr = m_devState.GetBluetoothDevices();
		
		uint32_t iActualSize = static_cast<uint32_t>(errDescr.size() + 1);
		if (m_iMemory->AllocMemory((void**)& pvarRetValue->pwstrVal, iActualSize * sizeof(WCHAR_T)))
		{
			convToShortWchar(&pvarRetValue->pwstrVal, errDescr.c_str(), iActualSize);
			pvarRetValue->wstrLen = iActualSize - 1;
			TV_VT(pvarRetValue) = VTYPE_PWSTR;
			return true;
		}
		
		
		return true;
		break;
	}
	default:
		break;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// ILocaleBase
//---------------------------------------------------------------------------//
void Infostart2018AddIn::SetLocale(const WCHAR_T* loc)
{
}

//---------------------------------------------------------------------------//
void Infostart2018AddIn::addError(uint32_t wcode, const wchar_t* source, const wchar_t* descriptor, long code)
{
	if (m_iConnect)
	{
		WCHAR_T *err = 0;
		WCHAR_T *descr = 0;

		convToShortWchar(&err, source);
		convToShortWchar(&descr, descriptor);

		m_iConnect->AddError(wcode, err, descr, code);

		delete[] descr;
		delete[] err;
	}
}

//---------------------------------------------------------------------------//
long Infostart2018AddIn::findName(const wchar_t* names[], const wchar_t* name, const uint32_t size) const
{
	long ret = -1;
	for (uint32_t i = 0; i < size; i++)
	{
		if (!wcscmp(names[i], name))
		{
			ret = i;
			break;
		}
	}
	return ret;
}

void Infostart2018AddIn::ToV8String(const wchar_t* wstr, tVariant* par)
{
	if (wstr)
	{
		int len = wcslen(wstr);
		m_iMemory->AllocMemory((void**)&par->pwstrVal, (len + 1) * sizeof(WCHAR_T));
		convToShortWchar(&par->pwstrVal, wstr);
		par->vt = VTYPE_PWSTR;
		par->wstrLen = len;
	}
	else
		par->vt = VTYPE_EMPTY;
}

WCHAR_T* Infostart2018AddIn::ToV8StringJni(jstring jstr, int* lSize)
{
	WCHAR_T* ret = NULL;
	if (jstr)
	{
		JNIEnv *jenv = getJniEnv();
		*lSize = jenv->GetStringLength(jstr);
		const WCHAR_T* pjstr = jenv->GetStringChars(jstr, NULL);
		m_iMemory->AllocMemory((void**)&ret, (*lSize + 1) * sizeof(WCHAR_T));
		for (auto i = 0; i < *lSize; ++i)
			ret[i] = pjstr[i];
		ret[*lSize] = 0;
		jenv->ReleaseStringChars(jstr, pjstr);
	}
	return ret;
}

bool Infostart2018AddIn::isNumericParameter(tVariant* par)
{
	return par->vt == VTYPE_I4 || par->vt == VTYPE_UI4 || par->vt == VTYPE_R8;
}

long Infostart2018AddIn::numericValue(tVariant* par)
{
	long ret = 0;
	switch (par->vt)
	{
	case VTYPE_I4:
		ret = par->lVal;
		break;
	case VTYPE_UI4:
		ret = par->ulVal;
		break;
	case VTYPE_R8:
		ret = par->dblVal;
		break;
	}
	return ret;
}
std::string Infostart2018AddIn::stringValue(tVariant* par)
{
	std::string ret = "";
	switch (par->vt)
	{
	case VTYPE_PSTR:
		ret = par->lVal;
		break;
	}
	return ret;
}

std::wstring Infostart2018AddIn::ToWStringJni(jstring jstr)
{
	std::wstring ret;
	if (jstr)
	{
		JNIEnv* env = getJniEnv();
		const jchar* jChars = env->GetStringChars(jstr, NULL);
		jsize jLen = env->GetStringLength(jstr);
		ret.assign(jChars, jChars + jLen);
		env->ReleaseStringChars(jstr, jChars);
	}
	return ret;
}


