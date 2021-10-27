// PAP Unnamed Keyboard.cpp : Defines the entry point for the application.
//
// TODO : Send & Receive information through bluethooth over to raspberry PI.
// Extras
// TODO : Change Keyboard Layout based on active windows (aka discord special keyboard layout or overwatch etc)
// TODO : Get Password Automatically from phone 2 fac auth.

#include "framework.h"              //Library with the main includes necessary for the aplication
#include "PAP Unnamed Keyboard.h"   //Library related to the resources needed for the application

#define MAX_LOADSTRING 100          //Max string size for text

// Global Variables:
HINSTANCE       hInst;                              //Current instance
HANDLE          secInst;                            //Second Thread
WCHAR           szTitle[MAX_LOADSTRING];            //The title bar text
WCHAR           szWindowClass[MAX_LOADSTRING];      //The main window class name
NOTIFYICONDATA  NotifyIcon;                         //Create System Tray variable
std::string     AKL;                                //Active Keyboard Layout ID
HMENU           hMainMenu;                          //Main Window Menu handle
HBITMAP         AKLhBitmap = NULL;                  //Keyboard Image Bitmap
HWND            ExhWnd;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);           //Class Registration
BOOL                InitInstance(HINSTANCE, int);                   //Main Window Declaration
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);            //Windows Procedure (message handler)
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);              //About Dialog Procedure (message handler)
void                menuCreate(HWND, UINT, WPARAM, LPARAM);         //Create the main menu window
void                AKLToggle(HWND, UINT, WPARAM, LPARAM);          //Toggles the Active Keyboard Layout thread
DWORD WINAPI        secondThreadFunc(LPVOID lpParam);               //Creates a second Thread for the application
std::string         getKeyboardLayout();                            //Gets Keyboard Layout Dynamically as an integer in its Identifier Code
void                SysTrayIcoCreate(HWND, UINT, WPARAM, LPARAM);   //Creates the System icon tray
void                SysTrayIcoMenu(HWND, UINT, WPARAM, LPARAM);     //Create System tray menu for the icon
void                loadAKLImages(HWND, UINT, WPARAM, LPARAM);      //Load Images for AKL
void                onPaint(HWND, UINT, WPARAM, LPARAM);            //Paint the window when needed

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,             //Main Function that will start the application
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Create a new thread to execute the get keyboard layout procedure.
    if ((secInst = CreateThread(NULL, 0, secondThreadFunc, NULL, 0, NULL)) == NULL)
    {
        return 0;
    }

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);                 //Window Application Title
    LoadStringW(hInstance, IDC_PAPUNNAMEDKEYBOARD, szWindowClass, MAX_LOADSTRING);  //Window Class Title
    MyRegisterClass(hInstance);                                                     //Class Registration

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;   //Cancel the application if no instance can be created
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAPUNNAMEDKEYBOARD));  //Accelerator for keystrokes and commands

    MSG msg;    //Message Variable that will be used in the handler

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))                     //Get top message in the message queue
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) //Run message through accelerator and get converted
        {
            TranslateMessage(&msg);                             //Translate the message
            DispatchMessage(&msg);                              //Dispatch it to the message handler
        }
    }

    return (int) msg.wParam;                                    //Finalize the application
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);                                                   //Window Class Size

    wcex.style          = CS_HREDRAW | CS_VREDRAW;                                      //Window Style
    wcex.lpfnWndProc    = WndProc;                                                      //Windows Message Handler
    wcex.cbClsExtra     = 0;                                                            
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;                                                    //Main Window ID
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAPUNNAMEDKEYBOARD)); //Load the Icon for the TaskBar
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);                               //Load the Cursor Wanted for inside Main Window Operations
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW-2);                                     //Window Background Colour
    wcex.lpszMenuName   = 0;                  
    wcex.lpszClassName  = szWindowClass;                                                //Window Class Name
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));         //Load Application Preffered smaller Icon

    return RegisterClassExW(&wcex);                                                     //Return Class Registered for use
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,   //Create the Window
      CW_USEDEFAULT, CW_USEDEFAULT, 1200, 500, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;     //Abort if for some reason anything goes wrong
   }

   ShowWindow(hWnd, nCmdShow);  //Show the window so it can be drawn
   UpdateWindow(hWnd);          //Update the window after it is created and exhibited

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_CREATE       - Runs once the application starts
//  WM_COMMAND      - Process the application Main menu
//  WM_SYSTRAYICO   - Process the System tray icon PopUp menu
//  WM_PAINT        - Paint the main window
//  WM_DESTROY      - Post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    ExhWnd = hWnd;  //Save hWnd as global variable
    switch (message)    //Switch message that has been handled by winproc
    {
    case WM_CREATE:
        {   
            menuCreate(hWnd, message, wParam, lParam);          //Create the menu for the main window
            SysTrayIcoCreate(hWnd, message, wParam, lParam);    //Create the system tray icon
        }
        break;

    case WM_COMMAND:    
        {
            // Parse the menu selections:
            switch (LOWORD(wParam))
            {
            case IDM_AKL:               //Toggle AKL Option
                AKLToggle(hWnd, message, wParam, lParam);
                break;

            case IDM_ABOUT:             //Start About Box Once Clicked on the about section 
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);    //Destroy Main Window 
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_SYSTRAYICO:
        {
            // systray msg callback 
            switch (LOWORD(lParam))
            {
            case WM_RBUTTONDOWN:    //on right button click open sys tray menu
                SysTrayIcoMenu(hWnd, message, wParam, lParam);
                break;

            default:
                break;
            }
        }
        break;

    case WM_PAINT:
        {
            onPaint(hWnd, message, wParam, lParam);     //Paint the Window
        }
        break;
        
    case WM_DESTROY:
        {
            DeleteObject(AKLhBitmap);                   //Delete AKL loaded image
            Shell_NotifyIcon(NIM_DELETE, &NotifyIcon);  //Delete System tray icon
            PostQuitMessage(0);                         //Quit the application and exit the code.
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

//
//  FUNCTION: About(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the About Box.
//
//  WM_INITDIALOG   - Runs once the About Box starts
//  WM_COMMAND      - Process the About Box User Inputs
//
//
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


//
//  FUNCTION: menuCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//  PURPOSE: Creates the Main Window Menu
//
//
void menuCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //Set Main Variables
    hMainMenu = CreateMenu();
    HMENU hFile = CreateMenu();
    HMENU hOptions  = CreateMenu();
    HMENU hAbout = CreateMenu();
    
    //Main Window Menu options
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR) hFile, L"File");
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR) hOptions, L"Options");
    AppendMenu(hMainMenu, MF_POPUP, (UINT_PTR) hAbout, L"About");
    
    //Main Window Menu SubOptions for "File"
    AppendMenu(hFile, MF_STRING, IDM_EXIT, L"Exit");

    //Main Window Menu SubOptions for "Options"
    AppendMenu(hOptions, MF_STRING, IDM_AKL, L"Active Keyboard Layout");
    CheckMenuItem(hOptions, IDM_AKL, MF_CHECKED);

    //Main Window Menu SubOptions for "About"
    AppendMenu(hAbout, MF_STRING, IDM_ABOUT, L"Copyright");

    SetMenu(hWnd, hMainMenu);
}

