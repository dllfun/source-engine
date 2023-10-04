//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#include "dt_send.h"
#include "mathlib/mathlib.h"
#include "mathlib/vector.h"
#include "tier0/dbg.h"
#include <tier0/vprof.h>
#include <tier0/icommandline.h>
#include "dt_utlvector_common.h"
#include "dt_stack.h"
#include "dt_instrumentation_server.h"
#include <checksum_crc.h>
#include "dt_utlvector_send.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define Bits2Bytes(b) ((b+7)>>3)

#if !defined(_STATIC_LINKED) || defined(GAME_DLL)


static CNonModifiedPointerProxy *s_pNonModifiedPointerProxyHead = NULL;


void SendProxy_UInt8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
void SendProxy_UInt16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
void SendProxy_UInt32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
#ifdef SUPPORTS_INT64
void SendProxy_UInt64ToInt64( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID);
#endif
const char *s_ElementNames[MAX_ARRAY_ELEMENTS] =
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

extern bool Sendprop_UsingDebugWatch();

CNonModifiedPointerProxy::CNonModifiedPointerProxy( SendTableProxyFn fn )
{
	m_pNext = s_pNonModifiedPointerProxyHead;
	s_pNonModifiedPointerProxyHead = this;
	m_Fn = fn;
}


CStandardSendProxiesV1::CStandardSendProxiesV1()
{
	m_Int8ToInt32 = SendProxy_Int8ToInt32;
	m_Int16ToInt32 = SendProxy_Int16ToInt32;
	m_Int32ToInt32 = SendProxy_Int32ToInt32;
#ifdef SUPPORTS_INT64
	m_Int64ToInt64 = SendProxy_Int64ToInt64;
#endif

	m_UInt8ToInt32 = SendProxy_UInt8ToInt32;
	m_UInt16ToInt32 = SendProxy_UInt16ToInt32;
	m_UInt32ToInt32 = SendProxy_UInt32ToInt32;
#ifdef SUPPORTS_INT64
	m_UInt64ToInt64 = SendProxy_UInt64ToInt64;
#endif
	
	m_FloatToFloat = SendProxy_FloatToFloat;
	m_VectorToVector = SendProxy_VectorToVector;
}

CStandardSendProxies::CStandardSendProxies()
{	
	m_DataTableToDataTable = SendProxy_DataTableToDataTable;
	m_SendLocalDataTable = SendProxy_SendLocalDataTable;
	m_ppNonModifiedPointerProxies = &s_pNonModifiedPointerProxyHead;
	
}
CStandardSendProxies g_StandardSendProxies;

class CSendTablePrecalc;
class CSendNode;



// This stack doesn't actually call any proxies. It uses the CSendProxyRecipients to tell
// what can be sent to the specified client.
class CPropCullStack : public CDatatableStack
{
public:
	CPropCullStack(
		CSendTablePrecalc* pPrecalc,
		int iClient,
		const CSendProxyRecipients* pOldStateProxies,
		const int nOldStateProxies,
		const CSendProxyRecipients* pNewStateProxies,
		const int nNewStateProxies
	) :

		CDatatableStack(pPrecalc, (unsigned char*)1, -1),
		m_pOldStateProxies(pOldStateProxies),
		m_nOldStateProxies(nOldStateProxies),
		m_pNewStateProxies(pNewStateProxies),
		m_nNewStateProxies(nNewStateProxies)
	{
		m_pPrecalc = pPrecalc;
		m_iClient = iClient;
	}

	inline unsigned char* CallPropProxy(CSendNode* pNode, int iProp, unsigned char* pStructBase)
	{
		if (pNode->GetDataTableProxyIndex() == DATATABLE_PROXY_INDEX_NOPROXY)
		{
			return (unsigned char*)1;
		}
		else
		{
			Assert(pNode->GetDataTableProxyIndex() < m_nNewStateProxies);
			bool bCur = m_pNewStateProxies[pNode->GetDataTableProxyIndex()].m_Bits.Get(m_iClient) != 0;

			if (m_pOldStateProxies)
			{
				Assert(pNode->GetDataTableProxyIndex() < m_nOldStateProxies);
				bool bPrev = m_pOldStateProxies[pNode->GetDataTableProxyIndex()].m_Bits.Get(m_iClient) != 0;
				if (bPrev != bCur)
				{
					if (bPrev)
					{
						// Old state had the data and the new state doesn't.
						return 0;
					}
					else
					{
						// Add ALL properties under this proxy because the previous state didn't have any of them.
						for (int i = 0; i < pNode->m_nRecursiveProps; i++)
						{
							if (m_nNewProxyProps < ARRAYSIZE(m_NewProxyProps))
							{
								m_NewProxyProps[m_nNewProxyProps] = pNode->m_iFirstRecursiveProp + i;
							}
							else
							{
								Error("CPropCullStack::CallPropProxy - overflowed m_nNewProxyProps");
							}

							++m_nNewProxyProps;
						}

						// Tell the outer loop that writes to m_pOutProps not to write anything from this
						// proxy since we just did.
						return 0;
					}
				}
			}

			return (unsigned char*)bCur;
		}
	}

	virtual void RecurseAndCallProxies(CSendNode* pNode, unsigned char* pStructBase)
	{
		// Remember where the game code pointed us for this datatable's data so 
		m_pProxies[pNode->GetRecursiveProxyIndex()] = pStructBase;

		for (int iChild = 0; iChild < pNode->GetNumChildren(); iChild++)
		{
			CSendNode* pCurChild = pNode->GetChild(iChild);

			unsigned char* pNewStructBase = NULL;
			if (pStructBase)
			{
				pNewStructBase = CallPropProxy(pCurChild, pCurChild->m_iDatatableProp, pStructBase);
			}

			RecurseAndCallProxies(pCurChild, pNewStructBase);
		}
	}

	inline void AddProp(int iProp)
	{
		if (m_nOutProps < m_nMaxOutProps)
		{
			m_pOutProps[m_nOutProps] = iProp;
		}
		else
		{
			Error("CPropCullStack::AddProp - m_pOutProps overflowed");
		}

		++m_nOutProps;
	}


	void CullPropsFromProxies(const int* pStartProps, int nStartProps, int* pOutProps, int nMaxOutProps)
	{
		m_nOutProps = 0;
		m_pOutProps = pOutProps;
		m_nMaxOutProps = nMaxOutProps;
		m_nNewProxyProps = 0;

		Init();

		// This list will have any newly available props written into it. Write a sentinel at the end.
		m_NewProxyProps[m_nNewProxyProps] = -1; // invalid marker
		int* pCurNewProxyProp = m_NewProxyProps;

		for (int i = 0; i < nStartProps; i++)
		{
			int iProp = pStartProps[i];

			// Fill in the gaps with any properties that are newly enabled by the proxies.
			while ((unsigned int)*pCurNewProxyProp < (unsigned int)iProp)
			{
				AddProp(*pCurNewProxyProp);
				++pCurNewProxyProp;
			}

			// Now write this property's index if the proxies are allowing this property to be written.
			if (IsPropProxyValid(iProp))
			{
				AddProp(iProp);

				// avoid that we add it twice.
				if (*pCurNewProxyProp == iProp)
					++pCurNewProxyProp;
			}
		}

		// add any remaining new proxy props
		while ((unsigned int)*pCurNewProxyProp < MAX_DATATABLE_PROPS)
		{
			AddProp(*pCurNewProxyProp);
			++pCurNewProxyProp;
		}
	}

	int GetNumOutProps()
	{
		return m_nOutProps;
	}


private:
	CSendTablePrecalc* m_pPrecalc;
	int						m_iClient;	// Which client it's encoding out for.
	const CSendProxyRecipients* m_pOldStateProxies;
	const int					m_nOldStateProxies;

