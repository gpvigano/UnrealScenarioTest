// UnrealScenarioTest - (c)2021 Giovanni Paolo Vigano' (CNR-STIIMA)

#pragma once

#include <string>
#include "CoreMinimal.h"

/**
Message logging utility.
*/
class UNREALSCENARIO_API Logger
{
public:

	Logger();

	/**
	Level of severity for message logging.
	*/
	enum ELogLevel
	{
		DEBUG = 0, VERBOSE, LOG, WARNING, ERROR, FATAL
	};

	/**
	Message structure for logging.
	*/
	struct MessageInfo
	{
		ELogLevel Level;
		FString message;
		FString category;
		bool ToConsole = true;
		bool ToScreen = true;
		FString Tag;
		MessageInfo() {}
		MessageInfo(ELogLevel level, FString msg, FString cat,
			bool toConsole = true, bool toScreen = true, FString tag = TEXT(""))
			: Level(level), message(msg), category(cat),
			ToConsole(toConsole), ToScreen(toScreen), Tag(tag)
		{}
	};


	/**
	 Compose a log message on screen and/or console
	 @param MessageInfo Information about the message to be printed.
	*/
	void PushMessage(const MessageInfo& MessageInfo);

	/**
	Print all the messages "pushed" on the internal stack and empty the stack.
	*/
	void FlushMessages();

	/**
	 Print a log message on screen and console
	 @param MessageInfo Information about the message to be printed.
	*/
	void LogMessage(const MessageInfo& MessageInfo);

	/**
	 Print a log message on screen and/or console.
	 @param Level Level of severity (see ELogLevel).
	 @param Message Message to be printed.
	 @param Category Category of the message.
	 @param ToConsole Print the message on console.
	 @param ToScreen Print the message on screen.
	 @param KeyTag Optional unique key to prevent the same message from being added multiple times.
	*/
	void LogMessage(
		ELogLevel Level,
		const FString& Message,
		const FString& Category,
		bool ToConsole = true,
		bool ToScreen = true,
		const FString& KeyTag = "");

	/**
	Current minimum severity level for message logging.
	*/
	ELogLevel CurrentLogLevel = VERBOSE;

	/**
	Messages are never printed on console.
	*/
	bool bConsoleDisabled = false;

	/**
	Messages are never printed on screen.
	*/
	bool bScreenDisabled = false;

private:

	TArray<MessageInfo> MessageList;

	TMap<FString, int32> MessageTagMap;
	int32 MessageTagIndex;
};
