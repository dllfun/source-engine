//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "dt_recv.h"
#include "mathlib/vector.h"
#include "tier1/strtools.h"
#include "dt_utlvector_common.h"
#include "dt.h"
#include "dt_recv_eng.h"
#include "dt_encode.h"
#include "dt_instrumentation.h"
#include "dt_stack.h"
#include "utllinkedlist.h"
#include "tier0/dbg.h"
#include "dt_recv_decoder.h"
#include "tier1/strtools.h"
#include "tier0/icommandline.h"
//#include "dt_common_eng.h"
#ifndef DEDICATED
#include "renamed_recvtable_compat.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#if !defined(_STATIC_LINKED) || defined(CLIENT_DLL)

const char *s_ClientElementNames[MAX_ARRAY_ELEMENTS] =
{
	"000", "001", "002", "003", "004", "005", "006", "007", "008", "009", 
	"010", "011", "012", "013", "014", "015", "016", "017", "018", "019",
	"020", "021", "022", "023", "024", "025", "026", "027", "028", "029",
	"030", "031", "032", "033", "034", "035", "036", "037", "038", "039",
	"040", "041", "042", "043", "044", "045", "046", "047", "048", "049",
	"050", "051", "052", "053", "054", "055", "056", "057", "058", "059",
	"060", "061", "062", "063", "064", "065", "066", "067", "068", "069",
	"070", "071", "072", "073", "074", "075", "076", "077", "078", "079",
	"080", "081", "082", "083", "084", "085", "086", "087", "088", "089",
	"090", "091", "092", "093", "094", "095", "096", "097", "098", "099",
	"100", "101", "102", "103", "104", "105", "106", "107", "108", "109",
	"110", "111", "112", "113", "114", "115", "116", "117", "118", "119",
	"120", "121", "122", "123", "124", "125", "126", "127", "128", "129",
	"130", "131", "132", "133", "134", "135", "136", "137", "138", "139",
	"140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
	"150", "151", "152", "153", "154", "155", "156", "157", "158", "159",
	"160", "161", "162", "163", "164", "165", "166", "167", "168", "169",
	"170", "171", "172", "173", "174", "175", "176", "177", "178", "179",
	"180", "181", "182", "183", "184", "185", "186", "187", "188", "189",
	"190", "191", "192", "193", "194", "195", "196", "197", "198", "199",
	"200", "201", "202", "203", "204", "205", "206", "207", "208", "209",
	"210", "211", "212", "213", "214", "215", "216", "217", "218", "219",
	"220", "221", "222", "223", "224", "225", "226", "227", "228", "229",
	"230", "231", "232", "233", "234", "235", "236", "237", "238", "239",
	"240", "241", "242", "243", "244", "245", "246", "247", "248", "249",
	"250", "251", "252", "253", "254", "255", "256", "257", "258", "259",
	"260", "261", "262", "263", "264", "265", "266", "267", "268", "269",
	"270", "271", "272", "273", "274", "275", "276", "277", "278", "279",
	"280", "281", "282", "283", "284", "285", "286", "287", "288", "289",
	"290", "291", "292", "293", "294", "295", "296", "297", "298", "299",
	"300", "301", "302", "303", "304", "305", "306", "307", "308", "309",
	"310", "311", "312", "313", "314", "315", "316", "317", "318", "319",
	"320", "321", "322", "323", "324", "325", "326", "327", "328", "329",
	"330", "331", "332", "333", "334", "335", "336", "337", "338", "339",
	"340", "341", "342", "343", "344", "345", "346", "347", "348", "349",
	"350", "351", "352", "353", "354", "355", "356", "357", "358", "359",
	"360", "361", "362", "363", "364", "365", "366", "367", "368", "369",
	"370", "371", "372", "373", "374", "375", "376", "377", "378", "379",
	"380", "381", "382", "383", "384", "385", "386", "387", "388", "389",
	"390", "391", "392", "393", "394", "395", "396", "397", "398", "399",
	"400", "401", "402", "403", "404", "405", "406", "407", "408", "409",
	"410", "411", "412", "413", "414", "415", "416", "417", "418", "419",
	"420", "421", "422", "423", "424", "425", "426", "427", "428", "429",
	"430", "431", "432", "433", "434", "435", "436", "437", "438", "439",
	"440", "441", "442", "443", "444", "445", "446", "447", "448", "449",
	"450", "451", "452", "453", "454", "455", "456", "457", "458", "459",
	"460", "461", "462", "463", "464", "465", "466", "467", "468", "469",
	"470", "471", "472", "473", "474", "475", "476", "477", "478", "479",
	"480", "481", "482", "483", "484", "485", "486", "487", "488", "489",
	"490", "491", "492", "493", "494", "495", "496", "497", "498", "499",
	"500", "501", "502", "503", "504", "505", "506", "507", "508", "509",
	"510", "511", "512", "513", "514", "515", "516", "517", "518", "519",
	"520", "521", "522", "523", "524", "525", "526", "527", "528", "529",
	"530", "531", "532", "533", "534", "535", "536", "537", "538", "539",
	"540", "541", "542", "543", "544", "545", "546", "547", "548", "549",
	"550", "551", "552", "553", "554", "555", "556", "557", "558", "559",
	"560", "561", "562", "563", "564", "565", "566", "567", "568", "569",
	"570", "571", "572", "573", "574", "575", "576", "577", "578", "579",
	"580", "581", "582", "583", "584", "585", "586", "587", "588", "589",
	"590", "591", "592", "593", "594", "595", "596", "597", "598", "599",
	"600", "601", "602", "603", "604", "605", "606", "607", "608", "609",
	"610", "611", "612", "613", "614", "615", "616", "617", "618", "619",
	"620", "621", "622", "623", "624", "625", "626", "627", "628", "629",
	"630", "631", "632", "633", "634", "635", "636", "637", "638", "639",
	"640", "641", "642", "643", "644", "645", "646", "647", "648", "649",
	"650", "651", "652", "653", "654", "655", "656", "657", "658", "659",
	"660", "661", "662", "663", "664", "665", "666", "667", "668", "669",
	"670", "671", "672", "673", "674", "675", "676", "677", "678", "679",
	"680", "681", "682", "683", "684", "685", "686", "687", "688", "689",
	"690", "691", "692", "693", "694", "695", "696", "697", "698", "699",
	"700", "701", "702", "703", "704", "705", "706", "707", "708", "709",
	"710", "711", "712", "713", "714", "715", "716", "717", "718", "719",
	"720", "721", "722", "723", "724", "725", "726", "727", "728", "729",
	"730", "731", "732", "733", "734", "735", "736", "737", "738", "739",
	"740", "741", "742", "743", "744", "745", "746", "747", "748", "749",
	"750", "751", "752", "753", "754", "755", "756", "757", "758", "759",
	"760", "761", "762", "763", "764", "765", "766", "767", "768", "769",
	"770", "771", "772", "773", "774", "775", "776", "777", "778", "779",
	"780", "781", "782", "783", "784", "785", "786", "787", "788", "789",
	"790", "791", "792", "793", "794", "795", "796", "797", "798", "799",
	"800", "801", "802", "803", "804", "805", "806", "807", "808", "809",
	"810", "811", "812", "813", "814", "815", "816", "817", "818", "819",
	"820", "821", "822", "823", "824", "825", "826", "827", "828", "829",
	"830", "831", "832", "833", "834", "835", "836", "837", "838", "839",
	"840", "841", "842", "843", "844", "845", "846", "847", "848", "849",
	"850", "851", "852", "853", "854", "855", "856", "857", "858", "859",
	"860", "861", "862", "863", "864", "865", "866", "867", "868", "869",
	"870", "871", "872", "873", "874", "875", "876", "877", "878", "879",
	"880", "881", "882", "883", "884", "885", "886", "887", "888", "889",
	"890", "891", "892", "893", "894", "895", "896", "897", "898", "899",
	"900", "901", "902", "903", "904", "905", "906", "907", "908", "909",
	"910", "911", "912", "913", "914", "915", "916", "917", "918", "919",
	"920", "921", "922", "923", "924", "925", "926", "927", "928", "929",
	"930", "931", "932", "933", "934", "935", "936", "937", "938", "939",
	"940", "941", "942", "943", "944", "945", "946", "947", "948", "949",
	"950", "951", "952", "953", "954", "955", "956", "957", "958", "959",
	"960", "961", "962", "963", "964", "965", "966", "967", "968", "969",
	"970", "971", "972", "973", "974", "975", "976", "977", "978", "979",
	"980", "981", "982", "983", "984", "985", "986", "987", "988", "989",
	"990", "991", "992", "993", "994", "995", "996", "997", "998", "999",
	"1000", "1001", "1002", "1003", "1004", "1005", "1006", "1007", "1008", "1009",
	"1010", "1011", "1012", "1013", "1014", "1015", "1016", "1017", "1018", "1019",
	"1020", "1021", "1022", "1023"

};