	const CSendProxyRecipients* m_pNewStateProxies;
	const int					m_nNewStateProxies;

	// The output property list.
	int* m_pOutProps;
	int						m_nMaxOutProps;
	int						m_nOutProps;

	int m_NewProxyProps[MAX_DATATABLE_PROPS + 1];
	int m_nNewProxyProps;
};

// ----------------------------------------------------------------------------- //
// CEncodeInfo
// Used by SendTable_Encode.
// ----------------------------------------------------------------------------- //
class CEncodeInfo : public CServerDatatableStack
{
public:
	CEncodeInfo(CSendTablePrecalc* pPrecalc, unsigned char* pStructBase, int objectID, bf_write* pOut) :
		CServerDatatableStack(pPrecalc, pStructBase, objectID),
		m_DeltaBitsWriter(pOut)
	{
	}

public:
	CDeltaBitsWriter m_DeltaBitsWriter;
};

// ---------------------------------------------------------------------- //
// Proxies.
// ---------------------------------------------------------------------- //
void SendProxy_AngleToFloat( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	float angle;

	angle = *((float*)pData);
	pOut->m_Float = anglemod( angle );

	Assert( IsFinite( pOut->m_Float ) );
}

void SendProxy_FloatToFloat( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Float = *((float*)pData);
	Assert( IsFinite( pOut->m_Float ) );
}

void SendProxy_QAngles( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	QAngle *v = (QAngle*)pData;
	pOut->m_Vector[0] = anglemod( v->x );
	pOut->m_Vector[1] = anglemod( v->y );
	pOut->m_Vector[2] = anglemod( v->z );
}

void SendProxy_VectorToVector( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	Vector& v = *(Vector*)pData;
	Assert( v.IsValid() );
	pOut->m_Vector[0] = v[0];
	pOut->m_Vector[1] = v[1];
	pOut->m_Vector[2] = v[2];
}

void SendProxy_VectorXYToVectorXY( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	Vector& v = *(Vector*)pData;
	Assert( v.IsValid() );
	pOut->m_Vector[0] = v[0];
	pOut->m_Vector[1] = v[1];
}

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
void SendProxy_QuaternionToQuaternion( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	Quaternion& q = *(Quaternion*)pData;
	Assert( q.IsValid() );
	pOut->m_Vector[0] = q[0];
	pOut->m_Vector[1] = q[1];
	pOut->m_Vector[2] = q[2];
	pOut->m_Vector[3] = q[3];
}
#endif

void SendProxy_Int8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((const char*)pData);
}

void SendProxy_Int16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((short*)pData);
}

void SendProxy_Int32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((int*)pData);
}

#ifdef SUPPORTS_INT64
void SendProxy_Int64ToInt64( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int64 = *((int64*)pData);
}
#endif

void SendProxy_UInt8ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((const unsigned char*)pData);
}

void SendProxy_UInt16ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_Int = *((unsigned short*)pData);
}

void SendProxy_UInt32ToInt32( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	memcpy( &pOut->m_Int, pData, sizeof(uint32) );
}
#ifdef SUPPORTS_INT64
void SendProxy_UInt64ToInt64( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	*((int64*)&pOut->m_Int64) = *((uint64*)pData);
}
#endif

void SendProxy_StringToString( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
	pOut->m_pString = (const char*)pData;
}

void* SendProxy_DataTableToDataTable( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	return (void*)pData;
}

void* SendProxy_DataTablePtrToDataTable( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	return *((void**)pData);
}

static void SendProxy_Empty( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID)
{
}

//-----------------------------------------------------------------------------
// Purpose: If the recipient is the same as objectID, go ahead and iterate down
//  the m_Local stuff, otherwise, act like it wasn't there at all.
// This way, only the local player receives information about him/herself.
// Input  : *pVarData - 
//			*pOut - 
//			objectID - 
//-----------------------------------------------------------------------------

void* SendProxy_SendLocalDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->SetOnly( objectID - 1 );
	return ( void * )pVarData;
}





// ---------------------------------------------------------------------- //
// Prop setup functions (for building tables).
// ---------------------------------------------------------------------- //
float AssignRangeMultiplier( int nBits, double range )
{
	uint32 iHighValue;
	if ( nBits == 32 )
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ((1 << (uint32)nBits) - 1);

	float fHighLowMul = iHighValue / range;
	if ( CloseEnough( range, 0 ) )
		fHighLowMul = iHighValue;
	
	// If the precision is messing us up, then adjust it so it won't.
	if ( (uint32)(fHighLowMul * range) > iHighValue ||
		 (fHighLowMul * range) > (double)iHighValue )
	{
		// Squeeze it down smaller and smaller until it's going to produce an integer
		// in the valid range when given the highest value.
		float multipliers[] = { 0.9999, 0.99, 0.9, 0.8, 0.7 };
		int i;
		for ( i=0; i < ARRAYSIZE( multipliers ); i++ )
		{
			fHighLowMul = (float)( iHighValue / range ) * multipliers[i];
			if ( (uint32)(fHighLowMul * range) > iHighValue ||
				(fHighLowMul * range) > (double)iHighValue )
			{
			}
			else
			{
				break;
			}
		}

		if ( i == ARRAYSIZE( multipliers ) )
		{
			// Doh! We seem to be unable to represent this range.
			Assert( false );
			return 0;
		}
	}

	return fHighLowMul;
}



SendProp SendPropFloat(
	const char *pVarName,		
	// Variable name.
	int offset,			// Offset into container structure.
	int sizeofVar,
	int nBits,			// Number of bits to use when encoding.
	int flags,
	float fLowValue,		// For floating point, low and high values.
	float fHighValue,		// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if ( varProxy == SendProxy_FloatToFloat )
	{
		Assert( sizeofVar == 0 || sizeofVar == 4 );
	}

	if ( nBits <= 0 || nBits == 32 )
	{
		flags |= SPROP_NOSCALE;
		fLowValue = 0.f;
		fHighValue = 0.f;
	}
	else
	{
		if(fHighValue == HIGH_DEFAULT)
			fHighValue = (1 << nBits);

		if (flags & SPROP_ROUNDDOWN)
			fHighValue = fHighValue - ((fHighValue - fLowValue) / (1 << nBits));
		else if (flags & SPROP_ROUNDUP)
			fLowValue = fLowValue + ((fHighValue - fLowValue) / (1 << nBits));
	}

	ret.m_Type = DPT_Float;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL | SPROP_COORD_MP | SPROP_COORD_MP_LOWPRECISION | SPROP_COORD_MP_INTEGRAL ) )
		ret.m_nBits = 0;

	return ret;
}

SendProp SendPropVector(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,					// Number of bits to use when encoding.
	int flags,
	float fLowValue,			// For floating point, low and high values.
	float fHighValue,			// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_VectorToVector)
	{
		Assert(sizeofVar == sizeof(Vector));
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Vector;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL | SPROP_COORD_MP | SPROP_COORD_MP_LOWPRECISION | SPROP_COORD_MP_INTEGRAL) )
		ret.m_nBits = 0;

	return ret;
}

SendProp SendPropVectorXY(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,					// Number of bits to use when encoding.
	int flags,
	float fLowValue,			// For floating point, low and high values.
	float fHighValue,			// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_VectorXYToVectorXY)
	{
		Assert(sizeofVar == sizeof(Vector));
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_VectorXY;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL | SPROP_COORD_MP | SPROP_COORD_MP_LOWPRECISION | SPROP_COORD_MP_INTEGRAL) )
		ret.m_nBits = 0;

	return ret;
}

