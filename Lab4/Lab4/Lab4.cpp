#include <Windows.h>
#include <psapi.h>
#include <tchar.h>
#include <CommCtrl.h>

// Глобальные переменные
HWND hwndList;  // Дескриптор списка
HWND hwndButton; // Для кнопки "Update Process List"
HWND hwndButtonPause;
HWND hwndButtonResume;
HWND hwndButtonTerminate;

// Объявление функций
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateProcessList(HWND hwndList);
void PauseProcess(DWORD processId);
void ResumeProcess(DWORD processId);
void TerminateProcess(DWORD processId);
void ShowMessage(LPCTSTR message, LPCTSTR title, UINT flags);

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
        ShowMessage(_T("Window Registration Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);

        return 0;
    }

    HWND hwnd = CreateWindow(_T("MemoryMonitorApp"), _T("My Window Memory"), WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hwnd)
    {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hwndList = CreateWindowEx(0, _T("LISTBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | LBS_NOTIFY | LBS_WANTKEYBOARDINPUT, 0, 0, 780, 520, hwnd, NULL, GetModuleHandle(NULL), NULL);
    if (!hwndList)
    {
        ShowMessage(_T("Window Registration Failed!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);

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
        hwndButton = CreateWindow(_T("BUTTON"), _T("Update"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 120, 520, 80, 30, hwnd, (HMENU)1001, NULL, NULL);
        hwndButtonPause = CreateWindow(_T("BUTTON"), _T("Pause"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 220, 520, 80, 30, hwnd, (HMENU)1002, NULL, NULL);
        hwndButtonResume = CreateWindow(_T("BUTTON"), _T("Resume"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 320, 520, 80, 30, hwnd, (HMENU)1003, NULL, NULL);
        hwndButtonTerminate = CreateWindow(_T("BUTTON"), _T("Terminate"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 420, 520, 80, 30, hwnd, (HMENU)1004, NULL, NULL);

        SetTimer(hwnd, 1, 5000, NULL); // Таймер с интервалом 5000 миллисекунд (5 секунд)

        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1001)
        { // ID кнопки "Update Process List" (1001)
            UpdateProcessList(hwndList);
        }
        else if (LOWORD(wParam) == 1002)
        { // ID кнопки "Pause" (1002)
            int selectedIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
            if (selectedIndex != LB_ERR) 
            {
                DWORD processId = 0;
                TCHAR info[MAX_PATH];

                SendMessage(hwndList, LB_GETTEXT, selectedIndex, (LPARAM)info);

                if (_stscanf_s(info, _T("PID: %d"), &processId) == 1)
                {
                    PauseProcess(processId);
                }   
            }
        }
        else if (LOWORD(wParam) == 1003)
        { // ID кнопки "Resume" (1003)
            int selectedIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
            if (selectedIndex != LB_ERR) 
            {
                DWORD processId = 0;
                TCHAR info[MAX_PATH];

                SendMessage(hwndList, LB_GETTEXT, selectedIndex, (LPARAM)info);

                if (_stscanf_s(info, _T("PID: %d"), &processId) == 1)
                {
                    ResumeProcess(processId);
                }
                        
            }
        }
        else if (LOWORD(wParam) == 1004)
        { // ID кнопки "Terminate" (1004)
            int selectedIndex = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
            if (selectedIndex != LB_ERR) 
            {
                DWORD processId = 0;
                TCHAR info[MAX_PATH];

                SendMessage(hwndList, LB_GETTEXT, selectedIndex, (LPARAM)info);

                if (_stscanf_s(info, _T("PID: %d"), &processId) == 1) 
                {
                    TerminateProcess(processId);
                }
            }
        }  
        break;


    case WM_TIMER:
        if (wParam == 1) {
            UpdateProcessList(hwndList);
        }
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
// Функция для приостановки процесса
void PauseProcess(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (hProcess)
    {
        DWORD result = SuspendThread(hProcess);
        if (result != (DWORD)-1)
        {
            ShowMessage(_T("Process has been paused successfully."), _T("Success"), MB_ICONINFORMATION | MB_OK);
        }
        else
        {
            ShowMessage(_T("Failed to pause the process!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        }
        CloseHandle(hProcess);
    }
    else
    {
        ShowMessage(_T("Failed to open the process for pausing!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
    }
}

// Функция для возобновления процесса
void ResumeProcess(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (hProcess)
    {
        DWORD result = ResumeThread(hProcess);
        if (result != (DWORD)-1)
        {
            ShowMessage(_T("Process has been resumed successfully."), _T("Success"), MB_ICONINFORMATION | MB_OK);
        }
        else
        {
            ShowMessage(_T("Failed to resume the process!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        }
        CloseHandle(hProcess);
    }
    else
    {
        ShowMessage(_T("Failed to open the process for resuming!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
    }
}


// Функция для завершения процесса
void TerminateProcess(DWORD processId)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess)
    {
        if (TerminateProcess(hProcess, 0))
        {
            ShowMessage(_T("Process has been terminated successfully."), _T("Success"), MB_ICONINFORMATION | MB_OK);
        }
        else 
        {
            ShowMessage(_T("Failed to terminate the process!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
        }
        CloseHandle(hProcess);
    }
    else
    {
        ShowMessage(_T("Failed to open the process for terminating!"), _T("Error"), MB_ICONEXCLAMATION | MB_OK);
    }
}

void ShowMessage(LPCTSTR message, LPCTSTR title, UINT flags)
{
    MessageBox(NULL, message, title, flags);
}
