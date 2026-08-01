#include "menu.h"

#include <string.h>

#include "OLED.h"
#include "key.h"

static MenuPage gMenuPages[MENU_MAX_PAGES];
static bool gMenuPageUsed[MENU_MAX_PAGES];
static uint8_t gMenuCurrentPage;
static bool gMenuEditing;
static MenuEvent gMenuEvent;

void Menu_Init(void)
{
    memset(gMenuPages, 0, sizeof(gMenuPages));
    memset(gMenuPageUsed, 0, sizeof(gMenuPageUsed));
    gMenuCurrentPage = 0U;
    gMenuEditing = false;
    gMenuEvent = MENU_EVT_NONE;
}

bool Menu_RegisterPage(uint8_t index, const char *title, MenuDrawFn draw)
{
    size_t len;

    if ((index >= MENU_MAX_PAGES) || (title == NULL) || (draw == NULL)) {
        return false;
    }
    gMenuPages[index].draw = draw;
    len = strlen(title);
    if (len >= MENU_TITLE_LEN) {
        len = MENU_TITLE_LEN - 1U;
    }
    memcpy(gMenuPages[index].title, title, len);
    gMenuPages[index].title[len] = '\0';
    gMenuPageUsed[index] = true;
    return true;
}

void Menu_SetPage(uint8_t index)
{
    if ((index < MENU_MAX_PAGES) && gMenuPageUsed[index]) {
        gMenuCurrentPage = index;
        gMenuEvent = MENU_EVT_PAGE_CHANGED;
    }
}

uint8_t Menu_GetPage(void)
{
    return gMenuCurrentPage;
}

bool Menu_IsEditing(void)
{
    return gMenuEditing;
}

MenuEvent Menu_GetEvent(void)
{
    MenuEvent evt = gMenuEvent;
    gMenuEvent = MENU_EVT_NONE;
    return evt;
}

void Menu_RunLoop(uint32_t elapsedMs)
{
    uint8_t keyCode;
    KeyEvent evt;
    uint8_t page;

    evt = Key_Poll(elapsedMs, &keyCode);

    switch (evt) {
        case KEY_EVENT_PRESS:
            if (keyCode & KEY_1) {
                /* previous page */
                page = gMenuCurrentPage;
                do {
                    page = (page == 0U) ? (MENU_MAX_PAGES - 1U) : (page - 1U);
                } while ((!gMenuPageUsed[page]) && (page != gMenuCurrentPage));
                if (page != gMenuCurrentPage) {
                    Menu_SetPage(page);
                }
            }
            if (keyCode & KEY_2) {
                /* next page */
                page = gMenuCurrentPage;
                do {
                    page = (page == (MENU_MAX_PAGES - 1U)) ? 0U : (page + 1U);
                } while ((!gMenuPageUsed[page]) && (page != gMenuCurrentPage));
                if (page != gMenuCurrentPage) {
                    Menu_SetPage(page);
                }
            }
            if (keyCode & KEY_3) {
                gMenuEditing = !gMenuEditing;
                gMenuEvent = MENU_EVT_EDIT_TOGGLED;
            }
            if (keyCode & KEY_5) {
                gMenuEvent = MENU_EVT_CONFIRM;
            }
            break;
        default:
            break;
    }

    /* Refresh OLED: title on line 1, page body on lines 2..4. */
    OLED_Clear();
    OLED_ShowString(1, 1, gMenuPages[gMenuCurrentPage].title);
    if (gMenuEditing) {
        OLED_ShowChar(1,
            (uint8_t) (strlen(gMenuPages[gMenuCurrentPage].title) + 1U),
            MENU_EDIT_MARK);
    }
    if (gMenuPages[gMenuCurrentPage].draw != NULL) {
        gMenuPages[gMenuCurrentPage].draw(gMenuCurrentPage, gMenuEditing);
    }
}