CRenamedRecvTableInfo* g_pRenamedRecvTableInfoHead = 0;

// ---------------------------------------------------------------------- //
// Prop setup functions (for building tables).
// ---------------------------------------------------------------------- //

RecvProp RecvPropFloat(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_FloatToFloat)
	{
		Assert(sizeofVar == 0 || sizeofVar == 4);
	}
#endif

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_Float;
	ret.m_Flags = flags;
	ret.SetProxyFn(varProxy);

	return ret;
}

RecvProp RecvPropVector(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}
#endif

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_Vector;
	ret.m_Flags = flags;
	ret.SetProxyFn(varProxy);

	return ret;
}

RecvProp RecvPropVectorXY(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}
#endif

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_VectorXY;
	ret.m_Flags = flags;
	ret.SetProxyFn(varProxy);

	return ret;
}

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!

RecvProp RecvPropQuaternion(
	const char* pVarName,
	int offset,
	int sizeofVar,	// Handled by RECVINFO macro, but set to SIZEOF_IGNORE if you don't want to bother.
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

#ifdef _DEBUG
	if (varProxy == RecvProxy_QuaternionToQuaternion)
	{
		Assert(sizeofVar == sizeof(Quaternion));
	}
#endif

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_Quaternion;
	ret.m_Flags = flags;
	ret.SetProxyFn(varProxy);

	return ret;
}
#endif

RecvProp RecvPropInt(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

	// If they didn't specify a proxy, then figure out what type we're writing to.
	if (varProxy == NULL)
	{
		if (sizeofVar == 1)
		{
			varProxy = RecvProxy_Int32ToInt8;
		}
		else if (sizeofVar == 2)
		{
			varProxy = RecvProxy_Int32ToInt16;
		}
		else if (sizeofVar == 4)
		{
			varProxy = RecvProxy_Int32ToInt32;
		}
#ifdef SUPPORTS_INT64		
		else if (sizeofVar == 8)
		{
			varProxy = RecvProxy_Int64ToInt64;
		}
#endif
		else
		{
			Assert(!"RecvPropInt var has invalid size");
			varProxy = RecvProxy_Int32ToInt8;	// safest one...
		}
	}

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
#ifdef SUPPORTS_INT64
	ret.m_RecvType = (sizeofVar == 8) ? DPT_Int64 : DPT_Int;
#else
	ret.m_RecvType = DPT_Int;
#endif
	ret.m_Flags = flags;
	ret.SetProxyFn(varProxy);

	return ret;
}

RecvProp RecvPropString(
	const char* pVarName,
	int offset,
	int bufferSize,
	int flags,
	RecvVarProxyFn varProxy
)
{
	RecvProp ret;

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_String;
	ret.m_Flags = flags;
	ret.m_StringBufferSize = bufferSize;
	ret.SetProxyFn(varProxy);

	return ret;
}

RecvProp RecvPropDataTable(
	const char* pVarName,
	int offset,
	int flags,
	const char* pTableName,
	DataTableRecvVarProxyFn varProxy
)
{
	RecvProp ret;

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_DataTable;
	ret.m_Flags = flags;
	ret.SetDataTableProxyFn(varProxy);
	ret.SetDataTable(NULL);
	if (pTableName) {
		ret.SetDataTableName(COM_StringCopy(pTableName));
	}

	return ret;
}

