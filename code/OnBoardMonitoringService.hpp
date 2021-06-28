#ifndef FDIR_DEMO_ONBOARDMONITORINGSERVICE_HPP
#define FDIR_DEMO_ONBOARDMONITORINGSERVICE_HPP

#include <cstdint>
#include "Service.hpp"
#include <etl/bitset.h>
#include <etl/variant.h>
#include <etl/vector.h>
#include <etl/pool.h>
#include "Parameters/SystemParameters.hpp"
#include "Logger.hpp"
#include "Services/EventReportService.hpp"

class OnBoardMonitoringService : public Service {
public:
    inline static const uint8_t ServiceType = 12;

    enum class CheckTypeID : uint8_t {
        ExpectedValue = 0,
        Limit = 1,
        Delta = 2
    };

    struct CheckValidityCondition {
        uint16_t parameterId;
        uint64_t mask;
        uint64_t expectedValue;

        bool check() {
            return (systemParameters.convertParameterValueToInt(parameterId) & mask) ==
                   (expectedValue & mask);
        }
    };

    struct CheckTransitionEntry {
        uint16_t pmonId;
        uint16_t monitoredParameterId;
        CheckTypeID checkType;
        float parameterValue;
        uint8_t previousStatus;
        uint8_t currentStatus;
        uint32_t transitionTime;
    };

    OnBoardMonitoringService() {
        serviceType = 12;
    }

    class ParameterMonitoringDefinitionBase {
    protected:
        uint16_t pmonId; ///< Parameter monitoring ID as defined in the standard
        uint16_t monitoredParameterId;
        uint32_t monitoringInterval;
        uint8_t repetitionNumber; ///< The number of checks needed to switch the parameter status

        std::optional<CheckValidityCondition> validityCondition;

        ParameterMonitoringDefinitionBase(uint16_t pmonId, uint16_t monitoredParameterId,
                                          uint32_t monitoringInterval,
                                          uint16_t repetitionNumber,
                                          const std::optional<CheckValidityCondition> &condition)
                : pmonId(pmonId), monitoredParameterId(monitoredParameterId),
                  monitoringInterval(monitoringInterval),
                  repetitionNumber(repetitionNumber), validityCondition(condition) {}

    public:
        virtual std::optional<CheckTransitionEntry> check(uint64_t timestamp) = 0;

        virtual void setEnabled(bool enabled) = 0;

        virtual void appendDefinitionReport(Message &message) = 0;

        uint16_t getPmonId() const {
            return pmonId;
        }
    };

    template<typename T, class CheckType>
    class ParameterMonitoringDefinition : public ParameterMonitoringDefinitionBase {
    public:
        ParameterMonitoringDefinition(uint16_t pmonId, uint16_t monitoredParameterId,
                                      uint32_t monitoringInterval,
                                      uint16_t repetitionNumber, CheckType checkParameters,
                                      const std::optional<CheckValidityCondition> &condition)
                : ParameterMonitoringDefinitionBase(pmonId, monitoredParameterId, monitoringInterval,
                                                    repetitionNumber, condition),
                  checkParameters(checkParameters) {}

        std::optional<CheckTransitionEntry> check(uint64_t timestamp) override;

        void setEnabled(bool enabled) override {
            this->enabled = enabled;
        }

        void appendDefinitionReport(Message &message) override;

    private:
        CheckType checkParameters;

        bool enabled = true;

        uint64_t lastCheckTimestamp = 0;
        uint8_t repetitionsCounted = 0;

        using CheckingStatus = typename CheckType::CheckingStatus;

        CheckingStatus previousStatus = CheckingStatus::Unchecked;
        CheckingStatus currentStatus = CheckingStatus::Unchecked;
        CheckingStatus futureStatus = CheckingStatus::Unchecked;
    };

    template<class T, class CheckType>
    void addParameterMonitoringDefinition(ParameterMonitoringDefinition<T, CheckType> &definition) {
        parameterMonitoringDefinitions.push_back(std::ref(definition));
    }

    void checkAll(uint64_t currentTimestamp) {
        if (not monitoringEnabled) {
            return;
        }

        for (auto &it: parameterMonitoringDefinitions) {
            auto transition = it.get().check(currentTimestamp);

            if (transition) {
                checkTransitionList.push_back(transition.value());
            }
        }

        if (checkTransitionList.full() || currentTimestamp > lastTransitionTimestamp + TransitionListPeriod) {
            lastTransitionTimestamp = currentTimestamp;
            checkTransitionReport();
        }
    }

    /**
     * TC[12,5]
     */
    void addParameterMonitoringDefinition(Message &message);

    /**
     * TC[12,6]
     */
    void deleteParameterMonitoringDefinition(Message &message);