//
//  FUNCTION: SysTrayIcoCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//  PURPOSE: Creates an Icon for the System Tray
//
//
void SysTrayIcoCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    NotifyIcon.hWnd = hWnd;                                                         //Connect it to the main app message handler
    NotifyIcon.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PAPUNNAMEDKEYBOARD));    //Load Icon for system tray
    NotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;                           //Load Flags that for functionality
    NotifyIcon.uCallbackMessage = WM_SYSTRAYICO;

    // This text will be shown as the icon's tooltip.
    StringCchCopy(NotifyIcon.szTip, ARRAYSIZE(NotifyIcon.szTip), L"PAP Unnamed Keyboard");

    //Add the Icon and show it
    Shell_NotifyIcon(NIM_ADD, &NotifyIcon);

    // Set the version
    Shell_NotifyIcon(NIM_SETVERSION, &NotifyIcon);

}

//
//  FUNCTION: SysTrayIcoMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//  PURPOSE: Creates the menu for the Icon in the system tray
//
//
void SysTrayIcoMenu(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    //Setting up coordinate variable
    POINT pt;
    GetCursorPos(&pt);  //function to figure out the coordinates of the mouse

    //Setting up PopUp menu variables
    HMENU hIcoPopMenu = CreatePopupMenu();
    HMENU hIPMFile = CreateMenu();
    HMENU hIPMptions = CreateMenu();

    //System Tray Icon PopUp Menu options
    AppendMenu(hIcoPopMenu, MF_POPUP, (UINT_PTR)hIPMFile, L"File");
    AppendMenu(hIcoPopMenu, MF_POPUP, (UINT_PTR)hIPMptions, L"Options");

    //System Tray Icon PopUp Menu SubOptions for "File"
    AppendMenu(hIPMFile, MF_STRING, IDM_EXIT, L"Exit");

    //System Tray Icon PopUp Menu SubOptions for "Options"
    AppendMenu(hIPMptions, MF_STRING, IDM_AKL, L"Active Keyboard Layout");

    //Check or uncheck the AKL option based on main menu option
    HMENU hMWMOptions = GetSubMenu(hMainMenu, 1);
    UINT state = GetMenuState(hMWMOptions, IDM_AKL, MF_BYCOMMAND);
    if (state == MF_CHECKED)
    {
        CheckMenuItem(hIPMptions, IDM_AKL, MF_CHECKED);
    }
    else
    {
        CheckMenuItem(hIPMptions, IDM_AKL, MF_UNCHECKED);
    }

    //Initialize the menu on mouse XY coordinates
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hIcoPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL);   //Track mouse coordinates and expose popup menu
}

