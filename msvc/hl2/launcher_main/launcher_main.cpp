// launcher_main.cpp : 鐎规矮绠熸惔鏃傛暏缁嬪绨惃鍕弳閸欙絿鍋ｉ妴?
//

//#include "framework.h"
//#include "launcher_main.h"
//
//#define MAX_LOADSTRING 100
//
//// 閸忋劌鐪崣姗€�?
//HINSTANCE hInst;                                // 瑜版挸澧犵€圭偘�?
//WCHAR szTitle[MAX_LOADSTRING];                  // 閺嶅洭顣介弽蹇旀瀮閺?
//WCHAR szWindowClass[MAX_LOADSTRING];            // 娑撹崵鐛ラ崣锝囪閸?
//
//// 濮濄倓鍞惍浣鼓侀崸妞捐厬閸栧懎鎯堥惃鍕毐閺佹壆娈戦崜宥呮倻婢圭増�?
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
//
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//                     _In_opt_ HINSTANCE hPrevInstance,
//                     _In_ LPWSTR    lpCmdLine,
//                     _In_ int       nCmdShow)
//{
//    UNREFERENCED_PARAMETER(hPrevInstance);
//    UNREFERENCED_PARAMETER(lpCmdLine);
//
//    // TODO: 閸︺劍顒濇径鍕杹缂冾喕鍞惍浣碘偓?
//
//    // 閸掓繂顫愰崠鏍у弿鐏炩偓鐎涙顑佹�?
//    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
//    LoadStringW(hInstance, IDC_LAUNCHERMAIN, szWindowClass, MAX_LOADSTRING);
//    MyRegisterClass(hInstance);
//
//    // 閹笛嗩攽鎼存梻鏁ょ粙瀣碍閸掓繂顫愰�?
//    if (!InitInstance (hInstance, nCmdShow))
//    {
//        return FALSE;
//    }
//
//    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAUNCHERMAIN));
//
//    MSG msg;
//
//    // 娑撶粯绉烽幁顖氭儕閻?
//    while (GetMessage(&msg, nullptr, 0, 0))
//    {
//        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
//        {
//            TranslateMessage(&msg);
//            DispatchMessage(&msg);
//        }
//    }
//
//    return (int) msg.wParam;
//}
//
//
//
////
////  閸戣姤鏆? MyRegisterClass()
////
////  閻╊喗鐖? 濞夈劌鍞界粣妤€褰涚猾姹団偓?
////
//ATOM MyRegisterClass(HINSTANCE hInstance)
//{
//    WNDCLASSEXW wcex;
//
//    wcex.cbSize = sizeof(WNDCLASSEX);
//
//    wcex.style          = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc    = WndProc;
//    wcex.cbClsExtra     = 0;
//    wcex.cbWndExtra     = 0;
//    wcex.hInstance      = hInstance;
//    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAUNCHERMAIN));
//    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
//    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
//    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAUNCHERMAIN);
//    wcex.lpszClassName  = szWindowClass;
//    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
//
//    return RegisterClassExW(&wcex);
//}
//
////
////   閸戣姤鏆? InitInstance(HINSTANCE, int)
////
////   閻╊喗鐖? 娣囨繂鐡ㄧ€圭偘绶ラ崣銉︾労楠炶泛鍨卞杞板瘜缁愭�?
////
////   濞夈劑鍣?
////
////        閸︺劍顒濋崙鑺ユ殶娑擃叏绱濋幋鎴滄粦閸︺劌鍙忕仦鈧崣姗€鍣烘稉顓濈箽鐎涙ê鐤勬笟瀣綖閺屽嫬�?
////        閸掓稑缂撻崪灞炬▔缁€杞板瘜缁嬪绨粣妤€褰涢�?
////
//BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//{
//   hInst = hInstance; // 鐏忓棗鐤勬笟瀣綖閺屽嫬鐡ㄩ崒銊ユ躬閸忋劌鐪崣姗€鍣烘�?
//
//   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
//
//   if (!hWnd)
//   {
//      return FALSE;
//   }
//
//   ShowWindow(hWnd, nCmdShow);
//   UpdateWindow(hWnd);
//
//   return TRUE;
//}
//
////
////  閸戣姤鏆? WndProc(HWND, UINT, WPARAM, LPARAM)
////
////  閻╊喗鐖? 婢跺嫮鎮婃稉鑽ょ崶閸欙絿娈戝☉鍫熶紖�?
////
////  WM_COMMAND  - 婢跺嫮鎮婃惔鏃傛暏缁嬪绨懣婊冨礋
////  WM_PAINT    - 缂佹ê鍩楁稉鑽ょ崶�?
////  WM_DESTROY  - 閸欐垿鈧線鈧偓閸戠儤绉烽幁顖氳嫙鏉╂柨娲?
////
////
//LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    switch (message)
//    {
//    case WM_COMMAND:
//        {
//            int wmId = LOWORD(wParam);
//            // 閸掑棙鐎介懣婊冨礋闁�?
//            switch (wmId)
//            {
//            case IDM_ABOUT:
//                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
//                break;
//            case IDM_EXIT:
//                DestroyWindow(hWnd);
//                break;
//            default:
//                return DefWindowProc(hWnd, message, wParam, lParam);
//            }
//        }
//        break;
//    case WM_PAINT:
//        {
//            PAINTSTRUCT ps;
//            HDC hdc = BeginPaint(hWnd, &ps);
//            // TODO: 閸︺劍顒濇径鍕潑閸旂姳濞囬�?hdc 閻ㄥ嫪鎹㈡担鏇犵帛閸ュ彞鍞�?..
//            EndPaint(hWnd, &ps);
//        }
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    default:
//        return DefWindowProc(hWnd, message, wParam, lParam);
//    }
//    return 0;
//}
//
//// 閳ユ粌鍙ф禍搴樷偓婵囶攱閻ㄥ嫭绉烽幁顖氼槱閻炲棛鈻兼惔蹇嬧�?
//INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
//{
//    UNREFERENCED_PARAMETER(lParam);
//    switch (message)
//    {
//    case WM_INITDIALOG:
//        return (INT_PTR)TRUE;
//
//    case WM_COMMAND:
//        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
//        {
//            EndDialog(hDlg, LOWORD(wParam));
//            return (INT_PTR)TRUE;
//        }
//        break;
//    }
//    return (INT_PTR)FALSE;
//}