#if 0 // We can't ship this since it changes the size of DTVariant to be 20 bytes instead of 16 and that breaks MODs!!!
SendProp SendPropQuaternion(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,					// Number of bits to use when encoding.
	int flags,
	float fLowValue,			// For floating point, low and high values.
	float fHighValue,			// High value. If HIGH_DEFAULT, it's (1<<nBits).
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_QuaternionToQuaternion)
	{
		Assert(sizeofVar == sizeof(Quaternion));
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Quaternion;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = fLowValue;
	ret.m_fHighValue = fHighValue;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & (SPROP_COORD | SPROP_NOSCALE | SPROP_NORMAL | SPROP_COORD_MP | SPROP_COORD_MP_LOWPRECISION | SPROP_COORD_MP_INTEGRAL) )
		ret.m_nBits = 0;

	return ret;
}
#endif

SendProp SendPropAngle(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_AngleToFloat)
	{
		Assert(sizeofVar == 4);
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Float;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = 0.0f;
	ret.m_fHighValue = 360.0f;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );
	ret.SetProxyFn( varProxy );

	return ret;
}


SendProp SendPropQAngles(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if(varProxy == SendProxy_AngleToFloat)
	{
		Assert(sizeofVar == 4);
	}

	if ( nBits == 32 )
		flags |= SPROP_NOSCALE;

	ret.m_Type = DPT_Vector;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );
	ret.m_fLowValue = 0.0f;
	ret.m_fHighValue = 360.0f;
	ret.m_fHighLowMul = AssignRangeMultiplier( ret.m_nBits, ret.m_fHighValue - ret.m_fLowValue );

	ret.SetProxyFn( varProxy );

	return ret;
}
  
SendProp SendPropInt(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int nBits,
	int flags,
	SendVarProxyFn varProxy
	)
{
	SendProp ret;

	if ( !varProxy )
	{
		if ( sizeofVar == 1 )
		{
			varProxy = SendProxy_Int8ToInt32;
		}
		else if ( sizeofVar == 2 )
		{
			varProxy = SendProxy_Int16ToInt32;
		}
		else if ( sizeofVar == 4 )
		{
			varProxy = SendProxy_Int32ToInt32;
		}
#ifdef SUPPORTS_INT64
		else if ( sizeofVar == 8 )
		{
			varProxy = SendProxy_Int64ToInt64;
		}
#endif
		else
		{
			Assert(!"SendPropInt var has invalid size");
			varProxy = SendProxy_Int8ToInt32;	// safest one...
		}
	}

	// Figure out # of bits if the want us to.
	if ( nBits <= 0 )
	{
		Assert( sizeofVar == 1 || sizeofVar == 2 || sizeofVar == 4 );
		nBits = sizeofVar * 8;
	}

#ifdef SUPPORTS_INT64
	ret.m_Type = (sizeofVar == 8) ? DPT_Int64 : DPT_Int;
#else
	ret.m_Type = DPT_Int;
#endif
	
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.m_nBits = nBits;
	ret.SetFlags( flags );

	// Use UInt proxies if they want unsigned data. This isn't necessary to encode
	// the values correctly, but it lets us check the ranges of the data to make sure
	// they're valid.
	ret.SetProxyFn( varProxy );
	if( ret.GetFlags() & SPROP_UNSIGNED )
	{
		if( varProxy == SendProxy_Int8ToInt32 )
			ret.SetProxyFn( SendProxy_UInt8ToInt32 );
		
		else if( varProxy == SendProxy_Int16ToInt32 )
			ret.SetProxyFn( SendProxy_UInt16ToInt32 );

		else if( varProxy == SendProxy_Int32ToInt32 )
			ret.SetProxyFn( SendProxy_UInt32ToInt32 );
			
#ifdef SUPPORTS_INT64
		else if( varProxy == SendProxy_Int64ToInt64 )
			ret.SetProxyFn( SendProxy_UInt64ToInt64 );
#endif
	}

	return ret;
}

SendProp SendPropString(
	const char *pVarName,
	int offset,
	int bufferLen,
	int flags,
	SendVarProxyFn varProxy)
{
	SendProp ret;

	Assert( bufferLen <= DT_MAX_STRING_BUFFERSIZE ); // You can only have strings with 8-bits worth of length.
	
	ret.m_Type = DPT_String;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.SetFlags( flags );
	ret.SetProxyFn( varProxy );

	return ret;
}

SendProp SendPropArray3(
	const char *pVarName,
	int offset,
	int sizeofVar,
	int elements,
	SendProp pArrayProp,
	SendTableProxyFn varProxy
	)
{
	SendProp ret;

	Assert( elements <= MAX_ARRAY_ELEMENTS );

	ret.m_Type = DPT_DataTable;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	ret.SetDataTableProxyFn( varProxy );
	
	SendProp *pArrayPropAllocated = new SendProp;
	*pArrayPropAllocated = pArrayProp;
	ret.SetArrayProp( pArrayPropAllocated );
	
	// Handle special proxy types where they always let all clients get the results.
	if ( varProxy == SendProxy_DataTableToDataTable || varProxy == SendProxy_DataTablePtrToDataTable )
	{
		ret.SetFlags( SPROP_PROXY_ALWAYS_YES );
	}

	SendProp *pProps = new SendProp[elements]; // TODO free that again
	
	for ( int i = 0; i < elements; i++ )
	{
		pProps[i] = pArrayProp;	// copy array element property setting
		pProps[i].SetOffset( i*sizeofVar ); // adjust offset
		pProps[i].m_pVarName = COM_StringCopy(s_ElementNames[i]);	// give unique name
		if (pVarName) {
			pProps[i].m_pParentArrayPropName = COM_StringCopy(pVarName); // For debugging...
		}
	}

	SendTable pTable = SendTable( pProps, elements, pVarName ); // TODO free that again
	GetSendTableManager()->RegisteSendTable(&pTable);
	if (pVarName) {
		ret.SetDataTableName(COM_StringCopy(pVarName));
	}
	//ret.SetDataTable( pTable );

	return ret;
}

SendProp SendPropDataTable(
	const char *pVarName,
	int offset,
	const char *pTableName,
	SendTableProxyFn varProxy
	)
{
	SendProp ret;

	ret.m_Type = DPT_DataTable;
	if (pVarName) {
		ret.m_pVarName = COM_StringCopy(pVarName);
	}
	ret.SetOffset( offset );
	if (pTableName) {
		ret.SetDataTableName(COM_StringCopy(pTableName));
	}
	ret.SetDataTable( NULL );
	ret.SetDataTableProxyFn( varProxy );
	
	// Handle special proxy types where they always let all clients get the results.
	if ( varProxy == SendProxy_DataTableToDataTable || varProxy == SendProxy_DataTablePtrToDataTable )
	{
		ret.SetFlags( SPROP_PROXY_ALWAYS_YES );
	}
	
	if ( varProxy == SendProxy_DataTableToDataTable && offset == 0 )
	{
		ret.SetFlags( SPROP_COLLAPSIBLE );
	}

	return ret;
}


SendProp InternalSendPropArray(
	const int elementCount,
	const int elementStride,
	const char *pName,
	ArrayLengthSendProxyFn arrayLengthFn
	)
{
	SendProp ret;

	ret.m_Type = DPT_Array;
	ret.m_nElements = elementCount;
	ret.m_ElementStride = elementStride;
	if (pName) {
		ret.m_pVarName = COM_StringCopy(pName);
	}
	ret.SetProxyFn( SendProxy_Empty );
	ret.m_pArrayProp = NULL;	// This gets set in SendTable_InitTable. It always points at the property that precedes
								// this one in the datatable's list.
	ret.SetArrayLengthProxy( arrayLengthFn );
		
	return ret;
}


