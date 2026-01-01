#include "chat.h"

#include <string.h>

void chat_init(ChatLog *log) {
    if (!log) {
        return;
    }
    memset(log, 0, sizeof(*log));
}

void chat_add(ChatLog *log, ChatSpeaker speaker, const char *text) {
    if (!log || !text) {
        return;
    }

    if (log->count >= CHAT_MAX_MESSAGES) {
        for (size_t i = 1; i < CHAT_MAX_MESSAGES; ++i) {
            log->entries[i - 1] = log->entries[i];
        }
        log->count = CHAT_MAX_MESSAGES - 1;
    }

    ChatEntry *entry = &log->entries[log->count++];
    entry->speaker = speaker;

    size_t inputLen = strnlen(text, CHAT_MESSAGE_LENGTH - 1);
    memcpy(entry->message, text, inputLen);
    entry->message[inputLen] = '\0';
}

const char *chat_speaker_label(ChatSpeaker speaker) {
    switch (speaker) {
        case CHAT_SPEAKER_WHITE:
            return "WHITE";
        case CHAT_SPEAKER_BLACK:
            return "BLACK";
        case CHAT_SPEAKER_SYSTEM:
        default:
            return "SYSTEM";
    }
}
