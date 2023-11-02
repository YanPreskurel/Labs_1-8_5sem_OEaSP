#include <iostream>
#include <windows.h>

CRITICAL_SECTION cs; // Критическая секция для взаимного исключения

void worker(int id)
{
    for (int i = 0; i < 5; i++)
    {
        EnterCriticalSection(&cs); // Захватываю критическую секцию

        std::cout << "Thread " << id << " performs work " << i << std::endl; // Критическая секция

        LeaveCriticalSection(&cs); // Освобождаю критическую секцию
    }
    std::cout << "\n";
}

int main()
{
    const int num_threads = 3;
    HANDLE threads[num_threads];

    InitializeCriticalSection(&cs); // Инициализация критической секции

    for (int i = 0; i < num_threads; i++)
    {
        threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)worker, (LPVOID)i, 0, NULL); // Создаю потоки
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE); // Жду завершения всех потоков

    for (int i = 0; i < num_threads; i++)
    {
        CloseHandle(threads[i]); // Закрываю дескрипторы потоков
    }

    DeleteCriticalSection(&cs); // Удаляю критическую секцию

    return 0;
}
