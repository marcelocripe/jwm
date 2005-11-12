/***************************************************************************
 ***************************************************************************/

#include "jwm.h"
#include "desktop.h"
#include "main.h"
#include "client.h"
#include "hint.h"
#include "pager.h"
#include "taskbar.h"
#include "error.h"
#include "menu.h"

char **desktopNames = NULL;

/***************************************************************************
 ***************************************************************************/
void InitializeDesktops() {
}

/***************************************************************************
 ***************************************************************************/
void StartupDesktops() {

	int x;

	if(desktopNames == NULL) {
		desktopNames = Allocate(desktopCount * sizeof(char*));
		for(x = 0; x < desktopCount; x++) {
			desktopNames[x] = NULL;
		}
	}
	for(x = 0; x < desktopCount; x++) {
		if(desktopNames[x] == NULL) {
			desktopNames[x] = Allocate(4 * sizeof(char));
			snprintf(desktopNames[x], 4, "%d", x + 1);
		}
	}

	

}

/***************************************************************************
 ***************************************************************************/
void ShutdownDesktops() {
}

/***************************************************************************
 ***************************************************************************/
void DestroyDesktops() {

	int x;
	if(desktopNames) {
		for(x = 0; x < desktopCount; x++) {
			Release(desktopNames[x]);
		}
		Release(desktopNames);
		desktopNames = NULL;
	}

}

/***************************************************************************
 ***************************************************************************/
void NextDesktop() {
	ChangeDesktop((currentDesktop + 1) % desktopCount);
}

/***************************************************************************
 ***************************************************************************/
void PreviousDesktop() {
	if(currentDesktop > 0) {
		ChangeDesktop(currentDesktop - 1);
	} else {
		ChangeDesktop(desktopCount - 1);
	}
}

/***************************************************************************
 ***************************************************************************/
void ChangeDesktop(int desktop) {

	ClientNode *np;
	int x;

	if(desktop >= desktopCount || desktop < 0) {
		return;
	}

	if(currentDesktop == desktop && !initializing) {
		return;
	}

	for(x = 0; x < LAYER_COUNT; x++) {
		for(np = nodes[x]; np; np = np->next) {
			if(np->state.status & STAT_STICKY) {
				continue;
			}
			if(np->state.desktop == desktop) {
				ShowClient(np);
			} else if(np->state.desktop == currentDesktop) {
				HideClient(np);
			}
		}
	}

	currentDesktop = desktop;

	SetCardinalAtom(rootWindow, ATOM_NET_CURRENT_DESKTOP, currentDesktop);
	SetCardinalAtom(rootWindow, ATOM_WIN_WORKSPACE, currentDesktop);

	RestackClients();

	UpdatePager();
	UpdateTaskBar();

}

/***************************************************************************
 ***************************************************************************/
void CreateDesktopMenu(const char *name, unsigned int mask,
	MenuType *menu) {

	MenuType *submenu;
	MenuItemType *item;
	int x;

	item = Allocate(sizeof(MenuItemType));
	item->iconName = NULL;
	item->flags = MENU_ITEM_NORMAL;
	item->next = menu->items;
	menu->items = item;

	item->name = Allocate(strlen(name) + 1);
	strcpy(item->name, name);
	item->command = NULL;

	submenu = Allocate(sizeof(MenuType));
	item->submenu = submenu;
	submenu->itemHeight = 0;
	submenu->items = NULL;
	submenu->label = NULL;

	for(x = desktopCount - 1; x >= 0; x--) {

		item = Allocate(sizeof(MenuItemType));
		item->iconName = NULL;
		item->submenu = NULL;
		item->flags = MENU_ITEM_NORMAL;
		item->next = submenu->items;
		submenu->items = item;

		item->command = Allocate(8 * sizeof(char));
		strcpy(item->command, "#desk ");
		item->command[5] = '0' + x;

		item->name = Allocate(strlen(desktopNames[x]) + 3);
		if(mask & (1 << x)) {
			strcpy(item->name, "[");
			strcat(item->name, desktopNames[x]);
			strcat(item->name, "]");
		} else {
			strcpy(item->name, " ");
			strcat(item->name, desktopNames[x]);
			strcat(item->name, " ");
		}

	}

}

/***************************************************************************
 ***************************************************************************/
const char *GetDesktopName(int desktop) {

	Assert(desktop >= 0);
	Assert(desktop < desktopCount);

	return desktopNames[desktop];
}

/***************************************************************************
 ***************************************************************************/
void SetDesktopCount(const char *str) {

	Assert(str);

	desktopCount = atoi(str);
	if(desktopCount <= 0 || desktopCount > MAX_DESKTOP_COUNT) {
		Warning("invalid desktop count: \"%s\"", str);
		desktopCount = DEFAULT_DESKTOP_COUNT;
	}

}

/***************************************************************************
 ***************************************************************************/
void SetDesktopName(int desktop, const char *str) {

	int x;

	Assert(desktop >= 0);
	Assert(desktop < desktopCount);

	if(!desktopNames) {
		desktopNames = Allocate(desktopCount * sizeof(char*));
		for(x = 0; x < desktopCount; x++) {
			desktopNames[x] = NULL;
		}
	}

	Assert(desktopNames[desktop] == NULL);

	desktopNames[desktop] = Allocate(strlen(str) + 1);
	strcpy(desktopNames[desktop], str);

}