RecvProp RecvPropArray3(
	const char* pVarName,
	int offset,
	int sizeofVar,
	int elements,
	RecvProp pArrayProp,
	DataTableRecvVarProxyFn varProxy
)
{
	RecvProp ret;

	Assert(elements <= MAX_ARRAY_ELEMENTS);

	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset(offset);
	ret.m_RecvType = DPT_DataTable;
	ret.SetDataTableProxyFn(varProxy);

	RecvProp* pProps = new RecvProp[elements]; // TODO free that again

	const char* pParentArrayPropName = ("%s", pVarName);//AllocateStringHelper()

	for (int i = 0; i < elements; i++)
	{
		pProps[i] = pArrayProp; // copy basic property settings 
		pProps[i].SetOffset(i * sizeofVar); // adjust offset
		pProps[i].m_pVarName = COM_StringCopy(s_ClientElementNames[i]); // give unique name
		pProps[i].SetParentArrayPropName(COM_StringCopy(pParentArrayPropName)); // For debugging...
	}

	RecvTable* pTable = new RecvTable(pProps, elements, pVarName); // TODO free that again
	GetRecvTableManager()->RegisteRecvTable(pTable);
	if (pVarName) {
		ret.SetDataTableName(COM_StringCopy(pVarName));
	}
	ret.SetDataTable(pTable);

	return ret;
}

RecvProp InternalRecvPropArray(
	const int elementCount,
	const int elementStride,
	const char* pName,
	ArrayLengthRecvProxyFn proxy
)
{
	RecvProp ret;

	ret.InitArray(elementCount, elementStride);
	if (pName) {
		ret.m_pVarName = COM_StringCopy(pName);
	}
	ret.SetArrayLengthProxy(proxy);

	return ret;
}


// ---------------------------------------------------------------------- //
// Proxies.
// ---------------------------------------------------------------------- //

void RecvProxy_FloatToFloat(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	Assert(IsFinite(pData->m_Value.m_Float));
	*((float*)pOut) = pData->m_Value.m_Float;
}

void RecvProxy_VectorToVector(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const float* v = pData->m_Value.m_Vector;

	Assert(IsFinite(v[0]) && IsFinite(v[1]) && IsFinite(v[2]));
	((float*)pOut)[0] = v[0];
	((float*)pOut)[1] = v[1];
	((float*)pOut)[2] = v[2];
}

void RecvProxy_VectorXYToVectorXY(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const float* v = pData->m_Value.m_Vector;

	Assert(IsFinite(v[0]) && IsFinite(v[1]));
	((float*)pOut)[0] = v[0];
	((float*)pOut)[1] = v[1];
}

void RecvProxy_QuaternionToQuaternion(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	const float* v = pData->m_Value.m_Vector;

	Assert(IsFinite(v[0]) && IsFinite(v[1]) && IsFinite(v[2]) && IsFinite(v[3]));
	((float*)pOut)[0] = v[0];
	((float*)pOut)[1] = v[1];
	((float*)pOut)[2] = v[2];
	((float*)pOut)[3] = v[3];
}

void RecvProxy_Int32ToInt8(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((unsigned char*)pOut) = (unsigned char)pData->m_Value.m_Int;
}

void RecvProxy_Int32ToInt16(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((unsigned short*)pOut) = (unsigned short)pData->m_Value.m_Int;
}

void RecvProxy_Int32ToInt32(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((uint32*)pOut) = (uint32)pData->m_Value.m_Int;
}

#ifdef SUPPORTS_INT64
void RecvProxy_Int64ToInt64(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	*((int64*)pOut) = (int64)pData->m_Value.m_Int64;
}
#endif

void RecvProxy_StringToString(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	char* pStrOut = (char*)pOut;
	if (pData->m_pRecvProp->m_StringBufferSize <= 0)
	{
		return;
	}

	for (int i = 0; i < pData->m_pRecvProp->m_StringBufferSize; i++)
	{
		pStrOut[i] = pData->m_Value.m_pString[i];
		if (pStrOut[i] == 0)
			break;
	}

	pStrOut[pData->m_pRecvProp->m_StringBufferSize - 1] = 0;
}

void DataTableRecvProxy_StaticDataTable(const RecvProp* pProp, void** pOut, void* pData, int objectID)
{
	*pOut = pData;
}

void DataTableRecvProxy_PointerDataTable(const RecvProp* pProp, void** pOut, void* pData, int objectID)
{
	*pOut = *((void**)pData);
}

CStandardRecvProxies::CStandardRecvProxies()
{
	m_Int32ToInt8 = RecvProxy_Int32ToInt8;
	m_Int32ToInt16 = RecvProxy_Int32ToInt16;
	m_Int32ToInt32 = RecvProxy_Int32ToInt32;
#ifdef SUPPORTS_INT64
	m_Int64ToInt64 = RecvProxy_Int64ToInt64;
#endif
	m_FloatToFloat = RecvProxy_FloatToFloat;
	m_VectorToVector = RecvProxy_VectorToVector;
}

CStandardRecvProxies g_StandardRecvProxies;
	
// ---------------------------------------------------------------------- //
// RecvProp.
// ---------------------------------------------------------------------- //
RecvProp::RecvProp()
{
	m_pExtraData = NULL;
	m_pVarName = NULL;
	m_Offset = 0;
	m_RecvType = DPT_Int;
	m_Flags = 0;
	m_ProxyFn = NULL;
	m_DataTableProxyFn = NULL;
	m_pDataTable = NULL;
	m_nElements = 1;
	m_ElementStride = -1;
	m_pArrayProp = NULL;
	m_ArrayLengthProxy = NULL;
	m_bInsideArray = false;
}

RecvProp::~RecvProp() 
{
	if (this->m_pVarName) {
		delete this->m_pVarName;
		this->m_pVarName = NULL;
	}
	if (this->m_pArrayProp) {
		delete this->m_pArrayProp;
		this->m_pArrayProp = NULL;
	}
	if (this->m_pDataTableName) {
		delete this->m_pDataTableName;
		this->m_pDataTableName = NULL;
	}
	if (this->m_pDataTable) {
		delete this->m_pDataTable;
		this->m_pDataTable = NULL;
	}
	if (this->m_pParentArrayPropName) {
		delete this->m_pParentArrayPropName;
		this->m_pParentArrayPropName = NULL;
	}
}

