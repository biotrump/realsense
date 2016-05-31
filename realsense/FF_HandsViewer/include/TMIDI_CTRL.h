#pragma once
HWND tmidi_init(void);
void tmidi_tempo(int pos = 180);
void tmidi_velocity(int pos = 180);
void tmidi_play(bool fclicked);
void tmidi_pause(bool fclicked);
void tmidi_stop(bool fclicked);