SendProp SendPropExclude(
	const char *pDataTableName,	// Data table name (given to BEGIN_SEND_TABLE and BEGIN_RECV_TABLE).
	const char *pPropName		// Name of the property to exclude.
	)
{
	SendProp ret;

	ret.SetFlags( SPROP_EXCLUDE );
	if (pDataTableName) {
		ret.m_pExcludeDTName = COM_StringCopy(pDataTableName);
	}
	if (pPropName) {
		ret.m_pVarName = COM_StringCopy(pPropName);
	}

	return ret;
}



// ---------------------------------------------------------------------- //
// SendProp
// ---------------------------------------------------------------------- //
SendProp::SendProp()
{
	m_pVarName = NULL;
	m_Offset = 0;
	m_pDataTable = NULL;
	m_ProxyFn = NULL;
	m_pExcludeDTName = NULL;
	m_pParentArrayPropName = NULL;

	
	m_Type = DPT_Int;
	m_Flags = 0;
	m_nBits = 0;

	m_fLowValue = 0.0f;
	m_fHighValue = 0.0f;
	m_pArrayProp = 0;
	m_ArrayLengthProxy = 0;
	m_nElements = 1;
	m_ElementStride = -1;
	m_pExtraData = NULL;
}


SendProp::~SendProp()
{
	if (this->m_pArrayProp && !this->m_pArrayProp->IsInsideArray()) {
		delete this->m_pArrayProp;
		this->m_pArrayProp = NULL;
	}
	if (this->m_pExcludeDTName) {
		delete this->m_pExcludeDTName;
		this->m_pExcludeDTName = NULL;
	}
	if (this->m_pParentArrayPropName) {
		delete this->m_pParentArrayPropName;
		this->m_pParentArrayPropName = NULL;
	}
	if (this->m_pParentArrayPropName) {
		delete this->m_pParentArrayPropName;
		this->m_pParentArrayPropName = NULL;
	}
	if (this->m_pVarName) {
		delete this->m_pVarName;
		this->m_pVarName = NULL;
	}
	if (this->m_pDataTableName) {
		delete this->m_pDataTableName;
		this->m_pDataTableName = NULL;
	}
	if (this->m_pDataTable) {
		delete this->m_pDataTable;
		this->m_pDataTable = NULL;
	}
	if (this->m_pExtraData) {
		delete this->m_pExtraData;
		this->m_pExtraData = NULL;
	}
}

SendProp& SendProp::operator=(const SendProp& srcSendProp) {
	if (this != &srcSendProp) {
		this->m_Type = (SendPropType)srcSendProp.m_Type;
		this->m_nBits = srcSendProp.m_nBits;
		this->m_fLowValue = srcSendProp.m_fLowValue;
		this->m_fHighValue = srcSendProp.m_fHighValue;
		if (this->m_pArrayProp && !this->m_pArrayProp->IsInsideArray()) {
			delete this->m_pArrayProp;
			this->m_pArrayProp = NULL;
		}
		if (srcSendProp.m_pArrayProp) {
			this->m_pArrayProp = new SendProp;
			*this->m_pArrayProp = *srcSendProp.m_pArrayProp;
		}
		this->m_ArrayLengthProxy = srcSendProp.m_ArrayLengthProxy;
		this->m_nElements = srcSendProp.m_nElements;
		this->m_ElementStride = srcSendProp.m_ElementStride;
		if (this->m_pExcludeDTName) {
			delete this->m_pExcludeDTName;
			this->m_pExcludeDTName = NULL;
		}
		if (srcSendProp.m_pExcludeDTName) {
			this->m_pExcludeDTName = COM_StringCopy(srcSendProp.m_pExcludeDTName);
		}
		if (this->m_pParentArrayPropName) {
			delete this->m_pParentArrayPropName;
			this->m_pParentArrayPropName = NULL;
		}
		if (srcSendProp.m_pParentArrayPropName) {
			this->m_pParentArrayPropName = COM_StringCopy(srcSendProp.m_pParentArrayPropName);
		}
		if (this->m_pVarName) {
			delete this->m_pVarName;
			this->m_pVarName = NULL;
		}
		if (srcSendProp.m_pVarName) {
			this->m_pVarName = COM_StringCopy(srcSendProp.m_pVarName);
		}
		this->m_fHighLowMul = srcSendProp.m_fHighLowMul;
		this->m_Flags = srcSendProp.m_Flags;
		this->m_ProxyFn = srcSendProp.m_ProxyFn;
		this->m_DataTableProxyFn = srcSendProp.m_DataTableProxyFn;
		if (this->m_pDataTableName) {
			delete this->m_pDataTableName;
			this->m_pDataTableName = NULL;
		}
		if (srcSendProp.m_pDataTableName) {
			this->m_pDataTableName = COM_StringCopy(srcSendProp.m_pDataTableName);
		}
		if (this->m_pDataTable) {
			delete this->m_pDataTable;
			this->m_pDataTable = NULL;
		}
		if (srcSendProp.m_pDataTable) {
			this->m_pDataTable = new SendTable();
			*this->m_pDataTable = *srcSendProp.m_pDataTable;
		}
		this->m_Offset = srcSendProp.m_Offset;
		if (this->m_pExtraData) {
			delete this->m_pExtraData;
			this->m_pExtraData = NULL;
		}
		if (srcSendProp.m_pExtraData) {
			this->m_pExtraData = new CSendPropExtra_UtlVector();
			*(CSendPropExtra_UtlVector*)this->m_pExtraData = *(CSendPropExtra_UtlVector*)srcSendProp.m_pExtraData;
		}
	}
	return *this;
}

void SendProp::SetDataTable(SendTable* pTable)
{
	if (this->m_pDataTable) {
		delete this->m_pDataTable;
		this->m_pDataTable = NULL;
	}
	m_pDataTable = pTable;
}

void SendProp::SetExtraData(const void* pData)
{
	if (this->m_pExtraData != pData) {
		if (this->m_pExtraData) {
			delete this->m_pExtraData;
			this->m_pExtraData = NULL;
		}
		m_pExtraData = pData;
	}
}

int SendProp::GetNumArrayLengthBits() const
{
	Assert( GetType() == DPT_Array );
#if _X360
	int elemCount = GetNumElements();
	if ( !elemCount )
		return 1;
	return (32 - _CountLeadingZeros(GetNumElements()));
#else
	return Q_log2( GetNumElements() ) + 1;
#endif
}


// ---------------------------------------------------------------------- //
// SendTable
// ---------------------------------------------------------------------- //
SendTable::SendTable()
{
	Construct( NULL, 0, NULL );
	Init();
	int aaa = 0;
	const char* Name = m_pNetTableName;
}


SendTable::SendTable(SendProp *pProps, int nProps, const char *pNetTableName)
{
	Construct( pProps, nProps, pNetTableName );
}


SendTable::~SendTable()
{
//	Assert( !m_pPrecalc );
	if (this->m_pNetTableName) {
		delete this->m_pNetTableName;
		this->m_pNetTableName = NULL;
	}
	if (this->m_pProps) {
		delete[] this->m_pProps;
		this->m_pProps = NULL;
	}
}