RecvProp& RecvProp::operator=(const RecvProp& srcRecvProp) {
	if (this != &srcRecvProp) {
		if (this->m_pVarName) {
			delete this->m_pVarName;
			this->m_pVarName = NULL;
		}
		if (srcRecvProp.m_pVarName) {
			this->m_pVarName = COM_StringCopy(srcRecvProp.m_pVarName);
		}
		this->m_RecvType = (SendPropType)srcRecvProp.m_RecvType;
		this->m_Flags = srcRecvProp.m_Flags;
		this->m_StringBufferSize = srcRecvProp.m_StringBufferSize;
		this->m_bInsideArray = srcRecvProp.m_bInsideArray;
		this->m_pExtraData = NULL;
		if (this->m_pArrayProp) {
			delete this->m_pArrayProp;
			this->m_pArrayProp = NULL;
		}
		if (srcRecvProp.m_pArrayProp) {
			this->m_pArrayProp = new RecvProp;
			*this->m_pArrayProp = *srcRecvProp.m_pArrayProp;
		}
		this->m_ArrayLengthProxy = srcRecvProp.m_ArrayLengthProxy;
		this->m_ProxyFn = srcRecvProp.m_ProxyFn;
		this->m_DataTableProxyFn = srcRecvProp.m_DataTableProxyFn;
		if (this->m_pDataTableName) {
			delete this->m_pDataTableName;
			this->m_pDataTableName = NULL;
		}
		if (srcRecvProp.m_pDataTableName) {
			this->m_pDataTableName = COM_StringCopy(srcRecvProp.m_pDataTableName);
		}
		if (this->m_pDataTable) {
			delete this->m_pDataTable;
			this->m_pDataTable = NULL;
		}
		if (srcRecvProp.m_pDataTable) {
			this->m_pDataTable = new RecvTable();
			*this->m_pDataTable = *srcRecvProp.m_pDataTable;
		}
		this->m_Offset = srcRecvProp.m_Offset;
		this->m_ElementStride = srcRecvProp.m_ElementStride;
		this->m_nElements = srcRecvProp.m_nElements;
		if (this->m_pParentArrayPropName) {
			delete this->m_pParentArrayPropName;
			this->m_pParentArrayPropName = NULL;
		}
		if (srcRecvProp.m_pParentArrayPropName) {
			this->m_pParentArrayPropName = COM_StringCopy(srcRecvProp.m_pParentArrayPropName);
		}
	}
	return *this;
}

void RecvProp::SetDataTable(RecvTable* pTable)
{
	if (this->m_pDataTable) {
		delete this->m_pDataTable;
		this->m_pDataTable = NULL;
	}
	m_pDataTable = pTable;
}

// ---------------------------------------------------------------------- //
// RecvTable.
// ---------------------------------------------------------------------- //
RecvTable::RecvTable()
{
	Construct( NULL, 0, NULL );
}

RecvTable::RecvTable(RecvProp *pProps, int nProps, const char *pNetTableName)
{
	Construct( pProps, nProps, pNetTableName );
}

RecvTable::~RecvTable()
{
	if (this->m_pProps) {
		delete[] this->m_pProps;
		this->m_pProps = NULL;
	}
	if (this->m_pNetTableName) {
		delete this->m_pNetTableName;
		this->m_pNetTableName = NULL;
	}
}

void RecvTable::Construct( RecvProp *pProps, int nProps, const char *pNetTableName )
{
	//m_pProps = pProps;
	//m_nProps = nProps;
	//m_pNetTableName = pNetTableName;
	this->m_nProps = nProps;
	if (this->m_pProps) {
		delete[] this->m_pProps;
		this->m_pProps = NULL;
	}
	this->m_pProps = this->m_nProps ? new RecvProp[this->m_nProps] : NULL;
	for (int iProp = 0; iProp < this->m_nProps; iProp++)
	{
		RecvProp* pProp = &this->m_pProps[iProp];
		const RecvProp* pRecvTableProp = &pProps[iProp];
		*pProp = *pRecvTableProp;
	}
	if (this->m_pNetTableName) {
		delete this->m_pNetTableName;
		this->m_pNetTableName = NULL;
	}
	if (pNetTableName) {
		this->m_pNetTableName = COM_StringCopy(pNetTableName);
	}
	m_pDecoder = NULL;
	m_bInitialized = false;
	m_bInMainList = false;
}

RecvTable& RecvTable::operator=(const RecvTable& srcRecvTable) {
	if (this != &srcRecvTable) {
		this->m_nProps = srcRecvTable.m_nProps;
		if (this->m_pProps) {
			delete[] this->m_pProps;
			this->m_pProps = NULL;
		}
		this->m_pProps = this->m_nProps ? new RecvProp[this->m_nProps] : NULL;
		for (int iProp = 0; iProp < this->m_nProps; iProp++)
		{
			RecvProp* pProp = &this->m_pProps[iProp];
			const RecvProp* pRecvTableProp = &srcRecvTable.m_pProps[iProp];
			*pProp = *pRecvTableProp;
		}
		if (this->m_pNetTableName) {
			delete this->m_pNetTableName;
			this->m_pNetTableName = NULL;
		}
		if (srcRecvTable.m_pNetTableName) {
			this->m_pNetTableName = COM_StringCopy(srcRecvTable.m_pNetTableName);
		}
	}
	return *this;
}

void RecvTable::InitRefRecvTable(RecvTableManager* pRecvTableNanager) {
	if (m_RefTableInited) {
		return;
	}
	m_RefTableInited = true;
	for (int i = 0; i < m_nProps; i++)
	{
		RecvProp* pProp = &m_pProps[i];
		if (pProp->GetType() == DPT_DataTable && pProp->GetDataTableName() && pProp->GetDataTableName()[0])
		{
			RecvTable* pSrcRecvTable = pRecvTableNanager->FindRecvTable(pProp->GetDataTableName());
			if (!pSrcRecvTable) {
				Error("not found RecvTable: %s\n", pProp->GetDataTableName());	// dedicated servers exit
			}
			pSrcRecvTable->m_bIsLeaf = false;
			pSrcRecvTable->InitRefRecvTable(pRecvTableNanager);
			RecvTable* pRecvTable = new RecvTable();
			*pRecvTable = *pSrcRecvTable;
			pProp->SetDataTable(pRecvTable);
		}
	}
}

void RecvTable::RecvTable_InitTable(RecvTableManager* pRecvTableNanager) {
	if (this->IsInMainList())
		return;

	// Shouldn't have a decoder yet.
	ErrorIfNot(!this->m_pDecoder,
		("RecvTable_Init: table '%s' has a decoder already.", this->GetName()));

	this->SetInMainList(true);

	//pRecvTableNanager->GetRecvTables().AddToTail(this);
	for (int i = 0; i < this->GetNumProps(); i++)
	{
		RecvProp* pProp = this->GetProp(i);

		if (pProp->GetType() == DPT_DataTable)
			pProp->GetDataTable()->RecvTable_InitTable(pRecvTableNanager);
	}
}

