#ifndef H_SCHEDULER_STRATEGY_INCLUDED
#define H_SCHEDULER_STRATEGY_INCLUDED

namespace Scheduler
{
    enum StrategyType {
        // simple - first suitable
        SimpleType,
        // least loaded - least loaded
        LeastLoadedType,
        // deferred - accumulate in pending list items than try schedule it
        DeferredType
    };
};

#endif