void SendTable::Construct( SendProp *pProps, int nProps, const char *pNetTableName )
{
	//m_pProps = pProps;
	//m_nProps = nProps;
	//m_pNetTableName = pNetTableName;
	if (this->m_pNetTableName) {
		delete this->m_pNetTableName;
		this->m_pNetTableName = NULL;
	}
	if (pNetTableName) {
		this->m_pNetTableName = COM_StringCopy(pNetTableName);
	}
	this->m_nProps = nProps;
	if (this->m_pProps) {
		delete[] this->m_pProps;
		this->m_pProps = NULL;
	}
	this->m_pProps = this->m_nProps ? new SendProp[this->m_nProps] : NULL;
	for (int iProp = 0; iProp < this->m_nProps; iProp++)
	{
		SendProp* pProp = &this->m_pProps[iProp];
		const SendProp* pSendTableProp = &pProps[iProp];
		*pProp = *pSendTableProp;
	}
	m_pPrecalc = 0;
	m_bInitialized = false;
	m_bHasBeenWritten = false;
	m_bHasPropsEncodedAgainstCurrentTickCount = false;
}

SendTable& SendTable::operator=(const SendTable& srcSendTable) {
	if (this != &srcSendTable) {
		if (this->m_pNetTableName) {
			delete this->m_pNetTableName;
			this->m_pNetTableName = NULL;
		}
		if (srcSendTable.m_pNetTableName) {
			this->m_pNetTableName = COM_StringCopy(srcSendTable.m_pNetTableName);
		}
		this->m_nProps = srcSendTable.m_nProps;
		if (this->m_pProps) {
			delete[] this->m_pProps;
			this->m_pProps = NULL;
		}
		this->m_pProps = this->m_nProps ? new SendProp[this->m_nProps] : NULL;
		for (int iProp = 0; iProp < this->m_nProps; iProp++)
		{
			SendProp* pProp = &this->m_pProps[iProp];
			const SendProp* pSendTableProp = &srcSendTable.m_pProps[iProp];
			*pProp = *pSendTableProp;
		}
	}
	return *this;
}

void SendTable::InitRefSendTable(SendTableManager* pSendTableNanager) {
	if (m_RefTableInited) {
		return;
	}
	m_RefTableInited = true;
	if (!V_strcmp(m_pNetTableName, "_ST_m_AnimOverlay_15")) {
		int aaa = 0;
	}
	for (int i = 0; i < m_nProps; i++)
	{
		SendProp* pProp = &m_pProps[i];
		if (pProp->GetType() == DPT_DataTable && pProp->GetDataTableName() && pProp->GetDataTableName()[0])
		{
			SendTable* pSrcSendTable = pSendTableNanager->FindSendTable(pProp->GetDataTableName());
			if (!pSrcSendTable) {
				Error("not found SendTable: %s\n", pProp->GetDataTableName());	// dedicated servers exit
			}
			pSrcSendTable->m_bIsLeaf = false;
			pSrcSendTable->InitRefSendTable(pSendTableNanager);
			SendTable* pSendTable = new SendTable();
			*pSendTable = *pSrcSendTable;
			pProp->SetDataTable(pSendTable);
		}
	}
}

static void SendTable_CalcNextVectorElems(SendTable* pTable)
{
	for (int i = 0; i < pTable->GetNumProps(); i++)
	{
		SendProp* pProp = pTable->GetProp(i);

		if (pProp->GetType() == DPT_DataTable)
		{
			SendTable_CalcNextVectorElems(pProp->GetDataTable());
		}
		else if (pProp->GetOffset() < 0)
		{
			pProp->SetOffset(-pProp->GetOffset());
			pProp->SetFlags(pProp->GetFlags() | SPROP_IS_A_VECTOR_ELEM);
		}
	}
}

// Spits out warnings for invalid properties and forces property values to
// be in valid ranges for the encoders and decoders.
static void SendTable_Validate(CSendTablePrecalc* pPrecalc)
{
	SendTable* pTable = pPrecalc->m_pSendTable;
	for (int i = 0; i < pTable->m_nProps; i++)
	{
		SendProp* pProp = &pTable->m_pProps[i];

		if (pProp->GetArrayProp())
		{
			if (pProp->GetArrayProp()->GetType() == DPT_DataTable)
			{
				Error("Invalid property: %s/%s (array of datatables) [on prop %d of %d (%s)].", pTable->m_pNetTableName, pProp->GetName(), i, pTable->m_nProps, pProp->GetArrayProp()->GetName());
			}
		}
		else
		{
			ErrorIfNot(pProp->GetNumElements() == 1, ("Prop %s/%s has an invalid element count for a non-array.", pTable->m_pNetTableName, pProp->GetName()));
		}

		// Check for 1-bit signed properties (their value doesn't get down to the client).
		if (pProp->m_nBits == 1 && !(pProp->GetFlags() & SPROP_UNSIGNED))
		{
			DataTable_Warning("SendTable prop %s::%s is a 1-bit signed property. Use SPROP_UNSIGNED or the client will never receive a value.\n", pTable->m_pNetTableName, pProp->GetName());
		}
	}

	for (int i = 0; i < pPrecalc->GetNumProps(); ++i)
	{
		const SendProp* pProp = pPrecalc->GetProp(i);
		if (pProp->GetFlags() & SPROP_ENCODED_AGAINST_TICKCOUNT)
		{
			pTable->SetHasPropsEncodedAgainstTickcount(true);
			break;
		}
	}
}

bool SendTable::SendTable_InitTable()
{
	if (this->m_pPrecalc)
		return true;

	// Create the CSendTablePrecalc.	
	CSendTablePrecalc* pPrecalc = new CSendTablePrecalc;
	this->m_pPrecalc = pPrecalc;

	pPrecalc->m_pSendTable = this;
	this->m_pPrecalc = pPrecalc;

	SendTable_CalcNextVectorElems(this);

	// Bind the instrumentation if -dti was specified.
	pPrecalc->m_pDTITable = ServerDTI_HookTable(this);

	// Setup its flat property array.
	if (!pPrecalc->SetupFlatPropertyArray())
		return false;

	SendTable_Validate(pPrecalc);
	return true;
}

void SendTable::SendTable_TermTable()
{
	if (!this->m_pPrecalc)
		return;

	delete this->m_pPrecalc;
	Assert(!this->m_pPrecalc); // Make sure it unbound itself.
}

int SendTable::SendTable_GetNumFlatProps()
{
	CSendTablePrecalc* pPrecalc = this->m_pPrecalc;
	ErrorIfNot(pPrecalc,
		("SendTable_GetNumFlatProps: missing pPrecalc.")
	);
	return pPrecalc->GetNumProps();
}

// compares properties and writes delta properties, it ignores reciepients
int SendTable::SendTable_WriteAllDeltaProps(
	const void* pFromData,
	const int	nFromDataBits,
	const void* pToData,
	const int nToDataBits,
	const int nObjectID,
	bf_write* pBufOut)
{
	// Calculate the delta props.
	int deltaProps[MAX_DATATABLE_PROPS];

	int nDeltaProps = this->SendTable_CalcDelta(
		pFromData,
		nFromDataBits,
		pToData,
		nToDataBits,
		deltaProps,
		ARRAYSIZE(deltaProps),
		nObjectID);

	// Write the properties.
	this->SendTable_WritePropList(
		pToData,				// object data
		nToDataBits,
		pBufOut,				// output buffer
		nObjectID,
		deltaProps,
		nDeltaProps
	);

	return nDeltaProps;
}

static bool s_debug_info_shown = false;
static int  s_debug_bits_start = 0;

