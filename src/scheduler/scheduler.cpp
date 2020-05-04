#include <scheduler.h>
#include <list>
#include <map>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <math.h>

namespace Scheduler
{
    Scheduler::Scheduler()
    {
        this->resource_pool = new std::list<Resource*>;
        this->bucket_pool = new std::list<Bucket*>;
        this->pending_items = new std::list<Item*>;
        this->scheduled_items = new std::list<Item*>;
        this->strategy = StrategyType::SimpleType;
    }

    void Scheduler::AddBucket(Bucket* bucket)
    {
        this->bucket_pool->push_back(bucket);
    }

    void Scheduler::Schedule()
    {
        if (this->strategy == StrategyType::DeferredType) {
            this->scheduleDeferred(nullptr);
        }
    }

    void Scheduler::ScheduleItem(Item* item)
    {
        bool scheduled = false;

        if (this->strategy == StrategyType::SimpleType) {
            // schedule first compatible bucket
            std::list<Bucket *>::iterator it;
            for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
                Bucket *bucket = *it;
                if (bucket->HasCapacityForItem(item)) {
                    this->bindBucketWidthItem(bucket, item);
                    scheduled = true;
                    break;
                }
            }
        } else if (this->strategy == StrategyType::LeastLoadedType) {
            // search least loaded bucket
            scheduled = this->scheduleLeastLoad(item);
        } else if (this->strategy == StrategyType::StatisticType) {
            // statistic
            scheduled = this->scheduleStatistic(item);
        } else if (this->strategy == StrategyType::DeferredType) {
            // deferred
            this->scheduleDeferred(item);
            return;
        }

