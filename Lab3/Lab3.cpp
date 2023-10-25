#include <Windows.h>
#include <psapi.h>
#include <tchar.h>
#include <CommCtrl.h>

// Глобальные переменные
HWND hwndList;  // Дескриптор списка
HWND hwndButton; // Для кнопки "Update Process List"

// Объявление функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateProcessList(HWND hwndList);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = _T("MemoryMonitorApp");

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindow(_T("MemoryMonitorApp"), _T("Memory Monitor"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwndList = CreateWindowEx(0, _T("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT, 0, 0, 780, 520, hwnd, NULL, GetModuleHandle(NULL), NULL);
    if (!hwndList)
    {
        MessageBox(NULL, _T("Listbox Creation Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateProcessList(hwndList);

    MSG Msg;

    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        // Инициализация
        // Создаем кнопку "Update Process List" при создании окна
        hwndButton = CreateWindow(_T("BUTTON"), _T("Update Process List"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 520, 200, 30, hwnd, (HMENU)1001, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1001)
        { // ID кнопки "Update Process List" (1001)
            UpdateProcessList(hwndList);
        }
        break;

    case WM_TIMER:
        UpdateProcessList(hwndList);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void UpdateProcessList(HWND hwndList)
{
    // Очищаем список перед обновлением
    SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        cProcesses = cbNeeded / sizeof(DWORD);

        for (DWORD i = 0; i < cProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (hProcess)
            {
                TCHAR processName[MAX_PATH];
                if (GetProcessImageFileName(hProcess, processName, MAX_PATH) > 0)
                {
                    // Извлекаем только имя файла из пути
                    TCHAR* processNameOnly = _tcsrchr(processName, '\\');
                    if (processNameOnly)
                    {
                        processNameOnly++;
                    }
                    else
                    {
                        processNameOnly = processName;
                    }

                    PROCESS_MEMORY_COUNTERS pmc;
                    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)) && pmc.WorkingSetSize > 0)
                    {
                        // Формируем строку с информацией о процессе и его памяти
                        TCHAR info[MAX_PATH];
                        _stprintf_s(info, _T("PID: %d, Name: %s, Working Set: %I64d bytes\n"), aProcesses[i], processNameOnly, pmc.WorkingSetSize);

                        // Добавляем информацию в список
                        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)info);
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
}
