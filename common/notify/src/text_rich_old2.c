#include "common.h"

#include "memory/memory.h"
#include "notify/text.h"

#include "base/panic.h"

#include "core/macros.h"
#include "notify/notify.h"

#include "io/io.h"

#include "scan/node.h"
#include "std/list.h"
#include "std/map.h"
#include "std/str.h"

#include "std/typed/vector.h"
#include "std/vector.h"

#include <stdlib.h>
#include <string.h>

typedef enum ui_element_type_t
{
    /// @brief a box that contains text
    eElementText,

    /// @brief a line either horizontal or vertical
    eElementLine,

    eElementCount
} ui_element_type_t;

typedef struct ui_text_t
{
    /// @brief the text to display
    const char *text;

    /// @brief the width of the box
    /// must be at least the length of the text
    size_t width;

    /// @brief the height of the box
    size_t height;

    /// @brief the line number of this text
    line_t line;
} ui_text_t;

typedef enum ui_line_direction_t
{
    eLineHorizontal,
    eLineVertical,

    eLineDirectionCount
} ui_line_direction_t;

typedef struct ui_line_t
{
    /// @brief the length of the line
    size_t length;

    /// @brief the direction of the line
    ui_line_direction_t direction;
} ui_line_t;

typedef struct ui_element_t
{
    ui_element_type_t type;

    // start position. top left corner of aabb box
    size_t x;
    size_t y;

    union {
        ui_text_t text;
        ui_line_t line;
    };
} ui_element_t;

typedef struct overlap_t
{
    // normal of the overlap
    size_t normal_x;
    size_t normal_y;

    // depth of the overlap
    size_t overlap;
} overlap_t;

typedef struct collide_t
{
    ui_element_t *a;
    ui_element_t *b;

    size_t normal_x;
    size_t normal_y;

    size_t overlap;
} collide_t;

static bool collide_lines(overlap_t *result, ui_element_t *a, ui_element_t *b)
{
    CTASSERT(a->type == eElementLine);
    CTASSERT(b->type == eElementLine);

    // lines can only collide if they are parallel

    ui_line_t a_line = a->line;
    ui_line_t b_line = b->line;

    if (a_line.direction != b_line.direction)
    {
        return false;
    }

    // lines can only collide if they are on the same axis

    if (a_line.direction == eLineHorizontal)
    {
        // lines can only collide if they are on the same y axis

        if (a->y != b->y)
        {
            return false;
        }

        // lines can only collide if they overlap on the x axis

        size_t a_start = a->x;
        size_t a_end = a_start + a_line.length;

        size_t b_start = b->x;
        size_t b_end = b_start + b_line.length;

        if (a_start > b_end || b_start > a_end)
        {
            return false;
        }

        // lines overlap on the x axis

        size_t overlap = MIN(a_end, b_end) - MAX(a_start, b_start);

        result->normal_x = 0;
        result->normal_y = 1;
        result->overlap = overlap;

        return true;
    }
    else
    {
        // lines can only collide if they are on the same x axis

        if (a->x != b->x)
        {
            return false;
        }

        // lines can only collide if they overlap on the y axis

        size_t a_start = a->y;
        size_t a_end = a_start + a_line.length;

        size_t b_start = b->y;
        size_t b_end = b_start + b_line.length;

        if (a_start > b_end || b_start > a_end)
        {
            return false;
        }

        // lines overlap on the y axis

        size_t overlap = MIN(a_end, b_end) - MAX(a_start, b_start);

        result->normal_x = 1;
        result->normal_y = 0;
        result->overlap = overlap;

        return true;
    }

    return false;
}

static bool collide_rects(overlap_t *result, ui_element_t *a, ui_element_t *b)
{
    CTASSERT(a->type == eElementText);
    CTASSERT(b->type == eElementText);

    return false;
}

static bool collide_line_rect(overlap_t *result, ui_element_t *a, ui_element_t *b)
{
    CTASSERT(a->type == eElementLine);
    CTASSERT(b->type == eElementText);

    return false;
}

static bool collide_rect_line(overlap_t *result, ui_element_t *a, ui_element_t *b)
{
    return collide_line_rect(result, b, a);
}

typedef bool (*collide_fn_t)(overlap_t *result, ui_element_t *a, ui_element_t *b);

static const collide_fn_t kCollideFn[eElementCount][eElementCount] = {
    [eElementText] = {
        [eElementText] = collide_rects,
        [eElementLine] = collide_rect_line
    },
    [eElementLine] = {
        [eElementText] = collide_line_rect,
        [eElementLine] = collide_lines
    }
};

