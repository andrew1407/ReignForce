#if (WITH_AUTOMATION_TESTS && WITH_EDITOR)

#include "Tests/TestUtils.h"


namespace TestUtils
{
    UWorld* GetGameWorld()
    {
        const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
        for (const auto& Context : WorldContexts)
            if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game) && Context.World())
                return Context.World();
        return nullptr;
    }
} // namespace TestUtils

#endif
