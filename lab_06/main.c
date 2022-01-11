#include <stdio.h>
#include <windows.h>

#define FALSE 0
#define TRUE 1

const DWORD wtr_sleep_tim = 50;
const DWORD rdr_sleep_tim = 30;

#define READERS_COUNT 5
#define WRITERS_COUNT 3
#define ITERATIONS 5

HANDLE mutex;
HANDLE can_read;
HANDLE can_write;

HANDLE writers[WRITERS_COUNT];
HANDLE readers[READERS_COUNT];

LONG waiting_writers = 0;
LONG waiting_readers = 0;
LONG active_readers = 0;

int active_writer = FALSE;

int value = 0;

void errExit(const char *msg);
void start_read(void);
void stop_read(void);
void stop_write(void);
void start_write(void);
DWORD WINAPI reader(LPVOID lpParams);
DWORD WINAPI writer(LPVOID lpParams);


int main()
{
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) errExit("Can't create mutex");

    can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (can_read == NULL) errExit("Can't create event can read");

    can_write = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (can_write == NULL) errExit("Can't create event can write");

    for (int i = 0; i < WRITERS_COUNT; i++)
    {
        writers[i] = CreateThread(NULL, 0,  writer, NULL, 0, NULL);
        if (writers[i] == NULL) errExit("Can't create writer");
    }

    for (int i = 0; i < READERS_COUNT; i++)
    {
        readers[i] = CreateThread(NULL, 0, reader, NULL, 0, NULL);
        if (readers[i] == NULL) errExit("Can't create reader");
    }

    WaitForMultipleObjects(WRITERS_COUNT, writers, TRUE, INFINITE);
    WaitForMultipleObjects(READERS_COUNT, readers, TRUE, INFINITE);

    CloseHandle(mutex);
    CloseHandle(can_read);
    CloseHandle(can_write);

    return EXIT_SUCCESS;
}

void start_read(void)
{
    InterlockedIncrement(&waiting_readers);

    WaitForSingleObject(mutex, INFINITE);

    if (active_writer || waiting_writers)
        WaitForSingleObject(can_read, INFINITE);

    InterlockedDecrement(&waiting_readers);
    InterlockedIncrement(&active_readers);
	
    SetEvent(can_read);

    ReleaseMutex(mutex);
}

void stop_read(void)
{
    InterlockedDecrement(&active_readers);

    if (waiting_readers == 0)
        SetEvent(can_write);
}

void start_write(void)
{
    InterlockedIncrement(&waiting_writers);
    if (active_writer || active_readers > 0)
        WaitForSingleObject(can_write, INFINITE);

    active_writer = TRUE;
    InterlockedDecrement(&waiting_writers);
    ResetEvent(can_write);
}

void stop_write(void)
{
    active_writer = FALSE;

    if (waiting_readers)
        SetEvent(can_read);
    else
        SetEvent(can_write);
}

DWORD WINAPI reader(LPVOID lpParams)
{
	while (value < WRITERS_COUNT * ITERATIONS)
	{
		start_read();
		printf("\tReader #%ld read value %d\n", GetCurrentThreadId(), value);
		stop_read();
		Sleep(rdr_sleep_tim);
	}

	return 0;
}

DWORD WINAPI writer(LPVOID lpParams)
{
	for (int i = 0; i < ITERATIONS; i++)
	{
		start_write();
		printf("Writer #%ld wrote value %ld\n", GetCurrentThreadId(), ++value);
		stop_write();
		Sleep(wtr_sleep_tim);
	}

	return 0;
}

void errExit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}