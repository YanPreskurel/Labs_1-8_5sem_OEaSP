#include <windows.h>
#include <windowsx.h> // Для макросов GET_X_LPARAM и GET_Y_LPARAM
#include <cmath>


// Объявление имени класса окна
const wchar_t CLASS_NAME[] = L"Графический редактор";

// Объявление процедуры окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Объявление констант для идентификации фигур
#define SHAPE_NONE 0 // Нет фигуры
#define SHAPE_CIRCLE 1 // Круг
#define SHAPE_RECTANGLE 2 // Прямоугольник

// Объявление глобальных переменных для хранения состояния приложения
int shape = SHAPE_NONE; // Текущая выбранная фигура
bool drawing = false; // Флаг, указывающий, рисует ли пользователь фигуру
int startX = 0; // Начальная координата X для рисования фигуры
int startY = 0; // Начальная координата Y для рисования фигуры
int endX = 0; // Конечная координата X для рисования фигуры
int endY = 0; // Конечная координата Y для рисования фигуры

// Точка входа для приложения
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Регистрация класса окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc; // Функция обратного вызова для обработки сообщений окна
    wc.hInstance = hInstance; // Дескриптор экземпляра приложения
    wc.lpszClassName = CLASS_NAME; // Имя класса окна
    RegisterClass(&wc); // Регистрация класса в системе

    // Создание окна
    HWND hwnd = CreateWindowEx(
        0, // Дополнительные стили окна
        CLASS_NAME, // Имя класса окна
        L"Графический редактор", // Заголовок окна
        WS_OVERLAPPEDWINDOW, // Стиль окна
        // Размер и положение окна
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, // Родительское окно
        NULL, // Меню окна
        hInstance, // Дескриптор экземпляра приложения
        NULL // Дополнительные данные приложения
    );

    if (hwnd == NULL) // Проверка на ошибку создания окна
    {
        return 0;
    }

    // Показать окно на экране
    ShowWindow(hwnd, nCmdShow);

    // Запустить цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); // Транслировать виртуальные коды клавиш в символы
        DispatchMessage(&msg); // Передать сообщение процедуре окна
    }

    return 0;
}

// Процедура окна для обработки сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY: // Сообщение о закрытии окна
        PostQuitMessage(0); // Отправить сообщение WM_QUIT в очередь сообщений для завершения приложения
        return 0;

    case WM_PAINT: // Сообщение о перерисовке окна
    {
        PAINTSTRUCT ps; // Структура для хранения информации о перерисовке
        HDC hdc = BeginPaint(hwnd, &ps); // Получить контекст устройства для рисования в окне

        // Нарисовать фигуру, если пользователь рисует
        if (drawing)
        {
            // Выбрать кисть для рисования
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // Красная кисть
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush); // Сохранить старую кисть

            // Выбрать фигуру для рисования
            switch (shape)
            {
            case SHAPE_CIRCLE: // Круг
            {
                // Найти радиус и центр круга
                int radius = (int)sqrt(pow(endX - startX, 2) + pow(endY - startY, 2)); // Радиус по расстоянию между начальной и конечной точками
                int centerX = startX; // Центр по начальной точке
                int centerY = startY;

                // Нарисовать круг
                Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
            }
            break;

            case SHAPE_RECTANGLE: // Прямоугольник
            {
                // Нарисовать прямоугольник
                Rectangle(hdc, startX, startY, endX, endY);
            }
            break;
            }

            // Восстановить старую кисть и удалить созданную кисть
            SelectObject(hdc, hOldBrush);
            DeleteObject(hBrush);
        }

        EndPaint(hwnd, &ps); // Освободить контекст устройства и пометить перерисованную область как перерисованную
    }
    return 0;

    case WM_LBUTTONDOWN: // Сообщение о нажатии левой кнопки мыши
    {
        // Запомнить начальную координату для рисования фигуры
        startX = GET_X_LPARAM(lParam); // Получить координату X из параметра lParam
        startY = GET_Y_LPARAM(lParam); // Получить координату Y из параметра lParam

        // Установить флаг рисования в истину
        drawing = true;
    }
    return 0;

    case WM_MOUSEMOVE: // Сообщение о перемещении мыши
    {
        // Проверить, рисует ли пользователь фигуру
        if (drawing)
        {
            // Запомнить конечную координату для рисования фигуры
            endX = GET_X_LPARAM(lParam); // Получить координату X из параметра lParam
            endY = GET_Y_LPARAM(lParam); // Получить координату Y из параметра lParam

            // Перерисовать окно для отображения фигуры
            InvalidateRect(hwnd, NULL, TRUE); // Пометить всю клиентскую область окна как неперерисованную
        }
    }
    return 0;

    case WM_LBUTTONUP: // Сообщение об отпускании левой кнопки мыши
    {
        // Проверить, рисует ли пользователь фигуру
        if (drawing)
        {
            // Запомнить конечную координату для рисования фигуры
            endX = GET_X_LPARAM(lParam); // Получить координату X из параметра lParam
            endY = GET_Y_LPARAM(lParam); // Получить координату Y из параметра lParam

            // Сбросить флаг рисования в ложь
            drawing = false;

            // Перерисовать окно для отображения фигуры
            InvalidateRect(hwnd, NULL, TRUE); // Пометить всю клиентскую область окна как неперерисованную
        }
    }
    return 0;

    case WM_KEYDOWN: // Сообщение о нажатии клавиши клавиатуры
    {
        // Проверить, какая клавиша была
        switch (wParam)
        {
        case 'C': // Клавиша C
            // Выбрать круг как фигуру для рисования
            shape = SHAPE_CIRCLE;
            break;

        case 'R': // Клавиша R
            // Выбрать прямоугольник как фигуру для рисования
            shape = SHAPE_RECTANGLE;
            break;

        case VK_ESCAPE: // Клавиша Escape
            // Сбросить выбранную фигуру и очистить окно
            shape = SHAPE_NONE;
            InvalidateRect(hwnd, NULL, TRUE); // Пометить всю клиентскую область окна как неперерисованную
            break;
        }
    }
    return 0;
    }

    // Вызвать стандартную процедуру окна для обработки других сообщений
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