void RecvTable::RecvTable_TermTable(RecvTableManager* pRecvTableNanager) {
	if (!this->IsInMainList())
		return;

	this->SetInMainList(false);
	this->m_pDecoder = 0;

	for (int i = 0; i < this->GetNumProps(); i++)
	{
		RecvProp* pProp = this->GetProp(i);

		if (pProp->GetType() == DPT_DataTable)
			pProp->GetDataTable()->RecvTable_TermTable(pRecvTableNanager);
	}
}

bool RecvTable::RecvTable_Decode(
	void* pStruct,
	bf_read* pIn,
	int objectID,
	bool updateDTI
)
{
	CRecvDecoder* pDecoder = this->m_pDecoder;
	ErrorIfNot(pDecoder,
		("RecvTable_Decode: table '%s' missing a decoder.", this->GetName())
	);

	// While there are properties, decode them.. walk the stack as you go.
	CClientDatatableStack theStack(pDecoder, (unsigned char*)pStruct, objectID);

	theStack.Init();
	int iStartBit = 0, nIndexBits = 0, iLastBit = pIn->GetNumBitsRead();
	unsigned int iProp;
	CDeltaBitsReader deltaBitsReader(pIn);
	while ((iProp = deltaBitsReader.ReadNextPropIndex()) < MAX_DATATABLE_PROPS)
	{
		theStack.SeekToProp(iProp);

		const RecvProp* pProp = pDecoder->GetProp(iProp);

		// Instrumentation (store the # bits for the prop index).
		if (g_bDTIEnabled)
		{
			iStartBit = pIn->GetNumBitsRead();
			nIndexBits = iStartBit - iLastBit;
		}

		DecodeInfo decodeInfo;
		decodeInfo.m_pStruct = theStack.GetCurStructBase();

		if (pProp)
		{
			decodeInfo.m_pData = theStack.GetCurStructBase() + pProp->GetOffset();
		}
		else
		{
			// They're allowed to be missing props here if they're playing back a demo.
			// This allows us to change the datatables and still preserve old demos.
			decodeInfo.m_pData = NULL;
		}

		decodeInfo.m_pRecvProp = theStack.IsCurProxyValid() ? pProp : NULL; // Just skip the data if the proxies are screwed.
		decodeInfo.m_pProp = pDecoder->GetSendProp(iProp);
		decodeInfo.m_pIn = pIn;
		decodeInfo.m_ObjectID = objectID;

		g_PropTypeFns[decodeInfo.m_pProp->GetType()].Decode(&decodeInfo);
		++g_nPropsDecoded;

		// Instrumentation (store # bits for the encoded property).
		if (updateDTI && g_bDTIEnabled)
		{
			iLastBit = pIn->GetNumBitsRead();
			DTI_HookDeltaBits(pDecoder, iProp, iLastBit - iStartBit, nIndexBits);
		}
	}

	return !pIn->IsOverflowed();
}


void RecvTable::RecvTable_DecodeZeros(void* pStruct, int objectID)
{
	CRecvDecoder* pDecoder = this->m_pDecoder;
	ErrorIfNot(pDecoder,
		("RecvTable_DecodeZeros: table '%s' missing a decoder.", this->GetName())
	);

	// While there are properties, decode them.. walk the stack as you go.
	CClientDatatableStack theStack(pDecoder, (unsigned char*)pStruct, objectID);

	theStack.Init();

	for (int iProp = 0; iProp < pDecoder->GetNumProps(); iProp++)
	{
		theStack.SeekToProp(iProp);

		// They're allowed to be missing props here if they're playing back a demo.
		// This allows us to change the datatables and still preserve old demos.
		const RecvProp* pProp = pDecoder->GetProp(iProp);
		if (!pProp)
			continue;

		DecodeInfo decodeInfo;
		decodeInfo.m_pStruct = theStack.GetCurStructBase();
		decodeInfo.m_pData = theStack.GetCurStructBase() + pProp->GetOffset();
		decodeInfo.m_pRecvProp = theStack.IsCurProxyValid() ? pProp : NULL; // Just skip the data if the proxies are screwed.
		decodeInfo.m_pProp = pDecoder->GetSendProp(iProp);
		decodeInfo.m_pIn = NULL;
		decodeInfo.m_ObjectID = objectID;

		g_PropTypeFns[pProp->GetType()].DecodeZero(&decodeInfo);
	}
}



int RecvTable::RecvTable_MergeDeltas(
	bf_read* pOldState,		// this can be null
	bf_read* pNewState,

	bf_write* pOut,

	int objectID,
	int* pChangedProps,
	bool updateDTI
)
{
	ErrorIfNot(this && pNewState && pOut,
		("RecvTable_MergeDeltas: invalid parameters passed.")
	);

	CRecvDecoder* pDecoder = this->m_pDecoder;
	ErrorIfNot(pDecoder, ("RecvTable_MergeDeltas: table '%s' is missing its decoder.", this->GetName()));

	int nChanged = 0;

	// Setup to read the delta bits from each buffer.
	CDeltaBitsReader oldStateReader(pOldState);
	CDeltaBitsReader newStateReader(pNewState);

	// Setup to write delta bits into the output.
	CDeltaBitsWriter deltaBitsWriter(pOut);

	unsigned int iOldProp = ~0u;
	if (pOldState)
		iOldProp = oldStateReader.ReadNextPropIndex();

	int iStartBit = 0, nIndexBits = 0, iLastBit = pNewState->GetNumBitsRead();

	unsigned int iNewProp = newStateReader.ReadNextPropIndex();

	while (1)
	{
		// Write any properties in the previous state that aren't in the new state.
		while (iOldProp < iNewProp)
		{
			deltaBitsWriter.WritePropIndex(iOldProp);
			oldStateReader.CopyPropData(deltaBitsWriter.GetBitBuf(), pDecoder->GetSendProp(iOldProp));
			iOldProp = oldStateReader.ReadNextPropIndex();
		}

		// Check if we're at the end here so the while() statement above can seek the old buffer
		// to its end too.
		if (iNewProp >= MAX_DATATABLE_PROPS)
			break;

		// If the old state has this property too, then just skip over its data.
		if (iOldProp == iNewProp)
		{
			oldStateReader.SkipPropData(pDecoder->GetSendProp(iOldProp));
			iOldProp = oldStateReader.ReadNextPropIndex();
		}

		// Instrumentation (store the # bits for the prop index).
		if (updateDTI && g_bDTIEnabled)
		{
			iStartBit = pNewState->GetNumBitsRead();
			nIndexBits = iStartBit - iLastBit;
		}

		// Now write the new state's value.
		deltaBitsWriter.WritePropIndex(iNewProp);
		newStateReader.CopyPropData(deltaBitsWriter.GetBitBuf(), pDecoder->GetSendProp(iNewProp));

		if (pChangedProps)
		{
			pChangedProps[nChanged] = iNewProp;
		}

		nChanged++;

		// Instrumentation (store # bits for the encoded property).
		if (updateDTI && g_bDTIEnabled)
		{
			iLastBit = pNewState->GetNumBitsRead();
			DTI_HookDeltaBits(pDecoder, iNewProp, iLastBit - iStartBit, nIndexBits);
		}

		iNewProp = newStateReader.ReadNextPropIndex();
	}

	Assert(nChanged <= MAX_DATATABLE_PROPS);

	ErrorIfNot(
		!(pOldState && pOldState->IsOverflowed()) && !pNewState->IsOverflowed() && !pOut->IsOverflowed(),
		("RecvTable_MergeDeltas: overflowed in RecvTable '%s'.", this->GetName())
	);

	return nChanged;
}


