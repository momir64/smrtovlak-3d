#include "Windows.h"
#include "Smrtovlak.h"

#ifdef _DEBUG
int main() {
	Smrtovlak smrtovlak;
	return smrtovlak.run();
}
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	Smrtovlak smrtovlak;
	return smrtovlak.run();
}
#endif