static inline void ShowEncodeDeltaWatchInfo(
	const SendTable* pTable,
	const SendProp* pProp,
	bf_read& buffer,
	const int objectID,
	const int index)
{
	if (!ShouldWatchThisProp(pTable, objectID, pProp->GetName()))
		return;

	//static int lastframe = -1;
	//if (g_pHost->Host_GetFrameCount() != lastframe)
	//{
	//	lastframe = g_pHost->Host_GetFrameCount();
	//	ConDMsg("delta entity: %i\n", objectID);
	//}

	// work on copy of bitbuffer
	bf_read copy = buffer;

	s_debug_info_shown = true;

	DecodeInfo info;
	info.m_pStruct = NULL;
	info.m_pData = NULL;
	info.m_pRecvProp = NULL;
	info.m_pProp = pProp;
	info.m_pIn = &copy;
	info.m_Value.m_Type = (SendPropType)pProp->m_Type;

	int startBit = copy.GetNumBitsRead();

	g_PropTypeFns[pProp->m_Type].Decode(&info);

	int bits = copy.GetNumBitsRead() - startBit;

	const char* type = g_PropTypeFns[pProp->m_Type].GetTypeNameString();
	const char* value = info.m_Value.ToString();

	ConDMsg("+ %s %s, %s, index %i, bits %i, value %s\n", pTable->GetName(), pProp->GetName(), type, index, bits, value);
}

void SendTable::SendTable_WritePropList(
	const void* pState,
	const int nBits,
	bf_write* pOut,
	const int objectID,
	const int* pCheckProps,
	const int nCheckProps
)
{
	if (nCheckProps == 0)
	{
		// Write single final zero bit, signifying that there no changed properties
		pOut->WriteOneBit(0);
		return;
	}

	bool bDebugWatch = Sendprop_UsingDebugWatch();

	s_debug_info_shown = false;
	s_debug_bits_start = pOut->GetNumBitsWritten();

	CSendTablePrecalc* pPrecalc = this->m_pPrecalc;
	CDeltaBitsWriter deltaBitsWriter(pOut);

	bf_read inputBuffer("SendTable_WritePropList->inputBuffer", pState, BitByte(nBits), nBits);
	CDeltaBitsReader inputBitsReader(&inputBuffer);

	// Ok, they want to specify a small list of properties to check.
	unsigned int iToProp = inputBitsReader.ReadNextPropIndex();
	int i = 0;
	while (i < nCheckProps)
	{
		// Seek the 'to' state to the current property we want to check.
		while (iToProp < (unsigned int)pCheckProps[i])
		{
			inputBitsReader.SkipPropData(pPrecalc->GetProp(iToProp));
			iToProp = inputBitsReader.ReadNextPropIndex();
		}

		if (iToProp >= MAX_DATATABLE_PROPS)
		{
			break;
		}

		if (iToProp == (unsigned int)pCheckProps[i])
		{
			const SendProp* pProp = pPrecalc->GetProp(iToProp);

			// Show debug stuff.
			if (bDebugWatch)
			{
				ShowEncodeDeltaWatchInfo(this, pProp, inputBuffer, objectID, iToProp);
			}

			// See how many bits the data for this property takes up.
			int nToStateBits;
			int iStartBit = pOut->GetNumBitsWritten();

			deltaBitsWriter.WritePropIndex(iToProp);
			inputBitsReader.CopyPropData(deltaBitsWriter.GetBitBuf(), pProp);

			nToStateBits = pOut->GetNumBitsWritten() - iStartBit;

			//aaa
			//TRACE_PACKET(("    Send Field (%s) = %d (%d bytes)\n", pProp->GetName(), nToStateBits, (nToStateBits + 7) / 8));

			// Seek to the next prop.
			iToProp = inputBitsReader.ReadNextPropIndex();
		}

		++i;
	}

	if (s_debug_info_shown)
	{
		int  bits = pOut->GetNumBitsWritten() - s_debug_bits_start;
		ConDMsg("= %i bits (%i bytes)\n", bits, Bits2Bytes(bits));
	}

	inputBitsReader.ForceFinished(); // avoid a benign assert
}

int SendTable::SendTable_CalcDelta(
	const void* pFromState,
	const int nFromBits,

	const void* pToState,
	const int nToBits,

	int* pDeltaProps,
	int nMaxDeltaProps,

	const int objectID
)
{
	CServerDTITimer timer(this, SERVERDTI_CALCDELTA);

	int* pDeltaPropsBase = pDeltaProps;
	int* pDeltaPropsEnd = pDeltaProps + nMaxDeltaProps;

	VPROF("SendTable_CalcDelta");

	// Trivial reject.
	//if ( CompareBitArrays( pFromState, pToState, nFromBits, nToBits ) )
	//{
	//	return 0;
	//}

	CSendTablePrecalc* pPrecalc = this->m_pPrecalc;

	bf_read toBits("SendTable_CalcDelta/toBits", pToState, BitByte(nToBits), nToBits);
	CDeltaBitsReader toBitsReader(&toBits);
	unsigned int iToProp = toBitsReader.ReadNextPropIndex();

	if (pFromState)
	{
		bf_read fromBits("SendTable_CalcDelta/fromBits", pFromState, BitByte(nFromBits), nFromBits);
		CDeltaBitsReader fromBitsReader(&fromBits);
		unsigned int iFromProp = fromBitsReader.ReadNextPropIndex();

		for (; iToProp < MAX_DATATABLE_PROPS; iToProp = toBitsReader.ReadNextPropIndex())
		{
			Assert((int)iToProp >= 0);

			// Skip any properties in the from state that aren't in the to state.
			while (iFromProp < iToProp)
			{
				fromBitsReader.SkipPropData(pPrecalc->GetProp(iFromProp));
				iFromProp = fromBitsReader.ReadNextPropIndex();
			}

			if (iFromProp == iToProp)
			{
				// The property is in both states, so compare them and write the index 
				// if the states are different.
				if (fromBitsReader.ComparePropData(&toBitsReader, pPrecalc->GetProp(iToProp)))
				{
					*pDeltaProps++ = iToProp;
					if (pDeltaProps >= pDeltaPropsEnd)
					{
						break;
					}
				}

				// Seek to the next property.
				iFromProp = fromBitsReader.ReadNextPropIndex();
			}
			else
			{
				// Only the 'to' state has this property, so just skip its data and register a change.
				toBitsReader.SkipPropData(pPrecalc->GetProp(iToProp));
				*pDeltaProps++ = iToProp;
				if (pDeltaProps >= pDeltaPropsEnd)
				{
					break;
				}
			}
		}

		Assert(iToProp == ~0u);

		fromBitsReader.ForceFinished();
	}
	else
	{
		for (; iToProp != (uint)-1; iToProp = toBitsReader.ReadNextPropIndex())
		{
			Assert((int)iToProp >= 0 && iToProp < MAX_DATATABLE_PROPS);

			const SendProp* pProp = pPrecalc->GetProp(iToProp);
			if (!g_PropTypeFns[pProp->m_Type].IsEncodedZero(pProp, &toBits))
			{
				*pDeltaProps++ = iToProp;
				if (pDeltaProps >= pDeltaPropsEnd)
				{
					break;
				}
			}
		}
	}

	// Return the # of properties that changed between 'from' and 'to'.
	return pDeltaProps - pDeltaPropsBase;
}

int SendTable::SendTable_CullPropsFromProxies(
	const int *pStartProps,
	int nStartProps,

	const int iClient,
	
	const CSendProxyRecipients *pOldStateProxies,
	const int nOldStateProxies, 
	
	const CSendProxyRecipients *pNewStateProxies,
	const int nNewStateProxies,
	
	int *pOutProps,
	int nMaxOutProps
	)
{
	Assert( !( nNewStateProxies && !pNewStateProxies ) );
	CPropCullStack stack( this->m_pPrecalc, iClient, pOldStateProxies, nOldStateProxies, pNewStateProxies, nNewStateProxies );
	
	stack.CullPropsFromProxies( pStartProps, nStartProps, pOutProps, nMaxOutProps );

	ErrorIfNot( stack.GetNumOutProps() <= nMaxOutProps, ("CullPropsFromProxies: overflow in '%s'.", this->GetName()) );
	return stack.GetNumOutProps();
}

