#include <windows.h> 

HINSTANCE hInst;
HWND hwndMainWnd;
HHOOK g_keyboardHook = NULL;

// Объявление переменных для хранения текущих координат, размера и цвета объекта
bool isAnimating = false;
double currentX = 325;
double currentY = 275;
int objectSize = 100;
int objectWidth = objectSize;
// Объявление переменной для хранения выбранной формы объекта (0 - круг, 1 - квадрат, 2 - прямоугольник)
int selectedShape = 0;

COLORREF selectedColor = RGB(100, 100, 100);

// Функция обратного вызова для перехвата клавиатуры, используемая с функцией SetWindowsHookEx
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // Если nCode равен HC_ACTION, то wParam и lParam содержат информацию о сообщении клавиатуры
    if (nCode == HC_ACTION) 
    {
        // Если wParam равен WM_KEYDOWN, то произошло нажатие клавиши
        if (wParam == WM_KEYDOWN) 
        {
            // Получение указателя на структуру KBDLLHOOKSTRUCT, содержащую данные о нажатой клавише
            KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;

            // Если нажатая клавиша - 'S' (start,stop)
            if (kbStruct->vkCode == 0x53)
            {
                // Если переменная isAnimating равна true, то остановить анимацию и удалить таймер с идентификатором 1
                if (isAnimating)
                {
                    isAnimating = false;
                    KillTimer(hwndMainWnd, 1);
                }
                // Иначе, если переменная isAnimating равна false, то запустить анимацию и установить таймер с идентификатором 1 и интервалом 50 мс
                else 
                {
                    isAnimating = true;
                    SetTimer(hwndMainWnd, 1, 50, NULL);
                }
            }
        }
    }
    // Передать сообщение следующей функции в цепочке хуков
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

// Функция обработки сообщений окна
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{

    switch (msg) 
    {
    case WM_CREATE:
        hwndMainWnd = hwnd;

        // Установка перехвата клавиатуры
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

        // Создание кнопок для выбора формы и цвета объекта
        CreateWindow(L"BUTTON", L"Circle", WS_CHILD | WS_VISIBLE, 10, 90, 100, 30, hwnd, (HMENU)1, hInst, NULL);
        CreateWindow(L"BUTTON", L"Square", WS_CHILD | WS_VISIBLE, 120, 90, 100, 30, hwnd, (HMENU)2, hInst, NULL);
        CreateWindow(L"BUTTON", L"Rectangle", WS_CHILD | WS_VISIBLE, 230, 90, 100, 30, hwnd, (HMENU)9, hInst, NULL);

        CreateWindow(L"BUTTON", L"Yellow", WS_CHILD | WS_VISIBLE, 10, 50, 100, 30, hwnd, (HMENU)3, hInst, NULL);
        CreateWindow(L"BUTTON", L"Red", WS_CHILD | WS_VISIBLE, 120, 50, 100, 30, hwnd, (HMENU)4, hInst, NULL);
        CreateWindow(L"BUTTON", L"Blue", WS_CHILD | WS_VISIBLE, 230, 50, 100, 30, hwnd, (HMENU)5, hInst, NULL);
       
        // Создание кнопок для выбора размера объекта
        CreateWindow(L"BUTTON", L"Small", WS_CHILD | WS_VISIBLE, 10, 10, 100, 30, hwnd, (HMENU)6, hInst, NULL);
        CreateWindow(L"BUTTON", L"Average", WS_CHILD | WS_VISIBLE, 120, 10, 100, 30, hwnd, (HMENU)7, hInst, NULL);
        CreateWindow(L"BUTTON", L"Big", WS_CHILD | WS_VISIBLE, 230, 10, 100, 30, hwnd, (HMENU)8, hInst, NULL);

        SendMessage(GetDlgItem(hwnd, 1), BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(GetDlgItem(hwnd, 3), BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(GetDlgItem(hwnd, 6), BM_SETCHECK, BST_CHECKED, 0);

        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Получение размеров клиентской области окна
        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(255, 128, 128));

        FillRect(hdc, &rect, hBackgroundBrush);
        DeleteObject(hBackgroundBrush);

        // Рисование объекта в зависимости от выбранной формы и цвета
        HBRUSH hBrush = CreateSolidBrush(selectedColor);
        SelectObject(hdc, hBrush);

        if (selectedShape == 0)
        {
            Ellipse(hdc, (int)currentX, (int)currentY, (int)(currentX + objectWidth), (int)(currentY + objectWidth));
        }
        else if(selectedShape == 1)
        {
            Rectangle(hdc, (int)currentX, (int)currentY, (int)(currentX + objectWidth), (int)(currentY + objectWidth));
        }
        else
        {
            Rectangle(hdc, (int)currentX, (int)currentY, (int)(currentX + objectWidth + 125), (int)(currentY + objectWidth + 25));
        }

        DeleteObject(hBrush);
        EndPaint(hwnd, &ps);
    }
    break;

    case WM_TIMER:
        if (isAnimating) 
        {
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            ScreenToClient(hwnd, &cursorPos);

            currentX = cursorPos.x - objectWidth / 2;
            currentY = cursorPos.y - objectWidth / 2;

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int clientWidth = clientRect.right - clientRect.left;
            int clientHeight = clientRect.bottom - clientRect.top;

            if (currentX < 0) 
            {
                currentX = 0;
            }
            if (currentY < 0)
            {
                currentY = 0;
            }
            if (currentX + objectWidth > clientWidth)
            {
                currentX = clientWidth - objectWidth;
            }
            if (currentY + objectWidth > clientHeight)
            {
                currentY = clientHeight - objectWidth;
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case 1:
            selectedShape = 0; // Выбор формы объекта - круг
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 2:
            selectedShape = 1; // Выбор формы объекта - квадрат
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 3:
            selectedColor = RGB(255, 255, 0); // Выбор цвета объекта - желтый
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 4:
            selectedColor = RGB(255, 0, 0); // Выбор цвета объекта - красный
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 5:
            selectedColor = RGB(0, 0, 255); // Выбор цвета объекта - синий
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 6:
            objectSize = 50; // Выбор размера объекта - маленький
            objectWidth = objectSize;
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 7:
            objectSize = 125; // Выбор размера объекта - средний
            objectWidth = objectSize;
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 8:
            objectSize = 350; // Выбор размера объекта - большой
            objectWidth = objectSize;
            InvalidateRect(hwnd, NULL, TRUE);
            break; 
        case 9:
            selectedShape = 2; // Выбор формы объекта - прямоугольник
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        break;

    case WM_DESTROY:
        // Завершение работы приложения
        PostQuitMessage(0);

        // Отключение перехвата клавиатуры
        if (g_keyboardHook != NULL) 
        {
            UnhookWindowsHookEx(g_keyboardHook);
            g_keyboardHook = NULL;
        }
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Главная функция приложения
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    hInst = hInstance;

    // Регистрация класса окна
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"MyClass", NULL };
    RegisterClassEx(&wc);

    // Создание окна с заголовком "Графический редактор"
    HWND hwnd = CreateWindow(L"MyClass", L"Графический редактор", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    // Отображение окна на экране
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Цикл обработки сообщений окна
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}