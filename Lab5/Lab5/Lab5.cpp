#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;

// Регистрируем новую программу в реестре с установкой пути к инсталятору
bool SetProgramRegistry(const string& programName, const string& installerPath) 
{
    HKEY hParentKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_WRITE, &hParentKey) == ERROR_SUCCESS)
    {
        HKEY hAppKey;
        if (RegCreateKey(hParentKey, programName.c_str(), &hAppKey) == ERROR_SUCCESS) 
        {
            RegSetValueEx(hAppKey, "DisplayName", 0, REG_SZ, (const BYTE*)programName.c_str(), programName.size());

            RegSetValueEx(hAppKey, "InstallString", 0, REG_SZ, (const BYTE*)installerPath.c_str(), installerPath.size());

            RegCloseKey(hAppKey);
            RegCloseKey(hParentKey);
            return true;
        }
    }
    return false;
}

// Запускаем инсталятор
bool RunInstaller(const string& installerPath)
{
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.lpFile = installerPath.c_str();
    ShExecInfo.nShow = SW_SHOWNORMAL;

    if (ShellExecuteEx(&ShExecInfo) == TRUE) 
    {
        WaitForSingleObject(ShExecInfo.hProcess, INFINITE); // ожидает завершения процесса установщика
        CloseHandle(ShExecInfo.hProcess); // дескриптор закрывается
        return true;
    }
    return false;
}

// Устаналиваем путь к деинсталятору
bool SetUninstallRegistry(const string& programName, const string& uninstallerPath) 
{
    HKEY hParentKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_WRITE, &hParentKey) == ERROR_SUCCESS) 
    {
        HKEY hAppKey;
        if (RegCreateKey(hParentKey, programName.c_str(), &hAppKey) == ERROR_SUCCESS)
        {

            RegSetValueEx(hAppKey, "UninstallString", 0, REG_SZ, (const BYTE*)uninstallerPath.c_str(), uninstallerPath.size());

            RegCloseKey(hAppKey);
            RegCloseKey(hParentKey);

            return true;
        }
    }
    return false;
}

// Запускаем деинсталятор
bool RunUninstaller(const string& programName)
{
    HKEY hParentKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_READ, &hParentKey) == ERROR_SUCCESS)
    {
        HKEY hAppKey;
        if (RegOpenKeyEx(hParentKey, programName.c_str(), 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
        {
            DWORD dataValueSize = 1024;
            char uninstallerPath[1024];
            if (RegQueryValueEx(hAppKey, "UninstallString", 0, NULL, (LPBYTE)uninstallerPath, &dataValueSize) == ERROR_SUCCESS) 
            {
                SHELLEXECUTEINFO ShExecInfo = { 0 };
                ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
                ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; // Вернуть дескриптор процессора для дальнейшего использования
                ShExecInfo.lpFile = uninstallerPath;
                ShExecInfo.nShow = SW_SHOWNORMAL;

                if (ShellExecuteEx(&ShExecInfo) == TRUE) 
                {
                    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
                    CloseHandle(ShExecInfo.hProcess);
                    return true;
                }
                else 
                {
                    cout << "Failed to run the uninstaller." << endl;
                    return false;
                }
            }
            RegCloseKey(hAppKey);
        }
        RegCloseKey(hParentKey);
    }
    return false;
}

// Список подключей в ключе реестра
void ListInstalledPrograms()
{
    HKEY hParentKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_READ, &hParentKey) == ERROR_SUCCESS) 
    {
        DWORD subkeyIndex = 0;
        char subkeyName[255];

        while (RegEnumKey(hParentKey, subkeyIndex, subkeyName, sizeof(subkeyName)) == ERROR_SUCCESS) 
        {
            HKEY hAppKey;
            if (RegOpenKeyEx(hParentKey, subkeyName, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
            {
                cout << "Program: " << subkeyName << endl;
                RegCloseKey(hAppKey);
            }
            subkeyIndex++;
        }
        RegCloseKey(hParentKey);
    }
}

int main() 
{
    while (true)
    {
        int choice;
        string programName;
        string installerPath;
        string uninstallerPath;

        cout << "1. List installed programs\n2. Register new program and set InstallerPath\n3. Set UninstallerPath\n4. Install program\n5. Uninstall program\n6. Exit\nEnter your choice: ";
        cin >> choice;

        if (choice == 1) 
        {
            ListInstalledPrograms();
        }
        else if (choice == 2) 
        {
            cout << "Enter the program name: ";
            cin.ignore();
            getline(cin, programName);

            cout << "Enter the installer path: ";
            getline(cin, installerPath);

            if (SetProgramRegistry(programName, installerPath))
            {
                cout << "Program information added to the registry." << endl;
            }
            else 
            {
                cout << "Failed to add program information to the registry." << endl;
            }
        }
        else if (choice == 3) 
        {
            cout << "Enter the program name to set UninstallString:";
            cin.ignore();
            getline(cin, programName);

            cout << "Enter the uninstaller path: ";
            getline(cin, uninstallerPath);

            if (SetUninstallRegistry(programName, uninstallerPath)) 
            {
                cout << "Uninstall information added to the registry." << endl;
            }
            else 
            {
                cout << "Failed to add uninstall information to the registry." << endl;
            }
        }
        else if (choice == 4)
        {
            cout << "Enter the program name to install: ";
            cin.ignore();
            getline(cin, programName);

            HKEY hInstallKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_READ, &hInstallKey) == ERROR_SUCCESS) 
            {
                HKEY hAppKey;
                if (RegOpenKeyEx(hInstallKey, programName.c_str(), 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
                {
                    DWORD valueSize = 1024;
                    char installPath[1024];

                    if (RegQueryValueEx(hAppKey, "InstallString", 0, NULL, (LPBYTE)installPath, &valueSize) == ERROR_SUCCESS) 
                    {
                        if (RunInstaller(installPath)) 
                        {
                            cout << "Program installed successfully!" << endl;
                        }
                        else
                        {
                            cout << "Failed to run the installer." << endl;
                        }
                    }
                    RegCloseKey(hAppKey);
                }
                RegCloseKey(hInstallKey);
            }
        }
        else if (choice == 5) 
        {
            cout << "Enter the program name to uninstall: ";
            cin.ignore();
            getline(cin, programName);

            HKEY hUninstallKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Lab5", 0, KEY_READ, &hUninstallKey) == ERROR_SUCCESS) 
            {
                HKEY hAppKey;
                if (RegOpenKeyEx(hUninstallKey, programName.c_str(), 0, KEY_READ, &hAppKey) == ERROR_SUCCESS)
                {
                    DWORD valueSize = 1024;
                    char uninstallPath[1024];

                    if (RegQueryValueEx(hAppKey, "UninstallString", 0, NULL, (LPBYTE)uninstallPath, &valueSize) == ERROR_SUCCESS)
                    {
                        if (RunUninstaller(programName)) 
                        {
                            cout << "Program uninstalled successfully!" << endl;
                        }
                        else 
                        {
                            cout << "Failed to run the uninstaller." << endl;
                        }
                    }
                    RegCloseKey(hAppKey);
                }
                RegCloseKey(hUninstallKey);
            }
        }
        else if (choice == 6) 
        {
            break;
        }
    }
    return 0;
}
