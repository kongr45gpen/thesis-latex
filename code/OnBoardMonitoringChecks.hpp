#ifndef ECSS_SERVICES_ONBOARDMONITORINGCHECKS_HPP
#define ECSS_SERVICES_ONBOARDMONITORINGCHECKS_HPP

#include <ServicePool.hpp>

template<typename T,
         typename = std::enable_if_t<std::is_enum_v<T>>>
inline constexpr std::underlying_type_t<T> underlyingValue(const T& value) {
    return static_cast<typename std::underlying_type<T>::type>(value);
}

template<typename T,
         typename = std::enable_if_t<!std::is_enum_v<T>>>
inline constexpr T underlyingValue(const T& value) {
    return value;
}

template<typename T>
struct ExpectedValueCheck {
    using type = T;

    static const inline OnBoardMonitoringService::CheckTypeID ID = OnBoardMonitoringService::CheckTypeID::ExpectedValue;

    enum class CheckingStatus : uint8_t {
		ExpectedValue = 0,
		Unchecked = 1,
		Invalid = 2,
		UnexpectedValue = 3
	};

	uint64_t mask; //TODO: Mask must have configurable size
	T expectedValue;
	uint16_t eventDefinitionID;

	CheckingStatus check(const T& parameter) {
		return ((mask & static_cast<uint64_t>(parameter))) == (mask & static_cast<uint64_t>(expectedValue)) ? CheckingStatus::ExpectedValue : CheckingStatus::UnexpectedValue;
	}

    void generateEvent(const CheckingStatus& status) {
        if (status == CheckingStatus::UnexpectedValue) {
            Services.eventReport.highSeverityAnomalyReport(static_cast<EventReportService::Event>(eventDefinitionID), "");
        }
    }

    void appendDefinitionReport(Message& message) {
	    message.append(uint64_t{mask});
	    message.append(underlyingValue(expectedValue));
	    message.append(eventDefinitionID);
	}
};

template<typename T>
struct LimitCheck {
    using type = T;

    static const inline OnBoardMonitoringService::CheckTypeID ID = OnBoardMonitoringService::CheckTypeID::Limit;

	enum class CheckingStatus : uint8_t {
		WithinLimits = 0,
		Unchecked = 1,
		Invalid = 2,
		BelowLowLimit = 4,
		AboveHighLimit = 5,
	};

	T lowLimit;
	uint16_t lowEventDefinitionID;
	T highLimit;
	uint16_t highEventDefinitionID;

	CheckingStatus check(const T& parameter) {
		if (parameter < lowLimit) {
			return CheckingStatus::BelowLowLimit;
		} else if (parameter > highLimit) {
			return CheckingStatus::AboveHighLimit;
		} else {
			return CheckingStatus::WithinLimits;
		}
	}

	void generateEvent(const CheckingStatus& status) {
		if (status == CheckingStatus::BelowLowLimit) {
			Services.eventReport.highSeverityAnomalyReport(static_cast<EventReportService::Event>(lowEventDefinitionID), "");
		} else if (status == CheckingStatus::AboveHighLimit) {
			Services.eventReport.highSeverityAnomalyReport(static_cast<EventReportService::Event>(highEventDefinitionID), "");
		}
	}

    void appendDefinitionReport(Message& message) {
        message.append(underlyingValue(lowLimit));
        message.append(lowEventDefinitionID);
        message.append(underlyingValue(highLimit));
        message.append(highEventDefinitionID);
    }
};

template<typename T>
struct DeltaCheck {
	enum class CheckingStatus : uint8_t {
		WithinThresholds = 0,
		Unchecked = 1,
		Invalid = 2,
		BelowLowThreshold = 6,
		AboveHighThreshold = 7,
	};

    static const inline OnBoardMonitoringService::CheckTypeID ID = OnBoardMonitoringService::CheckTypeID::Delta;

	T lowDeltaThreshold;
	uint16_t lowEventDefinitionID;
	T highDeltaThreshold;
	uint16_t highEventDefinitionID;
	uint16_t numberOfConsecutiveDeltaValues;

	CheckingStatus check() {
		return CheckingStatus::Invalid; // Not implemented
	}

    void generateEvent(const CheckingStatus& status) {
        if (status == CheckingStatus::BelowLowThreshold) {
            Services.eventReport.highSeverityAnomalyReport(static_cast<EventReportService::Event>(lowEventDefinitionID), "");
        } else if (status == CheckingStatus::AboveHighThreshold) {
            Services.eventReport.highSeverityAnomalyReport(static_cast<EventReportService::Event>(highEventDefinitionID), "");
        }
    }
};

#endif // ECSS_SERVICES_ONBOARDMONITORINGCHECKS_HPP