        if (!scheduled) {
            this->pending_items->push_back(item);
        }
    }

    std::map<int, int*>* Scheduler::__GetDistributionItems()
    {
        std::map<int, int*>* distribution;
        distribution = new std::map<int, int*>;
        std::list<Bucket*>::iterator it;
        for (it = this->bucket_pool->begin(); it != this->bucket_pool->end(); ++it) {
            Bucket* bucket = *it;
            int* items;
            items = new int[bucket->GetItems()->size() + 1];
            memset(items, 0, sizeof(int) * (bucket->GetItems()->size() + 1));

            std::list<Item*>::iterator itElement;
            int i = 0;
            for (itElement = bucket->GetItems()->begin(); itElement != bucket->GetItems()->end(); ++itElement) {
                items[i] = (*itElement)->GetId();
                i++;
            }

            distribution->emplace(bucket->GetID(), items);
        }
        return distribution;
    }

    void Scheduler::SetStrategy(StrategyType strategy)
    {
        this->strategy = strategy;
    }

    void Scheduler::QueueItem(Item * item)
    {
        this->pending_items->push_back(item);
    }

    std::list<Bucket*>* Scheduler::GetBucketPool()
    {
        return this->bucket_pool;
    }

    void Scheduler::bindBucketWidthItem(Bucket* bucket, Item* item)
    {
        bucket->AddItem(item);
        item->SetBucket(bucket->GetID());
        this->scheduled_items->push_back(item);
    }

    bool Scheduler::scheduleLeastLoad(Item *item)
    {
        float bestScore = 2.0f;
        Bucket* bestBucket = nullptr;

        std::list<Bucket*>::iterator itBucket;

        for (itBucket = this->bucket_pool->begin(); itBucket != this->bucket_pool->end(); ++itBucket) {
            Bucket* bucket = (*itBucket);
            if (bucket->HasCapacityForItem(item)) {
                float score = bucket->GetFillRate();
                if (score < bestScore) {
                    bestScore = score;
                    bestBucket = bucket;
                    continue;
                }
            }
        }

        if (bestBucket == nullptr) {
            return false;
        }

        this->bindBucketWidthItem(bestBucket, item);
        return true;
    }

    bool Scheduler::scheduleStatistic(Item* item)
    {
        if (this->scheduled_items->size() < SCHEDULE_STATISTIC_ITEMS_COUNT) {
            return this->scheduleLeastLoad(item);
        }

        Item* avgItem = this->getAverageItem();

        std::list<Item*>* randomItems = this->getRandomListItems(SCHEDULE_STATISTIC_RANDOM_ITEMS_COUNT);
        randomItems->push_front(item);
        randomItems->push_back(avgItem);

        // build matrix

        std::map<int, FillFactorMap*>* matrix = this->buildFillFactorMatrix(item, randomItems);

        // analyze fill factor matrix

        int bestBucketID = this->analyzeFillFactorMatrix(matrix);

        if (bestBucketID == -1) {
            return false;
        }

        Bucket* bucket = this->getBucketByID(bestBucketID);
        if (bucket != nullptr) {
            this->bindBucketWidthItem(bucket, item);
            return true;
        }

        return false;
    }

    Item* Scheduler::getAverageItem()
    {
        std::map<int, int>* itemResources;
        itemResources = new std::map<int, int>;
        std::list<Item*>::iterator itItem;
        std::map<int, int>::iterator itResources;

        for (itItem = this->scheduled_items->begin(); itItem != this->scheduled_items->end(); ++itItem) {
            Item* item = (*itItem);
            for (itResources = item->GetResources()->begin(); itResources != item->GetResources()->end(); ++itResources) {
                if (itemResources->find(itResources->first) != itemResources->end()) {
                    itemResources->find(itResources->first)->second += itResources->second;
                } else {
                    itemResources->insert(std::pair<int, int>(itResources->first, itResources->second));
                }
            }
        }

        for (itResources = itemResources->begin(); itResources != itemResources->end(); ++itResources) {
            itResources->second = itResources->second / this->scheduled_items->size();
        }

        return new Item(0, itemResources);
    }

    std::list<Item*>* Scheduler::getRandomListItems(int count)
    {
        std::list<Item*>* items = new std::list<Item*>;
        std::list<int> itemIds;
        std::list<Item*>::iterator itItem;
        std::list<int>::iterator itItemId;

        for (itItem = this->scheduled_items->begin(); itItem != this->scheduled_items->end(); ++itItem) {
            itemIds.push_back((*itItem)->GetId());
        }

        std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()
        );

        std::srand(unsigned(ms.count()));

        for (int i = 0; i < count && i < this->scheduled_items->size(); ++i) {
            int index = std::rand() % itemIds.size();
            int itemID = 0;
            int j = 0;

            for (itItemId = itemIds.begin(); itItemId != itemIds.end(); ++itItemId) {
                if (j == index) {
                    itemID = *itItemId;
                    itemIds.remove(itemID);
                    break;
                }
                j++;
            }

            if (itemID == 0) {
                continue;
            }

            for (itItem = this->scheduled_items->begin(); itItem != this->scheduled_items->end(); ++itItem) {
                if ((*itItem)->GetId() == itemID) {
                    items->push_back(*itItem);
                    break;
                }
            }
        }

        return items;
    }

    float Scheduler::getFillFactor(Item* item, std::map<int, int>* leftResources)
    {
        float fillFactor = -1.0f;

        std::map<int, int>::iterator itRes;
        float currentFillFactor = -1.0f;

        for (itRes = item->GetResources()->begin(); itRes != item->GetResources()->end(); ++itRes) {
            currentFillFactor = 0.0f;
            if (leftResources->find(itRes->first) != leftResources->end()) {
                currentFillFactor = (float) leftResources->find(itRes->first)->second / (float) itRes->second;
            }

            if (fillFactor < 0.0f || currentFillFactor < fillFactor) {
                fillFactor = currentFillFactor;
            }
        }

        return fillFactor;
    }

    int Scheduler::analyzeFillFactorMatrix(std::map<int, FillFactorMap*>* matrix)
    {
        int bestBucketID = -1;

        if (matrix->empty()) {
            return bestBucketID;
        }

        std::map<int, FillFactorMap *>::iterator itMatrix;

        int countFilteredBuckets = matrix->size();
        float scoreFilter = -2.0f;

        while (countFilteredBuckets > 0) {
            countFilteredBuckets = 0;

            for (itMatrix = matrix->begin(); itMatrix != matrix->end(); ++itMatrix) {
                FillFactorMap::iterator itFillFactorMap;
                bool fillBucket = true;

                for (itFillFactorMap = itMatrix->second->begin(); itFillFactorMap != itMatrix->second->end(); ++itFillFactorMap) {
                    if (itFillFactorMap->second < scoreFilter) {
                        fillBucket = false;
                        break;
                    }
                }

                if (fillBucket) {
                    countFilteredBuckets++;
                }
            }

            if (countFilteredBuckets > 0) {
                scoreFilter += 1.0f;
            }
        }

        scoreFilter -= 1.0f;

        float bestFillFactor = 0.0f;

        for (itMatrix = matrix->begin(); itMatrix != matrix->end(); ++itMatrix) {
            float curFillFactor = 0.0f;
            FillFactorMap::iterator itFillFactorMap;
            bool fillBucket = true;

            for (itFillFactorMap = itMatrix->second->begin(); itFillFactorMap != itMatrix->second->end(); ++itFillFactorMap) {
                if (itFillFactorMap->second < scoreFilter) {
                    fillBucket = false;
                    break;
                }

                curFillFactor += itFillFactorMap->second;
            }

            if (fillBucket) {
                if (bestBucketID == -1 || bestFillFactor < curFillFactor) {
                    bestBucketID = itMatrix->first;
                    bestFillFactor = curFillFactor;
                }
            }
        }

        return bestBucketID;
    }

    Bucket* Scheduler::getBucketByID(int bucketID)
    {
        std::list<Bucket*>::iterator itBucket;
        for (itBucket = this->bucket_pool->begin(); itBucket != this->bucket_pool->end(); ++itBucket) {
            Bucket* bucket = *itBucket;
            if (bucket->GetID() == bucketID) {
                return bucket;
            }
        }
        return nullptr;
    }

    bool Scheduler::scheduleDeferred(Item* item)
    {
        if (item != nullptr) {
            this->pending_items->push_back(item);
        }

        if (this->pending_items->size() < MAX_PENDING_ITEMS_FOR_SCHEDULE && item != nullptr) {
            return false;
        }

        std::list<Item*> removeItems;
        std::list<Item*>::iterator itItem;
        for (itItem = this->pending_items->begin(); itItem != this->pending_items->end(); ++itItem) {
            // build matrix
            std::map<int, FillFactorMap*>* matrix = this->buildFillFactorMatrix(*itItem, this->pending_items);

            // analyze fill factor matrix
            int bestBucketID = this->analyzeFillFactorMatrix(matrix);

            if (bestBucketID == -1) {
                continue;
            }

            Bucket* bucket = this->getBucketByID(bestBucketID);
            if (bucket != nullptr) {
                this->bindBucketWidthItem(bucket, *itItem);
                removeItems.push_back(*itItem);
            }
        }

        for (itItem = removeItems.begin(); itItem != removeItems.end(); ++itItem) {
            this->pending_items->remove(*itItem);
        }

        return false;
    }

    std::map<int, FillFactorMap*>* Scheduler::buildFillFactorMatrix(Item* item, std::list<Item*>* items)
    {
        // build matrix
        auto* matrix = new std::map<int, FillFactorMap*>;

        std::list<Item*>::iterator itItem;

        std::list<Bucket*>::iterator itBucket, itBucketNested;
        for (itBucket = this->bucket_pool->begin(); itBucket != this->bucket_pool->end(); ++itBucket) {
            Bucket* bucket = *itBucket;

            if (!bucket->HasCapacityForItem(item)) {
                continue;
            }

            std::map<int, int> testResource;
            std::map<int, int>::iterator itResource;
            for (itResource = bucket->GetLeft()->begin(); itResource != bucket->GetLeft()->end(); ++itResource) {
                testResource.insert(std::pair<int, int>(itResource->first, itResource->second));
            }
            for (itResource = item->GetResources()->begin(); itResource != item->GetResources()->end(); ++itResource) {
                testResource.find(itResource->first)->second -= itResource->second;
            }
            auto* fillFactorMap = new FillFactorMap;

            for (itItem = items->begin(); itItem != items->end(); ++itItem) {
                Item* curItem = *itItem;
                float fillFactorItem = 0.0f;

                for (itBucketNested = this->bucket_pool->begin(); itBucketNested != this->bucket_pool->end(); ++itBucketNested) {
                    Bucket* bucketNested = *itBucketNested;
                    if (bucketNested->GetID() != bucket->GetID()) {
                        fillFactorItem += ceil(this->getFillFactor(curItem, bucketNested->GetLeft()));
                    } else {
                        fillFactorItem += ceil(this->getFillFactor(curItem, &testResource));
                    }
                }

                fillFactorMap->insert(std::pair<int, float>(curItem->GetId(), fillFactorItem));
            }

            matrix->insert(std::pair<int, FillFactorMap*>(bucket->GetID(), fillFactorMap));
        }

        return matrix;
    }
}