static bool collide_elements(overlap_t *result, ui_element_t *a, ui_element_t *b)
{
    CTASSERT(a != NULL);
    CTASSERT(b != NULL);

    CTASSERT(a->type < eElementCount);
    CTASSERT(b->type < eElementCount);

    collide_fn_t fn = kCollideFn[a->type][b->type];

    return fn(result, a, b);
}

typedef struct ui_layout_t
{
    typevec_t *elements;

    typevec_t *manifolds;
} ui_layout_t;

static ui_element_t *ui_text(size_t x, size_t y, const char *text, line_t line)
{
    CTASSERT(text != NULL);

    ui_text_t body = {
        .text = text,
        .width = strlen(text),
        .height = 1,
        .line = line
    };

    ui_element_t *elem = MEM_ALLOC(sizeof(ui_element_t), "ui_text", NULL);
    elem->type = eElementText;
    elem->x = x;
    elem->y = y;
    elem->text = body;

    return elem;
}

static ui_element_t *ui_line(size_t x, size_t y, size_t length, ui_line_direction_t direction)
{
    ui_line_t body = {
        .length = length,
        .direction = direction
    };

    ui_element_t *elem = MEM_ALLOC(sizeof(ui_element_t), "ui_line", NULL);
    elem->type = eElementLine;
    elem->x = x;
    elem->y = y;
    elem->line = body;

    return elem;
}

static bool layout_collide(ui_layout_t *layout)
{
    CTASSERT(layout != NULL);

    size_t len = typevec_len(layout->elements);

    bool dirty = false;

    for (size_t i = 0; i < len; i++)
    {
        ui_element_t *a = typevec_offset(layout->elements, i);

        for (size_t j = i + 1; j < len; j++)
        {
            ui_element_t *b = typevec_offset(layout->elements, j);

            overlap_t overlap = { 0 };

            if (!collide_elements(&overlap, a, b)) {
                continue;
            }

            collide_t collide = {
                .a = a,
                .b = b,
                .normal_x = overlap.normal_x,
                .normal_y = overlap.normal_y,
                .overlap = overlap.overlap
            };

            typevec_push(layout->manifolds, &collide);

            dirty = true;
        }
    }

    return dirty;
}

static bool layout_resolve(ui_layout_t *layout)
{
    CTASSERT(layout != NULL);

    size_t len = typevec_len(layout->manifolds);

    for (size_t i = 0; i < len; i++)
    {
        collide_t *collide = typevec_offset(layout->manifolds, i);

        ui_element_t *a = collide->a;
        ui_element_t *b = collide->b;

        size_t normal_x = collide->normal_x;
        size_t normal_y = collide->normal_y;

        size_t depth = collide->overlap;

        CTASSERTF(normal_x != 0 || normal_y != 0, "overlap with no normal (%zu, %zu)", normal_x, normal_y);
        CTASSERTF(depth != 0, "overlap with no depth (%zu, %zu)", normal_x, normal_y);

    }

    return len > 0;
}

static bool layout_integrate(ui_layout_t *layout)
{
    CTASSERT(layout != NULL);

    if (!layout_collide(layout))
    {
        return false;
    }

    if (!layout_resolve(layout))
    {
        return false;
    }

    return true;
}

static void ui_add_element(ui_layout_t *layout, ui_element_t *elem)
{
    CTASSERT(layout != NULL);
    CTASSERT(elem != NULL);

    typevec_push(layout->elements, elem);
}

void print_file_contents(text_config_t config, sparse_text_t *text)
{
    CTU_UNUSED(config);

    CTASSERT(text != NULL);

    size_t count = sparse_text_count(text);

    ui_layout_t layout = {
        .elements = typevec_new(sizeof(ui_element_t), count),
        .manifolds = typevec_new(sizeof(collide_t), 4)
    };
}

void print_event_header(text_config_t config, const event_t *event)
{
    const diagnostic_t *diagnostic = event->diagnostic;
    const char *id = diagnostic->id;
    const char *sev = get_severity_name(diagnostic->severity);
    colour_t colour = get_severity_colour(diagnostic->severity);
    const char *lvl = fmt_coloured(config.colours, colour, "%s [%s]:", sev, id);

    io_printf(config.io, "%s %s\n", lvl, event->message);
}

void text_report_rich(text_config_t config, const event_t *event)
{
    CTASSERT(config.io != NULL);
    CTASSERT(event != NULL);

    sparse_report_t *report = sparse_report_new(event);
    vector_t *files = sparse_report_get_files(report);
    size_t len = vector_len(files);

    print_event_header(config, event);

    for (size_t i = 0; i < len; i++)
    {
        sparse_text_t *text = vector_get(files, i);
        print_file_contents(config, text);
    }
}
