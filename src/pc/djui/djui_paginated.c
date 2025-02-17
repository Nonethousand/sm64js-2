#include "djui.h"

  ////////////////
 // properties //
////////////////

  ////////////
 // events //
////////////

static struct DjuiButton* sPrevButton = NULL;
static struct DjuiButton* sNextButton = NULL;
static struct DjuiText* sPageNumText = NULL;

static s32 djui_paginated_get_count(struct DjuiPaginated* paginated) {
    s32 count = 0;
    struct DjuiBaseChild* dbc = paginated->layout->base.child;
    while (dbc != NULL) {
        count++;
        dbc = dbc->next;
    }

    return count;
}

static void djui_paginated_prev(struct DjuiBase* base) {
    struct DjuiPaginated* paginated = (struct DjuiPaginated*)base->parent;
    paginated->startIndex -= paginated->showCount;

    djui_base_set_enabled(&sPrevButton->base, (paginated->startIndex > 0));
    djui_base_set_enabled(&sNextButton->base, true);

    char pageNumString[32] = { 0 };
    snprintf(pageNumString, 32, "%d/%d", paginated->startIndex / paginated->showCount + 1, djui_paginated_get_count(paginated) / paginated->showCount + 1);
    djui_text_set_text(sPageNumText, pageNumString);

    if (paginated->startIndex < 0) { paginated->startIndex = 0; }
}

static void djui_paginated_next(struct DjuiBase* base) {
    struct DjuiPaginated* paginated = (struct DjuiPaginated*)base->parent;
    paginated->startIndex += paginated->showCount;
    s32 count = djui_paginated_get_count(paginated);

    djui_base_set_enabled(&sNextButton->base, (paginated->startIndex < count - 8));
    djui_base_set_enabled(&sPrevButton->base, true);

    char pageNumString[32] = { 0 };
    snprintf(pageNumString, 32, "%d/%d", paginated->startIndex / paginated->showCount + 1, count / paginated->showCount + 1);
    djui_text_set_text(sPageNumText, pageNumString);

    if (paginated->startIndex >= count) { paginated->startIndex -= paginated->showCount; }
}

void djui_paginated_calculate_height(struct DjuiPaginated* paginated) {
    struct DjuiBaseChild* dbc = paginated->layout->base.child;

    f32 height = 0;
    s32 count = 0;

    while (dbc != NULL) {
        struct DjuiBase* cbase = dbc->base;
        if (count < paginated->showCount) {
            if (height != 0) {
                height += paginated->layout->margin.value;
            }
            height += cbase->height.value;
        }
        count++;
        dbc = dbc->next;
    }

    if (count <= paginated->showCount) {
        djui_base_set_visible(&paginated->prevButton->base, false);
        djui_base_set_visible(&paginated->nextButton->base, false);
    } else {
        djui_base_set_visible(&paginated->prevButton->base, true);
        djui_base_set_visible(&paginated->nextButton->base, true);
        height += paginated->layout->margin.value;
        height += paginated->nextButton->base.height.value;
    }

    djui_base_set_size(&paginated->base, paginated->base.width.value, height);

    char pageNumString[32] = { 0 };
    snprintf(pageNumString, 32, "%d/%d", paginated->startIndex / paginated->showCount + 1, count / paginated->showCount + 1);
    djui_text_set_text(sPageNumText, pageNumString);
}

bool djui_paginated_render(struct DjuiBase* base) {
    struct DjuiPaginated* paginated = (struct DjuiPaginated*)base;
    struct DjuiBaseChild* dbc = paginated->layout->base.child;

    s32 index = 0;
    s32 shown = 0;

    while (dbc != NULL) {
        struct DjuiBase* cbase = dbc->base;
        if (index < paginated->startIndex || shown >= paginated->showCount) {
            djui_base_set_visible(cbase, false);
        } else {
            djui_base_set_visible(cbase, true);
            shown++;
        }
        index++;
        dbc = dbc->next;
    }

    djui_rect_render(base);
    return true;
}

static void djui_paginated_destroy(struct DjuiBase* base) {
    struct DjuiPaginated* paginated = (struct DjuiPaginated*)base;
    free(paginated);
}

struct DjuiPaginated* djui_paginated_create(struct DjuiBase* parent, u32 showCount) {
    struct DjuiPaginated* paginated = calloc(1, sizeof(struct DjuiPaginated));
    paginated->showCount = showCount;
    struct DjuiBase* base = &paginated->base;

    float bodyHeight = 416;

    djui_base_init(parent, base, djui_paginated_render, djui_paginated_destroy);
    djui_base_set_size_type(base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(base, 1.0, bodyHeight);
    djui_base_set_color(base, 0, 64, 0, 0);
    djui_base_set_alignment(base, DJUI_HALIGN_CENTER, DJUI_VALIGN_TOP);

    {
        struct DjuiFlowLayout* layout = djui_flow_layout_create(base);
        djui_base_set_alignment(&layout->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_TOP);
        djui_base_set_location(&layout->base, 0, 0);
        djui_base_set_size_type(&layout->base, DJUI_SVT_RELATIVE, DJUI_SVT_ABSOLUTE);
        djui_base_set_size(&layout->base, 1.0f, bodyHeight);
        djui_base_set_color(&layout->base, 0, 0, 0, 0);
        djui_flow_layout_set_margin(layout, 16);
        djui_flow_layout_set_flow_direction(layout, DJUI_FLOW_DIR_DOWN);
        paginated->layout = layout;
    }

    sPrevButton = djui_button_create(&paginated->base, "<", DJUI_BUTTON_STYLE_NORMAL, djui_paginated_prev);
    djui_base_set_alignment(&sPrevButton->base, DJUI_HALIGN_LEFT, DJUI_VALIGN_BOTTOM);
    djui_base_set_size_type(&sPrevButton->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&sPrevButton->base, 128, 32);
    djui_base_set_enabled(&sPrevButton->base, false);
    paginated->prevButton = sPrevButton;

    sPageNumText = djui_text_create(&paginated->base, "");
    djui_base_set_color(&sPageNumText->base, 200, 200, 200, 255);
    djui_base_set_alignment(&sPageNumText->base, DJUI_HALIGN_CENTER, DJUI_VALIGN_BOTTOM);
    sPageNumText->base.y.value -= 30;

    sNextButton = djui_button_create(&paginated->base, ">", DJUI_BUTTON_STYLE_NORMAL, djui_paginated_next);
    djui_base_set_alignment(&sNextButton->base, DJUI_HALIGN_RIGHT, DJUI_VALIGN_BOTTOM);
    djui_base_set_size_type(&sNextButton->base, DJUI_SVT_ABSOLUTE, DJUI_SVT_ABSOLUTE);
    djui_base_set_size(&sNextButton->base, 128, 32);
    paginated->nextButton = sNextButton;

    return paginated;
}