static bool SendTable_IsPropZero(CEncodeInfo* pInfo, unsigned long iProp)
{
	const SendProp* pProp = pInfo->GetCurProp();

	// Call their proxy to get the property's value.
	DVariant var;
	unsigned char* pBase = pInfo->GetCurStructBase();

	pProp->GetProxyFn()(
		pProp,
		pBase,
		pBase + pProp->GetOffset(),
		&var,
		0, // iElement
		pInfo->GetObjectID()
		);

	return g_PropTypeFns[pProp->m_Type].IsZero(pBase, &var, pProp);
}

static FORCEINLINE void SendTable_EncodeProp(CEncodeInfo* pInfo, unsigned long iProp)
{
	// Call their proxy to get the property's value.
	DVariant var;

	const SendProp* pProp = pInfo->GetCurProp();
	unsigned char* pStructBase = pInfo->GetCurStructBase();

	pProp->GetProxyFn()(
		pProp,
		pStructBase,
		pStructBase + pProp->GetOffset(),
		&var,
		0, // iElement
		pInfo->GetObjectID()
		);

	// Write the index.
	pInfo->m_DeltaBitsWriter.WritePropIndex(iProp);

	g_PropTypeFns[pProp->m_Type].Encode(
		pStructBase,
		&var,
		pProp,
		pInfo->m_DeltaBitsWriter.GetBitBuf(),
		pInfo->GetObjectID()
	);
}

bool SendTable::SendTable_Encode(
	const void* pStruct,
	bf_write* pOut,
	int objectID,
	CUtlMemory<CSendProxyRecipients>* pRecipients,
	bool bNonZeroOnly
)
{
	CSendTablePrecalc* pPrecalc = this->m_pPrecalc;
	ErrorIfNot(pPrecalc, ("SendTable_Encode: Missing m_pPrecalc for SendTable %s.", this->m_pNetTableName));
	if (pRecipients)
	{
		ErrorIfNot(pRecipients->NumAllocated() >= pPrecalc->GetNumDataTableProxies(), ("SendTable_Encode: pRecipients array too small."));
	}

	VPROF("SendTable_Encode");

	CServerDTITimer timer(this, SERVERDTI_ENCODE);

	// Setup all the info we'll be walking the tree with.
	CEncodeInfo info(pPrecalc, (unsigned char*)pStruct, objectID, pOut);
	info.m_pRecipients = pRecipients;	// optional buffer to store the bits for which clients get what data.

	info.Init();

	int iNumProps = pPrecalc->GetNumProps();

	for (int iProp = 0; iProp < iNumProps; iProp++)
	{
		// skip if we don't have a valid prop proxy
		if (!info.IsPropProxyValid(iProp))
			continue;

		info.SeekToProp(iProp);

		// skip empty prop if we only encode non-zero values
		if (bNonZeroOnly && SendTable_IsPropZero(&info, iProp))
			continue;

		SendTable_EncodeProp(&info, iProp);
	}

	return !pOut->IsOverflowed();
}

//-----------------------------------------------------------------------------
// Purpose: check integrity of an unpacked entity send table
//-----------------------------------------------------------------------------
bool SendTable::SendTable_CheckIntegrity(const void* pData, const int nDataBits)
{
#ifdef _DEBUG
	if (pData == NULL && nDataBits == 0)
		return true;

	bf_read bfRead("SendTable_CheckIntegrity", pData, Bits2Bytes(nDataBits), nDataBits);
	CDeltaBitsReader bitsReader(&bfRead);

	int iProp = -1;
	int iLastProp = -1;
	int nMaxProps = pTable->m_pPrecalc->GetNumProps();
	int nPropCount = 0;

	Assert(nMaxProps > 0 && nMaxProps < MAX_DATATABLE_PROPS);

	while (-1 != (iProp = bitsReader.ReadNextPropIndex()))
	{
		Assert((iProp >= 0) && (iProp < nMaxProps));

		// must be larger
		Assert(iProp > iLastProp);

		const SendProp* pProp = pTable->m_pPrecalc->GetProp(iProp);

		Assert(pProp);

		// ok check that SkipProp & IsEncodedZero read the same bit length
		int iStartBit = bfRead.GetNumBitsRead();
		g_PropTypeFns[pProp->GetType()].SkipProp(pProp, &bfRead);
		int nLength = bfRead.GetNumBitsRead() - iStartBit;

		Assert(nLength > 0); // a prop must have some bits

		bfRead.Seek(iStartBit); // read it again

		g_PropTypeFns[pProp->GetType()].IsEncodedZero(pProp, &bfRead);

		Assert(nLength == (bfRead.GetNumBitsRead() - iStartBit));

		nPropCount++;
		iLastProp = iProp;
	}

	Assert(nPropCount <= nMaxProps);
	Assert(bfRead.GetNumBytesLeft() < 4);
	Assert(!bfRead.IsOverflowed());

#endif

	return true;
}

SendTable* SendTableManager::RegisteSendTable(SendTable* pSrcSendTable) {

	SendTable* pSendTable = new SendTable;
	*pSendTable = *pSrcSendTable;
	// Read the name.
	//if (pSendTable->m_pNetTableName) {
	//	pTable->m_pNetTableName = COM_StringCopy(pSendTable->m_pNetTableName);
	//}
	//pTable->m_nProps = pSendTable->m_nProps;
	//pTable->m_pProps = pTable->m_nProps ? new SendProp[pTable->m_nProps] : 0;
	//for (int iProp = 0; iProp < pTable->m_nProps; iProp++)
	//{
	//	SendProp* pProp = &pTable->m_pProps[iProp];
	//	const SendProp* pSendTableProp = &pSendTable->m_pProps[iProp];

	//	pProp->m_Type = (SendPropType)pSendTableProp->m_Type;
	//	pProp->m_nBits = pSendTableProp->m_nBits;
	//	pProp->m_fLowValue = pSendTableProp->m_fLowValue;
	//	pProp->m_fHighValue = pSendTableProp->m_fHighValue;
	//	if (pSendTableProp->m_pArrayProp) {
	//		SendProp* pArrayPropAllocated = new SendProp;
	//		*pArrayPropAllocated = *pSendTableProp->m_pArrayProp;
	//		pProp->SetArrayProp(pArrayPropAllocated);
	//	}
	//	pProp->m_ArrayLengthProxy = pSendTableProp->m_ArrayLengthProxy;
	//	pProp->SetNumElements(pSendTableProp->GetNumElements());
	//	pProp->m_ElementStride = pSendTableProp->m_ElementStride;
	//	if (pSendTableProp->GetExcludeDTName()) {
	//		pProp->m_pExcludeDTName = COM_StringCopy(pSendTableProp->GetExcludeDTName());
	//	}
	//	if (pSendTableProp->GetParentArrayPropName()) {
	//		pProp->m_pParentArrayPropName = COM_StringCopy(pSendTableProp->GetParentArrayPropName());
	//	}
	//	if (pSendTableProp->GetName()) {
	//		pProp->m_pVarName = COM_StringCopy(pSendTableProp->GetName());
	//	}
	//	pProp->m_fHighLowMul = pSendTableProp->m_fHighLowMul;
	//	pProp->SetFlags(pSendTableProp->GetFlags());
	//	pProp->SetProxyFn(pSendTableProp->GetProxyFn());
	//	pProp->SetDataTableProxyFn(pSendTableProp->GetDataTableProxyFn());
	//	if (pSendTableProp->GetDataTableName()) {
	//		pProp->SetDataTableName(COM_StringCopy(pSendTableProp->GetDataTableName()));
	//	}
	//	pProp->SetOffset(pSendTableProp->GetOffset());
	//	pProp->SetExtraData(NULL);
	//}

	if (m_SendTableMap.Defined(pSendTable->GetName())) {
		Error("duplicate SendTable: %s\n", pSendTable->GetName());	// dedicated servers exit
	}
	else {
		m_SendTableMap[pSendTable->GetName()] = pSendTable;
	}
	if (!m_pSendTableHead)
	{
		m_pSendTableHead = pSendTable;
		pSendTable->m_pNext = NULL;
	}
	else
	{
		SendTable* p1 = m_pSendTableHead;
		SendTable* p2 = p1->m_pNext;

		// use _stricmp because Q_stricmp isn't hooked up properly yet
		if (_stricmp(p1->GetName(), pSendTable->GetName()) > 0)
		{
			pSendTable->m_pNext = m_pSendTableHead;
			m_pSendTableHead = pSendTable;
			p1 = NULL;
		}

		while (p1)
		{
			if (p2 == NULL || _stricmp(p2->GetName(), pSendTable->GetName()) > 0)
			{
				pSendTable->m_pNext = p2;
				p1->m_pNext = pSendTable;
				break;
			}
			p1 = p2;
			p2 = p2->m_pNext;
		}
	}
	return pSendTable;
}