void RecvTable::RecvTable_CopyEncoding(bf_read* pIn, bf_write* pOut, int objectID)
{
	this->RecvTable_MergeDeltas(NULL, pIn, pOut, objectID);
}

class CClientSendTable;


// Testing out this pattern.. you can write simple code blocks inside of
// codeToRun. The thing that sucks is that you can't access your function's
// local variables inside of codeToRun.
//
// If it used an iterator class, it could access local function variables,
// but the iterator class might be more trouble than it's worth.
#define FOR_EACH_PROP_R( TableType, pTablePointer, tableCode, propCode )	\
	class CPropVisitor 										\
	{ 														\
		public:												\
		static void Visit_R( TableType *pTable )					\
		{													\
			tableCode;										\
															\
			for ( int i=0; i < pTable->GetNumProps(); i++ )	\
			{												\
				TableType::PropType *pProp = pTable->GetProp( i );	\
															\
				propCode;									\
															\
				if ( pProp->GetType() == DPT_DataTable )	\
					Visit_R( pProp->GetDataTable() );		\
			}												\
		}													\
	};														\
	CPropVisitor::Visit_R( pTablePointer );				

#define SENDPROP_VISIT( pTablePointer, tableCode, propCode )  FOR_EACH_PROP_R( SendTable, pTablePointer, tableCode, propCode )
#define RECVPROP_VISIT( pTablePointer, tableCode, propCode )  FOR_EACH_PROP_R( RecvTable, pTablePointer, tableCode, propCode )
#define SETUP_VISIT() class CDummyClass {} // Workaround for parser bug in VC7.1

// ------------------------------------------------------------------------------------ //
// Globals.
// ------------------------------------------------------------------------------------ //



int g_nPropsDecoded = 0;

void RecvTableManager::RegisteRecvTable(RecvTable* pSrcRecvTable) {

	RecvTable* pRecvTable = new RecvTable();
	*pRecvTable = *pSrcRecvTable;

	if (m_RecvTableMap.Defined(pRecvTable->GetName())) {
		Error("duplicate RecvTable: %s\n", pRecvTable->GetName());	// dedicated servers exit
	}
	else {
		m_RecvTableMap[pRecvTable->GetName()] = pRecvTable;
	}
	if (!m_pRecvTableHead)
	{
		m_pRecvTableHead = pRecvTable;
		pRecvTable->m_pNext = NULL;
	}
	else
	{
		RecvTable* p1 = m_pRecvTableHead;
		RecvTable* p2 = p1->m_pNext;

		// use _stricmp because Q_stricmp isn't hooked up properly yet
		if (_stricmp(p1->GetName(), pRecvTable->GetName()) > 0)
		{
			pRecvTable->m_pNext = m_pRecvTableHead;
			m_pRecvTableHead = pRecvTable;
			p1 = NULL;
		}

		while (p1)
		{
			if (p2 == NULL || _stricmp(p2->GetName(), pRecvTable->GetName()) > 0)
			{
				pRecvTable->m_pNext = p2;
				p1->m_pNext = pRecvTable;
				break;
			}
			p1 = p2;
			p2 = p2->m_pNext;
		}
	}
}
// ------------------------------------------------------------------------------------ //
// Static helper functions.
// ------------------------------------------------------------------------------------ //

//RecvTable* RecvTableManager::FindRecvTable(const char* pName)
//{
//	//FOR_EACH_LL(g_RecvTables, i)
//	//{
//	//	if (stricmp(g_RecvTables[i]->GetName(), pName) == 0)
//	//		return g_RecvTables[i];
//	//}
//	//return 0;
//	if (m_RecvTableMap.Defined(pName)) {
//		return m_RecvTableMap[pName];
//	}
//	return NULL;
//}


//CClientSendTable* RecvTableManager::FindClientSendTable(const char* pName)
//{
//	FOR_EACH_LL(g_ClientSendTables, i)
//	{
//		CClientSendTable* pTable = g_ClientSendTables[i];
//
//		if (stricmp(pTable->GetName(), pName) == 0)
//			return pTable;
//	}
//
//	return NULL;
//}


// Find all child datatable properties for the send tables.
bool RecvTableManager::SetupClientSendTableHierarchy()
{
	for (SendTable* pCur = m_SendTableManager.GetSendTableHead(); pCur; pCur = pCur->m_pNext)
	{
		pCur->InitRefSendTable(&m_SendTableManager);
	}
	//FOR_EACH_LL(g_ClientSendTables, iClientTable)
	//{
	//	CClientSendTable* pTable = g_ClientSendTables[iClientTable];

	//	// For each datatable property, find the table it references.
	//	for (int iProp = 0; iProp < pTable->GetNumProps(); iProp++)
	//	{
	//		CClientSendProp* pClientProp = pTable->GetClientProp(iProp);
	//		SendProp* pProp = &pTable->m_SendTable.m_pProps[iProp];

	//		if (pProp->m_Type == DPT_DataTable)
	//		{
	//			const char* pTableName = pClientProp->GetTableName();
	//			ErrorIfNot(pTableName,
	//				("SetupClientSendTableHierarchy: missing table name for prop '%s'.", pProp->GetName())
	//			);

	//			CClientSendTable* pChild = FindClientSendTable(pTableName);
	//			if (!pChild)
	//			{
	//				DataTable_Warning("SetupClientSendTableHierarchy: missing SendTable '%s' (referenced by '%s').\n", pTableName, pTable->GetName());
	//				return false;
	//			}

	//			pProp->SetDataTable(&pChild->m_SendTable);
	//		}
	//	}
	//}

	return true;
}


