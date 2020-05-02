#ifndef H_SCHEDULER_STRATEGY_INCLUDED
#define H_SCHEDULER_STRATEGY_INCLUDED

namespace Scheduler
{
    enum StrategyType {
        // simple - first suitable
        SimpleType,
        // least loaded - least loaded
        LeastLoadedType,
        // statistic - schedule by statistic and history
        StatisticType,
        // deferred - accumulate in pending list items than try schedule it
        DeferredType
    };
}

#endif