    /**
     * TC[12,1]
     */
    void enableParameterMonitoringDefinitions(Message &message);

    /**
     * TC[12,2]
     */
    void disableParameterMonitoringDefinitions(Message &message);

    /**
     * TC[12,15]
     */
    void enableParameterMonitoringFunction(Message &message);

    /**
     * TC[12,16]
     */
    void disableParameterMonitoringFunction(Message &message);

    void reportParameterMonitoringDefinitions(Message &message);

    /**
     * TM[12,12]
     */
    void checkTransitionReport();

    void execute(Message &message);

private:
    inline static const int CheckTransitionListItems = 12;
    inline static const uint64_t TransitionListPeriod = 200;

    uint64_t lastTransitionTimestamp = 0;

    etl::vector<std::reference_wrapper<ParameterMonitoringDefinitionBase>, 20> parameterMonitoringDefinitions;

    bool monitoringEnabled = true;

    /**
     * @todo This variable should be atomic
     */
    etl::vector<CheckTransitionEntry, CheckTransitionListItems> checkTransitionList;

    etl::generic_pool<100, 8, 10> monitoringDefinitionPool;
};

template<typename T, class CheckType>
inline void OnBoardMonitoringService::ParameterMonitoringDefinition<T, CheckType>::appendDefinitionReport(
        Message &message) {
    message.append(uint16_t{pmonId});
    message.append(uint16_t{monitoredParameterId});
    if (validityCondition) {
        message.append(uint16_t{validityCondition->parameterId});
        message.append(uint64_t{validityCondition->mask});
        message.append(uint64_t{validityCondition->expectedValue});
    } else {
        message.append(uint16_t{0});
        message.append(uint64_t{0});
        message.append(uint64_t{0});
    }
    message.append(uint32_t{monitoringInterval});
    message.append(static_cast<uint8_t>(currentStatus));
    message.append(uint8_t{repetitionNumber});
    message.append(static_cast<uint8_t>(CheckType::ID));
    checkParameters.appendDefinitionReport(message);
}

template<typename T, class CheckType>
std::optional<OnBoardMonitoringService::CheckTransitionEntry>
OnBoardMonitoringService::ParameterMonitoringDefinition<T, CheckType>::check(uint64_t timestamp) {
    // TODO: Split the "monitoring interval" and "repetition count" capabilities of this function

    if (timestamp - lastCheckTimestamp < monitoringInterval) {
        return std::nullopt;
    }

    lastCheckTimestamp = timestamp;
    T parameterValue;
    if constexpr (std::is_same_v<T, float>) {
        parameterValue = systemParameters.convertParameterValueToFloat(monitoredParameterId);
    } else {
        parameterValue = systemParameters.getParameterValue<T>(monitoredParameterId);
    }

    CheckingStatus newStatus = checkParameters.check(parameterValue);

    bool transition = false;
    bool needsCheck = true;

    previousStatus = currentStatus;

    if (!enabled) {
        currentStatus = futureStatus = CheckType::CheckingStatus::Unchecked;
        repetitionsCounted = 0;
        transition = currentStatus != previousStatus;
        needsCheck = false;
    } else if (validityCondition && !validityCondition->check()) {
        currentStatus = futureStatus = CheckType::CheckingStatus::Invalid;
        repetitionsCounted = 0;
        transition = currentStatus != previousStatus;
        needsCheck = false;
    }

    if (needsCheck) {
        if (repetitionNumber <= 1) {
            currentStatus = futureStatus = newStatus;

            if (currentStatus != previousStatus) {
                transition = true;
            }
        } else {
            if (newStatus == futureStatus) {
                if (repetitionsCounted + 1 >= repetitionNumber) {
                    currentStatus = futureStatus;
                    repetitionsCounted = 0;

                    if (currentStatus != previousStatus) {
                        transition = true;
                    }
                } else {
                    ++repetitionsCounted;
                }
            } else {
                repetitionsCounted = 0;
                futureStatus = newStatus;
            }
        }
    }

    if (transition) {
        LOG_ERROR << "Monitoring status " << pmonId << " changed from " << static_cast<int>(previousStatus)
                  << " to " << static_cast<int>(currentStatus);
        checkParameters.generateEvent(currentStatus);

        return std::optional<OnBoardMonitoringService::CheckTransitionEntry>({
            pmonId,
            monitoredParameterId,
            CheckType::ID,
            static_cast<float>(parameterValue),
            static_cast<uint8_t>(previousStatus),
            static_cast<uint8_t>(currentStatus),
            timestamp
        });
    } else {
        return std::nullopt;
    }
}

#endif //FDIR_DEMO_ONBOARDMONITORINGSERVICE_HPP
