#include "RetailFunctions.hpp"

namespace Gen7Follower3gx
{

const RetailProvider g_RetailFunctionProviders[RetailFunctionCount] =
{
#define RETAIL_PROVIDER_STATIC RetailProvider::Static
#define RETAIL_PROVIDER_FIELDRO RetailProvider::FieldRo
#define RETAIL_FUNCTION(id, provider, symbol, exportName) RETAIL_PROVIDER_##provider,
#define RETAIL_FUNCTION_FAMILY(id, provider, symbol, regularExportName, ultraExportName) RETAIL_PROVIDER_##provider,
#define RETAIL_DATA(id, provider, symbol, exportName)
#include "RetailFunctions.def"
#undef RETAIL_DATA
#undef RETAIL_FUNCTION_FAMILY
#undef RETAIL_FUNCTION
#undef RETAIL_PROVIDER_FIELDRO
#undef RETAIL_PROVIDER_STATIC
};

const char* const g_RetailFunctionNames[RetailFunctionCount] =
{
#define RETAIL_FUNCTION(id, provider, symbol, exportName) exportName,
#define RETAIL_FUNCTION_FAMILY(id, provider, symbol, regularExportName, ultraExportName) regularExportName " | " ultraExportName,
#define RETAIL_DATA(id, provider, symbol, exportName)
#include "RetailFunctions.def"
#undef RETAIL_DATA
#undef RETAIL_FUNCTION_FAMILY
#undef RETAIL_FUNCTION
};

extern "C"
{
u32 g_RetailFunctionPointers[RetailFunctionCount] = {};
}

void ClearRetailFunctionPointers()
{
  for (u32 index = 0; index < RetailFunctionCount; ++index)
  {
    g_RetailFunctionPointers[index] = 0;
  }
}

} // namespace Gen7Follower3gx