//
//  FUNCTION: AKLToggle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//  PURPOSE: Toggles the Active Keyboard layout capture
//
//
void AKLToggle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //Set Main Variables
    HMENU   hMWMOptions = GetSubMenu(hMainMenu, 1);
    UINT    state       = GetMenuState(hMWMOptions, IDM_AKL, MF_BYCOMMAND);

    if (state == MF_CHECKED)                                    
    {
        SuspendThread(secInst);                             //Suspend AKL Thread
        CheckMenuItem(hMWMOptions, IDM_AKL, MF_UNCHECKED);  //Uncheck AKL Menu Option
        AKL = "";                                           //Set AKL to ""
        loadAKLImages(hWnd, message, wParam, lParam);       //Load "" Disabled Keyboard image
    }
    else
    {
        ResumeThread(secInst);                              //Resume AKL Thread
        CheckMenuItem(hMWMOptions, IDM_AKL, MF_CHECKED);    //Check AKL Menu Option
        AKL = getKeyboardLayout();                          //Set variable to Active Keyboard Layout
        loadAKLImages(hWnd, message, wParam, lParam);       //Load old keyboard image (gets updated instantly to the current AKL)
    }
}

//
//  PURPOSE: Run Second Thread of the application.
//
//
DWORD WINAPI secondThreadFunc(LPVOID lpParam)
{
    std::string pastAKL = "0";  //Variable to hold AKL past state (help detect change and save computer resources)

    // run Thread Loop
    while (true)
    {
        //Collect AKL info
        AKL = getKeyboardLayout();

        //Only load image when change is detected (lightweight)
        if (AKL != pastAKL)
        {
            pastAKL = AKL;                              //Save AKL state
            loadAKLImages(ExhWnd, NULL, NULL, NULL);    //Load new Keyboard layout for updated AKL
        }
    }
    return 0;
}

//
//  FUNCTION: getKeyboardLayout()
//
//  PURPOSE: Gets the information of the keyboard layout currently in use for the operating system
//
//
std::string getKeyboardLayout() 
{
    //Set main variables
    std::stringstream temp;
    int keyboardDec;
    std::string keyboardHex;

    //Get AKL information
    keyboardDec = HIWORD(
        GetKeyboardLayout(                          //Collect AKL from process ID of currently active window
            GetWindowThreadProcessId(               //Collect Process ID
                GetForegroundWindow(), NULL)));     //Window currently active

    temp << std::hex << keyboardDec << std::endl;   //Transform decimal into Hexadecimal   
    keyboardHex = temp.str();                       //Transform into String

    return keyboardHex;                             //Send AKL info back as String
}

//
//  FUNCTION: loadAKLImages()
//
//  PURPOSE: Loads the images necessary for working
//
//
void loadAKLImages(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if      (AKL == "")         //Load Disabled Keyboard Image
    {
        AKLhBitmap = (HBITMAP)LoadImage(hInst, L"KeyboardLayouts/ISO_Keyboard_Layout_Deactivated.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    else if (AKL == "816\n")    //Load PT Keyboard Image
    {
        AKLhBitmap = (HBITMAP)LoadImage(hInst, L"KeyboardLayouts/ISO_Keyboard_Layout_816.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    else if (AKL == "809\n")    //Load GB Keyboard Image
    {
        AKLhBitmap = (HBITMAP)LoadImage(hInst, L"KeyboardLayouts/ISO_Keyboard_Layout_809.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    else if (AKL == "f020\n")   //Load CMS Keyboard Image
    {
        AKLhBitmap = (HBITMAP)LoadImage(hInst, L"KeyboardLayouts/ISO_Keyboard_Layout_f020.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }

    if (AKLhBitmap == NULL)
    {
        MessageBox(NULL, L"Load Image Failed", L"Error", MB_OK);    //Show message box in case Loading fails
    };

    RedrawWindow(ExhWnd, NULL, NULL, RDW_INVALIDATE); //Update the image
}

//
//  FUNCTION: onPaint()
//
//  PURPOSE: Paints the Main window as neeeded.
//
//
void onPaint(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT     ps;
    HDC             hdc;
    BITMAP          bitmap;
    HDC             hdcMem;
    HGDIOBJ         oldBitmap;

    hdc = BeginPaint(hWnd, &ps);

    hdcMem = CreateCompatibleDC(hdc);
    oldBitmap = SelectObject(hdcMem, AKLhBitmap);

    GetObject(AKLhBitmap, sizeof(bitmap), &bitmap);
    int     posx = 70;
    int     posy = 45;
    int     sizex = 0;
    int     sizey = 0;
    BitBlt(hdc, posx, posy, bitmap.bmWidth, bitmap.bmHeight, hdcMem, sizex, sizey, SRCCOPY);

    SelectObject(hdcMem, oldBitmap);
    DeleteDC(hdcMem);

    EndPaint(hWnd, &ps);
}