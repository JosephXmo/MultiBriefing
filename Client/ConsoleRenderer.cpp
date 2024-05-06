#include <Windows.h>

int nScreenWidth = 180, nScreenHeight = 80;

void RenderConsole() {
	// Create Screen Buffer
	wchar_t* ScreenCompleteBuffer = new wchar_t[nScreenWidth * nScreenHeight + 1];
	memset(ScreenCompleteBuffer, ' ', sizeof(ScreenCompleteBuffer));
	HANDLE hConsole = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		NULL, 
		CONSOLE_TEXTMODE_BUFFER, 
		NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	while (true) {
		CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
		GetConsoleScreenBufferInfo(hConsole, &ConsoleInfo);
		if (nScreenWidth != ConsoleInfo.dwSize.X || nScreenHeight != ConsoleInfo.dwSize.Y) {
			nScreenWidth = ConsoleInfo.dwSize.X;
			nScreenHeight = ConsoleInfo.dwSize.Y;
			delete[] ScreenCompleteBuffer;
			ScreenCompleteBuffer = new wchar_t[nScreenWidth * nScreenHeight + 1];
			memset(ScreenCompleteBuffer, ' ', sizeof(ScreenCompleteBuffer));
		}

		// TODO: Print anything you want

		ScreenCompleteBuffer[0] = '\u250c';
		ScreenCompleteBuffer[nScreenWidth - 1] = '\u2510';
		ScreenCompleteBuffer[(nScreenHeight - 1) * nScreenWidth] = '\u2514';
		ScreenCompleteBuffer[nScreenHeight * nScreenWidth - 1] = '\u2518';
		for (int i = 1; i < nScreenWidth - 2; i++) {
			ScreenCompleteBuffer[i] = '\u2500';
			ScreenCompleteBuffer[i + (nScreenHeight - 1) * nScreenWidth] = '\u2500';
		}

		ScreenCompleteBuffer[nScreenHeight * nScreenWidth] = '\0';
		WriteConsoleOutputCharacterW(
			hConsole,
			ScreenCompleteBuffer,
			nScreenHeight * nScreenWidth + 1,
			{ 0, 0 },		// Write from 0, 0 of the console, which stops console from rolling down
			&dwBytesWritten
		);
	}
}