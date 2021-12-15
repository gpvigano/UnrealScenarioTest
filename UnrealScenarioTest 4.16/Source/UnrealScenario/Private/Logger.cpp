// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#include "Logger.h"
#include "Engine/GameEngine.h"

Logger::Logger()
{
	MessageTagIndex = 1000;

	// TODO: Log level configurable for final packaging/deployment

#if WITH_EDITOR
	CurrentLogLevel = DEBUG;
#endif
}


void Logger::PushMessage(const MessageInfo& messageInfo)
{
	MessageList.Add(messageInfo);
}


void Logger::FlushMessages()
{
	for (auto& msg : MessageList)
	{
		LogMessage(msg);
	}
	MessageList.Empty();
}


void Logger::LogMessage(const MessageInfo& messageInfo)
{
	const ELogLevel& logLevel = messageInfo.Level;
	const FString& message = messageInfo.message;
	const FString& category = messageInfo.category;
	const bool& toConsole = messageInfo.ToConsole;
	const bool& toScreen = messageInfo.ToScreen;
	const FString& tag = messageInfo.Tag;
	LogMessage(logLevel, message, category, toConsole, toScreen, tag);
}


void Logger::LogMessage(
	ELogLevel Level,
	const FString& Message,
	const FString& Category,
	bool ToConsole,
	bool ToScreen,
	const FString& KeyTag)
{
	int32 key = -1;

	if (!KeyTag.IsEmpty())
	{
		if (MessageTagMap.Contains(KeyTag))
		{
			key = MessageTagMap[KeyTag];
		}
		else
		{
			// automatically assign a number to the new tag
			key = MessageTagIndex++;
			MessageTagMap.Add(KeyTag, key);
		}
	}

	if (static_cast<int>(Level) < static_cast<int>(CurrentLogLevel))
	{
		return;
	}

	FColor logColor = FColor::White;
	float timeToDisplay = 5.0f;
	FString prefix;
	ELogVerbosity::Type logVerbosity;

	switch (Level)
	{
	case FATAL:
		logColor = FColor::Magenta;
		timeToDisplay = 0.0f;
		prefix = "Fatal! ";
		logVerbosity = ELogVerbosity::Fatal;
		break;
	case ERROR:
		logColor = FColor::Red;
		timeToDisplay = 30.0f;
		prefix = "Error! ";
		logVerbosity = ELogVerbosity::Error;
		break;
	case WARNING:
		logColor = FColor::Orange;
		timeToDisplay = 10.0f;
		prefix = "Warning! ";
		logVerbosity = ELogVerbosity::Warning;
		break;
	case LOG:
		logColor = FColor::Green;
		timeToDisplay = 5.0f;
		logVerbosity = ELogVerbosity::Display;
		break;
	case VERBOSE:
		logColor = FColor::Yellow;
		timeToDisplay = 2.5f;
		logVerbosity = ELogVerbosity::Verbose;
		break;
	default:
		logColor = FColor::White;
		timeToDisplay = 1.5f;
		logVerbosity = ELogVerbosity::VeryVerbose;
		break;
	}

	if (ToConsole && !bConsoleDisabled)
	{
		GLog->Log(logVerbosity, "[" + Category + "] " + Message);
	}

	if (ToScreen && GEngine && !bScreenDisabled)
	{
		FString logMessage = prefix + Message;
		if (!Category.IsEmpty())
		{
			logMessage = "[" + Category + "] " + logMessage;
		}
		GEngine->AddOnScreenDebugMessage(key, timeToDisplay, logColor, logMessage, false);
	}
}