bool SendTableManager::SendTable_Init()//SendTable** pTables, int nTables
{
	ErrorIfNot(!m_Inited,
		("SendTable_Init: called twice.")
	);
	m_Inited = true;

	int nTables = m_SendTableMap.GetNumStrings();
	// Initialize them all.
	for (int i = 0; i < nTables; i++)
	{
		if (!m_SendTableMap[i]->SendTable_InitTable())
			return false;
	}

	// Store off the SendTable list.
	//g_SendTables.CopyArray(pTables, nTables);

	m_SendTableCRC = SendTable_ComputeCRC();

	if (CommandLine()->FindParm("-dti"))
	{
		SendTable_PrintStats();
	}

	return true;
}

void SendTableManager::SendTable_PrintStats(void)
{
	int numTables = 0;
	int numFloats = 0;
	int numStrings = 0;
	int numArrays = 0;
	int numInts = 0;
	int numVecs = 0;
	int numVecXYs = 0;
	int numSubTables = 0;
	int numSendProps = 0;
	int numFlatProps = 0;
	int numExcludeProps = 0;

	int nTables = m_SendTableMap.GetNumStrings();
	for (int i = 0; i < nTables; i++)
	{
		SendTable* st = m_SendTableMap[i];

		numTables++;
		numSendProps += st->GetNumProps();
		numFlatProps += st->m_pPrecalc->GetNumProps();

		for (int j = 0; j < st->GetNumProps(); j++)
		{
			SendProp* sp = st->GetProp(j);

			if (sp->IsExcludeProp())
			{
				numExcludeProps++;
				continue; // no real sendprops
			}

			if (sp->IsInsideArray())
				continue;

			switch (sp->GetType())
			{
			case DPT_Int: numInts++; break;
			case DPT_Float: numFloats++; break;
			case DPT_Vector: numVecs++; break;
			case DPT_VectorXY: numVecXYs++; break;
			case DPT_String: numStrings++; break;
			case DPT_Array: numArrays++; break;
			case DPT_DataTable: numSubTables++; break;
			}
		}
	}

	Msg("Total Send Table stats\n");
	Msg("Send Tables   : %i\n", numTables);
	Msg("Send Props    : %i\n", numSendProps);
	Msg("Flat Props    : %i\n", numFlatProps);
	Msg("Int Props     : %i\n", numInts);
	Msg("Float Props   : %i\n", numFloats);
	Msg("Vector Props  : %i\n", numVecs);
	Msg("VectorXY Props: %i\n", numVecXYs);
	Msg("String Props  : %i\n", numStrings);
	Msg("Array Props   : %i\n", numArrays);
	Msg("Table Props   : %i\n", numSubTables);
	Msg("Exclu Props   : %i\n", numExcludeProps);
}

void SendTableManager::SendTable_Term()
{
	// Term all the SendTables.
	int nTables = m_SendTableMap.GetNumStrings();
	for (int i = 0; i < nTables; i++)
		m_SendTableMap[i]->SendTable_TermTable();

	// Clear the list of SendTables.
	//g_SendTables.Purge();
	m_SendTableCRC = 0;
	m_Inited = false;
}


CRC32_t SendTable_CRCTable(CRC32_t& crc, SendTable* pTable)
{
	CRC32_ProcessBuffer(&crc, (void*)pTable->m_pNetTableName, Q_strlen(pTable->m_pNetTableName));

	int nProps = LittleLong(pTable->m_nProps);
	CRC32_ProcessBuffer(&crc, (void*)&nProps, sizeof(pTable->m_nProps));

	// Send each property.
	for (int iProp = 0; iProp < pTable->m_nProps; iProp++)
	{
		const SendProp* pProp = &pTable->m_pProps[iProp];

		int type = LittleLong(pProp->m_Type);
		CRC32_ProcessBuffer(&crc, (void*)&type, sizeof(type));
		CRC32_ProcessBuffer(&crc, (void*)pProp->GetName(), Q_strlen(pProp->GetName()));

		int flags = LittleLong(pProp->GetFlags());
		CRC32_ProcessBuffer(&crc, (void*)&flags, sizeof(flags));

		if (pProp->m_Type == DPT_DataTable)
		{
			CRC32_ProcessBuffer(&crc, (void*)pProp->GetDataTable()->m_pNetTableName, Q_strlen(pProp->GetDataTable()->m_pNetTableName));
		}
		else
		{
			if (pProp->IsExcludeProp())
			{
				CRC32_ProcessBuffer(&crc, (void*)pProp->GetExcludeDTName(), Q_strlen(pProp->GetExcludeDTName()));
			}
			else if (pProp->GetType() == DPT_Array)
			{
				int numelements = LittleLong(pProp->GetNumElements());
				CRC32_ProcessBuffer(&crc, (void*)&numelements, sizeof(numelements));
			}
			else
			{
				float lowvalue;
				LittleFloat(&lowvalue, &pProp->m_fLowValue);
				CRC32_ProcessBuffer(&crc, (void*)&lowvalue, sizeof(lowvalue));

				float highvalue;
				LittleFloat(&highvalue, &pProp->m_fHighValue);
				CRC32_ProcessBuffer(&crc, (void*)&highvalue, sizeof(highvalue));

				int	bits = LittleLong(pProp->m_nBits);
				CRC32_ProcessBuffer(&crc, (void*)&bits, sizeof(bits));
			}
		}
	}

	return crc;
}
//-----------------------------------------------------------------------------
// Purpose: Computes the crc for all sendtables for the data sent in the class/table definitions
// Output : CRC32_t
//-----------------------------------------------------------------------------
CRC32_t SendTableManager::SendTable_ComputeCRC()
{
	CRC32_t result;
	CRC32_Init(&result);

	// walk the tables and checksum them
	int nTables = m_SendTableMap.GetNumStrings();
	for (int i = 0; i < nTables; i++)
	{
		SendTable* st = m_SendTableMap[i];
		result = SendTable_CRCTable(result, st);
	}


	CRC32_Final(&result);

	return result;
}

SendTable* SendTableManager::SendTabe_GetTable(int index)
{
	return  m_SendTableMap[index];
}

int	SendTableManager::SendTable_GetNum()
{
	return m_SendTableMap.GetNumStrings();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : CRC32_t
//-----------------------------------------------------------------------------
CRC32_t SendTableManager::SendTable_GetCRC()
{
	return m_SendTableCRC;
}

SendTableManager* GetSendTableManager() {
	static SendTableManager s_SendTableManager;
	return &s_SendTableManager;
}


#endif