static RecvProp* FindRecvProp(RecvTable* pTable, const char* pName)
{
	for (int i = 0; i < pTable->GetNumProps(); i++)
	{
		RecvProp* pProp = pTable->GetProp(i);

		if (stricmp(pProp->GetName(), pName) == 0)
			return pProp;
	}

	return NULL;
}


// See if the RecvProp is fit to receive the SendProp's data.
bool CompareRecvPropToSendProp(const RecvProp* pRecvProp, const SendProp* pSendProp)
{	
	while (1)
	{
		if (!pRecvProp || !pSendProp) {
			int aaa = 0;
		}
		ErrorIfNot(pRecvProp && pSendProp,
			("CompareRecvPropToSendProp: missing a property.")
		);

		if (pRecvProp->GetType() != pSendProp->GetType() || pRecvProp->IsInsideArray() != pSendProp->IsInsideArray())
		{
			return false;
		}

		if (pRecvProp->GetType() == DPT_Array)
		{
			if (pRecvProp->GetNumElements() != pSendProp->GetNumElements())
				return false;

			pRecvProp = pRecvProp->GetArrayProp();
			pSendProp = pSendProp->GetArrayProp();
		}
		else
		{
			return true;
		}
	}
}



bool RecvTableManager::MatchRecvPropsToSendProps_R(CUtlRBTree< MatchingProp_t, unsigned short >& lookup, char const* sendTableName, SendTable* pSendTable, RecvTable* pRecvTable, bool bAllowMismatches, bool* pAnyMismatches)
{
	for (int i = 0; i < pSendTable->m_nProps; i++)
	{
		SendProp* pSendProp = &pSendTable->m_pProps[i];

		if (pSendProp->IsExcludeProp() || pSendProp->IsInsideArray())
			continue;

		// Find a RecvProp by the same name and type.
		RecvProp* pRecvProp = 0;
		if (pRecvTable)
			pRecvProp = FindRecvProp(pRecvTable, pSendProp->GetName());

		if (pRecvProp)
		{
			if (!CompareRecvPropToSendProp(pRecvProp, pSendProp))
			{
				Warning("RecvProp type doesn't match server type for %s/%s\n", pSendTable->GetName(), pSendProp->GetName());
				return false;
			}

			MatchingProp_t info;
			info.m_pProp = pSendProp;
			info.m_pMatchingRecvProp = pRecvProp;

			lookup.Insert(info);
		}
		else
		{
			if (pAnyMismatches)
			{
				*pAnyMismatches = true;
			}

			Warning("Missing RecvProp for %s - %s/%s\n", sendTableName, pSendTable->GetName(), pSendProp->GetName());
			if (!bAllowMismatches)
			{
				return false;
			}
		}

		// Recurse.
		if (pSendProp->GetType() == DPT_DataTable)
		{
			if (!MatchRecvPropsToSendProps_R(lookup, sendTableName, pSendProp->GetDataTable(), FindRecvTable(pSendProp->GetDataTable()->m_pNetTableName), bAllowMismatches, pAnyMismatches))
				return false;
		}
	}

	return true;
}


// ------------------------------------------------------------------------------------ //
// Interface functions.
// ------------------------------------------------------------------------------------ //
bool RecvTableManager::RecvTable_Init()
{
	int nTables = m_RecvTableMap.GetNumStrings();
	for (int i = 0; i < nTables; i++)
	{
		RecvTable* pTable = m_RecvTableMap[i];

		pTable->RecvTable_InitTable(this);
	}

	return true;
}


void RecvTableManager::RecvTable_Term(bool clearall /*= true*/)
{
	DTI_Term();

	int nTables = m_RecvTableMap.GetNumStrings();
	for (int i = 0; i < nTables; i++)
	{
		RecvTable* pTable = m_RecvTableMap[i];

		pTable->RecvTable_TermTable(this);
	}

	if (clearall)
	{
		//g_RecvTables.Purge();
	}
	g_RecvDecoders.PurgeAndDeleteElements();
	//m_SendTableManager.SendTable_Term();
	m_SendTableManager.ClearAllSendTables();
}

RecvTable* RecvTableManager::DataTable_FindRenamedTable(const char* pOldTableName)
{
#ifdef DEDICATED
	return NULL;
#else
	//extern IBaseClientDLL* g_ClientDLL;
	//if (!g_ClientDLL)
	//	return NULL;

	// Get the renamed receive table list from the client DLL and see if we can find
	// a new name (assuming it was renamed at all).
	const CRenamedRecvTableInfo* pCur = g_pRenamedRecvTableInfoHead;// g_ClientDLL->GetRenamedRecvTableInfos();

	// This should be a very short list, so we'll do string compares until 2020 when
	// someone finds this code and the list has grown to 10,000.
	while (pCur && pCur->m_pOldName && pCur->m_pNewName)
	{
		if (!V_stricmp(pCur->m_pOldName, pOldTableName))
		{
			return FindRecvTable(pCur->m_pNewName);
		}

		pCur = pCur->m_pNext;
	}

	return NULL;
#endif
}

