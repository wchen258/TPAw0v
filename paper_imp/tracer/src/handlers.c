#include "handlers.h"

void report(const char* format, ... ) {
	va_list args;
	va_start(args, format);
	xil_vprintf(format, args);
	va_end(args);
	xil_printf("\n\r");
}
