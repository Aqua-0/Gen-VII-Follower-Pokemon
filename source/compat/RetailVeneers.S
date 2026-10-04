.syntax unified
.arm
.section .text.retail_veneers, "ax", %progbits
.align 2

.extern g_RetailFunctionPointers
.set retail_function_index, 0

#define RETAIL_FUNCTION(id, provider, symbol, exportName) \
  .global symbol; \
  .type symbol, %function; \
symbol:; \
  ldr ip, =g_RetailFunctionPointers + (retail_function_index * 4); \
  ldr ip, [ip]; \
  bx ip; \
  .size symbol, . - symbol; \
  .set retail_function_index, retail_function_index + 1
#define RETAIL_FUNCTION_FAMILY(id, provider, symbol, regularExportName, ultraExportName) \
  RETAIL_FUNCTION(id, provider, symbol, ultraExportName)
#define RETAIL_DATA(id, provider, symbol, exportName)
#include "RetailFunctions.def"
#undef RETAIL_DATA
#undef RETAIL_FUNCTION_FAMILY
#undef RETAIL_FUNCTION

.ltorg

.section .note.GNU-stack, "", %progbits