bool RecvTableManager::DataTable_SetupReceiveTableFromSendTable(SendTable* sendTable, bool bNeedsDecoder)
{
	SendTable* pSendTable = new SendTable();
	*pSendTable = *sendTable;
	m_SendTableManager.RegisteSendTable(pSendTable);

	//CClientSendTable* pClientSendTable = new CClientSendTable;
	//SendTable* pTable = &pClientSendTable->m_SendTable;
	//g_ClientSendTables.AddToTail(pClientSendTable);

	// Read the name.
	//pTable->m_pNetTableName = COM_StringCopy(sendTable->m_pNetTableName);

	// Create a decoder for it if necessary.
	if (bNeedsDecoder)
	{
		// Make a decoder for it.
		CRecvDecoder* pDecoder = new CRecvDecoder;
		g_RecvDecoders.AddToTail(pDecoder);

		RecvTable* pRecvTable = FindRecvTable(pSendTable->m_pNetTableName);
		if (!pRecvTable)
		{
			// Attempt to find a renamed version of the table.
			pRecvTable = DataTable_FindRenamedTable(pSendTable->m_pNetTableName);
			if (!pRecvTable)
			{
				DataTable_Warning("No matching RecvTable for SendTable '%s'.\n", pSendTable->m_pNetTableName);
				return false;
			}
		}

		pRecvTable->m_pDecoder = pDecoder;
		pDecoder->m_pTable = pRecvTable;

		pDecoder->m_pSendTable = pSendTable;
		pDecoder->m_Precalc.m_pSendTable = pSendTable;
		pSendTable->m_pPrecalc = &pDecoder->m_Precalc;

		// Initialize array properties.
		SetupArrayProps_R<RecvTable, RecvTable::PropType>(pRecvTable);
	}

	// Read the property list.
	//pTable->m_nProps = sendTable->m_nProps;
	//pTable->m_pProps = pTable->m_nProps ? new SendProp[pTable->m_nProps] : 0;
	//pClientSendTable->m_Props.SetSize(pTable->m_nProps);

	//for (int iProp = 0; iProp < pTable->m_nProps; iProp++)
	//{
	//	CClientSendProp* pClientProp = &pClientSendTable->m_Props[iProp];
	//	SendProp* pProp = &pTable->m_pProps[iProp];
	//	const SendProp* pSendTableProp = &sendTable->m_pProps[iProp];

	//	pProp->m_Type = (SendPropType)pSendTableProp->m_Type;
	//	pProp->m_pVarName = COM_StringCopy(pSendTableProp->GetName());
	//	pProp->SetFlags(pSendTableProp->GetFlags());

	//	if (CommandLine()->FindParm("-dti") && pSendTableProp->GetParentArrayPropName())
	//	{
	//		pProp->m_pParentArrayPropName = COM_StringCopy(pSendTableProp->GetParentArrayPropName());
	//	}

	//	if (pProp->m_Type == DPT_DataTable)
	//	{
	//		const char* pDTName = pSendTableProp->m_pExcludeDTName; // HACK

	//		if (pSendTableProp->GetDataTable())
	//			pDTName = pSendTableProp->GetDataTable()->m_pNetTableName;

	//		Assert(pDTName && Q_strlen(pDTName) > 0);

	//		pClientProp->SetTableName(COM_StringCopy(pDTName));

	//		// Normally we wouldn't care about this but we need to compare it against 
	//		// proxies in the server DLL in SendTable_BuildHierarchy.
	//		pProp->SetDataTableProxyFn(pSendTableProp->GetDataTableProxyFn());
	//		pProp->SetOffset(pSendTableProp->GetOffset());
	//	}
	//	else
	//	{
	//		if (pProp->IsExcludeProp())
	//		{
	//			pProp->m_pExcludeDTName = COM_StringCopy(pSendTableProp->GetExcludeDTName());
	//		}
	//		else if (pProp->GetType() == DPT_Array)
	//		{
	//			pProp->SetNumElements(pSendTableProp->GetNumElements());
	//		}
	//		else
	//		{
	//			pProp->m_fLowValue = pSendTableProp->m_fLowValue;
	//			pProp->m_fHighValue = pSendTableProp->m_fHighValue;
	//			pProp->m_nBits = pSendTableProp->m_nBits;
	//		}
	//	}
	//}

	return true;
}

static void CopySendPropsToRecvProps(
	CUtlRBTree< MatchingProp_t, unsigned short >& lookup,
	const CUtlVector<const SendProp*>& sendProps,
	CUtlVector<const RecvProp*>& recvProps
)
{
	recvProps.SetSize(sendProps.Count());
	for (int iSendProp = 0; iSendProp < sendProps.Count(); iSendProp++)
	{
		const SendProp* pSendProp = sendProps[iSendProp];
		MatchingProp_t search;
		search.m_pProp = (SendProp*)pSendProp;
		int idx = lookup.Find(search);
		if (idx == lookup.InvalidIndex())
		{
			recvProps[iSendProp] = 0;
		}
		else
		{
			recvProps[iSendProp] = lookup[idx].m_pMatchingRecvProp;
		}
	}
}

bool RecvTableManager::RecvTable_CreateDecoders(const CStandardSendProxies* pSendProxies, bool bAllowMismatches, bool* pAnyMismatches)
{
	DTI_Init();

	SETUP_VISIT();

	if (pAnyMismatches)
	{
		*pAnyMismatches = false;
	}

	// First, now that we've supposedly received all the SendTables that we need,
	// set their datatable child pointers.
	if (!SetupClientSendTableHierarchy())
		return false;

	FOR_EACH_LL(g_RecvDecoders, i)
	{
		CRecvDecoder* pDecoder = g_RecvDecoders[i];


		// It should already have been linked to its ClientSendTable.
		Assert(pDecoder->m_pSendTable);
		if (!pDecoder->m_pSendTable)
			return false;


		// For each decoder, precalculate the SendTable's flat property list.
		if (!pDecoder->m_Precalc.SetupFlatPropertyArray())
			return false;

		CUtlRBTree< MatchingProp_t, unsigned short >	PropLookup(0, 0, MatchingProp_t::LessFunc);

		// Now match RecvProp with SendProps.
		if (!MatchRecvPropsToSendProps_R(PropLookup, pDecoder->GetSendTable()->m_pNetTableName, pDecoder->GetSendTable(), FindRecvTable(pDecoder->GetSendTable()->m_pNetTableName), bAllowMismatches, pAnyMismatches))
			return false;

		// Now fill out the matching RecvProp array.
		CSendTablePrecalc* pPrecalc = &pDecoder->m_Precalc;
		CopySendPropsToRecvProps(PropLookup, pPrecalc->m_Props, pDecoder->m_Props);
		CopySendPropsToRecvProps(PropLookup, pPrecalc->m_DatatableProps, pDecoder->m_DatatableProps);

		DTI_HookRecvDecoder(pDecoder);
	}

	return true;
}

RecvTableManager* GetRecvTableManager() {
	static RecvTableManager s_RecvTableManager;
	return &s_RecvTableManager;
}


#endif
