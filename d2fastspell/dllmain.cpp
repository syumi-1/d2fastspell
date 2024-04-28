// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "detours.h"
#pragma comment (lib,"detours.lib")
#include <Psapi.h>


//导出函数 暂时先使用def
//#pragma comment(linker, "/EXPORT:_Init=dllmain._Init,@1")


extern "C" int _declspec(dllexport) __stdcall Init();
extern "C" int _declspec(dllexport) __stdcall End();


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

// 原始函数指针类型
extern "C" typedef int(__fastcall* sub_changeSpell)(int, DWORD*, int, int);

// 获取要hook的函数的地址（假设为0x54BE70）
//sub_54BE70_t targetFunction = (sub_54BE70_t)0x54BE70;
sub_changeSpell targetFunction = nullptr;


char toHexByte(BYTE byte)
{
	byte &= 0x0F;

	if (byte < 10)
	{
		return '0' + byte;
	}
	else
	{
		return 'A' + (byte - 10);
	}
}

// 根据特征码找函数地址
DWORD FindFunctionAddress(const char* pattern, size_t pattern_len)
{
	DWORD startAddress = 0x400000;
	DWORD endAddress = 0x700000;
	const BYTE* scanStart = reinterpret_cast<const BYTE*>(startAddress);
	const BYTE* scanEnd = reinterpret_cast<const BYTE*>(endAddress) - pattern_len;

	bool universal = false;
	for (size_t k = 0; k < pattern_len; k++) {
		if (pattern[k] == 0x00) {
			universal = true;
			break;
		}
	}
	if (universal) {
		for (const BYTE* p = scanStart; p < scanEnd; ++p)
		{
			bool found = true;
			for (size_t i = 0; i < sizeof(pattern); ++i)
			{

				if (pattern[i] != 0 && pattern[i] != p[i]) {
					found = false;
					break;
				}
			}
			if (found)
			{
				return reinterpret_cast<DWORD>(p);
			}
		}
	}
	else {
		for (const BYTE* p = scanStart; p < scanEnd; ++p)
		{
			if (memcmp(p, pattern, pattern_len) == 0)
			{
				return reinterpret_cast<DWORD>(p);
			}
		}

	}

	return 0;
}


int castSpell()
{
	HWND hWnd = GetForegroundWindow();  // 获取当前窗口句柄
	POINT cursorPos;
	GetCursorPos(&cursorPos);  // 获取当前鼠标位置
	// 将屏幕坐标转换为目标窗口的客户区坐标
	ScreenToClient(hWnd, &cursorPos);
	// 发送鼠标右键按下消息
	PostMessage(hWnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(cursorPos.x, cursorPos.y));
	// 发送鼠标右键释放消息
	PostMessage(hWnd, WM_RBUTTONUP, 0, MAKELPARAM(cursorPos.x, cursorPos.y));
	return 0;
}

// 用于hook函数的函数
int __fastcall hooked_sub_changeSpell(int a1, DWORD* a2, int a3, int a4)
{
	// 在这里执行您的钩子逻辑
	// ...

	int result = targetFunction(a1, a2, a3, a4);
	castSpell();

	return result;
}

extern "C" int _declspec(dllexport) __stdcall Init() {
	OutputDebugString(TEXT("D2fastspell:Start init."));

	const char pattern[] = { 0x55 ,0x8B ,0xEC ,0x51 ,0x83, 0x7D, 0x0C, 0x09, 0x8B };

	// 查找函数地址
	DWORD sub_changeSpell_address = FindFunctionAddress(pattern, sizeof(pattern));

	if (sub_changeSpell_address != 0)
	{
		OutputDebugString(TEXT("D2fastspell:Find Func."));
		targetFunction = reinterpret_cast<sub_changeSpell>(sub_changeSpell_address);
	}
	else
	{
		OutputDebugString(TEXT("D2fastspell:Can't Find Func Address."));
	}

	// 初始化Detours库
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	// 使用Detours hook函数
	DetourAttach(&(PVOID&)targetFunction, hooked_sub_changeSpell);

	// 提交Detours事务
	DetourTransactionCommit();

	// 程序继续执行...

	return 0;
}

extern "C" int _declspec(dllexport) __stdcall End() {
	// 卸载Detours hook
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(&(PVOID&)targetFunction, hooked_sub_changeSpell);
	DetourTransactionCommit();
	OutputDebugString(TEXT("D2fastspell:Finish end."));
	return 0;
}