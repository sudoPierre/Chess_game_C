#ifndef CHAT_H
#define CHAT_H

#include <stddef.h>
#include <stdbool.h>

#define CHAT_MAX_MESSAGES 32
#define CHAT_MESSAGE_LENGTH 128

typedef enum {
    CHAT_SPEAKER_WHITE = 0,
    CHAT_SPEAKER_BLACK,
    CHAT_SPEAKER_SYSTEM
} ChatSpeaker;

typedef struct {
    ChatSpeaker speaker;
    char message[CHAT_MESSAGE_LENGTH];
} ChatEntry;

typedef struct {
    ChatEntry entries[CHAT_MAX_MESSAGES];
    size_t count;
} ChatLog;

void chat_init(ChatLog *log);
void chat_add(ChatLog *log, ChatSpeaker speaker, const char *text);
const char *chat_speaker_label(ChatSpeaker speaker);

#endif // CHAT_H
