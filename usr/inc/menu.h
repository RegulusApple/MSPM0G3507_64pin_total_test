#ifndef MENU_H_
#define MENU_H_

#include <stdbool.h>
#include <stdint.h>

/*
 * Lightweight OLED + key menu state machine for field operation.
 *
 * Pages are registered with a title and a draw callback.  The menu keeps a
 * current page index; KEY_1 selects the previous page, KEY_2 the next page,
 * KEY_3 toggles edit mode (optional), KEY_5 exits to a root page (optional).
 * Call Menu_RunLoop() from the main loop at a moderate rate (e.g. every
 * 20-50 ms); it polls the keys and refreshes the OLED.
 */

#define MENU_MAX_PAGES      (8U)
#define MENU_TITLE_LEN      (12U)
#define MENU_EDIT_MARK      ('*')

typedef void (*MenuDrawFn)(uint8_t pageIndex, bool editing);

typedef struct {
    char title[MENU_TITLE_LEN];
    MenuDrawFn draw;
} MenuPage;

typedef enum {
    MENU_EVT_NONE = 0,
    MENU_EVT_PAGE_CHANGED,
    MENU_EVT_EDIT_TOGGLED,
    MENU_EVT_CONFIRM
} MenuEvent;

void Menu_Init(void);
bool Menu_RegisterPage(uint8_t index, const char *title, MenuDrawFn draw);
void Menu_SetPage(uint8_t index);
uint8_t Menu_GetPage(void);
bool Menu_IsEditing(void);
/* Call periodically; elapsedMs = ms since the previous call (same contract
 * as Key_Poll).  Polls keys and refreshes the OLED. */
void Menu_RunLoop(uint32_t elapsedMs);
MenuEvent Menu_GetEvent(void);

#endif /* MENU_H_ */
