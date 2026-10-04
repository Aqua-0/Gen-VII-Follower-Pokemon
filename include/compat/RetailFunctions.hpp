#pragma once

#include <3ds/types.h>

namespace Gen7Follower3gx
{

enum RetailFunctionId
{
#define RETAIL_FUNCTION(id, provider, symbol, exportName) RetailFunction_##id,
#define RETAIL_FUNCTION_FAMILY(id, provider, symbol, regularExportName, ultraExportName) RetailFunction_##id,
#define RETAIL_DATA(id, provider, symbol, exportName)
#include "RetailFunctions.def"
#undef RETAIL_DATA
#undef RETAIL_FUNCTION_FAMILY
#undef RETAIL_FUNCTION
  RetailFunctionCount,
};

enum class RetailProvider : u8
{
  Static,
  FieldRo,
};

extern const RetailProvider g_RetailFunctionProviders[RetailFunctionCount];
extern const char* const g_RetailFunctionNames[RetailFunctionCount];
extern "C" u32 g_RetailFunctionPointers[RetailFunctionCount];

void ClearRetailFunctionPointers();

} // namespace Gen7Follower3gx
